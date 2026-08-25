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

#ifndef VTHING_H
#define VTHING_H

#include "irrlicht.h"
#include "vbase.h"
#include <cstdint>

/************************
 * Forward declarations *
 ************************/

class Race;
class VVehicle;

struct VThing {
    irr::core::vector3df Position;
    MovementStruct Movement;
    irr::core::vector3df Displacement;

    int16_t Id = 0;
    int16_t Index = 0;
    int16_t Child = 0;
    int16_t Parent = 0;

    uint32_t AffectStatus = 0;
    int16_t AffectNumber = 0; //allows to specify the amount of damage dealt with an action
    uint16_t AffectWho = 0;   //allows to specify who is the cause of a damage (wo is the attacker)
    uint16_t ColideGroup = 0;

    int16_t Life = 0;

    uint32_t Seed = 0;

    int16_t Count = 0;
    irr::core::vector3df CollideSize;

    uint32_t Status = 0;
    int16_t Target = 0;
    int16_t Upgrade = 0;
    int8_t Member = 0;
    int8_t Action = 0;
    int8_t Group = 0;
    uint8_t TimeSlice = 0;

    //Pointer to the VVehicle object
    //if we are a player vehicle (grp10)
    VVehicle* vVehiclePnter = nullptr;
};

class VThingManager {
public:
    VThingManager(Race* parentRace);
    ~VThingManager();

    int32_t GetNumberThingsUsed();
    int32_t GetNumberThingsFree();

    VThing* thing_initialise_member(irr::core::vector3df position,
                                    irr::f32 angleXY,
                                    irr::f32 angleZY,
                                    irr::f32 angleXZ,
                                    int8_t group,
                                    int8_t member,
                                    int16_t id);

    VThing* thing_initialise(irr::core::vector3df position,
                                    irr::f32 angleXY,
                                    irr::f32 angleZY,
                                    irr::f32 angleXZ,
                                    int8_t group,
                                    int8_t member,
                                    int16_t id);

    void thing_delete(VThing* whichThing);

    //only use thing_remove is special cases!
    //default should be to use thing_delete
    void thing_remove(VThing* whichThing);

    VThing Thing[1000];

    uint8_t mapwho_delete(VThing* whichThing);
    uint8_t mapwho_add(VThing* whichThing, irr::core::vector3df position);
    uint8_t mapwho_move(VThing* whichThing, irr::core::vector3df position);

    uint8_t thing_overlapping(VThing* thing1, VThing* thing2);

    //whichThing is the Effect-Thing that affects player
    //vehicles
    uint8_t affect_thing(VThing* whichThing);

    int16_t thing_touching_anything(VThing* whichThing);

    //effect is the Effect-Thing that affects player
    //vehicles
    int16_t effect_affect_vehicle_exclusive(VThing* effect);

    void RunHousekeeping();

private:
    Race* mParentRace = nullptr;

    int16_t AffectList[1000];
    //I assume 0 is the initial Value, not sure
    int16_t AffectListIndex = 0;

    void ResetThingValues(VThing* whichThing);
};

#endif // VTHING_H
