/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "camera.h"

Camera::Camera(Race* race, EntityItem *entityItem, irr::scene::ISceneManager* smgr) {
    mSmgr = smgr;
    mRace = race;

    mEntityItem = entityItem;

    //get and store the position the camera is located at
    //set a slightly higher position
    mPosition = mEntityItem->getCenter() + irr::core::vector3df(0.0f, 1.0f, 0.0f);

    //store my value
    //I do not know what it means right now
    mValue = mEntityItem->getValue();

    //create my new camera SceneNode
    mCamSceneNode = mSmgr->addCameraSceneNode(nullptr, mPosition);
}

void Camera::SetActive(bool newState) {
    mActive = newState;
}

bool Camera::CanIObserveLocation(irr::core::vector3df location) {
    //I am close enough to this location?
    irr::f32 distanceSQ = (location - mPosition).getLengthSQ();

    //TODO: Also check if we have line of sight, for example cast a ray etc...

    if (distanceSQ < 120.0f)
        return true;

    return false;
}

void Camera::SetTargetPlayer(Player* newCameraTargetPlayer) {
   mFocusAtPlayer = newCameraTargetPlayer;
}

void Camera::Update() {
    if (!mActive)
        return;

    if (mFocusAtPlayer != nullptr) {
        mLookAt = mFocusAtPlayer->phobj->physicState.position;

        this->mCamSceneNode->setTarget(mLookAt);
    }
}

Camera::~Camera() {
    //cleanup camera stuff
    mCamSceneNode->remove();
}
