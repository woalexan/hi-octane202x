/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "../infrabase.h"
#include "entitymanager.h"
#include "../resources/levelfile.h"
#include "../models/levelterrain.h"
#include "../models/levelblocks.h"
#include "../models/column.h"
#include "../models/morph.h"
#include "../draw/drawdebug.h"
#include "../models/editorentity.h"
#include "../resources/texture.h"
#include "../utils/logging.h"
#include "../resources/levelfile.h"
#include "irrlicht.h"

//If specified color is not available, returns a white cube
irr::scene::IMesh* EntityManager::GetCubeMeshWithColor(ColorStruct* whichColor) {
    std::vector<std::pair<ColorStruct*, irr::scene::IMesh*>>::iterator it;

    for (it = mCubeMeshVec.begin(); it != mCubeMeshVec.end(); ++it) {
        if ((*it).first == whichColor) {
            //correct cube Mesh with this color found
            return (*it).second;
        }
    }

    //correct color not found, return white cube
    //which is at index0
    return mCubeMeshVec.at(0).second;
}

IMesh* EntityManager::CreateSelectionMeshBox(irr::core::vector3df scaleFactors, irr::video::SColor boxColor) {

    //Some TriggerCraft and TriggerRocket entities have an OffsetX and OffsetY value of 0
    //in this case make sure that a minimum Size of the Mesh is guaranteed, so that we see something
    if (scaleFactors.X < 0.01f) {
        scaleFactors.X = 0.05f;
    }

    if (scaleFactors.Y < 0.01f) {
        scaleFactors.Y = 0.05f;
    }

    if (scaleFactors.Z < 0.01f) {
        scaleFactors.Z = 0.05f;
    }

    //the specified color here does not matter, is replaced afterwards anyway
    irr::scene::IMesh* mesh = CreateCubeMesh(1.0f, mInfra->mDrawDebug->pink);

    irr::scene::IMeshManipulator* meshManipulator = mInfra->mDriver->getMeshManipulator();
    meshManipulator->scale(mesh, scaleFactors);
    meshManipulator->setVertexColors(mesh, boxColor);
    meshManipulator->setVertexColorAlpha(mesh, boxColor.getAlpha());

    return mesh;
}

void EntityManager::CleanupAllCubeMesh() {
    std::vector<std::pair<ColorStruct*, irr::scene::IMesh*>>::iterator it;
    irr::scene::IMesh* meshPntr = nullptr;

    for (it = mCubeMeshVec.begin(); it != mCubeMeshVec.end(); ) {
           meshPntr = (*it).second;

           it = mCubeMeshVec.erase(it);

           //get rid of the custom mesh from the Meshcache
           mInfra->mSmgr->getMeshCache()->removeMesh(meshPntr);
    }
}

EntityManager::EntityManager(InfrastructureBase* infra, LevelFile* levelRes, LevelTerrain* levelTerrain, LevelBlocks* levelBlocks,
                             TextureLoader* texLoader) {
    mInfra = infra;
    mLevelFile = levelRes;
    mLevelTerrain = levelTerrain;
    mLevelBlocks = levelBlocks;
    mTexLoader = texLoader;

    mEntityVec.clear();
    mSteamFountainVec.clear();

    //create a simple small Cube Mesh for Waypoint and
    //Wallsegment Editor Entity item objects
    mCubeMeshVec.clear();

    //Important note: Keep cube with Color White at the first position of the std::Vector!
    mCubeMeshVec.push_back(std::make_pair(mInfra->mDrawDebug->white, CreateCubeMesh(0.2f, mInfra->mDrawDebug->white)));

    //order of all remaining colors does not matter!
    mCubeMeshVec.push_back(std::make_pair(mInfra->mDrawDebug->grey, CreateCubeMesh(0.2f, mInfra->mDrawDebug->grey)));
    mCubeMeshVec.push_back(std::make_pair(mInfra->mDrawDebug->orange, CreateCubeMesh(0.2f, mInfra->mDrawDebug->orange)));
    mCubeMeshVec.push_back(std::make_pair(mInfra->mDrawDebug->red, CreateCubeMesh(0.2f, mInfra->mDrawDebug->red)));
    mCubeMeshVec.push_back(std::make_pair(mInfra->mDrawDebug->colorShieldCharger, CreateCubeMesh(0.2f, mInfra->mDrawDebug->colorShieldCharger)));
    mCubeMeshVec.push_back(std::make_pair(mInfra->mDrawDebug->colorFuelCharger, CreateCubeMesh(0.2f, mInfra->mDrawDebug->colorFuelCharger)));
    mCubeMeshVec.push_back(std::make_pair(mInfra->mDrawDebug->colorAmmoCharger, CreateCubeMesh(0.2f, mInfra->mDrawDebug->colorAmmoCharger)));

    //Create the box mesh for surrounding SteamFountains
    mSteamFountainMesh = CreateSelectionMeshBox(irr::core::vector3df(1.0f, 4.0f, 1.0f),
        DEF_EDITOR_ENTITYMANAGER_STEAMFOUNTAIN_SELMESHBOXCOLOR);

    mSelectionMeshVec.clear();
     
    //    //create empty checkpoint info vector
    //    checkPointVec = new std::vector<CheckPointInfoStruct*>;
    //    checkPointVec->clear();

          //create an empty waypoint link info vector
    //    wayPointLinkVec = new std::vector<WayPointLinkInfoStruct*>;
    //    wayPointLinkVec->clear();

    //    steamFountainVec = new std::vector<SteamFountain*>;
    //    steamFountainVec->clear();

    //    //my vector of recovery vehicles
    //    recoveryVec = new std::vector<Recovery*>;
    //    recoveryVec->clear();

    //    //my vector of cones on the race track
    //    coneVec = new std::vector<Cone*>;
    //    coneVec->clear();

    //    mTimerVec.clear();
    //    mExplosionEntityVec.clear();
}

EntityManager::~EntityManager() {
    CleanUpMorphs();

    CleanupAllCustomMesh();
    CleanupAllCubeMesh();

    CleanUpEntities();

    if (mSteamFountainMesh != nullptr) {
        //remove this Mesh from the Meshcache
        mInfra->mSmgr->getMeshCache()->removeMesh(mSteamFountainMesh);
        mSteamFountainMesh = nullptr;
    }

    if (mRenderToTargetTex != nullptr) {
        mInfra->mDriver->removeTexture(mRenderToTargetTex);
        mRenderToTargetTex = nullptr;
    }

    if (mTexImageRecoveryVehicle != nullptr) {
        mInfra->mDriver->removeTexture(mTexImageRecoveryVehicle);
        mTexImageRecoveryVehicle = nullptr;
    }

    if (mTexImageRaceVehicle != nullptr) {
        mInfra->mDriver->removeTexture(mTexImageRaceVehicle);
        mTexImageRaceVehicle = nullptr;
    }

    if (mTexImageCone != nullptr) {
        mInfra->mDriver->removeTexture(mTexImageCone);
        mTexImageCone = nullptr;
    }

    if (mTexImageEmpty != nullptr) {
        mInfra->mDriver->removeTexture(mTexImageEmpty);
        mTexImageEmpty = nullptr;
    }
}

void EntityManager::SetShowSpecialEditorEntityTransparentSelectionBoxes(bool visible) {
    mShowSpecialEditorEntityTransparentSelectionBoxes = visible;
}

void EntityManager::SetVisibleEntityType(Entity::EntityType whichType, bool visible) {
    std::vector<EditorEntity*>::iterator it;
    Entity::EntityType type;

    for (it = mEntityVec.begin(); it != mEntityVec.end(); ++it) {
        type = (*it)->GetEntityType();

        if (type == whichType) {
            if (visible) {
                //modified handling for special type like SteamFountains
                if ((type == Entity::SteamStrong) || (type == Entity::SteamLight)) {
                    (*it)->ShowEffect();
                    //in EntityMode show additional transparend box to be able to select
                    //this entity
                    if (mShowSpecialEditorEntityTransparentSelectionBoxes) {
                        (*it)->Show();
                    }
                    else {
                        (*it)->Hide();
                    }
                }
                else {
                    (*it)->Show();
                }
            } else {
                if ((type == Entity::SteamStrong) || (type == Entity::SteamLight)) {
                    (*it)->HideEffect();
                    (*it)->Hide();
                }
                else {
                    (*it)->Hide();
                }
            }
        }
    }
}

