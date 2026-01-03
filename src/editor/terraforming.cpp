/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "terraforming.h"
#include "../editorsession.h"
#include "../editor.h"
#include "itemselector.h"
#include "../draw/drawdebug.h"
#include "../models/levelterrain.h"
#include "../input/numbereditbox.h"
#include "../editor/uiconversion.h"

TerraformingMode::TerraformingMode(EditorSession* parentSession) : EditorMode(parentSession) {
     mModeNameStr.append(L"Terraforming");
}

TerraformingMode::~TerraformingMode() {
    //make sure my window is hidden at the end
    HideWindow();

    if (mGuiTerraformingMode.LblSelection != nullptr) {
        mGuiTerraformingMode.LblSelection->remove();
    }

    if (mGuiTerraformingMode.ButtonSelCell != nullptr) {
        mGuiTerraformingMode.ButtonSelCell->remove();
    }

    if (mGuiTerraformingMode.ButtonSelVertices != nullptr) {
        mGuiTerraformingMode.ButtonSelVertices->remove();
    }

    if (mGuiTerraformingMode.Vertice1NEB != nullptr) {
        delete mGuiTerraformingMode.Vertice1NEB;
    }

    if (mGuiTerraformingMode.Vertice2NEB != nullptr) {
        delete mGuiTerraformingMode.Vertice2NEB;
    }

    if (mGuiTerraformingMode.Vertice3NEB != nullptr) {
        delete mGuiTerraformingMode.Vertice3NEB;
    }

    if (mGuiTerraformingMode.Vertice4NEB != nullptr) {
        delete mGuiTerraformingMode.Vertice4NEB;
    }

    if (mGuiTerraformingMode.CellNEB != nullptr) {
        delete mGuiTerraformingMode.CellNEB;
    }

    if (mGuiTerraformingMode.MultipleSelectionNEB != nullptr) {
        delete mGuiTerraformingMode.MultipleSelectionNEB;
    }

    if (mGuiTerraformingMode.LblStepSize != nullptr) {
        mGuiTerraformingMode.LblStepSize->remove();
    }

    if (mGuiTerraformingMode.ComboBoxStepSize != nullptr) {
        mGuiTerraformingMode.ComboBoxStepSize->remove();
    }

    if (mGuiTerraformingMode.LblVertice1HeightInfo != nullptr) {
        mGuiTerraformingMode.LblVertice1HeightInfo->remove();
    }

    if (mGuiTerraformingMode.LblVertice2HeightInfo != nullptr) {
        mGuiTerraformingMode.LblVertice2HeightInfo->remove();
    }

    if (mGuiTerraformingMode.LblVertice3HeightInfo != nullptr) {
        mGuiTerraformingMode.LblVertice3HeightInfo->remove();
    }

    if (mGuiTerraformingMode.LblVertice4HeightInfo != nullptr) {
        mGuiTerraformingMode.LblVertice4HeightInfo->remove();
    }

    //cleanup all data
    if (Window != nullptr) {
        //remove the window of this Mode object
        Window->remove();
    }
}

void TerraformingMode::SetUiMultipleSelection() {
    size_t selNrVertices = mParentSession->mItemSelector->GetNumberSelectedCells();

    wchar_t* selInfo = new wchar_t[200];

    if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELVERTICES) {
        swprintf(selInfo, 190, L"%d vertices selected", (int)(selNrVertices));
    } else if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELCELLS) {
        swprintf(selInfo, 190, L"%d cells selected", (int)(selNrVertices));
    }

    mParentSession->mParentEditor->UpdateStatusbarText(selInfo);

    delete[] selInfo;

    if (selNrVertices > 1) {
        mGuiTerraformingMode.Vertice1NEB->SetVisible(false);
        mGuiTerraformingMode.Vertice2NEB->SetVisible(false);
        mGuiTerraformingMode.Vertice3NEB->SetVisible(false);
        mGuiTerraformingMode.Vertice4NEB->SetVisible(false);
        mGuiTerraformingMode.CellNEB->SetVisible(false);

        mGuiTerraformingMode.LblVertice1HeightInfo->setVisible(false);
        mGuiTerraformingMode.LblVertice2HeightInfo->setVisible(false);
        mGuiTerraformingMode.LblVertice3HeightInfo->setVisible(false);
        mGuiTerraformingMode.LblVertice4HeightInfo->setVisible(false);

        mGuiTerraformingMode.MultipleSelectionNEB->SetValue(0.0f);
        mGuiTerraformingMode.MultipleSelectionNEB->SetVisible(true);
    }
}

