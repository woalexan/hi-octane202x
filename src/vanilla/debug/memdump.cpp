/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "memdump.h"
#include <algorithm>
#include <sstream>
#include "../../utils/logging.h"
#include "binaryfile.h"
#include "datatools.h"
#include "structs/thing.h"
#include "structs/thinglist.h"
#include "structs/thingvehicle.h"
#include "structs/control.h"
#include "structs/vvectors.h"
#include "structs/cam.h"
#include "structs/basicstructs.h"

MemDump::MemDump(std::string dumpFile)
{
   mMemDumpData = new BinaryFile(dumpFile);
   mDataTools = new DataTools(this);

   //Thing = new ParseThing(this);
   ThingList = new ParseThingList(this);
   Vectors = new ParseVectors(this);
   EngineCamera = new ParseCamera(this);

   for (size_t x = 0; x < 256; x++) {
       for (size_t y = 0; y < 160; y++) {
          MapElements[x][y] = nullptr;
       }
   }

   mExistingControlVec.clear();
   mExistingPlayers.clear();
   mExistingPlayerVehicleVec.clear();
}

MemDump::~MemDump() {
    delete mDataTools;

  //  delete Thing;
    delete ThingList;
    delete mMemDumpData;
    delete Vectors;
    delete EngineCamera;

    for (size_t x = 0; x < 256; x++) {
        for (size_t y = 0; y < 160; y++) {
           if (MapElements[x][y] != nullptr) {
               delete MapElements[x][y];
               MapElements[x][y] = nullptr;
           }
        }
    }

    std::vector<ParseControlClass*>::iterator it;
    ParseControlClass* pntr;
    for (it = mExistingControlVec.begin(); it != mExistingControlVec.end(); ) {
        pntr = (*it);
        it = mExistingControlVec.erase(it);

        delete pntr;
    }

    std::vector<ParseThingVehicle*>::iterator it2;
    ParseThingVehicle* pntrVehicle;
    for (it2 = mExistingPlayerVehicleVec.begin(); it2 != mExistingPlayerVehicleVec.end(); ) {
        pntrVehicle = (*it2);
        it2 = mExistingPlayerVehicleVec.erase(it2);

        delete pntrVehicle;
    }
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

ParseThing* MemDump::ReturnThingWithIndex(int16_t whichIndex) {
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

//Returns nullptr if specified player is not found
//playerNr starts with index 1 for the first player
ParseThing* MemDump::ReturnThingPlayer(uint8_t playerNr) {
    //is the playerNr valid? range is from 1 up to 8
    if ((playerNr < 1) || (playerNr > 8)) {
        return nullptr;
    }

    std::vector<ParseThing*> allVehicles = ReturnThingsWithGroup(10);
    std::vector<ParseThing*>::iterator it;

    ParseThing* result = nullptr;

    //repair vehicles also have grp10, sort them out by looking at the
    //Member variable value in all of the returned things
    //Repair vehicles have a member value of 9
    //Id of Thing does carry the the player number
    for (it = allVehicles.begin(); it != allVehicles.end(); ++it) {
        if (((*it)->Member->GetRawValue() != 9) && ((*it)->Id->mRawValue == (uint8_t)(playerNr))) {
            //we found the correct player
            result = (*it);
            break;
        }
    }

    return (result);
}

//Returns nullptr if the control for specified player is not found
//playerNr starts with index 1 for the first player
ParseControlClass* MemDump::ReturnControlPlayer(uint8_t playerNr, size_t dumpLevelStructStart) {
    ParseControlClass* result = nullptr;

    //first try to get the player Thing
    ParseThing* playerThing = ReturnThingPlayer(playerNr);

    if (playerThing == nullptr) {
        //We did not find the Thing for this player
        return nullptr;
    }

    int16_t playerId = playerThing->Id->mRawValue;

    //The playerId selects which Control we need to parse
    size_t startAdr = dumpLevelStructStart + 0x22EDC + (playerId - 1) * 0x980;
    result = new ParseControlClass(this);
    result->Update(startAdr);

    return result;
}

void MemDump::ReadAllPlayers(size_t dumpLevelStructStart) {
    ParseThing* playerThingPntr;
    ParseControlClass* controlPntr;
    ParseThingVehicle* vehiclePntr;

    for (uint8_t playerNr = 1; playerNr < 9; playerNr++) {
        playerThingPntr = ReturnThingPlayer(playerNr);

        //does this player exist in this game?
        if (playerThingPntr != nullptr) {
            mExistingPlayers.push_back(playerThingPntr);

            //also get the vehicle struct for this player
            vehiclePntr = new ParseThingVehicle(this);
            vehiclePntr->Update(dumpLevelStructStart + 0x40B3C + playerThingPntr->VehicleIndex->mRawValue * 0x1F0);
            mExistingPlayerVehicleVec.push_back(vehiclePntr);

            //also get the control for this player
            controlPntr = ReturnControlPlayer(playerNr, dumpLevelStructStart);

            if (controlPntr != nullptr) {
                mExistingControlVec.push_back(controlPntr);
            }
        }
    }
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

void MemDump::ReadAllMapElements(size_t dumpLevelStructStart) {
    std::string name("");
    std::stringstream sstream;
    size_t currPos = 0x5A844 + dumpLevelStructStart;

    for (size_t y = 0; y < 160; y++) {
         for (size_t x = 0; x < 256; x++) {
           sstream << "X: " << (int)(x) << " Y: " << (int)(y);
           name = sstream.str();
           MapElements[x][y] = new MapElementClass(mDataTools, name, currPos);
           currPos += 0xC;
        }
    }
}

