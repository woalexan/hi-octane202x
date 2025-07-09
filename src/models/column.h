/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did translation to C++, and then modified code for my project)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef COLUMN_H
#define COLUMN_H

#include "irrlicht.h"
#include <vector>

using namespace irr;
using namespace video;
using namespace core;
using namespace scene;
using namespace io;
using namespace gui;

//struct needed for morphing, as
//we need to keep more information
//for later modification of SMeshBuffers
struct BlockFaceInfoStruct {
   //pointer to my vertices of the block face
   video::S3DVertex *vert1 = nullptr;
   video::S3DVertex *vert2 = nullptr;
   video::S3DVertex *vert3 = nullptr;
   video::S3DVertex *vert4 = nullptr;

   //stores the vertex index for all the Meshbuffers where this
   //vertices are part of
   std::vector<irr::u32> myMeshBufVertexId;

   //we need to keep pointer to my meshbuffer,
   //to be able to set it dirty if we have changed this vertice dynamically
   //during morphing
   std::vector<irr::scene::SMeshBuffer*> myMeshBuffers;

   //the textureId that is used to render
   //this face of the cube
   int16_t textureId;

   //original position before morphing
   //never modified, used as reference for morphs
   vector3d<irr::f32> originalPositionVert1;
   vector3d<irr::f32> originalPositionVert2;
   vector3d<irr::f32> originalPositionVert3;
   vector3d<irr::f32> originalPositionVert4;

   //current position, could be modified due to
   //morphing of column
   vector3d<irr::f32> currPositionVert1;
   vector3d<irr::f32> currPositionVert2;
   vector3d<irr::f32> currPositionVert3;
   vector3d<irr::f32> currPositionVert4;
};

struct BlockInfoStruct {
   BlockFaceInfoStruct* fN = nullptr;
   BlockFaceInfoStruct* fE = nullptr;
   BlockFaceInfoStruct* fS = nullptr;
   BlockFaceInfoStruct* fW = nullptr;
   BlockFaceInfoStruct* fT = nullptr;
   BlockFaceInfoStruct* fB = nullptr;
};

/************************
 * Forward declarations *
 ************************/

class LevelTerrain;
class LevelBlocks;
class ColumnDefinition;
struct ColumnSideGeometryInfo;
struct ColumnsByPositionStruct;
class LevelFile;
class MapEntry;

class Column {
public:
    Column(LevelTerrain* myTerrain, LevelBlocks* myLevelBlocks, ColumnDefinition* Def, vector3d<irr::f32> pos, LevelFile* levelRes);
    ~Column();

    MapEntry* GetMapEntry(int x, int y);

    vector3d<irr::f32> Size;

    bool DestroyOnMorph;
    bool Hidden;

    bool column_ready;

    //all stuff for morphing
    Column* MorphSource = nullptr;

    void ApplyMorph(float progress);

    irr::u16 GetNumberContainedBlocks();

    //returns the number of "missing" blocks at the base
    //of the column until the first block is found
    //can be used to detect tunnel roof elements etc...
    //if there is not a single existing block in the column
    //still will return 0, because then there is technically
    //no "gap" of blocks
    irr::u16 GetNumberMissingBlocksAtBase();

private:
    bool SetupGeometry();
    void MoveColumnVertex(irr::core::vector3df &vertex);

    void CleanUpBlockFaceInfoStruct(BlockFaceInfoStruct &pntr);
    void CleanUpBlockInfoStruct(BlockInfoStruct &pntr);

    BlockFaceInfoStruct* CreateNewCubeFace(vector3d<irr::f32> v1,
                                                   vector3d<irr::f32> v2,
                                                   vector3d<irr::f32> v3,
                                                   vector3d<irr::f32> v4,
                                                   std::vector<vector2d<irr::f32>> uv, vector3d<irr::f32> normal,
                                                   int textureId);

    std::vector<vector2d<irr::f32>> ApplyTexMod(vector2d<irr::f32> uvA, vector2d<irr::f32> uvB, vector2d<irr::f32> uvC, vector2d<irr::f32> uvD, int mod);
    std::vector<vector2d<irr::f32>> MakeUVs(int texMod);

    //void AddNewColumnVertice(vector3d<irr::f32> position, vector2d<irr::f32> uv, vector3d<irr::f32> normal);
    irr::f32 GetCurrentHeightTile(int x, int z);
    irr::f32 GetOriginalHeightTile(int x, int z);

    LevelBlocks* MyLevelBlocks = nullptr;
    LevelTerrain* MyTerrain = nullptr;

    float segmentSize;

    irr::u16 mNrBlocksInColumn = 0;

//protected:
public:
    //vector which contains all cubes of this column
    std::vector<BlockInfoStruct*> mBlockInfoVec;

    vector3d<irr::f32> Position;
    ColumnDefinition* Definition = nullptr;

    ColumnSideGeometryInfo* GeometryInfoList = nullptr;

    LevelFile* levelRes = nullptr;
};

#endif // COLUMN_H
