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

    /*irr::gui::IGUIComboBox* texCategoryList;
    irr::gui::IGUIComboBox* texModification;

    irr::gui::IGUIImage* CurrentSelectedTexture;
    irr::gui::IGUIStaticText* CurrentSelectedTextureIdText;

    irr::gui::IGUIStaticText* LabelSelectCubeFaces;
    irr::gui::IGUIButton* SelNButton;
    irr::gui::IGUIButton* SelEButton;
    irr::gui::IGUIButton* SelSButton;
    irr::gui::IGUIButton* SelWButton;
    irr::gui::IGUIButton* SelTButton;
    irr::gui::IGUIButton* SelBButton;

    irr::gui::IGUIButton* SelColumnFloorTextureIdButton;

    irr::gui::IGUIStaticText* CurrentIlluminationValue;*/
};

class TerraformingMode : public EditorMode {
private:
    
    virtual void CreateWindow();

    void NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);
    void DrawCellVertexCross(CurrentlySelectedEditorItemInfoStruct* mSelVertex, ColorStruct* color);
    
public:
    TerraformingMode(EditorSession* parentSession);
    virtual ~TerraformingMode();

    virtual void OnElementHovered(irr::s32 hoveredGuiId);
    virtual void OnElementLeft(irr::s32 leftGuiId);

    virtual void OnButtonClicked(irr::s32 buttonGuiId);

    virtual void OnLeftMouseButtonDown();

    virtual void OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem);
    virtual void OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem);

    void OnSelectedVertexUp();
    void OnSelectedVertexDown();

    GUITerraformingMode mGuiTerraformingMode;
};

#endif // TERRAFORMING_H
