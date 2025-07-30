/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did translation to C++, and then modified and extended code for my project)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "column.h"
#include "../utils/logging.h"
#include "../resources/levelfile.h"
#include "../resources/mapentry.h"
#include "levelterrain.h"
#include "levelblocks.h"
#include "../resources/blockdefinition.h"
#include "../resources/columndefinition.h"
#include "../utils/crc32.h"

//if specialPreviewColumn is true this column can be used to render block preview pictures via Render to Target, or for example to render
//a preview picture of a whole column; This functions are only usefull in the level editor of the game!
Column::Column(LevelTerrain* myTerrain, LevelBlocks* myLevelBlocks, ColumnDefinition* Def, vector3d<irr::f32> pos, LevelFile* levelResLevel,
                bool specialPreviewColumn, BlockDefinition* specialPreviewColumnBlockDefPntrA) {
   segmentSize = 1.0f; // must be 1 for Hi-Octane !!
   mLevelBlocks = myLevelBlocks;
   mTerrain = myTerrain;

   column_ready = true;
   DestroyOnMorph = false;
   Hidden = false;

   this->levelRes = levelResLevel;

   Definition = Def;
   Position = pos;
   mSpecialPreviewColumn = specialPreviewColumn;
   mPreviewBlockDefA = specialPreviewColumnBlockDefPntrA;

   mBlockInfoVec.clear();

   if (levelResLevel != nullptr && Def != nullptr) {
    if (SetupGeometry()) {
       // std::cout << "HiOctane Column loaded: " <<
       //            GeometryInfoList->vertices.size() << " vertices, " << std::endl << std::flush;
//                   GeometryInfoList->normalVboData.size() << " normals, " <<
//                   uvVboData.size() << " UVs, " <<
//                   mTexSource->NumLevelTextures << " textures, " <<
//                   indicesVboData.size() << " indices" <<*/ std::endl << std::flush;

    } else {
       logging::Error("Failed setting up column Mesh, exit race");
       column_ready = false;
    }
   }
}

Column::~Column() {
    std::vector<BlockInfoStruct*>:: iterator it;
    BlockInfoStruct* pntr;

    if (mBlockInfoVec.size() > 0) {
        for (it = mBlockInfoVec.begin(); it != mBlockInfoVec.end(); ) {
            pntr = (*it);

            it = mBlockInfoVec.erase(it);

            CleanUpBlockInfoStruct(*pntr);
        }
    }
}

void Column::CleanUpBlockFaceInfoStruct(BlockFaceInfoStruct &pntr) {
    std::vector<irr::scene::SMeshBuffer*>::iterator itMesh;
    irr::scene::SMeshBuffer* pntrMeshBuf;

    if (pntr.myMeshBuffers.size() > 0) {
          for (itMesh = pntr.myMeshBuffers.begin(); itMesh != pntr.myMeshBuffers.end(); ) {
              pntrMeshBuf = (*itMesh);
              itMesh = pntr.myMeshBuffers.erase(itMesh);

              //pntrMeshBuf->drop();
          }
    }

    //free created vertices
    if (pntr.vert1 != nullptr) {
        delete pntr.vert1;
        pntr.vert1 = nullptr;
    }

    if (pntr.vert2 != nullptr) {
        delete pntr.vert2;
        pntr.vert2 = nullptr;
    }

    if (pntr.vert3 != nullptr) {
        delete pntr.vert3;
        pntr.vert3 = nullptr;
    }

    if (pntr.vert4 != nullptr) {
        delete pntr.vert4;
        pntr.vert4 = nullptr;
    }
}

void Column::CleanUpBlockInfoStruct(BlockInfoStruct &pntr) {
   if (pntr.fN != nullptr) {
       CleanUpBlockFaceInfoStruct(*pntr.fN);
       pntr.fN = nullptr;
   }

   if (pntr.fE != nullptr) {
       CleanUpBlockFaceInfoStruct(*pntr.fE);
       pntr.fE = nullptr;
   }

   if (pntr.fS != nullptr) {
       CleanUpBlockFaceInfoStruct(*pntr.fS);
       pntr.fS = nullptr;
   }

   if (pntr.fW != nullptr) {
       CleanUpBlockFaceInfoStruct(*pntr.fW);
       pntr.fW = nullptr;
   }

   if (pntr.fB != nullptr) {
       CleanUpBlockFaceInfoStruct(*pntr.fB);
       pntr.fB = nullptr;
   }

   if (pntr.fT != nullptr) {
       CleanUpBlockFaceInfoStruct(*pntr.fT);
       pntr.fT = nullptr;
   }
}

