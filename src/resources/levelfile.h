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
#include "entityitem.h"

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
      irr::core::vector2di cellCoord;
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

    //if region with specified Id is not existing or not defined returns
    //nullptr
    MapTileRegionStruct* GetRegionStructForRegionId(irr::u8 regionId);

    std::vector<ColumnsStruct> Columns;

    //table starts at 0x00000000 offset until 0x00000017 offset, and is 24 bytes long
    std::vector<uint8_t> unknownTable0Data;

    //table starts at 0x00000c60 offset until 0x00000ca7 offset, and is 72 bytes long
    std::vector<uint8_t> unknownTable3168Data;

    //table starts at 0x00000cf0 offset until 0x00017eff offset, and is 94736 bytes long
    std::vector<uint8_t> unknownTable3312Data;

    //table starts at 0x00018EB4 offset until 0x0001e6eb offset, and is 22584 bytes long
    std::vector<uint8_t> unknownTable102068Data;

    //table starts at 0x0001f1ec offset until 0x0003c5df offset, and is 119796 bytes long
    std::vector<uint8_t> unknownTable127468Data;

    std::vector<uint8_t> regionTable;  //this table has 680 bytes (contains region definitions for the level)

    //table starts at 0x0003c734 offset until 0x00062C8B offset, and is 157016 bytes long
    std::vector<uint8_t> unknownTable247604Data;

    bool InvestigatePrintUnknownTableOffset3312();
    bool InvestigatePrintUnknownTableOffset102068();
    bool InvestigatePrintUnknownTableOffset127468();
    bool PrintRegionTable();
    bool InvestigatePrintUnknownTableOffset247604();

    //holds location info about the charging locations
    //and the map start region/location
    std::vector<MapTileRegionStruct*> *mMapRegionVec = nullptr;

    //Makes sure a specified input Blockdefinition exists in the current levelfile.
    //If it does not exist yet it is newly created;
    //Returns true if the requested block definition is available, and the index
    //in the vector of all existing Blockdefinitions is returned in output parameter
    //outIndex;
    //If there is an unexpected error, and no blockdefinition index with this parameters
    //can be supplied, this function returns false
    //If block definition was newly added (not existing yet) output bool return value is
    //set to true, if an already existing block definition is used false is returned
    bool RequestBlockDefinition(uint8_t pN, uint8_t pE, uint8_t pS, uint8_t pW, uint8_t pT, uint8_t pB,
                                       uint8_t pNMod, uint8_t pEMod, uint8_t pSMod, uint8_t pWMod, uint8_t pTMod, uint8_t pBMod,
                                       int16_t pUnknown1, int16_t pUnknown2, irr::u32 &outIndex, bool &newlyAdded);

    //Makes sure a specified input Columndefinition exists in the current levelfile.
    //If it does not exist yet it is newly created;
    //Returns true if the requested column definition is available, and the index
    //in the vector of all existing Columndefinitions is returned in output parameter
    //outIndex;
    //If there is an unexpected error, and no Columndefinition index with this parameters
    //can be supplied, this function returns false
    //If colum definition was newly added (not existing yet) output bool return value is
    //set to true, if an already existing column definition is used false is returned
    bool RequestColumnDefinition(int16_t pFloorTextureID, int16_t pUnknown1, int16_t pA, int16_t pB, int16_t pC, int16_t pD, int16_t pE, int16_t pF,
                                             int16_t pG, int16_t pH, irr::u32 &outIndex, bool &newlyAdded);

    //returns nullptr if a Column definition with this Id is not found
    ColumnDefinition* GetColumnDefinitionWithCertainId(int whichId);


    //returns nullptr if a block definition with this Id is not found
    BlockDefinition* GetBlockDefinitionWithCertainId(int whichId);

    void DebugWriteCellInfoToCsvFile(char* debugOutPutFileName);

    //Returns true if there is a column at the specified location
    //False for no column
    //Important note: in case of an invalid entry/coordinate this function
    //will also return false
    bool IsAColumnAtCoordinates(int x, int y);

    //Returns true if new entity was successfully created, false otherwise
    //if succesfull, returns the new index of the new entity in output parameter outIndex
    bool AddEntityAtCell(int x, int y, irr::f32 heightTerrain, Entity::EntityType ofType, irr::u32 &outIndex);

    void RemoveRegion(MapTileRegionStruct* region);

    //returns true if new region was created succesfully, False otherwise
    bool AddRegion(irr::u8 whichRegionId, irr::core::vector2di coord1, irr::core::vector2di coord2, irr::u8 newRegionType);

    void ChangeRegionType(irr::u8 whichRegionId, irr::u8 newRegionType);

    //returns true if changing location was succesfull, false otherwise
    bool ChangeRegionLocation(irr::u8 whichRegionId, irr::core::vector2di coord1, irr::core::vector2di coord2);

private:
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
                                       uint8_t newNMod, uint8_t newEMod, uint8_t newSMod, uint8_t newWMod, uint8_t newTMod, uint8_t newBMod,
                                      int16_t newUnknown1, int16_t newUnknown2, irr::u32 &outIndex);

    bool CmpColumnDefinition(ColumnDefinition* def, int16_t cmpFloorTextureID, int16_t compUnknown1, int16_t compA, int16_t compB, int16_t compC, int16_t compD, int16_t compE, int16_t compF,
                                       int16_t compG, int16_t compH);

    //returns true in case a specified input column definition was found in the vector of existing
    //columndefinitions in the current level file; false otherwise
    //in case definition was found, returns also the index in the vector for this element
    //in parameter fndOutIndex
    bool SearchColumnDefinitionIndex(int16_t cmpFloorTextureID, int16_t cmpUnknown1, int16_t compA, int16_t compB, int16_t compC, int16_t compD, int16_t compE, int16_t compF,
                                                int16_t compG, int16_t compH, irr::u32 &fndOutIndex);

    //Returns true if new column definition was successfully created, false otherwise
    //if succesfull, returns the new index of the new columndefinition in output parameter outIndex
    bool AddColumnDefinition(int16_t newFloorTextureID, int16_t newUnknown1, int16_t newA, int16_t newB, int16_t newC, int16_t newD, int16_t newE, int16_t newF,
                                        int16_t newG, int16_t newH, irr::u32 &outIndex);

    void RemovePOI(irr::u8 regionId);

    //Returns true if succesfull created, false otherwise
    bool AddPOI(MapTileRegionStruct* newRegion);

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
     bool loadUnknownTableOffset0();
     bool loadUnknownTableOffset3168();
     bool loadUnknownTableOffset3312();
     bool loadUnknownTableOffset102068();
     bool loadUnknownTableOffset127468();
     bool loadUnknownTableOffset247604();

     //for levelEditor functionality
     bool saveBlockTexTable();
     bool saveEntitiesTable();
     bool saveColumnsTable();
     bool saveMap();
     bool saveRegionTable();
     bool saveUnknownTables();
};

#endif // LEVELFILE_H
