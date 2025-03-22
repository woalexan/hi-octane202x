/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did translation to C++, and then modified and extended code for my project)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "levelterrain.h"

void LevelTerrain::ResetTerrainTileData() {
    int levelWidth = this->levelRes->Width();
    int levelHeight = this->levelRes->Height();

    for (int i = 0; i < levelWidth; i++) {
        for (int j = 0; j < levelHeight; j++) {
            pTerrainTiles[i][j].vert1 = NULL;
            pTerrainTiles[i][j].vert2 = NULL;
            pTerrainTiles[i][j].vert3 = NULL;
            pTerrainTiles[i][j].vert4 = NULL;
            pTerrainTiles[i][j].vert1UVcoord.set(0.0f, 0.0f);
            pTerrainTiles[i][j].vert2UVcoord.set(0.0f, 0.0f);
            pTerrainTiles[i][j].vert3UVcoord.set(0.0f, 0.0f);
            pTerrainTiles[i][j].vert4UVcoord.set(0.0f, 0.0f);
            pTerrainTiles[i][j].myMeshBuffers.clear();
            pTerrainTiles[i][j].currTileHeight = 0.0f;
            pTerrainTiles[i][j].m_draw_in_mesh = false;
            pTerrainTiles[i][j].vert1CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTiles[i][j].vert2CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTiles[i][j].vert3CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTiles[i][j].vert4CurrNormal.set(0.0f, 1.0f, 0.0f);
            pTerrainTiles[i][j].myMeshBufVertexId1.clear();
            pTerrainTiles[i][j].myMeshBufVertexId2.clear();
            pTerrainTiles[i][j].myMeshBufVertexId3.clear();
            pTerrainTiles[i][j].myMeshBufVertexId4.clear();
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
    //optimize terrain
    findTerrainOptimization();

    if (setupGeometry()) {
         std::cout << "HiOctane Terrain '" << mName << "' loaded: " <<
                    numVertices << " vertices, " <<
                    numNormals << " normals, " <<
                    numUVs << " UVs, " <<
                    mTexSource->NumLevelTextures << " textures, " <<
                    numIndices << " indices" << endl << std::flush;

          if (numVertices > 80000) {
                 std::cout << "More than 80000 vertices for Level Mesh! Drawing whole level not possible anymore. Need to exit application!" << endl << std::flush;
                 Terrain_ready = false;
          }

         std::cout << "HiOctane Terrain '" << mName << "' loaded ok"  << endl << std::flush;

    } else {
        std::cout << "failed setting up game model '" << mName << "'" << endl << std::flush;
        Terrain_ready = false;
    }

    if (Terrain_ready) {
        /***********************************************************/
        /* Create Terrain Mesh                                     */
        /***********************************************************/

        //Create Mesh for Terrain
        CreateTerrainMesh();

        //create Static SceneNode for Terrain
        StaticTerrainSceneNode = this->m_smgr->addMeshSceneNode(myStaticTerrainMesh, 0, IDFlag_IsPickable);

        //we need to rotate the terrain Mesh, otherwise it is upside down
        StaticTerrainSceneNode->setRotation(core::vector3df(0.0f, 0.0f, 180.0f));
        StaticTerrainSceneNode->setMaterialFlag(EMF_LIGHTING, mEnableLightning);

        StaticTerrainSceneNode->setMaterialFlag(EMF_FOG_ENABLE, true);

        //create dynamic SceneNode for Terrain
        DynamicTerrainSceneNode = this->m_smgr->addMeshSceneNode(myDynamicTerrainMesh, 0, IDFlag_IsPickable);

        //we need to rotate the terrain Mesh, otherwise it is upside down
        DynamicTerrainSceneNode->setRotation(core::vector3df(0.0f, 0.0f, 180.0f));
        DynamicTerrainSceneNode->setMaterialFlag(EMF_LIGHTING, mEnableLightning);

        DynamicTerrainSceneNode->setMaterialFlag(EMF_FOG_ENABLE, true);
    }
}

LevelTerrain::LevelTerrain(char* name, LevelFile* levelRes, scene::ISceneManager *mySmgr, irr::video::IVideoDriver* driver,
                           TextureLoader* textureSource, Race* mRaceParent, bool enableLightning) {
   this->m_driver = driver;
   this->m_smgr = mySmgr;
   this->mRace = mRaceParent;
   mEnableLightning = enableLightning;

   strcpy(mName, name);

   //this->m_texfile = texfile;
   mTexSource = textureSource;

   segmentSize = 1.0f; // must be 1 for Hi-Octane !!

   Terrain_ready = true;

   this->levelRes = levelRes;

   numVertices = 0;
   numIndices = 0;
   numUVs = 0;
   numNormals = 0;

   //reset my internal map!
   ResetTerrainTileData();
}

LevelTerrain::~LevelTerrain() {
  //remove my static SceneNode
  if (StaticTerrainSceneNode != NULL) {
    StaticTerrainSceneNode->remove();
    StaticTerrainSceneNode = NULL;
  }

  //remove my dynamic SceneNode
  if (DynamicTerrainSceneNode != NULL) {
    DynamicTerrainSceneNode->remove();
    DynamicTerrainSceneNode = NULL;
  }

  //free static terrain mesh TerrainMesh
  if (myStaticTerrainMesh != NULL) {
    this->m_smgr->getMeshCache()->removeMesh(myStaticTerrainMesh);
    myStaticTerrainMesh = NULL;
  }

  //free dynamic terrain mesh TerrainMesh
  if (myDynamicTerrainMesh != NULL) {
    this->m_smgr->getMeshCache()->removeMesh(myDynamicTerrainMesh);
    myDynamicTerrainMesh = NULL;
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
         if (pTerrainTiles[i][j].vert1 != NULL) {
            delete pTerrainTiles[i][j].vert1;
            pTerrainTiles[i][j].vert1 = NULL;
         }

         //free created vertices
         if (pTerrainTiles[i][j].vert2 != NULL) {
            delete pTerrainTiles[i][j].vert2;
            pTerrainTiles[i][j].vert2 = NULL;
         }

         //free created vertices
         if (pTerrainTiles[i][j].vert3 != NULL) {
            delete pTerrainTiles[i][j].vert3;
            pTerrainTiles[i][j].vert3 = NULL;
         }

         //free created vertices
         if (pTerrainTiles[i][j].vert4 != NULL) {
            delete pTerrainTiles[i][j].vert4;
            pTerrainTiles[i][j].vert4 = NULL;
         }
       }
    }
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
            m_driver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
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

void LevelTerrain::CreateTerrainMesh() {
    //create Mesh for the static Terrain

    //first make a mesh to hold the overall level terrain data
    myStaticTerrainMesh = new SMesh;
    myStaticTerrainMesh->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    //mesh for dynamic terrain
    myDynamicTerrainMesh = new SMesh;
    myDynamicTerrainMesh->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    std::vector<int>::iterator it;
    it = this->indicesVboDataStatic.begin();

    std::vector<int>::iterator it5;
    it5 = this->textureIdDataStatic.begin();

    //for each type of material (texture) we need to create a own MeshBuffer, as a MeshBuffer can have only triangles with the
    //same material
    //Note 02.01.2025: I had performance issues during morphs, because I believe the problem
    //is that all static parts of the terrain that do not change, and the dynamic smaller portions of the
    //level are all in the same mesh and SceneNodes; When now the dynamic smaller part is changed during an active
    //morph a lot of data transfers and recalculations have to be done unnecessary; To improve performance I thought it might
    //be a good idea to create own Meshbuffers for static terrain and dynamic terrain data, and also independent SceneNodes
    //Therefor below I not only create a Meshbuffer for each available TextureID for each material, but also additional I create
    //again the same amount of meshbuffers for the dynamic parts of the terrain
    std::vector<SMeshBuffer*> meshBuffers;

    u64 triangles_remaining = this->indicesVboDataStatic.size()/3;
    u64 triangles_done = 0;

    //set iterator5 back to beginning of data
    it5 = this->textureIdDataStatic.begin();

    SMeshBuffer *newBuf;

    //create a new SMeshBuffer for each material/texture type
    //first for the static parts of the terrain
    for (int idx = 0; idx < this->mTexSource->NumLevelTextures; idx++) {
      newBuf = new SMeshBuffer();

      //set texture/material for each SMeshBuffer
      newBuf->getMaterial().setTexture(0, this->mTexSource->levelTex[idx]);
      newBuf->getMaterial().Lighting = mEnableLightning;
      newBuf->getMaterial().Wireframe = false;
      //newBuf->getMaterial().AntiAliasing = EAAM_QUALITY;
      newBuf->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);
      meshBuffers.push_back(newBuf);
    }

    //seconds for the dynamic parts of the terrain
    for (int idx = 0; idx < this->mTexSource->NumLevelTextures; idx++) {
      newBuf = new SMeshBuffer();

      //set texture/material for each SMeshBuffer
      newBuf->getMaterial().setTexture(0, this->mTexSource->levelTex[idx]);
      newBuf->getMaterial().Lighting = mEnableLightning;
      newBuf->getMaterial().Wireframe = false;
      //newBuf->getMaterial().AntiAliasing = EAAM_QUALITY;
      newBuf->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);
      meshBuffers.push_back(newBuf);
    }

    //now we just need to sort the triangles into the different MeshBuffers according to their
    //textureId and dependent on if the are static or dynamic
    u32 nr;

    /*****************************************
     * Static terrain                        *
     * ***************************************/
    triangles_remaining = this->indicesVboDataStatic.size()/3;
    triangles_done = 0;

    video::SColor cubeColour2(255,255,255,255);

    u16 debug2;
    int usedMaterial = 0;

    it = this->indicesVboDataStatic.begin();
    it5 = this->textureIdDataStatic.begin();

    while (triangles_remaining > 0 ) {
        usedMaterial = (*it5);

        //for each triangle add 3 indices
        for (nr = 0; nr < 3; nr++) {
            //add index
            debug2 = ((u16)(*it));

            meshBuffers[usedMaterial]->Indices.push_back(debug2);

            it++;
        }

        it5++;

        triangles_remaining--;
        triangles_done++;
    }

    /*****************************************
     * Dynamic terrain                       *
     * ***************************************/
    triangles_remaining = this->indicesVboDataDynamic.size()/3;
    triangles_done = 0;

    usedMaterial = 0;

    it = this->indicesVboDataDynamic.begin();
    it5 = this->textureIdDataDynamic.begin();

    while (triangles_remaining > 0 ) {
        usedMaterial = (*it5);

        //for each triangle add 3 indices
        for (nr = 0; nr < 3; nr++) {
            //add index
            debug2 = ((u16)(*it));

            meshBuffers[usedMaterial + this->mTexSource->NumLevelTextures]->Indices.push_back(debug2);

            it++;
        }

        it5++;

        triangles_remaining--;
        triangles_done++;
    }

     int x, z;

     int Width = levelRes->Width();
     int Height = levelRes->Height();
     MapEntry *a;

     int nrTex = this->mTexSource->NumLevelTextures;

    //brute force variant: Add all available vertices to all SMBuffers
    //to make it work; better would be to only add vertices to SMBuffers
    //which are actually needed there; maybe improve later
    for (int idx2 = 0; idx2 < nrTex; idx2++) {
        //only process SMbuffer which actually has something inside it
        if ((meshBuffers[idx2]->getIndexCount() > 0)) {
            for (z = 0; z < Height; z++) {
              for (x = 0; x < Width; x++) {
                  a = GetMapEntry(x, z);

                  //only add vertices if terrain tile is drawn and was not optimized out
                  //we also need to store the meshBuffer and vertex index for later morphing of Terrain
                  if (this->pTerrainTiles[x][z].m_draw_in_mesh == true) {
                          //this tile is static and belongs into the static mesh buffer
                          if (this->pTerrainTiles[x][z].dynamicMesh == false) {

                              this->pTerrainTiles[x][z].myMeshBufVertexId1.push_back(meshBuffers[idx2]->getVertexCount());
                              meshBuffers[idx2]->Vertices.push_back(*this->pTerrainTiles[x][z].vert1);

                              this->pTerrainTiles[x][z].myMeshBufVertexId2.push_back(meshBuffers[idx2]->getVertexCount());
                              meshBuffers[idx2]->Vertices.push_back(*this->pTerrainTiles[x][z].vert2);

                              this->pTerrainTiles[x][z].myMeshBufVertexId3.push_back(meshBuffers[idx2]->getVertexCount());
                              meshBuffers[idx2]->Vertices.push_back(*this->pTerrainTiles[x][z].vert3);

                              this->pTerrainTiles[x][z].myMeshBufVertexId4.push_back(meshBuffers[idx2]->getVertexCount());
                              meshBuffers[idx2]->Vertices.push_back(*this->pTerrainTiles[x][z].vert4);

                              //keep pointer to my MeshBuffer
                              //we need this for morphing of Terrain!
                              this->pTerrainTiles[x][z].myMeshBuffers.push_back(meshBuffers[idx2]);
                         }
                  }
              }
            }

        meshBuffers[idx2]->BoundingBox.reset(0,0,0);
        meshBuffers[idx2]->recalculateBoundingBox();

        //meshBuffers[idx2+nrTex]->BoundingBox.reset(0,0,0);
        //meshBuffers[idx2+nrTex]->recalculateBoundingBox();

        //add SMeshbuffer to overall terrain mesh
        myStaticTerrainMesh->addMeshBuffer(meshBuffers[idx2]);
        //myDynamicTerrainMesh->addMeshBuffer(meshBuffers[idx2+nrTex]);

        myStaticTerrainMesh->recalculateBoundingBox();
        //myDynamicTerrainMesh->recalculateBoundingBox();
      }

        //only process SMbuffer which actually has something inside it
        if ((meshBuffers[idx2 + nrTex]->getIndexCount() > 0)) {
            for (z = 0; z < Height; z++) {
              for (x = 0; x < Width; x++) {
                  a = GetMapEntry(x, z);

                  //only add vertices if terrain tile is drawn and was not optimized out
                  //we also need to store the meshBuffer and vertex index for later morphing of Terrain
                  if (this->pTerrainTiles[x][z].m_draw_in_mesh == true) {
                          //this tile is dynamic and belongs into the dynamic mesh buffer
                          if (this->pTerrainTiles[x][z].dynamicMesh == true) {

                              this->pTerrainTiles[x][z].myMeshBufVertexId1.push_back(meshBuffers[idx2+nrTex]->getVertexCount());
                              meshBuffers[idx2+nrTex]->Vertices.push_back(*this->pTerrainTiles[x][z].vert1);

                              this->pTerrainTiles[x][z].myMeshBufVertexId2.push_back(meshBuffers[idx2+nrTex]->getVertexCount());
                              meshBuffers[idx2+nrTex]->Vertices.push_back(*this->pTerrainTiles[x][z].vert2);

                              this->pTerrainTiles[x][z].myMeshBufVertexId3.push_back(meshBuffers[idx2+nrTex]->getVertexCount());
                              meshBuffers[idx2+nrTex]->Vertices.push_back(*this->pTerrainTiles[x][z].vert3);

                              this->pTerrainTiles[x][z].myMeshBufVertexId4.push_back(meshBuffers[idx2+nrTex]->getVertexCount());
                              meshBuffers[idx2+nrTex]->Vertices.push_back(*this->pTerrainTiles[x][z].vert4);

                              //keep pointer to my MeshBuffer
                              //we need this for morphing of Terrain!
                              this->pTerrainTiles[x][z].myMeshBuffers.push_back(meshBuffers[idx2+nrTex]);
                         }
                  }
              }
            }

        meshBuffers[idx2+nrTex]->BoundingBox.reset(0,0,0);
        meshBuffers[idx2+nrTex]->recalculateBoundingBox();

        //add SMeshbuffer to overall terrain mesh
        myDynamicTerrainMesh->addMeshBuffer(meshBuffers[idx2+nrTex]);

        myDynamicTerrainMesh->recalculateBoundingBox();
      }

      //clean up SMeshbuffer, we do not need it anymore
      //note 16.03.2024: we need to keep the MeshBuffer so that we can change the Mesh
      //dynamically (for Morphing!)
      //meshBuffers[idx2]->drop();
    }

    //mark Terrain mesh as dirty, so that it is transfered again to graphics card
    myStaticTerrainMesh->setDirty();
    myStaticTerrainMesh->recalculateBoundingBox();

    myDynamicTerrainMesh->setDirty();
    myDynamicTerrainMesh->recalculateBoundingBox();
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

std::vector<vector2d<irr::f32>> LevelTerrain::MakeUVs(int textureId, int texMod) {
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
void LevelTerrain::findTerrainOptimization() {
    int x, z, i = 0;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    irr::f32 middle_cell_height;
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
        } else  this->pTerrainTiles[x][z].m_draw_in_mesh = true;
      }
    }
}

