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

   GeometryInfoList = new ColumnSideGeometryInfo();

   if (levelResLevel != NULL && Def != NULL) {
    if (setupGeometry()) {
        /*std::cout << "HiOctane Column loaded: " <<
                   positionVboData.size() << " vertices, " <<
                   normalVboData.size() << " normals, " <<
                   uvVboData.size() << " UVs, " <<
                   mTexSource->NumLevelTextures << " textures, " <<
                   indicesVboData.size() << " indices" << endl << std::flush;*/

    } else {
       logging::Error("Failed setting up column Mesh, exit race");
       column_ready = false;
    }
   }
}

Column::~Column() {
  if (this->GeometryInfoList != NULL) {
      if (this->GeometryInfoList->vertices.size() > 0) {
        std::vector<ColumnVerticeInfo>::iterator itVert;
        ColumnVerticeInfo* vertInfoPntr;

        std::vector<irr::scene::SMeshBuffer*>::iterator itMesh;
        irr::scene::SMeshBuffer* pntrMeshBuf;

        for (itVert = this->GeometryInfoList->vertices.begin(); itVert != this->GeometryInfoList->vertices.end(); ++itVert) {
            vertInfoPntr = &(*itVert);

            itVert = this->GeometryInfoList->vertices.erase(itVert);

            //free existing MeshBuffers
            if (vertInfoPntr->myMeshBuffers.size() > 0) {
                  for (itMesh = vertInfoPntr->myMeshBuffers.begin(); itMesh != vertInfoPntr->myMeshBuffers.end(); ) {
                      pntrMeshBuf = (*itMesh);

                      itMesh = vertInfoPntr->myMeshBuffers.erase(itMesh);

                      //Next line commented out, does crash program
                      //what is wrong here?
                      //pntrMeshBuf->drop();
                  }
            }

            //free existing vertex
            if (vertInfoPntr->vert != NULL) {
                delete vertInfoPntr->vert;
                vertInfoPntr->vert = NULL;
            }
        }
    }

   //free the geometry info list
   delete GeometryInfoList;
   GeometryInfoList = NULL;
 }
}

