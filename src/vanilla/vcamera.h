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

#ifndef VCAMERA_H
#define VCAMERA_H

#include "irrlicht.h"
#include "vbase.h"
#include <cstdint>
#include <vector>

/************************
 * Forward declarations *
 ************************/

class Race;
class VVehicle;
class ParseCamera;

struct VCameraStruct {
    //Position is the position of the camera in the engine
    irr::core::vector3df Position;

    //Destination is present in the original game, but seems to be not
    //used in the engine render call; Is just 0 bytes all the time
    //If bytes are changed nothing happens during frame rendering, and if you
    //write random bytes into the Destination the bytes just stay there, is not
    //updated at all during game loop
    //Therefore I just commented it out here
    //irr::core::vector3df Destination;

    //Angle controls how the camera is oriented
    irr::f32 AngleXY = 0.0f;
    irr::f32 AngleZY = 0.0f;
    irr::f32 AngleXZ = 0.0f;

    //I believe Lens sets the Field of view during rendering in the
    //original games engine; Value is initialized but does not change during
    //a game; If I modify the value then the rendering is affected, in a way
    //that more or less polygons outside of the middle of the frame area are rendered
    //irr::f32 Lens;

    //is calculated and used in external view to
    //create the effect that the camera position does move
    //towards or away from the moving vehicle
    irr::f32 Distance;

    //int16_t Follow;
    //Zoom has just a predefined value that does not change during the Race
    //Affects the calculation result of Distance above
    irr::f32 Zoom;

    int16_t Static;

    //Action controls which camera view is used
    //(6 = external view, 7 = default internal cockpit view)
    int16_t Action;
};

struct VCameraWindowStruct {
   VCameraStruct Camera;
   VCameraStruct ChaseCamera;
   /*irr::core::vector3df Position;
   irr::core::vector3df Size;
   int32_t Update;*/
};

class VCamera {

private:
    Race* mParentRace = nullptr;

    void camera_process_position(VVehicle* whichVehicle, VCameraWindowStruct& vanillaOutputCameraWindow,
                                 irr::f32 distance, irr::f32 degrees);

    VCameraWindowStruct mCameraWindow;
    VCameraStruct mRenderCamera;

    void camera_setup();

    int32_t move_point_see_point(irr::core::vector3df position_from, irr::core::vector3df position_to);

public:
    VCamera(Race* parentRace);
    ~VCamera();

    void camera_process(VVehicle* whichVehicle, int32_t force_camera_action);

    //sets an Irrlicht camera in a way to replicate the original games camera view
    void SetIrrlichtCamera(irr::scene::ICameraSceneNode* whichCamera, ParseCamera* setToState);
    void SetIrrlichtCamera(irr::scene::ICameraSceneNode* whichCamera, VCameraStruct* setToState);
};

#endif // VCAMERA_H
