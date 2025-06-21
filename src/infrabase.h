/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef INFRABASE_H
#define INFRABASE_H

#include <irrlicht.h>
#include <vector>
#include <cstdint>
#include "resources/readgamedata/preparedata.h"

using namespace std;

using namespace irr;
using namespace irr::core;
using namespace irr::video;
using namespace irr::scene;
using namespace irr::gui;

/************************
 * Forward declarations *
 ************************/

class Logger;
class PrepareData;
class GameText;
class MyEventReceiver;
class TimeProfiler;
class DrawDebug;

struct OriginalGameFolderInfoStruct {
    irr::io::IFileList* rootFolder = nullptr;
    irr::io::IFileList* dataFolder = nullptr;
    irr::io::IFileList* execFolder = nullptr;
    irr::io::IFileList* saveFolder = nullptr;
    irr::io::IFileList* mapsFolder = nullptr;
    irr::io::IFileList* objectsFolder = nullptr;
    irr::io::IFileList* soundFolder = nullptr;
};

class InfrastructureBase {
public:
  InfrastructureBase();
  ~InfrastructureBase();

  //get a random int in the range between min and max
  int randRangeInt(int min, int max);

  //get a random float value in the range of 0.0 up to 1.0
  float randFloat();

  dimension2d<u32> mScreenRes;
  Logger* mLogger = nullptr;

  void InfrastructureInit(dimension2d<u32> resolution, bool fullScreen, bool enableShadows);
  bool GetInfrastructureInitOk();

  //have all pointers as public
  //so that we can access them easily everywhere
  IrrlichtDevice* mDevice = nullptr;
  video::IVideoDriver* mDriver = nullptr;
  scene::ISceneManager* mSmgr = nullptr;

  IGUIEnvironment* mGuienv = nullptr;

  MyEventReceiver* mEventReceiver = nullptr;

  //my drawDebug object
  DrawDebug *mDrawDebug = nullptr;

  PrepareData* mPrepareData = nullptr;
  GameText* mGameTexts = nullptr;
  TimeProfiler* mTimeProfiler = nullptr;

  OriginalGameFolderInfoStruct* mOriginalGame = nullptr;

  std::vector<uint8_t> mGameVersionDate;
  bool mExtendedGame = false;

  //if specified file is not found, returns empty path
  irr::io::path LocateFileInFileList(irr::io::IFileList* fileList, irr::core::string<fschar_t> fileName);
  bool UpdateFileListSaveFolder();

  //lets create and store a direction vector for
  //later use (calculations), so that we do not have to do this
  //over and over again
  irr::core::vector3d<irr::f32>* xAxisDirVector = nullptr;
  irr::core::vector3d<irr::f32>* yAxisDirVector = nullptr;
  irr::core::vector3d<irr::f32>* zAxisDirVector = nullptr;

  virtual void HandleGuiEvent(const irr::SEvent& event);
  virtual void HandleMouseEvent(const irr::SEvent& event);

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
  bool InitGameResourcesInitialStep();
};


#endif // INFRABASE_H
