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
#include "../resources/texture.h"
#include "../infrabase.h"
#include "../models/morph.h"
#include "../resources/mapentry.h"
#include "../utils/logging.h"
#include "../resources/entityitem.h"
#include "../draw/drawdebug.h"

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
      for (int j = startRegion.tileYmin; j <= startRegion.tileYmax; j++) {
         for (int i = startRegion.tileXmin; i <= startRegion.tileXmax ; i++) {
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

    if (SetupGeometry()) {
         std::string infoMsg("HiOctane Terrain '");
         infoMsg.append(mName);
         infoMsg.append("' loaded: ");
         char hlpstr[20];

         //add number vertices
         sprintf(hlpstr, "%u", numVertices);
         infoMsg.append(hlpstr);
         infoMsg.append(" vertices, ");

         //add number normals
         sprintf(hlpstr, "%u", numNormals);
         infoMsg.append(hlpstr);
         infoMsg.append(" normals, ");

         //add number UVs
         sprintf(hlpstr, "%u", numUVs);
         infoMsg.append(hlpstr);
         infoMsg.append(" UVs, ");

         //add number textures
         sprintf(hlpstr, "%d", mTexSource->NumLevelTextures);
         infoMsg.append(hlpstr);
         infoMsg.append(" textures, ");

         //add number indices
         sprintf(hlpstr, "%u", numIndices);
         infoMsg.append(hlpstr);
         infoMsg.append(" indices");

         logging::Info(infoMsg);

         //std::cout << "HiOctane Terrain '" << mName << "' loaded ok"  << endl << std::flush;
         infoMsg.clear();
         infoMsg.append("HiOctane Terrain '");
         infoMsg.append(mName);
         infoMsg.append("' loaded ok");
         logging::Info(infoMsg);

    } else {
        //std::cout << "failed setting up game model '" << mName << "'" << endl << std::flush;
        std::string errMsg("Failed setting up Terrain '");
        errMsg.append(mName);
        errMsg.append("'");
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
    }
}

//finds the current (last in linked list) MeshBuffer info struct for a certain textureId
//returns nullptr in case of error
MeshBufferInfoStruct* LevelTerrain::FindLastMeshBufferInLinkedList(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId) {
    //vector index is textureId
    MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(forTextureId);

    if (currPntr == nullptr)
        return nullptr;

    //loop through MeshbufferInfoStruct until we find the last
    //existing Meshbuffer Element
    while (currPntr->nextPntr != nullptr) {
         currPntr = currPntr->nextPntr;
    }

    return currPntr;
}

//finds the first available MeshBuffer info struct for a certain textureId
//which has still space for 6 additional indices (an additional Quad)
//returns nullptr in case of error, or nothing available
MeshBufferInfoStruct* LevelTerrain::FindFirstMeshBufferForAdditionalQuad(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId) {
    //vector index is textureId
    MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(forTextureId);

    if (currPntr == nullptr)
        return nullptr;

    if (currPntr->remainingIndices >= 6)
        return currPntr;

    //loop through MeshbufferInfoStruct until we find the next
    //Meshbuffer element with at least 6 free indices
    while (currPntr->nextPntr != nullptr) {
         currPntr = currPntr->nextPntr;

         if (currPntr->remainingIndices >= 6)
             return currPntr;
    }

    //no available meshbuffer found with at
    //least 6 free indices
    return nullptr;
}

//counts the number of existing Meshbuffers for all possible
//Texture Ids
std::vector<irr::u8> LevelTerrain::ReturnMeshBufferCntPerTextureId(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec) {
    std::vector<irr::u8> result;

    result.clear();
    irr::u8 cnt;

    for (int i = 0; i < mTerrainAvailableTextureCount; i++) {
         //vector index is textureId
         MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(i);
         cnt = 0;

        if (currPntr == nullptr) {
            result.push_back(cnt);
            continue;
        }

        if (currPntr->meshBuf != nullptr) {
           cnt++;
        }

        //loop through MeshbufferInfoStruct until we find the last
        //existing Meshbuffer Element
        while (currPntr->nextPntr != nullptr) {
             currPntr = currPntr->nextPntr;

             if (currPntr->meshBuf != nullptr) {
                cnt++;
             }
        }

        result.push_back(cnt);
    }

    return result;
}

//adds an additional Meshbuffer for the specified textureId (material). Returns
//a pointer to the new added MeshBufferInfoStruct. In case something goes wrong
//returns nullptr
MeshBufferInfoStruct* LevelTerrain::AddAdditionalMeshBuffer(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId) {
    //plausi-check for forTextureId parameter
    if ((forTextureId < 0) || (forTextureId >= mTerrainAvailableTextureCount)) {
       //something wrong, exit!
       return nullptr;
    }

    //find out if there is already a Meshbuffer existing for this material?
    //we do not need to search in the vector, as I make sure that
    //there are always as many MeshBufferInfroStructs in this vector
    //as there are available terrain textures; index is textureId
    MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(forTextureId);

    MeshBufferInfoStruct* addToPntr = nullptr;

    //Meshbuffers for this Material are already existing?
    if (currPntr->meshBuf != nullptr) {
       //yes, it is
       addToPntr = currPntr;

       //loop through MeshbufferInfoStruct until we find the last
       //existing Meshbuffer Element
       //add a new element there
       while (addToPntr->nextPntr != nullptr) {
           addToPntr = addToPntr->nextPntr;
       }
    }

    SMeshBuffer* newBuf = new SMeshBuffer();

    //set texture/material for each SMeshBuffer
    newBuf->getMaterial().setTexture(0, this->mTexSource->levelTex[forTextureId]);
    newBuf->getMaterial().Lighting = mEnableLightning;
    newBuf->getMaterial().Wireframe = false;

    //newBuf->getMaterial().AntiAliasing = EAAM_QUALITY;
    newBuf->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    //if this is the first MeshBuffer for this material
    //simply replace the nullptr in the initial element
    if (currPntr->meshBuf == nullptr) {
        currPntr->meshBuf = newBuf;
        currPntr->remainingIndices = 65535;

        return currPntr;
    } else {
        //for each additional Meshbuffer we need to add a new MeshBufferInfoStruct
        //to encapsulate the new Meshbuffer
        MeshBufferInfoStruct* newStruct = new MeshBufferInfoStruct();
        newStruct->textureId = forTextureId;
        newStruct->meshBuf = newBuf;
        newStruct->remainingIndices = 65535;

        //add pointer to new info struct
        //into the last existing info struct for
        //this texture Id (single linked list)
        addToPntr->nextPntr = newStruct;

        return newStruct;
    }
}

void LevelTerrain::AddMeshBufferTile(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, TerrainTileData* tilePntr, int16_t textureId) {
    //what is the current Meshbuffer for the textureId
    //of the new tile
    MeshBufferInfoStruct* nextBufInfo = FindFirstMeshBufferForAdditionalQuad(targetMeshBufVec, textureId);

    //if routine returns nullptr something is wrong,
    //or no free meshbuffer currently available to add
    //new tile (quad)
    if (nextBufInfo == nullptr) {
        //no, create an additional MeshBuffer for this texture Id
        nextBufInfo = AddAdditionalMeshBuffer(targetMeshBufVec, textureId);

        //something wrong?
        if (nextBufInfo == nullptr)
            return;
    }

    //nextBufInfo contains now the Meshbuffer where we
    //want to add the additional tile (4 vertices, 6 indices)
    nextBufInfo->meshBuf->grab();

    irr::u16 firstIndexNewQuad = nextBufInfo->meshBuf->getVertexCount();

    //add the 4 existing vertices for this tile into the
    //meshbuffer vertice array
    nextBufInfo->meshBuf->Vertices.push_back(*tilePntr->vert1);
    nextBufInfo->meshBuf->Vertices.push_back(*tilePntr->vert2);
    nextBufInfo->meshBuf->Vertices.push_back(*tilePntr->vert3);
    nextBufInfo->meshBuf->Vertices.push_back(*tilePntr->vert4);

    //at the same time store in tile which index the vertices
    //have in the meshbuffer vertices array; we need this information later
    //for morphing
    tilePntr->myMeshBufVertexId1.push_back(firstIndexNewQuad);

    // add indices for the 2 new tris of the new
    //quad (this are 6 new indices)
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 1);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 3);

    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 1);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 2);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 3);

    //decrease remaining number of available indices in this Meshbuffer
    //for this textureId
    nextBufInfo->remainingIndices -= 6;

    nextBufInfo->meshBuf->drop();

    //also keep info which Meshbuffers we have used for this
    //tile; is also necessary for morphing later
    //current used Meshbuffer already stored for this tile?
    std::vector<irr::scene::SMeshBuffer*>::iterator it;
    bool found = false;

    for (it = tilePntr->myMeshBuffers.begin(); it != tilePntr->myMeshBuffers.end(); ++it) {
        if ((*it) == nextBufInfo->meshBuf) {
            found = true;
            break;
        }
    }

    if (!found) {
        //not yet part of vector, add also this (new) MeshBuffer
        tilePntr->myMeshBuffers.push_back(nextBufInfo->meshBuf);
    }

    //increase statistical values
    numUVs += 4;
    numVertices += 4;
    numNormals += 4;
    numIndices += 6;
}

