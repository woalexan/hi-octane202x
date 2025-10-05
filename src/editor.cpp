/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

//Note: Routine "setSkinTransparency" inside this Cpp file was taken from Irrlicht engine
//"Example 009 Mesh Viewer" source code file, and was not written by myself. But I do not know
//who developed this routine in the first place.

#include "editor.h"
#include "draw/gametext.h"
#include "editorsession.h"
#include "editor/editormode.h"
#include "editor/viewmode.h"
#include "utils/logging.h"
#include "utils/logger.h"
#include "utils/tprofile.h"
#include "editor/itemselector.h"
#include "models/levelterrain.h"
#include "models/levelblocks.h"
#include "editor/texturemode.h"
#include "editor/columndesign.h"
#include "editor/terraforming.h"
#include "editor/entitymode.h"
#include "input/numbereditbox.h"
#include "models/entitymanager.h"
#include "editor/regionmode.h"
#include "editor/uiconversion.h"
#include "font/font_manager.h"
#include <fstream>
#include "utils/fileutils.h"
#include <sstream>
#include <iomanip>

//Returns true if user level maps folder
//is available (or was succesfully created), False otherwise
//(In case creation failed)
bool Editor::PrepareUserMapsFolder() {
    try {
        PrepareSubDir("userdata");
        PrepareSubDir("userdata/levels");

        return true;
    }
    catch (const std::string &msg) {
        std::string msgExt("Preparation of user custom level maps folder failed");
        msgExt.append(msg);
        logging::Error(msgExt);
        return false;
    }
}

//fully initializes the remaining editor
//components
//before calling this function we need to be sure that all
//original game files were already extracted before and are
//available
bool Editor::InitEditorStep2() {
    if (DebugShowVariableBoxes) {
            //only for debugging
            dbgTimeProfiler = mGuienv->addStaticText(L"Location",
                   rect<s32>(100,150,300,200), false, true, nullptr, -1, true);

            dbgText = mGuienv->addStaticText(L"",
                   rect<s32>(100,250,300,350), false, true, nullptr, -1, true);

            /*dbgText2 = guienv->addStaticText(L"",
                   rect<s32>(350,200,450,300), false, true, nullptr, -1, true);*/
    }

    if (enableLightning) {
        mSmgr->addLightSceneNode(0, vector3df(0, 100, 100),
            video::SColorf(1.0f, 1.0f, 1.0f), 1000.0f, -1);
    }

    //set a minimum amount of light everywhere, to not have black areas
    //in the level
    if (enableLightning) {
       mSmgr->setAmbientLight(video::SColorf(0.4f, 0.4f, 0.4f));
    } else {
        //set max brightness everywhere
        mSmgr->setAmbientLight(video::SColorf(1.0f, 1.0f, 1.0f));
    }

    if (enableShadows) {
       mSmgr->setShadowColor(video::SColor(150,0,0,0));
    }

    //set Gui dialog transparency
    //between 0 and 255
    setSkinTransparency(200, mGuienv->getSkin());

    fontAndika = mFontManager->GetTtFont(mDriver, mDevice->getFileSystem(),
                     "media/andika/Andika-R.ttf", 12, true, true);

    if (fontAndika) {
         mGuienv->getSkin()->setFont(fontAndika);
    }

    noEditorSessionText = mGuienv->addStaticText(L"No EditorSession",
           rect<s32>(590,470,690,490), false, true, nullptr, -1, true);

    noEditorSessionText->setVisible(false);

    CreateMenue();

    return true;
}

//creates the most basic game infrastructure, and
//extracts basic things to be able to show a first
//graphical screen
bool Editor::InitEditorStep1() {
    dimension2d<u32> targetResolution;

    //set target screen resolution
    //targetResolution.set(640,480);
    targetResolution.set(1280,960);

    //initialize my infrastructure
    this->InfrastructureInit(targetResolution, fullscreen, enableShadows);
    if (!GetInfrastructureInitOk())
        return false;

    //load the background image we need
    //for data extraction screen rendering and
    //main menue
    if (!LoadBackgroundImage()) {
        return false;
    }

    return true;
}

void Editor::RunEditor() {
    //further game data needs to be extracted?
    if (!mPrepareData->GameDataAvailable()) {
        mEditorState = DEF_EDITORSTATE_EXTRACTDATA;
    } else {
        if (!InitEditorStep2()) {
            mEditorState = DEF_EDITORSTATE_ERROR;
        } else {
            UpdateMenueEntries();
            mEditorState = DEF_EDITORSTATE_NOSESSIONACTIVE;
        }
    }

    EditorLoop();
}

void Editor::UpdateMenueEntries() {
    //before update, remove all menue items
    //that are already existing, and refill
    //menue again with currently available items
    mMenu->removeAllItems();

    mFileMenu = nullptr;
    mEditMenu = nullptr;
    mModeMenu = nullptr;
    mViewMenu = nullptr;

    //file menue is always existing
    mMenu->addItem(L"File", -1, true, true);
    mFileMenu = mMenu->getSubMenu(0);

    //Edit/Mode and View only when currently an
    //EditSession is open
    if (mCurrentSession != nullptr) {
        mMenu->addItem(L"Edit", -1, true, true);
        mEditMenu = mMenu->getSubMenu(1);

        mMenu->addItem(L"Mode", -1, true, true);
        mModeMenu = mMenu->getSubMenu(2);

        mMenu->addItem(L"View", -1, true, true);
        mViewMenu = mMenu->getSubMenu(3);
    }

    PopulateFileMenueEntries();

    if (mCurrentSession != nullptr) {
        PopulateEditMenueEntries();
        PopulateModeMenueEntries();
        PopulateViewMenueEntries();
    }

    if (mCurrentSession == nullptr) {
        UpdateStatusbarText(L"Please create or load a map");

        noEditorSessionText->setVisible(true);
        noEditorSessionText->setEnabled(true);
    } else {
        noEditorSessionText->setVisible(false);
        noEditorSessionText->setEnabled(false);
    }
}

