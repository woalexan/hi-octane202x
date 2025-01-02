/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "cone.h"

Cone::Cone(Race* race, irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr) {
    ready = false;
    mSmgr = smgr;
    mRace = race;

    Position.X = x;
    Position.Y = y;
    Position.Z = z;

    coneMesh = smgr->getMesh("extract/models/cone0-0.obj");
    cone_node = smgr->addMeshSceneNode(coneMesh);

    cone_node->setPosition(Position);
    cone_node->setScale(irr::core::vector3d<irr::f32>(1,1,1));
    cone_node->setMaterialFlag(irr::video::EMF_LIGHTING, mRace->mGame->enableLightning);
}

Cone::~Cone() {
    //cleanup cone stuff

    //remove SceneNode
    this->cone_node->remove();

    //remove mesh
    mSmgr->getMeshCache()->removeMesh(coneMesh);
}
