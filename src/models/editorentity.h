/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef EDITORENTITY_H
#define EDITORENTITY_H

#include "irrlicht.h"
#include "../definitions.h"
#include <vector>
#include <cstdint>
#include <string>
#include "../resources/entityitem.h"

/************************
 * Forward declarations *
 ************************/

class EntityManager;

class EditorEntity {
private:
    //variables for a 3D model like cone, recovery vehicle...
    irr::scene::IMeshSceneNode* mSceneNode = nullptr;
    irr::scene::IAnimatedMesh*  mMesh = nullptr;
    irr::io::path mModelFileName;

    //variables for a collectable, or sprite
    irr::video::ITexture* mSpriteTex = nullptr;
    irr::scene::IBillboardSceneNode *mBillSceneNode = nullptr;
    irr::core::dimension2d<irr::u32> mTextureSize;

    bool mBoundingBoxVisible = false;

    void UpdateBoundingBox();

public:
    //Constructor for a Mesh based 3D model
    EditorEntity(EntityManager* parentManager, EntityItem* itemPntr, irr::io::path modelFileName);

    //Constructor for a texture/Sprite based 2D model
    EditorEntity(EntityManager* parentManager, EntityItem* itemPntr, irr::video::ITexture* texture);
    ~EditorEntity();
    
    EntityManager* mParentManager = nullptr;

    void SetNewHeight(irr::f32 newHeight);
    irr::f32 GetCurrentHeight();

    irr::core::aabbox3df mBoundingBox;
    vector3d<irr::f32> mPosition;
    irr::u16 GetSpriteNumber();

    void ShowBoundingBox();
    void HideBoundingBox();

    Entity::EntityType GetEntityType();
    irr::core::vector2di GetCellCoord();

    EntityItem* mEntityItem = nullptr;
};

#endif // EDITORENTITY_H
