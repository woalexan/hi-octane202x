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
    GUI_ID_DIALOG_ROOT_WINDOW  = 0x10000,

    GUI_ID_OPEN_LEVEL,
    GUI_ID_QUIT
};

/************************
 * Forward declarations *
 ************************/

class Logger;
class EditorSession;

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

    //special images for the game
    irr::video::ITexture* gameTitle = nullptr;
    irr::core::vector2di gameTitleDrawPos;
    irr::core::dimension2d<irr::u32> gameTitleSize;

    irr::video::ITexture* raceLoadingScr = nullptr;
    irr::core::vector2di raceLoadingScrDrawPos;
    irr::core::dimension2d<irr::u32> raceLoadingScrSize;

public:
    irr::video::ITexture* backgnd = nullptr;

    //overwrite HandleGuiEvent method for Editor
    virtual void HandleGuiEvent(const irr::SEvent& event);

    bool enableLightning = false;
    bool enableShadows = false;
    bool fullscreen = false;
    bool DebugShowVariableBoxes = true;

    //Returns true for success, false for error occured
    bool InitEditorStep1();
    bool InitEditorStep2();

    void RunEditor();

    Editor();
    ~Editor();
};

#endif // EDITOR_H
