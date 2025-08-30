/*
 The source code in this file was based on/derived from/translated/and afterwards modified from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did first a translation to C++, then modified it)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "entityitem.h"
#include "../utils/crc32.h"
#include "../definitions.h"

EntityItem::EntityItem(int id, int offset, std::vector<uint8_t> bytes) {
   this->m_ID = id;
   this->m_Bytes = bytes;
   this->m_Offset = offset;

   //for debugging of level saving, comment out later
   this->m_wBytes.resize(this->m_Bytes.size());
   std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);

   //each EntityItem entry is 24 bytes long
   //Byte 0:  Type
   //Byte 1:  SubType
   //Byte 2:  Group
   //Byte 3:  Group
   //Byte 4:  TargetGroup
   //Byte 5:  TargetGroup
   //Byte 6:  Unknown 1
   //Byte 7:  Unknown 1
   //Byte 8:  Unknown 2
   //Byte 9:  Unknown 2
   //Byte 10: Unknown 3
   //Byte 11: Unknown 3
   //Byte 12: NextID
   //Byte 13: NextID
   //Byte 14: Value
   //Byte 15: Value
   //Byte 16: X coord
   //Byte 17: X coord
   //Byte 18: Z coord (or Y whatever you want to call the 2nd coordinate in 2D map :) )
   //Byte 19: Z coord (or Y whatever you want to call the 2nd coordinate in 2D map :) )
   //Byte 20: OffsetX
   //Byte 21: OffsetX
   //Byte 22: OffsetY
   //Byte 23: OffsetY

   mRawType = getType();
   mRawSubType = getSubType();
   mGroup = decodeGroup();
   mTargetGroup = decodeTargetGroup();
   mNextId = decodeNextID();
   mValue = decodeValue();

   this->mEntityType = this->identify();

   mOffsetX = decodeOffsetX();
   mOffsetY = decodeOffsetY();

   //truncate position to get
   //cell base X, Y indexes
   mCell.X = (int)(getX());
   mCell.Y = (int)(getZ());

   //define my position
   irr::core::vector3df vecHlp(0.0f, /*-DEF_SEGMENTSIZE * 0.5f*/ DEF_SEGMENTSIZE * 0.5f, 0.0f /*DEF_SEGMENTSIZE * 0.5f*/);
   mCenter = this->getPos() + vecHlp;

   //read unknown data
   mUnknown1 = ConvertByteArray_ToInt16(bytes, 6);
   mUnknown2 = ConvertByteArray_ToInt16(bytes, 8);
   mUnknown3 = ConvertByteArray_ToInt16(bytes, 10);
}

//special constructor that is only used in LevelEditor for creation of new
//EntityItems via the EntityManager
EntityItem::EntityItem(int x, int y, irr::f32 heightTerrain, int id, int offset, Entity::EntityType ofType) {
   this->m_ID = id;
   this->m_Offset = offset;
   this->mEntityType = ofType;

   //first get levelfile type
   //and subtype back
   int8_t type;
   int8_t subtype;

   revIdentify(mEntityType, type, subtype);
   mRawType = type;
   mRawSubType = subtype;

   irr::core::vector3df newCenter;

   newCenter.X = - x * DEF_SEGMENTSIZE - 0.5f * DEF_SEGMENTSIZE;
   newCenter.Y = heightTerrain;
   newCenter.Z = y * DEF_SEGMENTSIZE + 0.5f * DEF_SEGMENTSIZE;

   setCenter(newCenter);

   this->setNextID(0);

   //default set Group to 1 so that the item
   //is immediately visible after level start
   this->setGroup(1);

   this->setValue(0);
   this->setOffsetX(0.0f);
   this->setOffsetY(0.0f);

   //set all unknown values 0 right now
   mUnknown1 = 0;
   mUnknown2 = 0;
   mUnknown3 = 0;

   //each EntityItem is 24 bytes long
   //create empty data vector with 24 zero bytes
   this->m_wBytes.resize(24);
   this->m_Bytes.resize(24);

   std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_wBytes.size(), 0);

   //fill out m_wBytes data
   WriteChanges();

   //copy m_wBytes data into m_Bytes data vector
   std::copy(this->m_wBytes.begin(), this->m_wBytes.end(), this->m_Bytes.begin());
}

EntityItem::~EntityItem() {
}

