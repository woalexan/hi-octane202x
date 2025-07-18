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
#include "irrmeshbuf.h"
#include "../models/levelterrain.h"

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

  mIrrMeshBuf->CleanupMeshBufferInfoStructs(mBlockwCollMeshBufferVec);
  mIrrMeshBuf->CleanupMeshBufferInfoStructs(mBlockwoCollMeshBufferVec);

  delete mBlocksMeshStats;
}

LevelBlocks::LevelBlocks(InfrastructureBase* infra, LevelTerrain* myTerrain, LevelFile* levelRes,
                         TextureLoader* textureSource, bool levelEditorMode, bool debugShowWallCollisionMesh, bool enableLightning) {
   MyTerrain = myTerrain;
   mInfra = infra;
   mEnableLightning = enableLightning;
   mLevelEditorMode = levelEditorMode;

   //this->m_texfile = texfile;
   mTexSource = textureSource;

   this->levelRes = levelRes;

   //just define to keep minimum 4 mesbuffers per textureId
   //for adding new cubes etc...
   mLevelEditorMinNrMeshBuffersNeeded = 4;

   mIrrMeshBuf = new IrrMeshBuf(mTexSource, mEnableLightning);
   mBlocksMeshStats = new MeshObjectStatsStruct();

   //initial fill the mBlockwCollMeshBufferVec and
   //mBlockwoCollMeshBufferVec vector with empty MeshBufferInfroStructs,
   //one for each possible level texture Id
   mIrrMeshBuf->InitializeMeshBufferInfoStructs(mBlockwCollMeshBufferVec);
   mIrrMeshBuf->InitializeMeshBufferInfoStructs(mBlockwoCollMeshBufferVec);

   ColumnsByPosition.clear();

   //create all buildings (column objects) out of the raw low level level data
   for(std::vector<ColumnsStruct>::iterator loopi = levelRes->Columns.begin(); loopi != levelRes->Columns.end(); ++loopi) {
       addColumn((*loopi).Columns, (*loopi).Vector3, levelRes);
   }

   segmentSize = DEF_SEGMENTSIZE;

   //update block definition usage count inside
   //blockdefinition objects
   UpdateBlockDefinitionUsageCnt();

   //generate Mesh with blocks
   //creates 2 meshes inside, one with collision detection
   //and one without (contains roof blocks (for example of tunnels) where player craft easily could get stuck
   //so we do not want to have collision detection for this ones)
   CreateBlocksMesh();

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
   sprintf(hlpstr, "%u", mBlocksMeshStats->numVertices);
   infoMsg.append(hlpstr);
   infoMsg.append(" vertices, ");

   //add number normals
   sprintf(hlpstr, "%u", mBlocksMeshStats->numNormals);
   infoMsg.append(hlpstr);
   infoMsg.append(" normals, ");

   //add number UVs
   sprintf(hlpstr, "%u", mBlocksMeshStats->numUVs);
   infoMsg.append(hlpstr);
   infoMsg.append(" UVs, ");

   //add number textures
   sprintf(hlpstr, "%d", mTexSource->NumLevelTextures);
   infoMsg.append(hlpstr);
   infoMsg.append(" textures, ");

   //add number indices
   sprintf(hlpstr, "%u", mBlocksMeshStats->numIndices);
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
bool LevelBlocks::SearchColumnWithPosition(int posKey, Column* &columnFnd) {
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
            if (SearchColumnWithPosition(key, fnd)) {
                 columns.push_back(fnd);
            }
        }
    }

    return columns;
}

void LevelBlocks::CreateBlocksMesh() {
    //create all buildings (column objects)
    std::vector<ColumnsByPositionStruct>::iterator loopi;
    ColumnsByPositionStruct GetColumn;
    std::vector<irr::u32> indiceOffset;
    irr::u8 cubeCnt;
    irr::u8 cubeIdx;

    //first create the building Mesh with
    //collision active
    blockMeshForCollision = new SMesh();
    blockMeshForCollision->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    std::vector<BlockInfoStruct*>::iterator it;

    for(loopi = ColumnsByPosition.begin(); loopi != ColumnsByPosition.end(); ++loopi) {
        GetColumn = (*loopi);
        cubeCnt = (irr::u8)(GetColumn.pColumn->mBlockInfoVec.size());
        cubeIdx = 0;

        for (it = GetColumn.pColumn->mBlockInfoVec.begin(); it != GetColumn.pColumn->mBlockInfoVec.end(); ++it) {
            //if collisionSelector = 1 then mesh contains all blocks
            //that are needed for collision detection
            if (GetColumn.pColumn->Definition->mInCollisionMesh[cubeIdx] == 1) {
                //we want collision detection for this block
                mIrrMeshBuf->AddMeshBufferBlock(mBlockwCollMeshBufferVec, (*it), *mBlocksMeshStats);
            }

            cubeIdx++;
        }
    }

    //get number of already existing Meshbuffers for all available Texture Ids of cubes with collision detection
    std::vector<irr::u8> nrMeshBuffersPerTexId = mIrrMeshBuf->ReturnMeshBufferCntPerTextureId(mBlockwCollMeshBufferVec);

    //if we are starting for the level editor we need to make sure that for each possible
    //texture Id existing we have enough meshbuffers available, so that the user is able to add more
    //cube faces later with a textureId that was not used before.
    //Because Irrlicht is limited to max 65535 indices per meshbuffer
    if (mLevelEditorMode) {
        irr::u8 buffersToAdd;

        int nrTextures = mIrrMeshBuf->GetNrTextures();

        //in for loop add additional "empty" meshbuffers
        for (int i = 0; i < nrTextures; i++) {
           buffersToAdd = mLevelEditorMinNrMeshBuffersNeeded - nrMeshBuffersPerTexId.at(i);

           for (int j = 0; j < buffersToAdd; j++) {
               mIrrMeshBuf->AddAdditionalMeshBuffer(mBlockwCollMeshBufferVec, i);
           }
        }
    }

    std::vector<irr::scene::SMeshBuffer*> bufList;
    std::vector<irr::scene::SMeshBuffer*>::iterator bufIt;

    int nrTextures = mIrrMeshBuf->GetNrTextures();

    for (int currTexId = 0; currTexId < nrTextures; currTexId++) {

        bufList = mIrrMeshBuf->ReturnAllMeshBuffersForTextureId(mBlockwCollMeshBufferVec, currTexId);

        for (bufIt = bufList.begin(); bufIt != bufList.end(); ++bufIt) {
              (*bufIt)->BoundingBox.reset(0,0,0);
              (*bufIt)->recalculateBoundingBox();

              //add SMeshbuffer to mesh
              blockMeshForCollision->addMeshBuffer((*bufIt));
              blockMeshForCollision->recalculateBoundingBox();
        }
   }

    //first create the building Mesh with
    //collision not active (unwanted)
    blockMeshWithoutCollision = new SMesh();
    blockMeshWithoutCollision->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    for(loopi = ColumnsByPosition.begin(); loopi != ColumnsByPosition.end(); ++loopi) {
        GetColumn = (*loopi);
        cubeCnt = (irr::u8)(GetColumn.pColumn->mBlockInfoVec.size());
        cubeIdx = 0;

        for (it = GetColumn.pColumn->mBlockInfoVec.begin(); it != GetColumn.pColumn->mBlockInfoVec.end(); ++it) {
            //if collisionSelector = 0 then mesh contains all blocks
            //that should not be included in collision detection
            if (GetColumn.pColumn->Definition->mInCollisionMesh[cubeIdx] == 0) {
                //we do not want collision detection for this block
                mIrrMeshBuf->AddMeshBufferBlock(mBlockwoCollMeshBufferVec, (*it), *mBlocksMeshStats);
            }

            cubeIdx++;
        }
    }

    //get number of already existing Meshbuffers for all available Texture Ids of cubes without collision detection
    nrMeshBuffersPerTexId = mIrrMeshBuf->ReturnMeshBufferCntPerTextureId(mBlockwoCollMeshBufferVec);

    //if we are starting for the level editor we need to make sure that for each possible
    //texture Id existing we have enough meshbuffers available, so that the user is able to add more
    //cube faces later with a textureId that was not used before.
    //Because Irrlicht is limited to max 65535 indices per meshbuffer
    if (mLevelEditorMode) {
        irr::u8 buffersToAdd;

        int nrTextures = mIrrMeshBuf->GetNrTextures();

        //in for loop add additional "empty" meshbuffers
        for (int i = 0; i < nrTextures; i++) {
           buffersToAdd = mLevelEditorMinNrMeshBuffersNeeded - nrMeshBuffersPerTexId.at(i);

           for (int j = 0; j < buffersToAdd; j++) {
               mIrrMeshBuf->AddAdditionalMeshBuffer(mBlockwoCollMeshBufferVec, i);
           }
        }
    }

    for (int currTexId = 0; currTexId < nrTextures; currTexId++) {

        bufList = mIrrMeshBuf->ReturnAllMeshBuffersForTextureId(mBlockwoCollMeshBufferVec, currTexId);

        for (bufIt = bufList.begin(); bufIt != bufList.end(); ++bufIt) {
              (*bufIt)->BoundingBox.reset(0,0,0);
              (*bufIt)->recalculateBoundingBox();

              //add SMeshbuffer to mesh
              blockMeshWithoutCollision->addMeshBuffer((*bufIt));
              blockMeshWithoutCollision->recalculateBoundingBox();
        }
   }

    //mark new mesh as dirty, so that it is transfered again to graphics card
    blockMeshForCollision->setDirty();
    blockMeshForCollision->recalculateBoundingBox();

    blockMeshWithoutCollision->setDirty();
    blockMeshWithoutCollision->recalculateBoundingBox();
}

