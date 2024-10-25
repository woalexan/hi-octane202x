/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "recovery.h"

Recovery::Recovery(irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr) {
    ready = false;
    mSmgr = smgr;

    mCurrentState = STATE_RECOVERY_IDLE;

    //my Irrlicht coordinate system is swapped at the x axis; correct this issue
    mPosition.X = -x;
    mPosition.Y = y;
    mPosition.Z = z;

    //remember the starting position, we need it later
    mStartingPosition = mPosition;

    RecoveryMesh = smgr->getMesh("extract/models/recov0-0.obj");
    Recovery_node = smgr->addMeshSceneNode(RecoveryMesh);

    Recovery_node->setPosition(mPosition);
    Recovery_node->setScale(irr::core::vector3d<irr::f32>(1,1,1));
    Recovery_node->setMaterialFlag(irr::video::EMF_LIGHTING, false);
}

bool Recovery::CurrentlyReadyforMission() {
    //we are ready for a new mission if we either are currently idling at our home
    //(starting) position, or if we are currently on the way back to the starting position
    if ((this->mCurrentState == STATE_RECOVERY_IDLE) || (this->mCurrentState == STATE_RECOVERY_GOBACKTOIDLEPOSITION))
        return true;

    //in all other states we are currently helping somebody
    //and can not start another mission
    return false;
}

void Recovery::SentToRepairMission(Player* repairTarget) {
    //we are now on our way
    this->mCurrentState = STATE_RECOVERY_MOVETODUTY;

    //remember the repair target (player which needs help)
    this->repairTarget = repairTarget;
}

Recovery::~Recovery() {
    //cleanup recovery stuff

    //remove SceneNode
    this->Recovery_node->remove();

    //remove mesh
    mSmgr->getMeshCache()->removeMesh(RecoveryMesh);
}
