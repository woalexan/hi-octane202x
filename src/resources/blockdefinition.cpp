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

BlockDefinition::BlockDefinition(int id, int offset, std::vector<uint8_t> bytes)  {
   this->m_ID = id;
   this->m_Bytes = bytes;
   this->m_Offset = offset;

   mN = decode_N();
   mE = decode_E();
   mS = decode_S();
   mW = decode_W();
   mT = decode_T();
   mB = decode_B();

   mNMod = decode_NMod();
   mEMod = decode_EMod();
   mSMod = decode_SMod();
   mWMod = decode_WMod();
   mTMod = decode_TMod();
   mBMod = decode_BMod();

   //for debugging of level saving, comment out later
   this->m_wBytes.resize(this->m_Bytes.size());
   std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);
}

BlockDefinition::~BlockDefinition() {
}

bool BlockDefinition::WriteChanges() {
   save_N(mN);
   save_E(mE);
   save_S(mS);
   save_W(mW);
   save_T(mT);
   save_B(mB);

   save_NMod(mNMod);
   save_EMod(mEMod);
   save_SMod(mSMod);
   save_WMod(mWMod);
   save_TMod(mTMod);
   save_BMod(mBMod);

   return(true);
}

uint8_t BlockDefinition::decode_N() {
   return(m_Bytes.at(0));
}

uint8_t BlockDefinition::decode_E() {
   return(m_Bytes.at(1));
}

uint8_t BlockDefinition::decode_S() {
   return(m_Bytes.at(2));
}

uint8_t BlockDefinition::decode_W() {
   return(m_Bytes.at(3));
}

uint8_t BlockDefinition::decode_T() {
    return(m_Bytes.at(4));
}

uint8_t BlockDefinition::decode_B() {
    return(m_Bytes.at(5));
}

uint8_t BlockDefinition::decode_NMod() {
    return(m_Bytes.at(6));
}

uint8_t BlockDefinition::decode_EMod() {
    return(m_Bytes.at(7));
}

uint8_t BlockDefinition::decode_SMod() {
    return(m_Bytes.at(8));
}

uint8_t BlockDefinition::decode_WMod() {
    return(m_Bytes.at(9));
}

uint8_t BlockDefinition::decode_TMod() {
    return(m_Bytes.at(10));
}

uint8_t BlockDefinition::decode_BMod() {
    return(m_Bytes.at(11));
}

void BlockDefinition::set_N(int newVal) {
  mN = static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_E(int newVal) {
  mE = static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_S(int newVal) {
  mS = static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_W(int newVal) {
  mW = static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_T(int newVal) {
  mT = static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_B(int newVal) {
  mB = static_cast<uint8_t>(newVal);
}

void BlockDefinition::setNMod(int newVal) {
  mNMod = static_cast<uint8_t>(newVal);
}

void BlockDefinition::setEMod(int newVal) {
  mEMod = static_cast<uint8_t>(newVal);
}

void BlockDefinition::setSMod(int newVal) {
  mSMod = static_cast<uint8_t>(newVal);
}

void BlockDefinition::setWMod(int newVal) {
  mWMod = static_cast<uint8_t>(newVal);
}

void BlockDefinition::setTMod(int newVal) {
  mTMod = static_cast<uint8_t>(newVal);
}

void BlockDefinition::setBMod(int newVal) {
  mBMod = static_cast<uint8_t>(newVal);
}

uint8_t BlockDefinition::get_N() {
   return(mN);
}

uint8_t BlockDefinition::get_E() {
   return(mE);
}

uint8_t BlockDefinition::get_S() {
   return(mS);
}

uint8_t BlockDefinition::get_W() {
   return(mW);
}

uint8_t BlockDefinition::get_T() {
    return(mT);
}

uint8_t BlockDefinition::get_B() {
    return(mB);
}

uint8_t BlockDefinition::NMod() {
    return(mNMod);
}

uint8_t BlockDefinition::EMod() {
    return(mEMod);
}

uint8_t BlockDefinition::SMod() {
    return(mSMod);
}

uint8_t BlockDefinition::WMod() {
    return(mWMod);
}

uint8_t BlockDefinition::TMod() {
    return(mTMod);
}

uint8_t BlockDefinition::BMod() {
    return(mBMod);
}

void BlockDefinition::save_N(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(0) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_E(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(1) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_S(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(2) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_W(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(3) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_T(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(4) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_B(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(5) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_NMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(6) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_EMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(7) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_SMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(8) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_WMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(9) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_TMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(10) = (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_BMod(int newVal) {
   uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(11) = (uint8_t)(inVal & 0x00FF);
}
