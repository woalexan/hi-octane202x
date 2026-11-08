/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef DATATOOLS_H
#define DATATOOLS_H

#include <cstdint>
#include <stdlib.h>
#include <string>

class MemDump;
class DataTools;

class UInt8_Num {
public:
    UInt8_Num(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    uint8_t mRawValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
};

class UInt16_Num {
public:
    UInt16_Num(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    uint16_t mRawValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
};

class UInt32_Num {
public:
    UInt32_Num(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    uint32_t mRawValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
};

class Int8_Num {
public:
    Int8_Num(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();
    int8_t GetRawValue();

    int8_t mRawValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
};

class Int16_Num {
public:
    Int16_Num(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    int16_t mRawValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
};

class Int32_Num {
public:
    Int32_Num(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    int32_t mRawValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
};

class FixedPointInt16_8R8Num {
public:
    FixedPointInt16_8R8Num(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    float mFloatValue;
    int16_t mRawValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
};

class FixedPointInt32_8R8Num {
public:
    FixedPointInt32_8R8Num(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    float mFloatValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
    int32_t mRawValue;
};

class FixedPointInt16_8R8NumAngle {
public:
    FixedPointInt16_8R8NumAngle(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    float mFloatValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
    int16_t mRawValue;
};

class FixedPointInt32_8R8NumAngle {
public:
    FixedPointInt32_8R8NumAngle(DataTools* parent, std::string name, size_t start_position);

    std::string GetAsString();

    float mFloatValue;

private:
    std::string mName;
    DataTools* mParent = nullptr;
    int32_t mRawValue;
};

class DataTools
{
public:
    DataTools(MemDump* parentDump);
    ~DataTools();

    int32_t ConvertByteArray_ToInt32(size_t start_position);
    int16_t ConvertByteArray_ToInt16(size_t start_position);
    int8_t ConvertByteArray_ToInt8(size_t start_position);
    uint8_t ConvertByteArray_ToUInt8(size_t start_position);
    uint32_t ConvertByteArray_ToUInt32(size_t start_position);
    uint16_t ConvertByteArray_ToUInt16(size_t start_position);

    //Convert a fixed point number (8.8 format) to a float
    float FixedPointToFloat8D8(int16_t fixedPntVal);

    UInt8_Num* AddUInt8_NumVar(std::string name, size_t start_position);
    UInt16_Num* AddUInt16_NumVar(std::string name, size_t start_position);
    UInt32_Num* AddUInt32_NumVar(std::string name, size_t start_position);
    Int8_Num* AddInt8_NumVar(std::string name, size_t start_position);
    Int16_Num* AddInt16_NumVar(std::string name, size_t start_position);
    Int32_Num* AddInt32_NumVar(std::string name, size_t start_position);

    FixedPointInt16_8R8Num* AddFixedPointInt16_8R8NumVar(std::string name, size_t start_position);
    FixedPointInt32_8R8Num* AddFixedPointInt32_8R8NumVar(std::string name, size_t start_position);

    FixedPointInt16_8R8NumAngle* AddFixedPointInt16_8R8NumVarAngle(std::string name, size_t start_position);
    FixedPointInt32_8R8NumAngle* AddFixedPointInt32_8R8NumVarAngle(std::string name, size_t start_position);

    //original "raw" angles (for example in thing->Movement.Angle.ZY, is struct Angle)
    //contain the absolute angle in current steps for a unit circle of 65536 steps
    //this function simply converts this angle into a floating point angle value
    //with a unit circle of 360.0f I use in Irrlicht
    float VanillaRawAngleToMyFloatingAngle(uint16_t rawVanillaAngle);
    float VanillaRawAngle32BitsToMyFloatingAngle(uint32_t rawVanillaAngle);

private:
    MemDump* mParentDump = nullptr;
};

#endif // DATATOOLS_H
