/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "missile.h"
#include "../utils/physics.h"
#include "../audio/sound.h"
#include "../models/levelterrain.h"
#include "../resources/texture.h"
#include "../utils/ray.h"
#include "explauncher.h"
#include "../race.h"
#include "../resources/mapentry.h"

Missile::Missile(MissileLauncher* mParentLauncher, irr::core::vector3df launchLoc, irr::core::vector3df targetLoc,
                 bool targetLocked, Player* lockedPlayer) {
    this->mParentLauncher = mParentLauncher;
    this->launchLocation = launchLoc;
    this->targetLocation = targetLoc;

    mSceneNodeMissile = mParentLauncher->mSmgr->addBillboardSceneNode();
    mSceneNodeMissile->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
    mSceneNodeMissile->setMaterialTexture(0, this->mParentLauncher->mMissileTex);
    mSceneNodeMissile->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    mSceneNodeMissile->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    //mSceneNodeMissile->setDebugDataVisible(irr::scene::EDS_BBOX);
    mSceneNodeMissile->setDebugDataVisible(irr::scene::EDS_OFF);

    //I want a random number between 0.8f and 1.2f
    //int rNum = rand();
    //irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX))  * 0.4f;

    irr::core::dimension2d<irr::f32> mInitSize(0.2f, 0.2f);

    mSceneNodeMissile->setPosition(launchLoc);
    mSceneNodeMissile->setSize(mInitSize);
    currentLocation = launchLoc;
    travelDir = (targetLoc - launchLoc).normalize();

    //targetLocked tells us if at the time of missile shooting (creation)
    //the player that shoot the missile had a lock (red marking) on the
    //opponent player, as long as the player locks at the target let the missile
    //adjust its target location to the location of the enemy player
    this->targetStillLocked = targetLocked;
    this->mLockedPlayer = lockedPlayer;
}

Missile::~Missile() {
    //cleanup any remaining Scenenodes
    std::vector<MissileSpriteStruct*>::iterator it;
    MissileSpriteStruct* pntr;

    for (it = mMissileSpriteVec.begin(); it != mMissileSpriteVec.end();) {
        pntr = (*it);

        it = mMissileSpriteVec.erase(it);

        RemoveSmokeSprite(pntr);
    }

    if (mSceneNodeMissile != nullptr) {
        mSceneNodeMissile->remove();
    }
}

void Missile::AddNewSmokeSprite(irr::core::vector3df newLocation) {
    irr::scene::ISceneManager *mSmgr = this->mParentLauncher->mSmgr;

    MissileSpriteStruct* newSprite = new MissileSpriteStruct();

    newSprite->mSceneNode = mSmgr->addBillboardSceneNode();
    newSprite->mSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
    newSprite->mSceneNode->setMaterialTexture(0, this->mParentLauncher->mSmokeTex);
    newSprite->mSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    newSprite->mSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    //first the sprite is fully visible
    newSprite->mCurrVerticeColor.set(255, 255, 255, 255);

    //newSprite->mSceneNode->setDebugDataVisible(irr::scene::EDS_BBOX);

    //I want a random number between 0.8f and 1.2f
    //int rNum = rand();
    //irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX))  * 0.4f;

    newSprite->mCurrSpriteSize.set(DEF_MISSILE_SMOKESPRITEINITSIZE, DEF_MISSILE_SMOKESPRITEINITSIZE);

    newSprite->mSceneNode->setPosition(newLocation);
    newSprite->mSceneNode->setColor(newSprite->mCurrVerticeColor);
    newSprite->mSpritePos = newLocation;
    newSprite->mSceneNode->setSize(newSprite->mCurrSpriteSize);
    newSprite->lifeTimeSecParam = DEF_MISSILE_SMOKESPRITELIFETIMESEC;

    this->mMissileSpriteVec.push_back(newSprite);
}

void Missile::RemoveSmokeSprite(MissileSpriteStruct* spriteToRemove) {
    //first get pointer to sceneNode
    irr::scene::ISceneNode *node = spriteToRemove->mSceneNode;

    node->setVisible(false);
    node->remove();
}

bool Missile::AreAllSmokeSpritesGone() {
    return (this->mMissileSpriteVec.size() == 0);
}

