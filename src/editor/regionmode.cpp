/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "regionmode.h"
#include "../editorsession.h"
#include "../editor.h"
#include "itemselector.h"
#include "../resources/levelfile.h"
#include "../models/levelterrain.h"
#include "../draw/drawdebug.h"
#include "uiconversion.h"
#include "../input/numbereditbox.h"
#include <iostream>

RegionMode::RegionMode(EditorSession* parentSession) : EditorMode(parentSession) {
       mModeNameStr.append(L"Regionmode");
}

RegionMode::~RegionMode() {
    //make sure my window is hidden at the end
    HideWindow();

    //cleanup all data
    if (mGuiRegionMode.XMinBox != nullptr) {
        delete mGuiRegionMode.XMinBox;
    }

    if (mGuiRegionMode.YMinBox != nullptr) {
        delete mGuiRegionMode.YMinBox;
    }

    if (mGuiRegionMode.XMaxBox != nullptr) {
        delete mGuiRegionMode.XMaxBox;
    }

    if (mGuiRegionMode.YMaxBox != nullptr) {
        delete mGuiRegionMode.YMaxBox;
    }

    if (mGuiRegionMode.RegionTable != nullptr) {
        mGuiRegionMode.RegionTable->remove();
    }

    if (mGuiRegionMode.ApplyButton != nullptr) {
        mGuiRegionMode.ApplyButton->remove();
    }

    if (mGuiRegionMode.RegionTypeComboBoxLabel != nullptr) {
        mGuiRegionMode.RegionTypeComboBoxLabel->remove();
    }

    if (mGuiRegionMode.RegionTypeComboBox != nullptr) {
        mGuiRegionMode.RegionTypeComboBox->remove();
    }

    if (mGuiRegionMode.SelectCoord1Button != nullptr) {
        mGuiRegionMode.SelectCoord1Button->remove();
    }

    if (mGuiRegionMode.SelectCoord2Button != nullptr) {
        mGuiRegionMode.SelectCoord2Button->remove();
    }

    if (Window != nullptr) {
        //remove the window of this Mode object
        Window->remove();
    }
}

void RegionMode::AddRegionTableEntry(irr::u8 entryNr, MapTileRegionStruct* whichRegion) {
    irr::s32 nrRowIdx = mGuiRegionMode.RegionTable->getRowCount();

    //create a new row
    mGuiRegionMode.RegionTable->addRow(nrRowIdx);

    irr::core::stringw typeStr("");

    //if unused region add empty "not used" line
    if (whichRegion == nullptr) {
       irr::core::stringw entryNrStr(entryNr);
       typeStr.append(L"Unused");

       mGuiRegionMode.RegionTable->setCellText(nrRowIdx, 0, entryNrStr);
       mGuiRegionMode.RegionTable->setCellText(nrRowIdx, 1, typeStr);

       return;
    }

    irr::core::stringw regionNr(whichRegion->regionId);
    irr::core::stringw XMin(whichRegion->tileXmin);
    irr::core::stringw YMin(whichRegion->tileYmin);
    irr::core::stringw XMax(whichRegion->tileXmax);
    irr::core::stringw YMax(whichRegion->tileYmax);

    switch (whichRegion->regionType) {
        case LEVELFILE_REGION_CHARGER_SHIELD: {
            typeStr.append(L"Shield Charger");
            break;
        }

        case LEVELFILE_REGION_CHARGER_FUEL: {
            typeStr.append(L"Fuel Charger");
            break;
        }

        case LEVELFILE_REGION_CHARGER_AMMO: {
            typeStr.append(L"Ammo Charger");
            break;
        }

        case LEVELFILE_REGION_START: {
            typeStr.append(L"Race start");
            break;
        }

        default: {
           typeStr.append(L"Undefined");
           break;
        }
    }

    mGuiRegionMode.RegionTable->setCellText(nrRowIdx, 0, regionNr);
    mGuiRegionMode.RegionTable->setCellText(nrRowIdx, 1, typeStr);
    mGuiRegionMode.RegionTable->setCellText(nrRowIdx, 2, XMin);
    mGuiRegionMode.RegionTable->setCellText(nrRowIdx, 3, YMin);
    mGuiRegionMode.RegionTable->setCellText(nrRowIdx, 4, XMax);
    mGuiRegionMode.RegionTable->setCellText(nrRowIdx, 5, YMax);
}