//special constructor only used for creation
//of InternalTemporaryWaypoint entity items
EntityItem::EntityItem(irr::core::vector3df position) {
    setCenter(position);

    //this entity type is always internalTemporaryWaypoint!
    this->mEntityType = Entity::EntityType::InternalTemporaryWaypoint;
}

void EntityItem::setCenter(irr::core::vector3df newCenter) {
    mCenter = newCenter;

    //recalculate also mCell information
    irr::core::vector3df vecHlp(0.0f, /*-DEF_SEGMENTSIZE * 0.5f*/ DEF_SEGMENTSIZE * 0.5f, 0.0f /*DEF_SEGMENTSIZE * 0.5f*/);
    irr::core::vector3df vecHlp2 =  mCenter - vecHlp;

    //we also need to swap X coordinate again
    //because my Irrlicht coordinate system is mirrored/swapped
    //compared to HiOctane level file

    //truncate position to get
    //cell base X, Y indexes
    mCell.X = (int)(-vecHlp2.X);
    mCell.Y = (int)(vecHlp2.Z);
}

int8_t EntityItem::getType() {
    return(this->m_Bytes.at(0));
}

irr::core::vector2df EntityItem::GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord) {
    threeDCoord = mCenter;

    irr::core::vector2df result(mCenter.X, mCenter.Z);

    return result;
}

int8_t EntityItem::getSubType() {
    return (this->m_Bytes.at(1));
}

int16_t EntityItem::decodeNextID() {
    int16_t result;

    result = static_cast<int16_t>((this->m_Bytes.at(13) << 8) | this->m_Bytes.at(12));
    return (result);
}

int16_t EntityItem::getNextID() {
    return (mNextId);
}

void EntityItem::setNextID(int16_t newNextID) {
    mNextId = newNextID;
}

irr::core::vector3d<float> EntityItem::getPos() {
    //my Irrlicht coordinate system is swapped/mirrored at the x axis compared with the coordinate
    //system in the level file, fix this issue here lowlevel
    irr::core::vector3d<float> result(-this->getX(), this->getY(), this->getZ());
    return(result);
}

int16_t EntityItem::decodeGroup() {
   return(this->m_Bytes.at(2) + this->m_Bytes.at(3) * 256);
}

int16_t EntityItem::getGroup() {
    return mGroup;
}

void EntityItem::setGroup(int16_t newGroup) {
    mGroup = newGroup;
}

int16_t EntityItem::decodeTargetGroup() {
   return(this->m_Bytes.at(4) + this->m_Bytes.at(5) * 256);
}

int16_t EntityItem::getTargetGroup() {
    return (mTargetGroup);
}

void EntityItem::setTargetGroup(int16_t newTargetGroup) {
    mTargetGroup = newTargetGroup;
}

int16_t EntityItem::decodeValue() {
   return(this->m_Bytes.at(14) + this->m_Bytes.at(15) * 256);
}

int16_t EntityItem::getValue() {
    return (mValue);
}

void EntityItem::setValue(int16_t newValue) {
    mValue = newValue;
}

irr::core::vector3df EntityItem::getCenter() {
    return (mCenter);
}

irr::core::vector2di EntityItem::getCell() {
    return (mCell);
}

//this function is used by the levelloader to set the
//correct Y coordinate based on level terrain data
void EntityItem::setY(float newvalue) {
    //initially we have no Y information, until this information
    //is set by the level loader based on the terrain information
    mCenter.Y = newvalue;  // set by level loader
}

float EntityItem::decodeOffsetX() {
   float result = float(this->m_Bytes.at(20)) + float(this->m_Bytes.at(21)) * 256.0f;
   return(result);
}

float EntityItem::decodeOffsetY() {
   float result = float(this->m_Bytes.at(22)) + float(this->m_Bytes.at(23)) * 256.0f;
   return(result);
}

float EntityItem::getX() {
    float result = ((float(this->m_Bytes.at(16)) / float(256)) + float(this->m_Bytes.at(17))) + DEF_SEGMENTSIZE * 0.5f;
    return(result);
}

float EntityItem::getY() {
    //initially we have no Y information, until this information
    //is set by the level loader based on the terrain information
    return(0.0f);
}

float EntityItem::getZ() {
    float result = (float(this->m_Bytes.at(18)) / float(256)) + float(this->m_Bytes.at(19)) + DEF_SEGMENTSIZE * 0.5f;
    return(result);
}

