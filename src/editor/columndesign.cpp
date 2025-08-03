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
}

ColumnDesigner::~ColumnDesigner() {
    //cleanup all data
    CleanupCurrentDefComboBoxEntries();
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
    swprintf(newEntry->entryText, 65, L"Id: %d: Shape: %s Occurence: %d", whichColDef->get_ID(), shapeString.c_str(), whichColDef->get_Occurence());

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

    // set skin font
    /*IGUIFont* font = env->getFont("fontlucida.png");
    if (font)
        env->getSkin()->setFont(font);*/

    irr::core::dimension2d<irr::u32> dim ( 600, 600 );

    //finally create the window
    Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Column Designer", 0, mGuiWindowId);

    mParentSession->mParentEditor->mGuienv->addStaticText ( L"Column Definitions:",
                                      rect<s32>( dim.Width - 400, 24, dim.Width - 310, 40 ),false, false, Window, -1, false );

    mGuiColumnDesigner.selColumnDefinition = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( dim.Width - 300, 24, dim.Width - 10, 40 ),
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
                                      rect<s32>( dim.Width - 400 , 55, dim.Width - 200, 65 ),false, false, Window, -1, false );

    mGuiColumnDesigner.CurrentSelectedColumnDefId = mParentSession->mParentEditor->mGuienv->addStaticText ( L"",
                                      rect<s32>( dim.Width - 400 , 65, dim.Width - 200, 75 ),false, false, Window, -1, false );

    irr::s32 mx = dim.Width - 400;
    irr::s32 my = 90;

    mGuiColumnDesigner.MoveUpColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my, mx + 85, my + 15), Window, GUI_ID_COLUMNDESIGNER_BUTTON_MOVEUPCOLUMN, L"Move up");

    mGuiColumnDesigner.MoveDownColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 30, mx + 85, my + 45), Window, GUI_ID_COLUMNDESIGNER_BUTTON_MOVEDOWNCOLUMN, L"Move down");

    mGuiColumnDesigner.AddColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 60, mx + 85, my + 75), Window, GUI_ID_COLUMNDESIGNER_BUTTON_ADDCOLUMN, L"Add column");

    mGuiColumnDesigner.RemoveColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 60, mx + 85, my + 75), Window, GUI_ID_COLUMNDESIGNER_BUTTON_REMOVECOLUMN, L"Remove column");

    mGuiColumnDesigner.ReplaceColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 90, mx + 85, my + 105), Window, GUI_ID_COLUMNDESIGNER_BUTTON_REPLACECOLUMN, L"Replace column");

    mGuiColumnDesigner.AddBlockButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 120, mx + 85, my + 135), Window, GUI_ID_COLUMNDESIGNER_BUTTON_ADDBLOCK, L"Add block");

    mGuiColumnDesigner.RemoveBlockButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my + 120, mx + 85, my + 135), Window, GUI_ID_COLUMNDESIGNER_BUTTON_REMOVEBLOCK, L"Remove block");
}

void ColumnDesigner::UpdateBlockPreviewGuiImages(Column* selColumn) {
    if (selColumn == nullptr)
        return;

    UpdateBlockPreviewGuiImages(selColumn->Definition);
}

void ColumnDesigner::UpdateBlockPreviewGuiImages(ColumnDefinition* selColumnDef) {
    if (selColumnDef == nullptr)
        return;

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

void ColumnDesigner::OnButtonClicked(irr::s32 buttonGuiId) {
    switch (buttonGuiId) {
        case GUI_ID_COLUMNDESIGNER_BUTTON_REMOVECOLUMN: {
             if ((mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) &&
                 (mParentSession->mItemSelector->mCurrSelectedItem.mColumnSelected != nullptr)) {
                    mParentSession->mLevelBlocks->RemoveColumn(mParentSession->mItemSelector->mCurrSelectedItem.mColumnSelected);

                    mParentSession->mItemSelector->UpdateTrianglesSelectors();

                    UpdateColumnDefComboBox();
             }
             break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_REPLACECOLUMN: {
             if ((mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) &&
                 (mParentSession->mItemSelector->mCurrSelectedItem.mColumnSelected != nullptr)) {
                    //which column definition is selected in the comboBox? to which column definition does the user want
                    //to change to?
                    ColumnDefinition* colDef = GetComboBoxSelectedColumnDefinition();

                    //if nothing is selected skip the operation!
                    if (colDef != nullptr) {
                            //first remove the existing column
                            mParentSession->mLevelBlocks->RemoveColumn(mParentSession->mItemSelector->mCurrSelectedItem.mColumnSelected);

                            //add the new column back
                            mParentSession->mLevelBlocks->AddColumnAtCell(mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X,
                                                                          mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y,
                                                                          colDef);

                            mParentSession->mItemSelector->UpdateTrianglesSelectors();

                            UpdateColumnDefComboBox();
                    }
             }
             break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_ADDCOLUMN: {
             if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
                    //which column definition is selected in the comboBox? what wants the user to add?
                    ColumnDefinition* colDef = GetComboBoxSelectedColumnDefinition();

                    mParentSession->mLevelBlocks->AddColumnAtCell(mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X,
                                                                  mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y,
                                                                  colDef);

                    mParentSession->mItemSelector->UpdateTrianglesSelectors();

                    UpdateColumnDefComboBox();
             }
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

void ColumnDesigner::OnElementLeft(irr::s32 leftGuiId) {
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