void Column::ApplyMorph(float progress) {
    std::vector<irr::scene::SMeshBuffer*>::iterator it2;
    S3DVertex *pntrVertices;
    irr::u32 idxMeshBuf;

    if (DestroyOnMorph) {
        this->Hidden = progress > 0.01f;

        //if this column is hidden move column vertices
        //so far below the level that the are not visible anymore
        //for the player, therefore the column is "hidden"
        u32 nrVertices = (irr::u32)(GeometryInfoList->vertices.size());

        for (unsigned int i = 0; i < nrVertices; i++) {
            irr::f32 homeHeight = GeometryInfoList->vertices[i].originalPosition.Y;
            irr::f32 h = homeHeight - 100.0f;

            //set new vertice height
            GeometryInfoList->vertices[i].currPosition.Y = h;

            //need to update vertice position for this vertice!
            //GeometryInfoList->vertices[i].positionDirty = true;

            idxMeshBuf = 0;

            for (it2 = this->GeometryInfoList->vertices[i].myMeshBuffers.begin(); it2 != this->GeometryInfoList->vertices[i].myMeshBuffers.end(); ++(it2)) {
                 (*it2)->grab();
                 void* pntrVert = (*it2)->getVertices();
                 pntrVertices = (S3DVertex*)pntrVert;
                 pntrVertices[this->GeometryInfoList->vertices[i].myMeshBufVertexId[idxMeshBuf]].Pos.Y = GeometryInfoList->vertices[i].currPosition.Y;

                 idxMeshBuf++;

                 (*it2)->drop();

                 //add this Meshbuffer to the list of overall dirty buffers
                // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
             }

            //this->GeometryInfoList->vertices[i].positionDirty = false;
        }

        return;
    }

    //the next commented block is more or less
    //similar in the HioctaneTools source code
    //but seems to be not needed, therefore
    //I commented it out
    /*if (MorphSource == NULL) {
        UpdateGeometry();
        return;
    }*/

     if (MorphSource->GeometryInfoList->vertices.size() != GeometryInfoList->vertices.size()) {
             logging::Error("Column morph - vertice count mismatch");
     }

    u32 nrVertices = (irr::u32)(GeometryInfoList->vertices.size());

    for (unsigned int i = 0; i < nrVertices; i++) {
        irr::f32 sourceHeight = MorphSource->GeometryInfoList->vertices[i].originalPosition.Y;
        irr::f32 homeHeight = GeometryInfoList->vertices[i].originalPosition.Y;
        irr::f32 h = homeHeight * (1.0f - progress) + sourceHeight * progress;

        //if this column is hidden move column vertices
        //so far below the level that the are not visible anymore
        //for the player, therefore the column is "hidden"
        if (this->Hidden) h -= 100.0f;

        //set new vertice height
        GeometryInfoList->vertices[i].currPosition.Y = h;

        //need to update vertice position for this vertice!
        //GeometryInfoList->vertices[i].positionDirty = true;

        idxMeshBuf = 0;

        for (it2 = this->GeometryInfoList->vertices[i].myMeshBuffers.begin(); it2 != this->GeometryInfoList->vertices[i].myMeshBuffers.end(); ++(it2)) {
             (*it2)->grab();
             void* pntrVert = (*it2)->getVertices();
             pntrVertices = (S3DVertex*)pntrVert;
             pntrVertices[this->GeometryInfoList->vertices[i].myMeshBufVertexId[idxMeshBuf]].Pos.Y = GeometryInfoList->vertices[i].currPosition.Y;

             idxMeshBuf++;

             (*it2)->drop();

             //add this Meshbuffer to the list of overall dirty buffers
            // AddDirtySMeshBuffer((*it2), dirtySMeshBuffers);
         }

        //this->GeometryInfoList->vertices[i].positionDirty = false;
    }

    //mark column vertices as dirty
    //this->MyLevelBlocks->SetColumnVerticeSMeshBufferVerticePositionsDirty();
}

