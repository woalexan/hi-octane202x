/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef RECOVERY_H
#define RECOVERY_H

#include <irrlicht.h>

#define STATE_RECOVERY_IDLE 0
#define STATE_RECOVERY_MOVETODUTY 1
#define STATE_RECOVERY_PLAYERGRABBED 2
#define STATE_RECOVERY_PUTPLAYERBACK 3
#define STATE_RECOVERY_GOBACKTOIDLEPOSITION 4

#define STATE_RECOVERY_MOVE_STATIONARY 0
#define STATE_RECOVERY_MOVE_MOVING 1
#define STATE_RECOVERY_MOVE_BRAKING 2
#define STATE_RECOVERY_MOVE_SLOWHOMINGIN 3

const irr::f32 RECOVERY_VEHICLE_SPEED = 12.0f;
const irr::f32 RECOVERY_VEHICLE_SPEED_CRAWL = 5.0f;

const irr::f32 RECOVERY_VEHICLE_DROPOFTARGET_FREESPACE_REQ = 0.1f;  //1.0f
const irr::f32 RECOVERY_VEHICLE_ACCELDEACCELRATE = 0.1f;

/************************
 * Forward declarations *
 ************************/

class Player;
struct WayPointLinkInfoStruct;
class Race;

class Recovery {
public:
    Recovery(Race* race, irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr);
    ~Recovery();

    bool ready;

    bool CurrentlyReadyforMission();
    void SentToRepairMission(Player* repairTarget);
    irr::core::vector3df GetCurrentPosition();

    void Update(irr::f32 deltaTime);

    irr::scene::IMeshSceneNode* Recovery_node = nullptr;

private:
    //my current position I am at
    irr::core::vector3df mPosition;
    irr::core::quaternion mOrientation;

    irr::core::vector3df mPlayerDropOfPosition;

    bool mPlayerDropOfDirVecFound;
    irr::f32 mPlayerDropOfAbsAngle;

    irr::core::vector3df localCoordClaw;
    irr::core::vector3df worldCoordClaw;

    //used to keep my starting (idle) position
    //in the current map
    irr::core::vector3df mStartingPosition;

    Player *repairTarget = nullptr;

    //my own current state (regarding state of current recovery mission)
    irr::u32 mCurrentState;

    //my own current state regarding my movement
    irr::u32 mCurrentMovementState;

    irr::f32 mDistanceStartBraking;

    irr::scene::IAnimatedMesh*  RecoveryMesh = nullptr;

    //recovery vehicle shadow SceneNode
    irr::scene::IShadowVolumeSceneNode* NodeShadow = nullptr;

    irr::scene::ISceneManager* mSmgr = nullptr;

    Race* mRace = nullptr;

    irr::f32 mSpeedFactor;

    irr::f32 mCurrentSpeed;
    irr::f32 mTargetSpeed;

    void UpdateSceneNode();
    void FindPlayerDropOfPosition();

    bool WayPointLinkAcceptableForDropOf(WayPointLinkInfoStruct* link);

    void State_MoveToDuty(irr::f32 deltaTime);
    void State_PlayerGrabbed(irr::f32 deltaTime);
    void State_GoBackToIdleState(irr::f32 deltaTime);

    //returns true if the recovery vehicle has reached the current movement target
    //returns false otherwise
    bool ControlMovement(irr::core::vector3df vecMov, irr::f32 deltaTime);
};

#endif // RECOVERY_H
