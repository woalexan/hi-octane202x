/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did just translation to C++, and extended with some new code)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "levelfile.h"
#include "../utils/logging.h"
#include <iostream>
#include <fstream>
#include "blockdefinition.h"
#include "columndefinition.h"
#include "mapentry.h"
#include "entityitem.h"
#include <iterator>
#include "../definitions.h"

/* LevelFile Layout

1. Offset 0          => Offset 96000   Entity-Table (stores 4000 Entities)
2. Offset 98012      => Offset 124636  Column-Definitions (stores 1024 Column Definitions)
3. Offset 124636     => Offset 141020  Block-Definitions  (stores 1024 Block Definitions)
4. Offset 246924     => Offset 247604  Region-Definitions (stores 8 Region Definitions)
5. Offset 404620     => Offset 896140  Tile-Data (256 x 160 Tiles)

*/

LevelFile::LevelFile(std::string filename) {
   this->m_Filename = filename;
   this->m_Ready = false;
   bool ready_result;

   mMapRegionVec = new std::vector<MapTileRegionStruct*>();

   /* try to open file to read */
   ifstream ifile;
   std::streampos fileSize;

   ifile.open(filename, std::ifstream::binary);
      if(ifile) {
         logging::Info("Level file found and openend succesfully");
      } else {
          logging::Error("Could not open Level file");
          return;
      }

    // get its size:
    ifile.seekg(0, std::ios::end);
    fileSize = ifile.tellg();
    ifile.seekg(0, std::ios::beg);

    //read the data
    this->m_bytes.resize(fileSize);
    ifile.read(reinterpret_cast<char*>(this->m_bytes.data()), this->m_bytes.size());

    ifile.close();

    char hlpstr[500];
    std::string msg("");

    if (ifile) {
        msg.clear();
        msg.append("Leveldata read succesfully (");
        size_t fileSizet = fileSize;
        snprintf(hlpstr, 500, "%zu", fileSizet);
        msg.append(hlpstr);
        msg.append(" bytes)");
        logging::Info(msg);
    }
    else {
        size_t fileReadsizet = ifile.gcount();
        msg.clear();
        msg.append("Leveldata file read error: only ");
        snprintf(hlpstr, 500, "%zu", fileReadsizet);
        msg.append(hlpstr);
        msg.append(" bytes could be read!");
        logging::Error(msg);
        return;
    }

    ready_result = loadBlockTexTable() && loadColumnsTable() && loadMap() && loadEntitiesTable() &&
            loadMapRegions() && loadUnknownTableOffset0() && loadUnknownTableOffset3168() &&
            loadUnknownTableOffset3312() && loadUnknownTableOffset102068() && loadUnknownTableOffset127468() && loadUnknownTableOffset247604();
    this->m_Ready = ready_result;

    //for debugging of level save functionality
    //disable later
    this->m_wBytes.resize(this->m_bytes.size());
    std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_bytes.size(), 0x55);

    if (false) {
        InvestigatePrintUnknownTableOffset3312();
        InvestigatePrintUnknownTableOffset102068();
        InvestigatePrintUnknownTableOffset127468();
        PrintRegionTable();
        InvestigatePrintUnknownTableOffset247604();
    }

    //DebugWriteCellInfoToCsvFile((char*)("dbgcellInfo.csv"));

    /*std::vector<unsigned char>::iterator it;
    unsigned int pos = 0;
    for (it = unknownTable247264Data.begin(); it != unknownTable247264Data.end(); ++it) {
        (*it) = 0;

        pos++;
    }*/

    //unknownTable247264Data.at(48 + 2 * 85) = 4;

    //SaveUnknownTableOffset247264();
/*
    pos = 0;

    for (it = unknownTable358222Data.begin(); it != unknownTable358222Data.end(); ++it) {
       // if (pos > 84) {
            (*it) = 0;
      //  }
        pos++;
    }

    SaveUnknownTableOffset358222();*/
}

LevelFile::~LevelFile() {
    //cleanup heap

    //delete existing entities
    std::vector<EntityItem*>::iterator itEntities;
    EntityItem* pntr;

    //delete all existing entities
    itEntities = this->Entities.begin();

    while (itEntities != this->Entities.end()) {
           pntr = (*itEntities);
           itEntities = this->Entities.erase(itEntities);
           delete pntr;
    }

    std::vector<BlockDefinition*>::iterator itBlockDefinitions;
    BlockDefinition* pntrBlockdef;

    //delete all existing blockdefinitions
    itBlockDefinitions = this->BlockDefinitions.begin();

    while (itBlockDefinitions != this->BlockDefinitions.end()) {
           pntrBlockdef = (*itBlockDefinitions);
           itBlockDefinitions = this->BlockDefinitions.erase(itBlockDefinitions);
           delete pntrBlockdef;
    }

    std::vector<ColumnDefinition*>::iterator itColumnDefinitions;
    ColumnDefinition* pntrColumDef;

    //delete all existing column definitions
    itColumnDefinitions = this->ColumnDefinitions.begin();

    while (itColumnDefinitions != this->ColumnDefinitions.end()) {
           pntrColumDef = (*itColumnDefinitions);
           itColumnDefinitions = this->ColumnDefinitions.erase(itColumnDefinitions);
           delete pntrColumDef;
    }

    //delete all available region info structs
    std::vector<MapTileRegionStruct*>::iterator itRegions;
    MapTileRegionStruct* pntrRegion;

    //delete all existing map regions
    itRegions = this->mMapRegionVec->begin();

    while (itRegions != this->mMapRegionVec->end()) {
           pntrRegion = (*itRegions);
           itRegions = this->mMapRegionVec->erase(itRegions);

           //delete the struct itself
           delete pntrRegion;
    }

    //delete all MapEntry objects
    for (int y = 0; y < Height(); y++) {
     for (int x = 0; x < Width(); x++) {

         MapEntry *entry = this->pMap[x][y];
         delete entry;
         this->pMap[x][y] = nullptr;
     }
    }
}

int LevelFile::Width() {
   return(LEVELFILE_WIDTH);
}

int LevelFile::Height() {
   return(LEVELFILE_HEIGHT);
}

std::string LevelFile::get_Filename() {
    return(this->m_Filename);
}

std::string LevelFile::get_Name() {
    return(this->m_Name);
}

void LevelFile::set_Filename(std::string new_Filename) {
    this->m_Filename = new_Filename;
}

void LevelFile::set_Name(std::string new_Name) {
    this->m_Name = new_Name;
}

bool LevelFile::get_Ready() {
    return(this->m_Ready);
}

void LevelFile::set_Ready(bool newstate) {
    this->m_Ready = newstate;
}

