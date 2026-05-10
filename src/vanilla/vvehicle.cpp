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
//completely different to the one in the original game. The original uses X and Y axis for the tile map, and Z is the height. I have
//X and Z for the tile map, and Y is the height. And to make things worse my Irrlicht vertices Y coordinates have a swapped sign (are negative)
//currently. I will need to find a way to either adjust the source code below without introducing new bugs, or to change my project
//to use the same coordinate system soon.

//I really want to thank aybe for giving me the opportunity to look much deeper into the original game inner workings as I was ever able before.
//Without this support I would not have been able to hopefully advance the current project more true to the original.

#include "vvehicle.h"
#include "../race.h"
#include "../game.h"
#include "../vanilla/vcalc.h"

void VVehicle::Update(irr::f32 frameDeltaTime) {
    //we want to increment mTimeSlice every 50ms
    //in the original game it starts counting at 0, increases every 50ms
    //and the overflows back from 0xFF to 00
    mAbsTimeIntegrator += frameDeltaTime;
    if (mAbsTimeIntegrator >= 0.05) {
        mAbsTimeIntegrator = 0.0f;
        if (ThingData.mTimeSlice < 0xFF) {
            ThingData.mTimeSlice++;
        } else {
            ThingData.mTimeSlice = 0;
        }
    }

    irr::core::vector3df delta;

    vehicle_control_from_player();
    vehicle_get_track_friction();
    vehicle_calculate_angle();
    vehicle_calculate_thrust(delta);
    mRace->mVCalc->move_displacement_slope(ThingData.Position, Slope);
    vehicle_calculate_momentum(delta);
    vehicle_calculate_movement_delta(delta);
    vehicle_colide_map(delta);
    vehicle_move_altitude(delta);
    vehicle_move_roll(delta);
    vehicle_move_tilt(delta);
    vehicle_move_mapwho(delta);
}

void VVehicle::SetupFlightModelConstants() {
    //setting up flight model constants
    //All values configured as in function initialiseVEHICLE_CAR in original game
    IncrementAdd.AngleXY = mRace->mVCalc->VanillaRawAngleToMyFloatingAngle(200);
    IncrementAdd.SpeedActual = mRace->mVCalc->FixedPointToFloat8D8(26);
    IncrementSub.AngleXY = mRace->mVCalc->VanillaRawAngleToMyFloatingAngle(100);
    IncrementSub.SpeedActual = mRace->mVCalc->FixedPointToFloat8D8(160);
    IncrementLimit.AngleXY = mRace->mVCalc->VanillaRawAngleToMyFloatingAngle(3640);
    IncrementLimit.SpeedActual = mRace->mVCalc->FixedPointToFloat8D8(240);

    mThrustEffectiveness = mRace->mVCalc->FixedPointToFloat8D8(100);
    mSideslipFriction = mRace->mVCalc->FixedPointToFloat8D8(7);
    mSideslipToThrust = mRace->mVCalc->FixedPointToFloat8D8(60);
    mBounce = mRace->mVCalc->FixedPointToFloat8D8(50);

    Stats.Behind = 100;

    mFriction = mRace->mVCalc->FixedPointToFloat8D8(10);
    mFrictionLimit = mRace->mVCalc->FixedPointToFloat8D8(15);

    FlightModel.SizeForward = mRace->mVCalc->FixedPointToFloat8D8(60);
    FlightModel.SizeRear = mRace->mVCalc->FixedPointToFloat8D8(60);
    FlightModel.SizeSideways = mRace->mVCalc->FixedPointToFloat8D8(60);
    FlightModel.SizeSensorOffset = mRace->mVCalc->FixedPointToFloat8D8(100);

    FlightModel.RideHeight = mRace->mVCalc->FixedPointToFloat8D8(50);
    FlightModel.BrakePower = mRace->mVCalc->FixedPointToFloat8D8(16);

    FlightModel.FrontLeft.Rebound = mRace->mVCalc->FixedPointToFloat8D8(50);
    FlightModel.FrontLeft.ReboundLimit = mRace->mVCalc->FixedPointToFloat8D8(80);

    FlightModel.FrontRight.Rebound = mRace->mVCalc->FixedPointToFloat8D8(50);
    FlightModel.FrontRight.ReboundLimit = mRace->mVCalc->FixedPointToFloat8D8(80);

    FlightModel.RearLeft.Rebound = mRace->mVCalc->FixedPointToFloat8D8(-56);
    FlightModel.RearLeft.ReboundLimit = mRace->mVCalc->FixedPointToFloat8D8(80);

    FlightModel.RearRight.Rebound = mRace->mVCalc->FixedPointToFloat8D8(-56);
    FlightModel.RearRight.ReboundLimit = mRace->mVCalc->FixedPointToFloat8D8(80);
}

