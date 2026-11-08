/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef THINGVEHICLE_H
#define THINGVEHICLE_H

#include <vector>
#include <stdlib.h>
#include <string>
#include <cstdint>

class MemDump;
class UInt8_Num;
class UInt16_Num;
class Int16_Num;
class Int32_Num;
class FixedPointInt16_8R8Num;
class FixedPointInt32_8R8Num;
class FixedPointInt16_8R8NumAngle;
class FixedPointInt32_8R8NumAngle;
class DataTools;
class Coord3DClass;
class MovementClass;

struct MomentumClass {
public:
    MomentumClass(DataTools* parent, std::string name, size_t startPosData);
    ~MomentumClass();

    std::string GetAsString();

private:
    DataTools* mParent = nullptr;
    std::string mName;

public:
    FixedPointInt32_8R8Num* DeltaX = nullptr;
    FixedPointInt32_8R8Num* DeltaY = nullptr;
    FixedPointInt32_8R8NumAngle* AngleXY = nullptr;
};

class SensorPointClass {
public:
    SensorPointClass(DataTools* parent, std::string name, size_t startPosData);
    ~SensorPointClass();

    std::string GetAsString();

private:
    DataTools* mParent = nullptr;
    std::string mName;

public:
    Coord3DClass* Position = nullptr;
    Int32_Num* CollideFlags;
    Int32_Num* CollideCount;
    FixedPointInt32_8R8Num* Zpos = nullptr;
    FixedPointInt32_8R8Num* ZposFloor = nullptr;
    FixedPointInt32_8R8Num* ZposDiff = nullptr;
    FixedPointInt16_8R8Num* ZposDisplacement = nullptr;
    UInt8_Num* Rebound;
    UInt8_Num* ReboundLimit;
};

class CollideControlClass {
public:
    CollideControlClass(DataTools* parent, std::string name, size_t startPosData);
    ~CollideControlClass();

    std::string GetAsString();

private:
    DataTools* mParent = nullptr;
    std::string mName;
    FixedPointInt16_8R8Num* CollideSizeForward = nullptr;
    FixedPointInt16_8R8Num* CollideSizeRear = nullptr;
    FixedPointInt16_8R8Num* CollideSizeSideways = nullptr;
    FixedPointInt16_8R8Num* CollideSizeSensorOffset = nullptr;

public:

    SensorPointClass* SensorPointFrontLeft = nullptr;
    SensorPointClass* SensorPointFrontRight = nullptr;
    SensorPointClass* SensorPointRearLeft = nullptr;
    SensorPointClass* SensorPointRearRight = nullptr;

    UInt16_Num *Flag = nullptr;
    UInt16_Num *FunctionFlag = nullptr;
    UInt16_Num* RideHeight = nullptr;
    UInt16_Num* BrakePower = nullptr;
};

class VehicleStatsClass {
public:
    VehicleStatsClass(DataTools* parent, std::string name, size_t startPosData);
    ~VehicleStatsClass();

    std::string GetAsString();

private:
    DataTools* mParent = nullptr;
    std::string mName;

public:

    Int16_Num* Behind = nullptr;
    Int16_Num* Fuel = nullptr;
    Int16_Num* Weapons = nullptr;
    Int16_Num* Health = nullptr;
    FixedPointInt16_8R8Num* Velocity = nullptr;
    Int16_Num* DamageCount = nullptr;
    Int16_Num* Handling = nullptr;
    Int16_Num* Speed = nullptr;
    Int16_Num* Armour = nullptr;
    Int16_Num* Weight = nullptr;
    Int16_Num* Invincable = nullptr;
    Int16_Num* Invisible = nullptr;
    Int16_Num* VehicleHit = nullptr;
};

class ParseThingVehicle {
public:
    ParseThingVehicle(MemDump* parentMemdump);
    ~ParseThingVehicle();

    void Update(size_t fromAdr);
    void Print();

private:
    MemDump* mParentMemDump = nullptr;

public:

    MomentumClass* Momentum = nullptr;
    MovementClass* MovementInput = nullptr;
    MovementClass* Increment = nullptr;
    MovementClass* IncrementAdd = nullptr;
    MovementClass* IncrementSub = nullptr;
    MovementClass* IncrementLimit = nullptr;

    CollideControlClass* FlightModel = nullptr;

    Coord3DClass* VehicleDisplacement = nullptr;
    Coord3DClass* VehicleSlope = nullptr;
    Coord3DClass* VehicleBump = nullptr;
    Coord3DClass* VehicleBonus = nullptr;

    VehicleStatsClass* VehicleStats = nullptr;

    Int16_Num* Index = nullptr;

    FixedPointInt16_8R8Num* Friction = nullptr;
    FixedPointInt16_8R8Num* FrictionLimit = nullptr;
    Int16_Num* ThrustEffectiveness = nullptr;
    FixedPointInt16_8R8Num* Bounce = nullptr;
    FixedPointInt16_8R8Num* MaximumZpos = nullptr;
    FixedPointInt16_8R8Num* SideslipFriction = nullptr;
    FixedPointInt16_8R8Num* SideslipToThrust = nullptr;
};

#endif // THINGVEHICLE_H
