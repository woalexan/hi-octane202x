/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did just translation to C++, and extended with some new code)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "mapentry.h"
#include <stdlib.h>
#include "../utils/crc32.h"
#include "../utils/logging.h"
#include "columndefinition.h"

MapEntry::MapEntry(int x, int z, int offset, std::vector<uint8_t> bytes, std::vector<ColumnDefinition*> columnDefinitions) {
    this->m_X = x;
    this->m_Z = z;

    this->m_Offset = offset;
    this->m_Bytes = bytes;
    this->m_Column = nullptr;

    //for debugging of level saving, comment out later
    this->m_wBytes.resize(this->m_Bytes.size());
    std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);

    //each map entry is 12 bytes long
    //Byte 0:  Cell Illumination value: This value controls how well illuminated a cell is
    //Byte 1:  Cell Illumination value: This value controls how well illuminated a cell is
    //Byte 2:  Height
    //Byte 3:  Height
    //Byte 4:  cid (cell id)
    //Byte 5:  cid (cell id)
    //Byte 6:  Point of Interest
    //Byte 7:  Point of Interest
    //Byte 8:  Reserved 1 (seems to be not used)
    //Byte 9:  Reserved 1 (seems to be not used)
    //Byte 10:  Texture Modification
    //Byte 11:  Reserved 2 (seems to be not used)

    //Reserved 1 & Reserved 2: For both values I checked in every level of the original
    //game. Is not a single time non zero. Was maybe reserved for
    //a future expansion, and never used (maybe reserved).

    this->m_Height = (((float)(bytes.at(2))) / 256.0f) + (float)(bytes.at(3));

    int16_t cid = ConvertByteArray_ToInt16(bytes, 4);

    if (cid < 0) { // is column of blocks?
       uint16_t element = -cid - 1;
       //std::cout << "Element " << element << std::endl;
       if (element > columnDefinitions.size()-1) {
        //error this should never happen
           logging::Error("Column ID outside allowed size found!!! This should not happen");
       } else {
           //use the next if statement for debugging purposes, if you one want to add columns
           //in the level with specific column definition numbers
           //if (element == 0) {
               this->m_Column = columnDefinitions.at(element);
           //}

           cid = columnDefinitions.at(element)->get_FloorTextureID();
       }
    }

    this->m_TextureId = cid;
    this->m_TextureModification = (uint8_t)(bytes.at(10));

    mPointOfInterest = ConvertByteArray_ToInt16(bytes, 6);

    //read cell illumination value
    mIllumination = ConvertByteArray_ToInt16(bytes, 0);

    //read also unknown data
    mReserved1 = ConvertByteArray_ToInt16(bytes, 8);
    mReserved2 = bytes.at(11);
}

MapEntry::~MapEntry() {
}

int8_t MapEntry::GetTextureModification() {
    int8_t result = (int8_t)((m_TextureModification >> 4));

    return result;
}

void MapEntry::SetTextureModification(int8_t newValue) {
    //We do not want to get negative values!
    if (newValue < 0) {
        return;
    }

    //Wolf 08.11.2025: clear current texture modification value bits
    //but keep other lowest 4 bits intact!
    m_TextureModification &= 0b1111;

    uint8_t newModBits = ((uint8_t)(newValue) << 4);

    //or in the new texture modification bits
    m_TextureModification |= newModBits;
}

uint8_t MapEntry::GetTextureModificationLowerNibble() {
    uint8_t result = (uint8_t)((m_TextureModification & 0b1111));

    return result;
}

void MapEntry::set_Column(ColumnDefinition* newColumnDefinition) {
    this->m_Column = newColumnDefinition;
}

ColumnDefinition* MapEntry::get_Column() {
    return(m_Column);
}

int MapEntry::get_X() {
    return(this->m_X);
}

int MapEntry::get_Z() {
    return(this->m_Z);
}

void MapEntry::set_X(int new_X) {
    this->m_X = new_X;
}

void MapEntry::set_Z(int new_Z) {
    this->m_Z = new_Z;
}

bool MapEntry::WriteChanges() {
    //convert height information to bytes
    /*unsigned char div = (unsigned char)(this->m_Height / 255.0f);
    float remainder = (this->m_Height - float(div));
    unsigned char rem = (unsigned char)(remainder);

    this->m_wBytes.at(2) = div;
    this->m_wBytes.at(3) = rem;*/

    //convert height information to bytes
    ConvertAndWriteFloatToByteArray(this->m_Height, this->m_wBytes, 2);

    //is this a column at this location
    if (this->get_Column() != nullptr) {
        //we have to calculate and store the data in a different way
        //calculate back the column id
        uint16_t element = this->get_Column()->get_ID();

        int16_t cid = -element;

        ConvertAndWriteInt16ToByteArray(cid, this->m_wBytes, 4);
    } else {
        //no column, just write texture ID
        ConvertAndWriteInt16ToByteArray(m_TextureId, this->m_wBytes, 4);
    }

    this->m_wBytes.at(10) = (uint8_t)(this->m_TextureModification);

    ConvertAndWriteInt16ToByteArray(mPointOfInterest, this->m_wBytes, 6);

    //write illumination value (how much light does a cell receive)
    ConvertAndWriteInt16ToByteArray(mIllumination, this->m_wBytes, 0);

    //write also unknown data
    ConvertAndWriteInt16ToByteArray(mReserved1, this->m_wBytes, 8);
    this->m_wBytes.at(11) = (uint8_t)(mReserved2);

    return true;
}
