/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "entitymode.h"
#include "../editorsession.h"
#include "../editor.h"
#include <iostream>
#include "../editor/itemselector.h"
#include "../draw/drawdebug.h"
#include "../models/editorentity.h"
#include "../resources/texture.h"
#include "../models/entitymanager.h"
#include <stdlib.h>
#include "../models/levelterrain.h"
#include <locale.h>

EntityModeEntityCategory::EntityModeEntityCategory(EntityMode* parent) {
    mParent = parent;

    mGuiEntityItemTypeSelectionVec.clear();
}

EntityModeEntityCategory::~EntityModeEntityCategory() {
    //cleanup all EntityItem types info structs and Gui Elements
    std::vector<GUIEntityModeEntityCategoryDataStruct*>::iterator it;
    GUIEntityModeEntityCategoryDataStruct* pntr;

    for (it = this->mGuiEntityItemTypeSelectionVec.begin(); it != this->mGuiEntityItemTypeSelectionVec.end(); ) {
        pntr = (*it);

        it = mGuiEntityItemTypeSelectionVec.erase(it);

        pntr->guiElement->remove();
        delete pntr;
    }
}

void EntityModeEntityCategory::HideAllEntityTypes() {
    std::vector<GUIEntityModeEntityCategoryDataStruct*>::iterator it;

    for (it = this->mGuiEntityItemTypeSelectionVec.begin(); it != this->mGuiEntityItemTypeSelectionVec.end(); ++it) {
        (*it)->guiElement->setEnabled(false);
        (*it)->guiElement->setVisible(false);

        //make sure hover variable is also reset
        (*it)->currHovered = false;
    }
}

void EntityModeEntityCategory::ShowAllEntityTypes() {
    std::vector<GUIEntityModeEntityCategoryDataStruct*>::iterator it;

    for (it = this->mGuiEntityItemTypeSelectionVec.begin(); it != this->mGuiEntityItemTypeSelectionVec.end(); ++it) {
        (*it)->guiElement->setEnabled(true);
        (*it)->guiElement->setVisible(true);

        //make sure hover variable is also reset
        (*it)->currHovered = false;
    }
}

void EntityModeEntityCategory::AddEntityType(irr::video::ITexture* texture, Entity::EntityType type) {
    irr::core::vector2di newPos(mParent->dimEntityTypeSelectionArea.UpperLeftCorner.X + mIdxWidth * this->mParent->mEntityTypeSelectionImageDimension,
                                mParent->dimEntityTypeSelectionArea.UpperLeftCorner.Y + 60 + mIdxHeight * this->mParent->mEntityTypeSelectionImageDimension);

    //calculate next entity type image position in the dialog
    this->mIdxWidth++;

    if (mIdxWidth >= mParent->mNrEntityCatWidth) {
        mIdxWidth = 0;
        mIdxHeight++;
    }

    //create the new entry
    GUIEntityModeEntityCategoryDataStruct* newStruct = new GUIEntityModeEntityCategoryDataStruct();
    newStruct->type = type;
    newStruct->texture = texture;

    std::string nameOfType = mParent->mParentSession->mEntityManager->GetNameForEntityType(type);

    newStruct->nameType.clear();
    newStruct->nameType.append(nameOfType);

    //get the next free (available Gui Id number)
    newStruct->guiElementId = this->mParent->mParentSession->GetNextFreeGuiId();

    irr::core::rect<s32> imageLocation(newPos.X, newPos.Y, newPos.X + this->mParent->mEntityTypeSelectionImageDimension,
                                   newPos.Y + this->mParent->mEntityTypeSelectionImageDimension);

    newStruct->guiElement =
               mParent->mParentSession->mParentEditor->mGuienv->addImage(imageLocation, mParent->Window, newStruct->guiElementId,
                                                L"", true);

    newStruct->guiElement->setScaleImage(true);
    newStruct->guiElement->setImage(texture);

    wchar_t* textDescription = new wchar_t[100];
    swprintf(textDescription, 100, L"%s", newStruct->nameType.c_str());
    newStruct->guiElement->setToolTipText(textDescription);
    delete[] textDescription;

    //default hide new images, and disable them so that
    //the do not sent Gui events when we go with the mouse
    //cursor over them
    newStruct->guiElement->setEnabled(false);
    newStruct->guiElement->setVisible(false);

    mGuiEntityItemTypeSelectionVec.push_back(newStruct);
}

