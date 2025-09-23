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
class SteamFountain;

class EditorEntity {
private:
    //variables for a 3D model like cone, recovery vehicle...
    irr::scene::IMeshSceneNode* mSceneNode = nullptr;
    irr::scene::IMesh*  mMesh = nullptr;
    irr::io::path mModelFileName;

    irr::core::vector3df mOffsetMeshPos;

    //variables for a collectable, or sprite
    irr::video::ITexture* mSpriteTex = nullptr;
    irr::scene::IBillboardSceneNode *mBillSceneNode = nullptr;
    irr::core::dimension2d<irr::u32> mTextureSize;

    //special variable if we are a steamfountain
    SteamFountain* mSteamFountain = nullptr;

    bool mBoundingBoxVisible = false;

    bool mTransparentMesh = false;

public:
    //Constructor for a Mesh based 3D model (Mesh in OBJ file)
    EditorEntity(EntityManager* parentManager, EntityItem* itemPntr, irr::io::path modelFileName);

    //Constructor for a Mesh based 3D model (Mesh preloaded and existing in Irrlicht already)
    EditorEntity(EntityManager* parentManager, EntityItem* itemPntr, irr::scene::IMesh* mesh);

    //Constructor for a texture/Sprite based 2D model
    EditorEntity(EntityManager* parentManager, EntityItem* itemPntr, irr::video::ITexture* texture);
    ~EditorEntity();

    void UpdateBoundingBox();

    //does a position update of the higher level model
    //based on the low level map EntityItem data
    void UpdatePosition();
    
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

    bool mVisible = true;

    void Hide();
    void Show();

    //in case of a special EditiorEntity (like SteamFountain) we also
    //have the two methods below, which allows to control if linked effect
    //is visible or not
    void ShowEffect();
    void HideEffect();

    void SetMeshPosOffset(irr::core::vector3df newOffset);

    //special EditorEntities need updates like the
    //SteamFoutains
    void Update(irr::f32 frameDeltaTime);
};

#endif // EDITORENTITY_H
