/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did translation to C++, and then modified and extended code for my project)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "levelterrain.h"
#include "levelblocks.h"
#include "column.h"
#include "../resources/texture.h"
#include "../infrabase.h"
#include "../models/morph.h"
#include "../resources/mapentry.h"
#include "../utils/logging.h"
#include "../resources/entityitem.h"
#include "../draw/drawdebug.h"
#include "irrmeshbuf.h"

void LevelTerrain::ResetTerrainTileData() {
    int levelWidth = this->levelRes->Width();
    int levelHeight = this->levelRes->Height();

    for (int i = 0; i < levelWidth; i++) {
        for (int j = 0; j < levelHeight; j++) {
            pTerrainTiles[i][j].vert1 = nullptr;
            pTerrainTiles[i][j].vert2 = nullptr;
            pTerrainTiles[i][j].vert3 = nullptr;
            pTerrainTiles[i][j].vert4 = nullptr;
            pTerrainTiles[i][j].vert1UVcoord.set(0.0f, 0.0f);
            pTerrainTiles[i][j].vert2UVcoord.set(0.0f, 0.0f);
            pTerrainTiles[i][j].vert3UVcoord.set(0.0f, 0.0f);
            pTerrainTiles[i][j].vert4UVcoord.set(0.0f, 0.0f);
            pTerrainTiles[i][j].myMeshBuffers.clear();
            pTerrainTiles[i][j].currTileHeight = 0.0f;
            if (mOptimizeMesh) {
                pTerrainTiles[i][j].m_draw_in_mesh = false;
            } else {
                pTerrainTiles[i][j].m_draw_in_mesh = true;
            }
            pTerrainTiles[i][j].vert1CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTiles[i][j].vert2CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTiles[i][j].vert3CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTiles[i][j].vert4CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTiles[i][j].myMeshBufVertexId1.clear();
            pTerrainTiles[i][j].vert1CurrPositionY = 0.0f;
            pTerrainTiles[i][j].vert2CurrPositionY = 0.0f;
            pTerrainTiles[i][j].vert3CurrPositionY = 0.0f;
            pTerrainTiles[i][j].vert4CurrPositionY = 0.0f;
            pTerrainTiles[i][j].VertUpdatedUVScoord = false;
            pTerrainTiles[i][j].RefreshNormals = false;
            pTerrainTiles[i][j].m_optimization_cnt = 0.0f;
            pTerrainTiles[i][j].vert1CurrPositionYDirty = false;
            pTerrainTiles[i][j].vert2CurrPositionYDirty = false;
            pTerrainTiles[i][j].vert3CurrPositionYDirty = false;
            pTerrainTiles[i][j].vert4CurrPositionYDirty = false;
        }
    }

    for (int i = 0; i < LEVELTERRAIN_WIDTH_ENDOFMAP; i++) {
        for (int j = 0; j < levelHeight; j++) {
            pTerrainTilesEndOfMap[i][j].vert1 = nullptr;
            pTerrainTilesEndOfMap[i][j].vert2 = nullptr;
            pTerrainTilesEndOfMap[i][j].vert3 = nullptr;
            pTerrainTilesEndOfMap[i][j].vert4 = nullptr;
            pTerrainTilesEndOfMap[i][j].vert1UVcoord.set(0.0f, 0.0f);
            pTerrainTilesEndOfMap[i][j].vert2UVcoord.set(0.0f, 0.0f);
            pTerrainTilesEndOfMap[i][j].vert3UVcoord.set(0.0f, 0.0f);
            pTerrainTilesEndOfMap[i][j].vert4UVcoord.set(0.0f, 0.0f);
            pTerrainTilesEndOfMap[i][j].myMeshBuffers.clear();
            pTerrainTilesEndOfMap[i][j].currTileHeight = 0.0f;

            //Always draw this specific tiles!
            pTerrainTilesEndOfMap[i][j].m_draw_in_mesh = true;

            pTerrainTilesEndOfMap[i][j].vert1CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTilesEndOfMap[i][j].vert2CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTilesEndOfMap[i][j].vert3CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTilesEndOfMap[i][j].vert4CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTilesEndOfMap[i][j].myMeshBufVertexId1.clear();
            pTerrainTilesEndOfMap[i][j].vert1CurrPositionY = 0.0f;
            pTerrainTilesEndOfMap[i][j].vert2CurrPositionY = 0.0f;
            pTerrainTilesEndOfMap[i][j].vert3CurrPositionY = 0.0f;
            pTerrainTilesEndOfMap[i][j].vert4CurrPositionY = 0.0f;
            pTerrainTilesEndOfMap[i][j].VertUpdatedUVScoord = false;
            pTerrainTilesEndOfMap[i][j].RefreshNormals = false;
            pTerrainTilesEndOfMap[i][j].m_optimization_cnt = 0.0f;
            pTerrainTilesEndOfMap[i][j].vert1CurrPositionYDirty = false;
            pTerrainTilesEndOfMap[i][j].vert2CurrPositionYDirty = false;
            pTerrainTilesEndOfMap[i][j].vert3CurrPositionYDirty = false;
            pTerrainTilesEndOfMap[i][j].vert4CurrPositionYDirty = false;
        }
    }
}

void LevelTerrain::DrawRegionOutline(MapTileRegionStruct* whichRegion, ColorStruct* whichColor) {
    if (whichRegion == nullptr)
        return;

    irr::core::vector3df pos1;
    irr::core::vector3df pos2;
    irr::core::vector3df pos3;
    irr::core::vector3df pos4;
    irr::core::vector2di cell;

    pos1.X = -whichRegion->tileXmin * DEF_SEGMENTSIZE;
    pos1.Z = whichRegion->tileYmin * DEF_SEGMENTSIZE;
    //increase Y coordinate slightly, too not get flicker effect between
    //drawn lines and terrain tiles itself
    pos1.Y = GetCurrentTerrainHeightForWorldCoordinate(pos1.X, pos1.Z, cell) + 0.1f;

    pos2.X = -whichRegion->tileXmax * DEF_SEGMENTSIZE;
    pos2.Z = whichRegion->tileYmax * DEF_SEGMENTSIZE;
    pos2.Y = GetCurrentTerrainHeightForWorldCoordinate(pos2.X, pos2.Z, cell) + 0.1f;

    pos3.X = -whichRegion->tileXmin * DEF_SEGMENTSIZE;
    pos3.Z = whichRegion->tileYmax * DEF_SEGMENTSIZE;
    pos3.Y = GetCurrentTerrainHeightForWorldCoordinate(pos3.X, pos3.Z, cell) + 0.1f;

    pos4.X = -whichRegion->tileXmax * DEF_SEGMENTSIZE;
    pos4.Z = whichRegion->tileYmin * DEF_SEGMENTSIZE;
    pos4.Y = GetCurrentTerrainHeightForWorldCoordinate(pos4.X, pos4.Z, cell) + 0.1f;

    mInfra->mDrawDebug->Draw3DRectangle(pos1, pos3, pos2, pos4, whichColor);
}

void LevelTerrain::DrawMapRegions() {
    std::vector<MapTileRegionStruct*>::iterator it;

    ColorStruct *color = mInfra->mDrawDebug->red;

    for (it = levelRes->mMapRegionVec->begin(); it != levelRes->mMapRegionVec->end(); ++it) {
           if ((*it)->regionType == LEVELFILE_REGION_CHARGER_FUEL) {
               color = mInfra->mDrawDebug->colorFuelCharger;
           } else if ((*it)->regionType == LEVELFILE_REGION_CHARGER_SHIELD) {
               color = mInfra->mDrawDebug->colorShieldCharger;
           } else if ((*it)->regionType == LEVELFILE_REGION_CHARGER_AMMO) {
               color = mInfra->mDrawDebug->colorAmmoCharger;
           } else if ((*it)->regionType == LEVELFILE_REGION_START) {
               color = mInfra->mDrawDebug->red;
           }

           DrawRegionOutline((*it), color);
   }
}

std::vector<irr::core::vector3df> LevelTerrain::GetPlayerRaceTrackStartLocations() {
  //first we need to get the "region" in which the player start points are located
  //There are two different ways for me to get them (maybe because I do not know the whole story
  //how the game works inside), each level seems to work differently
  //Way 1: for example works fine for levels 1, 2, 3; There is an "unknown" table at offset 247264
  //of the mapfile which contains the location of the map starting point; For this maps we can use this information
  std::vector<MapTileRegionStruct*>::iterator it;
  MapTileRegionStruct startRegion;
  bool regionFound = false;

  std::vector<irr::core::vector3df> fndStartLocations;
  fndStartLocations.clear();

  //see first if way 1 works for this map; do we find a specification for the starting point?
  for (it = this->levelRes->mMapRegionVec->begin(); it != this->levelRes->mMapRegionVec->end(); ++it) {
        if ((*it)->regionType == LEVELFILE_REGION_START) {
              //we found a start region, use this information
              startRegion = *(*it);
              regionFound = true;
           }
  }

  if (regionFound) {
      //we found starting region using way 1
      //but we want to find the actual location of the starting locations
      //on the map
      for (int j = (int)(startRegion.tileYmin); j <= (int)(startRegion.tileYmax); j++) {
         for (int i = (int)(startRegion.tileXmin); i <= (int)(startRegion.tileXmax); i++) {
              //textureId value 122 is the starting position for
              //all kind of race track textures
              if (this->levelRes->pMap[i][j]->m_TextureId == 122) {
                  irr::core::vector3df newLoc;
                  irr::f32 segSize = DEF_SEGMENTSIZE;
                  //our X axis is swapped compared to level file
                  newLoc.X = -(this->levelRes->pMap[i][j]->get_X()) * segSize - segSize * 0.5f;
                  newLoc.Z = (this->levelRes->pMap[i][j]->get_Z()) * segSize + segSize * 0.5f;
                  newLoc.Y = this->levelRes->pMap[i][j]->m_Height + HOVER_HEIGHT;

                  //add detected starting location to result vector
                  fndStartLocations.push_back(newLoc);
              }
          }
      }

      //TODO 18.08.2024: There is a map (it is original map number 6) with only 4 starting textureIds, if we need 8 players we will crash
      //implement another workaround here to calculate more starting locations based on the first found ones
      //(for example simply place the other crafts with a certain distance behind the first ones like the game seems to do)
      if (fndStartLocations.size() < 8) {
          irr::u8 missingPlaceNr = 8 - (irr::u8)(fndStartLocations.size());

          //just allow to add one additional starting places behind the ones
          //in the map
          if (missingPlaceNr > (irr::u8)(fndStartLocations.size())) {
              missingPlaceNr = (irr::u8)(fndStartLocations.size());
          }

          irr::f32 segSize = DEF_SEGMENTSIZE;

          for (irr::u8 cnt = 0; cnt < missingPlaceNr; cnt++) {
              irr::core::vector3df newLocation = fndStartLocations.at(cnt);

              //place the new starting position 1 segment behind
              newLocation.Z += segSize;

              //make sure Y is also correct, in case terrain is different there
              irr::core::vector2di outCell;

              GetCurrentTerrainHeightForWorldCoordinate(newLocation.X, newLocation.Z, outCell);
              newLocation.Y = this->levelRes->pMap[outCell.X][outCell.Y]->m_Height + HOVER_HEIGHT;

              //add additional starting location to result vector
              fndStartLocations.push_back(newLocation);
          }
      }


      return fndStartLocations;
  }

  //Way 2: If way 1 did not work, happens for example for levels 4 and 8,...; We can still search the region by searching
  //for the textureId for the starting locations in the level Terrain
  if (!regionFound) {
      int w = this->levelRes->Width();
      int h = this->levelRes->Height();
      //we need to use way 2
      for (int i = 0; i < w; i++) {
          for (int j = 0; j < h; j++) {
              //textureId value 122 is the starting position for
              //all kind of race track textures
              if (this->levelRes->pMap[i][j]->m_TextureId == 122) {
                  irr::core::vector3df newLoc;
                  irr::f32 segSize = DEF_SEGMENTSIZE;
                  //our X axis is swapped compared to level file
                  newLoc.X = -(this->levelRes->pMap[i][j]->get_X() * segSize) - segSize * 0.5f;
                  newLoc.Z = (this->levelRes->pMap[i][j]->get_Z() * segSize) + segSize * 0.5f;
                  newLoc.Y = this->levelRes->pMap[i][j]->m_Height + HOVER_HEIGHT;

                  //add detected starting location to result vector
                  fndStartLocations.push_back(newLoc);
              }
          }
      }
  }

  //TODO 18.08.2024: There is a map with only 4 starting textureIds, if we need 8 players we will crash
  //implement another workaround here to calculate more starting locations based on the first found ones
  //(for example simply place the other crafts with a certain distance behind the first ones like the game seems to do)
  return fndStartLocations;
}

bool LevelTerrain::CheckPosInsideRegion(int posX, int posY, MapTileRegionStruct* regionStructPntr) {
    if (posX < regionStructPntr->tileXmin)
        return false;

    if (posX > regionStructPntr->tileXmax)
        return false;

    if (posY > regionStructPntr->tileYmax)
        return false;

    if (posY < regionStructPntr->tileYmin)
        return false;

    //we are inside region, return true
    return true;
}

//returns true if two specified regions (r1x1, r1y1, r1x2, r1y2) and (r2x1, r2y1, r2x2, r2y2) are overlapping
//false otherwise
bool LevelTerrain::Overlapping(irr::f32 r1x1, irr::f32 r1y1, irr::f32 r1x2, irr::f32 r1y2,
                               irr::f32 r2x1, irr::f32 r2y1, irr::f32 r2x2, irr::f32 r2y2) {
    if (r1x1 > r2x2 || r2x1 > r1x2)
          return false;

    if (r1y1 > r2y2 || r2y1 > r1y2)
          return false;

      return true;
}

void LevelTerrain::CheckPosInsideChargingRegion(int posX, int posY, bool &chargeShield, bool &chargeFuel, bool &chargeAmmo) {
    std::vector<MapTileRegionStruct*>::iterator it;
    bool insideRegion;

    chargeShield = false;
    chargeFuel = false;
    chargeAmmo = false;

    for (it = this->levelRes->mMapRegionVec->begin(); it != this->levelRes->mMapRegionVec->end(); ++it) {
        if (((*it)->regionType != LEVELFILE_REGION_START) && (*it)->regionType != LEVELFILE_REGION_UNDEFINED) {
           insideRegion = CheckPosInsideRegion(posX, posY, (*it));

           //Sidenote 26.12.2024: There is also a special situation in level 5, where there is a
           //shield charger and ammo charger at the same location; At this location we will find
           //overlapping regions for Ammo and shield charger at the same place, which will become
           //active at the exactly same time, we must not prevent this somehow!
           if (insideRegion) {
               if ((*it)->regionType == LEVELFILE_REGION_CHARGER_FUEL) {
                   chargeFuel = true;
               }

               if ((*it)->regionType == LEVELFILE_REGION_CHARGER_SHIELD) {
                   chargeShield = true;
               }

               if ((*it)->regionType == LEVELFILE_REGION_CHARGER_AMMO) {
                   chargeAmmo = true;
               }
           }
        }
    }
}

