/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef TEXTUREMODE_H
#define TEXTUREMODE_H

#include "irrlicht.h"
#include "../definitions.h"
#include "editormode.h"
#include <vector>
#include <cstdint>

#define EDITOR_TEXCAT_TERRAIN 1000
#define EDITOR_TEXCAT_ROCK 1001
#define EDITOR_TEXCAT_GRASS 1002
#define EDITOR_TEXCAT_WATER 1003
#define EDITOR_TEXCAT_DIRTROAD 1004
#define EDITOR_TEXCAT_ROAD 1005
#define EDITOR_TEXCAT_ROADMARKERS 1006
#define EDITOR_TEXCAT_FENCE 1007
#define EDITOR_TEXCAT_CHARGINGSTATION 1008
#define EDITOR_TEXCAT_COLUMN 1009
#define EDITOR_TEXCAT_BUILDINGS 1010
#define EDITOR_TEXCAT_SIGNS 1011
#define EDITOR_TEXCAT_WALL 1012
#define EDITOR_TEXCAT_TECHNICAL 1013

/************************
 * Forward declarations *
 ************************/

class EditorSession;
class TextureMode;
struct GUITextureModeTexDataStruct;
struct CurrentlySelectedEditorItemInfoStruct;

class TextureModeTexCategory {
private:
      TextureMode* mParent = nullptr;

      irr::u8 mIdxWidth = 0;
      irr::u8 mIdxHeight = 0;

      std::vector<GUITextureModeTexDataStruct*> mGuiTextureSelectionVec;

public:
      TextureModeTexCategory(TextureMode* parent);
      ~TextureModeTexCategory();

      void AddTexture(int16_t textureId);

      void HideAllTextures();
      void ShowAllTextures();

      //return nullptr if no texture is found
      GUITextureModeTexDataStruct* SearchTextureById(irr::s32 guiElementId);

      //return nullptr if no texture is found
      GUITextureModeTexDataStruct* FoundCurrentlyHoveredTexture();

      const wchar_t* categoryName;
      irr::u8 categoryNr;
      irr::u32 mDialogComboBoxElementId;
};

struct GUITextureModificationDataStruct {
      const wchar_t* texModificationName;
      int8_t texModValue;
      irr::u32 mDialogComboBoxElementId;
};

struct GUITextureModeTexDataStruct {
    //TextureModeTexCategory* category;
    int16_t textureId;
    irr::s32 guiElementId;

    bool currHovered = false;

    irr::gui::IGUIImage* guiElement;
};

/* GUI Elements for Editor Texture Mode
*/
struct GUITextureMode
{
    GUITextureMode ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUIComboBox* texCategoryList;
    irr::gui::IGUIComboBox* texModification;

    irr::gui::IGUIImage* CurrentSelectedTexture;
    irr::gui::IGUIStaticText* CurrentSelectedTextureIdText;
    irr::gui::IGUIStaticText* texCategoryLabel;
    irr::gui::IGUIStaticText* texModLabel;
    irr::gui::IGUIStaticText* currSelectedLabel;

    irr::gui::IGUIStaticText* LabelSelectCubeFaces;
    irr::gui::IGUIButton* SelNButton;
    irr::gui::IGUIButton* SelEButton;
    irr::gui::IGUIButton* SelSButton;
    irr::gui::IGUIButton* SelWButton;
    irr::gui::IGUIButton* SelTButton;
    irr::gui::IGUIButton* SelBButton;

    irr::gui::IGUIButton* SelColumnFloorTextureIdButton;

    irr::gui::IGUIStaticText* CurrentIlluminationValue;
};

class TextureMode : public EditorMode {
private:
    void AddTextureCategory(const wchar_t* categoryName, irr::u8 categoryNr, irr::gui::IGUIComboBox* comboBoxPntr);

    //returns nullptr if category is not found
    TextureModeTexCategory* FindTextureCategory(irr::u8 categoryNr);

    //returns nullptr if category is not found
    TextureModeTexCategory* FindTextureCategoryByGuiId(irr::u32 guiId);
    GUITextureModificationDataStruct* FindTextureModificationByGuiId(irr::u32 guiId);

    void DefineAllTextures();
    void SelectTextureModeTexCategory(TextureModeTexCategory* newCat);
    void SelectTextureModification(int8_t newSelectedTexModification);

