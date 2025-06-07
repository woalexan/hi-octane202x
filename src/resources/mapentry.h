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

#include "tableitem.h"
#include "columndefinition.h"
#include "../utils/crc32.h"
#include "../utils/logging.h"
#include <iostream>

class MapEntry:public TableItem {
public:
    MapEntry(int x, int z, int offset, std::vector<uint8_t> bytes, std::vector<ColumnDefinition*> columnDefinitions);
    virtual ~MapEntry();

    float m_Height;
    int16_t m_TextureId;
    int8_t m_TextureModification;

   // ColumnDefinition Column { get; private set; }
    int get_X();
    int get_Z();
    void set_X(int new_X);
    void set_Z(int new_Z);
    void set_Column(ColumnDefinition* newColumnDefinition);
    ColumnDefinition* get_Column();

    bool WriteChanges() override;

    int16_t mPointOfInterest;

    int16_t mUnknown1;
    int16_t mUnknown2;
    int8_t mUnknown3;

protected:
    int m_X;
    int m_Z;

    ColumnDefinition* m_Column = nullptr;
};

#endif // MAPENTRY_H
