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
#include <cstdint>

#define DEF_COLUMNDEF_STATE_DEFAULT 0
#define DEF_COLUMNDEF_STATE_NEWLYADDED_KEEP 1
#define DEF_COLUMNDEF_STATE_NEWLYUNASSIGNEDONE 2

class ColumnDefinition : public TableItem {
public:
    ColumnDefinition(int id, int offset, std::vector<uint8_t> bytes);
    ~ColumnDefinition();

    //Alternative constructor used for the level editor
    //if offset is set to -1 (for example for special column for block preview) writing to
    //map file data vector is disabled.
    ColumnDefinition(int id, int offset, int newFloorTextureID, int16_t mUnknown1, int16_t newA, int16_t newB, int16_t newC, int16_t newD, int16_t newE,
                     int16_t newF, int16_t newG, int16_t newH, int16_t newOccurence);

    uint8_t get_Shape();
    int16_t get_FloorTextureID();
    int16_t get_Unknown1();
    int16_t get_Occurence();
    int16_t get_Unknown3();
    uint8_t get_Unknown4();
    int16_t get_A();
    int16_t get_B();
    int16_t get_C();
    int16_t get_D();
    int16_t get_E();
    int16_t get_F();
    int16_t get_G();
    int16_t get_H();

    bool WriteChanges() override;

    //for level editor, write into data
    void set_Shape(uint8_t newVal);
    void set_FloorTextureID(int16_t newVal);
    void set_Unknown1(int16_t newVal);
    void set_Occurence(int16_t newVal);
    void set_Unknown3(int16_t newVal);
    void set_Unknown4(uint8_t newVal);
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

    /*****************************************
     * LevelEditor only used variables start *
     *****************************************/

    //while column definition changes
    int m_initialID;

    //this columndefinition state variable is
    //only used for the leveleditor
    uint8_t mState = DEF_COLUMNDEF_STATE_DEFAULT;

    /*****************************************
     * LevelEditor only used variables end   *
     *****************************************/

private:
    int16_t mA;
    int16_t mB;
    int16_t mC;
    int16_t mD;
    int16_t mE;
    int16_t mF;
    int16_t mG;
    int16_t mH;

    int16_t mUnknown1;
    int16_t mOccurence;
    int16_t mUnknown3;
    uint8_t mUnknown4;

    int16_t mFloorTextureID;
    uint8_t mShape;

    uint8_t decode_Shape();
    int16_t decode_FloorTextureID();
    int16_t decode_Unknown1();
    int16_t decode_Occurence();
    int16_t decode_Unknown3();
    int16_t decode_A();
    int16_t decode_B();
    int16_t decode_C();
    int16_t decode_D();
    int16_t decode_E();
    int16_t decode_F();
    int16_t decode_G();
    int16_t decode_H();
};

#endif // COLUMNDEFINITION_H
