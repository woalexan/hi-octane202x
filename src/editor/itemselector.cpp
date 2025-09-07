/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "itemselector.h"
#include "../editorsession.h"
#include "../models/levelterrain.h"
#include "../models/levelblocks.h"
#include "../resources/mapentry.h"
#include "../resources/columndefinition.h"
#include "../editor.h"
#include "../draw/drawdebug.h"
#include "../models/column.h"
#include "../models/entitymanager.h"
#include "../models/editorentity.h"

ItemSelector::ItemSelector(EditorSession* parent) {
    mParent = parent;

    mRayTerrain = new Ray(mParent->mParentEditor->mDrawDebug);
    mRayColumns = new Ray(mParent->mParentEditor->mDrawDebug);

    createTriangleSelectors();

    //Add triangle selector of terrain to rayTarget mesh so that we can figure out
    //at which terrain cells the users mouse it pointing at currently
    mRayTerrain->AddRayTargetMesh(this->triangleSelectorStaticTerrain);
    mRayTerrain->AddRayTargetMesh(this->triangleSelectorDynamicTerrain);

    //do the same for all columns (blocks) in the level, so that the user
    //can select columns (blocks)
    mRayColumns->AddRayTargetMesh(this->triangleSelectorColumnswoCollision);
    mRayColumns->AddRayTargetMesh(this->triangleSelectorColumnswCollision);

    //at the start nothing is selected and nothing is highlighted
    mCurrSelectedItem.SelectedItemType = DEF_EDITOR_SELITEM_NONE;
    mCurrHighlightedItem.SelectedItemType = DEF_EDITOR_SELITEM_NONE;
}

ItemSelector::~ItemSelector() {
    delete mRayTerrain;
    delete mRayColumns;
}

void ItemSelector::UpdateTrianglesSelectors() {
    mRayTerrain->RemoveRayTargetMesh(this->triangleSelectorStaticTerrain);
    mRayTerrain->RemoveRayTargetMesh(this->triangleSelectorDynamicTerrain);

    mRayColumns->RemoveRayTargetMesh(this->triangleSelectorColumnswoCollision);
    mRayColumns->RemoveRayTargetMesh(this->triangleSelectorColumnswCollision);

    //unset the current triangle selector, this will also drop the current
    //selector
    mParent->mLevelBlocks->BlockCollisionSceneNode->setTriangleSelector(nullptr);
    mParent->mLevelBlocks->BlockWithoutCollisionSceneNode->setTriangleSelector(nullptr);

    mParent->mLevelTerrain->StaticTerrainSceneNode->setTriangleSelector(nullptr);
    mParent->mLevelTerrain->DynamicTerrainSceneNode->setTriangleSelector(nullptr);

    //Create the new triangle selectors
    //based on the new Meshes
    createTriangleSelectors();

    mRayTerrain->AddRayTargetMesh(this->triangleSelectorStaticTerrain);
    mRayTerrain->AddRayTargetMesh(this->triangleSelectorDynamicTerrain);

    mRayColumns->AddRayTargetMesh(this->triangleSelectorColumnswoCollision);
    mRayColumns->AddRayTargetMesh(this->triangleSelectorColumnswCollision);
}

void ItemSelector::SetEnableSelection(irr::u8 whichTypeItem, bool enabledState) {
    switch (whichTypeItem) {
        case DEF_EDITOR_SELITEM_ENACELLS: {
            mEnaSelectCells = enabledState;
            break;
        }

        case DEF_EDITOR_SELITEM_ENABLOCKS: {
            mEnaSelectBlocks = enabledState;
            break;
        }

        case DEF_EDITOR_SELITEM_ENAENTITIES: {
            mEnaSelectEntities = enabledState;
            break;
        }

        default: {
          break;
        }
    }
}

