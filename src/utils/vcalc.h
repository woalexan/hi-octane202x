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

#ifndef VCALC_H
#define VCALC_H

#include "irrlicht.h"
#include <cstdint>

/************************
 * Forward declarations *
 ************************/

class LevelTerrain;
class LevelBlocks;
class LevelFile;
class InfrastructureBase;

struct MovementStruct {
    irr::f32 AngleXY;
    irr::f32 AngleZY;
    irr::f32 AngleXZ;
    irr::f32 SpeedActual;
};

struct TestThing {
    irr::core::vector3df Position;
    irr::core::vector3df Displacement;
    MovementStruct Movement;

    bool Stationary = false;
    int16_t Life = 1000;
};

class VCalculations {

private:
    LevelTerrain* mLevelTerrain;
    LevelBlocks* mLevelBlocks;
    LevelFile* mLevelFile = nullptr;
    InfrastructureBase* mInfra = nullptr;

    TestThing* testThing1;
    irr::scene::IBillboardSceneNode* testNode;

    irr::video::ITexture* testTex = nullptr;

public:
    VCalculations(InfrastructureBase* infra, LevelFile* levelFile, LevelTerrain* levelTerrain, LevelBlocks* levelBlocks);

    void DebugDrawDisplacement(TestThing* whichThing);
    void DebugDraw();

    void AddTestObject(irr::core::vector3df position);
    int8_t UpdateTestObject(irr::f32 frameDeltaTime, TestThing& whichThing);

    void Update();

    //Convert a float into a fixed point number (8.8 format)
    int16_t FloatToFixedPoint8D8(irr::f32 floatVal);
    //Convert a float into a fixed point number (16.16 format)
    int32_t FloatToFixedPoint16D16(irr::f32 floatVal);

    //Convert a fixed point number (8.8 format) to a float
    irr::f32 FixedPointToFloat8D8(int16_t fixedPntVal);
    //Convert a fixed point number (16.16 format) to a float
    irr::f32 FixedPointToFloat16D16(int32_t fixedPntVal);

    //original "raw" angles (for example in thing->Movement.Angle.ZY, is struct Angle)
    //contain the absolute angle in current steps for a unit circle of 65536 steps
    //this function simply converts this angle into a floating point angle value
    //with a unit circle of 360.0f I use in Irrlicht
    irr::f32 VanillaRawAngleToMyFloatingAngle(uint16_t rawVanillaAngle);

    //Helper function to test one specific case for Verify_collide_map function
    bool Verify_collide_map_step(int16_t posXFixedPnt, int16_t posZFixedPnt, int16_t expYResultFixedPnt, int16_t whichSeqCaseTested);

    //Returns true of verify collide map works as expected, False
    //otherwise
    //Important: Only works correctly if unmodified (original) level1
    //file is loaded
    bool Verify_collide_map();

    //Returns true if all vanilla calculations work as expected, False otherwise
    //Important: Only works correctly if unmodified (original) level1
    //file is loaded
    bool Verify_vanilla_calculations();

    //Input position: Enter coordinates according to my game drawing coordinate
    //system (where X coordinates are mirrored at X-Axis, and posZ is my 2nd 2D coordinate-axis)
    //Returns the tile raw friction table value
    uint16_t map_colide_type(irr::core::vector3df position);

    //Input position: Enter coordinates according to my game drawing coordinate
    //system (where X coordinates are mirrored at X-Axis, and posZ is my 2nd 2D coordinate-axis)
    //Returns the tile friction value at the specified location
    uint16_t map_colide_friction(irr::core::vector3df position);

    //Function programmed to have similar behavior as function "collide_map"
    //in original game. But instead of using fixed point arithmetic I am using
    //floating point calculations.
    //Input posX & posZ: Enter coordinates according to my game drawing coordinate
    //system (where X coordinates are mirrored at X-Axis, and posZ is my 2nd 2D coordinate-axis)
    //Returns my Y-coordinate in Irrlicht coordinate system
    irr::f32 collide_map(irr::f32 posX, irr::f32 posZ);  //06.04.2026: Function verified to be correct

