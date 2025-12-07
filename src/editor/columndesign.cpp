/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "columndesign.h"
#include "../editorsession.h"
#include "../models/column.h"
#include "../resources/columndefinition.h"
#include "../draw/drawdebug.h"
#include "../editor.h"
#include "itemselector.h"
#include "../resources/texture.h"
#include "../models/levelterrain.h"
#include "../models/levelblocks.h"
#include "../resources/levelfile.h"
#include "../resources/mapentry.h"
#include <iostream>

ColumnDesigner::ColumnDesigner(EditorSession* parentSession) : EditorMode(parentSession) {
    mColumnDefComboBoxEntryVec.clear();

    mModeNameStr.append(L"Columndesign");
}

ColumnDesigner::~ColumnDesigner() {
    //make sure my window is hidden at the end
    HideWindow();

    //cleanup all data
    CleanupCurrentDefComboBoxEntries();

    //remove Gui elements
    if (mGuiColumnDesigner.selColumnDefinition != nullptr) {
        mGuiColumnDesigner.selColumnDefinition->remove();
    }

    if (mGuiColumnDesigner.CurrentSelectedCellInfo != nullptr) {
        mGuiColumnDesigner.CurrentSelectedCellInfo->remove();
    }

    if (mGuiColumnDesigner.CurrentSelectedColumnDefId != nullptr) {
        mGuiColumnDesigner.CurrentSelectedColumnDefId->remove();
    }

    if (mGuiColumnDesigner.ColumDefLabel != nullptr) {
        mGuiColumnDesigner.ColumDefLabel->remove();
    }

    if (mGuiColumnDesigner.blockAImageFront != nullptr) {
        mGuiColumnDesigner.blockAImageFront->remove();
    }

    if (mGuiColumnDesigner.blockBImageFront != nullptr) {
        mGuiColumnDesigner.blockBImageFront->remove();
    }

    if (mGuiColumnDesigner.blockCImageFront != nullptr) {
        mGuiColumnDesigner.blockCImageFront->remove();
    }

    if (mGuiColumnDesigner.blockDImageFront != nullptr) {
        mGuiColumnDesigner.blockDImageFront->remove();
    }

    if (mGuiColumnDesigner.blockEImageFront != nullptr) {
        mGuiColumnDesigner.blockEImageFront->remove();
    }

    if (mGuiColumnDesigner.blockFImageFront != nullptr) {
        mGuiColumnDesigner.blockFImageFront->remove();
    }

    if (mGuiColumnDesigner.blockGImageFront != nullptr) {
        mGuiColumnDesigner.blockGImageFront->remove();
    }

    if (mGuiColumnDesigner.blockHImageFront != nullptr) {
        mGuiColumnDesigner.blockHImageFront->remove();
    }

    if (mGuiColumnDesigner.blockAImageBack != nullptr) {
        mGuiColumnDesigner.blockAImageBack->remove();
    }

    if (mGuiColumnDesigner.blockBImageBack != nullptr) {
        mGuiColumnDesigner.blockBImageBack->remove();
    }

    if (mGuiColumnDesigner.blockCImageBack != nullptr) {
        mGuiColumnDesigner.blockCImageBack->remove();
    }

    if (mGuiColumnDesigner.blockDImageBack != nullptr) {
        mGuiColumnDesigner.blockDImageBack->remove();
    }

    if (mGuiColumnDesigner.blockEImageBack != nullptr) {
        mGuiColumnDesigner.blockEImageBack->remove();
    }

    if (mGuiColumnDesigner.blockFImageBack != nullptr) {
        mGuiColumnDesigner.blockFImageBack->remove();
    }

    if (mGuiColumnDesigner.blockGImageBack != nullptr) {
        mGuiColumnDesigner.blockGImageBack->remove();
    }

    if (mGuiColumnDesigner.blockHImageBack != nullptr) {
        mGuiColumnDesigner.blockHImageBack->remove();
    }

    if (mGuiColumnDesigner.AddColumnButton != nullptr) {
        mGuiColumnDesigner.AddColumnButton->remove();
    }

    if (mGuiColumnDesigner.RemoveColumnButton != nullptr) {
        mGuiColumnDesigner.RemoveColumnButton->remove();
    }

    if (mGuiColumnDesigner.ReplaceColumnButton != nullptr) {
        mGuiColumnDesigner.ReplaceColumnButton->remove();
    }

    if (mGuiColumnDesigner.MoveUpColumnButton != nullptr) {
        mGuiColumnDesigner.MoveUpColumnButton->remove();
    }

    if (mGuiColumnDesigner.MoveDownColumnButton != nullptr) {
        mGuiColumnDesigner.MoveDownColumnButton->remove();
    }

    if (mGuiColumnDesigner.RemoveBlockButton != nullptr) {
        mGuiColumnDesigner.RemoveBlockButton->remove();
    }

    if (mGuiColumnDesigner.AddBlockButton != nullptr) {
        mGuiColumnDesigner.AddBlockButton->remove();
    }

    if (Window != nullptr) {
        //remove the window of this Mode object
        Window->remove();
    }
}

