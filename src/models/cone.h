/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef CONE_H
#define CONE_H

#include <irrlicht/irrlicht.h>
#include "../race.h"

class Race; //Forward declaration

class Cone {
public:
    Cone(Race* race, irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr);
    ~Cone();

    bool ready;

private:
    irr::core::vector3df Position;

    irr::scene::IAnimatedMesh*  coneMesh;
    irr::scene::IMeshSceneNode* cone_node;

    irr::scene::ISceneManager* mSmgr;

    Race *mRace;
};

#endif // CONE_H
