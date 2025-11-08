/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "cone.h"
#include "../race.h"
#include "../game.h"
#include "../resources/mapentry.h"
#include "../models/levelterrain.h"
#include "../utils/physics.h"

Cone::Cone(Race* race, irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr) {
    mSmgr = smgr;
    mRace = race;

    Position.X = x;
    Position.Y = y;
    Position.Z = z;

    orientation.set(irr::core::vector3df(0.0f, 0.0f, 0.0f));

    coneMesh = smgr->getMesh("extract/models/cone0-0.obj");
    cone_node = smgr->addMeshSceneNode(coneMesh);

    cone_node->setPosition(Position);
    cone_node->setScale(irr::core::vector3d<irr::f32>(1,1,1));
    cone_node->setMaterialFlag(irr::video::EMF_LIGHTING, mRace->mGame->enableLightning);
    cone_node->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);

    irr::core::aabbox3df coneBox = cone_node->getTransformedBoundingBox();
    irr::core::vector3df coneExtend = coneBox.getExtent();

    mCenterHeight = coneExtend.Y / 2.0f;
}

Cone::~Cone() {
    //cleanup cone stuff

    //remove SceneNode
    this->cone_node->remove();

    //remove mesh
    mSmgr->getMeshCache()->removeMesh(coneMesh);
}

void Cone::Rotate(irr::f32 speedfactor) {
    irr::core::quaternion rotateFurther;

    //only allow to rotate cone model around X Axis
    rotateFurther.fromAngleAxis((15.0f / 180.0f) * speedfactor * irr::core::PI, rotAxis);
    rotateFurther.normalize();

    orientation *= rotateFurther;
    orientation.normalize();

    irr::core::vector3df rot;

    //set new rotation of cone
    this->orientation.toEuler(rot);
    this->cone_node->setRotation(rot * irr::core::RADTODEG);
}

void Cone::Update(irr::f32 deltaTime) {
    //if the cone is idle, just
    //return
    if (!mActivity)
        return;

    irr::f32 terrainHeight;
    int current_cell_calc_x, current_cell_calc_y;

    if (!mReachedFinalLocation) {
          irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

            //item is still moving, calculate next position
            this->Position = this->Position + currVelocity * speedFactor * 0.015f;
            this->currVelocity = this->currVelocity + this->mRace->mPhysics->mGravityVec * speedFactor * 0.015f;

            Rotate(speedFactor);

            //check if cone is currently moving towards ground, and is very close to race track ground (hits the ground)
            //in this case stop the movement of the cone, and fix it in position
            //only check more if the cone is currently falling towards the race track
            if (currVelocity.Y < 0.0f) {
                //yes, cone is falling down, now we need to calculate high about terrain tile below
                //calculate current cell below cone
                current_cell_calc_y = (int)(Position.Z / mRace->mLevelTerrain->segmentSize);
                current_cell_calc_x = -(int)(Position.X / mRace->mLevelTerrain->segmentSize);

                MapEntry* mEntry = mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);

                //is there actually an entry?
                if (mEntry != nullptr) {
                     terrainHeight = mRace->mLevelTerrain->pTerrainTiles[mEntry->get_X()][mEntry->get_Z()].currTileHeight;

                     //cone too close to terrain, stop the cone to continue further
                     if ((Position.Y - terrainHeight) < mCenterHeight) {
                         //to close to terrain, stop movement
                         mHitTerrain = true;
                         mReachedFinalLocation = true;
                         mActivity = false;
                     }
                } else {
                    //we did not find a valid entry, let cone disappear (hide it)
                    //because we set it to not visible the computer players will not see it
                    //and we can also not pick it up => no problem
                    mReachedFinalLocation = true;
                    cone_node->setVisible(false);
                    mActivity = false;
                }
            }
        } else {
              //item reached the final location, not moving anymore
        }

    //update model
    cone_node->setPosition(Position);
}

void Cone::WasHit(irr::core::vector3df movementDirection, irr::f32 collisionSpeed) {
    //cone is busy now
    mActivity = true;

    //setup initial velocity
    movementDirection.normalize();

    mVelocity.set(collisionSpeed * movementDirection.X, collisionSpeed * 1.0f, collisionSpeed * movementDirection.Z);
    currVelocity = mVelocity;

    //derive rotation axis, is direction vector perpendicular to the movement
    //direction
    rotAxis = this->mRace->mGame->yAxisDirVector->crossProduct(movementDirection);

    mReachedFinalLocation = false;
}