std::vector<vector2d<irr::f32>> LevelBlocks::ApplyTexMod(vector2d<irr::f32> uvA, vector2d<irr::f32> uvB, vector2d<irr::f32> uvC, vector2d<irr::f32> uvD, int mod) {
   std::vector<vector2d<irr::f32>> uvs;

   switch (mod) {
        case 1: //RotateNoneFlipX
                uvs.push_back(uvA);    //is correct, did check
                uvs.push_back(uvB);
                uvs.push_back(uvC);
                uvs.push_back(uvD);
                break;
        case 2: //RotateNoneFlipY
                uvs.push_back(uvD);    //is correct, did check
                uvs.push_back(uvC);
                uvs.push_back(uvB);
                uvs.push_back(uvA);
                break;
        case 3: //Rotate180FlipNone   //is correct, did check
                uvs.push_back(uvB);
                uvs.push_back(uvA);
                uvs.push_back(uvD);
                uvs.push_back(uvC);
                break;
        case 4: //Rotate270FlipY
                uvs.push_back(uvD);    //is correct, did check
                uvs.push_back(uvC);
                uvs.push_back(uvB);
                uvs.push_back(uvA);
                break;
        case 5: //Rotate90FlipNone    //is not used in the first 6 levels, therefore I can not check if this
                uvs.push_back(uvD);    //order is correct :(
                uvs.push_back(uvA);
                uvs.push_back(uvB);
                uvs.push_back(uvC);
                break;
        case 6: //Rotate270FlipNone   //is not used in the first 6 levels, therefore I can not check if this
                uvs.push_back(uvB);    //order is correct :(
                uvs.push_back(uvC);
                uvs.push_back(uvD);
                uvs.push_back(uvA);
                break;
        case 7: //Rotate90FlipY      //is not used in the first 6 levels, therefore I can not check if this
                uvs.push_back(uvC);   //order is correct :(
                uvs.push_back(uvB);
                uvs.push_back(uvA);
                uvs.push_back(uvD);
                break;
        case 0:
        default:
                uvs.push_back(uvA); //is correct, did check
                uvs.push_back(uvB);
                uvs.push_back(uvC);
                uvs.push_back(uvD);
             break;
   }
   return uvs;
}

std::vector<vector2d<irr::f32>> LevelBlocks::MakeUVs(int texMod) {
    vector2d<irr::f32> uvA;
    vector2d<irr::f32> uvB;
    vector2d<irr::f32> uvC;
    vector2d<irr::f32> uvD;

    uvA.X = 1.0f;   uvA.Y = 0.0f;
    uvB.X = 0.0f;   uvB.Y = 0.0f;
    uvC.X = 0.0f;   uvC.Y = 1.0f;
    uvD.X = 1.0f;   uvD.Y = 1.0f;

    return ApplyTexMod(uvA, uvB, uvC, uvD, texMod);
}

void LevelBlocks::ChangeMeshCubeFaceHeight(BlockFaceInfoStruct* whichFace, irr::f32 newV1y, irr::f32 newV2y, irr::f32 newV3y, irr::f32 newV4y) {
    if (whichFace == nullptr)
        return;

    whichFace->currPositionVert1.Y = newV1y;
    whichFace->currPositionVert2.Y = newV2y;
    whichFace->currPositionVert3.Y = newV3y;
    whichFace->currPositionVert4.Y = newV4y;

    std::vector<irr::scene::SMeshBuffer*>::iterator it;
    irr::u32 idxMeshBuf;
    S3DVertex *pntrVertices;
    irr::u32 vertexIdx;

    idxMeshBuf = 0;

    for (it = whichFace->myMeshBuffers.begin(); it != whichFace->myMeshBuffers.end(); ++it) {
        (*it)->grab();
        void* pntrVert = (*it)->getVertices();
        pntrVertices = (S3DVertex*)pntrVert;
        vertexIdx = whichFace->myMeshBufVertexId[idxMeshBuf];

        pntrVertices[vertexIdx].Pos.Y = newV1y;
        pntrVertices[vertexIdx + 1].Pos.Y = newV2y;
        pntrVertices[vertexIdx + 2].Pos.Y = newV3y;
        pntrVertices[vertexIdx + 3].Pos.Y = newV4y;

        idxMeshBuf++;

        (*it)->drop();
    }
}

void LevelBlocks::ChangeMeshCubeHeight(BlockInfoStruct* whichCube, irr::f32 newV1y, irr::f32 newV2y, irr::f32 newV3y, irr::f32 newV4y) {
    if (whichCube == nullptr)
        return;

    //change the cube face vertice heights one by one
    ChangeMeshCubeFaceHeight(whichCube->fB, newV2y, newV1y, newV4y, newV3y);
    ChangeMeshCubeFaceHeight(whichCube->fT, newV3y + segmentSize, newV4y + segmentSize, newV1y + segmentSize, newV2y + segmentSize);
    ChangeMeshCubeFaceHeight(whichCube->fN, newV4y + segmentSize, newV3y + segmentSize, newV3y , newV4y);
    ChangeMeshCubeFaceHeight(whichCube->fE, newV1y + segmentSize, newV4y + segmentSize, newV4y, newV1y);
    ChangeMeshCubeFaceHeight(whichCube->fS, newV2y + segmentSize, newV1y + segmentSize, newV1y, newV2y);
    ChangeMeshCubeFaceHeight(whichCube->fW, newV3y + segmentSize, newV2y + segmentSize, newV2y, newV3y);
}

void LevelBlocks::UpdateBlockMesh() {
    blockMeshForCollision->setDirty(EBT_VERTEX);
    blockMeshWithoutCollision->setDirty(EBT_VERTEX);
}

void LevelBlocks::TestHeightChange(Column* selColumnPntr, int mSelBlockNrSkippingMissingBlocks) {
    BlockInfoStruct* blockInfo = GetBlockInfoStruct(selColumnPntr, mSelBlockNrSkippingMissingBlocks);

    if (blockInfo == nullptr)
         return;

    irr::f32 cV1 = blockInfo->fB->currPositionVert1.Y;
    irr::f32 cV2 = blockInfo->fB->currPositionVert2.Y;
    irr::f32 cV3 = blockInfo->fB->currPositionVert3.Y;
    irr::f32 cV4 = blockInfo->fB->currPositionVert4.Y;

    ChangeMeshCubeHeight(blockInfo, cV1 , cV2 , cV3   , cV4 );

    UpdateBlockMesh();
}