float EntityItem::getOffsetX() {
    return (mOffsetX);
}

float EntityItem::getOffsetY() {
    return (mOffsetY);
}

void EntityItem::setOffsetX(float newOffsetX) {
    mOffsetX = newOffsetX;
}

void EntityItem::setOffsetY(float newOffsetY) {
    mOffsetY = newOffsetY;
}

Entity::EntityType EntityItem::getEntityType() {
    return(this->mEntityType);
}

void EntityItem::setEntityType(Entity::EntityType newEntityType) {
    this->mEntityType = newEntityType;
}

//Only used for writing debugging table file
//do not use otherwise in higher level code!
int8_t EntityItem::getRawType() {
    return mRawType;
}

//Only used for writing debugging table file
//do not use otherwise in higher level code!
int8_t EntityItem::getRawSubType() {
    return mRawSubType;
}

int16_t EntityItem::getUnknown1() {
    return mUnknown1;
}

int16_t EntityItem::getUnknown2() {
    return mUnknown2;
}

int16_t EntityItem::getUnknown3() {
    return mUnknown3;
}

Entity::EntityType EntityItem::identify() {
    if (mRawType == 1 && mRawSubType == 5) return(Entity::EntityType::Checkpoint);

    if (mRawType == 2 && mRawSubType == 1) return(Entity::EntityType::ExplosionParticles); // see level 4
    if (mRawType == 2 && mRawSubType == 2) return(Entity::EntityType::DamageCraft);        // see level 8
    if (mRawType == 2 && mRawSubType == 3) return(Entity::EntityType::Explosion);
    if (mRawType == 2 && mRawSubType == 5) return(Entity::EntityType::SteamStrong);
    if (mRawType == 2 && mRawSubType == 7) return(Entity::EntityType::MorphSource2);
    if (mRawType == 2 && mRawSubType == 8) return(Entity::EntityType::SteamLight);
    if (mRawType == 2 && mRawSubType == 9) return(Entity::EntityType::MorphSource1);
    if (mRawType == 2 && mRawSubType == 16) return(Entity::EntityType::MorphOnce);
    if (mRawType == 2 && mRawSubType == 23) return(Entity::EntityType::MorphPermanent);

    if (mRawType == 3 && mRawSubType == 3) return(Entity::EntityType::Camera);
    if (mRawType == 3 && mRawSubType == 6) return(Entity::EntityType::Cone);

    if (mRawType == 5 && mRawSubType == 0) return(Entity::EntityType::UnknownShieldItem);
    if (mRawType == 5 && mRawSubType == 1) return(Entity::EntityType::UnknownItem);

    if (mRawType == 5 && mRawSubType == 2) return(Entity::EntityType::ExtraShield);
    if (mRawType == 5 && mRawSubType == 3) return(Entity::EntityType::ShieldFull);
    if (mRawType == 5 && mRawSubType == 4) return(Entity::EntityType::DoubleShield);
    if (mRawType == 5 && mRawSubType == 5) return(Entity::EntityType::ExtraAmmo);
    if (mRawType == 5 && mRawSubType == 6) return(Entity::EntityType::AmmoFull);
    if (mRawType == 5 && mRawSubType == 7) return(Entity::EntityType::DoubleAmmo);
    if (mRawType == 5 && mRawSubType == 8) return(Entity::EntityType::ExtraFuel);
    if (mRawType == 5 && mRawSubType == 9) return(Entity::EntityType::FuelFull);
    if (mRawType == 5 && mRawSubType == 10) return(Entity::EntityType::DoubleFuel);
    if (mRawType == 5 && mRawSubType == 11) return(Entity::EntityType::MinigunUpgrade);
    if (mRawType == 5 && mRawSubType == 12) return(Entity::EntityType::MissileUpgrade);
    if (mRawType == 5 && mRawSubType == 13) return(Entity::EntityType::BoosterUpgrade);

    if (mRawType == 8 && mRawSubType == 0) return(Entity::EntityType::TriggerCraft);
    if (mRawType == 8 && mRawSubType == 1) return(Entity::EntityType::TriggerTimed);
    if (mRawType == 8 && mRawSubType == 3) return(Entity::EntityType::TriggerRocket);

    if (mRawType == 9 && mRawSubType == 0) return(Entity::EntityType::WallSegment);

    if (mRawType == 9 && mRawSubType == 1) return(Entity::EntityType::WaypointSlow);
    if (mRawType == 9 && mRawSubType == 2) return(Entity::EntityType::WaypointFuel);
    if (mRawType == 9 && mRawSubType == 3) return(Entity::EntityType::WaypointAmmo);
    if (mRawType == 9 && mRawSubType == 4) return(Entity::EntityType::WaypointShield);
    if (mRawType == 9 && mRawSubType == 6) return(Entity::EntityType::WaypointShortcut);
    if (mRawType == 9 && mRawSubType == 7) return(Entity::EntityType::WaypointSpecial1);
    if (mRawType == 9 && mRawSubType == 8) return(Entity::EntityType::WaypointSpecial2);
    if (mRawType == 9 && mRawSubType == 9) return(Entity::EntityType::WaypointFast);
    if (mRawType == 9 && mRawSubType == 10) return(Entity::EntityType::WaypointSpecial3);

    if (mRawType == 10 && mRawSubType == 9) return(Entity::EntityType::RecoveryTruck);

    return(Entity::EntityType::Unknown);
}