//is called when the editor mode
//is entered (becomes active)
void TerraformingMode::OnEnterMode() {
    mParentSession->mParentEditor->UpdateStatusbarText(L"Press '+' key to increase selected cell/vertice height, '-' key to decrease it");
}

void TerraformingMode::CreateWindow() {
    irr::core::dimension2d<irr::u32> dim ( 300, 250 );

    //finally create the window
    Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Terraforming", 0, mGuiWindowId);

    //move window to a better start location
    Window->move(irr::core::vector2d<irr::s32>(950,200));

    mGuiTerraformingMode.LblSelection = mParentSession->mParentEditor->mGuienv->addStaticText ( L"Select:",
                                                                                               rect<s32>( 20, 30, 80, 55 ), false, false, Window, -1, false );

    mGuiTerraformingMode.ButtonSelCell =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(90, 30, 140, 55), Window, GUI_ID_TERRAFORMING_WINDOW_BUTTONSELCELLS, L"Cells");

    mGuiTerraformingMode.ButtonSelVertices =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(160, 30, 230, 55), Window, GUI_ID_TERRAFORMING_WINDOW_BUTTONSELVERTICES, L"Vertices");

    mGuiTerraformingMode.LblStepSize= mParentSession->mParentEditor->mGuienv->addStaticText ( L"Stepsize:",
                                      rect<s32>( 185, dim.Height - 98, 160 + 185, dim.Height - 73 ),false, false, Window, -1, false );

    mGuiTerraformingMode.ComboBoxStepSize = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( 185, dim.Height - 73, 100 + 185, dim.Height - 53),
                                                                                          Window, GUI_ID_TERRAFORMING_WINDOW_STEPSIZE_COMBOBOX);

    mGuiTerraformingMode.ComboBoxStepSize->addItem(L" 1", 1);
    mGuiTerraformingMode.ComboBoxStepSize->addItem(L" 2", 2);
    mGuiTerraformingMode.ComboBoxStepSize->addItem(L" 5", 5);
    mGuiTerraformingMode.ComboBoxStepSize->addItem(L"10", 10);
    mGuiTerraformingMode.ComboBoxStepSize->addItem(L"20", 20);
    mGuiTerraformingMode.ComboBoxStepSize->addItem(L"50", 50);

    //default select "5"
    mGuiTerraformingMode.ComboBoxStepSize->setSelected(2);

    mGuiTerraformingMode.ComboBoxStepSize->setEnabled(true);
    mGuiTerraformingMode.ComboBoxStepSize->setVisible(true);
    mGuiTerraformingMode.LblStepSize->setEnabled(true);
    mGuiTerraformingMode.LblStepSize->setVisible(true);

    irr::s32 dx = 20;
    irr::s32 dy = 70;

    //Float Type NumberEditBox Vertice 1
    mGuiTerraformingMode.Vertice1NEB = new NumberEditBox(this, EDITOR_TERRAFORMING_HEIGHT_SHOWNRDECIMALPLACES, rect<s32> ( dx + 35, dy, dx + 100, dy + 25), true, Window);

    mGuiTerraformingMode.Vertice1NEB->SetValue(0.0f);
    mGuiTerraformingMode.Vertice1NEB->SetValueLimit(-20.0f, 20.0f);
    mGuiTerraformingMode.Vertice1NEB->AddLabel(L"V1:", rect<s32>( dx, dy + 2, dx + 25, dy + 27));
    mGuiTerraformingMode.Vertice1NEB->SetVisible(false);

    //in Cell Selection mode instead place a label containing the current
    //vertice 1 height information
    mGuiTerraformingMode.LblVertice1HeightInfo = mParentSession->mParentEditor->mGuienv->addStaticText ( L"V1 h:",
                                      rect<s32>( dx, dy + 2, dx + 125, dy + 27 ),false, false, Window, -1, false );

    mGuiTerraformingMode.LblVertice1HeightInfo->setVisible(false);
    mGuiTerraformingMode.LblVertice1HeightInfo->setEnabled(true);

    dx += 130;

    //Float Type NumberEditBox CellNEB
    mGuiTerraformingMode.CellNEB = new NumberEditBox(this, EDITOR_TERRAFORMING_HEIGHT_SHOWNRDECIMALPLACES, rect<s32> ( dx + 55, dy, dx + 120, dy + 25), true, Window);

    mGuiTerraformingMode.CellNEB->SetValue(0.0f);
    mGuiTerraformingMode.CellNEB->SetValueLimit(-20.0f, 20.0f);
    mGuiTerraformingMode.CellNEB->AddLabel(L"Cell avg:", rect<s32>( dx, dy + 2, dx + 50, dy + 27));
    mGuiTerraformingMode.CellNEB->SetVisible(false);

    dx -= 130;

    //Float Type NumberEditBox Vertice 2
    dy += 35;

    mGuiTerraformingMode.Vertice2NEB = new NumberEditBox(this, EDITOR_TERRAFORMING_HEIGHT_SHOWNRDECIMALPLACES, rect<s32> ( dx + 35, dy, dx + 100, dy + 25), true, Window);

    mGuiTerraformingMode.Vertice2NEB->SetValue(0.0f);
    mGuiTerraformingMode.Vertice2NEB->SetValueLimit(-20.0f, 20.0f);
    mGuiTerraformingMode.Vertice2NEB->AddLabel(L"V2:", rect<s32>( dx, dy + 2, dx + 25, dy + 27));
    mGuiTerraformingMode.Vertice2NEB->SetVisible(false);

    //in Cell Selection mode instead place a label containing the current
    //vertice 2 height information
    mGuiTerraformingMode.LblVertice2HeightInfo = mParentSession->mParentEditor->mGuienv->addStaticText ( L"V2 h:",
                                      rect<s32>( dx, dy + 2, dx + 125, dy + 27 ),false, false, Window, -1, false );

    mGuiTerraformingMode.LblVertice2HeightInfo->setVisible(false);
    mGuiTerraformingMode.LblVertice2HeightInfo->setEnabled(true);

    dx += 130;

    //Float Type NumberEditBox MultipleSelectionNEB
    mGuiTerraformingMode.MultipleSelectionNEB = new NumberEditBox(this, EDITOR_TERRAFORMING_HEIGHT_SHOWNRDECIMALPLACES, rect<s32> ( dx + 55, dy, dx + 120, dy + 25), true, Window);

    mGuiTerraformingMode.MultipleSelectionNEB->SetValue(0.0f);
    mGuiTerraformingMode.MultipleSelectionNEB->SetValueLimit(-20.0f, 20.0f);
    mGuiTerraformingMode.MultipleSelectionNEB->AddLabel(L"Set H:", rect<s32>( dx, dy + 2, dx + 50, dy + 27));
    mGuiTerraformingMode.MultipleSelectionNEB->SetVisible(false);

    dx -= 130;

    //Float Type NumberEditBox Vertice 3
    dy += 35;

    mGuiTerraformingMode.Vertice3NEB = new NumberEditBox(this, EDITOR_TERRAFORMING_HEIGHT_SHOWNRDECIMALPLACES, rect<s32> ( dx + 35, dy, dx + 100, dy + 25), true, Window);

    mGuiTerraformingMode.Vertice3NEB->SetValue(0.0f);
    mGuiTerraformingMode.Vertice3NEB->SetValueLimit(-20.0f, 20.0f);
    mGuiTerraformingMode.Vertice3NEB->AddLabel(L"V3:", rect<s32>( dx, dy + 2, dx + 25, dy + 27));
    mGuiTerraformingMode.Vertice3NEB->SetVisible(false);

    //in Cell Selection mode instead place a label containing the current
    //vertice 3 height information
    mGuiTerraformingMode.LblVertice3HeightInfo = mParentSession->mParentEditor->mGuienv->addStaticText ( L"V3 h:",
                                      rect<s32>( dx, dy + 2, dx + 125, dy + 27 ),false, false, Window, -1, false );

    mGuiTerraformingMode.LblVertice3HeightInfo->setVisible(false);
    mGuiTerraformingMode.LblVertice3HeightInfo->setEnabled(true);

    //Float Type NumberEditBox Vertice 4
    dy += 35;

    mGuiTerraformingMode.Vertice4NEB = new NumberEditBox(this, EDITOR_TERRAFORMING_HEIGHT_SHOWNRDECIMALPLACES, rect<s32> ( dx + 35, dy, dx + 100, dy + 25), true, Window);

    mGuiTerraformingMode.Vertice4NEB->SetValue(0.0f);
    mGuiTerraformingMode.Vertice4NEB->SetValueLimit(-20.0f, 20.0f);
    mGuiTerraformingMode.Vertice4NEB->AddLabel(L"V4:", rect<s32>( dx, dy + 2, dx + 25, dy + 27));
    mGuiTerraformingMode.Vertice4NEB->SetVisible(false);

    //in Cell Selection mode instead place a label containing the current
    //vertice 4 height information
    mGuiTerraformingMode.LblVertice4HeightInfo = mParentSession->mParentEditor->mGuienv->addStaticText ( L"V4 h:",
                                      rect<s32>( dx, dy + 2, dx + 125, dy + 27 ),false, false, Window, -1, false );

    mGuiTerraformingMode.LblVertice4HeightInfo->setVisible(false);
    mGuiTerraformingMode.LblVertice4HeightInfo->setEnabled(true);
}