bool LevelTerrain::setupGeometry() {
    int x, z, iStatic = 0;
    int iDynamic = 0;

    // generate vertices
    //std::vector<vector3d<irr::f32>> vertices;
    //std::vector<vector3d<irr::f32>> normals;
    //std::vector<vector2d<irr::f32>> uvs;

    numVertices = 0;
    numIndices = 0;
    numUVs = 0;
    numNormals = 0;

    float max = 0.0f;
    std::vector<int> indicesStatic;
    std::vector<int> indicesDynamic;

    int Width = levelRes->Width();
    int Height = levelRes->Height();

    video::SColor cubeColour2(255,255,255,255);
    core::vector3df normal;

    for (z = 0; z < Height; z++) {
      for (x = 0; x < Width; x++) {
        // 4 vertices - need separate UVs so cannot share
        MapEntry *a = GetMapEntry(x, z);

        //create 4 irrlicht vertices for this tile, regardless if we show the tile later or not!
        this->pTerrainTiles[x][z].vert1 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, cubeColour2, 0.0f, 0.0f);
        this->pTerrainTiles[x][z].vert2 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, cubeColour2, 0.0f, 0.0f);
        this->pTerrainTiles[x][z].vert3 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, cubeColour2, 0.0f, 0.0f);
        this->pTerrainTiles[x][z].vert4 = new video::S3DVertex(0.0f,0.0f,0.0f, 0.0f, 0.0f, 0.0f, cubeColour2, 0.0f, 0.0f);

        this->pTerrainTiles[x][z].vert1CurrPositionY = 0.0f;
        this->pTerrainTiles[x][z].vert2CurrPositionY = 0.0f;
        this->pTerrainTiles[x][z].vert3CurrPositionY = 0.0f;
        this->pTerrainTiles[x][z].vert4CurrPositionY = 0.0f;

        this->pTerrainTiles[x][z].vert1UVcoord.set(0.0f, 0.0f);
        this->pTerrainTiles[x][z].vert2UVcoord.set(0.0f, 0.0f);
        this->pTerrainTiles[x][z].vert3UVcoord.set(0.0f, 0.0f);
        this->pTerrainTiles[x][z].vert4UVcoord.set(0.0f, 0.0f);

        this->pTerrainTiles[x][z].currTileHeight = 0.0f;

            MapEntry *b = GetMapEntry(x + 1, z);
            MapEntry *c = GetMapEntry(x + 1, z + 1);
            MapEntry *d = GetMapEntry(x, z + 1);

            //vector3d<irr::f32> *newvec = new vector3d<irr::f32>(x       * segmentSize, -irr::f32(a->m_Height),  z * segmentSize);
            //vertices.insert(vertices.end(), *newvec);

            this->pTerrainTiles[x][z].vert1->Pos = vector3d<irr::f32>(x       * segmentSize, -irr::f32(a->m_Height),  z * segmentSize);
            this->pTerrainTiles[x][z].vert1CurrPositionY = this->pTerrainTiles[x][z].vert1->Pos.Y;
            this->pTerrainTiles[x][z].vert1CurrPositionYDirty = false;

            //vector3d<irr::f32> *newvec2 = new vector3d<irr::f32>((x + 1) * segmentSize, -irr::f32(b->m_Height), z * segmentSize);
            //vertices.insert(vertices.end(), *newvec2);

            this->pTerrainTiles[x][z].vert2->Pos = vector3d<irr::f32>((x + 1) * segmentSize, -irr::f32(b->m_Height), z * segmentSize);

            this->pTerrainTiles[x][z].vert2CurrPositionY = this->pTerrainTiles[x][z].vert2->Pos.Y;
            this->pTerrainTiles[x][z].vert2CurrPositionYDirty = false;

            //vector3d<irr::f32> *newvec3 = new vector3d<irr::f32>((x + 1) * segmentSize, -irr::f32(c->m_Height), (z + 1) * segmentSize);
            //vertices.insert(vertices.end(), *newvec3);

            this->pTerrainTiles[x][z].vert3->Pos = vector3d<irr::f32>((x + 1) * segmentSize, -irr::f32(c->m_Height), (z + 1) * segmentSize);

            this->pTerrainTiles[x][z].vert3CurrPositionY = this->pTerrainTiles[x][z].vert3->Pos.Y;
            this->pTerrainTiles[x][z].vert3CurrPositionYDirty = false;


            //vector3d<irr::f32> *newvec4 = new vector3d<irr::f32>(x       * segmentSize, -irr::f32(d->m_Height), (z + 1) * segmentSize);
            //vertices.insert(vertices.end(), *newvec4);

            this->pTerrainTiles[x][z].vert4->Pos = vector3d<irr::f32>(x       * segmentSize, -irr::f32(d->m_Height), (z + 1) * segmentSize);
            this->pTerrainTiles[x][z].vert4CurrPositionY = this->pTerrainTiles[x][z].vert4->Pos.Y;
            this->pTerrainTiles[x][z].vert4CurrPositionYDirty = false;

            //precalculate averaged tile height, this value will be for example used later
            //for player craft calculations...
            this->pTerrainTiles[x][z].currTileHeight = GetAveragedTileHeight(x, z);

            //texture atlas 4 UVs
            std::vector<vector2d<irr::f32>> newuvs;
            newuvs = MakeUVs(a->m_TextureId, a->m_TextureModification);
            //uvs.insert(uvs.end(), newuvs.begin(), newuvs.end());

            this->pTerrainTiles[x][z].vert1->TCoords = newuvs[0];
            this->pTerrainTiles[x][z].vert1UVcoord = newuvs[0];

            this->pTerrainTiles[x][z].vert2->TCoords = newuvs[1];
            this->pTerrainTiles[x][z].vert2UVcoord = newuvs[1];

            this->pTerrainTiles[x][z].vert3->TCoords = newuvs[2];
            this->pTerrainTiles[x][z].vert3UVcoord = newuvs[2];

            this->pTerrainTiles[x][z].vert4->TCoords = newuvs[3];
            this->pTerrainTiles[x][z].vert4UVcoord = newuvs[3];

            this->pTerrainTiles[x][z].VertUpdatedUVScoord = false;

            // add normals
            normal = computeNormalFromMapEntries(x    , z    , 1.0f);
            //normals.push_back(normal);
            this->pTerrainTiles[x][z].vert1->Normal = normal;
            this->pTerrainTiles[x][z].vert1CurrNormal = normal;

            normal = computeNormalFromMapEntries(x + 1, z    , 1.0f);
            //normals.push_back(normal);
            this->pTerrainTiles[x][z].vert2->Normal = normal;
            this->pTerrainTiles[x][z].vert2CurrNormal = normal;

            normal = computeNormalFromMapEntries(x + 1, z + 1, 1.0f);
            //normals.push_back(normal);
            this->pTerrainTiles[x][z].vert3->Normal = normal;
            this->pTerrainTiles[x][z].vert3CurrNormal = normal;

            normal = computeNormalFromMapEntries(x    , z + 1, 1.0f);
            //normals.push_back(normal);
            this->pTerrainTiles[x][z].vert4->Normal = normal;
            this->pTerrainTiles[x][z].vert4CurrNormal = normal;

            this->pTerrainTiles[x][z].RefreshNormals = false;

            //only create mesh for cell if it was not optimized away (non used parts of the level map)
            if (this->pTerrainTiles[x][z].m_draw_in_mesh == true) {
                if (this->pTerrainTiles[x][z].dynamicMesh == false) {
                    //this is static terrain (no morphing)
                     numUVs += 4;
                     numVertices += 4;

                     //store the texture ID information for the 2 tris
                     textureIdDataStatic.push_back(a->m_TextureId);
                     textureIdDataStatic.push_back(a->m_TextureId);

                     numNormals += 4;

                     // add indices for the 2 tris
                     indicesStatic.push_back(iStatic);
                     indicesStatic.push_back(iStatic + 1);
                     indicesStatic.push_back(iStatic + 3);

                     indicesStatic.push_back(iStatic + 1);
                     indicesStatic.push_back(iStatic + 2);
                     indicesStatic.push_back(iStatic + 3);

                     iStatic += 4;

                     numIndices += 6;

                     // determine max height
                     max = std::max(max, a->m_Height);
                } else {
                    //this is dynamic terrain (affected by morphing)
                     numUVs += 4;
                     numVertices += 4;

                     //store the texture ID information for the 2 tris
                     textureIdDataDynamic.push_back(a->m_TextureId);
                     textureIdDataDynamic.push_back(a->m_TextureId);

                     numNormals += 4;

                     // add indices for the 2 tris
                     indicesDynamic.push_back(iDynamic);
                     indicesDynamic.push_back(iDynamic + 1);
                     indicesDynamic.push_back(iDynamic + 3);

                     indicesDynamic.push_back(iDynamic + 1);
                     indicesDynamic.push_back(iDynamic + 2);
                     indicesDynamic.push_back(iDynamic + 3);

                     iDynamic += 4;

                     numIndices += 6;

                     // determine max height
                     max = std::max(max, a->m_Height);
                }
          }
       }
     }

   // positionVboData = vertices;
   // normalVboData = normals;
    indicesVboDataStatic = indicesStatic;
    indicesVboDataDynamic = indicesDynamic;
   // uvVboData = uvs;

    Size.X = levelRes->Width() * segmentSize;
    Size.Y = max;
    Size.Z = levelRes->Height() * segmentSize;

    return true;
}

