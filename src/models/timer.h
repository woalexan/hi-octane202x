/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef TIMER_H
#define TIMER_H

#include <irrlicht.h>

/************************
 * Forward declarations *
 ************************/

class Race;
class EntityItem;

class Timer {
public:
    Timer(EntityItem* entityItem, Race* mParentRace);
    ~Timer();

    EntityItem* mEntityItem = nullptr;

    void Trigger();
    void Update(irr::f32 frameDeltaTime);

private:
    Race* mRace = nullptr;

    irr::f32 mRemainingDelay;
    bool mRunning = false;

    irr::core::vector3df mPosition;
};

#endif // TIMER_H