//returns nullptr if no entity type item is found
GUIEntityModeEntityCategoryDataStruct* EntityModeEntityCategory::SearchEntityItemTypeById(irr::s32 guiElementId) {
    GUIEntityModeEntityCategoryDataStruct* result = nullptr;

    std::vector<GUIEntityModeEntityCategoryDataStruct*>::iterator it;

    for (it = mGuiEntityItemTypeSelectionVec.begin(); it != mGuiEntityItemTypeSelectionVec.end(); ++it) {
      if ((*it)->guiElementId == guiElementId) {
          result = (*it);
          break;
      }
    }

    return result;
}

//returns nullptr if no entity type item is found
GUIEntityModeEntityCategoryDataStruct* EntityModeEntityCategory::FoundCurrentlyHoveredEntityItemType() {
    GUIEntityModeEntityCategoryDataStruct* result = nullptr;

    std::vector<GUIEntityModeEntityCategoryDataStruct*>::iterator it;

    for (it = mGuiEntityItemTypeSelectionVec.begin(); it != mGuiEntityItemTypeSelectionVec.end(); ++it) {
      if ((*it)->currHovered) {
          result = (*it);
          break;
      }
    }

    return result;
}

//the entity category selection dialog needs all hover events
//to be able to properly select the category
void EntityMode::OnElementHovered(irr::s32 hoveredGuiId) {
    if (mCurrShownEntityCategory == nullptr)
        return;

    if (mParentSession->mUserInDialogState != DEF_EDITOR_USERINENTITYMODEDIALOG)
        return;

    //which entity category image was hovered?

    //returns nullptr if no category is found
    GUIEntityModeEntityCategoryDataStruct* pntr =  mCurrShownEntityCategory->SearchEntityItemTypeById(hoveredGuiId);

    if (pntr == nullptr)
        return;

    //we found the newly hovered entity type image in the
    //dialog
    pntr->currHovered = true;
}

//the entity category selection dialog needs all element left events
//to be able to properly select the category
void EntityMode::OnElementLeft(irr::s32 leftGuiId) {
    if (mCurrShownEntityCategory == nullptr)
        return;

    //which entity category image was left?

    //returns nullptr if no category is found
    GUIEntityModeEntityCategoryDataStruct* pntr =  mCurrShownEntityCategory->SearchEntityItemTypeById(leftGuiId);

    if (pntr == nullptr)
        return;

    //we found the new left entity type image in the
    //dialog
    pntr->currHovered = false;
}

EntityMode::EntityMode(EditorSession* parentSession) : EditorMode(parentSession) {
    mEntityCategoryVec.clear();
}

EntityMode::~EntityMode() {
    //cleanup all data
    std::vector<EntityModeEntityCategory*>::iterator it;
    EntityModeEntityCategory* pntr;

    for (it = this->mEntityCategoryVec.begin(); it != this->mEntityCategoryVec.end(); ) {
        pntr = (*it);

        it = mEntityCategoryVec.erase(it);

        delete pntr;
    }
}

