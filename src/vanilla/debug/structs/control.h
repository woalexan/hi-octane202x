/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef CONTROL_H
#define CONTROL_H

#include <vector>
#include <stdlib.h>
#include <string>
#include <cstdint>

class MemDump;
class Int8_Num;
class UInt8_Num;
class UInt16_Num;
class Int16_Num;
class Int32_Num;
class DataTools;

struct ConditionsClass {
public:
    ConditionsClass(DataTools* parent, std::string name, size_t startPosData);
    ~ConditionsClass();

    std::string GetAsString();

private:
    DataTools* mParent = nullptr;
    std::string mName;

public:
   Int32_Num* BumpAmount = nullptr;
   Int32_Num* RocketsLaunched = nullptr;
   Int32_Num* RocketsHit = nullptr;
   Int32_Num* Bullets = nullptr;
   Int32_Num* BulletsHit = nullptr;
   Int32_Num* HitRatio = nullptr;
   Int32_Num* MiniGunHeatup = nullptr;
   Int32_Num* Deaths[8];
   Int32_Num* DeathsCount = nullptr;
   Int32_Num* Kills[8];
   Int32_Num* KillsCount = nullptr;
   Int32_Num* Flags = nullptr;
   Int32_Num* LapTimes[100];
   Int32_Num* AverageLapTime = nullptr;
   Int32_Num* FastestLapTime = nullptr;
   Int32_Num* TotalTime = nullptr;
   Int32_Num* LapCount = nullptr;
   Int32_Num* FuelUsed = nullptr;
   Int32_Num* HealthUsed = nullptr;
   Int32_Num* WeaponsUsed = nullptr;
   Int32_Num* RacePosition = nullptr;
   Int32_Num* RacePositionFinishShowTime = nullptr;
   Int32_Num* RacePoints = nullptr;
   Int32_Num* GodFactor = nullptr;
   Int8_Num* FuelLowCounter = nullptr;
   Int8_Num* FuelRechargeCounter = nullptr;
   Int8_Num* FuelFullCounter = nullptr;
   Int8_Num* WeaponsLowCounter = nullptr;
   Int8_Num* WeaponsRechargeCounter = nullptr;
   Int8_Num* WeaponsFullCounter = nullptr;
   Int8_Num* HealthLowCounter = nullptr;
   Int8_Num* HealthRechargeCounter = nullptr;
   Int8_Num* HealthFullCounter = nullptr;
};

class ParseControlClass {
public:
    ParseControlClass(MemDump* parentMemdump);
    ~ParseControlClass();

    void Update(size_t fromAdr);
    void Print();

private:
    MemDump* mParentMemDump = nullptr;

public:

    ConditionsClass* Conditions = nullptr;

    Int16_Num* ViewType = nullptr;
    Int16_Num* Index = nullptr;
};

#endif // CONTROL_H