void ColumnDesigner::SetUiMultipleSelection(size_t nrColumnsSelected, size_t nrCellsSelected) {
    ColumnDefinition* noDef = nullptr;

    //deactivate block preview images
    UpdateBlockPreviewGuiImages(noDef);

    wchar_t* selInfo = new wchar_t[200];
    std::wstring infoStr(L"");

    if (nrColumnsSelected < 1) {
        swprintf(selInfo, 190, L"No column selected, ");
    } else {
        swprintf(selInfo, 190, L"%d columns selected, ", (int)(nrColumnsSelected));
    }

    infoStr.append(selInfo);

    if (nrCellsSelected < 1) {
        swprintf(selInfo, 190, L"No cells selected");
    } else {
        swprintf(selInfo, 190, L"%d cells selected", (int)(nrCellsSelected));
    }

    infoStr.append(selInfo);

    mParentSession->mParentEditor->UpdateStatusbarText(infoStr.c_str());
    mGuiColumnDesigner.CurrentSelectedColumnDefId->setText(infoStr.c_str());

    delete[] selInfo;

    wchar_t* textSelCell = new wchar_t[150];
    swprintf(textSelCell, 150, L"Cell X: na Y: na");
    mGuiColumnDesigner.CurrentSelectedCellInfo->setText(textSelCell);
    delete[] textSelCell;

    mGuiColumnDesigner.AddColumnButton->setEnabled(false);
    mGuiColumnDesigner.AddColumnButton->setVisible(false);

    mGuiColumnDesigner.RemoveColumnButton->setEnabled(false);
    mGuiColumnDesigner.RemoveColumnButton->setVisible(false);

    mGuiColumnDesigner.ReplaceColumnButton->setEnabled(false);
    mGuiColumnDesigner.ReplaceColumnButton->setVisible(false);

    if (nrCellsSelected > 0) {
        mGuiColumnDesigner.AddColumnButton->setEnabled(true);
        mGuiColumnDesigner.AddColumnButton->setVisible(true);
    }

    if (nrColumnsSelected > 0) {
        mGuiColumnDesigner.RemoveColumnButton->setEnabled(true);
        mGuiColumnDesigner.RemoveColumnButton->setVisible(true);

        mGuiColumnDesigner.ReplaceColumnButton->setEnabled(true);
        mGuiColumnDesigner.ReplaceColumnButton->setVisible(true);
    }

    mGuiColumnDesigner.MoveUpColumnButton->setEnabled(false);
    mGuiColumnDesigner.MoveUpColumnButton->setVisible(false);

    mGuiColumnDesigner.MoveDownColumnButton->setEnabled(false);
    mGuiColumnDesigner.MoveDownColumnButton->setVisible(false);

    mGuiColumnDesigner.AddBlockButton->setEnabled(false);
    mGuiColumnDesigner.AddBlockButton->setVisible(false);

    mGuiColumnDesigner.RemoveBlockButton->setEnabled(false);
    mGuiColumnDesigner.RemoveBlockButton->setVisible(false);
}

void ColumnDesigner::OnColumnDefinitionComboBoxChanged(irr::u32 newSelectedGuiId) {
    //which entry?
    ColumnDesignerColDefComboBoxEntry* entryPntr = FindColDefComboBoxEntry(newSelectedGuiId);

    //if entry not found, return
    if (entryPntr == nullptr)
        return;

    UpdateBlockPreviewGuiImages(entryPntr->colDefPntr);
}

//returns nullptr if no valid column definition is selected
ColumnDefinition* ColumnDesigner::GetComboBoxSelectedColumnDefinition() {
    irr::s32 currSelId = mGuiColumnDesigner.selColumnDefinition->getSelected();

    if (currSelId == -1)
        return nullptr;

    irr::s32 currData = mGuiColumnDesigner.selColumnDefinition->getItemData(currSelId);

    std::vector<ColumnDesignerColDefComboBoxEntry*>::iterator it;

    for (it = mColumnDefComboBoxEntryVec.begin(); it != mColumnDefComboBoxEntryVec.end(); ++it) {
        if ((*it)->comboBoxEntryId == currData) {
            //selected column definition found
            return (*it)->colDefPntr;
        }
    }

    //if entry not found
    return nullptr;
}

void ColumnDesigner::CreateNewColumnDefComboBoxEntry(ColumnDefinition* whichColDef) {
    if (whichColDef == nullptr)
        return;

    ColumnDesignerColDefComboBoxEntry* newEntry = new ColumnDesignerColDefComboBoxEntry;

    newEntry->colDefPntr = whichColDef;

    std::string shapeString = mParentSession->mLevelBlocks->CreateDbgShapeString(whichColDef);

    newEntry->entryText = new wchar_t[70];

    //07.09.2025: It seems Visual Studio automatically changes swprintf to instead using
    //safer function swprintf_s which would be fine for me
    //The problem is this function checks for validity of format strings, and simply %s as under GCC
    //is not valid when specifiying a normal non wide C-string, and as a result text output does not work (only
    //garbage is shown); To fix this we need to use special format string "%hs" under windows;
    //But because I am not sure if GCC will accept this under Linux, I will keep the original code under GCC
    //here

#ifdef _MSC_VER 
    swprintf(newEntry->entryText, 65, L"Id: %d: Shape: %hs Occurence: %d", whichColDef->get_ID(), shapeString.c_str(), whichColDef->get_Occurence());
#endif
#ifdef __GNUC__
    swprintf(newEntry->entryText, 65, L"Id: %d: Shape: %s Occurence: %d", whichColDef->get_ID(), shapeString.c_str(), whichColDef->get_Occurence());
#endif

    newEntry->comboBoxEntryId = mParentSession->GetNextFreeGuiId();

    mGuiColumnDesigner.selColumnDefinition->addItem(newEntry->entryText, newEntry->comboBoxEntryId);

    mColumnDefComboBoxEntryVec.push_back(newEntry);
}