void RegionMode::UpdateRegionTable() {
    MapTileRegionStruct* regionPntr;

    mGuiRegionMode.RegionTable->clearRows();

    //add all possible regions into the region table
    for (irr::u8 entryIdx = 0; entryIdx < 8; entryIdx++) {
        //if region with specified Id is not existing or not defined returns
        //nullptr
        regionPntr = mParentSession->mLevelRes->GetRegionStructForRegionId(entryIdx);

        AddRegionTableEntry(entryIdx, regionPntr);
    }
}

//when called the EditorMode is able to
//draw primitives via DrawDebug
void RegionMode::OnDraw() {
    //draw currently defined regions on the map
    mParentSession->mLevelTerrain->DrawMapRegions();

    //if currently a region is selected, draw its outline in white
    if (mLastSelectedRegion != nullptr) {
        mParentSession->mLevelTerrain->DrawRegionOutline(mLastSelectedRegion, mParentSession->mParentEditor->mDrawDebug->white);
    }
}

void RegionMode::UpdateUiDialog() {
    UpdateRegionTable();

    if (mLastSelectedRegion == nullptr) {
        mGuiRegionMode.ApplyButton->setEnabled(false);
        mGuiRegionMode.ApplyButton->setVisible(false);

        mGuiRegionMode.XMinBox->SetVisible(false);
        mGuiRegionMode.YMinBox->SetVisible(false);
        mGuiRegionMode.XMaxBox->SetVisible(false);
        mGuiRegionMode.YMaxBox->SetVisible(false);

        //do not preselect any region type in the ComboBox
        mGuiRegionMode.RegionTypeComboBox->setSelected(-1);

        mGuiRegionMode.SelectCoord1Button->setEnabled(false);
        mGuiRegionMode.SelectCoord1Button->setVisible(false);

        mGuiRegionMode.SelectCoord2Button->setEnabled(false);
        mGuiRegionMode.SelectCoord2Button->setVisible(false);
    } else {
        mGuiRegionMode.ApplyButton->setEnabled(true);
        mGuiRegionMode.ApplyButton->setVisible(true);

        mGuiRegionMode.XMinBox->SetVisible(true);
        mGuiRegionMode.YMinBox->SetVisible(true);
        mGuiRegionMode.XMaxBox->SetVisible(true);
        mGuiRegionMode.YMaxBox->SetVisible(true);

        //update all NumberEditBoxes with region coordinate entry
        mGuiRegionMode.XMinBox->SetValue((irr::s32)(mLastSelectedRegion->tileXmin));
        mGuiRegionMode.YMinBox->SetValue((irr::s32)(mLastSelectedRegion->tileYmin));
        mGuiRegionMode.XMaxBox->SetValue((irr::s32)(mLastSelectedRegion->tileXmax));
        mGuiRegionMode.YMaxBox->SetValue((irr::s32)(mLastSelectedRegion->tileYmax));

        mGuiRegionMode.SelectCoord1Button->setEnabled(true);
        mGuiRegionMode.SelectCoord1Button->setVisible(true);

        mGuiRegionMode.SelectCoord2Button->setEnabled(true);
        mGuiRegionMode.SelectCoord2Button->setVisible(true);

        //update region type checkbox
        irr::u32 selNewEntry = mGuiRegionMode.RegionTypeComboBox->getIndexForItemData(mLastSelectedRegion->regionType);
        mGuiRegionMode.RegionTypeComboBox->setSelected(selNewEntry);
    }

    //which line needs to be highlighted?
    //for rowIdx = -1 all rows are "unhighlighted"
    HighlightRegionTableRow(mLastSelectedTableRegionNr);
}

