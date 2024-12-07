/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ENTITYITEM_H
#define ENTITYITEM_H

#include "tableitem.h"
#include "irrlicht/irrlicht.h"
#include <vector>

using namespace irr::core;

class EntityItem : public TableItem {
public:
    EntityItem(int id, int offset, std::vector<unsigned char> bytes);
    virtual ~EntityItem();

    unsigned char get_Type();
    unsigned char get_SubType();
    int16_t get_NextID();
    float get_X();
    float get_Y();
    float get_Z();

    void set_Y(float newvalue);

    irr::core::vector3d<float> get_Pos();
    irr::core::vector3d<float> get_Center();

    irr::core::vector2df GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord);

    int get_Group();
    int get_TargetGroup();
    int get_Value();
    float get_OffsetX();
    float get_OffsetY();

    bool WriteChanges() override;

    enum EntityType {
              Unknown, UnknownShieldItem, UnknownItem,
              ExtraShield, ShieldFull, DoubleShield,
              ExtraAmmo, AmmoFull, DoubleAmmo,
              ExtraFuel, FuelFull, DoubleFuel,
              MinigunUpgrade, MissileUpgrade, BoosterUpgrade,
              WallSegment,
              WaypointFuel, WaypointAmmo, WaypointShield, WaypointSpecial1, WaypointSpecial2, WaypointSpecial3, WaypointFast, WaypointSlow, WaypointShortcut,
              RecoveryTruck,
              SteamStrong, SteamLight, Cone, Checkpoint,
              MorphSource1, MorphSource2, MorphOnce, MorphPermanent,
              TriggerCraft, TriggerTimed, TriggerRocket,
              DamageCraft,
              Explosion, ExplosionParticles
    };

    EntityType get_GameType();
    void set_GameType(EntityType new_GameType);

    void UpdateGameType();

protected:
    EntityType m_GameType;
    EntityType identify();

    float m_Y;
};

#endif // ENTITYITEM_H