//returns nullptr if specified block does not exit or is invalid
BlockInfoStruct* LevelBlocks::GetBlockInfoStruct(Column* selColumnPntr, int mSelBlockNrSkippingMissingBlocks) {
    if (selColumnPntr == nullptr)
        return nullptr;

    if ((mSelBlockNrSkippingMissingBlocks < 0) || (mSelBlockNrSkippingMissingBlocks >= (int)(selColumnPntr->mBlockInfoVec.size())))
        return nullptr;

    BlockInfoStruct* blockInfo = selColumnPntr->mBlockInfoVec.at(mSelBlockNrSkippingMissingBlocks);

    return blockInfo;
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

void LevelBlocks::DrawOutlineSelectedFace(BlockFaceInfoStruct* selFace, SMaterial* color) {
    irr::core::vector3df pos1 = selFace->vert1->Pos;
    irr::core::vector3df pos2 = selFace->vert2->Pos;
    irr::core::vector3df pos3 = selFace->vert3->Pos;
    irr::core::vector3df pos4 = selFace->vert4->Pos;

    mInfra->mDrawDebug->Draw3DRectangle(pos1, pos2, pos3, pos4, color);
}

/***********************************************************************
 * Functions mainly used by the LevelEditor and not the game itself    *
 ***********************************************************************/

void LevelBlocks::DrawOutlineSelectedColumn(Column* selColumnPntr, int nrBlockFromBase, SMaterial* color, SMaterial* selFaceColor, irr::u8 selFace) {
    if (selColumnPntr == nullptr)
        return;

    if ((nrBlockFromBase < 0) || (nrBlockFromBase >= (int)(selColumnPntr->mBlockInfoVec.size())))
        return;

   BlockInfoStruct* blockInfo = selColumnPntr->mBlockInfoVec.at(nrBlockFromBase);

   if (blockInfo == nullptr)
        return;

    DrawOutlineSelectedFace(blockInfo->fN, color);
    DrawOutlineSelectedFace(blockInfo->fE, color);
    DrawOutlineSelectedFace(blockInfo->fS, color);
    DrawOutlineSelectedFace(blockInfo->fW, color);
    DrawOutlineSelectedFace(blockInfo->fB, color);
    DrawOutlineSelectedFace(blockInfo->fT, color);

    //if we are not specially mark a specified face
    //we are done, exit
    if (selFace == DEF_SELBLOCK_FACENONE)
        return;

    //mark also a possible selected face
    switch (selFace) {
         case DEF_SELBLOCK_FACENORTH:  { DrawOutlineSelectedFace(blockInfo->fN, selFaceColor); break;}
         case DEF_SELBLOCK_FACEEAST:   { DrawOutlineSelectedFace(blockInfo->fE, selFaceColor); break;}
         case DEF_SELBLOCK_FACESOUTH:  { DrawOutlineSelectedFace(blockInfo->fS, selFaceColor); break;}
         case DEF_SELBLOCK_FACEWEST:   { DrawOutlineSelectedFace(blockInfo->fW, selFaceColor); break;}
         case DEF_SELBLOCK_FACEBOTTOM: { DrawOutlineSelectedFace(blockInfo->fB, selFaceColor); break;}
         case DEF_SELBLOCK_FACETOP:    { DrawOutlineSelectedFace(blockInfo->fT, selFaceColor); break;}
         default:  { break;}
    }
}

void LevelBlocks::DrawColumnSelectionGrid(Column* selColumnPntr, irr::video::SMaterial* color) {
    if (selColumnPntr == nullptr)
        return;

    irr::core::vector3df v1 = selColumnPntr->mBaseVert1Coord;
    irr::core::vector3df v2 = selColumnPntr->mBaseVert2Coord;
    irr::core::vector3df v3 = selColumnPntr->mBaseVert3Coord;
    irr::core::vector3df v4 = selColumnPntr->mBaseVert4Coord;

    irr::core::vector3df v1h = v1 + irr::core::vector3df(0.0f, segmentSize, 0.0f);
    irr::core::vector3df v2h = v2 + irr::core::vector3df(0.0f, segmentSize, 0.0f);
    irr::core::vector3df v3h = v3 + irr::core::vector3df(0.0f, segmentSize, 0.0f);
    irr::core::vector3df v4h = v4 + irr::core::vector3df(0.0f, segmentSize, 0.0f);

    //draw the grid over all 8 possible cubes, it does not matter
    //if the exit (are assigned to a block definition) right now
    for (irr::u8 idx = 0; idx < 8; idx++) {
        //draw a 3D rectangle for the base
        this->mInfra->mDrawDebug->Draw3DRectangle(v1, v2, v3, v4, color);

        //and 4 lines at the cubes edges
        this->mInfra->mDrawDebug->Draw3DLine(v1, v1h, color);
        this->mInfra->mDrawDebug->Draw3DLine(v2, v2h, color);
        this->mInfra->mDrawDebug->Draw3DLine(v3, v3h, color);
        this->mInfra->mDrawDebug->Draw3DLine(v4, v4h, color);

        v1.Y += segmentSize;
        v2.Y += segmentSize;
        v3.Y += segmentSize;
        v4.Y += segmentSize;
        v1h.Y += segmentSize;
        v2h.Y += segmentSize;
        v3h.Y += segmentSize;
        v4h.Y += segmentSize;
    }

    //draw a last 3D rectangle for the top
    this->mInfra->mDrawDebug->Draw3DRectangle(v1, v2, v3, v4, color);
}

//Derives the current texturing information about a selected block face
//returns true if the information was found, false otherwise
bool LevelBlocks::GetTextureInfoSelectedBlock(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks,
                                              irr::u8 selFace, int16_t& outCurrTextureId, uint8_t& outCurrTextureModification) {
    if (selColumnPntr == nullptr)
        return false;

    BlockInfoStruct* blockInfo = selColumnPntr->mBlockInfoVec.at(mSelBlockNrSkippingMissingBlocks);

    if (blockInfo == nullptr)
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
         case DEF_SELBLOCK_FACENORTH:  { outCurrTextureId = blockDef->get_N(); outCurrTextureModification = blockDef->get_NMod(); break;}
         case DEF_SELBLOCK_FACEEAST:   { outCurrTextureId = blockDef->get_E(); outCurrTextureModification = blockDef->get_EMod(); break;}
         case DEF_SELBLOCK_FACESOUTH:  { outCurrTextureId = blockDef->get_S(); outCurrTextureModification = blockDef->get_SMod(); break;}
         case DEF_SELBLOCK_FACEWEST:   { outCurrTextureId = blockDef->get_W(); outCurrTextureModification = blockDef->get_WMod(); break;}
         case DEF_SELBLOCK_FACEBOTTOM: { outCurrTextureId = blockDef->get_B(); outCurrTextureModification = blockDef->get_BMod(); break;}
         case DEF_SELBLOCK_FACETOP:    { outCurrTextureId = blockDef->get_T(); outCurrTextureModification = blockDef->get_TMod(); break;}
         default:  { return false; break;}
    }

    return true;
}