void ColumnDesigner::CleanupCurrentDefComboBoxEntries() {
    std::vector<ColumnDesignerColDefComboBoxEntry*>::iterator it;
    ColumnDesignerColDefComboBoxEntry* pntr;

    for (it = mColumnDefComboBoxEntryVec.begin(); it != mColumnDefComboBoxEntryVec.end(); ) {
        pntr = (*it);

        it = mColumnDefComboBoxEntryVec.erase(it);

        //free text of entry
        delete[] pntr->entryText;

        //free the struct itself
        delete pntr;
    }
}

//returns nullptr if entry is not found
ColumnDesignerColDefComboBoxEntry* ColumnDesigner::FindColDefComboBoxEntry(irr::s32 index) {
   std::vector<ColumnDesignerColDefComboBoxEntry*>::iterator it;

   for (it = mColumnDefComboBoxEntryVec.begin(); it != mColumnDefComboBoxEntryVec.end(); ++it) {
       if ((*it)->comboBoxEntryId == index) {
           return (*it);
       }
   }

   //entry not found
   return nullptr;
}

void ColumnDesigner::UpdateColumnDefComboBox() {
    if (mGuiColumnDesigner.selColumnDefinition == nullptr)
        return;

    mGuiColumnDesigner.selColumnDefinition->clear();

    if (mColumnDefComboBoxEntryVec.size() > 0) {
        CleanupCurrentDefComboBoxEntries();
    }

    //go through all existing column definitions of the map
    //and sort the column definitions according to the usage count
    //most used ones at the top
    //declaring vector of pairs containing number laps finished
    //and pointer to player
    vector< pair <irr::u32, ColumnDefinition*> > vecColDef;

    std::vector<ColumnDefinition*>::iterator it;

    for (it = mParentSession->mLevelRes->ColumnDefinitions.begin(); it != mParentSession->mLevelRes->ColumnDefinitions.end(); ++it) {
          vecColDef.push_back( make_pair((*it)->get_Occurence(), (*it)));
    }

    //sort vector pairs in ascending number of occurences
   std::sort(vecColDef.begin(), vecColDef.end());

   //start with most often used column definition in this map
   auto it2 = vecColDef.rbegin();

   for (it2 = vecColDef.rbegin(); it2 != vecColDef.rend(); ++it2) {
       CreateNewColumnDefComboBoxEntry((*it2).second);
   }
}