void Editor::PopulateFileMenueEntries() {
    /*************************************
     * Submenue File                     *
     *************************************/

    if (mFileMenu == nullptr)
        return;

    gui::IGUIContextMenu* submenu;

    if (mCurrentSession == nullptr) {
        mFileMenu->addItem(L"New empty level", GUI_ID_NEWEMPTYLEVEL, true, true);
        mFileMenu->addItem(L"Open level", GUI_ID_OPEN_LEVEL);
        mFileMenu->addItem(L"Original level", GUI_ID_ORIGINAL_LEVEL, true, true);

        submenu = mFileMenu->getSubMenu(0);
        if (submenu == nullptr)
            return;

        submenu->addItem(L"Rock", GUI_ID_NEWEMPTYLEVEL_ROCK);
        submenu->addItem(L"Vegetation", GUI_ID_NEWEMPTYLEVEL_VEGETATION);
        submenu->addItem(L"Snow", GUI_ID_NEWEMPTYLEVEL_SNOW);

        submenu = mFileMenu->getSubMenu(2);
        if (submenu == nullptr)
            return;

        submenu->addItem(L"Amazon Delta Turnpike", GUI_ID_ORIGINAL_AMAZON_DELTA_TURNPIKE);
        submenu->addItem(L"Trans-Asia Interstate", GUI_ID_ORIGINAL_TRANSASIA_INTERSTATE);
        submenu->addItem(L"Shanghai Dragon", GUI_ID_ORIGINAL_SHANGHAI_DRAGON);
        submenu->addItem(L"New Chernobyl Central", GUI_ID_ORIGINAL_NEW_CHERNOBYL_CENTRAL);
        submenu->addItem(L"Slam Canyon", GUI_ID_ORIGINAL_SLAM_CANYON);
        submenu->addItem(L"Thrak City", GUI_ID_ORIGINAL_THRAK_CITY);

        if (mExtendedGame) {
            submenu->addSeparator();
            submenu->addItem(L"Ancient Mine Town", GUI_ID_ORIGINAL_ANCIENT_MINE_TOWN);
            submenu->addItem(L"Arctic Land", GUI_ID_ORIGINAL_ARCTIC_LAND);
            submenu->addItem(L"Death Match Arena", GUI_ID_ORIGINAL_DEATH_MATCH_ARENA);
        }
    }

    if (mCurrentSession != nullptr) {
        mFileMenu->addItem(L"Save level", GUI_ID_SAVE_LEVEL);
        mFileMenu->addItem(L"Save as level", GUI_ID_SAVEAS_LEVEL);
    }

    mFileMenu->addSeparator();
    if (mCurrentSession != nullptr) {
        mFileMenu->addItem(L"Close level", GUI_ID_CLOSE_LEVEL);
    }

    mFileMenu->addItem(L"Quit", GUI_ID_QUIT);
}

void Editor::PopulateEditMenueEntries() {
    if (mEditMenu == nullptr)
        return;
}

void Editor::PopulateModeMenueEntries() {
    if (mModeMenu == nullptr)
        return;

    /*************************************
     * Mode View                         *
     *************************************/

    mModeMenu->addItem(L"View", GUI_ID_MODE_VIEW, true, false);
    mModeMenu->addItem(L"Terraforming", GUI_ID_MODE_TERRAFORMING, true, false);
    mModeMenu->addItem(L"Column Design", GUI_ID_MODE_COLUMNDESIGN, true, false);
    mModeMenu->addItem(L"Texturing", GUI_ID_MODE_TEXTURING, true, false);
    mModeMenu->addItem(L"Entity", GUI_ID_MODE_ENTITYMODE, true, false);
    mModeMenu->addItem(L"Region", GUI_ID_MODE_REGION, true, false);
}

void Editor::PopulateViewMenueEntries() {
    if (mViewMenu == nullptr)
        return;

    /*************************************
     * Submenue View                     *
     *************************************/

    mViewMenu->addItem(L"Collectibles", GUI_ID_VIEW_ENTITY_COLLECTIBLES, true, false, true, true);
    mViewMenu->addItem(L"Recovery vehicles", GUI_ID_VIEW_ENTITY_RECOVERY, true, false, true, true);
    mViewMenu->addItem(L"Cones", GUI_ID_VIEW_ENTITY_CONES, true, false, true, true);
    mViewMenu->addItem(L"Waypoints", GUI_ID_VIEW_ENTITY_WAYPOINTS, true, false, true, true);
    mViewMenu->addItem(L"Wallsegments", GUI_ID_VIEW_ENTITY_WALLSEGMENTS, true, false, true, true);
    mViewMenu->addItem(L"Triggers", GUI_ID_VIEW_ENTITY_TRIGGERS, true, false, true, true);
    mViewMenu->addItem(L"Cameras", GUI_ID_VIEW_ENTITY_CAMERAS, true, false, true, true);
    mViewMenu->addItem(L"Effects", GUI_ID_VIEW_ENTITY_EFFECTS, true, false, true, true);
    mViewMenu->addItem(L"Morphs", GUI_ID_VIEW_ENTITY_MORPHS, true, false, true, true);

    mViewMenu->addItem(L"Terrain", GUI_ID_VIEWMODE_TERRAIN, true, true);
    mViewMenu->addItem(L"Blocks", GUI_ID_VIEWMODE_BLOCKS, true, true);

    mViewMenu = mMenu->getSubMenu(3)->getSubMenu(9);
    mViewMenu->addItem(L"Off", GUI_ID_VIEW_TERRAIN_OFF);
    mViewMenu->addItem(L"Wireframe", GUI_ID_VIEW_TERRAIN_WIREFRAME);
    mViewMenu->addItem(L"Default", GUI_ID_VIEW_TERRAIN_DEFAULT);
    mViewMenu->addItem(L"Normals", GUI_ID_VIEW_TERRAIN_NORMALS);

    mViewMenu = mMenu->getSubMenu(3)->getSubMenu(10);
    mViewMenu->addItem(L"Off", GUI_ID_VIEW_BLOCKS_OFF);
    mViewMenu->addItem(L"Wireframe", GUI_ID_VIEW_BLOCKS_WIREFRAME);
    mViewMenu->addItem(L"Default", GUI_ID_VIEW_BLOCKS_DEFAULT);
    mViewMenu->addItem(L"Normals", GUI_ID_VIEW_BLOCKS_NORMALS);
}

void Editor::CreateMenue() {
    // create menu
    mMenu = mGuienv->addMenu();

    // add a status line help text
    StatusLine = mGuienv->addStaticText( 0, rect<s32>( 5,  mScreenRes.Height - 30,  mScreenRes.Width - 5, mScreenRes.Height - 10),
                                false, false, 0, -1, true);

    UpdateMenueEntries();
}

//Routine setSkinTransparency taken from Irrlicht engine
//"Example 009 Mesh Viewer"
void Editor::setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin)
{
    for (s32 i=0; i<irr::gui::EGDC_COUNT ; ++i)
    {
        video::SColor col = skin->getColor((EGUI_DEFAULT_COLOR)i);
        col.setAlpha(alpha);
        skin->setColor((EGUI_DEFAULT_COLOR)i, col);
    }
}

void Editor::ChangeViewModeTerrain(irr::u8 newViewMode) {
    if (mCurrentSession != nullptr) {
        if (mCurrentSession->mLevelTerrain != nullptr) {
           mCurrentSession->mLevelTerrain->SetViewMode(newViewMode);
        }
    }
}

void Editor::ChangeViewModeBlocks(irr::u8 newViewMode) {
    if (mCurrentSession != nullptr) {
        if (mCurrentSession->mLevelBlocks != nullptr) {
           mCurrentSession->mLevelBlocks->SetViewMode(newViewMode);
        }
    }
}

void Editor::UpdateEntityVisibilityMenueEntry(irr::u8 whichEntityClass, irr::s32 commandIdMenueEntry) {
    gui::IGUIContextMenu* subMenu = mMenu->getSubMenu(3);

    bool visible = mCurrentSession->mEntityManager->IsVisible(whichEntityClass);

    s32 idxMenuePnt = subMenu->findItemWithCommandId(commandIdMenueEntry, 0);

    if (idxMenuePnt == -1) {
        //menue entry not found
        return;
    }

    subMenu->setItemChecked(idxMenuePnt, visible);
}

