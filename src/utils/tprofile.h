/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef TPROFILE_H
#define TPROFILE_H

#include "irrlicht.h"
#include <vector>
#include "SFML/System.hpp"
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
    float minDurationMs = 0.0f;

    //holds average measured duration
    float avgDurationMs = 0.0f;

    //hold maximum measured duration
    float maxDurationMs = 0.0f;

private:

    uint8_t currWritePosition;
    float currSum;

    std::vector<float> *measDurations = nullptr;

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

    TimeProfilerResultObj* tIntOverallGameLoop = nullptr;
    TimeProfilerResultObj* tIntHandleInput = nullptr;
    TimeProfilerResultObj* tIntHandleComputerPlayers = nullptr;
    TimeProfilerResultObj* tIntRender3DScene = nullptr;
    TimeProfilerResultObj* tIntRender2D = nullptr;

    //measured time intervals inside race class (Advance time)
    TimeProfilerResultObj* tIntMorphing = nullptr;
    TimeProfilerResultObj* tIntWorldAware = nullptr;
    TimeProfilerResultObj* tIntUpdatePlayers = nullptr;
    TimeProfilerResultObj* tIntAdvancePhysics = nullptr;
    TimeProfilerResultObj* tIntAfterPhysicsUpdate = nullptr;
    TimeProfilerResultObj* tIntPlayerMonitoring = nullptr;
    TimeProfilerResultObj* tIntUpdateParticleSystems = nullptr;
    TimeProfilerResultObj* tIntUpdateCones = nullptr;
    TimeProfilerResultObj* tIntProcessTriggers = nullptr;

private:
    float lastTimeStampMs;
    std::vector<TimeProfilerResultObj*>* mTimeProfileResVec = nullptr;

    sf::Clock *mClock = nullptr;

    irr::core::rect<irr::s32> mCurrenttProfileOutputWindowPos;

    irr::gui::IGUIEnvironment* mGuiEnv = nullptr;
    irr::gui::IGUIStaticText* tProfileOutputWindow = nullptr;
    irr::u32 mNumberOftProfilerResultsShown;

    bool mWindowHidden;
    wchar_t* mText = nullptr;
};

#endif // TPROFILE_H
