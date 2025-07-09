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

    ChangeMeshCubeFaceHeight(whichCube->fB, newV1y, newV2y, newV3y, newV4y);
    ChangeMeshCubeFaceHeight(whichCube->fT, newV4y + segmentSize, newV3y + segmentSize, newV2y + segmentSize, newV1y + segmentSize);

    //ChangeMeshCubeFaceHeight(whichCube->fN, newV3y + segmentSize, newV3y , newV4y , newV3y + segmentSize );

    //ChangeMeshCubeFaceHeight(whichCube->fT, newV1y + segmentSize, newV2y + segmentSize, newV3y + segmentSize, newV4y + segmentSize);

    //ChangeMeshCubeFaceHeight(whichCube->fE, newV1y + segmentSize, newV2y, newV3y, newV4y + segmentSize);
}

void LevelBlocks::TestHeightChange(Column* selColumnPntr, int mSelBlockNrSkippingMissingBlocks) {
    BlockInfoStruct* blockInfo = GetBlockInfoStruct(selColumnPntr, mSelBlockNrSkippingMissingBlocks);

    if (blockInfo == nullptr)
         return;

    irr::f32 cV1 = blockInfo->fB->currPositionVert1.Y;
    irr::f32 cV2 = blockInfo->fB->currPositionVert2.Y;
    irr::f32 cV3 = blockInfo->fB->currPositionVert3.Y;
    irr::f32 cV4 = blockInfo->fB->currPositionVert4.Y;

    ChangeMeshCubeHeight(blockInfo, cV1 - 0.1f , cV2 , cV3  , cV4 );

    blockMeshForCollision->setDirty(EBT_VERTEX);
    blockMeshWithoutCollision->setDirty(EBT_VERTEX);
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

void LevelBlocks::SetCubeFaceTexture(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks,
                                     irr::u8 selFace, int16_t newTextureId) {
    //This higher level function has to do 2 independent things:
    // 1, modify the cube face configuration in the level/map file itself (so that next time we
    //    load the map again, everything is restored again in the same modified way)
    // 2, modify the current column block Mesh used by Irrlicht to show the user the
    //    current state of the column in the level. If we do not do this the level editor
    //    user can not see what he actually has changed already :)

    if (selColumnPntr == nullptr)
        return;

    BlockInfoStruct* blockInfo = selColumnPntr->mBlockInfoVec.at(mSelBlockNrSkippingMissingBlocks);

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

    uint8_t currNMod = blockDef->NMod();
    uint8_t currEMod = blockDef->EMod();
    uint8_t currSMod = blockDef->SMod();
    uint8_t currWMod = blockDef->WMod();
    uint8_t currTMod = blockDef->TMod();
    uint8_t currBMod = blockDef->BMod();

    //Replace the selected cube face with the new textureId
    //to create the new needed block definition, so that we can search
    //and if necessary create it in the current level file
    switch (selFace) {
         case DEF_SELBLOCK_FACENORTH:  { currN = newTextureId; break;}
         case DEF_SELBLOCK_FACEEAST:   { currE = newTextureId; break;}
         case DEF_SELBLOCK_FACESOUTH:  { currS = newTextureId; break;}
         case DEF_SELBLOCK_FACEWEST:   { currW = newTextureId; break;}
         case DEF_SELBLOCK_FACEBOTTOM: { currB = newTextureId; break;}
         case DEF_SELBLOCK_FACETOP:    { currT = newTextureId; break;}
         default:  { return; break;}
    }

    irr::u32 outIndex;

    //levelRes->DebugWriteBlockDefinitionTableToCsvFile((char*)("BlockDefBefore.csv"));
    //levelRes->DebugWriteColumnDefinitionTableToCsvFile((char*)("ColumnDefBefore.csv"));

    //"Request" the new block definition
    if (!this->MyTerrain->levelRes->RequestBlockDefinition(currN, currE, currS, currW, currT, currB, currNMod, currEMod, currSMod, currWMod, currTMod, currBMod, outIndex)) {
        //unexpected error creating block definition, possibly we reached the maximum possible number
        //of 1024 block definitions in the levelfile?
        return;
    }

     //levelRes->DebugWriteBlockDefinitionTableToCsvFile((char*)("BlockAfter1.csv"));

    //The new or found block definition index is returned in outIndex
    //modify the block definition in the selected column with the new one
    switch (nrBlockFromBase) {
        case 0: { selColumnPntr->Definition->set_A(outIndex); break; }
        case 1: { selColumnPntr->Definition->set_B(outIndex); break; }
        case 2: { selColumnPntr->Definition->set_C(outIndex); break; }
        case 3: { selColumnPntr->Definition->set_D(outIndex); break; }
        case 4: { selColumnPntr->Definition->set_E(outIndex); break; }
        case 5: { selColumnPntr->Definition->set_F(outIndex); break; }
        case 6: { selColumnPntr->Definition->set_G(outIndex); break; }
        case 7: { selColumnPntr->Definition->set_H(outIndex); break; }
        default: {
            return;
            break;
        }
    }

    //Make sure (possibly now) unused Blockdefinitions are deleted
    //Because I am not sure if the game can handle them, so make sure we
    //do not have them
    RemoveUnusedBlockDefinitions();

    //levelRes->DebugWriteBlockDefinitionTableToCsvFile((char*)("BlockAfter2.csv"));
    //levelRes->DebugWriteColumnDefinitionTableToCsvFile((char*)("ColumnDefAfter2.csv"));

    /******************************************************************
     * Part 2: According to new texture modify Irrlicht column Mesh   *
     ******************************************************************/

    // Part 1: Update the current Irrlicht Mesh
    if (selColumnPntr->Definition->mInCollisionMesh[mSelBlockNrSkippingMissingBlocks] == 1) {
        //this block has collision detection, so we need to delete its mesh
        //from the mesh with collision detection
        mIrrMeshBuf->RemoveMeshBufferCubeFace(mBlockwCollMeshBufferVec, selFacePntr, *mBlocksMeshStats);

        //setup new textureId the user has selected
        selFacePntr->textureId = newTextureId;

        //add back cube face mesh with the new textureId
        //so that the user sees the updated texture
        mIrrMeshBuf->AddMeshBufferCubeFace(mBlockwCollMeshBufferVec, selFacePntr, *mBlocksMeshStats);

        blockMeshForCollision->setDirty(EBT_VERTEX_AND_INDEX);
        blockMeshForCollision->recalculateBoundingBox();
    } else {
        //no collision detection, use other buffer without collision detection
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
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                cntResult.at(val) += 1;
            }
        }

        val = (*itCol)->get_B();

        if (val != 0) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                cntResult.at(val) += 1;
            }
        }

        val = (*itCol)->get_C();

        if (val != 0) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                cntResult.at(val) += 1;
            }
        }

        val = (*itCol)->get_D();

        if (val != 0) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                cntResult.at(val) += 1;
            }
        }

        val = (*itCol)->get_E();

        if (val != 0) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                cntResult.at(val) += 1;
            }
        }

        val = (*itCol)->get_F();

        if (val != 0) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                cntResult.at(val) += 1;
            }
        }

        val = (*itCol)->get_G();

        if (val != 0) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                cntResult.at(val) += 1;
            }
        }

        val = (*itCol)->get_H();

        if (val != 0) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                cntResult.at(val) += 1;
            }
        }
    }

    return (cntResult);
}