void Missile::UpdateSmokeSprites(irr::f32 DeltaTime) {
   //only add new smoke sprites to smoke trail if not yet exploded
   if (!exploded) {
        this->remainingDistToNextSmoke -= DeltaTime * DEF_MISSILE_SPEED;

        //add a new missile smoke cloud sprite in the trail?
        if (remainingDistToNextSmoke < 0.0f) {
            remainingDistToNextSmoke = DEF_MISSILE_SMOKEDIST;

            //add the new smoke sprite at the current location of the missile
            //I mean the location of the glowing ball that should ressemble the flying
            //missile
            AddNewSmokeSprite(this->currentLocation);
        }
   }

   //01.05.2025: for the calculations below we need to take
   //the current frame rate into consideration!
   irr::f32 speedFactor = (DeltaTime / (irr::f32)(1.0f / 60.0f));

   //now update all currently existing smoke sprites of the missile
   std::vector<MissileSpriteStruct*>::iterator it;
   MissileSpriteStruct* pntr;

   int rNum;

   for (it = mMissileSpriteVec.begin(); it != mMissileSpriteVec.end();) {
        (*it)->lifeTimeSecParam -= DeltaTime;

       irr::f32 lifeTimeAlphaVal = ((DEF_MISSILE_SMOKESPRITELIFETIMESEC - (*it)->lifeTimeSecParam) / DEF_MISSILE_SMOKESPRITELIFETIMESEC);

       if (lifeTimeAlphaVal > 1.0f)
           lifeTimeAlphaVal = 1.0f;

       if (lifeTimeAlphaVal < 0.0f)
           lifeTimeAlphaVal = 0.0f;

       //at the end of the lifetime fade the smoke out
       if (lifeTimeAlphaVal > 0.5f) {
           irr::u8 fadeVal = (irr::u8)(255.0f * (1.0f - (lifeTimeAlphaVal - 0.5f)/0.5f));

           //if we want to fade out (make the billBoardSceneNode more transparent)
           //we just need to set the Vertices colors more black
           (*it)->mCurrVerticeColor.set(255, fadeVal, fadeVal, fadeVal);
           (*it)->mSceneNode->setColor((*it)->mCurrVerticeColor);
       }

        //lifetime of smoke sprite over?
        if ((*it)->lifeTimeSecParam < 0.0f) {
                    pntr = (*it);
                    it = mMissileSpriteVec.erase(it);

                    RemoveSmokeSprite(pntr);
                } else {
                   //lifetime not yet over
                   (*it)->mCurrSpriteSize.Width += 0.02f * speedFactor;
                   (*it)->mCurrSpriteSize.Height += 0.02f * speedFactor;
                   (*it)->mSceneNode->setSize((*it)->mCurrSpriteSize);

                   //let smoke sprites float upwards slowly
                   //I want a random number between 0.04f and 0.08f
                   rNum = rand();
                   irr::f32 rNumFloat = 0.04f + (float(rNum) / float (RAND_MAX))  * 0.04f;

                   (*it)->mSpritePos += irr::core::vector3df(0.0f, rNumFloat * speedFactor, 0.0f);

                   (*it)->mSceneNode->setPosition((*it)->mSpritePos);

                   rNum = rand();
                   rNumFloat = 0.01f + (float(rNum) / float (RAND_MAX))  * 0.01f;

                   (*it)->mCurrSpriteSize = (*it)->mCurrSpriteSize * irr::core::vector2df(1.0f + rNumFloat * speedFactor, 1.0f + rNumFloat * speedFactor);

                   (*it)->mSceneNode->setSize((*it)->mCurrSpriteSize);

                   ++it;
        }
   }
}