bool EntityManager::IsVisible(irr::u8 whichEntityClass) {
    switch (whichEntityClass) {
        case DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES: {
            return(mShowCollectibles);
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY: {
           return(mShowRecoveryVehicles);
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_CONES: {
            return(mShowCones);
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS: {
            return(mShowWayPoints);
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS: {
            return(mShowWallSegments);
        }


        case DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS: {
            return(mShowTriggers);
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS: {
            return(mShowCameras);
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS: {
            return(mShowEffects);
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS: {
            return(mShowMorphs);
        }

        default: {
            return(false);
        }
    }
}

bool EntityManager::EntityIsPowerUpCollectible(EditorEntity* entity) {
    Entity::EntityType type = entity->mEntityItem->getEntityType();

    switch (type) {
        case Entity::EntityType::ExtraFuel:
        case Entity::EntityType::FuelFull:
        case Entity::EntityType::DoubleFuel:

        case Entity::EntityType::ExtraAmmo:
        case Entity::EntityType::AmmoFull:
        case Entity::EntityType::DoubleAmmo:

        case Entity::EntityType::ExtraShield:
        case Entity::EntityType::ShieldFull:
        case Entity::EntityType::DoubleShield:

        case Entity::EntityType::BoosterUpgrade:
        case Entity::EntityType::MissileUpgrade:
        case Entity::EntityType::MinigunUpgrade:  {
            return true;
        }

        default: {
           return false;
        }
    }
}

void EntityManager::SetVisible(irr::u8 whichEntityClass, bool visible) {
    switch (whichEntityClass) {
        case DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES: {
            mShowCollectibles = visible;
            SetVisibleEntityType(Entity::ExtraFuel, visible);
            SetVisibleEntityType(Entity::FuelFull, visible);
            SetVisibleEntityType(Entity::DoubleFuel, visible);

            SetVisibleEntityType(Entity::ExtraAmmo, visible);
            SetVisibleEntityType(Entity::AmmoFull, visible);
            SetVisibleEntityType(Entity::DoubleAmmo, visible);

            SetVisibleEntityType(Entity::ExtraShield, visible);
            SetVisibleEntityType(Entity::ShieldFull, visible);
            SetVisibleEntityType(Entity::DoubleShield, visible);

            SetVisibleEntityType(Entity::BoosterUpgrade, visible);
            SetVisibleEntityType(Entity::MissileUpgrade, visible);
            SetVisibleEntityType(Entity::MinigunUpgrade, visible);

            break;
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY: {
           mShowRecoveryVehicles = visible;
           SetVisibleEntityType(Entity::RecoveryTruck, visible);

           break;
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_CONES: {
            mShowCones = visible;
            SetVisibleEntityType(Entity::Cone, visible);

            break;
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS: {
            mShowWayPoints = visible;
            SetVisibleEntityType(Entity::WaypointAmmo, visible);
            SetVisibleEntityType(Entity::WaypointFuel, visible);
            SetVisibleEntityType(Entity::WaypointShield, visible);

            SetVisibleEntityType(Entity::WaypointFast, visible);
            SetVisibleEntityType(Entity::WaypointShortcut, visible);
            SetVisibleEntityType(Entity::WaypointSpecial1, visible);

            SetVisibleEntityType(Entity::WaypointSpecial2, visible);
            SetVisibleEntityType(Entity::WaypointSpecial3, visible);
            SetVisibleEntityType(Entity::WaypointSlow, visible);

            break;
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS: {
            mShowWallSegments = visible;

            SetVisibleEntityType(Entity::WallSegment, visible);

            break;
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS: {
            mShowMorphs = visible;

            SetVisibleEntityType(Entity::MorphOnce, visible);
            SetVisibleEntityType(Entity::MorphPermanent, visible);
            SetVisibleEntityType(Entity::MorphSource1, visible);
            SetVisibleEntityType(Entity::MorphSource2, visible);

            break;
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS: {
            mShowTriggers = visible;

            SetVisibleEntityType(Entity::TriggerCraft, visible);
            SetVisibleEntityType(Entity::TriggerTimed, visible);
            SetVisibleEntityType(Entity::TriggerRocket, visible);
            SetVisibleEntityType(Entity::Checkpoint, visible);

            break;
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS: {
            mShowEffects = visible;

            SetVisibleEntityType(Entity::SteamStrong, visible);
            SetVisibleEntityType(Entity::SteamLight, visible);
            SetVisibleEntityType(Entity::Explosion, visible);

            break;
        }

        case DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS: {
            mShowCameras = visible;

            SetVisibleEntityType(Entity::Camera, visible);

            break;
        }

        default: {
            break;
        }
    }
}

irr::scene::IMesh* EntityManager::CreateCubeMesh(irr::f32 size, ColorStruct* cubeColor) {
   //This Source code was copied from Irrlicht source code
   //CGeometryCreator.cpp, function CGeometryCreator::createCubeMesh
   SMeshBuffer* buffer = new SMeshBuffer();

   // Create indices
   const u16 u[36] = {   0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6,
             7,3,0,   7,6,3,   9,5,2,   9,8,5,   0,11,10,   0,10,7};

   buffer->Indices.set_used(36);

   for (u32 i=0; i<36; ++i)
       buffer->Indices[i] = u[i];

   // Create vertices
   buffer->Vertices.reallocate(12);

   buffer->Vertices.push_back(video::S3DVertex(0,0,0, -1,-1,-1, *cubeColor->color, 0, 1));
   buffer->Vertices.push_back(video::S3DVertex(1,0,0,  1,-1,-1, *cubeColor->color, 1, 1));
   buffer->Vertices.push_back(video::S3DVertex(1,1,0,  1, 1,-1, *cubeColor->color, 1, 0));
   buffer->Vertices.push_back(video::S3DVertex(0,1,0, -1, 1,-1, *cubeColor->color, 0, 0));
   buffer->Vertices.push_back(video::S3DVertex(1,0,1,  1,-1, 1, *cubeColor->color, 0, 1));
   buffer->Vertices.push_back(video::S3DVertex(1,1,1,  1, 1, 1, *cubeColor->color, 0, 0));
   buffer->Vertices.push_back(video::S3DVertex(0,1,1, -1, 1, 1, *cubeColor->color, 1, 0));
   buffer->Vertices.push_back(video::S3DVertex(0,0,1, -1,-1, 1, *cubeColor->color, 1, 1));
   buffer->Vertices.push_back(video::S3DVertex(0,1,1, -1, 1, 1, *cubeColor->color, 0, 1));
   buffer->Vertices.push_back(video::S3DVertex(0,1,0, -1, 1,-1, *cubeColor->color, 1, 1));
   buffer->Vertices.push_back(video::S3DVertex(1,0,1,  1,-1, 1, *cubeColor->color, 1, 0));
   buffer->Vertices.push_back(video::S3DVertex(1,0,0,  1,-1,-1, *cubeColor->color, 0, 0));

   // Recalculate bounding box
   buffer->BoundingBox.reset(0,0,0);

   for (u32 i=0; i<12; ++i)
    {
        buffer->Vertices[i].Pos -= core::vector3df(0.5f, 0.5f, 0.5f);
        buffer->Vertices[i].Pos *= size;
        buffer->BoundingBox.addInternalPoint(buffer->Vertices[i].Pos);
    }

    SMesh* mesh = new SMesh;
    mesh->addMeshBuffer(buffer);
    buffer->drop();

    mesh->recalculateBoundingBox();
    return mesh;
}

void EntityManager::DrawWayPointLinks() {
   std::vector<EditorEntity*>::iterator it;
   ColorStruct* color;
   EditorEntity* linkedItem;
   int16_t linkedItemId;

   for (it = mEntityVec.begin(); it != mEntityVec.end(); ++it) {
     if (EntityIsWayPoint(*it)) {
        linkedItemId = (*it)->mEntityItem->getNextID();

        //if nextId is zero no item is linked
        if (linkedItemId != 0) {
            linkedItem = GetEditorEntityWithId(linkedItemId);

            if (linkedItem != nullptr) {
                //get color we want to use for this line
                color = GetColorForWayPointType((*it)->GetEntityType());
                mInfra->mDrawDebug->Draw3DArrow((*it)->mPosition, linkedItem->mPosition, 0.5f, color, 0.1f);
            }
        }
     }
   }
}

void EntityManager::DrawWallSegments() {
   std::vector<EditorEntity*>::iterator it;
   EditorEntity* linkedItem;
   int16_t linkedItemId;

   for (it = mEntityVec.begin(); it != mEntityVec.end(); ++it) {
     if (EntityIsWallSegment(*it)) {
        linkedItemId = (*it)->mEntityItem->getNextID();

        //if nextId is zero no item is linked
        if (linkedItemId != 0) {
            linkedItem = GetEditorEntityWithId(linkedItemId);

            if (linkedItem != nullptr) {
                mInfra->mDrawDebug->Draw3DArrow((*it)->mPosition, linkedItem->mPosition, 0.5f, mInfra->mDrawDebug->red, 0.1f);
            }
        }
     }
   }
}

void EntityManager::UnlinkEntity(EditorEntity* whichItem) {
    if (whichItem == nullptr)
        return;

    //nextID = 0 means not linked
    whichItem->mEntityItem->setNextID(0);
}

void EntityManager::LinkEntity(EditorEntity* whichItem, EditorEntity* nextItem) {
    if ((whichItem == nullptr) || (nextItem == nullptr))
        return;

    whichItem->mEntityItem->setNextID(nextItem->mEntityItem->get_ID());
}

//returns nullpntr in case EditorEntity with the specified ID
//is not found
EditorEntity* EntityManager::GetEditorEntityWithId(int16_t whichId) {
    std::vector<EditorEntity*>::iterator it;

    for (it = mEntityVec.begin(); it != mEntityVec.end(); ++it) {
      if ((*it)->mEntityItem->get_ID() == whichId) {
         return (*it);
      }
    }

    //not found
    return nullptr;
}

bool EntityManager::EntityIsWayPoint(EditorEntity* entity) {
    Entity::EntityType type = entity->mEntityItem->getEntityType();

    switch (type) {
        case Entity::EntityType::WaypointAmmo:
        case Entity::EntityType::WaypointFuel:
        case Entity::EntityType::WaypointShield:
        case Entity::EntityType::WaypointFast:
        case Entity::EntityType::WaypointShortcut:
        case Entity::EntityType::WaypointSpecial1:
        case Entity::EntityType::WaypointSpecial2:
        case Entity::EntityType::WaypointSpecial3:
        case Entity::EntityType::WaypointSlow: {
            return true;
        }

        default: {
           return false;
        }
    }
}

bool EntityManager::EntityIsWallSegment(EditorEntity *entity) {
    Entity::EntityType type = entity->mEntityItem->getEntityType();

    switch (type) {
        case Entity::EntityType::WallSegment: {
            return true;
        }

        default: {
           return false;
        }
    }
}

void EntityManager::Draw() {
    if (mShowWayPoints) {
        DrawWayPointLinks();
    }

    if (mShowWallSegments) {
        DrawWallSegments();
    }
}

void EntityManager::CleanUpEntities() {

   if (ENTWallsegmentsLine_List->size() > 0) {
       std::vector<LineStruct*>::iterator it;
       LineStruct* pntr;
       for (it = ENTWallsegmentsLine_List->begin(); it != ENTWallsegmentsLine_List->end(); ) {
           pntr = (LineStruct*)(*it);
           it = ENTWallsegmentsLine_List->erase(it);

           //free name inside LineStruct
           delete[] pntr->name;

           //delete LineStruct itself
           delete pntr;
       }
   }

   delete ENTWallsegmentsLine_List;
   ENTWallsegmentsLine_List = nullptr;

   if (ENTWaypoints_List->size() > 0) {
       std::vector<EntityItem*>::iterator it;

       for (it = ENTWaypoints_List->begin(); it != ENTWaypoints_List->end(); ) {
           it = ENTWaypoints_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTWaypoints_List;
   ENTWaypoints_List = nullptr;

   if (ENTWallsegments_List->size() > 0) {
       std::list<EntityItem*>::iterator it;

       for (it = ENTWallsegments_List->begin(); it != ENTWallsegments_List->end(); ) {
           it = ENTWallsegments_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTWallsegments_List;
   ENTWallsegments_List = nullptr;

   if (ENTTriggers_List->size() > 0) {
       std::list<EntityItem*>::iterator it;

       for (it = ENTTriggers_List->begin(); it != ENTTriggers_List->end(); ) {
           it = ENTTriggers_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTTriggers_List;
   ENTTriggers_List = nullptr;

  /* if (ENTCollectablesVec->size() > 0) {
       std::vector<Collectable*>::iterator it;
       Collectable* pntr;
       for (it = ENTCollectablesVec->begin(); it != ENTCollectablesVec->end(); ) {
           pntr = (Collectable*)(*it);
           it = ENTCollectablesVec->erase(it);

           //delete Collectable itself
           //this frees SceneNode and texture inside
           //collectable implementation
           delete pntr;
       }
   }

   delete ENTCollectablesVec;
   ENTCollectablesVec = nullptr;*/

   EditorEntity* pntrEditorEntity;

   if (mEntityVec.size() > 0) {
       std::vector<EditorEntity*>::iterator it;

       for (it = mEntityVec.begin(); it != mEntityVec.end(); ) {
           pntrEditorEntity = (*it);
           it = mEntityVec.erase(it);

           //cleanup the EditorEntity object
           //itself
           delete pntrEditorEntity;
       }
   }
}

void EntityManager::CleanUpMorphs() {
    if (Morphs.size() > 0) {
        std::list<Morph*>::iterator it;
        Morph* pntr;
        for (it = Morphs.begin(); it != Morphs.end(); ) {
            pntr = (*it);
            it = Morphs.erase(it);

            //delete Morph itself
            delete pntr;
        }
    }
}

irr::f32 EntityManager::GetCollectableCenterHeight() {
    return (EntityManagerCollectableSize_h * 1.25f);
}

irr::core::vector2df EntityManager::GetCollectableSize() {
    irr::core::vector2df size(EntityManagerCollectableSize_w, EntityManagerCollectableSize_h);

    return size;
}

void EntityManager::CreateLevelEntities() {

    ENTWaypoints_List = new std::vector<EntityItem*>;
    ENTWaypoints_List->clear();

    ENTWallsegments_List = new std::list<EntityItem*>;
    ENTWallsegments_List->clear();

    ENTWallsegmentsLine_List = new std::vector<LineStruct*>;
    ENTWallsegmentsLine_List->clear();

    ENTTriggers_List = new std::list<EntityItem*>;
    ENTTriggers_List->clear();

    //ENTCollectablesVec = new std::vector<Collectable*>;
    //ENTCollectablesVec->clear();

    //create all level entities
    for(std::vector<EntityItem*>::iterator loopi = this->mLevelFile->Entities.begin(); loopi != this->mLevelFile->Entities.end(); ++loopi) {
        CreateEntity(*loopi);
    }

    CreateModelPictures();

    //uncomment next line for debugging entities
    //table. Do not forget to comment out again!
    //DebugWriteEntityTableToCsvFile((char*)("dbgEntitiesTable.csv"));
}

ColorStruct* EntityManager::GetColorForWayPointType(Entity::EntityType whichType) {
    switch (whichType) {
        case Entity::EntityType::WaypointAmmo: {
           return(mInfra->mDrawDebug->colorAmmoCharger);
        }

        case Entity::EntityType::WaypointFuel: {
            return(mInfra->mDrawDebug->colorFuelCharger);
        }

        case Entity::EntityType::WaypointShield: {
            return(mInfra->mDrawDebug->colorShieldCharger);
        }

        case Entity::EntityType::WaypointFast: {
            return(mInfra->mDrawDebug->orange);
        }

        //default use grey
        case Entity::EntityType::WaypointShortcut:
        case Entity::EntityType::WaypointSpecial1:
        case Entity::EntityType::WaypointSpecial2:
        case Entity::EntityType::WaypointSpecial3:
        case Entity::EntityType::WaypointSlow:
        default: {
           return(mInfra->mDrawDebug->grey);
        }
    }
}

//for an unknown/undefined entity type returns an empty string
//in case of an error
irr::io::path EntityManager::GetModelForEntityType(Entity::EntityType entityType) {
    switch (entityType) {
        case Entity::EntityType::WaypointAmmo:
        case Entity::EntityType::WaypointFuel:
        case Entity::EntityType::WaypointShield:
        case Entity::EntityType::WaypointShortcut:
        case Entity::EntityType::WaypointSpecial1:
        case Entity::EntityType::WaypointSpecial2:
        case Entity::EntityType::WaypointSpecial3:
        case Entity::EntityType::WaypointFast:
        case Entity::EntityType::WaypointSlow:

        case Entity::EntityType::WallSegment:
        {
            return irr::io::path("");
        }

       case Entity::EntityType::TriggerCraft:
       case Entity::EntityType::TriggerRocket: {
           return irr::io::path("");
       }

       case Entity::EntityType::TriggerTimed: {
           return irr::io::path("");
       }

       case Entity::EntityType::MorphOnce:
       case Entity::EntityType::MorphPermanent:
       case Entity::EntityType::MorphSource1:
       case Entity::EntityType::MorphSource2: {
            return irr::io::path("");
       }

       case Entity::EntityType::RecoveryTruck: {
            return irr::io::path("extract/models/recov0-0.obj");
        }

        case Entity::EntityType::Cone: {
            return irr::io::path("extract/models/cone0-0.obj");
        }

        case Entity::EntityType::Checkpoint: {
            return irr::io::path("");
        }

        case Entity::EntityType::Camera: {
            return irr::io::path("");
        }

        case Entity::EntityType::Explosion: {
            return irr::io::path("");
        }

        //this are default collectable items from
        //the map files
        case Entity::EntityType::ExtraFuel:
        case Entity::EntityType::FuelFull:
        case Entity::EntityType::DoubleFuel:
        case Entity::EntityType::ExtraAmmo:
        case Entity::EntityType::AmmoFull:
        case Entity::EntityType::DoubleAmmo:
        case Entity::EntityType::ExtraShield:
        case Entity::EntityType::ShieldFull:
        case Entity::EntityType::DoubleShield:
        case Entity::EntityType::BoosterUpgrade:
        case Entity::EntityType::MissileUpgrade:
        case Entity::EntityType::MinigunUpgrade:  {
                 /*   //if entity type is invalid for a collectable the function below will fallback
                    //to sprite number 42, which is a sprite I did not know the purpose of
                    irr::u16 spriteNr = GetCollectableSpriteNumber(entity.getEntityType());

                    //Point to the correct (billboard) texture
                    collectable = new Collectable(this->mParentEditor, p_entity, entity.getCenter(), mTexLoader->spriteTex.at(spriteNr), false);
                    ENTCollectablesVec->push_back(collectable);
                    break;*/

                  return irr::io::path("");
        }

        case Entity::EntityType::UnknownShieldItem:
            {
                   return irr::io::path("");
            }

        case Entity::EntityType::UnknownItem:
        case Entity::EntityType::Unknown:
            {
                   return irr::io::path("");
            }

        case Entity::EntityType::SteamStrong:
        case Entity::EntityType::SteamLight: {
                   return irr::io::path("");
        }

        default:
            {
                   return irr::io::path("");
            }
    }
}

void EntityManager::UpdateSteamFoutains(irr::f32 frameDeltaTime) {
    //update all steam fontains
    std::vector <EditorEntity*>::iterator it;

    for (it = mSteamFountainVec.begin(); it != mSteamFountainVec.end(); ++it) {
        (*it)->Update(frameDeltaTime);
    }
}

void EntityManager::SetFog(bool enabled) {
    std::vector <EditorEntity*>::iterator it;

    for (it = mEntityVec.begin(); it != mEntityVec.end(); ++it) {
        (*it)->SetFog(enabled);
    }
}

void EntityManager::CreateEntity(EntityItem* p_entity) {
    //Line line;
    irr::f32 w, h;
    //Collectable* collectable;
    //Box box;

    //make local variable which points on pointer
    EntityItem entity = *p_entity;
    EntityItem* next = nullptr;

    //    if (!GroupedEntities.ContainsKey(entity.Group)) GroupedEntities.Add(entity.Group, new List<EntityItem>());
    //    GroupedEntities[entity.Group].Add(entity);

    float boxSize = 0;
    //collectable = nullptr;

    int next_ID = entity.getNextID();
    bool exists;

    if (next_ID != 0) {
        //see if a entity with this ID exists
        exists = mLevelFile->ReturnEntityItemWithId(next_ID, &next);
    }

    Entity::EntityType type = entity.getEntityType();

    switch (type) {
    case Entity::EntityType::WaypointAmmo:
    case Entity::EntityType::WaypointFuel:
    case Entity::EntityType::WaypointShield:
    case Entity::EntityType::WaypointFast:
    case Entity::EntityType::WaypointShortcut:
    case Entity::EntityType::WaypointSpecial1:
    case Entity::EntityType::WaypointSpecial2:
    case Entity::EntityType::WaypointSpecial3:
    case Entity::EntityType::WaypointSlow: {
        EditorEntity* newEntity;
        irr::scene::IMesh* wMesh = nullptr;

        wMesh = GetCubeMeshWithColor(GetColorForWayPointType(type));

        //use the prepared cubeMesh for Waypoint editor entity items
        newEntity = new EditorEntity(this, p_entity, wMesh);

        //modify cube position, so that it is not
        //stuck in the terrain tiles
        newEntity->SetNewHeight(newEntity->GetCurrentHeight() + EntityManagerCubeHeightDistance);

        mEntityVec.push_back(newEntity);
        break;
    }

    case Entity::EntityType::WallSegment: {

        /* if (next != nullptr) {
             LineStruct *line = new LineStruct;
             line->A = entity.getCenter();
             line->B = next->getCenter();
             //line = new Line(entity.Center, next.Center, color);
             //line->name.clear();
             //line->name.append("Wall segment line ");
             //line->name.append(std::to_string(entity.get_ID()));
             //line->name.append(" to ");
             //line->name.append(std::to_string(next->get_ID()));

             line->name = new char[100];
             sprintf(&line->name[0], "Wall segment line %d to %d", entity.get_ID(), next->get_ID());

             //remember a line between both waypoints for debugging purposes
             ENTWallsegmentsLine_List->push_back(line);
         }
        ENTWallsegments_List->push_back(p_entity);*/

        EditorEntity* newEntity;
        irr::scene::IMesh* wMesh = nullptr;

        wMesh = GetCubeMeshWithColor(mInfra->mDrawDebug->red);

        //use the prepared cubeMesh for WallPointSegments as well
        newEntity = new EditorEntity(this, p_entity, wMesh);

        //modify cube position, so that it is not
        //stuck in the terrain tiles
        newEntity->SetNewHeight(newEntity->GetCurrentHeight() + EntityManagerCubeHeightDistance);

        mEntityVec.push_back(newEntity);

        break;
    }

    case Entity::EntityType::TriggerCraft:
    case Entity::EntityType::TriggerRocket:
    case Entity::EntityType::Checkpoint: {
        EditorEntity* newEntity;

        irr::core::vector3df scaleVal(p_entity->getOffsetX(), 1.0f, p_entity->getOffsetY());

        irr::scene::IMesh* newCustomMesh;

        if (type == Entity::EntityType::TriggerCraft) {
            newCustomMesh = CreateSelectionMeshBox(scaleVal,
                DEF_EDITOR_ENTITYMANAGER_CRAFTTRIGGER_SELMESHBOXCOLOR);
        }
        else if (type == Entity::EntityType::Checkpoint) {
            newCustomMesh = CreateSelectionMeshBox(scaleVal,
                DEF_EDITOR_ENTITYMANAGER_CHECKPOINT_SELMESHBOXCOLOR);
        }
        else {
            newCustomMesh = CreateSelectionMeshBox(scaleVal,
                DEF_EDITOR_ENTITYMANAGER_MISSILETRIGGER_SELMESHBOXCOLOR);
        }

        //use the custom mesh created above
        newEntity = new EditorEntity(this, p_entity, newCustomMesh);

        mSelectionMeshVec.push_back(std::make_pair(newEntity, newCustomMesh));

        //modify surrounding box position, so that it is not
        //stuck in the terrain tiles
        //newEntity->SetNewHeight(newEntity->GetCurrentHeight() + EntityManagerCubeHeightDistance);

        //we additionally need to offset the Mesh position by half the OffsetX and OffsetY distance,
        //so that the custom Selection Mesh for this item is displayed at the correct location
        //X offset needs to be used in negative direction, as our universe is swapped at the X-Axis
        newEntity->SetMeshPosOffset(irr::core::vector3df(-scaleVal.X * 0.5f, 0.5f, scaleVal.Z * 0.5f));

        mEntityVec.push_back(newEntity);

        break;
    }

    case Entity::EntityType::TriggerTimed: {
        //the stopwatch image is at index 1 of editorTex vector!
        EditorEntity* newEntity = new EditorEntity(this, p_entity, mTexLoader->editorTex.at(1));
        mEntityVec.push_back(newEntity);
        break;
    }

    case Entity::EntityType::MorphOnce:
    case Entity::EntityType::MorphPermanent: {
        EditorEntity* newEntity;

        irr::core::vector3df scaleVal(p_entity->getOffsetX(), 1.0f, p_entity->getOffsetY());

        irr::scene::IMesh* newCustomMesh;

        newCustomMesh = CreateSelectionMeshBox(scaleVal,
            DEF_EDITOR_ENTITYMANAGER_MORPHTARGET_SELMESHBOXCOLOR);

        //use the custom mesh created above
        newEntity = new EditorEntity(this, p_entity, newCustomMesh);

        mSelectionMeshVec.push_back(std::make_pair(newEntity, newCustomMesh));

        //modify surrounding box position, so that it is not
        //stuck in the terrain tiles
        //newEntity->SetNewHeight(newEntity->GetCurrentHeight() + EntityManagerCubeHeightDistance);

        //we additionally need to offset the Mesh position by half the OffsetX and OffsetY distance,
        //so that the custom Selection Mesh for this item is displayed at the correct location
        //X offset needs to be used in negative direction, as our universe is swapped at the X-Axis
        newEntity->SetMeshPosOffset(irr::core::vector3df(-scaleVal.X * 0.5f, 0.5f, scaleVal.Z * 0.5f));

        mEntityVec.push_back(newEntity);

        w = entity.getOffsetX() + 1.0f;
        h = entity.getOffsetY() + 1.0f;
        //box = new Box(0, 0, 0, w, 1, h, new Vector4(0.1f, 0.3f, 0.9f, 0.5f));
        //box.Position = entity.Pos + Vector3.UnitY * 0.01f;
        //AddNode(box);

        EntityItem* source;

        std::vector<Column*> sourceColumns;
        sourceColumns.clear();

        //see if a entity with this ID exists
        bool entFound = mLevelFile->ReturnEntityItemWithId(entity.getNextID(), &source);

        if (entFound) {
            sourceColumns = mLevelBlocks->ColumnsInRange(source->getCell().X, source->getCell().Y, w, h);
        }

        // morph for this entity and its linked source
        std::vector<Column*> targetColumns = mLevelBlocks->ColumnsInRange(entity.getCell().X, entity.getCell().Y, w, h);

        //for morph optimization we want to keep the dynamic changing map parts in their own MeshBuffers and own SceneNodes
        //for this I decided to mark the dynamic parts of the maps (morph cells) with a bool variable inside the terrain tile data
        //so that later we can put this cells into their own Meshbuffers/SceneNodes

        //Additional note 03.01.2025: We need to include a little bit more cells into the dynamic terrain around the
        //initial morphing area defined in the level (+/- 5 cells as defined below), because otherwise when we run a
        //morph there will be areas in the terrain at the seems between static & dynamic area that do not behave
        //correctly (for example hole appear where the player can see through). But including slightly more area into the dynamic
        //mesh this problem does not arise.
        irr::u32 baseX = entity.getCell().X - 5;
        irr::u32 baseY = entity.getCell().Y - 5;

        irr::core::vector2di cellCoord;
        for (irr::u32 idxX = 0; idxX < (w + 5); idxX++) {
            for (irr::u32 idxY = 0; idxY < (h + 5); idxY++) {
                cellCoord.set(idxX + baseX, idxY + baseY);
                this->mLevelTerrain->ForceTileGridCoordRange(cellCoord);

                this->mLevelTerrain->pTerrainTiles[cellCoord.X][cellCoord.Y].dynamicMesh = true;
            }
        }

        if (entFound) {
            baseX = source->getCell().X - 5;
            baseY = source->getCell().Y - 5;

            for (irr::u32 idxX = 0; idxX < (w + 5); idxX++) {
                for (irr::u32 idxY = 0; idxY < (h + 5); idxY++) {
                    cellCoord.set(idxX + baseX, idxY + baseY);
                    this->mLevelTerrain->ForceTileGridCoordRange(cellCoord);

                    this->mLevelTerrain->pTerrainTiles[cellCoord.X][cellCoord.Y].dynamicMesh = true;
                }
            }
        }

        // regular morph
        if (targetColumns.size() == sourceColumns.size())
        {
            for (unsigned int i = 0; i < targetColumns.size(); i++)
            {
                targetColumns[i]->MorphSource = sourceColumns[i];
                sourceColumns[i]->MorphSource = targetColumns[i];
            }
        }
        else
        {
            // permanent morphs dont destroy buildings, instead they morph the column based on terrain height
            if (entity.getEntityType() == Entity::EntityType::MorphPermanent)
            {
                // we need to update surrounding columns too because they could be affected (one side of them)
                // (problem comes from not using terrain height for all columns in realtime)
                targetColumns = mLevelBlocks->ColumnsInRange(entity.getCell().X - 1, entity.getCell().Y - 1, w + 1, h + 1);

                // create dummy morph source columns at source position
                std::vector<Column*>::iterator colIt;

                for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
                    vector3d<irr::f32> colPos(0.0f, 0.0f, 0.0f);
                    colPos.X = source->getCell().X + ((*colIt)->Position.X - entity.getCell().X);
                    colPos.Y = 0.0f;
                    colPos.Z = source->getCell().Y + ((*colIt)->Position.Z - entity.getCell().Y);

                    //Important: Do not create a special column here, this is a normal game map column!
                    (*colIt)->MorphSource = new Column(mLevelTerrain, mLevelBlocks, (*colIt)->Definition, colPos, mLevelFile, false, nullptr);
                }

                sourceColumns.clear();
            }
            else
            {
                // in this case (MorphOnce) there are no target columns and
                // (target and source areas are swapped from game perspective)
                // and buildings have to be destroyed as soon as the morph starts
                std::vector<Column*>::iterator colIt;

                for (colIt = sourceColumns.begin(); colIt != sourceColumns.end(); ++colIt) {
                    (*colIt)->DestroyOnMorph = true;
                }

                for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
                    (*colIt)->DestroyOnMorph = true;
                }
            }
        }

        // create and collect morph instances
        Morph* morph = new Morph(entity.get_ID(), source, p_entity, (int)w, (int)h,
            entity.getEntityType() == Entity::EntityType::MorphPermanent,
            this->mLevelTerrain, this->mLevelBlocks);
        std::vector<Column*>::iterator colIt;

        for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
            morph->Columns.push_back(*colIt);
        }

        Morphs.push_back(morph);

        // source
        morph = new Morph(entity.get_ID(), p_entity, source, (int)w, (int)h,
            entity.getEntityType() == Entity::EntityType::MorphPermanent,
            this->mLevelTerrain, this->mLevelBlocks);
        for (colIt = sourceColumns.begin(); colIt != sourceColumns.end(); ++colIt) {
            morph->Columns.push_back(*colIt);
        }

        Morphs.push_back(morph);
        break;
    }

    case Entity::EntityType::MorphSource1:
    case Entity::EntityType::MorphSource2: {
        EditorEntity* newEntity;

        irr::core::vector3df scaleVal(p_entity->getOffsetX(), 1.0f, p_entity->getOffsetY());

        irr::scene::IMesh* newCustomMesh;

        newCustomMesh = CreateSelectionMeshBox(scaleVal,
            DEF_EDITOR_ENTITYMANAGER_MORPHSOURCE_SELMESHBOXCOLOR);

        //use the custom mesh created above
        newEntity = new EditorEntity(this, p_entity, newCustomMesh);

        mSelectionMeshVec.push_back(std::make_pair(newEntity, newCustomMesh));

        //modify surrounding box position, so that it is not
        //stuck in the terrain tiles
        //newEntity->SetNewHeight(newEntity->GetCurrentHeight() + EntityManagerCubeHeightDistance);

        //we additionally need to offset the Mesh position by half the OffsetX and OffsetY distance,
        //so that the custom Selection Mesh for this item is displayed at the correct location
        //X offset needs to be used in negative direction, as our universe is swapped at the X-Axis
        newEntity->SetMeshPosOffset(irr::core::vector3df(-scaleVal.X * 0.5f, 0.5f, scaleVal.Z * 0.5f));

        mEntityVec.push_back(newEntity);

        w = entity.getOffsetX() + 1.0f;
        h = entity.getOffsetY() + 1.0f;

        //for morph optimization we want to keep the dynamic changing map parts in their own MeshBuffers and own SceneNodes
        //for this I decided to mark the dynamic parts of the maps (morph cells) with a bool variable inside the terrain tile data
        //so that later we can put this cells into their own Meshbuffers/SceneNodes
        irr::u32 baseX = entity.getCell().X - 5;
        irr::u32 baseY = entity.getCell().Y - 5;

        irr::core::vector2di cellCoord;
        for (irr::u32 idxX = 0; idxX < (w + 5); idxX++) {
            for (irr::u32 idxY = 0; idxY < (h + 5); idxY++) {
                cellCoord.set(idxX + baseX, idxY + baseY);
                this->mLevelTerrain->ForceTileGridCoordRange(cellCoord);

                this->mLevelTerrain->pTerrainTiles[cellCoord.X][cellCoord.Y].dynamicMesh = true;
            }
        }

        break;
    }

    case Entity::EntityType::Cone:
    case Entity::EntityType::RecoveryTruck: {
        /*TODO: Recovery *recov1 =
                new Recovery(this, entity.getCenter().X, entity.getCenter().Y + 6.0f, entity.getCenter().Z, mInfra->mSmgr);

        //remember all recovery vehicles in a vector for later use
        this->recoveryVec->push_back(recov1);*/
        irr::io::path modelFileName = GetModelForEntityType(type);
        EditorEntity* newEntity;

        if (modelFileName != "") {
            newEntity = new EditorEntity(this, p_entity, modelFileName);

            //for recovery vehicle we want to modify its position, so that it is not
            //stuck in the terrain tiles
            if (type == Entity::EntityType::RecoveryTruck) {
                newEntity->SetNewHeight(newEntity->GetCurrentHeight() + EntityManagerRecoveryVehicleFlyingHeight);
            }

            mEntityVec.push_back(newEntity);
        }

        break;
    }

    case Entity::EntityType::Camera: {
        //the camera image is at index 1 of editorTex vector!
        EditorEntity* newEntity = new EditorEntity(this, p_entity, mTexLoader->editorTex.at(0));
        mEntityVec.push_back(newEntity);
        break;
    }

    case Entity::EntityType::Explosion: {
        //For explosion entities use the Game Sprite number 4
        EditorEntity* newEntity = new EditorEntity(this, p_entity, mTexLoader->spriteTex.at(4));
        mEntityVec.push_back(newEntity);
        break;
    }

                                      //this are default collectable items from
                                      //the map files
    case Entity::EntityType::ExtraFuel:
    case Entity::EntityType::FuelFull:
    case Entity::EntityType::DoubleFuel:
    case Entity::EntityType::ExtraAmmo:
    case Entity::EntityType::AmmoFull:
    case Entity::EntityType::DoubleAmmo:
    case Entity::EntityType::ExtraShield:
    case Entity::EntityType::ShieldFull:
    case Entity::EntityType::DoubleShield:
    case Entity::EntityType::BoosterUpgrade:
    case Entity::EntityType::MissileUpgrade:
    case Entity::EntityType::MinigunUpgrade: {
        //if entity type is invalid for a collectable the function below will fallback
        //to sprite number 42, which is a sprite I did not know the purpose of
        irr::u16 spriteNr = GetCollectableSpriteNumber(entity.getEntityType());

        //Point to the correct (billboard) texture
        //collectable = new Collectable(this->mParentEditor, p_entity, entity.getCenter(), mTexLoader->spriteTex.at(spriteNr), false);
        //ENTCollectablesVec->push_back(collectable);

        EditorEntity* newEntity = new EditorEntity(this, p_entity, mTexLoader->spriteTex.at(spriteNr));
        mEntityVec.push_back(newEntity);
        break;
    }

    case Entity::EntityType::UnknownShieldItem:
    {
        //uncomment the next 2 lines to show this items also to the player
        // collectable = new Collectable(41, entity.get_Center(), color, driver);
        // ENTCollectables_List.push_back(collectable);
        break;
    }

    case Entity::EntityType::UnknownItem:
    case Entity::EntityType::Unknown:
    {
        //uncomment the next 2 lines to show this items also to the player
        // collectable = new Collectable(50, entity.get_Center(), color, driver);
        // ENTCollectables_List.push_back(collectable);
        break;
    }

    case Entity::EntityType::SteamStrong:
    case Entity::EntityType::SteamLight:
    {
        EditorEntity* newEntity;

        //use the prepared SteamFountain Mesh
        newEntity = new EditorEntity(this, p_entity, mSteamFountainMesh);

        //modify surrounding box position, so that it is not
        //stuck in the terrain tiles
        newEntity->SetNewHeight(newEntity->GetCurrentHeight() + EntityManagerSteamFountainBoxHeightDistance);

        mEntityVec.push_back(newEntity);
        mSteamFountainVec.push_back(newEntity);

        break;
    }

    default:
    {
        boxSize = 0.98f;
        break;
    }
    }
}

irr::video::ITexture* EntityManager::GetImageForEntityType(Entity::EntityType mEntityType) {
    switch (mEntityType) {
        case Entity::EntityType::ExtraFuel:
        case Entity::EntityType::FuelFull:
        case Entity::EntityType::DoubleFuel:
        case Entity::EntityType::ExtraAmmo:
        case Entity::EntityType::AmmoFull:
        case Entity::EntityType::DoubleAmmo:
        case Entity::EntityType::ExtraShield:
        case Entity::EntityType::ShieldFull:
        case Entity::EntityType::DoubleShield:
        case Entity::EntityType::BoosterUpgrade:
        case Entity::EntityType::MissileUpgrade:
        case Entity::EntityType::MinigunUpgrade:
        case Entity::EntityType::UnknownShieldItem: {
           irr::u16 nrSprite = GetCollectableSpriteNumber(mEntityType);
           return (mTexLoader->spriteTex.at(nrSprite));
        }

        case Entity::EntityType::RecoveryTruck: {
           //If Render to Target Texture was not available
           //return empty image
           if (!mInfra->mBlockPreviewEnabled)
               return mTexImageEmpty;

           return mTexImageRecoveryVehicle;
        }

        case Entity::EntityType::TriggerCraft: {
            //If Render to Target Texture was not available
            //return empty image
            if (!mInfra->mBlockPreviewEnabled)
                return mTexImageEmpty;

            return mTexImageRaceVehicle;
        }

        case Entity::EntityType::TriggerRocket: {
            return (mTexLoader->spriteTex.at(39));
        }

        case Entity::EntityType::MorphOnce:
        case Entity::EntityType::MorphPermanent:
        case Entity::EntityType::MorphSource1:
        case Entity::EntityType::MorphSource2:
        {
            return (mTexLoader->spriteTex.at(42));
        }

        case Entity::EntityType::Checkpoint: {
            return (mTexLoader->levelTex.at(121));
        }

        case Entity::EntityType::Cone: {
            //If Render to Target Texture was not available
            //return empty image
            if (!mInfra->mBlockPreviewEnabled)
                return mTexImageEmpty;

           return mTexImageCone;
        }

        case Entity::EntityType::Camera: {
           return mTexLoader->editorTex.at(0);
        }

        case Entity::EntityType::TriggerTimed: {
           return mTexLoader->editorTex.at(1);
        }

        case Entity::EntityType::Explosion: {
           //for explosion use ingame sprite number 4
           return mTexLoader->spriteTex.at(4);
        }

        default: {
           return mTexImageEmpty;
        }
    }
}

//returns filename of sprite file for collectable
//invalid entity types will revert to sprite number 42
irr::u16 EntityManager::GetCollectableSpriteNumber(Entity::EntityType mEntityType) {
    irr::u16 nrSprite;

    switch (mEntityType) {
        case Entity::EntityType::ExtraFuel:
        {
            nrSprite = 29;
            break;
        }

        case Entity::EntityType::FuelFull:
        {
            nrSprite = 30;
            break;
        }
        case Entity::EntityType::DoubleFuel:
        {
            nrSprite = 31;
            break;
        }

        case Entity::EntityType::ExtraAmmo:
        {
           nrSprite = 32;
           break;
        }
        case Entity::EntityType::AmmoFull:
            {
               nrSprite = 33;
               break;
            }
        case Entity::EntityType::DoubleAmmo:
            {
               nrSprite = 34;
               break;
            }

        case Entity::EntityType::ExtraShield:
            {
               nrSprite = 35;
               break;
            }
        case Entity::EntityType::ShieldFull:
            {
               nrSprite = 36;
               break;
            }
        case Entity::EntityType::DoubleShield:
            {
               nrSprite = 37;
               break;
            }

        case Entity::EntityType::BoosterUpgrade:
            {
               nrSprite = 40;
               break;
            }
        case Entity::EntityType::MissileUpgrade:
            {
              nrSprite = 39;
              break;
            }
        case Entity::EntityType::MinigunUpgrade:
            {
              nrSprite = 38;
              break;
            }

        case Entity::EntityType::UnknownShieldItem:
                {
                  nrSprite = 41;
                  break;
                }

        default: {
            nrSprite = 42;
            break;
        }
    }

  return nrSprite;
}

std::string EntityManager::GetNameForEntityType(Entity::EntityType mEntityType) {
     std::string name("");

     switch (mEntityType) {
        case Entity::EntityType::WaypointAmmo: {
           name.append("WaypointAmmo");
           return name;
        }

         case Entity::EntityType::WaypointFuel: {
            name.append("WaypointFuel");
            return name;
         }

         case Entity::EntityType::WaypointShield: {
            name.append("WaypointShield");
            return name;
         }

         case Entity::EntityType::WaypointShortcut: {
            name.append("WaypointShortcut");
            return name;
         }

         case Entity::EntityType::WaypointSpecial1: {
            name.append("WaypointSpecial1");
            return name;
         }

         case Entity::EntityType::WaypointSpecial2: {
            name.append("WaypointSpecial2");
            return name;
         }

         case Entity::EntityType::WaypointSpecial3: {
            name.append("WaypointSpecial3");
            return name;
         }

         case Entity::EntityType::WaypointFast: {
            name.append("WaypointFast");
            return name;
         }

         case Entity::EntityType::WaypointSlow: {
            name.append("WaypointSlow");
            return name;
         }

         case Entity::EntityType::WallSegment: {
            name.append("WallSegment");
            return name;
         }

         case Entity::EntityType::TriggerCraft: {
            name.append("TriggerCraft");
            return name;
         }

         case Entity::EntityType::TriggerRocket: {
            name.append("TriggerRocket");
            return name;
         }

         case Entity::EntityType::TriggerTimed: {
            name.append("TriggerTimed");
            return name;
         }

         case Entity::EntityType::MorphOnce: {
            name.append("MorphOnce");
            return name;
         }

         case Entity::EntityType::MorphPermanent: {
            name.append("MorphPermanent");
            return name;
         }

         case Entity::EntityType::MorphSource1: {
            name.append("MorphSource1");
            return name;
         }

         case Entity::EntityType::MorphSource2: {
            name.append("MorphSource2");
            return name;
         }

         case Entity::EntityType::Cone: {
            name.append("Cone");
            return name;
         }

         case Entity::EntityType::RecoveryTruck: {
            name.append("RecoveryTruck");
            return name;
         }

         case Entity::EntityType::Checkpoint: {
            name.append("Checkpoint");
            return name;
         }

         case Entity::EntityType::Camera: {
            name.append("Camera");
            return name;
         }

         case Entity::EntityType::Explosion: {
            name.append("Explosion");
            return name;
         }

         //this are default collectable items from
         //the map files
         case Entity::EntityType::ExtraFuel: {
            name.append("ExtraFuel");
            return name;
         }

         case Entity::EntityType::FuelFull: {
            name.append("FuelFull");
            return name;
         }

         case Entity::EntityType::DoubleFuel: {
            name.append("DoubleFuel");
            return name;
         }

         case Entity::EntityType::ExtraAmmo: {
            name.append("ExtraAmmo");
            return name;
         }

         case Entity::EntityType::AmmoFull: {
            name.append("AmmoFull");
            return name;
         }

         case Entity::EntityType::DoubleAmmo: {
            name.append("DoubleAmmo");
            return name;
         }

         case Entity::EntityType::ExtraShield: {
            name.append("ExtraShield");
            return name;
         }

         case Entity::EntityType::ShieldFull: {
            name.append("ShieldFull");
            return name;
         }

         case Entity::EntityType::DoubleShield: {
            name.append("DoubleShield");
            return name;
         }

         case Entity::EntityType::BoosterUpgrade: {
            name.append("BoosterUpgrade");
            return name;
         }

         case Entity::EntityType::MissileUpgrade: {
            name.append("MissileUpgrade");
            return name;
         }

         case Entity::EntityType::MinigunUpgrade: {
            name.append("MinigunUpgrade");
            return name;
         }

         case Entity::EntityType::UnknownShieldItem: {
            name.append("UnknownShieldItem");
            return name;
         }

         case Entity::EntityType::UnknownItem: {
            name.append("UnknownItem");
            return name;
         }

         case Entity::EntityType::Unknown: {
            name.append("Unknown");
            return name;
         }

         case Entity::EntityType::SteamStrong: {
            name.append("SteamStrong");
            return name;
         }

         case Entity::EntityType::SteamLight:  {
            name.append("SteamLight");
            return name;
         }

         default: {
            //simply return empty string
            return name;
         }
     }
}

void EntityManager::CreateModelPreview(char* modelFileName, irr::core::vector3df objPos,
                                       irr::core::vector3df camPos, irr::core::vector3df camLookAtPos,
                                       irr::video::ITexture& outputTexture) {
     if (!mInfra->mBlockPreviewEnabled)
         return;

     //first we need to create the model SceneNode
     irr::scene::IAnimatedMesh*  newMesh = mInfra->mSmgr->getMesh(modelFileName);
     irr::scene::IMeshSceneNode* newSceneNode = mInfra->mSmgr->addMeshSceneNode(newMesh);

     newSceneNode->setScale(irr::core::vector3d<irr::f32>(1,1,1));
     newSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
     newSceneNode->setPosition(objPos);

     irr::scene::ICameraSceneNode* newCamera = mInfra->mSmgr->addCameraSceneNode(0, camPos, camLookAtPos, -1, false);
     newCamera->setAspectRatio(1.0f);

     mInfra->mDriver->beginScene(false,false, 0);

     //store the current camera selection
     ICameraSceneNode* currCamera = mInfra->mSmgr->getActiveCamera();

     // draw scene into render target
     // set render target texture
     this->mInfra->mDriver->setRenderTarget(mRenderToTargetTex, true, true, video::SColor(0,0,0,255));

     //set preview camera as active camera
     this->mInfra->mSmgr->setActiveCamera(newCamera);

     // draw whole scene into render buffer
     mInfra->mSmgr->drawAll();

     //copy the preview picture to the specified output texture
     mInfra->CopyTexture(mRenderToTargetTex, &outputTexture);

     // set back old render target
     mInfra->mDriver->setRenderTarget(0, false, false, 0);

     //restore initial camera
     if (currCamera != nullptr) {
        this->mInfra->mSmgr->setActiveCamera(currCamera);
     }

     mInfra->mDriver->endScene();

     //Remove now unnecessary objects again
     newSceneNode->remove();
     newCamera->remove();
}

void EntityManager::CreateModelPictures() {
    //Define image size for model images
    //set to 100x100 pixels
    mRenderToTargetTexImageSize.set(100, 100);

    //create an empty texture/image in case feature render to target texture is not available
    mTexImageEmpty = mInfra->mDriver->addTexture(mRenderToTargetTexImageSize, "ModelPreviewEmpty");

    //Fill this new (unitialized) texture with random pixel colors with all 0 values (black)
    mInfra->FillTexture(mTexImageEmpty, 0);

    if (!mInfra->mBlockPreviewEnabled)
        return;

    //create the special "render to texture" texture
    mRenderToTargetTex = mInfra->mDriver->addRenderTargetTexture(mRenderToTargetTexImageSize, "RTT1");

    mTexImageRecoveryVehicle = mInfra->mDriver->addTexture(mRenderToTargetTexImageSize, "ModelPreviewRecoveryVehicle");
    mTexImageRaceVehicle = mInfra->mDriver->addTexture(mRenderToTargetTexImageSize, "ModelPreviewRaceVehicle");
    mTexImageCone = mInfra->mDriver->addTexture(mRenderToTargetTexImageSize, "ModelPreviewCone");

    //Create image of recovery vehicle
    CreateModelPreview((char*)("extract/models/recov0-0.obj"), irr::core::vector3df(40.0f, -50.0f, 0.0f),
                     irr::core::vector3df(44.325f, -51.7125f, 4.925f), irr::core::vector3df(40.5f, -48.5f, 0.0f), *mTexImageRecoveryVehicle);

    //Create image of a race vehicle
    CreateModelPreview((char*)("extract/models/bike0-0.obj"), irr::core::vector3df(40.5f, -48.5f, 0.0f),
        irr::core::vector3df(40.1f, -48.1f, -0.7f), irr::core::vector3df(40.5f, -48.5f, 0.0f), *mTexImageRaceVehicle);

    //Create image of cone
    CreateModelPreview((char*)("extract/models/cone0-0.obj"), irr::core::vector3df(40.0f, -50.0f, 0.0f),
                     irr::core::vector3df(40.4f, -49.6f, 0.4f), irr::core::vector3df(40.0f, -49.9f, 0.0f), *mTexImageCone);
}

bool EntityManager::AreModelImagesAvailable() {
    return (mInfra->mBlockPreviewEnabled);
}

void EntityManager::DebugWriteEntityTableToCsvFile(const char* debugOutPutFileName) {
   FILE* debugOutputFile = nullptr;

   debugOutputFile = fopen(debugOutPutFileName, "w");
   if (debugOutputFile == nullptr) {
         return;
   }

   std::vector<EntityItem*>::iterator it;

   //write a header
   fprintf(debugOutputFile, "Id;CellX;CellY;DX;DY;Description;Type;SubType;Group;TargetGroup;Link;Value;Unknown1;Unknown2;Unknown3;Offset\n");

   irr::s32 offsetX;
   irr::s32 offsetY;
   std::string description("");
   Entity::EntityType type;

   for (it = mLevelTerrain->levelRes->Entities.begin(); it != mLevelTerrain->levelRes->Entities.end(); ++it) {
        offsetX = (irr::s32)((*it)->getOffsetX());
        offsetY = (irr::s32)((*it)->getOffsetY());

        type = (*it)->getEntityType();

        description.clear();
        description.append(GetNameForEntityType(type));

        //write the next entry
        fprintf(debugOutputFile, "%d;%d;%d;%d;%d;%s;%u;%u;%d;%d;%d;%d;%d;%d;%d;%d\n",
                (*it)->get_ID(), (*it)->getCell().X, (*it)->getCell().Y, offsetX, offsetY,
                description.c_str(),  (*it)->getRawType(), (*it)->getRawSubType(), (*it)->getGroup(),
                (*it)->getTargetGroup(), (*it)->getNextID(), (*it)->getValue(), (*it)->getUnknown1(),
                (*it)->getUnknown2(), (*it)->getUnknown3(), (*it)->get_Offset());
   }

   fclose(debugOutputFile);
}

//Returns true if there is currently an entity item at the specified cell
//coordinates. Pointer to existing item is also returned via reference parameter returnPntr
//Returns false if there is no Entity item right now
bool EntityManager::IsEntityItemAtCellCoord(int x, int y, EditorEntity **returnPntr) {
    std::vector<EditorEntity*>::iterator it;
    irr::core::vector2di whichCell;

    for (it = mEntityVec.begin(); it != mEntityVec.end(); ++it) {
        whichCell = (*it)->GetCellCoord();

        if ((whichCell.X == x) && (whichCell.Y == y)) {
            //found an item there
            //return the pointer to the item via parameters
            *returnPntr = (*it);
            return true;
        }
    }

    //no entity found
    return false;
}

//returns true if move operation was succesfull, false otherwise
bool EntityManager::MoveEntityToCell(EditorEntity* itemToMove, int targetCellX, int targetCellY) {
    if (itemToMove == nullptr)
        return false;

    EditorEntity* existingEntity = nullptr;

    //Is there already an entity Item at the target location?
    //if so simply exit
    if (IsEntityItemAtCellCoord(targetCellX, targetCellY, &existingEntity)) {
        return false;
    }

    //change low level map data position
    irr::core::vector3df newCenterPos;
    irr::core::vector2di outCell;

    newCenterPos.X = - (irr::f32)(targetCellX) * DEF_SEGMENTSIZE - 0.5f;
    newCenterPos.Z = (irr::f32)(targetCellY) * DEF_SEGMENTSIZE + 0.5f;
    newCenterPos.Y = mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(newCenterPos.X, newCenterPos.Z, outCell);

    itemToMove->mEntityItem->setCenter(newCenterPos);

    //update higher level model position
    //as well
    itemToMove->UpdatePosition();

    //for certain types of entities we need to further adjust
    //scenenode position
    Entity::EntityType type = itemToMove->mEntityItem->getEntityType();

    switch (type) {
        case Entity::EntityType::WaypointAmmo:
        case Entity::EntityType::WaypointFuel:
        case Entity::EntityType::WaypointShield:
        case Entity::EntityType::WaypointFast:
        case Entity::EntityType::WaypointShortcut:
        case Entity::EntityType::WaypointSpecial1:
        case Entity::EntityType::WaypointSpecial2:
        case Entity::EntityType::WaypointSpecial3:
        case Entity::EntityType::WaypointSlow:
        case Entity::EntityType::WallSegment: {
            //modify cube position, so that it is not
            //stuck in the terrain tiles
            itemToMove->SetNewHeight(itemToMove->GetCurrentHeight() + EntityManagerCubeHeightDistance);

            break;
        }

        case Entity::EntityType::RecoveryTruck: {
            itemToMove->SetNewHeight(itemToMove->GetCurrentHeight() + EntityManagerRecoveryVehicleFlyingHeight);

            break;
        }

        case Entity::EntityType::SteamStrong:
        case Entity::EntityType::SteamLight:
        {
            //modify surrounding box position, so that it is not
            //stuck in the terrain tiles
            itemToMove->SetNewHeight(itemToMove->GetCurrentHeight() + EntityManagerSteamFountainBoxHeightDistance);

            break;
        }

        default: {
            break;
        }
    }

    return true;
}

void EntityManager::AddEntityAtCell(int x, int y, Entity::EntityType type) {
  EditorEntity* itemFound = nullptr;

  //if there is already an entity at this cell quit again
  if (IsEntityItemAtCellCoord(x, y, &itemFound)) {
      return;
  }

  irr::u32 outIndex;

  //what is the correct height for the new EntityItem
  irr::f32 height = -mLevelTerrain->GetHeightInterpolated((irr::f32)(x), (irr::f32)(y));

  if (!mLevelFile->AddEntityAtCell(x, y, height, type, outIndex)) {
       logging::Error("Failed to create new EntityItem");
       return;
  }

  std::string infoMsg("");
  char hlpstr[100];

  infoMsg.clear();
  infoMsg.append("Added new entity item at cell X = ");

  //add X
  sprintf(hlpstr, "%d", x);
  infoMsg.append(hlpstr);

  infoMsg.append(", Y = ");

  //add Y
  sprintf(hlpstr, "%d", y);
  infoMsg.append(hlpstr);

  infoMsg.append(", with Id = ");

  EntityItem* newItem = mLevelFile->Entities.at(outIndex);

  //add Id
  sprintf(hlpstr, "%d", newItem->get_ID());
  infoMsg.append(hlpstr);

  logging::Info(infoMsg);

  //if we create an entity that uses OffsetX and OffsetY we need to set initial
  //values > 0, so that the new entity can be seen
  if ((type == Entity::TriggerCraft) || (type == Entity::TriggerRocket)) {
      newItem->setOffsetX(1.0f);
      newItem->setOffsetY(1.0f);
  }

  if (type == Entity::Checkpoint) {
      newItem->setOffsetX(1.0f);
  }

  //Create also the higher level data for this
  //Entity
  CreateEntity(newItem);
}

void EntityManager::CleanupCustomMesh(EditorEntity* whichEntity) {
    std::vector < std::pair<EditorEntity*, irr::scene::IMesh*>>::iterator it;
    irr::scene::IMesh* meshPntr = nullptr;

    for (it = mSelectionMeshVec.begin(); it != mSelectionMeshVec.end();) {
        if ((*it).first == whichEntity) {
            meshPntr = (*it).second;

            it = mSelectionMeshVec.erase(it);

            //get rid of the custom mesh from the Meshcache
            mInfra->mSmgr->getMeshCache()->removeMesh(meshPntr);
        }
        else {
            it++;
        }
    }
}

void EntityManager::CleanupAllCustomMesh() {
    std::vector < std::pair<EditorEntity*, irr::scene::IMesh*>>::iterator it;
    irr::scene::IMesh* meshPntr = nullptr;

    for (it = mSelectionMeshVec.begin(); it != mSelectionMeshVec.end(); ) {
           meshPntr = (*it).second;

           it = mSelectionMeshVec.erase(it);

           //get rid of the custom mesh from the Meshcache
           mInfra->mSmgr->getMeshCache()->removeMesh(meshPntr);
    }
}

void EntityManager::UpdateCustomMesh(EditorEntity* whichEntity) {
    std::vector < std::pair<EditorEntity*, irr::scene::IMesh*>>::iterator it;
    irr::scene::IMesh* meshPntr = nullptr;

    for (it = mSelectionMeshVec.begin(); it != mSelectionMeshVec.end(); ++it) {
        if ((*it).first == whichEntity) {
            meshPntr = (*it).second;
            break;
        }
    }

    if (meshPntr != nullptr) {
        irr::scene::IMeshManipulator* meshManipulator = mInfra->mDriver->getMeshManipulator();

        irr::core::vector3df currMeshSize = meshPntr->getBoundingBox().getExtent();

        irr::core::vector3df scaleVal
            (whichEntity->mEntityItem->getOffsetX() / currMeshSize.X, 
             1.0f,
             whichEntity->mEntityItem->getOffsetY() / currMeshSize.Z);

        //Some TriggerCraft and TriggerRocket entities have an OffsetX and OffsetY value of 0
        //in this case make sure that a minimum Size of the Mesh is guaranteed, so that we see something
        if (scaleVal.X < 0.01f) {
            scaleVal.X = 0.05f;
        }

        if (scaleVal.Y < 0.01f) {
            scaleVal.Y = 0.05f;
        }

        if (scaleVal.Z < 0.01f) {
            scaleVal.Z = 0.05f;
        }

        meshManipulator->scale(meshPntr, scaleVal);

        whichEntity->UpdateBoundingBox();
    }
}

void EntityManager::RemoveEntity(EditorEntity* removeItem) {
    if (removeItem == nullptr)
        return;

    //This higher level function has to do 2 independent things:
    // 1, Remove the Irrlicht SceneNode which displays this EditorEntity item.
    //    So that it disappears for the user in the level editor
    // 2, Remove the entity from the level/map file EntityTable itself.
    //    So that if the user does load the map again, everything
    //    is restored again in the same modified way

    irr::core::vector2di cellCoord = removeItem->GetCellCoord();

    std::string infoMsg("");
    char hlpstr[100];

    infoMsg.clear();
    infoMsg.append("Remove entity item at cell X = ");

    //add X
    sprintf(hlpstr, "%d", cellCoord.X);
    infoMsg.append(hlpstr);

    infoMsg.append(", Y = ");

    //add Y
    sprintf(hlpstr, "%d", cellCoord.Y);
    infoMsg.append(hlpstr);

    infoMsg.append(", with Id = ");

    EntityItem* entityItem = removeItem->mEntityItem;

    //add Id
    sprintf(hlpstr, "%d", entityItem->get_ID());
    infoMsg.append(hlpstr);

    logging::Info(infoMsg);

    //DebugWriteEntityTableToCsvFile((char*)("EntityTableBefore.csv"));

    /******************************************************************
     * Part 1: Remove the higher level EntityItem data                *
     ******************************************************************/

    //if this was a special EditorEntity for SteamFountain remove this Item
    //also from my internal vector of SteamFoutains we need to update
    if ((removeItem->GetEntityType() == Entity::SteamStrong) ||
        (removeItem->GetEntityType() == Entity::SteamLight)) {
        std::vector<EditorEntity*>::iterator itSteam;

        for (itSteam = mSteamFountainVec.begin(); itSteam != mSteamFountainVec.end();) {
            if ((*itSteam) == removeItem) {
                itSteam = mSteamFountainVec.erase(itSteam);
            }
            else {
                itSteam++;
            }
        }
    }

    //Remove from the list of EditorEntity items
    std::vector<EditorEntity*>::iterator it;
    EditorEntity* pntr;

    for (it = mEntityVec.begin(); it != mEntityVec.end();) {
        if ((*it) == removeItem) {
            //delete this entry
            pntr = (*it);
            it = mEntityVec.erase(it);

            //delete the EditorEntity item
            delete pntr;
        } else {
            //go to next item
            it++;
        }
    }

    //if this entity has a custom Mesh make sure to clean it up too
    CleanupCustomMesh(removeItem);

    /******************************************************************
     * Part 2: Modify low level map data                              *
     ******************************************************************/

    //set all EntityItem LevelEditor states to default state
    std::vector<EntityItem*>::iterator itEntityItem;

    for (itEntityItem = this->mLevelFile->Entities.begin(); itEntityItem != this->mLevelFile->Entities.end(); ++itEntityItem) {
       (*itEntityItem)->mState = DEF_ENTITYITEM_STATE_DEFAULT;
    }

    //now we also want to remove the low level EntityItem from the
    //levelfile
    //this item we want to remove from the table
    //mark this wish with this state variable
    entityItem->mState = DEF_ENTITYITEM_STATE_NEWLYUNASSIGNEDONE;

    //Finally delete the marked EntityItem
    RemoveUnusedEntityTableEntries();

    //restore all links between EntityItems again
    //fixes are necessary because we changed the table
    //and Id numbers of the table entries
    RestoreEntityItemLinks();

    //restore default state again
    for (itEntityItem = this->mLevelFile->Entities.begin(); itEntityItem != this->mLevelFile->Entities.end(); ++itEntityItem) {
       (*itEntityItem)->mState = DEF_ENTITYITEM_STATE_DEFAULT;
    }

   //DebugWriteEntityTableToCsvFile((char*)("EntityTableAfter.csv"));
}

void EntityManager::RemoveUnusedEntityTableEntries() {
    std::string infoMsg("");
    char hlpstr[100];
    std::vector<EntityItem*>::iterator itEntity;
    EntityItem* toDelete;

    for (itEntity = mLevelFile->Entities.begin(); itEntity != mLevelFile->Entities.end(); ++itEntity) {
        //for easier debugging etc. keep EntityItem original
        //Id for every EntityItem before we modify it possibly
        (*itEntity)->m_initialID = (*itEntity)->get_ID();
    }

    //remove each entry marked for deletion
    //we know this based on the EntityItem internal state variable
    for (itEntity = mLevelFile->Entities.begin(); itEntity != mLevelFile->Entities.end(); ) {
        if ((*itEntity)->mState == DEF_ENTITYITEM_STATE_NEWLYUNASSIGNEDONE) {
            //an entry we need to remove, remove it

            toDelete = (*itEntity);

            infoMsg.clear();
            infoMsg.append("Removing EntityItem with Id = ");

            //add id
            sprintf(hlpstr, "%d", (*itEntity)->get_ID());

            infoMsg.append(hlpstr);
            logging::Info(infoMsg);

            //remove EntityItem vector entry
            itEntity = mLevelFile->Entities.erase(itEntity);

            //delete the EntityItem itself
            delete toDelete;
        } else {
            itEntity++;
        }
     }

    irr::u32 currId = 1;
    int baseOffset;

    //now all the to be removed EntityItems are removed, now it is important to reassign the remaining EntityItem
    //Ids in the table, so that all Ids stays in order, increasing one by one for each element
    for (itEntity = mLevelFile->Entities.begin(); itEntity != mLevelFile->Entities.end(); ++itEntity) {
            //set new Id for the next EntityItem
            (*itEntity)->set_ID(currId);

            //do not forget to also update Offset in file which depends
            //on new Id!
            baseOffset = 0 + currId * 24;  //entity item table actually starts at index 0

            (*itEntity)->set_Offset(baseOffset);

            currId++;
    }
}

void EntityManager::RestoreEntityItemLinks() {
    //loop through all currently existing EntityItems
    std::vector<EntityItem*>::iterator itSrc;
    std::vector<EntityItem*>::iterator itTarget;

    int oldID;
    int newID;

    for (itSrc = mLevelFile->Entities.begin(); itSrc != mLevelFile->Entities.end(); ++itSrc) {
        //If Id of this item has not changed, go to the next item
        if ((*itSrc)->m_initialID == (*itSrc)->get_ID())
            continue;

        oldID = (*itSrc)->m_initialID;
        newID = (*itSrc)->get_ID();

        //the Id has changed, now check if this item was linked from all other items over its old Id in variable NextId
        for (itTarget = mLevelFile->Entities.begin(); itTarget != mLevelFile->Entities.end(); ++itTarget) {
            //only update NextId one time!
             if (((*itTarget)->getNextID() == oldID) && (*itTarget)->mState != DEF_ENTITYITEM_STATE_LINKUPDATED) {
                 //there was a link
                 //correct NextId value with new Id of Src
                 (*itTarget)->setNextID(newID);

                 //only update NextId one time!
                 (*itTarget)->mState = DEF_ENTITYITEM_STATE_DEFAULT;
             }
        }
    }
}

void EntityManager::ChangeEntitiyGroup(EditorEntity* editorEntity, int16_t newGroupValue) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setGroup(newGroupValue);
}

void EntityManager::ChangeEntitiyTargetGroup(EditorEntity* editorEntity, int16_t newTargetGroupValue) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setTargetGroup(newTargetGroupValue);
}

void EntityManager::ChangeEntitiyNextId(EditorEntity* editorEntity, int16_t newNextIdValue) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setNextID(newNextIdValue);
}

void EntityManager::ChangeEntitiyUnknown1(EditorEntity* editorEntity, int16_t newUnknown1Value) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setUnknown1(newUnknown1Value);

}

void EntityManager::ChangeEntitiyUnknown2(EditorEntity* editorEntity, int16_t newUnknown2Value) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setUnknown2(newUnknown2Value);
}

void EntityManager::ChangeEntitiyUnknown3(EditorEntity* editorEntity, int16_t newUnknown3Value) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setUnknown3(newUnknown3Value);
}

void EntityManager::ChangeEntitiyValue(EditorEntity* editorEntity, int16_t newValue) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setValue(newValue);
}

void EntityManager::ChangeEntitiyOffsetX(EditorEntity* editorEntity, float newOffsetXValue) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setOffsetX(newOffsetXValue);
    UpdateCustomMesh(editorEntity);
}

void EntityManager::ChangeEntitiyOffsetY(EditorEntity* editorEntity, float newOffsetYValue) {
    if (editorEntity == nullptr)
        return;

    editorEntity->mEntityItem->setOffsetY(newOffsetYValue);
    UpdateCustomMesh(editorEntity);
}


