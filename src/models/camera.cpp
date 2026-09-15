/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "camera.h"
#include "../resources/entityitem.h"
#include "../race.h"
#include "../game.h"
#include "../vanilla/vcalc.h"
#include "../vanilla/vthing.h"

Camera::Camera(Race* race, EntityItem *entityItem, irr::scene::ISceneManager* smgr) {
    mSmgr = smgr;
    mRace = race;

    mEntityItem = entityItem;

    //get and store the position the camera is located at
    irr::core::vector3df irrPosition = mEntityItem->getCenter();

    //store my value
    //mAddedHeightFixedPoint allows to specify a height
    //which raises the camera above ground
    //It is defined by the value stored in the entity
    mAddedHeightFixedPoint = mEntityItem->getValue();

    //We do not have the height map information right
    //now. Is available only after setting up the Terrain geometry
    irrPosition.Y = 0.0f;

    irr::core::vector3df vanPos = mRace->mVCalc->IrrlichtToVanillaCoord(irrPosition);

    //get my thing
    ThingData =
         mRace->mThingManager->thing_initialise_member(vanPos, 0.0f, 0.0f, 0.0f, 3, 3, -1);

    //create my new camera SceneNode
    //mCamSceneNode = mSmgr->addCameraSceneNode(nullptr, mPosition);
}

//Update/initialize final height after
//Terrain geometry information is available
void Camera::InitializeHeight() {
    //The value of the camera entity allows it to move it higher
    //above the terrain by a defined distance
    ThingData->Position.Z = (mRace->mVCalc->map_altitude_lowest(ThingData->Position) + (irr::f32)(mAddedHeightFixedPoint) + 1.0f);

    //only for debugging the camera position
    //irr::core::vector3df dbgPos = mRace->mVCalc->VanillaToIrrlichtCoord(ThingData->Position);
    //mRace->mGame->mSmgr->addCubeSceneNode(0.04f, nullptr, -1, dbgPos);
}

Camera::~Camera() {
    if (ThingData != nullptr) {
        mRace->mThingManager->thing_delete(ThingData);
        ThingData = nullptr;
    }

    //cleanup camera stuff
    //mCamSceneNode->remove();
}
