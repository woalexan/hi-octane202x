/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "definitions.h"

Entity::EntityType IdentifyEntity(int8_t mRawType, int8_t mRawSubType) {
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
    if (mRawType == 9 && mRawSubType == 5) return(Entity::EntityType::WaypointUnknownVal5);
    if (mRawType == 9 && mRawSubType == 6) return(Entity::EntityType::WaypointShortcut);
    if (mRawType == 9 && mRawSubType == 7) return(Entity::EntityType::WaypointSpecial1);
    if (mRawType == 9 && mRawSubType == 8) return(Entity::EntityType::WaypointSpecial2);
    if (mRawType == 9 && mRawSubType == 9) return(Entity::EntityType::WaypointFast);
    if (mRawType == 9 && mRawSubType == 10) return(Entity::EntityType::WaypointSpecial3);

    if (mRawType == 10 && mRawSubType == 9) return(Entity::EntityType::RecoveryTruck);

    return(Entity::EntityType::Unknown);
}

void RevIdentifyEntity(Entity::EntityType newEntityType, int8_t &newType, int8_t &newSubType) {
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

       case Entity::EntityType::WaypointUnknownVal5: {
               newType = 9; newSubType = 5; break;
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
               //newType = mRawType; newSubType = mRawSubType; break;
               break;
       }

       default: {
           break;
       }
   }
}
