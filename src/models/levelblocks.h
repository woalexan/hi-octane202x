/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef LEVELBLOCKS_H
#define LEVELBLOCKS_H

#include "../resources/levelfile.h"
#include "../resources/mapentry.h"
#include <irrlicht.h>
#include "../resources/texture.h"
#include "../infrabase.h"
#include "../utils/logging.h"
#include "column.h"

using namespace irr;
using namespace video;
using namespace core;
using namespace scene;
using namespace io;
using namespace gui;

#define LEVELBLOCKS_VIEW_WIREFRAME 0
#define LEVELBLOCKS_VIEW_DEFAULT 1
#define LEVELBLOCKS_VIEW_DEBUGNORMALS 2

class LevelTerrain; //Forward declaration
class Column;       //Forward declaration
class InfrastructureBase; //Forward declaration

struct ColumnsByPositionStruct {
      int pos;
      Column *pColumn = nullptr;
};

class LevelBlocks {
public:
    LevelBlocks(InfrastructureBase* infra, LevelTerrain* myTerrain, LevelFile* levelRes,
                TextureLoader* textureSource, bool debugShowWallCollisionMesh, bool enableLightning);
    ~LevelBlocks();

    //MapEntry* GetMapEntry(int x, int y);

    SMesh * getBlocksMesh(int collisionSelector);
    std::vector<Column*> ColumnsInRange(int sx, int sz, float w, float h);
    void SetColumnVerticeSMeshBufferVerticePositionsDirty();

    bool GetCurrentCeilingHeightForTileCoord(vector2di cellCoord, irr::f32 &heightVal);

    vector3d<irr::f32> Size;

    bool Blocks_ready;

    //Mesh with blocks that should all be part of craft collision detection
    SMesh *blockMeshForCollision = nullptr;

    //Mesh with blocks that should not be part of craft collision detection
    //(for example used for blocks/cubes that are above the player craft in a ceiling
    //as the craft tends to get stuck below/in them in case we would do
    //collision detection between craft and this blocks
    SMesh *blockMeshWithoutCollision = nullptr;

    IMeshSceneNode *BlockCollisionSceneNode = nullptr;
    IMeshSceneNode *BlockWithoutCollisionSceneNode = nullptr;

    std::vector<ColumnsByPositionStruct> ColumnsByPosition;

    bool searchColumnWithPosition(int posKey, Column* &columnFnd);

    void SwitchViewMode();

private:   
    TextureLoader* mTexSource = nullptr;
    LevelTerrain* MyTerrain = nullptr;
    InfrastructureBase* mInfra = nullptr;

    //std::string m_texfile;

    void addColumn(ColumnDefinition* definition, vector3d<irr::f32> pos, LevelFile *levelRes);

    irr::s32 myCurrentViewMode = LEVELBLOCKS_VIEW_DEFAULT;

    irr::u32 numVertices;
    irr::u32 numIndices;
    irr::u32 numUVs;
    irr::u32 numNormals;

    bool mEnableLightning;

//protected:
public:
     LevelFile* levelRes = nullptr;

     float segmentSize;
};

#endif // LEVELBLOCKS_H
