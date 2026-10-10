/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "basicstructs.h"
#include "control.h"
#include "../memdump.h"
#include "../datatools.h"
#include <iostream>
#include <sstream>

ConditionsClass::ConditionsClass(DataTools* parent, std::string name, size_t startPosData) {
    mParent = parent;
    mName = name;

    BumpAmount = mParent->AddInt32_NumVar(std::string("BumpAmount"), startPosData);
    RocketsLaunched = mParent->AddInt32_NumVar(std::string("RocketsLaunched"), startPosData + 0x4);
    RocketsHit = mParent->AddInt32_NumVar(std::string("RocketsHit"), startPosData + 0x8);
    Bullets = mParent->AddInt32_NumVar(std::string("Bullets"), startPosData + 0xC);
    BulletsHit = mParent->AddInt32_NumVar(std::string("BulletsHit"), startPosData + 0x10);
    HitRatio = mParent->AddInt32_NumVar(std::string("HitRatio"), startPosData + 0x14);
    MiniGunHeatup = mParent->AddInt32_NumVar(std::string("MiniGunHeatup"), startPosData + 0x18);

    std::string strVarName("");
    char hlpstr[50];

    for (size_t idx = 0; idx < 8; idx++) {
        strVarName = "Deaths";
        snprintf(hlpstr, 50, "%zu", (size_t)(idx));
        strVarName.append(hlpstr);
        Deaths[idx] = mParent->AddInt32_NumVar(strVarName, startPosData + 0x1C + 0x4 * idx);
    }

    DeathsCount = mParent->AddInt32_NumVar(std::string("DeathsCount"), startPosData + 0x3C);

    for (size_t idx = 0; idx < 8; idx++) {
        strVarName = "Kills";
        snprintf(hlpstr, 50, "%zu", (size_t)(idx));
        strVarName.append(hlpstr);
        Kills[idx] = mParent->AddInt32_NumVar(strVarName, startPosData + 0x40 + 0x4 * idx);
    }

    KillsCount = mParent->AddInt32_NumVar(std::string("KillsCount"), startPosData + 0x60);
    Flags = mParent->AddInt32_NumVar(std::string("Flags"), startPosData + 0x64);

    for (size_t idx = 0; idx < 100; idx++) {
        strVarName = "LapTimes";
        snprintf(hlpstr, 50, "%zu", (size_t)(idx));
        strVarName.append(hlpstr);
        LapTimes[idx] = mParent->AddInt32_NumVar(strVarName, startPosData + 0x68 + 0x4 * idx);
    }

    AverageLapTime = mParent->AddInt32_NumVar(std::string("AverageLapTime"), startPosData + 0x1F8);
    FastestLapTime = mParent->AddInt32_NumVar(std::string("FastestLapTime"), startPosData + 0x1FC);
    TotalTime = mParent->AddInt32_NumVar(std::string("TotalTime"), startPosData + 0x200);
    LapCount = mParent->AddInt32_NumVar(std::string("LapCount"), startPosData + 0x204);
    FuelUsed = mParent->AddInt32_NumVar(std::string("FuelUsed"), startPosData + 0x208);
    HealthUsed = mParent->AddInt32_NumVar(std::string("HealthUsed"), startPosData + 0x20C);
    WeaponsUsed = mParent->AddInt32_NumVar(std::string("WeaponsUsed"), startPosData + 0x210);
    RacePosition = mParent->AddInt32_NumVar(std::string("RacePosition"), startPosData + 0x214);
    RacePositionFinishShowTime = mParent->AddInt32_NumVar(std::string("RacePositionFinishShowTime"),
                                                          startPosData + 0x218);
    RacePoints = mParent->AddInt32_NumVar(std::string("RacePoints"), startPosData + 0x21C);
    GodFactor = mParent->AddInt32_NumVar(std::string("GodFactor"), startPosData + 0x220);

    FuelLowCounter = mParent->AddInt8_NumVar(std::string("FuelLowCounter"), startPosData + 0x224);
    FuelRechargeCounter = mParent->AddInt8_NumVar(std::string("FuelRechargeCounter"), startPosData + 0x225);
    FuelFullCounter = mParent->AddInt8_NumVar(std::string("FuelFullCounter"), startPosData + 0x226);

    WeaponsLowCounter = mParent->AddInt8_NumVar(std::string("WeaponsLowCounter"), startPosData + 0x227);
    WeaponsRechargeCounter = mParent->AddInt8_NumVar(std::string("WeaponsRechargeCounter"), startPosData + 0x228);
    WeaponsFullCounter = mParent->AddInt8_NumVar(std::string("WeaponsFullCounter"), startPosData + 0x229);

    HealthLowCounter = mParent->AddInt8_NumVar(std::string("HealthLowCounter"), startPosData + 0x22A);
    HealthRechargeCounter = mParent->AddInt8_NumVar(std::string("HealthRechargeCounter"), startPosData + 0x22B);
    HealthFullCounter = mParent->AddInt8_NumVar(std::string("HealthFullCounter"), startPosData + 0x22C);
}

std::string ConditionsClass::GetAsString() {
    std::ostringstream output;

    //03.10.2026: TODO: add output (there is just to much stuff to print)

/*    output << mName << ": " << Behind->GetAsString() << ", " << Fuel->GetAsString() << ", " <<
              Weapons->GetAsString()  << ", " << Health->GetAsString() << ", " <<
              Velocity->GetAsString() << ", " << DamageCount->GetAsString() << ", " <<
              Handling->GetAsString() << ", " << Speed->GetAsString() << ", " << Armour->GetAsString() <<
              ", " << Weight->GetAsString() << ", " << Invincable->GetAsString() << ", " << Invisible->GetAsString() <<
              ", " << VehicleHit->GetAsString();*/

    return output.str();
}

