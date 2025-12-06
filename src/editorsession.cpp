/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "editor/texturemode.h"
#include "editor/columndesign.h"
#include "editor/viewmode.h"
#include "editorsession.h"
#include "utils/logger.h"
#include "utils/tprofile.h"
#include "models/levelblocks.h"
#include "models/levelterrain.h"
#include "editor.h"
#include "resources/texture.h"
#include "models/morph.h"
#include "utils/logging.h"
#include "models/column.h"
#include "models/collectable.h"
#include "draw/drawdebug.h"
#include "input/input.h"
#include "resources/mapentry.h"
#include "resources/columndefinition.h"
#include "resources/blockdefinition.h"
#include "editor/itemselector.h"
#include "editor/terraforming.h"
#include "editor/entitymode.h"
#include "models/entitymanager.h"
#include "editor/regionmode.h"

EditorSession::EditorSession(Editor* parentEditor, std::string levelRootPath, std::string levelName) {
    mParentEditor = parentEditor;

    mLevelRootPath = levelRootPath;
    mLevelName = levelName;
    ready = false;

    mTextureMode = new TextureMode(this);
    mColumnDesigner = new ColumnDesigner(this);
    mViewMode = new ViewMode(this);
    mTerraforming = new TerraformingMode(this);
    mEntityMode = new EntityMode(this);
    mRegionMode = new RegionMode(this);

//    //my vector of extended region data
//    mExtRegionVec = new std::vector<ExtendedRegionInfoStruct*>;
//    mExtRegionVec->clear();

//    //my vector of charging stations
//    mChargingStationVec = new std::vector<ChargingStation*>;
//    mChargingStationVec->clear();

//    mCollectableSpawnerVec.clear();

//    //my vector of player that need help
//    //of a recovery vehicle and are currently waiting
//    //for it
//    mPlayerWaitForRecoveryVec = new std::vector<Player*>;
//    mPlayerWaitForRecoveryVec->clear();

//    mPlayerVec.clear();
//    mPlayerPhysicObjVec.clear();
//    playerRaceFinishedVec.clear();
//    mTriggerRegionVec.clear();
//    mPendingTriggerTargetGroups.clear();
//    mType2CollectableForCleanupLater.clear();

//    //for the start of the race we want to trigger
//    //target group 1 once
//    mPendingTriggerTargetGroups.push_back(1);
}

//Returns true in case of success, False otherwise
bool EditorSession::SaveAs(std::string levelRootPath, std::string levelName) {
    //set new path and file information
    mLevelRootPath = levelRootPath;
    mLevelName = levelName;

    std::string newMapFileName(levelRootPath);
    newMapFileName.append(levelName);
    newMapFileName.append("-unpacked.dat");

    //save the map file itself
    return mLevelRes->Save(newMapFileName);
}

void EditorSession::UpdateMorphs(irr::f32 frameDeltaTime) {
    std::list<Morph*>::iterator itMorph;

    for (itMorph = mEntityManager->Morphs.begin(); itMorph != mEntityManager->Morphs.end(); ++itMorph) {
        (*itMorph)->Update(frameDeltaTime);
    }
}

void EditorSession::AdvanceTime(irr::f32 frameDeltaTime) {
    float progressMorph;

    //Handle morphs here if activated
    //in the level editor
    if (mRunMorphs) {
        absTimeMorph += frameDeltaTime;
        progressMorph = (float)fmin(1.0f, fmax(0.0f, 0.5f + sin(absTimeMorph)));

        std::list<Morph*>::iterator itMorph;

        for (itMorph = mEntityManager->Morphs.begin(); itMorph != mEntityManager->Morphs.end(); ++itMorph) {
                (*itMorph)->setProgress(progressMorph);
                this->mLevelTerrain->ApplyMorph((**itMorph));
                (*itMorph)->MorphColumns();
        }

        //Trigger Mesh update
        CheckForMeshUpdate();
     }

     //if (!AllowStartMorphsPerKey) {
     //        //update level morphs
     //        UpdateMorphs(frameDeltaTime);
     //}

     mParentEditor->mTimeProfiler->Profile(mParentEditor->mTimeProfiler->tIntMorphing);

     //Update "special" EditorEntities, like SteamFoutains
     mEntityManager->UpdateSteamFoutains(frameDeltaTime);
}

void EditorSession::ActivateMorphs() {
    absTimeMorph = 0.0f;
    mRunMorphs = true;
}

