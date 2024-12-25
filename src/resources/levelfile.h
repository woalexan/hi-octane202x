/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++, and extended with some new code)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef LEVELFILE_H
#define LEVELFILE_H

#include "irrlicht/irrlicht.h"
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <list>
#include "mapentry.h"
#include "entityitem.h"
#include "../utils/crc32.h"
#include "blockdefinition.h"
#include <iostream>

#define LEVELFILE_WIDTH 256
#define LEVELFILE_HEIGHT 160

#define LEVELFILE_REGION_UNDEFINED 0
#define LEVELFILE_REGION_CHARGER_SHIELD 1
#define LEVELFILE_REGION_CHARGER_FUEL 2
#define LEVELFILE_REGION_CHARGER_AMMO 3
#define LEVELFILE_REGION_START 4

using namespace irr::core;
using namespace std;

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
};

struct ColumnsStruct {
      irr::core::vector3d<float> Vector3;
      ColumnDefinition *Columns;
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

    std::vector<ColumnsStruct> Columns;

    std::vector<uint8_t> unknownTable247264Data;
    std::vector<uint8_t> unknownTable358222Data;

    bool SaveUnknownTableOffset247264();
    bool SaveUnknownTableOffset358222();
    bool InvestigatePrintUnknownTableOffset247264();

    //holds location info about the charging locations
    //and the map start region/location
    std::vector<MapTileRegionStruct*> *mMapRegionVec;

    //Helper function, Returns true if a certain tile has the specified textureId
    //Returns false otherwise, or if the specified location on the tile map is invalid
    bool CheckTileForTextureId(int posX, int posY, int textureId);

protected:
     std::string m_Filename;
     std::string m_Name;
     bool m_Ready;

     std::vector<uint8_t> m_bytes;
     bool loadEntitiesTable();
     bool loadBlockTexTable();
     bool loadColumnsTable();
     bool loadMap();

     bool loadMapRegions();
     bool loadUnknownTableOffset358222();

     //for levelEditor functionality
     bool saveBlockTexTable();
};

#endif // LEVELFILE_H
