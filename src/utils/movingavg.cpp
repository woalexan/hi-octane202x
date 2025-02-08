/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "movingavg.h"

MovingAverageCalculator::MovingAverageCalculator(irr::u8 setupType, irr::u8 numberValuesToAverage) {
    MovingAverageDataStruct* newStruct;

    if (setupType == MVG_AVG_TYPE_IRRFLOAT32) {
        newStruct = new MovingAverageDataStruct();
        newStruct->numberValuesToAverage = numberValuesToAverage;
        this->averageDataVec.push_back(newStruct);
    }

    if (setupType == MVG_AVG_TYPE_IRRCOREVECT3DF) {
        //for X
        newStruct = new MovingAverageDataStruct();
        newStruct->numberValuesToAverage = numberValuesToAverage;
        this->averageDataVec.push_back(newStruct);

        //for Y
        newStruct = new MovingAverageDataStruct();
        newStruct->numberValuesToAverage = numberValuesToAverage;
        this->averageDataVec.push_back(newStruct);

        //for Z
        newStruct = new MovingAverageDataStruct();
        newStruct->numberValuesToAverage = numberValuesToAverage;
        this->averageDataVec.push_back(newStruct);
    }
}

void MovingAverageCalculator::Update(irr::f32 inputValue, irr::u8 index) {
    MovingAverageDataStruct* pntr = this->averageDataVec.at(index);

    if (pntr->currentValuesCnt > pntr->numberValuesToAverage) {
        //Note 08.02.2025: I scrapped this solution right now with the overall sum
        //because I am worried about the round errors to add up over a long time

        //we have already too much values, remove the value that is
        //furthest in the past
        //pntr->currentSum -=  *this->averageDataVec.at(index)->currentValueList.begin();

        this->averageDataVec.at(index)->currentValueList.pop_front();
        pntr->currentValuesCnt--;
    }

    this->averageDataVec.at(index)->currentValueList.push_back(inputValue);
    //add the new value to the current
    //sum of values
    //pntr->currentSum += inputValue;
    pntr->currentValuesCnt++;

    //Note 08.02.2025: Because of reasoning above, just recalculate average right now, and loose performance :(
    //but have no problems with rounding errors rounding up over time
    std::list<irr::f32>::iterator itList;
    pntr->currentSum = 0.0f;

    for (itList = pntr->currentValueList.begin(); itList != pntr->currentValueList.end(); ++itList) {
         pntr->currentSum += (*itList);
    }

    //calculate new current average value
    pntr->currentAvgValue = pntr->currentSum / (irr::f32)(pntr->currentValuesCnt);
}

irr::f32 MovingAverageCalculator::AddNewValue(irr::f32 inputValue) {
    Update(inputValue, 0);

    return (this->averageDataVec.at(0)->currentAvgValue);
}

irr::core::vector3df MovingAverageCalculator::AddNewValue(irr::core::vector3df inputVector) {
    Update(inputVector.X, 0);
    Update(inputVector.Y, 1);
    Update(inputVector.Z, 2);

    irr::core::vector3df result(this->averageDataVec.at(0)->currentAvgValue,
                                this->averageDataVec.at(1)->currentAvgValue,
                                this->averageDataVec.at(2)->currentAvgValue);

    return result;
}

MovingAverageCalculator::~MovingAverageCalculator() {
    std::vector<MovingAverageDataStruct*>::iterator it;
    MovingAverageDataStruct* pntr;

    for (it = this->averageDataVec.begin(); it != this->averageDataVec.end(); ) {
       pntr = (*it);

       it = this->averageDataVec.erase(it);

       //delete the struct itself
       delete pntr;
    }
}