void EditorSession::DeactivateMorphs() {
    //reset level to original morph state
    //at map loading time
    absTimeMorph = 0.0f;

    std::list<Morph*>::iterator itMorph;

    //first reset all morphs progress back to zero
    //is important so that ReverseDestroyAllColumns call below works
    //correctly
    for (itMorph = mEntityManager->Morphs.begin(); itMorph != mEntityManager->Morphs.end(); ++itMorph) {
            (*itMorph)->setProgress(0.0f);
    }

    //first unhide (reverse destroy) destroyed columns
    mLevelBlocks->ReverseDestroyAllColumns();

    //apply morphs one last time
    //to reset all Irrlicht Mesh to default state
    for (itMorph = mEntityManager->Morphs.begin(); itMorph != mEntityManager->Morphs.end(); ++itMorph) {
            this->mLevelTerrain->ApplyMorph((**itMorph));
            (*itMorph)->MorphColumns();
    }

    //Trigger Mesh update
    CheckForMeshUpdate();

    mRunMorphs = false;
}

void EditorSession::HideUIElements() {
    if (mModeInfoText != nullptr) {
        mModeInfoText->setVisible(false);
        mModeInfoText->setEnabled(false);
    }

    if (mControlInfoText != nullptr) {
        mControlInfoText->setVisible(false);
        mControlInfoText->setEnabled(false);
    }

    if (mAssignedLevelInfoText != nullptr) {
        mAssignedLevelInfoText->setVisible(false);
        mAssignedLevelInfoText->setEnabled(false);
    }
}

void EditorSession::UnhideUIElements() {
    if (mModeInfoText != nullptr) {
        mModeInfoText->setVisible(true);
        mModeInfoText->setEnabled(true);
    }

    if (mControlInfoText != nullptr) {
        mControlInfoText->setVisible(true);
        mControlInfoText->setEnabled(true);
    }

    if (mAssignedLevelInfoText != nullptr) {
        mAssignedLevelInfoText->setVisible(true);
        mAssignedLevelInfoText->setEnabled(true);
    }
}

void EditorSession::CheckForMeshUpdate() {
    mLevelTerrain->CheckForMeshUpdate();
    mLevelBlocks->CheckForMeshUpdate();
}

EditorSession::~EditorSession() {
    if (mItemSelector != nullptr)
    {
        delete mItemSelector;
        mItemSelector = nullptr;
    }

    if (mTextureMode != nullptr)
    {
        delete mTextureMode;
        mTextureMode = nullptr;
    }

    if (mColumnDesigner != nullptr)
    {
        delete mColumnDesigner;
        mColumnDesigner = nullptr;
    }

    if (mRegionMode != nullptr)
    {
        delete mRegionMode;
        mRegionMode = nullptr;
    }

    if (mViewMode != nullptr)
    {
        delete mViewMode;
        mViewMode = nullptr;
    }

    if (mTerraforming != nullptr)
    {
        delete mTerraforming;
        mTerraforming = nullptr;
    }

    if (mEntityMode != nullptr)
    {
        delete mEntityMode;
        mEntityMode = nullptr;
    }

    if (mArrowRightBillSceneNode != nullptr) {
        mArrowRightBillSceneNode->remove();

        mArrowRightBillSceneNode = nullptr;
    }

    mModeInfoText->remove();
    mControlInfoText->remove();
    mAssignedLevelInfoText->remove();

    //free lowlevel level data
    delete mEntityManager;
    delete mLevelBlocks;
    delete mLevelTerrain;
    delete mLevelRes;

    //free all loaded textures
    delete mTexLoader;

    //remove all remaining SceneNodes
    mParentEditor->CleanupAllSceneNodes();
}

void EditorSession::UpdateAssignedLevelInfoText() {
    //is currently no level file assigned?
    if (mParentEditor->mCurrLevelWhichIsEdited == nullptr) {
        this->mAssignedLevelInfoText->setText(L"Project not saved!");
    } else {
        irr::core::stringw newLevelAssigenment(L"Level: ");
        newLevelAssigenment.append(mParentEditor->mCurrLevelWhichIsEdited->levelName.c_str());
        this->mAssignedLevelInfoText->setText(newLevelAssigenment.c_str());
    }
}