//the following function is for a special case, where the
//user wants to select the cell below a column (to change the
//column floor texture Id in the texturing tool)
//With a ray the user can not do this if the column is directly
//standing on the cell. Therefore the texturing tool is able to call
//the function below, to externally force a selection of the cell
void ItemSelector::SelectSpecifiedCellAtCoordinate(int x, int y) {
    int width = mParent->mLevelRes->Width();
    int height = mParent->mLevelRes->Height();

    if ((x < 0) || (y < 0) || (x >= width) || (y >= height)) {
        return;
    }

    mCurrSelectedItem.SelectedItemType = DEF_EDITOR_SELITEM_CELL;
    mCurrSelectedItem.mCellCoordSelected.set(x, y);

    //just take the first vertice as default
    mCurrSelectedItem.mCellCoordVerticeNrSelected = 0;

    return;
}

//the following function is for a special case, where the user
//selected a cell below an already existing EntityItem in EntityMode,
//and we want to actually force the selection of the EntityItem itself
//Therefore the EntityMode tool can call this function to select
//the Entity itself
void ItemSelector::SelectEntityAtCellCoordinate(int x, int y) {
    int width = mParent->mLevelRes->Width();
    int height = mParent->mLevelRes->Height();

    if ((x < 0) || (y < 0) || (x >= width) || (y >= height)) {
        return;
    }

    //which EditorEntity object is at this cell?
    EditorEntity* entityItem;

    if (mParent->mEntityManager->IsEntityItemAtCellCoord(x, y, &entityItem)) {
        if (entityItem != nullptr) {
            mCurrSelectedItem.SelectedItemType = DEF_EDITOR_SELITEM_ENTITY;
            mCurrSelectedItem.mEntitySelected = entityItem;

            mCurrSelectedItem.mCellCoordSelected.X = entityItem->GetCellCoord().X;
            mCurrSelectedItem.mCellCoordSelected.Y = entityItem->GetCellCoord().Y;
        }
    }
}

//derives more high level information about the highlighted terrain cell
void ItemSelector::DeriveHighlightedTerrainCellInformation(RayHitTriangleInfoStruct* hitTriangleInfo) {
    //a terrain cell is selected currently
    mCurrHighlightedItem.SelectedItemType = DEF_EDITOR_SELITEM_CELL;
    mCurrHighlightedItem.RayHitInfo = *hitTriangleInfo;
    mCurrHighlightedItem.mColumnSelected = nullptr;

    int closestVertice;

    //which cell coordinate is selected
    mCurrHighlightedItem.mCellCoordSelected =
            mParent->mLevelTerrain->GetClosestTileGridCoordToMapPosition(hitTriangleInfo->hitPointOnTriangle, closestVertice);

    mCurrHighlightedItem.mCellCoordVerticeNrSelected = closestVertice;
}

irr::core::vector3df ItemSelector::GetTriangleMidPoint(RayHitTriangleInfoStruct* triangleInfo) {
    irr::core::vector3df result;

    result.X = (triangleInfo->hitTriangle.pointA.X + triangleInfo->hitTriangle.pointB.X + triangleInfo->hitTriangle.pointC.X) / 3.0f;
    result.Y = (triangleInfo->hitTriangle.pointA.Y + triangleInfo->hitTriangle.pointB.Y + triangleInfo->hitTriangle.pointC.Y) / 3.0f;
    result.Z = (triangleInfo->hitTriangle.pointA.Z + triangleInfo->hitTriangle.pointB.Z + triangleInfo->hitTriangle.pointC.Z) / 3.0f;

    return result;
}

