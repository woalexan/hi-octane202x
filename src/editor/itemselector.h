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

/************************
 * Forward declarations *
 ************************/

class ColumnDefinition;
class Column;
class EditorSession;

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

    //the number of the block in the column
    //counting from the base, but non existing
    //blocks below are not counted
    int mSelBlockNrSkippingMissingBlocks;

    irr::u8 mSelBlockFaceDirection;
};

class ItemSelector {
private:
    EditorSession* mParent;

    bool mFrozen = false;

    irr::core::line3df mRayLine;

    //derives more high level information about the selected terrain cell
    void DeriveSelectedTerrainCellInformation(RayHitTriangleInfoStruct* hitTriangleInfo);

    //derives more high level information about the selected block
    void DeriveSelectedBlockInformation(RayHitTriangleInfoStruct* hitTriangleInfo, RayHitTriangleInfoStruct* hitSecondClosestTriangleInfo);

    //ray class to find intersection with Terrain Cells
    Ray* mRayTerrain = nullptr;

    //ray class to find intersection with Columns
    Ray* mRayColumns = nullptr;

    //creates final TriangleSelectors to be able to do ray
    //intersection from user mouse pointer to level environment
    //for object selection
    void createTriangleSelectors();

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

    //update the itemSelector, which means
    //we cast a ray from the current camera to the
    //mouse pointer position, and figure out which
    //items/entitys the players is currently selecting
    void Update();

    void OnLeftMouseButtonDown();

    void SetStateFrozen(bool frozen);
    bool GetStateFrozen();

    void DebugDraw();
};

#endif // ITEMSELECTOR_H