//if no entity whith this Id is found, returns false
bool LevelFile::ReturnEntityItemWithId(int searchId, EntityItem **fndItem) {
    bool notFound = true;
    std::vector<EntityItem*>::iterator looper;
    unsigned int itemIdx = 0;

    for(looper = this->Entities.begin(); notFound && (looper != this->Entities.end()); ++looper) {
        if ((*looper)->get_ID() == searchId) {
            notFound = false;
            *fndItem = this->Entities.at(itemIdx);
        }

        itemIdx++;
    }

    return !notFound;
}

bool LevelFile::loadEntitiesTable() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;
    std::vector<uint8_t> dataslice;

    this->Entities.clear();

    for (int i = 0; i < 4000; i++) {
        int baseOffset = i * 24;
        if (this->m_bytes.at(baseOffset) == 0) continue;

        startslice = this->m_bytes.begin() + baseOffset;
        endslice = this->m_bytes.begin()+ baseOffset + 24;
        dataslice.assign(startslice, endslice);

        EntityItem *item = new EntityItem(i, baseOffset, dataslice);
        item->setY(this->pMap[item->getCell().X][item->getCell().Y]->m_Height);
        this->Entities.push_back(item);

        //next lines are only for level writting debugging
        //comment out normally!
        /*item->WriteChanges();
        if (!std::equal (startslice,endslice, item->m_wBytes.begin())) {
            //there is an unexpected difference in the written data
            std::cout << "Unexpected difference in written entity data!" << std::endl << std::flush;
        }*/
      }

    return(true);
}

bool LevelFile::loadBlockTexTable() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;
    std::vector<uint8_t> dataslice;

    this->BlockDefinitions.clear();

    //BlockDefinitions = new OrderedDictionary<int, BlockDefinition>();

    for (int i = 0; i < 1024; i++) {
        int baseOffset = 124636 + i * 16;
        if (this->m_bytes.at(baseOffset) == 0) continue;

        startslice = this->m_bytes.begin() + baseOffset;
        endslice = this->m_bytes.begin()+ baseOffset + 16;
        dataslice.assign(startslice, endslice);

        BlockDefinition *item = new BlockDefinition(i, baseOffset, dataslice);
        this->BlockDefinitions.push_back(item);

        //next lines are only for level writting debugging
        //comment out normally!
        /*item->WriteChanges();
        if (!std::equal (startslice,endslice, item->m_wBytes.begin())) {
            //there is an unexpected difference in the written data
            std::cout << "Unexpected difference in written Block definition data!" << std::endl << std::flush;
        }*/
    }

  return(true);
}

bool LevelFile::loadColumnsTable() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;
    std::vector<uint8_t> dataslice;

    this->ColumnDefinitions.clear();

    //ColumnDefinitions = new OrderedDictionary<int, ColumnDefinition>();

    for (int i = 0; i < 1024; i++) {
        int baseOffset = 98012 + i * 26;
        if (this->m_bytes.at(baseOffset) == 0) continue;

        startslice = this->m_bytes.begin() + baseOffset;
        endslice = this->m_bytes.begin()+ baseOffset + 26;
        dataslice.assign(startslice, endslice);

        ColumnDefinition *item = new ColumnDefinition(i, baseOffset, dataslice);
        this->ColumnDefinitions.push_back(item);

        //next lines are only for level writting debugging
        //comment out normally!
        /*item->WriteChanges();
        if (!std::equal (startslice,endslice, item->m_wBytes.begin())) {
            //there is an unexpected difference in the written data
            std::cout << "Unexpected difference in written column definition data!" << std::endl << std::flush;
        }*/
    }

 return(true);
}

bool LevelFile::loadMap() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;
    std::vector<uint8_t> dataslice;

    ColumnsStruct NewStruct;
    MapPointOfInterest poi;

    //PointsOfInterest = new List<MapPointOfInterest>();
    PointsOfInterest.clear();
    Columns.clear();

    //  Columns = new OrderedDictionary<Vector3, ColumnDefinition>();
    //Map = new MapEntry[Width, Height];

    // entry is 12 bytes long, map is at end of file
    int numBytes = 12 * Width() * Height();

    int i = (int)(this->m_bytes.size()) - numBytes;

    for (int y = 0; y < Height(); y++) {
     for (int x = 0; x < Width(); x++) {
         startslice = this->m_bytes.begin() + i;
         endslice = this->m_bytes.begin()+ i + 12;
         dataslice.assign(startslice, endslice);

         MapEntry *entry = new MapEntry(x, y, i, dataslice, ColumnDefinitions);

         if (entry->get_Column() != nullptr) {
              NewStruct.Vector3 = irr::core::vector3d<float>((irr::f32)(x), 0.0f, (irr::f32)(y));
              NewStruct.Columns = entry->get_Column();

              this->Columns.push_back(NewStruct);
         }

         this->pMap[x][y] = entry;

         /*entry->WriteChanges();
         if (!std::equal (startslice,endslice, entry->m_wBytes.begin())) {
             //there is an unexpected difference in the written data
             std::cout << "Unexpected difference in written MapEntry data!" << std::endl << std::flush;
         }*/

         // check for points of interest, this POI then point into the region definition table
         //there we find the size of each region in number of cells and of what type this region is
        int16_t poiValue = (this->m_bytes.at(i + 7) << 8) | this->m_bytes.at(i + 6);
        if (poiValue > 0) {
            poi.Value = poiValue;
            poi.cellCoord.X = x;
            poi.cellCoord.Y = y;
            poi.Position = irr::core::vector3d<float>(-x * DEF_SEGMENTSIZE - 0.5f * DEF_SEGMENTSIZE,
                                entry->m_Height, y * DEF_SEGMENTSIZE + 0.5f * DEF_SEGMENTSIZE);
            PointsOfInterest.push_back(poi);
         }

        i += 12;
    }
   }

 return(true);
}

bool LevelFile::Save(std::string filename) {
   //prepare all data in second data byte array
   //better write the unknown table data first, not that we
   //overwrite other data afterwards again at wrong spots
   saveUnknownTables();

   saveRegionTable();

   //copy initial read level data to write level data
   //array, to make sure that all data makes sense we do not
   //yet know what it means
  // std::copy(this->m_bytes.begin(), this->m_bytes.end(), this->m_wBytes.begin());

   saveMap();
   saveColumnsTable();
   saveBlockTexTable();
   saveEntitiesTable();

   //write prepared data to output file
   std::ofstream outputfile(filename, std::ios::out|std::ios::binary);
   std::copy(this->m_wBytes.cbegin(), this->m_wBytes.cend(),
          std::ostream_iterator<uint8_t>(outputfile));

   outputfile.close();

   return(true);
}

