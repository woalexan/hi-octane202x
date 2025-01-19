/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef CAMERA_H
#define CAMERA_H

#include <irrlicht/irrlicht.h>
#include "../race.h"
#include "../models/player.h"

class Race; //Forward declaration
class Player; //Forward declaration

class Camera {
public:
    Camera(Race* race, EntityItem *entityItem, irr::scene::ISceneManager* smgr);
    ~Camera();

    void Update();

    //our Camera SceneNode from Irrlicht
    irr::scene::ICameraSceneNode* mCamSceneNode;

    void SetActive(bool newState);

    bool CanIObserveLocation(irr::core::vector3df location);
    void SetTargetPlayer(Player* newCameraTargetPlayer);

    //target player at which we currently focus at
    //NULL means we have no focus target right now
    Player* mFocusAtPlayer = NULL;

private:
    //my camera position according to the
    //level data
    irr::core::vector3df mPosition;

    //the position I look at right now
    irr::core::vector3df mLookAt;

    EntityItem* mEntityItem;
    
    irr::scene::ISceneManager* mSmgr;

    //camera does seem to have sometimes a value, where I do not know
    //right now what it means

    //the never seem to have an offsetX and offsetY value, and not
    //trigger target group; Also all the time the seem to belong to trigger group
    //1, so maybe this means the all become active at the start of the race
    int16_t mValue;

    Race *mRace;

    //if true camera is active, and looking for players
    bool mActive = false;
};

#endif // CAMERA_H
