/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "tprofile.h"

TimeProfilerResultObj::TimeProfilerResultObj(char* identifier) {
    strcpy(this->id, identifier);

    currWritePosition = 0;

    //create vector for time measurements
    this->measDurations = new std::vector<float>();

    //create initial values for vector
    for (uint8_t idx = 0; idx < TPROFILER_AVGNRSAMPLES; idx++) {
        measDurations->push_back(0.0f);
    }

    currSum = 0.0f;

    firstSample = true;
}

TimeProfilerResultObj::~TimeProfilerResultObj() {
   delete measDurations;
}

void TimeProfilerResultObj::AddMeasInterval(float measTimeInterval) {
  //remove oldest element from our current sum
  currSum -= measDurations->at(currWritePosition);

  //add last measurement result to our current sum
  currSum += measTimeInterval;

  //remember current measurement result to be able to remove it
  //later again
  measDurations->at(currWritePosition) = measTimeInterval;

  //advance index to now oldest element
  currWritePosition++;

  if (currWritePosition > (TPROFILER_AVGNRSAMPLES - 1)) {
      currWritePosition = 0;
  }

  //calculate current average value
  this->avgDurationMs = (currSum / TPROFILER_AVGNRSAMPLES);

  if (firstSample) {
      this->minDurationMs = measTimeInterval;
      this->maxDurationMs = measTimeInterval;
      firstSample = false;
  } else {
      if (measTimeInterval < this->minDurationMs) {
          this->minDurationMs = measTimeInterval;
      }

      if (measTimeInterval > this->maxDurationMs) {
          this->maxDurationMs = measTimeInterval;
      }
  }
}

void TimeProfilerResultObj::ResetStatistics() {
    for (uint8_t idx = 0; idx < TPROFILER_AVGNRSAMPLES; idx++) {
        measDurations->at(idx) = 0.0f;
    }

    currSum = 0.0f;
    currWritePosition = 0;

    firstSample = true;
}

void TimeProfiler::StartOfGameLoop() {
    //restart the clock, function also returns the measured time since the
    //last restart; this means in our case the overall last gameloop run time
    tIntOverallGameLoop->AddMeasInterval(mClock->restart().asMilliseconds());

    this->lastTimeStampMs = mClock->getElapsedTime().asMilliseconds();
}

void TimeProfiler::Profile(TimeProfilerResultObj* resultObj) {
    if (resultObj != NULL) {
        resultObj->AddMeasInterval(
                    mClock->getElapsedTime().asMilliseconds() - this->lastTimeStampMs);
    }

    this->lastTimeStampMs = mClock->getElapsedTime().asMilliseconds();
}

TimeProfiler::TimeProfiler(irr::gui::IGUIEnvironment* guienvPntr, irr::core::rect<irr::s32> logWindowPos) {
    //create my SFML clock
    //as soon as it is created it does start
    //running
    mClock = new sf::Clock();

    //create the result objects
    tIntOverallGameLoop = new TimeProfilerResultObj((char*)("overallGameLoop"));
    tIntHandleInput = new TimeProfilerResultObj((char*)("handleInput"));
    tIntAdvancePhysics = new TimeProfilerResultObj((char*)("advancePhysics"));
    tIntHandleComputerPlayers = new TimeProfilerResultObj((char*)("handleComputerPlayers"));
    tIntRender3DScene = new TimeProfilerResultObj((char*)("render3DScene"));
    tIntRender2D = new TimeProfilerResultObj((char*)("render2D"));

    //create results objects for details inside Race class
    tIntMorphing = new TimeProfilerResultObj((char*)("handleMorphing"));
    tIntWorldAware = new TimeProfilerResultObj((char*)("worldAware"));
    tIntUpdatePlayers = new TimeProfilerResultObj((char*)("updatePlayers"));
    tIntPlayerMonitoring = new TimeProfilerResultObj((char*)("playerMonitoring"));
    tIntUpdateParticleSystems = new TimeProfilerResultObj((char*)("updateParticleSystems"));
    tIntAfterPhysicsUpdate = new TimeProfilerResultObj((char*)("afterPhysicsUpdate"));
    tIntUpdateCones = new TimeProfilerResultObj((char*)("updateCones"));
    tIntProcessTriggers = new TimeProfilerResultObj((char*)("processTriggers"));

    mTimeProfileResVec = new std::vector<TimeProfilerResultObj*>();
    mTimeProfileResVec->push_back(tIntOverallGameLoop);
    mTimeProfileResVec->push_back(tIntHandleInput);
    mTimeProfileResVec->push_back(tIntAdvancePhysics);
    mTimeProfileResVec->push_back(tIntHandleComputerPlayers);
    mTimeProfileResVec->push_back(tIntRender3DScene);
    mTimeProfileResVec->push_back(tIntRender2D);
    mTimeProfileResVec->push_back(tIntMorphing);
    mTimeProfileResVec->push_back(tIntWorldAware);
    mTimeProfileResVec->push_back(tIntUpdatePlayers);
    mTimeProfileResVec->push_back(tIntPlayerMonitoring);
    mTimeProfileResVec->push_back(tIntUpdateParticleSystems);
    mTimeProfileResVec->push_back(tIntAfterPhysicsUpdate);
    mTimeProfileResVec->push_back(tIntUpdateCones);
    mTimeProfileResVec->push_back(tIntProcessTriggers);

    //create the time profiler result window
    mGuiEnv = guienvPntr;
    mCurrenttProfileOutputWindowPos = logWindowPos;

    mWindowHidden = true;

    //create the log window
    this->tProfileOutputWindow = mGuiEnv->addStaticText(L"",
           mCurrenttProfileOutputWindowPos, false, true, NULL, -1, true);

    //we need to deactivate word wrap because if we do not
    //there seems to be an Irrlicht bug or problem
    //that causes an unexpected malloc assert problems somehow
    this->tProfileOutputWindow->setWordWrap(false);

    //how many lines of time profiler results we can max show at a time
    //depends on the current logWindow size, and font height
    irr::u32 fontHeight =
            this->tProfileOutputWindow->getActiveFont()->getDimension(L"Lgq!").Height;

    mNumberOftProfilerResultsShown = logWindowPos.getHeight() / fontHeight;

    //initially hide window
    this->tProfileOutputWindow->setVisible(false);

    //create my internal text storage
    mText = new wchar_t[TPROFILER_MAXTEXTCHARS];
}

