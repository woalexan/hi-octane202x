/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef EDITORSESSION_H
#define EDITORSESSION_H

#include "infrabase.h"
#include "utils/logging.h"
#include "input/input.h"
#include "utils/tprofile.h"
#include "editor.h"
#include "utils/ray.h"
#include "models/morph.h"
#include "draw/drawdebug.h"
#include "models/collectable.h"

#define DEF_EDITOR_SELITEM_NONE 0
#define DEF_EDITOR_SELITEM_CELL 1
#define DEF_EDITOR_SELITEM_BLOCK 2

struct CurrentlySelectedEditorItemInfoStruct {
    irr::u8 SelectedItemType;

    //All the variables below only contain valid information
    //if SelectedItemType is != DEF_EDITOR_SELITEM_NONE
    RayHitTriangleInfoStruct RayHitInfo;

    //more detailed information in case a terrain
    //cell is currently selected
    irr::core::vector2di mCellCoordSelected;
    int mCellCoordVerticeNrSelected;

    //more detailed information in case a column
    //of blocks is selected
    Column* mColumnSelected = nullptr;
    ColumnDefinition* mColumnDefinitionSelected = nullptr;

    int mSelBlockNrStartingFromBase;
};

//! Macro for save Dropping an Element
#define dropElement(x)	if (x) { x->remove(); x = 0; }

/* GUI Elements
*/
struct GUI
{
    GUI ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    void drop()
    {
        dropElement ( Window );
    }

    IGUICheckBox* FullScreen;

    IGUITable* ArchiveList;
    IGUIButton* testButton;

    IGUIListBox* MapList;
    IGUITreeView* SceneTree;
    IGUIWindow* Window;
};

class Logger; //Forward declaration
class Editor; //Forward declaration
class InfrastructureBase; //Forward declaration
class DrawDebug; //Forward declaration
class Collectable; //Forward declaration

class EditorSession {
private:
    InfrastructureBase* mInfra = nullptr;

    irr::u8 mLevelNrLoaded;

    Editor* mParentEditor = nullptr;

    //my camera
    scene::ICameraSceneNode* mCamera = nullptr;

    //my drawDebug object
    DrawDebug* mDrawDebug = nullptr;

    //my texture loader
    TextureLoader *mTexLoader = nullptr;

    //ray class to find intersection with Terrain Cells
    Ray* mRayTerrain = nullptr;

    //ray class to find intersection with Columns
    Ray* mRayColumns = nullptr;

    EntityItem* mCurrSelEntityItem = nullptr;

    //handles the file data structure of the
    //level
    LevelFile *mLevelRes = nullptr;

    //handles the height map terrain
    //of the level
    LevelTerrain *mLevelTerrain = nullptr;

    //handles the columns (made of blocks)
    //of the level
    LevelBlocks *mLevelBlocks = nullptr;

    bool LoadLevel();
    void createLevelEntities();

    void AddCheckPoint(EntityItem entity);
    void AddWayPoint(EntityItem *entity, EntityItem *next);

    irr::u16 GetCollectableSpriteNumber(Entity::EntityType mEntityType);

    std::vector<LineStruct*> *ENTWallsegmentsLine_List = nullptr;
    std::vector<EntityItem*> *ENTWaypoints_List = nullptr;
    std::list<EntityItem*> *ENTWallsegments_List = nullptr;
    std::list<EntityItem*> *ENTTriggers_List = nullptr;
    std::vector<Collectable*> *ENTCollectablesVec = nullptr;

    //holds a list of all available level morphs
    std::list<Morph*> Morphs;

    void createEntity(EntityItem *p_entity, LevelFile *levelRes, LevelTerrain *levelTerrain, LevelBlocks* levelBlocks, irr::video::IVideoDriver *driver);
    void CleanUpEntities();
    void CleanUpMorphs();

    void UpdateMorphs(irr::f32 frameDeltaTime);

    bool DebugShowWallCollisionMesh = false;
    //variables to switch different debugging functions on and off
    bool DebugShowWaypoints = false;

    bool DebugShowCheckpoints = false;
    bool DebugShowWallSegments = false;
    bool DebugShowRegionsAndPointOfInterest = false;
    bool DebugShowTriggerRegions = false;

    bool DebugShowLowLevelTriangleSelection = true;

    //the necessary triangle selectors for ray intersecting onto columns of blocks
    irr::scene::ITriangleSelector* triangleSelectorColumnswCollision = nullptr;
    irr::scene::ITriangleSelector* triangleSelectorColumnswoCollision = nullptr;

    //necessary triangle selector for ray intersecting onto terrain
    irr::scene::ITriangleSelector* triangleSelectorStaticTerrain = nullptr;
    irr::scene::ITriangleSelector* triangleSelectorDynamicTerrain = nullptr;

    //creates final TriangleSelectors to be able to do ray
    //intersection from user mouse pointer to level environment
    //for object selection
    void createTriangleSelectors();

    void DeriveSelectedTerrainCellInformation(RayHitTriangleInfoStruct* hitTriangleInfo);
    void DeriveSelectedBlockInformation(RayHitTriangleInfoStruct* hitTriangleInfo, RayHitTriangleInfoStruct* hitSecondClosestTriangleInfo);

    void DrawOutlineSelectedCell(irr::core::vector2di selCellCoordinate, SMaterial* color);
    void DrawOutlineSelectedColumn(Column* selColumnPntr, int nrBlockFromBase, SMaterial* color);

    void setActiveCamera(scene::ICameraSceneNode* newActive);

    GUI gui;

public:
    EditorSession(InfrastructureBase* infra, Editor* parentEditor, irr::u8 loadLevelNr);
    ~EditorSession();

    void Init();
    void Render();
    void HandleBasicInput();
    void HandleMouse();
    void End();

    void TestDialog();

    CurrentlySelectedEditorItemInfoStruct mCurrSelectedItem;

    irr::core::vector2di mCurrentMousePos;

    RayHitTriangleInfoStruct triangleMouseHit;
    RayHitTriangleInfoStruct secondTriangleMouseHit;
    bool triangleHitByMouse = false;
    bool secondTriangleHitByMouse = false;

    /*bool mCellSelectedByMouse = false;
    irr::core::vector2di mCellCoordSelectedByMouse;
    int mCellCoordVerticeSelectedByMouse;*/

    bool ready;
    bool exitEditorSession = false;
};

#endif // EDITORSESSION_H