void TerraformingMode::OnButtonClicked(irr::s32 buttonGuiId) {
    switch (buttonGuiId) {
        case GUI_ID_TERRAFORMING_WINDOW_BUTTONSELVERTICES: {
             mOpMode = EDITOR_TERRAFORMING_OPMODE_SELVERTICES;
             mGuiTerraformingMode.LblVertice1HeightInfo->setVisible(false);
             mGuiTerraformingMode.LblVertice2HeightInfo->setVisible(false);
             mGuiTerraformingMode.LblVertice3HeightInfo->setVisible(false);
             mGuiTerraformingMode.LblVertice4HeightInfo->setVisible(false);
             mGuiTerraformingMode.MultipleSelectionNEB->SetVisible(false);

             //Deselect all currently selected items
             mParentSession->mItemSelector->DeselectAll();
             break;
        }

        case GUI_ID_TERRAFORMING_WINDOW_BUTTONSELCELLS: {
             mOpMode = EDITOR_TERRAFORMING_OPMODE_SELCELLS;

             mGuiTerraformingMode.Vertice1NEB->SetVisible(false);
             mGuiTerraformingMode.Vertice2NEB->SetVisible(false);
             mGuiTerraformingMode.Vertice3NEB->SetVisible(false);
             mGuiTerraformingMode.Vertice4NEB->SetVisible(false);
             mGuiTerraformingMode.MultipleSelectionNEB->SetVisible(false);

             mGuiTerraformingMode.LblVertice1HeightInfo->setVisible(true);
             mGuiTerraformingMode.LblVertice2HeightInfo->setVisible(true);
             mGuiTerraformingMode.LblVertice3HeightInfo->setVisible(true);
             mGuiTerraformingMode.LblVertice4HeightInfo->setVisible(true);

             //Deselect all currently selected items
             mParentSession->mItemSelector->DeselectAll();
             break;
        }

        default: {
            break;
        }
    }
}