void EntityMode::CreateWindow() {
    // set skin font
    /*IGUIFont* font = env->getFont("fontlucida.png");
    if (font)
        env->getSkin()->setFont(font);*/

    irr::core::dimension2d<irr::u32> dim ( 600, 500 );

    //finally create the window
    Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Entity Mode", 0, mGuiWindowId);

    mGuiEntityMode.LabelEntityCategory = mParentSession->mParentEditor->mGuienv->addStaticText ( L"Entity Category:",
                                      rect<s32>( dim.Width - 400, 24, dim.Width - 310, 40 ),false, false, Window, -1, false );

    mGuiEntityMode.EntityItemTypeCategoryList = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( dim.Width - 300, 24, dim.Width - 10, 40 ),
                                                                                          Window, GUI_ID_ENTITYCATEGORYCOMBOBOX);

    AddEntityTypeCategory(L"PowerUp", (irr::u8)(EDITOR_ENTITYCAT_POWERUPS), mGuiEntityMode.EntityItemTypeCategoryList);
    AddEntityTypeCategory(L"Models", (irr::u8)(EDITOR_ENTITYCAT_MODELS), mGuiEntityMode.EntityItemTypeCategoryList);
    AddEntityTypeCategory(L"Control", (irr::u8)(EDITOR_ENTITYCAT_CONTROL), mGuiEntityMode.EntityItemTypeCategoryList);
    AddEntityTypeCategory(L"Unused", (irr::u8)(EDITOR_ENTITYCAT_UNUSED), mGuiEntityMode.EntityItemTypeCategoryList);

    mGuiEntityMode.EntityItemTypeCategoryList->setToolTipText ( L"Select Entity Category" );

    //configure area in dialog where the possible entity categories are
    //shown
    dimEntityTypeSelectionArea.UpperLeftCorner.set(10, 110);
    dimEntityTypeSelectionArea.LowerRightCorner.set(dim.Width - 10, dim.Height - 40);

    irr::core::dimension2d dimEntityTypeSelectionSize = dimEntityTypeSelectionArea.getSize();

    //how much entity type pictures do we fit into the available entity types picture selection area, round down
    //the collectible entities sprites have the same dimension, just take the dimension from the first one, which is sprite number 29
    //dimension is 100x100 pixels; only exception is sprite number 41 which is not used in the game, this one is 94x90 pixels
    irr::core::dimension2d spriteDimension = this->mParentSession->mTexLoader->spriteTex.at(29)->getSize();

    mEntityTypeSelectionImageDimension = spriteDimension.Width;

    mNrEntityCatWidth = dimEntityTypeSelectionSize.Width / spriteDimension.Width;
    mNrEntityCatHeight = dimEntityTypeSelectionSize.Height / spriteDimension.Height;

    DefineAllEntityItemTypes();

    //preselect the powerup entities in the dialog
    SelectEntityModeEntityItemTypeCategory(FindEntityItemTypeCategory((irr::u8)(EDITOR_ENTITYCAT_POWERUPS)));

    mCurrentSelectedEntitySpriteLocation.set(10, 50);

    mParentSession->mParentEditor->mGuienv->addStaticText ( L"Current Selected:",
          rect<s32>( mCurrentSelectedEntitySpriteLocation.X,
                     mCurrentSelectedEntitySpriteLocation.Y - 15,
                     mCurrentSelectedEntitySpriteLocation.X + 70,
                     mCurrentSelectedEntitySpriteLocation.Y - 5),false, false, Window, -1, false );

    mGuiEntityMode.CurrentlySelectedEntityTypeStr =
            mParentSession->mParentEditor->mGuienv->addStaticText ( L"", rect<s32>( mCurrentSelectedEntitySpriteLocation.X,
                                                                                    mCurrentSelectedEntitySpriteLocation.Y + 5 + spriteDimension.Height,
                                                                                    mCurrentSelectedEntitySpriteLocation.X + 120,
                                                                                    mCurrentSelectedEntitySpriteLocation.Y + 20 + spriteDimension.Height),false, false, Window, -1, false );

    mGuiEntityMode.CurrentSelectedEntitySprite =
            mParentSession->mParentEditor->mGuienv->addImage(rect<s32>( mCurrentSelectedEntitySpriteLocation.X,
                                                                        mCurrentSelectedEntitySpriteLocation.Y,
                                                                        mCurrentSelectedEntitySpriteLocation.X + spriteDimension.Width,
                                                                        mCurrentSelectedEntitySpriteLocation.Y + spriteDimension.Height),
                                                                        Window, -1);

    mGuiEntityMode.CurrentSelectedEntitySprite->setScaleImage(true);

    //until user first selects something hide the current sprite
    mGuiEntityMode.CurrentSelectedEntitySprite->setEnabled(false);
    mGuiEntityMode.CurrentSelectedEntitySprite->setVisible(false);

    irr::s32 mx = mCurrentSelectedEntitySpriteLocation.X + spriteDimension.Width + 30;
    irr::s32 my = mCurrentSelectedEntitySpriteLocation.Y + 10;

    mGuiEntityMode.RemoveEntityButton
            = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx, my, mx + 100, my + 15), Window, GUI_ID_ENTITYWINDOW_BUTTONREMOVEENTITY, L"Remove Entity");

    //create the checkbox which allows to disable/enable running
    //Morphs
    /*mGuiViewMode.RunMorphsCheckbox = mParentSession->mParentEditor->mGuienv->addCheckBox(true, rect<s32> ( 10, 30, dim.Width - 10, 50),
                                                                                         Window, GUI_ID_VIEWMODEWINDOW_RUNMORPH_CHECKBOX, L"Run Morphs");

    mGuiViewMode.FogCheckbox = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 10, 45, dim.Width - 10, 75),
                                                                                         Window, GUI_ID_VIEWMODEWINDOW_FOG_CHECKBOX, L"Fog");

    mGuiViewMode.IlluminationCheckBox = mParentSession->mParentEditor->mGuienv->addCheckBox(true, rect<s32> ( 10, 100, dim.Width - 10, 130),
                                                                                           Window, GUI_ID_VIEWMODEWINDOW_ILLUMINATION_CHECKBOX, L"Illumination");*/
}

