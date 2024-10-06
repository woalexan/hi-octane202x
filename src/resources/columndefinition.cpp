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

ColumnDefinition::ColumnDefinition(int id, int offset, std::vector<unsigned char> bytes) {
  this->m_ID = id;
  this->m_Bytes = bytes;
  this->m_Offset = offset;

  //for debugging of level saving, comment out later
  this->m_wBytes.resize(this->m_Bytes.size());
  std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);

  /*std::vector<int> new_blocks;

  new_blocks.clear();
  new_blocks.push_back(this->get_A());
  new_blocks.push_back(this->get_B());
  new_blocks.push_back(this->get_C());
  new_blocks.push_back(this->get_D());
  new_blocks.push_back(this->get_E());
  new_blocks.push_back(this->get_F());
  new_blocks.push_back(this->get_G());
  new_blocks.push_back(this->get_H());*/

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

unsigned char ColumnDefinition::get_Shape() {
    return this->m_Bytes.at(0);
}

std::vector<int> ColumnDefinition::get_Blocks() {
    return(this->m_Blocks);
}

void ColumnDefinition::set_Blocks(std::vector<int> new_blocks) {
    this->m_Blocks = new_blocks;
}

int ColumnDefinition::get_FloorTextureID() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 2);
}

int ColumnDefinition::get_Unknown1() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 4);
}

int ColumnDefinition::get_A() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 6);
}

int ColumnDefinition::get_B() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 8);
}

int ColumnDefinition::get_C() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 10);
}

int ColumnDefinition::get_D() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 12);
}

int ColumnDefinition::get_E() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 14);
}

int ColumnDefinition::get_F() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 16);
}

int ColumnDefinition::get_G() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 18);
}

int ColumnDefinition::get_H() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 20);
}

int ColumnDefinition::get_Unknown2() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 22);
}

int ColumnDefinition::get_Unknown3() {
   return ConvertByteArray_ToInt16(this->m_Bytes, 24);
}

//the following functions are for the LevelEditor functionality
void ColumnDefinition::set_Shape(unsigned char newVal) {
    this->m_wBytes.at(0) = newVal;
}

void ColumnDefinition::set_FloorTextureID(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 2);
}

void ColumnDefinition::set_Unknown1(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 4);
}

void ColumnDefinition::set_A(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 6);
}

void ColumnDefinition::set_B(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 8);
}

void ColumnDefinition::set_C(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 10);
}

void ColumnDefinition::set_D(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 12);
}

void ColumnDefinition::set_E(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 14);
}

void ColumnDefinition::set_F(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 16);
}

void ColumnDefinition::set_G(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 18);
}

void ColumnDefinition::set_H(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 20);
}

void ColumnDefinition::set_Unknown2(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 22);
}

void ColumnDefinition::set_Unknown3(int newVal) {
   ConvertAndWriteInt16ToByteArray(newVal, this->m_wBytes, 24);
}

bool ColumnDefinition::WriteChanges() {
    this->set_A(this->m_Blocks.at(0));
    this->set_B(this->m_Blocks.at(1));
    this->set_C(this->m_Blocks.at(2));
    this->set_D(this->m_Blocks.at(3));
    this->set_E(this->m_Blocks.at(4));
    this->set_F(this->m_Blocks.at(5));
    this->set_G(this->m_Blocks.at(6));
    this->set_H(this->m_Blocks.at(7));

    return (true);
}