void ColumnDesigner::CreateWindow() {
    irr::core::dimension2d<irr::u32> dim ( 600, 600 );

    //finally create the window
    Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Column Designer", 0, mGuiWindowId);

    mGuiColumnDesigner.ColumDefLabel = mParentSession->mParentEditor->mGuienv->addStaticText ( L"Column Definitions:",
                                      rect<s32>( dim.Width - 400, 25, dim.Width - 290, 50 ),false, false, Window, -1, false );

    mGuiColumnDesigner.selColumnDefinition = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( dim.Width - 280, 24, dim.Width - 10, 49 ),
                                                                                                 Window, GUI_ID_COLUMNDEFSELECTIONCOMBOBOX);

    mGuiColumnDesigner.selColumnDefinition->setToolTipText ( L"Select from existing Column definitions" );

    //create entries for the column
    //definition comboBox
    UpdateColumnDefComboBox();

    /***************************************************************
     * Front images                                                *
     ***************************************************************/

    //create the column block preview:
    blockAImageFrontId = this->mParentSession->GetNextFreeGuiId();

    irr::core::vector2d<irr::s32> newPos(30, dim.Height - 80);

    mGuiColumnDesigner.blockAImageFront =
           mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewFrontNoCube,
                                          newPos, true, Window, blockAImageFrontId);

    blockBImageFrontId = this->mParentSession->GetNextFreeGuiId();

    newPos.Y -= 70;

    mGuiColumnDesigner.blockBImageFront =
           mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewFrontNoCube,
                                          newPos, true, Window, blockBImageFrontId);

    blockCImageFrontId = this->mParentSession->GetNextFreeGuiId();

    newPos.Y -= 70;

    mGuiColumnDesigner.blockCImageFront =
           mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewFrontNoCube,
                                          newPos, true, Window, blockCImageFrontId);

    blockDImageFrontId = this->mParentSession->GetNextFreeGuiId();

    newPos.Y -= 70;

    mGuiColumnDesigner.blockDImageFront =
           mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewFrontNoCube,
                                          newPos, true, Window, blockDImageFrontId);

    blockEImageFrontId = this->mParentSession->GetNextFreeGuiId();

    newPos.Y -= 70;

    mGuiColumnDesigner.blockEImageFront =
           mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewFrontNoCube,
                                          newPos, true, Window, blockEImageFrontId);

    blockFImageFrontId = this->mParentSession->GetNextFreeGuiId();

    newPos.Y -= 70;

    mGuiColumnDesigner.blockFImageFront =
           mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewFrontNoCube,
                                          newPos, true, Window, blockFImageFrontId);

    blockGImageFrontId = this->mParentSession->GetNextFreeGuiId();

    newPos.Y -= 70;

    mGuiColumnDesigner.blockGImageFront =
           mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewFrontNoCube,
                                          newPos, true, Window, blockGImageFrontId);

    blockHImageFrontId = this->mParentSession->GetNextFreeGuiId();

    newPos.Y -= 70;

    mGuiColumnDesigner.blockHImageFront =
           mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewFrontNoCube,
                                          newPos, true, Window, blockHImageFrontId);

    /***************************************************************
     * Back images                                                *
     ***************************************************************/

     //create the column block preview:
     blockAImageBackId = this->mParentSession->GetNextFreeGuiId();

     newPos.set(110, dim.Height - 80);

     mGuiColumnDesigner.blockAImageBack =
          mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewBackNoCube,
                                                  newPos, true, Window, blockAImageBackId);

     blockBImageBackId = this->mParentSession->GetNextFreeGuiId();

     newPos.Y -= 70;

     mGuiColumnDesigner.blockBImageBack =
          mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewBackNoCube,
                                                  newPos, true, Window, blockBImageBackId);

     blockCImageBackId = this->mParentSession->GetNextFreeGuiId();

     newPos.Y -= 70;

     mGuiColumnDesigner.blockCImageBack =
         mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewBackNoCube,
                                                  newPos, true, Window, blockCImageBackId);

     blockDImageBackId = this->mParentSession->GetNextFreeGuiId();

     newPos.Y -= 70;

     mGuiColumnDesigner.blockDImageBack =
         mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewBackNoCube,
                                                  newPos, true, Window, blockDImageBackId);

     blockEImageBackId = this->mParentSession->GetNextFreeGuiId();

     newPos.Y -= 70;

     mGuiColumnDesigner.blockEImageBack =
         mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewBackNoCube,
                                                  newPos, true, Window, blockEImageBackId);

     blockFImageBackId = this->mParentSession->GetNextFreeGuiId();

     newPos.Y -= 70;

     mGuiColumnDesigner.blockFImageBack =
         mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewBackNoCube,
                                                  newPos, true, Window, blockFImageBackId);

     blockGImageBackId = this->mParentSession->GetNextFreeGuiId();

     newPos.Y -= 70;

     mGuiColumnDesigner.blockGImageBack =
                   mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewBackNoCube,
                                                  newPos, true, Window, blockGImageBackId);

     blockHImageBackId = this->mParentSession->GetNextFreeGuiId();

     newPos.Y -= 70;

     mGuiColumnDesigner.blockHImageBack =
                   mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mLevelBlocks->texPreviewBackNoCube,
                                                  newPos, true, Window, blockHImageBackId);

    mGuiColumnDesigner.CurrentSelectedCellInfo = mParentSession->mParentEditor->mGuienv->addStaticText ( L"",
                                      rect<s32>( dim.Width - 400 , 55, dim.Width - 200, 80 ),false, false, Window, -1, false );

    mGuiColumnDesigner.CurrentSelectedColumnDefId = mParentSession->mParentEditor->mGuienv->addStaticText ( L"",
                                      rect<s32>( dim.Width - 400 , 90, dim.Width - 200, 115 ),false, false, Window, -1, false );

    irr::s32 mx = dim.Width - 400;
    irr::s32 my = 130;

    mGuiColumnDesigner.MoveUpColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my, mx + 85, my + 25), Window, GUI_ID_COLUMNDESIGNER_BUTTON_MOVEUPCOLUMN, L"Move up");

    mGuiColumnDesigner.MoveDownColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 50, mx + 85, my + 75), Window, GUI_ID_COLUMNDESIGNER_BUTTON_MOVEDOWNCOLUMN, L"Move down");

    mGuiColumnDesigner.AddColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 100, mx + 85, my + 125), Window, GUI_ID_COLUMNDESIGNER_BUTTON_ADDCOLUMN, L"Add column");

    mGuiColumnDesigner.RemoveColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 100, mx + 85, my + 125), Window, GUI_ID_COLUMNDESIGNER_BUTTON_REMOVECOLUMN, L"Remove column");

    mGuiColumnDesigner.ReplaceColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 150, mx + 85, my + 175), Window, GUI_ID_COLUMNDESIGNER_BUTTON_REPLACECOLUMN, L"Replace column");

    mGuiColumnDesigner.AddBlockButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 200, mx + 85, my + 225), Window, GUI_ID_COLUMNDESIGNER_BUTTON_ADDBLOCK, L"Add block");

    mGuiColumnDesigner.RemoveBlockButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 200, mx + 85, my + 225), Window, GUI_ID_COLUMNDESIGNER_BUTTON_REMOVEBLOCK, L"Remove block");

    //move window to a better start location
    Window->move(irr::core::vector2d<irr::s32>(650,200));
}

void ColumnDesigner::UpdateBlockPreviewGuiImages(Column* selColumn) {
    if (selColumn == nullptr)
        return;

    UpdateBlockPreviewGuiImages(selColumn->Definition);
}