void Column::ApplyMorph(float progress) {
    if (DestroyOnMorph) {
        this->Hidden = progress > 0.01f;

        //if this column is hidden move column vertices
        //so far below the level that the are not visible anymore
        //for the player, therefore the column is "hidden"
        irr::f32 newColBaseVertex1Y = this->mBaseVert1CoordOriginal.Y - 100.0f;
        irr::f32 newColBaseVertex2Y = this->mBaseVert2CoordOriginal.Y - 100.0f;
        irr::f32 newColBaseVertex3Y = this->mBaseVert3CoordOriginal.Y - 100.0f;
        irr::f32 newColBaseVertex4Y = this->mBaseVert4CoordOriginal.Y - 100.0f;

        this->AdjustMeshBaseVerticeHeight(newColBaseVertex1Y, newColBaseVertex2Y, newColBaseVertex3Y, newColBaseVertex4Y);

        this->mLevelBlocks->UpdateBlockMesh();

        return;
    }

    //the next commented block is more or less
    //similar in the HioctaneTools source code
    //but seems to be not needed, therefore
    //I commented it out
    /*if (MorphSource == nullptr) {
        UpdateGeometry();
        return;
    }*/

     if (MorphSource->mBlockInfoVec.size() != mBlockInfoVec.size()) {
             logging::Error("Column morph - block count mismatch");

             return;
     }

     if (MorphSource->Definition->get_Shape() != Definition->get_Shape()) {
             logging::Error("Column morph - Shape mismatch");

             return;
     }

     //interpolate new column morphing Y coordinates according to progress variable (0..1)
     //between this column, and the Source Morphing column
     irr::f32 newColBaseVertex1Y = (this->mBaseVert1CoordOriginal.Y) * (1.0f - progress) + MorphSource->mBaseVert1CoordOriginal.Y * progress;
     irr::f32 newColBaseVertex2Y = (this->mBaseVert2CoordOriginal.Y) * (1.0f - progress) + MorphSource->mBaseVert2CoordOriginal.Y * progress;
     irr::f32 newColBaseVertex3Y = (this->mBaseVert3CoordOriginal.Y) * (1.0f - progress) + MorphSource->mBaseVert3CoordOriginal.Y * progress;
     irr::f32 newColBaseVertex4Y = (this->mBaseVert4CoordOriginal.Y) * (1.0f - progress) + MorphSource->mBaseVert4CoordOriginal.Y * progress;

     //if this column is hidden move column vertices
     //so far below the level that the are not visible anymore
     //for the player, therefore the column is "hidden"
     if (this->Hidden) {
         newColBaseVertex1Y -= 100.0f;
         newColBaseVertex2Y -= 100.0f;
         newColBaseVertex3Y -= 100.0f;
         newColBaseVertex4Y -= 100.0f;
     }

     this->AdjustMeshBaseVerticeHeight(newColBaseVertex1Y, newColBaseVertex2Y, newColBaseVertex3Y, newColBaseVertex4Y);

     this->mLevelBlocks->UpdateBlockMesh();
}

//allows to change the height of the 4 base block vertices of the column mesh, and all blocks above are
//adjusted as well
void Column::AdjustMeshBaseVerticeHeight(irr::f32 newV1y, irr::f32 newV2y, irr::f32 newV3y, irr::f32 newV4y) {
    std::vector<BlockInfoStruct*>::iterator itBlock;

    irr::f32 newV1;
    irr::f32 newV2;
    irr::f32 newV3;
    irr::f32 newV4;

    //need also to update my current column base vertice coordinates
    mBaseVert1Coord.Y = newV1y;
    mBaseVert2Coord.Y = newV2y;
    mBaseVert3Coord.Y = newV3y;
    mBaseVert4Coord.Y = newV4y;

    //loop through all existing blocks in this column
    for (itBlock = mBlockInfoVec.begin(); itBlock != mBlockInfoVec.end(); ++itBlock) {
       //calculate new adjusted y vertices coordinates for this specific block in the column
       //we are lucky that we have stored the blocks position in the column before in the
       //BlockInfoStruct struct
       newV1 = (*itBlock)->idxBlockFromBaseCnt * segmentSize + newV1y;
       newV2 = (*itBlock)->idxBlockFromBaseCnt * segmentSize + newV2y;
       newV3 = (*itBlock)->idxBlockFromBaseCnt * segmentSize + newV3y;
       newV4 = (*itBlock)->idxBlockFromBaseCnt * segmentSize + newV4y;

       this->mLevelBlocks->ChangeMeshCubeHeight((*itBlock), newV1, newV2, newV3, newV4);
    }

    //tell the block Mesh that it needs to be updated
    mLevelBlocks->UpdateBlockMesh();
}

