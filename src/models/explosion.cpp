/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "explosion.h"

Explosion::Explosion(irr::core::vector3df targetLoc, ExplosionLauncher* parentExpLauncher) {
    this->targetLocation = targetLoc;
    this->mParentExplosionLauncher = parentExpLauncher;

    irr::f32 randVal = DEF_EXPLOSION_COORDINATE_RNDVAL;
    irr::f32 randVal2 = DEF_EXPLOSION_DETONATION_DELAY_RNDVAL;
    irr::f32 randVal3 = DEF_EXPLOSION_DETONATION_DELAY_DEBRIS_RNDVAL;
    irr::f32 rNumFloat1;
    irr::f32 rNumFloat2;
    irr::f32 rNumFloat3;
    irr::f32 rNumFloat4;
    irr::f32 rNumFloat5;
    int rNum;

    //create all detonations within this explosion
    for (int i = 0; i < DEF_EXPLOSION_NRDETONATIONS; i++) {
        ExplosionDetonationStruct* newDetonation = new ExplosionDetonationStruct();

        newDetonation->mSceneNodeDetonation = mParentExplosionLauncher->mSmgr->addBillboardSceneNode();
        newDetonation->mSceneNodeDetonation->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
        newDetonation->mSceneNodeDetonation->setMaterialFlag(irr::video::EMF_LIGHTING, true);
        newDetonation->mSceneNodeDetonation->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

        //newDetonation->mSceneNodeDetonation->setDebugDataVisible(irr::scene::EDS_BBOX);

        //lets randomize the location of the detonation a bit
        rNum = rand();
        rNumFloat1 = -randVal * 0.5f + (float(rNum) / float (RAND_MAX))  * randVal;

        rNum = rand();
        rNumFloat2 = (float(rNum) / float (RAND_MAX))  * randVal * 0.5f;

        rNum = rand();
        rNumFloat3 = -randVal * 0.5f + (float(rNum) / float (RAND_MAX))  * randVal;

        newDetonation->currDetonationPos = targetLoc + irr::core::vector3df(rNumFloat1, rNumFloat2, rNumFloat3);

        //also randomize the size slightly
        rNum = rand();
        rNumFloat3 = 0.5f + (float(rNum) / float (RAND_MAX))  * 1.0f;

        newDetonation->mInitSize.set(rNumFloat3, rNumFloat3);

        //set random detonation delay
        rNum = rand();
        rNumFloat1 = (float(rNum) / float (RAND_MAX))  * randVal2;

        newDetonation->detonationDelay = rNumFloat1;

        newDetonation->mSceneNodeDetonation->setPosition(newDetonation->currDetonationPos);
        newDetonation->mSceneNodeDetonation->setSize(newDetonation->mInitSize);

        newDetonation->mSceneNodeDetonation->setVisible(false);

        mExplosionSpriteVec.push_back(newDetonation);
    }

    irr::f32 timer = DEF_EXPLOSION_DEBRIS_DISTANCENEXTSPRITE;
    irr::f32 lifeTime = DEF_EXPLOSION_DEBRISSPRITELIFETIME;

    //create all flying debris for this explosion
    for (int i = 0; i < DEF_EXPLOSION_NRDEBRIS; i++) {
        ExplosionFlyingDebrisStruct* newDebris = new ExplosionFlyingDebrisStruct();

        //derive a random flying velocity direction vector for the debris
        rNum = rand();
        rNumFloat1 = 0.5f + (float(rNum) / float (RAND_MAX))  * 0.5f;

        rNum = rand();
        rNumFloat2 = 0.5f + (float(rNum) / float (RAND_MAX))  * 0.5f;

        rNum = rand();
        if (float(rNum) > (float (RAND_MAX) * 0.5f)) {
            rNumFloat1 = -rNumFloat1;
        }

        rNum = rand();
        if (float(rNum) > (float (RAND_MAX) * 0.5f)) {
            rNumFloat2 = -rNumFloat2;
        }

        rNum = rand();
        //velocity for Y axis only upwards, not that the object is shoot into the ground
        rNumFloat3 = 1.0f + (float(rNum) / float (RAND_MAX))  * 2.0f;

        newDebris->mVelocity.X = rNumFloat1;
        newDebris->mVelocity.Y = rNumFloat3;
        newDebris->mVelocity.Z = rNumFloat2;

        //derive random start location for shooting the debris out
        rNum = rand();
        rNumFloat1 = -randVal * 0.5f + (float(rNum) / float (RAND_MAX))  * randVal;

        rNum = rand();
        rNumFloat2 = (float(rNum) / float (RAND_MAX))  * randVal * 0.5f;

        rNum = rand();
        rNumFloat3 = -randVal * 0.5f + (float(rNum) / float (RAND_MAX))  * randVal;

        //also randomize the size slightly
        //rNum = rand();
        //rNumFloat4 = 0.5f + (float(rNum) / float (RAND_MAX))  * 1.0f;

        //first sprite (glowing ball for missile) should be only half the size
        rNumFloat4 = DEF_EXPLOSION_DEBRIS_SPRITESIZE * 0.5f;

        irr::core::dimension2d<irr::f32> newSpriteSize(rNumFloat4, rNumFloat4);

        //derive a random speed when we shoot the object out
        //let all debris sprites use the same velocity
        rNum = rand();
        rNumFloat5 =  5.0f + (float(rNum) / float (RAND_MAX))  * 10.0f;

        newDebris->mVelocity = rNumFloat5 * newDebris->mVelocity.normalize();

        //We need to calculate timer for adding sprites according to velocity of debris
        //so that independent of the speed we get the same distance between the sprites
        newDebris->timerAddNextSprite = (timer / newDebris->mVelocity.getLength());

        //remember spawn point where the debris is coming from
        newDebris->spawnPoint = targetLoc + irr::core::vector3df(rNumFloat1, rNumFloat2, rNumFloat3);

        //add first sprite to debris
        AddNewSpriteToDebris(*newDebris, newDebris->spawnPoint, newDebris->mVelocity, newSpriteSize, lifeTime);

        //set random detonation delay
        rNum = rand();
        rNumFloat1 = (float(rNum) / float (RAND_MAX))  * randVal3;

        newDebris->detonationDelay = rNumFloat1;

        this->mExplosionDebrisVec.push_back(newDebris);
    }

    exploding = true;
}

