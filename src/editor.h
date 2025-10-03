/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef EDITOR_H
#define EDITOR_H

#include "irrlicht.h"
#include "infrabase.h"
#include <vector>

using namespace irr;
using namespace gui;

#define DEF_EDITORSTATE_AFTERINIT 0
#define DEF_EDITORSTATE_EXTRACTDATA 1
#define DEF_EDITORSTATE_LOADDATA 2
#define DEF_EDITORSTATE_GAMETITLE 3
#define DEF_EDITORSTATE_SESSIONACTIVE 4
#define DEF_EDITORSTATE_ERROR 5

// Values used to identify individual GUI elements
enum
{
    GUI_ID_DIALOG_ROOT_WINDOW  = 0x20000,

    GUI_ID_NEWEMPTYLEVEL,
    GUI_ID_OPEN_LEVEL,
    GUI_ID_SAVE_LEVEL,
    GUI_ID_QUIT,

    GUI_ID_MODE_VIEW,
    GUI_ID_MODE_TEXTURING,
    GUI_ID_MODE_COLUMNDESIGN,
    GUI_ID_MODE_TERRAFORMING,
    GUI_ID_MODE_ENTITYMODE,
    GUI_ID_MODE_REGION,

    GUI_ID_VIEWMODE_TERRAIN,
    GUI_ID_VIEW_TERRAIN_OFF,
    GUI_ID_VIEW_TERRAIN_WIREFRAME,
    GUI_ID_VIEW_TERRAIN_DEFAULT,
    GUI_ID_VIEW_TERRAIN_NORMALS,

    GUI_ID_VIEWMODE_BLOCKS,
    GUI_ID_VIEW_BLOCKS_OFF,
    GUI_ID_VIEW_BLOCKS_WIREFRAME,
    GUI_ID_VIEW_BLOCKS_DEFAULT,
    GUI_ID_VIEW_BLOCKS_NORMALS,

    GUI_ID_VIEW_ENTITY_COLLECTIBLES,
    GUI_ID_VIEW_ENTITY_RECOVERY,
    GUI_ID_VIEW_ENTITY_CONES,
    GUI_ID_VIEW_ENTITY_WAYPOINTS,
    GUI_ID_VIEW_ENTITY_WALLSEGMENTS,
    GUI_ID_VIEW_ENTITY_TRIGGERS,
    GUI_ID_VIEW_ENTITY_CAMERAS,
    GUI_ID_VIEW_ENTITY_EFFECTS,
    GUI_ID_VIEW_ENTITY_MORPHS,

    GUI_ID_VIEWMODEWINDOW_RUNMORPH_CHECKBOX,
    GUI_ID_VIEWMODEWINDOW_FOG_CHECKBOX,
    GUI_ID_VIEWMODEWINDOW_ILLUMINATION_CHECKBOX,

    GUI_ID_TEXCATEGORYCOMBOBOX,
    GUI_ID_TEXMODIFICATIONCOMBOBOX,

    GUI_ID_COLUMNDEFSELECTIONCOMBOBOX,
    GUI_ID_COLUMNDESIGNER_BUTTON_ADDCOLUMN,
    GUI_ID_COLUMNDESIGNER_BUTTON_REMOVECOLUMN,
    GUI_ID_COLUMNDESIGNER_BUTTON_REPLACECOLUMN,
    GUI_ID_COLUMNDESIGNER_BUTTON_MOVEUPCOLUMN,
    GUI_ID_COLUMNDESIGNER_BUTTON_MOVEDOWNCOLUMN,
    GUI_ID_COLUMNDESIGNER_BUTTON_ADDBLOCK,
    GUI_ID_COLUMNDESIGNER_BUTTON_REMOVEBLOCK,

    GUI_ID_TEXTUREWINDOW_BUTTONSELECTN,
    GUI_ID_TEXTUREWINDOW_BUTTONSELECTE,
    GUI_ID_TEXTUREWINDOW_BUTTONSELECTS,
    GUI_ID_TEXTUREWINDOW_BUTTONSELECTW,
    GUI_ID_TEXTUREWINDOW_BUTTONSELECTT,
    GUI_ID_TEXTUREWINDOW_BUTTONSELECTB,
    GUI_ID_TEXTUREWINDOW_BUTTONSELECTCOLUMNFLOORTEXID,

    GUI_ID_ENTITYCATEGORYCOMBOBOX,
    GUI_ID_ENTITYWINDOW_BUTTONREMOVEENTITY,
    GUI_ID_ENTITYWINDOW_BUTTONMOVEENTITY,
    GUI_ID_ENTITYWINDOW_BUTTONLINKENTITY,
    GUI_ID_ENTITYWINDOW_BUTTONUNLINKENTITY,
    GUI_ID_ENTITYMODEWINDOW_CREATEATSTART_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_GROUP_EDITBOX,
    GUI_ID_ENTITYMODEWINDOW_ENTITYTABLE,

    GUI_ID_ENTITYMODEWINDOW_LIST_COLLECTIBLES_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_WAYPOINTS_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_WALLSEGMENTS_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_RECOVERY_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_CONES_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_MORPHS_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_CAMERAS_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_TRIGGERS_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_EXPLOSIONS_CHECKBOX,
    GUI_ID_ENTITYMODEWINDOW_LIST_CHECKPOINTS_CHECKBOX,