void LevelBlocks::SetCubeFaceTexture(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks,
                                     irr::u8 selFace, bool updateTexId, int16_t newTextureId, bool updateTexMod, uint8_t newTextureMod) {
    //This higher level function has to do 2 independent things:
    // 1, modify the cube face configuration in the level/map file itself (so that next time we
    //    load the map again, everything is restored again in the same modified way)
    // 2, modify the current column block Mesh used by Irrlicht to show the user the
    //    current state of the column in the level. If we do not do this the level editor
    //    user can not see what he actually has changed already :)

    if (selColumnPntr == nullptr)
        return;

    BlockInfoStruct* blockInfo = selColumnPntr->mBlockInfoVec.at(mSelBlockNrSkippingMissingBlocks);
    ColumnDefinition* columDefStart = selColumnPntr->Definition;

    if (blockInfo == nullptr)
        return;

    //if we are not specially mark a specified face
    //we are done, exit
    if (selFace == DEF_SELBLOCK_FACENONE)
        return;

    BlockFaceInfoStruct* selFacePntr = nullptr;

    switch (selFace) {
         case DEF_SELBLOCK_FACENORTH:  { selFacePntr = blockInfo->fN; break;}
         case DEF_SELBLOCK_FACEEAST:   { selFacePntr = blockInfo->fE; break;}
         case DEF_SELBLOCK_FACESOUTH:  { selFacePntr = blockInfo->fS; break;}
         case DEF_SELBLOCK_FACEWEST:   { selFacePntr = blockInfo->fW; break;}
         case DEF_SELBLOCK_FACEBOTTOM: { selFacePntr = blockInfo->fB; break;}
         case DEF_SELBLOCK_FACETOP:    { selFacePntr = blockInfo->fT; break;}
         default:  { return; break;}
    }

    /******************************************************************
     * Part 1: Modify low level map data                              *
     ******************************************************************/

    //how does the modified block definition look like of this cube?

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
            return;
            break;
        }
    }

    //need to subtract one, to convert Block Id to
    //"vector index"
    blockValue--;

    if ((blockValue < 0) || (blockValue >= (int16_t)(levelRes->BlockDefinitions.size())))
            return;

    BlockDefinition *blockDef =  levelRes->BlockDefinitions.at(blockValue);

    uint8_t currN = blockDef->get_N();
    uint8_t currE = blockDef->get_E();
    uint8_t currS = blockDef->get_S();
    uint8_t currW = blockDef->get_W() ;
    uint8_t currT = blockDef->get_T();
    uint8_t currB = blockDef->get_B();

    uint8_t currNMod = blockDef->get_NMod();
    uint8_t currEMod = blockDef->get_EMod();
    uint8_t currSMod = blockDef->get_SMod();
    uint8_t currWMod = blockDef->get_WMod();
    uint8_t currTMod = blockDef->get_TMod();
    uint8_t currBMod = blockDef->get_BMod();

    int16_t currUnknown1 = blockDef->get_Unknown1();
    int16_t currUnknown2 = blockDef->get_Unknown2();

    //mark block definition which was used until now at this location
    //as a block definition that right now got unassigned, so that
    //we know the occurence of this block definition needs to be decreased by one
    blockDef->mState = DEF_BLOCKDEF_STATE_NEWLYUNASSIGNEDONE;

    uint8_t currTexMod;

    //Replace the selected cube face with the new textureId
    //to create the new needed block definition, so that we can search
    //and if necessary create it in the current level file
    switch (selFace) {
         case DEF_SELBLOCK_FACENORTH:  { if (updateTexId) { currN = newTextureId; } currTexMod = currNMod; if (updateTexMod) { currNMod = newTextureMod; } break;}
         case DEF_SELBLOCK_FACEEAST:   { if (updateTexId) { currE = newTextureId; } currTexMod = currEMod; if (updateTexMod) { currEMod = newTextureMod; } break;}
         case DEF_SELBLOCK_FACESOUTH:  { if (updateTexId) { currS = newTextureId; } currTexMod = currSMod; if (updateTexMod) { currSMod = newTextureMod; } break;}
         case DEF_SELBLOCK_FACEWEST:   { if (updateTexId) { currW = newTextureId; } currTexMod = currWMod; if (updateTexMod) { currWMod = newTextureMod; } break;}
         case DEF_SELBLOCK_FACEBOTTOM: { if (updateTexId) { currB = newTextureId; } currTexMod = currBMod; if (updateTexMod) { currBMod = newTextureMod; } break;}
         case DEF_SELBLOCK_FACETOP:    { if (updateTexId) { currT = newTextureId; } currTexMod = currTMod; if (updateTexMod) { currTMod = newTextureMod; } break;}
         default:  { return; break;}
    }

    irr::u32 outIndex;
    irr::u32 outColumnIndex;

    //DebugWriteBlockDefinitionTableToCsvFile((char*)("BlockDefBefore.csv"));
    //DebugWriteColumnDefinitionTableToCsvFile((char*)("ColumnDefBefore.csv"));
    //DebugWriteDefinedColumnsTableToCsvFile((char*)("ColumnsBefore.csv"));

    bool newlyAdded;

    //"Request" the new block definition
    if (!this->MyTerrain->levelRes->RequestBlockDefinition(currN, currE, currS, currW, currT, currB, currNMod, currEMod, currSMod, currWMod, currTMod, currBMod,
                                                           currUnknown1, currUnknown2, outIndex, newlyAdded)) {
        //unexpected error creating block definition, possibly we reached the maximum possible number
        //of 1024 block definitions in the levelfile?
        return;
    }

    //DebugWriteBlockDefinitionTableToCsvFile((char*)("BlockAfter1.csv"));

    BlockDefinition* newCorrBlockDef = this->levelRes->BlockDefinitions.at(outIndex);

    //Make sure (possibly now) unused Blockdefinitions are deleted
    //Because I am not sure if the game can handle them, so make sure we
    //do not have them
    RemoveUnusedBlockDefinitions();

    //after the possible reorganization of the block definition
    //vector, search the (new) correct block definition again
    //so that the index variable is corrected
    std::vector<BlockDefinition*>::iterator itBlockDef;

    outIndex = 0;
    bool found = false;

    for (itBlockDef = this->levelRes->BlockDefinitions.begin(); itBlockDef != this->levelRes->BlockDefinitions.end(); ++itBlockDef) {
        if ((*itBlockDef) == newCorrBlockDef) {
            found = true;
            break;
        }

         outIndex++;
    }

    if (!found) {
        return;
    }

    //set all block definition states to default state
    //we do not need this flag anymore
    for (itBlockDef = this->levelRes->BlockDefinitions.begin(); itBlockDef != this->levelRes->BlockDefinitions.end(); ++itBlockDef) {
       (*itBlockDef)->mState = DEF_BLOCKDEF_STATE_DEFAULT;
    }

    //The new or found block definition index is returned in outIndex
    //The new column we get, what block Id entries would it have?
    int16_t newColBlockIdA = selColumnPntr->Definition->get_A();
    int16_t newColBlockIdB = selColumnPntr->Definition->get_B();
    int16_t newColBlockIdC = selColumnPntr->Definition->get_C();
    int16_t newColBlockIdD = selColumnPntr->Definition->get_D();
    int16_t newColBlockIdE = selColumnPntr->Definition->get_E();
    int16_t newColBlockIdF = selColumnPntr->Definition->get_F();
    int16_t newColBlockIdG = selColumnPntr->Definition->get_G();
    int16_t newColBlockIdH = selColumnPntr->Definition->get_H();

    int16_t newFloorTex = selColumnPntr->Definition->get_FloorTextureID();
    int16_t newUnknown1 = selColumnPntr->Definition->get_Unknown1();

    //modify the block definition Id with the new one
    switch (nrBlockFromBase) {
        case 0: { newColBlockIdA = this->levelRes->BlockDefinitions.at(outIndex)->get_ID(); break; }
        case 1: { newColBlockIdB = this->levelRes->BlockDefinitions.at(outIndex)->get_ID(); break; }
        case 2: { newColBlockIdC = this->levelRes->BlockDefinitions.at(outIndex)->get_ID(); break; }
        case 3: { newColBlockIdD = this->levelRes->BlockDefinitions.at(outIndex)->get_ID(); break; }
        case 4: { newColBlockIdE = this->levelRes->BlockDefinitions.at(outIndex)->get_ID(); break; }
        case 5: { newColBlockIdF = this->levelRes->BlockDefinitions.at(outIndex)->get_ID(); break; }
        case 6: { newColBlockIdG = this->levelRes->BlockDefinitions.at(outIndex)->get_ID(); break; }
        case 7: { newColBlockIdH = this->levelRes->BlockDefinitions.at(outIndex)->get_ID(); break; }
        default: {
            return;
            break;
        }
    }

    bool newlyCreatedColumn;

    //See if there is already a Column definition / type that fits with this new one
    //If not, create a new column definition, and use it
    //"Request" the new column definition
    if (!this->MyTerrain->levelRes->RequestColumnDefinition(newFloorTex, newUnknown1, newColBlockIdA, newColBlockIdB,
                      newColBlockIdC, newColBlockIdD, newColBlockIdE, newColBlockIdF,
                      newColBlockIdG, newColBlockIdH, outColumnIndex, newlyCreatedColumn)) {
        //unexpected error creating column definition, possibly we reached the maximum possible number
        //of 1024 column definitions in the levelfile?
        return;
    }

    ColumnDefinition* newCorrColumnDef = this->levelRes->ColumnDefinitions.at(outColumnIndex);

    if (newlyCreatedColumn) {
            //mark column definition which was used until now at this location
            //as a column definition that right now got unassigned, so that
            //we know the occurence of this column definition needs to be decreased by one
            columDefStart->mState = DEF_COLUMNDEF_STATE_NEWLYUNASSIGNEDONE;
    }

    //Make sure (possibly now) unused Columndefinitions are deleted
    //Because I am not sure if the game can handle them, so make sure we
    //do not have them
    RemoveUnusedColumnDefinitions();
    //RemoveUnusedColumnDefinitions(newlyCreatedColumn, newCorrColumnDef->get_ID(), true, columDefStart->get_ID());

    //after the possible reorganization of the column definition
    //vector, search the (new) correct column definition again
    //so that the index variable is corrected
    std::vector<ColumnDefinition*>::iterator itColumnDef;

    outColumnIndex = 0;
    found = false;

    for (itColumnDef = this->levelRes->ColumnDefinitions.begin(); itColumnDef != this->levelRes->ColumnDefinitions.end(); ++itColumnDef) {
        if ((*itColumnDef) == newCorrColumnDef) {
            found = true;
            break;
        }

        outColumnIndex++;
    }

    if (!found) {
        return;
    }

    //reconfigure the selected column to use the new column definition Id
    selColumnPntr->Definition = this->levelRes->ColumnDefinitions.at(outColumnIndex);

    //set all column definition states to default state
    //we do not need this flag anymore
    for (itColumnDef = this->levelRes->ColumnDefinitions.begin(); itColumnDef != this->levelRes->ColumnDefinitions.end(); ++itColumnDef) {
       (*itColumnDef)->mState = DEF_COLUMNDEF_STATE_DEFAULT;
    }

    irr::core::vector2di columnOutputCoord;

    bool columnFound = FindMapCoordinateForColumn(selColumnPntr, columnOutputCoord);

    if (!columnFound)
        return;

    MapEntry* entry = this->levelRes->pMap[columnOutputCoord.X][columnOutputCoord.Y];

    if (entry == nullptr)
        return;

    entry->set_Column(this->levelRes->ColumnDefinitions.at(outColumnIndex));
    entry->WriteChanges();

    //we need to update specific additional column definitions values
    UpdateColumDefinitions();

    //we need also to update Blockdefinition usage count
    UpdateBlockDefinitionUsageCnt();

    //DebugWriteBlockDefinitionTableToCsvFile((char*)("BlockAfter2.csv"));
    //DebugWriteColumnDefinitionTableToCsvFile((char*)("ColumnDefAfter2.csv"));
    //DebugWriteDefinedColumnsTableToCsvFile((char*)("ColumnsAfter.csv"));

    /******************************************************************
     * Part 2: According to new texture modify Irrlicht column Mesh   *
     ******************************************************************/

    // Part 1: Update the current Irrlicht Mesh
    if (selColumnPntr->Definition->mInCollisionMesh[mSelBlockNrSkippingMissingBlocks] == 1) {
        //this block has collision detection, so we need to delete its mesh
        //from the mesh with collision detection
        if (updateTexId) {
            mIrrMeshBuf->RemoveMeshBufferCubeFace(mBlockwCollMeshBufferVec, selFacePntr, *mBlocksMeshStats);

            //setup new textureId the user has selected
            selFacePntr->textureId = newTextureId;

            //add back cube face mesh with the new textureId
            //so that the user sees the updated texture
            mIrrMeshBuf->AddMeshBufferCubeFace(mBlockwCollMeshBufferVec, selFacePntr, *mBlocksMeshStats);

            blockMeshForCollision->setDirty(EBT_VERTEX_AND_INDEX);
            blockMeshForCollision->recalculateBoundingBox();
        }
    } else {
        //no collision detection, use other buffer without collision detection
        if (updateTexId) {
            mIrrMeshBuf->RemoveMeshBufferCubeFace(mBlockwoCollMeshBufferVec, selFacePntr, *mBlocksMeshStats);

            //setup new textureId the user has selected
            selFacePntr->textureId = newTextureId;

            //add back cube face mesh with the new textureId
            //so that the user sees the updated texture
            mIrrMeshBuf->AddMeshBufferCubeFace(mBlockwoCollMeshBufferVec, selFacePntr, *mBlocksMeshStats);

            blockMeshWithoutCollision->setDirty(EBT_VERTEX_AND_INDEX);
            blockMeshWithoutCollision->recalculateBoundingBox();
        }
    }

    //if texture modification of cube face has also changed
    //update it in Irrlicht Mesh as well
    if (updateTexMod && (currTexMod != newTextureMod)) {
        UpdateCubeFaceTextureModification(selColumnPntr, mSelBlockNrSkippingMissingBlocks, selFacePntr, newTextureMod);
    }
}