void EntityMode::NewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    //did the user select a cell with an existing entityItem on top of it?
    //if so we actually want to select the existing entityItem itself
    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        EditorEntity* existingEntity = nullptr;

        //Is there already an entity Item?
        if (mParentSession->mEntityManager->IsEntityItemAtCellCoord(newItemSelected.mCellCoordSelected.X,
                                                                    newItemSelected.mCellCoordSelected.Y,
                                                                    &existingEntity)) {
                //instead force selection of the existing entity item on top of this Cell
                mParentSession->mItemSelector->SelectEntityAtCellCoordinate(newItemSelected.mCellCoordSelected.X,
                                                                           newItemSelected.mCellCoordSelected.Y);

                //overwrite our local copy of the currently selected object, with the new one
                //otherwise the code below does not what it is supposed to do
                newItemSelected = mParentSession->mItemSelector->mCurrSelectedItem;
        }
    }

    if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
        //is our dialog already open?
        //if not open it
        ShowWindow();

        if (newItemSelected.mEntitySelected != nullptr) {

            wchar_t* selInfo = new wchar_t[200];
            Entity::EntityType type = newItemSelected.mEntitySelected->GetEntityType();
            std::string typeName = mParentSession->mEntityManager->GetNameForEntityType(type);

            swprintf(selInfo, 190, L"%s", typeName.c_str());

            /*wchar_t* coordInfo = new wchar_t[200];
            int res2 = swprintf(coordInfo, 190, L" at X = %d, Y = %d\0", newItemSelected.mCellCoordSelected.X, newItemSelected.mCellCoordSelected.Y);

            wcscat(selInfo, coordInfo);
            delete[] coordInfo;*/

            mParentSession->mParentEditor->UpdateStatusbarText(selInfo);
            mGuiEntityMode.CurrentlySelectedEntityTypeStr->setText(selInfo);
            delete[] selInfo;

            //update image of currently selected texture
            mGuiEntityMode.CurrentSelectedEntitySprite->setImage(mParentSession->mEntityManager->GetImageForEntityType(type));
            mGuiEntityMode.CurrentSelectedEntitySprite->setEnabled(true);
            mGuiEntityMode.CurrentSelectedEntitySprite->setVisible(true);

            //show remove entity button
            mGuiEntityMode.RemoveEntityButton->setEnabled(true);
            mGuiEntityMode.RemoveEntityButton->setVisible(true);

            //Hide all entity "Add buttons"
            ShowAllEntitiesAddButtons(false);
        }
    } else if (newItemSelected.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        //a Cell is selected, user show have option to add a new entity Item
        //is our dialog already open?
        //if not open it
        ShowWindow();

        wchar_t* selInfo = new wchar_t[200];
        swprintf(selInfo, 190, L"Cell at X = %d, Y = %d", newItemSelected.mCellCoordSelected.X, newItemSelected.mCellCoordSelected.Y);

        mParentSession->mParentEditor->UpdateStatusbarText(selInfo);
        mGuiEntityMode.CurrentlySelectedEntityTypeStr->setText(selInfo);
        delete[] selInfo;

        mGuiEntityMode.CurrentSelectedEntitySprite->setEnabled(false);
        mGuiEntityMode.CurrentSelectedEntitySprite->setVisible(false);

        //Hide remove entity button
        mGuiEntityMode.RemoveEntityButton->setEnabled(false);
        mGuiEntityMode.RemoveEntityButton->setVisible(false);

        //give user the ability to see the "Add Entity" buttons
        ShowAllEntitiesAddButtons(true);
    }
}