VVehicle::VVehicle(Race* mParentRace, irr::core::vector3d<irr::f32> NewPosition,
                   irr::core::vector3d<irr::f32> NewFrontAt) {

   mRace = mParentRace;

   SetupFlightModelConstants();

   //TODO important: Preset Sensor Zpos values as in the original game!
   //See function initialiseVEHICLE_CAR

   mCraftMesh = mRace->mGame->mSmgr->getMesh(irr::io::path("extract/models/car0-0.obj"));
   mCraftNode = mRace->mGame->mSmgr->addMeshSceneNode(mCraftMesh);

   //set player model initial orientation and position, later player craft is only moved by physics engine
   //also current change in Rotation of player craft model compared with this initial orientation is controlled by a
   //quaterion inside the physics engine object for this player craft as well
   mCraftNode->setRotation(((NewFrontAt-NewPosition).normalize()).getHorizontalAngle()+ irr::core::vector3df(0.0f, 180.0f, 0.0f));
   mCraftNode->setPosition(NewPosition);

   ThingData.Position = NewPosition;

   mCraftNode->setDebugDataVisible(EDS_BBOX);
   //mCraftNode->setDebugDataVisible(EDS_OFF);

   mCraftNode->setScale(irr::core::vector3d<irr::f32>(1,1,1));
   mCraftNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
   mCraftNode->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
   mCraftNode->setVisible(true);
}

void VVehicle::vehicle_get_track_friction() {
    if (FlightModel.Flag.Airbourn) {
        //if we are currently air bourne there is no friction
        mFriction = 0.0f;
        return;
    }

    //Run the following code only every 100ms once
    //Original game does the same
    if ((ThingData.mTimeSlice & 1) == 0) {
        //no, we are currently not air bourne
        //get the current friction value from the tile below
        uint16_t tileFriction = mRace->mVCalc->map_colide_friction(ThingData.Position);
        if (!tileFriction) {
            //friction is 0, set 15 (original game does the same)
            tileFriction = 15;
        }

        irr::f32 tileFrictionFloating =
                mRace->mVCalc->FixedPointToFloat8D8((int16_t)(tileFriction));

        irr::f32 newFriction;

        if (mFriction >= tileFrictionFloating) {
            newFriction = mFriction - 0.00390625f;
            if (tileFrictionFloating >= mFriction) {
                return;
            }
        } else {
            newFriction = mFriction + 0.00390625f;
        }

        mFriction = newFriction;
    }
}

void VVehicle::vehicle_calculate_angle() {
    //is the input angle != 0?
    if (fabs(MovementInput.AngleXY) > 0.175784) {
         irr::f32 v5 = 0.0f;
         irr::f32 vHelp = 0.0f;
         if (fabs(Increment.AngleXY) > 0.175784) {
           v5 = Increment.AngleXY / fabs(Increment.AngleXY);
         }

        if (fabs(MovementInput.AngleXY) > 0.175784) {
           vHelp = (MovementInput.AngleXY / fabs(MovementInput.AngleXY));
           if (v5 != vHelp) {
               goto LABEL_15_vehicle_calculate_angle;
           }
       } else if (fabs(v5) > 0.00390625f) {
LABEL_15_vehicle_calculate_angle:
           Increment.AngleXY *= (0.84375f);
           goto LABEL_16_vehicle_calculate_angle;
       }

LABEL_16_vehicle_calculate_angle:
       Increment.AngleXY += MovementInput.AngleXY;
       goto LABEL_18_vehicle_calculate_angle;
    }

    Increment.AngleXY *= (0.890625f);

LABEL_18_vehicle_calculate_angle:
    //Increment.AngleXY seems to be a relative angle
    //limit the maximum possible range
    if (Increment.AngleXY >= -IncrementLimit.AngleXY) {
        if (IncrementLimit.AngleXY < Increment.AngleXY) {
            Increment.AngleXY = IncrementLimit.AngleXY;
        }
    } else {
        Increment.AngleXY = -IncrementLimit.AngleXY;
    }

    ThingData.Movement.AngleXY += Increment.AngleXY;
}

