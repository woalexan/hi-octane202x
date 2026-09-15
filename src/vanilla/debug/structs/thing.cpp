/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "basicstructs.h"
#include "thing.h"
#include "../memdump.h"
#include "../datatools.h"
#include <iostream>
#include <sstream>

ParseThing::ParseThing(MemDump* parentMemdump) {
   mParentMemDump = parentMemdump;
}

void ParseThing::Print() {
    std::cout << Position->GetAsString() << std::endl;
    std::cout << Movement->GetAsString() << std::endl;
    std::cout << Colide->GetAsString() << std::endl;
    std::cout << Displacement->GetAsString() << std::endl;
    std::cout << Status->GetAsString() << std::endl;
    std::cout << Target->GetAsString() << std::endl;
    std::cout << Index->GetAsString() << std::endl;
    std::cout << Child->GetAsString() << std::endl;
    std::cout << Parent->GetAsString() << std::endl;
    std::cout << Count->GetAsString() << std::endl;
    std::cout << Id->GetAsString() << std::endl;
    std::cout << Life->GetAsString() << std::endl;
    std::cout << Seed->GetAsString() << std::endl;
    std::cout << Owner->GetAsString() << std::endl;
    std::cout << VehicleIndex->GetAsString() << std::endl;
    std::cout << NextThingInGroup->GetAsString() << std::endl;
    std::cout << Upgrade->GetAsString() << std::endl;
    std::cout << Member->GetAsString() << std::endl;
    std::cout << Action->GetAsString() << std::endl;
    std::cout << Group->GetAsString() << std::endl;
    std::cout << TimeSlice->GetAsString() << std::endl;
}

void ParseThing::Update(size_t fromAdr) {
    Position = new Coord3DClass(mParentMemDump->mDataTools, std::string("Position"), fromAdr);
    Movement = new MovementClass(mParentMemDump->mDataTools, std::string("Movement"), fromAdr + 0x10);
    Colide = new ColideClass(mParentMemDump->mDataTools, std::string("Colide"), fromAdr + 0x1C);
    Displacement = new Coord3DClass(mParentMemDump->mDataTools, std::string("Displacement"), fromAdr + 0x30);
    Status = mParentMemDump->mDataTools->AddUInt32_NumVar(std::string("Status"), fromAdr + 0x44);
    Target = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Target"), fromAdr + 0x48);
    Index = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Index"), fromAdr + 0x4A);
    Child = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Child"), fromAdr + 0x4C);
    Parent = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Parent"), fromAdr + 0x4E);
    Count = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Count"), fromAdr + 0x50);
    Id = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Id"), fromAdr + 0x52);
    Life = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Life"), fromAdr + 0x54);
    Seed = mParentMemDump->mDataTools->AddUInt16_NumVar(std::string("Seed"), fromAdr + 0x56);
    Owner = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Owner"), fromAdr + 0x58);
    VehicleIndex = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("VehicleIndex"), fromAdr + 0x5A);
    NextThingInGroup = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("NextThingInGroup"), fromAdr + 0x5C);
    Upgrade = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Upgrade"), fromAdr + 0x5E);
    Member = mParentMemDump->mDataTools->AddInt8_NumVar(std::string("Member"), fromAdr + 0x60);
    Action = mParentMemDump->mDataTools->AddInt8_NumVar(std::string("Action"), fromAdr + 0x61);
    Group = mParentMemDump->mDataTools->AddInt8_NumVar(std::string("Group"), fromAdr + 0x62);
    TimeSlice = mParentMemDump->mDataTools->AddInt8_NumVar(std::string("TimeSlice"), fromAdr + 0x63);
}

ParseThing::~ParseThing() {
}