    GUI_ID_REGIONMODEWINDOW_REGIONTABLE,
    GUI_ID_REGIONMODEWINDOW_APPLYBUTTON,
    GUI_ID_REGIONMODEWINDOW_SELECTCOORD1BUTTON,
    GUI_ID_REGIONMODEWINDOW_SELECTCOORD2BUTTON,
    GUI_ID_REGIONMODEWINDOW_TYPE_COMBOBOX,

    GUI_ID_TESTBUTTON,
    GUI_ID_SCROLLBAR,
    GUI_ID_TREE
};

/************************
 * Forward declarations *
 ************************/

class Logger;
class EditorSession;
class EditorMode;
class NumberEditBox;
class UiConversion;
class FontManager;

class Editor : public InfrastructureBase {
private:
    //Irrlicht related, for debugging of game
    IGUIStaticText* dbgTimeProfiler = nullptr;
    IGUIStaticText* dbgText = nullptr;
    IGUIStaticText* dbgText2 = nullptr;

    bool ExitEditor = false;

    //stores the current editor state
    irr::u8 mEditorState = DEF_EDITORSTATE_AFTERINIT;
    
    int lastFPS = -1;
    
    void EditorLoop();
    void EditorLoopSession(irr::f32 frameDeltaTime);
    
    EditorSession* mCurrentSession = nullptr;
    FontManager* mFontManager = nullptr;

    void RenderDataExtractionScreen();
    bool LoadBackgroundImage();
    void EditorLoopExtractData();
    //void GameLoopTitleScreenLoadData();
    //void GameLoopLoadRaceScreen();
    bool LoadAdditionalGameImages();

    bool LoadGameData();
    bool CreateNewEditorSession(int load_levelnr);

    void CreateMenue();
    void OnMenuItemSelected( IGUIContextMenu* menu );
    void OnButtonClicked(irr::s32 buttonId);
    void OnScrollbarMoved(irr::s32 scrollBarId);
    void OnElementFocused(irr::s32 elementId);
    void OnElementHovered(irr::s32 elementId);
    void OnElementLeft(irr::s32 elementId);
    void OnCheckBoxChanged(irr::s32 checkBoxId);
    void OnEditBoxEnterEvent(IGUIEditBox* editBox);
    void OnElementFocusLost(irr::s32 elementId);
    void OnTableSelected(irr::s32 elementId);

    //if function returns true the close action should be interrupted
    bool OnElementClose(irr::s32 elementId);

    void OnComboBoxChanged( IGUIComboBox* comboBox);

    void ChangeViewModeTerrain(irr::u8 newViewMode);
    void ChangeViewModeBlocks(irr::u8 newViewMode);
    void OnLeftMouseButtonDown();
    void OnLeftMouseButtonUp();

    //Routine setSkinTransparency taken from Irrlicht engine
    //"Example 009 Mesh Viewer"
    void setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin);

    //special images for the game
    irr::video::ITexture* gameTitle = nullptr;
    irr::core::vector2di gameTitleDrawPos;
    irr::core::dimension2d<irr::u32> gameTitleSize;

    irr::video::ITexture* raceLoadingScr = nullptr;
    irr::core::vector2di raceLoadingScrDrawPos;
    irr::core::dimension2d<irr::u32> raceLoadingScrSize;

    irr::gui::IGUIStaticText* StatusLine = nullptr;

    wchar_t *mCurrentStatusBarText = nullptr;

    std::vector<std::pair<irr::s32, NumberEditBox*>> mRegisteredNumberEditBoxes;

    void CheckForNumberEditBoxEvent(irr::s32 receivedGuiId);

    void ChangeEntityVisibility(IGUIContextMenu* menu);
    void UpdateEntityVisibilityMenueEntries();
    void UpdateEntityVisibilityMenueEntry(irr::u8 whichEntityClass, irr::s32 commandIdMenueEntry);

    gui::IGUIContextMenu* mMenu = nullptr;

    IGUIFont* fontAndika;

public:
    irr::video::ITexture* backgnd = nullptr;

    UiConversion* mUiConversion = nullptr;

    // We'll create a struct to record info on the mouse state
    struct SMouseState {
       core::position2di Position;
       bool LeftButtonDown;
       SMouseState() : LeftButtonDown(false) { }
    } MouseState;

    //overwrite HandleGuiEvent method for Editor
    //returns true if Gui Event should be canceled
    virtual bool HandleGuiEvent(const irr::SEvent& event);

    //overwrite HandleMouseEvent method for Editor
    virtual void HandleMouseEvent(const irr::SEvent& event);

    bool enableLightning = false;
    bool enableShadows = false;
    bool fullscreen = false;
    bool DebugShowVariableBoxes = false;

    //during development I like to disable the block preview
    //temporarily. This makes the leveleditor start some seconds faster
    bool enableBlockPreview = true;

    //Returns true for success, false for error occured
    bool InitEditorStep1();
    bool InitEditorStep2();

    void UpdateStatusbarText(const wchar_t *text);

    void RunEditor();

    void RegisterNumberEditBox(NumberEditBox* whichBox, irr::s32 boxGuiId);
    void UnregisterNumberEditBox(irr::s32 boxGuiId);

    Editor();
    ~Editor();
};

#endif // EDITOR_H
