/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ENTITYMODE_H
#define ENTITYMODE_H

#include "irrlicht.h"
#include "editormode.h"
#include <vector>
#include <cstdint>
#include <string>
#include "../resources/entityitem.h"

#define EDITOR_ENTITYCAT_POWERUPS 3000
#define EDITOR_ENTITYCAT_MODELS 3001
#define EDITOR_ENTITYCAT_CONTROL 3002
#define EDITOR_ENTITYCAT_UNUSED 3003

/************************
 * Forward declarations *
 ************************/

class EditorSession;
class EntityMode;
struct GUIEntityModeEntityCategoryDataStruct;

class EntityModeEntityCategory {
private:
      EntityMode* mParent = nullptr;

      irr::u8 mIdxWidth = 0;
      irr::u8 mIdxHeight = 0;

      std::vector<GUIEntityModeEntityCategoryDataStruct*> mGuiEntityItemTypeSelectionVec;

public:
      EntityModeEntityCategory(EntityMode* parent);
      ~EntityModeEntityCategory();

      void AddEntityType(irr::video::ITexture* texture, Entity::EntityType type);

      void HideAllEntityTypes();
      void ShowAllEntityTypes();

      //return nullptr if no entity type is found
      GUIEntityModeEntityCategoryDataStruct* SearchEntityItemTypeById(irr::s32 guiElementId);

      //return nullptr if no entity type is found
      GUIEntityModeEntityCategoryDataStruct* FoundCurrentlyHoveredEntityItemType();

      const wchar_t* categoryName;
      irr::u8 categoryNr;
      irr::u32 mDialogComboBoxElementId;
};

struct GUIEntityModeEntityCategoryDataStruct {
    Entity::EntityType type;
    std::string nameType;
    irr::video::ITexture* texture;
    irr::s32 guiElementId;

    bool currHovered = false;

    irr::gui::IGUIImage* guiElement;
};

/* GUI Elements for EntityMode
*/
struct GUIEntityMode
{
    GUIEntityMode ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUIComboBox* EntityItemTypeCategoryList;

    irr::gui::IGUIImage* CurrentSelectedEntitySprite;

    //irr::gui::IGUICheckBox* RunMorphsCheckbox;
    //irr::gui::IGUICheckBox* FogCheckbox;
    //irr::gui::IGUICheckBox* IlluminationCheckBox;

    irr::gui::IGUIStaticText* CurrentlySelectedEntityTypeStr;
    irr::gui::IGUIStaticText* LabelEntityCategory;

    irr::gui::IGUIButton* RemoveEntityButton;
};

class EntityMode : public EditorMode {
private:
    virtual void CreateWindow();

    void NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);

    void AddEntityTypeCategory(const wchar_t* categoryName, irr::u8 categoryNr, irr::gui::IGUIComboBox* comboBoxPntr);
    void AddDefaultCollectable(EntityModeEntityCategory* catPntr, Entity::EntityType type);

    //returns nullptr if category is not found
    EntityModeEntityCategory* FindEntityItemTypeCategory(irr::u8 categoryNr);

    //returns nullptr if category is not found
    EntityModeEntityCategory* FindEntityItemTypeCategoryByGuiId(irr::u32 guiId);

    void DefineAllEntityItemTypes();
    void SelectEntityModeEntityItemTypeCategory(EntityModeEntityCategory* newCat);

    EntityModeEntityCategory* mCurrShownEntityCategory = nullptr;

    void ShowAllEntitiesAddButtons(bool visible);

    void OnUserChangedToNewEntity(GUIEntityModeEntityCategoryDataStruct* entitySelectedInDialog);

public:
    EntityMode(EditorSession* parentSession);
    virtual ~EntityMode();

    //is called when the editor mode
    //is exited. Can be used to disable features
    //etc. that should only be active during this
    //mode is active
    virtual void OnExitMode();

    //is called when the editor mode
    //is entered (becomes active)
    virtual void OnEnterMode();

    virtual void OnCheckBoxChanged(irr::s32 checkboxId);

    virtual void OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem);
    virtual void OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem);

    virtual void OnLeftMouseButtonDown();

    virtual void OnButtonClicked(irr::s32 buttonGuiId);

    //the entity category selection dialog needs all hover events
    //to be able to properly select the category
    void OnElementHovered(irr::s32 hoveredGuiId);

    //the entity category selection dialog needs all element left events
    //to be able to properly select the category
    void OnElementLeft(irr::s32 leftGuiId);

    void EntityCategoryChanged(irr::u32 newSelectedGuiId);

    std::vector<EntityModeEntityCategory*> mEntityCategoryVec;

    irr::u8 mNrEntityCatWidth;
    irr::u8 mNrEntityCatHeight;

    irr::u32 mEntityTypeSelectionImageDimension;
    irr::core::recti dimEntityTypeSelectionArea;

    GUIEntityMode mGuiEntityMode;

    irr::core::vector2di mCurrentSelectedEntitySpriteLocation;
};

#endif // ENTITYMODE_H
