/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "fileoperationdialog.h"
#include "../editor.h"
#include <iostream>
#include "../utils/fileutils.h"

FileOperationDialog::FileOperationDialog(Editor* parentEditor) {
    mParentEditor = parentEditor;
}

void FileOperationDialog::CreateWindow() {
    irr::core::dimension2d<irr::u32> dim ( 500, 450 );

    //finally create the window
    Window = mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"File", 0, GUI_ID_FILEOPERATIONDIALOG_WINDOWID);

    //lets create two tabs in the file operation dialog window, one for accessing game levels,
    //and the other one to access user custom levels
    mGUIFileOperationDialog.tabCntrl = mParentEditor->mGuienv->addTabControl(
            core::rect<s32>(1, 20, dim.Width - 270 , dim.Height - 2), Window, true, true, GUI_ID_FILEOPERATIONDIALOG_TABCNTRL);

    mGUIFileOperationDialog.GameLevelTab = mGUIFileOperationDialog.tabCntrl->addTab(L"Game");
    mGUIFileOperationDialog.CustomLevelTab = mGUIFileOperationDialog.tabCntrl->addTab(L"Custom");

    mGUIFileOperationDialog.GameLevelTable =
      mParentEditor->mGuienv->addTable( rect<s32>( 10, 30, 212, dim.Height - 150), mGUIFileOperationDialog.GameLevelTab, GUI_ID_FILEOPERATIONDIALOG_GAMELEVELTABLE);

    mGUIFileOperationDialog.GameLevelTable->addColumn( L"Name", 0 );

    mGUIFileOperationDialog.GameLevelTable->setColumnWidth ( 0, 200 );
    mGUIFileOperationDialog.GameLevelTable->setToolTipText ( L"Shows all existing Game levels");

    mGUIFileOperationDialog.CustomLevelTable =
      mParentEditor->mGuienv->addTable( rect<s32>( 10, 30, 212, dim.Height - 150), mGUIFileOperationDialog.CustomLevelTab, GUI_ID_FILEOPERATIONDIALOG_CUSTOMLEVELTABLE);

    mGUIFileOperationDialog.CustomLevelTable->addColumn( L"Name", 0 );

    mGUIFileOperationDialog.CustomLevelTable->setColumnWidth ( 0, 200 );
    mGUIFileOperationDialog.CustomLevelTable->setToolTipText ( L"Shows all existing user custom levels");

    mGUIFileOperationDialog.CurrentlySelectedLevelMinimap =
            mParentEditor->mGuienv->addImage(rect<s32>( 240, 55, dim.Width - 20, dim.Height - 60), Window, -1);

    mGUIFileOperationDialog.CurrentlySelectedLevelMinimap->setScaleImage(true);

    mGUIFileOperationDialog.TriggerOperationButton
            = mParentEditor->mGuienv->addButton(core::recti(240, dim.Height - 45, 350, dim.Height - 20), Window, GUI_ID_FILEOPERATIONDIALOG_TRIGGEROPERATIONBUTTON, L"");

    mGUIFileOperationDialog.TriggerOperationButton->setEnabled(false);
    mGUIFileOperationDialog.TriggerOperationButton->setVisible(false);

    mGUIFileOperationDialog.CancelButton
            = mParentEditor->mGuienv->addButton(core::recti(360, dim.Height - 45, dim.Width - 20, dim.Height - 20), Window, GUI_ID_FILEOPERATIONDIALOG_CANCELBUTTON, L"Cancel");

    mGUIFileOperationDialog.CancelButton->setEnabled(true);
    mGUIFileOperationDialog.CancelButton->setVisible(true);

    mGUIFileOperationDialog.LabelCurrentSelLevelInfo
            = mParentEditor->mGuienv->addStaticText ( L"", rect<s32>( 240, 25, dim.Width - 20, 50 ),false, false, Window, -1, false );

    mGUIFileOperationDialog.LabelNoMiniMapAvailable
            = mParentEditor->mGuienv->addStaticText ( L"No Minimap available", rect<s32>( 300, 210, dim.Width - 20, 235 ),false, false, Window, -1, false );

    mGUIFileOperationDialog.LabelNoMiniMapAvailable->setVisible(false);

    mGUIFileOperationDialog.NewLevelNameEditBox
            = mParentEditor->mGuienv->addEditBox(L"", core::recti(10, dim.Height - 45, 150, dim.Height - 20), true, Window, GUI_ID_FILEOPERATIONDIALOG_NEWLEVELNAMEEDITBOX);

    mGUIFileOperationDialog.NewLevelNameEditBox->setMultiLine(false);
    mGUIFileOperationDialog.NewLevelNameEditBox->setWordWrap(false);

    mGUIFileOperationDialog.NewLevelNameEditBox->setVisible(false);
    mGUIFileOperationDialog.NewLevelNameEditBox->setEnabled(false);

    mGUIFileOperationDialog.LabelNewLevelName
        = mParentEditor->mGuienv->addStaticText ( L"New Level name:", rect<s32>( 10, dim.Height - 75 , 110, dim.Height - 50),false, false, Window, -1, false );

    mGUIFileOperationDialog.LabelNewLevelName->setVisible(false);
    mGUIFileOperationDialog.LabelNewLevelName->setEnabled(false);

    mGUIFileOperationDialog.NewButton
            = mParentEditor->mGuienv->addButton(core::recti(165, dim.Height - 45, 215, dim.Height - 20), Window, GUI_ID_FILEOPERATIONDIALOG_NEWBUTTON, L"New");

    mGUIFileOperationDialog.NewButton->setEnabled(false);
    mGUIFileOperationDialog.NewButton->setVisible(false);

    //move window to a better start location
    Window->move(irr::core::vector2d<irr::s32>(200,200));

    Window->setVisible(false);
    Window->setEnabled(false);
}

