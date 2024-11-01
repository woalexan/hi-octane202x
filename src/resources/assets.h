/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ASSETS_H
#define ASSETS_H

#include "irrlicht/irrlicht.h"
#include <vector>
#include "../utils/fileutils.h"
#include "../utils/crc32.h"

//if we ever want to implement the other languages
//stored in the game these are the values assigned to the current
//selected game language settings in CONFIG.DAT file
#define GAME_LANGUAGE_ENGLISH 0
#define GAME_LANGUAGE_GERMAN 1
#define GAME_LANGUAGE_FRENCH 2
#define GAME_LANGUAGE_SPANISH 3
#define GAME_LANGUAGE_ITALIAN 4

//values in CONFIG.DAT for the different race craft
//color schemes, this are the actual used values for each
//color scheme in this file
#define GAME_CRAFTCOLSCHEME_MADMEDICINE 0x06
#define GAME_CRAFTCOLSCHEME_ASSASSINS 0xAA
#define GAME_CRAFTCOLSCHEME_GOREHOUNDS 0x4A
#define GAME_CRAFTCOLSCHEME_FOOFIGHTERS 0x10
#define GAME_CRAFTCOLSCHEME_DETHFEST 0x8C
#define GAME_CRAFTCOLSCHEME_FIREPHREAKS 0x16
#define GAME_CRAFTCOLSCHEME_STORMRIDERS 0x17
#define GAME_CRAFTCOLSCHEME_BULLFROG 0x8F

struct RaceTrackInfoStruct {
    //number of this race track level
    irr::u8 levelNr;

    //the official name of the race track
    char name[50];

    //path to extracted obj
    //model for race track
    char meshFileName[50];

    //default number of laps
    //after game is run the first time
    //and user has not changed number of
    //laps yet
    irr::u8 defaultNrLaps;

    //the current (last selected number
    //of laps by user, is stored in config file)
    irr::u8 currSelNrLaps;

    //current best lap time
    irr::u32 bestLapTime;

    //name of current best player
    //on this race track
    //player names have max 8 chars
    char bestPlayer[10];

    //current best high score
    //for this race track
    irr::u32 bestHighScore;

    //name of current best high score
    //player
    //player names have max 8 chars
    char bestHighScorePlayer[10];

    //the mesh for the race track
    //at track selection page in menue
    irr::scene::IMesh* MeshTrack;
};

struct CraftInfoStruct {
    //number of this craft
    irr::u8 craftNr;

    //the official name
    //of this craft
    char name[50];

    //the stats of this craft
    irr::u8 statSpeed;
    irr::u8 statArmour;
    irr::u8 statWeight;
    irr::u8 statFirePower;

    //path to extracted obj
    //model for this craft
    char meshFileName[50];

    //we have a vector of meshes, because each craft
    //is available in different color schemes!
    std::vector<irr::scene::IMesh*> MeshCraft;
};

struct HighScoreEntryStruct {
    //number of this entry in the CONFIG.DAT file
    //starting with 0 for first entry

    //the name of the player for this entry
    //player names have max 8 chars
    char namePlayer[10];

    //the highscore value for this entry
    irr::u8 highscoreVal;

    //the integer value that contains the player
    //assessement according to the original games
    //assessement string table
    irr::u8 playerAssessementVal;
};

class Assets {
public:
    Assets(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::scene::ISceneManager* smgr,
           bool updateGameConfigFile);
    ~Assets();

    std::vector<RaceTrackInfoStruct*> *mRaceTrackVec;
    std::vector<CraftInfoStruct*> *mCraftVec;
    std::vector<char*> mCraftColorSchemeNames;
    std::vector<irr::u8> mCraftColorSchemeConfigDatFileValue;

    void SetNewMainPlayerName(char* newName);
    char* GetNewMainPlayerName();

    void SetCurrentChampionshipName(char* newName);
    char* GetCurrentChampionshipName();

    void SetCurrentGameLanguage(irr::u8 newLanguage);
    irr::u8 GetCurrentGameLanguage();

    void SetCurrentCraftColorScheme(irr::u8 newCraftColorScheme);
    irr::u8 GetCurrentCraftColorScheme();

    void SetNewRaceTrackDefaultNrLaps(irr::u32 nrRaceTrack, irr::u8 newNumberLaps);

    //assessementLevel goes from value 0 (highest appraisal) up
    //to value 20 (worst performance)
    char* GetDriverAssessementString(irr::u8 assessementLevel);
    irr::u8 GetNumberDriverAssessementStrings();

    //returns NULL in case of an unexpected error
    std::vector<HighScoreEntryStruct*>* GetHighScoreTable();

    void UpdateGameConfigFileExitGame();

private:
    irr::video::IVideoDriver* myDriver;
    irr::IrrlichtDevice* myDevice;
    irr::scene::ISceneManager* mySmgr;

    bool mUpdateGameConfigFile;

    irr::u8 currLevelNr = 1;
    irr::u8 currCraftNr = 1;

    void InitRaceTracks();
    void AddRaceTrack(char* nameTrack, char* meshFileName, irr::u8 defaultNrLaps);
    void InitCrafts();
    void AddCraft(char* nameCraft, char* meshFileName, irr::u8 statSpeed, irr::u8 statArmour, irr::u8 statWeight, irr::u8 statFirePower);

    char* currentConfigFileDataByteArray = NULL;
    size_t currentConfigFileDataByteArrayLen = 0;

    std::vector<char*>* driverAssessementStrings;
    void InitDriverAssessementStrings();
    void CleanUpDriverAssessementStrings();
    void AddDriverAssessementString(char* newString);

    bool mCurrentConfigFileRead;

    //Routines handling the default games config.dat file
    bool ReadGameConfigFile(char** targetBuf, size_t &outBufSize);
    bool WriteGameConfigFile();

    int32_t ConvertByteArray_ToInt32(char* bytes, size_t start_position);
    void ReadNullTerminatedString(char* bytes, size_t start_position, char* outString, irr::u8 maxStrLen);
    void WriteNullTerminatedString(char* bytes, size_t start_position, char* writeString, irr::u8 maxStrLen);

    //This functions only work in case the original games config file was already successfully read
    //before!
    bool DecodeCurrentRaceTrackStats(irr::u32 nrRaceTrack, RaceTrackInfoStruct* targetInfoStruct);
    bool DecodeMainPlayerName();
    bool DecodeHighScoreTable();
    bool DecodeCurrentGameLanguage();
    bool DecodeCurrentCraftColorScheme();
    bool DecodeCurrentChampionshipName();

    std::vector<HighScoreEntryStruct*>* highScoreTableVec;

    //game player names are limited to max
    //8 characters in Hi-Octane!
    //plus 1 termination char + 1 char extra :)
    char currMainPlayerName[10];

    //name of current active (last selected) championship in
    //config.dat file; length is limited to 12 characters in Hi-Octane!
    //plus 1 termination char + 1 char extra :)
    char currChampionshipName[14];

    //current selected game language, we only support english right now
    irr::u8 currSelectedGameLanguage = GAME_LANGUAGE_ENGLISH;

    //is the current selected craft colorscheme setting in CONFIG.DAT file
    //without config file the default start value if MAD MEDICINE
    irr::u8 currSelectedCraftColorScheme = GAME_CRAFTCOLSCHEME_MADMEDICINE;
};

#endif // ASSETS_H