void LevelBlocks::ReplaceBlockDefinitionIdWithNewOneInAllColumdefinitions(std::vector<irr::u32> changeFromIdVec, std::vector<irr::u32> changeToIdVec) {
    //loop through all currently existing columndefinitions
    std::vector<ColumnDefinition*>::iterator itCol;

    size_t entriesToChange = changeFromIdVec.size();

    //how many different Blockdefinitions do we have currently?
    irr::u32 nrBlockDef = this->levelRes->BlockDefinitions.size();

    int16_t val;

    bool Amod;
    bool Bmod;
    bool Cmod;
    bool Dmod;
    bool Emod;
    bool Fmod;
    bool Gmod;
    bool Hmod;

    int oldId;
    int newId;

    for (itCol = this->levelRes->ColumnDefinitions.begin(); itCol != this->levelRes->ColumnDefinitions.end(); ++itCol) {
        Amod = false;
        Bmod = false;
        Cmod = false;
        Dmod = false;
        Emod = false;
        Fmod = false;
        Gmod = false;
        Hmod = false;

      for (size_t idxEntry = 1; idxEntry < entriesToChange; idxEntry++) {

        oldId = (int)(changeFromIdVec.at(idxEntry));
        newId = (int)(changeToIdVec.at(idxEntry));

        val = (*itCol)->get_A();

        if ((!Amod) && (val != 0)) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                if (val == oldId) {
                    (*itCol)->set_A(newId);
                    Amod = true;
                }
            }
        }

        val = (*itCol)->get_B();

        if ((!Bmod) && (val != 0)) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                if (val == oldId) {
                    (*itCol)->set_B(newId);
                    Bmod = true;
                }
            }
        }

        val = (*itCol)->get_C();

        if ((!Cmod) && (val != 0)) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                if (val == oldId) {
                    (*itCol)->set_C(newId);
                    Cmod = true;
                }
            }
        }

        val = (*itCol)->get_D();

        if ((!Dmod) && (val != 0)) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                if (val == oldId) {
                    (*itCol)->set_D(newId);
                    Dmod = true;
                }
            }
        }

        val = (*itCol)->get_E();

          if ((!Emod) && (val != 0)) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                if (val == oldId) {
                    (*itCol)->set_E(newId);
                    Emod = true;
                }
            }
        }

        val = (*itCol)->get_F();

         if ((!Fmod) && (val != 0)) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                if (val == oldId) {
                    (*itCol)->set_F(newId);
                    Fmod = true;
                }
            }
        }

        val = (*itCol)->get_G();

         if ((!Gmod) && (val != 0)) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                if (val == oldId) {
                    (*itCol)->set_G(newId);
                    Gmod = true;
                }
            }
        }

        val = (*itCol)->get_H();

        if ((!Hmod) && (val != 0)) {
            if ((val > 0) && (val < (int16_t)(nrBlockDef))) {
                if (val == oldId) {
                    (*itCol)->set_H(newId);
                    Hmod = true;
                }
            }
        }
    }
  }
}