void TimeProfiler::GetTimeProfileResultDescending(wchar_t* outputText, int maxCharNr, int printNrEntries) {

    std::vector<TimeProfilerResultObj*>::iterator it;
    int remChars = maxCharNr;
    int remNrEntries = printNrEntries;

    wcscpy(outputText, L"");

    if (mTimeProfileResVec->size() < 1)
        return;

    //declaring vector of pairs containing measured avg time interval
    //measurement result and pointer to TimeProfilerResultObj
    std::vector< std::pair <irr::f32, TimeProfilerResultObj*> > sortedAvgTimeIntervals;

    for (it = mTimeProfileResVec->begin(); it != mTimeProfileResVec->end(); ++it) {
          sortedAvgTimeIntervals.push_back( std::make_pair((*it)->avgDurationMs, (*it)));
    }

    //sort vector pairs in descending avg time interval duration
    //we want to have the longest operations at the top
   std::sort(sortedAvgTimeIntervals.rbegin(), sortedAvgTimeIntervals.rend());

   //now create output text
   //start with longest operation
   auto it2 = sortedAvgTimeIntervals.begin();

   TimeProfilerResultObj* mTimeProfileResultPntr;

   irr::f32 avgTimeIntervalResMs;

   wchar_t entry[200];
   size_t currLen;

   for (it2 = sortedAvgTimeIntervals.begin(); (it2 != sortedAvgTimeIntervals.end()) && (remNrEntries > 0); ++it2) {
       avgTimeIntervalResMs = (it2)->first;
       mTimeProfileResultPntr = (it2)->second;

       remNrEntries--;

       swprintf(entry, 200, L"%s : %.3f ms avg\n", mTimeProfileResultPntr->id, mTimeProfileResultPntr->avgDurationMs);
       currLen = wcslen(entry);
       remChars -= currLen;

       if (remChars > 0) {
        wcscat(outputText, entry);
       } else {
           break;
       }
   }
}

bool TimeProfiler::IsWindowHidden() {
    return (mWindowHidden);
}

void TimeProfiler::HideWindow() {
    mWindowHidden = true;
    this->tProfileOutputWindow->setVisible(false);
}

void TimeProfiler::ShowWindow() {
   mWindowHidden = false;
   this->tProfileOutputWindow->setVisible(true);
}

void TimeProfiler::UpdateWindow() {
   //update my internal text with last results
   GetTimeProfileResultDescending(mText, TPROFILER_MAXTEXTCHARS, mNumberOftProfilerResultsShown);

   tProfileOutputWindow->setText(mText);
}

TimeProfiler::~TimeProfiler() {
    delete mClock;

    mTimeProfileResVec->clear();
    delete mTimeProfileResVec;

    delete tIntOverallGameLoop;
    delete tIntHandleInput;
    delete tIntAdvancePhysics;
    delete tIntHandleComputerPlayers;
    delete tIntRender3DScene;
    delete tIntRender2D;
    delete tIntMorphing;
    delete tIntWorldAware;
    delete tIntUpdatePlayers;
    delete tIntPlayerMonitoring;
    delete tIntUpdateParticleSystems;
    delete tIntAfterPhysicsUpdate;
    delete tIntUpdateCones;
    delete tIntProcessTriggers;

    //delete my internal text storage
    delete[] mText;
}