void EntityMode::SelectEntityModeEntityItemTypeCategory(EntityModeEntityCategory *newCat) {
    if (newCat == nullptr)
        return;

    //hide all categories
    std::vector<EntityModeEntityCategory*>::iterator it;

    for (it = this->mEntityCategoryVec.begin(); it != this->mEntityCategoryVec.end(); ++it) {
        (*it)->HideAllEntityTypes();
    }

    //only show the entity types of the newly selected category
    newCat->ShowAllEntityTypes();

    //store currently shown Entity category
    mCurrShownEntityCategory = newCat;
}

void EntityMode::ShowAllEntitiesAddButtons(bool visible) {
    if (!visible) {
        std::vector<EntityModeEntityCategory*>::iterator it;

        for (it = this->mEntityCategoryVec.begin(); it != this->mEntityCategoryVec.end(); ++it) {
            //hide category
            (*it)->HideAllEntityTypes();
        }
    } else {
        irr::s32 selectedCatId = mGuiEntityMode.EntityItemTypeCategoryList->getSelected();

        if (selectedCatId >= 0) {
                irr::u32 getSelectedData = mGuiEntityMode.EntityItemTypeCategoryList->getItemData((irr::u32)(selectedCatId));
                EntityModeEntityCategory* pntr = FindEntityItemTypeCategoryByGuiId(getSelectedData);

                if (pntr != nullptr) {
                      SelectEntityModeEntityItemTypeCategory(pntr);
                }
        }
    }

    if (!visible) {
        mGuiEntityMode.EntityItemTypeCategoryList->setVisible(false);
        mGuiEntityMode.EntityItemTypeCategoryList->setEnabled(false);
        mGuiEntityMode.LabelEntityCategory->setVisible(false);
        mGuiEntityMode.LabelEntityCategory->setEnabled(false);
    } else {
        mGuiEntityMode.EntityItemTypeCategoryList->setVisible(true);
        mGuiEntityMode.EntityItemTypeCategoryList->setEnabled(true);
        mGuiEntityMode.LabelEntityCategory->setVisible(true);
        mGuiEntityMode.LabelEntityCategory->setEnabled(true);
    }
}

void EntityMode::AddDefaultCollectable(EntityModeEntityCategory* catPntr, Entity::EntityType type) {
    irr::video::ITexture* texPntr;
    irr::u8 spriteNr;

    spriteNr = mParentSession->mEntityManager->GetCollectableSpriteNumber(type);
    texPntr = mParentSession->mTexLoader->spriteTex.at(spriteNr);
    catPntr->AddEntityType(texPntr, type);
}

