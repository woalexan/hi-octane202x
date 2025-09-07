/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "../editorsession.h"
#include "editorentity.h"
#include "../editor.h"
#include "../models/entitymanager.h"
#include "../resources/texture.h"
#include "../models/steamfountain.h"

//Constructor for a Mesh based 3D model (Mesh in OBJ file)
EditorEntity::EditorEntity(EntityManager* parentManager, EntityItem* itemPntr, irr::io::path modelFileName) {
    mParentManager = parentManager;
    mEntityItem = itemPntr;
    mModelFileName = modelFileName;

    //Position is the center of the BillboardSceneNode
    //Position in the level file could be the bottom location at the surface
    //therefore we need to add the height of the billboard to the Y coordinate
    mPosition = mEntityItem->getCenter();

    mMesh = parentManager->mInfra->mSmgr->getMesh(modelFileName);
    mSceneNode = parentManager->mInfra->mSmgr->addMeshSceneNode(mMesh);

    mSceneNode->setScale(irr::core::vector3d<irr::f32>(1,1,1));
    mSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    mSceneNode->setPosition(mPosition);
    mSceneNode->setVisible(true);

    UpdateBoundingBox();
}

//Constructor for a Mesh based 3D model (Mesh preloaded and existing in Irrlicht already)
EditorEntity::EditorEntity(EntityManager* parentManager, EntityItem* itemPntr, irr::scene::IMesh* mesh) {
    mParentManager = parentManager;
    mEntityItem = itemPntr;

    //Position is the center of the BillboardSceneNode
    //Position in the level file could be the bottom location at the surface
    //therefore we need to add the height of the billboard to the Y coordinate
    mPosition = mEntityItem->getCenter();

    //Special case for SteamFountain?
    if ((itemPntr->getEntityType() == Entity::SteamLight) || (itemPntr->getEntityType() == Entity::SteamStrong)) {
        //get the cloud sprite from the game
        mSpriteTex = mParentManager->mTexLoader->spriteTex.at(17);

        mTransparentMesh = true;

        //Create us a SteamFountain for visible effect
        mSteamFountain = new SteamFountain(mSpriteTex, itemPntr, mParentManager->mInfra->mSmgr, mParentManager->mInfra->mDriver,
            mPosition, 100);

        mSteamFountain->Activate();
    }

    mMesh = mesh;
    mSceneNode = parentManager->mInfra->mSmgr->addMeshSceneNode(mMesh);

    mSceneNode->setScale(irr::core::vector3d<irr::f32>(1,1,1));
    mSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);

    if (mTransparentMesh) {
        mSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    }

    mSceneNode->setPosition(mPosition);
    mSceneNode->setVisible(true);

    UpdateBoundingBox();
}

//Constructor for a texture/Sprite based 2D model
EditorEntity::EditorEntity(EntityManager* parentManager, EntityItem* itemPntr, irr::video::ITexture* texture) {
    mParentManager = parentManager;
    mEntityItem = itemPntr;
    mSpriteTex = texture;

    //Position is the center of the BillboardSceneNode
    //Position in the level file could be the bottom location at the surface
    //therefore we need to add the height of the billboard to the Y coordinate
    mPosition = itemPntr->getCenter();

    //Note 27.12.2024: put a little bit higher,
    //so that it can be collected much more reliable
    //with bounding box
    mPosition.Y += parentManager->GetCollectableCenterHeight();

    mBillSceneNode = parentManager->mInfra->mSmgr->addBillboardSceneNode();
    mBillSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
    mBillSceneNode->setMaterialTexture(0, mSpriteTex);

    //Important: let collectables (Billboards) unaffected by lightning,
    //otherwise there are sometimes not good to see for the player
    mBillSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    mBillSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    mBillSceneNode->setPosition(mPosition);
    mBillSceneNode->setSize(parentManager->GetCollectableSize());
    mBillSceneNode->setVisible(true);

    UpdateBoundingBox();
}

Entity::EntityType EditorEntity::GetEntityType() {
    return mEntityItem->getEntityType();
}

irr::core::vector2di EditorEntity::GetCellCoord() {
    return mEntityItem->getCell();
}

void EditorEntity::Update(irr::f32 frameDeltaTime) {
    if (mSteamFountain != nullptr) {
        mSteamFountain->TriggerUpdate(frameDeltaTime);
    }
}

void EditorEntity::UpdateBoundingBox() {
    if (mSceneNode != nullptr) {
        //get bounding box for this model
        mSceneNode->updateAbsolutePosition();
        mBoundingBox = this->mSceneNode->getTransformedBoundingBox();
    }

    if (mBillSceneNode != nullptr) {
        //get bounding box for this collectible
        mBillSceneNode->updateAbsolutePosition();
        mBoundingBox = this->mBillSceneNode->getTransformedBoundingBox();
    }
}

void EditorEntity::ShowBoundingBox() {
    if (!mBoundingBoxVisible) {
        if (mSceneNode != nullptr) {
            mSceneNode->setDebugDataVisible(EDS_BBOX);
        }
        if (mBillSceneNode != nullptr) {
            mBillSceneNode->setDebugDataVisible(EDS_BBOX);
        }
        mBoundingBoxVisible = true;
    }
}

void EditorEntity::HideBoundingBox() {
    if (mBoundingBoxVisible) {
        if (mSceneNode != nullptr) {
            mSceneNode->setDebugDataVisible(EDS_OFF);
        }
        if (mBillSceneNode != nullptr) {
            mBillSceneNode->setDebugDataVisible(EDS_OFF);
        }
        mBoundingBoxVisible = false;
    }
}

void EditorEntity::Hide() {
  if (mVisible) {
      mVisible = false;

      if (mSceneNode != nullptr) {
          mSceneNode->setVisible(false);
      }

      if (mBillSceneNode != nullptr) {
          mBillSceneNode->setVisible(false);
      }

      if (mSteamFountain != nullptr) {
          mSteamFountain->Hide();
      }
  }
}

void EditorEntity::Show() {
    if (!mVisible) {
        mVisible = true;

        if (mSceneNode != nullptr) {
            mSceneNode->setVisible(true);
        }

        if (mBillSceneNode != nullptr) {
            mBillSceneNode->setVisible(true);
        }

        if (mSteamFountain != nullptr) {
            mSteamFountain->Show();
        }
    }
}

void EditorEntity::SetNewHeight(irr::f32 newHeight) {
    mPosition.Y = newHeight;

    if (mSceneNode != nullptr) {
        mSceneNode->setPosition(mPosition);
    }

    if (mBillSceneNode != nullptr) {
        mBillSceneNode->setPosition(mPosition);
    }

    //we need to update the bounding box
    UpdateBoundingBox();
}

irr::f32 EditorEntity::GetCurrentHeight() {
    return mPosition.Y;
}

irr::u16 EditorEntity::GetSpriteNumber() {
    Entity::EntityType entityType = mEntityItem->getEntityType();

    return this->mParentManager->GetCollectableSpriteNumber(entityType);
}

EditorEntity::~EditorEntity() {
    //cleanup editor entity stuff

    //for 3D model type
    if (mSceneNode != nullptr) {
        //remove SceneNode
        this->mSceneNode->remove();

        //remove mesh
        this->mParentManager->mInfra->mSmgr->getMeshCache()->removeMesh(mMesh);
    }

    //for the 2D sprite type
    if (mBillSceneNode != nullptr) {
        //remove SceneNode
        this->mBillSceneNode->remove();
    }

    //if this was a special SteamFoutain enitorEntity
    //item cleanup steamFountain as well
    if (mSteamFountain != nullptr) {
        delete mSteamFountain;
    }
}