void Missile::Update(irr::f32 DeltaTime) {
    //first update all smoke sprites, and create new ones if
    //necessary
    UpdateSmokeSprites(DeltaTime);

    if (exploded) {
        //we are already exploded and all smoke sprites disappeared

        //if soundEngine is active report missile only as exploded
        //after explision sound was finished playing
        if (mParentLauncher->mParent->mRace->mSoundEngine->GetIsSoundActive()) {
            if (mExplodeSound != nullptr) {
                if (mExplodeSound->getStatus() == mExplodeSound->Stopped) {
                    mExplodeSound = nullptr;
                }
            } else {
                if (AreAllSmokeSpritesGone()) {
                   //mark that object (missile) can be deleted
                   objToBeDeleted = true;
                }
            }
        } else {
            //no sound is playing, report missile as exploded immediately
            //mark that object (missile) can be deleted
            if (AreAllSmokeSpritesGone()) {
                objToBeDeleted = true;
            }
        }

        return;

    } else {
        //missile not exploded yet, continue flight
        flightTime += DeltaTime;

        currentLocation += travelDir * DEF_MISSILE_SPEED * DeltaTime;

        //if this missile is still locked to the enemy player, update our missile direction
        if (this->targetStillLocked) {
            if (this->mLockedPlayer != nullptr) {
                //we have a locked enemy player
                //first make sure that the player that shoot me still
                //has the same player locked, if not I need to loose my lock
                //of the target
                if ((this->mParentLauncher->mParent->mTargetPlayer != this->mLockedPlayer) ||
                    (!this->mParentLauncher->mParent->mTargetMissleLock)) {
                    //we loose the lock! but for dealing damage later we still keep the reference to the
                    //initially targeted player as we can still hit him!
                    this->targetStillLocked = false;
                } else {
                    //we still have the target lock, update missile target position
                    targetLocation = this->mParentLauncher->mParent->mTargetPlayer->phobj->physicState.position;
                    travelDir = (targetLocation - currentLocation).normalize();
                }

            } else {
                //we have no locked player, for whatever reason
                this->targetStillLocked = false;
            }
        }

        if ((currentLocation - targetLocation).getLengthSQ() < 2.0f) {
            //we reached the target location
            //simply explode, let the missile disappear
            mSceneNodeMissile->setVisible(false);
            mSceneNodeMissile->remove();
            mSceneNodeMissile = nullptr;

            //did the missile explode close to the target player, if so deal
            //damage to the targeted enemy player
            if (this->mLockedPlayer != nullptr) {
                irr::f32 maxDealDamageRange = DEF_MISSILE_DEALDAMAGE_DISTRANGE;
                if ((currentLocation - this->mLockedPlayer->phobj->physicState.position).getLength() < maxDealDamageRange) {
                    //in the original game it needs 7 missiles at a player with initial full shield to
                    //break this player down with a destroyed craft
                    irr::f32 oneMissileDamage = this->mLockedPlayer->mPlayerStats->shieldMax / 7.0f;
                    //this->mLockedPlayer->Damage(oneMissileDamage);
                    mParentLauncher->mParent->mRace->DamagePlayer(this->mLockedPlayer, oneMissileDamage,
                                                                  DEF_RACE_DAMAGETYPE_MISSILE, mParentLauncher->mParent);
                }
            }

            //did we hit a missile trigger entity?
            CheckForHitOfMissileTrigger(currentLocation);

            //create the explosion at the target
            this->mParentLauncher->mParent->mRace->mExplosionLauncher->Trigger(currentLocation);

            mExplodeSound = mParentLauncher->mParent->mRace->mSoundEngine->PlaySound(SRES_GAME_EXPLODE, currentLocation, false);
            exploded = true;

            //missile exploded, need to exit here
            //because otherwise we get an issue with the source code
            //below, when we try to update position of missile that
            //does not exist anymore
            return;
        }

        //control Y coordinate of missile depending on terrain height
        //calculate current cell below missile
        int current_cell_calc_x, current_cell_calc_y;

        current_cell_calc_y = (int)(currentLocation.Z / mParentLauncher->mParent->mRace->mLevelTerrain->segmentSize);
        current_cell_calc_x = -(int)(currentLocation.X / mParentLauncher->mParent->mRace->mLevelTerrain->segmentSize);

        MapEntry* mEntry = mParentLauncher->mParent->mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);

        if (mEntry != nullptr) {
            currentLocation.Y =
                    mParentLauncher->mParent->mRace->mLevelTerrain->pTerrainTiles[mEntry->get_X()][mEntry->get_Z()].currTileHeight + 0.3f;
        }

        if (mSceneNodeMissile != nullptr) {
            mSceneNodeMissile->setPosition(currentLocation);
        }
    }
}

void Missile::CheckForHitOfMissileTrigger(irr::core::vector3df explodedAtLocation) {
    //which 2D map cell coordinate did the missile hit?
    irr::core::vector2di cellHit;

    cellHit.X = -(int)explodedAtLocation.X;
    cellHit.Y = (int)explodedAtLocation.Z;

    //the missile trigger regions defined in the original level map
    //files are only containing a single cell coordinate, and no
    //Offset X and Offset Y
    std::vector<MapTileRegionStruct*>::iterator itRegion;

    //check for each missile trigger region in level
    for (itRegion = mParentLauncher->mParent->mRace->mTriggerRegionVec.begin(); itRegion != mParentLauncher->mParent->mRace->mTriggerRegionVec.end(); ++itRegion) {
        //only check for regions which are missile trigger region
        if ((*itRegion)->regionType == LEVELFILE_REGION_TRIGGERMISSILE) {
            //did the missile explosion hit this area?
            if (cellHit ==  (*itRegion)->regionCenterTileCoord) {
                //yes, missile hit

                //is it a one time trigger? if so only trigger if we have not triggered before
                //or if it is a multiple time trigger just go ahead and trigger event
                if (((*itRegion)->mOnlyTriggerOnce && (!(*itRegion)->mAlreadyTriggered))
                        || (!(*itRegion)->mOnlyTriggerOnce)) {
                           if ((*itRegion)->mOnlyTriggerOnce) {
                               (*itRegion)->mAlreadyTriggered = true;
                           }

                        //tell race about it
                        mParentLauncher->mParent->mRace->PlayerMissileHitMissileTrigger(mParentLauncher->mParent, (*itRegion));
                }
            }
        }
    }
}

