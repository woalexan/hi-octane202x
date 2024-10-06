/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++, and extended with some new code)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "levelfile.h"

LevelFile::LevelFile(std::string filename) {
   this->m_Filename = filename;
   this->m_Ready = false;
   bool ready_result;

   mMapRegionVec = new std::vector<MapTileRegionStruct*>();

   /* try to open file to read */
   ifstream ifile;
   std::streampos fileSize;

   ifile.open(filename);
      if(ifile) {
         std::cout <<"Level file found and openend" << endl;
      } else {
        return;
      }

    // get its size:
    ifile.seekg(0, std::ios::end);
    fileSize = ifile.tellg();
    ifile.seekg(0, std::ios::beg);

    // read the data:
    std::vector<unsigned char> fileData(fileSize);
    ifile.read((char*) &fileData[0], fileSize);

    this->m_bytes = fileData;

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

    std::cout << "found map = " << mapname << endl;

    ready_result = loadBlockTexTable() && loadColumnsTable() && loadMap() && loadEntitiesTable() &&
            loadMapRegions() && loadUnknownTableOffset358222();
    this->m_Ready = ready_result;

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
         this->pMap[x][y] = NULL;
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
    uint itemIdx = 0;

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
    std::vector<unsigned char>::const_iterator startslice;
    std::vector<unsigned char>::const_iterator endslice;
    std::vector<unsigned char> dataslice;

    this->Entities.clear();

     //Entities = new OrderedDictionary<int, EntityItem>();

    for (int i = 0; i < 4000; i++) {
        int baseOffset = i * 24;
        if (this->m_bytes.at(baseOffset) == 0) continue;

        startslice = this->m_bytes.begin() + baseOffset;
        endslice = this->m_bytes.begin()+ baseOffset + 24;
        dataslice.assign(startslice, endslice);

        EntityItem *item = new EntityItem(i, baseOffset, dataslice);
        item->set_Y(this->pMap[(int)item->get_X()][(int)item->get_Z()]->m_Height);
        this->Entities.push_back(item);
       }

    return(true);
}

bool LevelFile::loadBlockTexTable() {
    std::vector<unsigned char>::const_iterator startslice;
    std::vector<unsigned char>::const_iterator endslice;
    std::vector<unsigned char> dataslice;

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
    }

  return(true);
}

bool LevelFile::loadColumnsTable() {
    std::vector<unsigned char>::const_iterator startslice;
    std::vector<unsigned char>::const_iterator endslice;
    std::vector<unsigned char> dataslice;

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
    }

 return(true);
}

bool LevelFile::loadMap() {
    std::vector<unsigned char>::const_iterator startslice;
    std::vector<unsigned char>::const_iterator endslice;
    std::vector<unsigned char> dataslice;

    ColumnsStruct NewStruct;
    MapPointOfInterest poi;

    //PointsOfInterest = new List<MapPointOfInterest>();
    PointsOfInterest.clear();
    Columns.clear();

    //  Columns = new OrderedDictionary<Vector3, ColumnDefinition>();
    //Map = new MapEntry[Width, Height];

    // entry is 12 bytes long, map is at end of file
    int numBytes = 12 * Width() * Height();

    int i = this->m_bytes.size() - numBytes;

    for (int y = 0; y < Height(); y++) {
     for (int x = 0; x < Width(); x++) {
         startslice = this->m_bytes.begin() + i;
         endslice = this->m_bytes.begin()+ i + 12;
         dataslice.assign(startslice, endslice);

         MapEntry *entry = new MapEntry(x, y, i, dataslice, ColumnDefinitions);

         if (entry->get_Column() != NULL) {
              NewStruct.Vector3 = irr::core::vector3d<float>(x, 0, y);
              NewStruct.Columns = entry->get_Column();

              this->Columns.push_back(NewStruct);
         }

         this->pMap[x][y] = entry;

         // check for points of interest
        int16_t poiValue = (this->m_bytes.at(i + 7) << 8) | this->m_bytes.at(i + 6);
        if (poiValue > 0) {
            poi.Value = poiValue;
            poi.Position = irr::core::vector3d<float>(x, 0.0f, y);
            PointsOfInterest.push_back(poi);
         }

        i += 12;
    }
   }
 return(true);
}

bool LevelFile::Save(std::string filename) {
    MapEntry* entry;
    std::vector<unsigned char> newdata;
    std::vector<unsigned char>::iterator ptr;
    int i;

    for (int y = 0; y < Height(); y++) {
        for (int x = 0; x < Width(); x++) {
            entry = this->pMap[x][y];
            entry->WriteChanges();
            newdata = entry->get_Bytes();

            int i = 0;
            for (ptr = newdata.begin(); ptr < newdata.end(); ptr++) {
                  this->m_bytes.at(entry->get_Offset()+i) = *ptr;
                  i++;
            }
         }
      }

   std::ofstream outputfile(filename, std::ios::out|std::ios::binary);
   std::copy(this->m_bytes.cbegin(), this->m_bytes.cend(),
          std::ostream_iterator<unsigned char>(outputfile));

   return(true);
}

bool LevelFile::loadMapRegions() {
    std::vector<unsigned char>::const_iterator startslice;
    std::vector<unsigned char>::const_iterator endslice;

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

bool LevelFile::InvestigatePrintUnknownTableOffset247264() {
    std::vector<unsigned char>::iterator it;

    int cnt = 0;

    char finalpath[100];

    strcpy(finalpath, this->get_Filename().c_str());
    strcat(finalpath, "-table247264.txt");

    //now we have our output filename
    FILE* oFile = fopen(finalpath, "w");
    if (oFile == NULL) {
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

    FILE* oFile = NULL;

    oFile = fopen(this->m_Filename.c_str(), "r+b");
    if (oFile == NULL) {
       return false;
    }

    std::vector<unsigned char>::iterator it;
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

    FILE* oFile = NULL;

    oFile = fopen(this->m_Filename.c_str(), "r+b");
    if (oFile == NULL) {
       return false;
    }

    std::vector<unsigned char>::iterator it;
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
    std::vector<unsigned char>::const_iterator startslice;
    std::vector<unsigned char>::const_iterator endslice;

    int baseOffset = 358222;

    startslice = this->m_bytes.begin() + baseOffset;
    //unknown table has 169 entries with each 16 bytes
    endslice = this->m_bytes.begin()+ baseOffset + 169 * 16;
    unknownTable358222Data.assign(startslice, endslice);

    return true;
}