void LevelBlocks::UpdateCubeFaceTextureModification(Column* selColumnPntr, int mSelBlockNrSkippingMissingBlocks,
                                                    BlockFaceInfoStruct* whichFace, uint8_t newTextureModifier) {
    //This function only needs to modify the current column/cube Mesh used by Irrlicht to show the user the
    //    current state of the new cube texture in the level. The modification of the level map
    //    file data is already handeled in the calling function

    //check for valid value of new texture modifier
    if ((newTextureModifier < 0) || (newTextureModifier > 7))
        return;

    std::vector<irr::scene::SMeshBuffer*>::iterator it;
    irr::scene::SMeshBuffer* meshBufPntr;

    //create the new UV information
    std::vector<vector2d<irr::f32>> newUVS = MakeUVs(newTextureModifier);

    irr::u32 vert1Idx;

    void* pntrVert;
    S3DVertex *pntrVertices;

    std::vector<irr::u32>::iterator itVertId1 = whichFace->myMeshBufVertexId.begin();

    irr::u32 bufIdx = 0;

    //iterate through all meshbuffers where this cube face is included, and
    //see if we find any of the indices of the vertices of this cube face included
    for (it = whichFace->myMeshBuffers.begin(); it != whichFace->myMeshBuffers.end(); ++it) {
        //what indices do my vertices have in this meshbuffer?
        vert1Idx = (*itVertId1);

        meshBufPntr = (*it);
        meshBufPntr->grab();

        pntrVert = meshBufPntr->getVertices();
        pntrVertices = (S3DVertex*)pntrVert;
        pntrVertices[whichFace->myMeshBufVertexId[bufIdx]].TCoords = newUVS.at(0);
        pntrVertices[whichFace->myMeshBufVertexId[bufIdx] + 1].TCoords = newUVS.at(1);
        pntrVertices[whichFace->myMeshBufVertexId[bufIdx] + 2].TCoords = newUVS.at(2);
        pntrVertices[whichFace->myMeshBufVertexId[bufIdx] + 3].TCoords = newUVS.at(3);

        bufIdx++;

        meshBufPntr->drop();

        meshBufPntr->setDirty(EBT_VERTEX_AND_INDEX);
    }

    if (selColumnPntr->Definition->mInCollisionMesh[mSelBlockNrSkippingMissingBlocks] == 1) {
        blockMeshForCollision->setDirty(EBT_VERTEX_AND_INDEX);
        blockMeshForCollision->recalculateBoundingBox();
    } else {
        blockMeshWithoutCollision->setDirty(EBT_VERTEX_AND_INDEX);
        blockMeshWithoutCollision->recalculateBoundingBox();
    }
}

void LevelBlocks::RemoveMeshCube(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks) {
    if (selColumnPntr == nullptr)
        return;

    BlockInfoStruct* blockInfo = selColumnPntr->mBlockInfoVec.at(mSelBlockNrSkippingMissingBlocks);

    if (blockInfo == nullptr)
        return;

    std::vector<MeshBufferInfoStruct*>* targetMeshBufVec;

    if (selColumnPntr->Definition->mInCollisionMesh[mSelBlockNrSkippingMissingBlocks] == 1) {
        //cube to delete has collision detection,
        //we need to take meshbuffers for cubes with collision detection
        targetMeshBufVec = &mBlockwCollMeshBufferVec;
    } else {
        //no collision detection, use other buffer without collision detection
        targetMeshBufVec = &mBlockwoCollMeshBufferVec;
    }

    //remove all 6 sides of the specified cube
    mIrrMeshBuf->RemoveMeshBufferCubeFace(*targetMeshBufVec, blockInfo->fB, *mBlocksMeshStats);
    mIrrMeshBuf->RemoveMeshBufferCubeFace(*targetMeshBufVec, blockInfo->fT, *mBlocksMeshStats);
    mIrrMeshBuf->RemoveMeshBufferCubeFace(*targetMeshBufVec, blockInfo->fN, *mBlocksMeshStats);
    mIrrMeshBuf->RemoveMeshBufferCubeFace(*targetMeshBufVec, blockInfo->fE, *mBlocksMeshStats);
    mIrrMeshBuf->RemoveMeshBufferCubeFace(*targetMeshBufVec, blockInfo->fS, *mBlocksMeshStats);
    mIrrMeshBuf->RemoveMeshBufferCubeFace(*targetMeshBufVec, blockInfo->fW, *mBlocksMeshStats);

    if (selColumnPntr->Definition->mInCollisionMesh[mSelBlockNrSkippingMissingBlocks] == 1) {
        blockMeshForCollision->setDirty(EBT_VERTEX_AND_INDEX);
        blockMeshForCollision->recalculateBoundingBox();
    } else {
        blockMeshWithoutCollision->setDirty(EBT_VERTEX_AND_INDEX);
        blockMeshWithoutCollision->recalculateBoundingBox();
    }
}

void LevelBlocks::UpdateBlockDefinitionUsageCnt() {
    std::vector<irr::u32> usagecnt = GetBlockDefinitionUsageCount();

    //how many different Blockdefinitions do we have currently?
    size_t nrBlockDef = this->levelRes->BlockDefinitions.size();

    for (size_t idx = 0; idx < nrBlockDef; idx++) {
        //set current block definition usage count inside the block definition
        //object
        this->levelRes->BlockDefinitions.at(idx)->usageCnt = usagecnt[idx];
    }
}

//Returns a vector which contains an element for each currently existing
//Block definition, and how often it is used on the current map
std::vector<irr::u32> LevelBlocks::GetBlockDefinitionUsageCount() {
    std::vector<irr::u32> cntResult;

    cntResult.clear();

    //how many different Blockdefinitions do we have currently?
    irr::u32 nrBlockDef = this->levelRes->BlockDefinitions.size();

    cntResult.resize(nrBlockDef);

    //loop through all existing columns, and cube
    std::vector<ColumnDefinition*>::iterator itCol;

    int16_t val;

    for (itCol = this->levelRes->ColumnDefinitions.begin(); itCol != this->levelRes->ColumnDefinitions.end(); ++itCol) {
        val = (*itCol)->get_A();

        if (val != 0) {
            if ((val > 0) && (val <= (int16_t)(nrBlockDef))) {
                cntResult.at(val - 1) += 1;
            }
        }

        val = (*itCol)->get_B();

        if (val != 0) {
            if ((val > 0) && (val <= (int16_t)(nrBlockDef))) {
                cntResult.at(val - 1) += 1;
            }
        }

        val = (*itCol)->get_C();

        if (val != 0) {
            if ((val > 0) && (val <= (int16_t)(nrBlockDef))) {
                cntResult.at(val - 1) += 1;
            }
        }

        val = (*itCol)->get_D();

        if (val != 0) {
            if ((val > 0) && (val <= (int16_t)(nrBlockDef))) {
                cntResult.at(val - 1) += 1;
            }
        }

        val = (*itCol)->get_E();

        if (val != 0) {
            if ((val > 0) && (val <= (int16_t)(nrBlockDef))) {
                cntResult.at(val - 1) += 1;
            }
        }

        val = (*itCol)->get_F();

        if (val != 0) {
            if ((val > 0) && (val <= (int16_t)(nrBlockDef))) {
                cntResult.at(val - 1) += 1;
            }
        }

        val = (*itCol)->get_G();

        if (val != 0) {
            if ((val > 0) && (val <= (int16_t)(nrBlockDef))) {
                cntResult.at(val - 1) += 1;
            }
        }

        val = (*itCol)->get_H();

        if (val != 0) {
            if ((val > 0) && (val <= (int16_t)(nrBlockDef))) {
                cntResult.at(val - 1) += 1;
            }
        }
    }

    return (cntResult);
}

