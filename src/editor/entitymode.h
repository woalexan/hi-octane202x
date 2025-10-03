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
#define EDITOR_ENTITYCAT_WAYPOINTS 3003
#define EDITOR_ENTITYCAT_UNUSED 3004

#define EDITOR_ENTITY_OPMODE_DEFAULT 0
#define EDITOR_ENTITY_OPMODE_SETMOVETARGET 1
#define EDITOR_ENTITY_OPMODE_CREATELINKSELNEXTITEM 2

/************************
 * Forward declarations *
 ************************/

class EditorSession;
class EntityMode;
class NumberEditBox;
struct GUIEntityModeEntityCategoryDataStruct;
class EditorEntity;

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
    irr::video::ITexture* texture = nullptr;
    irr::s32 guiElementId;

    bool currHovered = false;

    irr::gui::IGUIImage* guiElement = nullptr;
};

/* GUI Elements for EntityMode
*/
struct GUIEntityMode
{
    GUIEntityMode ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    /***************
     * Edit Tab    *
     ***************/

    irr::gui::IGUIComboBox* EntityItemTypeCategoryList;

    irr::gui::IGUIImage* CurrentSelectedEntitySprite;

    irr::gui::IGUIStaticText* CurrentSelectedEntityId;
    irr::gui::IGUIStaticText* CurrentlySelectedEntityTypeStr;
    irr::gui::IGUIStaticText* LabelCurrentlySelected;
    irr::gui::IGUIStaticText* LabelEntityCategory;

    irr::gui::IGUIButton* RemoveEntityButton;
    irr::gui::IGUIButton* MoveEntityButton;

    irr::gui::IGUIButton* LinkEntityButton;
    irr::gui::IGUIButton* UnlinkEntityButton;

    irr::gui::IGUITabControl* tabCntrl;
    irr::gui::IGUITab* EditTab;
    irr::gui::IGUITab* ListTab;

    /* Extra Ui Elements for Configuration of default Collectibles */
    irr::gui::IGUICheckBox* CollectibleCreateAtStart;
    NumberEditBox* GroupEditBox;
    NumberEditBox* TargetGroupEditBox;
    NumberEditBox* NextIdEditBox;
    NumberEditBox* ValueEditBox;

    //OffsetX and OffsetY according to the underlying
    //EntityItem and Levelfile seem to be a irr::f32 (float)
    //But actually when loading original levels the only contain whole integers
    //So we can get away with using NumberEditBox (does only support int) for them as well
    NumberEditBox* OffsetXEditBox;
    NumberEditBox* OffsetYEditBox;

    NumberEditBox* Unknown1EditBox;
    NumberEditBox* Unknown2EditBox;
    NumberEditBox* Unknown3EditBox;

    irr::gui::IGUICheckBox* ListWaypoints;
    irr::gui::IGUICheckBox* ListWallsegments;
    irr::gui::IGUICheckBox* ListRecoveryvehicles;
    irr::gui::IGUICheckBox* ListCones;
    irr::gui::IGUICheckBox* ListCollectibles;
    irr::gui::IGUICheckBox* ListMorphs;
    irr::gui::IGUICheckBox* ListCameras;
    irr::gui::IGUICheckBox* ListTriggers;
    irr::gui::IGUICheckBox* ListExplosions;
    irr::gui::IGUICheckBox* ListCheckpoints;

    /***************
     * List Tab    *
     ***************/

     irr::gui::IGUITable* EntityTable;
};

class EntityMode : public EditorMode {
private:
    virtual void CreateWindow();

    void NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);

    //returns true if move is succesfull, false otherwise
    bool EntityMoveTargetCellSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);

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

    void ShowUiDefaultSettings(bool visible);
    void UpdateUiDefaultSettings();

    void ChangeCreateAtStart(bool newValue);

    irr::u8 mOpMode = EDITOR_ENTITY_OPMODE_DEFAULT;

    EditorEntity* mLastSelectedEditorEntity = nullptr;

    void AddEntityTableEntry(EditorEntity* whichEntity);
    void UpdateEntitiesTable();

    bool mListWaypoints = true;
    bool mListWallsegments = true;
    bool mListRecoveryvehicles = true;
    bool mListCones = true;
    bool mListCollectibles = true;
    bool mListMorphs = true;
    bool mListCameras = true;
    bool mListTriggers = true;
    bool mListExplosions = true;
    bool mListCheckpoints = true;

    bool DoListEntityItem(EditorEntity* item);

    int16_t mLastSelectedTableEntityNr = -1;

    void HighlightEntityTableRow(EditorEntity* whichEntity);

    void UpdateUiDialog();

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
    virtual void OnTableSelected(irr::s32 elementId);

    virtual void OnNumberEditBoxNewValue(NumberEditBox* whichBox, irr::s32& newValue);

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
