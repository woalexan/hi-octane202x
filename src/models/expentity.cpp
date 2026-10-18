/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "expentity.h"
#include "../race.h"
#include "../vanilla/veffectmanager.h"
#include "../vanilla/vcalc.h"

void ExplosionEntity::Trigger() {
    if (!mAlreadyExploded) {
        mAlreadyExploded = true;

        irr::core::vector3df vanPos = mRace->mVCalc->IrrlichtToVanillaCoord(mPosition);

        //trigger explosion at correct location
        mRace->mEffectManager->AddEffect(EffectType::ExplosionMedium, vanPos, 0.0f, 0.0f, 0.0f, -1);
    }
}

ExplosionEntity::ExplosionEntity(EntityItem* entityItem, Race* mParentRace) {
   mRace = mParentRace;
   mEntityItem = entityItem;

   //store the explosion coordinate in map
   mPosition = entityItem->getCenter();
}

ExplosionEntity::~ExplosionEntity() {
}