void LevelBlocks::ReplaceBlockDefinitionIdWithNewOneInAllColumdefinitions() {
    //loop through all currently existing columndefinitions
    std::vector<ColumnDefinition*>::iterator itCol;

    int16_t newA;
    int16_t newB;
    int16_t newC;
    int16_t newD;
    int16_t newE;
    int16_t newF;
    int16_t newG;
    int16_t newH;

    std::vector<BlockDefinition*>::iterator itBlock;

    for (itCol = this->levelRes->ColumnDefinitions.begin(); itCol != this->levelRes->ColumnDefinitions.end(); ++itCol) {

        newA = (*itCol)->get_A();
        newB = (*itCol)->get_B();
        newC = (*itCol)->get_C();
        newD = (*itCol)->get_D();
        newE = (*itCol)->get_E();
        newF = (*itCol)->get_F();
        newG = (*itCol)->get_G();
        newH = (*itCol)->get_H();

        for (itBlock = this->levelRes->BlockDefinitions.begin(); itBlock != this->levelRes->BlockDefinitions.end(); ++itBlock) {

             //if the block Id of this block did not change during last block definition list redefinition
             //go to the next block
             if ((*itBlock)->m_initialID == (*itBlock)->get_ID())
                 continue;

             //if this is a newly added block definition, also skip
             if ((*itBlock)->mState == DEF_BLOCKDEF_STATE_NEWLYADDED_KEEP)
                 continue;

              //see if value of A was pointing to the old Id of this
              //block definition
              if ((newA != 0) && ((*itBlock)->m_initialID == newA)) {
                        //assign the new Id for this block after
                        //block definition reorganization
                        newA = (*itBlock)->get_ID();
              }

              //see if value of B was pointing to the old Id of this
              //block definition
              if ((newB != 0) && ((*itBlock)->m_initialID == newB)) {
                        //assign the new Id for this block after
                        //block definition reorganization
                        newB = (*itBlock)->get_ID();
              }

              //see if value of C was pointing to the old Id of this
              //block definition
              if ((newC != 0) && ((*itBlock)->m_initialID == newC)) {
                        //assign the new Id for this block after
                        //block definition reorganization
                        newC = (*itBlock)->get_ID();
              }

              //see if value of D was pointing to the old Id of this
              //block definition
              if ((newD != 0) && ((*itBlock)->m_initialID == newD)) {
                        //assign the new Id for this block after
                        //block definition reorganization
                        newD = (*itBlock)->get_ID();
              }

              //see if value of E was pointing to the old Id of this
              //block definition
              if ((newE != 0) && ((*itBlock)->m_initialID == newE)) {
                        //assign the new Id for this block after
                        //block definition reorganization
                        newE = (*itBlock)->get_ID();
              }

              //see if value of F was pointing to the old Id of this
              //block definition
              if ((newF != 0) && ((*itBlock)->m_initialID == newF)) {
                        //assign the new Id for this block after
                        //block definition reorganization
                        newF = (*itBlock)->get_ID();
              }

              //see if value of G was pointing to the old Id of this
              //block definition
              if ((newG != 0) && ((*itBlock)->m_initialID == newG)) {
                        //assign the new Id for this block after
                        //block definition reorganization
                        newG = (*itBlock)->get_ID();
              }

              //see if value of H was pointing to the old Id of this
              //block definition
              if ((newH != 0) && ((*itBlock)->m_initialID == newH)) {
                        //assign the new Id for this block after
                        //block definition reorganization
                        newH = (*itBlock)->get_ID();
             }
    }

    //now assign back possible modified values
    (*itCol)->set_A(newA);
    (*itCol)->set_B(newB);
    (*itCol)->set_C(newC);
    (*itCol)->set_D(newD);
    (*itCol)->set_E(newE);
    (*itCol)->set_F(newF);
    (*itCol)->set_G(newG);
    (*itCol)->set_H(newH);
  }
}

void LevelBlocks::RemoveUnusedBlockDefinitions() {
    std::string infoMsg("");
    char hlpstr[100];

    std::vector<BlockDefinition*>::iterator itBlock;
    BlockDefinition* toDelete;

    for (itBlock = MyTerrain->levelRes->BlockDefinitions.begin(); itBlock != MyTerrain->levelRes->BlockDefinitions.end(); ++itBlock) {
        //for later easier update of column definitions store original
        //Id for every block definition before we modify it possibly
        (*itBlock)->m_initialID = (*itBlock)->get_ID();

        //if this block definition is marked as newly unassigned
        //decrease its occurence variable by one
        if ((*itBlock)->mState == DEF_BLOCKDEF_STATE_NEWLYUNASSIGNEDONE) {
            (*itBlock)->usageCnt--;

            infoMsg.clear();
            infoMsg.append("Decrease usage count of block definition with Id = ");

            //add id
            sprintf(hlpstr, "%d", (*itBlock)->get_ID());

            infoMsg.append(hlpstr);
            logging::Info(infoMsg);
        }
    }

    //remove each entry with 0 usage, as long as it was not newly added just before this function call
    //we know this based on the block definition internal state variable
    for (itBlock = MyTerrain->levelRes->BlockDefinitions.begin(); itBlock != MyTerrain->levelRes->BlockDefinitions.end(); ) {
        if (((*itBlock)->usageCnt == 0) && ((*itBlock)->mState != DEF_BLOCKDEF_STATE_NEWLYADDED_KEEP)) {
            //an unused BlockDefinition entry, remove it

            toDelete = (*itBlock);

            infoMsg.clear();
            infoMsg.append("Removing unused block definition with Id = ");

            //add id
            sprintf(hlpstr, "%d", (*itBlock)->get_ID());

            infoMsg.append(hlpstr);
            logging::Info(infoMsg);

            //remove blockDefinition vector entry
            itBlock = MyTerrain->levelRes->BlockDefinitions.erase(itBlock);

            //delete the blockdefinition struct itself
            delete toDelete;
        } else {
            itBlock++;
        }
     }

    irr::u32 currId = 1;
    int baseOffset;

    //now all unused block definitions are removed, now it is important to reassign the Block
    //definition Ids in the table, so that all Ids stays in order, increasing one by one for each element
    for (itBlock = MyTerrain->levelRes->BlockDefinitions.begin(); itBlock != MyTerrain->levelRes->BlockDefinitions.end(); ++itBlock) {
            //set new Id for the next Block definition
            (*itBlock)->set_ID(currId);

            //do not forget to also update Offset in file which depends
            //on new Id!
            baseOffset = 124636 + currId * 16;
            (*itBlock)->set_Offset(baseOffset);

            currId++;
    }

    //need to update all links to the old Block Id in all columdefinitions of the level
    //to the newly assigned block definition Id
    ReplaceBlockDefinitionIdWithNewOneInAllColumdefinitions();
}

//17.07.2025: Best function until now, but I believe it has bugs and is too complicated!
/*void LevelBlocks::RemoveUnusedBlockDefinitions(bool excludeActive, irr::u32 excludeId, bool reduceCntByOneForIdActive, irr::u32 reduceCntByOneForId) {
    std::string infoMsg("");
    char hlpstr[100];
    BlockDefinition* whichDefExclude = nullptr;
    BlockDefinition* otherCheck = nullptr;

    if (excludeActive) {
        otherCheck = levelRes->GetBlockDefinitionWithCertainId(excludeId);
    }

    if (reduceCntByOneForIdActive) {
        BlockDefinition* whichDef = levelRes->GetBlockDefinitionWithCertainId(reduceCntByOneForId);

        //if we simply reuse the same block, do not decrease the mOccurence
        //so that we do not delete the block we reuse
        if ((whichDef != nullptr) && (whichDef != otherCheck)) {
                if (whichDef->usageCnt > 0) {
                    whichDef->usageCnt -= 1;

                    infoMsg.clear();
                    infoMsg.append("Block: Reduce usage cnt by one for Id = ");

                    //add id
                    sprintf(hlpstr, "%d", whichDef->get_ID());

                    infoMsg.append(hlpstr);
                    logging::Info(infoMsg);
                }
        }
    }

    if (excludeActive) {
        whichDefExclude = levelRes->GetBlockDefinitionWithCertainId(excludeId);
    }

    std::vector<BlockDefinition*>::iterator itBlock;
    BlockDefinition* toDelete;
    int oldId;

    irr::u32 currId = 1;
    int baseOffset;

    //std::vector<irr::u32> changeFromId;
    //std::vector<irr::u32> changeToId;

    bool keepUpdatingBlockIds = false;

    //changeFromId.clear();
    //changeToId.clear();

    for (itBlock = MyTerrain->levelRes->BlockDefinitions.begin(); itBlock != MyTerrain->levelRes->BlockDefinitions.end(); ++itBlock) {
        //for later easier update of column definitions store original
        //Id for every block definition before we modify it possibly
        (*itBlock)->m_initialID = (*itBlock)->get_ID();
    }

    //remove each entry with 0 usage
    for (itBlock = MyTerrain->levelRes->BlockDefinitions.begin(); itBlock != MyTerrain->levelRes->BlockDefinitions.end(); ) {
        if ((!(excludeActive && ((*itBlock) == whichDefExclude))) && ((*itBlock)->usageCnt == 0)) {
            //an unused BlockDefinition entry, remove it

            toDelete = (*itBlock);

            //keep old Id for later
            oldId = (*itBlock)->get_ID();

            infoMsg.clear();
            infoMsg.append("Removing unused block definition with Id = ");

            //add id
            sprintf(hlpstr, "%d", oldId);

            infoMsg.append(hlpstr);
            logging::Info(infoMsg);

            //remove blockDefinition vector entry
            itBlock = MyTerrain->levelRes->BlockDefinitions.erase(itBlock);

            //delete the blockdefinition struct itself
            delete toDelete;

            //from now on we need to update every entry
            //in the table that still follows to make sure all
            //block Ids are properly aligned (increasing by one again)
            keepUpdatingBlockIds = true;
        } else {

            //if this block should be excluded from erasing, leave it alone and go to
            //the next element; This is true for example for a newly added block definition
            //that is not assigned yet
            if (excludeActive) {
                if ((*itBlock) == whichDefExclude) {
                    infoMsg.clear();
                    infoMsg.append("Skip erasing block definition with Id = ");

                    //add id
                    sprintf(hlpstr, "%d", (*itBlock)->get_ID());

                    infoMsg.append(hlpstr);
                    logging::Info(infoMsg);
                }
            }
        }

          if (keepUpdatingBlockIds) {
                //because we at least erased one block definition entry before,
                //this next elements Id needs now to be adjusted down as well,
                //so that all Ids stays in order, increasing one by one for each element
                oldId = (*itBlock)->get_ID();

                //the next element gets the new (current Id)
                //remember which Id we replaced with which new Id
                //changeFromId.push_back(oldId);
                //changeToId.push_back(currId);

                infoMsg.clear();
                infoMsg.append("Modify block definition Id from = ");

                //add id
                sprintf(hlpstr, "%d", oldId);
                infoMsg.append(hlpstr);

                infoMsg.append(" to = ");
                sprintf(hlpstr, "%d", currId);
                infoMsg.append(hlpstr);

                logging::Info(infoMsg);

                //set new Id for the next Block definition
                (*itBlock)->set_ID(currId);

                //do not forget to also update Offset in file which depends
                //on new Id!
                baseOffset = 124636 + currId * 16;
                (*itBlock)->set_Offset(baseOffset);
            }

            itBlock++;
            currId++;
    }

    //need to update all links to the old Block Id in all columdefinitions of the level
    //to the newly assigned block definition Id
    ReplaceBlockDefinitionIdWithNewOneInAllColumdefinitions();
}*/

