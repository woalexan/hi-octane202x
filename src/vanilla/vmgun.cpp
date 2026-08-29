/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

//This source code was implemented by me based on the insight I gained into the original game. Some parts do more closely follow
//the Pseudo-C Code that is available to me, and other parts are heavily modified to hopefully work inside my existing project
//in the near future.
//The original game uses fixed point arithmetic for performance reasons (it had to run on 80486 after all), and is optimized everywhere you
//look at. I do not want to do the same, because the fixed point arithmetic makes it very difficult to read and understand. And performance should not
//be the problem nowadays. Therefore the biggest change which I have to do is to change all the logic to floating point arithmetic,
//while hopefully not breaking the concept behind everything.
//Additionally I will only try to reimplement the most important mechanisms and concepts from the original game where it benefits the overall project,
//but I will not change the overall project to use the same structs and data structures as the original. This would not work, and also does not
//make sense in my opinion.

//Important note: What makes this source code very difficult to handle is the fact, that my coordinate system in this existing project is
//completely different to the one in the original game. The original uses X and Y axis for the tile map, and Z is the height.
//For the levelfile and 2D map stuff I also use X any Y axis for the tile map most of the time.
//My 3D world setup (for rendering) using Irrlicht has X and Z for the tile map, and Y is the height. And to make things worse my Irrlicht vertice X and Y coordinates
//have a swapped sign (are negative) currently.

//I have decided to also use the original games coordinate system in all vanilla calculations. At the interface between
//original game calculations and Irrlicht 3D coordinate system I have then to convert from one coordinate system setup to the other.
//Thats the drawback I will have.

//I really want to thank aybe for giving me the opportunity to look much deeper into the original game inner workings as I was ever able before.
//Without this support I would not have been able to hopefully advance the current project more true to the original.

#include "vmgun.h"
#include "vvehicle.h"
#include "vcalc.h"
#include "../race.h"
#include "../game.h"
#include "../resources/texture.h"
#include "../vanilla/vthing.h"

VMGun::VMGun(Race* parentRace, VVehicle* owner) {
    mParentRace = parentRace;
    mOwner = owner;

    mBulletThings.clear();
    mShotVec.clear();

    LoadSprites();
}

VMGun::~VMGun() {
    //We do not need to care about Things itself, the belong to
    //the ThingManager, we only borrow Pointers to Things
    //we just need to make sure that we give our things back at the end

    std::vector<MGunShotStruct*>::iterator it2;
    MGunShotStruct* pntrShot;

    //cleanup all remaining shot objects
    for (it2 = mShotVec.begin(); it2 != mShotVec.end(); ) {
         pntrShot = (*it2);

         it2 = mShotVec.erase(it2);

         delete pntrShot;
    }

    //cleanup all currently existing but not useful anymore
    //bulletEffect objects
    BulletThingStruct* pntrBulletEffect;

    //delete all remaining BulletThings
    std::vector<BulletThingStruct*>::iterator it;
    for (it = mBulletThings.begin(); it != mBulletThings.end(); ) {
            pntrBulletEffect = (*it);
            it = mBulletThings.erase(it);

            CleanupBulletThing(pntrBulletEffect);
    }
}

//Returns true in case of success
//False otherwise
bool VMGun::LoadSprites() {
   irr::video::ITexture* newTex;

   //get pointer to preloaded texture
   newTex = this->mParentRace->mTexLoader->spriteTex.at(43);

   if (newTex == nullptr)
       return false;

   animTexList.push_back(newTex);

   newTex = this->mParentRace->mTexLoader->spriteTex.at(44);

   if (newTex == nullptr)
       return false;

   animTexList.push_back(newTex);

   newTex = this->mParentRace->mTexLoader->spriteTex.at(45);

   if (newTex == nullptr)
       return false;

   animTexList.push_back(newTex);

   newTex = this->mParentRace->mTexLoader->spriteTex.at(46);

   if (newTex == nullptr)
       return false;

   animTexList.push_back(newTex);

   return true;
}