void EntityItem::revIdentify(Entity::EntityType newEntityType, int8_t &newType, int8_t &newSubType) {
   switch (newEntityType) {
       case Entity::EntityType::Checkpoint: {
           newType = 1; newSubType = 5; break;
       }

       case Entity::EntityType::ExplosionParticles: {   // see level 4
               newType = 2; newSubType = 1; break;
           }

       case Entity::EntityType::DamageCraft: {   // see level 8
               newType = 2; newSubType = 2; break;
           }

       case Entity::EntityType::Explosion: {
               newType = 2; newSubType = 3; break;
           }

       case Entity::EntityType::SteamStrong: {
               newType = 2; newSubType = 5; break;
           }

       case Entity::EntityType::MorphSource2: {
               newType = 2; newSubType = 7; break;
           }

       case Entity::EntityType::SteamLight: {
               newType = 2; newSubType = 8; break;
           }

       case Entity::EntityType::MorphSource1: {
               newType = 2; newSubType = 9; break;
           }

       case Entity::EntityType::MorphOnce: {
               newType = 2; newSubType = 16; break;
           }

       case Entity::EntityType::MorphPermanent: {
               newType = 2; newSubType = 23; break;
           }

       case Entity::EntityType::Camera: {
               newType = 3; newSubType = 3; break;
           }

       case Entity::EntityType::Cone: {
               newType = 3; newSubType = 6; break;
           }

       case Entity::EntityType::UnknownShieldItem: {
               newType = 5; newSubType = 0; break;
           }

       case Entity::EntityType::UnknownItem: {
               newType = 5; newSubType = 1; break;
           }

       case Entity::EntityType::ExtraShield: {
               newType = 5; newSubType = 2; break;
           }

       case Entity::EntityType::ShieldFull: {
               newType = 5; newSubType = 3; break;
           }

       case Entity::EntityType::DoubleShield: {
               newType = 5; newSubType = 4; break;
           }

       case Entity::EntityType::ExtraAmmo: {
               newType = 5; newSubType = 5; break;
           }

       case Entity::EntityType::AmmoFull: {
               newType = 5; newSubType = 6; break;
           }

       case Entity::EntityType::DoubleAmmo: {
               newType = 5; newSubType = 7; break;
           }

       case Entity::EntityType::ExtraFuel: {
               newType = 5; newSubType = 8; break;
           }

       case Entity::EntityType::FuelFull: {
               newType = 5; newSubType = 9; break;
           }

       case Entity::EntityType::DoubleFuel: {
               newType = 5; newSubType = 10; break;
           }

       case Entity::EntityType::MinigunUpgrade: {
               newType = 5; newSubType = 11; break;
           }

       case Entity::EntityType::MissileUpgrade: {
               newType = 5; newSubType = 12; break;
           }

       case Entity::EntityType::BoosterUpgrade: {
               newType = 5; newSubType = 13; break;
           }

       case Entity::EntityType::TriggerCraft: {
               newType = 8; newSubType = 0; break;
           }

       case Entity::EntityType::TriggerTimed: {
               newType = 8; newSubType = 1; break;
           }

       case Entity::EntityType::TriggerRocket: {
               newType = 8; newSubType = 3; break;
           }

       case Entity::EntityType::WallSegment: {
               newType = 9; newSubType = 0; break;
           }

       case Entity::EntityType::WaypointSlow: {
               newType = 9; newSubType = 1; break;
           }

       case Entity::EntityType::WaypointFuel: {
               newType = 9; newSubType = 2; break;
           }

       case Entity::EntityType::WaypointAmmo: {
               newType = 9; newSubType = 3; break;
           }

       case Entity::EntityType::WaypointShield: {
               newType = 9; newSubType = 4; break;
           }

       case Entity::EntityType::WaypointShortcut: {
               newType = 9; newSubType = 6; break;
           }

       case Entity::EntityType::WaypointSpecial1: {
               newType = 9; newSubType = 7; break;
           }

       case Entity::EntityType::WaypointSpecial2: {
               newType = 9; newSubType = 8; break;
           }

       case Entity::EntityType::WaypointFast: {
               newType = 9; newSubType = 9; break;
           }

       case Entity::EntityType::WaypointSpecial3: {
               newType = 9; newSubType = 10; break;
           }

       case Entity::EntityType::RecoveryTruck: {
               newType = 10; newSubType = 9; break;
           }

       case Entity::EntityType::Unknown: {
               //in case of an unknown/undefined entity we read
               //put the raw type and subtype back into the file
               newType = mRawType; newSubType = mRawSubType; break;
       }

       default: {
           break;
       }
   }
}

