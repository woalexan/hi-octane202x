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
#include <string>

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
class BlockDefinition;
class IrrMeshBuf;
class EditorSession;
struct MeshObjectStatsStruct;
struct MeshBufferInfoStruct;
struct BlockFaceInfoStruct;
struct BlockInfoStruct;

struct ColumnsByPositionStruct {
      int pos;
      Column *pColumn = nullptr;
};

class LevelBlocks {
public:
    LevelBlocks(InfrastructureBase* infra, LevelTerrain* myTerrain, LevelFile* levelRes,
                TextureLoader* textureSource, bool levelEditorMode, bool debugShowWallCollisionMesh, bool enableLightning, bool enableBlockPreview);
    ~LevelBlocks();

    void CreateBlocksMesh();
    std::vector<Column*> ColumnsInRange(int sx, int sz, float w, float h);
    void SetColumnVerticeSMeshBufferVerticePositionsDirty();

    bool GetCurrentCeilingHeightForTileCoord(vector2di cellCoord, irr::f32 &heightVal);

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

    bool SearchColumnWithPosition(int posKey, Column* &columnFnd);

    void SetViewMode(irr::u8 newViewMode);
    irr::u8 GetCurrentViewMode();

    void DrawOutlineSelectedColumn(Column* selColumnPntr, int nrBlockFromBase, irr::video::SMaterial* color, SMaterial* selFaceColor, irr::u8 selFace = DEF_SELBLOCK_FACENONE);
    void DrawColumnSelectionGrid(Column* selColumnPntr, irr::video::SMaterial* colorGrid, bool drawCurrBlockToBeEdited, int nrBlockToBeEditedStartingFromBase,
                                 irr::video::SMaterial* colorBlockToBeEdited);

    //Derives the current texturing information about a selected block face
    //returns true if the information was found, false otherwise
    bool GetTextureInfoSelectedBlock(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks, irr::u8 selFace, int16_t& outCurrTextureId, uint8_t& outCurrTextureModification);

    irr::u16 mNrBlocksInLevel = 0;

    void TestHeightChange(Column* selColumnPntr, int mSelBlockNrSkippingMissingBlocks);

    void SetCubeFaceTexture(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks,
                                         irr::u8 selFace, bool updateTexId, int16_t newTextureId, bool updateTexMod, uint8_t newTextureModifier);

    void UpdateCubeFaceTextureModification(Column* selColumnPntr, int mSelBlockNrSkippingMissingBlocks,
                                                        BlockFaceInfoStruct* whichFace, uint8_t newTextureModifier, bool SetMeshDirty = true);

    void RemoveMeshCube(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks);
    void RemoveMeshColumn(Column* selColumnPntr);
    void RemoveColumn(Column* selColumnPntr);
    void RemoveBlock(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks);

    void AddColumnAtCell(int x, int y, ColumnDefinition* newColumDef);
    void AddMeshColumn(Column* column);
    void AddBlock(Column* selColumnPntr, int nrBlockFromBase, int mSelBlockNrSkippingMissingBlocks, BlockDefinition* whichBlockType);

    std::vector<vector2d<irr::f32>> ApplyTexMod(vector2d<irr::f32> uvA, vector2d<irr::f32> uvB, vector2d<irr::f32> uvC, vector2d<irr::f32> uvD, int mod);
    std::vector<vector2d<irr::f32>> MakeUVs(int texMod);

    std::vector<irr::u32> GetBlockDefinitionUsageCount();
    std::vector<irr::u32> GetColumnDefinitionUsageCount();

    void DebugWriteColumnDefinitionTableToCsvFile(char* debugOutPutFileName);
    void DebugWriteBlockDefinitionTableToCsvFile(char* debugOutPutFileName);
    void DebugWriteDefinedColumnsTableToCsvFile(char* debugOutPutFileName);

    void ChangeMeshCubeHeight(BlockInfoStruct* whichCube, irr::f32 newV1y, irr::f32 newV2y, irr::f32 newV3y, irr::f32 newV4y);

    void UpdateBlockDefinitionUsageCnt();
    void UpdateBlockMesh();

    void CreateBlockPreview(irr::video::ITexture& outputFrontTexture, irr::video::ITexture& outputBackTexture);

    irr::video::ITexture* texPreviewFrontNoCube = nullptr;
    irr::video::ITexture* texPreviewBackNoCube = nullptr;