void EditorSession::Init() {
    //we want to adjust the keymap for the free movable camera
    SKeyMap keyMap[4];

    keyMap[0].Action=EKA_MOVE_FORWARD;   keyMap[0].KeyCode=KEY_KEY_W;
    keyMap[1].Action=EKA_MOVE_BACKWARD;  keyMap[1].KeyCode=KEY_KEY_S;
    keyMap[2].Action=EKA_STRAFE_LEFT;    keyMap[2].KeyCode=KEY_KEY_A;
    keyMap[3].Action=EKA_STRAFE_RIGHT;   keyMap[3].KeyCode=KEY_KEY_D;

    //create a free moving camera that the user can use to
    //investigate the level/map, not used in actual game
    mCamera = mParentEditor->mSmgr->addCameraSceneNodeFPS(0, 100.0f,0.05f ,-1 ,
                                            keyMap, 4, false, 0.0f);

    mCamera->setPosition(irr::core::vector3df(0.0f, 0.0f, 0.0f));

    mCamera2 = mParentEditor->mSmgr->addCameraSceneNodeFPS(0, 100.0f,0.05f ,-1 ,
                                            keyMap, 4, false, 0.0f);

    mCamera2->setPosition(irr::core::vector3df(0.0f, 0.0f, 0.0f));

    mParentEditor->mSmgr->setActiveCamera(mCamera);

    if (!LoadLevel()) {
        //there was an error loading the level
        return;
    }

    //level was loaded ok, we can continue setup

    //create the current mode output info text
    this->mModeInfoText = mParentEditor->mGuienv->addStaticText(L"",
           irr::core::rect<irr::s32>(1150, 50, 1250, 75), false, true, nullptr, -1, true);

    this->mModeInfoText->setWordWrap(false);
    this->mModeInfoText->setVisible(true);

    this->mControlInfoText = mParentEditor->mGuienv->addStaticText(L"",
           irr::core::rect<irr::s32>(1150, 85, 1250, 110), false, true, nullptr, -1, true);

    this->mControlInfoText->setWordWrap(false);
    this->mControlInfoText->setVisible(true);

    this->mControlInfoText->setText(L"Freefly Mode");

    //create the current assigned level file information text
    this->mAssignedLevelInfoText = mParentEditor->mGuienv->addStaticText(L"",
           irr::core::rect<irr::s32>(1150, 120, 1250, 145), false, true, nullptr, -1, true);

    this->mAssignedLevelInfoText->setWordWrap(false);
    this->mAssignedLevelInfoText->setVisible(true);

    UpdateAssignedLevelInfoText();

    mParentEditor->UpdateStatusbarText(L"Press Space to change between FreeFyling Mode and Edit Mode/Using the UI");

    //create the object for path finding and services
    //mPath = new Path(this, mDrawDebug);

    //get player start locations from the level file
    /*mPlayerStartLocations =
        this->mLevelTerrain->GetPlayerRaceTrackStartLocations();*/

    //SetupTopRaceTrackPointerOrigin();

    //create a new Bezier object for testing
    //testBezier = new Bezier(mLevelTerrain, mDrawDebug);

    ready = true;

    //only to test if we can save a levelfile properly!
    //std::string testsaveName("testsave.dat");
    //this->mLevelRes->Save(testsaveName);
}

