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
#include "../utils/crc32.h"
#include "blockdefinition.h"
#include "columndefinition.h"
#include "mapentry.h"
#include "entityitem.h"
#include <iterator>
#include "../definitions.h"

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

    // read the data:
    //std::vector<uint8_t> fileData(fileSize);
    //ifile.read(reinterpret_cast<char*>(fileData.data()), fileData.size());

    //this->m_bytes = fileData;

    //read the data
    this->m_bytes.resize(fileSize);
    ifile.read(reinterpret_cast<char*>(this->m_bytes.data()), this->m_bytes.size());

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

    Crc32 crc;
    unsigned int cs = crc.ComputeChecksum(this->m_bytes);

    //use crc to find out which map was loaded
    std::string mapname("");
    switch (cs) {
      case 3308913189: mapname.append("Amazon Delta Turnpike"); break;
      case 2028380229: mapname.append("Trans-Asia Interstate"); break;
      case 3087166776: mapname.append("Shanghai Dragon"); break;
      case 1401140937: mapname.append("New Chernobyl Central"); break;
      case 4215346212: mapname.append("Slam Canyon"); break;
      case 3809451489: mapname.append("Thrak City"); break;
      case 3062313907: mapname.append("Ancient Mine Town"); break;
      case 3081898808: mapname.append("Arctic Land"); break;
      case 540505443: mapname.append("Death Match Arena"); break;
      default: mapname.append("unknown/Custom"); break;
    }

    msg.clear();
    msg.append("found map = ");
    msg.append(mapname);
    logging::Info(msg);

    ready_result = loadBlockTexTable() && loadColumnsTable() && loadMap() && loadEntitiesTable() &&
            loadMapRegions() && loadUnknownTableOffset358222(); /*&& loadUnknownTableOffset0() &&
            loadUnknownTableOffset96000() && loadUnknownTableOffset402192() && loadUnknownTableOffset362208()
            && loadUnknownTableOffset357664() && loadUnknownTableOffset355360() && loadUnknownTableOffset355104()
            && loadUnknownTableOffset354848() && loadUnknownTableOffset292096() && loadUnknownTableOffset160160()
            && loadUnknownTableOffset157760() && loadUnknownTableOffset155376() && loadUnknownTableOffset141008() &&
            loadUnknownTableOffset143408() && loadUnknownTableOffset145808() && loadUnknownTableOffset148192() &&
            loadUnknownTableOffset150592() && loadUnknownTableOffset152976();*/
    this->m_Ready = ready_result;

    //for debugging of level save functionality
    //disable later
    this->m_wBytes.resize(this->m_bytes.size());
    std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_bytes.size(), 0x55);

    //detect additional map regions
    DetectAdditionalRegionsTextureId();
    DetectAdditionalRegionsBasedOnColumns();

    //InvestigatePrintUnknownTableOffset247264();

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

