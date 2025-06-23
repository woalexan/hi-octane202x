/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did translation to C++, and then modified and extended code for my project)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef LEVELTERRAIN_H
#define LEVELTERRAIN_H

#include "irrlicht.h"
#include <vector>
#include "../resources/levelfile.h"
#include "player.h"

using namespace irr;
using namespace video;
using namespace core;
using namespace scene;
using namespace io;
using namespace gui;

#define LEVELTERRAIN_VIEW_OFF 0
#define LEVELTERRAIN_VIEW_WIREFRAME 1
#define LEVELTERRAIN_VIEW_DEFAULT 2
#define LEVELTERRAIN_VIEW_DEBUGNORMALS 3

typedef SColor colour_func(f32 x, f32 y, f32 z);

#define CELL_OPTIMIZATION_THRESHOLD 0.0f  //Optimization Threshold in percent;

#define DEF_LEVELTERRAIN_HEIGHTMAP_COLLISION_THRES 0.9f;  //steepness threshold to trigger
                                                          //collision with terrain tile map

/************************
 * Forward declarations *
 ************************/

class Morph;
class InfrastructureBase;
struct TerrainTileData;
class TextureLoader;
class MapEntry;

struct TerrainTileData {
    //pointers to my 4 vertices per tile to be able to morph Terrain
    //later during game (so that we have access to vertice data)
    video::S3DVertex *vert1 = nullptr;
    video::S3DVertex *vert2 = nullptr;
    video::S3DVertex *vert3 = nullptr;
    video::S3DVertex *vert4 = nullptr;

    //stores the tile vertex1 index for all the Meshbuffers where this
    //vertex is part of
    std::vector<irr::u32> myMeshBufVertexId1;
    //stores the tile vertex2 index for all the Meshbuffers where this
    //vertex is part of
    std::vector<irr::u32> myMeshBufVertexId2;
    //stores the tile vertex3 index for all the Meshbuffers where this
    //vertex is part of
    std::vector<irr::u32> myMeshBufVertexId3;
    //stores the tile vertex4 index for all the Meshbuffers where this
    //vertex is part of
    std::vector<irr::u32> myMeshBufVertexId4;

    //to allow easier recalculation of vertice normals during Terrain morphing also
    //store my vertice position for Y-axis information for every vertice here (positionVboData)
    irr::f32 vert1CurrPositionY = 0.0f;
    irr::f32 vert2CurrPositionY = 0.0f;
    irr::f32 vert3CurrPositionY = 0.0f;
    irr::f32 vert4CurrPositionY = 0.0f;

    bool vert1CurrPositionYDirty = false;
    bool vert2CurrPositionYDirty = false;
    bool vert3CurrPositionYDirty = false;
    bool vert4CurrPositionYDirty = false;

    //for easier morphing also store my current vertices normals here
    vector3d<irr::f32> vert1CurrNormal;
    vector3d<irr::f32> vert2CurrNormal;
    vector3d<irr::f32> vert3CurrNormal;
    vector3d<irr::f32> vert4CurrNormal;

    bool RefreshNormals = false;

    //for easier morphing also store my current vertice UVs coordinates here
    vector2d<irr::f32> vert1UVcoord;
    vector2d<irr::f32> vert2UVcoord;
    vector2d<irr::f32> vert3UVcoord;
    vector2d<irr::f32> vert4UVcoord;

    bool VertUpdatedUVScoord = false;

    //percentage value which is used to describe how many neighbor cells have differences in height or
    //texture coordinates to find out which cells can be optimized out for rendering purposes
    //a percentage of 0 means 0% neighboring cells are different to this cell; as higher the number as more likely
    //the current cell is part of the level itself that needs to be rendered
    float m_optimization_cnt = 0.0f;
    //remember final result of optimization process
    bool m_draw_in_mesh = true; //default is to draw the title/cell

    //Dynamic mesh means this tile is
    //part of a morph, and is updated during the game
    bool dynamicMesh = false;

    //we need to keep pointer to my meshbuffer,
    //to be able to set it dirty if we have changed a vertices dynamically
    std::vector<irr::scene::SMeshBuffer*> myMeshBuffers;

    //stores the current averaged tile height
    //is for example needed for player craft calculations
    //afterwards; when the Terrain does morph this value
    //keeps to be correct
    irr::f32 currTileHeight = 0.0f;
};

class LevelTerrain {
public:
    LevelTerrain(InfrastructureBase* infra, char* name, LevelFile* levelRes, TextureLoader* textureSource, bool optimizeMesh, bool enableLightning);
    ~LevelTerrain();

    void FinishTerrainInitialization();

    void ResetTerrainTileData();

    void ApplyMorph(Morph morph);
    vector3d<irr::f32> Size;
    bool Terrain_ready;
    TerrainTileData pTerrainTiles[LEVELFILE_WIDTH][LEVELFILE_HEIGHT];

    //static terrain mesh and SceneNode that is not affected
    //by any defined level morphs
    ISceneNode *StaticTerrainSceneNode = nullptr;
    SMesh *myStaticTerrainMesh = nullptr;

    //dynamic terrain mesh and SceneNode that is affected
    //by level morphs
    ISceneNode *DynamicTerrainSceneNode = nullptr;
    SMesh *myDynamicTerrainMesh = nullptr;

    LevelFile* levelRes = nullptr;

    irr::f32 segmentSize;

    irr::u16 get_width();
    irr::u16 get_heigth();

    void DrawOutlineSelectedCell(irr::core::vector2di selCellCoordinate, irr::video::SMaterial* color);