irr::f32 Column::GetCurrentHeightTile(int x, int z) {
    int Width = levelRes->Width();
    int Height = levelRes->Height();

    if (x < 0) x = 0;
    if (z < 0) z = 0;
    if (x > Width - 1) x = Width - 1;
    if (z > Height - 1) z = Height - 1;

    return mTerrain->pTerrainTiles[x][z].vert1CurrPositionY;
}

irr::f32 Column::GetOriginalHeightTile(int x, int z) {
    int Width = levelRes->Width();
    int Height = levelRes->Height();

    if (x < 0) x = 0;
    if (z < 0) z = 0;
    if (x > Width - 1) x = Width - 1;
    if (z > Height - 1) z = Height - 1;

    MapEntry *p = this->GetMapEntry(x, z);
    return p->m_Height;
}

//returns the number of "missing" blocks at the base
//of the column until the first block is found
//can be used to detect tunnel roof elements etc...
//if there is not a single existing block in the column
//still will return 0, because then there is technically
//no "gap" of blocks
irr::u16 Column::GetNumberMissingBlocksAtBase() {
    irr::u16 gapBlockCnt = 0;

    if (Definition->get_A() != 0)
        return gapBlockCnt;

    gapBlockCnt++;

    if (Definition->get_B() != 0)
        return gapBlockCnt;

    gapBlockCnt++;

    if (Definition->get_C() != 0)
        return gapBlockCnt;

    gapBlockCnt++;

    if (Definition->get_D() != 0)
        return gapBlockCnt;

    gapBlockCnt++;

    if (Definition->get_E() != 0)
        return gapBlockCnt;

    gapBlockCnt++;

    if (Definition->get_F() != 0)
        return gapBlockCnt;

    gapBlockCnt++;

    if (Definition->get_G() != 0)
        return gapBlockCnt;

    gapBlockCnt++;

    if (Definition->get_H() != 0)
        return gapBlockCnt;

    //no "gap" found, return 0
    return 0;
}

MapEntry* Column::GetMapEntry(int x, int y) {
    int Width = levelRes->Width();
    int Height = levelRes->Height();

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > Width - 1) x = Width - 1;
    if (y > Height - 1) y = Height - 1;
    return levelRes->pMap[x][y];
}

BlockFaceInfoStruct* Column::CreateNewCubeFace(vector3d<irr::f32> v1,
                                               vector3d<irr::f32> v2,
                                               vector3d<irr::f32> v3,
                                               vector3d<irr::f32> v4,
                                               std::vector<vector2d<irr::f32>> uv, vector3d<irr::f32> normal,
                                               int textureId) {
    video::SColor cubeColour3(255,255,255,255);

    BlockFaceInfoStruct* newFace = new BlockFaceInfoStruct();
    newFace->vert1 = new video::S3DVertex(v1.X, v1.Y, v1.Z, normal.X, normal.Y, normal.Z, cubeColour3, uv[0].X, uv[0].Y);
    newFace->vert2 = new video::S3DVertex(v2.X, v2.Y, v2.Z, normal.X, normal.Y, normal.Z, cubeColour3, uv[1].X, uv[1].Y);
    newFace->vert3 = new video::S3DVertex(v3.X, v3.Y, v3.Z, normal.X, normal.Y, normal.Z, cubeColour3, uv[2].X, uv[2].Y);
    newFace->vert4 = new video::S3DVertex(v4.X, v4.Y, v4.Z, normal.X, normal.Y, normal.Z, cubeColour3, uv[3].X, uv[3].Y);

    newFace->currPositionVert1 = v1;
    newFace->currPositionVert2 = v2;
    newFace->currPositionVert3 = v3;
    newFace->currPositionVert4 = v4;

    newFace->originalPositionVert1 = v1;
    newFace->originalPositionVert2 = v2;
    newFace->originalPositionVert3 = v3;
    newFace->originalPositionVert4 = v4;

    newFace->textureId = textureId;

    newFace->myMeshBuffers.clear();
    newFace->myMeshBufVertexId.clear();

    return (newFace);
}