//for rowIdx = -1 all rows are "unhighlighted"
void RegionMode::HighlightRegionTableRow(irr::s32 rowIdx) {
    irr::s32 rowCount = mGuiRegionMode.RegionTable->getRowCount();
    irr::s32 columnCount = mGuiRegionMode.RegionTable->getColumnCount();

    for (irr::s32 idx = 0; idx < rowCount; idx++) {
        for (irr::s32 idx2 = 0; idx2 < columnCount; idx2++) {
        if (idx == rowIdx) {
            mGuiRegionMode.RegionTable->setCellColor(idx, idx2, *mParentSession->mParentEditor->mDrawDebug->white->color);
        } else {
            mGuiRegionMode.RegionTable->setCellColor(idx, idx2, *mParentSession->mParentEditor->mDrawDebug->black->color);
        }
      }
    }
}

void RegionMode::OnTableSelected(irr::s32 elementId) {
    if (elementId == GUI_ID_REGIONMODEWINDOW_REGIONTABLE) {
        //another entry in the region table was selected by the user
        //which region was selected?
        irr::s32 rowSelected = mGuiRegionMode.RegionTable->getSelected();

        if (rowSelected != -1) {
            irr::u32 regionId;

            if (mParentSession->mParentEditor->mUiConversion->GetUIntFromTableEntry(mGuiRegionMode.RegionTable, rowSelected, 0, regionId)) {
                irr::u8 regionIdSec = (irr::u8)(regionId);

                mLastSelectedTableRegionNr = (irr::s32)(regionId);

                MapTileRegionStruct* regionPntr = mParentSession->mLevelRes->GetRegionStructForRegionId(regionIdSec);
                if (regionPntr != nullptr) {
                    //set user camera right to the selected region
                    mParentSession->MoveUserViewToLocation(mParentSession->mLevelTerrain->GetRegionMiddleWorldCoordinate(regionPntr), 25.0f); 
                }

                mLastSelectedRegion = regionPntr;
            }
        }

        UpdateUiDialog();
    }
}

//Returns -1 if specified region is not found
irr::s32 RegionMode::FindRegionTableRowIdxForMapTileRegionStruct(MapTileRegionStruct* whichRegion) {
    if (whichRegion == nullptr)
        return -1;

    irr::s32 rowCount = mGuiRegionMode.RegionTable->getRowCount();
    irr::u32 regionVal;

    for (irr::s32 idx = 0; idx < rowCount; idx++) {
        if (mParentSession->mParentEditor->mUiConversion->GetUIntFromTableEntry(mGuiRegionMode.RegionTable, idx, 0, regionVal)) {
           if (whichRegion->regionId == (irr::u8)(regionVal)) {
               //we have a match
               return ((irr::s32)(idx));
           }
        }
    }

    //we found no match
    return(-1);
}

