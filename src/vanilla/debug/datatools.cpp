/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "datatools.h"
#include "memdump.h"
#include "binaryfile.h"
#include <iomanip>
#include <iostream>
#include <sstream>

UInt8_Num::UInt8_Num(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToUInt8(start_position);
}

std::string UInt8_Num::GetAsString() {
    std::ostringstream output;

    std::string decStr = std::to_string(mRawValue);

    output << mName << ": 0x" << std::setfill('0') << std::setw(2) <<
                 std::right << std::uppercase << std::hex << int(mRawValue) << " (Dec:" << std::dec << decStr << ")";

    return output.str();
}

UInt16_Num::UInt16_Num(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToUInt16(start_position);
}

std::string UInt16_Num::GetAsString() {
    std::ostringstream output;

    std::string decStr = std::to_string(mRawValue);

    output << mName << ": 0x" << std::setfill('0') << std::setw(4) <<
                 std::right << std::uppercase << std::hex << int(mRawValue) << " (Dec:" << std::dec << decStr << ")";

    return output.str();
}

UInt32_Num::UInt32_Num(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToUInt32(start_position);
}

std::string UInt32_Num::GetAsString() {
    std::ostringstream output;

    std::string decStr = std::to_string(mRawValue);

    output << mName << ": 0x" << std::setfill('0') << std::setw(8) <<
                 std::right << std::uppercase << std::hex << int(mRawValue) << " (Dec:" << std::dec << decStr << ")";

    return output.str();
}

Int8_Num::Int8_Num(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToInt8(start_position);
}

std::string Int8_Num::GetAsString() {
    std::ostringstream output;

    std::string decStr = std::to_string(mRawValue);

    output << mName << ": 0x" << std::setfill('0') << std::setw(2) <<
                 std::right << std::uppercase << std::hex << int(mRawValue) << " (Dec:" << std::dec << decStr << ")";

    return output.str();
}

int8_t Int8_Num::GetRawValue() {
    return mRawValue;
}

Int16_Num::Int16_Num(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToInt16(start_position);
}

std::string Int16_Num::GetAsString() {
    std::ostringstream output;

    std::string decStr = std::to_string(mRawValue);

    output << mName << ": 0x" << std::setfill('0') << std::setw(4) <<
                 std::right << std::uppercase << std::hex << int(mRawValue) << " (Dec:" << std::dec << decStr << ")";

    return output.str();
}

Int32_Num::Int32_Num(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToInt32(start_position);
}

std::string Int32_Num::GetAsString() {
    std::ostringstream output;

    std::string decStr = std::to_string(mRawValue);

    output << mName << ": 0x" << std::setfill('0') << std::setw(8) <<
                 std::right << std::uppercase << std::hex << int(mRawValue) << " (Dec:" << std::dec << decStr << ")";

    return output.str();
}

FixedPointInt16_8R8Num::FixedPointInt16_8R8Num(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToInt16(start_position);
    mFloatValue = mParent->FixedPointToFloat8D8(mRawValue);
}

std::string FixedPointInt16_8R8Num::GetAsString() {
    std::ostringstream output;

    output << mName << ": 0x" << std::setfill('0') << std::setw(4) <<
                 std::right << std::uppercase << std::hex << mRawValue << " (" << mFloatValue << "f)";

    return output.str();
}

FixedPointInt32_8R8Num::FixedPointInt32_8R8Num(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToInt32(start_position);
    mFloatValue = mParent->FixedPointToFloat8D8(static_cast<int16_t>(mRawValue));
}

std::string FixedPointInt32_8R8Num::GetAsString() {
    std::ostringstream output;

    output << mName << ": 0x" << std::setfill('0') << std::setw(8) <<
                 std::right << std::uppercase << std::hex << mRawValue << " (" << mFloatValue << "f)";

    return output.str();
}

FixedPointInt16_8R8NumAngle::FixedPointInt16_8R8NumAngle(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToInt16(start_position);
    mFloatValue = mParent->VanillaRawAngleToMyFloatingAngle(mRawValue);
}

std::string FixedPointInt16_8R8NumAngle::GetAsString() {
    std::ostringstream output;

    output << mName << ": 0x" << std::setfill('0') << std::setw(4) <<
                 std::right << std::uppercase << std::hex << mRawValue << " (" << mFloatValue << "f °)";

    return output.str();
}

FixedPointInt32_8R8NumAngle::FixedPointInt32_8R8NumAngle(DataTools* parent, std::string name, size_t start_position) {
    mParent = parent;
    mName = name;
    mRawValue = mParent->ConvertByteArray_ToInt32(start_position);
    mFloatValue = mParent->VanillaRawAngleToMyFloatingAngle(static_cast<int16_t>(mRawValue));
}

std::string FixedPointInt32_8R8NumAngle::GetAsString() {
    std::ostringstream output;

    output << mName << ": 0x" << std::setfill('0') << std::setw(8) <<
                 std::right << std::uppercase << std::hex << mRawValue << " (" << mFloatValue << "f °)";

    return output.str();
}

DataTools::DataTools(MemDump* parentDump) {
    mParentDump = parentDump;
}

DataTools::~DataTools() {
}

UInt8_Num* DataTools::AddUInt8_NumVar(std::string name, size_t start_position) {
    UInt8_Num* newVar = new UInt8_Num(this, name, start_position);
    return newVar;
}

UInt16_Num* DataTools::AddUInt16_NumVar(std::string name, size_t start_position) {
    UInt16_Num* newVar = new UInt16_Num(this, name, start_position);
    return newVar;
}

UInt32_Num* DataTools::AddUInt32_NumVar(std::string name, size_t start_position) {
    UInt32_Num* newVar = new UInt32_Num(this, name, start_position);
    return newVar;
}

