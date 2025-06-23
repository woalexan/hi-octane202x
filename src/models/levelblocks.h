/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef LEVELBLOCKS_H
#define LEVELBLOCKS_H

#include <irrlicht.h>
#include <vector>
#include <cstdint>

using namespace irr;
using namespace video;
using namespace core;
using namespace scene;
using namespace io;
using namespace gui;

#define LEVELBLOCKS_VIEW_OFF 0
#define LEVELBLOCKS_VIEW_WIREFRAME 1
#define LEVELBLOCKS_VIEW_DEFAULT 2
#define LEVELBLOCKS_VIEW_DEBUGNORMALS 3

#define DEF_SELBLOCK_FACENONE 0
#define DEF_SELBLOCK_FACENORTH 1
#define DEF_SELBLOCK_FACEEAST 2
#define DEF_SELBLOCK_FACESOUTH 3
#define DEF_SELBLOCK_FACEWEST 4
#define DEF_SELBLOCK_FACETOP 5
#define DEF_SELBLOCK_FACEBOTTOM 6

/************************
 * Forward declarations *
 ************************/

class LevelTerrain;
class Column;
class InfrastructureBase;
class TextureLoader;
class LevelFile;
class ColumnDefinition;

struct ColumnsByPositionStruct {
      int pos;
      Column *pColumn = nullptr;
};

class LevelBlocks {
public:
    LevelBlocks(InfrastructureBase* infra, LevelTerrain* myTerrain, LevelFile* levelRes,
                TextureLoader* textureSource, bool debugShowWallCollisionMesh, bool enableLightning);
    ~LevelBlocks();

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

    void SetViewMode(irr::u8 newViewMode);
    irr::u8 GetCurrentViewMode();

    void DrawOutlineSelectedColumn(Column* selColumnPntr, int nrBlockFromBase, irr::video::SMaterial* color, SMaterial* selFaceColor, irr::u8 selFace = DEF_SELBLOCK_FACENONE);

    //Derives the current texturing information about a selected block face
    //returns true if the information was found, false otherwise
    bool GetTextureInfoSelectedBlock(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks, irr::u8 selFace, int16_t& outCurrTextureId, uint8_t& outCurrTextureModification);

    irr::u16 mNrBlocksInLevel = 0;

private:   
    TextureLoader* mTexSource = nullptr;
    LevelTerrain* MyTerrain = nullptr;

    InfrastructureBase* mInfra = nullptr;

    void addColumn(ColumnDefinition* definition, vector3d<irr::f32> pos, LevelFile *levelRes);
    irr::u8 mCurrentViewMode;

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