/*
void LevelTerrain::AddDirtySMeshBuffer(irr::scene::SMeshBuffer *newDirtyBuffer,
                                       std::vector<irr::scene::SMeshBuffer*> &currDirtyList)
{
    //only add new dirty buffer pointer to list if the new specified buffer is not yet
    //part of the list
    bool fnd = false;
    std::vector<irr::scene::SMeshBuffer*>::iterator it;

    for (it = currDirtyList.begin(); it != currDirtyList.end(); ++(it)) {
        if (newDirtyBuffer == (*it)) {
            fnd = true;
            break;
        }
    }

    //not found, add to list
    if (!fnd) {
        currDirtyList.push_back(newDirtyBuffer);
    }
}*/

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
                          MakeUVs(a->m_TextureId, a->m_TextureModification) :
                          MakeUVs(e->m_TextureId, e->m_TextureModification);

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

                           //add this Meshbuffer to the list of overall dirty buffers
                         //  AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
                       }

                      dirtyPos = true;
                  }

                  if (dx < morph.Width && dz > 0) {
                      idxMeshBuf = 0;

                      for (it2 = this->pTerrainTiles[x][z].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][z].myMeshBuffers.end(); ++(it2)) {
                           (*it2)->grab();
                           void* pntrVert = (*it2)->getVertices();
                           pntrVertices = (S3DVertex*)pntrVert;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId2[idxMeshBuf]].Pos.Y = this->pTerrainTiles[x][z].vert2CurrPositionY;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId2[idxMeshBuf]].TCoords = this->pTerrainTiles[x][z].vert2UVcoord;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId2[idxMeshBuf]].Normal = this->pTerrainTiles[x][z].vert2CurrNormal;

                           idxMeshBuf++;

                           (*it2)->drop();

                           //add this Meshbuffer to the list of overall dirty buffers
                          // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
                       }

                      dirtyPos = true;
                  }

                  if (dx < morph.Width && dz < morph.Height) {
                      idxMeshBuf = 0;

                      for (it2 = this->pTerrainTiles[x][z].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][z].myMeshBuffers.end(); ++(it2)) {
                           (*it2)->grab();
                           void* pntrVert = (*it2)->getVertices();
                           pntrVertices = (S3DVertex*)pntrVert;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId3[idxMeshBuf]].Pos.Y = this->pTerrainTiles[x][z].vert3CurrPositionY;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId3[idxMeshBuf]].TCoords = this->pTerrainTiles[x][z].vert3UVcoord;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId3[idxMeshBuf]].Normal = this->pTerrainTiles[x][z].vert3CurrNormal;

                           idxMeshBuf++;

                           (*it2)->drop();

                           //add this Meshbuffer to the list of overall dirty buffers
                          // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
                       }

                      dirtyPos = true;
                  }

                  if (dx > 0 && dz < morph.Height) {
                      idxMeshBuf = 0;

                      for (it2 = this->pTerrainTiles[x][z].myMeshBuffers.begin(); it2 != this->pTerrainTiles[x][z].myMeshBuffers.end(); ++(it2)) {
                           (*it2)->grab();
                           void* pntrVert = (*it2)->getVertices();
                           pntrVertices = (S3DVertex*)pntrVert;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId4[idxMeshBuf]].Pos.Y = this->pTerrainTiles[x][z].vert4CurrPositionY;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId4[idxMeshBuf]].TCoords = this->pTerrainTiles[x][z].vert4UVcoord;
                           pntrVertices[this->pTerrainTiles[x][z].myMeshBufVertexId4[idxMeshBuf]].Normal = this->pTerrainTiles[x][z].vert4CurrNormal;

                           idxMeshBuf++;

                           (*it2)->drop();

                           //add this Meshbuffer to the list of overall dirty buffers
                          // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
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
    /*if (this->mRace != NULL) {
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

    if (pntr != NULL) {
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

void LevelTerrain::SwitchViewMode() {
    switch (myCurrentViewMode) {
        case LEVELTERRAIN_VIEW_WIREFRAME: {
            //change to default mode
            StaticTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
            DynamicTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
            myCurrentViewMode = LEVELTERRAIN_VIEW_DEFAULT;
            break;
        }

    case LEVELTERRAIN_VIEW_DEFAULT: {
        //change to full debug mode (adding also Terrain vertices normals debug view)
        StaticTerrainSceneNode->setDebugDataVisible(EDS_FULL);
        DynamicTerrainSceneNode->setDebugDataVisible(EDS_FULL);
        myCurrentViewMode = LEVELTERRAIN_VIEW_DEBUGNORMALS;
        break;
    }

    case LEVELTERRAIN_VIEW_DEBUGNORMALS: {
        //change to wireframe mode
        StaticTerrainSceneNode->setDebugDataVisible(EDS_OFF);
        StaticTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, true);
        DynamicTerrainSceneNode->setDebugDataVisible(EDS_OFF);
        DynamicTerrainSceneNode->setMaterialFlag(EMF_WIREFRAME, true);
        myCurrentViewMode = LEVELTERRAIN_VIEW_WIREFRAME;
        break;
    }
  }
}