void Editor::UpdateEntityVisibilityMenueEntries() {
    if (mCurrentSession == nullptr)
        return;

    if (mCurrentSession->mEntityManager == nullptr)
        return;

    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES, GUI_ID_VIEW_ENTITY_COLLECTIBLES);
    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY, GUI_ID_VIEW_ENTITY_RECOVERY);
    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_CONES, GUI_ID_VIEW_ENTITY_CONES);
    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS, GUI_ID_VIEW_ENTITY_WAYPOINTS);
    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS, GUI_ID_VIEW_ENTITY_WALLSEGMENTS);
    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS, GUI_ID_VIEW_ENTITY_TRIGGERS);
    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS, GUI_ID_VIEW_ENTITY_CAMERAS);
    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS, GUI_ID_VIEW_ENTITY_EFFECTS);
    UpdateEntityVisibilityMenueEntry(DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS, GUI_ID_VIEW_ENTITY_MORPHS);
}

void Editor::ChangeEntityVisibility(IGUIContextMenu* menu) {
    s32 id = menu->getItemCommandId(menu->getSelectedItem());

    s32 idxMenuePnt = menu->findItemWithCommandId(id, 0);

    if (idxMenuePnt == -1) {
        //menue entry not found
        return;
    }

    bool visible = menu->isItemChecked(idxMenuePnt);

    if (mCurrentSession == nullptr)
        return;

    if (mCurrentSession->mEntityManager == nullptr)
        return;

    switch (id) {
        case GUI_ID_VIEW_ENTITY_COLLECTIBLES: {
           mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES, visible);
           break;
        }

        case GUI_ID_VIEW_ENTITY_RECOVERY:  {
            mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY, visible);
            break;
        }

        case GUI_ID_VIEW_ENTITY_CONES:  {
            mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CONES, visible);
            break;
        }

        case GUI_ID_VIEW_ENTITY_WAYPOINTS:  {
            mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS, visible);
            break;
        }

        case GUI_ID_VIEW_ENTITY_WALLSEGMENTS:  {
            mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS, visible);
            break;
        }

        case GUI_ID_VIEW_ENTITY_TRIGGERS:  {
            mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS, visible);
            break;
        }

        case GUI_ID_VIEW_ENTITY_CAMERAS:  {
            mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS, visible);
            break;
        }

        case GUI_ID_VIEW_ENTITY_EFFECTS: {
            mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS, visible);
            break;
        }

        case GUI_ID_VIEW_ENTITY_MORPHS: {
            mCurrentSession->mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS, visible);
            break;
        }

        default: {
            break;
        }
    }
}

void Editor::OpenOriginalLevel(irr::s32 menueItemId) {
    bool success;

    switch (menueItemId) {
        case GUI_ID_ORIGINAL_AMAZON_DELTA_TURNPIKE: {
            success = CreateNewEditorSession("extract/", "level0-1");
            break;
        }

        case GUI_ID_ORIGINAL_TRANSASIA_INTERSTATE: {
            success = CreateNewEditorSession("extract/", "level0-2");
            break;
        }

        case GUI_ID_ORIGINAL_SHANGHAI_DRAGON: {
            success = CreateNewEditorSession("extract/", "level0-3");
            break;
        }

        case GUI_ID_ORIGINAL_NEW_CHERNOBYL_CENTRAL: {
            success = CreateNewEditorSession("extract/", "level0-4");
            break;
        }

        case GUI_ID_ORIGINAL_SLAM_CANYON: {
            success = CreateNewEditorSession("extract/", "level0-5");
            break;
        }

        case GUI_ID_ORIGINAL_THRAK_CITY: {
            success = CreateNewEditorSession("extract/", "level0-6");
            break;
        }

        case GUI_ID_ORIGINAL_ANCIENT_MINE_TOWN: {
           if (mExtendedGame) {
               success = CreateNewEditorSession("extract/", "level0-7");
           } else {
               success = false;
           }

           break;
        }

        case GUI_ID_ORIGINAL_ARCTIC_LAND: {
           if (mExtendedGame) {
               success = CreateNewEditorSession("extract/", "level0-8");
           } else {
               success = false;
           }

           break;
        }

        case GUI_ID_ORIGINAL_DEATH_MATCH_ARENA: {
           if (mExtendedGame) {
               success = CreateNewEditorSession("extract/", "level0-9");
           } else {
               success = false;
           }

           break;
        }

        default: {
           success = false;
           break;
        }
    }

    if (!success) {
        mEditorState = DEF_EDITORSTATE_ERROR;
    } else {
        UpdateMenueEntries();

        mEditorState = DEF_EDITORSTATE_SESSIONACTIVE;
    }
}