    //Function programmed to have similar behavior as function "map_floor"
    //in original game.
    //Input position: Enter coordinates according to my game drawing coordinate
    //system (where X coordinates are mirrored at X-Axis, and posZ is my 2nd 2D coordinate-axis)
    //Returns my Y-coordinate in Irrlicht coordinate system
    irr::f32 map_floor(irr::core::vector3df position);  //06.04.2026: Function verified to be correct

    //This function was written to behave similar to the function
    //map_altitude_column_and_floor in the original game
    //Returns the height of Terrain or existing column block
    //directly below the specified 3D position
    irr::f32 map_altitude_column_and_floor(irr::core::vector3df position);

    //This function was written to behave similar to the function
    //map_colide in the original game
    //Does determine if a 3D input position is in free space, or if it does
    //collide with terrain or a column block
    //Returns 0 for no collision (free space)
    //Returns 8 for position below terrain height
    //Returns 16 for collision with existing column block
    int8_t map_colide(irr::core::vector3df position);

    //This function was written to behave similar to the function
    //map_colide_direction in the original game
    //Returns 0 if no collision occurs
    //Depending on which coordinate axis the collision occurs
    //sets flags 1, 2, 4
    int8_t map_colide_direction(irr::core::vector3df oldPosition, irr::core::vector3df newPosition);

    //This function was written to behave similar to the function
    //move_displacement_xyz in the original game
    //moves a 3D position by a certain displacement, direction
    //is also defined by the sign
    void move_displacement_xyz(irr::core::vector3df& position, irr::core::vector3df displacement, int8_t sign);

    //This function was written to behave similar to the function
    //move_displacement_slope in the original game
    //Calculates and returns the displacement for a terrain cell at the specified
    //3D position
    void move_displacement_slope(irr::core::vector3df position,
                                                irr::core::vector3df& displacement);

    //Helper function to test one specific case for Verify_collide_map function
    bool Verify_move_displacement_set_step(int16_t expXPos, int16_t expYPos, int16_t expZPos, bool verifyZPos,
               int16_t inputAngleXY, int16_t inputAngleZY, int16_t inputSpeed,
                int16_t whichSeqCaseTested);

    //Returns true of verify move_displacement_set works as expected, False
    //otherwise
    bool Verify_move_displacement_set();

    //In original game: angleXY and angleZY are a 16 bit integer, with range from 0 up to 0xFFFF
    //Angles have 8.8 Fixed Point Format; contain the absolute angle on a unit circle with 256 steps for a whole circle;
    //In the original game this integer value is then divided further by a factor of 32 (shift 5 bits to the right)
    //to get the index into the original sine table
    //Speed is a fixed point number with 8.8 format;
    //In my project: angleXY and angleZY is the absolute angle in deg, range 0.0f up to 360.0f
    //speed has a possible range from 0.0 up to 255.99609375, resolution in original game
    //is 0.00390625 for speed
    int8_t move_displacement_set(irr::core::vector3df& position, irr::f32 angleXY,
                                 irr::f32 angleZY, irr::f32 speed);

    //Helper function for function verification
    bool Verify_move_xyz_step(int16_t startXPos, int16_t startYPos, int16_t startZPos,
                                        int16_t endXPos, int16_t endYPos, int16_t endZPos,
               int16_t inputAngleXY, int16_t inputAngleZY, int16_t inputSpeed,
                int16_t whichSeqCaseTested);

    //Returns true of verify move_xyz works as expected, False
    //otherwise
    bool Verify_move_xyz();

    //In original game: angleXY and angleZY are a 16 bit integer, with range from 0 up to 0xFFFF
    //Angles have 8.8 Fixed Point Format; contain the absolute angle on a unit circle with 256 steps for a whole circle;
    //In the original game this integer value is then divided further by a factor of 32 (shift 5 bits to the right)
    //to get the index into the original sine table
    //Speed is a fixed point number with 8.8 format;
    //In my project: angleXY and angleZY is the absolute angle in deg, range 0.0f up to 360.0f
    //speed has a possible range from 0.0 up to 255.99609375, resolution in original game
    //is 0.00390625 for speed
    int8_t move_xyz(irr::core::vector3df& position, irr::f32 angleXY,
                                 irr::f32 angleZY, irr::f32 speed);
};

#endif // VCALC_H