//The function below reads the region definition table inside the level file
//and uses this information to define the regions in the map
//it seems maximum number of possible regions is 8, which leaves space for up to 7 charging
//areas and one race starting location
bool LevelFile::loadMapRegions() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 246924;

    startslice = this->m_bytes.begin() + baseOffset;
    //map region definition table has 8 entries with each 85 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 8 * 85;
    regionTable.assign(startslice, endslice);

    //extract region definition information
    for (int i = 0; i < 8; i++) {
        //is there an entry? if so create a MapTileRegionStruct with info
        //we have an entry if byte 0 is unequal to 0
        if (regionTable.at(i * 85) != 0x0) {
            //create new entry
            MapTileRegionStruct* newRegion = new MapTileRegionStruct();
            newRegion->regionId = i;

            //Byte 0 defines the region type
            newRegion->regionType = regionTable.at(i * 85);
            //value 1 means Shield charger location
            //value 2 means Fuel charger location
            //value 3 means Ammo charger location
            //value 4 means map start location

            //Byte 2 seems to only contain value 1, if entry is used, not interesting to read, but we need to write it like that
            //Byte 3 & 4 always contain a fixed predefined value according to number of entry from 224 up to 231 (do not read, but write)
            //Byte 5 seems to only contain value 3 if entry is used, not interesting to read, but we need to write it like that
            //Byte 12 always contain a fixed predefined value according to number of entry from 200 up to 207 (do not read, but write)
            //Byte 13 seems to only contain value 7 if entry is used, not interesting to read, but we need to write it like that
            //Byte 16 seem to always have the same value as Byte 4
            //Byte 17 seem to always have the same value as Byte 5
            //Byte 18 seems to only contain value 1 if entry is used, not interesting to read, but we need to write it like that
            //Bytes 39 & 40 have different values for the non extended game over the different levels (I saw in levels 1 and 2, I did not check
            //for the other levels), and the extended version of the game has (compared with non extended game another)
            //the same value for all levels, except level 7 which is an exception).
            //I tried for level 1 (for race start entry) and used two different value pairs, and compared the starting sequence of the original game
            //frame by frame. I did not see any obvious difference. Therefore I do not know yet what this values do. I will just write
            //the value used for almost all levels in extended version of the game.
            //Byte 69 contains value 245 + Value of byte offset 0 (Region type), not sure why we want to have this information also 2 times in the file
            //(anti cheat mechanism?)

            //middle point of rectangle that defines region is stored
            //at byte location 25 and location 27 of this current table entry line
            newRegion->regionCenterTileCoord.X = regionTable.at(i * 85 + 25);
            newRegion->regionCenterTileCoord.Y = regionTable.at(i * 85 + 27);

            //region size is specified in terms of tiles counted from
            //the middle point towards both axis directions
            //for "X-axis" the deltaX is stored at byte 46
            irr::u8 deltaX = regionTable.at(i * 85 + 46);

            //for "Y-axis" the deltaY is stored at byte 48
            irr::u8 deltaY = regionTable.at(i * 85 + 48);

            //now calculate region tile min/max for the later game code according
            //to all input information
            newRegion->tileXmin = newRegion->regionCenterTileCoord.X - deltaX;
            newRegion->tileXmax = newRegion->regionCenterTileCoord.X + deltaX;

            if (newRegion->tileXmin < 0)
                newRegion->tileXmin = 0;

            if (newRegion->tileXmin >= this->Width()) {
                newRegion->tileXmin = this->Width() - 1;
            }

            if (newRegion->tileXmax < 0)
                newRegion->tileXmax = 0;

            if (newRegion->tileXmax >= this->Width()) {
                newRegion->tileXmax = this->Width() - 1;
            }

            newRegion->tileYmin = newRegion->regionCenterTileCoord.Y - deltaY;
            newRegion->tileYmax = newRegion->regionCenterTileCoord.Y + deltaY;

            if (newRegion->tileYmin < 0)
                newRegion->tileYmin = 0;

            if (newRegion->tileYmin >= this->Height()) {
                newRegion->tileYmin = this->Height() - 1;
            }

            if (newRegion->tileYmax < 0)
                newRegion->tileYmax = 0;

            if (newRegion->tileYmax >= this->Height()) {
                newRegion->tileYmax = this->Height() - 1;
            }

            //add new region to my region vector
            this->mMapRegionVec->push_back(newRegion);
        }
    }

    return true;
}

bool LevelFile::PrintRegionTable() {
    std::vector<uint8_t>::iterator it;

    int cnt = 0;

    char finalpath[100];

    strcpy(finalpath, this->get_Filename().c_str());
    strcat(finalpath, "-regionTable.txt");

    //now we have our output filename
    FILE* oFile = fopen(finalpath, "w");
    if (oFile == nullptr) {
       return false;
    }

    //Region table has 8 entries with each 85 bytes
    for (int tableNr = 0; tableNr < 8; tableNr++) {
        it = this->regionTable.begin() + tableNr * 85;
        for (; it != this->regionTable.end();++it) {
            if (cnt == (tableNr + 1) * 85)
                break;

            fprintf(oFile, "; %u ", (*it));

            cnt++;
        }

        //go to next line
        fprintf(oFile, "\n");
    }

    //close file
    fclose(oFile);

    return true;
}

bool LevelFile::InvestigatePrintUnknownTableOffset127468() {
    std::vector<uint8_t>::iterator it;

    char finalpath[100];

    strcpy(finalpath, this->get_Filename().c_str());
    strcat(finalpath, "-table127468.txt");

    //now we have our output filename
    //FILE* oFile = fopen(finalpath, "w");
    FILE* oFile = fopen(finalpath, "w+b");
    if (oFile == nullptr) {
       return false;
    }

    it = this->unknownTable127468Data.begin();
    for (; it != this->unknownTable127468Data.end();++it) {
       //fprintf(oFile, "; %u ", (*it));
       fprintf(oFile, "%c", (*it));
    }

    //close file
    fclose(oFile);

    return true;
}

bool LevelFile::InvestigatePrintUnknownTableOffset247604() {
    std::vector<uint8_t>::iterator it;

    char finalpath[100];

    strcpy(finalpath, this->get_Filename().c_str());
    strcat(finalpath, "-table247604.txt");

    //now we have our output filename
    //FILE* oFile = fopen(finalpath, "w");
    FILE* oFile = fopen(finalpath, "w+b");
    if (oFile == nullptr) {
       return false;
    }

    it = this->unknownTable247604Data.begin();
    for (; it != this->unknownTable247604Data.end();++it) {
       //fprintf(oFile, "; %u ", (*it));
       fprintf(oFile, "%c", (*it));
    }

    //close file
    fclose(oFile);

    return true;
}

bool LevelFile::InvestigatePrintUnknownTableOffset102068() {
    std::vector<uint8_t>::iterator it;

    char finalpath[100];

    strcpy(finalpath, this->get_Filename().c_str());
    strcat(finalpath, "-table102068.txt");

    //now we have our output filename
    //FILE* oFile = fopen(finalpath, "w");
    FILE* oFile = fopen(finalpath, "w+b");
    if (oFile == nullptr) {
       return false;
    }

    it = this->unknownTable102068Data.begin();
    for (; it != this->unknownTable102068Data.end();++it) {
       //fprintf(oFile, "; %u ", (*it));
       fprintf(oFile, "%c", (*it));
    }

    //close file
    fclose(oFile);

    return true;
}