Int8_Num* DataTools::AddInt8_NumVar(std::string name, size_t start_position) {
    Int8_Num* newVar = new Int8_Num(this, name, start_position);
    return newVar;
}

Int16_Num* DataTools::AddInt16_NumVar(std::string name, size_t start_position) {
    Int16_Num* newVar = new Int16_Num(this, name, start_position);
    return newVar;
}

Int32_Num* DataTools::AddInt32_NumVar(std::string name, size_t start_position) {
    Int32_Num* newVar = new Int32_Num(this, name, start_position);
    return newVar;
}

FixedPointInt16_8R8Num* DataTools::AddFixedPointInt16_8R8NumVar(std::string name, size_t start_position) {
    FixedPointInt16_8R8Num* newVar = new FixedPointInt16_8R8Num(this, name, start_position);
    return newVar;
}

FixedPointInt32_8R8Num* DataTools::AddFixedPointInt32_8R8NumVar(std::string name, size_t start_position) {
    FixedPointInt32_8R8Num* newVar = new FixedPointInt32_8R8Num(this, name, start_position);
    return newVar;
}

FixedPointInt16_8R8NumAngle* DataTools::AddFixedPointInt16_8R8NumVarAngle(std::string name, size_t start_position) {
    FixedPointInt16_8R8NumAngle* newVar = new FixedPointInt16_8R8NumAngle(this, name, start_position);
    return newVar;
}

FixedPointInt32_8R8NumAngle* DataTools::AddFixedPointInt32_8R8NumVarAngle(std::string name, size_t start_position) {
    FixedPointInt32_8R8NumAngle* newVar = new FixedPointInt32_8R8NumAngle(this, name, start_position);
    return newVar;
}

//Convert a fixed point number (8.8 format) to a float
float DataTools::FixedPointToFloat8D8(int16_t fixedPntVal) {
    int8_t intPart = static_cast<int8_t>((fixedPntVal >> 8) & 0x00FF);
    float result = (float)(intPart);
    result += (fixedPntVal & 0x00FF) / 256.0f;
    return result;
}

//original "raw" angles (for example in thing->Movement.Angle.ZY, is struct Angle)
//contain the absolute angle for a unit circle of 256.0, in fixed point
//arithmetic format of 8.8
//this function simply converts this angle into a floating point angle value
//with a unit circle of 360.0f I use in my project and Irrlicht
float DataTools::VanillaRawAngleToMyFloatingAngle(uint16_t rawVanillaAngle) {
   uint16_t val = ((uint16_t)(rawVanillaAngle) & 0xFF00) >> 8;
   float result = (float)(val);

   val = ((uint16_t)(rawVanillaAngle) & 0x00FF);
   result += (float)(val) / 256.0f;

   result = (result / 256.0f) * 360.0f;

   if (result > 360.0f) {
       result -= 360.0f;
   }

   if (result < 0.0f) {
       result += 360.0f;
   }

   return result;
}

//contain the absolute angle for a unit circle of 256.0, in fixed point
//arithmetic format of 16.16
//this function simply converts this angle into a floating point angle value
//with a unit circle of 360.0f I use in my project and Irrlicht
float DataTools::VanillaRawAngle32BitsToMyFloatingAngle(uint32_t rawVanillaAngle) {
   uint32_t val = ((uint32_t)(rawVanillaAngle) & 0xFFFF0000) >> 16;
   float result = (float)(val);

   val = ((uint32_t)(rawVanillaAngle) & 0x0000FFFF);
   result += (float)(val) / 65536.0f;

   result = (result / 256.0f) * 360.0f;

   if (result > 360.0f) {
       result -= 360.0f;
   }

   if (result < 0.0f) {
       result += 360.0f;
   }

   return result;
}

int8_t DataTools::ConvertByteArray_ToInt8(size_t start_position) {
    int8_t result;

    result = static_cast<int8_t>(mParentDump->mMemDumpData->mData->at(start_position));
    return (result);
}

int16_t DataTools::ConvertByteArray_ToInt16(size_t start_position) {
    int16_t result;

    result = static_cast<int16_t>((mParentDump->mMemDumpData->mData->at(start_position+1) << 8) +
                                  mParentDump->mMemDumpData->mData->at(start_position));
    return (result);
}

int32_t DataTools::ConvertByteArray_ToInt32(size_t start_position) {
    int32_t result;

    result = static_cast<int32_t>(((mParentDump->mMemDumpData->mData->at(start_position+3) << 24)
                                   + (mParentDump->mMemDumpData->mData->at(start_position+2) << 16) +
                                   mParentDump->mMemDumpData->mData->at(start_position+1) << 8) +
                                  mParentDump->mMemDumpData->mData->at(start_position));
    return (result);
}

uint16_t DataTools::ConvertByteArray_ToUInt16(size_t start_position) {
    uint16_t result;

    result = static_cast<uint16_t>((mParentDump->mMemDumpData->mData->at(start_position+1) << 8) +
                                   mParentDump->mMemDumpData->mData->at(start_position));
    return (result);
}

uint8_t DataTools::ConvertByteArray_ToUInt8(size_t start_position) {
    uint8_t result;

    result = static_cast<uint8_t>(mParentDump->mMemDumpData->mData->at(start_position));
    return (result);
}

uint32_t DataTools::ConvertByteArray_ToUInt32(size_t start_position) {
    uint32_t result;

    result = static_cast<uint32_t>(((mParentDump->mMemDumpData->mData->at(start_position+3) << 24) +
                                    (mParentDump->mMemDumpData->mData->at(start_position+2) << 16) +
                                   mParentDump->mMemDumpData->mData->at(start_position+1) << 8) +
                                   mParentDump->mMemDumpData->mData->at(start_position));
    return (result);
}
