/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "entityitem.h"

EntityItem::EntityItem(int id, int offset, std::vector<unsigned char> bytes) {
   this->m_ID = id;
   this->m_Bytes = bytes;
   this->m_Offset = offset;

   //for debugging of level saving, comment out later
   this->m_wBytes.resize(this->m_Bytes.size());
   std::fill(m_wBytes.begin(), m_wBytes.begin() + this->m_Bytes.size(), 0);

   this->m_Y = 0.0f;   // set by level loader

   this->UpdateGameType();
}

EntityItem::~EntityItem() {

}

unsigned char EntityItem::get_Type() {
    return(this->m_Bytes.at(0));
}

irr::core::vector2df EntityItem::GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord) {
    irr::core::vector3df threeDCoordMyUniverse(-get_X(), get_Y(), get_Z());

    threeDCoord = threeDCoordMyUniverse;

    irr::core::vector2df result(threeDCoordMyUniverse.X, threeDCoordMyUniverse.Z);

    return result;
}

unsigned char EntityItem::get_SubType() {
    return (this->m_Bytes.at(1));
}

int16_t EntityItem::get_NextID() {
    int16_t result;

    result = static_cast<int16_t>((this->m_Bytes.at(13) << 8) | this->m_Bytes.at(12));
    return (result);
}

float EntityItem::get_X() {
    float result = (float(this->m_Bytes.at(16)) / float(256))+float(this->m_Bytes.at(17));
    return(result);
}

float EntityItem::get_Y() {
    return(this->m_Y);   // set by level loader
}

float EntityItem::get_Z() {
    float result = (float(this->m_Bytes.at(18)) / float(256))+float(this->m_Bytes.at(19));
    return(result);
}

irr::core::vector3d<float> EntityItem::get_Pos() {
    irr::core::vector3d<float> result(this->get_X(), this->get_Y(), this->get_Z());
    return(result);
}

irr::core::vector3d<float> EntityItem::get_Center() {
    irr::core::vector3d<float> vector_one(1.0f, 1.0f, 1.0f);
    irr::core::vector3d<float> result = this->get_Pos()+0.5f*vector_one;
    return(result);
}

int EntityItem::get_Group() {
   return(this->m_Bytes.at(2) + this->m_Bytes.at(3) * 256);
}

int EntityItem::get_TargetGroup() {
   return(this->m_Bytes.at(4) + this->m_Bytes.at(5) * 256);
}

int EntityItem::get_Value() {
   return(this->m_Bytes.at(14) + this->m_Bytes.at(15) * 256);
}

void EntityItem::set_Y(float newvalue) {
    this->m_Y = newvalue;  // set by level loader
}

float EntityItem::get_OffsetX() {
   float result = float(this->m_Bytes.at(20)) + float(this->m_Bytes.at(21)) * 256.0f;
   return(result);
}

float EntityItem::get_OffsetY() {
   float result = float(this->m_Bytes.at(22)) + float(this->m_Bytes.at(23)) * 256.0f;
   return(result);
}

EntityItem::EntityType EntityItem::get_GameType() {
    return(this->m_GameType);
}

void EntityItem::set_GameType(EntityType new_GameType) {
    this->m_GameType = new_GameType;
}

void EntityItem::UpdateGameType() {
    this->m_GameType = this->identify();
}

EntityItem::EntityType EntityItem::identify() {
    int Type = this->get_Type();
    int SubType = this->get_SubType();

    if (Type == 1 && SubType == 5) return(EntityItem::Checkpoint);

    if (Type == 2 && SubType == 1) return(EntityItem::ExplosionParticles); // see level 4
    if (Type == 2 && SubType == 2) return(EntityItem::DamageCraft);        // see level 8
    if (Type == 2 && SubType == 3) return(EntityItem::Explosion);
    if (Type == 2 && SubType == 5) return(EntityItem::SteamStrong);
    if (Type == 2 && SubType == 7) return(EntityItem::MorphSource2);
    if (Type == 2 && SubType == 8) return(EntityItem::SteamLight);
    if (Type == 2 && SubType == 9) return(EntityItem::MorphSource1);
    if (Type == 2 && SubType == 16) return(EntityItem::MorphOnce);
    if (Type == 2 && SubType == 23) return(EntityItem::MorphPermanent);

    if (Type == 3 && SubType == 6) return(EntityItem::Cone);

    if (Type == 5 && SubType == 0) return(EntityItem::UnknownShieldItem);
    if (Type == 5 && SubType == 1) return(EntityItem::UnknownItem);

    if (Type == 5 && SubType == 2) return(EntityItem::ExtraShield);
    if (Type == 5 && SubType == 3) return(EntityItem::ShieldFull);
    if (Type == 5 && SubType == 4) return(EntityItem::DoubleShield);
    if (Type == 5 && SubType == 5) return(EntityItem::ExtraAmmo);
    if (Type == 5 && SubType == 6) return(EntityItem::AmmoFull);
    if (Type == 5 && SubType == 7) return(EntityItem::DoubleAmmo);
    if (Type == 5 && SubType == 8) return(EntityItem::ExtraFuel);
    if (Type == 5 && SubType == 9) return(EntityItem::FuelFull);
    if (Type == 5 && SubType == 10) return(EntityItem::DoubleFuel);
    if (Type == 5 && SubType == 11) return(EntityItem::MinigunUpgrade);
    if (Type == 5 && SubType == 12) return(EntityItem::MissileUpgrade);
    if (Type == 5 && SubType == 13) return(EntityItem::BoosterUpgrade);

    if (Type == 8 && SubType == 0) return(EntityItem::TriggerCraft);
    if (Type == 8 && SubType == 1) return(EntityItem::TriggerTimed);
    if (Type == 8 && SubType == 3) return(EntityItem::TriggerRocket);

    if (Type == 9 && SubType == 0) return(EntityItem::WallSegment);

    if (Type == 9 && SubType == 1) return(EntityItem::WaypointSlow);
    if (Type == 9 && SubType == 2) return(EntityItem::WaypointFuel);
    if (Type == 9 && SubType == 3) return(EntityItem::WaypointAmmo);
    if (Type == 9 && SubType == 4) return(EntityItem::WaypointShield);
    if (Type == 9 && SubType == 6) return(EntityItem::WaypointShortcut);
    if (Type == 9 && SubType == 7) return(EntityItem::WaypointSpecial1);
    if (Type == 9 && SubType == 8) return(EntityItem::WaypointSpecial2);
    if (Type == 9 && SubType == 9) return(EntityItem::WaypointFast);
    if (Type == 9 && SubType == 10) return(EntityItem::WaypointSpecial3);

    if (Type == 10 && SubType == 9) return(EntityItem::RecoveryTruck);

    return(EntityItem::Unknown);
}

bool EntityItem::WriteChanges() {
   return(false);
}
