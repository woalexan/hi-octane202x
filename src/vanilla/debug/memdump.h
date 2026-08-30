/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MEMDUMP_H
#define MEMDUMP_H

#include <vector>
#include <stdlib.h>
#include <string>
#include <cstdint>
#include "irrlicht.h"

class BinaryFile;
class DataTools;
class ParseThingList;
class ParseThing;
class ParseThingVehicle;
class ParseVectors;
class ParseCamera;
class MapElementClass;

class MemDump
{
public:
    MemDump(std::string dumpFile);
    ~MemDump();

    bool FindDataInDump(std::vector<uint8_t> searchPattern, std::vector<size_t> &atOffset);
    void ReadThingList(size_t dumpLevelStructStart);

    void ReadAllThings(size_t dumpLevelStructStart);
    std::vector<ParseThing*> ReturnThingsWithGroup(int8_t whichGroup);
    ParseThing* ReturnThingsWithIndex(int16_t whichIndex);

    //Returns null if first player is not found
    ParseThing* ReturnThingFirstPlayer();

    BinaryFile* mMemDumpData = nullptr;
    DataTools* mDataTools = nullptr;

    ParseThingList* ThingList = nullptr;
    //ParseThing* Thing = nullptr;
    ParseVectors* Vectors = nullptr;

    void ReadVectors(size_t dumpLevelStructStart);

    ParseThingVehicle* ThingVehicle = nullptr;
    std::vector<ParseThing*> mExistingThingsVec;

    ParseCamera* EngineCamera = nullptr;
    void ReadEngineCamera();

    void ReadAllMapElements(size_t dumpLevelStructStart);

    MapElementClass* MapElements[256][160];
};

#endif // MEMDUMP_H
