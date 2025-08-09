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
//#include "../resources/levelfile.h"
//#include "../resources/mapentry.h"
//#include <iostream>

TerraformingMode::TerraformingMode(EditorSession* parentSession) : EditorMode(parentSession) {
}

TerraformingMode::~TerraformingMode() {
    //cleanup all data
}

void TerraformingMode::CreateWindow() {
    // set skin font
    /*IGUIFont* font = env->getFont("fontlucida.png");
    if (font)
        env->getSkin()->setFont(font);*/

    irr::core::dimension2d<irr::u32> dim ( 600, 500 );

    //finally create the window
    Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Terraforming", 0, mGuiWindowId);

   /* mParentSession->mParentEditor->mGuienv->addStaticText ( L"Texture Category:",
                                      rect<s32>( dim.Width - 400, 24, dim.Width - 310, 40 ),false, false, Window, -1, false );

    mGuiTextureMode.texCategoryList = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( dim.Width - 300, 24, dim.Width - 10, 40 ),
                                                                                          Window, GUI_ID_TEXCATEGORYCOMBOBOX);

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
                                      rect<s32>( dim.Width - 400, 50, dim.Width - 310, 66 ),false, false, Window, -1, false );

    mGuiTextureMode.texModification = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( dim.Width - 300, 50, dim.Width - 10, 66 ),
                                                                                          Window, GUI_ID_TEXMODIFICATIONCOMBOBOX);

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
                     mCurrentSelectedTextureImageLocation.Y - 5),false, false, Window, -1, false );

    mGuiTextureMode.CurrentSelectedTexture = mParentSession->mParentEditor->mGuienv->addImage(mParentSession->mTexLoader->levelTex.at(0),
                                                                      mCurrentSelectedTextureImageLocation, true, Window, -1);

    //until user first selects something hide the current texture selection image
    mGuiTextureMode.CurrentSelectedTexture->setEnabled(false);
    mGuiTextureMode.CurrentSelectedTexture->setVisible(false);

    //first set empty text, only update with texture Id when first cell is clicked by the user
    mGuiTextureMode.CurrentSelectedTextureIdText =
            mParentSession->mParentEditor->mGuienv->addStaticText ( L"", rect<s32>( mCurrentSelectedTextureImageLocation.X,
                                                                         mCurrentSelectedTextureImageLocation.Y + 68,
                                                                         mCurrentSelectedTextureImageLocation.X + 70,
                                                                         mCurrentSelectedTextureImageLocation.Y + 80),
                                                                         false, false, Window, -1, false );

    mGuiTextureMode.CurrentIlluminationValue = mParentSession->mParentEditor->mGuienv->addStaticText ( L"Illumination:",
                                                                                                       rect<s32>( 80, mCurrentSelectedTextureImageLocation.Y + 68, 160, mCurrentSelectedTextureImageLocation.Y + 80),
                                                                                                       false, false, Window, -1, false );

    mGuiTextureMode.LabelSelectCubeFaces = mParentSession->mParentEditor->mGuienv->addStaticText ( L"Select Cubefaces:",
                                      rect<s32>( 105 , 35, 180, 45 ),false, false, Window, -1, false );

    irr::s32 mx = 0;
    irr::s32 my = 22;
    mGuiTextureMode.SelNButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 125, my + 30, mx + 145, my + 45), Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTN, L"N");
    mGuiTextureMode.SelWButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 105, my + 50, mx + 125, my + 65), Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTW, L"W");
    mGuiTextureMode.SelEButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 145, my + 50, mx + 165, my + 65), Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTE, L"E");
    mGuiTextureMode.SelSButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 125, my + 70, mx + 145, my + 85), Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTS, L"S");
    mGuiTextureMode.SelTButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 145, my + 30, mx + 165, my + 45), Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTT, L"T");
    mGuiTextureMode.SelBButton = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 145, my + 70, mx + 165, my + 85), Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTB, L"B");

    mGuiTextureMode.SelColumnFloorTextureIdButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 190, my + 70, mx + 300, my + 85),
                                                              Window, GUI_ID_TEXTUREWINDOW_BUTTONSELECTCOLUMNFLOORTEXID, L"Select Column FloorTex Id");*/
}