/*
void LevelBlocks::ReplaceBlockDefinitionIdWithNewOneForAllColumns(int oldId, int newId) {
    //loop through all currently existing columns and cube
    //loop through all existing columns, and cube
    std::vector<ColumnDefinition*>::iterator itCol;

    //how many different Blockdefinitions do we have currently?
    //irr::u32 nrBlockDef = this->levelRes->BlockDefinitions.size();

    int16_t currA;
    int16_t currB;
    int16_t currC;
    int16_t currD;
    int16_t currE;
    int16_t currF;
    int16_t currG;
    int16_t currH;

    irr::u32 outIndex;
    ColumnDefinition* newDef;
    ColumnDefinition* oldDef;

    std::string infoMsg("");
    char hlpstr[100];

    infoMsg.clear();
    infoMsg.append("Replace block definition Id = ");

    //add id
    sprintf(hlpstr, "%d", oldId);

    infoMsg.append(hlpstr);
    infoMsg.append(" with Id = ");

    sprintf(hlpstr, "%d", newId);

    infoMsg.append(hlpstr);
    logging::Info(infoMsg);

    for (itCol = this->levelRes->ColumnDefinitions.begin(); itCol != this->levelRes->ColumnDefinitions.end(); ++itCol) {
        currA = (*itCol)->get_A();
        currB = (*itCol)->get_B();
        currC = (*itCol)->get_C();
        currD = (*itCol)->get_D();
        currE = (*itCol)->get_E();
        currF = (*itCol)->get_F();
        currG = (*itCol)->get_G();
        currH = (*itCol)->get_H();

        //is this "old" blockdefinition Id use somewhere, if not
        //continue with next column definition
        if ((currA != oldId) && (currB != oldId) && (currC != oldId) && (currD != oldId)
            && (currE != oldId) && (currF != oldId) && (currG != oldId) && (currH != oldId))
             continue;

        //This colum definition needs to be adjusted
        if (currA == oldId) {
            currA = newId;
        }

        if (currB == oldId) {
            currB = newId;
        }

        if (currC == oldId) {
            currC = newId;
        }

        if (currD == oldId) {
            currD = newId;
        }

        if (currE == oldId) {
            currE = newId;
        }

        if (currF == oldId) {
            currF = newId;
        }

        if (currG == oldId) {
            currG = newId;
        }

        if (currH == oldId) {
            currH = newId;
        }

        oldDef = (*itCol);

        //does this newly needed column definition
        //already exist?
        //"Request" the new column definition
        if (!this->MyTerrain->levelRes->RequestColumnDefinition(currA, currB, currC, currD, currE, currF, currG, currH, outIndex)) {
            //unexpected error creating column definition, possibly we reached the maximum possible number
            //of 1024 column definitions in the levelfile?
            return;
        }

        newDef = this->levelRes->ColumnDefinitions.at(outIndex);

        if (newDef == nullptr)
            return;

        ReplaceColumnDefinitionWithNewOneForAllColumns(oldDef, newDef);

        //Make sure (possibly now) unused Columndefinitions are deleted
        //Because I am not sure if the game can handle them, so make sure we
        //do not have them
        RemoveUnusedColumnDefinitions();
   }
}*/