bool LevelFile::InvestigatePrintUnknownTableOffset3312() {
    std::vector<uint8_t>::iterator it;

    char finalpath[100];

    strcpy(finalpath, this->get_Filename().c_str());
    strcat(finalpath, "-table3312.txt");

    //now we have our output filename
    //FILE* oFile = fopen(finalpath, "w");
    FILE* oFile = fopen(finalpath, "w+b");
    if (oFile == nullptr) {
       return false;
    }

    it = this->unknownTable3312Data.begin();
    for (; it != this->unknownTable3312Data.end();++it) {
       //fprintf(oFile, "; %u ", (*it));
       fprintf(oFile, "%c", (*it));
    }

    //close file
    fclose(oFile);

    return true;
}

//Returns true if there is a column at the specified location
//False for no column
//Important note: in case of an invalid entry/coordinate this function
//will also return false
bool LevelFile::IsAColumnAtCoordinates(int x, int y) {
    MapEntry* entry = pMap[x][y];

    if (entry == nullptr)
        return false;

    //no column?
    if (entry->get_Column() == nullptr)
        return false;

    //there is a column
    return true;
}

/*bool LevelFile::SaveUnknownTableOffset358222() {

    int baseOffset = 358222;

    char* ByteArray;
    ByteArray = new char[this->unknownTable358222Data.size()];

    FILE* oFile = nullptr;

    oFile = fopen(this->m_Filename.c_str(), "r+b");
    if (oFile == nullptr) {
       return false;
    }

    std::vector<uint8_t>::iterator it;
    unsigned long pos = 0;

    for (it = this->unknownTable358222Data.begin(); it != this->unknownTable358222Data.end(); ++it) {
        ByteArray[pos] = (*it);
        pos++;
    }

    fseek(oFile, baseOffset, SEEK_SET);
    fwrite(&ByteArray[0], 1, this->unknownTable358222Data.size(), oFile);

    fclose(oFile);

    return true;
}*/

bool LevelFile::loadUnknownTableOffset247604() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 247604;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 157016 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 157016;
    unknownTable247604Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset0() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 0;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 24 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 24;
    unknownTable0Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset3168() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 3168;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 72 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 72;
    unknownTable3168Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset3312() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 3312;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 94736 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 94736;
    unknownTable3312Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset102068() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 102068;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 22584 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 22584;
    unknownTable102068Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset127468() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 127468;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 119796 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 119796;
    unknownTable127468Data.assign(startslice, endslice);

    return true;
}

//if region with specified Id is not existing or not defined returns
//nullptr
MapTileRegionStruct* LevelFile::GetRegionStructForRegionId(irr::u8 regionId) {
    std::vector<MapTileRegionStruct*>::iterator it;

    for (it = mMapRegionVec->begin(); it != mMapRegionVec->end(); ++it) {
        if ((*it)->regionId == regionId) {
            //region Id was found
            if ((*it)->regionType != LEVELFILE_REGION_UNDEFINED) {
              return (*it);
            } else {
              return nullptr;
            }
        }
    }

    //region Id not found, return nullptr
    return nullptr;
}

void LevelFile::RemoveRegion(MapTileRegionStruct* region) {
    if (region == nullptr)
        return;

    //overwrite bytes for this region entry with 0 bytes
    std::vector<uint8_t>::iterator startadr;
    std::vector<uint8_t>::iterator endadr;

    //each entry has 85 bytes
    startadr = this->regionTable.begin() + (region->regionId * 85);
    endadr = this->regionTable.begin() + (region->regionId * 85) + 85;

    //set to all 0 bytes
    std::fill(startadr, endadr, 0);

    //now remove this region from the region vector
    std::vector<MapTileRegionStruct*>::iterator it;
    MapTileRegionStruct* pntr = nullptr;

    for (it = mMapRegionVec->begin(); it != mMapRegionVec->end(); ) {
        if ((*it) == region) {
            pntr = (*it);

            //remove the POI data in level file
            //as well
            RemovePOI(pntr->regionId);

            it = mMapRegionVec->erase(it);

            //delete the struct itself
            delete pntr;
        } else {
            it++;
        }
    }
}

void LevelFile::ChangeRegionType(irr::u8 whichRegionId, irr::u8 newRegionType) {
    //if whichRegionId is invalid return
     if (whichRegionId > 7)
         return;

     //if at the specified regionId there is no existing region
     //return
     MapTileRegionStruct* pntr = GetRegionStructForRegionId(whichRegionId);

     if (pntr == nullptr)
         return;

     //if new region type does not make sense for this operation exit here
     if ((newRegionType == LEVELFILE_REGION_UNDEFINED) || (newRegionType == LEVELFILE_REGION_TRIGGERCRAFT) ||
             (newRegionType == LEVELFILE_REGION_TRIGGERMISSILE))
         return;

     //change the region type inside of our region info struct
     pntr->regionType = newRegionType;

     //also modify the low level level file data itself

     //Byte 0 defines the region type
     regionTable.at(whichRegionId * 85) = (uint8_t)(newRegionType);
     //value 1 means Shield charger location
     //value 2 means Fuel charger location
     //value 3 means Ammo charger location
     //value 4 means map start location

     //byte offset 69 also includes the region type information a second time
     //Byte 69 contains value 245 + Value of byte offset 0 (Region type), not sure why we want to have this information also 2 times in the file
     //(anti cheat mechanism?)
     regionTable.at(whichRegionId * 85 + 69) = (uint8_t)(245)+(uint8_t)(newRegionType);
}

//returns true if changing location was succesfull, false otherwise
bool LevelFile::ChangeRegionLocation(irr::u8 whichRegionId, irr::core::vector2di coord1, irr::core::vector2di coord2) {
    //if whichRegionId is invalid return
     if (whichRegionId > 7)
         return false;

     //if at the specified regionId there is no existing region
     //return
     MapTileRegionStruct* pntr = GetRegionStructForRegionId(whichRegionId);

     if (pntr == nullptr)
         return false;

     irr::u8 storeType = pntr->regionType;

     //remove existing region
     RemoveRegion(pntr);

     //Add new region back, but with the new location
     return (AddRegion(whichRegionId, coord1, coord2, storeType));
}

