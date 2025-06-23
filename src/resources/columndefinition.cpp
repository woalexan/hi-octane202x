/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "columndefinition.h"
#include "../utils/crc32.h"

ColumnDefinition::ColumnDefinition(int id, int offset, std::vector<uint8_t> bytes) {
  this->m_ID = id;
  this->m_Bytes = bytes;
  this->m_Offset = offset;

  //for debugging of level saving, comment out later
  this->m_wBytes.resize(this->m_Bytes.size());
  std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);

  //each columndefinition is 26 bytes long
  //Byte 0:   Shape
  //Byte 1:   Unknown4
  //Byte 2:   Floor Texture ID
  //Byte 3:   Floor Texture ID
  //Byte 4:   Unknown1
  //Byte 5:   Unknown1
  //Byte 6:   A
  //Byte 7:   A
  //Byte 8:   B
  //Byte 9:   B
  //Byte 10:  C
  //Byte 11:  C
  //Byte 12:  D
  //Byte 13:  D
  //Byte 14:  E
  //Byte 15:  E
  //Byte 16:  F
  //Byte 17:  F
  //Byte 18:  G
  //Byte 19:  G
  //Byte 20:  H
  //Byte 21:  H
  //Byte 22:  Unknown2
  //Byte 23:  Unknown2
  //Byte 24:  Unknown3
  //Byte 25:  Unknown3

  mShape = decode_Shape();
  mFloorTextureID = decode_FloorTextureID();

  mA = decode_A();
  mB = decode_B();
  mC = decode_C();
  mD = decode_D();
  mE = decode_E();
  mF = decode_F();
  mG = decode_G();
  mH = decode_H();

  mUnknown1 = decode_Unknown1();
  mUnknown2 = decode_Unknown2();
  mUnknown3 = decode_Unknown3();
  mUnknown4 = this->m_Bytes.at(1);

  //we start from lowest block of colum upwards
  //first collision is active, the first time we find a gap in the
  //column going upwards (which means there is no block) we will disable collision
  //for all blocks found above
  //if we do this we make sure that the roof of tunnels, etc... is not part of the collision detection
  //and so the player can not get stuck in the roof etc.
  //we only add as much elements to the vector as there are blocks existing in the column
  //so that the indices for for loop in next data handling step in levelblocks.cpp are
  //correct
  int collisionActive = 1;
  mInCollisionMesh.clear();

  if (get_A() == 0) {
      collisionActive = 0;
  } else {
      mInCollisionMesh.push_back(collisionActive);
  }

  if (get_B() == 0) {
      collisionActive = 0;
  } else {
      mInCollisionMesh.push_back(collisionActive);
  }

  if (get_C() == 0) {
      collisionActive = 0;
  } else {
    mInCollisionMesh.push_back(collisionActive);
  }

  if (get_D() == 0) {
      collisionActive = 0;
  } else {
    mInCollisionMesh.push_back(collisionActive);
  }

  if (get_E() == 0) {
      collisionActive = 0;
  } else {
   mInCollisionMesh.push_back(collisionActive);
  }

  if (get_F() == 0) {
      collisionActive = 0;
  } else {
    mInCollisionMesh.push_back(collisionActive);
  }

  if (get_G() == 0) {
      collisionActive = 0;
  } else {
    mInCollisionMesh.push_back(collisionActive);
  }

  if (get_H() == 0) {
      collisionActive = 0;
  } else {
    mInCollisionMesh.push_back(collisionActive);
  }
}

ColumnDefinition::~ColumnDefinition() {
}

uint8_t ColumnDefinition::get_Shape() {
    return mShape;
}

int16_t ColumnDefinition::get_FloorTextureID() {
   return mFloorTextureID;
}

int16_t ColumnDefinition::get_Unknown1() {
   return mUnknown1;
}

int16_t ColumnDefinition::get_A() {
   return mA;
}

int16_t ColumnDefinition::get_B() {
   return mB;
}

int16_t ColumnDefinition::get_C() {
   return mC;
}

int16_t ColumnDefinition::get_D() {
   return mD;
}

int16_t ColumnDefinition::get_E() {
   return mE;
}