std::vector<irr::scene::SMeshBuffer*> LevelTerrain::ReturnAllMeshBuffersForTextureId(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t textureId) {
    std::vector<irr::scene::SMeshBuffer*> result;
    result.clear();

    //vector index is textureId
    MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(textureId);

    if (currPntr == nullptr)
        return result;

    if (currPntr->meshBuf != nullptr) {
        result.push_back(currPntr->meshBuf);
    }

    while (currPntr->nextPntr != nullptr) {
        if (currPntr->nextPntr->meshBuf != nullptr) {
            result.push_back(currPntr->nextPntr->meshBuf);
        }

        currPntr = currPntr->nextPntr;
    }

    //return list of found MeshBuffers
    return result;
}

void LevelTerrain::CleanupMeshBufferInfoStructs(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec) {
    std::vector<MeshBufferInfoStruct*> infoStructVec;
    std::vector<MeshBufferInfoStruct*>::iterator it;

    MeshBufferInfoStruct* pntrStruct;

    for (int i = 0; i < mTerrainAvailableTextureCount; i++) {
        pntrStruct = targetMeshBufVec.at(i);
        infoStructVec.clear();

        if (pntrStruct == nullptr)
            continue;

        infoStructVec.push_back(pntrStruct);

        while (pntrStruct->nextPntr != nullptr) {
            pntrStruct = pntrStruct->nextPntr;
            infoStructVec.push_back(pntrStruct);
        }

        for (it = infoStructVec.begin(); it != infoStructVec.end(); ) {
            pntrStruct = (*it);

            it = infoStructVec.erase(it);

            //delete MeshBufferInfoStruct
            delete pntrStruct;
        }
    }
}