void EntityMode::DefineAllEntityItemTypes() {
   EntityModeEntityCategory* catPntr = this->FindEntityItemTypeCategory((irr::u8)(EDITOR_ENTITYCAT_POWERUPS));

   //define the existing entity type categories available in the level editor
   if (catPntr != nullptr) {
       //EditorEntityPowerUp = {29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
       AddDefaultCollectable(catPntr, Entity::EntityType::ExtraFuel);
       AddDefaultCollectable(catPntr, Entity::EntityType::FuelFull);
       AddDefaultCollectable(catPntr, Entity::EntityType::DoubleFuel);

       AddDefaultCollectable(catPntr, Entity::EntityType::ExtraShield);
       AddDefaultCollectable(catPntr, Entity::EntityType::ShieldFull);
       AddDefaultCollectable(catPntr, Entity::EntityType::DoubleShield);

       AddDefaultCollectable(catPntr, Entity::EntityType::ExtraAmmo);
       AddDefaultCollectable(catPntr, Entity::EntityType::AmmoFull);
       AddDefaultCollectable(catPntr, Entity::EntityType::DoubleAmmo);

       AddDefaultCollectable(catPntr, Entity::EntityType::BoosterUpgrade);
       AddDefaultCollectable(catPntr, Entity::EntityType::MissileUpgrade);
       AddDefaultCollectable(catPntr, Entity::EntityType::MinigunUpgrade);
   }

   catPntr = this->FindEntityItemTypeCategory((irr::u8)(EDITOR_ENTITYCAT_UNUSED));
   if (catPntr != nullptr) {
        //EditorEntityUnused = {41, 42};
        AddDefaultCollectable(catPntr, Entity::EntityType::UnknownShieldItem);
   }

   catPntr = this->FindEntityItemTypeCategory((irr::u8)(EDITOR_ENTITYCAT_MODELS));
   if (catPntr != nullptr) {
       catPntr->AddEntityType(mParentSession->mEntityManager->mTexImageRecoveryVehicle, Entity::EntityType::RecoveryTruck);
       catPntr->AddEntityType(mParentSession->mEntityManager->mTexImageCone, Entity::EntityType::Cone);
   }

   catPntr = this->FindEntityItemTypeCategory((irr::u8)(EDITOR_ENTITYCAT_CONTROL));
   if (catPntr != nullptr) {
       //first texture item in editorTex vector is camera image
       catPntr->AddEntityType(mParentSession->mTexLoader->editorTex.at(0) , Entity::EntityType::Camera);
       //second texture item in editorTex vector is StopWatch image I want
       //to use for Time Trigger
       catPntr->AddEntityType(mParentSession->mTexLoader->editorTex.at(1), Entity::EntityType::TriggerTimed);
   }
}

//returns nullptr if category is not found
EntityModeEntityCategory* EntityMode::FindEntityItemTypeCategory(irr::u8 categoryNr) {
    std::vector<EntityModeEntityCategory*>::iterator it;

    EntityModeEntityCategory* result = nullptr;

    for (it = mEntityCategoryVec.begin(); it != mEntityCategoryVec.end(); ++it) {
        if ((*it)->categoryNr == categoryNr) {
            result = (*it);
            break;
        }
    }

    return result;
}

//returns nullptr if category is not found
EntityModeEntityCategory* EntityMode::FindEntityItemTypeCategoryByGuiId(irr::u32 guiId) {
    std::vector<EntityModeEntityCategory*>::iterator it;

    EntityModeEntityCategory* result = nullptr;

    for (it = mEntityCategoryVec.begin(); it != mEntityCategoryVec.end(); ++it) {
      if ((*it)->mDialogComboBoxElementId == guiId) {
          result = (*it);
          break;
      }
    }

    return result;
}

void EntityMode::AddEntityTypeCategory(const wchar_t *categoryName, irr::u8 categoryNr, irr::gui::IGUIComboBox *comboBoxPntr) {
     EntityModeEntityCategory* newCat = new EntityModeEntityCategory(this);
     newCat->categoryName = categoryName;
     newCat->categoryNr = categoryNr;

     //get next free Gui Element Id
     newCat->mDialogComboBoxElementId = mParentSession->GetNextFreeGuiId();

     //add new item to the dialog checkbox
     comboBoxPntr->addItem(categoryName, newCat->mDialogComboBoxElementId );

     mEntityCategoryVec.push_back(newCat);
}

void EntityMode::EntityCategoryChanged(irr::u32 newSelectedGuiId) {
    EntityModeEntityCategory* pntr = FindEntityItemTypeCategoryByGuiId(newSelectedGuiId);

    if (pntr != nullptr) {
          SelectEntityModeEntityItemTypeCategory(pntr);
    }
}