void VVehicle::vehicle_calculate_thrust(irr::core::vector3df& delta) {
    //TODO: add autodrive stuff at the top of this routine

    if (Increment.SpeedActual >= 0.0f) {
        //we are not Deaccelerating right now
        if (IncrementLimit.SpeedActual < Increment.SpeedActual) {
            Increment.SpeedActual = IncrementLimit.SpeedActual;
        }
    } else {
        //we are deaccelerating right now
        Increment.SpeedActual = 0.0f;
    }

    //TODO: add stuff about out of fuel at the end of this function
    ThingData.Movement.SpeedActual = (mThrustEffectiveness * Increment.SpeedActual) / 0.390625f;
    mRace->mVCalc->move_displacement_set(delta, ThingData.Movement.AngleXY, 0.0f, (ThingData.Movement.SpeedActual / 16.0f));
    ThingData.Movement.SpeedActual = Increment.SpeedActual;
}

void VVehicle::vehicle_calculate_momentum(irr::core::vector3df& delta) {
    irr::core::vector3df v44;

    delta.X += (Slope.X / 64.0f);
    delta.Y += (Slope.Y / 64.0f);
    v44.X = (Momentum.DeltaX / 4.0f);
    v44.Y = (Momentum.DeltaY / 4.0f);

    //TODO: add later if I care about a Bonus
    //Bonus.Xpos = v44.X * (Stats.Behind - 100) / 100
    //Bonus.Ypos = v44.Y * (Stats.Behind - 100) / 100

    irr::f32 v7;
    irr::f32 YPos;

    if (!FlightModel.Flag.Airbourn) {
        irr::f32 v6 = sqrt(v44.Y * v44.Y + v44.X * v44.X);
        if (v6 < 0.390625f) {
            v7 = 0.1953125f - v6;
            if (v7 >= 0.0f) {
                if (v7 >= 0.1953125) {
                    v7 = 0.1953125;
                }
            } else {
                v7 = 0.0f;
            }

            YPos = delta.Y;
            delta.X += delta.X * (v7 / 0.390625f);
            delta.Y = YPos + YPos * (v7 / 0.390625f);
        }

        Momentum.DeltaX += delta.X;
        Momentum.DeltaY += delta.Y;

        //** Add the missing booster stuff later **/

        if (FlightModel.Flag.Brake) {
            if (FlightModel.FunctionFlag.Brake) {
                Momentum.DeltaX *= (0.9765625f - FlightModel.BrakePower);
                Momentum.DeltaY *= (0.9765625f - FlightModel.BrakePower);
            }

        }
    }
}

//Note 01.05.2026: Function below is completely untested!
void VVehicle::vehicle_calculate_movement_delta(irr::core::vector3df& delta) {
    irr::core::vector3df position1(0.0f, 0.0f, 0.0f);
    irr::f32 v5;
    irr::f32 v7;
    irr::f32 v9;

    //TODO: add the Bump and Bonus four commented out lines below, if I know what
    //the do exactly

    delta.X = Momentum.DeltaX / 4.0f;
    delta.Y = Momentum.DeltaY / 4.0f;
    delta.Z = Displacement.Z;
    //delta.X += Bump.X;
    //delta.Y += Bump.Y;
    Stats.Velocity = mRace->mVCalc->distance_get_xyz(position1, delta);
    //delta.X += Bonus.X;
    irr::f32 Xpos = delta.X;
    //delta.Y += Bonus.Y;

    if (Xpos < -0.9765625f) {
        v5 = -0.9765625f;
vehicle_calculate_movement_delta_LABEL_4:
        delta.X = v5;
        goto vehicle_calculate_movement_delta_LABEL_5;
    }
    v5 = 0.9765625f;
    if (Xpos >= 0.98046875f) {
        goto vehicle_calculate_movement_delta_LABEL_4;
    }
vehicle_calculate_movement_delta_LABEL_5:

    //TODO: I am pretty sure I need to swap coordinates below, or not?

    irr::f32 Ypos = delta.Y;
    v7 = -0.9765625f;

    if ( (Ypos < -0.9765625f) || (v7 = 0.9765625f, Ypos >= 0.98046875f)) {
        delta.Y = v7;
    }

    irr::f32 Zpos = delta.Z;
    v9 = -0.9765625f;
    if ( (Zpos < -0.9765625f) || (v9 = 0.9765625f, Zpos >= 0.98046875f)) {
        delta.Z = v9;
    }

    Displacement.X = Momentum.DeltaX / 4.0f;
    Displacement.Y = Momentum.DeltaY / 4.0f;
}

