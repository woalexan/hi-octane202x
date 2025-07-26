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

/*TextureModeTexCategory::TextureModeTexCategory(TextureMode* parent) {
    mParent = parent;

    mGuiTextureSelectionVec.clear();
}

TextureModeTexCategory::~TextureModeTexCategory() {
    //cleanup all texture info structs and Gui Elements
    std::vector<GUITextureModeTexDataStruct*>::iterator it;
    GUITextureModeTexDataStruct* pntr;

    for (it = this->mGuiTextureSelectionVec.begin(); it != this->mGuiTextureSelectionVec.end(); ) {
        pntr = (*it);

        it = mGuiTextureSelectionVec.erase(it);

        pntr->guiElement->remove();
        delete pntr;
    }
}

void TextureModeTexCategory::HideAllTextures() {
    std::vector<GUITextureModeTexDataStruct*>::iterator it;

    for (it = this->mGuiTextureSelectionVec.begin(); it != this->mGuiTextureSelectionVec.end(); ++it) {
        (*it)->guiElement->setEnabled(false);
        (*it)->guiElement->setVisible(false);

        //make sure hover variable is also reset
        (*it)->currHovered = false;
    }
}

void TextureModeTexCategory::ShowAllTextures() {
    std::vector<GUITextureModeTexDataStruct*>::iterator it;

    for (it = this->mGuiTextureSelectionVec.begin(); it != this->mGuiTextureSelectionVec.end(); ++it) {
        (*it)->guiElement->setEnabled(true);
        (*it)->guiElement->setVisible(true);

        //make sure hover variable is also reset
        (*it)->currHovered = false;
    }
}

void TextureModeTexCategory::AddTexture(int16_t textureId) {
    irr::core::vector2di newPos(mParent->dimTexSelectionArea.UpperLeftCorner.X + mIdxWidth * this->mParent->mTexDimension,
                                mParent->dimTexSelectionArea.UpperLeftCorner.Y + 60 + mIdxHeight * this->mParent->mTexDimension);

    //calculate next texture image position in the dialog
    this->mIdxWidth++;

    if (mIdxWidth >= mParent->mNrTexWidth) {
        mIdxWidth = 0;
        mIdxHeight++;
    }

    //create the new entry
    GUITextureModeTexDataStruct* newStruct = new GUITextureModeTexDataStruct();
    newStruct->textureId = textureId;

    //get the next free (available Gui Id number)
    newStruct->guiElementId = this->mParent->mParentSession->GetNextFreeGuiId();

    //newStruct->textureCategory = this;
    newStruct->guiElement =
            mParent->mParentSession->mParentEditor->mGuienv->addImage(mParent->mParentSession->mTexLoader->levelTex.at(textureId),
                                          newPos, true, mParent->mGuiTextureMode.Window, newStruct->guiElementId);

    wchar_t* textTextureID = new wchar_t[50];
    swprintf(textTextureID, 50, L"Texture Id: %d", (int)(textureId));
    newStruct->guiElement->setToolTipText(textTextureID);
    delete[] textTextureID;

    //default hide new images, and disable them so that
    //the do not sent Gui events when we go with the mouse
    //cursor over them
    newStruct->guiElement->setEnabled(false);
    newStruct->guiElement->setVisible(false);

    mGuiTextureSelectionVec.push_back(newStruct);
}

//returns nullptr if no texture is found
GUITextureModeTexDataStruct* TextureModeTexCategory::SearchTextureById(irr::s32 guiElementId) {
    GUITextureModeTexDataStruct* result = nullptr;

    std::vector<GUITextureModeTexDataStruct*>::iterator it;

    for (it = mGuiTextureSelectionVec.begin(); it != mGuiTextureSelectionVec.end(); ++it) {
      if ((*it)->guiElementId == guiElementId) {
          result = (*it);
          break;
      }
    }

    return result;
}

//returns nullptr if no texture is found
GUITextureModeTexDataStruct* TextureModeTexCategory::FoundCurrentlyHoveredTexture() {
    GUITextureModeTexDataStruct* result = nullptr;

    std::vector<GUITextureModeTexDataStruct*>::iterator it;

    for (it = mGuiTextureSelectionVec.begin(); it != mGuiTextureSelectionVec.end(); ++it) {
      if ((*it)->currHovered) {
          result = (*it);
          break;
      }
    }

    return result;
}*/