//we can not remove scenenodes which have still
//an animations running when the race is over
//the race object uses this function to wait until
//all animations are done
bool VMGun::AllAnimationsFinished() {
    std::vector<BulletThingStruct*>::iterator it;

    for (it = this->mBulletThings.begin(); it != this->mBulletThings.end(); ++it) {
        if ((*it)->animator != nullptr) {
           if (!(*it)->animator->hasFinished())
               return false;
        }
    }

    return true;
}

void VMGun::initialiseSHOT_BULLET(MGunShotStruct* whichShot) {
    VThing* whichThing = whichShot->ThingPntr;

    whichThing->Life = 18;
    whichThing->CollideSize.set(0.125f, 0.125f, 0.125f);
    whichThing->ColideGroup = (1048 & 0xFFFE);
    mParentRace->mVCalc->move_displacement_set(whichThing->Displacement, whichThing->Movement.AngleXY,
                                               whichThing->Movement.AngleZY, 1.0f);
}

uint8_t VMGun::processSHOT_BULLET(MGunShotStruct* whichShot) {
    irr::core::vector3df position;
    int32_t i;
    int32_t v14;
    int32_t j;
    int32_t v1 = 0;
    std::vector<VVehicle*>::iterator it;
    uint8_t v8;
    int16_t upgrade;
    irr::f32 rNumFloat1;
    int rNum;
    int v22;
    int16_t v28;
    BulletThingStruct* v20;

    //if this shot is done with its work
    //and we are waiting still for cleanup just
    //exit
    if (whichShot->ReadyForCleanup)
        return 1;

    VThing* whichThing = whichShot->ThingPntr;

    position = whichThing->Position;
    for (i = 0; i < 4; ++i) {
        if (whichThing->Life < 0) {
            break;
        }
        mParentRace->mVCalc->move_displacement_xyz(position, whichThing->Displacement, 1);
        if (mParentRace->mVCalc->map_colide(position)) {
            whichThing->Life = 0;
            v1 = 1;
        } else {
           mParentRace->mVCalc->move_swap_positions(whichThing->Position, position);

           //only for debugging
           //irr::core::vector3df dbgPos = mParentRace->mVCalc->VanillaToIrrlichtCoord(whichThing->Position);
           //mParentRace->mGame->mSmgr->addCubeSceneNode(0.04f, nullptr, -1, dbgPos);

           //go through all the vehicles and see if the shot collides
           //with one of them
           it = mParentRace->mVanillaCraftVec.begin();
           while (1) {
               if (it == mParentRace->mVanillaCraftVec.end()) {
                   break;
               }

               //check only if it is not the vehicle that fired the shot
               //in the first place
               if (whichThing->Id != (*it)->ThingData->Id) {
                   v8 = mParentRace->mThingManager->thing_overlapping(whichThing, (*it)->ThingData);
                   if (v8) {
                       break;
                   }
               }

               ++it;
           }

           mParentRace->mVCalc->move_swap_positions(whichThing->Position, position);

           if (v8) {
               v1 = 1;
               ++mOwner->Conditions.BulletsHit;
               whichThing->Life = 0;
           }
        }
        --whichThing->Life;
    }

    mParentRace->mThingManager->mapwho_move(whichThing, position);
    if (whichThing->Life >= 0) {
        return 0;
    }

    if (v1) {
        upgrade = whichThing->Upgrade;
        v14 = 4;
        if (upgrade < 4 && upgrade >= 2) {
            v14 = 8;
        }

        for (j = 0; j < v14; ++j) {
            position = whichThing->Position;

            //derive a random deviation for Xpos
            rNum = rand();
            rNumFloat1 = ((float(rNum) / float (RAND_MAX)) * 0.1875f - 0.09375f);
            position.X += rNumFloat1;

            rNum = rand();
            rNumFloat1 = ((float(rNum) / float (RAND_MAX)) * 0.1875f - 0.09375f);
            position.Y += rNumFloat1;

            rNum = rand();
            rNumFloat1 = ((float(rNum) / float (RAND_MAX)) * 0.1875f - 0.09375f);
            position.Z += rNumFloat1;

            //Create a new BullectEffect Thing
            v20 = CreateBulletThing(&position,
                     whichThing->Movement.AngleXY, whichThing->Movement.AngleZY,
                     whichThing->Movement.AngleXZ, whichThing->Id);

            if (v20->ThingData != nullptr) {
                if (j) {
                    v20->ThingData->ColideGroup = 0;
                } else {
                    if (whichThing->Upgrade == 1) {
                        whichThing->AffectNumber = 3 * whichThing->AffectNumber / 2;
                    } else if (whichThing->Upgrade == 3) {
                        whichThing->AffectNumber *= 3;
                    }
                    whichThing->AffectNumber *= v14;
                }
            }
        }
    }

    mParentRace->mThingManager->thing_delete(whichThing);

    //this shot should be cleaned up now
    whichShot->ReadyForCleanup = true;

    //play us some sound
    //to be finished later
    if (v1) {
        v22 = rand() % 3;
        if (v22 == 1) {
            //sample_play(whichThing, 4);
            v28 = 4;
        } else if (v22 >= 2) {
            if (v22 != 2) {
                return 1;
            }

            //sample_play(whichThing, 6);
            v28 = 6;
        } else {
            if (v22) {
                return 1;
            }

            //sample_play(whichThing, 3);
            v28 = 3;
        }
        //sample_set_pitch(whichThing, v28, .... TBD)
    }

    return 1;
}

