/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ITEMSELECTOR_H
#define ITEMSELECTOR_H

#include "irrlicht.h"
//#include <vector>
//#include <list>
//#include "definitions.h"
#include "../utils/ray.h"

#define DEF_EDITOR_SELITEM_NONE 0
#define DEF_EDITOR_SELITEM_CELL 1
#define DEF_EDITOR_SELITEM_BLOCK 2
#define DEF_EDITOR_SELITEM_ENTITY 3

#define DEF_EDITOR_SELITEM_ENACELLS 1
#define DEF_EDITOR_SELITEM_ENABLOCKS 2
#define DEF_EDITOR_SELITEM_ENAENTITIES 3

/************************
 * Forward declarations *
 ************************/

class ColumnDefinition;
class Column;
class EditorSession;
class EditorEntity;

struct CurrentlySelectedEditorItemInfoStruct {
    irr::u8 SelectedItemType;

    //All the variables below only contain valid information
    //if SelectedItemType is != DEF_EDITOR_SELITEM_NONE
    RayHitTriangleInfoStruct RayHitInfo;

    //more detailed information in case a terrain
    //cell is currently selected
    irr::core::vector2di mCellCoordSelected;
    int mCellCoordVerticeNrSelected;

    //more detailed information in case a column
    //of blocks is selected
    Column* mColumnSelected = nullptr;
    ColumnDefinition* mColumnDefinitionSelected = nullptr;

    //the number of the block in the column
    //counting from the base, for this value it does
    //not matter if blocks below are not existing
    //every block position is counted
    int mSelBlockNrStartingFromBase;

    irr::u8 mSelBlockFaceDirection;

    //holds the entity item for the level editor
    //in case one is currently selected
    EditorEntity* mEntitySelected = nullptr;
};

class ItemSelector {
private:
    EditorSession* mParent;

    bool mFrozen = false;

    irr::core::line3df mRayLine;

    //derives more high level information about the highlighted terrain cell
    void DeriveHighlightedTerrainCellInformation(RayHitTriangleInfoStruct* hitTriangleInfo);

    //derives more high level information about the highlighted block
    void DeriveHighlightedBlockInformation(RayHitTriangleInfoStruct* hitTriangleInfo, RayHitTriangleInfoStruct* hitSecondClosestTriangleInfo);

    //ray class to find intersection with Terrain Cells
    Ray* mRayTerrain = nullptr;

    //ray class to find intersection with Columns
    Ray* mRayColumns = nullptr;

    //which type of items do we want to select
    //currently
    bool mEnaSelectCells = false;
    bool mEnaSelectBlocks = false;
    bool mEnaSelectEntities = false;

    bool mEnaMultipleVerticeSelection = false;
    bool mEnaMultipleBlockFaceSelection = false;

    //global flag do enable or disable
    //multiple item selection
    bool mEnaMultipleSelection = false;

    //creates final TriangleSelectors to be able to do ray
    //intersection from user mouse pointer to level environment
    //for object selection
    void CreateTriangleSelectors();

    //the necessary triangle selectors for ray intersecting onto columns of blocks
    irr::scene::ITriangleSelector* triangleSelectorColumnswCollision = nullptr;
    irr::scene::ITriangleSelector* triangleSelectorColumnswoCollision = nullptr;

    //necessary triangle selector for ray intersecting onto terrain
    irr::scene::ITriangleSelector* triangleSelectorStaticTerrain = nullptr;
    irr::scene::ITriangleSelector* triangleSelectorDynamicTerrain = nullptr;

    bool mDbgTerrainClosestTriangleHit = false;
    RayHitTriangleInfoStruct mDbgTerrainClosestTriangleHitData;

    bool mDbgBlockClosestTriangleHit = false;
    RayHitTriangleInfoStruct mDbgBlockClosestTriangleHitData;

    bool mDbgBlock2ndClosestTriangleHit = false;
    RayHitTriangleInfoStruct mDbgBlock2ndClosestTriangleHitData;