//derives more high level information about the highlighted block
void ItemSelector::DeriveHighlightedBlockInformation(RayHitTriangleInfoStruct* hitTriangleInfo, RayHitTriangleInfoStruct* hitSecondClosestTriangleInfo) {
    //a block of a column is selected currently
    mCurrHighlightedItem.SelectedItemType = DEF_EDITOR_SELITEM_BLOCK;
    mCurrHighlightedItem.RayHitInfo = *hitTriangleInfo;

    int closestVertice;

    //if we want to find the column position closer to the middle we can use the 2 hit triangles we got, from the enter and exit point
    //we just need to use the arithmetic middle value of enter and exit triangles
    irr::core::vector3df midPos1 = GetTriangleMidPoint(hitTriangleInfo);
    irr::core::vector3df midPos2 = GetTriangleMidPoint(hitSecondClosestTriangleInfo);

    irr::core::vector3df centerPoint = (midPos1 + midPos2) / 2.0f;

    dbgAvgPos = centerPoint;

    //according to the coordinates of the hitpoint, in which cell coordinate are way?
    irr::core::vector2di cellCoord =
        mParent->mLevelTerrain->GetClosestTileGridCoordToMapPosition(centerPoint, closestVertice);

    //also add information in which cell coordinate this column is standing at
    mCurrHighlightedItem.mCellCoordSelected.X = cellCoord.X;
    mCurrHighlightedItem.mCellCoordSelected.Y = cellCoord.Y;

    //is at the cellCoord a column?
    MapEntry* entry = mParent->mLevelTerrain->GetMapEntry(cellCoord.X, cellCoord.Y);

    ColumnDefinition* columDef = entry->get_Column();

    if (columDef != nullptr) {
        mCurrHighlightedItem.mColumnDefinitionSelected = columDef;

        //which column is this?
        //there is a "position" key we can use for search
        int posKey =  cellCoord.X + cellCoord.Y * mParent->mLevelTerrain->levelRes->Width();
        Column* columnPntr;

        if (mParent->mLevelBlocks->SearchColumnWithPosition(posKey, columnPntr)) {
            //we found the column
            mCurrHighlightedItem.mColumnSelected = columnPntr;
        } else {
            mCurrHighlightedItem.mColumnSelected = nullptr;
        }

        /*irr::f32 heightOfTerrain =
                mParent->mLevelTerrain->GetMapEntry(cellCoord.X,cellCoord.Y)->m_Height;*/

        irr::f32 heightOfTerrain =
                -mParent->mLevelTerrain->GetHeightInterpolated((irr::f32)(cellCoord.X),(irr::f32)(cellCoord.Y));

        //in which block from the base are we currently?
        int blockCntFromTerrain = 0;

        irr::f32 blockhmin = heightOfTerrain;
        irr::f32 blockhmax = heightOfTerrain + mParent->mLevelTerrain->segmentSize;

        while ((!((centerPoint.Y >= blockhmin) && (centerPoint.Y <= blockhmax))) && (blockCntFromTerrain < 8)) {
            blockhmin += mParent->mLevelTerrain->segmentSize;
            blockhmax += mParent->mLevelTerrain->segmentSize;
            blockCntFromTerrain++;
        }

        mCurrHighlightedItem.mSelBlockNrStartingFromBase = blockCntFromTerrain;

        //figure out which "face" of the column cube is closest to the user
        //means in which face the ray enters the cube
        irr::core::vector3df dirVecRay = (midPos2 - midPos1).normalize();

        irr::f32 dotProdXAxis = dirVecRay.dotProduct(*this->mParent->mParentEditor->xAxisDirVector);
        irr::f32 dotProdYAxis = dirVecRay.dotProduct(*this->mParent->mParentEditor->yAxisDirVector);
        irr::f32 dotProdZAxis = dirVecRay.dotProduct(*this->mParent->mParentEditor->zAxisDirVector);

        //we are watching into the axis-direction for the axis with the highest absolute value closest to 1.0
        irr::f32 absDotProdXAxis = fabs(dotProdXAxis);
        irr::f32 absDotProdYAxis = fabs(dotProdYAxis);
        irr::f32 absDotProdZAxis = fabs(dotProdZAxis);

        //in the original game block north face means: in my Z-AXIS positive direction
        // -||- block east means: in my X-axis positive direction
        // -||- block south means: in my Z-AXIS negative direction
        // -||- block west means: in my X-axis negative direction
        // -||- block bottom means: in my Y-axis positive direction
        // -||- block top means: in my Y-axis negative direction

        if (absDotProdXAxis > absDotProdYAxis) {
            if (absDotProdZAxis > absDotProdXAxis) {
                //maximum value is for ZAxis
                //now we need to look at the sign, if it is positive then we look into
                //Z axis direction, otherwise we look against Z axis direction
                if (dotProdZAxis > 0.0f) {
                    //we look into Z axis direction
                    //this means north
                    mCurrHighlightedItem.mSelBlockFaceDirection = DEF_SELBLOCK_FACENORTH;
                } else {
                    //we look against Z axis direction
                    //this means south
                    mCurrHighlightedItem.mSelBlockFaceDirection = DEF_SELBLOCK_FACESOUTH;
                }
            } else {
                //maximum value is for XAxis
                 if (dotProdXAxis > 0.0f) {
                    //this means west
                    mCurrHighlightedItem.mSelBlockFaceDirection = DEF_SELBLOCK_FACEWEST;
                 } else {
                     //this means east
                     mCurrHighlightedItem.mSelBlockFaceDirection = DEF_SELBLOCK_FACEEAST;
                 }
            }
        } else {
            if (absDotProdYAxis > absDotProdZAxis) {
                //maximum value is for YAxis
                 if (dotProdYAxis > 0.0f) {
                    //this means bottom
                    mCurrHighlightedItem.mSelBlockFaceDirection = DEF_SELBLOCK_FACEBOTTOM;
                 } else {
                    //this means top
                    mCurrHighlightedItem.mSelBlockFaceDirection = DEF_SELBLOCK_FACETOP;
                 }
            } else {
                //maximum value is for ZAxis
                if (dotProdZAxis > 0.0f) {
                    //this means north
                    mCurrHighlightedItem.mSelBlockFaceDirection = DEF_SELBLOCK_FACENORTH;
                } else {
                    //this means south
                    mCurrHighlightedItem.mSelBlockFaceDirection = DEF_SELBLOCK_FACESOUTH;
                }
            }
        }
    }
}