    irr::video::ITexture* GetBlockPreviewImage(Column* selColumn, int blockNrStartingFromBase, bool front);
    void UpdatePreviewBlockMesh(BlockDefinition* previewBlockDef);
    BlockDefinition* mCurrentPreviewedBlockDefinition = nullptr;

    void UpdatePreviewForBlockDefinition(BlockDefinition* blockDef);

private:   
    IrrMeshBuf* mIrrMeshBuf = nullptr;

    //Render To Target texture
    //needed for block preview
    video::ITexture* mRenderToTargetTex = nullptr;

    irr::core::vector2d<u32> mCubePreviewImageSize;

    //only used for the level editor; Minimum number
    //of needed meshbuffers for each textureId so that
    //user can add new cube faces (with before unused textureIds)
    irr::u8 mLevelEditorMinNrMeshBuffersNeeded;

    //a vector containing a MeshbufferInfoStruct (+Meshbuffer)
    //for each possible textureId of the terrain (cube faces) (256 different texture Ids)
    //this one with collision detection active
    std::vector<MeshBufferInfoStruct*> mBlockwCollMeshBufferVec;

    //this one with collision detection active
    std::vector<MeshBufferInfoStruct*> mBlockwoCollMeshBufferVec;

    TextureLoader* mTexSource = nullptr;
    LevelTerrain* MyTerrain = nullptr;

    InfrastructureBase* mInfra = nullptr;

    void AddColumn(ColumnDefinition* definition, vector3d<irr::f32> pos, LevelFile *levelRes);

    void DrawOutlineSelectedFace(BlockFaceInfoStruct* selFace, SMaterial* color);

    std::string CreateDbgShapeString(ColumnDefinition* colDef);

    void ChangeMeshCubeFaceHeight(BlockFaceInfoStruct* whichFace, irr::f32 newV1y, irr::f32 newV2y, irr::f32 newV3y, irr::f32 newV4y);

    BlockInfoStruct* GetBlockInfoStruct(Column* selColumnPntr, int mSelBlockNrSkippingMissingBlocks);
    void RemoveUnusedBlockDefinitions();
    //void RemoveUnusedBlockDefinitions(bool excludeActive, irr::u32 excludeId, bool reduceCntByOneForIdActive, irr::u32 reduceCntByOneForId);
    void ReplaceBlockDefinitionIdWithNewOneInAllColumdefinitions();

    void ReplaceColumnDefinitionWithNewOneForAllColumns(std::vector<irr::u32> changeFromIdVec, std::vector<irr::u32> changeToIdVec);
    //void RemoveUnusedColumnDefinitions(bool excludeActive, irr::u32 excludeId, bool reduceCntByOneForIdActive, irr::u32 reduceCntByOneForId);
    void RemoveUnusedColumnDefinitions();

    //Returns true if the specified column was found, false otherwise
    //If search succesfull outCoord output parameter returns the found map coordinates
    //(map entry coordinates) for this specified column
    bool FindMapCoordinateForColumn(Column* whichColumn, irr::core::vector2di& outCoord);

    //update certain values stored in the column definitions
    //that also the original game uses
    void UpdateColumDefinitions();

    //Returns simply 0 value if columnDefPntr is nullptr!
    irr::u8 GetColumnDefinitionShapeValue(ColumnDefinition* columnDefPntr);

    irr::u8 mCurrentViewMode;

    MeshObjectStatsStruct* mBlocksMeshStats;

    bool mEnableLightning;

    bool mLevelEditorMode;
    bool mEnableBlockPreview;

    //Special block definition for preview image creation (using render to target)
    BlockDefinition* mBlockPreviewBlockDef = nullptr;

    //Special column definition for preview image creation (using render to target)
    ColumnDefinition* mBlockPreviewColumnDef = nullptr;

    //Special column for block definition preview image creation (using render to target)
    Column* mBlockPreviewColumn = nullptr;

    irr::scene::ICameraSceneNode* mPreviewCameraFront = nullptr;
    irr::scene::ICameraSceneNode* mPreviewCameraBack = nullptr;

    void SetupBlockPreview();
    void CreateAllBlockDefinitionPreviews();
    void CleanUpBlockReview();

//protected:
public:
     LevelFile* levelRes = nullptr;

     float segmentSize;
};

#endif // LEVELBLOCKS_H
