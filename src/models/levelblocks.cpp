/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "levelblocks.h"
#include "../definitions.h"
#include "../infrabase.h"
#include "column.h"
#include "../resources/columndefinition.h"
#include "../resources/levelfile.h"
#include "../resources/texture.h"
#include "../utils/logging.h"
#include "../resources/mapentry.h"
#include "../draw/drawdebug.h"
#include "../resources/blockdefinition.h"

LevelBlocks::~LevelBlocks() {
  //remove existing SceneNodes
  if (BlockCollisionSceneNode != nullptr) {
      BlockCollisionSceneNode->remove();
      BlockCollisionSceneNode = nullptr;
  }

  if (BlockWithoutCollisionSceneNode != nullptr) {
      BlockWithoutCollisionSceneNode->remove();
      BlockWithoutCollisionSceneNode = nullptr;
  }

  //free existing meshes
  if (blockMeshForCollision != nullptr) {
    mInfra->mSmgr->getMeshCache()->removeMesh(blockMeshForCollision);
    blockMeshForCollision = nullptr;
  }

  if (blockMeshWithoutCollision != nullptr) {
    mInfra->mSmgr->getMeshCache()->removeMesh(blockMeshWithoutCollision);
    blockMeshWithoutCollision = nullptr;
  }

  if (this->ColumnsByPosition.size() > 0) {
    Column* colPntr;

    //free column info
    for(std::vector<ColumnsByPositionStruct>::iterator loopi = this->ColumnsByPosition.begin(); loopi != ColumnsByPosition.end();) {
        colPntr = (*loopi).pColumn;
        loopi = this->ColumnsByPosition.erase(loopi);

        //free the column
        delete colPntr;
    }
  }
}

LevelBlocks::LevelBlocks(InfrastructureBase* infra, LevelTerrain* myTerrain, LevelFile* levelRes,
                         TextureLoader* textureSource, bool debugShowWallCollisionMesh, bool enableLightning) {
   MyTerrain = myTerrain;
   mInfra = infra;
   mEnableLightning = enableLightning;

   //this->m_texfile = texfile;
   mTexSource = textureSource;

   ColumnsByPosition.clear();

   //create all buildings (column objects) out of the raw low level level data
   for(std::vector<ColumnsStruct>::iterator loopi = levelRes->Columns.begin(); loopi != levelRes->Columns.end(); ++loopi) {
       addColumn((*loopi).Columns, (*loopi).Vector3, levelRes);
   }

   segmentSize = DEF_SEGMENTSIZE;

   Blocks_ready = true;

   this->levelRes = levelRes;

   numVertices = 0;
   numIndices = 0;
   numUVs = 0;
   numNormals = 0;

   //generate Mesh with blocks that should all be part of craft collision detection
   blockMeshForCollision = getBlocksMesh(1);

   //generate Mesh with blocks that should not take part in craft collision detection
   //contains roof blocks (for example of tunnels) where player craft easily could get stuck
   //so we do not want to have collision detection for this ones
   blockMeshWithoutCollision = getBlocksMesh(0);

   blockMeshForCollision->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);
   blockMeshWithoutCollision->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

   //we need to use a addMeshSceneNode here, if we use an addOctreeSceneNode here morphing of blocks/columns does not work at all!
   BlockCollisionSceneNode = mInfra->mSmgr->addMeshSceneNode(blockMeshForCollision, 0, IDFlag_IsPickable);
   BlockWithoutCollisionSceneNode = mInfra->mSmgr->addMeshSceneNode(blockMeshWithoutCollision, 0, ID_IsNotPickable);

   BlockCollisionSceneNode->setMaterialFlag(EMF_LIGHTING, mEnableLightning);
   BlockCollisionSceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
   BlockCollisionSceneNode->setMaterialFlag(EMF_FRONT_FACE_CULLING, true);

   BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_LIGHTING, mEnableLightning);
   BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
   BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_FRONT_FACE_CULLING, true);

   SetViewMode(LEVELBLOCKS_VIEW_DEFAULT);

   if (debugShowWallCollisionMesh) {
      BlockCollisionSceneNode->setDebugDataVisible(EDS_BBOX);
   }

   /*std::cout << "HiOctane Blocks loaded: " <<
                   numVertices << " vertices, " <<
                   numNormals << " normals, " <<
                   numUVs << " UVs, " <<
                   mTexSource->NumLevelTextures << " textures, " <<
                   numIndices << " indices" << endl << std::flush;*/

   std::string infoMsg("HiOctane Blocks loaded : ");
   char hlpstr[20];

   //add number blocks in level
   sprintf(hlpstr, "%u", mNrBlocksInLevel);
   infoMsg.append(hlpstr);
   infoMsg.append(" blocks in level, ");

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
}

