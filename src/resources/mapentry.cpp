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

MapEntry::MapEntry(int x, int z, int offset, std::vector<uint8_t> bytes, std::vector<ColumnDefinition*> columnDefinitions) {
    this->m_X = x;
    this->m_Z = z;

    this->m_Offset = offset;
    this->m_Bytes = bytes;
    this->m_Column = NULL;

    //for debugging of level saving, comment out later
    this->m_wBytes.resize(this->m_Bytes.size());
    std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);

    //each map entry is 12 bytes long
    //Byte 0:  Unknown 1
    //Byte 1:  Unknown 1
    //Byte 2:  Height
    //Byte 3:  Height
    //Byte 4:  cid (cell id)
    //Byte 5:  cid (cell id)
    //Byte 6:  Point of Interest
    //Byte 7:  Point of Interest
    //Byte 8:  Unknown 2
    //Byte 9:  Unknown 2
    //Byte 10:  Texture Modification
    //Byte 11:  Unknown 3

    this->m_Height = (((float)(bytes.at(2))) / 256.0f) + (float)(bytes.at(3));

    int16_t cid = ConvertByteArray_ToInt16(bytes, 4);

    if (cid < 0) { // is column of blocks?
       uint16_t element = -cid - 1;
       if (element > columnDefinitions.size()-1) {
        //error this should never happen
           logging::Error("Column ID outside allowed size found!!! This should not happen");
       } else {
       this->m_Column = columnDefinitions.at(element);
       cid = m_Column->get_FloorTextureID();
       }
    }

    this->m_TextureId = cid;
    this->m_TextureModification = (bytes.at(10) >> 4);

    mPointOfInterest = ConvertByteArray_ToInt16(bytes, 6);

    //read also unknown data
    mUnknown1 = ConvertByteArray_ToInt16(bytes, 0);
    mUnknown2 = ConvertByteArray_ToInt16(bytes, 8);
    mUnknown3 = bytes.at(11);
}

MapEntry::~MapEntry() {
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
    if (this->get_Column() != NULL) {
        //we have to calculate and store the data in a different way
        //calculate back the column id
        uint16_t element = this->get_Column()->get_ID();

        int16_t cid = -element;

        ConvertAndWriteInt16ToByteArray(cid, this->m_wBytes, 4);
    } else {
        //no column, just write texture ID
        ConvertAndWriteInt16ToByteArray(m_TextureId, this->m_wBytes, 4);
    }

    this->m_wBytes.at(10) = (uint8_t)(this->m_TextureModification << 4);

    ConvertAndWriteInt16ToByteArray(mPointOfInterest, this->m_wBytes, 6);

    //write also unknown data
    ConvertAndWriteInt16ToByteArray(mUnknown1, this->m_wBytes, 0);
    ConvertAndWriteInt16ToByteArray(mUnknown2, this->m_wBytes, 8);
    this->m_wBytes.at(11) = (uint8_t)(mUnknown3);

    return true;
}