void ColumnDesigner::UpdateBlockPreviewGuiImages(ColumnDefinition* selColumnDef) {
    //GetBlockPreviewImage function handles everything
    //it also returns automatically the preview image for no existing block at this column position
    //in case there is no block present currently
    mGuiColumnDesigner.blockAImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 0, true));
    mGuiColumnDesigner.blockBImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 1, true));
    mGuiColumnDesigner.blockCImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 2, true));
    mGuiColumnDesigner.blockDImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 3, true));
    mGuiColumnDesigner.blockEImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 4, true));
    mGuiColumnDesigner.blockFImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 5, true));
    mGuiColumnDesigner.blockGImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 6, true));
    mGuiColumnDesigner.blockHImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 7, true));

    mGuiColumnDesigner.blockAImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 0, false));
    mGuiColumnDesigner.blockBImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 1, false));
    mGuiColumnDesigner.blockCImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 2, false));
    mGuiColumnDesigner.blockDImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 3, false));
    mGuiColumnDesigner.blockEImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 4, false));
    mGuiColumnDesigner.blockFImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 5, false));
    mGuiColumnDesigner.blockGImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 6, false));
    mGuiColumnDesigner.blockHImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumnDef, 7, false));
}

void ColumnDesigner::WindowControlButtons(bool newState) {
    mGuiColumnDesigner.AddColumnButton->setEnabled(!newState);
    mGuiColumnDesigner.AddColumnButton->setVisible(!newState);

    mGuiColumnDesigner.RemoveColumnButton->setEnabled(newState);
    mGuiColumnDesigner.RemoveColumnButton->setVisible(newState);

    mGuiColumnDesigner.ReplaceColumnButton->setEnabled(newState);
    mGuiColumnDesigner.ReplaceColumnButton->setVisible(newState);

    mGuiColumnDesigner.MoveUpColumnButton->setEnabled(newState);
    mGuiColumnDesigner.MoveUpColumnButton->setVisible(newState);

    mGuiColumnDesigner.MoveDownColumnButton->setEnabled(newState);
    mGuiColumnDesigner.MoveDownColumnButton->setVisible(newState);

    mGuiColumnDesigner.AddBlockButton->setEnabled(newState);
    mGuiColumnDesigner.AddBlockButton->setVisible(newState);

    mGuiColumnDesigner.RemoveBlockButton->setEnabled(newState);
    mGuiColumnDesigner.RemoveBlockButton->setVisible(newState);
}

void ColumnDesigner::OnSelectNewBlockForEditing(int newBlockSelectionForEditing) {
    //if value is invalid, ignore it
    if ((mCurrBlockToBeEditedSelection < 0) || (mCurrBlockToBeEditedSelection > 7)) {
        //something seems to be wrong, disable both buttons
        mGuiColumnDesigner.AddBlockButton->setEnabled(false);
        mGuiColumnDesigner.AddBlockButton->setVisible(false);

        mGuiColumnDesigner.RemoveBlockButton->setEnabled(false);
        mGuiColumnDesigner.RemoveBlockButton->setVisible(false);

        mSelectedBlockDefForEditing = nullptr;
        return;
    }

    //update currently seelcted block for editing
    mCurrBlockToBeEditedSelection = newBlockSelectionForEditing;

    //if at the new selected position in the column there is already a block existing
    //show and enable the RemoveBlock button
    //otherwise enable and show the AddBlock button
    int currBlockId;

    mSelectedColumnForEditing = mParentSession->mItemSelector->mCurrSelectedItem.mColumnSelected;

    ColumnDefinition* colDef = mSelectedColumnForEditing->Definition;

    if (colDef == nullptr) {
        //something seems to be wrong, disable both buttons
        mGuiColumnDesigner.AddBlockButton->setEnabled(false);
        mGuiColumnDesigner.AddBlockButton->setVisible(false);

        mGuiColumnDesigner.RemoveBlockButton->setEnabled(false);
        mGuiColumnDesigner.RemoveBlockButton->setVisible(false);

        mSelectedBlockDefForEditing = nullptr;
        return;
    }

    switch (newBlockSelectionForEditing) {
        case 0: {
             currBlockId = colDef->get_A();
             break;
        }

        case 1: {
             currBlockId = colDef->get_B();
             break;
        }

        case 2: {
             currBlockId = colDef->get_C();
             break;
        }

        case 3: {
             currBlockId = colDef->get_D();
             break;
        }

        case 4: {
             currBlockId = colDef->get_E();
             break;
        }

        case 5: {
             currBlockId = colDef->get_F();
             break;
        }

        case 6: {
             currBlockId = colDef->get_G();
             break;
        }

        case 7: {
             currBlockId = colDef->get_H();
             break;
        }

        default: {
            //something seems to be wrong, disable both buttons
            mGuiColumnDesigner.AddBlockButton->setEnabled(false);
            mGuiColumnDesigner.AddBlockButton->setVisible(false);

            mGuiColumnDesigner.RemoveBlockButton->setEnabled(false);
            mGuiColumnDesigner.RemoveBlockButton->setVisible(false);

            mSelectedBlockDefForEditing = nullptr;
            return;
        }
    }

    //no block existing there?
    if (currBlockId == 0) {
        mGuiColumnDesigner.AddBlockButton->setEnabled(true);
        mGuiColumnDesigner.AddBlockButton->setVisible(true);

        mGuiColumnDesigner.RemoveBlockButton->setEnabled(false);
        mGuiColumnDesigner.RemoveBlockButton->setVisible(false);

        mSelectedBlockDefForEditing = nullptr;
        return;
    }

    //returns nullptr if a block definition with this Id is not found
    BlockDefinition* blockDef = this->mParentSession->mLevelRes->GetBlockDefinitionWithCertainId(currBlockId);

    if (blockDef == nullptr) {
        mGuiColumnDesigner.AddBlockButton->setEnabled(true);
        mGuiColumnDesigner.AddBlockButton->setVisible(true);

        mGuiColumnDesigner.RemoveBlockButton->setEnabled(false);
        mGuiColumnDesigner.RemoveBlockButton->setVisible(false);

        mSelectedBlockDefForEditing = nullptr;
        return;
    }

    //yes, there is a block existing there currently
    mSelectedBlockDefForEditing = blockDef;

    mGuiColumnDesigner.AddBlockButton->setEnabled(false);
    mGuiColumnDesigner.AddBlockButton->setVisible(false);

    mGuiColumnDesigner.RemoveBlockButton->setEnabled(true);
    mGuiColumnDesigner.RemoveBlockButton->setVisible(true);
}

