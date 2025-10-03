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
#include "../input/numbereditbox.h"
#include "uiconversion.h"

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
               mParent->mParentSession->mParentEditor->mGuienv->addImage(imageLocation, mParent->mGuiEntityMode.EditTab, newStruct->guiElementId,
                                                L"", true);

    newStruct->guiElement->setScaleImage(true);
    newStruct->guiElement->setImage(texture);

    //07.09.2025: It seems Visual Studio automatically changes swprintf to instead using
    //safer function swprintf_s which would be fine for me
    //The problem is this function checks for validity of format strings, and simply %s as under GCC
    //is not valid when specifiying a normal non wide C-string, and as a result text output does not work (only
    //garbage is shown); To fix this we need to use special format string "%hs" under windows;
    //But because I am not sure if GCC will accept this under Linux, I will keep the original code under GCC
    //here

    wchar_t* textDescription = new wchar_t[100];

#ifdef _MSC_VER 
    swprintf(textDescription, 100, L"%hs", newStruct->nameType.c_str());
#endif
#ifdef __GNUC__
    swprintf(textDescription, 100, L"%s", newStruct->nameType.c_str());
#endif

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

    mModeNameStr.append(L"Entitymode");
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

    irr::core::dimension2d<irr::u32> dim ( 520, 500 );

    //finally create the window
    Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Entity Mode", 0, mGuiWindowId);

    //lets create two tabs in the entity window, one for editing of entity
    //items
    //and the other one to list existing entities
    mGuiEntityMode.tabCntrl = mParentSession->mParentEditor->mGuienv->addTabControl(
            core::rect<s32>(1, 20, dim.Width - 2 , dim.Height - 2), Window, true, true);

    mGuiEntityMode.EditTab = mGuiEntityMode.tabCntrl->addTab(L"Edit");
    mGuiEntityMode.ListTab = mGuiEntityMode.tabCntrl->addTab(L"List");

    /***************
     * Edit Tab    *
     ***************/

    mGuiEntityMode.LabelEntityCategory = mParentSession->mParentEditor->mGuienv->addStaticText ( L"Entity Category:",
                                      rect<s32>( dim.Width - 400, 25, dim.Width - 310, 50 ),false, false, mGuiEntityMode.EditTab, -1, false );

    mGuiEntityMode.EntityItemTypeCategoryList = mParentSession->mParentEditor->mGuienv->addComboBox(rect<s32>( dim.Width - 300, 24, dim.Width - 10, 49 ),
                                                                                          mGuiEntityMode.EditTab, GUI_ID_ENTITYCATEGORYCOMBOBOX);

    AddEntityTypeCategory(L"PowerUp", (irr::u8)(EDITOR_ENTITYCAT_POWERUPS), mGuiEntityMode.EntityItemTypeCategoryList);
    AddEntityTypeCategory(L"Models", (irr::u8)(EDITOR_ENTITYCAT_MODELS), mGuiEntityMode.EntityItemTypeCategoryList);
    AddEntityTypeCategory(L"Control", (irr::u8)(EDITOR_ENTITYCAT_CONTROL), mGuiEntityMode.EntityItemTypeCategoryList);
    AddEntityTypeCategory(L"Waypoints", (irr::u8)(EDITOR_ENTITYCAT_WAYPOINTS), mGuiEntityMode.EntityItemTypeCategoryList);
    AddEntityTypeCategory(L"Unused", (irr::u8)(EDITOR_ENTITYCAT_UNUSED), mGuiEntityMode.EntityItemTypeCategoryList);

    mGuiEntityMode.EntityItemTypeCategoryList->setToolTipText ( L"Select Entity Category" );

    //configure area in dialog where the possible entity categories are
    //shown
    //dimEntityTypeSelectionArea.UpperLeftCorner.set(10, 110);
    dimEntityTypeSelectionArea.UpperLeftCorner.set(10, 70);
    dimEntityTypeSelectionArea.LowerRightCorner.set(dim.Width - 10, dim.Height - 40);

    irr::core::dimension2d dimEntityTypeSelectionSize = dimEntityTypeSelectionArea.getSize();

    //Show the entity item symbols with a size of 50 x 50 pixels
    irr::core::dimension2d spriteDimension(50, 50);

    mEntityTypeSelectionImageDimension = spriteDimension.Width;

    mNrEntityCatWidth = dimEntityTypeSelectionSize.Width / spriteDimension.Width;
    mNrEntityCatHeight = dimEntityTypeSelectionSize.Height / spriteDimension.Height;

    DefineAllEntityItemTypes();

    //preselect the powerup entities in the dialog
    SelectEntityModeEntityItemTypeCategory(FindEntityItemTypeCategory((irr::u8)(EDITOR_ENTITYCAT_POWERUPS)));

    mCurrentSelectedEntitySpriteLocation.set(10, 50);

    mGuiEntityMode.LabelCurrentlySelected = mParentSession->mParentEditor->mGuienv->addStaticText ( L"Current Selected:",
          rect<s32>( mCurrentSelectedEntitySpriteLocation.X,
                     mCurrentSelectedEntitySpriteLocation.Y - 23,
                     mCurrentSelectedEntitySpriteLocation.X + 120,
                     mCurrentSelectedEntitySpriteLocation.Y + 2),false, false, mGuiEntityMode.EditTab, -1, false );

    mGuiEntityMode.CurrentlySelectedEntityTypeStr =
            mParentSession->mParentEditor->mGuienv->addStaticText ( L"", rect<s32>( mCurrentSelectedEntitySpriteLocation.X,
                                                                                    mCurrentSelectedEntitySpriteLocation.Y + 5 + spriteDimension.Height,
                                                                                    mCurrentSelectedEntitySpriteLocation.X + 120,
                                                                                    mCurrentSelectedEntitySpriteLocation.Y + 30 + spriteDimension.Height),false, false, mGuiEntityMode.EditTab, -1, false );

    mGuiEntityMode.CurrentSelectedEntitySprite =
            mParentSession->mParentEditor->mGuienv->addImage(rect<s32>( mCurrentSelectedEntitySpriteLocation.X,
                                                                        mCurrentSelectedEntitySpriteLocation.Y,
                                                                        mCurrentSelectedEntitySpriteLocation.X + spriteDimension.Width,
                                                                        mCurrentSelectedEntitySpriteLocation.Y + spriteDimension.Height),
                                                                        mGuiEntityMode.EditTab, -1);

    mGuiEntityMode.CurrentSelectedEntitySprite->setScaleImage(true);

    //until user first selects something hide the current sprite
    mGuiEntityMode.CurrentSelectedEntitySprite->setEnabled(false);
    mGuiEntityMode.CurrentSelectedEntitySprite->setVisible(false);

    mGuiEntityMode.LabelCurrentlySelected->setEnabled(false);
    mGuiEntityMode.LabelCurrentlySelected->setVisible(false);

    mGuiEntityMode.CurrentSelectedEntityId =
            mParentSession->mParentEditor->mGuienv->addStaticText ( L"", rect<s32>( mCurrentSelectedEntitySpriteLocation.X,
                                                                                    mCurrentSelectedEntitySpriteLocation.Y + 25 + spriteDimension.Height,
                                                                                    mCurrentSelectedEntitySpriteLocation.X + 120,
                                                                                    mCurrentSelectedEntitySpriteLocation.Y + 50 + spriteDimension.Height),false, false, mGuiEntityMode.EditTab, -1, false );

    mGuiEntityMode.CurrentSelectedEntityId->setEnabled(false);
    mGuiEntityMode.CurrentSelectedEntityId->setVisible(false);

    irr::s32 mx = mCurrentSelectedEntitySpriteLocation.X + spriteDimension.Width + 30;
    irr::s32 my = mCurrentSelectedEntitySpriteLocation.Y + 10;

    mGuiEntityMode.RemoveEntityButton
            = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 80, my, mx + 180, my + 25), mGuiEntityMode.EditTab, GUI_ID_ENTITYWINDOW_BUTTONREMOVEENTITY, L"Remove Entity");

    mGuiEntityMode.RemoveEntityButton->setEnabled(false);
    mGuiEntityMode.RemoveEntityButton->setVisible(false);

    mGuiEntityMode.MoveEntityButton
            = mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 80, my + 40, mx + 180, my + 65), mGuiEntityMode.EditTab, GUI_ID_ENTITYWINDOW_BUTTONMOVEENTITY, L"Move Entity");

    mGuiEntityMode.MoveEntityButton->setEnabled(false);
    mGuiEntityMode.MoveEntityButton->setVisible(false);

    mGuiEntityMode.LinkEntityButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 205, my, mx + 305, my + 25), mGuiEntityMode.EditTab, GUI_ID_ENTITYWINDOW_BUTTONLINKENTITY, L"Link Entity");

    mGuiEntityMode.UnlinkEntityButton =
            mParentSession->mParentEditor->mGuienv->addButton(core::recti(mx + 205, my + 40, mx + 305, my + 65), mGuiEntityMode.EditTab, GUI_ID_ENTITYWINDOW_BUTTONUNLINKENTITY, L"Unlink Entity");

    mGuiEntityMode.LinkEntityButton->setEnabled(false);
    mGuiEntityMode.LinkEntityButton->setVisible(false);

    mGuiEntityMode.UnlinkEntityButton->setEnabled(false);
    mGuiEntityMode.UnlinkEntityButton->setVisible(false);

    irr::s32 dx = mCurrentSelectedEntitySpriteLocation.X;
    irr::s32 dy = mCurrentSelectedEntitySpriteLocation.Y + 30;

    /********************************************
     * Group Edit Box and Create At Start       *
     ********************************************/

    //Create the Ui Elements with the settings for default Entity Items
    mGuiEntityMode.CollectibleCreateAtStart =
            mParentSession->mParentEditor->mGuienv->addCheckBox(true, rect<s32> ( dx, dy + 25 + spriteDimension.Height,
                                                                                  dx + 120, dy + 40 + spriteDimension.Height),
                                                                         mGuiEntityMode.EditTab, GUI_ID_ENTITYMODEWINDOW_CREATEATSTART_CHECKBOX, L"Create at Start");

    mGuiEntityMode.GroupEditBox = new NumberEditBox(this, L"1", rect<s32> ( dx + 80, dy + 55 + spriteDimension.Height,
                                                                             dx + 140, dy + 70 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.GroupEditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.GroupEditBox->AddLabel(L"Group:",
                                          rect<s32>( dx, dy + 52 + spriteDimension.Height,
                                                     dx + 70,
                                                     dy + 73 + spriteDimension.Height));

    /********************************************
     * Target Group Edit Box                    *
     ********************************************/

    mGuiEntityMode.TargetGroupEditBox = new NumberEditBox(this, L"0", rect<s32> ( dx + 80, dy + 82 + spriteDimension.Height,
                                                                             dx + 140, dy + 97 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.TargetGroupEditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.TargetGroupEditBox->AddLabel(L"TargetGroup:",
                                          rect<s32>( dx, dy + 79 + spriteDimension.Height,
                                                     dx + 70,
                                                     dy + 100 + spriteDimension.Height));

    /********************************************
     * NextId Edit Box                          *
     ********************************************/

    mGuiEntityMode.NextIdEditBox = new NumberEditBox(this, L"0", rect<s32> ( dx + 230, dy + 55 + spriteDimension.Height,
                                                                             dx + 290, dy + 70 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.NextIdEditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.NextIdEditBox->AddLabel(L"NextId:",
                                          rect<s32>( dx + 170, dy + 52 + spriteDimension.Height,
                                                     dx + 210,
                                                     dy + 73 + spriteDimension.Height));

    /********************************************
     * Value Edit Box                           *
     ********************************************/

    mGuiEntityMode.ValueEditBox = new NumberEditBox(this, L"0", rect<s32> ( dx + 230, dy + 55 + 28 + spriteDimension.Height,
                                                                             dx + 290, dy + 70 + 28 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.ValueEditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.ValueEditBox->AddLabel(L"Value:",
                                          rect<s32>( dx + 170, dy + 52 + 28 + spriteDimension.Height,
                                                     dx + 230,
                                                     dy + 73 + 28 + spriteDimension.Height));

    /********************************************
     * OffsetX Edit Box                         *
     ********************************************/

    mGuiEntityMode.OffsetXEditBox = new NumberEditBox(this, L"0", rect<s32> ( dx + 230, dy + 55 + 56 + spriteDimension.Height,
                                                                             dx + 290, dy + 70 + 56 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.OffsetXEditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.OffsetXEditBox->AddLabel(L"Offset X:",
                                          rect<s32>( dx + 170, dy + 52 + 56 + spriteDimension.Height,
                                                     dx + 230,
                                                     dy + 73 + 56 + spriteDimension.Height));

    /********************************************
     * OffsetY Edit Box                         *
     ********************************************/

    mGuiEntityMode.OffsetYEditBox = new NumberEditBox(this, L"0", rect<s32> ( dx + 230, dy + 55 + 56 + 28 + spriteDimension.Height,
                                                                             dx + 290, dy + 70 + 56 + 28 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.OffsetYEditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.OffsetYEditBox->AddLabel(L"Offset Y:",
                                          rect<s32>( dx + 170, dy + 52 + 56 + 28 + spriteDimension.Height,
                                                     dx + 230,
                                                     dy + 73 + 56 + 28 + spriteDimension.Height));

    /********************************************
     * Unknown Edit Boxes                       *
     ********************************************/

    mGuiEntityMode.Unknown1EditBox = new NumberEditBox(this, L"0", rect<s32> ( dx + 400, dy + 55 + spriteDimension.Height,
                                                                               dx + 460, dy + 70 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.Unknown1EditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.Unknown1EditBox->AddLabel(L"Unknown1:",
                                          rect<s32>( dx + 320, dy + 52 + spriteDimension.Height,
                                                     dx + 380,
                                                     dy + 73 + spriteDimension.Height));

    mGuiEntityMode.Unknown2EditBox = new NumberEditBox(this, L"0", rect<s32> ( dx + 400, dy + 55 + 28 + spriteDimension.Height,
                                                                               dx + 460, dy + 70 + 28 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.Unknown2EditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.Unknown2EditBox->AddLabel(L"Unknown2:",
                                          rect<s32>( dx + 320, dy + 52 + 28 + spriteDimension.Height,
                                                     dx + 380,
                                                     dy + 73 + 28 + spriteDimension.Height));

    mGuiEntityMode.Unknown3EditBox = new NumberEditBox(this, L"0", rect<s32> ( dx + 400, dy + 55 + 57 + spriteDimension.Height,
                                                                               dx + 460, dy + 70 + 57 + spriteDimension.Height), true,
                                                                              mGuiEntityMode.EditTab);

    mGuiEntityMode.Unknown3EditBox->SetValueLimit(-1, 32767);
    mGuiEntityMode.Unknown3EditBox->AddLabel(L"Unknown3:",
                                          rect<s32>( dx + 320, dy + 52 + 57 + spriteDimension.Height,
                                                     dx + 380,
                                                     dy + 73 + 57 + spriteDimension.Height));

    /***************
     * List Tab    *
     ***************/

    mGuiEntityMode.EntityTable =
            mParentSession->mParentEditor->mGuienv->addTable( rect<s32>( 10, 30, 478, dim.Height - 120), mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_ENTITYTABLE);

    mGuiEntityMode.EntityTable->addColumn ( L"Id", 0 );
    mGuiEntityMode.EntityTable->addColumn ( L"Type", 1 );
    mGuiEntityMode.EntityTable->addColumn ( L"NextId", 2 );
    mGuiEntityMode.EntityTable->addColumn ( L"CellX", 3 );
    mGuiEntityMode.EntityTable->addColumn ( L"CellY", 4 );
    mGuiEntityMode.EntityTable->addColumn ( L"OffsetX", 5 );
    mGuiEntityMode.EntityTable->addColumn ( L"OffsetY", 6 );
    mGuiEntityMode.EntityTable->addColumn ( L"Group", 7 );
    mGuiEntityMode.EntityTable->addColumn ( L"Value", 8 );

    mGuiEntityMode.EntityTable->setColumnWidth ( 0, 40 );
    mGuiEntityMode.EntityTable->setColumnWidth ( 1, 110 );
    mGuiEntityMode.EntityTable->setColumnWidth ( 2, 40 );
    mGuiEntityMode.EntityTable->setColumnWidth ( 3, 40 );
    mGuiEntityMode.EntityTable->setColumnWidth ( 4, 40 );
    mGuiEntityMode.EntityTable->setColumnWidth ( 5, 30 );
    mGuiEntityMode.EntityTable->setColumnWidth ( 6, 30 );
    mGuiEntityMode.EntityTable->setColumnWidth ( 7, 40 );
    mGuiEntityMode.EntityTable->setColumnWidth ( 8, 40 );
    mGuiEntityMode.EntityTable->setToolTipText ( L"Shows all existing Entities");

    mGuiEntityMode.ListCollectibles = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 10, dim.Height - 75 - 30, 90, dim.Height - 55 - 30),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_COLLECTIBLES_CHECKBOX, L"Collectibles");

    mGuiEntityMode.ListWaypoints = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 105, dim.Height - 75 - 30, 210, dim.Height - 55 - 30),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_WAYPOINTS_CHECKBOX, L"Waypoints");

    mGuiEntityMode.ListWallsegments = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 200, dim.Height - 75 - 30, 300, dim.Height - 55 - 30),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_WALLSEGMENTS_CHECKBOX, L"Wallsegments");

    mGuiEntityMode.ListRecoveryvehicles = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 315, dim.Height - 75 - 30, 400, dim.Height - 55 - 30),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_RECOVERY_CHECKBOX, L"Recovery");

    mGuiEntityMode.ListCones = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 410, dim.Height - 75 - 30, 515, dim.Height - 55 - 30),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_CONES_CHECKBOX, L"Cones");

    mGuiEntityMode.ListMorphs = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 10, dim.Height - 45 - 35, 90, dim.Height - 25 - 35),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_MORPHS_CHECKBOX, L"Morphs");

    mGuiEntityMode.ListCameras = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 105, dim.Height - 45 - 35, 210, dim.Height - 25 - 35),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_CAMERAS_CHECKBOX, L"Cameras");

    mGuiEntityMode.ListTriggers = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 200, dim.Height - 45 - 35, 300, dim.Height - 25 - 35),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_TRIGGERS_CHECKBOX, L"Triggers");

    mGuiEntityMode.ListExplosions = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 315, dim.Height - 45 - 35, 400, dim.Height - 25 - 35),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_EXPLOSIONS_CHECKBOX, L"Explosions");

    mGuiEntityMode.ListCheckpoints = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 410, dim.Height - 45 - 35, 515, dim.Height - 25 - 35),
                                                                                                   mGuiEntityMode.ListTab, GUI_ID_ENTITYMODEWINDOW_LIST_CHECKPOINTS_CHECKBOX, L"Checkpoints");

    mGuiEntityMode.ListCollectibles->setChecked(mListCollectibles);
    mGuiEntityMode.ListWaypoints->setChecked(mListWaypoints);
    mGuiEntityMode.ListWallsegments->setChecked(mListWallsegments);
    mGuiEntityMode.ListRecoveryvehicles->setChecked(mListRecoveryvehicles);
    mGuiEntityMode.ListCones->setChecked(mListCones);
    mGuiEntityMode.ListMorphs->setChecked(mListMorphs);
    mGuiEntityMode.ListCameras ->setChecked(mListCameras);
    mGuiEntityMode.ListTriggers->setChecked(mListTriggers);
    mGuiEntityMode.ListExplosions->setChecked(mListExplosions);
    mGuiEntityMode.ListCheckpoints->setChecked(mListCheckpoints);

    //move window to a better start location
    Window->move(irr::core::vector2d<irr::s32>(700,200));

    ShowUiDefaultSettings(false);
    UpdateUiDialog();
}

