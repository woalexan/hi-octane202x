/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "texturemode.h"
#include "../editorsession.h"
#include "../editor.h"
#include "../resources/texture.h"
#include "itemselector.h"
#include "../models/levelterrain.h"
#include "../models/levelblocks.h"
#include "../resources/levelfile.h"
#include "../resources/mapentry.h"
#include <iostream>

TextureModeTexCategory::TextureModeTexCategory(TextureMode* parent) {
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
}

TextureMode::TextureMode(EditorSession* parentSession) {
    mParentSession = parentSession;

    mTexCategoryVec.clear();
    mTexModificationVec.clear();
}

TextureMode::~TextureMode() {
    //cleanup all data
    std::vector<TextureModeTexCategory*>::iterator it;
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
    }
}

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
        }
    }
}

bool TextureMode::IsWindowOpen() {
    if (mGuiTextureMode.Window != nullptr)
        return true;

    return false;
}

irr::core::rect<irr::s32> TextureMode::GetWindowPosition() {
   if (mGuiTextureMode.Window != nullptr) {
    return mGuiTextureMode.Window->getAbsolutePosition();
   } else
    return (irr::core::rect<irr::s32>(0, 0, 0, 0));
}

void TextureMode::CreateWindow() {
    mGuiTextureMode.drop();

    // set skin font
    /*IGUIFont* font = env->getFont("fontlucida.png");
    if (font)
        env->getSkin()->setFont(font);*/

    irr::core::dimension2d<irr::u32> dim ( 600, 500 );

    mGuiTextureMode.Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Textures", 0, GUI_ID_TEXTUREWINDOW);

    mParentSession->mParentEditor->mGuienv->addStaticText ( L"Texture Category:",
                                      rect<s32>( dim.Width - 400, 24, dim.Width - 310, 40 ),false, false, mGuiTextureMode.Window, -1, false );

    mGuiTextureMode.texCategoryList = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( dim.Width - 300, 24, dim.Width - 10, 40 ), mGuiTextureMode.Window, GUI_ID_TEXCATEGORYCOMBOBOX);

    AddTextureCategory(L"Terrain", (irr::u8)(EDITOR_TEXCAT_TERRAIN), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Rock", (irr::u8)(EDITOR_TEXCAT_ROCK), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Grass", (irr::u8)(EDITOR_TEXCAT_GRASS), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Water", (irr::u8)(EDITOR_TEXCAT_WATER), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Dirtroad", (irr::u8)(EDITOR_TEXCAT_DIRTROAD), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Road", (irr::u8)(EDITOR_TEXCAT_ROAD), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Roadmarkers", (irr::u8)(EDITOR_TEXCAT_ROADMARKERS), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Fence", (irr::u8)(EDITOR_TEXCAT_FENCE), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Chargingstation", (irr::u8)(EDITOR_TEXCAT_CHARGINGSTATION), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Column", (irr::u8)(EDITOR_TEXCAT_COLUMN), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Buildings", (irr::u8)(EDITOR_TEXCAT_BUILDINGS), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Signs", (irr::u8)(EDITOR_TEXCAT_SIGNS), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Wall", (irr::u8)(EDITOR_TEXCAT_WALL), mGuiTextureMode.texCategoryList);
    AddTextureCategory(L"Technical", (irr::u8)(EDITOR_TEXCAT_TECHNICAL), mGuiTextureMode.texCategoryList);

    mGuiTextureMode.texCategoryList->setToolTipText ( L"Select Texture Category" );

    mParentSession->mParentEditor->mGuienv->addStaticText ( L"Texture Modification:",
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

    mGuiTextureMode.texModification->setToolTipText ( L"Select Texture Modification" );

    //configure area in dialog where the possible textures are
    //shown
    dimTexSelectionArea.UpperLeftCorner.set(10, 80);
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

    //gui.testButton = mParentEditor->mGuienv->addButton(core::recti(50, 50, 100, 80), gui.Window, GUI_ID_TESTBUTTON, L"Test");
}

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

