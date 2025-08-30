/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "irrlicht.h"
#include "../definitions.h"
#include <vector>
#include <list>
#include <cstdint>
#include "../resources/entityitem.h"
#include <string>

//preset values, also used in
//HiOctaneTools
const irr::f32 EntityManagerCollectableSize_w = 0.45f;
const irr::f32 EntityManagerCollectableSize_h = 0.45f;

const irr::f32 EntityManagerRecoveryVehicleFlyingHeight = 6.0f;

/************************
 * Forward declarations *
 ************************/

class InfrastructureBase;
class EditorEntity;
class LevelFile;
class LevelTerrain;
class LevelBlocks;
class Morph;
class TextureLoader;

class EntityManager {
private:
     LevelFile* mLevelFile = nullptr;
     LevelTerrain* mLevelTerrain = nullptr;
     LevelBlocks* mLevelBlocks = nullptr;
     TextureLoader* mTexLoader = nullptr;

     void CreateEntity(EntityItem *p_entity);

     //for an unknown/undefined entity type returns an empty string
     //in case of an error
     irr::io::path GetModelForEntityType(Entity::EntityType entityType);

     std::vector<LineStruct*> *ENTWallsegmentsLine_List = nullptr;
     std::vector<EntityItem*> *ENTWaypoints_List = nullptr;
     std::list<EntityItem*> *ENTWallsegments_List = nullptr;
     std::list<EntityItem*> *ENTTriggers_List = nullptr;
     //std::vector<Collectable*> *ENTCollectablesVec = nullptr;

     void CleanUpEntities();
     void CleanUpMorphs();

     bool DebugShowWaypoints = false;

     bool DebugShowCheckpoints = false;
     bool DebugShowWallSegments = false;

     //Render To Target texture
     //needed to create image of the recovery vehicle and cone
     irr::video::ITexture* mRenderToTargetTex = nullptr;

     irr::core::vector2d<irr::u32> mRenderToTargetTexImageSize;

     void CreateModelPictures();
     void CreateModelPreview(char* modelFileName, irr::core::vector3df objPos,
                             irr::core::vector3df camPos, irr::core::vector3df camLookAtPos,
                             irr::video::ITexture& outputTexture);

     void RemoveUnusedEntityTableEntries();
     void RestoreEntityItemLinks();

public:
    EntityManager(InfrastructureBase* infra, LevelFile* levelRes, LevelTerrain* levelTerrain, LevelBlocks* levelBlocks,
                  TextureLoader* texLoader);
    ~EntityManager();

    InfrastructureBase* mInfra = nullptr;

    irr::f32 GetCollectableCenterHeight();
    irr::core::vector2df GetCollectableSize();
    irr::u16 GetCollectableSpriteNumber(Entity::EntityType mEntityType);
    std::string GetNameForEntityType(Entity::EntityType mEntityType);
    irr::video::ITexture* GetImageForEntityType(Entity::EntityType mEntityType);

    void CreateLevelEntities();

    void AddEntityAtCell(int x, int y, Entity::EntityType type);

    //Returns true if there is currently an entity item at the specified cell
    //coordinates. Pointer to existing item is also returned via reference parameter returnPntr
    //Returns false if there is no Entity item right now
    bool IsEntityItemAtCellCoord(int x, int y, EditorEntity **returnPntr);

    //holds a list of all available level morphs
    std::list<Morph*> Morphs;

    //holds all available editor entity items
    //in the level
    std::vector<EditorEntity*> mEntityVec;

    void Draw();

    //Stores an image of a recovery vehicle
    irr::video::ITexture* mTexImageRecoveryVehicle = nullptr;

    //Stores an image of a cone
    irr::video::ITexture* mTexImageCone = nullptr;

    //Stores an empty image
    irr::video::ITexture* mTexImageEmpty = nullptr;

    bool AreModelImagesAvailable();

    void DebugWriteEntityTableToCsvFile(char* debugOutPutFileName);

    void RemoveEntity(EditorEntity* removeItem);


};

#endif // ENTITYMANAGER_H