void FileOperationDialog::SelectLevelForLoad() {
    mFileOpMode = FILEOP_MODE_LOAD;

    if (Window == nullptr) {
        CreateWindow();
    }

    mCurrLevelSelected = nullptr;
    mLoadedTextures.clear();

    mGUIFileOperationDialog.NewLevelNameEditBox->setVisible(false);
    mGUIFileOperationDialog.NewLevelNameEditBox->setEnabled(false);

    mGUIFileOperationDialog.NewButton->setEnabled(false);
    mGUIFileOperationDialog.NewButton->setVisible(false);

    mGUIFileOperationDialog.LabelNewLevelName->setVisible(false);
    mGUIFileOperationDialog.LabelNewLevelName->setEnabled(false);

    Window->setText(L"Select level to load");
    mGUIFileOperationDialog.TriggerOperationButton->setText(L"Load");

    mGUIFileOperationDialog.TriggerOperationButton->setEnabled(true);
    mGUIFileOperationDialog.TriggerOperationButton->setVisible(true);

    mGUIFileOperationDialog.CancelButton->setEnabled(true);
    mGUIFileOperationDialog.CancelButton->setVisible(true);

    //update list of available levels
    mParentEditor->FindExistingLevels();

    UpdateLevelTables();

    Window->setVisible(true);
    Window->setEnabled(true);
}

void FileOperationDialog::SelectLevelForSaveAs() {
    mFileOpMode = FILEOP_MODE_SAVEAS;

    if (Window == nullptr) {
        CreateWindow();
    }

    mCurrLevelSelected = nullptr;
    mLoadedTextures.clear();

    mGUIFileOperationDialog.NewLevelNameEditBox->setVisible(true);
    mGUIFileOperationDialog.NewLevelNameEditBox->setEnabled(true);

    mGUIFileOperationDialog.NewButton->setEnabled(true);
    mGUIFileOperationDialog.NewButton->setVisible(true);

    mGUIFileOperationDialog.LabelNewLevelName->setVisible(true);
    mGUIFileOperationDialog.LabelNewLevelName->setEnabled(true);

    Window->setText(L"Select location for SaveAs");
    mGUIFileOperationDialog.TriggerOperationButton->setText(L"SaveAs");

    mGUIFileOperationDialog.TriggerOperationButton->setEnabled(true);
    mGUIFileOperationDialog.TriggerOperationButton->setVisible(true);

    mGUIFileOperationDialog.CancelButton->setEnabled(true);
    mGUIFileOperationDialog.CancelButton->setVisible(true);

    //update list of available levels
    mParentEditor->FindExistingLevels();

    UpdateLevelTables();

    Window->setVisible(true);
    Window->setEnabled(true);
}

