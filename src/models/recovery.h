/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef RECOVERY_H
#define RECOVERY_H

#include <irrlicht/irrlicht.h>

class Recovery {
public:
    Recovery(irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr);
    ~Recovery();

    bool ready;

private:
    irr::core::vector3d<irr::f32> Position;

    irr::scene::IAnimatedMesh*  RecoveryMesh;
    irr::scene::IMeshSceneNode* Recovery_node;

    irr::scene::ISceneManager* mSmgr;
};

#endif // RECOVERY_H