//The second part of the terrain initialization can only be done
//after the map entities are loaded in another part of the code
//Because we need to know where the morph areas are, to be able to put
//dynamic parts of the terrain mesh into their own Meshbuffers and SceneNodes
//for performance improvement reasons
void LevelTerrain::FinishTerrainInitialization() {
    //for the game we want (need) to optimize the
    //mesh, for the editor we do not want to do this
    if (mOptimizeMesh) {
        //optimize terrain
        FindTerrainOptimization();
    }

    bool successGeometry = SetupGeometry();

    if (!mLevelEditorMode) {
        successGeometry = successGeometry && SetupGeometryEndOfMap();
    }

    if (successGeometry) {
         std::string infoMsg("HiOctane Terrain loaded: ");
         char hlpstr[20];

         //add number vertices
         sprintf(hlpstr, "%u", mTerrainMeshStats->numVertices);
         infoMsg.append(hlpstr);
         infoMsg.append(" vertices, ");

         //add number normals
         sprintf(hlpstr, "%u", mTerrainMeshStats->numNormals);
         infoMsg.append(hlpstr);
         infoMsg.append(" normals, ");

         //add number UVs
         sprintf(hlpstr, "%u", mTerrainMeshStats->numUVs);
         infoMsg.append(hlpstr);
         infoMsg.append(" UVs, ");

         //add number textures
         sprintf(hlpstr, "%d", mTexSource->NumLevelTextures);
         infoMsg.append(hlpstr);
         infoMsg.append(" textures, ");

         //add number indices
         sprintf(hlpstr, "%u", mTerrainMeshStats->numIndices);
         infoMsg.append(hlpstr);
         infoMsg.append(" indices");

         logging::Info(infoMsg);

         infoMsg.clear();
         infoMsg.append("HiOctane Terrain loaded ok");
         logging::Info(infoMsg);

    } else {
        std::string errMsg("Failed setting up Terrain");
        logging::Error(errMsg);
        Terrain_ready = false;
    }

    if (Terrain_ready) {
        /***********************************************************/
        /* Create Terrain Mesh                                     */
        /***********************************************************/

        //Create Mesh for Terrain
        //CreateTerrainMesh();

        //create Static SceneNode for Terrain
        StaticTerrainSceneNode = this->mInfra->mSmgr->addMeshSceneNode(myStaticTerrainMesh, 0, IDFlag_IsPickable);

        //we need to rotate the terrain Mesh, otherwise it is upside down
        StaticTerrainSceneNode->setRotation(core::vector3df(0.0f, 0.0f, 180.0f));
        StaticTerrainSceneNode->setMaterialFlag(EMF_LIGHTING, mEnableLightning);

        StaticTerrainSceneNode->setMaterialFlag(EMF_FOG_ENABLE, true);

        //create dynamic SceneNode for Terrain
        DynamicTerrainSceneNode = this->mInfra->mSmgr->addMeshSceneNode(myDynamicTerrainMesh, 0, IDFlag_IsPickable);

        //we need to rotate the terrain Mesh, otherwise it is upside down
        DynamicTerrainSceneNode->setRotation(core::vector3df(0.0f, 0.0f, 180.0f));
        DynamicTerrainSceneNode->setMaterialFlag(EMF_LIGHTING, mEnableLightning);

        DynamicTerrainSceneNode->setMaterialFlag(EMF_FOG_ENABLE, true);

        if (!mLevelEditorMode) {
            //create SceneNode for Terrain X < 0 coordinates
            StaticTerrainSceneNodeEndOfMap = this->mInfra->mSmgr->addMeshSceneNode(myStaticTerrainMeshEndOfMap, 0, ID_IsNotPickable);

            //we need to rotate the terrain Mesh, otherwise it is upside down
            StaticTerrainSceneNodeEndOfMap->setRotation(core::vector3df(0.0f, 0.0f, 180.0f));
            StaticTerrainSceneNodeEndOfMap->setMaterialFlag(EMF_LIGHTING, mEnableLightning);

            StaticTerrainSceneNodeEndOfMap->setMaterialFlag(EMF_FOG_ENABLE, true);
        }
    }
}

void LevelTerrain::SetLevelBlocks(LevelBlocks* levelBlocks) {
    mLevelBlocks = levelBlocks;
}

LevelTerrain::LevelTerrain(InfrastructureBase* infra, bool levelEditorMode, LevelFile* levelRes,
                           TextureLoader* textureSource, bool optimizeMesh, bool enableLightning) {
   this->mInfra = infra;
   mEnableLightning = enableLightning;
   mOptimizeMesh = optimizeMesh;
   mLevelEditorMode = levelEditorMode;
   mTexSource = textureSource;

   mTerrainMeshStats = new MeshObjectStatsStruct();

   mIrrMeshBuf = new IrrMeshBuf(mTexSource, mEnableLightning);

   //initial fill the mStaticMeshBufferVec and
   //mDynamicMeshBufferVec vector
   //with empty MeshBufferInfroStructs, one for each possible
   //level texture Id
   mIrrMeshBuf->InitializeMeshBufferInfoStructs(mStaticMeshBufferVec);
   mIrrMeshBuf->InitializeMeshBufferInfoStructs(mDynamicMeshBufferVec);

   if (!mLevelEditorMode) {
       //only in game we also need this MeshBufferInfoStruct
       mIrrMeshBuf->InitializeMeshBufferInfoStructs(mStaticMeshBufferEndOfMapVec);
   }

   segmentSize = 1.0f; // must be 1 for Hi-Octane !!

   Terrain_ready = true;

   this->levelRes = levelRes;

   mIrrMeshBuf->ResetMeshStats(mTerrainMeshStats);

   //reset my internal map!
   ResetTerrainTileData();

   //only used for the level editor; Minimum number
   //of needed meshbuffers for each textureId so that
   //worst case the user can fill each possible tile of the map
   //with the same textureId

   //calc max possible number of LevelTerrain indices, each tile needs
   //6 indices to render it
   irr::u32 maxNrMapIndices = levelRes->Width() * levelRes->Height() * 6;

   //each Irrlicht Meshbuffer can hold maximum 65535 indices
   //calculate how many Irrlicht Meshbuffers we will need minimum in
   //level editor
   mLevelEditorMinNrMeshBuffersNeeded = (irr::u8)(round((irr::f32)(maxNrMapIndices) / (irr::f32)(65535)));

   //Default use illumination (especially in the game)
   CalculateIllumination();

   mIlluminationEnabled = true;
}

LevelTerrain::~LevelTerrain() {
  //remove my static SceneNode
  if (StaticTerrainSceneNode != nullptr) {
    StaticTerrainSceneNode->remove();
    StaticTerrainSceneNode = nullptr;
  }

  //remove my dynamic SceneNode
  if (DynamicTerrainSceneNode != nullptr) {
    DynamicTerrainSceneNode->remove();
    DynamicTerrainSceneNode = nullptr;
  }

  if (!mLevelEditorMode) {
      if (StaticTerrainSceneNodeEndOfMap != nullptr) {
        StaticTerrainSceneNodeEndOfMap->remove();
        StaticTerrainSceneNodeEndOfMap = nullptr;
      }
  }

  //free static terrain mesh TerrainMesh
  if (myStaticTerrainMesh != nullptr) {
    this->mInfra->mSmgr->getMeshCache()->removeMesh(myStaticTerrainMesh);
    myStaticTerrainMesh = nullptr;
  }

  //free dynamic terrain mesh TerrainMesh
  if (myDynamicTerrainMesh != nullptr) {
    this->mInfra->mSmgr->getMeshCache()->removeMesh(myDynamicTerrainMesh);
    myDynamicTerrainMesh = nullptr;
  }

   if (!mLevelEditorMode) {
       if (myStaticTerrainMeshEndOfMap != nullptr) {
         this->mInfra->mSmgr->getMeshCache()->removeMesh(myStaticTerrainMeshEndOfMap);
         myStaticTerrainMeshEndOfMap = nullptr;
       }
   }

  int levelWidth = this->levelRes->Width();
  int levelHeight = this->levelRes->Height();

  std::vector<irr::scene::SMeshBuffer*>::iterator itMesh;
  irr::scene::SMeshBuffer* pntrMeshBuf;

  //free all terrain meshbuffers, etc...
  for (int i = 0; i < levelWidth; i++) {
      for (int j = 0; j < levelHeight; j++) {
          //are there any pointers to meshbuffers inside we
          //want to clear?
          if (pTerrainTiles[i][j].myMeshBuffers.size() > 0) {
                for (itMesh = pTerrainTiles[i][j].myMeshBuffers.begin(); itMesh != pTerrainTiles[i][j].myMeshBuffers.end(); ) {
                    pntrMeshBuf = (*itMesh);
                    itMesh = pTerrainTiles[i][j].myMeshBuffers.erase(itMesh);

                    //Next line commented out, does crash program
                    //what is wrong here?
                    //pntrMeshBuf->drop();
                }
          }

         //free created vertices
         if (pTerrainTiles[i][j].vert1 != nullptr) {
            delete pTerrainTiles[i][j].vert1;
            pTerrainTiles[i][j].vert1 = nullptr;
         }

         //free created vertices
         if (pTerrainTiles[i][j].vert2 != nullptr) {
            delete pTerrainTiles[i][j].vert2;
            pTerrainTiles[i][j].vert2 = nullptr;
         }

         //free created vertices
         if (pTerrainTiles[i][j].vert3 != nullptr) {
            delete pTerrainTiles[i][j].vert3;
            pTerrainTiles[i][j].vert3 = nullptr;
         }

         //free created vertices
         if (pTerrainTiles[i][j].vert4 != nullptr) {
            delete pTerrainTiles[i][j].vert4;
            pTerrainTiles[i][j].vert4 = nullptr;
         }
       }
  }

  if (!mLevelEditorMode) {
      //free all terrain meshbuffers, etc...
      //from EndOfMap
      int idxHelper;

      for (int j = 0; j < levelHeight; j++) {
         idxHelper = 0;
         for (int i = (levelWidth - LEVELTERRAIN_WIDTH_ENDOFMAP); i < levelWidth; i++) {
              //are there any pointers to meshbuffers inside we
              //want to clear?
              if (pTerrainTilesEndOfMap[idxHelper][j].myMeshBuffers.size() > 0) {
                    for (itMesh = pTerrainTilesEndOfMap[idxHelper][j].myMeshBuffers.begin(); itMesh != pTerrainTilesEndOfMap[idxHelper][j].myMeshBuffers.end(); ) {
                        pntrMeshBuf = (*itMesh);
                        itMesh = pTerrainTilesEndOfMap[idxHelper][j].myMeshBuffers.erase(itMesh);

                        //Next line commented out, does crash program
                        //what is wrong here?
                        //pntrMeshBuf->drop();
                    }
              }

             //free created vertices
             if (pTerrainTilesEndOfMap[idxHelper][j].vert1 != nullptr) {
                delete pTerrainTilesEndOfMap[idxHelper][j].vert1;
                pTerrainTilesEndOfMap[idxHelper][j].vert1 = nullptr;
             }

             //free created vertices
             if (pTerrainTilesEndOfMap[idxHelper][j].vert2 != nullptr) {
                delete pTerrainTilesEndOfMap[idxHelper][j].vert2;
                pTerrainTilesEndOfMap[idxHelper][j].vert2 = nullptr;
             }

             //free created vertices
             if (pTerrainTilesEndOfMap[idxHelper][j].vert3 != nullptr) {
                delete pTerrainTilesEndOfMap[idxHelper][j].vert3;
                pTerrainTilesEndOfMap[idxHelper][j].vert3 = nullptr;
             }

             //free created vertices
             if (pTerrainTilesEndOfMap[idxHelper][j].vert4 != nullptr) {
                delete pTerrainTilesEndOfMap[idxHelper][j].vert4;
                pTerrainTilesEndOfMap[idxHelper][j].vert4 = nullptr;
             }

             idxHelper++;
           }
      }
  }

  mIrrMeshBuf->CleanupMeshBufferInfoStructs(mStaticMeshBufferVec);
  mIrrMeshBuf->CleanupMeshBufferInfoStructs(mDynamicMeshBufferVec);

  if (!mLevelEditorMode) {
       mIrrMeshBuf->CleanupMeshBufferInfoStructs(mStaticMeshBufferEndOfMapVec);
  }

  delete mTerrainMeshStats;
}

void LevelTerrain::SetIllumination(bool enabled) {
  bool updateMesh = false;

  if (enabled) {
        if (!mIlluminationEnabled) {
            CalculateIllumination();
            updateMesh = true;
            mIlluminationEnabled = true;
        }
    } else {
        if (mIlluminationEnabled) {
            DisableIllumination();
            mIlluminationEnabled = false;
            updateMesh = true;
        }
    }

  if (updateMesh) {
      int levelWidth = this->levelRes->Width();
      int levelHeight = this->levelRes->Height();

      for (int i = 0; i < levelWidth; i++) {
          for (int j = 0; j < levelHeight; j++) {
                //skip Mesh update here!
                //only update Mesh once at the end
                UpdateTileVerticeColors(i, j, true);
          }
      }
  }

  //Update Mesh only once at the end
  CheckForMeshUpdate();
}

bool LevelTerrain::IsIlluminationEnabled() {
    return mIlluminationEnabled;
}

//Returns true if input texture is a roadtexture
//according to a predefined list
bool LevelTerrain::IsRoadTexture(irr::s32 texture, bool addExtendedTextures) {
    for (std::vector<irr::s32>::iterator itTex = roadTexIdsVec.begin(); itTex != roadTexIdsVec.end(); ++itTex) {
        if ((*itTex) == texture) {
            //texture found, exit
            return true;
        }
    }

    if (addExtendedTextures) {
        for (std::vector<irr::s32>::iterator itTex = roadTexIdsVecExtendedForWorldAware.begin(); itTex != roadTexIdsVecExtendedForWorldAware.end(); ++itTex) {
            if ((*itTex) == texture) {
                //texture found, exit
                return true;
            }
        }
    }

    return false;
}

bool LevelTerrain::IsChargingStationTexture(irr::s32 texture) {
    //bool found;
    //std::vector<irr::s32>::iterator it2;

    for (std::vector<irr::s32>::iterator itTex = chargerTexIdsVec.begin(); itTex != chargerTexIdsVec.end(); ++itTex) {
        if ((*itTex) == texture) {

            //texture found, exit
       /*     found = false;
            for (it2 = mDbgChargerTexFound.begin(); it2 != mDbgChargerTexFound.end(); ++it2) {
                if ((*it2) == (*itTex)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                mDbgChargerTexFound.push_back(*itTex);
            }*/
            return true;
        }
    }

    return false;
}

/*void LevelTerrain::DebugOutputFoundChargingTextures() {
    std::vector<irr::s32>::iterator it;
    char strhlp[500];
    std::string msg("");

    logging::Info("Found charger texture Ids:");

    for (it = mDbgChargerTexFound.begin(); it != mDbgChargerTexFound.end(); ++it) {
        msg.clear();
        snprintf(strhlp, 500, "%d", (*it));
        msg.append(strhlp);

        logging::Info(msg);
    }
}*/