void EntityMode::HighlightEntityTableRow(EditorEntity* whichEntity) {
    //-1 means no row (table entry) is hightlighted
    irr::s32 rowIdx = -1;

    irr::s32 rowCount = mGuiEntityMode.EntityTable->getRowCount();
    irr::s32 columnCount = mGuiEntityMode.EntityTable->getColumnCount();

    if (whichEntity != nullptr) {
       //is the current item even shown or filtered?
       if (DoListEntityItem(whichEntity)) {
           //yes it is shown in the table
           int16_t idEntry;
           irr::u32 currIdValue;
           int itemId = whichEntity->mEntityItem->get_ID();

           //check every currently shown row in which row the correct selected
           //Entity Item Id can be found?
           for (irr::s32 idx = 0; idx < rowCount; idx++) {
              if (mParentSession->mParentEditor->mUiConversion->GetUIntFromTableEntry(mGuiEntityMode.EntityTable, idx, 0, currIdValue)) {
                  idEntry = (int16_t)(currIdValue);
                  if (idEntry == itemId) {
                      //we found the correct table entry row
                      //we can exit this search
                      rowIdx = idx;

                      break;
                  }
              }
           }
       }
    }

    for (irr::s32 idx = 0; idx < rowCount; idx++) {
        for (irr::s32 idx2 = 0; idx2 < columnCount; idx2++) {
        if (idx == rowIdx) {
            mGuiEntityMode.EntityTable->setCellColor(idx, idx2, *mParentSession->mParentEditor->mDrawDebug->white->color);
        } else {
            mGuiEntityMode.EntityTable->setCellColor(idx, idx2, *mParentSession->mParentEditor->mDrawDebug->black->color);
        }
      }
    }
}