bool EntityItem::WriteChanges() {
    //first get levelfile type
    //and subtype back
    int8_t type;
    int8_t subtype;

    revIdentify(mEntityType, type, subtype);

    //store type again
    this->m_wBytes.at(0) = (unsigned char)(type);

    //store subType again
    this->m_wBytes.at(1) = (unsigned char)(subtype);

    //store group data again
    ConvertAndWriteInt16ToByteArray(mGroup, this->m_wBytes, 2);

    //store target group data again
    ConvertAndWriteInt16ToByteArray(mTargetGroup, this->m_wBytes, 4);

    //store nextID information
    ConvertAndWriteInt16ToByteArray(mNextId, this->m_wBytes, 12);

    //store value again
    ConvertAndWriteInt16ToByteArray(mValue, this->m_wBytes, 14);

    //store X, Z coordinates again
    //Y information (height) does not seem to be stored for entities;
    // I believe inside the game works mostly 2D (flat) for entities, collision
    // detection, waypoint calculations etc...
    irr::core::vector3df vecHlp(0.0f, /*-DEF_SEGMENTSIZE * 0.5f*/ DEF_SEGMENTSIZE * 0.5f, 0.0f /*DEF_SEGMENTSIZE * 0.5f*/);
    irr::core::vector3df vecHlp2 =  mCenter - vecHlp;

    //we also need to swap X coordinate again
    //because my Irrlicht coordinate system is mirrored/swapped
    //compared to HiOctane level file
    //22.03.2025: The additional in between steps with int variables
    //does not really make sense, but I want to loose the decimals points here,
    //and if I do not do this inbetween conversion visual studio complains about 
    //it
    int newXInt = (int)(-vecHlp2.X); //- (DEF_SEGMENTSIZE * 0.5f);
    int newZInt = (int)(vecHlp2.Z);  //- (DEF_SEGMENTSIZE * 0.5f);

    float newX = (float)(newXInt); 
    float newZ = (float)(newZInt); 

    //convert X information to bytes
    ConvertAndWriteFloatToByteArray(newX, this->m_wBytes, 16);

    //convert Z information to bytes
    ConvertAndWriteFloatToByteArray(newZ, this->m_wBytes, 18);

    //convert XOffset information to bytes
    ConvertAndWriteFloatToByteArray(mOffsetX, this->m_wBytes, 20, true);

    //convert YOffset information to bytes
    ConvertAndWriteFloatToByteArray(mOffsetY, this->m_wBytes, 22, true);

    //write unknown data
    ConvertAndWriteInt16ToByteArray(mUnknown1, this->m_wBytes, 6);
    ConvertAndWriteInt16ToByteArray(mUnknown2, this->m_wBytes, 8);
    ConvertAndWriteInt16ToByteArray(mUnknown3, this->m_wBytes, 10);

    return true;
}