void Editor::OnMenuItemSelected( IGUIContextMenu* menu )
{
    s32 id = menu->getItemCommandId(menu->getSelectedItem());

    switch(id)
    {
        case GUI_ID_MODE_VIEW: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mViewMode);
           }
           break;
        }

        case GUI_ID_MODE_TEXTURING: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mTextureMode);
           }
           break;
        }

        case GUI_ID_MODE_COLUMNDESIGN: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mColumnDesigner);
           }
           break;
        }

        case GUI_ID_MODE_TERRAFORMING: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mTerraforming);
           }
           break;
        }

        case GUI_ID_MODE_REGION: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mRegionMode);
           }
           break;
        }

        case GUI_ID_MODE_ENTITYMODE: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mEntityMode);
           }
           break;
        }

        case GUI_ID_VIEW_TERRAIN_OFF:  {
           ChangeViewModeTerrain(LEVELTERRAIN_VIEW_OFF);
           break;
        }

        case GUI_ID_VIEW_TERRAIN_WIREFRAME:  {
           ChangeViewModeTerrain(LEVELTERRAIN_VIEW_WIREFRAME);
           break;
        }

        case GUI_ID_VIEW_TERRAIN_DEFAULT:  {
           ChangeViewModeTerrain(LEVELTERRAIN_VIEW_DEFAULT);
           break;
        }

        case GUI_ID_VIEW_TERRAIN_NORMALS:  {
           ChangeViewModeTerrain(LEVELTERRAIN_VIEW_DEBUGNORMALS);
           break;
        }

        case GUI_ID_VIEW_BLOCKS_OFF:  {
           ChangeViewModeBlocks(LEVELBLOCKS_VIEW_OFF);
           break;
        }

        case GUI_ID_VIEW_BLOCKS_WIREFRAME:  {
           ChangeViewModeBlocks(LEVELBLOCKS_VIEW_WIREFRAME);
           break;
        }

        case GUI_ID_VIEW_BLOCKS_DEFAULT:  {
           ChangeViewModeBlocks(LEVELBLOCKS_VIEW_DEFAULT);
           break;
        }

        case GUI_ID_VIEW_BLOCKS_NORMALS:  {
           ChangeViewModeBlocks(LEVELBLOCKS_VIEW_DEBUGNORMALS);
           break;
        }

        case GUI_ID_VIEW_ENTITY_COLLECTIBLES:
        case GUI_ID_VIEW_ENTITY_RECOVERY:
        case GUI_ID_VIEW_ENTITY_CONES:
        case GUI_ID_VIEW_ENTITY_WAYPOINTS:
        case GUI_ID_VIEW_ENTITY_WALLSEGMENTS:
        case GUI_ID_VIEW_ENTITY_TRIGGERS:
        case GUI_ID_VIEW_ENTITY_CAMERAS:
        case GUI_ID_VIEW_ENTITY_EFFECTS:
        case GUI_ID_VIEW_ENTITY_MORPHS: {
            ChangeEntityVisibility(menu);
            break;
        }

        case GUI_ID_SAVE_LEVEL: {
            if (mCurrentSession != nullptr) {
                //mGuienv->addFileOpenDialog(L"Please select filename for Save");

                mCurrentSession->mLevelRes->Save("mlevel0-1.dat");
                //mCurrentSession->mLevelRes->Save("/home/wolfalex/hi/maps/level0-1.dat");
            }
            break;
        }

        case GUI_ID_SAVEAS_LEVEL: {
            if (mCurrentSession != nullptr) {
                //Note: If the user selected a file Irrlicht
                //will generate a GuiEvent that will be handled
                //in the GuiEvent Handler function
                //This means the file saving operation will happen somewhere else
                /*mGuienv->addFileOpenDialog(L"Select Save As filename", true, nullptr, GUI_ID_EDITORSESSION_SAVEAS_FILEOPENDIALOG,
                                           true, nullptr);*/
            }
            break;
        }

        case GUI_ID_CLOSE_LEVEL: {
            if (mCurrentSession != nullptr) {
                //This will trigger the close operation, a MessageBox will be shown
                //to ask if the user really wants to Close the EditorSession and possibly
                //lose unsaved data; The user answer will be returned in a Gui Event inside this Editor
                //class
                mCurrentSession->TriggerClose();
            }

            break;
        }

        case GUI_ID_NEWEMPTYLEVEL_ROCK: {
            if (mCurrentSession == nullptr) {
                 mNewLevelStyleSelector = DEF_EDITOR_NEWLEVELSTYLE_ROCK;
                 mEditorState = DEF_EDITORSTATE_CREATENEWEMPTYLEVEL;
            }
            break;
        }

        case GUI_ID_NEWEMPTYLEVEL_VEGETATION: {
            if (mCurrentSession == nullptr) {
                 mNewLevelStyleSelector = DEF_EDITOR_NEWLEVELSTYLE_VEGETATION;
                 mEditorState = DEF_EDITORSTATE_CREATENEWEMPTYLEVEL;
            }
            break;
        }

        case GUI_ID_NEWEMPTYLEVEL_SNOW: {
            if (mCurrentSession == nullptr) {
                 mNewLevelStyleSelector = DEF_EDITOR_NEWLEVELSTYLE_SNOW;
                 mEditorState = DEF_EDITORSTATE_CREATENEWEMPTYLEVEL;
            }
            break;
        }

        // File -> Quit
        case GUI_ID_QUIT: {
            ExitEditor = true;
            break;
        }

        case GUI_ID_ORIGINAL_AMAZON_DELTA_TURNPIKE:
        case GUI_ID_ORIGINAL_TRANSASIA_INTERSTATE:
        case GUI_ID_ORIGINAL_SHANGHAI_DRAGON:
        case GUI_ID_ORIGINAL_NEW_CHERNOBYL_CENTRAL:
        case GUI_ID_ORIGINAL_SLAM_CANYON:
        case GUI_ID_ORIGINAL_THRAK_CITY:
        case GUI_ID_ORIGINAL_ANCIENT_MINE_TOWN:
        case GUI_ID_ORIGINAL_ARCTIC_LAND:
        case GUI_ID_ORIGINAL_DEATH_MATCH_ARENA:
            {
                mWhichMenueItemWasClicked = id;
                mEditorState = DEF_EDITORSTATE_LOADDATA;
                break;
            }
    }
}