void RegionMode::CreateWindow() {
    irr::core::dimension2d<irr::u32> dim ( 400, 250 );

    //finally create the window
    Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Region Definition", 0, mGuiWindowId);

    mGuiRegionMode.RegionTable = mParentSession->mParentEditor->mGuienv->addTable( rect<s32>( 10, 30, 345, dim.Height - 90), Window, GUI_ID_REGIONMODEWINDOW_REGIONTABLE);
    mGuiRegionMode.RegionTable->addColumn ( L"Region Nr", 0 );
    mGuiRegionMode.RegionTable->addColumn ( L"Type", 1 );
    mGuiRegionMode.RegionTable->addColumn ( L"XMin", 2 );
    mGuiRegionMode.RegionTable->addColumn ( L"YMin", 3 );
    mGuiRegionMode.RegionTable->addColumn ( L"XMax", 4 );
    mGuiRegionMode.RegionTable->addColumn ( L"YMax", 5 );

    //Width of table is 280 pixels overall
    mGuiRegionMode.RegionTable->setColumnWidth ( 0, 40 );
    mGuiRegionMode.RegionTable->setColumnWidth ( 1, 100 );
    mGuiRegionMode.RegionTable->setColumnWidth ( 2, 40 );
    mGuiRegionMode.RegionTable->setColumnWidth ( 3, 40 );
    mGuiRegionMode.RegionTable->setColumnWidth ( 4, 40 );
    mGuiRegionMode.RegionTable->setColumnWidth ( 5, 40 );
    mGuiRegionMode.RegionTable->setToolTipText ( L"Shows all defined regions" );

    mGuiRegionMode.RegionTypeComboBoxLabel = mParentSession->mParentEditor->mGuienv->addStaticText ( L"Region Type:",
                                      rect<s32>( 10, dim.Height - 85, 160, dim.Height - 60 ),false, false, Window, -1, false );

    mGuiRegionMode.RegionTypeComboBox = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( 10, dim.Height - 65, 100, dim.Height - 40),
                                                                                          Window, GUI_ID_REGIONMODEWINDOW_TYPE_COMBOBOX);

    //possible types:
    //value 0 is used for undefined/unused regions
    //value 1 means Shield charger location
    //value 2 means Fuel charger location
    //value 3 means Ammo charger location
    //value 4 means map start location
    mGuiRegionMode.RegionTypeComboBox->addItem(L"Undefined", LEVELFILE_REGION_UNDEFINED);
    mGuiRegionMode.RegionTypeComboBox->addItem(L"Shield", LEVELFILE_REGION_CHARGER_SHIELD);
    mGuiRegionMode.RegionTypeComboBox->addItem(L"Fuel", LEVELFILE_REGION_CHARGER_FUEL);
    mGuiRegionMode.RegionTypeComboBox->addItem(L"Ammo", LEVELFILE_REGION_CHARGER_AMMO);
    mGuiRegionMode.RegionTypeComboBox->addItem(L"Race Start", LEVELFILE_REGION_START);

    //first select no item in ComboBox
    mGuiRegionMode.RegionTypeComboBox->setSelected(-1);

    mGuiRegionMode.RegionTypeComboBox->setEnabled(true);
    mGuiRegionMode.RegionTypeComboBox->setVisible(true);
    mGuiRegionMode.RegionTypeComboBoxLabel->setEnabled(true);
    mGuiRegionMode.RegionTypeComboBoxLabel->setVisible(true);

    irr::s32 mcx = 120;
    irr::s32 mcy = dim.Height - 80;

    //add the coordinate entry fields
    mGuiRegionMode.XMinBox = new NumberEditBox(this, L"0", rect<s32> ( mcx, mcy + 15, mcx + 35, mcy + 40), true, Window);
    mGuiRegionMode.XMinBox->SetValueLimit(0, mParentSession->mLevelRes->Width());
    mGuiRegionMode.XMinBox->AddLabel(L"Xmin:", rect<s32>( mcx, mcy - 5, mcx + 60, mcy + 10 ));
    mGuiRegionMode.XMinBox->SetVisible(false);

    mGuiRegionMode.YMinBox = new NumberEditBox(this, L"0", rect<s32> ( mcx + 45, mcy + 15, mcx + 35 + 45, mcy + 40), true, Window);
    mGuiRegionMode.YMinBox->SetValueLimit(0, mParentSession->mLevelRes->Height());
    mGuiRegionMode.YMinBox->AddLabel(L"Ymin:", rect<s32>( mcx + 45, mcy - 5, mcx + 45 + 60, mcy + 10 ));
    mGuiRegionMode.YMinBox->SetVisible(false);

    mGuiRegionMode.XMaxBox = new NumberEditBox(this, L"0", rect<s32> ( mcx + 90, mcy + 15, mcx + 35 + 90, mcy + 40), true, Window);
    mGuiRegionMode.XMaxBox->SetValueLimit(0, mParentSession->mLevelRes->Width());
    mGuiRegionMode.XMaxBox->AddLabel(L"Xmax:", rect<s32>( mcx + 90, mcy - 5, mcx + 90 + 60, mcy + 10 ));
    mGuiRegionMode.XMaxBox->SetVisible(false);

    mGuiRegionMode.YMaxBox = new NumberEditBox(this, L"0", rect<s32> ( mcx + 135, mcy + 15, mcx + 35 + 135, mcy + 40), true, Window);
    mGuiRegionMode.YMaxBox->SetValueLimit(0, mParentSession->mLevelRes->Height());
    mGuiRegionMode.YMaxBox->AddLabel(L"Ymax:", rect<s32>( mcx + 135, mcy - 5, mcx + 135 + 60, mcy + 10 ));
    mGuiRegionMode.YMaxBox->SetVisible(false);

    irr::s32 mx = mcx + 200;
    irr::s32 my = dim.Height - 64;
    mGuiRegionMode.ApplyButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my, mx + 50, my + 25), Window, GUI_ID_REGIONMODEWINDOW_APPLYBUTTON, L"Apply");

    mGuiRegionMode.SelectCoord1Button = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mcx + 1, mcy + 45, mcx + 81, mcy + 70), Window, GUI_ID_REGIONMODEWINDOW_SELECTCOORD1BUTTON, L"Set Coord1");
    mGuiRegionMode.SelectCoord2Button = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mcx + 91, mcy + 45, mcx + 171, mcy + 70), Window, GUI_ID_REGIONMODEWINDOW_SELECTCOORD2BUTTON, L"Set Coord2");

    mGuiRegionMode.SelectCoord1Button->setEnabled(false);
    mGuiRegionMode.SelectCoord1Button->setVisible(false);

    mGuiRegionMode.SelectCoord2Button->setEnabled(false);
    mGuiRegionMode.SelectCoord2Button->setVisible(false);

    //move window to a better start location
    Window->move(irr::core::vector2d<irr::s32>(850,200));

    mLastSelectedRegion = nullptr;
    UpdateUiDialog();
}