LevelTerrain::LevelTerrain(InfrastructureBase* infra, bool levelEditorMode, char* name, LevelFile* levelRes, TextureLoader* textureSource, bool optimizeMesh, bool enableLightning) {
   this->mInfra = infra;
   mEnableLightning = enableLightning;
   mOptimizeMesh = optimizeMesh;
   mLevelEditorMode = levelEditorMode;
   mTexSource = textureSource;

   mTerrainAvailableTextureCount = mTexSource->NumLevelTextures;

   mStaticMeshBufferVec.clear();
   mDynamicMeshBufferVec.clear();

   //initial fill the mStaticMeshBufferVec vector
   //with empty MeshBufferInfroStructs, one for each possible
   //level texture Id
   for (int i = 0; i < mTerrainAvailableTextureCount; i++) {
       MeshBufferInfoStruct* newStruct = new MeshBufferInfoStruct;
       newStruct->textureId = i;

       mStaticMeshBufferVec.push_back(newStruct);
   }

   //initial fill the mDynamicMeshBufferVec vector
   //with empty MeshBufferInfroStructs, one for each possible
   //level texture Id
   for (int i = 0; i < mTerrainAvailableTextureCount; i++) {
       MeshBufferInfoStruct* newStruct = new MeshBufferInfoStruct;
       newStruct->textureId = i;

       mDynamicMeshBufferVec.push_back(newStruct);
   }

   strcpy(mName, name);

   segmentSize = 1.0f; // must be 1 for Hi-Octane !!

   Terrain_ready = true;

   this->levelRes = levelRes;

   numVertices = 0;
   numIndices = 0;
   numUVs = 0;
   numNormals = 0;

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

  CleanupMeshBufferInfoStructs(mStaticMeshBufferVec);
  CleanupMeshBufferInfoStructs(mDynamicMeshBufferVec);
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

    irr::video::SColor shieldChargerColor(255, 24, 254, 56);
    irr::video::SColor fuelChargerColor(255, 121, 65, 198);
    irr::video::SColor ammoChargerColor(255, 254, 222, 48);
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
                        cPixelCol = fuelChargerColor;
                        break;
                     }

                case 47: { //Ammo charger
                        cPixelCol = ammoChargerColor;
                        break;
                     }

                case 51: { //Shield charger
                        cPixelCol = shieldChargerColor;
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
void LevelTerrain::ForceTileGridCoordRange(irr::core::vector2di &tileGridPos) {
    if (tileGridPos.X < 0) {
        tileGridPos.X = 0;
    }

    if (tileGridPos.Y < 0) {
        tileGridPos.Y = 0;
    }

    if (tileGridPos.X >= this->levelRes->Width()) {
        tileGridPos.X = (this->levelRes->Width() - 1);
    }

    if (tileGridPos.Y >= this->levelRes->Height()) {
        tileGridPos.Y = (this->levelRes->Height() - 1);
    }
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
            if ((MiddleCell->m_TextureId) != (Neighborcell->m_TextureId) || (MiddleCell->m_TextureModification != Neighborcell->m_TextureModification)) {
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

bool LevelTerrain::SetupGeometry() {
    int x, z = 0;

    float max = 0.0f;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    video::SColor cubeColour2(255,255,255,255);

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
        tile->vert1 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, cubeColour2, 0.0f, 0.0f);
        tile->vert2 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, cubeColour2, 0.0f, 0.0f);
        tile->vert3 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, cubeColour2, 0.0f, 0.0f);
        tile->vert4 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, cubeColour2, 0.0f, 0.0f);

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
        newuvs = MakeUVs(a->m_TextureModification);

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
                 AddMeshBufferTile(mStaticMeshBufferVec, tile, a->m_TextureId);
              } else {
                 //is a dynamic cell (is able to morph)
                 AddMeshBufferTile(mDynamicMeshBufferVec, tile, a->m_TextureId);
              }
        }
      }
    }

    //get number of already existing Meshbuffers for all available Texture Ids of Terrain
    std::vector<irr::u8> nrMeshBuffersPerTexId = ReturnMeshBufferCntPerTextureId(mStaticMeshBufferVec);

    //if we are starting for the level editor we need to make sure that for each possible
    //texture Id existing we have enough meshbuffers available, so that in worst case if user
    //tries to set all existing tiles of a map to the same texture Id, we do not run out of
    //meshbuffer indices overall. Because Irrlicht is limited to max 65535 indices per
    //meshbuffer
    if (mLevelEditorMode) {
        irr::u8 buffersToAdd;

        //in for loop add additional "empty" meshbuffers for this worst case scenario
        //first for static mesh
        for (int i = 0; i < mTerrainAvailableTextureCount; i++) {
           buffersToAdd = mLevelEditorMinNrMeshBuffersNeeded - nrMeshBuffersPerTexId.at(i);

           for (int j = 0; j < buffersToAdd; j++) {
               AddAdditionalMeshBuffer(mStaticMeshBufferVec, i);
           }
        }
    }

    nrMeshBuffersPerTexId = ReturnMeshBufferCntPerTextureId(mDynamicMeshBufferVec);

    if (mLevelEditorMode) {
        irr::u8 buffersToAdd;

        //in for loop add additional "empty" meshbuffers for this worst case scenario
        //first for static mesh
        for (int i = 0; i < mTerrainAvailableTextureCount; i++) {
           buffersToAdd = mLevelEditorMinNrMeshBuffersNeeded - nrMeshBuffersPerTexId.at(i);

           for (int j = 0; j < buffersToAdd; j++) {
               AddAdditionalMeshBuffer(mDynamicMeshBufferVec, i);
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

    for (int currTexId = 0; currTexId < mTerrainAvailableTextureCount; currTexId++) {

        bufList = ReturnAllMeshBuffersForTextureId(mStaticMeshBufferVec, currTexId);

        for (bufIt = bufList.begin(); bufIt != bufList.end(); ++bufIt) {
              (*bufIt)->BoundingBox.reset(0,0,0);
              (*bufIt)->recalculateBoundingBox();

              //add SMeshbuffer to overall terrain mesh
              myStaticTerrainMesh->addMeshBuffer((*bufIt));

              myStaticTerrainMesh->recalculateBoundingBox();
        }
   }

   for (int currTexId = 0; currTexId < mTerrainAvailableTextureCount; currTexId++) {

        bufList = ReturnAllMeshBuffersForTextureId(mDynamicMeshBufferVec, currTexId);

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

void LevelTerrain::ApplyMorph(Morph morph)
      {
          if (morph.getProgress() == morph.LastProgress)
              return;

          bool dirtyPos;

          //std::vector<irr::scene::SMeshBuffer*> dirtySMeshBuffers;
          irr::core::vector2di cellSrc = morph.Source->getCell();
          irr::core::vector2di cellTarget = morph.Target->getCell();

          int xSource = cellSrc.X - 1;
          int zSource = cellSrc.Y - 1;
          int xTarget = cellTarget.X - 1;
          int zTarget = cellTarget.Y - 1;
          //List<MapEntry> refreshNormals = new List<MapEntry>();

          // check if UVs need an update
          bool sourceEnabled = morph.getProgress() > 0.01f;
          bool updateUVs = morph.UVSFromSource != sourceEnabled;
          morph.UVSFromSource = sourceEnabled;

          std::vector<irr::scene::SMeshBuffer*>::iterator it2;
          std::vector<vector2d<irr::f32>> uvs;
          S3DVertex *pntrVertices;
          irr::u32 idxMeshBuf;
          bool updatedUVS;

          dirtyPos = false;

          // apply morph to positions and texture UVs
          for (int dz = 0; dz < morph.Height + 1; dz++)
          {
              for (int dx = 0; dx < morph.Width + 1; dx++)
              {
                  // source entries
                  int x = xSource + dx;
                  int z = zSource + dz;

                  MapEntry* a = GetMapEntry(x, z);
                  MapEntry* b = GetMapEntry(x + 1, z);
                  MapEntry* c = GetMapEntry(x + 1, z + 1);
                  MapEntry* d = GetMapEntry(x, z + 1);

                  //activate refreshNormals for entry a
                  this->pTerrainTiles[x][z].RefreshNormals = true;

                  // target entries
                  x = xTarget + dx;
                  z = zTarget + dz;

                  MapEntry* e = GetMapEntry(x, z);
                  MapEntry* f = GetMapEntry(x + 1, z);
                  MapEntry* g = GetMapEntry(x + 1, z + 1);
                  MapEntry* h = GetMapEntry(x, z + 1);

                  //activate refreshNormals for entry e
                  this->pTerrainTiles[x][z].RefreshNormals = true;

                  // set UVs either from source or from target
                  if (updateUVs && dx > 0 && dz > 0)
                  {
                      //create updated texture coordinates
                       uvs = sourceEnabled && !morph.Permanent ?
                          MakeUVs(a->m_TextureModification) :
                          MakeUVs(e->m_TextureModification);

                       updatedUVS = true;

                       //store precalculated results
                       this->pTerrainTiles[x][z].vert1UVcoord = uvs[0];
                       this->pTerrainTiles[x][z].vert2UVcoord = uvs[1];
                       this->pTerrainTiles[x][z].vert3UVcoord = uvs[2];
                       this->pTerrainTiles[x][z].vert4UVcoord = uvs[3];

                       this->pTerrainTiles[x][z].VertUpdatedUVScoord = true;
                  }

                  // calculate and set new Y values
                  if (dx > 0 && dz > 0) {
                     this->pTerrainTiles[x][z].vert1CurrPositionY = -(e->m_Height * (1.0f - morph.getProgress()) + a->m_Height * morph.getProgress());
                     this->pTerrainTiles[x][z].vert1CurrPositionYDirty = true;
                  }

                  if (dx < morph.Width && dz > 0) {
                     this->pTerrainTiles[x][z].vert2CurrPositionY = -(f->m_Height * (1.0f - morph.getProgress()) + b->m_Height * morph.getProgress());
                     this->pTerrainTiles[x][z].vert2CurrPositionYDirty = true;
                   }

                  if (dx < morph.Width && dz < morph.Height) {
                     this->pTerrainTiles[x][z].vert3CurrPositionY = -(g->m_Height * (1.0f - morph.getProgress()) + c->m_Height * morph.getProgress());
                     this->pTerrainTiles[x][z].vert3CurrPositionYDirty = true;
                  }

                 if (dx > 0 && dz < morph.Height) {
                     this->pTerrainTiles[x][z].vert4CurrPositionY =  -(h->m_Height * (1.0f - morph.getProgress()) + d->m_Height * morph.getProgress());
                     this->pTerrainTiles[x][z].vert4CurrPositionYDirty = true;
                 }

                 //recalculate averaged tile height, this value will be for example used later
                 //for player craft calculations...
                 this->pTerrainTiles[x][z].currTileHeight = GetAveragedTileHeight(x, z);
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
                  updatedUVS= false;

                  // target entries
                  int x = xTarget + dx;
                  int z = zTarget + dz;

                  // calculate and set new Y values
                  if (dx > 0 && dz > 0) {
                      idxMeshBuf = 0;
                      for (it2 = this->pTerrainTiles[x][z].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][z].myMeshBuffers.end(); ++(it2)) {
                           (*it2)->grab();
                           void* pntrVert = (*it2)->getVertices();
                           pntrVertices = (S3DVertex*)pntrVert;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf]].Pos.Y = this->pTerrainTiles[x][z].vert1CurrPositionY;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf]].TCoords = this->pTerrainTiles[x][z].vert1UVcoord;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf]].Normal = this->pTerrainTiles[x][z].vert1CurrNormal;

                           idxMeshBuf++;

                           (*it2)->drop();
                       }

                      dirtyPos = true;
                  }

                  if (dx < morph.Width && dz > 0) {
                      idxMeshBuf = 0;

                      for (it2 = this->pTerrainTiles[x][z].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][z].myMeshBuffers.end(); ++(it2)) {
                           (*it2)->grab();
                           void* pntrVert = (*it2)->getVertices();
                           pntrVertices = (S3DVertex*)pntrVert;

                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 1].Pos.Y = this->pTerrainTiles[x][z].vert2CurrPositionY;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 1].TCoords = this->pTerrainTiles[x][z].vert2UVcoord;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 1].Normal = this->pTerrainTiles[x][z].vert2CurrNormal;


                           idxMeshBuf++;

                           (*it2)->drop();
                       }

                      dirtyPos = true;
                  }

                  if (dx < morph.Width && dz < morph.Height) {
                      idxMeshBuf = 0;

                      for (it2 = this->pTerrainTiles[x][z].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][z].myMeshBuffers.end(); ++(it2)) {
                           (*it2)->grab();
                           void* pntrVert = (*it2)->getVertices();
                           pntrVertices = (S3DVertex*)pntrVert;

                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 2].Pos.Y = this->pTerrainTiles[x][z].vert3CurrPositionY;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 2].TCoords = this->pTerrainTiles[x][z].vert3UVcoord;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 2].Normal = this->pTerrainTiles[x][z].vert3CurrNormal;


                           idxMeshBuf++;

                           (*it2)->drop();
                       }

                      dirtyPos = true;
                  }

                  if (dx > 0 && dz < morph.Height) {
                      idxMeshBuf = 0;

                      for (it2 = this->pTerrainTiles[x][z].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][z].myMeshBuffers.end(); ++(it2)) {
                           (*it2)->grab();
                           void* pntrVert = (*it2)->getVertices();
                           pntrVertices = (S3DVertex*)pntrVert;

                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 3].Pos.Y = this->pTerrainTiles[x][z].vert4CurrPositionY;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 3].TCoords = this->pTerrainTiles[x][z].vert4UVcoord;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId1[idxMeshBuf] + 3].Normal = this->pTerrainTiles[x][z].vert4CurrNormal;


                           idxMeshBuf++;

                           (*it2)->drop();
                       }

                      dirtyPos = true;
                  }

              }
          }

          if (dirtyPos) {
                myDynamicTerrainMesh->setDirty(EBT_VERTEX);
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

/***********************************************************************
 * Functions mainly used by the LevelEditor and not the game itself    *
 ***********************************************************************/

void LevelTerrain::DrawOutlineSelectedCell(irr::core::vector2di selCellCoordinate, SMaterial* color) {
    irr::core::vector3df pos1 = pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert1->Pos;
    pos1.X = -pos1.X;
    pos1.Y = -pos1.Y;

    irr::core::vector3df pos2 = pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert2->Pos;
    pos2.X = -pos2.X;
    pos2.Y = -pos2.Y;

    irr::core::vector3df pos3 = pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert3->Pos;
    pos3.X = -pos3.X;
    pos3.Y = -pos3.Y;

    irr::core::vector3df pos4 = pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert4->Pos;
    pos4.X = -pos4.X;
    pos4.Y = -pos4.Y;

    this->mInfra->mDrawDebug->Draw3DRectangle(pos1, pos2, pos3, pos4, color);
}

//returns nullptr in case appropriate MeshBufferInfoStruct is not found
MeshBufferInfoStruct* LevelTerrain::FindMeshBufferInfoStructForMeshBuffer(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, irr::scene::SMeshBuffer* meshBufToFind) {
   std::vector<MeshBufferInfoStruct*>::iterator it;

   for (it = targetMeshBufVec.begin(); it != targetMeshBufVec.end(); ++it) {
       if ((*it)->meshBuf == meshBufToFind) {
           //found, return pointer
           return (*it);
       }
   }

   //not found
   return nullptr;
}

void LevelTerrain::RemoveMeshBufferTile(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, TerrainTileData* tilePntr) {
    //if this tile is optimized away, exit here
    if (!tilePntr->m_draw_in_mesh)
        return;

    std::vector<irr::scene::SMeshBuffer*>::iterator it;
    irr::scene::SMeshBuffer* meshBufPntr;

    irr::u32 vert1Idx;
    irr::u32 vert2Idx;
    irr::u32 vert3Idx;
    irr::u32 vert4Idx;

    irr::u32 nrIndices;
    bool dirty;

    std::vector<irr::u32>::iterator itVertId1 = tilePntr->myMeshBufVertexId1.begin();

    MeshBufferInfoStruct* pInfoStruct;

    /******************************************************************
     * Part 1: Remove existing tile from Meshbuffers and Mesh         *
     ******************************************************************/

    //iterate through all meshbuffers where this tile is included, and
    //see if we find any of the indices of the vertices of this tile included
    for (it = tilePntr->myMeshBuffers.begin(); it != tilePntr->myMeshBuffers.end(); ) {
        //what indices do my vertices have in this meshbuffer?
        vert1Idx = (*itVertId1);
        vert2Idx = (*itVertId1) + 1;
        vert3Idx = (*itVertId1) + 2;
        vert4Idx = (*itVertId1) + 3;

        meshBufPntr = (*it);

        //we need to adjust the number of free indices in the meshbuffer
        //in case we delete indices later
        //Therefore we need to find the correct MeshBufferInfoStruct for
        //this Meshbuffer
        pInfoStruct = FindMeshBufferInfoStructForMeshBuffer(targetMeshBufVec, meshBufPntr);

        meshBufPntr->grab();

        nrIndices = meshBufPntr->getIndexCount();
        dirty = false;

        for (irr::u32 idxCnt = 0; idxCnt < nrIndices; idxCnt++) {
            //if we find any indices of the tile to remove, remove it from the indices
            //array
            if ((meshBufPntr->Indices[idxCnt] == vert1Idx) ||
               (meshBufPntr->Indices[idxCnt] == vert2Idx) ||
               (meshBufPntr->Indices[idxCnt] == vert3Idx) ||
               (meshBufPntr->Indices[idxCnt] == vert4Idx)) {
                    meshBufPntr->Indices.erase(idxCnt);

                    dirty = true;

                    nrIndices--;
                    if (pInfoStruct != nullptr) {
                        //There is one more available index
                        //again in indices array of Meshbuffer
                        pInfoStruct->remainingIndices++;
                    }

                    //after erasing the element the same idxCnt points now
                    //already to the next element
                    if (idxCnt >= 1) {
                       idxCnt--;
                    }
              }
        }

        meshBufPntr->drop();

        if (dirty) {
            meshBufPntr->setDirty(EBT_VERTEX_AND_INDEX);

            //we also remove this meshBuffer index from the myMeshBuffers
            //vector
            it = tilePntr->myMeshBuffers.erase(it);

            itVertId1 = tilePntr->myMeshBufVertexId1.erase(itVertId1);
        } else {
            //advance to the next myMeshBuffers position
            it++;

            itVertId1++;
        }
    }
}

void LevelTerrain::SetCellTexture(int posX, int posY, int16_t newTextureId) {
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

    //set new texture modifier value in lowlevel map data
    entry->m_TextureId = newTextureId;

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
        RemoveMeshBufferTile(mStaticMeshBufferVec, tTilePntr);

        //Add new mesh with new textureId
        AddMeshBufferTile(mStaticMeshBufferVec, tTilePntr, newTextureId);

        myStaticTerrainMesh->setDirty(EBT_VERTEX_AND_INDEX);
        myStaticTerrainMesh->recalculateBoundingBox();
    } else {
        //Remove existing mesh for this tile
        RemoveMeshBufferTile(mDynamicMeshBufferVec, tTilePntr);

        //Add new mesh with new textureId
        AddMeshBufferTile(mDynamicMeshBufferVec, tTilePntr, newTextureId);

        myDynamicTerrainMesh->setDirty(EBT_VERTEX_AND_INDEX);
        myDynamicTerrainMesh->recalculateBoundingBox();
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
    entry->m_TextureModification = newTextureModifier;

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