irr::video::IImage* LevelTerrain::CreateMiniMapInfo(irr::u32 &startWP, irr::u32 &endWP, irr::u32 &startHP, irr::u32 &endHP) {
    //iterate through all level tiles
    irr::u32 width = this->get_width();
    irr::u32 height = this->get_heigth();

    //Texture Tile Id info:
    //Texture ID = 43 -> Fuel charger
    //Texture ID = 47 -> Ammo charger
    //Texture ID = 51 -> Shield charger
    //116 - 156 Road Textures

    irr::video::SColor roadColor(255, 73, 81, 85);
    irr::video::SColor finishLineColor(255, 105, 121, 129);
    irr::video::SColor transparent(0, 0, 0, 0);
    irr::video::SColor cPixelCol;
    int texID;
    bool isRoadTex;

    //figure out real size of racetrack (getting rid of empty space around race track which has texture ID of 0)
    irr::u32 startW = 0;
    irr::u32 endW = 0;
    irr::u32 startH = 0;
    irr::u32 endH = 0;
    bool allPixelsZero;
    bool raceTrackStarted = false;
    bool dynamicTerrainCell;

    //find area with race track for height dimension
    //Important note: Do not include terrain tiles which are used for
    //morphing (dynamic terrain), because this areas are often outside of
    //the race track (hidden from player). If we include them, then the minimap position
    //will be off as the race track is assumed to be much bigger then it actually is
    for (irr::u32 idxH = 0; idxH < height; idxH++) {

        allPixelsZero = true;
         for (irr::u32 idxW = 0; idxW < width; idxW++) {
              texID = this->GetMapEntry(idxW, idxH)->m_TextureId;
              dynamicTerrainCell = this->pTerrainTiles[idxW][idxH].dynamicMesh;

              //exclude dynamic terrain tiles (morphing)!
              //see comment above!
              if ((texID != 0) && !dynamicTerrainCell) {
                  allPixelsZero = false;
                  break;
              }
         }

         if (!raceTrackStarted) {
            if (!allPixelsZero) {
                startH = idxH;
                raceTrackStarted = true;
            }
         } else {
             if (allPixelsZero) {
                            endH = idxH;
                            break;
                        }
         }
    }

    //find area with race track for width dimension
    raceTrackStarted = false;

    for (irr::u32 idxW = 0; idxW < width; idxW++) {

        allPixelsZero = true;
         for (irr::u32 idxH = 0; idxH < height; idxH++) {
              texID = this->GetMapEntry(idxW, idxH)->m_TextureId;
              dynamicTerrainCell = this->pTerrainTiles[idxW][idxH].dynamicMesh;

             //exclude dynamic terrain tiles (morphing)!
             //see comment above!
            if ((texID != 0) && !dynamicTerrainCell) {
                  allPixelsZero = false;
                  break;
              }
         }

         if (!raceTrackStarted) {
            if (!allPixelsZero) {
             startW = idxW;
             raceTrackStarted = true;
         }
         } else {
             if (allPixelsZero) {
                endW = idxW;
                break;
             }
         }
    }

    //return detected race track positioning information to caller
    //as this information is later needed to convert player locations
    //to minimap 2D positions
    startHP = startH;
    endHP = endH;
    startWP = startW;
    endWP = endW;

    //create an empty image, for each Terrain tile one pixel
    irr::video::IImage* imgMiniMap =
            mInfra->mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
                                  irr::core::dimension2d<irr::u32>(endW - startW, endH - startH));

    for (irr::u32 posx = startW; posx < endW; posx++) {
        for (irr::u32 posy = startH; posy < endH; posy++) {
            texID = this->GetMapEntry(posx, posy)->m_TextureId;

            cPixelCol = transparent;

            switch (texID) {
                case 43: {  //Fuel charger
                        cPixelCol = *mInfra->mDrawDebug->colorFuelCharger->color;
                        break;
                     }

                case 47: { //Ammo charger
                        cPixelCol = *mInfra->mDrawDebug->colorAmmoCharger->color;
                        break;
                     }

                case 51: { //Shield charger
                        cPixelCol = *mInfra->mDrawDebug->colorShieldCharger->color;
                        break;
                    }

                case 120:   //finish line
                case 121: {
                        cPixelCol = finishLineColor;
                        break;
                    }

                default:
                        break;
             }

            //do not add extended road textures here!
            //this would break the minimap creation
            isRoadTex = IsRoadTexture(texID, false);

            //any road texture?
            if (isRoadTex) {
                cPixelCol = roadColor;
            }

            imgMiniMap->setPixel(posx - startW, posy - startH, cPixelCol);
        }
    }

    return imgMiniMap;
}

std::vector<vector2d<irr::f32>> LevelTerrain::ApplyTexMod(vector2d<irr::f32> uvA, vector2d<irr::f32> uvB, vector2d<irr::f32> uvC, vector2d<irr::f32> uvD, int mod) {
   std::vector<vector2d<irr::f32>> uvs;

   switch (mod) {
        case 1: //RotateNoneFlipX
                uvs.push_back(uvB);
                uvs.push_back(uvA);
                uvs.push_back(uvD);
                uvs.push_back(uvC);
                break;
        case 2: //RotateNoneFlipY
                uvs.push_back(uvD);
                uvs.push_back(uvC);
                uvs.push_back(uvB);
                uvs.push_back(uvA);
                break;
        case 3: //Rotate180FlipNone
                uvs.push_back(uvC);
                uvs.push_back(uvD);
                uvs.push_back(uvA);
                uvs.push_back(uvB);
                break;
        case 4: //Rotate270FlipY
                uvs.push_back(uvA);
                uvs.push_back(uvD);
                uvs.push_back(uvC);
                uvs.push_back(uvB);
                break;
        case 5: //Rotate90FlipNone
                uvs.push_back(uvD);
                uvs.push_back(uvA);
                uvs.push_back(uvB);
                uvs.push_back(uvC);
                break;
        case 6: //Rotate270FlipNone
                uvs.push_back(uvB);
                uvs.push_back(uvC);
                uvs.push_back(uvD);
                uvs.push_back(uvA);
                break;
        case 7: //Rotate90FlipY
                uvs.push_back(uvC);
                uvs.push_back(uvB);
                uvs.push_back(uvA);
                uvs.push_back(uvD);
                break;
        case 0:
        default:
                uvs.push_back(uvA);
                uvs.push_back(uvB);
                uvs.push_back(uvC);
                uvs.push_back(uvD);
                break;
   }
   return uvs;
}

std::vector<vector2d<irr::f32>> LevelTerrain::MakeUVs(int texMod) {
    vector2d<irr::f32> uvA;
    vector2d<irr::f32> uvB;
    vector2d<irr::f32> uvC;
    vector2d<irr::f32> uvD;

    uvA.X = 0.0f; uvA.Y = 0.0f;
    uvB.X = 1.0f; uvB.Y = 0.0f;
    uvC.X = 1.0f; uvC.Y = 1.0f;
    uvD.X = 0.0f; uvD.Y = 1.0f;

    return ApplyTexMod(uvA, uvB, uvC, uvD, texMod);
}

MapEntry* LevelTerrain::GetMapEntry(int x, int y) {
    int Width = levelRes->Width();
    int Height = levelRes->Height();

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > Width - 1) x = Width - 1;
    if (y > Height - 1) y = Height - 1;
    return levelRes->pMap[x][y];
}

irr::f32 LevelTerrain::GetHeightInterpolated(irr::f32 x, irr::f32 z)
{
    // world to grid coords
    s32 ix = round32(x / segmentSize);
    s32 iz = round32(z / segmentSize);

    // check bounds
    if (ix < 0) ix = 0;
    if (iz < 0) iz = 0;

    int width = this->get_width();
    int height = this->get_heigth();

    if (ix > width - 1) ix = width - 1;
    if (iz > height - 1) iz = height - 1;

    int xPlusOne = min(ix + 1, width - 1);
    int zPlusOne = min(iz + 1, height - 1);

    // determine exact height
    //int ia = 4 * (ix       + iz       * levelRes.Width);
    //int ib = 4 * (xPlusOne + iz       * levelRes.Width);
    //int ic = 4 * (xPlusOne + zPlusOne * levelRes.Width);
    //int id = 4 * (ix       + zPlusOne * levelRes.Width);

    // get heights and proceed...
    irr::f32 a = this->pTerrainTiles[ix][iz].vert1CurrPositionY;
    irr::f32 b = this->pTerrainTiles[xPlusOne][iz].vert1CurrPositionY;
    irr::f32 c = this->pTerrainTiles[xPlusOne][zPlusOne].vert1CurrPositionY;
    irr::f32 d = this->pTerrainTiles[ix][zPlusOne].vert1CurrPositionY;

    irr::f32 hheight = 0.0f;

    irr::f32 sqX = x - ix;
    irr::f32 sqZ = z - iz;

    if (sqX + sqZ < 1)
    {
        hheight = a;
        hheight += (b - a) * sqX;
        hheight += (d - a) * sqZ;
    }
    else
    {
        hheight = c;
        hheight += (b - c) * (1.0f - sqZ);
        hheight += (d - c) * (1.0f - sqX);
    }

    return hheight;
}

irr::f32 LevelTerrain::GetAveragedTileHeight(int x, int z)
{
    // check bounds
    if (x < 0) x = 0;
    if (z < 0) z = 0;

    int width = this->get_width();
    int height = this->get_heigth();

    if (x > width - 1) x = width - 1;
    if (z > height - 1) z = height - 1;

    irr::f32 avgHeight = -(this->pTerrainTiles[x][z].vert1CurrPositionY + this->pTerrainTiles[x][z].vert2CurrPositionY +
            this->pTerrainTiles[x][z].vert3CurrPositionY + this->pTerrainTiles[x][z].vert4CurrPositionY) / 4.0f;

    return avgHeight;
}

//next function is used for terrain tile based collision
//detection
//x1, z1 = pnt 1 location tile, where to start
//x2, z2 = pnt 2 location tile, tile further away from player
void LevelTerrain::GetCollisionPlaneBetweenNeighboringTiles(int x1, int z1, int x2, int z2,
                       irr::core::vector3df &collPlanePos1, irr::core::vector3df &collPlanePos2,
                       irr::core::vector3df &collResolutionDirVec) {

    //what is the spartial relationsship between
    //first and second tile?
    //there are 8 possibilities
    irr::s8 deltaX = (x2 - x1);
    irr::s8 deltaZ = (z2 - z1);

    irr::core::vector3df hlp;
    irr::core::vector3df v4 = this->pTerrainTiles[x1][z1].vert4->Pos;
    v4.X = -v4.X;
    v4.Y = -v4.Y;

    irr::core::vector3df v3 = this->pTerrainTiles[x1][z1].vert3->Pos;
    v3.X = -v3.X;
    v3.Y = -v3.Y;

    irr::core::vector3df v2 = this->pTerrainTiles[x1][z1].vert2->Pos;
    v2.X = -v2.X;
    v2.Y = -v2.Y;

    irr::core::vector3df v1 = this->pTerrainTiles[x1][z1].vert1->Pos;
    v1.X = -v1.X;
    v1.Y = -v1.Y;

    switch (deltaX) {
        case -1 : {
            switch (deltaZ) {
                case -1: {
                    //corner tile left/upwards has 3 stepnessess
                    hlp = (v3 - v1) * irr::core::vector3df(0.5f, 1.0f, 0.5f);
                    collPlanePos1 = v4 - hlp;
                    collPlanePos2 = v2 - hlp;
                    hlp.normalize();
                    collResolutionDirVec = hlp;
                    break;
                 }
                case 0: {
                    //tile left of first tile, here there are only 2 stepnessess
                    collPlanePos1 = v1;
                    collPlanePos2 = v4;
                    hlp = v3 - v4;
                    hlp.normalize();
                    collResolutionDirVec = hlp;
                    break;
                }
                case +1: {
                    //corner tile left/downwards has 3 stepnessess
                    hlp = (v2 - v4) * irr::core::vector3df(0.5f, 1.0f, 0.5f);
                    collPlanePos1 = v1 - hlp;
                    collPlanePos2 = v3 - hlp;
                    hlp.normalize();
                    collResolutionDirVec = hlp;
                    break;
                }
            }

            break;
        }

        case 0 : {
            switch (deltaZ) {
                case -1: {
                    //tile upwards has 2 stepnessess
                    collPlanePos1 = v1;
                    collPlanePos2 = v2;
                    hlp = v4 - v1;
                    hlp.normalize();
                    collResolutionDirVec = hlp;

                    break;
                 }
                case 0: {
                    //neighbor and first tile are identical, return no stepness
                    break;
                }
                case +1: {
                    //tile downwards has 2 stepnessess
                    collPlanePos1 = v3;
                    collPlanePos2 = v4;
                    hlp = v1 - v4;
                    hlp.normalize();
                    collResolutionDirVec = hlp;
                    break;
                }
            }

            break;
        }

    case +1 : {
        switch (deltaZ) {
            case -1: {
                //corner tile right/upwards has 3 stepnessess
                hlp = (v4 - v2) * irr::core::vector3df(0.5f, 1.0f, 0.5f);
                collPlanePos1 = v1 - hlp;
                collPlanePos2 = v3 - hlp;
                hlp.normalize();
                collResolutionDirVec = hlp;
                break;
             }
            case 0: {
                //tile right of first tile, here there are only 2 stepnessess
                collPlanePos1 = v2;
                collPlanePos2 = v3;
                hlp = v4 - v3;
                hlp.normalize();
                collResolutionDirVec = hlp;
                break;
            }
            case +1: {
                //corner tile right/downwards has 3 stepnessess
                hlp = (v1 - v3) * irr::core::vector3df(0.5f, 1.0f, 0.5f);
                collPlanePos1 = v2 - hlp;
                collPlanePos2 = v4 - hlp;
                hlp.normalize();
                collResolutionDirVec = hlp;
                break;
            }
        }

        break;
        }
    }
}

