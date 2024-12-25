/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef BLOCKDEFINITION_H
#define BLOCKDEFINITION_H

#include "tableitem.h"
#include <vector>
#include <cstdint>

class BlockDefinition : public TableItem {
public:
    BlockDefinition(int id, int offset, std::vector<uint8_t> bytes);
    ~BlockDefinition();

    bool WriteChanges() override;

    uint8_t get_N();
    uint8_t get_E();
    uint8_t get_S();
    uint8_t get_W();
    uint8_t get_T();
    uint8_t get_B();

    uint8_t NMod();
    uint8_t EMod();
    uint8_t SMod();
    uint8_t WMod();
    uint8_t TMod();
    uint8_t BMod();

    //for the levelEditor functionality
    void set_N(int newVal);
    void set_E(int newVal);
    void set_S(int newVal);
    void set_W(int newVal);
    void set_T(int newVal);
    void set_B(int newVal);

    void setNMod(int newVal);
    void setEMod(int newVal);
    void setSMod(int newVal);
    void setWMod(int newVal);
    void setTMod(int newVal);
    void setBMod(int newVal);

private:
    uint8_t mN;
    uint8_t mE;
    uint8_t mS;
    uint8_t mW;
    uint8_t mT;
    uint8_t mB;

    uint8_t mNMod;
    uint8_t mEMod;
    uint8_t mSMod;
    uint8_t mWMod;
    uint8_t mTMod;
    uint8_t mBMod;

    uint8_t decode_N();
    uint8_t decode_E();
    uint8_t decode_S();
    uint8_t decode_W();
    uint8_t decode_T();
    uint8_t decode_B();

    uint8_t decode_NMod();
    uint8_t decode_EMod();
    uint8_t decode_SMod();
    uint8_t decode_WMod();
    uint8_t decode_TMod();
    uint8_t decode_BMod();

    void save_N(int newVal);
    void save_E(int newVal);
    void save_S(int newVal);
    void save_W(int newVal);
    void save_T(int newVal);
    void save_B(int newVal);

    void save_NMod(int newVal);
    void save_EMod(int newVal);
    void save_SMod(int newVal);
    void save_WMod(int newVal);
    void save_TMod(int newVal);
    void save_BMod(int newVal);

};

#endif // BLOCKDEFINITION_H