//returns true if new region was created succesfully, False otherwise
bool LevelFile::AddRegion(irr::u8 whichRegionId, irr::core::vector2di coord1, irr::core::vector2di coord2, irr::u8 newRegionType) {
   //if whichRegionId is invalid return
    if (whichRegionId > 7)
        return false;

   //if at the specified regionId there is already an existing region
   //fail creation and return
   MapTileRegionStruct* pntr = GetRegionStructForRegionId(whichRegionId);

   if (pntr != nullptr)
       return false;

   //if new region type is not specified or invalid return
   if ((newRegionType == LEVELFILE_REGION_UNDEFINED) || (newRegionType == LEVELFILE_REGION_TRIGGERCRAFT) ||
           (newRegionType == LEVELFILE_REGION_TRIGGERMISSILE))
       return false;

   //region with this Id does not yet exist
   //calculate middle cell
   irr::f32 midX = (((irr::f32)(coord2.X) - (irr::f32)(coord1.X)) / 2.0f) + (irr::f32)(coord1.X);
   irr::f32 midY = (((irr::f32)(coord2.Y) - (irr::f32)(coord1.Y)) / 2.0f) + (irr::f32)(coord1.Y);

   //round towards next integer
   pntr = new MapTileRegionStruct();
   pntr->regionId = whichRegionId;
   pntr->tileXmax = coord2.X;
   pntr->tileYmax = coord2.Y;
   pntr->tileXmin = coord1.X;
   pntr->tileYmin = coord1.Y;
   pntr->regionCenterTileCoord.set((irr::s32)(irr::core::round32(midX)), (irr::s32)(irr::core::round32(midY)));
   pntr->regionType = newRegionType;

   //add the new region to the vector of defined regions
   mMapRegionVec->push_back(pntr);

   //Byte 0 defines the region type
   regionTable.at(whichRegionId * 85) = (uint8_t)(newRegionType);
   //value 1 means Shield charger location
   //value 2 means Fuel charger location
   //value 3 means Ammo charger location
   //value 4 means map start location

   //Byte 2 seems to only contain value 1, if entry is used, not interesting to read, but we need to write it like that
   regionTable.at(whichRegionId * 85 + 2) = (uint8_t)(1);

   //Byte 3 & 4 always contain a fixed predefined value according to number of entry from 224 up to 231 (do not read, but write)
   uint8_t valDependentRegionId = (uint8_t)(224) + (uint8_t)(whichRegionId);
   regionTable.at(whichRegionId * 85 + 3) = valDependentRegionId;
   regionTable.at(whichRegionId * 85 + 4) = valDependentRegionId;

   //Byte 5 seems to only contain value 3 if entry is used, not interesting to read, but we need to write it like that
   regionTable.at(whichRegionId * 85 + 5) = (uint8_t)(3);

   //Byte 12 always contain a fixed predefined value according to number of entry from 200 up to 207 (do not read, but write)
   uint8_t valDependentRegionId2 = (uint8_t)(200) + (uint8_t)(whichRegionId);
   regionTable.at(whichRegionId * 85 + 12) = valDependentRegionId2;

   //Byte 13 seems to only contain value 7 if entry is used, not interesting to read, but we need to write it like that
   regionTable.at(whichRegionId * 85 + 13) = (uint8_t)(7);

   //Byte 16 seem to always have the same value as Byte 4
   uint8_t val = regionTable.at(whichRegionId * 85 + 4);
   regionTable.at(whichRegionId * 85 + 16) = val;

   //Byte 17 seem to always have the same value as Byte 5
   val = regionTable.at(whichRegionId * 85 + 5);
   regionTable.at(whichRegionId * 85 + 17) = val;

   //Byte 18 seems to only contain value 1 if entry is used, not interesting to read, but we need to write it like that
   regionTable.at(whichRegionId * 85 + 18) = (uint8_t)(1);

   //middle point of rectangle that defines region is stored
   //at byte location 25 and location 27 of this current table entry line
   regionTable.at(whichRegionId * 85 + 25) = (irr::u8)(pntr->regionCenterTileCoord.X);
   regionTable.at(whichRegionId * 85 + 27) = (irr::u8)(pntr->regionCenterTileCoord.Y);

   irr::u8 deltaX = pntr->tileXmax - pntr->regionCenterTileCoord.X;
   irr::u8 deltaY = pntr->tileYmax - pntr->regionCenterTileCoord.Y;

   //Bytes 39 & 40 have different values for the non extended game over the different levels (I saw in levels 1 and 2, I did not check
   //for the other levels), and the extended version of the game has (compared with non extended game another)
   //the same value for all levels, except level 7 which is an exception).
   //I tried for level 1 (for race start entry) and used two different value pairs, and compared the starting sequence of the original game
   //frame by frame. I did not see any obvious difference. Therefore I do not know yet what this values do. I will just write
   //the value used for almost all levels in extended version of the game.
   regionTable.at(whichRegionId * 85 + 39) = (uint8_t)(232);
   regionTable.at(whichRegionId * 85 + 40) = (uint8_t)(54);

   //region size is specified in terms of tiles counted from
   //the middle point towards both axis directions
   //for "X-axis" the deltaX is stored at byte 46
   regionTable.at(whichRegionId * 85 + 46) = deltaX;

   //for "Y-axis" the deltaY is stored at byte 48
   regionTable.at(whichRegionId * 85 + 48) = deltaY;

   //Byte 69 contains value 245 + Value of byte offset 0 (Region type), not sure why we want to have this information also 2 times in the file
   //(anti cheat mechanism?)
   regionTable.at(whichRegionId * 85 + 69) = (uint8_t)(245)+(uint8_t)(newRegionType);

   //we also need to add the POI (point of interest) pointing onto this
   //new region
   return (AddPOI(pntr));
}

//Returns true if succesfull created, false otherwise
bool LevelFile::AddPOI(MapTileRegionStruct* newRegion) {
    if (newRegion == nullptr)
        return false;

    int16_t POIValToAdd = 992 + newRegion->regionId;

    MapEntry* entry = pMap[newRegion->regionCenterTileCoord.X][newRegion->regionCenterTileCoord.Y];

    if (entry == nullptr)
        return false;

    //set new POI value at the cell in the middle
    //of the new region
    entry->mPointOfInterest = POIValToAdd;

    MapPointOfInterest newPointOfInterest;
    newPointOfInterest.Value = POIValToAdd;
    newPointOfInterest.Position.X = - newRegion->regionCenterTileCoord.X * DEF_SEGMENTSIZE - 0.5f;
    newPointOfInterest.Position.Z =  newRegion->regionCenterTileCoord.Y * DEF_SEGMENTSIZE + 0.5f;
    newPointOfInterest.Position.Y = entry->m_Height;
    newPointOfInterest.cellCoord.set(newRegion->regionCenterTileCoord.X, newRegion->regionCenterTileCoord.Y);

    //add new POI to list
    PointsOfInterest.push_back(newPointOfInterest);

    return true;
}