//Returns true if an entity was selected
//by the user, false otherwise
bool ItemSelector::CheckForEntitySelection(irr::core::line3df rayLine, EditorEntity** selectedEntityItem) {
    std::vector<EditorEntity*>::iterator itEntity;
    irr::f32 distSquared;

    for (itEntity = mParent->mEntityManager->mEntityVec.begin(); itEntity != mParent->mEntityManager->mEntityVec.end(); ++itEntity) {
        //(*itEntity)->HideBoundingBox();

        //if Entity is currently not visible skip it
        if (!(*itEntity)->mVisible)
            continue;

         //coarse check, if we are very far away from the object with the camera, it can not be selected right now
        distSquared = (mParent->mCamera->getPosition() - (*itEntity)->mPosition).getLengthSQ();

        if (distSquared > 1000.0f)
            continue;

        //(*itEntity)->ShowBoundingBox();

        if ((*itEntity)->mBoundingBox.intersectsWithLine(rayLine)) {
            //(*itEntity)->ShowBoundingBox();
            *selectedEntityItem = (*itEntity);
            return true;
        }
    }

    return false;
}

//update the itemSelector, which means
//we cast a ray from the current camera to the
//mouse pointer position, and figure out which
//items/entitys the players is currently selecting
void ItemSelector::Update() {
    //if we are not currently frozen, update current ray line through
    //the users mouse cursor
    if (!mFrozen) {
        mRayLine = mParent->mParentEditor->mSmgr->getSceneCollisionManager()->getRayFromScreenCoordinates(
                         mParent->mCurrentMousePos, mParent->mCamera);
    }

    mCurrHighlightedItem.SelectedItemType = DEF_EDITOR_SELITEM_NONE;
    mCurrHighlightedItem.mColumnSelected = nullptr;
    mCurrHighlightedItem.mColumnDefinitionSelected = nullptr;
    mCurrHighlightedItem.mEntitySelected = nullptr;

    /***********************************************************************
     * First try mouse ray intersection with Terrain to find possible      *
     * selected Terrain cell                                               *
     ***********************************************************************/

    //core::line3df ray;

    //const scene::SViewFrustum* frustum = mCamera->getViewFrustum();
    //ray.start = frustum->getNearLeftUp() + (frustum->getNearRightDown()-frustum->getNearLeftUp())*0.5f;
    //ray.start = rayLine.start;
    //ray.end = rayLine.end;

    irr::core::vector3df dirVec = (mRayLine.end - mRayLine.start).normalize();
    mRayLine.end = mRayLine.start + dirVec * 100.0f;

    mDbgTerrainClosestTriangleHit = false;

    /*cam1PosInsideMouseHandler = mCamera->getAbsolutePosition();
    mFpsCameraLocation = cam1PosInsideMouseHandler;
    ray.start = mDbgRay.start;
    ray.end = mDbgRay.end;*/

    //mSceneNodeGluedToFPSCamera->updateAbsolutePosition();
    //ray.start = mSceneNodeGluedToFPSCamera->getAbsolutePosition();

   // irr::core::vector3df dirVec = (ray.end - ray.start).normalize();

    //irr::core::vector3df endPoint = ray.start + ray.getVector().normalize() * 100.0f;
    //irr::core::vector3df endPoint = ray.start + dirVec * 100.0f;
    if (mEnaSelectEntities) {
            if (CheckForEntitySelection(mRayLine, &mCurrHighlightedItem.mEntitySelected)) {
                //an editor entityitem is currently selected
                mCurrHighlightedItem.SelectedItemType = DEF_EDITOR_SELITEM_ENTITY;

                //also return cell coordinate where this EntityItem is located at
                mCurrHighlightedItem.mCellCoordSelected.X = mCurrHighlightedItem.mEntitySelected->GetCellCoord().X;
                mCurrHighlightedItem.mCellCoordSelected.Y = mCurrHighlightedItem.mEntitySelected->GetCellCoord().Y;

                //we can exit here
                //selection of entities has priority
                return;
            }
    }

    //built a ray cast 3d line to find out at which 3D object the users mouse
    //is pointing at
    std::vector<irr::core::vector3di> voxels;

    std::vector<RayHitTriangleInfoStruct*> allHitTrianglesTerrain;

    //dbgRayStart = cam1PosInsideMouseHandler;
    //dbgRayEnd =  ray.end;

    //with ReturnOnlyClosestTriangles = true!
    allHitTrianglesTerrain = mRayTerrain->ReturnTrianglesHitByRay( mRayTerrain->mRayTargetSelectors,
                                  mRayLine.start, mRayLine.end, 10, true);

    size_t vecSize = (int)(allHitTrianglesTerrain.size());
    std::vector<RayHitTriangleInfoStruct*>::iterator it;

    irr::f32 minDistanceTerrain;
    RayHitTriangleInfoStruct* nearestTriangleHitTerrain = nullptr;

    if (vecSize > 0) {
            //we have at least one triangle, we need to find the closest one to the user
            bool firstElement = true;
            irr::f32 currDist;

            for (it = allHitTrianglesTerrain.begin(); it != allHitTrianglesTerrain.end(); ++it) {
                currDist = (*it)->distFromRayStartSquared;
                if (firstElement) {
                    firstElement = false;
                    minDistanceTerrain = currDist;
                    nearestTriangleHitTerrain = (*it);
                } else if (currDist < minDistanceTerrain) {
                    minDistanceTerrain = currDist;
                    nearestTriangleHitTerrain = (*it);
                }
            }
    }

    if (nearestTriangleHitTerrain != nullptr) {
        mDbgTerrainClosestTriangleHit = true;
        mDbgTerrainClosestTriangleHitData = *nearestTriangleHitTerrain;
    }

    /***********************************************************************
     * Second try mouse ray intersection with all columms of blocks        *
     * that we have in the level                                           *
     ***********************************************************************/

    std::vector<RayHitTriangleInfoStruct*> allHitTrianglesColumns;

    //with ReturnOnlyClosestTriangles = true!
    allHitTrianglesColumns = mRayColumns->ReturnTrianglesHitByRay( mRayColumns->mRayTargetSelectors,
                                  mRayLine.start, mRayLine.end, 10, true);

    vecSize = (int)(allHitTrianglesColumns.size());

    irr::f32 minDistanceColumns;
    RayHitTriangleInfoStruct* nearestTriangleHitColumns = nullptr;
    RayHitTriangleInfoStruct* secondNearestTriangleHitColumns = nullptr;
    irr::f32 secondMinDistanceColumns;

    mDbgBlockClosestTriangleHit = false;
    mDbgBlock2ndClosestTriangleHit = false;

    //if we hit through a cube we should have at least 2 hit triangles, one for the front where the ray enters,
    //the other one at the opposite site where the ray exits again
    if (vecSize > 1) {
            //we have at least two triangles, we need to find the closest and the second closest to the user
            bool firstElement = true;
            irr::f32 currDist;

            for (it = allHitTrianglesColumns.begin(); it != allHitTrianglesColumns.end(); ++it) {
                currDist = (*it)->distFromRayStartSquared;
                if (firstElement) {
                    firstElement = false;
                    minDistanceColumns = currDist;
                    nearestTriangleHitColumns = (*it);
                } else if (currDist < minDistanceColumns) {
                    minDistanceColumns = currDist;
                    nearestTriangleHitColumns = (*it);
                }
            }

            if (nearestTriangleHitColumns != nullptr) {
                    //now find the seconds closest triangle
                    firstElement = true;

                    for (it = allHitTrianglesColumns.begin(); it != allHitTrianglesColumns.end(); ++it) {
                        //for the second element we need to skip the already found closest element
                        //so that we find the 2nd closest triangle
                         currDist = (*it)->distFromRayStartSquared;

                        //19.06.2025: I believe I have a lot of explanation to do why I have this weird +0.02f
                        //below. I want to find the triangle below that is the second furthest away from the user
                        //position. In my mind this was easy, just take the next closest triangle which is not the triangle
                        //closest to the player. Except it did not work, because as I found out after some days of debugging:
                        //There seems to be actually two triangles at the same location which we find with the ray
                        //intersection from the triangle picker; I still did not find out, but wanted to fix the issue here.
                        //To get rid of the second unexpected triangle at the same location, I skip until I find the next
                        //triangle that is further away in distance that the first one; this removes the duplicate triangle!
                        if (((*it) != nearestTriangleHitColumns) && (currDist > (minDistanceColumns + 0.02f))) {
                            if (firstElement) {
                                firstElement = false;
                                secondMinDistanceColumns = currDist;
                                secondNearestTriangleHitColumns = (*it);
                            } else if (currDist < secondMinDistanceColumns) {
                                secondMinDistanceColumns = currDist;
                                secondNearestTriangleHitColumns = (*it);
                            }
                        }
                    }
            }
    }

    if (nearestTriangleHitColumns != nullptr) {
            mDbgBlockClosestTriangleHit = true;
            mDbgBlockClosestTriangleHitData = *nearestTriangleHitColumns;
    }

    if (secondNearestTriangleHitColumns != nullptr) {
            mDbgBlock2ndClosestTriangleHit = true;
            mDbgBlock2ndClosestTriangleHitData = *secondNearestTriangleHitColumns;
    }

    //for acceptable cube intersection of a column we need to have at least two hit triangles!
    bool foundCubeIntersection = ((nearestTriangleHitColumns != nullptr) && (secondNearestTriangleHitColumns != nullptr));

    //if we have found both an intersection of a block/cube and a terrain tile, take whatever is closest to the player
    if ((foundCubeIntersection) && (nearestTriangleHitTerrain != nullptr)) {
        if (minDistanceTerrain < minDistanceColumns) {
            //do not care about the cubes, terrain is closer
            foundCubeIntersection = false;
        } else {
            //cube is closer, take cube
            nearestTriangleHitTerrain = nullptr;
        }
    }

    //find out if the closest triangle we hit is from the terrain or
    //a column. In case we find a hit triangle for both ray intersections
    //the triangle with the shortest distance to the players mouse pointer
    //wins and is selected
    RayHitTriangleInfoStruct* triangleHit = nullptr;

    if (foundCubeIntersection && mEnaSelectBlocks) {
         triangleHit = nearestTriangleHitColumns;
         //secondTriangleHitByMouse = true;
         //secondTriangleMouseHit = *secondNearestTriangleHitColumns;
        DeriveHighlightedBlockInformation(nearestTriangleHitColumns, secondNearestTriangleHitColumns);
    }

    if ((nearestTriangleHitTerrain != nullptr) && mEnaSelectCells) {
        triangleHit = nearestTriangleHitTerrain;
        DeriveHighlightedTerrainCellInformation(nearestTriangleHitTerrain);
    }

    //store the triangle hit also for low level ray
    //implementation debugging in render function of level editor
    if (triangleHit != nullptr) {
        //triangleHitByMouse = true;
        //triangleMouseHit = *triangleHit;
    }

    //cleanup triangle hit information again
    //otherwise we have a memory leak!
    mRayColumns->EmptyTriangleHitInfoVector(allHitTrianglesColumns);

    //cleanup triangle hit information again
    //otherwise we have a memory leak!
    mRayTerrain->EmptyTriangleHitInfoVector(allHitTrianglesTerrain);
}