ColumnDesigner::ColumnDesigner(EditorSession* parentSession) : EditorMode(parentSession) {
    //mTexCategoryVec.clear();
    //mTexModificationVec.clear();
}

ColumnDesigner::~ColumnDesigner() {
    //cleanup all data
   /* std::vector<TextureModeTexCategory*>::iterator it;
    TextureModeTexCategory* pntr;

    for (it = this->mTexCategoryVec.begin(); it != this->mTexCategoryVec.end(); ) {
        pntr = (*it);

        it = mTexCategoryVec.erase(it);

        delete pntr;
    }

    std::vector<GUITextureModificationDataStruct*>::iterator it2;
    GUITextureModificationDataStruct* pntr2;

    for (it2 = this->mTexModificationVec.begin(); it2 != this->mTexModificationVec.end(); ) {
        pntr2 = (*it2);

        it2 = mTexModificationVec.erase(it2);

        delete pntr2;
    }*/
}
/*
void TextureMode::AddTextureModification(const wchar_t* entryName, int8_t texModValue, irr::gui::IGUIComboBox* comboBoxPntr) {
    GUITextureModificationDataStruct* newEntry = new GUITextureModificationDataStruct();
    newEntry->texModificationName = entryName;
    newEntry->texModValue = texModValue;

    //get next free Gui Element Id
    newEntry->mDialogComboBoxElementId = mParentSession->GetNextFreeGuiId();

    //add new item to the dialog checkbox
    comboBoxPntr->addItem(entryName, newEntry->mDialogComboBoxElementId );

    mTexModificationVec.push_back(newEntry);
}

void TextureMode::AddTextureCategory(const wchar_t* categoryName, irr::u8 categoryNr, irr::gui::IGUIComboBox* comboBoxPntr) {
     TextureModeTexCategory* newCat = new TextureModeTexCategory(this);
     newCat->categoryName = categoryName;
     newCat->categoryNr = categoryNr;

     //get next free Gui Element Id
     newCat->mDialogComboBoxElementId = mParentSession->GetNextFreeGuiId();

     //add new item to the dialog checkbox
     comboBoxPntr->addItem(categoryName, newCat->mDialogComboBoxElementId );

     mTexCategoryVec.push_back(newCat);
}

void TextureMode::TextureCategoryChanged(irr::u32 newSelectedGuiId) {
    TextureModeTexCategory* pntr = FindTextureCategoryByGuiId(newSelectedGuiId);

    if (pntr != nullptr) {
          SelectTextureModeTexCategory(pntr);
    }
}

void TextureMode::TextureModificationChanged(irr::u32 newSelectedGuiId) {
    GUITextureModificationDataStruct* pntr = FindTextureModificationByGuiId(newSelectedGuiId);

    if (pntr != nullptr) {
        std::cout << "new tex modification value " << (int)(pntr->texModValue) << std::endl;
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
            mParentSession->mLevelTerrain->SetCellTextureModification(
                       mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X,
                       mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y,
                       pntr->texModValue);
        } else if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
            mParentSession->mLevelBlocks->SetCubeFaceTexture(mParentSession->mItemSelector->mCurrSelectedItem.mColumnSelected,
                                                             mParentSession->mItemSelector->mCurrSelectedItem.mSelBlockNrStartingFromBase,
                                                             mParentSession->mItemSelector->mCurrSelectedItem.mSelBlockNrSkippingMissingBlocks,
                                                             mParentSession->mItemSelector->mCurrSelectedItem.mSelBlockFaceDirection, false, 0, true, pntr->texModValue);
        }
    }
}*/

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
                                                                                                 Window, GUI_ID_COLUMNSELECTIONCOMBOBOX);

    mGuiColumnDesigner.selColumnDefinition->setToolTipText ( L"Select from existing Column definitions" );

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


    /*mParentSession->mParentEditor->mGuienv->addStaticText ( L"Texture Modification:",
                                      rect<s32>( dim.Width - 400, 50, dim.Width - 310, 66 ),false, false, mGuiTextureMode.Window, -1, false );

    mGuiTextureMode.texModification = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( dim.Width - 300, 50, dim.Width - 10, 66 ), mGuiTextureMode.Window, GUI_ID_TEXMODIFICATIONCOMBOBOX);

    AddTextureModification(L"Default", 0, mGuiTextureMode.texModification);
    AddTextureModification(L"RotateNoneFlipX", 1, mGuiTextureMode.texModification);
    AddTextureModification(L"RotateNoneFlipY", 2, mGuiTextureMode.texModification);
    AddTextureModification(L"Rotate180FlipNone", 3, mGuiTextureMode.texModification);
    AddTextureModification(L"Rotate270FlipY", 4, mGuiTextureMode.texModification);
    AddTextureModification(L"Rotate90FlipNone", 5, mGuiTextureMode.texModification);
    AddTextureModification(L"Rotate270FlipNone", 6, mGuiTextureMode.texModification);
    AddTextureModification(L"Rotate90FlipY", 7, mGuiTextureMode.texModification);

    mGuiTextureMode.texModification->setToolTipText ( L"Select Texture Modification" );*/

    //configure area in dialog where the possible textures are
    //shown
    /*dimTexSelectionArea.UpperLeftCorner.set(10, 80);
    dimTexSelectionArea.LowerRightCorner.set(dim.Width - 10, dim.Height - 40);

    irr::core::dimension2d dimTexSelectionSize = dimTexSelectionArea.getSize();

    //how much texture pictures do we fit into the available texture picture selection area, round down
    //all textures have the same dimension, just take the dimension from the first one
    irr::core::dimension2d texDimension = this->mParentSession->mTexLoader->levelTex.at(0)->getSize();

    mTexDimension = texDimension.Width;

    mNrTexWidth = dimTexSelectionSize.Width / texDimension.Width;
    mNrTexHeight = dimTexSelectionSize.Height / texDimension.Height;

    DefineAllTextures();

    //preselect the Terrain textures in the dialog
    SelectTextureModeTexCategory(FindTextureCategory((irr::u8)(EDITOR_TEXCAT_TERRAIN)));

    mCurrentSelectedTextureImageLocation.set(10, 50);

    mParentSession->mParentEditor->mGuienv->addStaticText ( L"Current Selected:",
          rect<s32>( mCurrentSelectedTextureImageLocation.X,
                     mCurrentSelectedTextureImageLocation.Y - 15,
                     mCurrentSelectedTextureImageLocation.X + 70,
                     mCurrentSelectedTextureImageLocation.Y - 5),false, false, mGuiTextureMode.Window, -1, false );

    mGuiTextureMode.CurrentSelectedTexture = mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mTexLoader->levelTex.at(0),
                                                                      mCurrentSelectedTextureImageLocation, true, mGuiTextureMode.Window, -1);

    //until user first selects something hide the current texture selection image
    mGuiTextureMode.CurrentSelectedTexture->setEnabled(false);
    mGuiTextureMode.CurrentSelectedTexture->setVisible(false);

    //first set empty text, only update with texture Id when first cell is clicked by the user
    mGuiTextureMode.CurrentSelectedTextureIdText = mParentSession->mParentEditor->mGuienv->addStaticText ( L"",
                                                                                             rect<s32>( mCurrentSelectedTextureImageLocation.X,
                                                                                                        mCurrentSelectedTextureImageLocation.Y + 68,
                                                                                                        mCurrentSelectedTextureImageLocation.X + 70,
                                                                                                        mCurrentSelectedTextureImageLocation.Y + 80),false, false, mGuiTextureMode.Window, -1, false );
    irr::s32 mx = 0;
    irr::s32 my = 22;
    mGuiTextureMode.SelNButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 125, my + 30, mx + 145, my + 45), mGuiTextureMode.Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTN, L"N");
    mGuiTextureMode.SelWButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 105, my + 50, mx + 125, my + 65), mGuiTextureMode.Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTW, L"W");
    mGuiTextureMode.SelEButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 145, my + 50, mx + 165, my + 65), mGuiTextureMode.Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTE, L"E");
    mGuiTextureMode.SelSButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 125, my + 70, mx + 145, my + 85), mGuiTextureMode.Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTS, L"S");
    mGuiTextureMode.SelTButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 145, my + 30, mx + 165, my + 45), mGuiTextureMode.Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTT, L"T");
    mGuiTextureMode.SelBButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 145, my + 70, mx + 165, my + 85), mGuiTextureMode.Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTB, L"B");*/

    mGuiColumnDesigner.CurrentSelectedCellInfo = mParentSession->mParentEditor->mGuienv->addStaticText ( L"",
                                      rect<s32>( dim.Width - 400 , 55, dim.Width - 200, 65 ),false, false, Window, -1, false );

    mGuiColumnDesigner.CurrentSelectedColumnDefId = mParentSession->mParentEditor->mGuienv->addStaticText ( L"",
                                      rect<s32>( dim.Width - 400 , 65, dim.Width - 200, 75 ),false, false, Window, -1, false );

    irr::s32 mx = dim.Width - 400;
    irr::s32 my = 90;

    mGuiColumnDesigner.AddColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my, mx + 85, my + 15), Window, GUI_ID_COLUMNDESIGNER_BUTTON_ADDCOLUMN, L"Add column");

    mGuiColumnDesigner.RemoveColumnButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my, mx + 85, my + 15), Window, GUI_ID_COLUMNDESIGNER_BUTTON_REMOVECOLUMN, L"Remove column");
}

