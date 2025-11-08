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

#define DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES 1
#define DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY 2
#define DEF_EDITOR_ENTITYMANAGER_SHOW_CONES 3
#define DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS 4
#define DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS 5
#define DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS 6
#define DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS 7
#define DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS 8
#define DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS 9

#define DEF_EDITOR_ENTITYMANAGER_STEAMFOUNTAIN_SELMESHBOXCOLOR irr::video::SColor(100, 252, 221, 145)
#define DEF_EDITOR_ENTITYMANAGER_CRAFTTRIGGER_SELMESHBOXCOLOR irr::video::SColor(100, 181, 247, 187)
#define DEF_EDITOR_ENTITYMANAGER_MISSILETRIGGER_SELMESHBOXCOLOR irr::video::SColor(100, 255, 253, 87)
#define DEF_EDITOR_ENTITYMANAGER_CHECKPOINT_SELMESHBOXCOLOR irr::video::SColor(100, 121, 185, 255)
#define DEF_EDITOR_ENTITYMANAGER_MORPHTARGET_SELMESHBOXCOLOR irr::video::SColor(100, 236, 183, 252)
#define DEF_EDITOR_ENTITYMANAGER_MORPHSOURCE_SELMESHBOXCOLOR irr::video::SColor(100, 189, 217, 235)

//preset values, also used in
//HiOctaneTools
const irr::f32 EntityManagerCollectableSize_w = 0.45f;
const irr::f32 EntityManagerCollectableSize_h = 0.45f;

const irr::f32 EntityManagerRecoveryVehicleFlyingHeight = 6.0f;
const irr::f32 EntityManagerCubeHeightDistance = 0.3f;
const irr::f32 EntityManagerSteamFountainBoxHeightDistance = 2.0f;

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
struct ColorStruct;

class EntityManager {
private:
     LevelFile* mLevelFile = nullptr;
     LevelTerrain* mLevelTerrain = nullptr;
     LevelBlocks* mLevelBlocks = nullptr;
   
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

     irr::scene::IMesh* CreateCubeMesh(irr::f32 size, ColorStruct* cubeColor);

     void CleanupCustomMesh(EditorEntity* whichEntity);
     void UpdateCustomMesh(EditorEntity* whichEntity);
     void CleanupAllCustomMesh();
     void CleanupAllCubeMesh();

     //Different defined colors for the cube Entities

     //If specified color is not available, returns a white cube
     irr::scene::IMesh* GetCubeMeshWithColor(ColorStruct* whichColor);
     void DrawWayPointLinks();
     void DrawWallSegments();

     ColorStruct* GetColorForWayPointType(Entity::EntityType whichType);

     void SetVisibleEntityType(Entity::EntityType whichType, bool visible);
    
     bool mShowSpecialEditorEntityTransparentSelectionBoxes = false;

     bool mShowCollectibles = false;
     bool mShowRecoveryVehicles = false;
     bool mShowCones = false;
     bool mShowWayPoints = false;
     bool mShowWallSegments = false;
     bool mShowTriggers = false;
     bool mShowCameras = false;
     bool mShowEffects = false;
     bool mShowMorphs = false;

     std::vector<EditorEntity*> mSteamFountainVec;

     irr::scene::IMesh* CreateSelectionMeshBox(irr::core::vector3df scaleFactors, irr::video::SColor boxColor);

     std::vector<std::pair<EditorEntity*, irr::scene::IMesh*>> mSelectionMeshVec;

public:
    EntityManager(InfrastructureBase* infra, LevelFile* levelRes, LevelTerrain* levelTerrain, LevelBlocks* levelBlocks,
                  TextureLoader* texLoader);
    ~EntityManager();

    InfrastructureBase* mInfra = nullptr;
    TextureLoader* mTexLoader = nullptr;

    void UpdateSteamFoutains(irr::f32 frameDeltaTime);

    void SetVisible(irr::u8 whichEntityClass, bool visible);
    bool IsVisible(irr::u8 whichEntityClass);

    void SetShowSpecialEditorEntityTransparentSelectionBoxes(bool visible);

    void SetFog(bool enabled);

    irr::f32 GetCollectableCenterHeight();
    irr::core::vector2df GetCollectableSize();
    irr::u16 GetCollectableSpriteNumber(Entity::EntityType mEntityType);
    std::string GetNameForEntityType(Entity::EntityType mEntityType);
    irr::video::ITexture* GetImageForEntityType(Entity::EntityType mEntityType);

    bool EntityIsWayPoint(EditorEntity* entity);
    bool EntityIsWallSegment(EditorEntity* entity);
    bool EntityIsPowerUpCollectible(EditorEntity* entity);

    void CreateLevelEntities();

    void AddEntityAtCell(int x, int y, Entity::EntityType type);
    void RemoveEntity(EditorEntity* removeItem);
    void UnlinkEntity(EditorEntity* whichItem);
    void LinkEntity(EditorEntity* whichItem, EditorEntity* nextItem);

    //returns true if move operation was succesfull, false otherwise
    bool MoveEntityToCell(EditorEntity* itemToMove, int targetCellX, int targetCellY);

    void ChangeEntitiyGroup(EditorEntity* editorEntity, int16_t newGroupValue);
    void ChangeEntitiyTargetGroup(EditorEntity* editorEntity, int16_t newTargetGroupValue);
    void ChangeEntitiyNextId(EditorEntity* editorEntity, int16_t newNextIdValue);

    void ChangeEntitiyUnknown1(EditorEntity* editorEntity, int16_t newUnknown1Value);
    void ChangeEntitiyUnknown2(EditorEntity* editorEntity, int16_t newUnknown2Value);
    void ChangeEntitiyUnknown3(EditorEntity* editorEntity, int16_t newUnknown3Value);

    void ChangeEntitiyValue(EditorEntity* editorEntity, int16_t newValue);

    void ChangeEntitiyOffsetX(EditorEntity* editorEntity, float newOffsetXValue);
    void ChangeEntitiyOffsetY(EditorEntity* editorEntity, float newOffsetYValue);

    //Returns true if there is currently an entity item at the specified cell
    //coordinates. Pointer to existing item is also returned via reference parameter returnPntr
    //Returns false if there is no Entity item right now
    bool IsEntityItemAtCellCoord(int x, int y, EditorEntity **returnPntr);

    //returns nullpntr in case EditorEntity with the specified ID
    //is not found
    EditorEntity* GetEditorEntityWithId(int16_t whichId);

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

    //Sotres an image of a race vehicle
    irr::video::ITexture* mTexImageRaceVehicle = nullptr;

    //Stores an empty image
    irr::video::ITexture* mTexImageEmpty = nullptr;

    //a simple Cube Mesh for Waypoint and Wallsegment level Editor
    //entity items
    std::vector<std::pair<ColorStruct*, irr::scene::IMesh*>> mCubeMeshVec;

    //a simple box Mesh for selection of SteamFountains
    irr::scene::IMesh* mSteamFountainMesh;

    bool AreModelImagesAvailable();

    void DebugWriteEntityTableToCsvFile(char* debugOutPutFileName);
};

#endif // ENTITYMANAGER_H
