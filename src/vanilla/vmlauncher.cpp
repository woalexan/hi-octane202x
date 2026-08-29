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

#include "vmlauncher.h"
#include "vvehicle.h"
#include "vcalc.h"
#include "../race.h"
#include "../game.h"
#include "../resources/texture.h"
#include "../vanilla/vthing.h"
#include "veffectmanager.h"

VMLauncher::VMLauncher(Race* parentRace, VVehicle* owner) {
    mParentRace = parentRace;
    mOwner = owner;
    mMissileShotVec.clear();
}

VMLauncher::~VMLauncher() {
}

void VMLauncher::initialiseSHOT_MISSILE(MMissileShotStruct* whichMissileShot) {
    VThing* whichThing = whichMissileShot->ThingPntr;

    whichThing->Life = 113;
    whichThing->CollideSize.set(0.25f, 0.25f, 0.25f);
    whichThing->ColideGroup = (1048 & 0xFFFE);
    whichThing->Movement.SpeedActual = 0.28125f;
}

//This function is also called with period of ~50ms
uint8_t VMLauncher::processSHOT_MISSILE(MMissileShotStruct* whichMissileShot) {
    irr::core::vector3df position_from;
    int32_t v1;
    int32_t v5;
    irr::f32 v9;
    irr::f32 v10;
    bool v13;
    int8_t v14;
    irr::f32 v16;
    bool v17;
    uint8_t v19;
    irr::f32 v29;
    irr::f32 v34;
    irr::f32 v36;
    irr::f32 v37;
    int32_t upgrade;
    irr::f32 xy;
    irr::f32 xyz;
    irr::f32 difference;
    irr::f32 zy;
    int rNum;
    int v27;
    irr::f32 zPos;
    VThing* v25 = nullptr;
    VThing* v31 = nullptr;
    VVehicle* targetVehicle = nullptr;
    std::vector<VVehicle*>::iterator it;
    irr::core::vector3df dbgPos;
    EffectInfoStruct* infoStruct = nullptr;
    EffectInfoStruct* infoStruct2 = nullptr;

    //if missile shot object has done its job already
    //and just waits to be deleted, return without doing anything
    if (whichMissileShot->ReadyForCleanup)
        return 1;

    VThing* whichThing = whichMissileShot->ThingPntr;

    position_from = whichThing->Position;
    v5 = 4;
    v1 = 0;
    while (whichThing->Life >= 0) {
        upgrade = whichThing->Upgrade;
        if (upgrade == 1) {
            goto processSHOT_MISSILE_LABEL_14;
        }
        if (upgrade >= 2) {
          if (upgrade == 2) {
             v1 = 3;
             goto processSHOT_MISSILE_LABEL_15;
          }
          if (upgrade == 3) {
processSHOT_MISSILE_LABEL_14:
              v1 = 1;
          }
        } else if (!whichThing->Upgrade) {
            goto processSHOT_MISSILE_LABEL_14;
        }
processSHOT_MISSILE_LABEL_15:

        if (whichThing->Target) {
            targetVehicle = mParentRace->GetVehicleWithId((size_t)(Target));
            if (whichThing->Count == 1) {
                  xy = mParentRace->mVCalc->angle_get_xy(position_from, targetVehicle->ThingData->Position);
                  difference = mParentRace->mVCalc->angle_get_difference(whichThing->Movement.AngleXY, xy);
                  zy = mParentRace->mVCalc->angle_get_zy(position_from, targetVehicle->ThingData->Position);
                  v34 = mParentRace->mVCalc->angle_get_difference(whichThing->Movement.AngleZY, zy);
                  v9 = -0.999755859375f;
                  if (difference < -0.999755859375f || (v9 = 0.999755859375f, difference >= 1.0052490234375f)) {
                      difference = v9;
                  }
                  v10 = -0.999755859375f;
                  if (v34 < -0.999755859375f || (v10 = 0.999755859375f, v34 >= 1.0052490234375f)) {
                      v34 = v10;
                  }
                  whichThing->Movement.AngleXY += difference;
                  whichThing->Movement.AngleZY += v34;
            }
            if (targetVehicle->ThingData->Group == 10) {
               if ((fabs(targetVehicle->ClosestMissile) < 0.00390625f) || (xyz = mParentRace->mVCalc->distance_get_xyz(targetVehicle->ThingData->Position, whichThing->Position),
                                                     v13 = xyz >= targetVehicle->ClosestMissile,
                                                     v37 = xyz,
                                                     !v13)) {
                    targetVehicle->ClosestMissile = v37;
               }
            }
        }
        mParentRace->mVCalc->move_xyz(position_from, whichThing->Movement.AngleXY, whichThing->Movement.AngleZY,
                                      whichThing->Movement.SpeedActual);
        v14 = mParentRace->mVCalc->map_colide(position_from);
        if (!v14) {
            goto processSHOT_MISSILE_LABEL_39;
        }
        if ((v14 & 0x10) == 0) {
            if ((v14 & 8) == 0) {
                  goto processSHOT_MISSILE_LABEL_40;
            }
            v36 = mParentRace->mVCalc->map_floor(position_from);
            v16 = position_from.Z - v36;
            v13 = (v16 < 0.0f);
            v17 = (v16 < 0.07421875f);
            if (v13) {
               if ((v36 - position_from.Z) < 0.07421875f) {
                   goto processSHOT_MISSILE_LABEL_37;
               }
            } else if (v17) {
processSHOT_MISSILE_LABEL_37:
                if ((position_from.Z - v36) < 0.03125f) {
                    v14 = 0;
                    position_from.Z = v36 + 0.03125f;
                }
                goto processSHOT_MISSILE_LABEL_39;
            }
            whichThing->Life = 0;
            v14 = 1;
            goto processSHOT_MISSILE_LABEL_37;
        }
        whichThing->Life = 0;
        v14 = 1;
processSHOT_MISSILE_LABEL_39:
processSHOT_MISSILE_LABEL_40:
        mParentRace->mVCalc->move_swap_positions(whichThing->Position, position_from);

        //only for debugging
        dbgPos = mParentRace->mVCalc->VanillaToIrrlichtCoord(whichThing->Position);
        mParentRace->mGame->mSmgr->addCubeSceneNode(0.03f, nullptr, -1, dbgPos);

        //go through all the vehicles and see if the missile collides
        //with one of them
        it = mParentRace->mVanillaCraftVec.begin();
        while (1) {
            if (it == mParentRace->mVanillaCraftVec.end()) {
                break;
            }

            //check only if it is not the vehicle that fired the shot
            //in the first place
            if (whichThing->Id != (*it)->ThingData->Id) {
                v19 = mParentRace->mThingManager->thing_overlapping(whichThing, (*it)->ThingData);
                if (v19) {
                    break;
                }
            }

            ++it;
        }

        mParentRace->mVCalc->move_swap_positions(whichThing->Position, position_from);

        //Rocket hit a player/vehicle
        if (v19) {
            v14 = 1;
            ++mOwner->Conditions.RocketsHit;
            whichThing->Life = 0;
        }
        --v5;

        //Rocket did not hit a player/vehicle?
        if (!v14) {
            rNum = rand();
            if (rNum % 3u) {
                //I believe this create an EFFECT_SMOKE
                infoStruct = mParentRace->mEffectManager->AddEffect(EffectType::Smoke, position_from,
                                                                   whichThing->Movement.AngleXY,
                                                                   whichThing->Movement.AngleZY,
                                                                   whichThing->Movement.AngleXZ,
                                                                   whichThing->Id);
                if (infoStruct != nullptr) {
                    v25 = infoStruct->thingPntr;
                    if (v25 != nullptr) {
                        v27 = rand();
                        zPos = v25->Position.Z;
                        v25->Movement.SpeedActual = mParentRace->mVCalc->FixedPointToFloat8D8((v27 & 0xF) + 16);
                        v25->Position.Z = zPos + 0.0625f;
                    }
                }
            }
        }

        --whichThing->Life;
        if (!v5) {
            break;
        }
    }

    mParentRace->mThingManager->mapwho_move(whichThing, position_from);
    if (whichThing->Life > 0) {
        return 0;
    }

    if (v1) {
      v29 = 119.9981689453125f * (irr::f32)(v1) + 120.003662109375f;
      do {
         position_from = whichThing->Position;
         if (v1 >= 2) {
             mParentRace->mVCalc->move_xyz(position_from, v29, 0.0f, 2.0f);
         }
         //I believe this create an EFFECT_EXPLOSION_MEDIUM
         infoStruct2 = mParentRace->mEffectManager->AddEffect(EffectType::ExplosionMedium, position_from, whichThing->Movement.AngleXY,
                                                      whichThing->Movement.AngleZY, whichThing->Movement.AngleXZ,
                                                      whichThing->Id);
           if (infoStruct2 != nullptr) {
             v31 = infoStruct2->thingPntr;
             if (v31 != nullptr) {
                if (v1 >= 2) {
                    v31->AffectNumber *= 2;
                }
                whichThing->AffectStatus |= 0x1000000u;
             }
          }
         --v1;
         v29 -= 119.9981689453125f;
      } while (v1);
    }

    mParentRace->mThingManager->thing_delete(whichThing);
    whichMissileShot->ReadyForCleanup = true;

    return 1;
}