void Column::RemoveBlock(int nrBlockStartingFromBase) {
    if ((nrBlockStartingFromBase < 0) || (nrBlockStartingFromBase > 7))
        return;

    bool blockExists = false;

    //make sure at this location there is currently a block
    //otherwise exit
    switch (nrBlockStartingFromBase) {
        case 0: {
            if (Definition->get_A() != 0) {
                blockExists = true;
            }
            break;
        }
        case 1: {
            if (Definition->get_B() != 0) {
                blockExists = true;

            }
            break;
        }
        case 2: {
            if (Definition->get_C() != 0) {
                blockExists = true;
            }
            break;
        }
        case 3: {
            if (Definition->get_D() != 0) {
                blockExists = true;
            }
            break;
        }
        case 4: {
            if (Definition->get_E() != 0) {
                blockExists = true;
            }
            break;
        }
        case 5: {
            if (Definition->get_F() != 0) {
                blockExists = true;
            }
            break;
        }
        case 6: {
            if (Definition->get_G() != 0) {
                blockExists = true;
            }
            break;
        }
        case 7: {
            if (Definition->get_H() != 0) {
                blockExists = true;
            }
            break;
        }
    }

    if (!blockExists)
        return;

    irr::u16 skipBlocks = GetNumberMissingBlocksAtBase();

    //remove the BlockInfoStruct data that handles the removed
    //block
    std::vector<BlockInfoStruct*>:: iterator it;
    BlockInfoStruct* pntr;

    it = mBlockInfoVec.begin() + (nrBlockStartingFromBase - skipBlocks);

    pntr = (*it);

    it = mBlockInfoVec.erase(it);

    CleanUpBlockInfoStruct(*pntr);

    if (mNrBlocksInColumn > 0) {
        mNrBlocksInColumn--;
    }

    //27.07.2025: TODO: if we need it one time (I think right now we dont) we need to adjust
    //column property "Size" here, because if we remove a block at the base or top
    //the physical size of the column can change
    //but do not care right now
}

void Column::MoveColumnVertex(irr::core::vector3df &vertex) {
    vertex.X -= Position.X + 1;
    vertex.Y += Position.Y;
    vertex.Z += Position.Z;
}

