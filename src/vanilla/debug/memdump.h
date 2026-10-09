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
class ParseControlClass;
class MapElementClass;

class MemDump
{
public:
    MemDump(std::string dumpFile);
    ~MemDump();

    bool FindDataInDump(std::vector<uint8_t> searchPattern, std::vector<size_t> &atOffset);
    void ReadThingList(size_t dumpLevelStructStart);
    void ReadAllPlayers(size_t dumpLevelStructStart);

    void ReadAllThings(size_t dumpLevelStructStart);
    std::vector<ParseThing*> ReturnThingsWithGroup(int8_t whichGroup);
    ParseThing* ReturnThingWithIndex(int16_t whichIndex);

    //Returns nullptr if specified player is not found
    //playerNr starts with index 1 for the first player
    ParseThing* ReturnThingPlayer(uint8_t playerNr);

    //Returns nullptr if the control for specified player is not found
    //playerNr starts with index 1 for the first player
    ParseControlClass* ReturnControlPlayer(uint8_t playerNr, size_t dumpLevelStructStart);

    BinaryFile* mMemDumpData = nullptr;
    DataTools* mDataTools = nullptr;

    ParseThingList* ThingList = nullptr;
    //ParseThing* Thing = nullptr;
    ParseVectors* Vectors = nullptr;

    void ReadVectors(size_t dumpLevelStructStart);

    std::vector<ParseThing*> mExistingThingsVec;

    //for the three vectors below, the matching Player Thing, Control Struct
    //and Vehicle struct for a player are always at the same index of the vectors
    std::vector<ParseThing*> mExistingPlayers;
    std::vector<ParseControlClass*> mExistingControlVec;
    //the vector below only contains the player vehicles, does not contain
    //the repair vehicles
    std::vector<ParseThingVehicle*> mExistingPlayerVehicleVec;

    ParseCamera* EngineCamera = nullptr;
    void ReadEngineCamera();

    void ReadAllMapElements(size_t dumpLevelStructStart);

    MapElementClass* MapElements[256][160];
};

#endif // MEMDUMP_H