void TerraformingMode::UpdateUiEditNumberboxes(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    mGuiTerraformingMode.Vertice1NEB->SetVisible(false);
    mGuiTerraformingMode.Vertice2NEB->SetVisible(false);
    mGuiTerraformingMode.Vertice3NEB->SetVisible(false);
    mGuiTerraformingMode.Vertice4NEB->SetVisible(false);
    mGuiTerraformingMode.CellNEB->SetVisible(false);
    mGuiTerraformingMode.MultipleSelectionNEB->SetVisible(false);

    mGuiTerraformingMode.LblVertice1HeightInfo->setVisible(false);
    mGuiTerraformingMode.LblVertice2HeightInfo->setVisible(false);
    mGuiTerraformingMode.LblVertice3HeightInfo->setVisible(false);
    mGuiTerraformingMode.LblVertice4HeightInfo->setVisible(false);

    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELVERTICES) {
            //only show the EditBox for the selected Vertex
            if (newItemSelected.mCellCoordVerticeNrSelected == 1) {
                mGuiTerraformingMode.Vertice1NEB->SetVisible(true);
            }

            if (newItemSelected.mCellCoordVerticeNrSelected == 2) {
                mGuiTerraformingMode.Vertice2NEB->SetVisible(true);
            }

            if (newItemSelected.mCellCoordVerticeNrSelected == 3) {
                mGuiTerraformingMode.Vertice3NEB->SetVisible(true);
            }

            if (newItemSelected.mCellCoordVerticeNrSelected == 4) {
                mGuiTerraformingMode.Vertice4NEB->SetVisible(true);
            }
        } else if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELCELLS) {
                mGuiTerraformingMode.CellNEB->SetVisible(true);

                mGuiTerraformingMode.LblVertice1HeightInfo->setVisible(true);
                mGuiTerraformingMode.LblVertice2HeightInfo->setVisible(true);
                mGuiTerraformingMode.LblVertice3HeightInfo->setVisible(true);
                mGuiTerraformingMode.LblVertice4HeightInfo->setVisible(true);
        }
    }
}

