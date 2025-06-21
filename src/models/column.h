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
struct ColumnVerticeInfo {
   //pointer to the vertex
   video::S3DVertex *vert = nullptr;

   //stores the vertex index for all the Meshbuffers where this
   //vertex is part of
   std::vector<irr::u32> myMeshBufVertexId;

   //we need to keep pointer to my meshbuffer,
   //to be able to set it dirty if we have changed this vertice dynamically
   //during morphing
   std::vector<irr::scene::SMeshBuffer*> myMeshBuffers;

   //original position before morphing
   //never modified, used as reference for morphs
   vector3d<irr::f32> originalPosition;

   //current position, could be modified due to
   //morphing of column
   vector3d<irr::f32> currPosition;

   //bool positionDirty = false;
};

struct ColumnSideGeometryInfo {
   std::vector<int> indicesVboData;
   //std::vector<vector3d<irr::f32>> OriginalVertices;
   //std::vector<vector3d<irr::f32>> positionVboData;
   //std::vector<vector3d<irr::f32>> normalVboData;
   //std::vector<vector2d<irr::f32>> uvVboData;

    //Texture ID information for each face of the cubes
    //of the column
    std::vector<int> textureIdData;

    std::vector<ColumnVerticeInfo> vertices;
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

private:
    bool setupGeometry();
    std::vector<vector2d<irr::f32>> ApplyTexMod(vector2d<irr::f32> uvA, vector2d<irr::f32> uvB, vector2d<irr::f32> uvC, vector2d<irr::f32> uvD, int mod);
    std::vector<vector2d<irr::f32>> MakeUVs(int texMod);

    void AddNewColumnVertice(vector3d<irr::f32> position, vector2d<irr::f32> uv, vector3d<irr::f32> normal);
    irr::f32 GetCurrentHeightTile(int x, int z);
    irr::f32 GetOriginalHeightTile(int x, int z);

    void UpdateVertices();

    LevelBlocks* MyLevelBlocks = nullptr;
    LevelTerrain* MyTerrain = nullptr;

    float segmentSize;

    irr::u16 mNrBlocksInColumn = 0;

//protected:
public:
    vector3d<irr::f32> Position;
    ColumnDefinition* Definition = nullptr;

    ColumnSideGeometryInfo* GeometryInfoList = nullptr;

    LevelFile* levelRes = nullptr;
};

#endif // COLUMN_H