void TerraformingMode::OnButtonClicked(irr::s32 buttonGuiId) {
    switch (buttonGuiId) {
   /*     case GUI_ID_TEXTUREWINDOW_BUTTONSELECTN: {
             SelectOtherBlockFace(DEF_SELBLOCK_FACENORTH);
             break;
        }

        case GUI_ID_TEXTUREWINDOW_BUTTONSELECTE: {
             SelectOtherBlockFace(DEF_SELBLOCK_FACEEAST);
             break;
        }

        case GUI_ID_TEXTUREWINDOW_BUTTONSELECTS: {
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
        }

        case GUI_ID_TEXTUREWINDOW_BUTTONSELECTCOLUMNFLOORTEXID: {
            SelectColumnFloorTexture();
            break;
        }*/
        default: {
            break;
        }
    }
}

void TerraformingMode::NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        //is our dialog already open?
        //if not open it
        ShowWindow();

        //a cell was selected
        //which texture Id does this cell have
      /*  MapEntry* entry;
        entry = this->mParentSession->mLevelTerrain->levelRes->pMap[newItemSelected.mCellCoordSelected.X][newItemSelected.mCellCoordSelected.Y];

        wchar_t* selInfo = new wchar_t[200];

        swprintf(selInfo, 190, L"Selected Cell X = %d, Y = %d", newItemSelected.mCellCoordSelected.X, newItemSelected.mCellCoordSelected.Y);
        mParentSession->mParentEditor->UpdateStatusbarText(selInfo);

        delete[] selInfo;

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

            wchar_t* illuminationInfo = new wchar_t[50];
            swprintf(illuminationInfo, 45, L"Illumination: %d", entry->mIllumination);

            mGuiTextureMode.CurrentIlluminationValue->setText(illuminationInfo);
            delete[] illuminationInfo;

            SelectTextureModification(texMod);*/
        }
    /*} else if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
        //is our dialog already open?
        //if not open it
        ShowWindow();

        //for a selected block unhide the window block
        //options (gui elements)
        WindowControlBlockOptions(true);

        //a block face was selected
        //which texture Id does this face have
        int16_t currTextureId;
        uint8_t currTextureMod;

        wchar_t* selInfo = new wchar_t[200];

        swprintf(selInfo, 190, L"Selected Column at Cell X = %d, Y = %d", newItemSelected.mCellCoordSelected.X, newItemSelected.mCellCoordSelected.Y);
        mParentSession->mParentEditor->UpdateStatusbarText(selInfo);

        delete[] selInfo;

        if (newItemSelected.mColumnSelected != nullptr) {
            if (mParentSession->mLevelBlocks->GetTextureInfoSelectedBlock(newItemSelected.mColumnSelected,
                                                                          newItemSelected.mSelBlockNrStartingFromBase,
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
    }*/
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

      /*  case DEF_EDITOR_USERINTEXTUREDIALOG: {
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
        }*/

        default: {
           break;
        }
    }
}

void TerraformingMode::OnElementHovered(irr::s32 hoveredGuiId) {
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

void TerraformingMode::OnElementLeft(irr::s32 leftGuiId) {
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

void TerraformingMode::OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem) {
    if (mCurrSelectedItem == nullptr)
        return;

    if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        //Draw the whole selected cell
        //mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrSelectedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->cyan);

        //Draw the "cross" at the currently selected vertex
        DrawCellVertexCross(mCurrSelectedItem, mParentSession->mParentEditor->mDrawDebug->cyan);
    }

  /*  //mark the currently user selected item for texturing mode
    if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrSelectedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->blue);
    } else if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
        mParentSession->mLevelBlocks->DrawOutlineSelectedColumn(mCurrSelectedItem->mColumnSelected,
                                                         mCurrSelectedItem->mSelBlockNrStartingFromBase,
                                                         mParentSession->mParentEditor->mDrawDebug->white,
                                                         mParentSession->mParentEditor->mDrawDebug->blue,
                                                         mCurrSelectedItem->mSelBlockFaceDirection);
    }*/
}

