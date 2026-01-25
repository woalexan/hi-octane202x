/*
 Copyright (C) 2025-2026 Wolf Alexander

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
#include <string>

#define DEF_EDITOR_USERINNODIALOG 0
#define DEF_EDITOR_USERINTEXTUREDIALOG 1
#define DEF_EDITOR_USERINCOLUMNDESIGNERDIALOG 2
#define DEF_EDITOR_USERINTERRAFORMINGDIALOG 3
#define DEF_EDITOR_USERINVIEWMODEDIALOG 4
#define DEF_EDITOR_USERINENTITYMODEDIALOG 5
#define DEF_EDITOR_USERINREGIONMODEDIALOG 6

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
class ColumnDesigner;
class EditorMode;
class ViewMode;
class TerraformingMode;
class EditorEntity;
class EntityMode;
class EntityManager;
class RegionMode;
struct CurrentlySelectedEditorItemInfoStruct;
struct MapConfigStruct;
class MiniMap;

/* GUI Elements
*/
struct GUI
{
    GUI ()
    {
        memset ( this, 0, sizeof ( *this ) );
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
    irr::gui::IGUIScrollBar* Scrollbar;
};

class EditorSession {
public:
    std::string mLevelRootPath;
    std::string mLevelName;

    Editor* mParentEditor = nullptr;

    //my texture loader
    TextureLoader* mTexLoader = nullptr;

    //handles the file data structure of the
    //level
    LevelFile *mLevelRes = nullptr;

    //my camera
    irr::scene::ICameraSceneNode* mCamera = nullptr;
    irr::scene::ICameraSceneNode* mCamera2 = nullptr;

    //handles the height map terrain
    //of the level
    LevelTerrain* mLevelTerrain = nullptr;

    //handles the columns (made of blocks)
    //of the level
    LevelBlocks* mLevelBlocks = nullptr;

    EntityManager* mEntityManager = nullptr;

    ItemSelector* mItemSelector = nullptr;

    TextureMode* mTextureMode = nullptr;
    ColumnDesigner* mColumnDesigner = nullptr;
    ViewMode* mViewMode = nullptr;
    TerraformingMode* mTerraforming = nullptr;
    EntityMode* mEntityMode = nullptr;
    RegionMode* mRegionMode = nullptr;

    bool mFreeFlyMode;

    irr::s32 GetNextFreeGuiId();

    void AdvanceTime(irr::f32 frameDeltaTime);
    void ActivateMorphs();
    void DeactivateMorphs();

    //Returns true if morphing is currently
    //enabled
    bool IsMorphingRunning();

    void SetFog(bool enabled);
    void SetIllumination(bool enabled);
    bool IsIlluminationEnabled();

    void RemoveEverythingFromLevel();

    void MoveUserViewToLocation(irr::core::vector3df newCameraLookAtPnt, irr::f32 cameraDistance);

    void DrawCellVertexCross(CurrentlySelectedEditorItemInfoStruct* mSelVertex, ColorStruct* color,
                             irr::f32 reduceCrossSizeFactor = 0.0f);

    //BillboardSceneNode which contains a white arrow
    //to indicate to the user that a move operation is currently ongoing
    irr::scene::IBillboardSceneNode *mArrowRightBillSceneNode = nullptr;

    void ShowArrowPointingRightAtCell(irr::core::vector2di cellCoord);
    void HideArrowPointingRight();

    //Will ask the user if he is sure to close the
    //EditorSession and to lose unsaved changes
    //If answer is Yes, the MessageBox will create an Gui Event
    //If Cancel is choosen, then Close operation will not commence
    void TriggerClose();

private:

    bool LoadLevel();

    //start at Id = 1000;
    irr::s32 mNextFreeGuiId = 1000;

    void UpdateMorphs(irr::f32 frameDeltaTime);

    bool DebugShowWallCollisionMesh = false;
    //variables to switch different debugging functions on and off

    bool DebugShowRegionsAndPointOfInterest = false;
    bool DebugShowTriggerRegions = false;

    bool DebugShowLowLevelTriangleSelection = true;

    irr::u8 mLastUserInDialogState = DEF_EDITOR_USERINNODIALOG;

    GUI gui;

    bool mRunMorphs = false;
    irr::f32 absTimeMorph = 0.0f;

    irr::gui::IGUIStaticText* mModeInfoText = nullptr;
    irr::gui::IGUIStaticText* mControlInfoText = nullptr;
    irr::gui::IGUIStaticText* mAssignedLevelInfoText = nullptr;

    MapConfigStruct* mMapConfig = nullptr;

    MiniMap* mMiniMap = nullptr;
    bool mDrawMiniMap = true;

public:
    EditorSession(Editor* parentEditor, std::string levelRootPath, std::string levelName);
    ~EditorSession();

    void HideUIElements();
    void UnhideUIElements();

    void Init();
    void Render();
    void HandleBasicInput();

    bool GetMiniMapAvailable();

    void SetMode(EditorMode* selMode);

    void Render2D(irr::f32 frameDeltaTime);

    void ToggleMiniMapVisibility();

    //Returns true in case of success, False otherwise
    bool SaveAs(std::string levelRootPath, std::string levelName);

    void UpdateAssignedLevelInfoText();

    void TrackActiveDialog();

    EditorMode* mEditorMode = nullptr;

    irr::u8 mUserInDialogState = DEF_EDITOR_USERINNODIALOG;

    irr::core::vector2d<irr::s32> mCurrentMousePos;

    /*bool mCellSelectedByMouse = false;
    irr::core::vector2di mCellCoordSelectedByMouse;
    int mCellCoordVerticeSelectedByMouse;*/

    bool ready;
    //bool exitEditorSession = false;

    void CheckForMeshUpdate();
};

#endif // EDITORSESSION_H