void TextureMode::NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        //is our dialog already open?
        //if not open it
        if (mGuiTextureMode.Window == nullptr)
            CreateWindow();

        //a cell was selected
        //which texture Id does this cell have
        MapEntry* entry;
        entry = this->mParentSession->mLevelTerrain->levelRes->pMap[newItemSelected.mCellCoordSelected.X][newItemSelected.mCellCoordSelected.Y];

        if (entry != nullptr) {
            int16_t texId = entry->m_TextureId;
            int8_t texMod = entry->m_TextureModification;

            //update image of currently selected texture
            mGuiTextureMode.CurrentSelectedTexture->setImage(mParentSession->mTexLoader->levelTex.at(texId));
            mGuiTextureMode.CurrentSelectedTexture->setEnabled(true);
            mGuiTextureMode.CurrentSelectedTexture->setVisible(true);

            wchar_t* textTextureID = new wchar_t[50];
            swprintf(textTextureID, 50, L"Texture Id: %d", (int)(texId));
            mGuiTextureMode.CurrentSelectedTextureIdText->setText(textTextureID);
            delete[] textTextureID;

            SelectTextureModification(texMod);
        }
    } else if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
        //is our dialog already open?
        //if not open it
        if (mGuiTextureMode.Window == nullptr)
            CreateWindow();

        //a block face was selected
        //which texture Id does this face have
        int16_t currTextureId;
        uint8_t currTextureMod;

        if (newItemSelected.mColumnSelected != nullptr) {
            if (mParentSession->mLevelBlocks->GetTextureInfoSelectedBlock(newItemSelected.mColumnSelected,
                                                                          newItemSelected.mSelBlockNrStartingFromBase,
                                                                          newItemSelected.mSelBlockNrSkippingMissingBlocks,
                                                                          newItemSelected.mSelBlockFaceDirection,
                                                                          currTextureId,
                                                                          currTextureMod)) {
                    //we received the block face information
                   //update image of currently selected texture
                   mGuiTextureMode.CurrentSelectedTexture->setImage(mParentSession->mTexLoader->levelTex.at(currTextureId));
                   mGuiTextureMode.CurrentSelectedTexture->setEnabled(true);
                   mGuiTextureMode.CurrentSelectedTexture->setVisible(true);

                   wchar_t* textTextureID = new wchar_t[50];
                   swprintf(textTextureID, 50, L"Texture Id: %d", (int)(currTextureId));
                   mGuiTextureMode.CurrentSelectedTextureIdText->setText(textTextureID);
                   delete[] textTextureID;

                   SelectTextureModification(currTextureMod);
            }
        }
    }
}

void TextureMode::OnLeftMouseButtonDown() {
    switch (mParentSession->mUserInDialogState) {
         case DEF_EDITOR_USERINNODIALOG: {
            //is now a new level cell or block selected?
            if ((mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) ||
               (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK)) {
                    NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);
            }

            break;
         }

        case DEF_EDITOR_USERINTEXTUREDIALOG: {
            //are we hovering right now over exactly one of my
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
            }

           break;
        }

        default: {
           break;
        }
    }
}

void TextureMode::OnElementHovered(irr::u32 hoveredGuiId) {
    if (mCurrShownTexCategory == nullptr)
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
    pntr->currHovered = true;
}

void TextureMode::OnElementLeft(irr::u32 leftGuiId) {
    if (mCurrShownTexCategory == nullptr)
        return;

    //which texture image was left?

    //returns nullptr if no texture is found
    GUITextureModeTexDataStruct* pntr =  mCurrShownTexCategory->SearchTextureById(leftGuiId);

    if (pntr == nullptr)
        return;

    //we found the new left texture image in the
    //dialog
    pntr->currHovered = false;
}

void TextureMode::OnUserChangedToNewTexture(CurrentlySelectedEditorItemInfoStruct whichItem, int16_t newTextureId) {
   //only trigger action if either a cell or a block face is currently selected
   if (whichItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
       std::cout << "changed Cell TexID to " << newTextureId << std::endl;
       mParentSession->mLevelTerrain->SetCellTexture(whichItem.mCellCoordSelected.X, whichItem.mCellCoordSelected.Y, newTextureId);
   } else if (whichItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
       std::cout << "changed block TexID to " << newTextureId << std::endl;
   }
}