void ColumnDesigner::UpdateBlockPreviewGuiImages(Column* selColumn) {
    if (selColumn == nullptr)
        return;

    //GetBlockPreviewImage function handles everything
    //it also returns automatically the preview image for no existing block at this column position
    //in case there is no block present currently
    mGuiColumnDesigner.blockAImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 0, true));
    mGuiColumnDesigner.blockBImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 1, true));
    mGuiColumnDesigner.blockCImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 2, true));
    mGuiColumnDesigner.blockDImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 3, true));
    mGuiColumnDesigner.blockEImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 4, true));
    mGuiColumnDesigner.blockFImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 5, true));
    mGuiColumnDesigner.blockGImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 6, true));
    mGuiColumnDesigner.blockHImageFront->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 7, true));

    mGuiColumnDesigner.blockAImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 0, false));
    mGuiColumnDesigner.blockBImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 1, false));
    mGuiColumnDesigner.blockCImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 2, false));
    mGuiColumnDesigner.blockDImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 3, false));
    mGuiColumnDesigner.blockEImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 4, false));
    mGuiColumnDesigner.blockFImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 5, false));
    mGuiColumnDesigner.blockGImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 6, false));
    mGuiColumnDesigner.blockHImageBack->setImage(mParentSession->mLevelBlocks->GetBlockPreviewImage(selColumn, 7, false));
}