VThing* VMLauncher::CreateMissileShot(irr::core::vector3df* position,
                          irr::f32 angleXY, irr::f32 angleZY,
                          irr::f32 angleXZ, int16_t id) {

    MMissileShotStruct* newStruct = new MMissileShotStruct;

    VThing* newThing =
         mParentRace->mThingManager->thing_initialise(*position, angleXY, angleZY, angleXZ, 6, 3, mOwner->ThingData->Id);

    newStruct->ThingPntr = newThing;
    newStruct->ReadyForCleanup = false;

    initialiseSHOT_MISSILE(newStruct);

    mMissileShotVec.push_back(newStruct);

    return newThing;
}

//Mainly contains the functionality of function
//processWEAPON_ROCKET_GUN of the original game
void VMLauncher::Update(irr::f32 frameDeltaTime) {
    int16_t v9;
    irr::f32 angleXY;
    irr::f32 angleXZ;
    irr::f32 angleZY;
    VVehicle* targetVehicle = nullptr;
    int16_t upgrade;
    int32_t v13;
    irr::core::vector3df* p_Position;
    irr::core::vector3df position;
    VThing* v15 = nullptr;
    int rNum;
    irr::f32 v18;
    int16_t triggerTime;

    std::vector<MMissileShotStruct*>::iterator it2;
    MMissileShotStruct* pntrMissileShot;

    //cleanup all missile shot objects that were marked to be cleaned up
    for (it2 = mMissileShotVec.begin(); it2 != mMissileShotVec.end(); ) {
        if ((*it2)->ReadyForCleanup) {
            pntrMissileShot = (*it2);

            it2 = mMissileShotVec.erase(it2);

            delete pntrMissileShot;
        } else {
            ++it2;
        }
    }

    //add delta time up to see when we need to update
    //the slower parts of the MGun
    mAbsTimeAcc += frameDeltaTime;

    if (mAbsTimeAcc >= 0.05f) {
        mAbsTimeAcc = 0.0f;

        //first update all currently existing missile shots
        std::vector<MMissileShotStruct*>::iterator itShot;
        for (itShot = mMissileShotVec.begin(); itShot != mMissileShotVec.end(); ++itShot) {
             processSHOT_MISSILE((*itShot));

             //we need to update the TimeSlice variable in
             //the SHOT_MISSILE Thing
             mParentRace->mThingManager->UpdateTimeSlice((*itShot)->ThingPntr);
        }

        //The source code below which is taken from "processWEAPON_ROCKET_GUN"
        //has to be executed every ~50ms according to measurements in the
        //emulator using the original game
        if (mOwner != nullptr) {
            if (Trigger) {
                if (!TriggerTime) {
                   v9 = mOwner->Stats.Weapons;
                   if (v9 >= 1666) {
                      //is no computer player?
                      if (mOwner->GetControlOrigin() != 8) {
                      //   mOwner->Stats.Weapons = v9 - 1666;
                         mOwner->Conditions.WeaponsUsed += 1666;
                      }
                      angleXZ = mOwner->View.AngleXZ;
                      angleXY = mOwner->View.AngleXY;
                      angleZY = mOwner->View.AngleZY;
                      if (Target) {
                          targetVehicle = mParentRace->GetVehicleWithId((size_t)(Target));
                          angleXY = mParentRace->mVCalc->angle_get_xy(mOwner->ThingData->Position, targetVehicle->ThingData->Position);
                          angleZY = mParentRace->mVCalc->angle_get_zy(mOwner->ThingData->Position, targetVehicle->ThingData->Position);
                      }
                      upgrade = Upgrade;
                      v13 = 1;
                      if (upgrade >= 0) {
                        if (upgrade < 3 || (v13 = 2, upgrade == 3)) {
                             p_Position = &mOwner->ThingData->Position;
                             do {
                                 v15 = CreateMissileShot(p_Position, angleXY, angleZY, angleXZ, mOwner->ThingData->Id);

                                 if (v15 != nullptr) {
                                    rNum = rand();
                                    v15->Count = ((rNum % 0x64u) < this->Count);
                                    ++RocketsLaunched;
                                    this->Status = this->Status ^ 0x10;
                                    v18 = -90.0f;
                                    if ((this->Status & 0x10) != 0) {
                                        v18 = 90.0f;
                                    }
                                    TriggerTime = 12;
                                    position.X = v15->Position.X;
                                    position.Y = v15->Position.Y;
                                    position.Z = v15->Position.Z - 0.03125f;
                                    mParentRace->mVCalc->move_xyz(position, v15->Movement.AngleXY + v18, 0.0f, 0.3125f);
                                    mParentRace->mThingManager->mapwho_move(v15, position);
                                    //sample_play(v7, 23);
                                    if (this->Target) {
                                        v15->Target = this->Target;
                                    }
                                    v15->Upgrade = this->Upgrade;
                                 }
                                 --v13;
                                 p_Position = &mOwner->ThingData->Position;
                             } while (v13);
                             Trigger = 0;
                             return;
                        }
                      }
                   }
    VMLauncherUpdate_LABEL_31:
                   Trigger = 0;
                   return;
                }
            } else {
                triggerTime = TriggerTime;
            }
            if (triggerTime > 0) {
                TriggerTime = triggerTime - 1;
            }
            goto VMLauncherUpdate_LABEL_31;
        }
    }
}