void LevelBlocks::addColumn(ColumnDefinition* definition, vector3d<irr::f32> pos, LevelFile *levelRes) {
    Column *column = new Column(MyTerrain, this, definition, pos, levelRes);

    this->mNrBlocksInLevel += column->GetNumberContainedBlocks();

    ColumnsByPositionStruct NewVar;
    NewVar.pos =  (int)pos.X + (int)pos.Z * levelRes->Width();
    NewVar.pColumn = column;

    ColumnsByPosition.push_back(NewVar);
}

//if column with key is found, returns true; false otherwise
//if column found returns pointer to column, nullptr otherwise
bool LevelBlocks::searchColumnWithPosition(int posKey, Column* &columnFnd) {
    bool notFound = true;

    std::vector<ColumnsByPositionStruct>::iterator it;
    unsigned int elementIdx = 0;

    //search through all of the available columns
    for (it = ColumnsByPosition.begin(); notFound && (it != ColumnsByPosition.end()); ++it) {
        if ((*it).pos == posKey) {
            //we found the correct column
            columnFnd = ColumnsByPosition.at(elementIdx).pColumn;
            notFound = false;
        }
        elementIdx++;
    }

    return !notFound;
}

void LevelBlocks::SetColumnVerticeSMeshBufferVerticePositionsDirty() {
    this->blockMeshForCollision->setDirty(EBT_VERTEX);
    this->blockMeshWithoutCollision->setDirty(EBT_VERTEX);
}

std::vector<Column*> LevelBlocks::ColumnsInRange(int sx, int sz, float w, float h)
{
    std::vector<Column*> columns;
    Column *fnd; //(MyTerrain, this, nullptr, irr::core::vector3d<float>(0.0f, 0.0f, 0.0f), nullptr);

    for(int z = 0; z < h; z++)
    {
        for (int x = 0; x < w; x++)
        {
            int key = sx + x + (sz + z) * levelRes->Width();

            //we need to search if there is a column with this pos in our list
            if (searchColumnWithPosition(key, fnd)) {
                 columns.push_back(fnd);
            }
        }
    }

    return columns;
}

