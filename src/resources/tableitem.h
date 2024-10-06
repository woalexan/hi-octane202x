/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef TABLEITEM_H
#define TABLEITEM_H

#include <vector>

class TableItem {
public:
    TableItem();
    virtual ~TableItem();

    int get_ID();
    int get_Offset();
    std::vector<unsigned char> get_Bytes();

    virtual bool WriteChanges();

 protected:
    int m_ID;
    int m_Offset;

    std::vector<unsigned char> m_Bytes;

    //for debugging of level saving, comment out later
    std::vector<unsigned char> m_wBytes;

    void set_ID(int newID);
    void set_Offset(int newOffset);
    void set_Bytes(std::vector<unsigned char> new_bytes);
};

#endif // TABLEITEM_H