void TerraformingMode::NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    size_t nrSelCells = mParentSession->mItemSelector->GetNumberSelectedCells();

    if (nrSelCells < 1)
        return;

    //is our dialog already open?
    //if not open it
    ShowWindow();

    if (nrSelCells > 1) {
        SetUiMultipleSelection();
        return;
    } else {
       mParentSession->mParentEditor->UpdateStatusbarText(L"Press '+' key to increase selected cell/vertice height, '-' key to decrease it");
    }

    //Exactly one item is selected, show more details
    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {

        UpdateUiEditNumberboxes(newItemSelected);

        irr::s32 coordX = mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X;
        irr::s32 coordY = mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y;

        //Y-coordinate from original map has flipped sign in this project
        irr::f32 currV1h = -mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert1CurrPositionY;
        irr::f32 currV2h = -mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert2CurrPositionY;
        irr::f32 currV3h = -mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert3CurrPositionY;
        irr::f32 currV4h = -mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert4CurrPositionY;

        //Update the labels showing vertice height
        UpdateHeightLbl(1, currV1h);
        UpdateHeightLbl(2, currV2h);
        UpdateHeightLbl(3, currV3h);
        UpdateHeightLbl(4, currV4h);

        if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELVERTICES) {
            mGuiTerraformingMode.Vertice1NEB->SetValue(currV1h);
            mGuiTerraformingMode.Vertice2NEB->SetValue(currV2h);
            mGuiTerraformingMode.Vertice3NEB->SetValue(currV3h);
            mGuiTerraformingMode.Vertice4NEB->SetValue(currV4h);
        } else {
           irr::f32 avgVh = (currV1h + currV2h + currV3h + currV4h) / 4.0f;
           mGuiTerraformingMode.CellNEB->SetValue(avgVh);
        }
    }
}

void TerraformingMode::UpdateHeightLbl(irr::u32 verticeNr, irr::f32 newValue) {
    stringw newLblTxt("");

    if (verticeNr == 1) {
        newLblTxt.append(L"V1 h: ");
    }

    if (verticeNr == 2) {
        newLblTxt.append(L"V2 h: ");
    }

    if (verticeNr == 3) {
        newLblTxt.append(L"V3 h: ");
    }

    if (verticeNr == 4) {
        newLblTxt.append(L"V4 h: ");
    }

    stringw newValueStr(newValue);

    //limit number of shown decimal places
    stringw newValueStrLimited(
                mParentSession->mParentEditor->mUiConversion->NumberStringLimitDecimalPlaces(newValueStr, EDITOR_TERRAFORMING_HEIGHT_SHOWNRDECIMALPLACES));

    newLblTxt.append(newValueStrLimited);

    if (verticeNr == 1) {
        mGuiTerraformingMode.LblVertice1HeightInfo->setText(newLblTxt.c_str());
    }

    if (verticeNr == 2) {
        mGuiTerraformingMode.LblVertice2HeightInfo->setText(newLblTxt.c_str());
    }

    if (verticeNr == 3) {
        mGuiTerraformingMode.LblVertice3HeightInfo->setText(newLblTxt.c_str());
    }

    if (verticeNr == 4) {
        mGuiTerraformingMode.LblVertice4HeightInfo->setText(newLblTxt.c_str());
    }
}