//function can generate two kinds of meshes
//if collisionSelector = 1 then mesh contains all blocks
//that are needed for collision detection
//if collisionSelector = 0 then mesh contains all blocks
//that should not be included in collision detection
SMesh* LevelBlocks::getBlocksMesh(int collisionSelector) {
    //create all buildings (column objects)
    std::vector<ColumnsByPositionStruct>::iterator loopi;
    ColumnsByPositionStruct GetColumn;
    std::vector<irr::u32> indiceOffset;

    //first make a mesh to hold the overall building data
    SMesh * BuildingMesh = new SMesh();

    //for each type of material (texture) we need to create a own MeshBuffer, as a MeshBuffer can have only triangles with the
    //same material
    std::vector<SMeshBuffer*> meshBuffers;

    //uint num_indices = 0;
    //uint num_vertices = 0;
    //uint num_normals = 0;

    //uint vertexbasenr = 0;

    std::list<ColumnSideGeometryInfo*>::iterator it;

    video::SColor cubeColour3(255,255,255,255);

    vector3d<irr::f32> debug_pos;
    vector3d<irr::f32> debug_norm;
    vector2d<irr::f32> debug_uv;

    SMeshBuffer *newBuf;

    //create a new SMeshBuffer for each material/texture type
    for (int idx = 0; idx < this->mTexSource->NumLevelTextures; idx++) {
        newBuf = new SMeshBuffer();

        //set texture/material for each SMeshBuffer
        newBuf->getMaterial().setTexture(0, this->mTexSource->levelTex[idx]);
        newBuf->getMaterial().Lighting = mEnableLightning;
        newBuf->getMaterial().Wireframe = false;
        newBuf->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

        meshBuffers.push_back(newBuf);

        indiceOffset.push_back(0);
    }

    int currMaterial;
    std::vector<int>::iterator texIt;
    std::vector<int>::iterator indicesVboIt;

    int cubeCnt = 0;

    irr::u32 indnext;
    irr::u32 indbase;
    irr::u32 indsum;

    //copy needed indices
    for(loopi = ColumnsByPosition.begin(); loopi != ColumnsByPosition.end(); ++loopi) {
        GetColumn = (*loopi);

        //each cube has 6 textures, therefore we can calculate number of cubes to draw
        cubeCnt = (int)((*loopi).pColumn->GeometryInfoList->textureIdData.size()) / 6;

        texIt = (*loopi).pColumn->GeometryInfoList->textureIdData.begin();
        indicesVboIt = (*loopi).pColumn->GeometryInfoList->indicesVboData.begin();

        //iterate through all blocks of column
        for (int cubeIdx = 0; cubeIdx < cubeCnt; cubeIdx++) {
            //only add mesh for blocks that are either part of collision detection
            //or not, selected on input parameter
            if (GetColumn.pColumn->Definition->mInCollisionMesh[cubeIdx] == collisionSelector) {
                    //iterate through all 6 sides
                    for (int cidx = 0; cidx < 6; cidx++) {
                        //material of side
                        currMaterial = (*texIt);

                        //copy 6 indices per side
                        for (int iidx = 0; iidx < 6; iidx++) {
                            indbase = (*indicesVboIt);
                            indnext = indiceOffset[currMaterial];
                            indsum = indbase+indnext;
                            meshBuffers[currMaterial]->Indices.push_back(indsum);
                            indicesVboIt++;
                            numIndices++;
                        }

                        texIt++;
                        indiceOffset[currMaterial] += 4;
                    }
            }
         }
    }

    int currVerticeIdx;

    //copy all vertices to all SMeshBuffers
    for(loopi = ColumnsByPosition.begin(); loopi != ColumnsByPosition.end(); ++loopi) {
            GetColumn = (*loopi);

            currVerticeIdx = 0;

            //positionVboIt = (*loopi).pColumn->GeometryInfoList->positionVboData.begin();
            //normalVboIt = (*loopi).pColumn->GeometryInfoList.normalVboData.begin();
            //uvVboIt = (*loopi).pColumn->GeometryInfoList.uvVboData.begin();
            texIt = (*loopi).pColumn->GeometryInfoList->textureIdData.begin();

            //each cube has 24 vertices, calculate number of cubes in this column
            cubeCnt = (int)((*loopi).pColumn->GeometryInfoList->vertices.size()) / 24;

            //also create statistics over vertice count, indice count, ...
            //numVertices += (irr::u32)((*loopi).pColumn->GeometryInfoList->vertices.size());
            //numIndices += (irr::u32)((*loopi).pColumn->GeometryInfoList->indicesVboData.size());
            //numNormals += cubeCnt * 6; //for each cube/box we have 6 normals (one normal for each side)
            //numUVs += cubeCnt * 24; //for each cube/box we have 24 UVs

            //iterate through all cubes
            for (int cubeIdx = 0; cubeIdx < cubeCnt; cubeIdx++) {
                //only add mesh for blocks that are either part of collision detection
                //or not, selected on input parameter
                if (GetColumn.pColumn->Definition->mInCollisionMesh[cubeIdx] == collisionSelector) {
                    //iterate through all sides of the current block/column and fill all SMeshbuffers

                    for (int cidx = 0; cidx < 6; cidx++) {
                        //for each side copy 4 vertices, 4 normals and 4 uvs
                        currMaterial = (*texIt);

                        numNormals += 1;
                        numUVs += 4;

                        for (int vidx = 0; vidx < 4; vidx++) {
                            //copy the vertices to the correct SMeshbuffer

                            //we need to shift position of vertices by position of the cubes, otherwise all cubes will be rendered at the same location
                            /*video::S3DVertex *newvertex = new video::S3DVertex((*positionVboIt).X - GetColumn.pColumn->Position.X-1,
                                                                           (*positionVboIt).Y + GetColumn.pColumn->Position.Y,
                                                                           (*positionVboIt).Z + GetColumn.pColumn->Position.Z,
                                                                           (*normalVboIt).X, (*normalVboIt).Y, (*normalVboIt).Z, cubeColour3, (*uvVboIt).X, (*uvVboIt).Y);*/

                            //vertice was already created before
                            //we need to shift position of vertices by position of the cubes, otherwise all cubes will be rendered at the same location
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].vert->Pos.X -= GetColumn.pColumn->Position.X + 1;
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].vert->Pos.Y += GetColumn.pColumn->Position.Y;
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].vert->Pos.Z += GetColumn.pColumn->Position.Z;

                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].originalPosition.X -= GetColumn.pColumn->Position.X + 1;
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].originalPosition.Y += GetColumn.pColumn->Position.Y;
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].originalPosition.Z += GetColumn.pColumn->Position.Z;

                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].currPosition.X -= GetColumn.pColumn->Position.X + 1;
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].currPosition.Y += GetColumn.pColumn->Position.Y;
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].currPosition.Z += GetColumn.pColumn->Position.Z;

                            //store my index in this MeshBuffer for later direct access
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].myMeshBufVertexId.push_back(meshBuffers[currMaterial]->getVertexCount());

                            //add vertices to all SMeshBuffers
                            meshBuffers[currMaterial]->Vertices.push_back(*(*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].vert);

                            //keep pointer to my MeshBuffer
                            //we need this for morphing of columns later!
                            (*loopi).pColumn->GeometryInfoList->vertices[currVerticeIdx].myMeshBuffers.push_back(meshBuffers[currMaterial]);

                            currVerticeIdx++;
                            numVertices++;
                        }

                        texIt++;
                    }
                }
            }
     }

         //now create final overall Mesh from SMeshbuffers
        for (int idx2 = 0; idx2 < this->mTexSource->NumLevelTextures; idx2++) {
            //only process SMbuffer which actually has something inside it
            if ((meshBuffers[idx2]->getIndexCount() > 0)) {

                meshBuffers[idx2]->BoundingBox.reset(0,0,0);
                meshBuffers[idx2]->recalculateBoundingBox();

                //add SMeshbuffer to overall building mesh
                BuildingMesh->addMeshBuffer(meshBuffers[idx2]);

                BuildingMesh->recalculateBoundingBox();
            }

          //clean up SMeshbuffer, we do not need it anymore
          //note 24.03.2024: we need to keep the MeshBuffer so that we can change the Mesh
          //dynamically (for Morphing!)
          //meshBuffers[idx2]->drop();
        }

    BuildingMesh->setDirty();
    BuildingMesh->recalculateBoundingBox();

    return(BuildingMesh);
}

