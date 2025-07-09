/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef IRRMESHBUF_H
#define IRRMESHBUF_H

#include <irrlicht.h>
#include <vector>
#include <cstdint>

using namespace irr;
using namespace video;
using namespace core;
using namespace scene;
using namespace io;
using namespace gui;

struct MeshBufferInfoStruct {
    irr::scene::SMeshBuffer* meshBuf = nullptr;

    //pointer to the next MeshBufferInfoStruct if necessary
    //because every Meshbuffer can hold max 65535 indices
    //is nullptr if there is no additional meshbuffer anymore
    MeshBufferInfoStruct* nextPntr = nullptr;

    irr::u16 remainingIndices = 0;

    //stores the textureId of the material
    //inside this meshbuffer
    int16_t textureId;
};

struct MeshObjectStatsStruct {
    irr::u32 numVertices;
    irr::u32 numIndices;
    irr::u32 numUVs;
    irr::u32 numNormals;
};

/************************
 * Forward declarations *
 ************************/

class TerrainTileData;
class TextureLoader;
struct BlockFaceInfoStruct;
struct BlockInfoStruct;

class IrrMeshBuf {
public:
    IrrMeshBuf(TextureLoader* texSource, bool enableLighning);
    ~IrrMeshBuf();

    void InitializeMeshBufferInfoStructs(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec);
    void CleanupMeshBufferInfoStructs(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec);

    //adds an additional Meshbuffer for the specified textureId (material). Returns
    //a pointer to the new added MeshBufferInfoStruct. In case something goes wrong
    //returns nullptr
    MeshBufferInfoStruct* AddAdditionalMeshBuffer(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId);

    //counts the number of existing Meshbuffers for all possible
    //Texture Ids
    std::vector<irr::u8> ReturnMeshBufferCntPerTextureId(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec);
    std::vector<irr::scene::SMeshBuffer*> ReturnAllMeshBuffersForTextureId(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t textureId);

    //used for LevelTerrain
    void AddMeshBufferTile(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, TerrainTileData* tilePntr, int16_t textureId, MeshObjectStatsStruct &statpntr);
    void RemoveMeshBufferTile(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, TerrainTileData* tilePntr, MeshObjectStatsStruct &statpntr);

    //user for LevelBlocks (building cubes)
    void AddMeshBufferCubeFace(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, BlockFaceInfoStruct* blockFaceInfo, MeshObjectStatsStruct &statpntr);
    void AddMeshBufferBlock(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, BlockInfoStruct* blockInfo, MeshObjectStatsStruct &statpntr);
    void RemoveMeshBufferCubeFace(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, BlockFaceInfoStruct* blockFaceInfo, MeshObjectStatsStruct &statpntr);

    void ResetMeshStats(MeshObjectStatsStruct* statPntr);

    int GetNrTextures();

private:
    TextureLoader* mTexSource = nullptr;
    int mAvailableTextureCount;

    bool mEnableLightning;

    //finds the current (last in linked list) MeshBuffer info struct for a certain textureId
    //returns nullptr in case of error
    MeshBufferInfoStruct* FindLastMeshBufferInLinkedList(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId);

    //finds the first available MeshBuffer info struct for a certain textureId
    //which has still space for 6 additional indices (an additional Quad)
    //returns nullptr in case of error, or nothing available
    MeshBufferInfoStruct* FindFirstMeshBufferForAdditionalQuad(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId);

    //returns nullptr in case appropriate MeshBufferInfoStruct is not found
    MeshBufferInfoStruct* FindMeshBufferInfoStructForMeshBuffer(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, irr::scene::SMeshBuffer* meshBufToFind);
};

#endif // IRRMESHBUF_H