//Helper function, Returns true if a certain tile has the specified textureId
//Returns false otherwise, or if the specified location on the tile map is invalid
bool LevelFile::CheckTileForTextureId(int posX, int posY, int textureId) {
    if ((posX < 0) || (posY < 0))
        return false;

    if ((posX >= this->Width()) || (posY >= this->Height()))
        return false;

    if (this->pMap[posX][posY]->m_TextureId != textureId)
        return false;

    //tile has the specified textureId
    //return true;
    return true;
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

     //Entities = new OrderedDictionary<int, EntityItem>();

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

         // check for points of interest
        int16_t poiValue = (this->m_bytes.at(i + 7) << 8) | this->m_bytes.at(i + 6);
        if (poiValue > 0) {
            poi.Value = poiValue;
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
   //saveUnknownTables();

   //copy initial read level data to write level data
   //array, to make sure that all data makes sense we do not
   //yet know what it means
   std::copy(this->m_bytes.begin(), this->m_bytes.end(), this->m_wBytes.begin());

   saveMap();
   saveColumnsTable();
   saveBlockTexTable();
   saveEntitiesTable();

   //write prepared data to output file
   std::ofstream outputfile(filename, std::ios::out|std::ios::binary);
   std::copy(this->m_wBytes.cbegin(), this->m_wBytes.cend(),
          std::ostream_iterator<uint8_t>(outputfile));

   return(true);
}

/*******************************************************************************
 * Important note: In the first 2 or 3 levels of the game (I forgot exactly)   *
 * This "region" information inside this level table is working perfectly, and *
 * helps to tell where we need to charge something, and where the player start *
 * locations are. Problem is in the higher level number levels of the game the *
 * charge areas and start location is not properly defined anymore in this     *
 * table. I believe what happend is, that during the first less complex levels *
 * (less number charges etc..) this system worked great, but later the levels  *
 * got more complex and the four entries in this table were far not enough     *
 * anymore. I believe the game developers did not care/were not able to change *
 * this table, and I (at least believe) the game falls back to use the         *
 * textureID in the level data to find out where stuff happens, or where the   *
 * start locations are exactly. At least this is what I will do. Problem: the  *
 * texture ID solution alone does also not work, because for example there     *
 * is a level (I believe level 2) where the shield charger has not the correct *
 * TextureID places in the level and therefore we also rely on the table data  *
 * below in the level file. So we have to use both solutions in parallel to    *
 * make it work. And there is even a third case in levels 5 and 6 (see further *
 * below)                                                                      *
 *******************************************************************************/
//The function below reads the available "region" data table inside the level file
//and uses this information to define first regions
bool LevelFile::loadMapRegions() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 247264;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table (later found out it is for map region definition) has 4 entries with each 85 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 4 * 85;
    unknownTable247264Data.assign(startslice, endslice);

    //also extract data we already know from this "unknown" table
    for (int i = 0; i < 4; i++) {
        //is there an entry? if so create a MapTileRegionStruct with info
        //we have an entry if byte 0 is unequal to 0
        if (unknownTable247264Data.at(i * 85) != 0x0) {
            //create new entry
            MapTileRegionStruct* newRegion = new MapTileRegionStruct();
            newRegion->regionId = i;

            //Byte 0 defines the region type
            newRegion->regionType = unknownTable247264Data.at(i * 85);
            //value 1 means Shield charger location
            //value 2 means Fuel charger location
            //value 3 means Ammo charger location
            //value 4 means map start location

            //middle point of rectangle that defines region is stored
            //at byte location 25 and location 27 of this current table entry line
            newRegion->regionCenterTileCoord.X = unknownTable247264Data.at(i * 85 + 25);
            newRegion->regionCenterTileCoord.Y = unknownTable247264Data.at(i * 85 + 27);

            //region size is specified in terms of tiles counted from
            //the middle point towards both axis directions
            //for "X-axis" the deltaX is stored at byte 46
            irr::u8 deltaX = unknownTable247264Data.at(i * 85 + 46);

            //for "Y-axis" the deltaY is stored at byte 48
            irr::u8 deltaY = unknownTable247264Data.at(i * 85 + 48);

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

void LevelFile::VerifyRegionFound(irr::core::vector2di startCell, int foundTextureId, irr::u8 expectedRegionType) {
    //look around the startCell into all directions wo see how far the region
    //described by the foundTextureId does extend
    irr::u16 tileXmin;
    irr::u16 tileXmax;
    irr::u16 tileYmin;
    irr::u16 tileYmax;
    irr::u16 currCoord;

    bool mapEndHit = false;
    bool texFnd;

    currCoord = startCell.X;

    do {
        texFnd = this->CanIFindTextureIdAroundCell(currCoord, startCell.Y, foundTextureId);

        currCoord -= 1;

        if (currCoord < 0)
            mapEndHit = true;
    } while ((!mapEndHit) && texFnd);

    if (!texFnd) {
        tileXmin = currCoord + 1;
    } else {
        tileXmin = currCoord;
    }

    currCoord = startCell.X;
    mapEndHit = false;

    do {
        texFnd = this->CanIFindTextureIdAroundCell(currCoord, startCell.Y, foundTextureId);
        currCoord += 1;

        if (currCoord >= LEVELFILE_WIDTH)
            mapEndHit = true;
    } while ((!mapEndHit) && texFnd);

    if (!texFnd) {
        tileXmax = currCoord - 1;
    } else {
      tileXmax = currCoord;
    }

    currCoord = startCell.Y;
    mapEndHit = false;

    do {
        texFnd = this->CanIFindTextureIdAroundCell(startCell.X, currCoord, foundTextureId);
        currCoord -= 1;

        if (currCoord < 0)
            mapEndHit = true;
    } while ((!mapEndHit) && texFnd);

    if (!texFnd) {
        tileYmin = currCoord + 1;
    } else {
        tileYmin = currCoord;
    }

    currCoord = startCell.Y;
    mapEndHit = false;

    do {
        texFnd = this->CanIFindTextureIdAroundCell(startCell.X, currCoord, foundTextureId);
        currCoord += 1;

        if (currCoord >= LEVELFILE_HEIGHT)
            mapEndHit = true;
    } while ((!mapEndHit) && texFnd);

    if (!texFnd) {
     tileYmax = currCoord - 1;
    } else {
      tileYmax = currCoord;
    }

    irr::u16 midCoordX;
    irr::u16 midCoordY;

    //calculate region middle cell
    midCoordX = ((tileXmax - tileXmin) / 2) + tileXmin;
    midCoordY = ((tileYmax - tileYmin) / 2) + tileYmin;

    irr::s16 deltaCoordX;
    irr::s16 deltaCoordY;
    bool regionAlreadyFound = false;

    //is there already a region defined close to this middle point
    //could be that this charger is already defined by the "region" data table
    //inside the levelfile, if so skip adding this region a second time
    std::vector<MapTileRegionStruct*>::iterator it;

    for (it = this->mMapRegionVec->begin(); it != this->mMapRegionVec->end(); ++it) {
        deltaCoordX = (*it)->regionCenterTileCoord.X - midCoordX;
        deltaCoordY = (*it)->regionCenterTileCoord.Y - midCoordY;

        if ((abs(deltaCoordX) < 3) && (abs(deltaCoordY) < 3)) {
            //we found possible region match
            if ((*it)->regionType == expectedRegionType) {
                //yes match
                regionAlreadyFound = true;
                break;
            }
        }
    }

    //have we found a match, if not add this region
    if (!regionAlreadyFound) {
        //calculate remaining stuff
        MapTileRegionStruct *newRegion = new MapTileRegionStruct();

        newRegion->regionId = (irr::u8)(mMapRegionVec->size());
        newRegion->regionType = expectedRegionType;
        newRegion->tileXmin = tileXmin;
        newRegion->tileXmax = tileXmax;
        newRegion->tileYmin = tileYmin;
        newRegion->tileYmax = tileYmax;
        newRegion->regionCenterTileCoord.set(midCoordX, midCoordY);

        //add the new region to the region vector
        this->mMapRegionVec->push_back(newRegion);
    }
}

//The second function below takes the TextureID information from the level file
//and tries to locate additional regions that were initially not stored in the
//"region" data table of the level file (because table did not have enough space or
//whatever reason there was, see description above)
void LevelFile::DetectAdditionalRegionsTextureId() {
    //the point of Interest table in the level File also point
    //towards the start area / chargers in all levels
    //we can use this information as well to detect more regions
    std::list<MapPointOfInterest>::iterator it;

    bool fuelChargerFnd;
    bool shieldChargerFnd;
    bool ammoChargerFnd;
    bool startPointFnd;
    irr::core::vector3df pos3D;
    irr::core::vector2di cell;

    for (it = this->PointsOfInterest.begin(); it !=this->PointsOfInterest.end(); ++it) {
         pos3D = (*it).Position;

         cell.X = (irr::s32)(-pos3D.X / DEF_SEGMENTSIZE);
         cell.Y = (irr::s32)(pos3D.Z / DEF_SEGMENTSIZE);

         //shield charger has texture ID #51
         //ammo charger has texture ID #47
         //fuel charger has texture ID #43
         //a start point has texture ID #122
         fuelChargerFnd = CanIFindTextureIdAroundCell(cell.X, cell.Y, 43);
         shieldChargerFnd = CanIFindTextureIdAroundCell(cell.X, cell.Y, 51);
         ammoChargerFnd = CanIFindTextureIdAroundCell(cell.X, cell.Y, 47);
         startPointFnd = CanIFindTextureIdAroundCell(cell.X, cell.Y, 122);

         //do we only find one type of charger or region? if so continue
         if (fuelChargerFnd && (!shieldChargerFnd) && (!ammoChargerFnd) && (!startPointFnd)) {
             //we could have found a fuel charger
             VerifyRegionFound(cell, 43, LEVELFILE_REGION_CHARGER_FUEL);
         }

         if ((!fuelChargerFnd) && shieldChargerFnd && (!ammoChargerFnd) && (!startPointFnd)) {
             //we could have found a shield charger
             VerifyRegionFound(cell, 51, LEVELFILE_REGION_CHARGER_SHIELD);
         }

         if ((!fuelChargerFnd) && (!shieldChargerFnd) && ammoChargerFnd && (!startPointFnd)) {
             //we could have found an ammo charger
             VerifyRegionFound(cell, 47, LEVELFILE_REGION_CHARGER_AMMO);
         }

         if ((!fuelChargerFnd) && (!shieldChargerFnd) && (!ammoChargerFnd) && startPointFnd) {
             //we could have found a start location
             VerifyRegionFound(cell, 122, LEVELFILE_REGION_START);
         }
    }
}

//Helper function for finding charging stations in a third way (used in level 5 and 6)
//returns true if we find the defined texture at least once at this block of any of the faces
//if not returns false
bool LevelFile::CanIFindDefinedTextureAtBlock(BlockDefinition* blockPntr, int textureIdSymbol) {
    if (blockPntr != nullptr) {
        //check all textures of all faces of this block if the are the same as the
        //defined texture ID
        if (blockPntr->get_N() == (uint8_t)(textureIdSymbol))
            return true;

        if (blockPntr->get_E() == (uint8_t)(textureIdSymbol))
            return true;

        if (blockPntr->get_S() == (uint8_t)(textureIdSymbol))
            return true;

        if (blockPntr->get_W() == (uint8_t)(textureIdSymbol))
            return true;

        if (blockPntr->get_B() == (uint8_t)(textureIdSymbol))
            return true;

        if (blockPntr->get_T() == (uint8_t)(textureIdSymbol))
            return true;
    }

    return false;
}

//Helper function for finding charging stations in a third way (used in level 5 and 6)
bool LevelFile::CanIFindColumnWithDefinedTextureOnItAtLocation(int posX, int posY, int textureIdSymbol) {
    //is there even a column at this location
    MapEntry* entry = this->pMap[posX][posY];

    if (entry->get_Column() != nullptr) {
        //there is a column
        ColumnDefinition* columnPntr = entry->get_Column();
        BlockDefinition* blockPntr;
        std::vector<int> blockIDs;
        blockIDs.clear();

        //we are only accepting columns as charging stations where the lowest
        //"cube/box" is not present, so that the driver can driver below it
        if (columnPntr->mInCollisionMesh.at(0) == 0) {
            //collision in lowest block not active, not existing => ok
            //now check all symbol textures at this column if we find the
            //defined textureID on it

            //we need to check this block
            if (columnPntr->get_B() != 0) {
               blockIDs.push_back(columnPntr->get_B());
            }
            if (columnPntr->get_C() != 0) {
               blockIDs.push_back(columnPntr->get_C());
            }
            if (columnPntr->get_D() != 0) {
               blockIDs.push_back(columnPntr->get_D());
            }
            if (columnPntr->get_E() != 0) {
               blockIDs.push_back(columnPntr->get_E());
            }
            if (columnPntr->get_F() != 0) {
               blockIDs.push_back(columnPntr->get_F());
            }
            if (columnPntr->get_G() != 0) {
               blockIDs.push_back(columnPntr->get_G());
            }
            if (columnPntr->get_H() != 0) {
               blockIDs.push_back(columnPntr->get_H());
            }

         //now we have a vector of all block IDs of block definitions we need to check for the
         //defined texture ID
         std::vector<int>::iterator it;
         for (it = blockIDs.begin(); it != blockIDs.end(); ++it) {
             //find block with this block ID
             std::vector<BlockDefinition*>::iterator itBlock;
             blockPntr = nullptr;

             for (itBlock = this->BlockDefinitions.begin(); itBlock != this->BlockDefinitions.end(); ++itBlock) {
                 if ((*itBlock)->get_ID() == (*it)) {
                     blockPntr = (*itBlock);
                     break;
                 }
             }

             //did we find the correct block definition, if so continue
             if (CanIFindDefinedTextureAtBlock(blockPntr, textureIdSymbol)) {
                 //we found the textureID at one of the faces of the block
                 //we can exit here
                 return true;
             }
         }
       }
    }

    //we did not find the textureID we were looking
    //for
    return false;
}

//Helper function for finding charging stations // start points in a second way
bool LevelFile::CanIFindTextureIdAroundCell(int posX, int posY, int textureId) {
    //should return true if below or one tile around the specified cell
    //we find the specified texture Id, false otherwise
    if (this->CheckTileForTextureId(posX, posY, textureId))
        return true;

    if (this->CheckTileForTextureId(posX - 1, posY, textureId))
        return true;

    if (this->CheckTileForTextureId(posX + 1, posY, textureId))
        return true;

    if (this->CheckTileForTextureId(posX, posY - 1, textureId))
        return true;

    if (this->CheckTileForTextureId(posX, posY + 1, textureId))
        return true;

    if (this->CheckTileForTextureId(posX - 1, posY - 1, textureId))
        return true;

    if (this->CheckTileForTextureId(posX + 1, posY + 1, textureId))
        return true;

    if (this->CheckTileForTextureId(posX + 1, posY - 1, textureId))
        return true;

    if (this->CheckTileForTextureId(posX - 1, posY + 1, textureId))
        return true;

    //nothing found, return false
    return false;
}

//Helper function for finding charging stations // start points in a third way
bool LevelFile::CanIFindTextureIdAtColumnsAroundCell(int posX, int posY, int textureId) {
    //should return true if a column at and around (checks also all columns next to this cell)
    //a specified cell has at least a face with a certain defined textureId on it
    //false otherwise
    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX, posY, textureId))
        return true;

    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX - 1, posY, textureId))
        return true;

    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX + 1, posY, textureId))
        return true;

    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX, posY - 1, textureId))
        return true;

    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX, posY + 1, textureId))
        return true;

    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX - 1, posY - 1, textureId))
        return true;

    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX + 1, posY + 1, textureId))
        return true;

    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX + 1, posY - 1, textureId))
        return true;

    if (this->CanIFindColumnWithDefinedTextureOnItAtLocation(posX - 1, posY + 1, textureId))
        return true;

    //nothing found, return false
    return false;
}