void LevelBlocks::RemoveUnusedBlockDefinitions() {
    //first get usage count for each existing block definition
    std::vector<irr::u32> usageCnt = GetBlockDefinitionUsageCount();

    std::vector<irr::u32>::iterator it;
    std::vector<BlockDefinition*>::iterator itBlock;
    BlockDefinition* toDelete;
    int nextId;
    int oldId;

    irr::u32 currIdx = 0;
    irr::u32 deletedCnt = 0;
    irr::u32 cntIndexUp;
    int baseOffset;

    std::string infoMsg("");
    char hlpstr[100];

    std::vector<irr::u32> changeFromId;
    std::vector<irr::u32> changeToId;

    changeFromId.clear();
    changeToId.clear();

    //remove each entry with 0 usage
    //we need to skip first element, because there is no usage for BlockDefinition with
    //Id = 0 in the game;
    for (it = usageCnt.begin() + 1; it != usageCnt.end(); ++it) {
        if ((*it) == 0) {
            //an unused BlockDefinition entry, remove it
            //the order of the elements in usageCnt vector and BlockDefinitions vector
            //below are identical
            itBlock = MyTerrain->levelRes->BlockDefinitions.begin() + currIdx - deletedCnt;

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

            cntIndexUp = 0;

            //reassign Ids for all following BlockDefinitions
            for (; itBlock != MyTerrain->levelRes->BlockDefinitions.end(); ++itBlock) {

                cntIndexUp++;

                //set next Id for the element
                //at this vector position
                nextId = currIdx - deletedCnt + cntIndexUp;

                //remember which Id we replaced with which new Id
                changeFromId.push_back(nextId);
                changeToId.push_back(oldId);

                //set new Id for the next Block definition
                (*itBlock)->set_ID(nextId);

                //do not forget to also update Offset in file which depends
                //on new Id!
                baseOffset = 124636 + nextId * 16;
                (*itBlock)->set_Offset(baseOffset);

                //keep old Id for later
                oldId = (*itBlock)->get_ID();
            }

            //delete the blockdefinition struct itself
            delete toDelete;

            deletedCnt++;
        }

        currIdx++;
    }

    //need to update all links to the old Block Id in all columdefinitions of the level
    //to the newly assigned block definition Id
    ReplaceBlockDefinitionIdWithNewOneInAllColumdefinitions(changeFromId, changeToId);
}

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
            if ((val > 0) && (val < (int)(nrColumnDef))) {
                cntResult.at(val) += 1;
            }
        }
    }

    return (cntResult);
}

