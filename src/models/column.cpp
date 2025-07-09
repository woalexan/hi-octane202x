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
#include "../resources/blockdefinition.h"
#include "../resources/columndefinition.h"
#include "../utils/crc32.h"

Column::Column(LevelTerrain* myTerrain, LevelBlocks* myLevelBlocks, ColumnDefinition* Def, vector3d<irr::f32> pos, LevelFile* levelResLevel) {
   segmentSize = 1.0f; // must be 1 for Hi-Octane !!
   MyLevelBlocks = myLevelBlocks;
   MyTerrain = myTerrain;

   column_ready = true;
   DestroyOnMorph = false;
   Hidden = false;

   this->levelRes = levelResLevel;

   Definition = Def;
   Position = pos;

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

//void Column::ApplyMorph(float progress) {
//    std::vector<irr::scene::SMeshBuffer*>::iterator it2;
//    S3DVertex *pntrVertices;
//    irr::u32 idxMeshBuf;

//    if (DestroyOnMorph) {
//        this->Hidden = progress > 0.01f;

//        //if this column is hidden move column vertices
//        //so far below the level that the are not visible anymore
//        //for the player, therefore the column is "hidden"
//        u32 nrVertices = (irr::u32)(GeometryInfoList->vertices.size());

//        for (unsigned int i = 0; i < nrVertices; i++) {
//            irr::f32 homeHeight = GeometryInfoList->vertices[i].originalPosition.Y;
//            irr::f32 h = homeHeight - 100.0f;

//            //set new vertice height
//            GeometryInfoList->vertices[i].currPosition.Y = h;

//            //need to update vertice position for this vertice!
//            //GeometryInfoList->vertices[i].positionDirty = true;

//            idxMeshBuf = 0;

//            for (it2 = this->GeometryInfoList->vertices[i].myMeshBuffers.begin(); it2 != this->GeometryInfoList->vertices[i].myMeshBuffers.end(); ++(it2)) {
//                 (*it2)->grab();
//                 void* pntrVert = (*it2)->getVertices();
//                 pntrVertices = (S3DVertex*)pntrVert;
//                 pntrVertices[this->GeometryInfoList->vertices[i].myMeshBufVertexId[idxMeshBuf]].Pos.Y = GeometryInfoList->vertices[i].currPosition.Y;

//                 idxMeshBuf++;

//                 (*it2)->drop();

//                 //add this Meshbuffer to the list of overall dirty buffers
//                // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
//             }

//            //this->GeometryInfoList->vertices[i].positionDirty = false;
//        }

//        return;
//    }

//    //the next commented block is more or less
//    //similar in the HioctaneTools source code
//    //but seems to be not needed, therefore
//    //I commented it out
//    /*if (MorphSource == nullptr) {
//        UpdateGeometry();
//        return;
//    }*/

//     if (MorphSource->GeometryInfoList->vertices.size() != GeometryInfoList->vertices.size()) {
//             logging::Error("Column morph - vertice count mismatch");
//     }

//    u32 nrVertices = (irr::u32)(GeometryInfoList->vertices.size());

//    for (unsigned int i = 0; i < nrVertices; i++) {
//        irr::f32 sourceHeight = MorphSource->GeometryInfoList->vertices[i].originalPosition.Y;
//        irr::f32 homeHeight = GeometryInfoList->vertices[i].originalPosition.Y;
//        irr::f32 h = homeHeight * (1.0f - progress) + sourceHeight * progress;

//        //if this column is hidden move column vertices
//        //so far below the level that the are not visible anymore
//        //for the player, therefore the column is "hidden"
//        if (this->Hidden) h -= 100.0f;

//        //set new vertice height
//        GeometryInfoList->vertices[i].currPosition.Y = h;

//        //need to update vertice position for this vertice!
//        //GeometryInfoList->vertices[i].positionDirty = true;

//        idxMeshBuf = 0;

//        for (it2 = this->GeometryInfoList->vertices[i].myMeshBuffers.begin(); it2 != this->GeometryInfoList->vertices[i].myMeshBuffers.end(); ++(it2)) {
//             (*it2)->grab();
//             void* pntrVert = (*it2)->getVertices();
//             pntrVertices = (S3DVertex*)pntrVert;
//             pntrVertices[this->GeometryInfoList->vertices[i].myMeshBufVertexId[idxMeshBuf]].Pos.Y = GeometryInfoList->vertices[i].currPosition.Y;

//             idxMeshBuf++;

//             (*it2)->drop();

//             //add this Meshbuffer to the list of overall dirty buffers
//            // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
//         }

//        //this->GeometryInfoList->vertices[i].positionDirty = false;
//    }

//    //mark column vertices as dirty
//    //this->MyLevelBlocks->SetColumnVerticeSMeshBufferVerticePositionsDirty();
//}

//below is the new function
void Column::ApplyMorph(float progress) {
//    std::vector<irr::scene::SMeshBuffer*>::iterator it2;
//    S3DVertex *pntrVertices;
//    irr::u32 idxMeshBuf;

//    if (DestroyOnMorph) {
//        this->Hidden = progress > 0.01f;

//        //if this column is hidden move column vertices
//        //so far below the level that the are not visible anymore
//        //for the player, therefore the column is "hidden"
//        u32 nrVertices = (irr::u32)(GeometryInfoList->vertices.size());

//        for (unsigned int i = 0; i < nrVertices; i++) {
//            irr::f32 homeHeight = GeometryInfoList->vertices[i].originalPosition.Y;
//            irr::f32 h = homeHeight - 100.0f;

//            //set new vertice height
//            GeometryInfoList->vertices[i].currPosition.Y = h;

//            //need to update vertice position for this vertice!
//            //GeometryInfoList->vertices[i].positionDirty = true;

//            idxMeshBuf = 0;

//            for (it2 = this->GeometryInfoList->vertices[i].myMeshBuffers.begin(); it2 != this->GeometryInfoList->vertices[i].myMeshBuffers.end(); ++(it2)) {
//                 (*it2)->grab();
//                 void* pntrVert = (*it2)->getVertices();
//                 pntrVertices = (S3DVertex*)pntrVert;
//                 pntrVertices[this->GeometryInfoList->vertices[i].myMeshBufVertexId[idxMeshBuf]].Pos.Y = GeometryInfoList->vertices[i].currPosition.Y;

//                 idxMeshBuf++;

//                 (*it2)->drop();

//                 //add this Meshbuffer to the list of overall dirty buffers
//                // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
//             }

//            //this->GeometryInfoList->vertices[i].positionDirty = false;
//        }

//        return;
//    }

//    //the next commented block is more or less
//    //similar in the HioctaneTools source code
//    //but seems to be not needed, therefore
//    //I commented it out
//    /*if (MorphSource == nullptr) {
//        UpdateGeometry();
//        return;
//    }*/

//     if (MorphSource->GeometryInfoList->vertices.size() != GeometryInfoList->vertices.size()) {
//             logging::Error("Column morph - vertice count mismatch");
//     }

//    u32 nrVertices = (irr::u32)(GeometryInfoList->vertices.size());

//    for (unsigned int i = 0; i < nrVertices; i++) {
//        irr::f32 sourceHeight = MorphSource->GeometryInfoList->vertices[i].originalPosition.Y;
//        irr::f32 homeHeight = GeometryInfoList->vertices[i].originalPosition.Y;
//        irr::f32 h = homeHeight * (1.0f - progress) + sourceHeight * progress;

//        //if this column is hidden move column vertices
//        //so far below the level that the are not visible anymore
//        //for the player, therefore the column is "hidden"
//        if (this->Hidden) h -= 100.0f;

//        //set new vertice height
//        GeometryInfoList->vertices[i].currPosition.Y = h;

//        //need to update vertice position for this vertice!
//        //GeometryInfoList->vertices[i].positionDirty = true;

//        idxMeshBuf = 0;

//        for (it2 = this->GeometryInfoList->vertices[i].myMeshBuffers.begin(); it2 != this->GeometryInfoList->vertices[i].myMeshBuffers.end(); ++(it2)) {
//             (*it2)->grab();
//             void* pntrVert = (*it2)->getVertices();
//             pntrVertices = (S3DVertex*)pntrVert;
//             pntrVertices[this->GeometryInfoList->vertices[i].myMeshBufVertexId[idxMeshBuf]].Pos.Y = GeometryInfoList->vertices[i].currPosition.Y;

//             idxMeshBuf++;

//             (*it2)->drop();

//             //add this Meshbuffer to the list of overall dirty buffers
//            // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
//         }

//        //this->GeometryInfoList->vertices[i].positionDirty = false;
//    }

//    //mark column vertices as dirty
//    //this->MyLevelBlocks->SetColumnVerticeSMeshBufferVerticePositionsDirty();
}

irr::f32 Column::GetCurrentHeightTile(int x, int z) {
    int Width = levelRes->Width();
    int Height = levelRes->Height();

    if (x < 0) x = 0;
    if (z < 0) z = 0;
    if (x > Width - 1) x = Width - 1;
    if (z > Height - 1) z = Height - 1;

    return MyTerrain->pTerrainTiles[x][z].vert1CurrPositionY;
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

std::vector<vector2d<irr::f32>> Column::ApplyTexMod(vector2d<irr::f32> uvA, vector2d<irr::f32> uvB, vector2d<irr::f32> uvC, vector2d<irr::f32> uvD, int mod) {
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

std::vector<vector2d<irr::f32>> Column::MakeUVs(int texMod) {
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

void Column::MoveColumnVertex(irr::core::vector3df &vertex) {
    vertex.X -= Position.X + 1;
    vertex.Y += Position.Y;
    vertex.Z += Position.Z;
}

bool Column::SetupGeometry() {
    int x, z;

    x = (int)Position.X;
    z = (int)Position.Z;

    // 4 corners
    MapEntry *pa = this->GetMapEntry(x + 1, z);
    irr::f32 a = pa->m_Height;

    MapEntry *pb = this->GetMapEntry(x, z);
    irr::f32 b = pb->m_Height;

    MapEntry *pc = this->GetMapEntry(x, z + 1);
    irr::f32 c = pc->m_Height;

    MapEntry *pd = this->GetMapEntry(x + 1, z + 1);
    irr::f32 d = pd->m_Height;

    irr::f32 h = 0.0f;

    std::vector<vector2d<irr::f32>> newuvsS;
    std::vector<vector2d<irr::f32>> newuvsW;
    std::vector<vector2d<irr::f32>> newuvsN;
    std::vector<vector2d<irr::f32>> newuvsE;
    std::vector<vector2d<irr::f32>> newuvsT;
    std::vector<vector2d<irr::f32>> newuvsB;

    int textIDInfoS;
    int textIDInfoW;
    int textIDInfoN;
    int textIDInfoE;
    int textIDInfoT;
    int textIDInfoB;

    mNrBlocksInColumn = 0;

    // Iterate through all blocks of this column
    for (int bitNum = 0; bitNum < 8; bitNum++) {

        if ((Definition->get_Shape() & (1 << bitNum)) == 0) continue;

        std::vector<unsigned char> data = Definition->get_Bytes();

        int16_t blockDefId = ConvertByteArray_ToInt16(data, 6 + bitNum * 2);
        BlockDefinition *blockDef = levelRes->BlockDefinitions.at(blockDefId - 1);

        // bottom vertices
        h = bitNum * segmentSize;
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
        newuvsS = MakeUVs(blockDef->SMod());
        textIDInfoS = blockDef->get_S();

        newuvsW = MakeUVs(blockDef->WMod());
        textIDInfoW = blockDef->get_W();

        newuvsN = MakeUVs(blockDef->NMod());
        textIDInfoN = blockDef->get_N();

        newuvsE = MakeUVs(blockDef->EMod());
        textIDInfoE = blockDef->get_E();

        newuvsT = MakeUVs(blockDef->TMod());
        textIDInfoT = blockDef->get_T();

        newuvsB = MakeUVs(blockDef->BMod());
        textIDInfoB = blockDef->get_B();

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

        //define all faces

        //north side
        newBlock->fN = CreateNewCubeFace(*F, *E, *A, *B, newuvsN, *nN, textIDInfoN);

        //east side
        newBlock->fE = CreateNewCubeFace(*G, *F, *B, *C, newuvsE, *nE, textIDInfoE);

        //south side
        newBlock->fS = CreateNewCubeFace(*H, *G, *C, *D, newuvsS, *nS, textIDInfoS);

        //west side
        newBlock->fW = CreateNewCubeFace(*E, *H, *D, *A, newuvsW, *nW, textIDInfoW);

        //top side
        newBlock->fT = CreateNewCubeFace(*E, *F, *G, *H, newuvsT, *nT, textIDInfoT);

        //bottom side
        newBlock->fB = CreateNewCubeFace(*D, *C, *B, *A, newuvsB, *nB, textIDInfoB);

        //add new block to my vector of blocks
        mBlockInfoVec.push_back(newBlock);

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
