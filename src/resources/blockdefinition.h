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
    BlockDefinition(int id, int offset, std::vector<unsigned char> bytes);
    ~BlockDefinition();

    bool WriteChanges() override;

    int get_N();
    int get_E();
    int get_S();
    int get_W();
    int get_T();
    int get_B();

    int NMod();
    int EMod();
    int SMod();
    int WMod();
    int TMod();
    int BMod();

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
};

#endif // BLOCKDEFINITION_H
