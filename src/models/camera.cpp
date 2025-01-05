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
    mPosition = mEntityItem->getCenter();

    //store my value
    //I do not know what it means right now
    mValue = mEntityItem->getValue();

    //create my new camera SceneNode
    mCamSceneNode = mSmgr->addCameraSceneNode(NULL, mPosition);
}

void Camera::SetActive(bool newState) {
    mActive = newState;
}

void Camera::Update() {
    if (!mActive)
        return;

    //is a player close to me?
    irr::f32 distanceSQ = (this->mRace->mPlayerVec.at(0)->phobj->physicState.position - mPosition).getLengthSQ();

    if (distanceSQ < 50.0f) {
        mFocusAtPlayer = this->mRace->mPlayerVec.at(0);
        mCanSeePlayer = true;
    } else {
        mFocusAtPlayer = NULL;
        mCanSeePlayer = false;
    }

    if (mFocusAtPlayer != NULL) {
        mLookAt = mFocusAtPlayer->phobj->physicState.position;

        this->mCamSceneNode->setTarget(mLookAt);
    }
}

Camera::~Camera() {
    //cleanup camera stuff
}