void TerraformingMode::OnLeftMouseButtonDown() {
    switch (mParentSession->mUserInDialogState) {
         case DEF_EDITOR_USERINNODIALOG: {
            //is now a new level cell or block selected?
            if ((mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) ||
               (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK)) {
                    NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);
            }

            break;
         }

        case DEF_EDITOR_USERINTERRAFORMINGDIALOG: {
           break;
        }

        default: {
           break;
        }
    }
}

void TerraformingMode::OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem) {
    if (mCurrSelectedItem == nullptr)
        return;

    if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        //Are we editiing vertices right now?
        //if so draw the currently selected vertice
        if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELVERTICES) {
            size_t nrSelCells = mParentSession->mItemSelector->GetNumberSelectedCells();

            //Draw the whole selected cell
            mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrSelectedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->white);

            if (nrSelCells > 1) {
                //Draw the "cross" at the currently selected vertex with reduced size
                mParentSession->DrawCellVertexCross(mCurrSelectedItem, mParentSession->mParentEditor->mDrawDebug->cyan, 0.6f);
            } else {
                //Draw the "cross" at the currently selected vertex, with normal size
                mParentSession->DrawCellVertexCross(mCurrSelectedItem, mParentSession->mParentEditor->mDrawDebug->cyan);
            }
        }

        //If we currently do select/edit based on cells, draw the currently
        //selected cell itself
        if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELCELLS) {
            //Draw the whole selected cell
            mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrSelectedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->cyan);
        }
    }
}

void TerraformingMode::OnSelectedVertexModifyHeight(CurrentlySelectedEditorItemInfoStruct whichItem, irr::f32 deltaH) {
    if (whichItem.SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return;

    irr::s32 coordX = whichItem.mCellCoordSelected.X;
    irr::s32 coordY = whichItem.mCellCoordSelected.Y;

    irr::f32 currV1h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert1CurrPositionY;
    irr::f32 currV2h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert2CurrPositionY;
    irr::f32 currV3h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert3CurrPositionY;
    irr::f32 currV4h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert4CurrPositionY;
    irr::f32 newH = 0.0f;

    switch (whichItem.mCellCoordVerticeNrSelected) {
        case 1: {
          newH = currV1h - deltaH;
          mGuiTerraformingMode.Vertice1NEB->SetValue(-newH);
          UpdateHeightLbl(1, -newH);
          break;
        }
        case 2: {
          newH = currV2h - deltaH;
          mGuiTerraformingMode.Vertice2NEB->SetValue(-newH);
          UpdateHeightLbl(2, -newH);
          break;
        }
        case 3: {
          newH = currV3h - deltaH;
          mGuiTerraformingMode.Vertice3NEB->SetValue(-newH);
          UpdateHeightLbl(3, -newH);
          break;
        }
        case 4: {
          newH = currV4h - deltaH;
          mGuiTerraformingMode.Vertice4NEB->SetValue(-newH);
          UpdateHeightLbl(4, -newH);
          break;
        }
        default: {
          return;
        }
    }

    //After the next command we need to ourself trigger a Mesh update!
    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, whichItem.mCellCoordVerticeNrSelected, newH);
}

void TerraformingMode::OnSelectedVertexSetHeight(CurrentlySelectedEditorItemInfoStruct whichItem, irr::f32 newH) {
    if (whichItem.SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return;

    irr::s32 coordX = whichItem.mCellCoordSelected.X;
    irr::s32 coordY = whichItem.mCellCoordSelected.Y;

    switch (whichItem.mCellCoordVerticeNrSelected) {
        case 1: {
          mGuiTerraformingMode.Vertice1NEB->SetValue(-newH);
          UpdateHeightLbl(1, -newH);
          break;
        }
        case 2: {
          mGuiTerraformingMode.Vertice2NEB->SetValue(-newH);
          UpdateHeightLbl(2, -newH);
          break;
        }
        case 3: {
          mGuiTerraformingMode.Vertice3NEB->SetValue(-newH);
          UpdateHeightLbl(3, -newH);
          break;
        }
        case 4: {
          mGuiTerraformingMode.Vertice4NEB->SetValue(-newH);
          UpdateHeightLbl(4, -newH);
          break;
        }
        default: {
          return;
        }
    }

    //After the next command we need to ourself trigger a Mesh update!
    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, whichItem.mCellCoordVerticeNrSelected, newH);
}

