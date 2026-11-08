/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef DBGINTERFACE_H
#define DBGINTERFACE_H

#include <vector>
#include <stdlib.h>
#include <string>
#include <cstdint>
#include <iomanip>
#include "../vvehicle.h"
#include "../vtrack.h"

class MemDump;
class BinaryFile;
class MovementClass;
class SensorPointClass;
class ParseVectors;
class ParseColVectClass;
class ParseColVectsListClass;
class ParseControlClass;
class CollideControlClass;

struct DiffByte {
    size_t atOffset;
    uint8_t valData1;
    uint8_t valData2;
};

class DbgInterface
{
public:
    DbgInterface();
    ~DbgInterface();

    void Init(std::string memDumpFileName, std::string memDumpFileName2, std::string levelFileName);

    //playerNr starts with index 1 for first player in the function calls below
    void SetVehiclePhysicsStatePlayerFromMemDump(VVehicle& targetVehicle, MemDump* srcDump, uint8_t playerNr);
    void SetCurrentStatsPlayerFromMemDump(VVehicle& targetVehicle, MemDump* srcDump, uint8_t playerNr);
    void SetControlConditionsPlayerFromMemDump(VVehicle& targetVehicle, MemDump* srcDump, uint8_t playerNr);
    void CompareVehicleStatePlayerWithMemDump(VVehicle& compareVehicle, MemDump* compareDump, uint8_t playerNr);
    void CompareVehicleStateBetweenMemDumps(MemDump* dump1, MemDump* dump2, uint8_t playerNr);
    void CompareVectorsWithMemDump(VTrack& compareVector1, ParseVectors* compareVector2);

    //This function parses memory content at address 0x801FFEB8 to derive value of local
    //variable Delta in processVEHICLE_CAR. Warning: Execute this function only if the memory
    //dump was made at the correct breakpoints and the variable currently really exists in
    //the memory dump. Otherwise you will read garbage values. Variable only exists inside
    //function processVEHICLE_CAR;
    irr::core::vector3df ParseVehiclePhysicsDeltaVariable(MemDump* srcDump);

    MemDump* newDump = nullptr;
    MemDump* newDump2 = nullptr;

    size_t mDumpLevelStructStart;

    std::vector<DiffByte> CompareData(std::vector<uint8_t> data1, std::vector<uint8_t> data2);
    void PrintCompareDataResult(std::vector<DiffByte> comparisonResult, int memOffset);

private:
    BinaryFile* levelData = nullptr;

    void WriteDataToBinaryFile(std::string fileName, std::vector<uint8_t> data);

    void CompareTable(std::string tableName, std::vector<uint8_t> table1, std::vector<uint8_t> table2);

    void CompareTwoFloats(std::string varName, irr::f32 val1, irr::f32 val2);
    void CompareTwoInt16s(std::string varName, int16_t val1, int16_t val2);
    void CompareTwoUInt16s(std::string varName, uint16_t val1, uint16_t val2);
    void CompareMovementData(std::string prefixName, MovementStruct* compareStruct, MovementClass* compareClass);
    void CompareSensorPointData(std::string prefixName, VehicleSensorPointStruct* compareStruct, SensorPointClass* compareClass);
    void CompareMovementDataBetweenMemDumps(std::string prefixName, MovementClass* compareClass1, MovementClass* compareClass2);
    void CompareSensorPointDataBetweenTwoMemDumpts(std::string prefixName, SensorPointClass* compareClass1, SensorPointClass* compareClass2);

    void CompareTracksColVect(std::string prefixName, TrackColVectStruct& compareStruct, ParseColVectClass* compareClass);
    void CompareTracksColVectList(std::string prefixName, TrackColVectListStruct& compareStruct, ParseColVectsListClass* compareClass);

    void CopyMovementClassToMovementStruct(MovementStruct& targetStruct, MovementClass* srcClass);
    void CopySensorPointClassToVehicleSensorPointStruct(VehicleSensorPointStruct& targetStruct, SensorPointClass* srcClass);
    void CopyAllFlightModelFlagsToStruct(VehicleCollideControlStruct& targetStruct, CollideControlClass* srcClass);
};

#endif // DBGINTERFACE_H
