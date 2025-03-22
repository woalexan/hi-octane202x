/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef TPROFILE_H
#define TPROFILE_H

#include "irrlicht/irrlicht.h"
#include <vector>
#include "SFML/System.hpp"
#include <algorithm>
#include <cstdint>
#include "wchar.h"

#define TPROFILER_AVGNRSAMPLES 10
#define TPROFILER_MAXTEXTCHARS 500

class TimeProfilerResultObj {
public:

    TimeProfilerResultObj(char* identifier);
    ~TimeProfilerResultObj();

    void AddMeasInterval(float measTimeInterval);
    void ResetStatistics();

    //placeholder to describe result
    char id[50];

    //hold minimum measured duration
    float minDurationMs;

    //holds average measured duration
    float avgDurationMs;

    //hold maximum measured duration
    float maxDurationMs;

private:

    uint8_t currWritePosition;
    float currSum;

    std::vector<float> *measDurations;

    bool firstSample;
};

class TimeProfiler {
    
public:
    TimeProfiler(irr::gui::IGUIEnvironment* guienvPntr, irr::core::rect<irr::s32> logWindowPos);
    ~TimeProfiler();

    void StartOfGameLoop();
    void Profile(TimeProfilerResultObj* resultObj);
    void GetTimeProfileResultDescending(wchar_t* outputText, int maxCharNr, int printNrEntries);

    void HideWindow();
    void ShowWindow();
    bool IsWindowHidden();

    void UpdateWindow();

    TimeProfilerResultObj* tIntOverallGameLoop;
    TimeProfilerResultObj* tIntHandleInput;
    TimeProfilerResultObj* tIntHandleComputerPlayers;
    TimeProfilerResultObj* tIntRender3DScene;
    TimeProfilerResultObj* tIntRender2D;

    //measured time intervals inside race class (Advance time)
    TimeProfilerResultObj* tIntMorphing;
    TimeProfilerResultObj* tIntWorldAware;
    TimeProfilerResultObj* tIntUpdatePlayers;
    TimeProfilerResultObj* tIntAdvancePhysics;
    TimeProfilerResultObj* tIntAfterPhysicsUpdate;
    TimeProfilerResultObj* tIntPlayerMonitoring;
    TimeProfilerResultObj* tIntUpdateParticleSystems;
    TimeProfilerResultObj* tIntUpdateCones;
    TimeProfilerResultObj* tIntProcessTriggers;

private:
    float lastTimeStampMs;
    std::vector<TimeProfilerResultObj*>* mTimeProfileResVec;

    sf::Clock *mClock;

    irr::core::rect<irr::s32> mCurrenttProfileOutputWindowPos;

    irr::gui::IGUIEnvironment* mGuiEnv;
    irr::gui::IGUIStaticText* tProfileOutputWindow;
    irr::u32 mNumberOftProfilerResultsShown;

    bool mWindowHidden;
    wchar_t* mText;
};

#endif // TPROFILE_H
