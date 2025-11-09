/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef REGIONMODE_H
#define REGIONMODE_H

#include "irrlicht.h"
#include "../definitions.h"
#include "editormode.h"

#define DEF_REGION_OPMODE_DEFAULT 0
#define DEF_REGION_OPMODE_GETCOORD1 1
#define DEF_REGION_OPMODE_GETCOORD2 2

/************************
 * Forward declarations *
 ************************/

class EditorSession;
struct CurrentlySelectedEditorItemInfoStruct;
struct ColorStruct;
struct MapTileRegionStruct;

/* GUI Elements for Editor Terraforming Mode
*/
struct GUIRegionMode
{
    GUIRegionMode ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUITable* RegionTable;
    irr::gui::IGUIScrollBar* RegionTableVertScrollBar;

    irr::gui::IGUIButton* ApplyButton;

    irr::gui::IGUIStaticText* RegionTypeComboBoxLabel;
    irr::gui::IGUIComboBox* RegionTypeComboBox;

    NumberEditBox* XMinBox;
    NumberEditBox* YMinBox;
    NumberEditBox* XMaxBox;
    NumberEditBox* YMaxBox;

    irr::gui::IGUIButton* SelectCoord1Button;
    irr::gui::IGUIButton* SelectCoord2Button;
};

class RegionMode : public EditorMode {
private:
    
    virtual void CreateWindow();

    void NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);

    void UpdateRegionTable();

    //for rowIdx = -1 all rows are "unhighlighted"
    void HighlightRegionTableRow(irr::s32 rowIdx);

    void AddRegionTableEntry(irr::u8 entryNr, MapTileRegionStruct* whichRegion);

    //Returns -1 if specified region is not found
    irr::s32 FindRegionTableRowIdxForMapTileRegionStruct(MapTileRegionStruct* whichRegion);

    void ApplyChanges();

    //returns true if the newly entered region coordinates in the Ui
    //pass the plausi check, false otherwise
    bool PlausiCheckCoordinates(irr::f32 XMin, irr::f32 YMin, irr::f32 XMax, irr::f32 YMax);

    MapTileRegionStruct* mLastSelectedRegion = nullptr;

    //-1 means no entry in table selected
    irr::s32 mLastSelectedTableRegionNr = -1;

    void UpdateUiDialog();

    void ChangePositionRegionTable(irr::s32 deltaPositionCnt);

    irr::u8 mOpMode = DEF_REGION_OPMODE_DEFAULT;
    
public:
    RegionMode(EditorSession* parentSession);
    virtual ~RegionMode();

    virtual void OnButtonClicked(irr::s32 buttonGuiId);
    virtual void OnLeftMouseButtonDown();
    virtual void OnTableSelected(irr::s32 elementId);
    virtual void OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem);

    void OnRegionTypeComboBoxChanged(irr::u32 newSelectedGuiId);

    virtual void OnKeyPressedInWindow(irr::EKEY_CODE whichKeyPressed);

    //is called when the editor mode
    //is entered (becomes active)
    virtual void OnEnterMode();

    //when called the EditorMode is able to
    //draw primitives via DrawDebug
    virtual void OnDraw();

    GUIRegionMode mGuiRegionMode;
};

#endif // REGIONMODE_H