void EntityMode::AddEntityTableEntry(EditorEntity* whichEntity) {
    if (whichEntity == nullptr) {
        return;
    }

    irr::s32 nrRowIdx = mGuiEntityMode.EntityTable->getRowCount();

    //create a new row
    mGuiEntityMode.EntityTable->addRow(nrRowIdx);

    irr::core::stringw typeStr(mParentSession->mEntityManager->GetNameForEntityType(whichEntity->GetEntityType()).c_str());
    irr::core::stringw entryNrStr(whichEntity->mEntityItem->get_ID());

    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 0, entryNrStr);
    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 1, typeStr);

    irr::core::stringw nextId(whichEntity->mEntityItem->getNextID());
    irr::core::stringw cellX(whichEntity->mEntityItem->getCell().X);
    irr::core::stringw cellY(whichEntity->mEntityItem->getCell().Y);
    irr::core::stringw offsetX((int)(whichEntity->mEntityItem->getOffsetX()));
    irr::core::stringw offsetY((int)(whichEntity->mEntityItem->getOffsetY()));
    irr::core::stringw group(whichEntity->mEntityItem->getGroup());
    irr::core::stringw value(whichEntity->mEntityItem->getValue());

    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 2, nextId);
    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 3, cellX);
    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 4, cellY);
    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 5, offsetX);
    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 6, offsetY);
    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 7, group);
    mGuiEntityMode.EntityTable->setCellText(nrRowIdx, 8, value);
}

