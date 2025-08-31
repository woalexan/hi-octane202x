/*
 The source code in this file was based on/derived from/translated/and afterwards modified from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did first a translation to C++, then modified it)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ENTITYITEM_H
#define ENTITYITEM_H

#include "tableitem.h"
#include "irrlicht.h"
#include <cstdint>

//This two states are only used in the LevelEditor
#define DEF_ENTITYITEM_STATE_DEFAULT 0
#define DEF_ENTITYITEM_STATE_NEWLYUNASSIGNEDONE 1
#define DEF_ENTITYITEM_STATE_LINKUPDATED 2

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
              Explosion, ExplosionParticles, Camera,
              //the InternalTemporaryWaypoint type is not stored inside the
              //game level files, it is only used for computer player routing/control
              InternalTemporaryWaypoint
        };
}

class EntityItem : public TableItem {
public:
    //constructor for entity items stored inside game level files
    EntityItem(int id, int offset, std::vector<unsigned char> bytes);
    virtual ~EntityItem();

    //special constructor that is only used in LevelEditor for creation of new
    //EntityItems via the EntityManager
    EntityItem(int x, int y, irr::f32 heightTerrain, int id, int offset, Entity::EntityType ofType);

    //special constructor only used for creation
    //of InternalTemporaryWaypoint entity items
    EntityItem(irr::core::vector3df position);

    int16_t getNextID();
    void setNextID(int16_t newNextID);

    void setY(float newvalue);

    irr::core::vector2df GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord);

    int16_t getValue();
    void setValue(int16_t newValue);

    void setUnknown1(int16_t newValue);
    void setUnknown2(int16_t newValue);
    void setUnknown3(int16_t newValue);

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

    //Only used for writing debugging table file
    //do not use otherwise in higher level code!
    int8_t getRawType();
    int8_t getRawSubType();

    int16_t getUnknown1();
    int16_t getUnknown2();
    int16_t getUnknown3();

    /*****************************************
     * LevelEditor only used variables start *
     *****************************************/

    //while EntityItem is removed from
    //levelfile table this variable helps us
    int m_initialID;

    //this EntityItem state variable is
    //only used for the leveleditor
    uint8_t mState = DEF_ENTITYITEM_STATE_DEFAULT;

    /*****************************************
     * LevelEditor only used variables end   *
     *****************************************/

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
