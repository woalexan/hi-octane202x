/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef EDITORSESSION_H
#define EDITORSESSION_H

#include "irrlicht.h"
#include <vector>
#include <list>
#include "definitions.h"
#include "resources/entityitem.h"
#include "utils/ray.h"

#define DEF_EDITOR_USERINNODIALOG 0
#define DEF_EDITOR_USERINTEXTUREDIALOG 1

/************************
 * Forward declarations *
 ************************/

class Logger;
class Editor;
class DrawDebug;
class Collectable;
class Column;
class ColumnDefinition;
class TextureLoader;
class LevelBlocks;
class LevelTerrain;
class LevelFile;
class Morph;
class ItemSelector;
class TextureMode;

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

    irr::gui::IGUICheckBox* FullScreen;

    irr::gui::IGUITable* Table;
    irr::gui::IGUIButton* testButton;
    irr::gui::IGUIImage* texImage0;
    irr::gui::IGUIImage* texImage1;
    irr::gui::IGUIImage* texImage2;
    irr::gui::IGUIImage* texImage3;
    irr::gui::IGUIImage* texImage4;
    irr::gui::IGUIImage* texImage5;
    irr::gui::IGUIImage* texImage6;
    irr::gui::IGUIImage* texImage7;
    irr::gui::IGUIImage* texImage8;
    irr::gui::IGUIImage* texImage9;
    irr::gui::IGUIImage* texImage10;

    irr::gui::IGUIComboBox* texCategoryList;
    irr::gui::IGUIWindow* Window;
    irr::gui::IGUIScrollBar* Scrollbar;
};

class EditorSession {
public:
    irr::u8 mLevelNrLoaded;

    Editor* mParentEditor = nullptr;

    //my texture loader
    TextureLoader* mTexLoader = nullptr;

    EntityItem* mCurrSelEntityItem = nullptr;

    //handles the file data structure of the
    //level
    LevelFile *mLevelRes = nullptr;

    //my camera
    irr::scene::ICameraSceneNode* mCamera = nullptr;
    irr::scene::ICameraSceneNode* mCamera2 = nullptr;

    //handles the height map terrain
    //of the level
    LevelTerrain *mLevelTerrain = nullptr;

    //handles the columns (made of blocks)
    //of the level
    LevelBlocks *mLevelBlocks = nullptr;

    ItemSelector* mItemSelector = nullptr;

    TextureMode* mTextureMode = nullptr;

    irr::s32 GetNextFreeGuiId();

private:

    bool LoadLevel();
    void createLevelEntities();

    void AddCheckPoint(EntityItem entity);
    void AddWayPoint(EntityItem *entity, EntityItem *next);

    irr::u16 GetCollectableSpriteNumber(Entity::EntityType mEntityType);

    //start at Id = 1000;
    irr::s32 mNextFreeGuiId = 1000;

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

    void setActiveCamera(irr::scene::ICameraSceneNode* newActive);

    irr::u8 mLastUserInDialogState = DEF_EDITOR_USERINNODIALOG;

    GUI gui;

public:
    EditorSession(Editor* parentEditor, irr::u8 loadLevelNr);
    ~EditorSession();

    void Init();
    void Render();
    void HandleBasicInput();
    void End();

    void TestDialog();
    void TrackActiveDialog();

    irr::u8 mUserInDialogState = DEF_EDITOR_USERINNODIALOG;

    irr::core::vector2d<irr::s32> mCurrentMousePos;

    /*bool mCellSelectedByMouse = false;
    irr::core::vector2di mCellCoordSelectedByMouse;
    int mCellCoordVerticeSelectedByMouse;*/

    bool ready;
    bool exitEditorSession = false;
};

#endif // EDITORSESSION_H