irr::u8 FileOperationDialog::GetCurrentFileOperationMode() {
    return (mFileOpMode);
}

LevelFolderInfoStruct* FileOperationDialog::GetCurrentSelectedLevel() {
    return (mCurrLevelSelected);
}

//Returns nullptr if selected table entry can not be found
LevelFolderInfoStruct* FileOperationDialog::IdentifySelectedLevel(std::vector<LevelFolderInfoStruct*> whichLevelVector,
                                                                  irr::core::stringw tableText) {
    std::vector<LevelFolderInfoStruct*>::iterator it;
    irr::core::stringw hlpStr(L"");

    for (it = whichLevelVector.begin(); it != whichLevelVector.end(); ++it) {
        while (!hlpStr.empty()) {
            hlpStr.erase(0);
        }

        hlpStr.append((*it)->levelName.c_str());

        //does the entry text match with the current iterator name?
        if (tableText.equals_ignore_case(hlpStr)) {
            //match found
            return (*it);
        }
    }

    //no match found
    return nullptr;
}

void FileOperationDialog::OnTableSelected(irr::s32 elementId) {
    if (elementId == GUI_ID_FILEOPERATIONDIALOG_GAMELEVELTABLE) {
        //another level file entry was selected in the
        //game level table
        irr::s32 rowSelected = mGUIFileOperationDialog.GameLevelTable->getSelected();

        if (rowSelected != -1) {
                irr::core::stringw cellTxt = mGUIFileOperationDialog.GameLevelTable->getCellText(rowSelected, 0);

                LevelFolderInfoStruct* matchPntr = IdentifySelectedLevel(mParentEditor->mGameLevelVec, cellTxt);

                mCurrLevelSelected = matchPntr;
        }
    }

    if (elementId == GUI_ID_FILEOPERATIONDIALOG_CUSTOMLEVELTABLE) {
        //another level file entry was selected in the
        //custom level table
        irr::s32 rowSelected = mGUIFileOperationDialog.CustomLevelTable->getSelected();

        if (rowSelected != -1) {
                irr::core::stringw cellTxt = mGUIFileOperationDialog.CustomLevelTable->getCellText(rowSelected, 0);

                LevelFolderInfoStruct* matchPntr = IdentifySelectedLevel(mParentEditor->mCustomLevelVec, cellTxt);

                mCurrLevelSelected = matchPntr;
        }
    }

    if (mCurrLevelSelected != nullptr) {
        bool noMinimap = false;

        irr::core::stringw newSelLevel(mCurrLevelSelected->description.c_str());
        mGUIFileOperationDialog.LabelCurrentSelLevelInfo->setText(newSelLevel.c_str());

        irr::io::path miniMapFileName = mParentEditor->GetMiniMapFileName(mCurrLevelSelected);

        if (!miniMapFileName.empty()) {
            if (FileExists(miniMapFileName.c_str()) == 1) {
                //the specified minimap file exists
                mCurrMinimap = mParentEditor->mDriver->getTexture(miniMapFileName);

                if (mCurrMinimap != nullptr) {
                    //store pointers to loaded minimap textures, so that later we can clean
                    //them up again, this is important
                    mLoadedTextures.push_back(mCurrMinimap);

                    mGUIFileOperationDialog.CurrentlySelectedLevelMinimap->setImage(mCurrMinimap);
                    mGUIFileOperationDialog.CurrentlySelectedLevelMinimap->setVisible(true);
                    mGUIFileOperationDialog.LabelNoMiniMapAvailable->setVisible(false);
                } else {
                    noMinimap = true;
                }
            } else {
                noMinimap = true;
            }
        } else {
            noMinimap = true;
        }

       if (noMinimap) {
            mGUIFileOperationDialog.CurrentlySelectedLevelMinimap->setVisible(false);
            mGUIFileOperationDialog.LabelNoMiniMapAvailable->setVisible(true);
       }
    } else {
        //no level selected
        mGUIFileOperationDialog.LabelNoMiniMapAvailable->setVisible(false);
        mGUIFileOperationDialog.CurrentlySelectedLevelMinimap->setVisible(false);
        mGUIFileOperationDialog.LabelCurrentSelLevelInfo->setText(L"");
    }
}