bool EditorSession::LoadLevel() {
//    if ((mLevelNrLoaded < 1) || (mLevelNrLoaded > 9)) {
//        logging::Error("Level number only possible from 1 up to 9!");
//        return false;
//    }

//    int load_texnr = mLevelNrLoaded;
//    if (mLevelNrLoaded == 7) load_texnr = 1; // original game has this hardcoded too

//   /***********************************************************/
//   /* Load selected level file                                */
//   /***********************************************************/
//   char levelfilename[50];
//   char str[20];

//   strcpy(levelfilename, "extract/level0-");
//   sprintf(str, "%d", mLevelNrLoaded);
//   strcat(levelfilename, str);
//   strcat(levelfilename, "/level0-");
//   strcat(levelfilename, str);
//   strcat(levelfilename, "-unpacked.dat");

//   //only for debugging
//   //strcpy(levelfilename, "/home/wolfalex/hi/maps/level0-1.dat");

//   char texfilename[50];
//   strcpy(texfilename, "extract/level0-");
//   sprintf(str, "%d", load_texnr);
//   strcat(texfilename, str);
//   strcat(texfilename, "/tex");

   std::string levelfilename("");
   std::string texfilename("");

   texfilename.append(mLevelRootPath);
   levelfilename.append(mLevelRootPath);

   texfilename.append("tex");

   levelfilename.append(mLevelName);
   levelfilename.append("-unpacked.dat");

   std::string spritefilename("extract/sprites/tmaps");

   /***********************************************************/
   /* Load level textures                                     */
   /***********************************************************/
   //Note: We need to additional load the level editor textures!
   mTexLoader = new TextureLoader(mParentEditor->mDriver, texfilename.c_str(), spritefilename.c_str(), true);

   //was loading textures succesfull? if not interrupt
   if (!this->mTexLoader->mLoadSuccess) {
       logging::Error("EditorSession::LoadTextures failed, exiting");
       return false;
   }

   /***********************************************************/
   /* Load selected level file                                */
   /***********************************************************/

   //load the level data itself
   this->mLevelRes = new LevelFile(this->mParentEditor, levelfilename);

   //was loading level data succesful? if not interrupt
   if (!this->mLevelRes->get_Ready()) {
       logging::Error("EditorSession::LoadLevel failed, exiting");
       return false;
   }

   /***********************************************************/
   /* Prepare level terrain                                   */
   /***********************************************************/
   //for the level editor do not optimize the Terrain mesh!
   this->mLevelTerrain = new LevelTerrain(this->mParentEditor, true, this->mLevelRes,
                                          mTexLoader, false, false);

   /***********************************************************/
   /* Create building (cube) Mesh                             */
   /***********************************************************/
   //this routine also generates the column/block collision information inside that
   //we need for collision detection later
   this->mLevelBlocks = new LevelBlocks(this->mParentEditor, this->mLevelTerrain, this->mLevelRes, mTexLoader, true,
                                        DebugShowWallCollisionMesh, false, mParentEditor->enableBlockPreview);

   //we can only set levelBlocks afterwards in Terrain
   //unfortunetly! do not forget it!
   mLevelTerrain->SetLevelBlocks(mLevelBlocks);

   mEntityManager = new EntityManager(this->mParentEditor, mLevelRes, mLevelTerrain, mLevelBlocks, mTexLoader);

   //create all level entities
   //this are not only items to pickup by the player
   //but also waypoints, collision information, checkpoints
   //level morph information and so on...
   mEntityManager->CreateLevelEntities();

   //The second part of the terrain initialization can only be done
   //after the map entities are loaded in another part of the code
   //Because we need to know where the morph areas are, to be able to put
   //dynamic parts of the terrain mesh into their own Meshbuffers and SceneNodes
   //for performance improvement reasons
   mLevelTerrain->FinishTerrainInitialization();

   if (this->mLevelTerrain->Terrain_ready == false) {
       //something went wrong with the terrain loading, exit application
       return false;
   }

   //do not show special Editor entity item (for example SteamFountain) selection boxes
   mEntityManager->SetShowSpecialEditorEntityTransparentSelectionBoxes(false);

   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES, true);
   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY, true);
   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CONES, true);

   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS, false);
   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS, false);

   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS, false);
   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS, false);
   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS, true);
   mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS, false);

   //create my Item selector which allows the user to
   //select level items by moving the move cursor onto them
   this->mItemSelector = new ItemSelector(this);

  // driver->setFog(video::SColor(0,138,125,81), video::EFT_FOG_LINEAR, 100, 250, .03f, false, true);

  //Final data calculation for all checkpoints
  //Find for each waypoint normal direction of race
  //so that we know if the player passes checkpoints
  //in forwards or reverse direction
  //CheckPointPostProcessing();

  //create existing charging stations
  //CreateChargingStations();

  //create a bounding box for valid player
  //location testing
  mLevelTerrain->StaticTerrainSceneNode->updateAbsolutePosition();

  //create an additional BillboardSceneNode which shows an white arrow
  //to show the user that currently a move action is going on
  mArrowRightBillSceneNode = mParentEditor->mSmgr->addBillboardSceneNode();
  mArrowRightBillSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
  mArrowRightBillSceneNode->setMaterialTexture(0, mTexLoader->editorTex.at(4));

  //Important: let collectables (Billboards) unaffected by lightning
  mArrowRightBillSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
  mArrowRightBillSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

  mArrowRightBillSceneNode->setSize(mEntityManager->GetCollectableSize());
  mArrowRightBillSceneNode->setVisible(false);

  return true;
}