Explosion::~Explosion() {
}

void Explosion::AddNewSpriteToDebris(ExplosionFlyingDebrisStruct &debrisPntr, irr::core::vector3df newLocation,
                                     irr::core::vector3df currVel, irr::core::dimension2d<irr::f32> newSpriteSize, irr::f32 lifeTime) {
    irr::scene::IBillboardSceneNode* newSceneNode = mParentExplosionLauncher->mSmgr->addBillboardSceneNode();

    newSceneNode->setMaterialTexture(0, mParentExplosionLauncher->animTexList[debrisPntr.currTexNrFlyingDebris]);

    //with every sprite we add to the flying debris we want to use the next texture in
    //the explosion texture order; the game seems to do the same, the last texture is a cloud
    //texture we just keep repeating; Increase index of used texture until we can not increase it anymore
    //because we reached already the cloud sprite
    //first texture is the glowing ball for the missile itself
    if (debrisPntr.currTexNrFlyingDebris < (mParentExplosionLauncher->animTexList.size() - 1)) {
        debrisPntr.currTexNrFlyingDebris++;
    }

    newSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
    newSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, true);
    newSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    //newSceneNode->setDebugDataVisible(irr::scene::EDS_BBOX);

    newSceneNode->setPosition(newLocation);
    newSceneNode->setSize(newSpriteSize);

    //first hide new sprite
    newSceneNode->setVisible(false);

    debrisPntr.mSceneNodeDebrisSprite.push_back(newSceneNode);
    debrisPntr.currVelocity.push_back(currVel);
    debrisPntr.currSpritePos.push_back(newLocation);
    debrisPntr.mCurrSpriteSize.push_back(newSpriteSize);
    debrisPntr.remainingLifeTime.push_back(lifeTime);
}

void Explosion::UpdateDetonations(irr::f32 DeltaTime) {
    //cycle through all of my detonations
    std::vector<ExplosionDetonationStruct*>::iterator it;
    ExplosionDetonationStruct* pntr;

    //first clean up detonations that are already done (happened already)
    for (it = this->mExplosionSpriteVec.begin(); it != this->mExplosionSpriteVec.end();) {
          if ((*it)->detonated) {
            pntr = (*it);
            //we need to clean this one up
            it = mExplosionSpriteVec.erase(it);

            //get rid of the sceneNode
            pntr->mSceneNodeDetonation->remove();

            //delete the struct itself
            delete pntr;
          } else ++it;
    }

    //now update all the remaining explosion objects
    for (it = this->mExplosionSpriteVec.begin(); it != this->mExplosionSpriteVec.end(); ++it) {

        if (!(*it)->detonated) {

            //first trigger explosion animation when delayDetonation time has passed
            if (!(*it)->currDetonating) {
                (*it)->detonationDelay -= DeltaTime;
                if ((*it)->detonationDelay < 0.0f) {
                    //delay is over, trigger detonation

                    (*it)->currDetonating = true;
                    (*it)->mSceneNodeDetonation->setVisible(true);
                    (*it)->animator = mParentExplosionLauncher->mSmgr->createTextureAnimator(mParentExplosionLauncher->animTexList, 50, false);
                    (*it)->mSceneNodeDetonation->addAnimator((*it)->animator);
                }
            } else {
                //detonations occurs right now
                //let detonations fly outwards and upwards
                //(*it)->currDetonationPos.Y += 0.02f;

                (*it)->mSceneNodeDetonation->setPosition((*it)->currDetonationPos);

                if ((*it)->animator->hasFinished()) {
                    (*it)->animator->drop();
                    (*it)->currDetonating = false;
                    (*it)->detonated = true;
                    (*it)->mSceneNodeDetonation->setVisible(false);
                }
            }
        } else {
            //detonation happend already, we are done, cleanup up detonation again

        }
    }
}