int16_t ColumnDefinition::get_F() {
   return mF;
}

int16_t ColumnDefinition::get_G() {
   return mG;
}

int16_t ColumnDefinition::get_H() {
   return mH;
}

int16_t ColumnDefinition::get_Unknown2() {
   return mUnknown2;
}

int16_t ColumnDefinition::get_Unknown3() {
   return mUnknown3;
}

uint8_t ColumnDefinition::decode_Shape() {
    return this->m_Bytes.at(0);
}

int16_t ColumnDefinition::decode_FloorTextureID() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 2);
}

int16_t ColumnDefinition::decode_Unknown1() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 4);
}

int16_t ColumnDefinition::decode_A() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 6);
}

int16_t ColumnDefinition::decode_B() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 8);
}

int16_t ColumnDefinition::decode_C() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 10);
}

int16_t ColumnDefinition::decode_D() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 12);
}

int16_t ColumnDefinition::decode_E() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 14);
}

int16_t ColumnDefinition::decode_F() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 16);
}

int16_t ColumnDefinition::decode_G() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 18);
}

int16_t ColumnDefinition::decode_H() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 20);
}

int16_t ColumnDefinition::decode_Unknown2() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 22);
}

int16_t ColumnDefinition::decode_Unknown3() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 24);
}

//the following functions are for the LevelEditor functionality
void ColumnDefinition::set_Shape(uint8_t newVal) {
   mShape = newVal;
}

void ColumnDefinition::set_FloorTextureID(int16_t newVal) {
   mFloorTextureID = newVal;
}

void ColumnDefinition::set_Unknown1(int16_t newVal) {
   mUnknown1 = newVal;
}

void ColumnDefinition::set_A(int16_t newVal) {
   mA = newVal;
}

void ColumnDefinition::set_B(int16_t newVal) {
   mB = newVal;
}

void ColumnDefinition::set_C(int16_t newVal) {
   mC = newVal;
}

void ColumnDefinition::set_D(int16_t newVal) {
   mD = newVal;
}

void ColumnDefinition::set_E(int16_t newVal) {
   mE = newVal;
}

void ColumnDefinition::set_F(int16_t newVal) {
   mF = newVal;
}

void ColumnDefinition::set_G(int16_t newVal) {
   mG = newVal;
}

void ColumnDefinition::set_H(int16_t newVal) {
   mH = newVal;
}

void ColumnDefinition::set_Unknown2(int16_t newVal) {
   mUnknown2 = newVal;
}

void ColumnDefinition::set_Unknown3(int16_t newVal) {
   mUnknown3 = newVal;
}

bool ColumnDefinition::WriteChanges() {
    //store shape
    this->m_wBytes.at(0) = static_cast<uint8_t>(mShape);

    //store FloorTextureID
    ConvertAndWriteInt16ToByteArray(mFloorTextureID, this->m_wBytes, 2);

    //store unknown 1 field
    ConvertAndWriteInt16ToByteArray(mUnknown1, this->m_wBytes, 4);

    //store unknown 2 field
    ConvertAndWriteInt16ToByteArray(mUnknown2, this->m_wBytes, 22);

    //store unknown 3 field
    ConvertAndWriteInt16ToByteArray(mUnknown3, this->m_wBytes, 24);

    //store blocks A up to H
    ConvertAndWriteInt16ToByteArray(mA, this->m_wBytes, 6);
    ConvertAndWriteInt16ToByteArray(mB, this->m_wBytes, 8);
    ConvertAndWriteInt16ToByteArray(mC, this->m_wBytes, 10);
    ConvertAndWriteInt16ToByteArray(mD, this->m_wBytes, 12);
    ConvertAndWriteInt16ToByteArray(mE, this->m_wBytes, 14);
    ConvertAndWriteInt16ToByteArray(mF, this->m_wBytes, 16);
    ConvertAndWriteInt16ToByteArray(mG, this->m_wBytes, 18);
    ConvertAndWriteInt16ToByteArray(mH, this->m_wBytes, 20);

    //write unknown4 data
    this->m_wBytes.at(1) = static_cast<uint8_t>(mUnknown4);

    return (true);
}