//returns the current height of the column (size in Y direction) in parameter currHeight
BlockInfoStruct* Column::CreateGeometryBlock(BlockDefinition* blockDef, int blockNrFromBase, irr::f32 a,
                                 irr::f32 b, irr::f32 c, irr::f32 d, irr::f32 &currHeight) {
    irr::f32 h;

    std::vector<vector2d<irr::f32>> newuvsS;
    std::vector<vector2d<irr::f32>> newuvsW;
    std::vector<vector2d<irr::f32>> newuvsN;
    std::vector<vector2d<irr::f32>> newuvsE;
    std::vector<vector2d<irr::f32>> newuvsT;
    std::vector<vector2d<irr::f32>> newuvsB;

    int texIDInfoS;
    int texIDInfoW;
    int texIDInfoN;
    int texIDInfoE;
    int texIDInfoT;
    int texIDInfoB;

    // bottom vertices
    h = blockNrFromBase * segmentSize;
    vector3d<irr::f32> *A = new vector3d<irr::f32>(0.0f, a + h,  0.0f);
    vector3d<irr::f32> *B = new vector3d<irr::f32>(segmentSize, b + h,  0.0f);
    vector3d<irr::f32> *C = new vector3d<irr::f32>(segmentSize, c + h, segmentSize);
    vector3d<irr::f32> *D = new vector3d<irr::f32>(0.0f, d + h, segmentSize);

    // top vertices
    h += segmentSize;
    vector3d<irr::f32> *E = new vector3d<irr::f32>(0.0f, a + h, 0.0f);
    vector3d<irr::f32> *F = new vector3d<irr::f32>(segmentSize, b + h, 0.0f);
    vector3d<irr::f32> *G = new vector3d<irr::f32>(segmentSize, c + h, segmentSize);
    vector3d<irr::f32> *H = new vector3d<irr::f32>(0.0f, d + h, segmentSize);

    //we need to move this column to the correct map location
    //because otherwise all of the columns would be in the level
    //origin
    MoveColumnVertex(*A);
    MoveColumnVertex(*B);
    MoveColumnVertex(*C);
    MoveColumnVertex(*D);
    MoveColumnVertex(*E);
    MoveColumnVertex(*F);
    MoveColumnVertex(*G);
    MoveColumnVertex(*H);

    // texture atlas UVs
    newuvsS = mLevelBlocks->MakeUVs(blockDef->get_SMod());
    texIDInfoS = blockDef->get_S();

    newuvsW = mLevelBlocks->MakeUVs(blockDef->get_WMod());
    texIDInfoW = blockDef->get_W();

    newuvsN = mLevelBlocks->MakeUVs(blockDef->get_NMod());
    texIDInfoN = blockDef->get_N();

    newuvsE = mLevelBlocks->MakeUVs(blockDef->get_EMod());
    texIDInfoE = blockDef->get_E();

    newuvsT = mLevelBlocks->MakeUVs(blockDef->get_TMod());
    texIDInfoT = blockDef->get_T();

    newuvsB = mLevelBlocks->MakeUVs(blockDef->get_BMod());
    texIDInfoB = blockDef->get_B();

    // normals
    vector3d<irr::f32> *nN = new vector3d<irr::f32>(0.0f, 0.0f, -1.0f);
    vector3d<irr::f32> *nE = new vector3d<irr::f32>(1.0f, 0.0f, 0.0f);
    vector3d<irr::f32> *nS = new vector3d<irr::f32>(0.0f, 0.0f, 1.0f);
    vector3d<irr::f32> *nW = new vector3d<irr::f32>(-1.0f, 0.0f, 0.0f);
    vector3d<irr::f32> *nT = new vector3d<irr::f32>(0.0f, 1.0f, 0.0f);
    vector3d<irr::f32> *nB = new vector3d<irr::f32>(0.0f, -1.0f, 0.0f);

    //create all vertices and store them
    //create a new block
    BlockInfoStruct* newBlock = new BlockInfoStruct();
    newBlock->idxBlockFromBaseCnt = (irr::u8)(blockNrFromBase);

    //define all faces
    //north side
    newBlock->fN = CreateNewCubeFace(*F, *E, *A, *B, newuvsN, *nN, texIDInfoN);

    //east side
    newBlock->fE = CreateNewCubeFace(*G, *F, *B, *C, newuvsE, *nE, texIDInfoE);

    //south side
    newBlock->fS = CreateNewCubeFace(*H, *G, *C, *D, newuvsS, *nS, texIDInfoS);

    //west side
    newBlock->fW = CreateNewCubeFace(*E, *H, *D, *A, newuvsW, *nW, texIDInfoW);

    //top side
    newBlock->fT = CreateNewCubeFace(*E, *F, *G, *H, newuvsT, *nT, texIDInfoT);

    //bottom side
    newBlock->fB = CreateNewCubeFace(*D, *C, *B, *A, newuvsB, *nB, texIDInfoB);

    //cleanup
    delete A;
    delete B;
    delete C;
    delete D;
    delete E;
    delete F;
    delete G;
    delete H;

    delete nN;
    delete nE;
    delete nS;
    delete nW;
    delete nT;
    delete nB;

    currHeight = h;

    return newBlock;
}

