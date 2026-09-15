/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef CAMERA_H
#define CAMERA_H

#include <irrlicht.h>
#include "../vanilla/vthing.h"

/************************
 * Forward declarations *
 ************************/

class Race;
class EntityItem;
struct VThing;

class Camera {
public:
    Camera(Race* race, EntityItem *entityItem, irr::scene::ISceneManager* smgr);
    ~Camera();

    //Update/initialize final height after
    //Terrain geometry information is available
    void InitializeHeight();

    //Pointer to my thing
    VThing* ThingData = nullptr;

    //our Camera SceneNode from Irrlicht
    //irr::scene::ICameraSceneNode* mCamSceneNode = nullptr;

private:
    EntityItem* mEntityItem = nullptr;
    
    irr::scene::ISceneManager* mSmgr = nullptr;

    //the never seem to have an offsetX and offsetY value, and not
    //trigger target group; Also all the time the seem to belong to trigger group
    //1, so maybe this means the all become active at the start of the race

    //mAddedHeightFixedPoint allows to specify a height
    //which raises the camera above ground
    int16_t mAddedHeightFixedPoint;

    Race *mRace = nullptr;
};

#endif // CAMERA_H