void ColumnDesigner::WindowControlColumnPreview(bool newState) {
    mGuiColumnDesigner.blockAImageFront->setVisible(newState);
    mGuiColumnDesigner.blockAImageFront->setEnabled(newState);

    mGuiColumnDesigner.blockBImageFront->setVisible(newState);
    mGuiColumnDesigner.blockBImageFront->setEnabled(newState);

    mGuiColumnDesigner.blockCImageFront->setVisible(newState);
    mGuiColumnDesigner.blockCImageFront->setEnabled(newState);

    mGuiColumnDesigner.blockDImageFront->setVisible(newState);
    mGuiColumnDesigner.blockDImageFront->setEnabled(newState);

    mGuiColumnDesigner.blockEImageFront->setVisible(newState);
    mGuiColumnDesigner.blockEImageFront->setEnabled(newState);

    mGuiColumnDesigner.blockFImageFront->setVisible(newState);
    mGuiColumnDesigner.blockFImageFront->setEnabled(newState);

    mGuiColumnDesigner.blockGImageFront->setVisible(newState);
    mGuiColumnDesigner.blockGImageFront->setEnabled(newState);

    mGuiColumnDesigner.blockHImageFront->setVisible(newState);
    mGuiColumnDesigner.blockHImageFront->setEnabled(newState);

    mGuiColumnDesigner.blockAImageBack->setVisible(newState);
    mGuiColumnDesigner.blockAImageBack->setEnabled(newState);

    mGuiColumnDesigner.blockBImageBack->setVisible(newState);
    mGuiColumnDesigner.blockBImageBack->setEnabled(newState);

    mGuiColumnDesigner.blockCImageBack->setVisible(newState);
    mGuiColumnDesigner.blockCImageBack->setEnabled(newState);

    mGuiColumnDesigner.blockDImageBack->setVisible(newState);
    mGuiColumnDesigner.blockDImageBack->setEnabled(newState);

    mGuiColumnDesigner.blockEImageBack->setVisible(newState);
    mGuiColumnDesigner.blockEImageBack->setEnabled(newState);

    mGuiColumnDesigner.blockFImageBack->setVisible(newState);
    mGuiColumnDesigner.blockFImageBack->setEnabled(newState);

    mGuiColumnDesigner.blockGImageBack->setVisible(newState);
    mGuiColumnDesigner.blockGImageBack->setEnabled(newState);

    mGuiColumnDesigner.blockHImageBack->setVisible(newState);
    mGuiColumnDesigner.blockHImageBack->setEnabled(newState);

    mGuiColumnDesigner.AddColumnButton->setEnabled(!newState);
    mGuiColumnDesigner.AddColumnButton->setVisible(!newState);

    mGuiColumnDesigner.RemoveColumnButton->setEnabled(newState);
    mGuiColumnDesigner.RemoveColumnButton->setVisible(newState);
}
/*
void TextureMode::SelectTextureModification(int8_t newSelectedTexModification) {
   if ((newSelectedTexModification < 0) || (newSelectedTexModification > 7))
       return;

   //set to not enabled to prevent triggering of events
   mGuiTextureMode.texModification->setEnabled(false);
   mGuiTextureMode.texModification->setSelected((irr::s32)(newSelectedTexModification));
   mGuiTextureMode.texModification->setEnabled(true);
}

void TextureMode::SelectTextureModeTexCategory(TextureModeTexCategory* newCat) {
    if (newCat == nullptr)
        return;

    //hide all categories
    std::vector<TextureModeTexCategory*>::iterator it;

    for (it = this->mTexCategoryVec.begin(); it != this->mTexCategoryVec.end(); ++it) {
        (*it)->HideAllTextures();
    }

    //only show the textures of the newly selected category
    newCat->ShowAllTextures();

    //store currently shown texture category
    mCurrShownTexCategory = newCat;
}

void TextureMode::DefineAllTextures() {

   TextureModeTexCategory* catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_TERRAIN));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorTerrainTextures.begin(); itTex != EditorTerrainTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_ROCK));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorRockTextures.begin(); itTex != EditorRockTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_GRASS));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorGrassTextures.begin(); itTex != EditorGrassTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_WATER));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorWaterTextures.begin(); itTex != EditorWaterTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_DIRTROAD));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorDirtroadTextures.begin(); itTex != EditorDirtroadTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_ROAD));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorRoadTextures.begin(); itTex != EditorRoadTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_ROADMARKERS));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorRoadmarkersTextures.begin(); itTex != EditorRoadmarkersTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_FENCE));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorFenceTextures.begin(); itTex != EditorFenceTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_CHARGINGSTATION));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorChargingStationTextures.begin(); itTex != EditorChargingStationTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_COLUMN));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorColumnTextures.begin(); itTex != EditorColumnTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_BUILDINGS));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorBuildingsTextures.begin(); itTex != EditorBuildingsTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_SIGNS));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorSignsTextures.begin(); itTex != EditorSignsTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_WALL));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorWallTextures.begin(); itTex != EditorWallTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }

   catPntr = this->FindTextureCategory((irr::u8)(EDITOR_TEXCAT_TECHNICAL));
   if (catPntr != nullptr) {
        for (std::vector<irr::u8>::iterator itTex = EditorTechnicalTextures.begin(); itTex != EditorTechnicalTextures.end(); ++itTex) {
            catPntr->AddTexture((*itTex));
        }
   }
}

//returns nullptr if category is not found
TextureModeTexCategory* TextureMode::FindTextureCategory(irr::u8 categoryNr) {
    std::vector<TextureModeTexCategory*>::iterator it;

    TextureModeTexCategory* result = nullptr;

    for (it = mTexCategoryVec.begin(); it != mTexCategoryVec.end(); ++it) {
        if ((*it)->categoryNr == categoryNr) {
            result = (*it);
            break;
        }
    }

    return result;
}

//returns nullptr if category is not found
TextureModeTexCategory* TextureMode::FindTextureCategoryByGuiId(irr::u32 guiId) {
    std::vector<TextureModeTexCategory*>::iterator it;

    TextureModeTexCategory* result = nullptr;

    for (it = mTexCategoryVec.begin(); it != mTexCategoryVec.end(); ++it) {
      if ((*it)->mDialogComboBoxElementId == guiId) {
          result = (*it);
          break;
      }
    }

    return result;
}

//returns nullptr if texture modification setting is not found
GUITextureModificationDataStruct* TextureMode::FindTextureModificationByGuiId(irr::u32 guiId) {
    std::vector<GUITextureModificationDataStruct*>::iterator it;

    GUITextureModificationDataStruct* result = nullptr;

    for (it = mTexModificationVec.begin(); it != mTexModificationVec.end(); ++it) {
      if ((*it)->mDialogComboBoxElementId == guiId) {
          result = (*it);
          break;
      }
    }

    return result;
}

void TextureMode::SelectOtherBlockFace(irr::u8 newFaceSelection) {
    //only continue if actually a block is currently selected
    if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType != DEF_EDITOR_SELITEM_BLOCK)
        return;

    mParentSession->mItemSelector->mCurrSelectedItem.mSelBlockFaceDirection = newFaceSelection;

    //update window information
    NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);
}*/