void LevelFile::RemovePOI(irr::u8 regionId) {
    //visit every cell and make sure that the specified POI
    //value (depends on regionId) is not linked anywhere anymore
    MapEntry* entry;

    //Linking of POI to level file offset for
    //region with specific regionId
    /*
    POI level file offset RegionId
    992	246924              0
    993	247009              1
    994	247094              2
    995	247179              3
    996	247264              4
    997	247349              5
    998	247434              6
    999	247519              7
    */

    int16_t POIToFind = 992 + regionId;

    for (int y = 0; y < Height(); y++) {
         for (int x = 0; x < Width(); x++) {

             entry = this->pMap[x][y];

             if (entry != nullptr) {
                 if (entry->mPointOfInterest == POIToFind) {
                     //we need to erase this link to this POI from this cell
                     entry->mPointOfInterest = 0;
                 }
             }
         }
    }

    //Delete this POI from the PointsOfInterest vector as well
    std::list<MapPointOfInterest>::iterator listIt;

    for (listIt = PointsOfInterest.begin(); listIt != PointsOfInterest.end(); ) {
        if ((*listIt).Value == POIToFind) {
            listIt = PointsOfInterest.erase(listIt);
        } else {
            listIt++;
        }
    }
}

/*************************************************************
 *                                                           *
 * Block Definition helper routines used for level editor    *
 *                                                           *
 *************************************************************/

//returns true if a block definition object
//contains exactly the specified input texture Ids and
//texture modification values
//False otherwise
bool LevelFile::CmpBlockDefinition(BlockDefinition* def, uint8_t compN, uint8_t compE, uint8_t compS, uint8_t compW, uint8_t compT, uint8_t compB,
                                   uint8_t compNMod, uint8_t compEMod, uint8_t compSMod, uint8_t compWMod, uint8_t compTMod, uint8_t compBMod) {
    if (def->get_N() != compN)
        return false;

    if (def->get_E() != compE)
        return false;

    if (def->get_S() != compS)
        return false;

    if (def->get_W() != compW)
        return false;

    if (def->get_B() != compB)
        return false;

    if (def->get_T() != compT)
        return false;

    if (def->get_NMod() != compNMod)
        return false;

    if (def->get_EMod() != compEMod)
        return false;

    if (def->get_SMod() != compSMod)
        return false;

    if (def->get_WMod() != compWMod)
        return false;

    if (def->get_BMod() != compBMod)
        return false;

    if (def->get_TMod() != compTMod)
        return false;

    //are identical
    return true;
}

//returns true in case a specified input block definition was found in the vector of existing
//blockdefinitions in the current level file; false otherwise
//in case definition was found, returns also the index in the vector for this element
//in parameter fndOutIndex
bool LevelFile::SearchBlockDefinitionIndex(uint8_t compN, uint8_t compE, uint8_t compS, uint8_t compW, uint8_t compT, uint8_t compB,
                                           uint8_t compNMod, uint8_t compEMod, uint8_t compSMod, uint8_t compWMod, uint8_t compTMod, uint8_t compBMod, irr::u32 &fndOutIndex) {
    std::vector<BlockDefinition*>::iterator it;
    irr::u32 currIdx = 0;

    for (it = BlockDefinitions.begin(); it != BlockDefinitions.end(); ++it) {
        if (CmpBlockDefinition((*it), compN, compE, compS, compW, compT, compB,
                               compNMod, compEMod, compSMod, compWMod, compTMod, compBMod)) {
            //found this definition
            fndOutIndex = currIdx;

            return true;
        }

        currIdx++;
    }

    //not found
    return false;
}

//Returns true if new block definition was successfully created, false otherwise
//if succesfull, returns the new index of the new blockdefinition in output parameter outIndex
bool LevelFile::AddBlockDefinition(uint8_t newN, uint8_t newE, uint8_t newS, uint8_t newW, uint8_t newT, uint8_t newB,
                                   uint8_t newNMod, uint8_t newEMod, uint8_t newSMod, uint8_t newWMod, uint8_t newTMod, uint8_t newBMod,
                                   int16_t newUnknown1, int16_t newUnknown2, irr::u32 &outIndex) {

    int i;
    int baseOffset;

    //first find next free block definition location
    //in current level file, if nothing free return with no success

    //it seems we should skip i = 0 here, in the original level the first
    //index i = 0 is still filled with zero bytes; better do the same
    for (i = 1; i < 1024; i++) {
        baseOffset = 124636 + i * 16;
        if (this->m_bytes.at(baseOffset) == 0) {
            //we found a free space
            break;
        }
    }

    //no free space anymore?
    if (i == 1024) {
        return false;
    }

    //create the new blockdefinition, use the alternative constructor
    //for the leveleditor for this
    BlockDefinition *newDef = new BlockDefinition(i, baseOffset, newN, newE, newS, newW, newT, newB, newNMod,
                                                  newEMod, newSMod, newWMod, newTMod, newBMod, newUnknown1, newUnknown2);

    std::string infoMsg("");
    char hlpstr[100];

    infoMsg.clear();
    infoMsg.append("Add new block definition with Id = ");

    //add id
    sprintf(hlpstr, "%d", newDef->get_ID());
    infoMsg.append(hlpstr);
    logging::Info(infoMsg);

    //add the new blockdefinition to the end of the
    //vector with all current blockdefinitions
    this->BlockDefinitions.push_back(newDef);

    //return the index for the
    //new blockdefinition
    outIndex = (irr::u32)(BlockDefinitions.size()) - 1;

    return true;
}

//Makes sure a specified input Blockdefinition exists in the current levelfile.
//If it does not exist yet it is newly created;
//Returns true if the requested block definition is available, and the index
//in the vector of all existing Blockdefinitions is returned in output parameter
//outIndex;
//If there is an unexpected error, and no blockdefinition index with this parameters
//can be supplied, this function returns false
//If block definition was newly added (not existing yet) output bool return value is
//set to true, if an already existing block definition is used false is returned
bool LevelFile::RequestBlockDefinition(uint8_t pN, uint8_t pE, uint8_t pS, uint8_t pW, uint8_t pT, uint8_t pB,
                                   uint8_t pNMod, uint8_t pEMod, uint8_t pSMod, uint8_t pWMod, uint8_t pTMod, uint8_t pBMod,
                                    int16_t pUnknown1, int16_t pUnknown2, irr::u32 &outIndex, bool &newlyAdded) {

    irr::u32 outFndIndex;

    //does this block definition already exist?
    if (!SearchBlockDefinitionIndex(pN, pE, pS, pW, pT, pB, pNMod, pEMod, pSMod, pWMod, pTMod, pBMod, outFndIndex)) {
        //specified block definition is not yet existing, create it
        if (!AddBlockDefinition(pN, pE, pS, pW, pT, pB, pNMod, pEMod, pSMod, pWMod, pTMod, pBMod, pUnknown1, pUnknown2, outFndIndex)) {
            //could not create new block definition
            //no more space for new block definition in level file?
            return false;
        } else {
             newlyAdded = true;
        }
    } else {
        //existing block definition was found
        newlyAdded = false;

        std::string infoMsg("");
        char hlpstr[100];

        infoMsg.clear();
        infoMsg.append("Reusing existing block definition with Id = ");

        //add id
        sprintf(hlpstr, "%d", BlockDefinitions.at(outFndIndex)->get_ID());
        infoMsg.append(hlpstr);
        logging::Info(infoMsg);
    }

    outIndex = outFndIndex;

    //block definition does exist
    return true;
}

