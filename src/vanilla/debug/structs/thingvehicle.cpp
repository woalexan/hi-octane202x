/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "basicstructs.h"
#include "thingvehicle.h"
#include "../memdump.h"
#include "../datatools.h"
#include <iostream>
#include <sstream>

SensorPointClass::SensorPointClass(DataTools* parent, std::string name, size_t startPosData) {
    mParent = parent;
    mName = name;
    Position = new Coord3DClass(parent, std::string("Position"), startPosData);
    CollideFlags = mParent->AddInt32_NumVar(std::string("CollideFlags"), startPosData + 0x8);
    CollideCount = mParent->AddInt32_NumVar(std::string("CollideCount"), startPosData + 0xC);
    Zpos = mParent->AddFixedPointInt32_8R8NumVar(std::string("Zpos"), startPosData + 0x10);
    ZposFloor = mParent->AddFixedPointInt32_8R8NumVar(std::string("ZposFloor"), startPosData + 0x14);
    ZposDiff = mParent->AddFixedPointInt32_8R8NumVar(std::string("ZposDiff"), startPosData + 0x18);
    ZposDisplacement = mParent->AddFixedPointInt16_8R8NumVar(std::string("ZposDisplacement"), startPosData + 0x1C);
    Rebound = mParent->AddUInt8_NumVar(std::string("Rebound"), startPosData + 0x1E);
    ReboundLimit = mParent->AddUInt8_NumVar(std::string("ReboundLimit"), startPosData + 0x1F);
}

std::string SensorPointClass::GetAsString() {
    std::ostringstream output;

    output << mName << ": " << Position->GetAsString() << ", " << CollideFlags->GetAsString() << ", " <<
              CollideCount->GetAsString()  << ", " << Zpos->GetAsString() << ", " <<
              ZposFloor->GetAsString() << ", " << ZposDiff->GetAsString() << ", " <<
              ZposDisplacement->GetAsString() << ", " << Rebound->GetAsString() << ", " << ReboundLimit->GetAsString();

    return output.str();
}

SensorPointClass::~SensorPointClass() {
    if (Position != nullptr) {
        delete Position;
        Position = nullptr;
    }
    if (CollideFlags != nullptr) {
        delete CollideFlags;
        CollideFlags = nullptr;
    }
    if (CollideCount != nullptr) {
        delete CollideCount;
        CollideCount = nullptr;
    }
    if (Zpos != nullptr) {
        delete Zpos;
        Zpos = nullptr;
    }
    if (ZposFloor != nullptr) {
        delete ZposFloor;
        ZposFloor = nullptr;
    }
    if (ZposDiff != nullptr) {
        delete ZposDiff;
        ZposDiff = nullptr;
    }
    if (ZposDisplacement != nullptr) {
        delete ZposDisplacement;
        ZposDisplacement = nullptr;
    }
    if (Rebound != nullptr) {
        delete Rebound;
        Rebound = nullptr;
    }
    if (ReboundLimit != nullptr) {
        delete ReboundLimit;
        ReboundLimit = nullptr;
    }
}

CollideControlClass::CollideControlClass(DataTools* parent, std::string name, size_t startPosData) {
    mParent = parent;
    mName = name;
    CollideSizeForward  = mParent->AddFixedPointInt16_8R8NumVar(std::string("CollideSizeForward"), startPosData);
    CollideSizeRear  = mParent->AddFixedPointInt16_8R8NumVar(std::string("CollideSizeRear"), startPosData + 0x2);
    CollideSizeSideways  = mParent->AddFixedPointInt16_8R8NumVar(std::string("CollideSizeSideways"), startPosData + 0x4);
    CollideSizeSensorOffset  = mParent->AddFixedPointInt16_8R8NumVar(std::string("CollideSizeSensorOffset"), startPosData + 0x6);

    SensorPointFrontLeft = new SensorPointClass(parent, std::string("SensorPointFrontLeft"), startPosData + 0x8);
    SensorPointFrontRight = new SensorPointClass(parent, std::string("SensorPointFrontRight"), startPosData + 0x28);
    SensorPointRearLeft = new SensorPointClass(parent, std::string("SensorPointRearLeft"), startPosData + 0x48);
    SensorPointRearRight = new SensorPointClass(parent, std::string("SensorPointRearRight"), startPosData + 0x68);

    RideHeight = mParent->AddUInt16_NumVar(std::string("RideHeight"), startPosData + 0x8C);
    BrakePower = mParent->AddUInt16_NumVar(std::string("BrakePower"), startPosData + 0x8E);
}