void Explosion::UpdateDebris(irr::f32 DeltaTime) {
    //cycle through all of my debris
    std::vector<ExplosionFlyingDebrisStruct*>::iterator it;
    ExplosionFlyingDebrisStruct* pntr;
    std::vector<irr::scene::IBillboardSceneNode*>::iterator it2;
    irr::scene::IBillboardSceneNode* pntr2;

    //first clean up debris that is not visible anymore (happened already)
    for (it = this->mExplosionDebrisVec.begin(); it != this->mExplosionDebrisVec.end();) {
          if ((*it)->detonated) {
            pntr = (*it);
            //we need to clean this one up
            it = mExplosionDebrisVec.erase(it);

            //get rid of all of the sceneNodes inside this debris
            for (it2 = pntr->mSceneNodeDebrisSprite.begin(); it2 != pntr->mSceneNodeDebrisSprite.end(); ++it2) {
                pntr2 = (*it2);

                it2 = pntr->mSceneNodeDebrisSprite.erase(it2);

                //remove the sceneNode
                pntr2->remove();
            }

            //delete the struct itself
            delete pntr;
          } else ++it;
    }

    int idx;
    irr::f32 rNumFloat4 = DEF_EXPLOSION_DEBRIS_SPRITESIZE;
    irr::f32 lifeTime = DEF_EXPLOSION_DEBRISSPRITELIFETIME;
    bool spriteRemaining = false;
    irr::f32 terrainHeight;
    //int rNum;

    int current_cell_calc_x, current_cell_calc_y;

    //now update all the remaining explosion objects
    for (it = this->mExplosionDebrisVec.begin(); it != this->mExplosionDebrisVec.end(); ++it) {

        if (!(*it)->detonated) {

            //first shoot debris away in predefined direction after delayDetonation time has passed
            if (!(*it)->currDetonating) {
                (*it)->detonationDelay -= DeltaTime;
                if ((*it)->detonationDelay < 0.0f) {
                    //delay is over, shoot debris away

                    (*it)->currDetonating = true;

                    //make all sprites visible
                    for (it2 = (*it)->mSceneNodeDebrisSprite.begin(); it2 != (*it)->mSceneNodeDebrisSprite.end(); ++it2) {
                        (*it2)->setVisible(true);
                    }
                   }
            } else {
                //debris is flying right now, update position of all existing sprites
                idx = 0;
                for (it2 = (*it)->mSceneNodeDebrisSprite.begin(); it2 != (*it)->mSceneNodeDebrisSprite.end(); ++it2) {
                    (*it)->currSpritePos[idx] = (*it)->currSpritePos[idx] + (*it)->currVelocity[idx] * DeltaTime;
                    (*it)->currVelocity[idx] = (*it)->currVelocity[idx] + this->mParentExplosionLauncher->mParentRace->mPhysics->mGravityVec * DeltaTime;

                    (*it2)->setPosition((*it)->currSpritePos[idx]);

                    //check if sprite is currently moving towards ground, and is very close to race track ground (hits the ground)
                    //in this case let the debris sprite disappear
                    //only check more if the sprite is currently falling towards the race track
                    if ((*it)->currVelocity[idx].Y < 0.0f) {
                        //yes, sprite is falling down, now we need to calculate high about terrain tile below
                        //calculate current cell below sprite
                        current_cell_calc_y = ((*it)->currSpritePos[idx].Z / mParentExplosionLauncher->mParentRace->mLevelTerrain->segmentSize);
                        current_cell_calc_x = -((*it)->currSpritePos[idx].X / mParentExplosionLauncher->mParentRace->mLevelTerrain->segmentSize);

                        MapEntry* mEntry = mParentExplosionLauncher->mParentRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);

                        //is there actually an entry?
                        if (mEntry != NULL) {
                             terrainHeight = mParentExplosionLauncher->mParentRace->mLevelTerrain->pTerrainTiles[mEntry->get_X()][mEntry->get_Z()].currTileHeight;

                             //sprite to close to terrain, if so stop the sprite to continue further
                             if (((*it)->currSpritePos[idx].Y - terrainHeight) < 0.3f) {
                                 //to close to terrain, hide sprite
                                 (*it2)->setVisible(false);
                             }
                        } else {
                            //we did not find a valid entry, end sprite here
                            (*it2)->setVisible(false);
                        }
                    }

                    //lifetime left for current sprite?
                    (*it)->remainingLifeTime[idx] -= DeltaTime;

                    if ((*it)->remainingLifeTime[idx] < 0.0f) {
                        //lifetime over, hide sprite
                        (*it2)->setVisible(false);
                    } else {
                        spriteRemaining = true;
                    }

                    idx++;
                }

                //time to add next sprite to flying debris?
                (*it)->timerAddNextSprite -= DeltaTime;

                if ((*it)->timerAddNextSprite < 0.0f) {
                    irr::f32 timer = DEF_EXPLOSION_DEBRIS_DISTANCENEXTSPRITE;

                    //We need to calculate timer for adding sprites according to velocity of debris
                    //so that independent of the speed we get the same distance between the sprites
                    (*it)->timerAddNextSprite = (timer / (*it)->mVelocity.getLength());

                    //can add another sprite, or already enough?
                    if ((*it)->mSceneNodeDebrisSprite.size() < DEF_EXPLOSION_NRDEBRISSPRITES) {

                        irr::core::dimension2d<irr::f32> newSpriteSize;

                        //also randomize the size slightly
                        //rNum = rand();
                        //rNumFloat4 = 0.5f + (float(rNum) / float (RAND_MAX))  * 1.0f;

                        newSpriteSize.set(rNumFloat4, rNumFloat4);

                        //add another sprite to this flying debris, at the stored location where before the last position
                        //update the last sprite was located at
                        AddNewSpriteToDebris(*(*it), (*it)->spawnPoint, (*it)->mVelocity, newSpriteSize, lifeTime);

                        //make the new sprite visible
                        (*it)->mSceneNodeDebrisSprite.at((*it)->mSceneNodeDebrisSprite.size() - 1)->setVisible(true);

                        spriteRemaining = true;
                    }
                }

                //all sprites inactive, can we remove this explosion debris?
                if (!spriteRemaining) {
                    //set detonated, during the next update all of this debris
                    //will be removed
                    (*it)->detonated = true;
                }

               /* if ((*it)->animator->hasFinished()) {
                    (*it)->animator->drop();
                    (*it)->currDetonating = false;
                    (*it)->detonated = true;
                    (*it)->mSceneNodeDetonation->setVisible(false);
                }*/
            }
        } else {
            //shooting debris happend already, we are done, cleanup up flying debris again

        }
    }

}