//Returns a vector which contains an element for each currently existing
//Column definition, and how often it is used on the current map
//(how many column use this column definition)
std::vector<irr::u32> LevelBlocks::GetColumnDefinitionUsageCount() {
    std::vector<irr::u32> cntResult;

    cntResult.clear();

    //how many different Columndefinitions do we have currently?
    irr::u32 nrColumnDef = this->levelRes->ColumnDefinitions.size();

    cntResult.resize(nrColumnDef);

    //loop through all existing columns
    std::vector<ColumnsByPositionStruct>::iterator itCol;

    int val;

    for (itCol = this->ColumnsByPosition.begin(); itCol != this->ColumnsByPosition.end(); ++itCol) {
        val = (*itCol).pColumn->Definition->get_ID();

        if (val != 0) {
            if ((val > 0) && (val <= (int)(nrColumnDef))) {
                cntResult.at(val - 1) += 1;
            }
        }
    }

    return (cntResult);
}

//update certain values stored in the column definitions
//that also the original game uses
void LevelBlocks::UpdateColumDefinitions() {
    //first important value that is stored inside
    //the column definitions as well is the occurence (usage)
    //for each column definition, we need to keep this value updated
    std::vector<irr::u32> usage = GetColumnDefinitionUsageCount();

    std::vector<ColumnDefinition*>::iterator it;
    irr::u32 idx = 0;
    irr::u8 newShape;

    for (it = this->levelRes->ColumnDefinitions.begin(); it != this->levelRes->ColumnDefinitions.end(); ++it) {
        (*it)->set_Occurence((int16_t)(usage.at(idx)));

        idx++;

        //keep shape up to date
        newShape = GetColumnDefinitionShapeValue(*it);
        (*it)->set_Shape(newShape);
    }
}

//Returns simply 0 value if columnDefPntr is nullptr!
irr::u8 LevelBlocks::GetColumnDefinitionShapeValue(ColumnDefinition* columnDefPntr) {
    if (columnDefPntr == nullptr)
        return 0;

    irr::u8 result = 0;

    if (columnDefPntr->get_A() != 0)
        result |= 1;

    if (columnDefPntr->get_B() != 0)
        result |= 2;

    if (columnDefPntr->get_C() != 0)
        result |= 4;

    if (columnDefPntr->get_D() != 0)
        result |= 8;

    if (columnDefPntr->get_E() != 0)
        result |= 16;

    if (columnDefPntr->get_F() != 0)
        result |= 32;

    if (columnDefPntr->get_G() != 0)
        result |= 64;

    if (columnDefPntr->get_H() != 0)
        result |= 128;

    return result;
}

std::string LevelBlocks::CreateDbgShapeString(ColumnDefinition* colDef) {
    std::string resultStr("");

    if (colDef == nullptr)
        return resultStr;

    irr::u8 value = GetColumnDefinitionShapeValue(colDef);
    irr::u8 div = 128;

    for (irr::u8 idx = 0; idx < 8; idx++) {
        if ((value & div) == div) {
            resultStr.append("1");
        } else {
            resultStr.append("0");
        }

        div /= 2;
    }

    return resultStr;
}

void LevelBlocks::DebugWriteColumnDefinitionTableToCsvFile(char* debugOutPutFileName) {
   FILE* debugOutputFile = nullptr;

   debugOutputFile = fopen(debugOutPutFileName, "w");
   if (debugOutputFile == nullptr) {
         return;
   }

   std::vector<ColumnDefinition*>::iterator it;

   //write a header
   fprintf(debugOutputFile, "ColumnId;Shape;FloorTexId;Unknown1;A;B;C;D;E;F;G;H;Offset;Occurence\n");

   for (it = levelRes->ColumnDefinitions.begin(); it != levelRes->ColumnDefinitions.end(); ++it) {
        //write the next entry
        fprintf(debugOutputFile, "%d;%s;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
                (*it)->get_ID(), CreateDbgShapeString(*it).c_str(), (*it)->get_FloorTextureID(), (*it)->get_Unknown1(),
                (*it)->get_A(), (*it)->get_B(), (*it)->get_C(),  (*it)->get_D(), (*it)->get_E(),
                (*it)->get_F(),  (*it)->get_G(), (*it)->get_H(), (*it)->get_Offset(), (*it)->get_Occurence());
   }

   fclose(debugOutputFile);
}

void LevelBlocks::DebugWriteBlockDefinitionTableToCsvFile(char* debugOutPutFileName) {
   FILE* debugOutputFile = nullptr;

   debugOutputFile = fopen(debugOutPutFileName, "w");
   if (debugOutputFile == nullptr) {
         return;
   }

   std::vector<BlockDefinition*>::iterator it;

   //write a header
   fprintf(debugOutputFile, "BlockId;N;E;S;W;B;T;NMod;EMod;SMod;WMod;BMod;TMod;Unknown1;Unknown2;Offset;Occurence\n");

   for (it = levelRes->BlockDefinitions.begin(); it != levelRes->BlockDefinitions.end(); ++it) {

        //write the next entry
        fprintf(debugOutputFile, "%d;%u;%u;%u;%u;%u;%u;%u;%u;%u;%u;%u;%u;%d;%d;%d;%d\n",
                (*it)->get_ID(), (*it)->get_N(), (*it)->get_E(), (*it)->get_S(),
                (*it)->get_W(),  (*it)->get_B(), (*it)->get_T(), (*it)->get_NMod(),
                (*it)->get_EMod(), (*it)->get_SMod(), (*it)->get_WMod(), (*it)->get_BMod(),
                (*it)->get_TMod(), (*it)->get_Unknown1(), (*it)->get_Unknown2(),
                (*it)->get_Offset(), (*it)->usageCnt);
   }

   fclose(debugOutputFile);
}

void LevelBlocks::DebugWriteDefinedColumnsTableToCsvFile(char* debugOutPutFileName) {
   FILE* debugOutputFile = nullptr;

   debugOutputFile = fopen(debugOutPutFileName, "w");
   if (debugOutputFile == nullptr) {
         return;
   }

   std::vector<ColumnsByPositionStruct>::iterator it;
   int nrColumn = 1;
   irr::core::vector2di columnPos;
   Column* colPntr;

   //write a header
   fprintf(debugOutputFile, "Column Nr;Cell X;Cell Y;ColumnDef Id;Nr Blocks;PosValue\n");

   for (it = ColumnsByPosition.begin(); it != ColumnsByPosition.end(); ++it) {
        colPntr = (*it).pColumn;

        if (colPntr == nullptr)
            continue;

        columnPos.X = (int)(colPntr->Position.X);
        columnPos.Y = (int)(colPntr->Position.Z);

        //write the next entry
        fprintf(debugOutputFile, "%d;%d;%d;%d;%u;%d\n",
                nrColumn, columnPos.X, columnPos.Y, (*it).pColumn->Definition->get_ID(),
                (*it).pColumn->GetNumberContainedBlocks(),  (*it).pos);

        nrColumn++;
   }

   fclose(debugOutputFile);
}

void LevelBlocks::ReplaceColumnDefinitionWithNewOneForAllColumns(std::vector<irr::u32> changeFromIdVec, std::vector<irr::u32> changeToIdVec) {
    //how many different columndefinitions do we have currently?
    irr::u32 nrColumnDef = this->levelRes->ColumnDefinitions.size();

    size_t entriesToChange = changeFromIdVec.size();

    std::string infoMsg("");
    char hlpstr[100];

    int oldId;
    int newId;
    ColumnDefinition* oldDef;
    ColumnDefinition* newDef;

    //int val;

    bool colMod;

    int height = levelRes->Height();
    int width = levelRes->Width();

    MapEntry* entry;

    //iterate through tile map, and fix all columndefinitions
    for (int y = 0; y < height; y++) {
     for (int x = 0; x < width; x++) {
          entry = levelRes->pMap[x][y];

          if (entry == nullptr)
              continue;

          colMod = false;

          for (size_t idxEntry = 1; idxEntry < entriesToChange; idxEntry++) {
              oldId = (int)(changeFromIdVec.at(idxEntry));
              newId = (int)(changeToIdVec.at(idxEntry));

             /* oldDef = this->levelRes->GetColumnDefinitionWithCertainId(oldId);
              newDef = this->levelRes->GetColumnDefinitionWithCertainId(newId);

              if ((oldDef == nullptr) || (newDef == nullptr))
                  continue;*/

              //if a new column was added at the end of the list, then
              //there will be an entry more at the end of changeFromIdVec that
              //contains an change from Id that would be too long for the ColumnDefinitions vector
              //if this happens interrupt this entry, we do not need it, because there is for sure not
              //yet a column with this newly created columndefinitionId that we would have to update
              if ((oldId - 1) >= nrColumnDef)
                  continue;

              oldDef = this->levelRes->ColumnDefinitions.at(oldId - 1);
              newDef = this->levelRes->ColumnDefinitions.at(newId - 1);

              if ((!colMod) && (entry->get_Column() == oldDef)) {
                  infoMsg.clear();
                  infoMsg.append("Reassign Column at MapEntry X = ");

                  //add x coord
                  sprintf(hlpstr, "%d", x);

                  infoMsg.append(hlpstr);
                  infoMsg.append(" Y = ");

                  //add y coord
                  sprintf(hlpstr, "%d", y);
                  infoMsg.append(hlpstr);

                  infoMsg.append(" from Id = ");

                  //add old Id
                  sprintf(hlpstr, "%d", oldDef->get_ID());
                  infoMsg.append(hlpstr);

                  infoMsg.append(" to Id = ");

                  //add new Id
                  sprintf(hlpstr, "%d", newDef->get_ID());
                  infoMsg.append(hlpstr);

                  logging::Info(infoMsg);

                  entry->set_Column(newDef);

                  //update entry data, which will convert the new Id for this colum definition
                  //into the correct cid value inside this MapEntry
                  entry->WriteChanges();

                  colMod = true;
              }
         }
     }
   }
}

