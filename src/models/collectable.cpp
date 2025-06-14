/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "collectable.h"
#include "../infrabase.h"

//this constructor is for the first type of entity/collectable (which is created based on a game map file entity item)
Collectable::Collectable(InfrastructureBase* infra, EntityItem* entityItem, vector3d<irr::f32> pos, irr::video::ITexture* texture, bool enableLightning) {
    this->mEntityItem = entityItem;
    this->mInfra = infra;

    mEnableLightning = enableLightning;

    //Point to the correct (billboard) texture
    collectable_tex = texture;
    texturesize = collectable_tex->getSize();

    SetupSceneNode(mEntityItem->getEntityType(), pos);

    //default is non visible after
    //start of game, and before entity group
    //1 is triggered (group 1 is triggered once
    //at game start to make initial collectables visible)
    this->isVisible = false;

    //I am not active (visible), hide my sceneNode
    this->billSceneNode->setVisible(false);
}

//this constructor is for the second type of entity/collectable (which is temporarily spawned when a player craft breaks down)
Collectable::Collectable(InfrastructureBase* infra, Entity::EntityType type, vector3d<irr::f32> pos, irr::video::ITexture* texture, bool enableLightning) {
    mInfra = infra;

    //for the second type of collectable (spawned temporary collectable)
    //there is no entity Item object in the background, is always nullptr
    this->mEntityItem = nullptr;

    //the type of collectable is stored for this second type
    //in a different member variable directly in this object
    this->mEntityType = type;

    mEnableLightning = enableLightning;

    //Point to the correct (billboard) texture
    collectable_tex = texture;
    texturesize = collectable_tex->getSize();

    SetupSceneNode(type, pos);

    //for the spawned collectables the default
    //setting is visible immediately
    this->isVisible = true;

    //I am active (visible), show my sceneNode
    this->billSceneNode->setVisible(true);
}

irr::f32 Collectable::GetCollectableCenterHeight() {
    return (CollectableSize_h * 1.25f);
}

void Collectable::SetupSceneNode(Entity::EntityType type, irr::core::vector3df pos) {
    //Position is the center of the BillboardSceneNode
    //Position in the level file could be the bottom location at the surface
    //therefore we need to add the height of the billboard to the Y coordinate
    Position = pos;

    //Note 27.12.2024: put a little bit higher,
    //so that it can be collected much more reliable
    //with bounding box
    Position.Y += GetCollectableCenterHeight();

    this->m_Size.set(CollectableSize_w, CollectableSize_h, irr::f32(0.01f));

    this->billSceneNode = mInfra->mSmgr->addBillboardSceneNode();
    this->billSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
    this->billSceneNode->setMaterialTexture(0, collectable_tex);

    //Important: let collectables (Billboards) unaffected by lightning,
    //otherwise there are sometimes not good to see for the player
    this->billSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    this->billSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    this->billSceneNode->setPosition(Position);
    this->billSceneNode->setSize(irr::core::dimension2d<irr::f32>(CollectableSize_w, CollectableSize_h));

    //get bounding box for this collectible
    this->billSceneNode->updateAbsolutePosition();
    this->boundingBox = this->billSceneNode->getTransformedBoundingBox();
}

irr::core::vector2df Collectable::GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord) {
    threeDCoord = this->Position;
    irr::core::vector2df result(this->Position.X, this->Position.Z);

    return result;
}

//allows to update position if collectable
//for example used by the collectablespawner
//Important note: does NOT update the position of an
//underlying entityItem, only useful for type 2 collectable!
void Collectable::UpdatePosition(irr::core::vector3df newPostion) {
    this->Position = newPostion;

    this->billSceneNode->setPosition(this->Position);
}

void Collectable::SetVisible(bool visible) {
    this->isVisible = visible;

    this->billSceneNode->setVisible(visible);
}

//this function must be used to get the type of collectable
//because this function takes care of the two different types
//of collectable possible
Entity::EntityType Collectable::GetCollectableType() {
    //type 1 collectable (from map file)?
    if (mEntityItem != nullptr) {
        //type 1, information is stored within a EntityItem object
        //from the level/map file
        return (mEntityItem->getEntityType());
    }

    //is a type 2 collectable (spawned, temporariy)
    //not stored inside the map file
    //here type is stored inside a member variable
    //directly in this object
    return mEntityType;
}

void Collectable::UpdateType2Collectable(irr::f32 deltaTime) {
    //if this is a type 1 collectable, and this function was
    //accidently called, just exit
    if (this->mEntityItem != nullptr)
        return;

    //reduce remaining lifetime with deltaTime
    this->remainingLifeTime -= deltaTime;

    //time for item to disappear?
    if (this->remainingLifeTime < 0.0f) {
        SetVisible(false);
    }
}

bool Collectable::GetType2CollectableCleanUpNecessary() {
    if (this->remainingLifeTime < 0.0f) {
        return true;
    }

    return false;
}

Collectable::~Collectable() {
    //cleanup collectable stuff

    //remove SceneNode
    this->billSceneNode->remove();
}

bool Collectable::GetIfVisible() {
    return this->isVisible;
}

void Collectable::PickedUp() {
    this->isVisible = false;

    //I am not active (visible) anymore, hide my sceneNode
    this->billSceneNode->setVisible(false);

   //if type 2 collectible, delete this collectible
   if (this->mEntityItem == nullptr) {
        this->remainingLifeTime = -1.0f;
   }
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