    irr::core::vector3df GetTriangleMidPoint(RayHitTriangleInfoStruct* triangleInfo);

    //Returns true if an entity was selected
    //by the user, false otherwise
    bool CheckForEntitySelection(irr::core::line3df rayLine, EditorEntity** selectedEntityItem);

    void SelectAdditionalItem(CurrentlySelectedEditorItemInfoStruct additionalItem);

    void DeselectAllAdditionalItems();
    void DeleteSpecificAdditionalItem(CurrentlySelectedEditorItemInfoStruct whichItem);

    //Returns true if two specified selected items are actually the same cell/block/entity item
    bool ItemTheSame(CurrentlySelectedEditorItemInfoStruct item1, CurrentlySelectedEditorItemInfoStruct item2);

    //Returns true if a specified item is already selected
    bool ItemAlreadySelected(CurrentlySelectedEditorItemInfoStruct whichItem);

    void DeselectSpecificItem(CurrentlySelectedEditorItemInfoStruct whichItem);

public:
    ItemSelector(EditorSession* parent);
    ~ItemSelector();

    irr::core::vector3df dbgAvgPos;

    bool OnlyShowOneTriag = false;

    //the following variable stores the currently "highlighted"
    //item in the level, when the user moves the mouse cursor above it
    CurrentlySelectedEditorItemInfoStruct mCurrHighlightedItem;

    //the following variable stores the currently selected
    //item in the level. This is the last item that was
    //highlighted when the user pressed down the left mouse
    //button
    CurrentlySelectedEditorItemInfoStruct mCurrSelectedItem;

    irr::u16 mAdditionalSelectedItemCnt = 0;
    std::vector<CurrentlySelectedEditorItemInfoStruct*> mAdditionalSelectedItemVec;

    //update the itemSelector, which means
    //we cast a ray from the current camera to the
    //mouse pointer position, and figure out which
    //items/entitys the players is currently selecting
    void Update();

    //needs to be called in case the Irrlicht
    //Meshes have changed in the level editor
    void UpdateTrianglesSelectors();

    void OnLeftMouseButtonDown();

    void SetStateFrozen(bool frozen);
    bool GetStateFrozen();

    void DeselectAll();

    size_t GetNumberSelectedItems();
    size_t GetNumberSelectedTextureSurfaces();
    size_t GetNumberSelectedColumns();

    void Draw();

    //allows to globally enable or disable selection of multiple items
    void SetAllowMultipleSelections(bool multipleSelectionsOn);

    //allows to enable or disable selection of multiple vertices
    //Does only work if SetAllowMultipleSelections was called with true
    //before, so that multiple selections are enabled globally in the first
    //place
    void SetEnaMultipleVerticeSelection(bool multipleVerticeSelectionOn);

    //allows to enable or disable selection of multiple block faces
    //Does only work if SetAllowMultipleSelections was called with true
    //before, so that multiple selections are enabled globally in the first
    //place
    void SetEnaMultipleBlockFacesSelection(bool multipleBlockFacesSelectionOn);

    //whichTypeItem = DEF_EDITOR_SELITEM_ENACELLS, DEF_EDITOR_SELITEM_ENABLOCKS, or
    //DEF_EDITOR_SELITEM_ENAENTITIES
    void SetEnableSelection(irr::u8 whichTypeItem, bool enabledState);

    //the following function is for a special case, where the
    //user wants to select the cell below a column (to change the
    //column floor texture Id in the texturing tool)
    //With a ray the user can not do this if the column is directly
    //standing on the cell. Therefore the texturing tool is able to call
    //the function below, to externally force a selection of the cell
    void SelectSpecifiedCellAtCoordinate(int x, int y);

    //the following function is for a special case, where the user
    //selected a cell below an already existing EntityItem in EntityMode,
    //and we want to actually force the selection of the EntityItem itself
    //Therefore the EntityMode tool can call this function to select
    //the Entity itself
    void SelectEntityAtCellCoordinate(int x, int y);
};

#endif // ITEMSELECTOR_H