bool EntityMode::DoListEntityItem(EditorEntity* item) {
    if (item == nullptr)
        return false;

    if (item->mEntityItem == nullptr)
        return false;

    if (mParentSession->mEntityManager->EntityIsPowerUpCollectible(item)) {
        return mListCollectibles;
    }

    if (mParentSession->mEntityManager->EntityIsWayPoint(item)) {
        return mListWaypoints;
    }

    switch (item->mEntityItem->getEntityType()) {
        case Entity::EntityType::Cone: {
           return mListCones;
        }

        case Entity::EntityType::Camera: {
           return mListCameras;
        }

        case Entity::EntityType::Unknown:
        case Entity::EntityType::UnknownItem:
        case Entity::EntityType::UnknownShieldItem:
        {
            return true;
        }

        case Entity::EntityType::Explosion: {
               return mListExplosions;
        }

        case Entity::EntityType::MorphOnce:
        case Entity::EntityType::MorphSource1:
        case Entity::EntityType::MorphSource2:
        case Entity::EntityType::MorphPermanent: {
            return mListMorphs;
        }

        case Entity::EntityType::WallSegment: {
            return mListWallsegments;
        }

        case Entity::EntityType::RecoveryTruck: {
            return mListRecoveryvehicles;
        }

        case Entity::EntityType::TriggerCraft:
        case Entity::EntityType::TriggerTimed:
        case Entity::EntityType::TriggerRocket: {
            return mListTriggers;
        }

        case Entity::EntityType::Checkpoint: {
            return mListCheckpoints;
        }

        case Entity::EntityType::SteamLight:
        case Entity::EntityType::SteamStrong:
        case Entity::EntityType::DamageCraft: {
            return true;
        }

        default: {
            return true;
        }
    }
}

