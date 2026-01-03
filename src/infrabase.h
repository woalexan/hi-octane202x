/*
 Copyright (C) 2025-2026 Wolf Alexander

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

#define INFRA_RUNNING_AS_GAME 0
#define INFRA_RUNNING_AS_EDITOR 1

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
class FontManager;

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

struct MapConfigStruct {
    //Configuration for the Sky
    std::string SkyImageFileVanilla;
    std::string SkyImageFileUpgradedSky;
    irr::video::SColor cloudColorCenter1;
    irr::video::SColor cloudColorInner1;
    irr::video::SColor cloudColorOuter1;
    irr::video::SColor cloudColorCenter2;
    irr::video::SColor cloudColorInner2;
    irr::video::SColor cloudColorOuter2;
    irr::video::SColor cloudColorCenter3;
    irr::video::SColor cloudColorInner3;
    irr::video::SColor cloudColorOuter3;
    bool EnableLensFlare;
    irr::core::vector3df lensflareLocation;

    //Music configuration
    std::string MusicFile;

    //Minimap "calibration" value
    //to correctly map to X-Y
    //level coordinates
    bool minimapCalSet;
    irr::core::vector2di minimapCalStartVal;
    irr::core::vector2di minimapCalEndVal;

    //texture base location
    std::string texBaseLocation;
    bool useCustomTextures;
};

struct GameConfigStruct {
   bool enableDoubleResolution;
   bool useUpgradedSky;
   bool enableShadows;
   bool enableVSync;
   bool skipIntro;
};

class InfrastructureBase {
public:
  InfrastructureBase(int pArgc, char **pArgv, irr::u8 runningAsVal);
  ~InfrastructureBase();

  irr::u8 mRunningAs;

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

  //have all pointers as public
  //so that we can access them easily everywhere
  IrrlichtDevice* mDevice = nullptr;
  video::IVideoDriver* mDriver = nullptr;
  scene::ISceneManager* mSmgr = nullptr;

  IGUIEnvironment* mGuienv = nullptr;

  gui::IGUIFont* guiFont = nullptr;
  FontManager* mFontManager = nullptr;

  MyEventReceiver* mEventReceiver = nullptr;

  //my drawDebug object
  DrawDebug *mDrawDebug = nullptr;

  PrepareData* mPrepareData = nullptr;
  GameText* mGameTexts = nullptr;
  TimeProfiler* mTimeProfiler = nullptr;
  Crc32* mCrc32 = nullptr;
  Attribution* mAttribution = nullptr;

  //Note: only used for the game itself
  //not for the Level editor
  GameConfigStruct* mGameConfig = nullptr;

  OriginalGameFolderInfoStruct* mOriginalGame = nullptr;

  std::vector<uint8_t> mGameVersionDate;
  bool mExtendedGame = false;

  irr::io::IFileList* CreateFileList(irr::io::path whichAbsPath);

  //if specified file is not found, returns empty path
  irr::io::path LocateFileInFileList(irr::io::IFileList* fileList, irr::core::string<fschar_t> fileName,
                                     bool ignoreFileEnding = false);
  bool UpdateFileListSaveFolder();

  io::path RemoveFileEndingFromFileName(io::path fileName);
  io::path GetFileEndingFromFileName(io::path fileName);

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

  irr::io::path GetMiniMapFileName(LevelFolderInfoStruct* whichLevel);
  irr::io::path GetMiniMapFileName(std::string levelRootPath);

  irr::io::path GetMapConfigFileName(LevelFolderInfoStruct* whichLevel);
  irr::io::path GetMapConfigFileName(std::string levelRootPath);

  irr::core::stringw SColorToXmlSettingStr(irr::video::SColor* whichColor);
  irr::core::stringw Vector3dfToXmlSettingStr(irr::core::vector3df* whichVector);
  irr::core::stringw Vector2diToXmlSettingStr(irr::core::vector2di* whichVector);
  irr::core::stringw BoolToXmlSettingStr(bool inputVal);

  //Returns true in case the input string contains a number, False if there are any other characters
  //that are no digits;
  bool WStringContainsNumber(irr::core::stringw inputStr, bool allowDecimalNumber);

  //Returns false if Xml value payload string is missformed, True otherwise
  //The output payload string is returned in the second parameter
  bool GetXmlValuePayload(irr::core::stringw inputStr, irr::core::stringw &payloadStr);

  irr::u32 CntNumberCharacterOccurence(irr::core::stringw* inputStr, wchar_t delimiter);
  void SplitWStringAtDelimiterChar(irr::core::stringw* inputStr, wchar_t delimiter, std::vector<irr::core::stringw> &outWStrVec, bool addEmptyStrings = false);

  //Returns false if input string is invalid (can not be parsed), True otherwise
  //Output value is returned in second parameter
  bool XmlSettingStrToBool(irr::core::stringw inputStr, bool& outValue);

  //Returns false if input string is invalid (can not be parsed), True otherwise
  //Output value is returned in second parameter
  bool XmlSettingStrToVector3df(irr::core::stringw inputStr, irr::core::vector3df& outValue);

  //Returns false if input string is invalid (can not be parsed), True otherwise
  //Output value is returned in second parameter
  bool XmlSettingStrToVector2di(irr::core::stringw inputStr, irr::core::vector2di& outVector);

  //Returns false if input string is invalid (can not be parsed), True otherwise
  //Output value is returned in second parameter
  bool XmlSettingStrToSColor(irr::core::stringw inputStr, irr::video::SColor& outColor);

  bool GetCloudColorValue(irr::core::map<irr::core::stringw, irr::core::stringw> *valueMap, irr::core::stringw keyName, irr::video::SColor &outColor);

  //Returns true in case of success, False otherwise
  bool WriteMapConfigFile(const std::string fileName, MapConfigStruct* configStruct);

  //Returns true in case of success, False otherwise
  //Read values are returned in second parameter
  bool ReadMapConfigFile(const std::string fileName, MapConfigStruct &configStruct);

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

  //Returns true in case of success, False otherwise
  bool InitStage1();
  bool InitStage2();
  bool InitStage3();

  bool WriteGameConfigXmlFile(IrrlichtDevice *device);

  //Returns true in case of success, false otherwise
  bool LoadLevelConfigData(std::string levelRootPath, MapConfigStruct** outMapTarget);

private:
  //Irrlicht stuff
  bool mFullscreen;

  IGUIFont* fontAndika;

  //this paths are all absolute paths
  irr::io::path mGameRootDir;
  irr::io::path mOriginalGameRootDir;
  irr::io::path saveFolderDataPath;

  //Returns true if original game found,
  //False otherwise, resulting original
  //game root dir is stored in mOriginalGameRootDir
  bool LocateOriginalGame();
  bool DetermineOriginalGameVersion();
  bool ProcessGameVersionDate();

  //Returns true for success, false for error occured
  bool InitGameResourcesInitialStep();

  bool ParseOriginalGameCreditsWorkaround(std::vector<uint8_t> mGameCreditsInformation, size_t startIdx);

  bool ReadGameConfigXmlFile(IrrlichtDevice *device);
};


#endif // INFRABASE_H