void TerraformingMode::OnSelectedCellModifyHeight(CurrentlySelectedEditorItemInfoStruct whichItem, irr::f32 deltaH) {
    if (whichItem.SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return;

    irr::s32 coordX = whichItem.mCellCoordSelected.X;
    irr::s32 coordY = whichItem.mCellCoordSelected.Y;

    irr::f32 currV1h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert1CurrPositionY;
    irr::f32 currV2h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert2CurrPositionY;
    irr::f32 currV3h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert3CurrPositionY;
    irr::f32 currV4h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert4CurrPositionY;
    irr::f32 newH = 0.0f;
    irr::f32 avgH = 0.0f;

    newH = currV1h - deltaH;
    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 1, newH);
    UpdateHeightLbl(1, -newH);

    avgH += -newH;

    newH = currV2h - deltaH;
    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 2, newH);
    UpdateHeightLbl(2, -newH);

    avgH += -newH;

    newH = currV3h - deltaH;
    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 3, newH);
    UpdateHeightLbl(3, -newH);

    avgH += -newH;

    newH = currV4h - deltaH;
    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 4, newH);
    UpdateHeightLbl(4, -newH);

    avgH += -newH;
    avgH = avgH / 4.0f;

    mGuiTerraformingMode.CellNEB->SetValue(avgH);
}

void TerraformingMode::OnSelectedCellSetHeight(CurrentlySelectedEditorItemInfoStruct whichItem, irr::f32 newH) {
    if (whichItem.SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return;

    irr::s32 coordX = whichItem.mCellCoordSelected.X;
    irr::s32 coordY = whichItem.mCellCoordSelected.Y;

    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 1, newH);
    UpdateHeightLbl(1, -newH);

    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 2, newH);
    UpdateHeightLbl(2, -newH);

    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 3, newH);
    UpdateHeightLbl(3, -newH);

    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 4, newH);
    UpdateHeightLbl(4, -newH);
}

irr::f32 TerraformingMode::GetCurrentStepSize() {
    irr::s32 selStepSize = mGuiTerraformingMode.ComboBoxStepSize->getSelected();

    if (selStepSize == -1) {
        return 0.0f;
    }

    irr::u32 stepSize = (irr::u32)(mGuiTerraformingMode.ComboBoxStepSize->getItemData(selStepSize));

    return (EDITOR_TERRAFORMING_MINDELTA * (irr::f32)(stepSize));
}

void TerraformingMode::OnSelectedItemUp() {
    if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELVERTICES) {
        OnSelectedVertexModifyHeight(mParentSession->mItemSelector->mCurrSelectedItem, GetCurrentStepSize());

        std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
        for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
             it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end(); ++it) {
               OnSelectedVertexModifyHeight(*(*it), GetCurrentStepSize());
        }
    }

    if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELCELLS) {
        OnSelectedCellModifyHeight(mParentSession->mItemSelector->mCurrSelectedItem, GetCurrentStepSize());

        std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
        for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
             it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end(); ++it) {
               OnSelectedCellModifyHeight(*(*it), GetCurrentStepSize());
        }
    }

    mParentSession->CheckForMeshUpdate();
}

void TerraformingMode::OnSelectedItemDown() {
    if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELVERTICES) {
        OnSelectedVertexModifyHeight(mParentSession->mItemSelector->mCurrSelectedItem, -GetCurrentStepSize());

        std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
        for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
             it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end(); ++it) {
               OnSelectedVertexModifyHeight(*(*it), -GetCurrentStepSize());
        }
    }

    if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELCELLS) {
        OnSelectedCellModifyHeight(mParentSession->mItemSelector->mCurrSelectedItem, -GetCurrentStepSize());

        std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
        for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
             it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end(); ++it) {
               OnSelectedCellModifyHeight(*(*it), -GetCurrentStepSize());
        }
    }

    mParentSession->CheckForMeshUpdate();
}

