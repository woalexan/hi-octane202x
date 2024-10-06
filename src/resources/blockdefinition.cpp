/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "blockdefinition.h"

BlockDefinition::BlockDefinition(int id, int offset, std::vector<unsigned char> bytes)  {
   this->m_ID = id;
   this->m_Bytes = bytes;
   this->m_Offset = offset;

   //for debugging of level saving, comment out later
   this->m_wBytes.resize(this->m_Bytes.size());
   std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);
}

BlockDefinition::~BlockDefinition() {
}

bool BlockDefinition::WriteChanges() {
   return(false);
}

int BlockDefinition::get_N() {
   return(m_Bytes.at(0));
}

int BlockDefinition::get_E() {
   return(m_Bytes.at(1));
}

int BlockDefinition::get_S() {
   return(m_Bytes.at(2));
}

int BlockDefinition::get_W() {
   return(m_Bytes.at(3));
}

int BlockDefinition::get_T() {
    return(m_Bytes.at(4));
}

int BlockDefinition::get_B() {
    return(m_Bytes.at(5));
}

int BlockDefinition::NMod() {
    return(m_Bytes.at(6));
}

int BlockDefinition::EMod() {
    return(m_Bytes.at(7));
}

int BlockDefinition::SMod() {
    return(m_Bytes.at(8));
}

int BlockDefinition::WMod() {
    return(m_Bytes.at(9));
}

int BlockDefinition::TMod() {
    return(m_Bytes.at(10));
}

int BlockDefinition::BMod() {
    return(m_Bytes.at(11));
}

void BlockDefinition::set_N(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(0) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::set_E(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(1) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::set_S(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(2) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::set_W(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(3) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::set_T(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(4) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::set_B(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(5) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::setNMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(6) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::setEMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(7) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::setSMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(8) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::setWMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(9) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::setTMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(10) = (unsigned char)(inVal & 0x00FF);
}

void BlockDefinition::setBMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(11) = (unsigned char)(inVal & 0x00FF);
}