VThing* VMGun::CreateShot(irr::core::vector3df* position,
                          irr::f32 angleXY, irr::f32 angleZY,
                          irr::f32 angleXZ, int16_t id) {

    MGunShotStruct* newStruct = new MGunShotStruct;

    VThing* newThing =
        mParentRace->mThingManager->thing_initialise(*position, angleXY, angleZY, angleXZ,
                                                        6, 0, id);

    newStruct->ThingPntr = newThing;
    newStruct->ReadyForCleanup = false;

    initialiseSHOT_BULLET(newStruct);

    mShotVec.push_back(newStruct);

    return newThing;
}

BulletThingStruct* VMGun::CreateBulletThing(irr::core::vector3df* position,
                                   irr::f32 angleXY, irr::f32 angleZY,
                                          irr::f32 angleXZ, int16_t id) {
    BulletThingStruct* newBulletThing = new BulletThingStruct;

    //Create a new Thing for the Bullet Effect
    newBulletThing->ThingData =
            mParentRace->mThingManager->thing_initialise(*position,
                 angleXY, angleZY, angleXZ, 2, 4, id);

    //Life and CollideSize for bullet is defined in
    //initializeEFFECT_BULLET
    newBulletThing->ThingData->Life = 4;
    newBulletThing->ThingData->CollideSize.set(1.0f, 1.0f, 1.0f);
    newBulletThing->ThingData->ColideGroup = (1048 & 0xFFFE);
    newBulletThing->ThingData->AffectNumber = 60;
    newBulletThing->ThingData->AffectStatus |= 4;

    //Setup the Irrlicht SceneNode as well
    newBulletThing->animSprite = mParentRace->mGame->mSmgr->addBillboardSceneNode();
    newBulletThing->animSprite->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );

    newBulletThing->animSprite->setMaterialFlag(irr::video::EMF_LIGHTING, true);
    newBulletThing->animSprite->setMaterialFlag(irr::video::EMF_ZBUFFER, true);
    newBulletThing->animSprite->setSize(irr::core::dimension2d<irr::f32>(0.2f, 0.2f));

    UpdateSceneNode(newBulletThing->animSprite, newBulletThing->ThingData->Position);

    newBulletThing->animSprite->setVisible(true);
    newBulletThing->animator = mParentRace->mGame->mSmgr->createTextureAnimator(animTexList, 50, false);
    newBulletThing->animSprite->addAnimator(newBulletThing->animator);
    newBulletThing->animatorActive = true;

    mBulletThings.push_back(newBulletThing);

    return newBulletThing;
}

void VMGun::UpdateSceneNode(irr::scene::IBillboardSceneNode* whichNode, irr::core::vector3df vanPos) {
    irr::core::vector3df irrPos =
            mParentRace->mVCalc->VanillaToIrrlichtCoord(vanPos);

     whichNode->setPosition(irrPos);
}