void LevelFile::VerifyRegionFoundViaColumns(irr::core::vector2di startCell, int foundTextureId, irr::u8 expectedRegionType) {
    //look around the startCell into all directions to see how far the region
    //described by the foundTextureId does extend based on existing columns
    irr::u16 tileXmin;
    irr::u16 tileXmax;
    irr::u16 tileYmin;
    irr::u16 tileYmax;
    irr::u16 currCoord;

    bool mapEndHit = false;
    bool texFnd;

    currCoord = startCell.X;

    do {
        texFnd = this->CanIFindTextureIdAtColumnsAroundCell(currCoord, startCell.Y, foundTextureId);

        currCoord -= 1;

        if (currCoord < 0)
            mapEndHit = true;
    } while ((!mapEndHit) && texFnd);

    if (!texFnd) {
        tileXmin = currCoord + 1;
    } else {
        tileXmin = currCoord;
    }

    currCoord = startCell.X;
    mapEndHit = false;

    do {
        texFnd = this->CanIFindTextureIdAtColumnsAroundCell(currCoord, startCell.Y, foundTextureId);
        currCoord += 1;

        if (currCoord >= LEVELFILE_WIDTH)
            mapEndHit = true;
    } while ((!mapEndHit) && texFnd);

    if (!texFnd) {
        tileXmax = currCoord - 1;
    } else {
      tileXmax = currCoord;
    }

    currCoord = startCell.Y;
    mapEndHit = false;

    do {
        texFnd = this->CanIFindTextureIdAtColumnsAroundCell(startCell.X, currCoord, foundTextureId);
        currCoord -= 1;

        if (currCoord < 0)
            mapEndHit = true;
    } while ((!mapEndHit) && texFnd);

    if (!texFnd) {
        tileYmin = currCoord + 1;
    } else {
        tileYmin = currCoord;
    }

    currCoord = startCell.Y;
    mapEndHit = false;

    do {
        texFnd = this->CanIFindTextureIdAtColumnsAroundCell(startCell.X, currCoord, foundTextureId);
        currCoord += 1;

        if (currCoord >= LEVELFILE_HEIGHT)
            mapEndHit = true;
    } while ((!mapEndHit) && texFnd);

    if (!texFnd) {
     tileYmax = currCoord - 1;
    } else {
      tileYmax = currCoord;
    }

    irr::u16 midCoordX;
    irr::u16 midCoordY;

    //calculate region middle cell
    midCoordX = ((tileXmax - tileXmin) / 2) + tileXmin;
    midCoordY = ((tileYmax - tileYmin) / 2) + tileYmin;

    irr::s16 deltaCoordX;
    irr::s16 deltaCoordY;
    bool regionAlreadyFound = false;

    //is there already a region defined close to this middle point
    //could be that this charger is already defined by the "region" data table
    //inside the levelfile, if so skip adding this region a second time
    std::vector<MapTileRegionStruct*>::iterator it;

    for (it = this->mMapRegionVec->begin(); it != this->mMapRegionVec->end(); ++it) {
        deltaCoordX = (*it)->regionCenterTileCoord.X - midCoordX;
        deltaCoordY = (*it)->regionCenterTileCoord.Y - midCoordY;

        if ((abs(deltaCoordX) < 3) && (abs(deltaCoordY) < 3)) {
            //we found possible region match
            if ((*it)->regionType == expectedRegionType) {
                //yes match
                regionAlreadyFound = true;
                break;
            }
        }
    }

    //have we found a match, if not add this region
    if (!regionAlreadyFound) {
        //calculate remaining stuff
        MapTileRegionStruct *newRegion = new MapTileRegionStruct();

        newRegion->regionId = (irr::u8)(mMapRegionVec->size());
        newRegion->regionType = expectedRegionType;
        newRegion->tileXmin = tileXmin;
        newRegion->tileXmax = tileXmax;
        newRegion->tileYmin = tileYmin;
        newRegion->tileYmax = tileYmax;
        newRegion->regionCenterTileCoord.set(midCoordX, midCoordY);

        //add the new region to the region vector
        this->mMapRegionVec->push_back(newRegion);
    }
}

