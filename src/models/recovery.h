/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef RECOVERY_H
#define RECOVERY_H

#include <irrlicht/irrlicht.h>
#include "player.h"

#define STATE_RECOVERY_IDLE 0
#define STATE_RECOVERY_MOVETODUTY 1
#define STATE_RECOVERY_PLAYERGRABBED 2
#define STATE_RECOVERY_PUTPLAYERBACK 3
#define STATE_RECOVERY_GOBACKTOIDLEPOSITION 4

class Player; //Forward declaration

class Recovery {
public:
    Recovery(irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr);
    ~Recovery();

    bool ready;

    bool CurrentlyReadyforMission();
    void SentToRepairMission(Player* repairTarget);

private:
    //my current position I am at
    irr::core::vector3df mPosition;

    //used to keep my starting (idle) position
    //in the current map
    irr::core::vector3df mStartingPosition;

    Player *repairTarget;

    //my own current state
    irr::u32 mCurrentState;

    irr::scene::IAnimatedMesh*  RecoveryMesh;
    irr::scene::IMeshSceneNode* Recovery_node;

    irr::scene::ISceneManager* mSmgr;
};

#endif // RECOVERY_H