void TerraformingMode::OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem) {
    //Do we want to select vertices?
    //If so draw the terrain grid
    if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELVERTICES) {
            //Draw terrain grid
            //we can only draw the grid in a limited area below
            //the users camera, because otherwise the drawing performance
            //would go down way to much
            irr::core::vector3df userCamPos = this->mParentSession->mCamera->getPosition();

            irr::core::vector2di cellPosBelowUserCamera;

            cellPosBelowUserCamera.X = (irr::s32)(-userCamPos.X / DEF_SEGMENTSIZE);
            cellPosBelowUserCamera.Y = (irr::s32)(userCamPos.Z / DEF_SEGMENTSIZE);

            mParentSession->mLevelTerrain->DrawTerrainGrid(cellPosBelowUserCamera.X, cellPosBelowUserCamera.Y, 20,
                                                                   mParentSession->mParentEditor->mDrawDebug->white);

            //09.08.2025: Keep the same order, first the BLOCK code below, then AFTERWARDS the
            //if for the Cell, Important!
            if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
                   int xCoord = mCurrHighlightedItem->mCellCoordSelected.X;
                   int yCoord = mCurrHighlightedItem->mCellCoordSelected.Y;

                   //Force selection of the terrain cell instead of the block the user has highlighted
                   //right now
                   mParentSession->mItemSelector->SelectSpecifiedCellAtCoordinate(xCoord, yCoord);
            }

            if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
                //Draw the "cross" at the currently highlighted vertex
                mParentSession->DrawCellVertexCross(mCurrHighlightedItem, mParentSession->mParentEditor->mDrawDebug->green);

                //mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrHighlightedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->white);
            }
    }

    //if we select cells then outline the currently selected cell
    if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELCELLS) {
        if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
            mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrHighlightedItem->mCellCoordSelected,
                           mParentSession->mParentEditor->mDrawDebug->white);
        }
    }
}

void TerraformingMode::OnFloatNumberEditBoxNewValue(NumberEditBox* whichBox, irr::f32& newValue) {
    if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return;

    irr::s32 coordX = mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X;
    irr::s32 coordY = mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y;

    if (whichBox == mGuiTerraformingMode.Vertice1NEB) {
        //Y-coordinate from original map has flipped sign in this project
        mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 1, -newValue);
        UpdateHeightLbl(1, -newValue);
    }

    if (whichBox == mGuiTerraformingMode.Vertice2NEB) {
        //Y-coordinate from original map has flipped sign in this project
        mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 2, -newValue);
        UpdateHeightLbl(2, -newValue);
    }

    if (whichBox == mGuiTerraformingMode.Vertice3NEB) {
        //Y-coordinate from original map has flipped sign in this project
        mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 3, -newValue);
        UpdateHeightLbl(3, -newValue);
    }

    if (whichBox == mGuiTerraformingMode.Vertice4NEB) {
        //Y-coordinate from original map has flipped sign in this project
        mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 4, -newValue);
        UpdateHeightLbl(4, -newValue);
    }

    if (whichBox == mGuiTerraformingMode.CellNEB) {
        //Y-coordinate from original map has flipped sign in this project
        mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 1, -newValue);
        mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 2, -newValue);
        mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 3, -newValue);
        mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, 4, -newValue);

        UpdateHeightLbl(1, -newValue);
        UpdateHeightLbl(2, -newValue);
        UpdateHeightLbl(3, -newValue);
        UpdateHeightLbl(4, -newValue);
    }

    if (whichBox == mGuiTerraformingMode.MultipleSelectionNEB) {
        if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELVERTICES) {
            OnSelectedVertexSetHeight(mParentSession->mItemSelector->mCurrSelectedItem, -newValue);

            std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
            for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
                 it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end(); ++it) {
                    OnSelectedVertexSetHeight(*(*it), -newValue);
            }
        } else if (mOpMode == EDITOR_TERRAFORMING_OPMODE_SELCELLS) {
            OnSelectedCellSetHeight(mParentSession->mItemSelector->mCurrSelectedItem, -newValue);

            std::vector<CurrentlySelectedEditorItemInfoStruct*>::iterator it;
            for (it = mParentSession->mItemSelector->mAdditionalSelectedItemVec.begin();
                 it != mParentSession->mItemSelector->mAdditionalSelectedItemVec.end(); ++it) {
                   OnSelectedCellSetHeight(*(*it), -newValue);
            }
        }
    }

    mParentSession->CheckForMeshUpdate();
}