void EntityMode::UpdateEntitiesTable() {
    std::vector<EditorEntity*>::iterator it;

    mGuiEntityMode.EntityTable->clearRows();

    //add all existing EntityItems into the entity table
    for (it = mParentSession->mEntityManager->mEntityVec.begin(); it != mParentSession->mEntityManager->mEntityVec.end(); ++it) {
        //do we want to include this entity type in the table?
        //depends on the user selected checkboxes
        if (DoListEntityItem((*it))) {
            AddEntityTableEntry((*it));
        }
    }
}

void EntityMode::ShowUiDefaultSettings(bool visible) {
  mGuiEntityMode.CurrentSelectedEntityId->setEnabled(visible);
  mGuiEntityMode.CurrentSelectedEntityId->setVisible(visible);

  mGuiEntityMode.LinkEntityButton->setEnabled(visible);
  mGuiEntityMode.LinkEntityButton->setVisible(visible);

  mGuiEntityMode.UnlinkEntityButton->setEnabled(visible);
  mGuiEntityMode.UnlinkEntityButton->setVisible(visible);

  mGuiEntityMode.CollectibleCreateAtStart->setVisible(visible);
  mGuiEntityMode.CollectibleCreateAtStart->setEnabled(visible);

  mGuiEntityMode.GroupEditBox->SetEnabled(visible);
  mGuiEntityMode.GroupEditBox->SetVisible(visible);

  mGuiEntityMode.TargetGroupEditBox->SetEnabled(visible);
  mGuiEntityMode.TargetGroupEditBox->SetVisible(visible);

  mGuiEntityMode.NextIdEditBox->SetEnabled(visible);
  mGuiEntityMode.NextIdEditBox->SetVisible(visible);

  mGuiEntityMode.Unknown1EditBox->SetEnabled(visible);
  mGuiEntityMode.Unknown1EditBox->SetVisible(visible);

  mGuiEntityMode.Unknown2EditBox->SetEnabled(visible);
  mGuiEntityMode.Unknown2EditBox->SetVisible(visible);

  mGuiEntityMode.Unknown3EditBox->SetEnabled(visible);
  mGuiEntityMode.Unknown3EditBox->SetVisible(visible);

  mGuiEntityMode.ValueEditBox->SetEnabled(visible);
  mGuiEntityMode.ValueEditBox->SetVisible(visible);

  mGuiEntityMode.OffsetXEditBox->SetEnabled(visible);
  mGuiEntityMode.OffsetXEditBox->SetVisible(visible);

  mGuiEntityMode.OffsetYEditBox->SetEnabled(visible);
  mGuiEntityMode.OffsetYEditBox->SetVisible(visible);

  //Update the Ui elements with the current values
  if (visible) {
      UpdateUiDefaultSettings();
  }
}

void EntityMode::UpdateUiDialog() {
    UpdateEntitiesTable();

    //which line needs to be highlighted?
    //for rowIdx = -1 all rows are "unhighlighted"
    HighlightEntityTableRow(mLastSelectedEditorEntity);
}

void EntityMode::UpdateUiDefaultSettings() {
    if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType != DEF_EDITOR_SELITEM_ENTITY)
        return;

    EditorEntity* pntr = mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected;
    int16_t group = pntr->mEntityItem->getGroup();

    if (group == 1) {
         mGuiEntityMode.CollectibleCreateAtStart->setChecked(true);
         mGuiEntityMode.GroupEditBox->SetEnabled(false);
    } else {
         mGuiEntityMode.CollectibleCreateAtStart->setChecked(false);
         mGuiEntityMode.GroupEditBox->SetEnabled(true);
    }

    mGuiEntityMode.GroupEditBox->SetValue(group);

    int16_t targetGroup = pntr->mEntityItem->getTargetGroup();

    mGuiEntityMode.TargetGroupEditBox->SetValue(targetGroup);

    int16_t nextId = pntr->mEntityItem->getNextID();

    mGuiEntityMode.NextIdEditBox->SetValue(nextId);

    int16_t unknown1 = pntr->mEntityItem->getUnknown1();
    mGuiEntityMode.Unknown1EditBox->SetValue(unknown1);

    int16_t unknown2 = pntr->mEntityItem->getUnknown2();
    mGuiEntityMode.Unknown2EditBox->SetValue(unknown2);

    int16_t unknown3 = pntr->mEntityItem->getUnknown3();
    mGuiEntityMode.Unknown3EditBox->SetValue(unknown3);

    int16_t value = pntr->mEntityItem->getValue();
    mGuiEntityMode.ValueEditBox->SetValue(value);

    //OffsetX and OffsetY according to the underlying
    //EntityItem and Levelfile seem to be a irr::f32 (float)
    //But actually when loading original levels the only contain whole integers
    //So we can get away with using NumberEditBox (does only support int) for them as well
    int16_t offsetX = (int16_t)(pntr->mEntityItem->getOffsetX());
    mGuiEntityMode.OffsetXEditBox->SetValue(offsetX);

    int16_t offsetY = (int16_t)(pntr->mEntityItem->getOffsetY());
    mGuiEntityMode.OffsetYEditBox->SetValue(offsetY);
}

