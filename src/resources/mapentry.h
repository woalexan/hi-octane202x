/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did just translation to C++, and extended with some new code)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MAPENTRY_H
#define MAPENTRY_H

#include <cstdint>
#include <vector>
#include "tableitem.h"

/************************
 * Forward declarations *
 ************************/

class ColumnDefinition;

class MapEntry:public TableItem {
public:
    MapEntry(int x, int z, int offset, std::vector<uint8_t> bytes, std::vector<ColumnDefinition*> columnDefinitions);
    virtual ~MapEntry();

    float m_Height;
    int16_t m_TextureId;

    int get_X();
    int get_Z();
    void set_X(int new_X);
    void set_Z(int new_Z);
    void set_Column(ColumnDefinition* newColumnDefinition);
    ColumnDefinition* get_Column();

    bool WriteChanges() override;

    void SetTextureModification(int8_t newValue);
    int8_t GetTextureModification();

    uint8_t GetTextureModificationLowerNibble();

    int16_t mPointOfInterest;

    //This value controls how well illuminated a cell
    //is. This also affects blocks/column located above.
    //a typical default illumination value seems to be 8191.
    //a spot that is in the shadow is a value of approx. 12000,
    //If a cell receives more light then normal we look at values around 6000.
    int16_t mIllumination;

    //For both values I checked in every level of the original
    //game. Is not a single time non zero. Was maybe reserved for
    //a future expansion, and never used (maybe reserved).
    int16_t mReserved1;
    int8_t mReserved2;

protected:
    int m_X;
    int m_Z;

    ColumnDefinition* m_Column = nullptr;

    uint8_t m_TextureModification;
};

#endif // MAPENTRY_H