//next function is used for terrain tile based collision
//detection
//x1, z1 = current player location tile, tile which defines in which
//direction of steepness of neighboring tile is calculated
//x2, z2 = tile location of neighboring tile for which the steepness should
//be calculated
irr::f32 LevelTerrain::GetSteepnessOfNeighboringTile(int x1, int z1, int x2, int z2,
                       irr::core::vector3df &collPlanePos1, irr::core::vector3df &collPlanePos2,
                       irr::core::vector3df &collResolutionDirVec) {
    // check bounds
    if (x1 < 0) x1 = 0;
    if (z1 < 0) z1 = 0;

    if (x2 < 0) x2 = 0;
    if (z2 < 0) z2 = 0;

    int width = this->get_width();
    int height = this->get_heigth();

    if (x1 > width - 1) x1 = width - 1;
    if (z1 > height - 1) z1 = height - 1;

    if (x2 > width - 1) x2 = width - 1;
    if (z2 > height - 1) z2 = height - 1;

    //what is the spartial relationsship between
    //first and neighboring tile?
    //there are 8 possibilities
    irr::s8 deltaX = (x2 - x1);
    irr::s8 deltaZ = (z2 - z1);

    irr::f32 stepness1 = 0.0f;
    irr::f32 stepness2 = 0.0f;
    irr::f32 stepness3 = 0.0f;

    irr::core::vector3df hlp;
    irr::core::vector3df v4 = this->pTerrainTiles[x1][z1].vert4->Pos;
    v4.X = -v4.X;
    v4.Y = -v4.Y;

    irr::core::vector3df v3 = this->pTerrainTiles[x1][z1].vert3->Pos;
    v3.X = -v3.X;
    v3.Y = -v3.Y;

    irr::core::vector3df v2 = this->pTerrainTiles[x1][z1].vert2->Pos;
    v2.X = -v2.X;
    v2.Y = -v2.Y;

    irr::core::vector3df v1 = this->pTerrainTiles[x1][z1].vert1->Pos;
    v1.X = -v1.X;
    v1.Y = -v1.Y;

    switch (deltaX) {
        case -1 : {
            switch (deltaZ) {
                case -1: {
                    //corner tile left/upwards has 3 stepnessess
                    stepness1 = (-this->pTerrainTiles[x2][z2].vert2CurrPositionY + this->pTerrainTiles[x2][z2].vert3CurrPositionY);
                    stepness2 = (-this->pTerrainTiles[x2][z2].vert1CurrPositionY + this->pTerrainTiles[x2][z2].vert3CurrPositionY);
                    stepness3 = (-this->pTerrainTiles[x2][z2].vert4CurrPositionY + this->pTerrainTiles[x2][z2].vert3CurrPositionY);       

                    hlp = (v3 - v1) * irr::core::vector3df(0.5f, 1.0f, 0.5f);
                    collPlanePos1 = v4 - hlp;
                    collPlanePos2 = v2 - hlp;
                    hlp.normalize();
                    collResolutionDirVec = hlp;
                    break;
                 }
                case 0: {
                    //tile left of first tile, here there are only 2 stepnessess
                    stepness1 = (-this->pTerrainTiles[x2][z2].vert1CurrPositionY + this->pTerrainTiles[x2][z2].vert2CurrPositionY);
                    stepness2 = (-this->pTerrainTiles[x2][z2].vert4CurrPositionY + this->pTerrainTiles[x2][z2].vert3CurrPositionY);
                    collPlanePos1 = v1;
                    collPlanePos2 = v4;
                    hlp = v3 - v4;
                    hlp.normalize();
                    collResolutionDirVec = hlp;
                    break;
                }
                case +1: {
                    //corner tile left/downwards has 3 stepnessess
                    stepness1 = (-this->pTerrainTiles[x2][z2].vert1CurrPositionY + this->pTerrainTiles[x2][z2].vert2CurrPositionY);
                    stepness2 = (-this->pTerrainTiles[x2][z2].vert4CurrPositionY + this->pTerrainTiles[x2][z2].vert2CurrPositionY);
                    stepness3 = (-this->pTerrainTiles[x2][z2].vert3CurrPositionY + this->pTerrainTiles[x2][z2].vert2CurrPositionY);
                    hlp = (v2 - v4) * irr::core::vector3df(0.5f, 1.0f, 0.5f);
                    collPlanePos1 = v1 - hlp;
                    collPlanePos2 = v3 - hlp;
                    hlp.normalize();
                    collResolutionDirVec = hlp;
                    break;
                }
            }

            break;
        }

        case 0 : {
            switch (deltaZ) {
                case -1: {
                    //tile upwards has 2 stepnessess
                    stepness1 = (-this->pTerrainTiles[x2][z2].vert1CurrPositionY + this->pTerrainTiles[x2][z2].vert4CurrPositionY);
                    stepness2 = (-this->pTerrainTiles[x2][z2].vert2CurrPositionY + this->pTerrainTiles[x2][z2].vert3CurrPositionY);
                    collPlanePos1 = v1;
                    collPlanePos2 = v2;
                    hlp = v4 - v1;
                    hlp.normalize();
                    collResolutionDirVec = hlp;

                    break;
                 }
                case 0: {
                    //neighbor and first tile are identical, return no stepness
                    break;
                }
                case +1: {
                    //tile downwards has 2 stepnessess
                    stepness1 = (-this->pTerrainTiles[x2][z2].vert4CurrPositionY + this->pTerrainTiles[x2][z2].vert1CurrPositionY);
                    stepness2 = (-this->pTerrainTiles[x2][z2].vert3CurrPositionY + this->pTerrainTiles[x2][z2].vert2CurrPositionY);
                    collPlanePos1 = v3;
                    collPlanePos2 = v4;
                    hlp = v1 - v4;
                    hlp.normalize();
                    collResolutionDirVec = hlp;
                    break;
                }
            }

            break;
        }

    case +1 : {
        switch (deltaZ) {
            case -1: {
                //corner tile right/upwards has 3 stepnessess
                stepness1 = (-this->pTerrainTiles[x2][z2].vert3CurrPositionY + this->pTerrainTiles[x2][z2].vert4CurrPositionY);
                stepness2 = (-this->pTerrainTiles[x2][z2].vert2CurrPositionY + this->pTerrainTiles[x2][z2].vert4CurrPositionY);
                stepness3 = (-this->pTerrainTiles[x2][z2].vert1CurrPositionY + this->pTerrainTiles[x2][z2].vert4CurrPositionY);
                hlp = (v4 - v2) * irr::core::vector3df(0.5f, 1.0f, 0.5f);
                collPlanePos1 = v1 - hlp;
                collPlanePos2 = v3 - hlp;
                hlp.normalize();
                collResolutionDirVec = hlp;
                break;
             }
            case 0: {
                //tile right of first tile, here there are only 2 stepnessess
                stepness1 = (-this->pTerrainTiles[x2][z2].vert2CurrPositionY + this->pTerrainTiles[x2][z2].vert1CurrPositionY);
                stepness2 = (-this->pTerrainTiles[x2][z2].vert3CurrPositionY + this->pTerrainTiles[x2][z2].vert4CurrPositionY);
                collPlanePos1 = v2;
                collPlanePos2 = v3;
                hlp = v4 - v3;
                hlp.normalize();
                collResolutionDirVec = hlp;
                break;
            }
            case +1: {
                //corner tile right/downwards has 3 stepnessess
                stepness1 = (-this->pTerrainTiles[x2][z2].vert4CurrPositionY + this->pTerrainTiles[x2][z2].vert1CurrPositionY);
                stepness2 = (-this->pTerrainTiles[x2][z2].vert3CurrPositionY + this->pTerrainTiles[x2][z2].vert1CurrPositionY);
                stepness3 = (-this->pTerrainTiles[x2][z2].vert2CurrPositionY + this->pTerrainTiles[x2][z2].vert1CurrPositionY);
                hlp = (v1 - v3) * irr::core::vector3df(0.5f, 1.0f, 0.5f);
                collPlanePos1 = v2 - hlp;
                collPlanePos2 = v4 - hlp;
                hlp.normalize();
                collResolutionDirVec = hlp;
                break;
            }
        }

        break;
        }
    }

  //derive max steepness we can find
  //and return it
  irr::f32 maxSteepness = stepness1;

  if (stepness2 > maxSteepness) {
      maxSteepness = stepness2;
  }

  if (stepness3 > maxSteepness) {
      maxSteepness = stepness3;
  }

  return maxSteepness;
}

void LevelTerrain::RecalculateNormals(MapEntry *entry)
{
    //int max = positionVboData.Length;
    //int perRow = levelRes.Width;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    // update normals
    int valZ = entry->get_Z();
    int valX = entry->get_X();

    for (int z = valZ - 1; z <= valZ + 1; z++)
    {
        for (int x = valX - 1; x <= valX + 1; x++)
        {
            if (x > Width - 1 || z > Height - 1) continue;

            //int id = 4 * (x + z * perRow);
            //if (id < 0 || id >= max) continue;

            //normalVboData[id    ] = computeNormalFromPositionsBuffer(x, z, 1.0f);
            //normalVboData[id + 1] = computeNormalFromPositionsBuffer(x + 1, z, 1.0f);
            //normalVboData[id + 2] = computeNormalFromPositionsBuffer(x + 1, z + 1, 1.0f);
            //normalVboData[id + 3] = computeNormalFromPositionsBuffer(x, z + 1, 1.0f);

            this->pTerrainTiles[x][z].vert1CurrNormal = computeNormalFromPositionsBuffer(x, z, 1.0f);
            this->pTerrainTiles[x][z].vert2CurrNormal = computeNormalFromPositionsBuffer(x + 1, z, 1.0f);
            this->pTerrainTiles[x][z].vert3CurrNormal = computeNormalFromPositionsBuffer(x + 1, z + 1, 1.0f);
            this->pTerrainTiles[x][z].vert4CurrNormal = computeNormalFromPositionsBuffer(x, z + 1, 1.0f);
        }
    }
}

//outNrVertice returns the number of the vertice
//for the tile starting with 1, vertice 1 = 1, vertice 2 = 2, ...
irr::core::vector2di LevelTerrain::GetClosestTileGridCoordToMapPosition(irr::core::vector3df mapPosition, int &outNrVertice) {
    irr::core::vector2di result;

    irr::f32 dX = (-mapPosition.X / (irr::f32)(this->segmentSize));
    irr::f32 dY = (mapPosition.Z / (irr::f32)(this->segmentSize));

    //truncate to int
    result.X = irr::s32(dX);
    result.Y = irr::s32(dY);

    if ((dX - result.X) > 0.5f) {
        //X rounded up
        if (dY - result.Y > 0.5f) {
            //Y rounded up
            outNrVertice = 3;
        } else {
            //Y rounded down
            outNrVertice = 2;
        }
    } else {
        //X rounded down
        //Y rounded up
        if (dY - result.Y > 0.5f) {
            outNrVertice = 4;
        } else {
            //Y rounded down
            outNrVertice = 1;
        }
    }

    //make sure result is within the valid range!
    //if not force it to be valid
    ForceTileGridCoordRange(result);

    return result;
}

//Helper function which makes sure that regardless of the coordinates
//we never exit the valid coordinate range
//Returns true if we were inside the terrain grid with the coordinates
//Returns false if were landed outside of the valid grid area, and the coordinates
//were adjusted
bool LevelTerrain::ForceTileGridCoordRange(irr::core::vector2di &tileGridPos) {
    bool adjust = false;

    if (tileGridPos.X < 0) {
        tileGridPos.X = 0;
        adjust = true;
    }

    if (tileGridPos.Y < 0) {
        tileGridPos.Y = 0;
        adjust = true;
    }

    if (tileGridPos.X >= this->levelRes->Width()) {
        tileGridPos.X = (this->levelRes->Width() - 1);
        adjust = true;
    }

    if (tileGridPos.Y >= this->levelRes->Height()) {
        tileGridPos.Y = (this->levelRes->Height() - 1);
        adjust = true;
    }

    return adjust;
}

vector3d<irr::f32> LevelTerrain::computeNormalFromPositionsBuffer(irr::s32 x, irr::s32 z, irr::f32 intensity)
{
    int Width = levelRes->Width();
    int Height = levelRes->Height();

    if (x < 0 || x > Width - 1 || z < 0 || z > Height - 1)
        return vector3d<irr::f32>(0.0f, 1.0f, 0.0f);

    //int perLine = 4 * levelRes.Width;
    //int id = 4 * x + z * perLine;

    //float h = positionVboData[id].Y;
    //use current vertice 1 Y position for calculation
    float h = this->pTerrainTiles[x][z].vert1CurrPositionY;

    irr::f32 a = z - 1 >= 0 ? this->pTerrainTiles[x][z-1].vert1CurrPositionY : h;
    irr::f32 b = x + 1 < Width ? this->pTerrainTiles[x+1][z].vert1CurrPositionY : h;
    irr::f32 c = z + 1 < Height ? this->pTerrainTiles[x][z+1].vert1CurrPositionY : h;
    irr::f32 d = x - 1 >= 0 ? this->pTerrainTiles[x-1][z].vert1CurrPositionY : h;

    vector3d<irr::f32> *nA = new vector3d<irr::f32>(0.0f, a - h, -intensity);
    vector3d<irr::f32> *nB = new vector3d<irr::f32>(intensity, b - h, 0.0f);
    vector3d<irr::f32> *nC = new vector3d<irr::f32>(0.0f, c - h, intensity);
    vector3d<irr::f32> *nD = new vector3d<irr::f32>(-intensity, d - h, 0.0f);

    vector3d<irr::f32> normal = 0.25f * (nA->crossProduct(*nB) + nB->crossProduct(*nC) + nC->crossProduct(*nD) + nD->crossProduct(*nA));
    normal.normalize();

    delete nA;
    delete nB;
    delete nC;
    delete nD;

    return normal;
}

//calculate the one normal that is centered at the surface of the tile, and points in a 90 degress angle outwards (surfaceNormal of tile)
//is needed for collision resolution
vector3d<irr::f32> LevelTerrain::computeTileSurfaceNormalFromPositionsBuffer(irr::s32 x, irr::s32 z) {
    int Width = levelRes->Width();
    int Height = levelRes->Height();

    if (x < 0 || x > Width - 1 || z < 0 || z > Height - 1)
        return vector3d<irr::f32>(0.0f, 1.0f, 0.0f);

    irr::core::vector3df v1 = this->pTerrainTiles[x][z].vert1->Pos;
    irr::core::vector3df v2 = this->pTerrainTiles[x][z].vert2->Pos;
    irr::core::vector3df v3 = this->pTerrainTiles[x][z].vert3->Pos;

    irr::core::plane3df plane(v1, v2, v3);
    irr::core::vector3df normal = plane.Normal;

    return normal;
}

vector3d<irr::f32> LevelTerrain::computeNormalFromMapEntries(int x, int z, float intensity) {
    MapEntry *a = GetMapEntry(x    , z - 1);
    MapEntry *b = GetMapEntry(x + 1, z);
    MapEntry *c = GetMapEntry(x    , z + 1);
    MapEntry *d = GetMapEntry(x - 1, z);

    MapEntry *n = GetMapEntry(x, z);

    vector3d<irr::f32> *nA = new vector3d<irr::f32>( 0.0f, (irr::f32(a->m_Height) - irr::f32(n->m_Height))* intensity, -intensity);
    vector3d<irr::f32> *nB = new vector3d<irr::f32>( intensity, (irr::f32(b->m_Height) - irr::f32(n->m_Height))*intensity,  0.0f);
    vector3d<irr::f32> *nC = new vector3d<irr::f32>( 0.0f, (irr::f32(c->m_Height) - irr::f32(n->m_Height)) * intensity,  intensity);
    vector3d<irr::f32> *nD = new vector3d<irr::f32>(- intensity, (irr::f32(d->m_Height) - irr::f32(n->m_Height))* intensity,  0.0f);

    vector3d<irr::f32> normal = 0.25f * (nA->crossProduct(*nB) + nB->crossProduct(*nC) + nC->crossProduct(*nD) + nD->crossProduct(*nA));
    normal.normalize();

    delete nA;
    delete nB;
    delete nC;
    delete nD;

    return normal;
}

int LevelTerrain::TerrainOptimization_compareCells(MapEntry *MiddleCell, MapEntry *Neighborcell) {
  int opti_cnt_result = 0;

  //first compare the height of the 2 cells
  if (abs(irr::f32(MiddleCell->m_Height) - irr::f32(Neighborcell->m_Height)) > 1e-12) {
     //height is not the same, we are done
     opti_cnt_result++;
  } else {
            //height is the same, now we want to check the uv information if the are identical
            if ((MiddleCell->m_TextureId) != (Neighborcell->m_TextureId) || (MiddleCell->GetTextureModification() != Neighborcell->GetTextureModification())) {
                //Texture seems to be different, we are done
                opti_cnt_result++;
            }
        }
  return(opti_cnt_result);
}