void RegionMode::OnRegionTypeComboBoxChanged(irr::u32 newSelectedGuiId) {
    //if currently a new region is defined, we need to reset value of Coordinate Entry boxes
    //and show them
    if (mLastSelectedRegion == nullptr) {
        mGuiRegionMode.XMinBox->SetValue(0);
        mGuiRegionMode.YMinBox->SetValue(0);
        mGuiRegionMode.XMaxBox->SetValue(0);
        mGuiRegionMode.YMaxBox->SetValue(0);

        mGuiRegionMode.XMinBox->SetVisible(true);
        mGuiRegionMode.YMinBox->SetVisible(true);
        mGuiRegionMode.XMaxBox->SetVisible(true);
        mGuiRegionMode.YMaxBox->SetVisible(true);

        mGuiRegionMode.ApplyButton->setVisible(true);
        mGuiRegionMode.ApplyButton->setEnabled(true);

        mGuiRegionMode.SelectCoord1Button->setVisible(true);
        mGuiRegionMode.SelectCoord1Button->setEnabled(true);

        mGuiRegionMode.SelectCoord2Button->setVisible(true);
        mGuiRegionMode.SelectCoord2Button->setEnabled(true);
    }
}

//returns true if the newly entered region coordinates in the Ui
//pass the plausi check, false otherwise
bool RegionMode::PlausiCheckCoordinates(irr::u16 XMin, irr::u16 YMin, irr::u16 XMax, irr::u16 YMax) {
    //any values outside of possible range?
    if ((XMin >= mParentSession->mLevelRes->Width()) || (XMax >= mParentSession->mLevelRes->Width())) {
            //show a messagebox with error
            mParentSession->mParentEditor->mGuienv->addMessageBox(
                            L"Issue", L"At least one X coordinate is larger then the maximum possible level width", true, EMBF_OK,
                        Window->getParent(), -1, 0);

            return false;
    }

    if ((YMin >= mParentSession->mLevelRes->Height()) || (YMax >= mParentSession->mLevelRes->Height())) {
            //show a messagebox with error
            mParentSession->mParentEditor->mGuienv->addMessageBox(
                            L"Issue", L"At least one Y coordinate is larger then the maximum possible level height", true, EMBF_OK,
                        Window->getParent(), -1, 0);

            return false;
    }

    //check if Max coordinate has actually larger coordinate values then the Min coordinate
    if (!((XMax > XMin) && (YMax > YMin))) {
        //show a messagebox with error
        mParentSession->mParentEditor->mGuienv->addMessageBox(
                        L"Issue", L"XMax/YMax coordinate values must be larger then XMin/YMin coordinate values", true, EMBF_OK,
                    Window->getParent(), -1, 0);

        return false;
    }

    //if width or height of region is not at least 3 cells
    //fail this verification
    irr::u8 width = XMax - XMin;

    if (width < 3) {
        //show a messagebox with error
        mParentSession->mParentEditor->mGuienv->addMessageBox(
                        L"Issue", L"Please make the region at least 3 cells wide in Y direction", true, EMBF_OK,
                    Window->getParent(), -1, 0);

        return false;
    }

    irr::u8 height = YMax - YMin;

    if (height < 3) {
        //show a messagebox with error
        mParentSession->mParentEditor->mGuienv->addMessageBox(
                        L"Issue", L"Please make the region at least 3 cells wide in X direction", true, EMBF_OK,
                    Window->getParent(), -1, 0);

        return false;
    }

    //Final verification: Does the new region area overlap with another existing region
    //if so this is a problem. Of course we can not do the overlap check with the region we
    //are currently editing, otherwise we always have an overlap :)
    std::vector<MapTileRegionStruct*>::iterator it;
    bool overlapFound = false;

    for (it = mParentSession->mLevelRes->mMapRegionVec->begin(); it != mParentSession->mLevelRes->mMapRegionVec->end(); ++it) {
       if (mLastSelectedRegion != (*it)) {
           //this region is not the one we currently edit => we need to check
           if (mParentSession->mLevelTerrain->Overlapping(
                       XMin, YMin, XMax, YMax, (*it)->tileXmin, (*it)->tileYmin, (*it)->tileXmax, (*it)->tileYmax)) {
               //we do overlap
               overlapFound = true;
               break;
           }
       }
    }

    if (overlapFound) {
        //show a messagebox with error
        mParentSession->mParentEditor->mGuienv->addMessageBox(
                        L"Issue", L"New region must not overlap existing one", true, EMBF_OK,
                    Window->getParent(), -1, 0);

        return false;
    }

    //all checks were ok, return success (true)
    return true;
}