void FileOperationDialog::CleanupLoadedMinimapTextures() {
    std::vector<irr::video::ITexture*>::iterator it;
    irr::video::ITexture* pntr;

    if (mLoadedTextures.size() > 0) {
        for (it = mLoadedTextures.begin(); it != mLoadedTextures.end(); ++it) {
            pntr = (*it);

            //remove the texture in Irrlicht
            //from the Texture cache
            mParentEditor->mDriver->removeTexture(pntr);
        }
    }
}

//Checks if entered new level name is valid
//if so this command returns true, and returns the new level name as
//a parameter; If the new level name is not valid then returns false,
//and the file operation dialog is not closed. In case of no success the reason
//for the fail is returned in parameter newLevelname as well
bool FileOperationDialog::OnNewButtonClicked(std::wstring& result) {
   //get new selected level name text
   irr::core::stringw newName = mGUIFileOperationDialog.NewLevelNameEditBox->getText();

   mFileOpMode = FILEOP_MODE_SAVEAS_NEW;

   //new name string empty?
   if (newName.empty()) {
      result.append(L"Please enter new level name in EditBox");

      return false;
   }

   if (!mParentEditor->CheckForValidLevelName(std::wstring(newName.c_str()))) {
       //invalid new level name
       result.append(L"New level name may only contain a-z, A-Z, 0-9, _, and - character");

       return false;
   }

   //new entered name is valid, create new
   //LevelFolderInfoStruct struct
   LevelFolderInfoStruct* newStruct = new LevelFolderInfoStruct();
   newStruct->isCustomLevel = true;

   std::string newLevelName = mParentEditor->WStringToStdString(std::wstring(newName.c_str()));

   std::wstring strhlper(newName.c_str());
   newStruct->levelName = newLevelName;
   newStruct->description.append("Custom (modified)");
   newStruct->levelBaseDir.append("userdata/levels/");;
   newStruct->levelBaseDir.append(strhlper.c_str());
   newStruct->levelBaseDir.append("/");

   newStruct->levelFileName.append(newStruct->levelBaseDir);
   newStruct->levelFileName.append(strhlper.c_str());
   newStruct->levelFileName.append("-unpacked.dat");

   mCurrLevelSelected = newStruct;

   return true;
}

void FileOperationDialog::OnButtonClicked() {
   //in case of button press (whatever button)
   //we simply want to hide window for the next time we
   //need it
   Window->setVisible(false);
   Window->setEnabled(false);

   mGUIFileOperationDialog.TriggerOperationButton->setEnabled(false);
   mGUIFileOperationDialog.TriggerOperationButton->setVisible(false);

   mGUIFileOperationDialog.CancelButton->setEnabled(false);
   mGUIFileOperationDialog.CancelButton->setVisible(false);

   mGUIFileOperationDialog.NewButton->setEnabled(false);
   mGUIFileOperationDialog.NewButton->setVisible(false);

   //make sure the minimap Image GUI Element does not show the texture anymore
   //that we will delete below
   mGUIFileOperationDialog.CurrentlySelectedLevelMinimap->setImage(nullptr);

   //unload all loaded minimap textures in Irrlicht, so that a map minimap
   //has changed Irrlicht is forced to update the minimap texture as well
   //Otherwise Irrlicht would just reuse the existing (now wrong) texture
   //in the video memory
   CleanupLoadedMinimapTextures();
}

void FileOperationDialog::OnTabChanged(irr::s32 elementId) {
    if (elementId == GUI_ID_FILEOPERATIONDIALOG_TABCNTRL) {
      irr::s32 activeTab = mGUIFileOperationDialog.tabCntrl->getActiveTab();

      if (activeTab == 0) {
          OnTableSelected(GUI_ID_FILEOPERATIONDIALOG_GAMELEVELTABLE);
      }

      if (activeTab == 1) {
          OnTableSelected(GUI_ID_FILEOPERATIONDIALOG_CUSTOMLEVELTABLE);
      }
    }
}