bool LevelTerrain::Terrain_Optimization_isValid_Cell_coordinate(int xcoord, int zcoord) {
    bool valid = true;

    if ((xcoord < 0) || (xcoord > (levelRes->Width()))) valid = false;
    if ((zcoord < 0) || (zcoord > (levelRes->Height()))) valid = false;

    return(valid);
}

//as there seems to be a limit for howmuch polygons we can draw with Irrlicht in the same Mesh,
//and the amount of triangles available is only enough for approx. half the possible level size, I decided
//to implement a way to remove unnecessary triangles from rendering (which are not used for the level itself);
//This should help to stay below the maximum possible triangle count and should also improve rendering performance
void LevelTerrain::FindTerrainOptimization() {
    int x, z;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    int neighb_cellx;
    int neighb_cellz;

    int amount_neighb = 3;
    int diffposx;
    int diffposz;

    int optival;
    int sumnb;

    MapEntry *nB;

    //for each cell in the overall map go through its closed neighbors (moving box of neigbors) and compare the cell height for each vertex, and the uv coordinates
    //count the number of neighbors that do not match; then at the end store the count value in the mapentry variable for each cell
    //With this information we then can later decide which part of the level is usefull and which is most likely not used and can be skipped during rendering

    //go through all cells
    for (z = 0; z < Height; z++) {
      for (x = 0; x < Width; x++) {
        //first analyze middle cell which is used as the reference
        MapEntry *cell = GetMapEntry(x, z);

        optival = 0;
        sumnb = 0;

        //now move through the elements of neighbors in the box centered around the middle cell
        for (diffposz = 0; diffposz < (amount_neighb*2); diffposz++) {
            neighb_cellz = z + diffposz - amount_neighb; //offset of coordinate
            for (diffposx = 0; diffposx < (amount_neighb*2); diffposx++) {
                neighb_cellx = x  + diffposx - amount_neighb; //offset of coordinate

                //we do not want to check middle cell itself
                if ((neighb_cellz != z) && (neighb_cellx != x)) {
                  if (Terrain_Optimization_isValid_Cell_coordinate(neighb_cellx, neighb_cellz)) {
                        //is a valid neighbor cell coordinate
                        nB = GetMapEntry(neighb_cellx, neighb_cellz);

                        sumnb++;

                        //compare middle cell with neighbor
                        optival += TerrainOptimization_compareCells(cell, nB);
                      }
                }
            }
         }

        //calculate percentage of different neighbors and remeber result for middle cell for later optimization decision
        //cell->m_optimization_cnt = (float(optival)/(float(sumnb)/100.0f));
        this->pTerrainTiles[x][z].m_optimization_cnt = (float(optival)/(float(sumnb)/100.0f));

        //important: exclude cells with TextureID = 96, as this is water; this needs to be done that we do not optimize lakes out of the map!
      //  if ((cell->m_optimization_cnt <= CELL_OPTIMIZATION_THRESHOLD) && (cell->m_TextureId != 96)) {

        //important: only optimize cells out which are normally used in the unused areas (dirt or the pink texture), to make sure that we do not optimize
        //out lakes etc...
        if ((this->pTerrainTiles[x][z].m_optimization_cnt <= CELL_OPTIMIZATION_THRESHOLD) && ((cell->m_TextureId == 0) || (cell->m_TextureId == 160)) ) {
           //cell can be optimized away
           this->pTerrainTiles[x][z].m_draw_in_mesh = false;
        } else this->pTerrainTiles[x][z].m_draw_in_mesh = true;
      }
    }
}

int16_t LevelTerrain::GetIlluminationValueVertice1(int x, int y) {
    MapEntry *a = GetMapEntry(x, y);
    MapEntry *b = GetMapEntry(x + 1, y);
    MapEntry *c = GetMapEntry(x + 1, y + 1);
    MapEntry *d = GetMapEntry(x, y +  1);

    return (a->mIllumination + b->mIllumination + c->mIllumination + d->mIllumination) / 4;
}

int16_t LevelTerrain::GetIlluminationValueVertice2(int x, int y) {
    MapEntry *a = GetMapEntry(x, y);
    MapEntry *b = GetMapEntry(x - 1, y);
    MapEntry *c = GetMapEntry(x, y + 1);
    MapEntry *d = GetMapEntry(x - 1, y +  1);

    return (a->mIllumination + b->mIllumination + c->mIllumination + d->mIllumination) / 4;
}

int16_t LevelTerrain::GetIlluminationValueVertice3(int x, int y) {
    MapEntry *a = GetMapEntry(x, y);
    MapEntry *b = GetMapEntry(x - 1, y);
    MapEntry *c = GetMapEntry(x - 1, y - 1);
    MapEntry *d = GetMapEntry(x, y -  1);

    return (a->mIllumination + b->mIllumination + c->mIllumination + d->mIllumination) / 4;
}

int16_t LevelTerrain::GetIlluminationValueVertice4(int x, int y) {
    MapEntry *a = GetMapEntry(x, y);
    MapEntry *b = GetMapEntry(x, y - 1);
    MapEntry *c = GetMapEntry(x + 1, y - 1);
    MapEntry *d = GetMapEntry(x + 1, y);

    return (a->mIllumination + b->mIllumination + c->mIllumination + d->mIllumination) / 4;
}

irr::video::SColor LevelTerrain::CalcVertexColorForIllumination(int16_t illuminationValue) {
   irr::f32 colVal;
   irr::u32 colValInt;

   //illuminationValue = 6144.0 (LEVEL_TERRAIN_ILLMINVAL) in level 1 should give SColor (255, 255, 255, 255) //brightest possible
   //illuminationValue = 12800.0 (LEVEL_TERRAIN_ILLMAXVAL) should give darkest SColor (255, LEVEL_TERRAIN_COLOR_DARKEST, LEVEL_TERRAIN_COLOR_DARKEST, LEVEL_TERRAIN_COLOR_DARKEST)
   colVal = 255.0f - mKIllumination * (irr::f32)(illuminationValue) + 147.0f;

   colValInt = (irr::u32)(colVal);
   if (colValInt < 0) {
        colVal = 0;
    }

    if (colValInt > 255) {
        colValInt = 255;
    }

    if (colValInt < mMinVertexCol) {
        mMinVertexCol = colValInt;
    }

    if (colValInt > mMaxVertexCol) {
        mMaxVertexCol = colValInt;
    }

    irr::video::SColor result(255, colValInt, colValInt, colValInt);

    return result;
}

void LevelTerrain::DisableIllumination() {
    int x, z;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    TerrainTileData* tile;

    irr::video::SColor fullColor(255, 255, 255, 255);

    for (z = 0; z < Height; z++) {
      for (x = 0; x < Width; x++) {
        tile = &pTerrainTiles[x][z];

        tile->vert1Color = fullColor;
        tile->vert2Color = fullColor;
        tile->vert3Color = fullColor;
        tile->vert4Color = fullColor;
       }
    }
}

void LevelTerrain::CalculateIllumination() {
    int x, z = 0;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    TerrainTileData* tile;

    //preproccess illumination slope value
    mKIllumination = (255.0f - LEVEL_TERRAIN_COLOR_DARKEST) / (LEVEL_TERRAIN_ILLMAXVAL - LEVEL_TERRAIN_ILLMINVAL);

    int16_t illumnVertex1;
    int16_t illumnVertex2;
    int16_t illumnVertex3;
    int16_t illumnVertex4;

    for (z = 0; z < Height; z++) {
      for (x = 0; x < Width; x++) {
        illumnVertex3 = GetIlluminationValueVertice1(x, z);
        illumnVertex4 = GetIlluminationValueVertice2(x, z);
        illumnVertex1 = GetIlluminationValueVertice3(x, z);
        illumnVertex2 = GetIlluminationValueVertice4(x, z);

        tile = &pTerrainTiles[x][z];

        tile->vert1Color = CalcVertexColorForIllumination(illumnVertex1);
        tile->vert2Color = CalcVertexColorForIllumination(illumnVertex2);
        tile->vert3Color = CalcVertexColorForIllumination(illumnVertex3);
        tile->vert4Color = CalcVertexColorForIllumination(illumnVertex4);

        //also stored the initial values for later
        //morphing
        tile->vert1ColorInitial = tile->vert1Color;
        tile->vert2ColorInitial = tile->vert2Color;
        tile->vert3ColorInitial = tile->vert3Color;
        tile->vert4ColorInitial = tile->vert4Color;
       }
    }

    std::cout << "Min VertexCol value = " << mMinVertexCol << ", Max VertexCol value = " << mMaxVertexCol << std::endl;
}

bool LevelTerrain::SetupGeometry() {
    int x, z = 0;

    float max = 0.0f;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    core::vector3df normal;

    TerrainTileData* tile;
    std::vector<vector2d<irr::f32>> newuvs;

    MapEntry *a;
    MapEntry *b;
    MapEntry *c;
    MapEntry *d;

    /*********************************************************
     * First setup vertices for all possible Terrain tiles   *
     *********************************************************/

    for (z = 0; z < Height; z++) {
      for (x = 0; x < Width; x++) {
        // 4 vertices - need separate UVs so cannot share
        a = GetMapEntry(x, z);
        b = GetMapEntry(x + 1, z);
        c = GetMapEntry(x + 1, z + 1);
        d = GetMapEntry(x, z + 1);

        tile = &pTerrainTiles[x][z];

        //create 4 irrlicht vertices for this tile, regardless if we show the tile later or not!
        tile->vert1 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, tile->vert1Color, 0.0f, 0.0f);
        tile->vert2 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, tile->vert2Color, 0.0f, 0.0f);
        tile->vert3 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, tile->vert3Color, 0.0f, 0.0f);
        tile->vert4 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, tile->vert4Color, 0.0f, 0.0f);

        tile->vert1->Pos.set(x       * segmentSize, -irr::f32(a->m_Height),  z * segmentSize);
        tile->vert1CurrPositionY = tile->vert1->Pos.Y;
        tile->vert1CurrPositionYDirty = false;

        tile->vert2->Pos.set((x + 1) * segmentSize, -irr::f32(b->m_Height), z * segmentSize);
        tile->vert2CurrPositionY = tile->vert2->Pos.Y;
        tile->vert2CurrPositionYDirty = false;

        tile->vert3->Pos.set((x + 1) * segmentSize, -irr::f32(c->m_Height), (z + 1) * segmentSize);
        tile->vert3CurrPositionY = tile->vert3->Pos.Y;
        tile->vert3CurrPositionYDirty = false;

        tile->vert4->Pos.set(x       * segmentSize, -irr::f32(d->m_Height), (z + 1) * segmentSize);
        tile->vert4CurrPositionY = tile->vert4->Pos.Y;
        tile->vert4CurrPositionYDirty = false;

        //precalculate averaged tile height, this value will be for example used later
        //for player craft calculations...
        tile->currTileHeight = GetAveragedTileHeight(x, z);

        //texture atlas 4 UVs
        newuvs = MakeUVs(a->GetTextureModification());

        tile->vert1->TCoords = newuvs[0];
        tile->vert1UVcoord = newuvs[0];

        tile->vert2->TCoords = newuvs[1];
        tile->vert2UVcoord = newuvs[1];

        tile->vert3->TCoords = newuvs[2];
        tile->vert3UVcoord = newuvs[2];

        tile->vert4->TCoords = newuvs[3];
        tile->vert4UVcoord = newuvs[3];

        tile->VertUpdatedUVScoord = false;

        // add normals
        normal = computeNormalFromMapEntries(x    , z    , 1.0f);
        tile->vert1->Normal = normal;
        tile->vert1CurrNormal = normal;

        normal = computeNormalFromMapEntries(x + 1, z    , 1.0f);
        tile->vert2->Normal = normal;
        tile->vert2CurrNormal = normal;

        normal = computeNormalFromMapEntries(x + 1, z + 1, 1.0f);
        tile->vert3->Normal = normal;
        tile->vert3CurrNormal = normal;

        normal = computeNormalFromMapEntries(x    , z + 1, 1.0f);
        tile->vert4->Normal = normal;
        tile->vert4CurrNormal = normal;

        tile->RefreshNormals = false;
      }
    }

    //now add all visible cells (only cells that were
    //not optimized away (non used parts of the level map))
    for (z = 0; z < Height; z++) {
      for (x = 0; x < Width; x++) {
          tile = &pTerrainTiles[x][z];

          if (tile->m_draw_in_mesh == true) {
              a = GetMapEntry(x, z);

              // determine max height
              max = std::max(max, a->m_Height);

              if (!tile->dynamicMesh) {
                 //is a static cell (does not morph)
                 mIrrMeshBuf->AddMeshBufferTile(mStaticMeshBufferVec, tile, a->m_TextureId, *mTerrainMeshStats);
              } else {
                 //is a dynamic cell (is able to morph)
                 mIrrMeshBuf->AddMeshBufferTile(mDynamicMeshBufferVec, tile, a->m_TextureId, *mTerrainMeshStats);
              }
        }
      }
    }

    //get number of already existing Meshbuffers for all available Texture Ids of Terrain
    std::vector<irr::u8> nrMeshBuffersPerTexId = mIrrMeshBuf->ReturnMeshBufferCntPerTextureId(mStaticMeshBufferVec);

    //if we are starting for the level editor we need to make sure that for each possible
    //texture Id existing we have enough meshbuffers available, so that in worst case if user
    //tries to set all existing tiles of a map to the same texture Id, we do not run out of
    //meshbuffer indices overall. Because Irrlicht is limited to max 65535 indices per
    //meshbuffer
    if (mLevelEditorMode) {
        irr::u8 buffersToAdd;

        int nrTextures = mIrrMeshBuf->GetNrTextures();

        //in for loop add additional "empty" meshbuffers for this worst case scenario
        //first for static mesh
        for (int i = 0; i < nrTextures; i++) {
           buffersToAdd = mLevelEditorMinNrMeshBuffersNeeded - nrMeshBuffersPerTexId.at(i);

           for (int j = 0; j < buffersToAdd; j++) {
               mIrrMeshBuf->AddAdditionalMeshBuffer(mStaticMeshBufferVec, i);
           }
        }
    }

    nrMeshBuffersPerTexId = mIrrMeshBuf->ReturnMeshBufferCntPerTextureId(mDynamicMeshBufferVec);

    if (mLevelEditorMode) {
        irr::u8 buffersToAdd;

        int nrTextures = mIrrMeshBuf->GetNrTextures();

        //in for loop add additional "empty" meshbuffers for this worst case scenario
        //first for static mesh
        for (int i = 0; i < nrTextures; i++) {
           buffersToAdd = mLevelEditorMinNrMeshBuffersNeeded - nrMeshBuffersPerTexId.at(i);

           for (int j = 0; j < buffersToAdd; j++) {
               mIrrMeshBuf->AddAdditionalMeshBuffer(mDynamicMeshBufferVec, i);
           }
        }
    }

    //create Mesh for the static Terrain

    //first make a mesh to hold the overall level terrain data
    myStaticTerrainMesh = new SMesh;
    myStaticTerrainMesh->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    //mesh for dynamic terrain
    myDynamicTerrainMesh = new SMesh;
    myDynamicTerrainMesh->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    std::vector<irr::scene::SMeshBuffer*> bufList;
    std::vector<irr::scene::SMeshBuffer*>::iterator bufIt;

    int nrTextures = mIrrMeshBuf->GetNrTextures();

    for (int currTexId = 0; currTexId < nrTextures; currTexId++) {

        bufList = mIrrMeshBuf->ReturnAllMeshBuffersForTextureId(mStaticMeshBufferVec, currTexId);

        for (bufIt = bufList.begin(); bufIt != bufList.end(); ++bufIt) {
              (*bufIt)->BoundingBox.reset(0,0,0);
              (*bufIt)->recalculateBoundingBox();

              //add SMeshbuffer to overall terrain mesh
              myStaticTerrainMesh->addMeshBuffer((*bufIt));

              myStaticTerrainMesh->recalculateBoundingBox();
        }
   }

   for (int currTexId = 0; currTexId < nrTextures; currTexId++) {

        bufList = mIrrMeshBuf->ReturnAllMeshBuffersForTextureId(mDynamicMeshBufferVec, currTexId);

        for (bufIt = bufList.begin(); bufIt != bufList.end(); ++bufIt) {
              (*bufIt)->BoundingBox.reset(0,0,0);
              (*bufIt)->recalculateBoundingBox();

              //add SMeshbuffer to overall terrain mesh
              myDynamicTerrainMesh->addMeshBuffer((*bufIt));

              myDynamicTerrainMesh->recalculateBoundingBox();
        }
   }

   //mark Terrain mesh as dirty, so that it is transfered again to graphics card
   myStaticTerrainMesh->setDirty();
   myStaticTerrainMesh->recalculateBoundingBox();

   myDynamicTerrainMesh->setDirty();
   myDynamicTerrainMesh->recalculateBoundingBox();

   Size.X = levelRes->Width() * segmentSize;
   Size.Y = max;
   Size.Z = levelRes->Height() * segmentSize;

   return true;
}