void EditorSession::ShowArrowPointingRightAtCell(irr::core::vector2di cellCoord) {
    irr::core::vector3df pos(0.0f, 0.0f, 0.0f);

    pos.X = -cellCoord.X * DEF_SEGMENTSIZE - 0.5f;
    pos.Z = cellCoord.Y * DEF_SEGMENTSIZE + 0.5f;

    irr::core::vector2di outCell;

    pos.Y = mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(pos.X, pos.Z, outCell) + 0.2f;

    mArrowRightBillSceneNode->setPosition(pos);
    mArrowRightBillSceneNode->setVisible(true);
}

void EditorSession::HideArrowPointingRight() {
    mArrowRightBillSceneNode->setVisible(false);
}

void EditorSession::SetIllumination(bool enabled) {
    mLevelTerrain->SetIllumination(enabled);
    mLevelBlocks->SetIllumination(enabled);
}

bool EditorSession::IsIlluminationEnabled() {
    return (mLevelTerrain->IsIlluminationEnabled() && mLevelBlocks->IsIlluminationEnabled());
}

irr::s32 EditorSession::GetNextFreeGuiId() {
    irr::s32 newId = mNextFreeGuiId;
    mNextFreeGuiId++;

    return newId;
}

void EditorSession::DrawCellVertexCross(CurrentlySelectedEditorItemInfoStruct* mSelVertex, ColorStruct* color) {
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
            &mLevelTerrain->pTerrainTiles[xSel][ySel];

    TerrainTileData* tile2 = nullptr;

    int width = mLevelRes->Width();
    int height = mLevelRes->Height();

    if ((xSel + 1) < width) {
        tile2 = &mLevelTerrain->pTerrainTiles[xSel + 1][ySel];
    }

    TerrainTileData* tile3 = nullptr;

    if ((xSel - 1) >= 0) {
        tile3 = &mLevelTerrain->pTerrainTiles[xSel - 1][ySel];
    }

    TerrainTileData* tile4 = nullptr;

    if ((ySel + 1) < height) {
        tile4 = &mLevelTerrain->pTerrainTiles[xSel][ySel + 1];
    }

    tile4 = &mLevelTerrain->pTerrainTiles[xSel][ySel + 1];

    TerrainTileData* tile5 = nullptr;

    if ((ySel - 1) >= 0) {
        tile5 = &mLevelTerrain->pTerrainTiles[xSel][ySel - 1];
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
    mParentEditor->mDrawDebug->Draw3DLine(markerPos, markerPos2, color);
    mParentEditor->mDrawDebug->Draw3DLine(markerPos, markerPos3, color);
    mParentEditor->mDrawDebug->Draw3DLine(markerPos, markerPos4, color);
    mParentEditor->mDrawDebug->Draw3DLine(markerPos, markerPos5, color);
}

void EditorSession::Render() {
    //if we do not use XEffects we can simply render the sky
    //with XEffect this does not work, need a solution for this!
//    if (!mInfra->mUseXEffects) {
//        //we need to draw sky image first, the remaining scene will be drawn on top of it
//        DrawSky();
//    }

    //draw 3D world coordinate axis with arrows
    mParentEditor->mDrawDebug->DrawWorldCoordinateSystemArrows();

    mEntityManager->Draw();

/*
    if (DebugShowLowLevelTriangleSelection) {
        if (triangleHitByMouse) {
             mParentEditor->mDrawDebug->Draw3DTriangleOutline(&triangleMouseHit.hitTriangle, mParentEditor->mDrawDebug->white);
        }

        if (secondTriangleHitByMouse) {
             mParentEditor->mDrawDebug->Draw3DTriangleOutline(&secondTriangleMouseHit.hitTriangle, mParentEditor->mDrawDebug->red);
        }
    }*/

    //function call below only for debugging possibility
    //of ItemSelector
    mItemSelector->Draw();

    if (mEditorMode != mViewMode) {
       //show currently highlighted level item (item over which
       //the users mouse cursor is currently)
       mEditorMode->OnDrawHighlightedLevelItem(&mItemSelector->mCurrHighlightedItem);

       //draw currently selected level item (item which the user
       //clicked the last time with the left mouse button)
       mEditorMode->OnDrawSelectedLevelItem(&mItemSelector->mCurrSelectedItem);
    }

    //call currently selected editor mode
    //draw method
    if (mEditorMode != nullptr) {
       mEditorMode->OnDraw();
    }

     //mDrawDebug->Draw3DLine(mDbgRay.start , mDbgRay.end, mDrawDebug->cyan);
   //   mDrawDebug->Draw3DLine(*mDrawDebug->origin, dbgRayEnd, mDrawDebug->blue);

        //mDrawDebug->Draw3DLine(dbgRayStart, dbgRayEnd, mDrawDebug->blue);

          //mDrawDebug->Draw3DRectangle(dbgRayStart, dbgRayEnd, -dbgRayStart, -dbgRayEnd, mDrawDebug->blue);

    /*if (mCellSelectedByMouse) {
        DrawOutlineSelectedCell(mCellCoordSelectedByMouse, mDrawDebug->white);
    }*/

//    if (DebugShowCheckpoints) {
//          //draw all checkpoint lines for debugging purposes
//          mInfra->mDriver->setMaterial(*mDrawDebug->blue);
//          for(CheckPoint_iterator = checkPointVec->begin(); CheckPoint_iterator != checkPointVec->end(); ++CheckPoint_iterator) {
//              mInfra->mDriver->draw3DLine((*CheckPoint_iterator)->pLineStruct->A, (*CheckPoint_iterator)->pLineStruct->B);
//          }
//    }

//        if (DebugShowRegionsAndPointOfInterest) {
//                std::list<MapPointOfInterest>::iterator it;

//                for (it = this->mLevelRes->PointsOfInterest.begin(); it != this->mLevelRes->PointsOfInterest.end(); ++it) {
//                    mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, (*it).Position,
//                                           this->mDrawDebug->pink);
//                }

//                IndicateMapRegions();
//        }

//        if (DebugShowTriggerRegions) {
//            IndicateTriggerRegions();
//        }

       /* if (mChargingStationVec->size() > 0) {
            std::vector<ChargingStation*>::iterator it;
            for (it = mChargingStationVec->begin(); it != mChargingStationVec->end(); ++it) {
                (*it)->DebugDraw();
            }
        }*/

}

void EditorSession::MoveUserViewToLocation(irr::core::vector3df newCameraLookAtPnt, irr::f32 cameraDistance) {
    irr::core::vector3df newCamLocation;

    newCamLocation.X = newCameraLookAtPnt.X + sqrt(cameraDistance);
    newCamLocation.Z = newCameraLookAtPnt.Z;
    newCamLocation.Y = newCameraLookAtPnt.Y + sqrt(cameraDistance);

    mCamera->setPosition(newCamLocation);
    mCamera->setTarget(newCameraLookAtPnt);
}

void EditorSession::HandleBasicInput() {
    //update current mouse Position
    mCurrentMousePos = mParentEditor->MouseState.Position;

    //update item selection
    if ((mItemSelector != nullptr) && (mEditorMode != mViewMode)) {
        mItemSelector->Update();
    }

    if(mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_F4))
    {
         if (mParentEditor->mLogger->IsWindowHidden()) {
             mParentEditor->mLogger->ShowWindow();
         } else {
             mParentEditor->mLogger->HideWindow();
         }

         if (mParentEditor->mTimeProfiler->IsWindowHidden()) {
             mParentEditor->mTimeProfiler->ShowWindow();
         } else {
             mParentEditor->mTimeProfiler->HideWindow();
         }
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_C)) {
        scene::ICameraSceneNode * camera =
                mParentEditor->mDevice->getSceneManager()->getActiveCamera();

        if (camera == this->mCamera) {
            mItemSelector->SetStateFrozen(true);
            mParentEditor->mDevice->getSceneManager()->setActiveCamera(mCamera2);
            std::cout << "now cam 2, frozen" << std::endl;
        } else if (camera == this->mCamera2) {
              mItemSelector->SetStateFrozen(false);
               mParentEditor->mDevice->getSceneManager()->setActiveCamera(mCamera);
                 std::cout << "now cam, unfrozen" << std::endl;
        }
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
        TriggerClose();
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_M)) {
        ActivateMorphs();
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_N)) {
        DeactivateMorphs();
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_T)) {

    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_PLUS)) {
        if (mEditorMode != nullptr) {
            if (mEditorMode == mTerraforming) {
                mTerraforming->OnSelectedItemUp();
            }
        }
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_MINUS)) {
        if (mEditorMode != nullptr) {
            if (mEditorMode == mTerraforming) {
                mTerraforming->OnSelectedItemDown();
            }
        }
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_H)) {
        //mParentEditor->UpdateStatusbarText(L"Testtext");
     /*   if (mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
           mItemSelector->mCurrSelectedItem.mColumnSelected->AdjustMeshBaseVerticeHeight( 14.0f, 14.0f, 14.0f, 14.0f);
        }*/
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_SPACE)) {
         scene::ICameraSceneNode * camera =
                 mParentEditor->mDevice->getSceneManager()->getActiveCamera();
                 if (camera)
                    {
                        camera->setInputReceiverEnabled( !camera->isInputReceiverEnabled() );
                        if (camera->isInputReceiverEnabled()) {
                              this->mControlInfoText->setText(L"Freefly Mode");
                              mFreeFlyMode = true;
                              if (mEditorMode != nullptr) {
                                  mEditorMode->HideWindow();
                              }
                        } else {
                              this->mControlInfoText->setText(L"Edit Mode");
                              mFreeFlyMode = false;
                                if (mEditorMode != nullptr) {
                                    mEditorMode->ShowWindow();
                                }
                        }
                    }
              }

    //in editing mode, when user is in an active editor mode, and in the editor
    //mode dialog window, forward certain keypresses to be able to control editor mode window
    if (!mFreeFlyMode) {
        if (mUserInDialogState != DEF_EDITOR_USERINNODIALOG) {
               if (mEditorMode != nullptr) {
                    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_UP)) {
                         mEditorMode->OnKeyPressedInWindow(irr::KEY_UP);
                    }

                    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_DOWN)) {
                         mEditorMode->OnKeyPressedInWindow(irr::KEY_DOWN);
                    }
               }
        }
    }
}