void ColumnDesigner::OnRemoveColumn(CurrentlySelectedEditorItemInfoStruct whichItem) {
    if ((whichItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) &&
        (whichItem.mColumnSelected != nullptr)) {
           mParentSession->mLevelBlocks->RemoveColumn(whichItem.mColumnSelected);
    }
}

void ColumnDesigner::OnReplaceColumn(CurrentlySelectedEditorItemInfoStruct whichItem, ColumnDefinition* replaceWithColDef) {
    if ((whichItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) &&
        (whichItem.mColumnSelected != nullptr)) {
            //first remove the existing column
            mParentSession->mLevelBlocks->RemoveColumn(whichItem.mColumnSelected);

            //add the new column back
            mParentSession->mLevelBlocks->AddColumnAtCell(whichItem.mCellCoordSelected.X,
                                                          whichItem.mCellCoordSelected.Y,
                                                          replaceWithColDef);
    }
}

void ColumnDesigner::OnAddColumn(CurrentlySelectedEditorItemInfoStruct whichItem, ColumnDefinition* addColDef) {
    if (whichItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
           mParentSession->mLevelBlocks->AddColumnAtCell(whichItem.mCellCoordSelected.X, whichItem.mCellCoordSelected.Y,
                                                         addColDef);
    }
}