bool LevelTerrain::SetupGeometryEndOfMap() {
    int x, z = 0;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    core::vector3df normal;

    TerrainTileData* tile;
    std::vector<vector2d<irr::f32>> newuvs;

    MapEntry *a;
    MapEntry *b;
    MapEntry *c;
    MapEntry *d;

    int idxHelper;
    int xCoordHelper;
    TerrainTileData* origTile;

    /********************************************
     * Setup vertices for this special tiles    *
     ********************************************/

    for (z = 0; z < Height; z++) {
       idxHelper = 0;
       xCoordHelper = LEVELTERRAIN_WIDTH_ENDOFMAP - 1;
      for (x = (Width - LEVELTERRAIN_WIDTH_ENDOFMAP); x < Width; x++) {
        // 4 vertices - need separate UVs so cannot share
        a = GetMapEntry(x , z);
        b = GetMapEntry(x + 1, z);
        c = GetMapEntry(x + 1, z + 1);
        d = GetMapEntry(x , z + 1);

        tile = &pTerrainTilesEndOfMap[idxHelper][z];
        origTile = &pTerrainTiles[x][z];

        //Set the same vertex color data as the original tile
        tile->vert1Color = origTile->vert1Color;
        tile->vert2Color = origTile->vert2Color;
        tile->vert3Color = origTile->vert3Color;
        tile->vert4Color = origTile->vert4Color;

        tile->vert1ColorInitial = origTile->vert1ColorInitial;
        tile->vert2ColorInitial = origTile->vert2ColorInitial;
        tile->vert3ColorInitial = origTile->vert3ColorInitial;
        tile->vert4ColorInitial = origTile->vert4ColorInitial;

        //create 4 irrlicht vertices for this tile, regardless if we show the tile later or not!
        tile->vert1 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, tile->vert1Color, 0.0f, 0.0f);
        tile->vert2 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, tile->vert2Color, 0.0f, 0.0f);
        tile->vert3 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, tile->vert3Color, 0.0f, 0.0f);
        tile->vert4 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, tile->vert4Color, 0.0f, 0.0f);

        tile->vert1->Pos.set(- (xCoordHelper + 1)      * segmentSize, -irr::f32(a->m_Height), z * segmentSize);
        tile->vert1CurrPositionY = tile->vert1->Pos.Y;
        tile->vert1CurrPositionYDirty = false;

        tile->vert2->Pos.set(- xCoordHelper * segmentSize, -irr::f32(b->m_Height), z * segmentSize);
        tile->vert2CurrPositionY = tile->vert2->Pos.Y;
        tile->vert2CurrPositionYDirty = false;

        tile->vert3->Pos.set(- xCoordHelper * segmentSize, -irr::f32(c->m_Height), (z + 1) * segmentSize);
        tile->vert3CurrPositionY = tile->vert3->Pos.Y;
        tile->vert3CurrPositionYDirty = false;

        tile->vert4->Pos.set(- (xCoordHelper + 1)       * segmentSize, -irr::f32(d->m_Height), (z + 1) * segmentSize);
        tile->vert4CurrPositionY = tile->vert4->Pos.Y;
        tile->vert4CurrPositionYDirty = false;

        //precalculate averaged tile height, this value will be for example used later
        //for player craft calculations...
        tile->currTileHeight = GetAveragedTileHeight(x, z);

        //texture atlas 4 UVs
        newuvs = MakeUVs(a->GetTextureModification());

        tile->vert1->TCoords = newuvs[0];
        tile->vert1UVcoord = newuvs[0];

        tile->vert2->TCoords = newuvs[1];
        tile->vert2UVcoord = newuvs[1];

        tile->vert3->TCoords = newuvs[2];
        tile->vert3UVcoord = newuvs[2];

        tile->vert4->TCoords = newuvs[3];
        tile->vert4UVcoord = newuvs[3];

        tile->VertUpdatedUVScoord = false;

        // add normals
        normal = computeNormalFromMapEntries(x    , z    , 1.0f);
        tile->vert1->Normal = normal;
        tile->vert1CurrNormal = normal;

        normal = computeNormalFromMapEntries(x + 1, z    , 1.0f);
        tile->vert2->Normal = normal;
        tile->vert2CurrNormal = normal;

        normal = computeNormalFromMapEntries(x + 1, z + 1, 1.0f);
        tile->vert3->Normal = normal;
        tile->vert3CurrNormal = normal;

        normal = computeNormalFromMapEntries(x    , z + 1, 1.0f);
        tile->vert4->Normal = normal;
        tile->vert4CurrNormal = normal;

        tile->RefreshNormals = false;

        idxHelper++;
        xCoordHelper--;
      }
    }

    //now add all Terrain cells
    for (z = 0; z < Height; z++) {
        idxHelper = 0;
        for (x = (Width - LEVELTERRAIN_WIDTH_ENDOFMAP); x < Width; x++) {
          a = GetMapEntry(x , z);

          tile = &pTerrainTilesEndOfMap[idxHelper][z];
          idxHelper++;

          mIrrMeshBuf->AddMeshBufferTile(mStaticMeshBufferEndOfMapVec, tile, a->m_TextureId, *mTerrainMeshStats);
      }
    }

    //create Mesh for the static Terrain for X < 0 coordinates

    //first make the Mesh
    myStaticTerrainMeshEndOfMap = new SMesh;
    myStaticTerrainMeshEndOfMap->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    std::vector<irr::scene::SMeshBuffer*> bufList;
    std::vector<irr::scene::SMeshBuffer*>::iterator bufIt;

    int nrTextures = mIrrMeshBuf->GetNrTextures();

    for (int currTexId = 0; currTexId < nrTextures; currTexId++) {

        bufList = mIrrMeshBuf->ReturnAllMeshBuffersForTextureId(mStaticMeshBufferEndOfMapVec, currTexId);

        for (bufIt = bufList.begin(); bufIt != bufList.end(); ++bufIt) {
              (*bufIt)->BoundingBox.reset(0,0,0);
              (*bufIt)->recalculateBoundingBox();

              //add SMeshbuffer to overall terrain mesh
              myStaticTerrainMeshEndOfMap->addMeshBuffer((*bufIt));

              myStaticTerrainMeshEndOfMap->recalculateBoundingBox();
        }
   }

   //mark Terrain mesh as dirty, so that it is transfered again to graphics card
   myStaticTerrainMeshEndOfMap->setDirty();
   myStaticTerrainMeshEndOfMap->recalculateBoundingBox();

   return true;
}

void LevelTerrain::UpdateCellMeshVertex1(int x, int y) {
    if (!this->pTerrainTiles[x][y].vert1CurrPositionYDirty)
        return;

    std::vector<irr::scene::SMeshBuffer*>::iterator it2;
    S3DVertex *pntrVertices;
    irr::u32 idxMeshBuf;

    idxMeshBuf = 0;
    for (it2 = this->pTerrainTiles[x][y].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][y].myMeshBuffers.end(); ++(it2)) {
         (*it2)->grab();
         void* pntrVert = (*it2)->getVertices();
         pntrVertices = (S3DVertex*)pntrVert;
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf]].Pos.Y = this->pTerrainTiles[x][y].vert1CurrPositionY;
         if (this->pTerrainTiles[x][y].VertUpdatedUVScoord) {
            pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf]].TCoords = this->pTerrainTiles[x][y].vert1UVcoord;
            pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf]].Color = this->pTerrainTiles[x][y].vert1Color;
         }
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf]].Normal = this->pTerrainTiles[x][y].vert1CurrNormal;

         idxMeshBuf++;

         (*it2)->drop();
     }

    this->pTerrainTiles[x][y].vert1CurrPositionYDirty = false;
}

void LevelTerrain::UpdateCellMeshVertex2(int x, int y) {
    if (!this->pTerrainTiles[x][y].vert2CurrPositionYDirty)
        return;

    std::vector<irr::scene::SMeshBuffer*>::iterator it2;
    S3DVertex *pntrVertices;
    irr::u32 idxMeshBuf;

    idxMeshBuf = 0;

    for (it2 = this->pTerrainTiles[x][y].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][y].myMeshBuffers.end(); ++(it2)) {
         (*it2)->grab();
         void* pntrVert = (*it2)->getVertices();
         pntrVertices = (S3DVertex*)pntrVert;

         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 1].Pos.Y = this->pTerrainTiles[x][y].vert2CurrPositionY;
         if (this->pTerrainTiles[x][y].VertUpdatedUVScoord) {
            pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 1].TCoords = this->pTerrainTiles[x][y].vert2UVcoord;
            pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 1].Color = this->pTerrainTiles[x][y].vert2Color;
         }
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 1].Normal = this->pTerrainTiles[x][y].vert2CurrNormal;

         idxMeshBuf++;

         (*it2)->drop();
     }

      this->pTerrainTiles[x][y].vert2CurrPositionYDirty = false;
}

void LevelTerrain::UpdateCellMeshVertex3(int x, int y) {
    if (!this->pTerrainTiles[x][y].vert3CurrPositionYDirty)
        return;

    std::vector<irr::scene::SMeshBuffer*>::iterator it2;
    S3DVertex *pntrVertices;
    irr::u32 idxMeshBuf;

    idxMeshBuf = 0;

    for (it2 = this->pTerrainTiles[x][y].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][y].myMeshBuffers.end(); ++(it2)) {
         (*it2)->grab();
         void* pntrVert = (*it2)->getVertices();
         pntrVertices = (S3DVertex*)pntrVert;

         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 2].Pos.Y = this->pTerrainTiles[x][y].vert3CurrPositionY;
         if (this->pTerrainTiles[x][y].VertUpdatedUVScoord) {
            pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 2].TCoords = this->pTerrainTiles[x][y].vert3UVcoord;
            pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 2].Color = this->pTerrainTiles[x][y].vert3Color;
         }
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 2].Normal = this->pTerrainTiles[x][y].vert3CurrNormal;

         idxMeshBuf++;

         (*it2)->drop();
     }

     this->pTerrainTiles[x][y].vert3CurrPositionYDirty = false;
}

void LevelTerrain::UpdateCellMeshVertex4(int x, int y) {
    if (!this->pTerrainTiles[x][y].vert4CurrPositionYDirty)
        return;

    std::vector<irr::scene::SMeshBuffer*>::iterator it2;
    S3DVertex *pntrVertices;
    irr::u32 idxMeshBuf;

    idxMeshBuf = 0;

    for (it2 = this->pTerrainTiles[x][y].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][y].myMeshBuffers.end(); ++(it2)) {
         (*it2)->grab();
         void* pntrVert = (*it2)->getVertices();
         pntrVertices = (S3DVertex*)pntrVert;

         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 3].Pos.Y = this->pTerrainTiles[x][y].vert4CurrPositionY;
         if (this->pTerrainTiles[x][y].VertUpdatedUVScoord) {
            pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 3].TCoords = this->pTerrainTiles[x][y].vert4UVcoord;
            pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 3].Color = this->pTerrainTiles[x][y].vert4Color;
         }
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 3].Normal = this->pTerrainTiles[x][y].vert4CurrNormal;

         idxMeshBuf++;

         (*it2)->drop();
     }

    this->pTerrainTiles[x][y].vert4CurrPositionYDirty = false;
}

void LevelTerrain::SetFog(bool enabled) {
      StaticTerrainSceneNode->setMaterialFlag(EMF_FOG_ENABLE, enabled);
      DynamicTerrainSceneNode->setMaterialFlag(EMF_FOG_ENABLE, enabled);
}

