/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "levelblocks.h"
#include "../definitions.h"

LevelBlocks::~LevelBlocks() {
  //remove existing SceneNodes
  if (BlockCollisionSceneNode != NULL) {
      BlockCollisionSceneNode->remove();
      BlockCollisionSceneNode = NULL;
  }

  if (BlockWithoutCollisionSceneNode != NULL) {
      BlockWithoutCollisionSceneNode->remove();
      BlockWithoutCollisionSceneNode = NULL;
  }

  //free existing meshes
  if (blockMeshForCollision != NULL) {
    this->MySmgr->getMeshCache()->removeMesh(blockMeshForCollision);
    blockMeshForCollision = NULL;
  }

  if (blockMeshWithoutCollision != NULL) {
    this->MySmgr->getMeshCache()->removeMesh(blockMeshWithoutCollision);
    blockMeshWithoutCollision = NULL;
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

LevelBlocks::LevelBlocks(LevelTerrain* myTerrain,
                         LevelFile* levelRes, scene::ISceneManager *mySmgr, irr::video::IVideoDriver* driver, TextureLoader* textureSource,
                         bool enableLightning) {
   this->m_driver = driver;
   MyTerrain = myTerrain;
   MySmgr = mySmgr;
   mEnableLightning = enableLightning;

   //this->m_texfile = texfile;
   mTexSource = textureSource;

   ColumnsByPosition.clear();

   //create all buildings (column objects) out of the raw low level level data
   for(std::vector<ColumnsStruct>::iterator loopi = levelRes->Columns.begin(); loopi != levelRes->Columns.end(); ++loopi) {
       addColumn((*loopi).Columns, (*loopi).Vector3, levelRes, driver, textureSource);
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
   BlockCollisionSceneNode = MySmgr->addMeshSceneNode(blockMeshForCollision, 0, IDFlag_IsPickable);
   BlockWithoutCollisionSceneNode = MySmgr->addMeshSceneNode(blockMeshWithoutCollision, 0, ID_IsNotPickable);

   BlockCollisionSceneNode->setMaterialFlag(EMF_LIGHTING, mEnableLightning);
   BlockCollisionSceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
   BlockCollisionSceneNode->setMaterialFlag(EMF_FRONT_FACE_CULLING, true);

   //Uncomment next line to only see wireframe of the Buildings
   BlockCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, false);

   BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_LIGHTING, mEnableLightning);
   BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
   BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_FRONT_FACE_CULLING, true);

   //Uncomment next line to only see wireframe of the Buildings
   BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, false);

   std::cout << "HiOctane Blocks loaded: " <<
                   numVertices << " vertices, " <<
                   numNormals << " normals, " <<
                   numUVs << " UVs, " <<
                   mTexSource->NumLevelTextures << " textures, " <<
                   numIndices << " indices" << endl << std::flush;
}

void LevelBlocks::addColumn(ColumnDefinition* definition, vector3d<irr::f32> pos, LevelFile *levelRes, irr::video::IVideoDriver *driver, TextureLoader* texureSource) {
    Column *column = new Column(MyTerrain, this, definition, pos, levelRes);

    ColumnsByPositionStruct NewVar;
    NewVar.pos =  (int)pos.X + (int)pos.Z * levelRes->Width();
    NewVar.pColumn = column;

    ColumnsByPosition.push_back(NewVar);
}

//if column with key is found, returns true; false otherwise
//if column found returns pointer to column, NULL otherwise
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
    Column *fnd; //(MyTerrain, this, NULL, irr::core::vector3d<float>(0.0f, 0.0f, 0.0f), NULL);

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

    //copy needed indices
   // for(loopi = ColumnsByPosition.begin(); loopi != ColumnsByPosition.end(); ++loopi) {
    //    GetColumn = (*loopi);

        //create a new SMeshBuffer for each material/texture type
        for (int idx = 0; idx < this->mTexSource->NumLevelTextures; idx++) {
            newBuf = new SMeshBuffer();

            //set number of indices and vertices needed for each SMeshBuffer
             //for each triangle we need 3 indices and 3 vertices
            /*  newBuf->Indices.set_used(numTrianglesPerMaterialList[idx]*3);
           newBuf->Vertices.set_used(numTrianglesPerMaterialList[idx]*3);*/

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

        for (int cubeIdx = 0; cubeIdx < cubeCnt; cubeIdx++) {
            //only add mesh for blocks that are either part of collision detection
            //or not, selected on input parameter
            if (GetColumn.pColumn->Definition->mInCollisionMesh[cubeIdx] == collisionSelector) {

        //iterate through all blocks of column
        //for (it = (*loopi).pColumn->GeometryInfoList.begin(); it != (*loopi).pColumn->GeometryInfoList.end(); ++it) {
            //iterate through all sides of the current block/column and fill SMeshBuffers according to
            //current materials of the sides

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
                }

                texIt++;
                indiceOffset[currMaterial] += 4;
            }
        }

          }
        }
    //}

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
            numVertices += (irr::u32)((*loopi).pColumn->GeometryInfoList->vertices.size());
            numIndices += (irr::u32)((*loopi).pColumn->GeometryInfoList->indicesVboData.size());
            numNormals += cubeCnt * 6; //for each cube/box we have 6 normals (one normal for each side)
            numUVs += cubeCnt * 24; //for each cube/box we have 24 UVs

            //iterate through all cubes
            for (int cubeIdx = 0; cubeIdx < cubeCnt; cubeIdx++) {
                //only add mesh for blocks that are either part of collision detection
                //or not, selected on input parameter
                if (GetColumn.pColumn->Definition->mInCollisionMesh[cubeIdx] == collisionSelector) {
                    //iterate through all sides of the current block/column and fill all SMeshbuffers

                    for (int cidx = 0; cidx < 6; cidx++) {
                        //for each side copy 4 vertices, 4 normals and 4 uvs
                        currMaterial = (*texIt);

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

    if (pntr != NULL) {
        ColumnDefinition* colDef = pntr->get_Column();
        if (colDef != NULL) {
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

void LevelBlocks::SwitchViewMode() {
    switch (myCurrentViewMode) {
        case LEVELBLOCKS_VIEW_WIREFRAME: {
            //change to default mode
            BlockCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
            BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, false);
            myCurrentViewMode = LEVELBLOCKS_VIEW_DEFAULT;
            break;
        }

    case LEVELBLOCKS_VIEW_DEFAULT: {
        //change to full debug mode (adding also Terrain vertices normals debug view)
        BlockCollisionSceneNode->setDebugDataVisible(EDS_FULL);
        BlockWithoutCollisionSceneNode->setDebugDataVisible(EDS_FULL);
        myCurrentViewMode = LEVELBLOCKS_VIEW_DEBUGNORMALS;
        break;
    }

    case LEVELBLOCKS_VIEW_DEBUGNORMALS: {
        //change to wireframe mode
        BlockCollisionSceneNode->setDebugDataVisible(EDS_OFF);
        BlockCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, true);

        BlockWithoutCollisionSceneNode->setDebugDataVisible(EDS_OFF);
        BlockWithoutCollisionSceneNode->setMaterialFlag(EMF_WIREFRAME, true);
        myCurrentViewMode = LEVELBLOCKS_VIEW_WIREFRAME;
        break;
    }
  }
}

