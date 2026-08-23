/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "basicstructs.h"
#include "thinglist.h"
#include "../memdump.h"
#include "../datatools.h"
#include <iostream>
#include <sstream>

ParseThingList::ParseThingList(MemDump* parentMemdump) {
   mParentMemDump = parentMemdump;
}

void ParseThingList::Print() {
}

void ParseThingList::Read(size_t dumpLevelStructStart) {
    //Read ThingList next Thing index
    size_t startOffThingList = dumpLevelStructStart + 0x17700;

    NextIndex = mParentMemDump->mDataTools->ConvertByteArray_ToInt32(startOffThingList + 0x7D0);

    //there are maximum 1000 things (max index is 999);
    //NextIndex points to the array index of the next thing
    //that will be created; Therefore we can calculate number of
    //currently existing things
    this->mNrThingsExisting = (999 - NextIndex) - 1;

    std::cout << "ThingList NextIndex = " << int(NextIndex) << std::endl;

    std::cout << "Current number of existing Things = " << int(mNrThingsExisting) << std::endl;

    //Read Thing Index table
    for (size_t currIdx = 0; currIdx < 1000; currIdx++) {
        Thing[currIdx] = mParentMemDump->mDataTools->ConvertByteArray_ToInt16(startOffThingList + 0x2 * currIdx);
    }
}

ParseThingList::~ParseThingList() {
}