//Returns true in case of success
//False otherwise
bool MissileLauncher::LoadSprites() {
    //load the glowing ball (missile) from game sprites
    //mMissileTex = mDriver->getTexture("extract/sprites/tmaps0000.png");

    //get the pointer to the preloaded texture
    mMissileTex = this->mParent->mRace->mTexLoader->spriteTex.at(0);

    if (mMissileTex == nullptr)
        return false;

    mMissileTexSize = mMissileTex->getSize();

    //load the smoke cloud sprite that is used behind the missle
    //mSmokeTex = mDriver->getTexture("extract/sprites/tmaps0017.png");

    //get the pointer to the preloaded texture
    mSmokeTex = this->mParent->mRace->mTexLoader->spriteTex.at(17);

    if (mSmokeTex == nullptr)
        return false;

    mSmokeTexSize = mSmokeTex->getSize();

   return true;
}

//get the missile launch location in front of the current player position
std::vector<irr::core::vector3df> MissileLauncher::GetMissileLaunchLocation(bool fireTwoMissiles) {
    irr::core::vector3df Loc1;
    irr::core::vector3df Loc2;

    std::vector<irr::core::vector3df> result;
    result.clear();

    if (!fireTwoMissiles) {
        Loc1 = mParent->phobj->physicState.position + 2.0f * mParent->craftForwardDirVec;
    }  else {
        //we need to fire two missiles
        //offset missile launch position to the side a bit
        Loc1 = mParent->phobj->physicState.position + 2.0f * mParent->craftForwardDirVec + 1.0f * mParent->craftSidewaysToRightVec;
        Loc2 = mParent->phobj->physicState.position + 2.0f * mParent->craftForwardDirVec - 1.0f * mParent->craftSidewaysToRightVec;
    }

    //calculate current cell below bullet impact position
    int current_cell_calc_x, current_cell_calc_y;

    current_cell_calc_y = (int)(Loc1.Z / mParent->mRace->mLevelTerrain->segmentSize);
    current_cell_calc_x = -(int)(Loc1.X / mParent->mRace->mLevelTerrain->segmentSize);

    MapEntry* mEntry1 = mParent->mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);
    MapEntry* mEntry2 = nullptr;

    if (mEntry1 != nullptr) {
        Loc1.Y = mParent->mRace->mLevelTerrain->pTerrainTiles[mEntry1->get_X()][mEntry1->get_Z()].currTileHeight + 0.3f;
    }

    result.push_back(Loc1);

    if (fireTwoMissiles) {
        current_cell_calc_y = (int)(Loc2.Z / mParent->mRace->mLevelTerrain->segmentSize);
        current_cell_calc_x = -(int)(Loc2.X / mParent->mRace->mLevelTerrain->segmentSize);

        mEntry2 = mParent->mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);
        if (mEntry2 != nullptr) {
            Loc2.Y = mParent->mRace->mLevelTerrain->pTerrainTiles[mEntry2->get_X()][mEntry2->get_Z()].currTileHeight + 0.3f;
        }

         result.push_back(Loc2);
    }

    return result;
}