std::string CollideControlClass::GetAsString() {
    std::ostringstream output;

    output << mName << ": " << CollideSizeForward->GetAsString() << ", " << CollideSizeRear->GetAsString() << ", " <<
              CollideSizeSideways->GetAsString()  << ", " << CollideSizeSensorOffset->GetAsString() << ", " <<
              SensorPointFrontLeft->GetAsString() << ", " << SensorPointFrontRight->GetAsString() << ", " <<
              SensorPointRearLeft->GetAsString() << ", " << SensorPointRearRight->GetAsString() << ", " << RideHeight->GetAsString() <<
              ", " << BrakePower->GetAsString();

    return output.str();
}

CollideControlClass::~CollideControlClass() {
    if (CollideSizeForward != nullptr) {
        delete CollideSizeForward;
        CollideSizeForward = nullptr;
    }
    if (CollideSizeRear != nullptr) {
        delete CollideSizeRear;
        CollideSizeRear = nullptr;
    }
    if (CollideSizeSideways != nullptr) {
        delete CollideSizeSideways;
        CollideSizeSideways = nullptr;
    }
    if (CollideSizeSensorOffset != nullptr) {
        delete CollideSizeSensorOffset;
        CollideSizeSensorOffset = nullptr;
    }
    if (SensorPointFrontLeft != nullptr) {
        delete SensorPointFrontLeft;
        SensorPointFrontLeft = nullptr;
    }
    if (SensorPointFrontRight != nullptr) {
        delete SensorPointFrontRight;
        SensorPointFrontRight = nullptr;
    }
    if (SensorPointRearLeft != nullptr) {
        delete SensorPointRearLeft;
        SensorPointRearLeft = nullptr;
    }
    if (SensorPointRearRight != nullptr) {
        delete SensorPointRearRight;
        SensorPointRearRight = nullptr;
    }
    if (RideHeight != nullptr) {
        delete RideHeight;
        RideHeight = nullptr;
    }
    if (BrakePower != nullptr) {
        delete BrakePower;
        BrakePower = nullptr;
    }
}

VehicleStatsClass::VehicleStatsClass(DataTools* parent, std::string name, size_t startPosData) {
    mParent = parent;
    mName = name;

    Behind = mParent->AddInt16_NumVar(std::string("Behind"), startPosData);
    Fuel = mParent->AddInt16_NumVar(std::string("Fuel"), startPosData + 0x2);
    Weapons = mParent->AddInt16_NumVar(std::string("Weapons"), startPosData + 0x4);
    Health = mParent->AddInt16_NumVar(std::string("Health"), startPosData + 0x6);
    Velocity = mParent->AddFixedPointInt16_8R8NumVar(std::string("Velocity"), startPosData + 0x8);
    DamageCount = mParent->AddInt16_NumVar(std::string("DamageCount"), startPosData + 0xA);
    Handling = mParent->AddInt16_NumVar(std::string("Handling"), startPosData + 0xC);
    Speed = mParent->AddInt16_NumVar(std::string("Speed"), startPosData + 0xE);
    Armour = mParent->AddInt16_NumVar(std::string("Armour"), startPosData + 0x10);
    Weight = mParent->AddInt16_NumVar(std::string("Weight"), startPosData + 0x12);
    Invincable = mParent->AddInt16_NumVar(std::string("Invincable"), startPosData + 0x14);
    Invisible = mParent->AddInt16_NumVar(std::string("Invisible"), startPosData + 0x16);
    VehicleHit = mParent->AddInt16_NumVar(std::string("VehicleHit"), startPosData + 0x18);
}

std::string VehicleStatsClass::GetAsString() {
    std::ostringstream output;

    output << mName << ": " << Behind->GetAsString() << ", " << Fuel->GetAsString() << ", " <<
              Weapons->GetAsString()  << ", " << Health->GetAsString() << ", " <<
              Velocity->GetAsString() << ", " << DamageCount->GetAsString() << ", " <<
              Handling->GetAsString() << ", " << Speed->GetAsString() << ", " << Armour->GetAsString() <<
              ", " << Weight->GetAsString() << ", " << Invincable->GetAsString() << ", " << Invisible->GetAsString() <<
              ", " << VehicleHit->GetAsString();

    return output.str();
}