    irr::f32 GetCurrentTerrainHeightForWorldCoordinate(irr::f32 x, irr::f32 z, vector2di &outCellCoord);
    irr::f32 GetHeightInterpolated(irr::f32 x, irr::f32 z);
    MapEntry* GetMapEntry(int x, int y);
    irr::core::vector2di GetClosestTileGridCoordToMapPosition(irr::core::vector3df mapPosition, int &outNrVertice);
    void ForceTileGridCoordRange(irr::core::vector2di &tileGridPos);

    void SetViewMode(irr::u8 newViewMode);
    irr::u8 GetCurrentViewMode();

    irr::video::IImage* CreateMiniMapInfo(irr::u32 &startWP, irr::u32 &endWP, irr::u32 &startHP, irr::u32 &endHP);
    vector3d<irr::f32> computeNormalFromPositionsBuffer(irr::s32 x, irr::s32 z, irr::f32 intensity);
    void CheckPosInsideChargingRegion(int posX, int posY, bool &chargeShield, bool &chargeFuel, bool &chargeAmmo);
    std::vector<irr::core::vector3df> GetPlayerRaceTrackStartLocations();

    //next function is used for terrain tile based collision
    //detection
    //x1, z1 = current player location tile, tile which defines in which
    //direction of steepness of neighboring tile is calculated
    //x2, z2 = tile location of neighboring tile for which the steepness should
    //be calculated
    irr::f32 GetSteepnessOfNeighboringTile(int x1, int z1, int x2, int z2, irr::core::vector3df &collPlanePos1, irr::core::vector3df &collPlanePos2,
                                           irr::core::vector3df &collResolutionDirVec);

    void GetCollisionPlaneBetweenNeighboringTiles(int x1, int z1, int x2, int z2,
                           irr::core::vector3df &collPlanePos1, irr::core::vector3df &collPlanePos2,
                           irr::core::vector3df &collResolutionDirVec);

    vector3d<irr::f32> computeTileSurfaceNormalFromPositionsBuffer(irr::s32 x, irr::s32 z);
    bool CheckPosInsideRegion(int posX, int posY, MapTileRegionStruct* regionStructPntr);

    //definition of road texture elements
    //most important textures for auto generation of
    //minimap
    std::vector<irr::s32> roadTexIdsVec = {31, 60, 61, 62, 63, 64, 65, 67, 69, 76,
                                            77, 78, 79, 81, 82, 86, 116, 117, 118, 119,
                                           122, 124, 125, 126, 127, 128, 129, 130, 131, 132, 136, 137, 138, 139, 140,
                                           141, 142, 143, 144, 145, 148, 149, 150, 151, 152, 154, 155,
                                           96, 4};

    //some more additional texture IDs for roads which would disturb
    //the minimap creation if we would add them to roadTexIdsVec directly
    //but we need them in worldaware to properly detect the local width of the race track
    std::vector<irr::s32> roadTexIdsVecExtendedForWorldAware = {
        43, 47, 51, 120, 121,             123, 134, 135, 146, 147, 153 };

    //texture IDs we can use the detect charging areas
    std::vector<irr::s32> chargerTexIdsVec = {0, 2, 43, 47, 51, 31, 60, 61, 62, 63, 64, 65, 67, 69, 76,
                                            77, 78, 79, 81, 82, 86, 116, 117, 118, 119,
                                           122, 124, 125, 126, 127, 128, 129, 130, 131, 132, 136, 137, 138, 139, 140,
                                           141, 142, 143, 144, 145, 148, 149, 150, 151, 152, 154, 155,
                                           96, 4};

    bool IsRoadTexture(irr::s32 texture, bool addExtendedTextures = false);
    bool IsChargingStationTexture(irr::s32 texture);

    void SetCellTexture(int posX, int posY, int16_t newTextureId, int8_t newTextureModifier);

    //void DebugOutputFoundChargingTextures();

private:
    bool setupGeometry();
    void findTerrainOptimization();
    bool Terrain_Optimization_isValid_Cell_coordinate(int xcoord, int zcoord);
    int TerrainOptimization_compareCells(MapEntry *MiddleCell, MapEntry *Neighborcell);
    vector3d<irr::f32> computeNormalFromMapEntries(int x, int z, float intensity);
    std::vector<vector2d<irr::f32>> ApplyTexMod(vector2d<irr::f32> uvA, vector2d<irr::f32> uvB, vector2d<irr::f32> uvC, vector2d<irr::f32> uvD, int mod);
    std::vector<vector2d<irr::f32>> MakeUVs(int texMod);

    std::vector<SMeshBuffer*> meshBuffers;
    std::vector<int16_t> meshBufferTexIdVec;

    void CreateTerrainMesh();
    irr::u16 GetMeshBufferIndexForTextureId(int posX, int posY, int16_t newTextureId);

    //only used for debugging
    //std::vector<irr::s32> mDbgChargerTexFound;

    InfrastructureBase* mInfra = nullptr;

    TextureLoader* mTexSource = nullptr;
    //std::string m_texfile;

    bool mEnableLightning;
    bool mOptimizeMesh;

    char mName[50];

    /*void AddDirtySMeshBuffer(irr::scene::SMeshBuffer *newDirtyBuffer,
                                           std::vector<irr::scene::SMeshBuffer*> &currDirtyList);*/

    void RecalculateNormals(MapEntry *entry);
    irr::f32 GetAveragedTileHeight(int x, int z);

    std::vector<int> indicesVboDataStatic;
    std::vector<int> textureIdDataStatic;

    std::vector<int> indicesVboDataDynamic;
    std::vector<int> textureIdDataDynamic;

    irr::u32 numVertices;
    irr::u32 numIndices;
    irr::u32 numUVs;
    irr::u32 numNormals;

    irr::u8 mCurrentViewMode;
};

#endif // LEVELTERRAIN_H