void ColumnDesigner::OnButtonClicked(irr::s32 buttonGuiId) {
    switch (buttonGuiId) {
        case GUI_ID_COLUMNDESIGNER_BUTTON_REMOVECOLUMN: {
             if ((mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) &&
                 (mParentSession->mItemSelector->mCurrSelectedItem.mColumnSelected != nullptr)) {
                    mParentSession->mLevelBlocks->RemoveColumn(mParentSession->mItemSelector->mCurrSelectedItem.mColumnSelected);
             }
             break;
        }

        case GUI_ID_COLUMNDESIGNER_BUTTON_ADDCOLUMN: {
             if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
                    ColumnDefinition* colDef = mParentSession->mLevelRes->ColumnDefinitions.at(2);
                    mParentSession->mLevelBlocks->AddColumnAtCell(mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X,
                                                                  mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y,
                                                                  colDef);
             }
             break;
        }

       /* case GUI_ID_TEXTUREWINDOW_BUTTONSELECTS: {
             SelectOtherBlockFace(DEF_SELBLOCK_FACESOUTH);
             break;
        }

        case GUI_ID_TEXTUREWINDOW_BUTTONSELECTW: {
             SelectOtherBlockFace(DEF_SELBLOCK_FACEWEST);
             break;
        }

        case GUI_ID_TEXTUREWINDOW_BUTTONSELECTT: {
             SelectOtherBlockFace(DEF_SELBLOCK_FACETOP);
             break;
        }

        case GUI_ID_TEXTUREWINDOW_BUTTONSELECTB: {
            SelectOtherBlockFace(DEF_SELBLOCK_FACEBOTTOM);
            break;
        }*/
    }
}

