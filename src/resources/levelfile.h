/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did just translation to C++, and extended with some new code)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef LEVELFILE_H
#define LEVELFILE_H

#include "irrlicht.h"
#include <vector>
#include <list>
#include <string>
#include <cstdint>

#define LEVELFILE_WIDTH 256
#define LEVELFILE_HEIGHT 160

#define LEVELFILE_REGION_UNDEFINED 0
#define LEVELFILE_REGION_CHARGER_SHIELD 1
#define LEVELFILE_REGION_CHARGER_FUEL 2
#define LEVELFILE_REGION_CHARGER_AMMO 3
#define LEVELFILE_REGION_START 4
#define LEVELFILE_REGION_TRIGGERCRAFT 5
#define LEVELFILE_REGION_TRIGGERMISSILE 6

using namespace irr::core;
using namespace std;

/************************
 * Forward declarations *
 ************************/

class BlockDefinition;
class MapEntry;
class EntityItem;
class ColumnDefinition;

struct MapPointOfInterest {
      irr::core::vector3d<float> Position;
      int Value;
};

struct MapTileRegionStruct {
    irr::u8 regionId;
    irr::u8 regionType = LEVELFILE_REGION_UNDEFINED;
    irr::u16 tileXmin;
    irr::u16 tileXmax;
    irr::u16 tileYmin;
    irr::u16 tileYmax;
    irr::core::vector2di regionCenterTileCoord;

    //allows to store trigger target group for
    //craft trigger regions and missile triggers
    int16_t mTargetGroup = 0;

    bool mOnlyTriggerOnce = false;
    bool mAlreadyTriggered = false;
};

struct ColumnsStruct {
      irr::core::vector3d<float> Vector3;
      ColumnDefinition *Columns = nullptr;
};

class LevelFile {
public:
    LevelFile(std::string filename);
    ~LevelFile();

    int Width();
    int Height();

    std::string get_Filename();
    std::string get_Name();

    void set_Filename(std::string new_Filename);
    void set_Name(std::string new_Name);

    bool get_Ready();
    void set_Ready(bool newstate);

    bool Save(std::string filename);

    MapEntry* pMap[LEVELFILE_WIDTH][LEVELFILE_HEIGHT];

    std::list<MapPointOfInterest> PointsOfInterest;

    bool ReturnEntityItemWithId(int searchId, EntityItem **fndItem);
    std::vector<EntityItem*> Entities;

    std::vector<BlockDefinition*> BlockDefinitions;
    std::vector<ColumnDefinition*> ColumnDefinitions;

    void DebugWriteColumnDefinitionTableToCsvFile(char* debugOutPutFileName);
    void DebugWriteBlockDefinitionTableToCsvFile(char* debugOutPutFileName);

    std::vector<ColumnsStruct> Columns;

    //table starts at 0x00000000 offset until 0x0000000f offset, and is 16 bytes long
    std::vector<uint8_t> unknownTable0Data;

    //table starts at 0x00017700 offset until 0x00017eff offset, and is 2048 bytes long
    std::vector<uint8_t> unknownTable96000Data;

    //table starts at 0x000226d0 offset until 0x0002271f offset, and is 80 bytes long
    std::vector<uint8_t> unknownTable141008Data;

    //table starts at 0x00023030 offset until 0x0002306f offset, and is 64 bytes long
    std::vector<uint8_t> unknownTable143408Data;

    //table starts at 0x00023990 offset until 0x000239cf  offset, and is 64 bytes long
    std::vector<uint8_t> unknownTable145808Data;

    //table starts at 0x000242e0 offset until 0x0002431f offset,  and is 64 bytes long
    std::vector<uint8_t> unknownTable148192Data;

    //table starts at 0x00024c40 offset until 0x00024c7f offset,  and is 64 bytes long
    std::vector<uint8_t> unknownTable150592Data;

    //table starts at 0x00025590 offset until 0x000255cf offset,  and is 64 bytes long
    std::vector<uint8_t> unknownTable152976Data;

    //table starts at 0x00025ef0 offset until 0x00025f2f offset, and is 64 bytes long
    std::vector<uint8_t> unknownTable155376Data;