uint8_t VMGun::processEFFECT_BULLET(BulletThingStruct* whichBulletThing) {
    irr::f32 v2;

    //if this bullet has done its job, and waits for cleanup
    //just exit here
    if (whichBulletThing->ReadyForCleanup)
        return 1;

    v2 = whichBulletThing->ThingData->Movement.AngleXY + 11.375f;
    whichBulletThing->ThingData->Life--;

    whichBulletThing->ThingData->Movement.AngleXY = v2;
    if (whichBulletThing->ThingData->Life < 0) {
        //is not needed anymore, should be deleted
        whichBulletThing->ReadyForCleanup = true;

        return 1;
    } else {
        if ((whichBulletThing->ThingData->Status & 8) == 0) {
            if (whichBulletThing->ThingData->ColideGroup) {
                mParentRace->mThingManager->effect_affect_vehicle_exclusive(whichBulletThing->ThingData);
                mParentRace->mThingManager->affect_thing(whichBulletThing->ThingData);
            }
            whichBulletThing->ThingData->Status |= 8u;
        }
    }

    return 0;
}

void VMGun::CleanupBulletThing(BulletThingStruct* whichBulletThing) {
    if (whichBulletThing == nullptr)
        return;

    //give back the thing
    if (whichBulletThing->ThingData != nullptr) {
        mParentRace->mThingManager->thing_delete(whichBulletThing->ThingData);
        whichBulletThing->ThingData = nullptr;
    }

    whichBulletThing->animSprite->removeAnimator(whichBulletThing->animator);
    whichBulletThing->animator->drop();
    whichBulletThing->animator = nullptr;

    //remove the SceneNode as well
    whichBulletThing->animSprite->remove();

    //delete the struct itself
    delete whichBulletThing;
}