void LevelTerrain::ApplyMorph(Morph& morph)
      {
          if (morph.getProgress() == morph.LastProgress)
              return;

          bool dirtyPos;

          irr::core::vector2di cellSrc = morph.Source->getCell();
          irr::core::vector2di cellTarget = morph.Target->getCell();

          //the "-1" are necessary, because otherwise the morph
          //happens slightly at the "wrong" location at the map
          int xSource = cellSrc.X - 1;
          int zSource = cellSrc.Y - 1;
          int xTarget = cellTarget.X - 1;
          int zTarget = cellTarget.Y - 1;

          // check if UVs or texture Ids need an update
          bool sourceEnabled = morph.getProgress() > 0.01f;
          bool updateUVs = (morph.UVSFromSource != sourceEnabled);
          morph.UVSFromSource = sourceEnabled;

          std::vector<vector2d<irr::f32>> uvs;
          bool updatedUVS = false;

          dirtyPos = false;

          int xIdxSrc;
          int zIdxSrc;

          int xIdxTarget;
          int zIdxTarget;

          bool illumnFromSource;

          // apply morph to positions and texture UVs
          for (int dz = 0; dz <= morph.Height; dz++)
          {
              for (int dx = 0; dx <= morph.Width; dx++)
              {
                  // source entries
                  xIdxSrc = xSource + dx;
                  zIdxSrc = zSource + dz;

                  MapEntry* a = GetMapEntry(xIdxSrc, zIdxSrc);
                  MapEntry* b = GetMapEntry(xIdxSrc + 1, zIdxSrc);
                  MapEntry* c = GetMapEntry(xIdxSrc + 1, zIdxSrc + 1);
                  MapEntry* d = GetMapEntry(xIdxSrc, zIdxSrc + 1);

                  //activate refreshNormals for entry a
                  this->pTerrainTiles[xIdxSrc][zIdxSrc].RefreshNormals = true;

                  // target entries
                  xIdxTarget = xTarget + dx;
                  zIdxTarget = zTarget + dz;

                  MapEntry* e = GetMapEntry(xIdxTarget, zIdxTarget);
                  MapEntry* f = GetMapEntry(xIdxTarget + 1, zIdxTarget);
                  MapEntry* g = GetMapEntry(xIdxTarget + 1, zIdxTarget + 1);
                  MapEntry* h = GetMapEntry(xIdxTarget, zIdxTarget + 1);

                  //activate refreshNormals for entry e
                  this->pTerrainTiles[xIdxTarget][zIdxTarget].RefreshNormals = true;

                  // set UVs either from source or from target
                  if (updateUVs && (dx > 0) && (dz > 0))
                  {
                      //create updated texture coordinates
                      if (a->GetTextureModification() != e->GetTextureModification()) {
                               uvs = sourceEnabled && !morph.Permanent ?
                                  MakeUVs(a->GetTextureModification()) :
                                  MakeUVs(e->GetTextureModification());

                               updatedUVS = true;

                               //store precalculated results
                               this->pTerrainTiles[xIdxTarget][zIdxTarget].vert1UVcoord = uvs[0];
                               this->pTerrainTiles[xIdxTarget][zIdxTarget].vert2UVcoord = uvs[1];
                               this->pTerrainTiles[xIdxTarget][zIdxTarget].vert3UVcoord = uvs[2];
                               this->pTerrainTiles[xIdxTarget][zIdxTarget].vert4UVcoord = uvs[3];
                      }

                       //we also need to update the texture Ids here; The target cell gets the
                       //texture Ids from the source cell here!
                       //only update the Mesh here!
                       if (a->m_TextureId != e->m_TextureId) {
                           (sourceEnabled && !morph.Permanent) ? this->SetCellTexture(xIdxTarget, zIdxTarget, a->m_TextureId, true, true) :
                                           this->SetCellTexture(xIdxTarget, zIdxTarget, e->m_TextureId, true, true);

                            updatedUVS = true;
                       }

                       this->pTerrainTiles[xIdxTarget][zIdxTarget].VertUpdatedUVScoord = true;

                       //update illumination
                       illumnFromSource = sourceEnabled && !morph.Permanent;

                       if (!mIlluminationEnabled) {
                           irr::video::SColor fullColor(255, 255, 255, 255);

                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert1Color = fullColor;
                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert2Color = fullColor;
                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert3Color = fullColor;
                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert4Color = fullColor;
                       } else if (illumnFromSource) {
                           //store precalculated results
                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert1Color =
                                   this->pTerrainTiles[xIdxSrc][zIdxSrc].vert1ColorInitial;

                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert2Color =
                                   this->pTerrainTiles[xIdxSrc][zIdxSrc].vert2ColorInitial;

                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert3Color =
                                   this->pTerrainTiles[xIdxSrc][zIdxSrc].vert3ColorInitial;

                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert4Color =
                                   this->pTerrainTiles[xIdxSrc][zIdxSrc].vert4ColorInitial;
                       } else {
                           //store precalculated results
                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert1Color =
                                   this->pTerrainTiles[xIdxTarget][zIdxTarget].vert1ColorInitial;

                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert2Color =
                                   this->pTerrainTiles[xIdxTarget][zIdxTarget].vert2ColorInitial;

                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert3Color =
                                   this->pTerrainTiles[xIdxTarget][zIdxTarget].vert3ColorInitial;

                           this->pTerrainTiles[xIdxTarget][zIdxTarget].vert4Color =
                                   this->pTerrainTiles[xIdxTarget][zIdxTarget].vert4ColorInitial;
                       }

                       //update illumination of possible existing column at this cell
                       CheckAndUpdateVertexColorExistingColumn(xIdxTarget, zIdxTarget);
                  }

                  // calculate and set new Y values
                  if (dx > 0 && dz > 0) {
                     this->pTerrainTiles[xIdxTarget][zIdxTarget].vert1CurrPositionY =
                              -(e->m_Height * (1.0f - morph.getProgress()) + a->m_Height * morph.getProgress());
                     this->pTerrainTiles[xIdxTarget][zIdxTarget].vert1CurrPositionYDirty = true;
                  }

                  if ((dx < morph.Width) && (dz > 0)) {
                     this->pTerrainTiles[xIdxTarget][zIdxTarget].vert2CurrPositionY =
                              -(f->m_Height * (1.0f - morph.getProgress()) + b->m_Height * morph.getProgress());
                     this->pTerrainTiles[xIdxTarget][zIdxTarget].vert2CurrPositionYDirty = true;
                   }

                  if ((dx < morph.Width) && (dz < morph.Height)) {
                     this->pTerrainTiles[xIdxTarget][zIdxTarget].vert3CurrPositionY =
                              -(g->m_Height * (1.0f - morph.getProgress()) + c->m_Height * morph.getProgress());
                     this->pTerrainTiles[xIdxTarget][zIdxTarget].vert3CurrPositionYDirty = true;
                  }

                 if ((dx > 0) && (dz < morph.Height)) {
                     this->pTerrainTiles[xIdxTarget][zIdxTarget].vert4CurrPositionY =
                             -(h->m_Height * (1.0f - morph.getProgress()) + d->m_Height * morph.getProgress());
                     this->pTerrainTiles[xIdxTarget][zIdxTarget].vert4CurrPositionYDirty = true;
                 }

                 //recalculate averaged tile height, this value will be for example used later
                 //for player craft calculations...
                 this->pTerrainTiles[xIdxTarget][zIdxTarget].currTileHeight = GetAveragedTileHeight(xIdxTarget, zIdxTarget);
              }
          }

          // recalculate all necessary vertice normals
          for (int dz = 0; dz < morph.Height + 1; dz++)
          {
              for (int dx = 0; dx < morph.Width + 1; dx++)
              {
                  // source entries
                  int x = xSource + dx;
                  int z = zSource + dz;

                  MapEntry* a = GetMapEntry(x, z);

                  if (this->pTerrainTiles[x][z].RefreshNormals) {
                      RecalculateNormals(a);
                      this->pTerrainTiles[x][z].RefreshNormals = false;
                  }

                  // target entries
                  x = xTarget + dx;
                  z = zTarget + dz;

                  MapEntry* e = GetMapEntry(x, z);

                  if (this->pTerrainTiles[x][z].RefreshNormals) {
                      RecalculateNormals(e);
                      this->pTerrainTiles[x][z].RefreshNormals = false;
                  }
              }

          }

          // now update all vertices in SMeshBuffers
          for (int dz = 0; dz < morph.Height + 1; dz++)
          {
              for (int dx = 0; dx < morph.Width + 1; dx++)
              {
                  // target entries
                  int x = xTarget + dx;
                  int z = zTarget + dz;

                  // calculate and set new Y values
                  if (dx > 0 && dz > 0) {
                      //update the terrain cell Mesh vertices
                      UpdateCellMeshVertex1(x, z);

                      dirtyPos = true;
                  }

                  if (dx < morph.Width && dz > 0) {
                      //update the terrain cell Mesh vertices
                      UpdateCellMeshVertex2(x, z);

                      dirtyPos = true;
                  }

                  if (dx < morph.Width && dz < morph.Height) {
                      //update the terrain cell Mesh vertices
                      UpdateCellMeshVertex3(x, z);

                      dirtyPos = true;
                  }

                  if (dx > 0 && dz < morph.Height) {
                      //update the terrain cell Mesh vertices
                      UpdateCellMeshVertex4(x, z);

                      dirtyPos = true;
                  }

                  //if we have updated the UV coordinates, reset
                  //flag that we need to still update them
                  if (this->pTerrainTiles[x][z].VertUpdatedUVScoord) {
                      this->pTerrainTiles[x][z].VertUpdatedUVScoord = false;
                  }
              }
          }

          if (dirtyPos) {
                mNeedMeshUpdate = LEVELTERRAIN_MESH_VERTEXUPDATENEEDED;
          }

          if (updatedUVS) {
              mNeedMeshUpdate = LEVELTERRAIN_MESH_VERTEXANDINDEXUPDATENEEDED;
          }
}

void LevelTerrain::CheckForMeshUpdate() {
    if (mNeedMeshUpdate == LEVELTERRAIN_MESH_VERTEXUPDATENEEDED) {
        myDynamicTerrainMesh->setDirty(EBT_VERTEX);
        if (mLevelEditorMode) {
            myStaticTerrainMesh->setDirty(EBT_VERTEX);
        }
        mNeedMeshUpdate = LEVELTERRAIN_MESH_NOUPDATENEEDED;
    } else if (mNeedMeshUpdate == LEVELTERRAIN_MESH_VERTEXANDINDEXUPDATENEEDED) {
        myDynamicTerrainMesh->setDirty(EBT_VERTEX_AND_INDEX);
        if (mLevelEditorMode) {
            myStaticTerrainMesh->setDirty(EBT_VERTEX_AND_INDEX);
        }
        mNeedMeshUpdate = LEVELTERRAIN_MESH_NOUPDATENEEDED;
    }
}

void LevelTerrain::UpdateTileVerticeColors(int x, int y, bool skipMeshUpdate) {
    std::vector<irr::scene::SMeshBuffer*>::iterator it2;
    S3DVertex *pntrVertices;
    irr::u32 idxMeshBuf;

    idxMeshBuf = 0;
    for (it2 = this->pTerrainTiles[x][y].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][y].myMeshBuffers.end(); ++(it2)) {
         (*it2)->grab();
         void* pntrVert = (*it2)->getVertices();
         pntrVertices = (S3DVertex*)pntrVert;
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf]    ].Color = this->pTerrainTiles[x][y].vert1Color;
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 1].Color = this->pTerrainTiles[x][y].vert2Color;
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 2].Color = this->pTerrainTiles[x][y].vert3Color;
         pntrVertices[this->pTerrainTiles[x][y].myMeshBufVertexId1[idxMeshBuf] + 3].Color = this->pTerrainTiles[x][y].vert4Color;

         idxMeshBuf++;

         (*it2)->drop();
     }

    if (!skipMeshUpdate) {
        myStaticTerrainMesh->setDirty(EBT_VERTEX);
        myDynamicTerrainMesh->setDirty(EBT_VERTEX);
    } else {
        mNeedMeshUpdate = LEVELTERRAIN_MESH_VERTEXUPDATENEEDED;
    }
}

irr::f32 LevelTerrain::GetCurrentTerrainHeightForWorldCoordinate(irr::f32 x, irr::f32 z, vector2di &outCellCoord) {
    /*if (this->mRace != nullptr) {
        if (this->mRace->DebugHitBreakpoint) {
            this->mRace->DebugHitBreakpoint = false;
        }
    }*/

    //what cell are we in?
    vector2di cell((irr::s32)( - x / this->segmentSize), (irr::s32)(z / this->segmentSize));

    ForceTileGridCoordRange(cell);

    outCellCoord = cell;

    //get pntr to this tile
    TerrainTileData *pntr = &pTerrainTiles[cell.X][cell.Y];

    irr::f32 yRes;

    if (pntr != nullptr) {
        irr::f32 slopeX = -pntr->vert2CurrPositionY + pntr->vert1CurrPositionY;
        irr::f32 slopeZ = -pntr->vert4CurrPositionY + pntr->vert1CurrPositionY;

        vector2df modulus(-x - irr::f32(cell.X), z - irr::f32(cell.Y));

        yRes = -pntr->vert1CurrPositionY + slopeX * modulus.X + slopeZ * modulus.Y;
    } else {
        yRes = 0.0f;
    }

    return yRes;
}

irr::u16 LevelTerrain::get_width() {
    return(this->levelRes->Width());
}

irr::u16 LevelTerrain::get_heigth()  {
    return(this->levelRes->Height());
}

void LevelTerrain::DrawTerrainGrid(int gridMidPointX, int gridMidPointY, int gridSize, ColorStruct* color) {
    int width = this->levelRes->Width();
    int height = this->levelRes->Height();

    irr::core::vector2di selCell;

    int startX = gridMidPointX - gridSize;
    int endX = gridMidPointX + gridSize;

    if (endX > width) {
        endX = width;
    }

    if (startX < 0) {
        startX = 0;
    }

    int startY = gridMidPointY - gridSize;
    int endY = gridMidPointY + gridSize;

    if (endY > height) {
        endY = height;
    }

    if (startY < 0) {
        startY = 0;
    }

    for (int x = startX; x < endX; x++) {
       for (int y = startY; y < endY; y++) {
          selCell.X = x;
          selCell.Y = y;
          DrawOutlineSelectedCell(selCell, color);
        }
    }
}

irr::u8 LevelTerrain::GetCurrentViewMode() {
    return mCurrentViewMode;
}

void LevelTerrain::SetViewMode(irr::u8 newViewMode) {
    mCurrentViewMode = newViewMode;

    switch (newViewMode) {
       case LEVELTERRAIN_VIEW_OFF: {
             StaticTerrainSceneNode->setVisible(false);
             DynamicTerrainSceneNode->setVisible(false);

             StaticTerrainSceneNode->setDebugDataVisible(EDS_OFF);
             DynamicTerrainSceneNode->setDebugDataVisible(EDS_OFF);
             break;
       }

        case LEVELTERRAIN_VIEW_WIREFRAME: {
            //change to wireframe view
            StaticTerrainSceneNode->setVisible(true);
            DynamicTerrainSceneNode->setVisible(true);

            StaticTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, true);
            DynamicTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, true);

            StaticTerrainSceneNode->setDebugDataVisible(EDS_OFF);
            DynamicTerrainSceneNode->setDebugDataVisible(EDS_OFF);
            break;
        }

        case LEVELTERRAIN_VIEW_DEFAULT: {
            //change to default mode (textured)
            StaticTerrainSceneNode->setVisible(true);
            DynamicTerrainSceneNode->setVisible(true);

            StaticTerrainSceneNode->setDebugDataVisible(EDS_OFF);
            DynamicTerrainSceneNode->setDebugDataVisible(EDS_OFF);

            StaticTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
            DynamicTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
            break;
        }

        case LEVELTERRAIN_VIEW_DEBUGNORMALS: {
           //change to normals debug mode (adding also Terrain vertices normals debug view)
           StaticTerrainSceneNode->setVisible(true);
           DynamicTerrainSceneNode->setVisible(true);

           StaticTerrainSceneNode->setDebugDataVisible(EDS_FULL);
           DynamicTerrainSceneNode->setDebugDataVisible(EDS_FULL);

           StaticTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
           DynamicTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
           break;
        }
    }
}

void LevelTerrain::CheckAndUpdateVertexColorExistingColumn(int xTile, int yTile) {
    //is there a column at cell coordinates x/y? if not return
    if (!levelRes->IsAColumnAtCoordinates(xTile, yTile))
        return;

    //there is a column => get pointer to column

    //there is a "position" key we can use for search
    int posKey =  xTile + yTile * levelRes->Width();
    Column* columnPntr;

    if (mLevelBlocks->SearchColumnWithPosition(posKey, columnPntr)) {
        //we found a column, modify its vertex colors
        columnPntr->UpdateIllumination();
    }
}

irr::core::vector3df LevelTerrain::GetRegionMiddleWorldCoordinate(MapTileRegionStruct* region) {
    irr::core::vector3df pos(0.0f, 0.0f, 0.0f);

    if (region == nullptr)
        return pos;

    irr::core::vector2df midCell = region->regionCenterTileCoord;

    pos.X = -midCell.X * DEF_SEGMENTSIZE - 0.5f;
    pos.Z = midCell.Y * DEF_SEGMENTSIZE + 0.5f;

    irr::core::vector2di outCell;

    pos.Y = GetCurrentTerrainHeightForWorldCoordinate(pos.X, pos.Z, outCell);

    return pos;
}