    //table starts at 0x00026840 offset until 0x0002687f offset, and is 64 bytes long
    std::vector<uint8_t> unknownTable157760Data;

    //table starts at 0x000271a0 offset until 0x000271bf offset, and is 32 bytes long
    std::vector<uint8_t> unknownTable160160Data;

    std::vector<uint8_t> unknownTable247264Data;  //this table has 340 bytes (contains region definitions)

    //table starts at 0x00047500 offset until 0x0004757f offset, and is 128 bytes long
    std::vector<uint8_t> unknownTable292096Data;

    //table starts at 0x00056a20 offset until 0x00056a8f offset, and is 112 bytes long
    std::vector<uint8_t> unknownTable354848Data;

    //table starts at 0x00056b20 offset until 0x00056b7f offset, and is 96 bytes long
    std::vector<uint8_t> unknownTable355104Data;

    //table starts at 0x00056c20 offset until 0x00056c9f offset, and is 128 bytes long
    std::vector<uint8_t> unknownTable355360Data;

    //table starts at 0x00057520 offset until 0x0005772f offset, and is 528 bytes long
    std::vector<uint8_t> unknownTable357664Data;

    //table starts at 0x000586e0 offset until 0x0005966f offset, and is 3984 bytes long
    std::vector<uint8_t> unknownTable362208Data;

    std::vector<uint8_t> unknownTable358222Data; //this table has 2704 bytes (unknown contents)

    //table starts at 0x00062310 offset until 0x000625bf offset, and is 688 bytes long
    std::vector<uint8_t> unknownTable402192Data;

    bool SaveUnknownTableOffset247264();
    bool SaveUnknownTableOffset358222();
    bool InvestigatePrintUnknownTableOffset247264();

    //holds location info about the charging locations
    //and the map start region/location
    std::vector<MapTileRegionStruct*> *mMapRegionVec = nullptr;

    //Helper function, Returns true if a certain tile has the specified textureId
    //Returns false otherwise, or if the specified location on the tile map is invalid
    bool CheckTileForTextureId(int posX, int posY, int textureId);
    bool CanIFindTextureIdAroundCell(int posX, int posY, int textureId);

    //Makes sure a specified input Blockdefinition exists in the current levelfile.
    //If it does not exist yet it is newly created;
    //Returns true if the requested block definition is available, and the index
    //in the vector of all existing Blockdefinitions is returned in output parameter
    //outIndex;
    //If there is an unexpected error, and no blockdefinition index with this parameters
    //can be supplied, this function returns false
    bool RequestBlockDefinition(uint8_t pN, uint8_t pE, uint8_t pS, uint8_t pW, uint8_t pT, uint8_t pB,
                                       uint8_t pNMod, uint8_t pEMod, uint8_t pSMod, uint8_t pWMod, uint8_t pTMod, uint8_t pBMod, irr::u32 &outIndex);

    //Makes sure a specified input Columndefinition exists in the current levelfile.
    //If it does not exist yet it is newly created;
    //Returns true if the requested column definition is available, and the index
    //in the vector of all existing Columndefinitions is returned in output parameter
    //outIndex;
    //If there is an unexpected error, and no Columndefinition index with this parameters
    //can be supplied, this function returns false
    bool RequestColumnDefinition(int16_t pA, int16_t pB, int16_t pC, int16_t pD, int16_t pE, int16_t pF,
                                             int16_t pG, int16_t pH, irr::u32 &outIndex);

private:
    void DetectAdditionalRegionsTextureId();
    void VerifyRegionFound(irr::core::vector2di startCell, int foundTextureId, irr::u8 expectedRegionType);

    //Helper function for finding charging stations in a third way (used in level 5 and 6)
    //returns true if we find the defined texture at least once at this block of any of the faces
    //if not returns false
    bool CanIFindDefinedTextureAtBlock(BlockDefinition* blockPntr, int textureIdSymbol);

    //Helper function for finding charging stations in a third way (used in level 5 and 6)
    bool CanIFindColumnWithDefinedTextureOnItAtLocation(int posX, int posY, int textureIdSymbol);
    bool CanIFindTextureIdAtColumnsAroundCell(int posX, int posY, int textureId);
    void VerifyRegionFoundViaColumns(irr::core::vector2di startCell, int foundTextureId, irr::u8 expectedRegionType);
    void DetectAdditionalRegionsBasedOnColumns();