ConditionsClass::~ConditionsClass() {
    if (BumpAmount != nullptr) {
        delete BumpAmount;
        BumpAmount = nullptr;
    }

    if (RocketsLaunched != nullptr) {
        delete RocketsLaunched;
        RocketsLaunched = nullptr;
    }

    if (RocketsHit != nullptr) {
        delete RocketsHit;
        RocketsHit = nullptr;
    }

    if (Bullets != nullptr) {
        delete Bullets;
        Bullets = nullptr;
    }

    if (BulletsHit != nullptr) {
        delete BulletsHit;
        BulletsHit = nullptr;
    }

    if (HitRatio != nullptr) {
        delete HitRatio;
        HitRatio = nullptr;
    }

    if (MiniGunHeatup != nullptr) {
        delete MiniGunHeatup;
        MiniGunHeatup = nullptr;
    }

    for (size_t idx = 0; idx < 8; idx++) {
        if (Deaths[idx] != nullptr) {
            delete Deaths[idx];
            Deaths[idx] = nullptr;
        }
    }

    if (DeathsCount != nullptr) {
        delete DeathsCount;
        DeathsCount = nullptr;
    }

    for (size_t idx = 0; idx < 8; idx++) {
        if (Kills[idx] != nullptr) {
            delete Kills[idx];
            Kills[idx] = nullptr;
        }
    }

    if (KillsCount != nullptr) {
        delete KillsCount;
        KillsCount = nullptr;
    }

    if (Flags != nullptr) {
        delete Flags;
        Flags = nullptr;
    }

    for (size_t idx = 0; idx < 8; idx++) {
        if (LapTimes[idx] != nullptr) {
            delete LapTimes[idx];
            LapTimes[idx] = nullptr;
        }
    }

    if (AverageLapTime != nullptr) {
        delete AverageLapTime;
        AverageLapTime = nullptr;
    }

    if (FastestLapTime != nullptr) {
        delete FastestLapTime;
        FastestLapTime = nullptr;
    }

    if (TotalTime != nullptr) {
        delete TotalTime;
        TotalTime = nullptr;
    }

    if (LapCount != nullptr) {
        delete LapCount;
        LapCount = nullptr;
    }

    if (FuelUsed != nullptr) {
        delete FuelUsed;
        FuelUsed = nullptr;
    }

    if (HealthUsed != nullptr) {
        delete HealthUsed;
        HealthUsed = nullptr;
    }

    if (WeaponsUsed != nullptr) {
        delete WeaponsUsed;
        WeaponsUsed = nullptr;
    }

    if (RacePosition != nullptr) {
        delete RacePosition;
        RacePosition = nullptr;
    }

    if (RacePositionFinishShowTime != nullptr) {
        delete RacePositionFinishShowTime;
        RacePositionFinishShowTime = nullptr;
    }

    if (RacePoints != nullptr) {
        delete RacePoints;
        RacePoints = nullptr;
    }

    if (GodFactor != nullptr) {
        delete GodFactor;
        GodFactor = nullptr;
    }

    if (FuelLowCounter != nullptr) {
        delete FuelLowCounter;
        FuelLowCounter = nullptr;
    }

    if (FuelRechargeCounter != nullptr) {
        delete FuelRechargeCounter;
        FuelRechargeCounter = nullptr;
    }

    if (FuelFullCounter != nullptr) {
        delete FuelFullCounter;
        FuelFullCounter = nullptr;
    }

    if (WeaponsLowCounter != nullptr) {
        delete WeaponsLowCounter;
        WeaponsLowCounter = nullptr;
    }

    if (WeaponsRechargeCounter != nullptr) {
        delete WeaponsRechargeCounter;
        WeaponsRechargeCounter = nullptr;
    }

    if (WeaponsFullCounter != nullptr) {
        delete WeaponsFullCounter;
        WeaponsFullCounter = nullptr;
    }

    if (HealthLowCounter != nullptr) {
        delete HealthLowCounter;
        HealthLowCounter = nullptr;
    }

    if (HealthRechargeCounter != nullptr) {
        delete HealthRechargeCounter;
        HealthRechargeCounter = nullptr;
    }

    if (HealthFullCounter != nullptr) {
        delete HealthFullCounter;
        HealthFullCounter = nullptr;
    }
}

ParseControlClass::ParseControlClass(MemDump* parentMemdump) {
   mParentMemDump = parentMemdump;
}

void ParseControlClass::Print() {
    //03.10.2026: TODO: add output (there is just to much stuff to print)
   /* std::cout << Momentum->GetAsString() << std::endl; */
}

void ParseControlClass::Update(size_t fromAdr) {
    //TODO: if needed add other stuff as well later

    Conditions = new ConditionsClass(mParentMemDump->mDataTools, std::string("Conditions"), fromAdr + 0x14);
    ViewType = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("ViewType"), fromAdr + 0x974);
    Index = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Index"), fromAdr + 0x978);
}

ParseControlClass::~ParseControlClass() {
    if (Conditions != nullptr) {
        delete Conditions;
        Conditions = nullptr;
    }

    if (ViewType != nullptr) {
        delete ViewType;
        ViewType = nullptr;
    }

    if (Index != nullptr) {
        delete Index;
        Index = nullptr;
    }
}