void RegionMode::ApplyChanges() {
      bool delRegion = false;
      irr::u32 currentCheckBoxSelRegion;

      //currentComboBoxSelIdx == -1 means that no ComboBox entry is currently selected, which would mean for me
      //delete the currently selected region, if any region is selected in the table
      irr::s32 currentComboBoxSelIdx = mGuiRegionMode.RegionTypeComboBox->getSelected();

      if (currentComboBoxSelIdx == -1) {
          //delete a possible currently selected Region
          delRegion = true;
      } else {
          //ComboBox has a selected entry, get its data (= region type)
          currentCheckBoxSelRegion = mGuiRegionMode.RegionTypeComboBox->getItemData(currentComboBoxSelIdx);

          if (currentCheckBoxSelRegion == LEVELFILE_REGION_UNDEFINED) {
              //if region is now undefinded, also means we want to remove this entry!
              delRegion = true;
          }
      }

      //first check if we want to remove an entry, if so do it, then exit
      if (delRegion) {
          if (mLastSelectedRegion != nullptr) {
               //remove this region from the level file
               mParentSession->mLevelRes->RemoveRegion(mLastSelectedRegion);

               mLastSelectedRegion = nullptr;

               UpdateUiDialog();
           }

          return;
      }

      //if we are still in this function, we either have to create a new entry, or modify
      //the selected entry in the table

      //First plausi Check: If the newly selected region type is a new Start Region,
      //and there is already a start region present, output error message and return
      //unpossible change result (false)

      if (currentCheckBoxSelRegion == LEVELFILE_REGION_START) {
            //is there already a start region defined?
            std::vector<MapTileRegionStruct*>::iterator it;

            for (it = mParentSession->mLevelRes->mMapRegionVec->begin(); it != mParentSession->mLevelRes->mMapRegionVec->end(); ++it) {
                if ((*it)->regionType == LEVELFILE_REGION_START) {
                    //a start region is already existing
                    //change combo box to no selected item again
                    mGuiRegionMode.RegionTypeComboBox->setSelected(-1);

                    //show a messagebox with error
                    mParentSession->mParentEditor->mGuienv->addMessageBox(
                                    L"Issue", L"Can not add another race start region, before removing the existing one", true, EMBF_OK,
                                Window->getParent(), -1, 0);

                    return;
                }
            }

            //no existing start region was found, we can add a first one
            //simply continue execution below
      }

      irr::u16 newXMin = (irr::u16)(mGuiRegionMode.XMinBox->GetValue());
      irr::u16 newYMin = (irr::u16)(mGuiRegionMode.YMinBox->GetValue());
      irr::u16 newXMax = (irr::u16)(mGuiRegionMode.XMaxBox->GetValue());
      irr::u16 newYMax = (irr::u16)(mGuiRegionMode.YMaxBox->GetValue());

      //if some new entered coordinates fail plausi check exit
      if (!PlausiCheckCoordinates(newXMin, newYMin, newXMax, newYMax))
          return;

      irr::core::vector2di coord1(newXMin, newYMin);
      irr::core::vector2di coord2(newXMax, newYMax);

      //if mLastSelectedRegion == nullptr we want to add a new region
      if (mLastSelectedRegion == nullptr) {
          if (!mParentSession->mLevelRes->AddRegion(mLastSelectedTableRegionNr, coord1, coord2, currentCheckBoxSelRegion)) {
              //failed to create new Region
              mParentSession->mParentEditor->mGuienv->addMessageBox(
                              L"Issue", L"Failed to create new Region", true, EMBF_OK,
                          Window->getParent(), -1, 0);

              return;
          }

          //select the new region
          mLastSelectedRegion = mParentSession->mLevelRes->GetRegionStructForRegionId(mLastSelectedTableRegionNr);

          UpdateUiDialog();

          return;
      }

      //if we are still here we want to update an existing region parameters
      //has the region type changed?
      if (mLastSelectedRegion->regionType != currentCheckBoxSelRegion) {
          mParentSession->mLevelRes->ChangeRegionType(mLastSelectedTableRegionNr, currentCheckBoxSelRegion);
      }

      //if we need to change the location
      if ((mLastSelectedRegion->tileXmin != newXMin) || (mLastSelectedRegion->tileYmin != newYMin) ||
         (mLastSelectedRegion->tileXmax != newXMax) || (mLastSelectedRegion->tileYmax != newYMax)) {
          if (!mParentSession->mLevelRes->ChangeRegionLocation(mLastSelectedTableRegionNr, coord1, coord2)) {
              //failed to change Region location
              mParentSession->mParentEditor->mGuienv->addMessageBox(
                              L"Issue", L"Failed to change region location", true, EMBF_OK,
                          Window->getParent(), -1, 0);

              return;
          }
      }

      UpdateUiDialog();

      return;
}