//This function mostly implements the functionality
//of the function "processWEAPON_MINI_GUN" of the
//original game
void VMGun::Update(irr::f32 frameDeltaTime) {
    int32_t v5;
    int32_t v10;
    int16_t triggerRestrictionCount;
    irr::f32 angleXY;
    irr::f32 angleZY;
    irr::f32 angleXZ;
    VVehicle* targetVehicle = nullptr;
    irr::core::vector3df* p_Position;
    irr::core::vector3df position;
    uint32_t v13;
    irr::f32 v14;
    int16_t v18;
    VThing* v12;

    //add delta time up to see when we need to update
    //the slower parts of the MGun
    mAbsTimeAcc += frameDeltaTime;

    if (mAbsTimeAcc >= 0.05f) {
        mAbsTimeAcc = 0.0f;

        //cleanup all currently existing but not useful anymore
        //bulletEffect objects
        BulletThingStruct* pntrBulletEffect;
        bool deleteObj;

        std::vector<BulletThingStruct*>::iterator it;
        for (it = mBulletThings.begin(); it != mBulletThings.end(); ) {
            deleteObj = false;
            if ((*it)->ReadyForCleanup) {
                deleteObj = true;
                //make sure that the animation is already done
                //is the animation done?
                if ((*it)->animatorActive) {
                    if (!(*it)->animator->hasFinished()) {
                        deleteObj = false;
                    }
                }
            }

            if (deleteObj) {
                pntrBulletEffect = (*it);
                it = mBulletThings.erase(it);

                CleanupBulletThing(pntrBulletEffect);
            } else
            {
                ++it;
            }
        }

        std::vector<MGunShotStruct*>::iterator it2;
        MGunShotStruct* pntrShot;

        //cleanup all shot objects that were marked to be cleaned up
        for (it2 = mShotVec.begin(); it2 != mShotVec.end(); ) {
            if ((*it2)->ReadyForCleanup) {
                pntrShot = (*it2);

                it2 = mShotVec.erase(it2);

                delete pntrShot;
            } else {
                ++it2;
            }
        }

        for (it = mBulletThings.begin(); it != mBulletThings.end(); ++it) {
            processEFFECT_BULLET((*it));

            //we need to update the TimeSlice variable in
            //the EFFECT_BULLET Thing
            mParentRace->mThingManager->UpdateTimeSlice((*it)->ThingData);
        }

        //first update all currently existing shots
        std::vector<MGunShotStruct*>::iterator itShot;
        for (itShot = mShotVec.begin(); itShot != mShotVec.end(); ++itShot) {
             processSHOT_BULLET((*itShot));

             //we need to update the TimeSlice variable in
             //the SHOT_BULLER_THING
             mParentRace->mThingManager->UpdateTimeSlice((*itShot)->ThingPntr);
        }

        if (mOwner != nullptr) {
          v5 = 0;
          if (Trigger) {
             if (TriggerTime < 100) {
                 v5 = 1;
             } else {
                triggerRestrictionCount = TriggerRestrictionCount;
                TriggerRestrictionCount = triggerRestrictionCount - 1;
                if (triggerRestrictionCount) {
                    //sample_stop(v6, 15)
                    //sample_play(v6, 14);
                    mParentRace->mSoundEngine->PlaySound(SRES_GAME_MGUN_SHOTFAILED, false);
                } else {
                    v5 = 1;
                    TriggerRestrictionCount = 10;
                }
                TriggerTime = 100;
             }
          if (v5) {
            angleXY = mOwner->View.AngleXY;
            angleZY = mOwner->View.AngleZY;
            angleXZ = mOwner->View.AngleXZ;
            if (Target) {
                targetVehicle = mParentRace->GetVehicleWithId((size_t)(Target));
                angleXY = mParentRace->mVCalc->angle_get_xy(mOwner->ThingData->Position,
                                                      targetVehicle->ThingData->Position);
                angleZY = mParentRace->mVCalc->angle_get_zy(mOwner->ThingData->Position,
                                                      targetVehicle->ThingData->Position);
            }
            v10 = 1;
            p_Position = &mOwner->ThingData->Position;
            do {
              //Note 22.08.2026: CreateShot internally also creates the needed VThing
              //for the MGun shot
              v12 = CreateShot(p_Position, angleXY, angleZY, angleXZ, mOwner->ThingData->Id);
              if (v12 != nullptr) {
                  ++mOwner->Conditions.Bullets;
                  //sample_play(v6, 15);
                  mParentRace->mSoundEngine->PlaySound(SRES_GAME_MGUN_SINGLESHOT, false);
                  v13 = mOwner->ThingData->Status ^ 0x10;
                  mOwner->ThingData->Status = v13;
                  v14 = -90.0f;
                  if ((v13 & 0x10) != 0) {
                      v14 = 90.0f;
                  }
                  position.X = v12->Position.X;
                  position.Y = v12->Position.Y;
                  position.Z = v12->Position.Z - 0.03125f;
                  mParentRace->mVCalc->move_xyz(position, v14 + v12->Movement.AngleXY, 0.0f, 0.125f);
                  mParentRace->mThingManager->mapwho_move(v12, position);
                  TriggerTime += 2;
                  if (TriggerTime >= 100) {
                      ++mOwner->Conditions.MiniGunHeatup;
                  }
                  //need to set the bullets own upgrade level!
                  v12->Upgrade = this->Upgrade;
              }
              --v10;
              p_Position = &mOwner->ThingData->Position;
            } while (v10);
          }
          Trigger = 0;
          return;
        }
        if (Upgrade == 1) {
            v18 = TriggerTime - 17;
        } else if (Upgrade >= 2) {
          if (Upgrade == 2) {
              v18 = TriggerTime - 25;
          } else
          {
              if (Upgrade != 3) {
                  goto processWEAPON_MINI_GUN_LABEL36;
              }
              v18 = TriggerTime - 33;
          }
        } else
        {
          if (Upgrade) {
                   goto processWEAPON_MINI_GUN_LABEL36;
          }
          v18 = TriggerTime - 10;
        }
        TriggerTime = v18;
    processWEAPON_MINI_GUN_LABEL36:
        if (TriggerTime >= 0) {
            return;
        }
        TriggerTime = 0;
      }
    }
}