void EditorSession::TrackActiveDialog() {
    mLastUserInDialogState = mUserInDialogState;
    mUserInDialogState = DEF_EDITOR_USERINNODIALOG;

    if (mTextureMode != nullptr) {
        if (mTextureMode->IsWindowOpen()) {
           irr::core::rect<s32> windowPos = mTextureMode->GetWindowPosition();
           if (windowPos.isPointInside(mCurrentMousePos)) {
               //mouse cursor is currently inside
               //texture mode window
               mUserInDialogState = DEF_EDITOR_USERINTEXTUREDIALOG;
           }
        }
    }

    if (mViewMode != nullptr) {
        if (mViewMode->IsWindowOpen()) {
           irr::core::rect<s32> windowPos = mViewMode->GetWindowPosition();
           if (windowPos.isPointInside(mCurrentMousePos)) {
               //mouse cursor is currently inside
               //view mode window
               mUserInDialogState = DEF_EDITOR_USERINVIEWMODEDIALOG;
           }
        }
    }

    if (mTerraforming != nullptr) {
        if (mTerraforming->IsWindowOpen()) {
           irr::core::rect<s32> windowPos = mTerraforming->GetWindowPosition();
           if (windowPos.isPointInside(mCurrentMousePos)) {
               //mouse cursor is currently inside
               //terraforming window
               mUserInDialogState = DEF_EDITOR_USERINTERRAFORMINGDIALOG;
           }
        }
    }

    if (mEntityMode != nullptr) {
        if (mEntityMode->IsWindowOpen()) {
           irr::core::rect<s32> windowPos = mEntityMode->GetWindowPosition();
           if (windowPos.isPointInside(mCurrentMousePos)) {
               //mouse cursor is currently inside
               //EntityMode window
               mUserInDialogState = DEF_EDITOR_USERINENTITYMODEDIALOG;
           }
        }
    }

    if (mColumnDesigner != nullptr) {
        if (mColumnDesigner->IsWindowOpen()) {
           irr::core::rect<s32> windowPos = mColumnDesigner->GetWindowPosition();
           if (windowPos.isPointInside(mCurrentMousePos)) {
               //mouse cursor is currently inside
               //column designer window
               mUserInDialogState = DEF_EDITOR_USERINCOLUMNDESIGNERDIALOG;
           }
        }
    }

    if (mRegionMode != nullptr) {
        if (mRegionMode->IsWindowOpen()) {
           irr::core::rect<s32> windowPos = mRegionMode->GetWindowPosition();
           if (windowPos.isPointInside(mCurrentMousePos)) {
               //mouse cursor is currently inside
               //Region mode window
               mUserInDialogState = DEF_EDITOR_USERINREGIONMODEDIALOG;
           }
        }
    }

    if (mLastUserInDialogState != mUserInDialogState) {
        if (mUserInDialogState == DEF_EDITOR_USERINTEXTUREDIALOG) {
            //std::cout << "Mouse cursor entered TextureMode window!" << std::endl;
        }

        if (mUserInDialogState == DEF_EDITOR_USERINCOLUMNDESIGNERDIALOG) {
            //std::cout << "Mouse cursor entered ColumnDesigner window!" << std::endl;
        }

        if (mUserInDialogState == DEF_EDITOR_USERINNODIALOG) {
            //std::cout << "Mouse cursor is currently over no window!" << std::endl;
        }
    }
}