void RegionMode::OnButtonClicked(irr::s32 buttonGuiId) {
    switch (buttonGuiId) {
        case GUI_ID_REGIONMODEWINDOW_APPLYBUTTON: {
             ApplyChanges();
             break;
        }

        case GUI_ID_REGIONMODEWINDOW_SELECTCOORD1BUTTON: {
                mOpMode = DEF_REGION_OPMODE_GETCOORD1;
                mParentSession->mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, true);
                break;
        }

        case GUI_ID_REGIONMODEWINDOW_SELECTCOORD2BUTTON: {
                mOpMode = DEF_REGION_OPMODE_GETCOORD2;
                mParentSession->mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, true);
                break;
        }

        default: {
            break;
        }
    }
}

void RegionMode::NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        //is our dialog already open?
        //if not open it
        ShowWindow();

        //a cell was selected
        if (mOpMode == DEF_REGION_OPMODE_GETCOORD1) {
            mGuiRegionMode.XMinBox->SetValue((irr::s32)(newItemSelected.mCellCoordSelected.X));
            mGuiRegionMode.YMinBox->SetValue((irr::s32)(newItemSelected.mCellCoordSelected.Y));

            mParentSession->mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, false);
            mOpMode = DEF_REGION_OPMODE_DEFAULT;

            return;
        }

        if (mOpMode == DEF_REGION_OPMODE_GETCOORD2) {
            mGuiRegionMode.XMaxBox->SetValue((irr::s32)(newItemSelected.mCellCoordSelected.X));
            mGuiRegionMode.YMaxBox->SetValue((irr::s32)(newItemSelected.mCellCoordSelected.Y));

            mParentSession->mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, false);
            mOpMode = DEF_REGION_OPMODE_DEFAULT;

            return;
        }

    }
}