//returns true if move is succesfull, false otherwise
bool EntityMode::EntityMoveTargetCellSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
    //check again, if there is already an entity item at the target location
    //return here, and interrupt operation
    if (newItemSelected.SelectedItemType != DEF_EDITOR_SELITEM_CELL)
        return false;

    EditorEntity* existingEntity = nullptr;

    //Is there already an entity Item at the target location?
    //if so simply exit
    if (mParentSession->mEntityManager->IsEntityItemAtCellCoord(newItemSelected.mCellCoordSelected.X,
                                                                newItemSelected.mCellCoordSelected.Y,
                                                                &existingEntity)) {
        return false;
    }

    //mLastSelectedEditorEntity contains the EditorEntity item
    //that needs to be moved
    if (mLastSelectedEditorEntity == nullptr)
        return false;

    return (mParentSession->mEntityManager->MoveEntityToCell(mLastSelectedEditorEntity,
                                                     newItemSelected.mCellCoordSelected.X,
                                                     newItemSelected.mCellCoordSelected.Y));
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

                mLastSelectedEditorEntity = newItemSelected.mEntitySelected;
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

            //07.09.2025: It seems Visual Studio automatically changes swprintf to instead using
            //safer function swprintf_s which would be fine for me
            //The problem is this function checks for validity of format strings, and simply %s as under GCC
            //is not valid when specifiying a normal non wide C-string, and as a result text output does not work (only
            //garbage is shown); To fix this we need to use special format string "%hs" under windows;
            //But because I am not sure if GCC will accept this under Linux, I will keep the original code under GCC
            //here

            #ifdef _MSC_VER 
                swprintf(selInfo, 190, L"%hs", typeName.c_str());
            #endif
            #ifdef __GNUC__
                swprintf(selInfo, 190, L"%s", typeName.c_str());
            #endif

            /*wchar_t* coordInfo = new wchar_t[200];
            int res2 = swprintf(coordInfo, 190, L" at X = %d, Y = %d\0", newItemSelected.mCellCoordSelected.X, newItemSelected.mCellCoordSelected.Y);

            wcscat(selInfo, coordInfo);
            delete[] coordInfo;*/

            mParentSession->mParentEditor->UpdateStatusbarText(selInfo);
            mGuiEntityMode.CurrentlySelectedEntityTypeStr->setText(selInfo);
            delete[] selInfo;

            //update Id number with Id of currently selected element
            wchar_t* idStrInfo = new wchar_t[20];

            swprintf(idStrInfo, 17, L"Id: %d", newItemSelected.mEntitySelected->mEntityItem->get_ID());
            mGuiEntityMode.CurrentSelectedEntityId->setText(idStrInfo);
            delete[] idStrInfo;

            //update image of currently selected texture
            mGuiEntityMode.CurrentSelectedEntitySprite->setImage(mParentSession->mEntityManager->GetImageForEntityType(type));
            mGuiEntityMode.CurrentSelectedEntitySprite->setEnabled(true);
            mGuiEntityMode.CurrentSelectedEntitySprite->setVisible(true);

            mGuiEntityMode.LabelCurrentlySelected->setEnabled(true);
            mGuiEntityMode.LabelCurrentlySelected->setVisible(true);

            //show remove entity button
            mGuiEntityMode.RemoveEntityButton->setEnabled(true);
            mGuiEntityMode.RemoveEntityButton->setVisible(true);

            //show mode entity button
            mGuiEntityMode.MoveEntityButton->setEnabled(true);
            mGuiEntityMode.MoveEntityButton->setVisible(true);

            mLastSelectedEditorEntity = newItemSelected.mEntitySelected;

            ShowUiDefaultSettings(true);

            UpdateUiDialog();

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

        //Hide move entity button
        mGuiEntityMode.MoveEntityButton->setEnabled(false);
        mGuiEntityMode.MoveEntityButton->setVisible(false);

        mGuiEntityMode.LabelCurrentlySelected->setEnabled(false);
        mGuiEntityMode.LabelCurrentlySelected->setVisible(false);

        mLastSelectedEditorEntity = nullptr;

        ShowUiDefaultSettings(false);

        UpdateUiDialog();

        //give user the ability to see the "Add Entity" buttons
        ShowAllEntitiesAddButtons(true);
    }
}