void ColumnDesigner::NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        //is our dialog already open?
        //if not open it
        ShowWindow();

        //for a selected cell hide the window block
        //options (gui elements)
        WindowControlColumnPreview(false);

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
        WindowControlColumnPreview(true);

        if ((newItemSelected.mColumnSelected != nullptr) && (newItemSelected.mColumnSelected->Definition != nullptr)) {
                wchar_t* selInfo = new wchar_t[200];
                swprintf(selInfo, 190, L"Column at Cell X = %d, Y = %d, Id = %d",
                         newItemSelected.mCellCoordSelected.X, newItemSelected.mCellCoordSelected.Y,
                         newItemSelected.mColumnSelected->Definition->get_ID());

                mParentSession->mParentEditor->UpdateStatusbarText(selInfo);

                mGuiColumnDesigner.CurrentSelectedColumnDefId->setText(selInfo);

                delete[] selInfo;

                UpdateBlockPreviewGuiImages(newItemSelected.mColumnSelected);
        }
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
           /* //are we hovering right now over exactly one of my
            //texture selection images?
            if (mCurrShownTexCategory != nullptr) {
                //returns nullptr if no texture image in the dialog
                //is currently hovered with the mouse
                GUITextureModeTexDataStruct* pntr = mCurrShownTexCategory->FoundCurrentlyHoveredTexture();
                if (pntr != nullptr) {
                    //some new texture image was selected
                    //handle action inside this function call
                    OnUserChangedToNewTexture(mParentSession->mItemSelector->mCurrSelectedItem, pntr->textureId);
                }
            }*/

           break;
        }

        default: {
           break;
        }
    }
}

