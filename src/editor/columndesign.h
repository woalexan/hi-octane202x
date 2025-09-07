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
class BlockDefinition;
class ColumnDefinition;

/* GUI Elements for Editor Column Designer
*/
struct GUIColumnDesigner
{
    GUIColumnDesigner ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUIComboBox* selColumnDefinition;

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

    irr::gui::IGUIButton* AddColumnButton;
    irr::gui::IGUIButton* RemoveColumnButton;
    irr::gui::IGUIButton* ReplaceColumnButton;

    irr::gui::IGUIButton* MoveUpColumnButton;
    irr::gui::IGUIButton* MoveDownColumnButton;

    irr::gui::IGUIButton* RemoveBlockButton;
    irr::gui::IGUIButton* AddBlockButton;
};

struct ColumnDesignerColDefComboBoxEntry {
    wchar_t* entryText = nullptr;
    irr::s32 comboBoxEntryId;
    ColumnDefinition* colDefPntr = nullptr;
};

class ColumnDesigner : public EditorMode {
private:
    void CreateNewColumnDefComboBoxEntry(ColumnDefinition* whichColDef);
    void CleanupCurrentDefComboBoxEntries();

    //returns nullptr if entry is not found
    ColumnDesignerColDefComboBoxEntry* FindColDefComboBoxEntry(irr::s32 index);

    std::vector<ColumnDesignerColDefComboBoxEntry*> mColumnDefComboBoxEntryVec;

    void NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);

    virtual void CreateWindow();
    void WindowControlButtons(bool newState);

    irr::s32 blockAImageFrontId = -1;
    irr::s32 blockBImageFrontId = -1;
    irr::s32 blockCImageFrontId = -1;
    irr::s32 blockDImageFrontId = -1;
    irr::s32 blockEImageFrontId = -1;
    irr::s32 blockFImageFrontId = -1;
    irr::s32 blockGImageFrontId = -1;
    irr::s32 blockHImageFrontId = -1;

    irr::s32 blockAImageBackId = -1;
    irr::s32 blockBImageBackId = -1;
    irr::s32 blockCImageBackId = -1;
    irr::s32 blockDImageBackId = -1;
    irr::s32 blockEImageBackId = -1;
    irr::s32 blockFImageBackId = -1;
    irr::s32 blockGImageBackId = -1;
    irr::s32 blockHImageBackId = -1;

   void UpdateBlockPreviewGuiImages(Column* selColumn);
   void UpdateBlockPreviewGuiImages(ColumnDefinition* selColumnDef);

   //Stores the currently selected block for editing of the
   //currently selected column
   int mCurrBlockToBeEditedSelection = 0;

   irr::gui::IGUIImage* mBlockPreviewCurrentlyHovered = nullptr;

   irr::gui::IGUIImage* FindCurrentlyHoveredBlockPreviewImage(irr::s32 hoveredGuiId);
   void SelectBlockToEditFromBlockPreview(irr::gui::IGUIImage* currHoveredBlockPreviewImage);

   void OnSelectNewBlockForEditing(int newBlockSelectionForEditing);

   Column* mSelectedColumnForEditing = nullptr;
   BlockDefinition* mSelectedBlockDefForEditing = nullptr;

   void OnRemoveBlock();
   void OnAddBlock();
   void UpdateColumnDefComboBox();

   //returns nullptr if no valid column definition is selected
   ColumnDefinition* GetComboBoxSelectedColumnDefinition();

public:
    ColumnDesigner(EditorSession* parentSession);
    virtual ~ColumnDesigner();

    void OnColumnDefinitionComboBoxChanged(irr::u32 newSelectedGuiId);

    //we need the following two Gui events
    virtual void OnElementHovered(irr::s32 hoveredGuiId);
    virtual void OnElementLeft(irr::s32 leftGuiId);

    virtual void OnButtonClicked(irr::s32 buttonGuiId);

    virtual void OnLeftMouseButtonDown();

    void OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem);
    void OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem);

    GUIColumnDesigner mGuiColumnDesigner;
};

#endif // COLUMNDESIGN_H
