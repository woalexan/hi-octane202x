/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef INFRABASE_H
#define INFRABASE_H

#include <iostream>
#include <irrlicht.h>
#include "input/input.h"
#include "utils/logger.h"
#include "resources/assets.h"
#include "resources/readgamedata/preparedata.h"
#include "utils/tprofile.h"

using namespace std;

using namespace irr;
using namespace irr::core;
using namespace irr::video;
using namespace irr::scene;
using namespace irr::gui;

class Logger; //Forward declaration

class InfrastructureBase {
public:
  InfrastructureBase(dimension2d<u32> resolution, bool fullScreen, bool enableShadows);
  ~InfrastructureBase();

  //get a random int in the range between min and max
  int randRangeInt(int min, int max);

  //get a random float value in the range of 0.0 up to 1.0
  float randFloat();

  dimension2d<u32> mScreenRes;
  Logger* mLogger;

  bool GetInitOk();

  //have all pointers as public
  //so that we can access them easily everywhere
  IrrlichtDevice* mDevice;
  video::IVideoDriver* mDriver;
  scene::ISceneManager* mSmgr;
  MyEventReceiver* mEventReceiver;
  IGUIEnvironment* mGuienv;

  PrepareData* mPrepareData;
  GameText* mGameTexts;
  TimeProfiler* mTimeProfiler;

private:
  //Irrlicht stuff

  bool mEnableShadows;
  bool mFullscreen;

  bool mInitOk = false;

  //Returns true for success, false for error occured
  bool InitIrrlicht();
  bool InitGameResources();
};


#endif // INFRABASE_H