//Note 01.05.2026: Function below is completely untested!
void VVehicle::vehicle_move_altitude(irr::core::vector3df& delta) {
     irr::core::vector3df position;

     //TODO Important: Do I need to swap the coordinates? Y is height for me, Z in the
     //original game!!! Most likely this code is still wrong

     position.X = ThingData.Position.X + delta.X;
     position.Y = ThingData.Position.Y + delta.Y;
     position.Z = ThingData.Position.Z + delta.Z;

     irr::f32 craftHeightSum = FlightModel.FrontLeft.Zpos + FlightModel.FrontRight.Zpos;
     position.Z = craftHeightSum / 2.0f;

     irr::f32 terrainHeight = mRace->mVCalc->map_altitude_column_and_floor(position);

     irr::f32 distCraftAboveTerrain = position.Z - terrainHeight;

     //Are we currently Airbourne?
     if (distCraftAboveTerrain < 0.58984375f) {
         //We are not Airbourn currently
         FlightModel.Flag.Airbourn = false;
         if (distCraftAboveTerrain < 0.390625f) {
             position.Z = terrainHeight + 0.390625f;
         }
     } else {
         //We are Airbourn currently
        FlightModel.Flag.Airbourn = true;
     }

     Displacement.Z = position.Z - ThingData.Position.Z;
     delta.Z = position.Z - ThingData.Position.Z;
}

//Note 01.05.2026: Function below is completely untested!
void VVehicle::vehicle_move_tilt(irr::core::vector3df& delta) {
    irr::core::vector3df position;
    irr::f32 v11;
    irr::f32 v6;
    irr::f32 v8;
    bool v10;
    irr::f32 craftTerrainAvgDist;
    irr::f32 terrainHeight;

    position.X = ThingData.Position.X + delta.X;
    position.Y = ThingData.Position.Y + delta.Y;
    position.Z = ThingData.Position.Z + delta.Z;

    if (FlightModel.Flag.Airbourn) {
        //yes, we are in the air right now
        ThingData.Movement.AngleZY += 0.2471923828125f;

        v11 = -45.0f;

        if (ThingData.Movement.AngleZY < -45.0f) {
            goto vehicle_move_tilt_LABEL_10;
        }

        v11 = 45.0f;

        //Note: The assembly at this location is really tricky;
        //Pseudo - C code completely differs to what code I have below, but
        //from investigation of pure assembly it seems more that this below is correct?
        //but could be wrong! and source of a possible bug
        if (ThingData.Movement.AngleZY < 45.0054931640625f) {
            v11 = 0.0054931640625f;
            goto vehicle_move_tilt_LABEL_10;
        }
    } else {
        //we are not AirBourn
        //get terrain height below
        terrainHeight = mRace->mVCalc->map_altitude_column_and_floor(position);
        v6 = FlightModel.SizeForward;
        craftTerrainAvgDist = terrainHeight - ((FlightModel.FrontRight.ZposFloor
                                               + FlightModel.FrontLeft.ZposFloor) * 0.5f);

        if (fabs(craftTerrainAvgDist) < v6) {
          //not sure if for craftTerrainAvgDist parameter in arctanPlusMultiply32 call below
          //I need to give whole float number, or only fractional part? If bug clarify later
          v8 =
            ThingData.Movement.AngleZY +
               (mRace->mVCalc->arctanPlusMultiply32(craftTerrainAvgDist, -v6) - ThingData.Movement.AngleZY) / 4.0f;

          ThingData.Movement.AngleZY = v8;
          v11 = -24.9993896484375f;
          v10 = (v8 < v11);

          if (!v10) {
              if (v8 >= 25.0048828125f) {
                  ThingData.Movement.AngleZY = 24.9993896484375f;
              }
              return;
          }

vehicle_move_tilt_LABEL_10:
          ThingData.Movement.AngleZY = v11;
        }
    }
}

