/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef EXPENTITY_H
#define EXPENTITY_H

#include "../race.h"

class Race; //Forward declaration

class ExplosionEntity {
public:
    ExplosionEntity(EntityItem* entityItem, Race* mParentRace);
    ~ExplosionEntity();

    EntityItem* mEntityItem;

    void Trigger();

private:
    Race* mRace;
    
    bool mAlreadyExploded = false;

    irr::core::vector3df mPosition;
};

#endif // EXPENTITY_H