void LevelBlocks::ReplaceColumnDefinitionWithNewOneForAllColumns(ColumnDefinition* oldDef, ColumnDefinition* newDef) {
    //how many different columns do we have currently?
    //irr::u32 nrColumns = this->ColumnsByPosition.size();

    std::string infoMsg("");
    char hlpstr[100];

    infoMsg.clear();
    infoMsg.append("Replace column definition Id = ");

    //add id
    sprintf(hlpstr, "%d", oldDef->get_ID());

    infoMsg.append(hlpstr);
    infoMsg.append(" with Id = ");

    sprintf(hlpstr, "%d", newDef->get_ID());

    infoMsg.append(hlpstr);
    logging::Info(infoMsg);


    //loop through all existing columns
    std::vector<ColumnsByPositionStruct>::iterator itCol;

    for (itCol = this->ColumnsByPosition.begin(); itCol != this->ColumnsByPosition.end(); ++itCol) {
        if ((*itCol).pColumn->Definition == oldDef) {
            (*itCol).pColumn->Definition = newDef;
        }
    }

    int height = levelRes->Height();
    int width = levelRes->Width();

    MapEntry* entry;

    //iterate through tile map, and fix all columndefinitions
    for (int y = 0; y < height; y++) {
     for (int x = 0; x < width; x++) {
         entry = levelRes->pMap[x][y];

         if ((entry != nullptr) && (entry->get_Column() == oldDef)) {
             entry->set_Column(newDef);

             //update entry data, which will convert the new Id for this colum definition
             //into the correct cid value inside this MapEntry
             entry->WriteChanges();
         }
    }
   }
}

void LevelBlocks::RemoveUnusedColumnDefinitions() {
    //first get usage count for each existing column definition
    std::vector<irr::u32> usageCnt = GetColumnDefinitionUsageCount();

    std::vector<irr::u32>::iterator it;
    std::vector<ColumnDefinition*>::iterator itCol;
    ColumnDefinition* toDelete;
    int nextId;
    int oldId;

    ColumnDefinition* oldDef;
    ColumnDefinition* newDef;

    irr::u32 currIdx = 0;
    irr::u32 deletedCnt = 0;
    irr::u32 cntIndexUp;
    int baseOffset;

    std::string infoMsg("");
    char hlpstr[100];

    //remove each entry with 0 usage
    //we need to skip first element, because there is no usage for ColumnDefinition with
    //Id = 0 in the game;
    for (it = usageCnt.begin() + 1; it != usageCnt.end(); ++it) {
        if ((*it) == 0) {
            //an unused ColumnDefinition entry, remove it
            //the order of the elements in usageCnt vector and ColumnDefinitions vector
            //below are identical
            itCol = MyTerrain->levelRes->ColumnDefinitions.begin() + currIdx - deletedCnt;

            toDelete = (*itCol);

            //keep old Id for later
            oldDef = (*itCol);
            oldId = (*itCol)->get_ID();

            infoMsg.clear();
            infoMsg.append("Removing unused column definition with Id = ");

            //add id
            sprintf(hlpstr, "%d", oldId);

            infoMsg.append(hlpstr);

            logging::Info(infoMsg);

            //remove ColumnDefinition vector entry
            itCol = MyTerrain->levelRes->ColumnDefinitions.erase(itCol);

            cntIndexUp = 0;

            //reassign Ids for all following ColumnDefinitions
            for (; itCol != MyTerrain->levelRes->ColumnDefinitions.end(); ++itCol) {

                cntIndexUp++;

                newDef = (*itCol);

                //set next Id for the element
                //at this vector position
                nextId = currIdx - deletedCnt + cntIndexUp;

                //set new Id for the next column definition
                (*itCol)->set_ID(nextId);

                //do not forget to also update Offset in file which depends
                //on new Id!
                baseOffset = 98012 + nextId * 26;
                (*itCol)->set_Offset(baseOffset);

                //need to update map entry cid values, to reflect new assigned
                //colum definition Id
                ReplaceColumnDefinitionWithNewOneForAllColumns(oldDef, newDef);

                //keep old Id for later
                oldDef = (*itCol);
                oldId = (*itCol)->get_ID();
            }

            //delete the colum defintion struct itself
            delete toDelete;

            deletedCnt++;
        }

        currIdx++;
    }
}