//Note 01.05.2026: Function below is completely untested!
void VVehicle::vehicle_move_roll(irr::core::vector3df& delta) {
    irr::f32 v10;
    irr::f32 v8;
    irr::f32 comp;
    irr::f32 craftHeightDiff;
    irr::f32 absCraftHeightDiff;
    bool v9;

    if (FlightModel.Flag.Airbourn) {
        //yes, we are in the air right now
        v10 = 0.9375f * ThingData.Movement.AngleXZ;
        goto vehicle_move_roll_LABEL_9;
    }

    craftHeightDiff = FlightModel.FrontLeft.ZposFloor -
                                 FlightModel.FrontRight.ZposFloor;

    absCraftHeightDiff = fabs(craftHeightDiff);

    if (absCraftHeightDiff < (2.0f * FlightModel.SizeSideways)) {
        v10 = ThingData.Movement.AngleXZ +
                (mRace->mVCalc->arctanPlusMultiply32(craftHeightDiff, -2.0f * FlightModel.SizeSideways)
                 -ThingData.Movement.AngleXZ) / 8.0f;

        ThingData.Movement.AngleXZ = v10;
        v8 = v10;
        comp = -24.9993896484375f;
        v9 = (v10 < comp);
        v10 = comp;

        if (!v9) {
              if (v8 >= 25.0048828125f) {
                  ThingData.Movement.AngleXZ = 24.9993896484375f;
              }
              return;
        }

vehicle_move_roll_LABEL_9:
          ThingData.Movement.AngleXZ = v10;
    }
}

void VVehicle::vehicle_sensor_point_process(VehicleSensorPointStruct& sensor, irr::core::vector3df& slope,
                                            int8_t terrain) {
    irr::f32 RideHeight = FlightModel.RideHeight;
    irr::f32 ZposDisplacement;

    irr::f32 Zpos = sensor.Zpos;
    irr::f32 Xpos;
    irr::f32 Ypos;

    irr::f32 v9;
    irr::f32 v10;

    if (Zpos >= (RideHeight + sensor.ZposFloor)) {
        ZposDisplacement = sensor.ZposDisplacement;
    } else {
        if ((terrain & 4) == 0) {
            Xpos = fabs(slope.X);
            if (Xpos >= 1.95703125f) {
                sensor.CollideFlags |= 1u;
            }

            Ypos = fabs(slope.Y);
            if (Ypos >= 1.95703125f) {
                sensor.CollideFlags |= 2u;
            }
        }

        v9 = RideHeight + sensor.ZposFloor - sensor.Zpos;
        v10 = v9 * sensor.Rebound;
        sensor.ZposDisplacement = v9;
        v10 /= 256.0f;
        sensor.ZposDisplacement = v10;
        if (sensor.ReboundLimit < v10) {
            sensor.ZposDisplacement = sensor.ReboundLimit;
        }
        Zpos = sensor.ZposDisplacement;
        ZposDisplacement = RideHeight + sensor.ZposFloor;
    }

    sensor.Zpos = ZposDisplacement + Zpos;
    irr::f32 v12 = sensor.Zpos - (RideHeight + sensor.ZposFloor);
    sensor.ZposDiff = v12;

    if (mMaximumZpos < v12) {
        sensor.ZposDisplacement -= mMaximumZpos * ((float)(Stats.Behind) / 100.0f);
    }

    irr::f32 v16 = -0.2734375f * ((float)(Stats.Behind) / 100.0f);

    if (sensor.ZposDisplacement >= v16) {
        if (sensor.ZposDisplacement >= 0.39453125f) {
            sensor.ZposDisplacement = 0.390625f;
        }
    } else {
        sensor.ZposDisplacement = v16;
    }

    if (sensor.ZposDisplacement >= 3.91015625f) {
        logging::Warning("vehicle_sensor_point_process: Z TOO BIG!");
    }
}

