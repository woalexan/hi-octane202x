/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef TERRAFORMING_H
#define TERRAFORMING_H

#include "irrlicht.h"
#include "../definitions.h"
#include "editormode.h"
#include <vector>
#include <cstdint>

#define EDITOR_TERRAFORMING_OPMODE_SELVERTICES 0
#define EDITOR_TERRAFORMING_OPMODE_SELCELLS 1

//Height resolution in the levelfile for vertices/tile height is (1.0f / 256.0f)
#define EDITOR_TERRAFORMING_MINDELTA 0.00390625f

#define EDITOR_TERRAFORMING_HEIGHT_SHOWNRDECIMALPLACES 5

/************************
 * Forward declarations *
 ************************/

class EditorSession;
struct CurrentlySelectedEditorItemInfoStruct;
struct ColorStruct;

/* GUI Elements for Editor Terraforming Mode
*/
struct GUITerraformingMode
{
    GUITerraformingMode ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUIButton* ButtonSelCell;
    irr::gui::IGUIButton* ButtonSelVertices;

    irr::gui::IGUIStaticText* LblSelection;
    irr::gui::IGUIStaticText* LblStepSize;
    irr::gui::IGUIComboBox* ComboBoxStepSize;

    NumberEditBox* Vertice1NEB;
    NumberEditBox* Vertice2NEB;
    NumberEditBox* Vertice3NEB;
    NumberEditBox* Vertice4NEB;

    NumberEditBox* CellNEB;

    NumberEditBox* MultipleSelectionNEB;

    irr::gui::IGUIStaticText* LblVertice1HeightInfo;
    irr::gui::IGUIStaticText* LblVertice2HeightInfo;
    irr::gui::IGUIStaticText* LblVertice3HeightInfo;
    irr::gui::IGUIStaticText* LblVertice4HeightInfo;
};

class TerraformingMode : public EditorMode {
private:

    irr::u8 mOpMode = EDITOR_TERRAFORMING_OPMODE_SELVERTICES;
    
    virtual void CreateWindow();

    void NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);
    void UpdateUiEditNumberboxes(CurrentlySelectedEditorItemInfoStruct newItemSelected);
    void UpdateHeightLbl(irr::u32 verticeNr, irr::f32 newValue);

    void OnSelectedVertexModifyHeight(CurrentlySelectedEditorItemInfoStruct whichItem, irr::f32 deltaH);
    void OnSelectedCellModifyHeight(CurrentlySelectedEditorItemInfoStruct whichItem, irr::f32 deltaH);
    void OnSelectedVertexSetHeight(CurrentlySelectedEditorItemInfoStruct whichItem, irr::f32 newH);
    void OnSelectedCellSetHeight(CurrentlySelectedEditorItemInfoStruct whichItem, irr::f32 newH);

    irr::f32 GetCurrentStepSize();

    void SetUiMultipleSelection();

public:
    TerraformingMode(EditorSession* parentSession);
    virtual ~TerraformingMode();

    virtual void OnButtonClicked(irr::s32 buttonGuiId);

    virtual void OnLeftMouseButtonDown();

    virtual void OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem);
    virtual void OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem);

    virtual void OnFloatNumberEditBoxNewValue(NumberEditBox* whichBox, irr::f32& newValue);

    void OnSelectedItemUp();
    void OnSelectedItemDown();

    //is called when the editor mode
    //is entered (becomes active)
    virtual void OnEnterMode();

    GUITerraformingMode mGuiTerraformingMode;
};

#endif // TERRAFORMING_H
