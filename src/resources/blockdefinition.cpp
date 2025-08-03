/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "blockdefinition.h"
#include "../utils/crc32.h"

BlockDefinition::BlockDefinition(int id, int offset, std::vector<uint8_t> bytes)  {
   this->m_ID = id;
   this->m_Bytes = bytes;
   this->m_Offset = offset;

   //11.07.2025: East seems to be swapped
   //with West when comparing loaded map with
   //original game
   //each blockDefinition is 16 bytes long
   //Byte 0:   N
   //Byte 1:   W
   //Byte 2:   S
   //Byte 3:   E
   //Byte 4:   T
   //Byte 5:   B
   //Byte 6:   NMod  (19.07.2025: does not seem to work in original game, reason unknown, bug in game that was never fixed?)
   //Byte 7:   WMod  (19.07.2025: does not seem to work in original game, reason unknown, bug in game that was never fixed?)
   //Byte 8:   SMod  (works as expected)
   //Byte 9:   EMod  (19.07.2025: does not seem to work in original game, reason unknown, bug in game that was never fixed?)
   //Byte 10:  TMod  (works as expected)
   //Byte 11:  BMod  (works as expected)
   //Byte 12:  Unknown1 (TODO: this value seems to be used, but I do not known for what exactly right now)
   //Byte 13:  Unknown1
   //Byte 14:  Unknown2 (in all 9 existing maps there is not a single place where this value
   //Byte 15:  Unknown2  is not zero; seems to be reserved for the future, but was never used)

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

   //read unknown data
   mUnknown1 = ConvertByteArray_ToInt16(bytes, 12);
   mUnknown2 = ConvertByteArray_ToInt16(bytes, 14);

   //for debugging of level saving, comment out later
   this->m_wBytes.resize(this->m_Bytes.size());
   std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);
}

//alternative constructor for usage with the level editor
//to add a new block definition to the level file
//if offset is set to -1 (for example for special column for block preview) writing to
//map file data vector is disabled.
BlockDefinition::BlockDefinition(int id, int offset, uint8_t newN, uint8_t newE, uint8_t newS, uint8_t newW, uint8_t newT, uint8_t newB,
                uint8_t newNMod, uint8_t newEMod, uint8_t newSMod, uint8_t newWMod, uint8_t newTMod, uint8_t newBMod, int16_t newUnknown1, int16_t newUnknown2) {
    this->m_ID = id;
    this->m_Offset = offset;

    mN = newN;
    mE = newE;
    mS = newS;
    mW = newW;
    mT = newT;
    mB = newB;

    mNMod = newNMod;
    mEMod = newEMod;
    mSMod = newSMod;
    mWMod = newWMod;
    mTMod = newTMod;
    mBMod = newBMod;

    mUnknown1 = newUnknown1;
    mUnknown2 = newUnknown2;

    //important: in case a new block definition is added
    //in level editor first mark its state as newly added, so that it is
    //not immediately removed again afterwards because it is not yet assigned to
    //any column definition
    mState = DEF_BLOCKDEF_STATE_NEWLYADDED_KEEP;

    //each blockDefinition is 16 bytes long
    //create empty data vector with 16 zero bytes
    this->m_wBytes.resize(16);
    this->m_Bytes.resize(16);

    std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_wBytes.size(), 0);

    //encode data into m_wBytes vector
    WriteChanges();

    //copy m_wBytes data into m_Bytes data vector
    std::copy(this->m_wBytes.begin(), this->m_wBytes.end(), this->m_Bytes.begin());
}

BlockDefinition::~BlockDefinition() {
}

bool BlockDefinition::WriteChanges() {
   //I have the ability to create "special" block definitions that are
    //used for example for block preview rendering; to prevent that this block definitions are
    //written accidently to the level file low level data, I set offset of this
    //block definitions to -1; In case we find -1 here prevent writting of data!
    if (m_Offset == -1)
        return true;

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
   save_BMod(mBMod);
   save_TMod(mTMod);

   //write unknown data
   ConvertAndWriteInt16ToByteArray(mUnknown1, this->m_wBytes, 12);
   ConvertAndWriteInt16ToByteArray(mUnknown2, this->m_wBytes, 14);

   return(true);
}

uint8_t BlockDefinition::decode_N() {
   return(m_Bytes.at(0));
}

uint8_t BlockDefinition::decode_E() {
    //11.07.2025: East seems to be swapped
    //with West when comparing loaded map with
    //original game
   //return(m_Bytes.at(1));
   return(m_Bytes.at(3));
}

