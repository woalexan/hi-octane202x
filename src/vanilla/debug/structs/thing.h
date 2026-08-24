/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef THING_H
#define THING_H

#include <vector>
#include <stdlib.h>
#include <string>
#include <cstdint>

class MemDump;
class UInt8_Num;
class UInt16_Num;
class UInt32_Num;
class Int8_Num;
class Int16_Num;
class Int32_Num;
class FixedPointInt16_8R8Num;
class FixedPointInt16_8R8NumAngle;
class DataTools;
class Coord3DClass;
class MovementClass;
class ColideClass;

class ParseThing {
public:
    ParseThing(MemDump* parentMemdump);
    ~ParseThing();

    void Update(size_t fromAdr);
    void Print();

private:
    MemDump* mParentMemDump = nullptr;

public:
    Coord3DClass* Position = nullptr;
    MovementClass* Movement = nullptr;
    ColideClass* Colide = nullptr;
    Coord3DClass* Displacement = nullptr;

    UInt32_Num* Status = nullptr;
    Int16_Num* Target = nullptr;
    Int16_Num* Index = nullptr;
    Int16_Num* Child = nullptr;
    Int16_Num* Parent = nullptr;
    Int16_Num* Count = nullptr;
    Int16_Num* Id = nullptr;
    Int16_Num* Life = nullptr;
    UInt16_Num* Seed = nullptr;
    Int16_Num* Owner = nullptr;
    Int16_Num* VehicleIndex = nullptr;
    Int16_Num* NextThingInGroup = nullptr;
    Int16_Num* Upgrade = nullptr;
    Int8_Num* Member = nullptr;
    Int8_Num* Action = nullptr;
    Int8_Num* Group = nullptr;
    Int8_Num* TimeSlice = nullptr;
};

#endif // THING_H