void VVehicle::vehicle_sensor_point_projection(irr::core::vector3df& delta) {
    irr::core::vector3df position;
    irr::core::vector3df new_position;
    irr::core::vector3df displacement;

    irr::f32 Forward = FlightModel.SizeForward;
    irr::f32 Sideways = FlightModel.SizeSideways;

    position.X = ThingData.Position.X;
    position.Z = ThingData.Position.Z;
    new_position.X = ThingData.Position.X;
    new_position.Z = ThingData.Position.Z;

    /* Sensor Front Right */

    mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY + 90.0f, ThingData.Movement.AngleXZ, Sideways);
    mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY, ThingData.Movement.AngleZY, Forward);
    new_position = position + delta;

    FlightModel.FrontRight.Position.X = position.X;
    FlightModel.FrontRight.Position.Z = position.Z;
    FlightModel.FrontRight.CollideFlags = mRace->mVCalc->map_colide_direction_xy(position, new_position);
    mRace->mVCalc->move_displacement_slope(new_position, displacement);
    FlightModel.FrontRight.ZposFloor = mRace->mVCalc->map_altitude_column_and_floor(new_position);
    int8_t v13 = mRace->mVCalc->map_colide_type(new_position);
    vehicle_sensor_point_process(FlightModel.FrontRight, displacement, v13);

    /* Sensor Front Left */

    Sideways *= 2.0f;
    mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY - 90.0f, -ThingData.Movement.AngleXZ, Sideways);
    new_position = position + delta;

    FlightModel.FrontLeft.Position.X = position.X;
    FlightModel.FrontLeft.Position.Z = position.Z;
    FlightModel.FrontLeft.CollideFlags = mRace->mVCalc->map_colide_direction_xy(position, new_position);
    mRace->mVCalc->move_displacement_slope(new_position, displacement);
    FlightModel.FrontLeft.ZposFloor = mRace->mVCalc->map_altitude_column_and_floor(new_position);
    int8_t v16 = mRace->mVCalc->map_colide_type(new_position);
    vehicle_sensor_point_process(FlightModel.FrontLeft, displacement, v16);

    /* Sensor Rear Left */

    mRace->mVCalc->move_xyz(position, -ThingData.Movement.AngleXY, -ThingData.Movement.AngleZY,
             FlightModel.SizeRear + Forward);
    new_position = position + delta;

    FlightModel.RearLeft.Position.X = position.X;
    FlightModel.RearLeft.Position.Z = position.Z;
    FlightModel.RearLeft.CollideFlags = mRace->mVCalc->map_colide_direction_xy(position, new_position);
    mRace->mVCalc->move_displacement_slope(new_position, displacement);
    FlightModel.RearLeft.ZposFloor = mRace->mVCalc->map_altitude_column_and_floor(new_position);
    int8_t v19 = mRace->mVCalc->map_colide_type(new_position);
    vehicle_sensor_point_process(FlightModel.RearLeft, displacement, v19);

    /* Sensor Rear Right */

    mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY + 90.0f, ThingData.Movement.AngleXZ, Sideways);
    new_position = position + delta;

    FlightModel.RearRight.Position.X = position.X;
    FlightModel.RearRight.Position.Z = position.Z;
    FlightModel.RearRight.CollideFlags = mRace->mVCalc->map_colide_direction_xy(position, new_position);
    mRace->mVCalc->move_displacement_slope(new_position, displacement);
    FlightModel.RearRight.ZposFloor = mRace->mVCalc->map_altitude_column_and_floor(new_position);
    int8_t v22 = mRace->mVCalc->map_colide_type(new_position);
    vehicle_sensor_point_process(FlightModel.RearRight, displacement, v22);
}