/********************************************************
 * Above I already mentioned detection of charging      *
 * areas (fuel, ammo, shield) via a "region" table      *
 * map inside the level file (only allows max. 4        *
 * entries, the detection via Texture ID of the level   *
 * file heightmap. Until level 5 and 6 I thought this   *
 * should solve all my issues regarding this topic.     *
 * But in level 5 and 6 there are at least two charging *
 * areas where there is no entry in the "region" table  *
 * and no correct TextureID visible at this locations   *
 * Only way to tell that there is something special is  *
 * a PointOfInterest marker in the levelfile we already *
 * read, and columns above the player with the          *
 * charging station type symbol. I believe this is all  *
 * we can use here. Therefore I need also a third       *
 * kind of charging station detection, detection via    *
 * column symbols around PointOfInterest locations      *
 * The function below will execute this task.           *
 ********************************************************/

void LevelFile::DetectAdditionalRegionsBasedOnColumns() {
    //the point of Interest table in the level File also point
    //towards the start area / chargers in all levels
    //we can use this information as well to detect more regions
    std::list<MapPointOfInterest>::iterator it;

    bool fuelChargerFnd;
    bool shieldChargerFnd;
    bool ammoChargerFnd;

    irr::core::vector3df pos3D;
    irr::core::vector2di cell;

    for (it = this->PointsOfInterest.begin(); it !=this->PointsOfInterest.end(); ++it) {
         pos3D = (*it).Position;

         cell.X = (irr::s32)(-pos3D.X / DEF_SEGMENTSIZE);
         cell.Y = (irr::s32)(pos3D.Z / DEF_SEGMENTSIZE);

         //TextureIDs interesting for column charger station symbols
         //Fuel = 42
         //Ammo = 46
         //Shield = 50

         fuelChargerFnd = CanIFindTextureIdAtColumnsAroundCell(cell.X, cell.Y, 42);
         shieldChargerFnd = CanIFindTextureIdAtColumnsAroundCell(cell.X, cell.Y, 50);
         ammoChargerFnd = CanIFindTextureIdAtColumnsAroundCell(cell.X, cell.Y, 46);

         if (fuelChargerFnd) {
             //we could have found a fuel charger
             VerifyRegionFoundViaColumns(cell, 42, LEVELFILE_REGION_CHARGER_FUEL);
         }

         if (shieldChargerFnd) {
             //we could have found a shield charger
             VerifyRegionFoundViaColumns(cell, 50, LEVELFILE_REGION_CHARGER_SHIELD);
         }

         if (ammoChargerFnd) {
             //we could have found an ammo charger
             VerifyRegionFoundViaColumns(cell, 46, LEVELFILE_REGION_CHARGER_AMMO);
         }
    }
}