//creates final TriangleSelectors to be able to do ray
//intersection from user mouse pointer to level environment
//for object selection
void ItemSelector::createTriangleSelectors() {
   triangleSelectorColumnswCollision = mParent->mParentEditor->mSmgr->createOctreeTriangleSelector(
                mParent->mLevelBlocks->blockMeshForCollision, mParent->mLevelBlocks->BlockCollisionSceneNode, 128);
   mParent->mLevelBlocks->BlockCollisionSceneNode->setTriangleSelector(triangleSelectorColumnswCollision);

   triangleSelectorColumnswoCollision = mParent->mParentEditor->mSmgr->createOctreeTriangleSelector(
                mParent->mLevelBlocks->blockMeshWithoutCollision, mParent->mLevelBlocks->BlockWithoutCollisionSceneNode, 128);
   mParent->mLevelBlocks->BlockWithoutCollisionSceneNode->setTriangleSelector(triangleSelectorColumnswoCollision);

   triangleSelectorStaticTerrain = mParent->mParentEditor->mSmgr->createOctreeTriangleSelector(
               mParent->mLevelTerrain->myStaticTerrainMesh, mParent->mLevelTerrain->StaticTerrainSceneNode, 128);
   mParent->mLevelTerrain->StaticTerrainSceneNode->setTriangleSelector(triangleSelectorStaticTerrain);

   triangleSelectorDynamicTerrain = mParent->mParentEditor->mSmgr->createOctreeTriangleSelector(
               mParent->mLevelTerrain->myDynamicTerrainMesh, mParent->mLevelTerrain->DynamicTerrainSceneNode, 128);
   mParent->mLevelTerrain->DynamicTerrainSceneNode->setTriangleSelector(triangleSelectorDynamicTerrain);
}

