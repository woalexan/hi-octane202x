/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "dbginterface.h"
#include "memdump.h"
#include "binaryfile.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../utils/logging.h"
#include "structs/thing.h"
#include "structs/thingvehicle.h"
#include "structs/basicstructs.h"
#include "structs/vvectors.h"
#include "datatools.h"

void DbgInterface::WriteDataToBinaryFile(std::string fileName, std::vector<uint8_t> data) {
    const std::vector<uint8_t> buffer(data);
    std::ofstream outFile(fileName, std::ios::out | std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    outFile.close();
}

std::vector<DiffByte> DbgInterface::CompareData(std::vector<uint8_t> data1, std::vector<uint8_t> data2) {
    size_t compareLen = data1.size();
    if (data2.size() < compareLen) {
        compareLen = data2.size();
    }

    std::vector<DiffByte> result;
    result.clear();

    //compare compareLen bytes
    size_t currIdx;

    for (currIdx = 0; currIdx < compareLen; currIdx++) {
        if (data1.at(currIdx) != data2.at(currIdx)) {
            //a new difference
            DiffByte diffByteFnd;
            diffByteFnd.atOffset = currIdx;
            diffByteFnd.valData1 = data1.at(currIdx);
            diffByteFnd.valData2 = data2.at(currIdx);
            result.push_back(diffByteFnd);
        }
    }

    return result;
}

void DbgInterface::CompareTable(std::string tableName, std::vector<uint8_t> table1, std::vector<uint8_t> table2) {
    std::vector<DiffByte> compareResult = CompareData(table1, table2);

    std::string infoMsg(tableName);
    char str[256];

    snprintf(str, sizeof str, ": %zu", compareResult.size());
    infoMsg.append(str);
    infoMsg.append(" different bytes found");

    logging::Info(infoMsg);
}

void DbgInterface::Init(std::string memDumpFileName, std::string memDumpFileName2, std::string levelFileName) {
    newDump = new MemDump(memDumpFileName);
    if (!memDumpFileName2.empty()) {
        newDump2 = new MemDump(memDumpFileName2);
    }

    levelData = new BinaryFile(levelFileName);

    std::vector<uint8_t> searchPattern = levelData->GetDataRange(0, 31);
    std::vector<size_t> fndLoc;

    bool fnd = newDump->FindDataInDump(searchPattern, fndLoc);
    if (!fnd) {
        logging::Info("Not found");
    } else {
        std::string infoMsg("Level start found at ");

        char str[256];

        mDumpLevelStructStart = 0;

        std::vector<size_t>::iterator it;
        for (it = fndLoc.begin(); it != fndLoc.end(); ++it) {
            snprintf(str, sizeof str, "%06zx ", (*it));
            infoMsg.append(str);
            mDumpLevelStructStart = (*it);
        }

        logging::Info(infoMsg);

        //ParseAllThings();

        newDump->ReadThingList(mDumpLevelStructStart);
        newDump->ReadAllThings(mDumpLevelStructStart);
        newDump->ReadVectors(mDumpLevelStructStart);

        if (newDump2 != nullptr) {
            newDump2->ReadThingList(mDumpLevelStructStart);
            newDump2->ReadAllThings(mDumpLevelStructStart);
            newDump2->ReadVectors(mDumpLevelStructStart);
        }

        //std::vector<ParseThing*> vehicles = newDump->ReturnThingsWithGroup(10);

        //newDump->Thing->Update(mDumpLevelStructStart + 0x2849C + 865 * 0x64);
        //newDump->Thing->Print();

       // newDump->ThingVehicle->Update(mDumpLevelStructStart + 0x40B3C + 5 * 0x1F0);

       // newDump->ThingVehicle->Print();
    }
}

void DbgInterface::CopyMovementClassToMovementStruct(MovementStruct& targetStruct, MovementClass* srcClass) {
    targetStruct.AngleXY = srcClass->AngleXY->mFloatValue;
    targetStruct.AngleXZ = srcClass->AngleXZ->mFloatValue;
    targetStruct.AngleZY = srcClass->AngleZY->mFloatValue;
    targetStruct.SpeedActual = srcClass->SpeedActual->mFloatValue;
}

void DbgInterface::CompareTracksColVect(std::string prefixName, TrackColVectStruct& compareStruct, ParseColVectClass* compareClass) {
    std::string name1(prefixName);
    name1.append(".Pos1.X");
    CompareTwoInt16s(name1, static_cast<int16_t>(compareStruct.pos1X), compareClass->Pos1->XPos->mRawValue);

    name1 = prefixName;
    name1.append(".Pos1.Y");
    CompareTwoFloats(name1, static_cast<int16_t>(compareStruct.pos1Y), compareClass->Pos1->YPos->mRawValue);

    name1 = prefixName;
    name1.append(".Pos1.Z");
    CompareTwoFloats(name1, static_cast<int16_t>(compareStruct.pos1Z), compareClass->Pos1->ZPos->mRawValue);

    name1 = prefixName;
    name1.append(".Pos2.X");
    CompareTwoFloats(name1, static_cast<int16_t>(compareStruct.pos2X), compareClass->Pos2->XPos->mRawValue);

    name1 = prefixName;
    name1.append(".Pos2.Y");
    CompareTwoFloats(name1, static_cast<int16_t>(compareStruct.pos2Y), compareClass->Pos2->YPos->mRawValue);

    name1 = prefixName;
    name1.append(".Pos2.Z");
    CompareTwoFloats(name1, static_cast<int16_t>(compareStruct.pos2Z), compareClass->Pos2->ZPos->mRawValue);

    name1 = prefixName;
    name1.append(".Angle");
    CompareTwoFloats(name1, compareStruct.Angle, compareClass->Angle->mFloatValue);
}

void DbgInterface::CompareTracksColVectList(std::string prefixName, TrackColVectListStruct& compareStruct, ParseColVectsListClass* compareClass) {
    std::string extPrefixName(prefixName);
    extPrefixName.append(".Vect");
    CompareTwoInt16s(extPrefixName, compareStruct.Vect, static_cast<int16_t>(compareClass->Vect->mRawValue));

    extPrefixName = prefixName;
    extPrefixName.append(".NextColList");
    CompareTwoInt16s(extPrefixName, compareStruct.NextColList, static_cast<int16_t>(compareClass->NextColList->mRawValue));
}

void DbgInterface::CompareVectorsWithMemDump(VTrack& compareVector1, ParseVectors* compareVector2) {
    CompareTwoUInt16s(std::string("NextColVect"), static_cast<uint16_t>(compareVector1.NextColVect), compareVector2->NextColVect->mRawValue);
    CompareTwoUInt16s(std::string("NextVectsList"), static_cast<uint16_t>(compareVector1.NextVectsList), compareVector2->NextVectsList->mRawValue);

    CompareTwoInt16s(std::string("TrackCollisionVector.X"), static_cast<int16_t>(compareVector1.TrackCollisionVectorX), compareVector2->TrackCollisionVector->XPos->mRawValue);
    CompareTwoInt16s(std::string("TrackCollisionVector.Y"), static_cast<int16_t>(compareVector1.TrackCollisionVectorY), compareVector2->TrackCollisionVector->YPos->mRawValue);

    CompareTwoFloats(std::string("TrackCollisionVectorAngle"), compareVector1.TrackCollisionVectorAngle, compareVector2->TrackCollisionVectorAngle->mFloatValue);

    size_t nrVects = static_cast<size_t>(compareVector1.NextColVect - 1);
    size_t nrVects2 = static_cast<size_t>(compareVector2->NextColVect->mRawValue - 1);

    if (nrVects2 < nrVects) {
        nrVects = nrVects2;
    }

    if (nrVects > 250) {
        nrVects = 250;
    }

    for (size_t idx = 1; idx < nrVects; idx++) {
        std::ostringstream output;

        output << "ColVect @" << std::dec << idx;

        CompareTracksColVect(std::string(output.str()), compareVector1.ColVects[idx], compareVector2->ColVects[idx]);
    }

    size_t nrVectList = static_cast<size_t>(compareVector1.NextVectsList - 1);
    size_t nrVectList2 = static_cast<size_t>(compareVector2->NextVectsList->mRawValue - 1);

    if (nrVectList2 < nrVectList) {
        nrVectList = nrVectList2;
    }

    if (nrVectList > 10000) {
        nrVectList = 10000;
    }

    for (size_t idx = 1; idx < nrVectList; idx++) {
        std::ostringstream output;

        output << "ColVectsList @" << std::dec << int(idx);

        CompareTracksColVectList(std::string(output.str()), compareVector1.ColVectsList[idx], compareVector2->ColVectsList[idx]);
    }
}

void DbgInterface::CompareMovementData(std::string prefixName, MovementStruct* compareStruct, MovementClass* compareClass) {
    std::string name1(prefixName);
    name1.append(".AngleXY");
    CompareTwoFloats(name1, compareStruct->AngleXY, compareClass->AngleXY->mFloatValue);

    name1 = prefixName;
    name1.append(".AngleXZ");
    CompareTwoFloats(name1, compareStruct->AngleXZ, compareClass->AngleXZ->mFloatValue);

    name1 = prefixName;
    name1.append(".AngleZY");
    CompareTwoFloats(name1, compareStruct->AngleZY, compareClass->AngleZY->mFloatValue);

    name1 = prefixName;
    name1.append(".SpeedActual");
    CompareTwoFloats(name1, compareStruct->SpeedActual, compareClass->SpeedActual->mFloatValue);
}

void DbgInterface::CompareMovementDataBetweenMemDumps(std::string prefixName, MovementClass* compareClass1, MovementClass* compareClass2) {
    std::string name1(prefixName);
    name1.append(".AngleXY");
    CompareTwoFloats(name1, compareClass1->AngleXY->mFloatValue, compareClass2->AngleXY->mFloatValue);

    name1 = prefixName;
    name1.append(".AngleXZ");
    CompareTwoFloats(name1, compareClass1->AngleXZ->mFloatValue, compareClass2->AngleXZ->mFloatValue);

    name1 = prefixName;
    name1.append(".AngleZY");
    CompareTwoFloats(name1, compareClass1->AngleZY->mFloatValue, compareClass2->AngleZY->mFloatValue);

    name1 = prefixName;
    name1.append(".SpeedActual");
    CompareTwoFloats(name1, compareClass1->SpeedActual->mFloatValue, compareClass2->SpeedActual->mFloatValue);
}

void DbgInterface::CompareSensorPointData(std::string prefixName, VehicleSensorPointStruct* compareStruct, SensorPointClass* compareClass) {
    std::string name1(prefixName);
    name1.append(".Position.X");
    CompareTwoFloats(name1, compareStruct->Position.X, compareClass->Position->XPos->mFloatValue);

    name1 = prefixName;
    name1.append(".Position.Y");
    CompareTwoFloats(name1, compareStruct->Position.Y, compareClass->Position->YPos->mFloatValue);

    name1 = prefixName;
    name1.append(".Position.Z");
    CompareTwoFloats(name1, compareStruct->Position.Z, compareClass->Position->ZPos->mFloatValue);

    name1 = prefixName;
    name1.append(".Zpos");
    CompareTwoFloats(name1, compareStruct->Zpos, compareClass->Zpos->mFloatValue);

    name1 = prefixName;
    name1.append(".ZposDiff");
    CompareTwoFloats(name1, compareStruct->ZposDiff, compareClass->ZposDiff->mFloatValue);

    name1 = prefixName;
    name1.append(".ZposDisplacement");
    CompareTwoFloats(name1, compareStruct->ZposDisplacement, compareClass->ZposDisplacement->mFloatValue);

    name1 = prefixName;
    name1.append(".ZposFloor");
    CompareTwoFloats(name1, compareStruct->ZposFloor, compareClass->ZposFloor->mFloatValue);

    name1 = prefixName;
    name1.append(".CollideFlags");
    CompareTwoInt16s(name1, (int16_t)(compareStruct->CollideFlags), (int16_t)(compareClass->CollideFlags->mRawValue));

    irr::f32 compValueRebound = (irr::f32)(compareClass->Rebound->mRawValue) / 256.0f;
    name1 = prefixName;
    name1.append(".Rebound");
    CompareTwoFloats(name1, compareStruct->Rebound, compValueRebound);

    irr::f32 compValueReboundLimit = (irr::f32)(compareClass->ReboundLimit->mRawValue) / 256.0f;
    name1 = prefixName;
    name1.append(".ReboundLimit");
    CompareTwoFloats(name1, compareStruct->ReboundLimit, compValueReboundLimit);
}

void DbgInterface::CompareSensorPointDataBetweenTwoMemDumpts(std::string prefixName, SensorPointClass* compareClass1, SensorPointClass* compareClass2) {
    std::string name1(prefixName);
    name1.append(".Position.X");
    CompareTwoFloats(name1, compareClass1->Position->XPos->mFloatValue, compareClass2->Position->XPos->mFloatValue);

    name1 = prefixName;
    name1.append(".Position.Y");
    CompareTwoFloats(name1, compareClass1->Position->YPos->mFloatValue, compareClass2->Position->YPos->mFloatValue);

    name1 = prefixName;
    name1.append(".Position.Z");
    CompareTwoFloats(name1, compareClass1->Position->ZPos->mFloatValue, compareClass2->Position->ZPos->mFloatValue);

    name1 = prefixName;
    name1.append(".Zpos");
    CompareTwoFloats(name1, compareClass1->Zpos->mFloatValue, compareClass2->Zpos->mFloatValue);

    name1 = prefixName;
    name1.append(".ZposDiff");
    CompareTwoFloats(name1, compareClass1->ZposDiff->mFloatValue, compareClass2->ZposDiff->mFloatValue);

    name1 = prefixName;
    name1.append(".ZposDisplacement");
    CompareTwoFloats(name1, compareClass1->ZposDisplacement->mFloatValue, compareClass2->ZposDisplacement->mFloatValue);

    name1 = prefixName;
    name1.append(".ZposFloor");
    CompareTwoFloats(name1, compareClass1->ZposFloor->mFloatValue, compareClass2->ZposFloor->mFloatValue);

    name1 = prefixName;
    name1.append(".CollideFlags");
    CompareTwoInt16s(name1, (int16_t)(compareClass1->CollideFlags->mRawValue), (int16_t)(compareClass2->CollideFlags->mRawValue));

    irr::f32 compValueRebound2 = (irr::f32)(compareClass2->Rebound->mRawValue) / 256.0f;
    irr::f32 compValueRebound1 = (irr::f32)(compareClass1->Rebound->mRawValue) / 256.0f;
    name1 = prefixName;
    name1.append(".Rebound");
    CompareTwoFloats(name1, compValueRebound1, compValueRebound2);

    irr::f32 compValueReboundLimit1 = (irr::f32)(compareClass1->ReboundLimit->mRawValue) / 256.0f;
    irr::f32 compValueReboundLimit2 = (irr::f32)(compareClass2->ReboundLimit->mRawValue) / 256.0f;
    name1 = prefixName;
    name1.append(".ReboundLimit");
    CompareTwoFloats(name1, compValueReboundLimit1, compValueReboundLimit2);
}

void DbgInterface::CopySensorPointClassToVehicleSensorPointStruct(VehicleSensorPointStruct& targetStruct, SensorPointClass* srcClass) {
    targetStruct.Position.X = srcClass->Position->XPos->mFloatValue;
    targetStruct.Position.Y = srcClass->Position->YPos->mFloatValue;
    targetStruct.Position.Z = srcClass->Position->ZPos->mFloatValue;
    targetStruct.Zpos = srcClass->Zpos->mFloatValue;
    targetStruct.ZposDiff = srcClass->ZposDiff->mFloatValue;
    targetStruct.ZposDisplacement = srcClass->ZposDisplacement->mFloatValue;
    targetStruct.ZposFloor = srcClass->ZposFloor->mFloatValue;
    targetStruct.CollideFlags = (int8_t)(srcClass->CollideFlags->mRawValue);
    targetStruct.Rebound = (irr::f32)(srcClass->Rebound->mRawValue) / 256.0f;
    targetStruct.ReboundLimit = (irr::f32)(srcClass->ReboundLimit->mRawValue) / 256.0f;
}

void DbgInterface::CompareTwoFloats(std::string varName, irr::f32 val1, irr::f32 val2) {
    bool output = false;
    irr::f32 diff = (val2 - val1);

    if ((fabs(val1) < 0.001953125) && (fabs(diff) > 0.001953125)) {
        output = true;
    }

    if (!output) {
        irr::f32 relPercDiff = diff / (val1 / 100.0f);
        if (fabs(relPercDiff) > 0.0005f) {
            //a relative change of for then 5 percent
            output = true;
        }
    }

    if (output) {
        std::ostringstream outputMsg;
        outputMsg << varName << ": " << val1 << " => " << val2;
        logging::Info(outputMsg.str());
    }
}

void DbgInterface::CompareTwoInt16s(std::string varName, int16_t val1, int16_t val2) {
    if (val1 != val2) {
        std::ostringstream outputMsg;

        std::string decStr1 = std::to_string(val1);
        std::string decStr2 = std::to_string(val2);

        outputMsg << varName << ": Val1 = 0x" << std::setfill('0') << std::setw(4) <<
                     std::right << std::uppercase << std::hex << int(val1) << " (Dec:" << std::dec << decStr1 << ") / "
               << "Val2 = 0x" << std::setfill('0') << std::setw(4) <<
                                      std::right << std::uppercase << std::hex << int(val2) << " (Dec:" << std::dec << decStr2 << ")";

        logging::Info(outputMsg.str());
    }
}

void DbgInterface::CompareTwoUInt16s(std::string varName, uint16_t val1, uint16_t val2) {
    if (val1 != val2) {
        std::ostringstream outputMsg;

        std::string decStr1 = std::to_string(val1);
        std::string decStr2 = std::to_string(val2);

        outputMsg << varName << ": Val1 = 0x" << std::setfill('0') << std::setw(4) <<
                     std::right << std::uppercase << std::hex << int(val1) << " (Dec:" << std::dec << decStr1 << ") / "
               << "Val2 = 0x" << std::setfill('0') << std::setw(4) <<
                                      std::right << std::uppercase << std::hex << int(val2) << " (Dec:" << std::dec << decStr2 << ")";

        logging::Info(outputMsg.str());
    }
}

void DbgInterface::CompareVehicleStatePlayerWithMemDump(VVehicle& compareVehicle, MemDump* compareDump) {
    ParseThing* playerThing = compareDump->ReturnThingFirstPlayer();

    if (playerThing == nullptr) {
        return;
    }

    //VehicleIndex in Thing tells us the index of the VehicleThing in the array of
    //possible VehicleThings that is linked to this specific vehicle;
    //parse its data
    compareDump->ThingVehicle->Update(mDumpLevelStructStart + 0x40B3C + playerThing->VehicleIndex->mRawValue * 0x1F0);

    //Now compare values of internal vehicle with vehicle from memory dump of original game
    CompareTwoFloats(std::string("ThingData.Position.X"), compareVehicle.ThingData.Position.X, playerThing->Position->XPos->mFloatValue);
    CompareTwoFloats(std::string("ThingData.Position.Y"), compareVehicle.ThingData.Position.Y, playerThing->Position->YPos->mFloatValue);
    CompareTwoFloats(std::string("ThingData.Position.Z"), compareVehicle.ThingData.Position.Z, playerThing->Position->ZPos->mFloatValue);

    CompareTwoFloats(std::string("ThingData.Displacement.X"), compareVehicle.ThingData.Displacement.X, playerThing->Displacement->XPos->mFloatValue);
    CompareTwoFloats(std::string("ThingData.Displacement.Y"), compareVehicle.ThingData.Displacement.Y, playerThing->Displacement->YPos->mFloatValue);
    CompareTwoFloats(std::string("ThingData.Displacement.Z"), compareVehicle.ThingData.Displacement.Z, playerThing->Displacement->ZPos->mFloatValue);

    CompareTwoInt16s(std::string("ThingData.Life"), compareVehicle.ThingData.Life, playerThing->Life->mRawValue);

    CompareMovementData(std::string("ThingData.Movement."), &compareVehicle.ThingData.Movement, playerThing->Movement);

    CompareTwoInt16s(std::string("ThingData.TimeSlice"), static_cast<int16_t>(static_cast<int8_t>(compareVehicle.ThingData.mTimeSlice)), static_cast<int16_t>(playerThing->TimeSlice->mRawValue));

    //targetVehicle.KeyPressedAccel
    //targetVehicle.KeyPressedDeaccel
    //targetVehicle.KeyPressedTurnLeft
    //targetVehicle.KeyPressedTurnRight

    CompareTwoFloats(std::string("Displacement.X"), compareVehicle.Displacement.X, compareDump->ThingVehicle->VehicleDisplacement->XPos->mFloatValue);
    CompareTwoFloats(std::string("Displacement.Y"), compareVehicle.Displacement.Y, compareDump->ThingVehicle->VehicleDisplacement->YPos->mFloatValue);
    CompareTwoFloats(std::string("Displacement.Z"), compareVehicle.Displacement.Z, compareDump->ThingVehicle->VehicleDisplacement->ZPos->mFloatValue);

    CompareMovementData(std::string("Increment."), &compareVehicle.Increment, compareDump->ThingVehicle->Increment);
    CompareMovementData(std::string("IncrementAdd."), &compareVehicle.IncrementAdd, compareDump->ThingVehicle->IncrementAdd);
    CompareMovementData(std::string("IncrementSub."), &compareVehicle.IncrementSub, compareDump->ThingVehicle->IncrementSub);
    CompareMovementData(std::string("IncrementLimit."), &compareVehicle.IncrementLimit, compareDump->ThingVehicle->IncrementLimit);

    CompareTwoFloats(std::string("Slope.X"), compareVehicle.Slope.X, compareDump->ThingVehicle->VehicleSlope->XPos->mFloatValue);
    CompareTwoFloats(std::string("Slope.Y"), compareVehicle.Slope.Y, compareDump->ThingVehicle->VehicleSlope->YPos->mFloatValue);
    CompareTwoFloats(std::string("Slope.Z"), compareVehicle.Slope.Z, compareDump->ThingVehicle->VehicleSlope->ZPos->mFloatValue);

    CompareTwoInt16s(std::string("Behind"), compareVehicle.Stats.Behind, compareDump->ThingVehicle->VehicleStats->Behind->mRawValue);
    CompareTwoFloats(std::string("Stats.Velocity"), compareVehicle.Stats.Velocity, compareDump->ThingVehicle->VehicleStats->Velocity->mFloatValue);

    CompareSensorPointData(std::string("FlightModel.FrontLeft"), &compareVehicle.FlightModel.FrontLeft, compareDump->ThingVehicle->FlightModel->SensorPointFrontLeft);
    CompareSensorPointData(std::string("FlightModel.FrontRight"), &compareVehicle.FlightModel.FrontRight, compareDump->ThingVehicle->FlightModel->SensorPointFrontRight);
    CompareSensorPointData(std::string("FlightModel.RearLeft"), &compareVehicle.FlightModel.RearLeft, compareDump->ThingVehicle->FlightModel->SensorPointRearLeft);
    CompareSensorPointData(std::string("FlightModel.RearRight"), &compareVehicle.FlightModel.RearRight, compareDump->ThingVehicle->FlightModel->SensorPointRearRight);

    CompareTwoFloats(std::string("Momentum.AngleXY"), compareVehicle.Momentum.AngleXY, compareDump->ThingVehicle->Momentum->AngleXY->mFloatValue);
    CompareTwoFloats(std::string("Momentum.DeltaX"), compareVehicle.Momentum.DeltaX, compareDump->ThingVehicle->Momentum->DeltaX->mFloatValue);
    CompareTwoFloats(std::string("Momentum.DeltaY"), compareVehicle.Momentum.DeltaY, compareDump->ThingVehicle->Momentum->DeltaY->mFloatValue);

    CompareTwoFloats(std::string("Friction"), compareVehicle.mFriction, compareDump->ThingVehicle->Friction->mFloatValue);
    CompareTwoFloats(std::string("FrictionLimit"), compareVehicle.mFrictionLimit, compareDump->ThingVehicle->FrictionLimit->mFloatValue);
    CompareTwoInt16s(std::string("ThrustEffectiveness"), compareVehicle.mThrustEffectiveness, compareDump->ThingVehicle->ThrustEffectiveness->mRawValue);
    CompareTwoFloats(std::string("SideslipToThrust"), compareVehicle.mSideslipToThrust, compareDump->ThingVehicle->SideslipToThrust->mFloatValue);
    CompareTwoFloats(std::string("SideslipFriction"), compareVehicle.mSideslipFriction, compareDump->ThingVehicle->SideslipFriction->mFloatValue);

    CompareMovementData(std::string("MovementInput."), &compareVehicle.MovementInput, compareDump->ThingVehicle->MovementInput);

    CompareTwoFloats(std::string("MaximumZpos"), compareVehicle.mMaximumZpos, compareDump->ThingVehicle->MaximumZpos->mFloatValue);
    CompareTwoFloats(std::string("Bounce"), compareVehicle.mBounce, compareDump->ThingVehicle->Bounce->mFloatValue);
}

void DbgInterface::CompareVehicleStateBetweenMemDumps(MemDump* dump1, MemDump* dump2) {
    ParseThing* playerThing1 = dump1->ReturnThingFirstPlayer();

    if (playerThing1 == nullptr) {
        return;
    }

    ParseThing* playerThing2 = dump2->ReturnThingFirstPlayer();

    if (playerThing2 == nullptr) {
        return;
    }

    //VehicleIndex in Thing tells us the index of the VehicleThing in the array of
    //possible VehicleThings that is linked to this specific vehicle;
    //parse its data
    dump1->ThingVehicle->Update(mDumpLevelStructStart + 0x40B3C + playerThing1->VehicleIndex->mRawValue * 0x1F0);
    dump2->ThingVehicle->Update(mDumpLevelStructStart + 0x40B3C + playerThing2->VehicleIndex->mRawValue * 0x1F0);

    CompareTwoFloats(std::string("ThingData.Position.X"), playerThing1->Position->XPos->mFloatValue, playerThing2->Position->XPos->mFloatValue);
    CompareTwoFloats(std::string("ThingData.Position.Y"), playerThing1->Position->YPos->mFloatValue, playerThing2->Position->YPos->mFloatValue);
    CompareTwoFloats(std::string("ThingData.Position.Z"), playerThing1->Position->ZPos->mFloatValue, playerThing2->Position->ZPos->mFloatValue);

    CompareTwoFloats(std::string("ThingData.Displacement.X"), playerThing1->Displacement->XPos->mFloatValue, playerThing2->Displacement->XPos->mFloatValue);
    CompareTwoFloats(std::string("ThingData.Displacement.Y"), playerThing1->Displacement->YPos->mFloatValue, playerThing2->Displacement->YPos->mFloatValue);
    CompareTwoFloats(std::string("ThingData.Displacement.Z"), playerThing1->Displacement->ZPos->mFloatValue, playerThing2->Displacement->ZPos->mFloatValue);

    CompareTwoInt16s(std::string("ThingData.Life"), playerThing1->Life->mRawValue, playerThing2->Life->mRawValue);

    CompareMovementDataBetweenMemDumps(std::string("ThingData.Movement."), playerThing1->Movement, playerThing2->Movement);

    CompareTwoInt16s(std::string("ThingData.TimeSlice"), (int16_t)(playerThing1->TimeSlice->mRawValue), (int16_t)(playerThing2->TimeSlice->mRawValue));

    //targetVehicle.KeyPressedAccel
    //targetVehicle.KeyPressedDeaccel
    //targetVehicle.KeyPressedTurnLeft
    //targetVehicle.KeyPressedTurnRight

    CompareTwoFloats(std::string("Displacement.X"), dump1->ThingVehicle->VehicleDisplacement->XPos->mFloatValue, dump2->ThingVehicle->VehicleDisplacement->XPos->mFloatValue);
    CompareTwoFloats(std::string("Displacement.Y"), dump1->ThingVehicle->VehicleDisplacement->YPos->mFloatValue, dump2->ThingVehicle->VehicleDisplacement->YPos->mFloatValue);
    CompareTwoFloats(std::string("Displacement.Z"), dump1->ThingVehicle->VehicleDisplacement->ZPos->mFloatValue, dump2->ThingVehicle->VehicleDisplacement->ZPos->mFloatValue);

    CompareMovementDataBetweenMemDumps(std::string("Increment."), dump1->ThingVehicle->Increment, dump2->ThingVehicle->Increment);
    CompareMovementDataBetweenMemDumps(std::string("IncrementAdd."), dump1->ThingVehicle->IncrementAdd, dump2->ThingVehicle->IncrementAdd);
    CompareMovementDataBetweenMemDumps(std::string("IncrementSub."), dump1->ThingVehicle->IncrementSub, dump2->ThingVehicle->IncrementSub);
    CompareMovementDataBetweenMemDumps(std::string("IncrementLimit."), dump1->ThingVehicle->IncrementLimit, dump2->ThingVehicle->IncrementLimit);

    CompareTwoFloats(std::string("Slope.X"), dump1->ThingVehicle->VehicleSlope->XPos->mFloatValue, dump2->ThingVehicle->VehicleSlope->XPos->mFloatValue);
    CompareTwoFloats(std::string("Slope.Y"), dump1->ThingVehicle->VehicleSlope->YPos->mFloatValue, dump2->ThingVehicle->VehicleSlope->YPos->mFloatValue);
    CompareTwoFloats(std::string("Slope.Z"), dump1->ThingVehicle->VehicleSlope->ZPos->mFloatValue, dump2->ThingVehicle->VehicleSlope->ZPos->mFloatValue);

    CompareTwoInt16s(std::string("Behind"), dump1->ThingVehicle->VehicleStats->Behind->mRawValue, dump2->ThingVehicle->VehicleStats->Behind->mRawValue);
    CompareTwoFloats(std::string("Stats.Velocity"), dump1->ThingVehicle->VehicleStats->Velocity->mFloatValue, dump2->ThingVehicle->VehicleStats->Velocity->mFloatValue);

    CompareSensorPointDataBetweenTwoMemDumpts(std::string("FlightModel.FrontLeft"), dump1->ThingVehicle->FlightModel->SensorPointFrontLeft, dump2->ThingVehicle->FlightModel->SensorPointFrontLeft);
    CompareSensorPointDataBetweenTwoMemDumpts(std::string("FlightModel.FrontRight"), dump1->ThingVehicle->FlightModel->SensorPointFrontRight, dump2->ThingVehicle->FlightModel->SensorPointFrontRight);
    CompareSensorPointDataBetweenTwoMemDumpts(std::string("FlightModel.RearLeft"), dump1->ThingVehicle->FlightModel->SensorPointRearLeft, dump2->ThingVehicle->FlightModel->SensorPointRearLeft);
    CompareSensorPointDataBetweenTwoMemDumpts(std::string("FlightModel.RearRight"), dump1->ThingVehicle->FlightModel->SensorPointRearRight, dump2->ThingVehicle->FlightModel->SensorPointRearRight);

    CompareTwoFloats(std::string("Momentum.AngleXY"), dump1->ThingVehicle->Momentum->AngleXY->mFloatValue, dump2->ThingVehicle->Momentum->AngleXY->mFloatValue);
    CompareTwoFloats(std::string("Momentum.DeltaX"), dump1->ThingVehicle->Momentum->DeltaX->mFloatValue, dump2->ThingVehicle->Momentum->DeltaX->mFloatValue);
    CompareTwoFloats(std::string("Momentum.DeltaY"), dump1->ThingVehicle->Momentum->DeltaY->mFloatValue, dump2->ThingVehicle->Momentum->DeltaY->mFloatValue);

    CompareTwoFloats(std::string("Friction"), dump1->ThingVehicle->Friction->mFloatValue, dump2->ThingVehicle->Friction->mFloatValue);
    CompareTwoFloats(std::string("FrictionLimit"), dump1->ThingVehicle->FrictionLimit->mFloatValue, dump2->ThingVehicle->FrictionLimit->mFloatValue);
    CompareTwoInt16s(std::string("ThrustEffectiveness"), dump1->ThingVehicle->ThrustEffectiveness->mRawValue, dump2->ThingVehicle->ThrustEffectiveness->mRawValue);
    CompareTwoFloats(std::string("SideslipToThrust"), dump1->ThingVehicle->SideslipToThrust->mFloatValue, dump2->ThingVehicle->SideslipToThrust->mFloatValue);
    CompareTwoFloats(std::string("SideslipFriction"), dump1->ThingVehicle->SideslipFriction->mFloatValue, dump2->ThingVehicle->SideslipFriction->mFloatValue);

    CompareMovementDataBetweenMemDumps(std::string("MovementInput."), dump1->ThingVehicle->MovementInput, dump2->ThingVehicle->MovementInput);

    CompareTwoFloats(std::string("MaximumZpos"), dump1->ThingVehicle->MaximumZpos->mFloatValue, dump2->ThingVehicle->MaximumZpos->mFloatValue);
    CompareTwoFloats(std::string("Bounce"), dump1->ThingVehicle->Bounce->mFloatValue, dump2->ThingVehicle->Bounce->mFloatValue);
}

void DbgInterface::SetVehicleStatePlayerFromMemDump(VVehicle& targetVehicle, MemDump* srcDump) {
    ParseThing* playerThing = srcDump->ReturnThingFirstPlayer();

    if (playerThing == nullptr) {
        return;
    }

    //VehicleIndex in Thing tells us the index of the VehicleThing in the array of
    //possible VehicleThings that is linked to this specific vehicle;
    //parse its data
    srcDump->ThingVehicle->Update(mDumpLevelStructStart + 0x40B3C + playerThing->VehicleIndex->mRawValue * 0x1F0);

    //now copy parsed data into my internal game object for debugging
    //purposes
    targetVehicle.ThingData.Position.X = playerThing->Position->XPos->mFloatValue;
    targetVehicle.ThingData.Position.Y = playerThing->Position->YPos->mFloatValue;
    targetVehicle.ThingData.Position.Z = playerThing->Position->ZPos->mFloatValue;

    targetVehicle.ThingData.Displacement.X = playerThing->Displacement->XPos->mFloatValue;
    targetVehicle.ThingData.Displacement.Y = playerThing->Displacement->YPos->mFloatValue;
    targetVehicle.ThingData.Displacement.Z = playerThing->Displacement->ZPos->mFloatValue;

    targetVehicle.ThingData.Life = playerThing->Life->mRawValue;

    CopyMovementClassToMovementStruct(targetVehicle.ThingData.Movement, playerThing->Movement);

    targetVehicle.ThingData.mTimeSlice = (uint8_t)(playerThing->TimeSlice->GetRawValue());

    //targetVehicle.KeyPressedAccel
    //targetVehicle.KeyPressedDeaccel
    //targetVehicle.KeyPressedTurnLeft
    //targetVehicle.KeyPressedTurnRight
    targetVehicle.Displacement.X = srcDump->ThingVehicle->VehicleDisplacement->XPos->mFloatValue;
    targetVehicle.Displacement.Y = srcDump->ThingVehicle->VehicleDisplacement->YPos->mFloatValue;
    targetVehicle.Displacement.Z = srcDump->ThingVehicle->VehicleDisplacement->ZPos->mFloatValue;

    CopyMovementClassToMovementStruct(targetVehicle.Increment, srcDump->ThingVehicle->Increment);
    CopyMovementClassToMovementStruct(targetVehicle.IncrementAdd, srcDump->ThingVehicle->IncrementAdd);
    CopyMovementClassToMovementStruct(targetVehicle.IncrementSub, srcDump->ThingVehicle->IncrementSub);
    CopyMovementClassToMovementStruct(targetVehicle.IncrementLimit, srcDump->ThingVehicle->IncrementLimit);

    targetVehicle.Slope.X = srcDump->ThingVehicle->VehicleSlope->XPos->mFloatValue;
    targetVehicle.Slope.Y = srcDump->ThingVehicle->VehicleSlope->YPos->mFloatValue;
    targetVehicle.Slope.Z = srcDump->ThingVehicle->VehicleSlope->ZPos->mFloatValue;

    targetVehicle.Stats.Behind = srcDump->ThingVehicle->VehicleStats->Behind->mRawValue;
    targetVehicle.Stats.Velocity = srcDump->ThingVehicle->VehicleStats->Velocity->mFloatValue;

    CopySensorPointClassToVehicleSensorPointStruct(targetVehicle.FlightModel.FrontLeft, srcDump->ThingVehicle->FlightModel->SensorPointFrontLeft);
    CopySensorPointClassToVehicleSensorPointStruct(targetVehicle.FlightModel.FrontRight, srcDump->ThingVehicle->FlightModel->SensorPointFrontRight);
    CopySensorPointClassToVehicleSensorPointStruct(targetVehicle.FlightModel.RearLeft, srcDump->ThingVehicle->FlightModel->SensorPointRearLeft);
    CopySensorPointClassToVehicleSensorPointStruct(targetVehicle.FlightModel.RearRight, srcDump->ThingVehicle->FlightModel->SensorPointRearRight);

    targetVehicle.Momentum.AngleXY = srcDump->ThingVehicle->Momentum->AngleXY->mFloatValue;
    targetVehicle.Momentum.DeltaX = srcDump->ThingVehicle->Momentum->DeltaX->mFloatValue;
    targetVehicle.Momentum.DeltaY = srcDump->ThingVehicle->Momentum->DeltaY->mFloatValue;
    targetVehicle.mFriction = srcDump->ThingVehicle->Friction->mFloatValue;
    targetVehicle.mFrictionLimit = srcDump->ThingVehicle->FrictionLimit->mFloatValue;
    targetVehicle.mThrustEffectiveness = srcDump->ThingVehicle->ThrustEffectiveness->mRawValue;
    targetVehicle.mSideslipToThrust = srcDump->ThingVehicle->SideslipToThrust->mFloatValue;
    targetVehicle.mSideslipFriction = srcDump->ThingVehicle->SideslipFriction->mFloatValue;

    CopyMovementClassToMovementStruct(targetVehicle.MovementInput, srcDump->ThingVehicle->MovementInput);
    targetVehicle.mMaximumZpos = srcDump->ThingVehicle->MaximumZpos->mFloatValue;
    targetVehicle.mBounce = srcDump->ThingVehicle->Bounce->mFloatValue;
}

DbgInterface::DbgInterface()
{
}

DbgInterface::~DbgInterface()
{
    if (newDump != nullptr) {
        delete newDump;
        newDump = nullptr;
    }

    if (newDump2 != nullptr) {
        delete newDump2;
        newDump2 = nullptr;
    }

    if (levelData != nullptr) {
        delete levelData;
        levelData = nullptr;
    }
}