void Editor::OnButtonClicked(irr::s32 buttonId) {
    //depending over which window the user mouse cursor
    //is currently, sent the button click Id to the correct
    //window
    if (mCurrentSession == nullptr)
        return;

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINTEXTUREDIALOG) {
        mCurrentSession->mTextureMode->OnButtonClicked(buttonId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINVIEWMODEDIALOG) {
        mCurrentSession->mViewMode->OnButtonClicked(buttonId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINENTITYMODEDIALOG) {
        mCurrentSession->mEntityMode->OnButtonClicked(buttonId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINCOLUMNDESIGNERDIALOG) {
        mCurrentSession->mColumnDesigner->OnButtonClicked(buttonId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINREGIONMODEDIALOG) {
        mCurrentSession->mRegionMode->OnButtonClicked(buttonId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINTERRAFORMINGDIALOG) {
        mCurrentSession->mTerraforming->OnButtonClicked(buttonId);
    }
}

void Editor::OnCheckBoxChanged(irr::s32 checkBoxId) {
    //depending over which window the user mouse cursor
    //is currently, sent the changed checkbox Id to the correct
    //window
    if (mCurrentSession == nullptr)
        return;

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINTEXTUREDIALOG) {
        mCurrentSession->mTextureMode->OnCheckBoxChanged(checkBoxId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINVIEWMODEDIALOG) {
        mCurrentSession->mViewMode->OnCheckBoxChanged(checkBoxId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINENTITYMODEDIALOG) {
        mCurrentSession->mEntityMode->OnCheckBoxChanged(checkBoxId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINCOLUMNDESIGNERDIALOG) {
        mCurrentSession->mColumnDesigner->OnCheckBoxChanged(checkBoxId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINREGIONMODEDIALOG) {
        mCurrentSession->mRegionMode->OnCheckBoxChanged(checkBoxId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINTERRAFORMINGDIALOG) {
        mCurrentSession->mTerraforming->OnCheckBoxChanged(checkBoxId);
    }
}

void Editor::OnScrollbarMoved(irr::s32 scrollBarId) {
   //switch (scrollBarId) {
     /*  case GUI_ID_SCROLLBAR: {
           std::cout << "scrollbar" << std::endl;
           break;
       }*/
   //}
}

void Editor::OnElementFocused(irr::s32 elementId) {
  //std::cout << "Element Focus " << elementId << std::endl;
}

void Editor::OnElementFocusLost(irr::s32 elementId) {
  //std::cout << "Element Focus " << elementId << std::endl;
  CheckForNumberEditBoxEvent(elementId);

  if (mCurrentSession != nullptr) {
        if (mCurrentSession->mEditorMode != nullptr) {
                mCurrentSession->mEditorMode->OnElementFocusLost(elementId);
        }
   }
}

void Editor::OnElementHovered(irr::s32 elementId) {
  //std::cout << "Element Hovered " << elementId << std::endl;

    if (mCurrentSession != nullptr) {
          if (mCurrentSession->mEditorMode != nullptr) {
                  mCurrentSession->mEditorMode->OnElementHovered(elementId);
          }
     }
}

void Editor::OnTableSelected(irr::s32 elementId) {
   //std::cout << "Table selection changed " << elementId << std::endl;

   if (mCurrentSession != nullptr) {
          if (mCurrentSession->mEditorMode != nullptr) {
                  mCurrentSession->mEditorMode->OnTableSelected(elementId);
          }
   }
}

void Editor::OnElementLeft(irr::s32 elementId) {
  //std::cout << "Element Left " << elementId << std::endl;
  if (mCurrentSession != nullptr) {
       if (mCurrentSession->mEditorMode != nullptr) {
            mCurrentSession->mEditorMode->OnElementLeft(elementId);
       }
  }
}

//if function returns true the close action should be interrupted
bool Editor::OnElementClose(irr::s32 elementId) {
  //std::cout << "Element Close " << elementId << std::endl;

  //prevent that user can close the editor windows
  //otherwise the program will crash the next time we need the
  //window again; just hide the window and interrupt the close call
  if (mCurrentSession != nullptr) {
      if (mCurrentSession->mEditorMode != nullptr) {
            mCurrentSession->mEditorMode->HideWindow();

            return true;
      }
  }

  return false;
}

void Editor::OnMessageBoxYes(irr::s32 elementId) {
  switch(elementId) {
      case GUI_ID_EDITORSESSION_MSGBOX_SURECLOSE: {
         //user pressed yes in the AreYouSureToCloseEditorSession
         //Messagebox => we want to exit the current EditorSession
         mEditorState = DEF_EDITORSTATE_CLOSECURRENTSESSION;
         break;
      }
  }
}

void Editor::OnComboBoxChanged(IGUIComboBox* comboBox) {
  u32 val = comboBox->getItemData ( comboBox->getSelected() );
  //std::cout << "ComboBox changed " << val << std::endl;

  if (comboBox->getID() == GUI_ID_TEXCATEGORYCOMBOBOX) {
    mCurrentSession->mTextureMode->TextureCategoryChanged(val);
  }

  if (comboBox->getID() == GUI_ID_TEXMODIFICATIONCOMBOBOX) {
    mCurrentSession->mTextureMode->TextureModificationChanged(val);
  }

  if (comboBox->getID() == GUI_ID_COLUMNDEFSELECTIONCOMBOBOX) {
      mCurrentSession->mColumnDesigner->OnColumnDefinitionComboBoxChanged(val);
  }

  if (comboBox->getID() == GUI_ID_ENTITYCATEGORYCOMBOBOX) {
      mCurrentSession->mEntityMode->EntityCategoryChanged(val);
  }

  if (comboBox->getID() == GUI_ID_REGIONMODEWINDOW_TYPE_COMBOBOX) {
      mCurrentSession->mRegionMode->OnRegionTypeComboBoxChanged(val);
  }
}

void Editor::OnEditBoxEnterEvent(IGUIEditBox* editBox) {
  s32 val = editBox->getID();
  //std::cout << "EditBox Enter Event " << val << std::endl;

  if (val == -1)
      return;

  CheckForNumberEditBoxEvent(val);
}

void Editor::OnLeftMouseButtonDown() {
    MouseState.LeftButtonDown = true;

    //did the user select the currently highlighted
    //object? tell itemSelector that the user has clicked
    //with the left mouse button
    if (mCurrentSession != nullptr) {
      if (mCurrentSession->mItemSelector != nullptr) {
          mCurrentSession->mItemSelector->OnLeftMouseButtonDown();
      }

      if (mCurrentSession->mEditorMode != nullptr) {
          mCurrentSession->mEditorMode->OnLeftMouseButtonDown();
      }
    }
}

void Editor::OnLeftMouseButtonUp() {
    MouseState.LeftButtonDown = false;
}

//overwrite HandleMouseEvent method for Editor
void Editor::HandleMouseEvent(const irr::SEvent& event) {
    switch(event.MouseInput.Event)
    {
      case EMIE_LMOUSE_PRESSED_DOWN:
            OnLeftMouseButtonDown();
            break;

      case EMIE_LMOUSE_LEFT_UP:
            OnLeftMouseButtonUp();
            break;

      case EMIE_MOUSE_MOVED:
            MouseState.Position.X = event.MouseInput.X;
            MouseState.Position.Y = event.MouseInput.Y;
            break;

      default:
            // We won't use the wheel
            break;
      }
}

//overwrite HandleGuiEvent method for Editor
//returns true if Gui Event should be canceled
bool Editor::HandleGuiEvent(const irr::SEvent& event) {
    irr::s32 id = event.GUIEvent.Caller->getID();

    switch(event.GUIEvent.EventType)
    {
        case EGET_MENU_ITEM_SELECTED: {
          // a menu item was clicked
          OnMenuItemSelected( (IGUIContextMenu*)event.GUIEvent.Caller );
          break;
        }

        case EGET_BUTTON_CLICKED: {
            // a button was clicked
            OnButtonClicked(id);
            break;
        }

        case EGET_CHECKBOX_CHANGED: {
            //a checkbox was changed
            OnCheckBoxChanged(id);
            break;
        }

        case EGET_SCROLL_BAR_CHANGED: {
            //a scrollbar was moved
            OnScrollbarMoved(id);
            break;
        }

        case EGET_ELEMENT_FOCUSED: {
            //an element got the focus
            OnElementFocused(id);
            break;
        }

        case EGET_ELEMENT_FOCUS_LOST: {
            //an element lost the focus
            //for the EditorBox I want to use
            //this event to know when the user
            //stops editing to trigger an update
            OnElementFocusLost(id);
            break;
        }

        case EGET_ELEMENT_HOVERED: {
            //user hovered over an element
            OnElementHovered(id);
            break;
        }

        case EGET_ELEMENT_LEFT : {
            //user left an element
            OnElementLeft(id);
            break;
        }

        case EGET_COMBO_BOX_CHANGED: {
            //user changed a combobox selection
            OnComboBoxChanged((IGUIComboBox*)event.GUIEvent.Caller);
            break;
        }

        case EGET_TABLE_CHANGED: {
            //user changed selection in a table
            OnTableSelected(id);
            break;
        }

         case EGET_ELEMENT_CLOSED: {
            //user tried to close an Ui element/window
            if (OnElementClose(id)) {
                //we want to prevent closing this element
                return true;
            }

            //we do not want to cancel this event
            return false;
        }

        case EGET_EDITBOX_ENTER: {
            //user pressed Enter in an EditBox
            OnEditBoxEnterEvent((IGUIEditBox*)event.GUIEvent.Caller);
            break;
        }

        case EGET_MESSAGEBOX_YES: {
            //User pressed yes in a MessageBox
            OnMessageBoxYes(id);
            break;
        }

        case EGET_FILE_SELECTED: {
            if (id == GUI_ID_EDITORSESSION_SAVEAS_FILEOPENDIALOG) {
                if (mCurrentSession == nullptr)
                    return false;

                  //Save map under the filename selected in the SaveAs File dialog
                  IGUIFileOpenDialog* dialog =
                    (IGUIFileOpenDialog*)event.GUIEvent.Caller;

                  std::string filename("");
                  filename.append(core::stringc(dialog->getFileName()).c_str());
                  mCurrentSession->mLevelRes->Save(filename);
            }

            break;
        }

        default: {
                break;
        }
     }

    //we do not want to cancel this event
    return false;
}

bool Editor::LoadAdditionalGameImages() {
     mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //load gameTitle
     gameTitle = mDriver->getTexture("extract/images/title.png");

     if (gameTitle == nullptr) {
         //there was a texture loading error
         //just return with false
         return false;
     }

     gameTitleSize = gameTitle->getSize();
     //calculate position to draw gameTitle so that it is centered on the screen
     //because most likely target resolution does not fit with image resolution
     gameTitleDrawPos.X = (mScreenRes.Width - gameTitleSize.Width) / 2;
     gameTitleDrawPos.Y = (mScreenRes.Height - gameTitleSize.Height) / 2;

     //load race loading screen
     raceLoadingScr = mDriver->getTexture("extract/images/onet0-1.png");

     if (raceLoadingScr == nullptr) {
         //there was a texture loading error
         //just return with false
         return false;
     }

     raceLoadingScrSize = raceLoadingScr->getSize();
     //calculate position to draw race loading screen so that it is centered on the screen
     //because maybe target resolution does not fit with image resolution
     raceLoadingScrDrawPos.X = (mScreenRes.Width - raceLoadingScrSize.Width) / 2;
     raceLoadingScrDrawPos.Y = (mScreenRes.Height - raceLoadingScrSize.Height) / 2;

     mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

     return true;
}

bool Editor::LoadBackgroundImage() {
    mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    //first load background image for menue

    //backgnd = mDriver->getTexture("extract/images/oscr0-1.png");
    backgnd = mDriver->getTexture("extract/images/oscr0-1-x2.png");

    if (backgnd == nullptr) {
        //there was a texture loading error
        //just return with false
        return false;
    }

    irr::core::dimension2d<irr::u32> backgndSize;

    backgndSize = backgnd->getSize();
    if ((backgndSize.Width != mScreenRes.Width) ||
        (backgndSize.Height != mScreenRes.Height)) {
        logging::Error("Background image does not fit with the selected screen resolution");
        //background texture size does not fit with selected screen resolution
        return false;
    }

    mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}

void Editor::RenderDataExtractionScreen() {
    //first draw background picture
     mDriver->draw2DImage(backgnd, irr::core::vector2di(0, 0),
                         irr::core::recti(0, 0, mScreenRes.Width, mScreenRes.Height)
                         , 0, irr::video::SColor(255,255,255,255), true);

     char* infoText = strdup("EXTRACTING GAME DATA, PLEASE STANDBY...");
     char* currStepText = strdup(mPrepareData->currentStepDescription.c_str());

     irr::u32 textHeight = mGameTexts->GetHeightPixelsGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA);
     irr::u32 textWidth = mGameTexts->GetWidthPixelsGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA);

     irr::core::position2di txtPos;
     txtPos.X = mScreenRes.Width / 2 - textWidth / 2;
     txtPos.Y = mScreenRes.Height / 2 - textHeight / 2;

     //write info text, warning: at the data extraction stage of the game only single font GameMenueWhiteTextSmallSVGA is
     //available!
     mGameTexts->DrawGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA, txtPos);

     textHeight = mGameTexts->GetHeightPixelsGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA);
     textWidth = mGameTexts->GetWidthPixelsGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA);

     txtPos.X = mScreenRes.Width / 2 - textWidth / 2;
     txtPos.Y += 25;

     mGameTexts->DrawGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA, txtPos);

     free(infoText);
     free(currStepText);
}

void Editor::EditorLoopExtractData() {
    //execute next data extraction step
    //returns true if data extraction process is finished
    try {
        if (mPrepareData->ExecuteNextStep()) {
            //data extraction is finished
            //continue editor initialization
            if (!InitEditorStep2()) {
                mEditorState = DEF_EDITORSTATE_ERROR;
                return;
            }
            UpdateMenueEntries();
            mEditorState = DEF_EDITORSTATE_NOSESSIONACTIVE;
            return;
        }
    }
     catch (const std::string &msg) {
         std::string msgExt("Step 2 of game assets preparation operation failed: ");
         msgExt.append(msg);
         logging::Error(msgExt);
         mEditorState = DEF_EDITORSTATE_ERROR;
         return;
    }

    mDriver->beginScene(true,true,
    video::SColor(255,100,101,140));

    //update graphical image about current status
    RenderDataExtractionScreen();

    mDriver->endScene();
}

void Editor::EditorLoopNoSessionOpen() {
    mDriver->beginScene(true,true,
    video::SColor(255,100,101,140));

    //draw background picture
    mDriver->draw2DImage(backgnd, irr::core::vector2di(0, 0),
                         irr::core::recti(0, 0, mScreenRes.Width, mScreenRes.Height)
                         , 0, irr::video::SColor(255,255,255,255), true);

    //draw Gui
    mGuienv->drawAll();

    mDriver->endScene();
}

void Editor::UpdateStatusbarText(const wchar_t *text) {
    wcscpy(mCurrentStatusBarText, text);

    if ( StatusLine != nullptr ) {
          StatusLine->setText(mCurrentStatusBarText);
    }
}

void Editor::EditorLoopSession(irr::f32 frameDeltaTime) {

    mTimeProfiler->StartOfGameLoop();

    mCurrentSession->AdvanceTime(frameDeltaTime);

    mCurrentSession->HandleBasicInput();
    mCurrentSession->TrackActiveDialog();

    mTimeProfiler->Profile(mTimeProfiler->tIntHandleInput);

    //Update Time Profiler results
    mTimeProfiler->UpdateWindow();

    if (DebugShowVariableBoxes) {

            wchar_t* text2 = new wchar_t[400];

            swprintf(text2, 390, L"");

            /*if (mCurrentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
                swprintf(text2, 390, L"Cell X = %d Y = %d  %u", this->mCurrentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X, this->mCurrentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y,
                         this->mCurrentSession->mItemSelector->columTrianglesHitCnt);
            } else if (mCurrentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
                swprintf(text2, 390, L"Blocks %d %u", this->mCurrentSession->mItemSelector->mCurrSelectedItem.mSelBlockNrStartingFromBase, this->mCurrentSession->mItemSelector->columTrianglesHitCnt);
            } else {
               swprintf(text2, 390, L"%u", this->mCurrentSession->mItemSelector->columTrianglesHitCnt);
            }*/

            dbgText->setText(text2);

            delete[] text2;
    }

   /* if (mCurrentSession->UpdateBlockPreview) {
        mCurrentSession->UpdateBlockPreview = false;
        mCurrentSession->mLevelBlocks->CreateBlockPreview();
    }*/

    mDriver->beginScene(true,true,
     video::SColor(255,100,101,140));

    //render scene: terrain, blocks, player craft, entities...
    mCurrentSession->Render();

    mSmgr->drawAll();

    mTimeProfiler->Profile(mTimeProfiler->tIntRender3DScene);

    //render log window
    mLogger->Render();

    //last draw text debug output from Irrlicht
    mGuienv->drawAll();

    mTimeProfiler->Profile(mTimeProfiler->tIntRender2D);

    mDriver->endScene();

    //does the user want to exit the LevelEditor?
    if (false) {

        ExitEditor = true;

//        //if we were in game debugging mode simply skip
//        //main menue, and exit game immediately
//        if (mDebugRace || mDebugDemoMode) {
//            ExitGame = true;
//        } else {
//            mGameState = DEF_GAMESTATE_MENUE;

//            if (this->lastRaceStat != nullptr) {
//                //Show race statistics
//                MainMenue->ShowRaceStats(lastRaceStat);
//            } else {
//                //there are no race statistics to show
//                //simply return to the main menue
//                MainMenue->ShowMainMenue();
//            }
//        }
    }
}

void Editor::EditorLoop() {

    // In order to do framerate independent movement, we have to know
    // how long it was since the last frame
    u32 then = mDevice->getTimer()->getTime();

    while (mDevice->run() && (ExitEditor == false)) {
       /*  if (device->isWindowActive())
            {*/

        //Work out a frame delta time.
        const u32 now = mDevice->getTimer()->getTime();
        f32 frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
        then = now;

        switch (mEditorState) {
            case DEF_EDITORSTATE_EXTRACTDATA: {
                EditorLoopExtractData();
                break;
            }

            case DEF_EDITORSTATE_NOSESSIONACTIVE: {
                //ParentEditor is ready for a session,
                //but no session is loaded yet
                EditorLoopNoSessionOpen();
                break;
            }

            case DEF_EDITORSTATE_LOADDATA: {
                //load original level
                OpenOriginalLevel(mWhichMenueItemWasClicked);
                break;
            }

          /*  //shows game title, loads game data
            case DEF_GAMESTATE_GAMETITLE: {
                   GameLoopTitleScreenLoadData();
                   break;
            }

            case DEF_GAMESTATE_INITDEMO:
            case DEF_GAMESTATE_INITRACE: {
                   GameLoopLoadRaceScreen();
                   break;
            }*/

            case DEF_EDITORSTATE_SESSIONACTIVE: {
                EditorLoopSession(frameDeltaTime);
                break;
            }

            case DEF_EDITORSTATE_CLOSECURRENTSESSION: {
                //clean up current editor session data
                delete mCurrentSession;
                mCurrentSession = nullptr;

                //make sure that TimeProfiler window is hidden
                mTimeProfiler->HideWindow();

                //make sure that Logger Window is hidden
                mLogger->HideWindow();

                UpdateMenueEntries();
                mEditorState = DEF_EDITORSTATE_NOSESSIONACTIVE;

                break;
            }

            case DEF_EDITORSTATE_CREATENEWEMPTYLEVEL: {
                bool success;

                if (CreateNewEmptyLevel(mNewLevelStyleSelector, "newlevel")) {
                    //new level creation was succesfull
                    logging::Info("New level creation was succesfull");

                    //now load the new level
                    success = CreateNewEditorSession("userdata/levels/", "newlevel");

                    if (!success) {
                        mEditorState = DEF_EDITORSTATE_NOSESSIONACTIVE;
                        logging::Error("Failed to load new level");
                        mGuienv->addMessageBox(L"Error", L"Failed to load new level", true, EMBF_OK, nullptr, -1, nullptr);
                    } else {
                        UpdateMenueEntries();

                        mEditorState = DEF_EDITORSTATE_SESSIONACTIVE;
                    }
                } else {
                    mGuienv->addMessageBox(L"Error", L"New level creation failed", true, EMBF_OK, nullptr, -1, nullptr);
                    mEditorState = DEF_EDITORSTATE_NOSESSIONACTIVE;
                }

                break;
            }

            case DEF_EDITORSTATE_ERROR: {
                //there was an error, exit game
                ExitEditor = true;
                break;
            }
        }

        int fps = mDriver->getFPS();

        if (lastFPS != fps) {
                         core::stringw tmp(L"Hi-Editor [");
                         tmp += mDriver->getName();
                         tmp += L"] Triangles drawn: ";
                         tmp += mDriver->getPrimitiveCountDrawn();
                         tmp += " @ fps: ";
                         tmp += fps;

                         mDevice->setWindowCaption(tmp.c_str());
                         lastFPS = fps;
                     }
            //}
   }

   mDriver->drop();
}

bool Editor::CreateNewEditorSession(std::string levelRootPath, std::string levelName) {
    if (mCurrentSession != nullptr)
        return false;

    //create a new editor session
    mCurrentSession = new EditorSession(this, levelRootPath, levelName);

    mCurrentSession->Init();

    if (!mCurrentSession->ready) {
        //there was a problem with EditorSession initialization
        logging::Error("EditorSession creation failed!");
        return false;
    }

    //We first need to update the Menue-Entries
    //so that the next command does not crash with nullptr
    //reference
    UpdateMenueEntries();

    //update current visible Entities
    UpdateEntityVisibilityMenueEntries();

    //start in ViewMode
    mCurrentSession->SetMode((EditorMode*)(mCurrentSession->mViewMode));

    return true;
}

void Editor::RegisterNumberEditBox(NumberEditBox* whichBox, irr::s32 boxGuiId) {
   mRegisteredNumberEditBoxes.push_back(std::make_pair(boxGuiId, whichBox));
}

void Editor::UnregisterNumberEditBox(irr::s32 boxGuiId) {
   std::vector<std::pair<irr::s32, NumberEditBox*>>::iterator it;

   for (it = mRegisteredNumberEditBoxes.begin(); it != mRegisteredNumberEditBoxes.end(); ) {
       if ((*it).first == boxGuiId) {
           //we need to erase this entry
           it = mRegisteredNumberEditBoxes.erase(it);
       } else {
           //go to the next element
           ++it;
       }
   }
}

void Editor::CheckForNumberEditBoxEvent(irr::s32 receivedGuiId) {
   std::vector<std::pair<irr::s32, NumberEditBox*>>::iterator it;

   for (it = mRegisteredNumberEditBoxes.begin(); it != mRegisteredNumberEditBoxes.end(); ++it) {
      if ((*it).first == receivedGuiId) {
          //we found an event for this NumberEditBox
          (*it).second->OnEditBoxEditEndedEvent();
          return;
      }
   }
}

//Returns true in case of success, False otherwise
bool Editor::CopyLevelTextures(std::string originMapFolder, std::string targetMapFolder) {
    //if origin directory is not present, fail operation
    if (IsDirectoryPresent(originMapFolder.c_str()) == -1) {
        return false;
    }

    //if target directory is not present, fail operation
    if (IsDirectoryPresent(targetMapFolder.c_str()) == -1) {
        return false;
    }

    //there should be 256 texture files, one for each Texture Id
    for (int tileIdx = 0; tileIdx < 256; tileIdx++) {
        std::stringstream fpOrigin;
        fpOrigin << originMapFolder << "tex" << std::setw(4) << std::setfill('0') << tileIdx << ".png";
        std::string finalpathOrigin = fpOrigin.str();

        std::stringstream fpTarget;
        fpTarget << targetMapFolder << "tex" << std::setw(4) << std::setfill('0') << tileIdx << ".png";
        std::string finalpathTarget = fpTarget.str();

        //only execute operation if file exists, otherwise fail
        if (FileExists(finalpathOrigin.c_str()) == 1) {
            //origin file exists

            //Returns 1 in case of unexpected error, 0 for success
            if (copy_file(finalpathOrigin.c_str(), finalpathTarget.c_str()) == 1) {
                //copy failed
                return false;
            }
        } else {
            //Expected origin file is missing, Fail
            return false;
        }
    }

    //copying of all textures was succesfull
    return true;
}

//Returns true if succesfull, False otherwise
//Note: outputMapName is not a folder path, instead a single word (name)
//for the map
bool Editor::CreateNewEmptyLevel(irr::u32 newLevelStype, std::string outputMapName) {
    //Make sure user custom level map
    //folder already exists
    if (!PrepareUserMapsFolder()) {
        return false;
    }

    std::string mapSubFolder("userdata/levels/");
    mapSubFolder.append(outputMapName);

    //create a sub directory for new level
    try {
        PrepareSubDir(mapSubFolder.c_str());
    }
    catch (const std::string &msg) {
        std::string msgExt("Preparation of new map subfolder failed");
        msgExt.append(msg);
        logging::Error(msgExt);
        return false;
    }

    //copy the level textures depending on the selected
    //level graphics style
    std::string originLevel("extract/");
    std::string originLevelFile("extract/");

    switch (newLevelStype) {
       case DEF_EDITOR_NEWLEVELSTYLE_ROCK: {
           originLevel.append("level0-1/");
           originLevelFile.append("level0-1/level0-1-unpacked.dat");
           break;
       }

       case DEF_EDITOR_NEWLEVELSTYLE_VEGETATION: {
            originLevel.append("level0-2/");
            originLevelFile.append("level0-2/level0-2-unpacked.dat");
            break;
       }

        case DEF_EDITOR_NEWLEVELSTYLE_SNOW: {
             originLevel.append("level0-3/");
             originLevelFile.append("level0-3/level0-3-unpacked.dat");
             break;
        }

        default: {
           return false;
        }
    }

    mapSubFolder.append("/");

    //now copy the textures from Origin to Target (new level)
    if (!CopyLevelTextures(originLevel, mapSubFolder)) {
        logging::Error("CreateNewEmptyLevel: Copy of terrain textures failed");
        return false;
    }

    std::string newLevelFileName("");
    newLevelFileName.append(mapSubFolder);
    newLevelFileName.append(outputMapName);
    newLevelFileName.append("-unpacked.dat");

    if (!CreateNewEmptyLevelFile(originLevelFile, newLevelFileName)) {
        logging::Error("CreateNewEmptyLevel: Creation of new level map file failed");
        return false;
    }

    //operation was succesfull
    return true;
}

//Returns true if succesfull, False otherwise
bool Editor::CreateNewEmptyLevelFile(std::string originFileName, std::string outputFileName) {
    ifstream ifile;
    std::streampos fileSize;

    ifile.open(originFileName.c_str(), std::ifstream::binary);
    if(ifile) {
          logging::Info("CreateNewEmptyLevel: Origin file found and openend succesfully");
       } else {
           logging::Error("CreateNewEmptyLevel: Could not open Origin Level file");
           return false;
    }

    // get its size:
    ifile.seekg(0, std::ios::end);
    fileSize = ifile.tellg();
    ifile.seekg(0, std::ios::beg);

    std::vector<uint8_t>* bytesVec = new std::vector<uint8_t>();

    //read the data
    bytesVec->resize(fileSize);
    ifile.read(reinterpret_cast<char*>(bytesVec->data()), bytesVec->size());

    ifile.close();

    char hlpstr[500];
    std::string msg("");

     if (ifile) {
         msg.clear();
         msg.append("Origin level file read succesfully (");
         size_t fileSizet = fileSize;
         snprintf(hlpstr, 500, "%zu", fileSizet);
         msg.append(hlpstr);
         msg.append(" bytes)");
         logging::Info(msg);
     }
     else {
         size_t fileReadsizet = ifile.gcount();
         msg.clear();
         msg.append("Origin level file read error: only ");
         snprintf(hlpstr, 500, "%zu", fileReadsizet);
         msg.append(hlpstr);
         msg.append(" bytes could be read!");
         logging::Error(msg);

         delete bytesVec;

         return false;
   }

   //first remove all entities
   //there can be max 4000 entities,
   //stored from file offset 0 up to 3999 * 24 + 24
   std::vector<uint8_t>::iterator startIt;
   std::vector<uint8_t>::iterator endIt;

   startIt = bytesVec->begin();
   endIt = bytesVec->begin() + 3999 * 24 + 24;

   std::fill(startIt, endIt, 0);

   //remove all column definitions
   //there can be max 1024 of them
   //stored from file offset 98012 up to 124636
   startIt = bytesVec->begin() + 98012;
   endIt = bytesVec->begin() + 98012 + 1023 * 26 + 26;

   std::fill(startIt, endIt, 0);

   //remove all block definitions
   //there can be max 1024 of them
   //stored from file offset 124636 up to 141020
   startIt = bytesVec->begin() + 124636;
   endIt = bytesVec->begin() + 124636 + 1023 * 16 + 16;

   std::fill(startIt, endIt, 0);

   //removes all region definitions
   //there can be max 8 of them
   //stored from file offset 246924 up to 247604
   startIt = bytesVec->begin() + 246924;
   endIt = bytesVec->begin() + 247604;

   std::fill(startIt, endIt, 0);

   //setup all tiles to the same texture and height
   //there are 256 x 160 tiles;
   //Tile data is from offset 404620 up to offset 896140

   //Each tile has the following layout:

   //each map entry is 12 bytes long
   //Byte 0:  Cell Illumination value: This value controls how well illuminated a cell is
   //Byte 1:  Cell Illumination value: This value controls how well illuminated a cell is
   //Byte 2:  Height
   //Byte 3:  Height
   //Byte 4:  cid (cell id)
   //Byte 5:  cid (cell id)
   //Byte 6:  Point of Interest
   //Byte 7:  Point of Interest
   //Byte 8:  Reserved 1 (seems to be not used)
   //Byte 9:  Reserved 1 (seems to be not used)
   //Byte 10:  Texture Modification
   //Byte 11:  Reserved 2 (seems to be not used)

   //Reserved 1 & Reserved 2: For both values I checked in every level of the original
   //game. Is not a single time non zero. Was maybe reserved for
   //a future expansion, and never used (maybe reserved).
   startIt = bytesVec->begin() + 404620;
   endIt = bytesVec->begin() + 896140;

   //first fill all tiles with all 0 bytes
   std::fill(startIt, endIt, 0);

   // entry is 12 bytes long, map is at end of file
   int numBytes = 12 * LEVELFILE_WIDTH * LEVELFILE_HEIGHT;

   int i = (int)(fileSize) - numBytes;

   for (int y = 0; y < LEVELFILE_HEIGHT; y++) {
    for (int x = 0; x < LEVELFILE_WIDTH; x++) {
        //set all cells to default Illumination value of 9600
        startIt = bytesVec->begin() + i;
        (*startIt) = 128;

        startIt = bytesVec->begin() + i + 1;
        (*startIt) = 37;

        //set all cells to height 8.0f
        startIt = bytesVec->begin() + i + 2;
        (*startIt) = 0;

        startIt = bytesVec->begin() + i + 3;
        (*startIt) = 8;

        //set all cells to first TextureId 0
        startIt = bytesVec->begin() + i + 4;
        (*startIt) = 0;

        startIt = bytesVec->begin() + i + 5;
        (*startIt) = 0;

        i += 12;
   }
  }

  //write modified data to output map file
  std::ofstream outputfile(outputFileName.c_str(), std::ios::out|std::ios::binary);
  std::copy(bytesVec->cbegin(), bytesVec->cend(),
         std::ostream_iterator<uint8_t>(outputfile));

  outputfile.close();

  delete bytesVec;

  return true;
}

Editor::Editor() {
    //allocate memory for current editor statusbar text
    mCurrentStatusBarText = new wchar_t[400];
    swprintf(mCurrentStatusBarText, 390, L"");

    mRegisteredNumberEditBoxes.clear();

    mUiConversion = new UiConversion(this);
    mFontManager = new FontManager();
}

Editor::~Editor() {
    //cleanup background images
    if (backgnd != nullptr) {
        backgnd->drop();
        backgnd = nullptr;
    }

    if (gameTitle != nullptr) {
        gameTitle->drop();
        gameTitle = nullptr;
    }

    if (raceLoadingScr != nullptr) {
        raceLoadingScr->drop();
        raceLoadingScr = nullptr;
    }

    if (mCurrentStatusBarText != nullptr) {
           delete[] mCurrentStatusBarText;
    }

    if (mUiConversion != nullptr) {
        delete mUiConversion;
        mUiConversion = nullptr;
    }

    if (mFontManager != nullptr) {
        delete mFontManager;
        mFontManager = nullptr;
    }
}