void RegionMode::OnLeftMouseButtonDown() {
    switch (mParentSession->mUserInDialogState) {
         case DEF_EDITOR_USERINNODIALOG: {
            //is now a new level cell or vertex selected?
            if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
                    NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);
            }
            break;
         }

         case DEF_EDITOR_USERINREGIONMODEDIALOG: {
           break;
        }

        default: {
           break;
        }
    }
}

void RegionMode::OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem) {
    //Draw terrain grid
    //we can only draw the grid in a limited area below
    //the users camera, because otherwise the drawing performance
    //would go down way to much
    /*mParentSession->mLevelTerrain->DrawTerrainGrid(mCurrHighlightedItem->mCellCoordSelected.X, mCurrHighlightedItem->mCellCoordSelected.Y, 10,
                                                   mParentSession->mParentEditor->mDrawDebug->white);*/

    irr::core::vector3df userCamPos = this->mParentSession->mCamera->getPosition();

    irr::core::vector2di cellPosBelowUserCamera;

    cellPosBelowUserCamera.X = (irr::s32)(-userCamPos.X / DEF_SEGMENTSIZE);
    cellPosBelowUserCamera.Y = (irr::s32)(userCamPos.Z / DEF_SEGMENTSIZE);

    if ((mOpMode == DEF_REGION_OPMODE_GETCOORD1) || (mOpMode == DEF_REGION_OPMODE_GETCOORD2)) {

            mParentSession->mLevelTerrain->DrawTerrainGrid(cellPosBelowUserCamera.X, cellPosBelowUserCamera.Y, 20,
                                                                   mParentSession->mParentEditor->mDrawDebug->white);
    }

    //09.08.2025: Keep the same order, first the BLOCK code below, then AFTERWARDS the
    //if for the Cell, Important!
    if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
           int xCoord = mCurrHighlightedItem->mCellCoordSelected.X;
           int yCoord = mCurrHighlightedItem->mCellCoordSelected.Y;

           //Force selection of the terrain cell instead of the block the user has highlighted
           //right now
           mParentSession->mItemSelector->SelectSpecifiedCellAtCoordinate(xCoord, yCoord);
    }

     if ((mOpMode == DEF_REGION_OPMODE_GETCOORD1) || (mOpMode == DEF_REGION_OPMODE_GETCOORD2)) {
            if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
                //Draw the "cross" at the currently highlighted vertex
                mParentSession->DrawCellVertexCross(mCurrHighlightedItem, mParentSession->mParentEditor->mDrawDebug->green);

                //mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrHighlightedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->white);
            }
     }
}

//is called when the editor mode
//is entered (becomes active)
void RegionMode::OnEnterMode() {
   //when this mode is activated the first
   //time create the window
   if (Window == nullptr) {
       CreateWindow();
   } else {
       //make sure existing window is not
       //hidden
       this->ShowWindow();
   }

   //make sure we start in default operation
   //mode
   mOpMode = DEF_REGION_OPMODE_DEFAULT;
}