//Returns true if morphing is currently
//enabled
bool EditorSession::IsMorphingRunning() {
  return mRunMorphs;
}

void EditorSession::SetFog(bool enabled) {
   mLevelBlocks->SetFog(enabled);
   mLevelTerrain->SetFog(enabled);
   mEntityManager->SetFog(enabled);
}

void EditorSession::TriggerClose() {
   mParentEditor->mGuienv->addMessageBox(L"Question", L"Sure to close EditorSession and to lose unsaved data?",
                                         true, EMBF_YES + EMBF_CANCEL, 0, GUI_ID_EDITORSESSION_MSGBOX_SURECLOSE, nullptr);
}

void EditorSession::RemoveEverythingFromLevel() {
    //make sure all morphs are currently disabled!
    DeactivateMorphs();

    //remove every existing column
    mLevelBlocks->RemoveEveryColumn();

    mLevelTerrain->ResetTextureCompleteTerrain(0);
}

void EditorSession::SetMode(EditorMode* selMode) {
    if (selMode == nullptr)
        return;

    //hide possible open window of current
    //editor mode
    if (mEditorMode != nullptr) {
        mEditorMode->HideWindow();

        //call OnExit function
        mEditorMode->OnExitMode();
    }

    //set new Editor mode
    mEditorMode = selMode;

    //update editor mode name info text
    this->mModeInfoText->setText(mEditorMode->mModeNameStr.c_str());

    //control what ItemSelector is actually
    //Selecting dependent on the new Editor Mode
    if (selMode == mViewMode) {
        //do not select anything in ViewMode
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, false);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENABLOCKS, false);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENAENTITIES, false);

        //for special editor entities like SteamFountain, do not show the transparent
        //selection boxes
        mEntityManager->SetShowSpecialEditorEntityTransparentSelectionBoxes(false);

        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CONES, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS, false);
    } else if ((selMode == mTextureMode) || (selMode == mColumnDesigner)) {
        //do only select cells and Blocks in Texture and Column Designer Mode
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, true);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENABLOCKS, true);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENAENTITIES, false);

        //for special editor entities like SteamFountain, do not show the transparent
        //selection boxes
        mEntityManager->SetShowSpecialEditorEntityTransparentSelectionBoxes(false);

        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CONES, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS, false);
    } else if (selMode == mTerraforming) {
        //do only select cells in Terraforming mode, the columns are modified depending
        //on the cells below them
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, true);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENABLOCKS, false);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENAENTITIES, false);

        //for special editor entities like SteamFountain, do not show the transparent
        //selection boxes
        mEntityManager->SetShowSpecialEditorEntityTransparentSelectionBoxes(false);

        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CONES, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS, false);
    } else if (selMode == mEntityMode) {
        //do select cells and entities in EntityMode
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, true);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENABLOCKS, false);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENAENTITIES, true);

        //for special editor entities like SteamFountain, do show the transparent
        //selection boxes in entity mode 
        mEntityManager->SetShowSpecialEditorEntityTransparentSelectionBoxes(true);

        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CONES, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS, true);
    } else if (selMode == mRegionMode) {
        //default do not select anything in RegionMode when we start this mode
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENACELLS, false);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENABLOCKS, false);
        mItemSelector->SetEnableSelection(DEF_EDITOR_SELITEM_ENAENTITIES, false);

        //for special editor entities like SteamFountain, do not show the transparent
        //selection boxes
        mEntityManager->SetShowSpecialEditorEntityTransparentSelectionBoxes(false);

        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_COLLECTIBLES, true);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_RECOVERY, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CONES, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WAYPOINTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_WALLSEGMENTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_TRIGGERS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_CAMERAS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_EFFECTS, false);
        mEntityManager->SetVisible(DEF_EDITOR_ENTITYMANAGER_SHOW_MORPHS, false);
    }

    //call OnEnter function
    mEditorMode->OnEnterMode();
}
