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

#define INFRA_LEVEL_ORIGINALGAME_VANILLA 0
#define INFRA_LEVEL_ORIGINALGAME_MODIFIED 1
#define INFRA_LEVEL_CUSTOM 2

//valid size of the unpacked level file in bytes
#define INFRA_LEVEL_FILE_VALIDSIZE_BYTES 896140

/************************
 * Forward declarations *
 ************************/

class Logger;
class PrepareData;
class GameText;
class MyEventReceiver;
class TimeProfiler;
class DrawDebug;
class Crc32;
class Attribution;

struct OriginalGameFolderInfoStruct {
    irr::io::IFileList* rootFolder = nullptr;
    irr::io::IFileList* dataFolder = nullptr;
    irr::io::IFileList* execFolder = nullptr;
    irr::io::IFileList* saveFolder = nullptr;
    irr::io::IFileList* mapsFolder = nullptr;
    irr::io::IFileList* objectsFolder = nullptr;
    irr::io::IFileList* soundFolder = nullptr;
};

struct LevelFolderInfoStruct {
    std::string levelName;
    irr::io::path levelFileName;
    irr::io::path levelBaseDir;
    bool isCustomLevel;
    std::string description;
};

struct OriginalGameCreditStruct {
    std::string role;
    std::vector<std::string> individualsVec;
};

class InfrastructureBase {
public:
  InfrastructureBase(int pArgc, char **pArgv);
  ~InfrastructureBase();

  //decoded command line parameter
  //information
  void ParseCommandLineInformation(int pArgc, char **pArgv);

  //Parsed command line information
  //first entry always contains the absolute path
  //to this executable
  std::vector<std::string> mCLIVec;

  bool mWriteLogFile = false;

  //native desktop resolution
  core::dimension2d<u32> mNativeResolution;

  void CleanupAllSceneNodes();

  std::string WStringToStdString(std::wstring inputStr);

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

  gui::IGUIFont* guiFont = nullptr;

  MyEventReceiver* mEventReceiver = nullptr;

  //my drawDebug object
  DrawDebug *mDrawDebug = nullptr;

  PrepareData* mPrepareData = nullptr;
  GameText* mGameTexts = nullptr;
  TimeProfiler* mTimeProfiler = nullptr;
  Crc32* mCrc32 = nullptr;
  Attribution* mAttribution = nullptr;

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

  //returns true if Gui Event should be canceled
  virtual bool HandleGuiEvent(const irr::SEvent& event);

  virtual void HandleMouseEvent(const irr::SEvent& event);

  bool mBlockPreviewEnabled = true;

  void CopyTexture(irr::video::ITexture* source, irr::video::ITexture* target);
  void FillTexture(irr::video::ITexture* target, unsigned char fillvalue);

  //Returns the number of bytes per pixel for a certain ECOLOR_FORMAT
  //returns 0 for an undefined ECOLOR_FORMAT
  irr::u32 ReturnBytesPerPixel(irr::video::ECOLOR_FORMAT colFormat);

  //Returns true in case of success, False otherwise
  bool WriteMiniMapCalFile(std::string fileName, irr::u32 startWP, irr::u32 endWP, irr::u32 startHP, irr::u32 endHP);

  //Returns true in case of success, False otherwise
  bool ReadMiniMapCalFile(std::string fileName, irr::u32 &startWP, irr::u32 &endWP, irr::u32 &startHP, irr::u32 &endHP);

  irr::io::path GetMiniMapFileName(LevelFolderInfoStruct* whichLevel);
  irr::io::path GetMiniMapFileName(std::string levelRootPath);

  irr::io::path GetMiniMapCalFileName(LevelFolderInfoStruct* whichLevel);
  irr::io::path GetMiniMapCalFileName(std::string levelRootPath);

  //Returns a vector of existing levels in a specified root directory
  void GetExistingLevelInfo(std::string rootDir, bool markAsCustomLevel, std::vector<LevelFolderInfoStruct*> &levelInfoVec);

  std::string GetDescriptionLevel(io::path levelFilename);

  //Returns true if specified unpacked level file seems to be valid
  //False otherwise
  bool IsUnpackedLevelFileValid(io::path levelFilename);

  //Returns true if specified character is valid for a levelname
  //False otherwise
  //valid are a-z, A-Zm 0-9, _, - characters
  bool IsValidCharForLevelName(wchar_t* whichChar);

  //Returns true if specified level name is valid
  //means only a-z, A-Zm 0-9, _, - characters
  //False otherwise
  bool CheckForValidLevelName(std::wstring levelName);

  //Returns true in case of success, False otherwise
  bool ParseOriginalGameCredits(std::vector<OriginalGameCreditStruct*>& originalGameCredits);

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

  bool ParseOriginalGameCreditsWorkaround(std::vector<uint8_t> mGameCreditsInformation, size_t startIdx);
};


#endif // INFRABASE_H
