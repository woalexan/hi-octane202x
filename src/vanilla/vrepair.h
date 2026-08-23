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

#ifndef VREPAIR_H
#define VREPAIR_H

#include "irrlicht.h"
#include "vbase.h"
#include "vvehicle.h"
#include "vrepair.h"

/************************
 * Forward declarations *
 ************************/

class Race;

class VRepair {
public:
    VRepair(Race* mParentRace, irr::core::vector3d<irr::f32> NewPosition,
             irr::scene::ISceneManager* smgr);
    ~VRepair();

    void Initialize();
    void Update(irr::f32 frameDeltaTime);

    //Thing data
    VThing* ThingData = nullptr;

    Race* mRace = nullptr;


    //flight model constants
    MovementStruct IncrementAdd;

    VehicleViewStruct View;

    //Needs to be public to be able to add it to
    //X-Effects filter, TODO: change later
    irr::scene::IMeshSceneNode* RecoveryNode = nullptr;

private:
    irr::f32 mAbsTimeIntegrator = 0.0f;

    //In the original game the repair vehicle also reuses
    //the default race vehicle struct (with a lot of variables not used
    //at the end). I wanted to give it its own struct; It seems the original
    //game reuses BumpDamage of the default car to store an additional waypoint
    //information;
    uint16_t BumpDamage;

    //This variables seems to have something to do with
    //checkpoint handling
    size_t Counter[8];

    //In my implementation TargetVehicle replaces the role
    //of CurrentWaypoint in the original game for the recovery
    //vehicle
    VVehicle* TargetVehicle = nullptr;

    void repair_vehicle_move_toward(irr::core::vector3df t_position,
                                    irr::core::vector3df& delta, irr::f32 speed, irr::f32 dist);

    void repair_vehicle_rotate_car(VVehicle* targetVehicle);
    uint16_t repair_vehicle_find_drop_waypoint(VVehicle* targetVehicle);

    void repair_vehicle_execute_action0x1A(irr::core::vector3df pos1,
                                                    irr::core::vector3df pos2, irr::core::vector3df& delta);

    void repair_vehicle_execute_action0x1B(irr::core::vector3df pos1,
                                                    irr::core::vector3df pos2, irr::core::vector3df& delta);

    void repair_vehicle_execute_action0x1C(irr::core::vector3df pos1,
                                                    irr::core::vector3df pos2, irr::core::vector3df& delta);

    void repair_vehicle_execute_action0x1D(irr::core::vector3df pos1,
                                                    irr::core::vector3df pos2, irr::core::vector3df& delta);

    void repair_vehicle_execute_action0x1E(irr::core::vector3df pos1,
                                                    irr::core::vector3df pos2, irr::core::vector3df& delta);

    void repair_vehicle_execute_action0x1F();

    uint8_t repair_vehicle_drop_point_ok(uint16_t waypoint);

    void initialiseVEHICLE_POLICE_HELICOPTER();

    void repair_vehicle_set_camera();

    //Things needed for Irrlicht
    irr::scene::ISceneManager* mSmgr = nullptr;

    irr::scene::IAnimatedMesh* RecoveryMesh = nullptr;

    void UpdateSceneNode();
};

#endif // VREPAIR_H