bool LevelBlocks::GetCurrentCeilingHeightForTileCoord(vector2di cellCoord, irr::f32 &heightVal) {

    //get pntr to this tile
    MapEntry* pntr = this->levelRes->pMap[cellCoord.X][cellCoord.Y];

    irr::u8 firstCollDetBlock = 0;
    irr::u8 nrBlocks = 0;

    if (pntr != nullptr) {
        ColumnDefinition* colDef = pntr->get_Column();
        if (colDef != nullptr) {
            //mInCollisionMesh has a "thermometer code" inside
            //for all the blocks beginning at the terrain level
            //in upwards direction where no collision detection can take place
            //there is a 0 inside; from the moment when collision detection is activated
            //there are 1 values inside for the remaining blocks upwards
            nrBlocks = (irr::u8)(colDef->mInCollisionMesh.size());

            firstCollDetBlock = nrBlocks;

            for (irr::u8 blockCnt = 0; blockCnt < nrBlocks; blockCnt++) {
                if (colDef->mInCollisionMesh[blockCnt] == 1) {
                    //there is already collision detection inside this block
                    //there is no ceiling anymore beginning from this position
                    firstCollDetBlock = blockCnt;
                    break;
                }
            }
        }
    }

    if (firstCollDetBlock > 0) {
        //return the height where the ceiling is located
        //at this cell; return true because we found a ceiling
        heightVal = firstCollDetBlock * this->segmentSize;
        return true;
    }

    //there is no ceiling, return false
    return false;
}

