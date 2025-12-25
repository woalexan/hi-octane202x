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
#include "draw/attribution.h"
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
#include "editor/fileoperationdialog.h"
#include "resources/readgamedata/preparedata.h"
#include "input/input.h"
#include "font/font_manager.h"
#include <fstream>
#include "utils/fileutils.h"
#include "utils/tiny-process-library/process.hpp"
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

void Editor::CleanupExistingLevelData() {
    std::vector<LevelFolderInfoStruct*>::iterator it;
    LevelFolderInfoStruct* pntr;

    if (mGameLevelVec.size() > 0) {
        for (it = mGameLevelVec.begin(); it != mGameLevelVec.end(); ++it) {
            pntr = (*it);

            //delete the struct itself
            delete pntr;
        }
    }

    if (mCustomLevelVec.size() > 0) {
        for (it = mCustomLevelVec.begin(); it != mCustomLevelVec.end(); ++it) {
            pntr = (*it);

            //delete the struct itself
            delete pntr;
        }
    }
}

void Editor::FindExistingLevels() {
    //first cleanup possible existing data
    CleanupExistingLevelData();

    //search existing levels of original game
    //false parameter as this are not custom levels!
    GetExistingLevelInfo(std::string("extract"), false, mGameLevelVec);

    //search existing custom levels
    //true parameter to mark found levels as user custom levels
    GetExistingLevelInfo(std::string("userdata/levels"), true, mCustomLevelVec);
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

//creates the most basic editor infrastructure, and
//extracts basic things to be able to show a first
//graphical screen
bool Editor::InitEditorStep1() {
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
    mTestMenu = nullptr;
    mInfoMenu = nullptr;

    irr::u32 idx;

    //file menue is always existing
    idx = mMenu->addItem(L"File", -1, true, true);
    mFileMenu = mMenu->getSubMenu(idx);

    //Edit/Mode and View only when currently an
    //EditSession is open
    if (mCurrentSession != nullptr) {
        idx = mMenu->addItem(L"Edit", -1, true, true);
        mEditMenu = mMenu->getSubMenu(idx);

        idx = mMenu->addItem(L"Mode", -1, true, true);
        mModeMenu = mMenu->getSubMenu(idx);

        idx = mMenu->addItem(L"View", -1, true, true);
        mViewMenu = mMenu->getSubMenu(idx);

        idx = mMenu->addItem(L"Test", -1, true, true);
        mTestMenu = mMenu->getSubMenu(idx);

        idx = mMenu->addItem(L"Info", -1, true, true);
        mInfoMenu = mMenu->getSubMenu(idx);
    } else {
        idx = mMenu->addItem(L"Info", -1, true, true);
        mInfoMenu = mMenu->getSubMenu(idx);

        mEditMenu = nullptr;
        mModeMenu = nullptr;
        mViewMenu = nullptr;
        mTestMenu = nullptr;
    }

    PopulateFileMenueEntries();

    if (mCurrentSession != nullptr) {
        PopulateEditMenueEntries();
        PopulateModeMenueEntries();
        PopulateViewMenueEntries();
        PopulateTestMenueEntries();
    }

    PopulateInfoMenueEntries();

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

        submenu = mFileMenu->getSubMenu(0);
        if (submenu == nullptr)
            return;

        submenu->addItem(L"Rock", GUI_ID_NEWEMPTYLEVEL_ROCK);
        submenu->addItem(L"Vegetation", GUI_ID_NEWEMPTYLEVEL_VEGETATION);
        submenu->addItem(L"Snow", GUI_ID_NEWEMPTYLEVEL_SNOW);
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

    /*************************************
     * Edit View                         *
     *************************************/
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

void Editor::PopulateTestMenueEntries() {
    if (mTestMenu == nullptr)
        return;

    /*************************************
     * Submenue Test                     *
     *************************************/

    mTestMenu->addItem(L"Modify Map File", GUI_ID_TEST_MODIFYMAPFILE, true, false);

    mTestMenu->addItem(L"No Cpu Players", GUI_ID_TEST_ADDNOCPUPLAYERS, true, false, true, true);
    if (mTestMapNoCpuPlayer) {
       mTestMenu->setItemChecked(1, true);
    } else {
        mTestMenu->setItemChecked(1, false);
    }

    mTestMenu->addItem(L"Test in hi-octane20xx", GUI_ID_TEST_TESTHIOCTANCE20XX, true, false);
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

    gui::IGUIContextMenu* submenu;

    submenu = mViewMenu->getSubMenu(9);
    submenu->addItem(L"Off", GUI_ID_VIEW_TERRAIN_OFF);
    submenu->addItem(L"Wireframe", GUI_ID_VIEW_TERRAIN_WIREFRAME);
    submenu->addItem(L"Default", GUI_ID_VIEW_TERRAIN_DEFAULT);
    submenu->addItem(L"Normals", GUI_ID_VIEW_TERRAIN_NORMALS);

    submenu = mViewMenu->getSubMenu(10);
    submenu->addItem(L"Off", GUI_ID_VIEW_BLOCKS_OFF);
    submenu->addItem(L"Wireframe", GUI_ID_VIEW_BLOCKS_WIREFRAME);
    submenu->addItem(L"Default", GUI_ID_VIEW_BLOCKS_DEFAULT);
    submenu->addItem(L"Normals", GUI_ID_VIEW_BLOCKS_NORMALS);
}

void Editor::PopulateInfoMenueEntries() {
    if (mInfoMenu == nullptr)
        return;

    /*************************************
     * Submenue Info                     *
     *************************************/

    if (mCurrentSession != nullptr) {
        mInfoMenu->addItem(L"Write table", GUI_ID_INFO_WRITETABLE, true, true, false, false);

        gui::IGUIContextMenu* submenu;

        submenu = mInfoMenu->getSubMenu(0);
        submenu->addItem(L"Cells", GUI_ID_INFO_WRITETABLE_CELLS);
        submenu->addItem(L"Entities", GUI_ID_INFO_WRITETABLE_ENTITIES);
        submenu->addItem(L"Block Definitions", GUI_ID_INFO_WRITETABLE_BLOCKDEF);
        submenu->addItem(L"Column Definitions", GUI_ID_INFO_WRITETABLE_COLUMNDEF);
        submenu->addItem(L"Columns", GUI_ID_INFO_WRITETABLE_COLUMNS);
        submenu->addItem(L"Regions", GUI_ID_INFO_WRITETABLE_REGIONS);
    }

    mInfoMenu->addItem(L"Attribution", GUI_ID_INFO_ATTRIBUTION, true, false);
    mInfoMenu->addItem(L"About", GUI_ID_INFO_ABOUT, true, false);
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
    bool visible = mCurrentSession->mEntityManager->IsVisible(whichEntityClass);

    s32 idxMenuePnt = mViewMenu->findItemWithCommandId(commandIdMenueEntry, 0);

    if (idxMenuePnt == -1) {
        //menue entry not found
        return;
    }

    mViewMenu->setItemChecked(idxMenuePnt, visible);
}

void Editor::UpdateEntityVisibilityMenueEntries() {
    if (mCurrentSession == nullptr)
        return;

    if (mCurrentSession->mEntityManager == nullptr)
        return;

    if (mViewMenu == nullptr)
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

void Editor::OpenLevel() {
    if (mSelLevelForFileOperation == nullptr)
        return;

    bool success =
            CreateNewEditorSession(std::string(mSelLevelForFileOperation->levelBaseDir.c_str()), mSelLevelForFileOperation->levelName);

    if (!success) {
        mCurrLevelWhichIsEdited = nullptr;

        mEditorState = DEF_EDITORSTATE_ERROR;
    } else {
        //We first need to update the Menue-Entries
        //so that the next command does not crash with nullptr
        //reference
        UpdateMenueEntries();

        //update current visible Entities
        UpdateEntityVisibilityMenueEntries();

        mCurrLevelWhichIsEdited = mSelLevelForFileOperation;
        mCurrentSession->UpdateAssignedLevelInfoText();

        mEditorState = DEF_EDITORSTATE_SESSIONACTIVE;
    }
}

//Returns true in case of success, False otherwise
bool Editor::SaveAsLevel(bool saveAsNewLevel) {
    if (mSelLevelForFileOperation == nullptr)
        return false;

    std::string currOpenLevelRootDir = mCurrentSession->mLevelRootPath;
    std::string targetLevelRootDir("");

    targetLevelRootDir.append(mSelLevelForFileOperation->levelBaseDir.c_str());

    if (saveAsNewLevel) {
        //create the level base dir if not existing
        if (!PrepareCustomLevelDirectory(mSelLevelForFileOperation->levelName)) {
            //user level folder preparation failed
            return false;
        }
    }

    //first copy all level terrain textures
    //from current level to target level
    bool success = CopyLevelTextures(currOpenLevelRootDir, targetLevelRootDir);

    if (!success) {
        logging::Error("SaveAsLevel: Level terrain texture copy operation failed");
    } else {
        logging::Info("SaveAsLevel: Level terrain texture copy operation was succesfull");
    }

    //Save the level file itself as well
    success = success && mCurrentSession->SaveAs(targetLevelRootDir, mSelLevelForFileOperation->levelName);

    //copy minimap file as well if it is existing already
    irr::io::path miniMapNameTarget = GetMiniMapFileName(mSelLevelForFileOperation);
    irr::io::path miniMapNameSource = GetMiniMapFileName(currOpenLevelRootDir);

    if (FileExists(miniMapNameSource.c_str()) == 1) {
        //minimap is also there, copy
        logging::Info("SaveAsLevel: Copy also existing minimap image file");
        if (copy_file(miniMapNameSource.c_str(), miniMapNameTarget.c_str()) != 0) {
            success = false;

            logging::Error("SaveAsLevel: Minimap file copy operation failed");
        } else {
            logging::Info("SaveAsLevel: Minimap file copy operation was succesfull");
        }
    }

    //copy map config file as well if it is existing already
    irr::io::path mapConfigFileTarget = GetMapConfigFileName(mSelLevelForFileOperation);
    irr::io::path mapConfigFileSource = GetMapConfigFileName(currOpenLevelRootDir);

    if (FileExists(mapConfigFileSource.c_str()) == 1) {
        //map config file is also there, copy
        logging::Info("SaveAsLevel: Copy also existing mapconfig file");
        if (copy_file(mapConfigFileSource.c_str(), mapConfigFileTarget.c_str()) != 0) {
            success = false;

            logging::Error("SaveAsLevel: Map config file copy operation failed");
        } else {
            logging::Info("SaveAsLevel: Map config file copy operation was succesfull");
        }
    }

    return success;
}

void Editor::TestMapinHioctance20XX() {
    cout << "Call to hi-octance20XX" << endl;
    std::string cmdStr("");
    cmdStr.append("./hi-octane202x ");

    //Test command line option is used to directly load a
    //level in hi-octane202x
    if (mTestMapNoCpuPlayer) {
      //if we do not want to include cpu players
      //simply state option nocpu
      cmdStr.append("nocpu ");
    }

    cmdStr.append("test ");
    cmdStr.append(mCurrentSession->mLevelRootPath);
    cmdStr.erase(cmdStr.begin() + cmdStr.size() - 1);

    TinyProcessLib::Process hiprocess(cmdStr.c_str(), "", [](const char *bytes, size_t n) {
      cout << "Output from stdout: " << std::string(bytes, n);
    }, [](const char *bytes, size_t n) {
      cout << "Output from stderr: " << std::string(bytes, n);
      //add a newline for prettier output on some platforms:
      if(bytes[n-1]!='\n')
        cout << endl;
    });
    auto exit_status=hiprocess.get_exit_status();
    cout << "hi-octane202x returned: " << exit_status << " (" << (exit_status==0?"success":"failure") << ")" << endl;
    //this_thread::sleep_for(chrono::seconds(5));
}

void Editor::TestMapModification() {
  //Commented out, only for testing purposes
  //  ModifyMapFile(std::string("extract/level0-1/level0-1-unpacked.dat"), std::string("extract/level0-1/LEVEL0-1-step2-nocolumns.DAT"));
}

void Editor::DebugPrintTable(irr::s32 id) {
    if (mCurrentSession == nullptr)
        return;

    std::string basePath("");

    if (mCurrLevelWhichIsEdited != nullptr) {
        basePath.append(mCurrLevelWhichIsEdited->levelBaseDir.c_str());
    }

    switch (id) {
        case GUI_ID_INFO_WRITETABLE_CELLS: {
           std::string finalName("");
           finalName.append(basePath);
           finalName.append("table-cells.csv");
           mCurrentSession->mLevelRes->DebugWriteCellInfoToCsvFile(finalName.c_str());
           break;
        }

        case GUI_ID_INFO_WRITETABLE_ENTITIES: {
            std::string finalName("");
            finalName.append(basePath);
            finalName.append("table-entities.csv");
            mCurrentSession->mEntityManager->DebugWriteEntityTableToCsvFile(finalName.c_str());
            break;
        }

        case GUI_ID_INFO_WRITETABLE_BLOCKDEF: {
            std::string finalName("");
            finalName.append(basePath);
            finalName.append("table-blockdefinitions.csv");
            mCurrentSession->mLevelBlocks->DebugWriteBlockDefinitionTableToCsvFile(finalName.c_str());
            break;
        }

        case GUI_ID_INFO_WRITETABLE_COLUMNDEF: {
            std::string finalName("");
            finalName.append(basePath);
            finalName.append("table-columndefinitions.csv");
            mCurrentSession->mLevelBlocks->DebugWriteColumnDefinitionTableToCsvFile(finalName.c_str());
            break;
        }

        case GUI_ID_INFO_WRITETABLE_COLUMNS: {
            std::string finalName("");
            finalName.append(basePath);
            finalName.append("table-columns.csv");
            mCurrentSession->mLevelBlocks->DebugWriteDefinedColumnsTableToCsvFile(finalName.c_str());
            break;
        }

        case GUI_ID_INFO_WRITETABLE_REGIONS: {
            std::string finalName("");
            finalName.append(basePath);
            finalName.append("table-regions.csv");
            mCurrentSession->mLevelRes->DebugPrintRegionTable(finalName.c_str());
            break;
        }
    }
}

void Editor::OnMenuItemSelected( IGUIContextMenu* menu )
{
    s32 id = menu->getItemCommandId(menu->getSelectedItem());

    switch(id)
    {
        case GUI_ID_INFO_ATTRIBUTION: {
            StartAttribution();
            break;
        }

        case GUI_ID_INFO_ABOUT: {
            ShowAboutWindow();
            break;
        }

        case GUI_ID_INFO_WRITETABLE_CELLS:
        case GUI_ID_INFO_WRITETABLE_ENTITIES:
        case GUI_ID_INFO_WRITETABLE_BLOCKDEF:
        case GUI_ID_INFO_WRITETABLE_COLUMNDEF:
        case GUI_ID_INFO_WRITETABLE_COLUMNS:
        case GUI_ID_INFO_WRITETABLE_REGIONS: {
            DebugPrintTable(id);
        }

        case GUI_ID_TEST_MODIFYMAPFILE: {
           TestMapModification();
           break;
        }

        case GUI_ID_TEST_TESTHIOCTANCE20XX: {
           TestMapinHioctance20XX();
           break;
        }

        case GUI_ID_TEST_ADDNOCPUPLAYERS: {
           mTestMapNoCpuPlayer = !mTestMapNoCpuPlayer;
           break;
        }

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

        case GUI_ID_OPEN_LEVEL: {
            if (mCurrentSession == nullptr) {
                //mFileOperationDialog = new FileOperationDialog(this);
                mFileOperationDialog->SelectLevelForLoad();
            }

            break;
        }

        case GUI_ID_SAVE_LEVEL: {
            if (mCurrentSession != nullptr) {
                //is there already a level file assigned, if so simply save to it
                if (mCurrLevelWhichIsEdited != nullptr) {
                    //save to the current existing level file
                    mCurrentSession->mLevelRes->Save(mCurrLevelWhichIsEdited->levelFileName.c_str());
                } else {
                    //no level file assigned yet, this is our first save
                    //Therefore we need to execute SaveAs
                    mFileOperationDialog->SelectLevelForSaveAs();
                }
            }
            break;
        }

        case GUI_ID_SAVEAS_LEVEL: {
            if (mCurrentSession != nullptr) {
                mFileOperationDialog->SelectLevelForSaveAs();
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
    }
}

void Editor::TriggerFileOperation(irr::s32 elementId) {
    if (elementId == GUI_ID_FILEOPERATIONDIALOG_TRIGGEROPERATIONBUTTON) {
        //Which Operation should we trigger
        if (mFileOperationDialog->GetCurrentFileOperationMode() == FILEOP_MODE_LOAD) {
            mSelLevelForFileOperation = mFileOperationDialog->GetCurrentSelectedLevel();

            mEditorState = DEF_EDITORSTATE_LOADDATA;
            return;
        }

        if (mFileOperationDialog->GetCurrentFileOperationMode() == FILEOP_MODE_SAVEAS) {
            mSelLevelForFileOperation = mFileOperationDialog->GetCurrentSelectedLevel();

            mEditorState = DEF_EDITORSTATE_SAVEAS_OVERWRITE_CONFIRM;
            return;
        }
    } else if (elementId == GUI_ID_FILEOPERATIONDIALOG_NEWBUTTON) {
        if (mFileOperationDialog->GetCurrentFileOperationMode() == FILEOP_MODE_SAVEAS_NEW) {
            mSelLevelForFileOperation = mFileOperationDialog->GetCurrentSelectedLevel();

            mEditorState = DEF_EDITORSTATE_SAVEAS_NEW;

            //add new future level to list of existing
            //user custom levels
            this->mCustomLevelVec.push_back(mSelLevelForFileOperation);

            return;
        }
    }
}

void Editor::OnButtonClicked(irr::s32 buttonId) {
    //is this an event for the FileOperationsDialog?
    //first check if user wants to save Level as a new level
    //for this we need to check if the user has supplied a correct
    //new level name
    if (buttonId == GUI_ID_FILEOPERATIONDIALOG_NEWBUTTON) {
             std::wstring result;

             //Next command checks if entered new level name is valid
             //if so this command returns true, and returns the new level name as
             //a parameter; If the new level name is not valid then returns false,
             //and the file operation dialog is not closed
             if (!mFileOperationDialog->OnNewButtonClicked(result)) {
                 //no valid name
                  mGuienv->addMessageBox(L"Error", result.c_str(), true, EMBF_OK, nullptr, -1, nullptr);

                  return;
             }

             //we have a valid new name
             //next command simply hides the file operation dialog again
             mFileOperationDialog->OnButtonClicked();

             TriggerFileOperation(buttonId);

             return;
    }

    if ((buttonId == GUI_ID_FILEOPERATIONDIALOG_TRIGGEROPERATIONBUTTON) ||
        (buttonId == GUI_ID_FILEOPERATIONDIALOG_CANCELBUTTON)) {
             //next command simply hides the file operation dialog again
             mFileOperationDialog->OnButtonClicked();

             TriggerFileOperation(buttonId);

             return;
    }

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

   //was a table in the FileOperationDialog selected?
   if ((elementId == GUI_ID_FILEOPERATIONDIALOG_GAMELEVELTABLE) ||
       (elementId == GUI_ID_FILEOPERATIONDIALOG_CUSTOMLEVELTABLE)) {
       mFileOperationDialog->OnTableSelected(elementId);

       return;
   }

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

      case GUI_ID_EDITOR_MSGBOX_CONFIRM_OVERWRITE: {
         //user pressed yes in the Messagebox for confirming
         //overwriting existing level => we want to trigger SaveAs
         //operation
         mEditorState = DEF_EDITORSTATE_OVERWRITE_EXISTING_LEVEL;
         break;
      }
  }
}

void Editor::OnMessageBoxNo(irr::s32 elementId) {
  switch(elementId) {
      case GUI_ID_EDITOR_MSGBOX_CONFIRM_OVERWRITE: {
         //user pressed no in the Messagebox for confirming
         //overwriting existing level => we want to cancel the SaveAs
         //operation
         mEditorState = DEF_EDITORSTATE_SESSIONACTIVE;
         break;
      }
  }
}

void Editor::OnTabChanged(irr::s32 elementId) {
  switch(elementId) {
      case GUI_ID_FILEOPERATIONDIALOG_TABCNTRL: {
        mFileOperationDialog->OnTabChanged(elementId);
        break;
      }
  }
}

void Editor::ShowAboutWindow() {
    mGuienv->addMessageBox(L"About Hi-Editor", L"Hi-Octane Level editor, Contributors: Wolf Alexander (woalexan), mbillingr",
                       true, EMBF_OK, 0, -1, nullptr);
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

void Editor::OnRightMouseButtonDown() {
    MouseState.RightButtonDown = true;

    if (mCurrentSession != nullptr) {
        if (mCurrentSession->mEditorMode != nullptr) {
            mCurrentSession->mEditorMode->OnRightMouseButtonDown();
        }
    }
}

void Editor::OnRightMouseButtonUp() {
    MouseState.RightButtonDown = false;
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

      case EMIE_RMOUSE_PRESSED_DOWN:
            OnRightMouseButtonDown();
            break;

      case EMIE_RMOUSE_LEFT_UP:
            OnRightMouseButtonUp();
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

        case EGET_MESSAGEBOX_NO: {
            //User pressed no in a MessageBox
            OnMessageBoxNo(id);
            break;
        }

        case EGET_TAB_CHANGED: {
            OnTabChanged(id);
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

    //skip the next commands if we start an attribution
    //during EditorSession. So that we do not mess something
    //up in the background
    if (mEditorState != DEF_EDITORSTATE_ATTRIBUTION) {
        mCurrentSession->AdvanceTime(frameDeltaTime);

        mCurrentSession->HandleBasicInput();
        mCurrentSession->TrackActiveDialog();
    }

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

void Editor::StartAttribution() {
    if (mAttribution == nullptr)
        return;

    irr::u8 state = mAttribution->GetState();

    if (state == DEF_ATTR_STATE_PRESENTING) {
        return;
    }

    if (state == DEF_ATTR_STATE_UNINITIALIZED) {
       mAttribution->Init();
    }

    state = mAttribution->GetState();

    if (state == DEF_ATTR_STATE_INITERROR) {
        return;
    }

    if ((state != DEF_ATTR_STATE_READY) && (state != DEF_ATTR_STATE_PRESENTATIONDONE)) {
        return;
    }

    if (mCurrentSession != nullptr) {
        if (mCurrentSession->mEditorMode != nullptr) {
            mCurrentSession->mEditorMode->HideWindow();
        }

        //Hide additional Ui elements that would
        //disturb attribution
        mCurrentSession->HideUIElements();
    }

    //Remember/Store the current (initial) editor
    //state. So that we can restore it later again
    mInitialEditorState = mEditorState;

    mEditorState = DEF_EDITORSTATE_ATTRIBUTION;

    //deactivate main menue
    //hide everything that would disturb
    mMenu->setEnabled(false);
    mMenu->setVisible(false);

    StatusLine->setEnabled(false);
    StatusLine->setVisible(false);

    noEditorSessionText->setEnabled(false);
    noEditorSessionText->setVisible(false);

    mAttribution->SetScrollSpeed(2);
    mAttribution->SetFadingParameters(200, 0.0f, 1.0f);
    mAttribution->Start();
}

void Editor::AttributionEnded() {
    if (mCurrentSession != nullptr) {
        if (mCurrentSession->mEditorMode != nullptr) {
            mCurrentSession->mEditorMode->ShowWindow();
        }

        mCurrentSession->UnhideUIElements();
    }

    mMenu->setEnabled(true);
    mMenu->setVisible(true);

    StatusLine->setEnabled(true);
    StatusLine->setVisible(true);

    if (mCurrentSession == nullptr) {
        noEditorSessionText->setEnabled(true);
        noEditorSessionText->setVisible(true);
    }

    //restore the initial editor state again
    mEditorState = mInitialEditorState;
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

            case DEF_EDITORSTATE_ATTRIBUTION: {
                mAttribution->Update(frameDeltaTime);

                if (mAttribution->GetState() == DEF_ATTR_STATE_PRESENTATIONDONE) {
                    //presentation is finished
                    AttributionEnded();
                }

                //Does user want to interrupt the presentation?
                 if(mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
                     mAttribution->Stop();

                     AttributionEnded();

                     //Note: Keep this break here, it is important!
                     break;
                 }

                //Important note 09.11.2025: Never ever put
                //a break here; It is not here by purpose, I want to
                //fall through to DEF_EDITORSTATE_NOSESSIONACTIVE case below!
            }
            case DEF_EDITORSTATE_NOSESSIONACTIVE: {
                //ParentEditor is ready for a session,
                //but no session is loaded yet
                EditorLoopNoSessionOpen();
                break;
            }

            case DEF_EDITORSTATE_LOADDATA: {
                //load level
                OpenLevel();
                break;
            }

            case DEF_EDITORSTATE_SAVEAS_NEW: {
                //save as new level
                //Parameter true = Save as new level
                if (!SaveAsLevel(true)) {
                    mGuienv->addMessageBox(L"Error", L"Level SaveAs Operation Failed", true, EMBF_OK, nullptr, -1, nullptr);
                    mCurrLevelWhichIsEdited = nullptr;
                } else {
                    mCurrLevelWhichIsEdited = mSelLevelForFileOperation;
                }

                mCurrentSession->UpdateAssignedLevelInfoText();

                mEditorState = DEF_EDITORSTATE_SESSIONACTIVE;
                break;
            }

            //Important note: We need to call the method EditorLoopSession for state
            //DEF_EDITORSTATE_SAVEAS_OVERWRITE_CONFIRM_WAIT too, otherwise Irrlicht
            //engine will get stuck in this state as no rendering etc.. occurs anymore
            case DEF_EDITORSTATE_SAVEAS_OVERWRITE_CONFIRM_WAIT:
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

                mCurrLevelWhichIsEdited = nullptr;

                break;
            }

            case DEF_EDITORSTATE_CREATENEWEMPTYLEVEL: {
                bool success;

                if (CreateNewEmptyLevel(mNewLevelStyleSelector, "newlevel")) {
                    //new level creation was succesfull
                    logging::Info("New level creation was succesfull");

                    //now load the new level
                    success = CreateNewEditorSession("userdata/levels/newlevel/", "newlevel");

                    if (!success) {
                        mEditorState = DEF_EDITORSTATE_NOSESSIONACTIVE;
                        logging::Error("Failed to load new level");
                        mGuienv->addMessageBox(L"Error", L"Failed to load new level", true, EMBF_OK, nullptr, -1, nullptr);
                    } else {
                        //We first need to update the Menue-Entries
                        //so that the next command does not crash with nullptr
                        //reference
                        UpdateMenueEntries();

                        //update current visible Entities
                        UpdateEntityVisibilityMenueEntries();

                        mEditorState = DEF_EDITORSTATE_SESSIONACTIVE;
                        mCurrentSession->UpdateAssignedLevelInfoText();
                    }
                } else {
                    mGuienv->addMessageBox(L"Error", L"New level creation failed", true, EMBF_OK, nullptr, -1, nullptr);
                    mEditorState = DEF_EDITORSTATE_NOSESSIONACTIVE;
                }

                break;
            }

            case DEF_EDITORSTATE_SAVEAS_OVERWRITE_CONFIRM: {
                mGuienv->addMessageBox(L"Confirm", L"Are you sure you want to overwrite the selected level?", true, EMBF_YES + EMBF_NO,
                                       nullptr, GUI_ID_EDITOR_MSGBOX_CONFIRM_OVERWRITE, nullptr);

                //Important note: We need to call the method EditorLoopSession here too, otherwise Irrlicht
                //engine will get stuck as no rendering etc.. occurs anymore
                EditorLoopSession(frameDeltaTime);
                mEditorState = DEF_EDITORSTATE_SAVEAS_OVERWRITE_CONFIRM_WAIT;
                break;
            }

            case DEF_EDITORSTATE_OVERWRITE_EXISTING_LEVEL: {
                if (!SaveAsLevel()) {
                    mGuienv->addMessageBox(L"Error", L"Level SaveAs Operation Failed", true, EMBF_OK, nullptr, -1, nullptr);
                }

                mCurrentSession->UpdateAssignedLevelInfoText();
                mEditorState = DEF_EDITORSTATE_SESSIONACTIVE;
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

//Returns true in case of success, False otherwise
bool Editor::CreateDefaultMapConfigFile(std::string targetMapFolder) {
   irr::io::path configFilePath = GetMapConfigFileName(targetMapFolder);

    MapConfigStruct newConfig;

    //Default use sky of first level, and music of first level
    newConfig.SkyImageFileVanilla.append("extract/sky/modsky0-0.png");
    newConfig.SkyImageFileUpgradedSky.append("media/sky/skydome.tga");
    newConfig.MusicFile.append("extract/music/TGAME1.XMI");

    newConfig.EnableLensFlare = false;
    newConfig.lensflareLocation.set(0.0f,0.0f,0.0f);
    newConfig.cloudColorCenter1.set(220, 220, 220, 220);
    newConfig.cloudColorInner1.set(180, 180, 180, 180);
    newConfig.cloudColorOuter1.set(0, 0, 0, 0);

    newConfig.cloudColorCenter2.set(220, 220, 220, 220);
    newConfig.cloudColorInner2.set(180, 180, 180, 180);
    newConfig.cloudColorOuter2.set(0, 0, 0, 0);

    newConfig.cloudColorCenter3.set(220, 220, 220, 220);
    newConfig.cloudColorInner3.set(180, 180, 180, 180);
    newConfig.cloudColorOuter3.set(0, 0, 0, 0);

    //default minimap not set, not configured
    newConfig.minimapCalSet = false;
    newConfig.minimapCalStartVal.set(0, 0);
    newConfig.minimapCalEndVal.set(0, 0);

    if (!WriteMapConfigFile(std::string(configFilePath.c_str()), &newConfig)) {
        return false;
    }

    return true;
}

//Returns true if succesfull, False otherwise
//Note: outputMapName is not a folder path, instead a single word (name)
//for the map
bool Editor::PrepareCustomLevelDirectory(std::string outputMapName) {
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

    return true;
}

//Returns true if succesfull, False otherwise
//Note: outputMapName is not a folder path, instead a single word (name)
//for the map
bool Editor::CreateNewEmptyLevel(irr::u32 newLevelStype, std::string outputMapName) {
    std::string mapSubFolder("userdata/levels/");
    mapSubFolder.append(outputMapName);

    if (!PrepareCustomLevelDirectory(outputMapName)) {
        //user level folder preparation failed
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

    //since now no level file is assigned anymore
    mCurrLevelWhichIsEdited = nullptr;

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

    //Create a default mapconfig file
    if (!CreateDefaultMapConfigFile(mapSubFolder)) {
        logging::Error("Failed to create default mapconfig.xml file!");
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

//Returns true if succesfull, False otherwise
bool Editor::ModifyMapFile(std::string originFileName, std::string outputFileName) {
    ifstream ifile;
    std::streampos fileSize;

    ifile.open(originFileName.c_str(), std::ifstream::binary);
    if(ifile) {
          logging::Info("ModifyMapFile: Origin file found and openend succesfully");
       } else {
           logging::Error("ModifyMapFile: Could not open Origin Level file");
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

   std::vector<uint8_t>::iterator startIt;
   std::vector<uint8_t>::iterator endIt;

   //first remove all entities
   //there can be max 4000 entities,
   //stored from file offset 0 up to 3999 * 24 + 24
   if (false) {
      startIt = bytesVec->begin();
      endIt = bytesVec->begin() + 3999 * 24 + 24;

      std::fill(startIt, endIt, 0);
   }

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
   if (false) {
     startIt = bytesVec->begin() + 246924;
     endIt = bytesVec->begin() + 247604;

     std::fill(startIt, endIt, 0);
   }


   if (false) {
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
   }

  //write modified data to output map file
  std::ofstream outputfile(outputFileName.c_str(), std::ios::out|std::ios::binary);
  std::copy(bytesVec->cbegin(), bytesVec->cend(),
         std::ostream_iterator<uint8_t>(outputfile));

  outputfile.close();

  delete bytesVec;

  return true;
}

Editor::Editor(int argc, char **argv) : InfrastructureBase(argc, argv, INFRA_RUNNING_AS_EDITOR) {
    //allocate memory for current editor statusbar text
    mCurrentStatusBarText = new wchar_t[400];
    swprintf(mCurrentStatusBarText, 390, L"");

    mRegisteredNumberEditBoxes.clear();

    mUiConversion = new UiConversion(this);
    mFontManager = new FontManager();
    mFileOperationDialog = new FileOperationDialog(this);
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

    if (mFileOperationDialog != nullptr) {
        delete mFileOperationDialog;
        mFileOperationDialog = nullptr;
    }

    CleanupExistingLevelData();
}