void LevelBlocks::RemoveUnusedColumnDefinitions() {
    std::string infoMsg("");
    char hlpstr[100];
    std::vector<ColumnDefinition*>::iterator itCol;
    ColumnDefinition* toDelete;

    for (itCol = MyTerrain->levelRes->ColumnDefinitions.begin(); itCol != MyTerrain->levelRes->ColumnDefinitions.end(); ++itCol) {
        //for easier debugging etc. keep column definitions original
        //Id for every column definition before we modify it possibly
        (*itCol)->m_initialID = (*itCol)->get_ID();

        //if this column definition is marked as newly unassigned
        //decrease its occurence variable by one
        if ((*itCol)->mState == DEF_COLUMNDEF_STATE_NEWLYUNASSIGNEDONE) {
            (*itCol)->set_Occurence((*itCol)->get_Occurence() - 1);

            infoMsg.clear();
            infoMsg.append("Decrease mOccurence of column definition with Id = ");

            //add id
            sprintf(hlpstr, "%d", (*itCol)->get_ID());

            infoMsg.append(hlpstr);
            logging::Info(infoMsg);
        }
    }

    //remove each entry with 0 usage, as long as it was not newly added just before this function call
    //we know this based on the column definition internal state variable
    for (itCol = MyTerrain->levelRes->ColumnDefinitions.begin(); itCol != MyTerrain->levelRes->ColumnDefinitions.end(); ) {
        if (((*itCol)->get_Occurence() == 0) && ((*itCol)->mState != DEF_COLUMNDEF_STATE_NEWLYADDED_KEEP)) {
            //an unused ColumnDefinition entry, remove it

            toDelete = (*itCol);

            infoMsg.clear();
            infoMsg.append("Removing unused column definition with Id = ");

            //add id
            sprintf(hlpstr, "%d", (*itCol)->get_ID());

            infoMsg.append(hlpstr);
            logging::Info(infoMsg);

            //remove ColumnDefinition vector entry
            itCol = MyTerrain->levelRes->ColumnDefinitions.erase(itCol);

            //delete the columndefinition struct itself
            delete toDelete;
        } else {
            itCol++;
        }
     }

    irr::u32 currId = 1;
    int baseOffset;

    //now all unused column definitions are removed, now it is important to reassign the column
    //definition Ids in the table, so that all Ids stays in order, increasing one by one for each element
    for (itCol = MyTerrain->levelRes->ColumnDefinitions.begin(); itCol != MyTerrain->levelRes->ColumnDefinitions.end(); ++itCol) {
            //set new Id for the next column definition
            (*itCol)->set_ID(currId);

            //do not forget to also update Offset in file which depends
            //on new Id!
            baseOffset = 98012 + currId * 26;
            (*itCol)->set_Offset(baseOffset);

            currId++;
    }

    //need to update all links to the old column definitions Id we modified in all colums of the level
    //to the newly assigned column definition Id
    //ReplaceColumnDefinitionWithNewOneForAllColumns(changeFromId, changeToId);
}

//Best version before 17.07.2025, but buggy, crashes
/*void LevelBlocks::RemoveUnusedColumnDefinitions(bool excludeActive, irr::u32 excludeId, bool reduceCntByOneForIdActive, irr::u32 reduceCntByOneForId) {
    std::string infoMsg("");
    char hlpstr[100];
    ColumnDefinition* whichDefExclude = nullptr;

    if (reduceCntByOneForIdActive) {
        ColumnDefinition* whichDef = levelRes->GetColumnDefinitionWithCertainId(reduceCntByOneForId);
        ColumnDefinition* otherCheck = nullptr;

        if (excludeActive) {
            otherCheck = levelRes->GetColumnDefinitionWithCertainId(excludeId);
        }

        //if we simply reuse the same column, do not decrease the mOccurence
        //so that we do not delete the column we reuse
        if ((whichDef != nullptr) && (otherCheck != whichDef)) {
                if (whichDef->get_Occurence() > 0) {
                    whichDef->set_Occurence(whichDef->get_Occurence() - 1);

                    infoMsg.clear();
                    infoMsg.append("Column: Reduce mOccurence by one for Id = ");

                    //add id
                    sprintf(hlpstr, "%d", whichDef->get_ID());

                    infoMsg.append(hlpstr);
                    logging::Info(infoMsg);
                }
        }
    }

    if (excludeActive) {
        whichDefExclude = levelRes->GetColumnDefinitionWithCertainId(excludeId);
    }

    std::vector<ColumnDefinition*>::iterator itCol;
    ColumnDefinition* toDelete;
    int oldId;

    irr::u32 currId = 1;
    int baseOffset;

    std::vector<irr::u32> changeFromId;
    std::vector<irr::u32> changeToId;

    bool keepUpdatingColumnIds = false;

    changeFromId.clear();
    changeToId.clear();

    //remove each entry with 0 usage
    for (itCol = MyTerrain->levelRes->ColumnDefinitions.begin(); itCol != MyTerrain->levelRes->ColumnDefinitions.end(); ) {

        if ((!(excludeActive && ((*itCol) == whichDefExclude))) && ((*itCol)->get_Occurence() == 0)) {
            //an unused ColumnDefinition entry, remove it
            toDelete = (*itCol);

            //keep old Id for later
            oldId = (*itCol)->get_ID();

            infoMsg.clear();
            infoMsg.append("Removing unused column definition with Id = ");

            //add id
            sprintf(hlpstr, "%d", oldId);

            infoMsg.append(hlpstr);
            logging::Info(infoMsg);

            //remove ColumnDefinition vector entry
            itCol = MyTerrain->levelRes->ColumnDefinitions.erase(itCol);

            //delete the ColumDefinition struct itself
            delete toDelete;

            //from now on we need to update every entry
            //in the table that still follows to make sure all
            //column Ids are properly aligned (increasing by one again)
            keepUpdatingColumnIds = true;
        } else {
            //if this column should be excluded from erasing, leave it alone and go to
            //the next element; This is true for example for a newly added column definition
            //that is not assigned yet
            if (excludeActive) {
                if ((*itCol) == whichDefExclude) {
                    infoMsg.clear();
                    infoMsg.append("Skip erasing column definition with Id = ");

                    //add id
                    sprintf(hlpstr, "%d", (*itCol)->get_ID());

                    infoMsg.append(hlpstr);
                    logging::Info(infoMsg);
                }
            }
        }

        if (keepUpdatingColumnIds) {
            //because we at least erased one column definition entry before,
            //this next elements Id needs now to be adjusted down as well,
            //so that all Ids stays in order, increasing one by one for each element
            oldId = (*itCol)->get_ID();

            //the next element gets the new (current Id)
            //remember which Id we replaced with which new Id
            changeFromId.push_back(oldId);
            changeToId.push_back(currId);

            infoMsg.clear();
            infoMsg.append("Modify column definition Id from = ");

            //add id
            sprintf(hlpstr, "%d", oldId);
            infoMsg.append(hlpstr);

            infoMsg.append(" to = ");
            sprintf(hlpstr, "%d", currId);
            infoMsg.append(hlpstr);

            logging::Info(infoMsg);

            //set new Id for the next column definition
            (*itCol)->set_ID(currId);

            //do not forget to also update Offset in file which depends
            //on new Id!
            baseOffset = 98012 + currId * 26;
            (*itCol)->set_Offset(baseOffset);
        }

        itCol++;
        currId++;
    }

    //need to update all links to the old column definitions Id we modified in all colums of the level
    //to the newly assigned column definition Id
    //ReplaceColumnDefinitionWithNewOneForAllColumns(changeFromId, changeToId);
}*/

//Returns true if the specified column was found, false otherwise
//If search succesfull outCoord output parameter returns the found map coordinates
//(map entry coordinates) for this specified column
bool LevelBlocks::FindMapCoordinateForColumn(Column* whichColumn, irr::core::vector2di& outCoord) {

    std::vector<ColumnsByPositionStruct>::iterator itColStruct;

    if (whichColumn == nullptr)
        return false;

    for (itColStruct = ColumnsByPosition.begin(); itColStruct != ColumnsByPosition.end(); ++itColStruct) {
        if ((*itColStruct).pColumn == whichColumn) {
            outCoord.X = (int)((*itColStruct).pColumn->Position.X);
            outCoord.Y = (int)((*itColStruct).pColumn->Position.Z);

            return true;
        }
    }

    //specified column not found
    return false;
}