    //returns true if a block definition object
    //contains exactly the specified input texture Ids and
    //texture modification values
    //False otherwise
    bool CmpBlockDefinition(BlockDefinition* def, uint8_t compN, uint8_t compE, uint8_t compS, uint8_t compW, uint8_t compT, uint8_t compB,
                                       uint8_t compNMod, uint8_t compEMod, uint8_t compSMod, uint8_t compWMod, uint8_t compTMod, uint8_t compBMod);

    //returns true in case a specified input block definition was found in the vector of existing
    //blockdefinitions in the current level file; false otherwise
    //in case definition was found, returns also the index in the vector for this element
    //in parameter fndOutIndex
    bool SearchBlockDefinitionIndex(uint8_t compN, uint8_t compE, uint8_t compS, uint8_t compW, uint8_t compT, uint8_t compB,
                                    uint8_t compNMod, uint8_t compEMod, uint8_t compSMod, uint8_t compWMod, uint8_t compTMod, uint8_t compBMod, irr::u32 &fndOutIndex);

    //Returns true if new block definition was successfully created, false otherwise
    //if succesfull, returns the new index of the new blockdefinition in output parameter outIndex
    bool AddBlockDefinition(uint8_t newN, uint8_t newE, uint8_t newS, uint8_t newW, uint8_t newT, uint8_t newB,
                                       uint8_t newNMod, uint8_t newEMod, uint8_t newSMod, uint8_t newWMod, uint8_t newTMod, uint8_t newBMod, irr::u32 &outIndex);

    bool CmpColumnDefinition(ColumnDefinition* def, int16_t compA, int16_t compB, int16_t compC, int16_t compD, int16_t compE, int16_t compF,
                                       int16_t compG, int16_t compH);

    //returns true in case a specified input column definition was found in the vector of existing
    //columndefinitions in the current level file; false otherwise
    //in case definition was found, returns also the index in the vector for this element
    //in parameter fndOutIndex
    bool SearchColumnDefinitionIndex(int16_t compA, int16_t compB, int16_t compC, int16_t compD, int16_t compE, int16_t compF,
                                                int16_t compG, int16_t compH, irr::u32 &fndOutIndex);

    //Returns true if new column definition was successfully created, false otherwise
    //if succesfull, returns the new index of the new columndefinition in output parameter outIndex
    bool AddColumnDefinition(int16_t newA, int16_t newB, int16_t newC, int16_t newD, int16_t newE, int16_t newF,
                                        int16_t newG, int16_t newH, irr::u32 &outIndex);

protected:
     std::string m_Filename;
     std::string m_Name;
     bool m_Ready;

     std::vector<uint8_t> m_bytes;

     //for debugging of level write and
     //modification function, disable later
     //to save much memory
     std::vector<uint8_t> m_wBytes;

     bool loadEntitiesTable();
     bool loadBlockTexTable();
     bool loadColumnsTable();
     bool loadMap();

     bool loadMapRegions();
     bool loadUnknownTableOffset358222();
     bool loadUnknownTableOffset0();
     bool loadUnknownTableOffset96000();
     bool loadUnknownTableOffset402192();
     bool loadUnknownTableOffset362208();
     bool loadUnknownTableOffset357664();
     bool loadUnknownTableOffset355360();
     bool loadUnknownTableOffset355104();
     bool loadUnknownTableOffset354848();
     bool loadUnknownTableOffset292096();
     bool loadUnknownTableOffset160160();
     bool loadUnknownTableOffset157760();
     bool loadUnknownTableOffset155376();
     bool loadUnknownTableOffset141008();
     bool loadUnknownTableOffset143408();
     bool loadUnknownTableOffset145808();
     bool loadUnknownTableOffset148192();
     bool loadUnknownTableOffset150592();
     bool loadUnknownTableOffset152976();

     //for levelEditor functionality
     bool saveBlockTexTable();
     bool saveEntitiesTable();
     bool saveColumnsTable();
     bool saveMap();
     bool saveUnknownTables();
};

#endif // LEVELFILE_H