void EntityMode::OnTableSelected(irr::s32 elementId) {
    if (elementId == GUI_ID_ENTITYMODEWINDOW_ENTITYTABLE) {
        //another entry in the entity table was selected by the user
        //which entity was selected?
        irr::s32 rowSelected = mGuiEntityMode.EntityTable->getSelected();

        if (rowSelected != -1) {
            irr::u32 entityId;

            if (mParentSession->mParentEditor->mUiConversion->GetUIntFromTableEntry(mGuiEntityMode.EntityTable, rowSelected, 0, entityId)) {
                mLastSelectedTableEntityNr = (int16_t)(entityId);

                EditorEntity* entityPntr = mParentSession->mEntityManager->GetEditorEntityWithId(mLastSelectedTableEntityNr);
                if (entityPntr != nullptr) {
                    //set user camera right to the selected EditorEntity item
                    mParentSession->MoveUserViewToLocation(entityPntr->mPosition, 25.0f);
                }

                mLastSelectedEditorEntity = entityPntr;

                irr::core::vector2di cellCoord = entityPntr->GetCellCoord();

                mParentSession->mItemSelector->SelectEntityAtCellCoordinate(cellCoord.X, cellCoord.Y);
            }

            NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);

            UpdateUiDefaultSettings();

            //which line needs to be highlighted?
            HighlightEntityTableRow(mLastSelectedEditorEntity);
        }
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
    irr::u16 spriteNr;

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
       catPntr->AddEntityType(mParentSession->mTexLoader->spriteTex.at(12), Entity::EntityType::SteamLight);
       catPntr->AddEntityType(mParentSession->mTexLoader->spriteTex.at(17), Entity::EntityType::SteamStrong);
   }

   catPntr = this->FindEntityItemTypeCategory((irr::u8)(EDITOR_ENTITYCAT_WAYPOINTS));
   if (catPntr != nullptr) {
       catPntr->AddEntityType(mParentSession->mTexLoader->levelTex.at(40), Entity::EntityType::WaypointFuel);
       catPntr->AddEntityType(mParentSession->mTexLoader->levelTex.at(44), Entity::EntityType::WaypointAmmo);
       catPntr->AddEntityType(mParentSession->mTexLoader->levelTex.at(48), Entity::EntityType::WaypointShield);
       catPntr->AddEntityType(mParentSession->mTexLoader->editorTex.at(2), Entity::EntityType::WaypointSlow);
       catPntr->AddEntityType(mParentSession->mTexLoader->editorTex.at(3), Entity::EntityType::WaypointFast);
   }

   catPntr = this->FindEntityItemTypeCategory((irr::u8)(EDITOR_ENTITYCAT_CONTROL));
   if (catPntr != nullptr) {
       //first texture item in editorTex vector is camera image
       catPntr->AddEntityType(mParentSession->mTexLoader->editorTex.at(0) , Entity::EntityType::Camera);
       catPntr->AddEntityType(mParentSession->mTexLoader->levelTex.at(121), Entity::EntityType::Checkpoint);
       //second texture item in editorTex vector is StopWatch image I want
       //to use for Time Trigger
       catPntr->AddEntityType(mParentSession->mTexLoader->editorTex.at(1), Entity::EntityType::TriggerTimed);
       catPntr->AddEntityType(mParentSession->mEntityManager->mTexImageRaceVehicle, Entity::EntityType::TriggerCraft);
       catPntr->AddEntityType(mParentSession->mTexLoader->spriteTex.at(39), Entity::EntityType::TriggerRocket);
       catPntr->AddEntityType(mParentSession->mTexLoader->spriteTex.at(4), Entity::EntityType::Explosion);
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
    //make sure we return in Default OP Mode the next
    //time
    mOpMode = EDITOR_ENTITY_OPMODE_DEFAULT;

    mParentSession->HideArrowPointingRight();
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

void EntityMode::ChangeCreateAtStart(bool newValue) {
   //Do not create entity at start anymore
   if (!newValue) {
       //Set other groupValue then 1 (which would create item
       //again at start) to indicate other behavior in Ui dialog
       mParentSession->mEntityManager->ChangeEntitiyGroup(
                   mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, 0);
   } else {
       //Set groupValue of 1 (which does create item
       //again at start)
       mParentSession->mEntityManager->ChangeEntitiyGroup(
                   mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, 1);
   }

   UpdateUiDefaultSettings();
}

void EntityMode::OnCheckBoxChanged(irr::s32 checkboxId) {
    if (checkboxId == GUI_ID_ENTITYMODEWINDOW_CREATEATSTART_CHECKBOX) {
        //is the checkbox now checked or not?
        ChangeCreateAtStart(mGuiEntityMode.CollectibleCreateAtStart->isChecked());
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_CONES_CHECKBOX) {
        mListCones = !mListCones;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_MORPHS_CHECKBOX) {
        mListMorphs = !mListMorphs;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_CAMERAS_CHECKBOX) {
        mListCameras = !mListCameras;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_RECOVERY_CHECKBOX) {
        mListRecoveryvehicles = !mListRecoveryvehicles;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_TRIGGERS_CHECKBOX) {
        mListTriggers = !mListTriggers;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_WAYPOINTS_CHECKBOX) {
        mListWaypoints = !mListWaypoints;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_EXPLOSIONS_CHECKBOX) {
        mListExplosions = !mListExplosions;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_COLLECTIBLES_CHECKBOX) {
        mListCollectibles = !mListCollectibles;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_WALLSEGMENTS_CHECKBOX) {
        mListWallsegments = !mListWallsegments;
        UpdateEntitiesTable();
    } else if (checkboxId == GUI_ID_ENTITYMODEWINDOW_LIST_CHECKPOINTS_CHECKBOX) {
        mListCheckpoints = !mListCheckpoints;
        UpdateEntitiesTable();
    }
}

void EntityMode::OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem) {
    if (mCurrSelectedItem == nullptr)
        return;

    //mark the currently user selected item for entity mode
    if (mCurrSelectedItem->SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
        mParentSession->mParentEditor->mDrawDebug->DrawAround3DBoundingBox(
                    &mCurrSelectedItem->mEntitySelected->mBoundingBox, mParentSession->mParentEditor->mDrawDebug->cyan);

        //highlight connection to linked entity item with a white arrow
        int16_t linkedItemId = mCurrSelectedItem->mEntitySelected->mEntityItem->getNextID();

        //if nextId is zero no item is linked
        //otherwise there is a link
        if (linkedItemId != 0) {
            EditorEntity* linkedItem = mParentSession->mEntityManager->GetEditorEntityWithId(linkedItemId);

            if (linkedItem != nullptr) {
                mParentSession->mParentEditor->mDrawDebug->Draw3DArrow(
                            mCurrSelectedItem->mEntitySelected->mPosition, linkedItem->mPosition,
                            0.5f, mParentSession->mParentEditor->mDrawDebug->cyan, 0.1f);
            }
        }
    }
}