void ColumnDesigner::OnElementHovered(irr::s32 hoveredGuiId) {
    /*if (mCurrShownTexCategory == nullptr)
        return;

    if (mParentSession->mUserInDialogState != DEF_EDITOR_USERINTEXTUREDIALOG)
        return;

    //which texture was hovered?

    //returns nullptr if no texture is found
    GUITextureModeTexDataStruct* pntr =  mCurrShownTexCategory->SearchTextureById(hoveredGuiId);

    if (pntr == nullptr)
        return;

    //we found the newly hovered texture image in the
    //dialog
    pntr->currHovered = true;*/
}

void ColumnDesigner::OnElementLeft(irr::s32 leftGuiId) {
   /* if (mCurrShownTexCategory == nullptr)
        return;

    //which texture image was left?

    //returns nullptr if no texture is found
    GUITextureModeTexDataStruct* pntr =  mCurrShownTexCategory->SearchTextureById(leftGuiId);

    if (pntr == nullptr)
        return;

    //we found the new left texture image in the
    //dialog
    pntr->currHovered = false;*/
}

void ColumnDesigner::OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem) {
    if (mCurrSelectedItem == nullptr)
        return;

    if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrSelectedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->orange);
    } else if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
             mParentSession->mLevelBlocks->DrawColumnSelectionGrid(mCurrSelectedItem->mColumnSelected, mParentSession->mParentEditor->mDrawDebug->orange);
        }
}

void ColumnDesigner::OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem) {
    if (mCurrHighlightedItem == nullptr)
        return;

    if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrHighlightedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->white);
    } else if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
         mParentSession->mLevelBlocks->DrawColumnSelectionGrid(mCurrHighlightedItem->mColumnSelected, mParentSession->mParentEditor->mDrawDebug->cyan);
    }
}

/*
void TextureMode::OnUserChangedToNewTexture(CurrentlySelectedEditorItemInfoStruct whichItem, int16_t newTextureId) {
   //only trigger action if either a cell or a block face is currently selected
   if (whichItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
       std::cout << "changed Cell TexID to " << newTextureId << std::endl;
       mParentSession->mLevelTerrain->SetCellTexture(whichItem.mCellCoordSelected.X, whichItem.mCellCoordSelected.Y, newTextureId);
   } else if (whichItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
       std::cout << "changed block TexID to " << newTextureId << std::endl;
       mParentSession->mLevelBlocks->SetCubeFaceTexture(
                   whichItem.mColumnSelected, whichItem.mSelBlockNrStartingFromBase, whichItem.mSelBlockNrSkippingMissingBlocks,
                   whichItem.mSelBlockFaceDirection, true, newTextureId, false, 0);
   }
}
*/