//returns nullptr if a Column definition with this Id is not found
ColumnDefinition* LevelFile::GetColumnDefinitionWithCertainId(int whichId) {
    std::vector<ColumnDefinition*>::iterator itCol;

    for (itCol = ColumnDefinitions.begin(); itCol != ColumnDefinitions.end(); ++itCol) {
        if ((*itCol)->get_ID() == whichId) {
                return (*itCol);
        }
    }

    return nullptr;
}

//returns nullptr if a block definition with this Id is not found
BlockDefinition* LevelFile::GetBlockDefinitionWithCertainId(int whichId) {
    std::vector<BlockDefinition*>::iterator itBlock;

    for (itBlock = BlockDefinitions.begin(); itBlock != BlockDefinitions.end(); ++itBlock) {
        if ((*itBlock)->get_ID() == whichId) {
                return (*itBlock);
        }
    }

    return nullptr;
}

/*************************************************************
 *                                                           *
 * Column Definition helper routines used for level editor   *
 *                                                           *
 *************************************************************/

//returns true if a column definition object (therefore a column)
//contains exactly the specified input block definition values (means
//does consist of the specified blocks from the input parameters)
//False otherwise
bool LevelFile::CmpColumnDefinition(ColumnDefinition* def, int16_t cmpFloorTextureID, int16_t compUnknown1, int16_t compA, int16_t compB, int16_t compC,
                                    int16_t compD, int16_t compE, int16_t compF, int16_t compG, int16_t compH) {

    //in the original column definitions of the original
    //maps of the game I saw that also the FloorTextureID
    //is taken into account when seperating the columns into
    //different column definitions
    if (def->get_FloorTextureID() != cmpFloorTextureID)
        return false;

    //in the original column definitions of the original
    //maps of the game I saw that also this unknown1 value
    //is taken into account when seperating the columns into
    //different column definitions
    if (def->get_Unknown1() != compUnknown1)
        return false;

    if (def->get_A() != compA)
        return false;

    if (def->get_B() != compB)
        return false;

    if (def->get_C() != compC)
        return false;

    if (def->get_D() != compD)
        return false;

    if (def->get_E() != compE)
        return false;

    if (def->get_F() != compF)
        return false;

    if (def->get_G() != compG)
        return false;

    if (def->get_H() != compH)
        return false;

    //are identical
    return true;
}

//returns true in case a specified input column definition was found in the vector of existing
//columndefinitions in the current level file; false otherwise
//in case definition was found, returns also the index in the vector for this element
//in parameter fndOutIndex
bool LevelFile::SearchColumnDefinitionIndex(int16_t cmpFloorTextureID, int16_t cmpUnknown1, int16_t compA, int16_t compB, int16_t compC, int16_t compD, int16_t compE, int16_t compF,
                                            int16_t compG, int16_t compH, irr::u32 &fndOutIndex) {
    std::vector<ColumnDefinition*>::iterator it;
    irr::u32 currIdx = 0;

    for (it = ColumnDefinitions.begin(); it != ColumnDefinitions.end(); ++it) {
        if (CmpColumnDefinition((*it), cmpFloorTextureID, cmpUnknown1, compA, compB, compC, compD, compE, compF,
                               compG, compH)) {
            //found this definition
            fndOutIndex = currIdx;

            return true;
        }

        currIdx++;
    }

    //not found
    return false;
}

//Returns true if new column definition was successfully created, false otherwise
//if succesfull, returns the new index of the new columndefinition in output parameter outIndex
bool LevelFile::AddColumnDefinition(int16_t newFloorTextureID, int16_t newUnknown1, int16_t newA, int16_t newB, int16_t newC, int16_t newD, int16_t newE, int16_t newF,
                                    int16_t newG, int16_t newH, irr::u32 &outIndex) {

    int i;
    int baseOffset;

    //first find next free column definition location
    //in current level file, if nothing free return with no success

    //it seems we should skip i = 0 here, in the original level the first
    //index i = 0 is still filled with zero bytes; better do the same
    for (i = 1; i < 1024; i++) {
        baseOffset = 98012 + i * 26;
        if (this->m_bytes.at(baseOffset) == 0) {
            //we found a free space
            break;
        }
    }

    //no free space anymore?
    if (i == 1024) {
        return false;
    }

    //create the new columdefinition, use the alternative constructor
    //for the leveleditor for this
    ColumnDefinition *newDef = new ColumnDefinition(i, baseOffset, newFloorTextureID, newUnknown1, newA, newB, newC, newD, newE, newF, newG, newH, 1);

    std::string infoMsg("");
    char hlpstr[100];

    infoMsg.clear();
    infoMsg.append("Add new column definition with Id = ");

    //add id
    sprintf(hlpstr, "%d", newDef->get_ID());
    infoMsg.append(hlpstr);
    logging::Info(infoMsg);

    //add the new columndefinition to the end of the
    //vector with all current columndefinitions
    this->ColumnDefinitions.push_back(newDef);

    //return the index for the
    //new columndefinition
    outIndex = (irr::u32)(ColumnDefinitions.size()) - 1;

    return true;
}

//Makes sure a specified input Columndefinition exists in the current levelfile.
//If it does not exist yet it is newly created;
//Returns true if the requested column definition is available, and the index
//in the vector of all existing Columndefinitions is returned in output parameter
//outIndex;
//If there is an unexpected error, and no Columndefinition index with this parameters
//can be supplied, this function returns false
//If colum definition was newly added (not existing yet) output bool return value is
//set to true, if an already existing column definition is used false is returned
bool LevelFile::RequestColumnDefinition(int16_t pFloorTextureID, int16_t pUnknown1, int16_t pA, int16_t pB, int16_t pC, int16_t pD, int16_t pE, int16_t pF,
                                         int16_t pG, int16_t pH, irr::u32 &outIndex, bool &newlyAdded) {

    irr::u32 outFndIndex;

    //does this column definition already exist?
    if (!SearchColumnDefinitionIndex(pFloorTextureID, pUnknown1, pA, pB, pC, pD, pE, pF, pG, pH, outFndIndex)) {
        //specified column definition is not yet existing, create it
        if (!AddColumnDefinition(pFloorTextureID, pUnknown1, pA, pB, pC, pD, pE, pF, pG, pH, outFndIndex)) {
            //could not create new column definition
            //no more space for new column definition in level file?
            return false;
        } else {
            //new column definition was created succesfully
            newlyAdded = true;
        }
    } else {
        //was existing before
        newlyAdded = false;

        std::string infoMsg("");
        char hlpstr[100];

        infoMsg.clear();
        infoMsg.append("Reusing existing column definition with Id = ");

        //add id
        sprintf(hlpstr, "%d", ColumnDefinitions.at(outFndIndex)->get_ID());
        infoMsg.append(hlpstr);
        logging::Info(infoMsg);
    }

    outIndex = outFndIndex;

    //column definition does exist
    return true;
}

