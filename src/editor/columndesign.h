/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef COLUMNDESIGN_H
#define COLUMNDESIGN_H

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
class Column;

/* GUI Elements for Editor Column Designer
*/
struct GUIColumnDesigner
{
    GUIColumnDesigner ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUIComboBox* selColumnDefinition;
    //irr::gui::IGUIComboBox* texModification;

    //irr::gui::IGUIImage* CurrentSelectedTexture;

    irr::gui::IGUIStaticText* CurrentSelectedCellInfo;
    irr::gui::IGUIStaticText* CurrentSelectedColumnDefId;

    irr::gui::IGUIImage* blockAImageFront;
    irr::gui::IGUIImage* blockBImageFront;
    irr::gui::IGUIImage* blockCImageFront;
    irr::gui::IGUIImage* blockDImageFront;
    irr::gui::IGUIImage* blockEImageFront;
    irr::gui::IGUIImage* blockFImageFront;
    irr::gui::IGUIImage* blockGImageFront;
    irr::gui::IGUIImage* blockHImageFront;

    irr::gui::IGUIImage* blockAImageBack;
    irr::gui::IGUIImage* blockBImageBack;
    irr::gui::IGUIImage* blockCImageBack;
    irr::gui::IGUIImage* blockDImageBack;
    irr::gui::IGUIImage* blockEImageBack;
    irr::gui::IGUIImage* blockFImageBack;
    irr::gui::IGUIImage* blockGImageBack;
    irr::gui::IGUIImage* blockHImageBack;

    //irr::gui::IGUIStaticText* LabelSelectCubeFaces;
    irr::gui::IGUIButton* AddColumnButton;
    irr::gui::IGUIButton* RemoveColumnButton;
    /*irr::gui::IGUIButton* SelSButton;
    irr::gui::IGUIButton* SelWButton;
    irr::gui::IGUIButton* SelTButton;
    irr::gui::IGUIButton* SelBButton;*/

};

class ColumnDesigner : public EditorMode {
private:
    //void AddTextureCategory(const wchar_t* categoryName, irr::u8 categoryNr, irr::gui::IGUIComboBox* comboBoxPntr);

    //returns nullptr if category is not found
    //TextureModeTexCategory* FindTextureCategory(irr::u8 categoryNr);

    //returns nullptr if category is not found
    //TextureModeTexCategory* FindTextureCategoryByGuiId(irr::u32 guiId);
    //GUITextureModificationDataStruct* FindTextureModificationByGuiId(irr::u32 guiId);

    /*void DefineAllTextures();
    void SelectTextureModeTexCategory(TextureModeTexCategory* newCat);
    void SelectTextureModification(int8_t newSelectedTexModification);

    void OnUserChangedToNewTexture(CurrentlySelectedEditorItemInfoStruct whichItem, int16_t newTextureId);*/
    void NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);

    /*void AddTextureModification(const wchar_t* entryName, int8_t texModValue, irr::gui::IGUIComboBox* comboBoxPntr);*/
    
    //TextureModeTexCategory* mCurrShownTexCategory = nullptr;

    //std::vector<GUITextureModificationDataStruct*> mTexModificationVec;

    virtual void CreateWindow();
    void WindowControlColumnPreview(bool newState);
    //void SelectOtherBlockFace(irr::u8 newFaceSelection);

    irr::s32 blockAImageFrontId;
    irr::s32 blockBImageFrontId;
    irr::s32 blockCImageFrontId;
    irr::s32 blockDImageFrontId;
    irr::s32 blockEImageFrontId;
    irr::s32 blockFImageFrontId;
    irr::s32 blockGImageFrontId;
    irr::s32 blockHImageFrontId;

    irr::s32 blockAImageBackId;
    irr::s32 blockBImageBackId;
    irr::s32 blockCImageBackId;
    irr::s32 blockDImageBackId;
    irr::s32 blockEImageBackId;
    irr::s32 blockFImageBackId;
    irr::s32 blockGImageBackId;
    irr::s32 blockHImageBackId;

   void UpdateBlockPreviewGuiImages(Column* selColumn);

public:
    ColumnDesigner(EditorSession* parentSession);
    virtual ~ColumnDesigner();

    /*void TextureCategoryChanged(irr::u32 newSelectedGuiId);
    void TextureModificationChanged(irr::u32 newSelectedGuiId);*/

    //we need the following two Gui events
    virtual void OnElementHovered(irr::s32 hoveredGuiId);
    virtual void OnElementLeft(irr::s32 leftGuiId);

    virtual void OnButtonClicked(irr::s32 buttonGuiId);

    virtual void OnLeftMouseButtonDown();

    void OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem);
    void OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem);

    GUIColumnDesigner mGuiColumnDesigner;

    //std::vector<TextureModeTexCategory*> mTexCategoryVec;

    //irr::u8 mNrTexWidth;
    //irr::u8 mNrTexHeight;

    //irr::u32 mTexDimension;
    //irr::core::recti dimTexSelectionArea;
    //irr::core::vector2di mCurrentSelectedTextureImageLocation;
};

#endif // COLUMNDESIGN_H
