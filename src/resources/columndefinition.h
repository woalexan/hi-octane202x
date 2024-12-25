/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef COLUMNDEFINITION_H
#define COLUMNDEFINITION_H

#include "tableitem.h"
#include <vector>
#include "../utils/crc32.h"

class ColumnDefinition : public TableItem {
public:
    ColumnDefinition(int id, int offset, std::vector<uint8_t> bytes);
    ~ColumnDefinition();

    uint8_t get_Shape();
    int16_t get_FloorTextureID();
    int16_t get_Unknown1();
    int16_t get_Unknown2();
    int16_t get_Unknown3();
    int16_t get_A();
    int16_t get_B();
    int16_t get_C();
    int16_t get_D();
    int16_t get_E();
    int16_t get_F();
    int16_t get_G();
    int16_t get_H();

    std::vector<int> get_Blocks();

    bool WriteChanges() override;

    //for level editor, write into data
    void set_Shape(unsigned char newVal);
    void set_FloorTextureID(int16_t newVal);
    void set_Unknown1(int16_t newVal);
    void set_Unknown2(int16_t newVal);
    void set_Unknown3(int16_t newVal);
    void set_A(int16_t newVal);
    void set_B(int16_t newVal);
    void set_C(int16_t newVal);
    void set_D(int16_t newVal);
    void set_E(int16_t newVal);
    void set_F(int16_t newVal);
    void set_G(int16_t newVal);
    void set_H(int16_t newVal);

    //if 0 block will not be part of collision mesh for
    //player craft collision detection
    //if 1 then block will be part of collision mesh
    //information order is A, B, C, D, E, F, G, H
    //but only add element to vector if a block really exists
    //in the column
    std::vector<int> mInCollisionMesh;

protected:
    std::vector<int> m_Blocks;

    void set_Blocks(std::vector<int> new_blocks);
};

#endif // COLUMNDEFINITION_H