void Column::UpdateVertices() {

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

std::vector<vector2d<irr::f32>> Column::MakeUVs(int textureId, int texMod) {
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

void Column::AddNewColumnVertice(vector3d<irr::f32> position, vector2d<irr::f32> uv, vector3d<irr::f32> normal) {
    video::SColor cubeColour3(255,255,255,255);

    ColumnVerticeInfo* pntrVertInfo = new ColumnVerticeInfo();
    pntrVertInfo->originalPosition = position;
    pntrVertInfo->currPosition = position;
    pntrVertInfo->vert = new video::S3DVertex(position.X, position.Y, position.Z, normal.X, normal.Y, normal.Z, cubeColour3, uv.X, uv.Y);

    //add new vertice to list of vertices of this column
    GeometryInfoList->vertices.push_back(*pntrVertInfo);
}

bool Column::setupGeometry() {
    int x, z, i = 0;

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

    GeometryInfoList->vertices.clear();
    GeometryInfoList->textureIdData.clear();
    GeometryInfoList->indicesVboData.clear();

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

    for (int bitNum = 0; bitNum < 8; bitNum++) { // all blocks of this column

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

        // texture atlas UVs
        newuvsS = MakeUVs(blockDef->get_S(), blockDef->SMod());
        textIDInfoS = blockDef->get_S();

        newuvsW = MakeUVs(blockDef->get_W(), blockDef->WMod());
        textIDInfoW = blockDef->get_W();

        newuvsN = MakeUVs(blockDef->get_N(), blockDef->NMod());
        textIDInfoN = blockDef->get_N();

        newuvsE = MakeUVs(blockDef->get_E(), blockDef->EMod());
        textIDInfoE = blockDef->get_E();

        newuvsT = MakeUVs(blockDef->get_T(), blockDef->TMod());
        textIDInfoT = blockDef->get_T();

        newuvsB = MakeUVs(blockDef->get_B(), blockDef->BMod());
        textIDInfoB = blockDef->get_B();

        // normals
        vector3d<irr::f32> *nN = new vector3d<irr::f32>(0.0f, 0.0f, -1.0f);
        vector3d<irr::f32> *nE = new vector3d<irr::f32>(1.0f, 0.0f, 0.0f);
        vector3d<irr::f32> *nS = new vector3d<irr::f32>(0.0f, 0.0f, 1.0f);
        vector3d<irr::f32> *nW = new vector3d<irr::f32>(-1.0f, 0.0f, 0.0f);
        vector3d<irr::f32> *nT = new vector3d<irr::f32>(0.0f, 1.0f, 0.0f);
        vector3d<irr::f32> *nB = new vector3d<irr::f32>(0.0f, -1.0f, 0.0f);

        //create all vertices and store them

        //define all faces

        //north side
        AddNewColumnVertice(*F, newuvsN[0], *nN);
        AddNewColumnVertice(*E, newuvsN[1], *nN);
        AddNewColumnVertice(*A, newuvsN[2], *nN);
        AddNewColumnVertice(*B, newuvsN[3], *nN);
        GeometryInfoList->textureIdData.push_back(textIDInfoN);

        //east side
        AddNewColumnVertice(*G, newuvsE[0], *nE);
        AddNewColumnVertice(*F, newuvsE[1], *nE);
        AddNewColumnVertice(*B, newuvsE[2], *nE);
        AddNewColumnVertice(*C, newuvsE[3], *nE);
        GeometryInfoList->textureIdData.push_back(textIDInfoE);

        //south side
        AddNewColumnVertice(*H, newuvsS[0], *nS);
        AddNewColumnVertice(*G, newuvsS[1], *nS);
        AddNewColumnVertice(*C, newuvsS[2], *nS);
        AddNewColumnVertice(*D, newuvsS[3], *nS);
        GeometryInfoList->textureIdData.push_back(textIDInfoS);

        //west side
        AddNewColumnVertice(*E, newuvsW[0], *nW);
        AddNewColumnVertice(*H, newuvsW[1], *nW);
        AddNewColumnVertice(*D, newuvsW[2], *nW);
        AddNewColumnVertice(*A, newuvsW[3], *nW);
        GeometryInfoList->textureIdData.push_back(textIDInfoW);

        //top side
        AddNewColumnVertice(*E, newuvsT[0], *nT);
        AddNewColumnVertice(*F, newuvsT[1], *nT);
        AddNewColumnVertice(*G, newuvsT[2], *nT);
        AddNewColumnVertice(*H, newuvsT[3], *nT);
        GeometryInfoList->textureIdData.push_back(textIDInfoT);

        //bottom side
        AddNewColumnVertice(*D, newuvsB[0], *nB);
        AddNewColumnVertice(*C, newuvsB[1], *nB);
        AddNewColumnVertice(*B, newuvsB[2], *nB);
        AddNewColumnVertice(*A, newuvsB[3], *nB);
        GeometryInfoList->textureIdData.push_back(textIDInfoB);

        // indices for 2 tris
        for (int j = 0; j < 6; j++) {
            i = 0;
            GeometryInfoList->indicesVboData.push_back(i);
            GeometryInfoList->indicesVboData.push_back(i + 1);
            GeometryInfoList->indicesVboData.push_back(i + 3);

            GeometryInfoList->indicesVboData.push_back(i + 1);
            GeometryInfoList->indicesVboData.push_back(i + 2);
            GeometryInfoList->indicesVboData.push_back(i + 3);

            //i += 4;
         }

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
     }

    Size.X = segmentSize;
    Size.Y = h;
    Size.Z = segmentSize;

    return true;
}
