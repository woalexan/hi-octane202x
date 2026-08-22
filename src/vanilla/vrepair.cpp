/*
 Copyright (C) 2026 Wolf Alexander

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

#include "vrepair.h"
#include "../race.h"
#include "vcalc.h"
#include "vtrack.h"
#include "../game.h"
#include "vvehicle.h"
#include "vthing.h"

VRepair::VRepair(Race* mParentRace, irr::core::vector3d<irr::f32> NewPosition,
                 irr::scene::ISceneManager* smgr) {
    mSmgr = smgr;
    mRace = mParentRace;

    irr::core::vector3df vanPos = mRace->mVCalc->IrrlichtToVanillaCoord(NewPosition);

    //get my ThingData
    //TODO: the -1 at the end is not correct, fix!
    //Member value = 9 means repair craft
    ThingData =
            mRace->mThingManager->thing_initialise_member(vanPos, 0.0f, 0.0f, 0.0f, 10, 9, -1);

    ThingData->Position = vanPos;
}

//We can only finish initialization
//after level data was loaded completely
//Therefore make 2 step progress for
//creating repair vehicle
void VRepair::Initialize() {
    initialiseVEHICLE_POLICE_HELICOPTER();

    RecoveryMesh = mSmgr->getMesh("extract/models/recov0-0.obj");
    RecoveryNode = mSmgr->addMeshSceneNode(RecoveryMesh);

    RecoveryNode->setScale(irr::core::vector3d<irr::f32>(1.0f,1.0f,1.0f));
    RecoveryNode->setMaterialFlag(irr::video::EMF_LIGHTING, mRace->mGame->enableLightning);
    RecoveryNode->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);

    repair_vehicle_set_camera();
    UpdateSceneNode();
}

VRepair::~VRepair() {
    //cleanup Irrlicht stuff

    //remove SceneNode
    this->RecoveryNode->remove();

    //remove mesh
    mSmgr->getMeshCache()->removeMesh(RecoveryMesh);

    if (ThingData != nullptr) {
        mRace->mThingManager->thing_delete(ThingData);
        ThingData = nullptr;
    }
}

void VRepair::UpdateSceneNode() {
    if (RecoveryNode == nullptr) {
        return;
    }

    mRace->UpdateSceneNodeModel(RecoveryNode, &View);
}

void VRepair::initialiseVEHICLE_POLICE_HELICOPTER() {
    ThingData->Action = 31;

    irr::f32 v2 = mRace->mVCalc->map_altitude_column_and_floor(ThingData->Position);
    ThingData->Position.Z = (v2 +  4.0f);
    IncrementAdd.AngleXY = 9.99755859375f;
    IncrementAdd.SpeedActual = 0.1640625f;

    //init other variables as well (just to make sure!)
    ThingData->Movement.AngleXY = 0.0f;
    ThingData->Movement.AngleXZ = 0.0f;
    ThingData->Movement.AngleZY = 0.0f;
    ThingData->Movement.SpeedActual = 0.0f;

    ThingData->Displacement.X = 0.0f;
    ThingData->Displacement.Y = 0.0f;
    ThingData->Displacement.Z = 0.0f;

    //CollideSize does not really matter, is not used in repair
    //vehicle; just want to make sure that I do not have
    //undefined variables, the init values are just
    //a random picked value, I am not sure if the game
    //actually initializes this and to which value
    ThingData->CollideSize.set(1.0f, 1.0f, 1.0f);
}

void VRepair::repair_vehicle_move_toward(irr::core::vector3df t_position,
                                         irr::core::vector3df& delta, irr::f32 speed, irr::f32 dist) {
    irr::core::vector3df position_from;
    irr::f32 xy;
    irr::f32 v10;
    irr::f32 v11;
    irr::f32 v12;
    irr::f32 v14;
    irr::f32 v15;

    position_from = ThingData->Position;
    xy = mRace->mVCalc->angle_get_xy(position_from, t_position);
    v10 = mRace->mVCalc->angle_get_difference(ThingData->Movement.AngleXY, xy);
    if (dist < 8.0f) {
        v11 = speed * dist;
        if (speed >= 0.39453125f) {
            v11 = 0.390625f * dist;
        }
        speed = (v11 / 8.0f) + 0.05859375f;
    }

    if (v10 >= 0.50537109375f) {
        speed = 0.0f;
    }

    v12 = IncrementAdd.AngleXY;
    if (v10 >= -v12) {
        //v13 = delta;
        if (v12 >= v10) {
            goto repair_vehicle_move_toward_LABEL_12;
        }
        v10 = IncrementAdd.AngleXY;
    } else {
        v10 = -IncrementAdd.AngleXY;
    }
    //v13 = delta;
repair_vehicle_move_toward_LABEL_12:
    ThingData->Movement.AngleXY += v10;
    mRace->mVCalc->move_displacement_set(delta, ThingData->Movement.AngleXY, 0.0f, speed);
    position_from = ThingData->Position + delta;
    v14 = mRace->mVCalc->map_altitude_column_and_floor(position_from);
    v15 = v14 - (position_from.Z - 6.0f);
    if (v15 >= -0.1171875f) {
        if (v15 >= 0.16015625f) {
            v15 = 0.15625f;
        }
    } else {
        v15 = -0.1171875f;
    }
    position_from.Z += v15;
    delta.Z += v15;

    mRace->mThingManager->mapwho_move(ThingData, position_from);
    //alternative right now:
    //ThingData->Position = position_from;
}

void VRepair::repair_vehicle_rotate_car(VVehicle* targetVehicle) {
    uint16_t v2;
    irr::core::vector3df position;
    irr::f32 xy;
    irr::f32 difference;

    v2 = mRace->mVTrack->track_waypoint_child(BumpDamage);
    mRace->mVTrack->track_waypoint_position_set(position, v2);
    xy = mRace->mVCalc->angle_get_xy(targetVehicle->ThingData->Position, position);
    difference = mRace->mVCalc->angle_get_difference(targetVehicle->ThingData->Movement.AngleXY, xy);
    if (difference >= -59.996337890625f) {
        if (difference >= 60.0018310546875f) {
            difference = 59.996337890625f;
        }
    } else {
        difference = -59.996337890625f;
    }
    targetVehicle->ThingData->Movement.AngleXY += difference;
}

uint16_t VRepair::repair_vehicle_find_drop_waypoint(VVehicle* targetVehicle) {
   uint16_t result;
   uint16_t v6;
   int32_t v9minIdx;
   uint16_t v10;
   uint16_t v7;
   uint16_t v8;
   irr::f32 v13;
   irr::f32 v3;
   int32_t v2 = 30;
   int32_t v12;
   int32_t v15;
   bool firstLoop;
   VThing* checkPntStructCounter0 = nullptr;
   VThing* checkPntStructCounter2 = nullptr;
   irr::core::vector3df position;
   irr::f32 colSizeX;
   irr::f32 colSizeY;

    std::vector<VThing*>::iterator it;

   //which checkpoint is in this vehicle Count[0] value?
   for (it = mRace->mVanillaCheckpointVec.begin();
        it != mRace->mVanillaCheckpointVec.end(); ++it) {
          if ((*it)->Index == targetVehicle->Counter[0]) {
              checkPntStructCounter0 = (*it);
              break;
          }
   }

   if (checkPntStructCounter0 == nullptr) {
       v6 = 0;
   } else {
       position = checkPntStructCounter0->Position;
       colSizeX = checkPntStructCounter0->CollideSize.X;
       colSizeY = checkPntStructCounter0->CollideSize.Y;

       v6 = mRace->mVTrack->track_waypoint_colide_area(position, colSizeX, colSizeY);
   }

   v7 = v6;
   v8 = v6;
   v9minIdx = static_cast<int32_t>(v6);

   //which checkpoint is in this vehicle Count[2] value?
   for (it = mRace->mVanillaCheckpointVec.begin();
        it != mRace->mVanillaCheckpointVec.end(); ++it) {
          if ((*it)->Index == targetVehicle->Counter[2]) {
              checkPntStructCounter2 = (*it);
              break;
          }
   }

   if (checkPntStructCounter2 == nullptr) {
       v10 = 0;
   } else {
       position = checkPntStructCounter2->Position;
       colSizeX = checkPntStructCounter2->CollideSize.X;
       colSizeY = checkPntStructCounter2->CollideSize.Y;

       v10 = mRace->mVTrack->track_waypoint_colide_area(position, colSizeX, colSizeY);
   }

   if (v7 != v10) {
        v12 = 30;
        firstLoop = true;
        do {
            --v2;
            if (!(v12 << 16)) {
                break;
            }
            v13 = mRace->mVTrack->track_waypoint_distance(targetVehicle->ThingData->Position, v8);
            //we want to find the minimum value of v13
            if (firstLoop) {
               //whatever the value is the first
               //loop result is the first minimum
               v3 = v13;
               v9minIdx = static_cast<int32_t>(v8);
               firstLoop = false;
            } else if (v13 < v3) {
                //we found a new minimum value
                v3 = v13;
                v9minIdx = static_cast<int32_t>(v8);
            }
            v8 = mRace->mVTrack->track_waypoint_child(v8);
            v12 = v2;
        } while (v8 != v10);
   }

   result = static_cast<uint16_t>(v9minIdx);
   if (!(v9minIdx << 16)) {
       //This hardcoded condition is really in the original
       //game (Playstation 1 version). Not sure what the effect is
       //Maybe it fixes a level design issue in level 5
       v15 = (v9minIdx << 16);
       //currentLevelNr starts to count with 0
       //for the first level in the original game
       if (mRace->IsOriginalLevel5Loaded()) {
           v15 = 2031616;
       }

       result = static_cast<uint16_t>(v15 >> 16);
       return result;
   }

   return result;
}

uint8_t VRepair::repair_vehicle_drop_point_ok(uint16_t waypoint) {
    std::vector<VRepair*>::iterator it;
    VRepair* pntr = nullptr;
    int8_t action;

    for (it = mRace->mVanillaRepairVehicleVec.begin();
         it != mRace->mVanillaRepairVehicleVec.end(); ++it) {
            if ((*it) != this) {
                pntr = (*it);
                action = pntr->ThingData->Action;
                if (((action - 29) < 2) ||
                   (action == 28) && pntr->ThingData->Count) {
                    if (pntr->BumpDamage == waypoint) {
                       return 0;
                    }
                }
            }
    }

    return 1;
}

void VRepair::repair_vehicle_set_camera() {
    //the View is the position and orientation of the
    //repair craft model
    View.Position = ThingData->Position + irr::core::vector3df(0.0f, 0.0f, 0.8f);
    View.AngleXY = ThingData->Movement.AngleXY;
    View.AngleZY = ThingData->Movement.AngleZY;
    View.AngleXZ = ThingData->Movement.AngleXZ; // + 4.0f * Increment.AngleXY;
}

void VRepair::repair_vehicle_execute_action0x1A(irr::core::vector3df pos1,
                                                irr::core::vector3df pos2, irr::core::vector3df& delta) {
    irr::f32 dist;

    dist = mRace->mVCalc->distance_get_xy(pos1, pos2);

    if (dist < 6.00390625f) {
        ThingData->Action = 0x1B;
        ThingData->Count = 20;
    } else {
        repair_vehicle_move_toward(pos2, delta, 0.859375f, dist);
    }

    repair_vehicle_set_camera();
}

void VRepair::repair_vehicle_execute_action0x1B(irr::core::vector3df pos1,
                                                irr::core::vector3df pos2, irr::core::vector3df& delta) {

    irr::f32 count = (irr::f32)(ThingData->Count);

    irr::f32 v16 = pos2.X - pos1.X;
    irr::f32 v17 = (v16 / count);
    irr::f32 v18 = v17;
    delta.X = v17;
    irr::f32 v20 = pos2.Y - pos1.Y;
    irr::f32 v21 = (v20 / count);
    delta.Y = v21;
    irr::f32 v23 = pos2.Z - (pos1.Z - 0.78125f);
    pos1.X += v18;
    pos1.Y += v21;
    delta.Z = (v23 / count);
    pos1.Z += delta.Z;

    mRace->mThingManager->mapwho_move(ThingData, pos1);
    //next line is the current alternative
    //ThingData->Position = pos1;
    ThingData->Count--;

    if (!ThingData->Count) {
        TargetVehicle->ThingData->Action = 0x17;
        ThingData->Action = 0x1C;
        BumpDamage = repair_vehicle_find_drop_waypoint(TargetVehicle);
        if (BumpDamage) {
            ThingData->Count = 0;
            repair_vehicle_set_camera();
            return;
        }

        BumpDamage = mRace->mVTrack->track_waypoint_absolute_nearest(ThingData->Position);
        if (BumpDamage) {
            ThingData->Count = 0;
            repair_vehicle_set_camera();
            return;
        }

        BumpDamage = 1;
        ThingData->Count = 0;
        repair_vehicle_set_camera();
        return;
    }

    repair_vehicle_set_camera();
}

void VRepair::repair_vehicle_execute_action0x1C(irr::core::vector3df pos1,
                                                irr::core::vector3df pos2, irr::core::vector3df& delta) {
    irr::f32 xy;
    uint16_t v29;
    irr::f32 v30;
    irr::f32 difference;
    irr::core::vector3df position;
    bool v32;

    if (!ThingData->Count) {
        mRace->mVTrack->track_waypoint_position_set(pos2, BumpDamage);
        xy = mRace->mVCalc->distance_get_xy(pos1, pos2);

        if (xy < 4.00390625f) {
            ++ThingData->Count;
        } else {
            repair_vehicle_move_toward(pos2, delta, 0.5859375f, xy);
            pos1 = ThingData->Position;

            mRace->mThingManager->mapwho_move(ThingData, pos1);
            //ThingData->Position = pos1;
            pos1.Z -= 0.78125f;
            mRace->mThingManager->mapwho_move(TargetVehicle->ThingData, pos1);
            //TargetVehicle->ThingData->Position = pos1;
        }

        v29 = mRace->mVTrack->track_waypoint_child(BumpDamage);
        mRace->mVTrack->track_waypoint_position_set(position, v29);
        v30 = mRace->mVCalc->angle_get_xy(TargetVehicle->ThingData->Position, position);
        difference =
                mRace->mVCalc->angle_get_difference(TargetVehicle->ThingData->Movement.AngleXY, v30);

        v32 = (difference < 60.0018310546875f);
        if (difference < -59.996337890625f) {
            difference = -59.996337890625f;
            TargetVehicle->ThingData->Movement.AngleXY += difference;
            TargetVehicle->vehicle_set_camera();
            repair_vehicle_set_camera();
            return;
        }

        if (!v32) {
            difference = 59.996337890625f;
        }

        TargetVehicle->ThingData->Movement.AngleXY += difference;
        TargetVehicle->vehicle_set_camera();
        repair_vehicle_set_camera();
        return;
    }

    if (ThingData->Count == 1) {
        if (repair_vehicle_drop_point_ok(BumpDamage)) {
            ThingData->Action = 0x1D;
            ThingData->Count = 14;
        } else {
            BumpDamage =
                    mRace->mVTrack->track_waypoint_child(BumpDamage);
            ThingData->Count = 0;
        }
    }

    repair_vehicle_set_camera();
}

void VRepair::repair_vehicle_execute_action0x1D(irr::core::vector3df pos1,
                                                irr::core::vector3df pos2, irr::core::vector3df& delta) {

    mRace->mVTrack->track_waypoint_position_set(pos2, BumpDamage);

    irr::f32 count = (irr::f32)(ThingData->Count);

    irr::f32 v34 = pos2.X - pos1.X;
    irr::f32 v35 = (v34 / count);
    delta.X = v35;
    irr::f32 v38 = pos2.Y - pos1.Y;
    irr::f32 v39 = (v38 / count);
    delta.Y = v39;
    irr::f32 v41 = pos2.Z - (pos1.Z - 1.5625f);
    pos1.X += v35;
    pos1.Y += v39;
    delta.Z = (v41 / count);
    pos1.Z += delta.Z;

    mRace->mThingManager->mapwho_move(ThingData, pos1);
    //next line is the current alternative
    //ThingData->Position = pos1;
    pos1.Z -= 0.78125f;
    mRace->mThingManager->mapwho_move(TargetVehicle->ThingData, pos1);
    //next line is the current alternative
    //TargetVehicle->ThingData->Position = pos1;
    TargetVehicle->vehicle_set_camera();
    ThingData->Count--;

    if (ThingData->Count == 2) {
        ThingData->Action = 0x1E;
        ThingData->Count = 0;
    }

    repair_vehicle_set_camera();
}

void VRepair::repair_vehicle_execute_action0x1E(irr::core::vector3df pos1,
                                                irr::core::vector3df pos2, irr::core::vector3df& delta) {

    bool v32;
    irr::f32 zpos;
    irr::f32 v48;
    uint16_t v44;
    irr::f32 difference;
    irr::core::vector3df position;
    irr::f32 v45;

    if (ThingData->Count == 1) {
        TargetVehicle->CurrentWaypoint = BumpDamage;
        TargetVehicle->FlightModel.Flag.AutoStop = false;
        TargetVehicle->ThingData->Action = 25;
        ++ThingData->Count;
    } else if (ThingData->Count >= 2) {
        if (ThingData->Count == 2) {
           pos1.Z += 0.09765625f;
           mRace->mThingManager->mapwho_move(ThingData, pos1);
           //ThingData->Position = pos1;
           zpos = pos1.Z;
           v48 = mRace->mVCalc->map_altitude_column_and_floor(pos1);
           if ((v48 + 5.0f) < zpos) {
               ThingData->Action = 31;
               repair_vehicle_set_camera();
               return;
           }
        }
    } else if (!ThingData->Count) {
        if (TargetVehicle->ThingData->Action == 24) {
            ++ThingData->Count;
        }

        v44 = mRace->mVTrack->track_waypoint_child(BumpDamage);
        mRace->mVTrack->track_waypoint_position_set(position, v44);
        v45 = mRace->mVCalc->angle_get_xy(TargetVehicle->ThingData->Position, position);
        difference =
                mRace->mVCalc->angle_get_difference(TargetVehicle->ThingData->Movement.AngleXY, v45);

        v32 = (difference < 60.0018310546875f);
        if (difference >= -59.996337890625f) {
            if (!v32) {
                difference = 59.996337890625f;
            }
        } else {
            difference = -59.996337890625f;
        }

        TargetVehicle->ThingData->Movement.AngleXY += difference;
        TargetVehicle->vehicle_set_camera();
    }

    repair_vehicle_set_camera();
}

void VRepair::repair_vehicle_execute_action0x1F() {
    //search for a vehicle with Action == 0x14
    //This action means a vehicle needs recovery
    std::vector<VVehicle*>::iterator it;
    VVehicle* pntr = nullptr;

    for (it = mRace->mVanillaCraftVec.begin(); it != mRace->mVanillaCraftVec.end(); ++it) {
        if ((*it)->ThingData->Action == 0x14) {
            pntr = (*it);
            break;
        }
    }

    if (pntr != nullptr) {
        //we found a vehicle that needs help
        TargetVehicle = pntr;
        TargetVehicle->ThingData->Action = 0x16;

        //I skipped some code regarding deathmatch here!

        ThingData->Action = 0x1A;
        repair_vehicle_set_camera();
        return;
    }

    repair_vehicle_set_camera();
}

//Note 01.08.2026: Function processVEHICLE_POLICE_HELICOPTER
//in the original game (measured with emulator) is called 3 times in a row
//after 0.115 ms again, followed by a call ~50 ms later; I can only assume right
//now that the three calls in 0.115ms distance inbetween could be a
//programming bug, and only the 50ms call period time does sense and is
//wanted;
void VRepair::Update(irr::f32 frameDeltaTime) {
    irr::core::vector3df position1;
    irr::core::vector3df position2;
    irr::core::vector3df delta;

    //we want to increment mTimeSlice every 50ms
    //in the original game it starts counting at 0, increases every 50ms
    //and the overflows back from 0xFF to 00
    mAbsTimeIntegrator += frameDeltaTime;
    if (mAbsTimeIntegrator >= 0.05) {
        mAbsTimeIntegrator = 0.0f;
        if (ThingData->TimeSlice < 0xFF) {
            ThingData->TimeSlice++;
        } else {
            ThingData->TimeSlice = 0;
        }

        //the following code should run every ~50ms
        //timing close enough when called
        //here

        //position1 holds the current position
        //of this recovery vehicle
        position1 = ThingData->Position;

        //position2 holds the current position
        //of the target vehicle we want to help
        if (TargetVehicle != nullptr) {
            position2 = TargetVehicle->ThingData->Position;
        } else {
            position2.set(0.0f, 0.0f, 0.0f);
        }

        switch (ThingData->Action) {
            case 0x1A: {
                repair_vehicle_execute_action0x1A(position1, position2, delta);
                break;
            }

            case 0x1B: {
                repair_vehicle_execute_action0x1B(position1, position2, delta);
                break;
            }

            case 0x1C: {
               repair_vehicle_execute_action0x1C(position1, position2, delta);
               break;
            }

            case 0x1D: {
               repair_vehicle_execute_action0x1D(position1, position2, delta);
               break;
            }

            case 0x1E: {
                repair_vehicle_execute_action0x1E(position1, position2, delta);
                break;
            }

            case 0x1F: {
                //this is the recovery vehicle idle case, where the
                //recovery vehicle searches constantly for a vehicle that
                //needs help
                repair_vehicle_execute_action0x1F();
                break;
            }

            default: {
                repair_vehicle_set_camera();
                break;
            }
        }

        UpdateSceneNode();
    }
}