void EntityMode::OnExitMode() {
}

//is called when the editor mode
//is entered (becomes active)
void EntityMode::OnEnterMode() {
   //when this mode is activated the first
   //time create the window
   if (Window == nullptr) {
       CreateWindow();
   } else {
       //make sure existing window is not
       //hidden
       this->ShowWindow();
   }
}

void EntityMode::OnCheckBoxChanged(irr::s32 checkboxId) {
    /*if (checkboxId == GUI_ID_VIEWMODEWINDOW_RUNMORPH_CHECKBOX) {
        //is the checkbox now checked or not?
        if (mGuiViewMode.RunMorphsCheckbox->isChecked()) {
            //make sure morphing is enabled
            if (!mParentSession->IsMorphingRunning()) {
                mParentSession->ActivateMorphs();
            }
        } else {
            //morphs were disabled by the user
            //in the checkbox
            mParentSession->DeactivateMorphs();
        }
    } else if (checkboxId == GUI_ID_VIEWMODEWINDOW_FOG_CHECKBOX) {
        //Fog enabled or disabled?
        bool fogEnabled = mGuiViewMode.FogCheckbox->isChecked();

        mParentSession->SetFog(fogEnabled);
    } else if (checkboxId == GUI_ID_VIEWMODEWINDOW_ILLUMINATION_CHECKBOX) {
        //Illumination enabled or disabled?
        bool illuminationEnabled = mGuiViewMode.IlluminationCheckBox->isChecked();

        mParentSession->SetIllumination(illuminationEnabled);
    }*/
}

void EntityMode::OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem) {
    if (mCurrSelectedItem == nullptr)
        return;

    //mark the currently user selected item for entity mode
    if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
        mParentSession->mParentEditor->mDrawDebug->DrawAround3DBoundingBox(
                    &mCurrSelectedItem->mEntitySelected->mBoundingBox, mParentSession->mParentEditor->mDrawDebug->cyan);
    }
}

void EntityMode::OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem) {
    if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
        mParentSession->mParentEditor->mDrawDebug->DrawAround3DBoundingBox(
                    &mCurrHighlightedItem->mEntitySelected->mBoundingBox, mParentSession->mParentEditor->mDrawDebug->white);
    } else if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrHighlightedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->white);
    }
}

void EntityMode::OnUserChangedToNewEntity(GUIEntityModeEntityCategoryDataStruct* entitySelectedInDialog) {
   if (entitySelectedInDialog == nullptr)
       return;

   //only continue if currently a cell is selected (normally this is the case because if no cell is selected
   //the entity Add buttons are not visible, and therefore can not be clicked), but make 100% sure
   if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
       //add selected entity via EntityManager function call
       mParentSession->mEntityManager->AddEntityAtCell(mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X,
                                                       mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y,
                                                       entitySelectedInDialog->type);
   }
}

void EntityMode::OnLeftMouseButtonDown() {
    switch (mParentSession->mUserInDialogState) {
         case DEF_EDITOR_USERINNODIALOG: {
            //new entity selected
            NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);
            break;
         }

         case DEF_EDITOR_USERINENTITYMODEDIALOG: {
            //are we hovering right now over exactly one of my
            //Entity selection images?
            if (mCurrShownEntityCategory != nullptr) {
                //returns nullptr if no entity image in the dialog
                //is currently hovered with the mouse
                GUIEntityModeEntityCategoryDataStruct* pntr = mCurrShownEntityCategory->FoundCurrentlyHoveredEntityItemType();
                if (pntr != nullptr) {
                    //some new entity image was selected
                    //handle action inside this function call
                    OnUserChangedToNewEntity(pntr);
                }
            }

           break;
        }

        default: {
           break;
        }
    }
}

void EntityMode::OnButtonClicked(irr::s32 buttonGuiId) {
    switch (buttonGuiId) {
        case GUI_ID_ENTITYWINDOW_BUTTONREMOVEENTITY: {
             if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
                mParentSession->mEntityManager->RemoveEntity(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected);
             }
             break;
        }
    }
}
