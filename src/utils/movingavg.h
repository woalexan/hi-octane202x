/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MOVINGAVG_H
#define MOVINGAVG_H

#include "irrlicht/irrlicht.h"
#include <list>
#include <vector>

#define MVG_AVG_TYPE_IRRFLOAT32 0
#define MVG_AVG_TYPE_IRRCOREVECT3DF 1

struct MovingAverageDataStruct {
    irr::f32 currentSum = 0.0f;

    std::list<irr::f32> currentValueList;

    irr::u8 currentValuesCnt = 0;
    irr::u8 numberValuesToAverage = 1;

    irr::f32 currentAvgValue = 0.0f;
};

class MovingAverageCalculator {
public:
    MovingAverageCalculator(irr::u8 setupType, irr::u8 numberValuesToAverage);
    ~MovingAverageCalculator();

    irr::f32 AddNewValue(irr::f32 inputValue);
    irr::core::vector3df AddNewValue(irr::core::vector3df inputVector);

private:
    std::vector<MovingAverageDataStruct*> averageDataVec;
    void Update(irr::f32 inputValue, irr::u8 index);
};

#endif // MOVINGAVG_H