VehicleStatsClass::~VehicleStatsClass() {
    if (Behind != nullptr) {
        delete Behind;
        Behind = nullptr;
    }
    if (Fuel != nullptr) {
        delete Fuel;
        Fuel = nullptr;
    }
    if (Weapons != nullptr) {
        delete Weapons;
        Weapons = nullptr;
    }
    if (Health != nullptr) {
        delete Health;
        Health = nullptr;
    }
    if (Velocity != nullptr) {
        delete Velocity;
        Velocity = nullptr;
    }
    if (DamageCount != nullptr) {
        delete DamageCount;
        DamageCount = nullptr;
    }
    if (Handling != nullptr) {
        delete Handling;
        Handling = nullptr;
    }
    if (Speed != nullptr) {
        delete Speed;
        Speed = nullptr;
    }
    if (Armour != nullptr) {
        delete Armour;
        Armour = nullptr;
    }
    if (Weight != nullptr) {
        delete Weight;
        Weight = nullptr;
    }
    if (Invincable != nullptr) {
        delete Invincable;
        Invincable = nullptr;
    }
    if (Invisible != nullptr) {
        delete Invisible;
        Invisible = nullptr;
    }
    if (VehicleHit != nullptr) {
        delete VehicleHit;
        VehicleHit = nullptr;
    }
}

MomentumClass::MomentumClass(DataTools* parent, std::string name, size_t startPosData) {
    mParent = parent;
    mName = name;
    DeltaX =  mParent->AddFixedPointInt32_8R8NumVar(std::string("DeltaX"), startPosData);
    DeltaY =  mParent->AddFixedPointInt32_8R8NumVar(std::string("DeltaY"), startPosData + 4);
    AngleXY = mParent->AddFixedPointInt32_8R8NumVarAngle(std::string("AngleXY"), startPosData + 8);
}

std::string MomentumClass::GetAsString() {
    std::ostringstream output;

    output << mName << ": " << DeltaX->GetAsString() << ", " << DeltaY->GetAsString() << ", " << AngleXY->GetAsString();

    return output.str();
}

MomentumClass::~MomentumClass() {
    if (DeltaX != nullptr) {
        delete DeltaX;
        DeltaX = nullptr;
    }
    if (DeltaY != nullptr) {
        delete DeltaY;
        DeltaY = nullptr;
    }
    if (AngleXY != nullptr) {
        delete AngleXY;
        AngleXY = nullptr;
    }
}

ParseThingVehicle::ParseThingVehicle(MemDump* parentMemdump) {
   mParentMemDump = parentMemdump;
}

void ParseThingVehicle::Print() {
    std::cout << Momentum->GetAsString() << std::endl;

    std::cout << MovementInput->GetAsString() << std::endl;
    std::cout << Increment->GetAsString() << std::endl;
    std::cout << IncrementAdd->GetAsString() << std::endl;
    std::cout << IncrementSub->GetAsString() << std::endl;
    std::cout << IncrementLimit->GetAsString() << std::endl;

    std::cout << FlightModel->GetAsString() << std::endl;

    std::cout << VehicleDisplacement->GetAsString() << std::endl;
    std::cout << VehicleSlope->GetAsString() << std::endl;
    std::cout << VehicleBump->GetAsString() << std::endl;
    std::cout << VehicleBonus->GetAsString() << std::endl;

    std::cout << Index->GetAsString() << std::endl;

    std::cout << Friction->GetAsString() << std::endl;
    std::cout << FrictionLimit->GetAsString() << std::endl;
    std::cout << ThrustEffectiveness->GetAsString() << std::endl;
    std::cout << Bounce->GetAsString() << std::endl;
    std::cout << MaximumZpos->GetAsString() << std::endl;
    std::cout << SideslipFriction->GetAsString() << std::endl;
    std::cout << SideslipToThrust->GetAsString() << std::endl;
}