void MissileLauncher::Trigger() {

    //if player can not shoot right now simply exit
    if (!this->mParent->mPlayerStats->mPlayerCanShoot)
        return;

    //is there even a missile left for this player?
    this->mParent->mPlayerStats->ammoVal -= 1.0f;

    if (this->mParent->mPlayerStats->ammoVal < 0.0f) {
        this->mParent->mPlayerStats->ammoVal = 0.0f;
        return;
    }

    Player* lockedPlayer = nullptr;
    bool targetIsLocked = false;

    bool skipAnimation = false;
    bool fireTwoMissiles = false;

    //the original game fires a second missile at the same time for
    //the highest achieved upgrade level of missile launcher, but does
    //only decrease missile count by one, level  3 is the highest level
    if (this->mParent->mPlayerStats->currRocketUpgradeLevel >= 3) {
        //fire a second missile
        fireTwoMissiles = true;
    }

    irr::core::vector3df shotTargetLoc1;
    irr::core::vector3df shotTargetLoc2;
    std::vector<irr::core::vector3df> launchLoc = GetMissileLaunchLocation(fireTwoMissiles);

     if (mParent->mTargetPlayer != nullptr) {
         //we have currently a player targeted, fire at the player
         shotTargetLoc1 = mParent->mTargetPlayer->phobj->physicState.position;
         shotTargetLoc2 = mParent->mTargetPlayer->phobj->physicState.position;

         //is the missile locked currently at the target
         //(which means player was targeted lock enough, and is
         //enclosed in red target box)
         if (mParent->mTargetMissleLock) {
              lockedPlayer = mParent->mTargetPlayer;
              targetIsLocked = true;

              //this was a hit, add to hit statistics for accuracy
              mParent->mPlayerStats->shootsHit++;
         } else {
             //we fired at a green marked player
             //this was a miss, add to miss statistics for accuracy
             mParent->mPlayerStats->shootsMissed++;
         }

     } else {
         //this was a miss, add to miss statistics for accuracy
         mParent->mPlayerStats->shootsMissed++;

         //we do not target any player right now
         //the missile should target the terrain
         //shotTargetLoc = GetBulletImpactPoint();
         RayHitTriangleInfoStruct triangleHit;
         if (mParent->GetWeaponTarget(triangleHit)) {
             //we found a valid target
             shotTargetLoc1 = triangleHit.hitPointOnTriangle;
             shotTargetLoc2 = triangleHit.hitPointOnTriangle;
         } else {
             //did not find a shooting location target
             //but we we want to really fire a rocket, just shoot it straight
             //some distance; because it would be weird if player presses shoot missile
             //key and no missile is fired!
             //just set missile target to be player craft forward direction and set distance to maximum available distance
             //forward until next terrain obstacle, this should aim approx. to the wall/columns/terrain or whatever in front of player
             shotTargetLoc1 = launchLoc.at(0) + this->mParent->craftForwardDirVec * this->mParent->mCraftDistanceAvailFront;
             if (fireTwoMissiles) {
                shotTargetLoc2 = launchLoc.at(1) + this->mParent->craftForwardDirVec * this->mParent->mCraftDistanceAvailFront;
             }
             //skipAnimation = true;
         }
     }

     //only create a new missile if we have a target to shot at
     if (!skipAnimation) {
        Missile* newMissile = new Missile(this, launchLoc.at(0), shotTargetLoc1, targetIsLocked, lockedPlayer);

        this->mCurrentMissilesVec.push_back(newMissile);

        if (fireTwoMissiles) {
            //add a second missile
            Missile* newMissile2 = new Missile(this, launchLoc.at(1), shotTargetLoc2, targetIsLocked, lockedPlayer);

            this->mCurrentMissilesVec.push_back(newMissile2);
        }

        shooting = true;
     }

     if (mShotSound == nullptr) {
            mShotSound = mParent->mRace->mSoundEngine->PlaySound(SRES_GAME_MISSILE_SHOT, launchLoc.at(0), false);
     }
}

void MissileLauncher::Update(irr::f32 DeltaTime) {

   std::vector <Missile*>::iterator it;
   Missile* mPntr;

   if (mCurrentMissilesVec.size() > 0 ) {
           //search for missiles that have exploded already and should be deleted
           //erase this missile from my list of current missiles vector
           for (it = this->mCurrentMissilesVec.begin(); it != this->mCurrentMissilesVec.end();) {
               if ((*it)->objToBeDeleted) {
                    mPntr = (*it);
                    it = mCurrentMissilesVec.erase(it);

                    //delete the missile object itself
                    delete mPntr;
                    mPntr = nullptr;
               } else ++it;
            }
   }

   if (mCurrentMissilesVec.size() > 0 ) {
         //update all my current missiles
        for (it = this->mCurrentMissilesVec.begin(); it != this->mCurrentMissilesVec.end(); ++it) {
            (*it)->Update(DeltaTime);
        }
   }

    if (mShotSound != nullptr) {
        if (mShotSound->getStatus() == mShotSound->Stopped) {
            mShotSound = nullptr;
        }
    }
}

MissileLauncher::MissileLauncher(Player* myParentPlayer, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver *driver) {
    mParent = myParentPlayer;
    mSmgr = smgr;
    mDriver = driver;

    ready = true;

    if (!LoadSprites()) {
        ready = false;
    }
}

MissileLauncher::~MissileLauncher() {
    std::vector <Missile*>::iterator it;
    Missile* mPntr;

    //remove remaining missiles
    for (it = this->mCurrentMissilesVec.begin(); it != this->mCurrentMissilesVec.end(); ) {
            mPntr = (*it);
            it = mCurrentMissilesVec.erase(it);
            delete mPntr;
    }
}