/***********************************************************************
 * Functions mainly used by the LevelEditor and not the game itself    *
 ***********************************************************************/

//Sets the texture Id for every tile of the complete terrain
//to the specified value
void LevelTerrain::ResetTextureCompleteTerrain(int newTexId) {
    int width = get_width();
    int height = get_heigth();

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            SetCellTexture(x, y, newTexId, false);
        }
    }
}

void LevelTerrain::DrawOutlineSelectedCell(irr::core::vector2di selCellCoordinate, ColorStruct* color) {
    irr::core::vector3df pos1 = pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert1->Pos;
    pos1.X = -pos1.X;
    pos1.Y = -pos1.Y + 0.05f; //draw a little bit above the terrain cells, so that we do not get "flicker" effect

    irr::core::vector3df pos2 = pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert2->Pos;
    pos2.X = -pos2.X;
    pos2.Y = -pos2.Y + 0.05f;

    irr::core::vector3df pos3 = pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert3->Pos;
    pos3.X = -pos3.X;
    pos3.Y = -pos3.Y + 0.05f;

    irr::core::vector3df pos4 = pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert4->Pos;
    pos4.X = -pos4.X;
    pos4.Y = -pos4.Y + 0.05f;

    this->mInfra->mDrawDebug->Draw3DRectangle(pos1, pos2, pos3, pos4, color);
}

void LevelTerrain::CheckAndUpdateHeightExistingColumn(int x, int y, int whichVertex, irr::f32 newHeightValue) {
    //is there a column at cell coordinates x/y? if not return
    if (!levelRes->IsAColumnAtCoordinates(x, y))
        return;

    //there is a column => get pointer to column

    //there is a "position" key we can use for search
    int posKey =  x + y * levelRes->Width();
    Column* columnPntr;

    irr::f32 currV1height;
    irr::f32 currV2height;
    irr::f32 currV3height;
    irr::f32 currV4height;

    if (mLevelBlocks->SearchColumnWithPosition(posKey, columnPntr)) {
        //we found a column, modify its height if necessary
        //get current column base heights for all 4 corners
        currV1height = columnPntr->mBaseVert1Coord.Y;
        currV2height = columnPntr->mBaseVert2Coord.Y;
        currV3height = columnPntr->mBaseVert3Coord.Y;
        currV4height = columnPntr->mBaseVert4Coord.Y;

        //modify the value for the changed corner
        //Important note 09.08.2025: it is not nice, but
        //the vertex number order is different between the cell
        //corner vertices, and the column vertices
        //so we have to map it here properly!
        //The case values below are therefore no mistake!
        switch (whichVertex) {
            case 1: {
                 currV4height = newHeightValue;
                 break;
            }
            case 2: {
                 currV3height = newHeightValue;
                 break;
            }
            case 3: {
                 currV2height = newHeightValue;
                 break;
            }
            case 4: {
                 currV1height = newHeightValue;
                 break;
            }
            default: {
                return;
            }
        }

        columnPntr->AdjustMeshBaseVerticeHeight(currV1height, currV2height, currV3height, currV4height);
    }
}

void LevelTerrain::SetNewCellVertexHeight(int x, int y, int whichVertex, irr::f32 newHeightValue) {
    //This higher level function has to do 2 independent things:
    // 1, Modify the terrain cell vertex height in the low level map data itself
    //    So that next time the map is loaded this changes becomes permanent
    // 2, Modify the Irrlicht Terrain Mesh vertices height so that the change
    //    becomes immediately visible in the level editor

    MapEntry* a = GetMapEntry(x, y);
    MapEntry* b = GetMapEntry(x + 1, y);
    MapEntry* c = GetMapEntry(x + 1, y + 1);
    MapEntry* d = GetMapEntry(x, y + 1);

    irr::core::vector2di cell(x,y);

    switch (whichVertex) {
        case 1: {
           //set new Y value in low level map
           a->m_Height = -newHeightValue;

           //set new Y values in Irrlicht Mesh
           this->pTerrainTiles[x][y].vert1CurrPositionY = newHeightValue;
           this->pTerrainTiles[x][y].vert1->Pos.Y = newHeightValue;
           this->pTerrainTiles[x][y].vert1CurrPositionYDirty = true;

           CheckAndUpdateHeightExistingColumn(x, y, 1, -newHeightValue);

           cell.X = x - 1;
           cell.Y = y;

           //Returns true if we were inside the terrain grid with the coordinates
           //Returns false if were landed outside of the valid grid area, and the coordinates
           //were adjusted
           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert2CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert2->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert2CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 2, -newHeightValue);
           }

           cell.X = x - 1;
           cell.Y = y - 1;

           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert3CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert3->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert3CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 3, -newHeightValue);
           }

           cell.X = x;
           cell.Y = y - 1;

           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert4CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert4->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert4CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 4, -newHeightValue);
           }

           break;
        }

        case 2: {
           //set new Y value in low level map
           b->m_Height = -newHeightValue;

           //set new Y values in Irrlicht Mesh
           this->pTerrainTiles[x][y].vert2CurrPositionY = newHeightValue;
           this->pTerrainTiles[x][y].vert2->Pos.Y = newHeightValue;
           this->pTerrainTiles[x][y].vert2CurrPositionYDirty = true;

           CheckAndUpdateHeightExistingColumn(x, y, 2, -newHeightValue);

           cell.X = x + 1;
           cell.Y = y;

           //Returns true if we were inside the terrain grid with the coordinates
           //Returns false if were landed outside of the valid grid area, and the coordinates
           //were adjusted
           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert1CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert1->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert1CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 1, -newHeightValue);
           }

           cell.X = x;
           cell.Y = y - 1;

           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert3CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert3->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert3CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 3, -newHeightValue);
           }

           cell.X = x + 1;
           cell.Y = y - 1;

           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert4CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert4->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert4CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 4, -newHeightValue);
           }

           break;
        }

        case 3: {
           //set new Y value in low level map
           c->m_Height = -newHeightValue;

           //set new Y values in Irrlicht Mesh
           this->pTerrainTiles[x][y].vert3CurrPositionY = newHeightValue;
           this->pTerrainTiles[x][y].vert3->Pos.Y = newHeightValue;
           this->pTerrainTiles[x][y].vert3CurrPositionYDirty = true;

           CheckAndUpdateHeightExistingColumn(x, y, 3, -newHeightValue);

           cell.X = x + 1;
           cell.Y = y;

           //Returns true if we were inside the terrain grid with the coordinates
           //Returns false if were landed outside of the valid grid area, and the coordinates
           //were adjusted
           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert4CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert4->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert4CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 4, -newHeightValue);
           }

           cell.X = x + 1;
           cell.Y = y + 1;

           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert1CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert1->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert1CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 1, -newHeightValue);
           }

           cell.X = x;
           cell.Y = y + 1;

           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert2CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert2->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert2CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 2, -newHeightValue);
           }

           break;
        }

        case 4: {
           //set new Y value in low level map
           d->m_Height = -newHeightValue;

           //set new Y values in Irrlicht Mesh
           this->pTerrainTiles[x][y].vert4CurrPositionY = newHeightValue;
           this->pTerrainTiles[x][y].vert4->Pos.Y = newHeightValue;
           this->pTerrainTiles[x][y].vert4CurrPositionYDirty = true;

           CheckAndUpdateHeightExistingColumn(x, y, 4, -newHeightValue);

           cell.X = x - 1;
           cell.Y = y;

           //Returns true if we were inside the terrain grid with the coordinates
           //Returns false if were landed outside of the valid grid area, and the coordinates
           //were adjusted
           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert3CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert3->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert3CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 3, -newHeightValue);
           }

           cell.X = x - 1;
           cell.Y = y + 1;

           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert2CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert2->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert2CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 2, -newHeightValue);
           }

           cell.X = x;
           cell.Y = y + 1;

           if (!ForceTileGridCoordRange(cell)) {
               this->pTerrainTiles[cell.X][cell.Y].vert1CurrPositionY = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert1->Pos.Y = newHeightValue;
               this->pTerrainTiles[cell.X][cell.Y].vert1CurrPositionYDirty = true;

               CheckAndUpdateHeightExistingColumn(cell.X, cell.Y, 1, -newHeightValue);
           }

           break;
        }

        default: {
           return;
        }
    }

   irr::core::vector2di startPos(x-1, y-1);
   irr::core::vector2di endPos(x+1, y+1);

   ForceTileGridCoordRange(startPos);
   ForceTileGridCoordRange(endPos);

   for (int xIdx = startPos.X; xIdx <= endPos.X; xIdx++) {
        for (int yIdx = startPos.Y; yIdx <= endPos.Y; yIdx++) {
            //recalculate averaged tile height, this value will be for example used later
            //for player craft calculations...
            this->pTerrainTiles[xIdx][yIdx].currTileHeight = GetAveragedTileHeight(xIdx, yIdx);
        }
   }

   //recalculate all necessary vertice normals
   RecalculateNormals(a);

   for (int xIdx = startPos.X; xIdx <= endPos.X; xIdx++) {
        for (int yIdx = startPos.Y; yIdx <= endPos.Y; yIdx++) {
           UpdateCellMeshVertex1(xIdx, yIdx);
           UpdateCellMeshVertex2(xIdx, yIdx);
           UpdateCellMeshVertex3(xIdx, yIdx);
           UpdateCellMeshVertex4(xIdx, yIdx);
        }
   }

   myStaticTerrainMesh->setDirty(EBT_VERTEX);
   myDynamicTerrainMesh->setDirty(EBT_VERTEX);
}

void LevelTerrain::SetCellTexture(int posX, int posY, int16_t newTextureId, bool onlyUpdateMesh, bool doNotSetMeshDirty) {
    //This higher level function has to do 2 independent things:
    // 1, modify the cell configuration in the level/map file itself (so that next time we
    //    load the map again, everything is restored again in the same modified way)
    // 2, modify the current terrain Mesh used by Irrlicht to show the user the
    //    current state of the Terrain in the level. If we do not do this the level editor
    //    user can not see what he actually has changed already :)

    /******************************************************************
     * Part 1: Modify low level map data                              *
     ******************************************************************/

    //First edit low level/map data itself
    MapEntry* entry = levelRes->pMap[posX][posY];

    if (entry == nullptr)
        return;

    if (!onlyUpdateMesh) {
        //set new texture modifier value in lowlevel map data
        entry->m_TextureId = newTextureId;
    }

    /******************************************************************
     * Part 2: According to new texture modify Irrlicht Terrain Mesh  *
     ******************************************************************/

    // Part 1: Update the current Irrlicht Mesh
    TerrainTileData* tTilePntr = &this->pTerrainTiles[posX][posY];

    //if this tile is optimized away, exit here
    if (!tTilePntr->m_draw_in_mesh)
        return;

    if (!tTilePntr->dynamicMesh) {
        //Remove existing mesh for this tile
        mIrrMeshBuf->RemoveMeshBufferTile(mStaticMeshBufferVec, tTilePntr, *mTerrainMeshStats);

        //Add new mesh with new textureId
        mIrrMeshBuf->AddMeshBufferTile(mStaticMeshBufferVec, tTilePntr, newTextureId, *mTerrainMeshStats);

        if (!doNotSetMeshDirty) {
            myStaticTerrainMesh->setDirty(EBT_VERTEX_AND_INDEX);
            myStaticTerrainMesh->recalculateBoundingBox();
        }
    } else {
        //Remove existing mesh for this tile
        mIrrMeshBuf->RemoveMeshBufferTile(mDynamicMeshBufferVec, tTilePntr, *mTerrainMeshStats);

        //Add new mesh with new textureId
        mIrrMeshBuf->AddMeshBufferTile(mDynamicMeshBufferVec, tTilePntr, newTextureId, *mTerrainMeshStats);

        if (!doNotSetMeshDirty) {
            myDynamicTerrainMesh->setDirty(EBT_VERTEX_AND_INDEX);
            myDynamicTerrainMesh->recalculateBoundingBox();
        }
    }
}

void LevelTerrain::SetCellTextureModification(int posX, int posY, int8_t newTextureModifier) {
    //This higher level function has to do 2 independent things:
    // 1, modify the cell configuration in the level/map file itself (so that next time we
    //    load the map again, everything is restored again in the same modified way)
    // 2, modify the current terrain Mesh used by Irrlicht to show the user the
    //    current state of the Terrain in the level. If we do not do this the level editor
    //    user can not see what he actually has changed already :)

    //check for valid value of new texture modifier
    if ((newTextureModifier < 0) || (newTextureModifier > 7))
        return;

    /******************************************************************
     * Part 1: Modify low level map data                              *
     ******************************************************************/
    MapEntry* entry = levelRes->pMap[posX][posY];

    if (entry == nullptr)
        return;

    //set new texture modifier value in lowlevel map data
    entry->SetTextureModification(newTextureModifier);

    /******************************************************************
     * Part 2: According to new texture modify Irrlicht Terrain Mesh  *
     ******************************************************************/
    TerrainTileData* tTilePntr = &this->pTerrainTiles[posX][posY];

    //if this tile is optimized away, exit here
    if (!tTilePntr->m_draw_in_mesh)
        return;

    std::vector<irr::scene::SMeshBuffer*>::iterator it;
    irr::scene::SMeshBuffer* meshBufPntr;

    //create the new UV information
    std::vector<vector2d<irr::f32>> newUVS = MakeUVs(newTextureModifier);

    irr::u32 vert1Idx;

    void* pntrVert;
    S3DVertex *pntrVertices;

    std::vector<irr::u32>::iterator itVertId1 = tTilePntr->myMeshBufVertexId1.begin();

    irr::u32 bufIdx = 0;

    //iterate through all meshbuffers where this tile is included, and
    //see if we find any of the indices of the vertices of this tile included
    for (it = tTilePntr->myMeshBuffers.begin(); it != tTilePntr->myMeshBuffers.end(); ++it) {
        //what indices do my vertices have in this meshbuffer?
        vert1Idx = (*itVertId1);

        meshBufPntr = (*it);
        meshBufPntr->grab();

        pntrVert = meshBufPntr->getVertices();
        pntrVertices = (S3DVertex*)pntrVert;
        pntrVertices[this->pTerrainTiles[posX][posY].myMeshBufVertexId1[bufIdx]].TCoords = newUVS.at(0);
        pntrVertices[this->pTerrainTiles[posX][posY].myMeshBufVertexId1[bufIdx] + 1].TCoords = newUVS.at(1);
        pntrVertices[this->pTerrainTiles[posX][posY].myMeshBufVertexId1[bufIdx] + 2].TCoords = newUVS.at(2);
        pntrVertices[this->pTerrainTiles[posX][posY].myMeshBufVertexId1[bufIdx] + 3].TCoords = newUVS.at(3);

        bufIdx++;

        meshBufPntr->drop();

        meshBufPntr->setDirty(EBT_VERTEX_AND_INDEX);
    }

    if (!tTilePntr->dynamicMesh) {
        myStaticTerrainMesh->setDirty();
        myStaticTerrainMesh->recalculateBoundingBox();
    } else {
        myDynamicTerrainMesh->setDirty();
        myDynamicTerrainMesh->recalculateBoundingBox();
    }
}

