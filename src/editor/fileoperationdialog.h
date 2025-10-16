/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef FILEOPERATIONDIALOG_H
#define FILEOPERATIONDIALOG_H

#include "irrlicht.h"
#include "../editor.h"

#define FILEOP_MODE_UNDEFINED 0
#define FILEOP_MODE_LOAD 1
#define FILEOP_MODE_SAVEAS 2
#define FILEOP_MODE_SAVEAS_NEW 3

/************************
 * Forward declarations *
 ************************/

class Editor;

/* GUI Elements for the FileOperationDialog
*/
struct GUIFileOperationDialog
{
    GUIFileOperationDialog ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUITabControl* tabCntrl;
    irr::gui::IGUITab* GameLevelTab;
    irr::gui::IGUITab* CustomLevelTab;

    irr::gui::IGUITable* GameLevelTable;
    irr::gui::IGUITable* CustomLevelTable;

    irr::gui::IGUIImage* CurrentlySelectedLevelMinimap;

    irr::gui::IGUIButton* TriggerOperationButton;
    irr::gui::IGUIButton* CancelButton;
    irr::gui::IGUIButton* NewButton;

    irr::gui::IGUIStaticText* LabelCurrentSelLevelInfo;
    irr::gui::IGUIStaticText* LabelNoMiniMapAvailable;

    irr::gui::IGUIStaticText* LabelNewLevelName;
    irr::gui::IGUIEditBox* NewLevelNameEditBox = nullptr;
};

class FileOperationDialog {
public:
    FileOperationDialog(Editor* parentEditor);
    ~FileOperationDialog();

     GUIFileOperationDialog mGUIFileOperationDialog;

     void SelectLevelForLoad();
     void SelectLevelForSaveAs();

     void OnTableSelected(irr::s32 elementId);
     void OnTabChanged(irr::s32 elementId);
     void OnButtonClicked();

     //Checks if entered new level name is valid
     //if so this command returns true and the new level save information
     //is prepared inside this class to be derived later
     //If the new level name is not valid then method returns false,
     //and the file operation dialog is not closed; The issue is described in
     //return parameter string result
     bool OnNewButtonClicked(std::wstring& result);

     irr::u8 GetCurrentFileOperationMode();

     LevelFolderInfoStruct* GetCurrentSelectedLevel();
    
private:
    Editor* mParentEditor = nullptr;

    irr::gui::IGUIWindow* Window = nullptr;
    irr::video::ITexture* mCurrMinimap = nullptr;

    void CreateWindow();
    void UpdateLevelTables();

    //Returns nullptr if selected table entry can not be found
    LevelFolderInfoStruct* IdentifySelectedLevel(std::vector<LevelFolderInfoStruct*> whichLevelVector,
                                                 irr::core::stringw tableText);

    LevelFolderInfoStruct* mCurrLevelSelected = nullptr;

    irr::u8 mFileOpMode = FILEOP_MODE_UNDEFINED;

    std::vector<irr::video::ITexture*> mLoadedTextures;

    void CleanupLoadedMinimapTextures();
};

#endif // FILEOPERATIONDIALOG_H
