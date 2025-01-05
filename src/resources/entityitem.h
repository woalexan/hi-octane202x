/*
 The source code in this file was based on/derived from/translated/and afterwards modified from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did first a translation to C++, then modified it)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ENTITYITEM_H
#define ENTITYITEM_H

#include "tableitem.h"
#include "irrlicht/irrlicht.h"
#include "../definitions.h"
#include "../utils/crc32.h"

using namespace irr::core;

namespace Entity {
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
              Explosion, ExplosionParticles, Camera
        };
}

class EntityItem : public TableItem {
public:
    EntityItem(int id, int offset, std::vector<unsigned char> bytes);
    virtual ~EntityItem();

    int16_t getNextID();
    void setNextID(int16_t newNextID);

    void setY(float newvalue);

    irr::core::vector2df GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord);

    int16_t getValue();
    void setValue(int16_t newValue);

    float getOffsetX();
    float getOffsetY();
    void setOffsetX(float newOffsetX);
    void setOffsetY(float newOffsetY);

    irr::core::vector3df getCenter();
    irr::core::vector2di getCell();
    void setCenter(irr::core::vector3df newCenter);

    bool WriteChanges() override;

    Entity::EntityType getEntityType();
    void setEntityType(Entity::EntityType newEntityType);

    int16_t getGroup();
    void setGroup(int16_t newGroup);

    int16_t getTargetGroup();
    void setTargetGroup(int16_t newTargetGroup);

private:
    irr::core::vector3d<float> getPos();

    float getX();
    float getY();
    float getZ();

    int8_t getType();
    int8_t getSubType();
    void revIdentify(Entity::EntityType newEntityType, int8_t &newType, int8_t &newSubType);
    Entity::EntityType identify();

    int16_t decodeNextID();
    int16_t decodeGroup();
    int16_t decodeTargetGroup();
    int16_t decodeValue();
    float decodeOffsetX();
    float decodeOffsetY();

    Entity::EntityType mEntityType;
    //also store raw read Type and subType
    //in case we have an undefined/unknown entity type,
    //so when we save the level we can write the information back
    //to the level file
    int8_t mRawType;
    int8_t mRawSubType;

    int16_t mGroup;
    int16_t mTargetGroup;
    int16_t mNextId;
    int16_t mValue;
    irr::core::vector3df mCenter;
    irr::core::vector2di mCell;

    int16_t mUnknown1;
    int16_t mUnknown2;
    int16_t mUnknown3;

    float mOffsetX;
    float mOffsetY;
};

#endif // ENTITYITEM_H