void TerraformingMode::OnSelectedVertexUp() {
    if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return;

    irr::s32 coordX = mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X;
    irr::s32 coordY = mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y;

    irr::f32 currV1h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert1CurrPositionY;
    irr::f32 currV2h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert2CurrPositionY;
    irr::f32 currV3h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert3CurrPositionY;
    irr::f32 currV4h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert4CurrPositionY;
    irr::f32 newH = 0.0f;

    switch (mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordVerticeNrSelected) {
        case 1: {
            newH = currV1h - 0.2f;
            break;
        }
        case 2: {
            newH = currV2h - 0.2f;
            break;
        }
        case 3: {
            newH = currV3h - 0.2f;
            break;
        }
        case 4: {
            newH = currV4h - 0.2f;
            break;
        }
        default: {
            return;
        }
    }

    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordVerticeNrSelected, newH);
}

void TerraformingMode::OnSelectedVertexDown() {
    if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return;

    irr::s32 coordX = mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X;
    irr::s32 coordY = mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y;

    irr::f32 currV1h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert1CurrPositionY;
    irr::f32 currV2h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert2CurrPositionY;
    irr::f32 currV3h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert3CurrPositionY;
    irr::f32 currV4h = mParentSession->mLevelTerrain->pTerrainTiles[coordX][coordY].vert4CurrPositionY;
    irr::f32 newH = 0.0f;

    switch (mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordVerticeNrSelected) {
        case 1: {
            newH = currV1h + 0.2f;
            break;
        }
        case 2: {
            newH = currV2h + 0.2f;
            break;
        }
        case 3: {
            newH = currV3h + 0.2f;
            break;
        }
        case 4: {
            newH = currV4h + 0.2f;
            break;
        }
        default: {
            return;
        }
    }

    mParentSession->mLevelTerrain->SetNewCellVertexHeight(coordX, coordY, mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordVerticeNrSelected, newH);
}