void LevelBlocks::SetViewMode(irr::u8 newViewMode) {
    mCurrentViewMode = newViewMode;

    switch (newViewMode) {
       case LEVELBLOCKS_VIEW_OFF: {
             BlockCollisionSceneNode->setVisible(false);
             BlockWithoutCollisionSceneNode->setVisible(false);

             BlockCollisionSceneNode->setDebugDataVisible(EDS_OFF);
             BlockWithoutCollisionSceneNode->setDebugDataVisible(EDS_OFF);
             break;
       }

        case LEVELBLOCKS_VIEW_WIREFRAME: {
            //change to wireframe view
            BlockCollisionSceneNode->setVisible(true);
            BlockWithoutCollisionSceneNode->setVisible(true);

            BlockCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, true);
            BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, true);

            BlockCollisionSceneNode->setDebugDataVisible(EDS_OFF);
            BlockWithoutCollisionSceneNode->setDebugDataVisible(EDS_OFF);
            break;
        }

        case LEVELBLOCKS_VIEW_DEFAULT: {
            //change to default mode (textured)
            BlockCollisionSceneNode->setVisible(true);
            BlockWithoutCollisionSceneNode->setVisible(true);

            BlockCollisionSceneNode->setDebugDataVisible(EDS_OFF);
            BlockWithoutCollisionSceneNode->setDebugDataVisible(EDS_OFF);

            BlockCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
            BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
            break;
        }

        case LEVELBLOCKS_VIEW_DEBUGNORMALS: {
           //change to normals debug mode (adding also Terrain vertices normals debug view)
           BlockCollisionSceneNode->setVisible(true);
           BlockWithoutCollisionSceneNode->setVisible(true);

           BlockCollisionSceneNode->setDebugDataVisible(EDS_FULL);
           BlockWithoutCollisionSceneNode->setDebugDataVisible(EDS_FULL);

           BlockCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
           BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
           break;
        }
    }
}

irr::u8 LevelBlocks::GetCurrentViewMode() {
    return mCurrentViewMode;
}

void LevelBlocks::DrawOutlineSelectedColumn(Column* selColumnPntr, int nrBlockFromBase, SMaterial* color, SMaterial* selFaceColor, irr::u8 selFace) {
    if (selColumnPntr == nullptr)
        return;

    int idx1 = nrBlockFromBase * 24;
    int nrVertices = selColumnPntr->GeometryInfoList->vertices.size();

    irr::core::vector3df pos1;
    irr::core::vector3df pos2;
    irr::core::vector3df pos3;
    irr::core::vector3df pos4;

    //specified block does not exit!
    if (!((idx1 >= 0) && (idx1 < (nrVertices - 23))))
        return;

    for (int idx = 0; idx < 6; idx++) {
            pos1 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + idx * 4).currPosition;
            pos2 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + idx * 4 + 1).currPosition;
            pos3 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + idx * 4 + 2).currPosition;
            pos4 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + idx * 4 + 3).currPosition;

            mInfra->mDrawDebug->Draw3DRectangle(pos1, pos2, pos3, pos4, color);
    }

    //if we are not specially mark a specified face
    //we are done, exit
    if (selFace == DEF_SELBLOCK_FACENONE)
        return;

    int selFaceIdx = -1;

    //mark also a possible selected face
    switch (selFace) {
         case DEF_SELBLOCK_FACENORTH:  { selFaceIdx = 0;  break;}
         case DEF_SELBLOCK_FACEEAST:   { selFaceIdx = 1;  break;}
         case DEF_SELBLOCK_FACESOUTH:  { selFaceIdx = 2;  break;}
         case DEF_SELBLOCK_FACEWEST:   { selFaceIdx = 3;  break;}
         case DEF_SELBLOCK_FACEBOTTOM: { selFaceIdx = 5;  break;}
         case DEF_SELBLOCK_FACETOP:    { selFaceIdx = 4;  break;}
         default:  { selFaceIdx = -1; break;}
    }

    if (selFaceIdx == -1)
        return;

    pos1 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + selFaceIdx * 4).currPosition;
    pos2 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + selFaceIdx * 4 + 1).currPosition;
    pos3 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + selFaceIdx * 4 + 2).currPosition;
    pos4 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + selFaceIdx * 4 + 3).currPosition;

    //the selected face, draw with different color
    mInfra->mDrawDebug->Draw3DRectangle(pos1, pos2, pos3, pos4, selFaceColor);
}