void ItemSelector::OnLeftMouseButtonDown() {
    //only process event if user is currently over no
    //window
    if (mParent->mUserInDialogState != DEF_EDITOR_USERINNODIALOG)
        return;

    //if an element was highlighted when the user pressed down the
    //left mouse button => select this item now
    if (mCurrHighlightedItem.SelectedItemType != DEF_EDITOR_SELITEM_NONE) {
        //copy all the information
        mCurrSelectedItem = mCurrHighlightedItem;
    }
}

void ItemSelector::SetStateFrozen(bool frozen) {
    mFrozen = frozen;
}

bool ItemSelector::GetStateFrozen() {
    return mFrozen;
}

void ItemSelector::Draw() {
   //the commented out source code below is for selection ray intersection debugging only

   //draw current ray that is used for item/entity selection
   /* mParent->mParentEditor->mDrawDebug->Draw3DLine(mRayLine.start , mRayLine.end, mParent->mParentEditor->mDrawDebug->orange);

   if (mDbgTerrainClosestTriangleHit) {
        mParent->mParentEditor->mDrawDebug->Draw3DTriangleOutline(&mDbgTerrainClosestTriangleHitData.hitTriangle, mParent->mParentEditor->mDrawDebug->white);
   }

   if (mDbgBlockClosestTriangleHit) {
        mParent->mParentEditor->mDrawDebug->Draw3DTriangleOutline(&mDbgBlockClosestTriangleHitData.hitTriangle, mParent->mParentEditor->mDrawDebug->red);
   }
   if (!OnlyShowOneTriag) {
       if (mDbgBlock2ndClosestTriangleHit) {
            mParent->mParentEditor->mDrawDebug->Draw3DTriangleOutline(&mDbgBlock2ndClosestTriangleHitData.hitTriangle, mParent->mParentEditor->mDrawDebug->blue);
       }
   }

   mParent->mParentEditor->mDrawDebug->Draw3DLine(*mParent->mParentEditor->mDrawDebug->origin , dbgAvgPos, mParent->mParentEditor->mDrawDebug->pink);*/
}