void EntityMode::OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem) {
    if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
        mParentSession->mParentEditor->mDrawDebug->DrawAround3DBoundingBox(
                    &mCurrHighlightedItem->mEntitySelected->mBoundingBox, mParentSession->mParentEditor->mDrawDebug->white);
    } else if (mCurrHighlightedItem->SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        mParentSession->mLevelTerrain->DrawOutlineSelectedCell(mCurrHighlightedItem->mCellCoordSelected, mParentSession->mParentEditor->mDrawDebug->white);

        //if user tries to currently move an entity item, show an additional arrow symbol above the currently
        //highlighted cell
        if (mOpMode == EDITOR_ENTITY_OPMODE_SETMOVETARGET) {
            //If there is currently already an entity item at the selected target
            //cell then do not show the target movement arrow symbol
            //otherwise show the symbol
            EditorEntity* pntrItem = nullptr;

            //Returns true if there is currently an entity item at the specified cell
            //coordinates. Pointer to existing item is also returned via reference parameter returnPntr
            //Returns false if there is no Entity item right now
            if (!mParentSession->mEntityManager->IsEntityItemAtCellCoord(mCurrHighlightedItem->mCellCoordSelected.X,
                                                                         mCurrHighlightedItem->mCellCoordSelected.Y, &pntrItem)) {
                //There is currently no entity item at the selected cell
                mParentSession->ShowArrowPointingRightAtCell(mCurrHighlightedItem->mCellCoordSelected);
            } else {
                //there is already an entity item at the currently selected cell
                //Hide the arrow symbol
                mParentSession->HideArrowPointingRight();
            }
        }
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
            if (mOpMode == EDITOR_ENTITY_OPMODE_DEFAULT) {
                //new entity selected
                NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);
            } else if (mOpMode == EDITOR_ENTITY_OPMODE_SETMOVETARGET) {
                EntityMoveTargetCellSelected(mParentSession->mItemSelector->mCurrSelectedItem);

                mOpMode = EDITOR_ENTITY_OPMODE_DEFAULT;

                mParentSession->HideArrowPointingRight();
             } else if (mOpMode == EDITOR_ENTITY_OPMODE_CREATELINKSELNEXTITEM) {
                if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
                    //user selected the link target entity item
                    //if user just selected the same item again as the link target cancel operation here
                    if (mLastSelectedEditorEntity == mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected) {
                        //user wants to link item on itself, cancel
                        mOpMode = EDITOR_ENTITY_OPMODE_DEFAULT;
                    } else {
                       //proper link needs to be established
                       mParentSession->mEntityManager->LinkEntity(mLastSelectedEditorEntity,
                                          mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected);

                       //for the user it feels more intuitive when after linking the items
                       //the initial item is selected again
                       irr::core::vector2di coord = mLastSelectedEditorEntity->mEntityItem->getCell();

                       mParentSession->mItemSelector->SelectEntityAtCellCoordinate(coord.X, coord.Y);

                       UpdateUiDefaultSettings();

                       mOpMode = EDITOR_ENTITY_OPMODE_DEFAULT;
                    }
                } else {
                    //user did not select the expected link target entity item,
                    //but instead a cell
                    //lets cancel this operation
                    mOpMode = EDITOR_ENTITY_OPMODE_DEFAULT;
                }
            }

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

                //make sure to deselect currently selected Entity and too instead select cell below
                mParentSession->mItemSelector->SelectSpecifiedCellAtCoordinate(
                    mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X,
                    mParentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y);

                NewLevelItemSelected(mParentSession->mItemSelector->mCurrSelectedItem);

                mLastSelectedEditorEntity = nullptr;
             }

             break;
        }

        case GUI_ID_ENTITYWINDOW_BUTTONMOVEENTITY: {
            if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
                //change to internal operation mode where we have to set a new
                //target cell for this entity (a move target)
                mOpMode = EDITOR_ENTITY_OPMODE_SETMOVETARGET;
            }

            break;
        }

        case GUI_ID_ENTITYWINDOW_BUTTONUNLINKENTITY: {
            if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
                mParentSession->mEntityManager->UnlinkEntity(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected);

                UpdateUiDefaultSettings();
            }

            break;
        }

        case GUI_ID_ENTITYWINDOW_BUTTONLINKENTITY: {
            if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
                //now user needs to select the next item to link to
                mOpMode = EDITOR_ENTITY_OPMODE_CREATELINKSELNEXTITEM;
            }

            break;
        }
    }
}

void EntityMode::OnNumberEditBoxNewValue(NumberEditBox* whichBox, irr::s32& newValue) {
    if (whichBox == mGuiEntityMode.GroupEditBox) {
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyGroup(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (int16_t)(newValue));
        }
    }

    if (whichBox == mGuiEntityMode.TargetGroupEditBox) {
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyTargetGroup(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (int16_t)(newValue));
        }
    }

    if (whichBox == mGuiEntityMode.NextIdEditBox) {
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyNextId(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (int16_t)(newValue));
        }
    }

    if (whichBox == mGuiEntityMode.Unknown1EditBox) {
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyUnknown1(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (int16_t)(newValue));
        }
    }

    if (whichBox == mGuiEntityMode.Unknown2EditBox) {
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyUnknown2(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (int16_t)(newValue));
        }
    }

    if (whichBox == mGuiEntityMode.Unknown3EditBox) {
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyUnknown3(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (int16_t)(newValue));
        }
    }

    if (whichBox == mGuiEntityMode.ValueEditBox) {
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyValue(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (int16_t)(newValue));
        }
    }

    if (whichBox == mGuiEntityMode.OffsetXEditBox) {
        //OffsetX and OffsetY according to the underlying
        //EntityItem and Levelfile seem to be a irr::f32 (float)
        //But actually when loading original levels the only contain whole integers
        //So we can get away with using NumberEditBox (does only support int) for them as well
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyOffsetX(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (float)(newValue));
        }
    }

    if (whichBox == mGuiEntityMode.OffsetYEditBox) {
        if (mParentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_ENTITY) {
             mParentSession->mEntityManager->ChangeEntitiyOffsetY(mParentSession->mItemSelector->mCurrSelectedItem.mEntitySelected, (float)(newValue));
        }
    }
}