void FileOperationDialog::UpdateLevelTables() {
    mGUIFileOperationDialog.GameLevelTable->clearRows();

    std::vector<LevelFolderInfoStruct*>::iterator it;
    irr::s32 nrRowIdx;

    for (it = mParentEditor->mGameLevelVec.begin(); it != mParentEditor->mGameLevelVec.end(); ++it) {

        nrRowIdx = mGUIFileOperationDialog.GameLevelTable->getRowCount();

        //create a new row
        mGUIFileOperationDialog.GameLevelTable->addRow(nrRowIdx);
        irr::core::stringw levelName((*it)->levelName.c_str());
        mGUIFileOperationDialog.GameLevelTable->setCellText(nrRowIdx, 0, levelName);
    }

    mGUIFileOperationDialog.CustomLevelTable->clearRows();

    for (it = mParentEditor->mCustomLevelVec.begin(); it != mParentEditor->mCustomLevelVec.end(); ++it) {

        nrRowIdx = mGUIFileOperationDialog.CustomLevelTable->getRowCount();

        //create a new row
        mGUIFileOperationDialog.CustomLevelTable->addRow(nrRowIdx);
        irr::core::stringw levelName((*it)->levelName.c_str());
        mGUIFileOperationDialog.CustomLevelTable->setCellText(nrRowIdx, 0, levelName);
    }
}

FileOperationDialog::~FileOperationDialog() {
    if (mGUIFileOperationDialog.CurrentlySelectedLevelMinimap != nullptr) {
        mGUIFileOperationDialog.CurrentlySelectedLevelMinimap->remove();
        mGUIFileOperationDialog.CurrentlySelectedLevelMinimap = nullptr;
    }

    if (mGUIFileOperationDialog.LabelNoMiniMapAvailable != nullptr) {
        mGUIFileOperationDialog.LabelNoMiniMapAvailable->remove();
        mGUIFileOperationDialog.LabelNoMiniMapAvailable = nullptr;
    }

    if (mGUIFileOperationDialog.LabelCurrentSelLevelInfo != nullptr) {
        mGUIFileOperationDialog.LabelCurrentSelLevelInfo->remove();
        mGUIFileOperationDialog.LabelCurrentSelLevelInfo = nullptr;
    }

    if (mGUIFileOperationDialog.LabelNewLevelName != nullptr) {
        mGUIFileOperationDialog.LabelNewLevelName->remove();
        mGUIFileOperationDialog.LabelNewLevelName = nullptr;
    }

    if (mGUIFileOperationDialog.NewLevelNameEditBox != nullptr) {
        mGUIFileOperationDialog.NewLevelNameEditBox->remove();
        mGUIFileOperationDialog.NewLevelNameEditBox = nullptr;
    }

    if (mGUIFileOperationDialog.TriggerOperationButton != nullptr) {
        mGUIFileOperationDialog.TriggerOperationButton->remove();
        mGUIFileOperationDialog.TriggerOperationButton = nullptr;
    }

    if (mGUIFileOperationDialog.CancelButton != nullptr) {
        mGUIFileOperationDialog.CancelButton->remove();
        mGUIFileOperationDialog.CancelButton = nullptr;
    }

    if (mGUIFileOperationDialog.NewButton != nullptr) {
        mGUIFileOperationDialog.NewButton->remove();
        mGUIFileOperationDialog.NewButton = nullptr;
    }

    if (mGUIFileOperationDialog.GameLevelTable != nullptr) {
        mGUIFileOperationDialog.GameLevelTable->remove();
        mGUIFileOperationDialog.GameLevelTable = nullptr;
    }

    if (mGUIFileOperationDialog.CustomLevelTable != nullptr) {
        mGUIFileOperationDialog.CustomLevelTable->remove();
        mGUIFileOperationDialog.CustomLevelTable = nullptr;
    }

    if (mGUIFileOperationDialog.GameLevelTab != nullptr) {
        mGUIFileOperationDialog.GameLevelTab->remove();
        mGUIFileOperationDialog.GameLevelTab = nullptr;
    }

    if (mGUIFileOperationDialog.CustomLevelTab != nullptr) {
        mGUIFileOperationDialog.CustomLevelTab->remove();
        mGUIFileOperationDialog.CustomLevelTab = nullptr;
    }

    if (mGUIFileOperationDialog.tabCntrl != nullptr) {
        mGUIFileOperationDialog.tabCntrl->remove();
        mGUIFileOperationDialog.tabCntrl = nullptr;
    }

    if (Window != nullptr) {
        Window->remove();
        Window = nullptr;
    }

    //unload all loaded minimap textures in Irrlicht
    CleanupLoadedMinimapTextures();
}