void VVehicle::vehicle_colide_map(irr::core::vector3df& delta) {
    vehicle_sensor_point_projection(delta);

    //Note 02.05.2026: The original code is really weird here with a lot of int type conversions
    //and logical combinations; I used a pencil and paper, and I believe whats actually going on
    //is the code I have written here below; But of course this change could have introduced a bug!
    int8_t v9 = (FlightModel.FrontRight.CollideFlags | FlightModel.FrontLeft.CollideFlags |
                 FlightModel.RearLeft.CollideFlags | FlightModel.RearRight.CollideFlags);

    //only enter is there is a collision indicated right now
    if (v9) {
        irr::f32 Xpos;

        //any collision Flag bit location 0 set?
        if ((v9 & 1) != 0) {
           Xpos = fabs(delta.X);
           if (Xpos >= 0.04296875f) {
               //TODO: What to do with the next line?
               //thing->Colide.Affect.Status |= 0x200u;
           }
           delta.X = 0.0f;
           Momentum.DeltaX *= mBounce;
           Momentum.DeltaX = -Momentum.DeltaX;
        }

        //any collision Flag bit location 1 set?
        if ((v9 & 2) != 0) {
            irr::f32 Ypos;

            Ypos = fabs(delta.Y);
            if (Ypos >= 0.04296875f) {
                //TODO: What to do with the next line?
                //thing->Colide.Affect.Status |= 0x200u;
            }
            delta.Y = 0.0f;
            Momentum.DeltaY *= mBounce;
            Momentum.DeltaY = -Momentum.DeltaY;
        }
    }

   if (FlightModel.FrontLeft.CollideFlags) {
       mRace->mVCalc->move_xyz(delta, ThingData.Movement.AngleXY + 90.0f, 0.0f, 0.05859375f);
   }

   if (FlightModel.FrontRight.CollideFlags) {
       mRace->mVCalc->move_xyz(delta, ThingData.Movement.AngleXY - 90.0f, 0.0f, 0.05859375f);
   }

   if (FlightModel.RearLeft.CollideFlags) {
       mRace->mVCalc->move_xyz(delta, ThingData.Movement.AngleXY + 90.0f, 0.0f, 0.05859375f);
   }

   if (FlightModel.RearRight.CollideFlags) {
       mRace->mVCalc->move_xyz(delta, ThingData.Movement.AngleXY - 90.0f, 0.0f, 0.05859375f);
   }
}

void VVehicle::vehicle_move_mapwho(irr::core::vector3df& delta) {
    irr::f32 Xpos;
    irr::f32 Ypos;
    irr::f32 Zpos;
    irr::f32 v6;
    irr::f32 v8;
    irr::f32 v10;

    //irr::core::vector3df position;

    Xpos = delta.X;
    v6 = -0.8984375f;
    if (Xpos < -0.8984375f || (v6 = 0.8984375f, Xpos >= 0.90234375f)) {
        delta.X = v6;
    }

    Ypos = delta.Y;
    v8 = -0.8984375f;
    if (Ypos < -0.8984375f || (v8 = 0.8984375f, Ypos >= 0.90234375f)) {
        delta.Y = v8;
    }

    Zpos = delta.Z;
    v10 = -0.5859375f;
    if (Zpos < -0.5859375f || (v10 = 0.5859375f, Zpos >= 0.58984375f)) {
        delta.Z = v10;
    }

    //position.X = ThingData.Position.X + delta.X;
    //position.Y = ThingData.Position.Y + delta.Y;
    //position.Z = ThingData.Position.Z + delta.Z;
    //mapwho_move(thing, &position);
}

void VVehicle::vehicle_control_from_player() {
    MovementInput.AngleXY = 0.0f;
    MovementInput.AngleXZ = 0.0f;
    MovementInput.AngleZY = 0.0f;
    MovementInput.SpeedActual = 0.0f;

    if (KeyPressedAccel) {
        Increment.SpeedActual = IncrementAdd.SpeedActual;
    } else if (KeyPressedDeaccel) {
        Increment.SpeedActual = -IncrementAdd.SpeedActual;
    }
}

VVehicle::~VVehicle() {
    //Remove my Scenenode from
    //Scenemanager
    if (this->mCraftNode != nullptr) {
        mCraftNode->remove();
        mCraftNode = nullptr;
    }

    //Remove my player Mesh
    if (this->mCraftMesh != nullptr) {
       mRace->mGame->mSmgr->getMeshCache()->removeMesh(this->mCraftMesh);
       this->mCraftMesh = nullptr;
    }
}