void TerraformingMode::DrawCellVertexCross(CurrentlySelectedEditorItemInfoStruct* mSelVertex, irr::video::SMaterial* color) {
    if (mSelVertex->SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return;

    irr::core::vector3df markerPos;
    irr::core::vector3df markerPos2;
    irr::core::vector3df markerPos3;
    irr::core::vector3df markerPos4;
    irr::core::vector3df markerPos5;

    irr::s32 xSel = mSelVertex->mCellCoordSelected.X;
    irr::s32 ySel = mSelVertex->mCellCoordSelected.Y;

    TerrainTileData* tile =
            &mParentSession->mLevelTerrain->pTerrainTiles[xSel][ySel];

    TerrainTileData* tile2 = nullptr;

    int width = mParentSession->mLevelRes->Width();
    int height = mParentSession->mLevelRes->Height();

    if ((xSel + 1) < width) {
        tile2 = &mParentSession->mLevelTerrain->pTerrainTiles[xSel + 1][ySel];
    }

    TerrainTileData* tile3 = nullptr;

    if ((xSel - 1) >= 0) {
        tile3 = &mParentSession->mLevelTerrain->pTerrainTiles[xSel - 1][ySel];
    }

    TerrainTileData* tile4 = nullptr;

    if ((ySel + 1) < height) {
        tile4 = &mParentSession->mLevelTerrain->pTerrainTiles[xSel][ySel + 1];
    }

    tile4 = &mParentSession->mLevelTerrain->pTerrainTiles[xSel][ySel + 1];

    TerrainTileData* tile5 = nullptr;

    if ((ySel - 1) >= 0) {
        tile5 = &mParentSession->mLevelTerrain->pTerrainTiles[xSel][ySel - 1];
    }

    if ((tile == nullptr) || (tile2 == nullptr) || (tile3 == nullptr) || (tile4 == nullptr) || (tile5 == nullptr))
        return;

    switch (mSelVertex->mCellCoordVerticeNrSelected) {
                case 1: {
                    markerPos = tile->vert1->Pos;
                    if (tile2->vert1 != nullptr) {
                        markerPos2 = tile2->vert1->Pos;
                    } else {
                        markerPos2 = markerPos;
                    }
                    if (tile3->vert1 != nullptr) {
                        markerPos3 = tile3->vert1->Pos;
                    } else {
                        markerPos3 = markerPos;
                    }
                    if (tile4->vert1 != nullptr) {
                        markerPos4 = tile4->vert1->Pos;
                    } else {
                        markerPos4 = markerPos;
                    }
                    if (tile5->vert1 != nullptr) {
                    markerPos5 = tile5->vert1->Pos;
                    } else {
                        markerPos5 = markerPos;
                    }
                    break;
                }

                case 2: {
                    markerPos = tile->vert2->Pos;
                    if (tile2->vert2 != nullptr) {
                        markerPos2 = tile2->vert2->Pos;
                    } else {
                        markerPos2 = markerPos;
                    }
                    if (tile3->vert2 != nullptr) {
                    markerPos3 = tile3->vert2->Pos;
                    } else {
                        markerPos3 = markerPos;
                    }
                    if (tile4->vert2 != nullptr) {
                        markerPos4 = tile4->vert2->Pos;
                    } else {
                        markerPos4 = markerPos;
                    }
                    if (tile5->vert2 != nullptr) {
                        markerPos5 = tile5->vert2->Pos;
                    } else {
                        markerPos5 = markerPos;
                    }
                    break;
                }

                case 3: {
                    markerPos = tile->vert3->Pos;
                    if (tile2->vert3 != nullptr) {
                        markerPos2 = tile2->vert3->Pos;
                    } else {
                        markerPos2 = markerPos;
                    }
                    if (tile3->vert3 != nullptr) {
                        markerPos3 = tile3->vert3->Pos;
                    } else {
                        markerPos3 = markerPos;
                    }
                    if (tile4->vert3 != nullptr) {
                        markerPos4 = tile4->vert3->Pos;
                    } else {
                        markerPos4 = markerPos;
                    }
                    if (tile5->vert3 != nullptr) {
                        markerPos5 = tile5->vert3->Pos;
                    } else {
                        markerPos5 = markerPos;
                    }
                    break;
                }

                case 4: {
                    markerPos = tile->vert4->Pos;
                    if (tile2->vert4 != nullptr) {
                        markerPos2 = tile2->vert4->Pos;
                    } else {
                        markerPos2 = markerPos;
                    }
                    if (tile3->vert4 != nullptr) {
                        markerPos3 = tile3->vert4->Pos;
                    } else {
                        markerPos3 = markerPos;
                    }
                    if (tile4->vert4 != nullptr) {
                        markerPos4 = tile4->vert4->Pos;
                    } else {
                        markerPos4 = markerPos;
                    }
                    if (tile5->vert4 != nullptr) {
                        markerPos5 = tile5->vert4->Pos;
                    } else {
                        markerPos5 = markerPos;
                    }
                    break;
                }

                default: {
                   return;
                }
    }

    markerPos.X = -markerPos.X;
    markerPos.Y = -markerPos.Y + 0.05f;

    markerPos2.X = -markerPos2.X;
    markerPos2.Y = -markerPos2.Y + 0.05f;

    markerPos3.X = -markerPos3.X;
    markerPos3.Y = -markerPos3.Y + 0.05f;

    markerPos4.X = -markerPos4.X;
    markerPos4.Y = -markerPos4.Y + 0.05f;

    markerPos5.X = -markerPos5.X;
    markerPos5.Y = -markerPos5.Y + 0.05f;

    //Draw the "cross" at the selected vertex
    mParentSession->mParentEditor->mDrawDebug->Draw3DLine(markerPos, markerPos2, color);
    mParentSession->mParentEditor->mDrawDebug->Draw3DLine(markerPos, markerPos3, color);
    mParentSession->mParentEditor->mDrawDebug->Draw3DLine(markerPos, markerPos4, color);
    mParentSession->mParentEditor->mDrawDebug->Draw3DLine(markerPos, markerPos5, color);
}

void TerraformingMode::OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem) {
    //Draw terrain grid
    //we can only draw the grid in a limited area below
    //the users camera, because otherwise the drawing performance
    //would go down way to much
    /*mParentSession->mLevelTerrain->DrawTerrainGrid(mCurrHighlightedItem->mCellCoordSelected.X, mCurrHighlightedItem->mCellCoordSelected.Y, 10,
                                                   mParentSession->mParentEditor->mDrawDebug->white);*/

    irr::core::vector3df userCamPos = this->mParentSession->mCamera->getPosition();

    irr::core::vector2di cellPosBelowUserCamera;

    cellPosBelowUserCamera.X = -userCamPos.X / DEF_SEGMENTSIZE;
    cellPosBelowUserCamera.Y = userCamPos.Z / DEF_SEGMENTSIZE;

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
        DrawCellVertexCross(mCurrHighlightedItem, mParentSession->mParentEditor->mDrawDebug->green);

        //mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrHighlightedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->white);
    }
}
