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

#ifndef VVEHICLE_H
#define VVEHICLE_H

#include <irrlicht.h>
#include <cstdint>
#include "../vanilla/vcalc.h"

struct VehicleSensorPointStruct {
    irr::core::vector3df Position;
    int8_t CollideFlags;
    irr::f32 Zpos;
    irr::f32 ZposFloor;
    irr::f32 ZposDiff;
    irr::f32 ZposDisplacement;
    irr::f32 Rebound;
    irr::f32 ReboundLimit;
};

struct VehicleControlFlagsStruct {
    bool Booster;
    bool Brake;
    bool Airbourn;
    bool BarrelRoll;
    bool AutoPilot;
    bool AutoRefuel;
    bool AutoRearm;
    bool AutoRepair;
    bool AutoStop;
    bool AutoPilotSet;
    bool AutoDrive;
    bool HealthDeath;
    bool FuelDeath;
    bool Reposition;
};

struct VehicleFunctionFlagsStruct {
    bool Booster;
    bool Brake;
    bool BarrelRoll;
};

struct VehicleCollideControlStruct {
    irr::f32 SizeForward;
    irr::f32 SizeRear;
    irr::f32 SizeSideways;
    irr::f32 SizeSensorOffset;
    VehicleSensorPointStruct FrontLeft;
    VehicleSensorPointStruct FrontRight;
    VehicleSensorPointStruct RearLeft;
    VehicleSensorPointStruct RearRight;
    VehicleControlFlagsStruct Flag;
    VehicleFunctionFlagsStruct FunctionFlag;
    irr::f32 RideHeight;
    irr::f32 BrakePower;
};

struct VehicleStatsStruct {
    irr::f32 Velocity;

    int16_t Behind;
};

/************************
 * Forward declarations *
 ************************/

class Race;

class VVehicle {
public:
    VVehicle(Race* mParentRace, irr::core::vector3d<irr::f32> NewPosition,
             irr::core::vector3d<irr::f32> NewFrontAt);
    ~VVehicle();

    void Update(irr::f32 frameDeltaTime);

    bool KeyPressedTurnLeft = false;
    bool KeyPressedTurnRight = false;
    bool KeyPressedAccel = false;
    bool KeyPressedDeaccel = false;

private:
    Race* mRace = nullptr;

    //Thing data
    ThingDataStruct ThingData;

    MomentumStruct Momentum;

    //for player input
    MovementStruct MovementInput;

    MovementStruct Increment;

    //flight model constants
    MovementStruct IncrementAdd;
    MovementStruct IncrementSub;
    MovementStruct IncrementLimit;

    VehicleCollideControlStruct FlightModel;
    irr::core::vector3df Displacement;
    irr::core::vector3df Slope;

    //Stats
    VehicleStatsStruct Stats;

    //variables which only I use in my project
    irr::f32 mAbsTimeIntegrator = 0.0f;

    //the mesh for the Irrlicht SceneNode model
    irr::scene::IAnimatedMesh* mCraftMesh = nullptr;
    irr::scene::IMeshSceneNode* mCraftNode = nullptr;

    irr::f32 mThrustEffectiveness = 0.0f;
    irr::f32 mSideslipFriction = 0.0f;
    irr::f32 mSideslipToThrust = 0.0f;

    //flight model parameters
    irr::f32 mFriction = 0.0f;
    irr::f32 mFrictionLimit = 0.0f;
    irr::f32 mBounce = 0.0f;
    irr::f32 mMaximumZpos = 0.0f;

    void SetupFlightModelConstants();

    void vehicle_get_track_friction();
    void vehicle_calculate_angle();
    void vehicle_calculate_thrust(irr::core::vector3df& delta);
    void vehicle_calculate_momentum(irr::core::vector3df& delta);
    void vehicle_calculate_movement_delta(irr::core::vector3df& delta);
    void vehicle_move_altitude(irr::core::vector3df& delta);
    void vehicle_control_from_player();
    void vehicle_move_tilt(irr::core::vector3df& delta);
    void vehicle_move_roll(irr::core::vector3df& delta);
    void vehicle_sensor_point_projection(irr::core::vector3df& delta);
    void vehicle_sensor_point_process(VehicleSensorPointStruct& sensor, irr::core::vector3df& slope, int8_t terrain);
    void vehicle_colide_map(irr::core::vector3df& delta);
    void vehicle_move_mapwho(irr::core::vector3df& delta);
};

#endif // VVEHICLE_H