//Derives the current texturing information about a selected block face
//returns true if the information was found, false otherwise
bool LevelBlocks::GetTextureInfoSelectedBlock(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks,
                                              irr::u8 selFace, int16_t& outCurrTextureId, uint8_t& outCurrTextureModification) {
    if (selColumnPntr == nullptr)
        return false;

    int idx1 = mSelBlockNrSkippingMissingBlocks * 24;
    int nrVertices = selColumnPntr->GeometryInfoList->vertices.size();

    //specified block does not exit!
    if (!((idx1 >= 0) && (idx1 < (nrVertices - 23))))
        return false;

    //if we are not specially mark a specified face
    //we are done, exit
    if (selFace == DEF_SELBLOCK_FACENONE)
        return false;

    int16_t blockValue;

    //get correct selected cube
    switch (nrBlockFromBase) {
        case 0: { blockValue = selColumnPntr->Definition->get_A(); break; }
        case 1: { blockValue = selColumnPntr->Definition->get_B(); break; }
        case 2: { blockValue = selColumnPntr->Definition->get_C(); break; }
        case 3: { blockValue = selColumnPntr->Definition->get_D(); break; }
        case 4: { blockValue = selColumnPntr->Definition->get_E(); break; }
        case 5: { blockValue = selColumnPntr->Definition->get_F(); break; }
        case 6: { blockValue = selColumnPntr->Definition->get_G(); break; }
        case 7: { blockValue = selColumnPntr->Definition->get_H(); break; }
        default: {
            return false;
            break;
        }
    }

    blockValue--;

    if ((blockValue < 0) || (blockValue >= (int16_t)(levelRes->BlockDefinitions.size())))
            return false;

    BlockDefinition *blockDef =  levelRes->BlockDefinitions.at(blockValue);

    //mark also a possible selected face
    switch (selFace) {
         case DEF_SELBLOCK_FACENORTH:  { outCurrTextureId = blockDef->get_N(); outCurrTextureModification = blockDef->NMod(); break;}
         case DEF_SELBLOCK_FACEEAST:   { outCurrTextureId = blockDef->get_E(); outCurrTextureModification = blockDef->EMod(); break;}
         case DEF_SELBLOCK_FACESOUTH:  { outCurrTextureId = blockDef->get_S(); outCurrTextureModification = blockDef->SMod(); break;}
         case DEF_SELBLOCK_FACEWEST:   { outCurrTextureId = blockDef->get_W(); outCurrTextureModification = blockDef->WMod(); break;}
         case DEF_SELBLOCK_FACEBOTTOM: { outCurrTextureId = blockDef->get_B(); outCurrTextureModification = blockDef->BMod(); break;}
         case DEF_SELBLOCK_FACETOP:    { outCurrTextureId = blockDef->get_T(); outCurrTextureModification = blockDef->TMod(); break;}
         default:  { return false; break;}
    }

    return true;
}

