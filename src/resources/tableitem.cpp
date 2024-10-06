/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "tableitem.h"

TableItem::TableItem() {
}

TableItem::~TableItem() {

}

int TableItem::get_ID() {
    return(this->m_ID);
}

int TableItem::get_Offset() {
    return(this->m_Offset);
}

std::vector<unsigned char> TableItem::get_Bytes() {
    return(this->m_Bytes);
}

void TableItem::set_ID(int newID) {
    this->m_ID = newID;
}

void TableItem::set_Offset(int newOffset) {
    this->m_Offset = newOffset;
}

void TableItem::set_Bytes(std::vector<unsigned char> new_bytes) {
    this->m_Bytes = new_bytes;
}

bool TableItem::WriteChanges() {
     return(false);
}