//Returns true in case of success
//False otherwise
bool ExplosionLauncher::LoadSprites() {
    //load the explosion sprites

    ITexture* newTex;
    char fileName[50];
    char fname[20];

    //there are 12 explosion sprites to be loaded
    //for the explosion animation
    for (long i = 0; i < 13; i++) {

        strcpy(fileName, "extract/sprites/tmaps");
        sprintf (fname, "%0*lu.png", 4, i);
        strcat(fileName, fname);

        newTex = mDriver->getTexture(fileName);

        if (newTex == NULL)
            return false;

        animTexList.push_back(newTex);
    }

    return true;
}

void ExplosionLauncher::Trigger(irr::core::vector3df targetLocation) { 

    Explosion *newExplosion = new Explosion(targetLocation, this);

    this->mCurrentExplosionVec.push_back(newExplosion);
}

void ExplosionLauncher::Update(irr::f32 DeltaTime) {

   std::vector <Explosion*>::iterator it;

   if (mCurrentExplosionVec .size() > 0 ) {

    //search for missiles that have exploded already and should be deleted
    //erase this missile from my list of current missiles vector
   /* for (it = this->mCurrentMissilesVec.begin(); it != this->mCurrentMissilesVec.end();) {
       if ((*it)->objToBeDeleted) {
            mPntr = (*it);
            it = mCurrentMissilesVec.erase(it);

            //delete the missile object itself
            delete mPntr;
            mPntr = NULL;
       } else ++it;
    }*/
   }

   if (mCurrentExplosionVec.size() > 0 ) {
     //update all my current active explosions
    for (it = this->mCurrentExplosionVec.begin(); it != this->mCurrentExplosionVec.end(); ++it) {
        (*it)->UpdateDetonations(DeltaTime);
        (*it)->UpdateDebris(DeltaTime);
    }
   }

    /*if (mShotSound != NULL) {
        if (mShotSound->getStatus() == mShotSound->Stopped) {
            mShotSound = NULL;
        }
    }*/
}

ExplosionLauncher::ExplosionLauncher(Race* myParentRace, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver *driver) {
    mParentRace = myParentRace;
    mSmgr = smgr;
    mDriver = driver;

    ready = true;

    if (!LoadSprites()) {
        ready = false;
    }
}