void ColumnDesigner::OnButtonClicked(irr::s32 buttonGuiId) {
    switch (buttonGuiId) {
        case GUI_ID_COLUMNDESIGNER_BUTTON_REMOVECOLUMN: {
                //apply action to all currently selected items (columns)
                OnRemoveColumn(mParentSession->mItemSelector->mCurrSelectedItem);

                std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
                for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
                      it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end();
                        ++it) {
                            OnRemoveColumn(*(*it));
                }

                mParentSession->mItemSelector->UpdateTrianglesSelectors();

                UpdateColumnDefComboBox();
                break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_REPLACECOLUMN: {
               //which column definition is selected in the comboBox? to which column definition does the user want
               //to change to?
               ColumnDefinition* colDef = GetComboBoxSelectedColumnDefinition();

               //if nothing is selected skip the operation!
               if (colDef != nullptr) {
                       //apply action to all currently selected items (columns)
                       OnReplaceColumn(mParentSession->mItemSelector->mCurrSelectedItem, colDef);

                       std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
                       for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
                             it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end();
                               ++it) {
                                   OnReplaceColumn(*(*it), colDef);
                       }

                       mParentSession->mItemSelector->UpdateTrianglesSelectors();

                       UpdateColumnDefComboBox();
             }
             break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_ADDCOLUMN: {
              //which column definition is selected in the comboBox? what wants the user to add?
              ColumnDefinition* colDef = GetComboBoxSelectedColumnDefinition();

              //apply action to all currently selected items (cells)
              OnAddColumn(mParentSession->mItemSelector->mCurrSelectedItem, colDef);

              std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
              for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
                     it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end();
                        ++it) {
                           OnAddColumn(*(*it), colDef);
              }

              mParentSession->mItemSelector->UpdateTrianglesSelectors();

              UpdateColumnDefComboBox();
              break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_MOVEUPCOLUMN: {
            if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
                int newSelValue = mCurrBlockToBeEditedSelection;
                newSelValue++;

                if (newSelValue > 7) {
                    newSelValue = 7;
                }

                OnSelectNewBlockForEditing(newSelValue);
            }
            break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_MOVEDOWNCOLUMN: {
            if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
                int newSelValue = mCurrBlockToBeEditedSelection;
                newSelValue--;

                if (newSelValue < 0) {
                    newSelValue = 0;
                }

                OnSelectNewBlockForEditing(newSelValue);
            }
            break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_REMOVEBLOCK: {
             OnRemoveBlock();
             break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_ADDBLOCK: {
             OnAddBlock();
             break;
        }
    }
}

void ColumnDesigner::OnAddBlock() {
    if (mSelectedColumnForEditing == nullptr)
        return;

    //default just add existing first block definition
    //if there is not a single existing block definition yet,
    //create an initial one
    if (mParentSession->mLevelBlocks->levelRes->BlockDefinitions.size() < 1) {
        mParentSession->mLevelBlocks->CreateInitialBlockDefinition();
    }

    //just add a block with the first available block definition
    mParentSession->mLevelBlocks->AddBlock(mSelectedColumnForEditing, mCurrBlockToBeEditedSelection, mParentSession->mLevelBlocks->levelRes->BlockDefinitions.at(0));

    mParentSession->mItemSelector->UpdateTrianglesSelectors();

    //call function to update Ui dialog button for adding/removing block
    OnSelectNewBlockForEditing(mCurrBlockToBeEditedSelection);

    UpdateColumnDefComboBox();
}

void ColumnDesigner::OnRemoveBlock() {
    if (mSelectedColumnForEditing == nullptr)
        return;

     mParentSession->mLevelBlocks->RemoveBlock(mSelectedColumnForEditing, mCurrBlockToBeEditedSelection);

     mParentSession->mItemSelector->UpdateTrianglesSelectors();

     //call function to update Ui dialog button for adding/removing block
     OnSelectNewBlockForEditing(mCurrBlockToBeEditedSelection);

     UpdateColumnDefComboBox();
}

void ColumnDesigner::NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    size_t nrSelColumns = mParentSession->mItemSelector->GetNumberSelectedColumns();
    size_t nrSelCells = mParentSession->mItemSelector->GetNumberSelectedCells();

    if ((nrSelColumns < 1) && (nrSelCells < 1))
        return;

    //is our dialog already open?
    //if not open it
    ShowWindow();

    if ((nrSelColumns > 1) || (nrSelCells > 1)) {
        SetUiMultipleSelection(nrSelColumns, nrSelCells);
        return;
    }

    //exactly either one cell or one column is selected
    //Show more details
    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        //is our dialog already open?
        //if not open it
        ShowWindow();

        //for a selected cell hide the window block
        //options (gui elements)
        WindowControlButtons(false);

        mSelectedColumnForEditing = nullptr;
        mSelectedBlockDefForEditing = nullptr;

        //a cell was selected
        //which texture Id does this cell have
        MapEntry* entry;
        entry = this->mParentSession->mLevelTerrain->levelRes->pMap[newItemSelected.mCellCoordSelected.X][newItemSelected.mCellCoordSelected.Y];

        wchar_t* selInfo = new wchar_t[200];

        swprintf(selInfo, 190, L"Selected Cell X = %d, Y = %d", newItemSelected.mCellCoordSelected.X, newItemSelected.mCellCoordSelected.Y);
        mParentSession->mParentEditor->UpdateStatusbarText(selInfo);

        delete[] selInfo;

        if (entry != nullptr) {
            wchar_t* textSelCell = new wchar_t[150];
            swprintf(textSelCell, 150, L"Cell X: %d Y: %d", (int)(newItemSelected.mCellCoordSelected.X), (int)(newItemSelected.mCellCoordSelected.Y));
            mGuiColumnDesigner.CurrentSelectedCellInfo->setText(textSelCell);
            delete[] textSelCell;

            mGuiColumnDesigner.CurrentSelectedColumnDefId->setText(L"No column");
        }
    } else if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
        //is our dialog already open?
        //if not open it
        ShowWindow();

        //for a selected block unhide the window block
        //options (gui elements)
        WindowControlButtons(true);

        if ((newItemSelected.mColumnSelected != nullptr) && (newItemSelected.mColumnSelected->Definition != nullptr)) {
                wchar_t* selInfo = new wchar_t[200];
                swprintf(selInfo, 190, L"Column at Cell X = %d, Y = %d, Id = %d",
                         newItemSelected.mCellCoordSelected.X, newItemSelected.mCellCoordSelected.Y,
                         newItemSelected.mColumnSelected->Definition->get_ID());

                mParentSession->mParentEditor->UpdateStatusbarText(selInfo);

                mGuiColumnDesigner.CurrentSelectedColumnDefId->setText(selInfo);

                delete[] selInfo;

                UpdateBlockPreviewGuiImages(newItemSelected.mColumnSelected);

                OnSelectNewBlockForEditing(mCurrBlockToBeEditedSelection);
        }
     }
}

