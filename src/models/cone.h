/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef CONE_H
#define CONE_H

#include <irrlicht.h>
#include "../race.h"

class Race; //Forward declaration

class Cone {
public:
    Cone(Race* race, irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr);
    ~Cone();

    void WasHit(irr::core::vector3df movementDirection, irr::f32 collisionSpeed);
    void Update(irr::f32 deltaTime);

    irr::core::vector3df Position;
    bool mActivity = false;
private:
    irr::core::quaternion orientation;
    irr::core::vector3d<irr::f32> rotAxis;

    irr::scene::IAnimatedMesh*  coneMesh;
    irr::scene::IMeshSceneNode* cone_node;

    irr::scene::ISceneManager* mSmgr;

    Race *mRace;

    irr::core::vector3df currVelocity;
    irr::core::vector3df mVelocity;

    bool mHitTerrain = false;

    bool mReachedFinalLocation = true;
    irr::f32 mCenterHeight;

    void Rotate();
};

#endif // CONE_H