bool LevelFile::InvestigatePrintUnknownTableOffset247264() {
    std::vector<uint8_t>::iterator it;

    int cnt = 0;

    char finalpath[100];

    strcpy(finalpath, this->get_Filename().c_str());
    strcat(finalpath, "-table247264.txt");

    //now we have our output filename
    FILE* oFile = fopen(finalpath, "w");
    if (oFile == nullptr) {
       return false;
    }

    //unknown table has 4 entries with each 85 bytes
    for (int tableNr = 0; tableNr < 4; tableNr++) {
        it = this->unknownTable247264Data.begin() + tableNr * 85;
        for (; it != this->unknownTable247264Data.end();++it) {
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

bool LevelFile::SaveUnknownTableOffset247264() {

    int baseOffset = 247264;

    char* ByteArray;
    ByteArray = new char[this->unknownTable247264Data.size()];

    FILE* oFile = nullptr;

    oFile = fopen(this->m_Filename.c_str(), "r+b");
    if (oFile == nullptr) {
       return false;
    }

    std::vector<uint8_t>::iterator it;
    unsigned long pos = 0;

    for (it = this->unknownTable247264Data.begin(); it != this->unknownTable247264Data.end(); ++it) {
        ByteArray[pos] = (*it);
        pos++;
    }

    fseek(oFile, baseOffset, SEEK_SET);
    fwrite(&ByteArray[0], 1, this->unknownTable247264Data.size(), oFile);

    fclose(oFile);

    return true;
}

bool LevelFile::SaveUnknownTableOffset358222() {

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
}

bool LevelFile::loadUnknownTableOffset358222() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 358222;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 169 entries with each 16 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 169 * 16;
    unknownTable358222Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset0() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 0;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 16 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 16;
    unknownTable0Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset96000() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 96000;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 2048 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 2048;
    unknownTable96000Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset402192() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 402192;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 688 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 688;
    unknownTable402192Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset362208() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 362208;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 3984 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 3984;
    unknownTable362208Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset357664() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 357664;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 528 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 528;
    unknownTable357664Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset355360() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 355360;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 128 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 128;
    unknownTable355360Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset355104() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 355104;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 96 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 96;
    unknownTable355104Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset354848() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 354848;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 112 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 112;
    unknownTable354848Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset292096() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 292096;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 128 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 128;
    unknownTable292096Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset160160() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 160160;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 32 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 32;
    unknownTable160160Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset157760() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 157760;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 64 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 64;
    unknownTable157760Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset155376() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 155376;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 64 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 64;
    unknownTable155376Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset141008() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 141008;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 80 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 80;
    unknownTable141008Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset143408() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 143408;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 64 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 64;
    unknownTable143408Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset145808() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 145808;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 64 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 64;
    unknownTable145808Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset148192() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 148192;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 64 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 64;
    unknownTable148192Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset150592() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 150592;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 64 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 64;
    unknownTable150592Data.assign(startslice, endslice);

    return true;
}

bool LevelFile::loadUnknownTableOffset152976() {
    std::vector<uint8_t>::const_iterator startslice;
    std::vector<uint8_t>::const_iterator endslice;

    int baseOffset = 152976;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 64 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 64;
    unknownTable152976Data.assign(startslice, endslice);

    return true;
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
    outIndex = BlockDefinitions.size() - 1;

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
    outIndex = ColumnDefinitions.size() - 1;

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

            //TODO: add to save POI (point of interests)
        }
      }
    }

   return(true);
}