void ColumnDesigner::SelectBlockToEditFromBlockPreview(irr::gui::IGUIImage* currHoveredBlockPreviewImage) {
    if (currHoveredBlockPreviewImage == nullptr)
        return;

    //for both the front and back block preview image we select the same block position
    //front or back does not matter, user can use both to select
    if ((currHoveredBlockPreviewImage == mGuiColumnDesigner.blockAImageFront) ||
        (currHoveredBlockPreviewImage == mGuiColumnDesigner.blockAImageBack)) {
           OnSelectNewBlockForEditing(0);
    }

    if ((currHoveredBlockPreviewImage == mGuiColumnDesigner.blockBImageFront) ||
        (currHoveredBlockPreviewImage == mGuiColumnDesigner.blockBImageBack)) {
          OnSelectNewBlockForEditing(1);
    }

    if ((currHoveredBlockPreviewImage == mGuiColumnDesigner.blockCImageFront) ||
        (currHoveredBlockPreviewImage == mGuiColumnDesigner.blockCImageBack)) {
         OnSelectNewBlockForEditing(2);
    }

    if ((currHoveredBlockPreviewImage == mGuiColumnDesigner.blockDImageFront) ||
        (currHoveredBlockPreviewImage == mGuiColumnDesigner.blockDImageBack)) {
         OnSelectNewBlockForEditing(3);
    }

    if ((currHoveredBlockPreviewImage == mGuiColumnDesigner.blockEImageFront) ||
        (currHoveredBlockPreviewImage == mGuiColumnDesigner.blockEImageBack)) {
         OnSelectNewBlockForEditing(4);
    }

    if ((currHoveredBlockPreviewImage == mGuiColumnDesigner.blockFImageFront) ||
        (currHoveredBlockPreviewImage == mGuiColumnDesigner.blockFImageBack)) {
         OnSelectNewBlockForEditing(5);
    }

    if ((currHoveredBlockPreviewImage == mGuiColumnDesigner.blockGImageFront) ||
        (currHoveredBlockPreviewImage == mGuiColumnDesigner.blockGImageBack)) {
         OnSelectNewBlockForEditing(6);
    }

    if ((currHoveredBlockPreviewImage == mGuiColumnDesigner.blockHImageFront) ||
        (currHoveredBlockPreviewImage == mGuiColumnDesigner.blockHImageBack)) {
         OnSelectNewBlockForEditing(7);
    }
}

void ColumnDesigner::OnLeftMouseButtonDown() {
    switch (mParentSession->mUserInDialogState) {
         case DEF_EDITOR_USERINNODIALOG: {
            //is now a new level cell or block selected?
            if ((mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) ||
               (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK)) {
                    NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);
            }

            break;
         }

        case DEF_EDITOR_USERINCOLUMNDESIGNERDIALOG: {
            //are we hovering right now over exactly one of my
            //texture selection images?
            if (mBlockPreviewCurrentlyHovered != nullptr) {
                if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
                    SelectBlockToEditFromBlockPreview(mBlockPreviewCurrentlyHovered);
                }
            }

           break;
        }

        default: {
           break;
        }
    }
}

//returns nullptr if currently no block preview image
//is hovered by the user
irr::gui::IGUIImage* ColumnDesigner::FindCurrentlyHoveredBlockPreviewImage(irr::s32 hoveredGuiId) {
    //which block preview image was hovered?
    if (blockAImageFrontId == hoveredGuiId)
        return mGuiColumnDesigner.blockAImageFront;

    if (blockBImageFrontId == hoveredGuiId)
        return mGuiColumnDesigner.blockBImageFront;

    if (blockCImageFrontId == hoveredGuiId)
        return mGuiColumnDesigner.blockCImageFront;

    if (blockDImageFrontId == hoveredGuiId)
        return mGuiColumnDesigner.blockDImageFront;

    if (blockEImageFrontId == hoveredGuiId)
        return mGuiColumnDesigner.blockEImageFront;

    if (blockFImageFrontId == hoveredGuiId)
        return mGuiColumnDesigner.blockFImageFront;

    if (blockGImageFrontId == hoveredGuiId)
        return mGuiColumnDesigner.blockGImageFront;

    if (blockHImageFrontId == hoveredGuiId)
        return mGuiColumnDesigner.blockHImageFront;

    if (blockAImageBackId == hoveredGuiId)
        return mGuiColumnDesigner.blockAImageBack;

    if (blockBImageBackId == hoveredGuiId)
        return mGuiColumnDesigner.blockBImageBack;

    if (blockCImageBackId == hoveredGuiId)
        return mGuiColumnDesigner.blockCImageBack;

    if (blockDImageBackId == hoveredGuiId)
        return mGuiColumnDesigner.blockDImageBack;

    if (blockEImageBackId == hoveredGuiId)
        return mGuiColumnDesigner.blockEImageBack;

    if (blockFImageBackId == hoveredGuiId)
        return mGuiColumnDesigner.blockFImageBack;

    if (blockGImageBackId == hoveredGuiId)
        return mGuiColumnDesigner.blockGImageBack;

    if (blockHImageBackId == hoveredGuiId)
        return mGuiColumnDesigner.blockHImageBack;

    return nullptr;
}

void ColumnDesigner::OnElementHovered(irr::s32 hoveredGuiId) {
    if (mParentSession->mUserInDialogState != DEF_EDITOR_USERINCOLUMNDESIGNERDIALOG)
        return;

    mBlockPreviewCurrentlyHovered = FindCurrentlyHoveredBlockPreviewImage(hoveredGuiId);
}

void ColumnDesigner::OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem) {
    if (mCurrSelectedItem == nullptr)
        return;

    if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrSelectedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->orange);
    } else if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
             mParentSession->mLevelBlocks->DrawColumnSelectionGrid(mCurrSelectedItem->mColumnSelected, mParentSession->mParentEditor->mDrawDebug->orange,
                                                                   true, mCurrBlockToBeEditedSelection, mParentSession->mParentEditor->mDrawDebug->white);
        }
}

void ColumnDesigner::OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem) {
    if (mCurrHighlightedItem == nullptr)
        return;

    if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrHighlightedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->white);
    } else if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
         mParentSession->mLevelBlocks->DrawColumnSelectionGrid(mCurrHighlightedItem->mColumnSelected, mParentSession->mParentEditor->mDrawDebug->cyan, false,
                                                               0, mParentSession->mParentEditor->mDrawDebug->cyan);
    }
}
