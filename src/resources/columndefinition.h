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
    ColumnDefinition(int id, int offset, std::vector<unsigned char> bytes);
    ~ColumnDefinition();

    unsigned char get_Shape();
    int get_FloorTextureID();
    int get_Unknown1();
    int get_Unknown2();
    int get_Unknown3();
    int get_A();
    int get_B();
    int get_C();
    int get_D();
    int get_E();
    int get_F();
    int get_G();
    int get_H();

    std::vector<int> get_Blocks();

    bool WriteChanges() override;

    //for level editor, write into data
    void set_Shape(unsigned char newVal);
    void set_FloorTextureID(int newVal);
    void set_Unknown1(int newVal);
    void set_Unknown2(int newVal);
    void set_Unknown3(int newVal);
    void set_A(int newVal);
    void set_B(int newVal);
    void set_C(int newVal);
    void set_D(int newVal);
    void set_E(int newVal);
    void set_F(int newVal);
    void set_G(int newVal);
    void set_H(int newVal);

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
