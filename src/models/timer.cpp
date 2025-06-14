/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "timer.h"
#include "../resources/entityitem.h"
#include "../race.h"

void Timer::Trigger() {
    if (!mRunning) {
        //according to movAX13h (HiOctaneTools) a value of 100
        //is approximately 4 seconds
        mRemainingDelay = ((irr::f32)(mEntityItem->getValue()) / 100.0f) * 4.0f;

        mRunning = true;
    }
}

void Timer::Update(irr::f32 frameDeltaTime) {
    if (mRunning) {
        mRemainingDelay -= frameDeltaTime;

        if (mRemainingDelay <= 0.0f) {
            mRunning = false;
            //we need to trigger our target group
            //tell race about it
            mRace->TimedTriggerOccured(this);
        }
    }
}

Timer::Timer(EntityItem* entityItem, Race* mParentRace) {
   mRace = mParentRace;
   mEntityItem = entityItem;

   //Timer has also a position in the map
   mPosition = entityItem->getCenter();
}

Timer::~Timer() {
}