bool Column::SetupGeometry() {
    int x, z;

    x = (int)Position.X;
    z = (int)Position.Z;

    MapEntry *pa;
    MapEntry *pb;
    MapEntry *pc;
    MapEntry *pd;

    irr::f32 a;
    irr::f32 b;
    irr::f32 c;
    irr::f32 d;

    if (!mSpecialPreviewColumn) {
            //default level map based column
            //column Y position is based on level terrain

            // 4 corners
            pa = this->GetMapEntry(x + 1, z);
            a = pa->m_Height;

            pb = this->GetMapEntry(x, z);
            b = pb->m_Height;

            pc = this->GetMapEntry(x, z + 1);
            c = pc->m_Height;

            pd = this->GetMapEntry(x + 1, z + 1);
            d = pd->m_Height;
    } else {
        //special non level map based column
        //ignore terrain height
        a = 0;
        b = 0;
        c = 0;
        d = 0;
    }

    irr::f32 h;

    //set the current column base vertice coordinates
    mBaseVert1Coord.set(segmentSize, d, segmentSize);
    mBaseVert2Coord.set(0.0f, c, segmentSize);
    mBaseVert3Coord.set(0.0f, b, 0.0f);
    mBaseVert4Coord.set(segmentSize, a,  0.0f);

    //we need to move this column to the correct map location
    //because otherwise all of the columns would be in the level
    //origin
    MoveColumnVertex(mBaseVert1Coord);
    MoveColumnVertex(mBaseVert2Coord);
    MoveColumnVertex(mBaseVert3Coord);
    MoveColumnVertex(mBaseVert4Coord);

    //store the (initial) vertices heights also
    //in a second variable, used for later column morphing
    mBaseVert1CoordOriginal = mBaseVert1Coord;
    mBaseVert2CoordOriginal = mBaseVert2Coord;
    mBaseVert3CoordOriginal = mBaseVert3Coord;
    mBaseVert4CoordOriginal = mBaseVert4Coord;

    mNrBlocksInColumn = 0;
    int16_t blockDefId;
    BlockDefinition *blockDef;
    BlockInfoStruct* newBlock;

    // Iterate through all blocks of this column
    for (int bitNum = 0; bitNum < 8; bitNum++) {

        //the next line skips creation of currently non existing blocks
        if ((Definition->get_Shape() & (1 << bitNum)) == 0) continue;

        //next two liens: Best code before 21.07.2025
        //std::vector<unsigned char> data = Definition->get_Bytes();
        //int16_t blockDefId = ConvertByteArray_ToInt16(data, 6 + bitNum * 2);

        //for a non special preview column (normal level column from the map) we take the blockdefinition
        //according to the block Id of the map entry
        if (!mSpecialPreviewColumn) {
                switch (bitNum) {
                    case 0: {
                        blockDefId = Definition->get_A();
                        break;
                    }
                    case 1: {
                        blockDefId = Definition->get_B();
                        break;
                    }
                    case 2: {
                        blockDefId = Definition->get_C();
                        break;
                    }
                    case 3: {
                        blockDefId = Definition->get_D();
                        break;
                    }
                    case 4: {
                        blockDefId = Definition->get_E();
                        break;
                    }
                    case 5: {
                        blockDefId = Definition->get_F();
                        break;
                    }
                    case 6: {
                        blockDefId = Definition->get_G();
                        break;
                    }
                    case 7: {
                        blockDefId = Definition->get_H();
                        break;
                    }
                }

                blockDef = levelRes->BlockDefinitions.at(blockDefId - 1);
        } else {
            //this is a special column, we use the supplied block definition pointers instead
            //that can point everywhere
            switch (bitNum) {
                case 0:
                default: {
                    blockDef = this->mPreviewBlockDefA;
                    break;
                }
                /*case 1: {
                    blockDef = this->mPreviewBlockDefB;
                    break;
                }
                case 2: {
                    blockDef = this->mPreviewBlockDefC;
                    break;
                }
                case 3: {
                    blockDef = this->mPreviewBlockDefD;
                    break;
                }
                case 4: {
                    blockDef = this->mPreviewBlockDefE;
                    break;
                }
                case 5: {
                    blockDef = this->mPreviewBlockDefF;
                    break;
                }
                case 6: {
                    blockDef = this->mPreviewBlockDefG;
                    break;
                }
                case 7: {
                    blockDef = this->mPreviewBlockDefH;
                    break;
                }*/
            }
        }

        //create the geometry for this block/cube
        //returns the current height of the column (size in Y direction)
        newBlock = CreateGeometryBlock(blockDef, bitNum, a, b, c, d, h);

        //add new block to my vector of blocks
        mBlockInfoVec.push_back(newBlock);

        mNrBlocksInColumn++;
     }

    Size.X = segmentSize;
    Size.Y = h;
    Size.Z = segmentSize;

    return true;
}

irr::u16 Column::GetNumberContainedBlocks() {
    return (mNrBlocksInColumn);
}
