/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "collectable.h"

Collectable::Collectable(EntityItem* entityItem,
                         int number, vector3d<irr::f32> pos, irr::scene::ISceneManager* mSmgr, irr::video::IVideoDriver* driver) {
    this->m_driver = driver;
    this->m_smgr = mSmgr;
    this->mEntityItem = entityItem;

    //preset values, also used in
    //HiOctaneTools
    irr::f32 w = 0.45f;
    irr::f32 h = 0.45f;

    //Position is the center of the BillboardSceneNode
    //Position in the level file could be the bottom location at the surface
    //therefore we need to add the height of the billboard to the Y coordinate
    Position = pos;
    //Position.Y += (h * 0.5f);

    //Note 27.12.2024: put a little bit higher,
    //so that it can be collected much more reliable
    //with bounding box
    Position.Y += (h * 0.75f);

    this->m_Size.set(w, h, irr::f32(0.01f));

    char fname[20];
    sprintf (fname, "%0*d", 4, number);

    m_texfile.clear();
    m_texfile.append("extract/sprites/tmaps");
    m_texfile.append(fname);
    m_texfile.append(".png");

    //loading the specified entity (billboard) texture file
    collectable_tex = driver->getTexture(m_texfile.c_str());
    texturesize = collectable_tex->getSize();

    this->billSceneNode = this->m_smgr->addBillboardSceneNode();
    this->billSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
    this->billSceneNode->setMaterialTexture(0, collectable_tex);
    this->billSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, true);
    this->billSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    this->billSceneNode->setPosition(Position);
    this->billSceneNode->setSize(irr::core::dimension2d<irr::f32>(w, h));

    //get bounding box for this collectible
    this->billSceneNode->updateAbsolutePosition();
    this->boundingBox = this->billSceneNode->getTransformedBoundingBox();

    //default is non visible after
    //start of game, and before entity group
    //1 is triggered (group 1 is triggered once
    //at game start to make initial collectables visible)
    this->isVisible = false;

    //I am not active (visible), hide my sceneNode
    this->billSceneNode->setVisible(false);
}

irr::core::vector2df Collectable::GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord) {
    threeDCoord = this->Position;
    irr::core::vector2df result(this->Position.X, this->Position.Z);

    return result;
}

Collectable::~Collectable() {
    //cleanup collectable stuff

    //remove SceneNode
    this->billSceneNode->remove();

    //remove texture
    m_driver->removeTexture(collectable_tex);
}

bool Collectable::GetIfVisible() {
    return this->isVisible;
}

void Collectable::PickedUp() {
    this->isVisible = false;

    //I am not active (visible) anymore, hide my sceneNode
    this->billSceneNode->setVisible(false);
}

//when a collectable is triggered it
//becomes visible to the player, and can be
//picked up
void Collectable::Trigger() {
    if (!this->isVisible) {
        this->isVisible = true;

        //unhide sceneNode
        this->billSceneNode->setVisible(true);
    }
}