void LevelFile::DebugWriteCellInfoToCsvFile(char* debugOutPutFileName) {
   FILE* debugOutputFile = nullptr;

   debugOutputFile = fopen(debugOutPutFileName, "w");
   if (debugOutputFile == nullptr) {
         return;
   }

   MapEntry* entry;

   //write a header
   fprintf(debugOutputFile, "Cell X;Cell Y;Height;TexId;TexMod;POI;Illumination;Reserved1;Reserved2\n");
   for (int x = 0; x < LEVELFILE_WIDTH; x++) {
       for (int y = 0; y < LEVELFILE_HEIGHT; y++) {
           entry = pMap[x][y];

           if (entry != nullptr) {
               //write the next entry
               fprintf(debugOutputFile, "%d;%d;%lf;%d;%d;%d;%d;%d;%d\n",
                    x, y, entry->m_Height, entry->m_TextureId,
                    entry->m_TextureModification, entry->mPointOfInterest,
                     entry->mIllumination, entry->mReserved1, entry->mReserved2);
            }
      }
   }

   fclose(debugOutputFile);
}

//Returns true if new entity was successfully created, false otherwise
//if succesfull, returns the new index of the new entity in output parameter outIndex
bool LevelFile::AddEntityAtCell(int x, int y, irr::f32 heightTerrain, Entity::EntityType ofType, irr::u32 &outIndex) {
    int i;
    int baseOffset;

    //first find next free entity item location
    //in current level file, if nothing free return with no success

    //it seems we should skip i = 0 here, in the original level the first
    //index i = 0 is still filled with zero bytes; better do the same
    int icurrentMaxId = 0;

    std::vector<EntityItem*>::iterator it;
    for (it = Entities.begin(); it != Entities.end(); ++it) {
       if ((*it)->get_ID() > icurrentMaxId) {
           icurrentMaxId = (*it)->get_ID();
       }
    }

    //icurrentMaxId now contains the highest existing Entity Id used currently
    //in this map
    //next free Id is increment by 1
    i = icurrentMaxId + 1;

    //no free space anymore?
    if (i == 4000) {
        return false;
    }

    //we still have free space => create item
    baseOffset = i * 24;

    //create the new Entity Item, use the alternative constructor
    //for the leveleditor for this
    //create the new low level entity Item
    EntityItem* newItem = new EntityItem(x, y, heightTerrain, i, baseOffset, ofType);

    std::string infoMsg("");
    char hlpstr[100];

    infoMsg.clear();
    infoMsg.append("Add new EntityItem with Id = ");

    //add id
    sprintf(hlpstr, "%d", newItem->get_ID());
    infoMsg.append(hlpstr);
    logging::Info(infoMsg);

    //add the new EntityItem to the end of the
    //vector with all current EntityItems
    this->Entities.push_back(newItem);

    //return the index for the
    //new EntityItem
    outIndex = (irr::u32)(Entities.size()) - 1;

    return true;
}

/**************************************
 * Save Map Stuff                     *
 **************************************/

bool LevelFile::saveBlockTexTable() {
    std::vector<BlockDefinition*>::iterator it;

    //iterate overall all existing block definitions and
    //collect data
    for (it = BlockDefinitions.begin(); it != BlockDefinitions.end(); ++it) {
        //first prepare updated/new data
        (*it)->WriteChanges();

        //copy new data to overall save data array
        std::copy((*it)->m_wBytes.begin(), (*it)->m_wBytes.end(), this->m_wBytes.begin()
                  + (*it)->get_Offset());
    }

   return(true);
}

bool LevelFile::saveEntitiesTable() {
    std::vector<EntityItem*>::iterator it;

    //iterate overall all existing Entities and
    //collect data
    for (it = Entities.begin(); it != Entities.end(); ++it) {
        //first prepare updated/new data
        (*it)->WriteChanges();

        //copy new data to overall save data array
        std::copy((*it)->m_wBytes.begin(), (*it)->m_wBytes.end(), this->m_wBytes.begin()
                  + (*it)->get_Offset());
    }

   return(true);
}

bool LevelFile::saveColumnsTable() {
    std::vector<ColumnDefinition*>::iterator it;

    //iterate overall all existing Column Definitions and
    //collect data
    for (it = ColumnDefinitions.begin(); it != ColumnDefinitions.end(); ++it) {
        //first prepare updated/new data
        (*it)->WriteChanges();

        //copy new data to overall save data array
        std::copy((*it)->m_wBytes.begin(), (*it)->m_wBytes.end(), this->m_wBytes.begin()
                  + (*it)->get_Offset());
    }

   return(true);
}

bool LevelFile::saveMap() {
    MapEntry* mapEntry;

    //iterate overall all existing map entries and
    //collect data
    for (int y = 0; y < Height(); y++) {
     for (int x = 0; x < Width(); x++)  {
        mapEntry = this->pMap[x][y];

        if (mapEntry != nullptr) {
            //first prepare updated/new data
            mapEntry->WriteChanges();

            //copy new data to overall save data array
            std::copy(mapEntry->m_wBytes.begin(), mapEntry->m_wBytes.end(), this->m_wBytes.begin()
                      + mapEntry->get_Offset());
        }
      }
    }

   return(true);
}

bool LevelFile::saveRegionTable() {
    //region table, starts at file offset 246924
    std::copy(this->regionTable.begin(), regionTable.end(), this->m_wBytes.begin()
              + 246924);

    return true;
}

bool LevelFile::saveUnknownTables() {
    //third table Offset0
    std::copy(this->unknownTable0Data.begin(), unknownTable0Data.end(), this->m_wBytes.begin()
              + 0);

    //third table Offset3168
    std::copy(this->unknownTable3168Data.begin(), unknownTable3168Data.end(), this->m_wBytes.begin()
              + 3168);

    //fourth table Offset3312
    std::copy(this->unknownTable3312Data.begin(), unknownTable3312Data.end(), this->m_wBytes.begin()
              + 3312);

    //fourth table Offset102068
    std::copy(this->unknownTable102068Data.begin(), unknownTable102068Data.end(), this->m_wBytes.begin()
              + 102068);

    //std::fill(this->unknownTable247604Data.begin(), unknownTable247604Data.end(), 0);

    //std::fill(this->unknownTable127468Data.begin(), unknownTable127468Data.end(), 0);

    //12th table Offset127468
    std::copy(this->unknownTable127468Data.begin(), unknownTable127468Data.end(), this->m_wBytes.begin()
              + 127468);

    //11th table Offset247604
    std::copy(this->unknownTable247604Data.begin(), unknownTable247604Data.end(), this->m_wBytes.begin()
              + 247604);

    return true;
}

