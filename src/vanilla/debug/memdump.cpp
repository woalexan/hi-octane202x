/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "memdump.h"
#include <algorithm>
#include "../../utils/logging.h"
#include "binaryfile.h"
#include "datatools.h"
#include "structs/thing.h"
#include "structs/thinglist.h"
#include "structs/thingvehicle.h"
#include "structs/vvectors.h"
#include "structs/cam.h"
#include "structs/basicstructs.h"

MemDump::MemDump(std::string dumpFile)
{
   mMemDumpData = new BinaryFile(dumpFile);
   mDataTools = new DataTools(this);

   //Thing = new ParseThing(this);
   ThingList = new ParseThingList(this);
   ThingVehicle = new ParseThingVehicle(this);
   Vectors = new ParseVectors(this);
   EngineCamera = new ParseCamera(this);
}

MemDump::~MemDump() {
    delete mDataTools;

  //  delete Thing;
    delete ThingList;
    delete ThingVehicle;
    delete mMemDumpData;
    delete Vectors;
    delete EngineCamera;
}

void MemDump::ReadVectors(size_t dumpLevelStructStart) {
    Vectors->Update(dumpLevelStructStart + 0x4E5AC);
}

void MemDump::ReadThingList(size_t dumpLevelStructStart) {
    ThingList->Read(dumpLevelStructStart);
}

void MemDump::ReadAllThings(size_t dumpLevelStructStart) {
    mExistingThingsVec.clear();

    //Read all non-zero index Things
    size_t startOffThings = dumpLevelStructStart + 0x2849C;
    size_t idxOff;
    int16_t idxValue;

    for (size_t currIdx = 0; currIdx < 1000; currIdx++) {
        //read the Group, if the group is nonzero then the
        //thing is currently active
        idxOff = startOffThings + currIdx * 0x64 + 0x62;
        idxValue = mDataTools->ConvertByteArray_ToInt8(idxOff);

        if (idxValue != 0) {
            ParseThing* newThing = new ParseThing(this);
            newThing->Update(startOffThings + currIdx * 0x64);
            mExistingThingsVec.push_back(newThing);
        }
    }
}

std::vector<ParseThing*> MemDump::ReturnThingsWithGroup(int8_t whichGroup) {
    std::vector<ParseThing*>::iterator it;
    std::vector<ParseThing*> result;
    result.clear();

    for (it = mExistingThingsVec.begin(); it != mExistingThingsVec.end(); ++it) {
        if ((*it)->Group->GetRawValue() == whichGroup) {
            result.push_back(*it);
        }
    }

    return result;
}

ParseThing* MemDump::ReturnThingsWithIndex(int16_t whichIndex) {
    std::vector<ParseThing*>::iterator it;
    ParseThing* result = nullptr;

    for (it = mExistingThingsVec.begin(); it != mExistingThingsVec.end(); ++it) {
        if ((*it)->Index->mRawValue == whichIndex) {
            result = (*it);
            break;
        }
    }

    return result;
}

//Returns null if first player is not found
ParseThing* MemDump::ReturnThingFirstPlayer() {
    std::vector<ParseThing*> allVehicles = ReturnThingsWithGroup(10);
    std::vector<ParseThing*>::iterator it;

    ParseThing* result = nullptr;

    //repair vehicles also have grp10, sort them out by looking at the
    //Member variable value in all of the returned things
    //Repair vehicles have a member value of 9
    for (it = allVehicles.begin(); it != allVehicles.end(); ++it) {
        if ((*it)->Member->GetRawValue() != 9) {
            //we found a player, which is not a repair vehicle
            result = (*it);
            break;
        }
    }

    return (result);
}

void MemDump::ReadEngineCamera() {
  //this camera seems always to be located at 0x11EEF8
  EngineCamera->Update(0x11EEF8);
}

bool MemDump::FindDataInDump(std::vector<uint8_t> searchPattern, std::vector<size_t> &atOffset) {
    if (mMemDumpData == nullptr) {
        return false;
    }

    if (mMemDumpData->mData == nullptr) {
        return false;
    }

    std::vector<size_t> result;
    result.clear();

    std::vector<uint8_t>::iterator fndLoc;
    bool continueSearch = true;
    std::vector<uint8_t>::iterator currSearchStart = mMemDumpData->mData->begin();

    while (continueSearch) {
        fndLoc = std::search(currSearchStart, mMemDumpData->mData->end(), searchPattern.begin(), searchPattern.end());

        if (fndLoc == mMemDumpData->mData->end()) {
            //not found anymore
            continueSearch = false;
        } else {
            size_t currOfs = std::distance(mMemDumpData->mData->begin(), fndLoc);
            result.push_back(currOfs);
            currSearchStart = mMemDumpData->mData->begin() + currOfs + 1;
            if (currSearchStart == mMemDumpData->mData->end()) {
                continueSearch = false;
            }
        }
    }

    atOffset = result;

    if (result.size() > 0) {
        return true;
    } else {
        return false;
    }
}