uint8_t BlockDefinition::decode_S() {
   return(m_Bytes.at(2));
}

uint8_t BlockDefinition::decode_W() {
   //return(m_Bytes.at(3));
    //11.07.2025: East seems to be swapped
    //with West when comparing loaded map with
    //original game
   return(m_Bytes.at(1));
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
    //11.07.2025: East seems to be swapped
    //with West when comparing loaded map with
    //original game
    //return(m_Bytes.at(7));
    return(m_Bytes.at(9));
}

uint8_t BlockDefinition::decode_SMod() {
    return(m_Bytes.at(8));
}

uint8_t BlockDefinition::decode_WMod() {
    //11.07.2025: East seems to be swapped
    //with West when comparing loaded map with
    //original game
    //return(m_Bytes.at(9));
    return(m_Bytes.at(7));
}

uint8_t BlockDefinition::decode_TMod() {
    return(m_Bytes.at(10));
}

uint8_t BlockDefinition::decode_BMod() {
    return(m_Bytes.at(11));
}

void BlockDefinition::set_N(uint8_t newVal) {
  mN = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_E(uint8_t newVal) {
  mE = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_S(uint8_t newVal) {
  mS = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_W(uint8_t newVal) {
  mW = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_T(uint8_t newVal) {
  mT = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::set_B(uint8_t newVal) {
  mB = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::setNMod(uint8_t newVal) {
  mNMod = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::setEMod(uint8_t newVal) {
  mEMod = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::setSMod(uint8_t newVal) {
  mSMod = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::setWMod(uint8_t newVal) {
  mWMod = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::setTMod(uint8_t newVal) {
  mTMod = newVal; //static_cast<uint8_t>(newVal);
}

void BlockDefinition::setBMod(uint8_t newVal) {
  mBMod =  newVal; //static_cast<uint8_t>(newVal);
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

uint8_t BlockDefinition::get_NMod() {
    return(mNMod);
}

uint8_t BlockDefinition::get_EMod() {
    return(mEMod);
}

uint8_t BlockDefinition::get_SMod() {
    return(mSMod);
}

uint8_t BlockDefinition::get_WMod() {
    return(mWMod);
}

uint8_t BlockDefinition::get_TMod() {
    return(mTMod);
}

uint8_t BlockDefinition::get_BMod() {
    return(mBMod);
}

int16_t BlockDefinition::get_Unknown1() {
    return (mUnknown1);
}

int16_t BlockDefinition::get_Unknown2() {
    return (mUnknown2);
}

void BlockDefinition::save_N(uint8_t newVal) {
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(0) = newVal; //(uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_E(uint8_t newVal) {
   //11.07.2025: East seems to be swapped
   //with West when comparing loaded map with
   //original game
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   //this->m_wBytes.at(1) = (uint8_t)(inVal & 0x00FF);
    //uint16_t inVal = static_cast<uint16_t>(newVal);
    this->m_wBytes.at(3) = newVal; //(uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_S(uint8_t newVal) {
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(2) = newVal; //(uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_W(uint8_t newVal) {
   //11.07.2025: East seems to be swapped
   //with West when comparing loaded map with
   //original game
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   //this->m_wBytes.at(3) = (uint8_t)(inVal & 0x00FF);
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(1) = newVal; //(uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_T(uint8_t newVal) {
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(4) = newVal; //(uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_B(uint8_t newVal) {
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(5) = newVal; // (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_NMod(uint8_t newVal) {
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(6) = newVal; //(uint8_t)(newVal & 0x00FF);
}

void BlockDefinition::save_EMod(uint8_t newVal) {
   //11.07.2025: East seems to be swapped
   //with West when comparing loaded map with
   //original game
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(9) = newVal; // (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_SMod(uint8_t newVal) {
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(8) = newVal; // (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_WMod(uint8_t newVal) {
   //11.07.2025: East seems to be swapped
   //with West when comparing loaded map with
   //original game
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   //this->m_wBytes.at(9) = (uint8_t)(inVal & 0x00FF);
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(7) = newVal; // (uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_TMod(uint8_t newVal) {
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(10) = newVal; //(uint8_t)(inVal & 0x00FF);
}

void BlockDefinition::save_BMod(uint8_t newVal) {
   //uint16_t inVal = static_cast<uint16_t>(newVal);
   this->m_wBytes.at(11) = newVal; //(uint8_t)(inVal & 0x00FF);
}