    void OnUserChangedToNewTexture(CurrentlySelectedEditorItemInfoStruct whichItem, int16_t newTextureId);
    void NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);

    void SelectColumnFloorTexture();

    void AddTextureModification(const wchar_t* entryName, int8_t texModValue, irr::gui::IGUIComboBox* comboBoxPntr);

    /* 21.06.2025: Best categories I found so far for the available
       Texture types in the game

    Terrain:
    0, 1, 2, 3, 4, 69, 72,
    83, 123, 160

    Rock:
    87, 88, 89, 90, 91, 94, 95, 98
    99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110,
    111, 112, 113, 114, 115

    Grass:
    92, 93, 97

    Water:
    56, 57, 58, 59,  80, 84, 85, 96,
    127

    Dirtroad:
    60, 61, 62, 63, 64, 65, 66, 67,
    68

    Road:
    81, 82, 86, 129, 134, 135, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150,
    151, 152, 153, 154, 155

    Roadmarkers:
    136, 137, 138, 139, 128, 130, 131,
    120, 121, 122

    Fence:
    31, 33

    Chargingstations:
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51

    Column:
    29, 30, 32, 73

    Buildings:
    12, 13, 14, 15, 24, 25, 26, 27

    Signs:
    34, 35, 70, 71, 74, 75

    Wall:
    5, 6, 7, 8, 10, 11, 16, 17, 18, 19,
    20, 21, 22, 23, 28, 36, 37, 38, 39,
    52, 53, 54, 55, 133, 156

    Technical:
    9, 76, 77, 78, 79, 116, 117, 118, 119,
    124, 125, 126, 132*/

    //setup different texture categories available in the level editor
    std::vector<irr::u8> EditorTerrainTextures = {0, 1, 2, 3, 4, 69, 72,
                                                  83, 123, 160};

    std::vector<irr::u8> EditorRockTextures = {87, 88, 89, 90, 91, 94, 95, 98,
                                                  99, 100, 101, 102, 103, 104,
                                                  105, 106, 107, 108, 109, 110,
                                                  111, 112, 113, 114, 115};

    std::vector<irr::u8> EditorGrassTextures = {92, 93, 97};

    std::vector<irr::u8> EditorWaterTextures = {56, 57, 58, 59,  80, 84, 85, 96,
                                                127};

    std::vector<irr::u8> EditorDirtroadTextures = {60, 61, 62, 63, 64, 65, 66, 67,
                                                   68};

    std::vector<irr::u8> EditorRoadTextures = {81, 82, 86, 129, 134, 135, 140, 141, 142, 143,
                                               144, 145, 146, 147, 148, 149, 150,
                                               151, 152, 153, 154, 155};

    std::vector<irr::u8> EditorRoadmarkersTextures = {136, 137, 138, 139, 128, 130, 131,
                                                      120, 121, 122};

    std::vector<irr::u8> EditorFenceTextures = {31, 33};

    std::vector<irr::u8> EditorChargingStationTextures = {40, 41, 42, 43, 44, 45, 46, 47,
                                                          48, 49, 50, 51};

    std::vector<irr::u8> EditorColumnTextures = {29, 30, 32, 73};

    std::vector<irr::u8> EditorBuildingsTextures = {12, 13, 14, 15, 24, 25, 26, 27};

    std::vector<irr::u8> EditorSignsTextures = {34, 35, 70, 71, 74, 75};

    std::vector<irr::u8> EditorWallTextures = {5, 6, 7, 8, 10, 11, 16, 17, 18, 19,
                                               20, 21, 22, 23, 28, 36, 37, 38, 39,
                                               52, 53, 54, 55, 133, 156};

    std::vector<irr::u8> EditorTechnicalTextures = {9, 76, 77, 78, 79, 116, 117, 118, 119,
                                               124, 125, 126, 132};

    TextureModeTexCategory* mCurrShownTexCategory = nullptr;

    std::vector<GUITextureModificationDataStruct*> mTexModificationVec;

    void WindowControlBlockOptions(bool newState);
    void SelectOtherBlockFace(irr::u8 newFaceSelection);

    virtual void CreateWindow();
    
public:
    TextureMode(EditorSession* parentSession);
    virtual ~TextureMode();

    void TextureCategoryChanged(irr::u32 newSelectedGuiId);
    void TextureModificationChanged(irr::u32 newSelectedGuiId);

    //we need the following two Gui events to be able to properly select
    //textures in the texture selection dialog
    virtual void OnElementHovered(irr::s32 hoveredGuiId);
    virtual void OnElementLeft(irr::s32 leftGuiId);

    virtual void OnButtonClicked(irr::s32 buttonGuiId);

    virtual void OnLeftMouseButtonDown();

    virtual void OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem);
    virtual void OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem);

    GUITextureMode mGuiTextureMode;

    std::vector<TextureModeTexCategory*> mTexCategoryVec;

    irr::u8 mNrTexWidth;
    irr::u8 mNrTexHeight;

    irr::u32 mTexDimension;
    irr::core::recti dimTexSelectionArea;
    irr::core::vector2di mCurrentSelectedTextureImageLocation;
};

#endif // TEXTUREMODE_H