void ParseThingVehicle::Update(size_t fromAdr) {
    Momentum = new MomentumClass(mParentMemDump->mDataTools, std::string("Momentum"), fromAdr);

    MovementInput = new MovementClass(mParentMemDump->mDataTools, std::string("MovementInput"), fromAdr + 0xC);
    Increment = new MovementClass(mParentMemDump->mDataTools, std::string("Increment"), fromAdr + 0x18);
    IncrementAdd = new MovementClass(mParentMemDump->mDataTools, std::string("IncrementAdd"), fromAdr + 0x24);
    IncrementSub = new MovementClass(mParentMemDump->mDataTools, std::string("IncrementSub"), fromAdr + 0x30);
    IncrementLimit = new MovementClass(mParentMemDump->mDataTools, std::string("IncrementLimit"), fromAdr + 0x3C);

    FlightModel = new CollideControlClass(mParentMemDump->mDataTools, std::string("FlightModel"), fromAdr + 0x50);

    VehicleDisplacement = new Coord3DClass(mParentMemDump->mDataTools, std::string("VehicleDisplacement"), fromAdr + 0xE0);
    VehicleSlope = new Coord3DClass(mParentMemDump->mDataTools, std::string("VehicleSlope"), fromAdr + 0xE8);
    VehicleBump = new Coord3DClass(mParentMemDump->mDataTools, std::string("VehicleBump"), fromAdr + 0xF0);
    VehicleBonus = new Coord3DClass(mParentMemDump->mDataTools, std::string("VehicleBonus"), fromAdr + 0xF8);

    VehicleStats = new VehicleStatsClass(mParentMemDump->mDataTools, std::string("VehicleStats"), fromAdr + 0x17C);

    Index = mParentMemDump->mDataTools->AddInt16_NumVar(std::string("Index"), fromAdr + 0x1CC);

    Friction = mParentMemDump->mDataTools->AddFixedPointInt16_8R8NumVar("Friction", fromAdr + 0x1CE);
    FrictionLimit = mParentMemDump->mDataTools->AddFixedPointInt16_8R8NumVar("FrictionLimit", fromAdr + 0x1D0);
    ThrustEffectiveness = mParentMemDump->mDataTools->AddInt16_NumVar("ThrustEffectiveness", fromAdr + 0x1D2);
    Bounce = mParentMemDump->mDataTools->AddFixedPointInt16_8R8NumVar("Bounce", fromAdr + 0x1D4);
    MaximumZpos = mParentMemDump->mDataTools->AddFixedPointInt16_8R8NumVar("MaximumZpos", fromAdr + 0x1D6);
    SideslipFriction = mParentMemDump->mDataTools->AddFixedPointInt16_8R8NumVar("SideslipFriction", fromAdr + 0x1D8);
    SideslipToThrust = mParentMemDump->mDataTools->AddFixedPointInt16_8R8NumVar("SideslipToThrust", fromAdr + 0x1DA);
}

ParseThingVehicle::~ParseThingVehicle() {
    if (Momentum != nullptr) {
        delete Momentum;
        Momentum = nullptr;
    }

    if (MovementInput != nullptr) {
        delete MovementInput;
        MovementInput = nullptr;
    }

    if (Increment != nullptr) {
        delete Increment;
        Increment = nullptr;
    }

    if (IncrementAdd != nullptr) {
        delete IncrementAdd;
        IncrementAdd = nullptr;
    }

    if (IncrementSub != nullptr) {
        delete IncrementSub;
        IncrementSub = nullptr;
    }


    if (IncrementLimit != nullptr) {
        delete IncrementLimit;
        IncrementLimit = nullptr;
    }

    if (FlightModel != nullptr) {
        delete FlightModel;
        FlightModel = nullptr;
    }

    if (VehicleDisplacement != nullptr) {
        delete VehicleDisplacement;
        VehicleDisplacement = nullptr;
    }

    if (VehicleSlope != nullptr) {
        delete VehicleSlope;
        VehicleSlope = nullptr;
    }

    if (VehicleBump != nullptr) {
        delete VehicleBump;
        VehicleBump = nullptr;
    }

    if (VehicleBonus != nullptr) {
        delete VehicleBonus;
        VehicleBonus = nullptr;
    }

    if (VehicleStats != nullptr) {
        delete VehicleStats;
        VehicleStats = nullptr;
    }

    if (Index != nullptr) {
        delete Index;
        Index = nullptr;
    }

    if (Friction != nullptr) {
        delete Friction;
        Friction = nullptr;
    }

    if (FrictionLimit != nullptr) {
        delete FrictionLimit;
        FrictionLimit = nullptr;
    }

    if (ThrustEffectiveness != nullptr) {
        delete ThrustEffectiveness;
        ThrustEffectiveness = nullptr;
    }

    if (Bounce != nullptr) {
        delete Bounce;
        Bounce = nullptr;
    }

    if (MaximumZpos != nullptr) {
        delete MaximumZpos;
        MaximumZpos = nullptr;
    }

    if (SideslipFriction != nullptr) {
        delete SideslipFriction;
        SideslipFriction = nullptr;
    }

    if (SideslipToThrust != nullptr) {
        delete SideslipToThrust;
        SideslipToThrust = nullptr;
    }
}
