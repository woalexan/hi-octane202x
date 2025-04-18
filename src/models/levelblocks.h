/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef LEVELBLOCKS_H
#define LEVELBLOCKS_H

#include "../resources/levelfile.h"
#include "../resources/mapentry.h"
#include <irrlicht.h>
#include "../resources/texture.h"
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

struct ColumnsByPositionStruct {
      int pos;
      Column *pColumn;
};

class LevelBlocks {
public:
    LevelBlocks(LevelTerrain* myTerrain, LevelFile* levelRes, irr::scene::ISceneManager *mySmgr, irr::video::IVideoDriver *driver,
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
    SMesh *blockMeshForCollision;

    //Mesh with blocks that should not be part of craft collision detection
    //(for example used for blocks/cubes that are above the player craft in a ceiling
    //as the craft tends to get stuck below/in them in case we would do
    //collision detection between craft and this blocks
    SMesh *blockMeshWithoutCollision;

    IMeshSceneNode *BlockCollisionSceneNode;
    IMeshSceneNode *BlockWithoutCollisionSceneNode;

    std::vector<ColumnsByPositionStruct> ColumnsByPosition;

    void SwitchViewMode();

private:   
    irr::video::IVideoDriver *m_driver;
    TextureLoader* mTexSource;
    LevelTerrain* MyTerrain;
    scene::ISceneManager *MySmgr;

    //std::string m_texfile;

    void addColumn(ColumnDefinition* definition, vector3d<irr::f32> pos, LevelFile *levelRes, irr::video::IVideoDriver *driver, TextureLoader* texureSource);
    bool searchColumnWithPosition(int posKey, Column* &columnFnd);

    irr::s32 myCurrentViewMode = LEVELBLOCKS_VIEW_DEFAULT;

    irr::u32 numVertices;
    irr::u32 numIndices;
    irr::u32 numUVs;
    irr::u32 numNormals;

    bool mEnableLightning;

//protected:
public:
     LevelFile* levelRes;

     float segmentSize;
};

#endif // LEVELBLOCKS_H
