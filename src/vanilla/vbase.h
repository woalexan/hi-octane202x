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

#ifndef VBASE_H
#define VBASE_H

#include "irrlicht.h"
#include <cstdint>

/************************
 * Forward declarations *
 ************************/

struct MovementStruct {
    irr::f32 AngleXY = 0.0f;
    irr::f32 AngleZY = 0.0f;
    irr::f32 AngleXZ = 0.0f;
    irr::f32 SpeedActual = 0.0f;
};

struct MomentumStruct {
    irr::f32 DeltaX = 0.0f;
    irr::f32 DeltaY = 0.0f;
    irr::f32 AngleXY = 0.0f;
};

struct ThingDataStruct {
    irr::core::vector3df Position;
    MovementStruct Movement;
    irr::core::vector3df Displacement;

    bool Stationary = false;
    int16_t Life = 1000;
    uint8_t mTimeSlice = 0;
};

#endif // VBASE_H
