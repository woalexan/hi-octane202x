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
#include "utils/logging.h"
#include <vector>

using namespace std;

using namespace irr;
using namespace irr::core;
using namespace irr::video;
using namespace irr::scene;
using namespace irr::gui;

class Logger; //Forward declaration
class PrepareData; //Forward declaration

struct OriginalGameFolderInfoStruct {
    irr::io::IFileList* rootFolder;
    irr::io::IFileList* dataFolder;
    irr::io::IFileList* execFolder;
    irr::io::IFileList* saveFolder;
    irr::io::IFileList* mapsFolder;
    irr::io::IFileList* objectsFolder;
    irr::io::IFileList* soundFolder;
};

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

  OriginalGameFolderInfoStruct* mOriginalGame;

  std::vector<uint8_t> mGameVersionDate;
  bool mExtendedGame = false;

  //if specified file is not found, returns empty path
  irr::io::path LocateFileInFileList(irr::io::IFileList* fileList, irr::core::string<fschar_t> fileName);
  bool UpdateFileListSaveFolder();

private:
  //Irrlicht stuff

  bool mEnableShadows;
  bool mFullscreen;

  bool mInitOk = false;

  //this paths are all absolute paths
  irr::io::path mGameRootDir;
  irr::io::path mOriginalGameRootDir;
  irr::io::path saveFolderDataPath;

  irr::io::IFileList* CreateFileList(irr::io::path whichAbsPath);

  //Returns true if original game found,
  //False otherwise, resulting original
  //game root dir is stored in mOriginalGameRootDir
  bool LocateOriginalGame();
  bool DetermineOriginalGameVersion();
  bool ProcessGameVersionDate();

  //Returns true for success, false for error occured
  bool InitIrrlicht();
  bool InitGameResources();
};


#endif // INFRABASE_H