bool LevelFile::saveUnknownTables() {
    //first Table Offset247264
    std::copy(this->unknownTable247264Data.begin(), unknownTable247264Data.end(), this->m_wBytes.begin()
              + 247264);

    //second table Offset358222
    std::copy(this->unknownTable358222Data.begin(), unknownTable358222Data.end(), this->m_wBytes.begin()
              + 358222);

    //third table Offset0
    std::copy(this->unknownTable0Data.begin(), unknownTable0Data.end(), this->m_wBytes.begin()
              + 0);

    //fourth table Offset96000
    std::copy(this->unknownTable96000Data.begin(), unknownTable96000Data.end(), this->m_wBytes.begin()
              + 96000);

    //fifth table Offset402192
    std::copy(this->unknownTable402192Data.begin(), unknownTable402192Data.end(), this->m_wBytes.begin()
              + 402192);

    //sixth table Offset362208
    std::copy(this->unknownTable362208Data.begin(), unknownTable362208Data.end(), this->m_wBytes.begin()
              + 362208);

    //7th table Offset357664
    std::copy(this->unknownTable357664Data.begin(), unknownTable357664Data.end(), this->m_wBytes.begin()
              + 357664);

    //8th table Offset355360
    std::copy(this->unknownTable355360Data.begin(), unknownTable355360Data.end(), this->m_wBytes.begin()
              + 355360);

    //9th table Offset355104
    std::copy(this->unknownTable355104Data.begin(), unknownTable355104Data.end(), this->m_wBytes.begin()
              + 355104);

    //10th table Offset354848
    std::copy(this->unknownTable354848Data.begin(), unknownTable354848Data.end(), this->m_wBytes.begin()
              + 354848);

    //11th table Offset292096
    std::copy(this->unknownTable292096Data.begin(), unknownTable292096Data.end(), this->m_wBytes.begin()
              + 292096);

    //12th table Offset160160
    std::copy(this->unknownTable160160Data.begin(), unknownTable160160Data.end(), this->m_wBytes.begin()
              + 160160);

    //13th table Offset157760
    std::copy(this->unknownTable157760Data.begin(), unknownTable157760Data.end(), this->m_wBytes.begin()
              + 157760);

    //14th table Offset155376
    std::copy(this->unknownTable155376Data.begin(), unknownTable155376Data.end(), this->m_wBytes.begin()
              + 155376);

    //15th table Offset141008
    std::copy(this->unknownTable141008Data.begin(), unknownTable141008Data.end(), this->m_wBytes.begin()
              + 141008);

    //16th table Offset143408
    std::copy(this->unknownTable143408Data.begin(), unknownTable143408Data.end(), this->m_wBytes.begin()
              + 143408);

    //17th table Offset145808
    std::copy(this->unknownTable145808Data.begin(), unknownTable145808Data.end(), this->m_wBytes.begin()
              + 145808);


    //18th table Offset148192
    std::copy(this->unknownTable148192Data.begin(), unknownTable148192Data.end(), this->m_wBytes.begin()
              + 148192);

    //19th table Offset150592
    std::copy(this->unknownTable150592Data.begin(), unknownTable150592Data.end(), this->m_wBytes.begin()
              + 150592);

    //20th table Offset152976
    std::copy(this->unknownTable152976Data.begin(), unknownTable152976Data.end(), this->m_wBytes.begin()
              + 152976);

    return true;
}


