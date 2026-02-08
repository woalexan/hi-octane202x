/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "assets.h"
#include "../game.h"
#include "../race.h"

/* 30.03.2025: Additional settings in CONFIG.DAT for extended original game version:
 *
    offset: 0x34D value from 0 (lowest lives) up to 4 (seems to be 5 lives?):  ("Death match lives")
            Default value is the minimum (0) value
    offset: 0x34E value from 2 (2 hot seat players, min) up to 8 (max possible players): ("Hot Seat Players")
            The number of selected bars in the UI for this config setting can also not be moved below 2 selected bars
            Default value is the minimum value (2)
    offset: 0x34F value from 1 (lowest possible value) up to 5 (max possible value): ("Hot Seat Racing Time")
            The number of selected bars in the UI for this config setting can also not be moved below 1 selected bar
            Default value is the minimum value (1)

    offset: 0x350  Start of Player2 name if other name is set, if no name is set there is simply a 0 byte (null termination char there)
    offset: 0x370  Start of Player3 name, --
    offset: 0x390  Start of Player4 name, --
    offset: 0x3B0  Start of Player5 name, --
    offset: 0x3D0  Start of Player6 name, --
    offset: 0x3F0  Start of Player7 name, --
    offset: 0x410  Start of Player8 name, --

    Extended options that are not saved in the CONFIG.DAT file, but simply reset every time the game starts:

     "Use saved best clone lap": just set to true when game starts, not stored in CONFIG.DAT
 */

Assets::Assets(Game* game, bool updateGameConfigFile) {
    this->mGame = game;
    mUpdateGameConfigFile = updateGameConfigFile;

    mCurrentConfigFileRead = false;

    //try to locate config.dat file
    //location that was found is stored in member
    //variable mAbsPathConfigFile
    if (!FindGameConfigFile()) {
        //config.dat not found or invalid

        //create a completely new config.dat file from scratch
        //with default settings
        //this routine also sets up member config variables
        //directly inside with default values
        CreateNewConfigFileContents(&currentConfigFileDataByteArray, currentConfigFileDataByteArrayLen);

        //save the new file
        char filenameTestFile[100];

        strcpy(filenameTestFile, this->mGame->mOriginalGame->saveFolder->getPath().c_str());
        strcat(filenameTestFile, (char*)("config.dat"));

        //save the new config.dat file
        WriteGameConfigFile(filenameTestFile, &currentConfigFileDataByteArray, currentConfigFileDataByteArrayLen);

        //remember config.dat filename/file path
        mAbsPathConfigFile = strdup(filenameTestFile);
    } else {
        //config.dat file found
        if (ReadGameConfigFile(mAbsPathConfigFile, &currentConfigFileDataByteArray, currentConfigFileDataByteArrayLen)) {
            mCurrentConfigFileRead = true;
        }

        if (mCurrentConfigFileRead) {
            DecodeMainPlayerName(&currentConfigFileDataByteArray);
            DecodeHighScoreTable();
            DecodeCurrentGameLanguage();
            DecodeLastSelectedRaceTrack(&currentConfigFileDataByteArray);
            DecodeLastSelectedCraft(&currentConfigFileDataByteArray);
            DecodeGameDifficultySetting(&currentConfigFileDataByteArray);
            DecodeCurrentCraftColorScheme(&currentConfigFileDataByteArray);
            char* pntr = &this->currChampionshipName[0];
            DecodeCurrentChampionshipName(&currentConfigFileDataByteArray, &pntr);
            DecodeAudioVolumes();
            DecodeGraphicSettings(&currentConfigFileDataByteArray);

            //decode additional settings for extended version of the game
            if (mGame->mExtendedGame) {
                DecodeExtGameAdditionalPlayerNames(&currentConfigFileDataByteArray);
                DecodeDeathMatchLives(&currentConfigFileDataByteArray);
                DecodeHotSeatPlayers(&currentConfigFileDataByteArray);
                DecodeHotSeatRacingTime(&currentConfigFileDataByteArray);
            }
        }
    }

    //if we have no extended game version, make sure
    //that player2 up to player8 player name is empty
    //so that we always use the default names for computer players
    if (!mGame->mExtendedGame) {
        strcpy(currPlayer2Name, "");
        strcpy(currPlayer3Name, "");
        strcpy(currPlayer4Name, "");
        strcpy(currPlayer5Name, "");
        strcpy(currPlayer6Name, "");
        strcpy(currPlayer7Name, "");
        strcpy(currPlayer8Name, "");

        //also set default value for DeathMatchLives
        currDeathMatchLives = 0;
        currHotSeatPlayers = 2;
        currHotSeatRacingTime = 1;
    }

    InitDriverAssessementStrings();

    mRaceTrackVec = new std::vector<RaceTrackInfoStruct*>();
    mRaceTrackVec->clear();

    mCraftVec = new std::vector<CraftInfoStruct*>();
    mCraftVec->clear();

    mPilotVec = new std::vector<PilotInfoStruct*>();
    mPilotVec->clear();

    //init all race tracks
    InitRaceTracks();

    //init all crafts
    InitCrafts();

    //init all computer player pilots
    InitCpPilots();
}

Assets::~Assets() {
    std::vector<RaceTrackInfoStruct*>::iterator itRaceTrack;
    RaceTrackInfoStruct* pntrTrack;

    //delete all existing race tracks
    itRaceTrack = this->mRaceTrackVec->begin();

    while (itRaceTrack != this->mRaceTrackVec->end()) {
           pntrTrack = (*itRaceTrack);
           itRaceTrack = this->mRaceTrackVec->erase(itRaceTrack);

           //cleanup mesh
           mGame->mSmgr->getMeshCache()->removeMesh(pntrTrack->MeshTrack);

           //delete struct itself
           delete pntrTrack;
    }

    delete mRaceTrackVec;

    std::vector<PilotInfoStruct*>::iterator itPilot;
    PilotInfoStruct* pntrPilot;

    //delete all existing pilots
    itPilot = this->mPilotVec->begin();

    while (itPilot != this->mPilotVec->end()) {
           pntrPilot = (*itPilot);
           itPilot = this->mPilotVec->erase(itPilot);

           //delete struct itself
           delete pntrPilot;
    }

    delete mPilotVec;

    //delete all existing crafts (different color schemes)
    std::vector<CraftInfoStruct*>::iterator itCraft;
    CraftInfoStruct* pntrCraft;

    itCraft = this->mCraftVec->begin();

    while (itCraft != this->mCraftVec->end()) {
           pntrCraft = (*itCraft);
           itCraft = this->mCraftVec->erase(itCraft);

           //cleanup all meshes (we have different color schemes)
           for (unsigned long j = 0; j < pntrCraft->MeshCraft.size(); j++) {
                mGame->mSmgr->getMeshCache()->removeMesh(pntrCraft->MeshCraft.at(j));
           }

           //delete struct itself
           delete pntrCraft;
    }

    delete mCraftVec;

    //delete all colorscheme names for craft
    std::vector<char*>::iterator itNames;
    char* pntrStr;
    for (itNames = this->mCraftColorSchemeNames.begin(); itNames != this->mCraftColorSchemeNames.end(); ++itNames) {
        pntrStr = (*itNames);

        itNames = this->mCraftColorSchemeNames.erase(itNames);

        //delete current string
        delete[] pntrStr;
    }

    CleanUpDriverAssessementStrings();

    if (mCurrentConfigFileRead) {
        delete[] this->currentConfigFileDataByteArray;
    }

    //free abs path member variables
    //to config file
    free(mAbsPathConfigFile);
    mAbsPathConfigFile = nullptr;
}

//Helper function which compares our internal config data
//with the current contents from the config.dat file
//if there are differences the config.dat file is written
//when exiting the game
void Assets::UpdateGameConfigFileExitGame() {
    //if we never read the config.file
    //or should not modify game config file simply exit
    if (!mCurrentConfigFileRead || !mUpdateGameConfigFile)
        return;

    char* compDataArrStr = nullptr;
    size_t compDataLength;

    //reread current file contents in config.dat for
    //comparing with internal config data
    if (!ReadGameConfigFile(this->mAbsPathConfigFile, &compDataArrStr, compDataLength)) {
        return;
    }

    bool rewriteFile = false;

    //now compare data
    if (currentConfigFileDataByteArrayLen != compDataLength) {
        rewriteFile = true;
    } else {
        for (size_t pos = 0; pos < compDataLength; pos++) {
            if (compDataArrStr[pos] != currentConfigFileDataByteArray[pos]) {
                rewriteFile = true;
                break;
            }
        }
    }

    if (rewriteFile) {
        //write modified CONFIG.DAT file
        WriteGameConfigFile(mAbsPathConfigFile, &currentConfigFileDataByteArray, currentConfigFileDataByteArrayLen);
    }

    delete[] compDataArrStr;
}

void Assets::ReadNullTerminatedString(char* bytes, size_t start_position, char** outString, irr::u8 maxStrLen) {
  irr::u8 currRemaining = maxStrLen;
  bool endFound = false;
  char currChar;
  size_t currPos = start_position;

  char helpstr[2];

  strcpy(*outString, "");

  while (!endFound && currRemaining > 0) {
      currChar = bytes[currPos];

      if (currChar == 0) {
          endFound = true;
      } else {
          helpstr[0] = currChar;
          helpstr[1] = 0;

          strcat(*outString, helpstr);

          currRemaining--;
          currPos++;
      }
  }
}

void Assets::WriteNullTerminatedString(char* bytes, size_t start_position, char* writeString, irr::u8 maxStrLen) {

  irr::u8 currRemaining = maxStrLen;
  bool endFound = false;
  char currChar;
  size_t currPosDest = start_position;
  size_t currPosSrc = 0;

  while (!endFound && currRemaining > 0) {
      currChar = writeString[currPosSrc];

      if (currChar == 0) {
          endFound = true;
      } else {
          bytes[currPosDest] = currChar;

          currRemaining--;
          currPosDest++;
          currPosSrc++;
      }
  }
}

//nrRaceTrack = level number starting with 1
void Assets::DecodeCurrentRaceTrackStats(irr::u32 nrRaceTrack, RaceTrackInfoStruct* targetInfoStruct) {
    //config file offsets found for the current
    //racetrack stats (each racetrack seems to have 74 bytes of stat
    //data available)
    //level 1 = 0x29AA = 10666 dec
    //level 2 = 0x29F4 = 10740 dec
    //level 3 = 0x2A3E = 10814 dec
    //level 4 = 0x2A88 = 10888 dec
    //level 5 = 0x2AD2 = 10962 dec
    //level 6 = 0x2B1C = 11036 dec
    //...

    //data layout for each race track stat in config.dat seems to be
    //[4 Bytes best lap value][9 bytes for player name best lap, 9th byte = string termination char][23 bytes of
    //unknown data][4 Bytes best race value][9 bytes for player name best race, 9th byte = string termination char]
    //[23 bytes of unknown data][1 Byte for current number laps set][1 Byte of unknown data]

    //calculate correct offset in file
    //for specified race track
    size_t dataOffset = 0x29AA + 74 * (nrRaceTrack - 1);

    //read best lap time
    targetInfoStruct->bestLapTime = (irr::u32)(ConvertByteArray_ToInt32(currentConfigFileDataByteArray, dataOffset));
    //player names in Hi-Octane are limited to max 8 characters
    char* pntr = &targetInfoStruct->bestPlayer[0];

    ReadNullTerminatedString(currentConfigFileDataByteArray, dataOffset + 4, &pntr, 8);

    //read best highscore
    targetInfoStruct->bestHighScore = (irr::u32)(ConvertByteArray_ToInt32(currentConfigFileDataByteArray, dataOffset + 36));
    //player names in Hi-Octane are limited to max 8 characters
    pntr = &targetInfoStruct->bestHighScorePlayer[0];

    ReadNullTerminatedString(currentConfigFileDataByteArray, dataOffset + 40, &pntr, 8);

    //get current number of set laps for this race track
    targetInfoStruct->currSelNrLaps = (irr::u8)(currentConfigFileDataByteArray[dataOffset + 72]);
}

//helper function which reads the current set main player (player1)
//name from CONFIG.DAT file
void Assets::DecodeMainPlayerName(char** bufPntr) {
    //current player 1 name is stored in CONFIG.DAT
    //at offset 0x094F
    //player names in Hi-Octane are limited to max 8 characters
    char* pntr = &this->currMainPlayerName[0];

    ReadNullTerminatedString((*bufPntr), 0x094F, &pntr, 8);
}

//helper function for extended game version config.dat which reads possible
//players 2 up to 8 set names from CONFIG.DAT file
void Assets::DecodeExtGameAdditionalPlayerNames(char** bufPntr) {
    //additional player 2 up to 8 names are stored in CONFIG.DAT
    //at the following offsets
    //player names in Hi-Octane are limited to max 8 characters
    //offset: 0x350  Start of Player2 name if other name is set, if no name is set there is simply a 0 byte (null termination char there)
    char* pntr = &this->currPlayer2Name[0];
    ReadNullTerminatedString((*bufPntr), 0x350, &pntr, 8);

    //offset: 0x370  Start of Player3 name, --
    pntr = &this->currPlayer3Name[0];
    ReadNullTerminatedString((*bufPntr), 0x370, &pntr, 8);

    //offset: 0x390  Start of Player4 name, --
    pntr = &this->currPlayer4Name[0];
    ReadNullTerminatedString((*bufPntr), 0x390, &pntr, 8);

    //offset: 0x3B0  Start of Player5 name, --
    pntr = &this->currPlayer5Name[0];
    ReadNullTerminatedString((*bufPntr), 0x3B0, &pntr, 8);

    //offset: 0x3D0  Start of Player6 name, --
    pntr = &this->currPlayer6Name[0];
    ReadNullTerminatedString((*bufPntr), 0x3D0, &pntr, 8);

    //offset: 0x3F0  Start of Player7 name, --
    pntr = &this->currPlayer7Name[0];
    ReadNullTerminatedString((*bufPntr), 0x3F0, &pntr, 8);

    //offset: 0x410  Start of Player8 name, --
    pntr = &this->currPlayer8Name[0];
    ReadNullTerminatedString((*bufPntr), 0x410, &pntr, 8);
}

//helper function which reads the current championship
//name from CONFIG.DAT file
void Assets::DecodeCurrentChampionshipName(char** bufPntr, char** outStr) {
    //current championship name is stored in CONFIG.DAT
    //at offset 0x0
    //championship name in Hi-Octane is limited to max 12 characters
    ReadNullTerminatedString((*bufPntr), 0x0, outStr, 12);
}

int32_t Assets::ConvertByteArray_ToInt32(char* bytes, size_t start_position) {
    int32_t result;

    result = static_cast<int32_t>(  (bytes[start_position + 3] << 24) +
                                    (bytes[start_position + 2] << 16) +
                                    (bytes[start_position + 1] << 8) +
                                    (bytes[start_position]));
    return (result);
}

void Assets::ConvertInt32_ToByteArray(char* outBytes, size_t start_position, int32_t inputVal) {
    uint8_t byteLSB = (uint8_t)(inputVal & 0xFF);
    uint8_t byteLeftOfLSB = (uint8_t)((inputVal >> 8) & 0xFF);
    uint8_t byteRightOfMSB = (uint8_t)((inputVal >> 16) & 0xFF);
    uint8_t byteMSB = (uint8_t)((inputVal >> 24) & 0xFF);

    outBytes[start_position] = byteLSB;
    outBytes[start_position + 1] = byteLeftOfLSB;
    outBytes[start_position + 2] = byteRightOfMSB;
    outBytes[start_position + 3] = byteMSB;
}

//allows to set a new number of default laps for a racetrack
//if the new number is different to the current value will also
//update value in CONFIG.DAT file, if file update is enabled
void Assets::SetNewRaceTrackDefaultNrLaps(irr::u32 nrRaceTrack, irr::u8 newNumberLaps) {
    //only update if value has changed at all!
    if (this->mRaceTrackVec->at(nrRaceTrack)->currSelNrLaps != newNumberLaps) {
        this->mRaceTrackVec->at(nrRaceTrack)->currSelNrLaps = newNumberLaps;

        //config file offsets found for the current
        //racetrack stats (each racetrack seems to have 74 bytes of stat
        //data available)
        //level 1 = 0x29AA = 10666 dec
        //level 2 = 0x29F4 = 10740 dec
        //level 3 = 0x2A3E = 10814 dec
        //level 4 = 0x2A88 = 10888 dec
        //level 5 = 0x2AD2 = 10962 dec
        //level 6 = 0x2B1C = 11036 dec
        //...

        //data layout for each race track stat in config.dat seems to be
        //[4 Bytes best lap value][9 bytes for player name best lap, 9th byte = string termination char][23 bytes of
        //unknown data][4 Bytes best race value][9 bytes for player name best race, 9th byte = string termination char]
        //[23 bytes of unknown data][1 Byte for current number laps set][1 Byte of unknown data, must have 0x0F value]

        //calculate correct offset in file
        //for specified race track
        size_t dataOffset = 0x29AA + 74 * (nrRaceTrack - 1);

        //set new current number of laps for this race track
        currentConfigFileDataByteArray[dataOffset + 72] = char(newNumberLaps);
    }
}

void Assets::AddDriverAssessementString(const char* newString) {
    char* stringPntr = strdup(newString);

    driverAssessementStrings->push_back(stringPntr);
}

void Assets::InitDriverAssessementStrings() {
  driverAssessementStrings = new std::vector<char*>();
  driverAssessementStrings->clear();

  //Strings taken from Hi-Octane Exe file
  //With the last ones I am not sure anymore if the belong to
  //the driver assessement or not. I will need to find this out in
  //the future
  AddDriverAssessementString("IMMORTAL");
  AddDriverAssessementString("HARDWIRED");
  AddDriverAssessementString("OUTTA CONTROL");
  AddDriverAssessementString("DEMON");
  AddDriverAssessementString("DEADLY");
  AddDriverAssessementString("WRECKER");
  AddDriverAssessementString("DANGEROUS");
  AddDriverAssessementString("TAILGATER");
  AddDriverAssessementString("ANTISOCIAL");
  AddDriverAssessementString("DUST DEVIL");
  AddDriverAssessementString("UPWARDLY MOBILE");
  AddDriverAssessementString("MEDIOCRE");
  AddDriverAssessementString("TOO SLOW");
  AddDriverAssessementString("ENDANGERED SPECIES");
  AddDriverAssessementString("TARGET PRACTICE");
  AddDriverAssessementString("BACK MARKER");
  AddDriverAssessementString("CANYON KISSER");
  AddDriverAssessementString("VICTIM");
  AddDriverAssessementString("SCRAP");
  AddDriverAssessementString("SMEAR");
  //I believe the following string does not actually belong
  //to the driver assessement strings in Hi-Octance, because
  //the race stats page lists 20 different ratings, and not 21
  //AddDriverAssessementString("CHEATING SUCKS");
}

irr::u8 Assets::GetNumberDriverAssessementStrings() {
    return (irr::u8)(this->driverAssessementStrings->size());
}

char* Assets::GetDriverAssessementString(irr::u8 assessementLevel) {
    return this->driverAssessementStrings->at(assessementLevel);
}

void Assets::CleanUpDriverAssessementStrings() {
  std::vector<char*>::iterator it;
  char* strPntr;

  for (it = this->driverAssessementStrings->begin(); it != this->driverAssessementStrings->end(); ) {
      strPntr = (*it);

      //remove entry from vector
      it = this->driverAssessementStrings->erase(it);

      //free string itself as well
      free(strPntr);
  }
}

//returns true if a config.dat file was
//located that is valid, the absolute path to this file is stored
//in member variable mAbsPathConfigFile
bool Assets::FindGameConfigFile() {
    //we know there must be already a subfolder "HIOCTANE.CD/SAVE"
    //in the original games folder, because the source code in the infrabase
    //takes care of this. If the folder was not there at the first start
    //of the game this folder is created
    //we just need to check now if the config.dat file is already there as well
    //or if not

    mAbsPathConfigFile = nullptr;

    irr::io::path configFilePath =
            mGame->LocateFileInFileList(mGame->mOriginalGame->saveFolder, irr::core::string<fschar_t>("config.dat"));

    if (configFilePath.empty()) {
        logging::Info("I did not find an existing config.dat file => create default config");
        return false;
    }

    //does the config.dat file have a valid file size?
    FILE* iFile;

    iFile = fopen(configFilePath.c_str(), "rb");
    fseek(iFile, 0L, SEEK_END);
    size_t sizeFile = ftell(iFile);
    fclose(iFile);

    if (((!mGame->mExtendedGame) && (sizeFile != CONFIG_DAT_SIZE_DEFAULT_GAME))
         || ((mGame->mExtendedGame) && (sizeFile != CONFIG_DAT_SIZE_EXTENDED_GAME))) {
        logging::Warning("Existing config.dat file filesize does not fit to expected size => recreate it");
        return false;
    }

    //we found a valid config.dat file
    //create a copy of this string and store
    //it in member variable
    //we need to free this variable later!
    mAbsPathConfigFile = strdup(configFilePath.c_str());

    //we did find the config.dat file
    return true;
}

//this function reads an existing original game
//config file
//returns true if a config file was read
//false otherwise
//is also used for reading championship game save files
bool Assets::ReadGameConfigFile(char* filename, char** targetBuf, size_t &outBufSize) {
  //read specified file
  FILE* iFile;

  iFile = fopen(filename, "rb");
  fseek(iFile, 0L, SEEK_END);
  size_t size = ftell(iFile);
  fseek(iFile, 0L, SEEK_SET);

  size_t counter = 0;

  *targetBuf = new char[size];

  //remember amount of data for the next write
  outBufSize = size;

  if (iFile != nullptr)
  {
      do {
          (*targetBuf)[counter] = fgetc(iFile);
          counter++;
      } while (counter < size);
      fclose(iFile);
  } else {
      delete[] *targetBuf;
      *targetBuf = nullptr;

      return false;
  }

  return true;
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetNewMainPlayerName(char* newName) {
    SetNewMainPlayerName(newName, &currentConfigFileDataByteArray);
}

void Assets::SetNewMainPlayerName(char* newName, char** bufPntr) {
    strcpy(this->currMainPlayerName, newName);

    //write new name also in the config file
    //data array
    //main player name is stored at offset 0x094F in CONFIG.DAT
    //name is limited to 8 characters only!
    WriteNullTerminatedString(*bufPntr, 0x094F, currMainPlayerName, 8);

    //make sure at byte 9 after this offset with store a 0 character (string termination)
    (*bufPntr)[0x094F + 9] = 0;
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetNewExtGameAdditionalPlayerNames(irr::u8 setPlayerNr,
                                                char* newName) {
    SetNewExtGameAdditionalPlayerNames(setPlayerNr, newName,
                             &currentConfigFileDataByteArray);
}

//function for the extended version of the game, where the player 2 up to 8
//names can also be set as well, valid value for setPlayerNr is from 2 (2nd player)
//up to max 8 (8th player)
void Assets::SetNewExtGameAdditionalPlayerNames(irr::u8 setPlayerNr,
                 char* newName, char** bufPntr) {
    if (!mGame->mExtendedGame)
        return;

    if ((setPlayerNr < 2) || (setPlayerNr > 8))
        return;

    switch (setPlayerNr) {
        case 2: {
            strcpy(this->currPlayer2Name, newName);

            //write new name also in the config file
            //data array
            //2nd player name is stored at offset 0x350 in CONFIG.DAT
            //name is limited to 8 characters only!
            WriteNullTerminatedString(*bufPntr, 0x350, currPlayer2Name, 8);

            //make sure at byte 9 after this offset with store a 0 character (string termination)
            (*bufPntr)[0x350 + 9] = 0;
            break;
         }

        case 3: {
            strcpy(this->currPlayer3Name, newName);

            //write new name also in the config file
            //data array
            //3rd player name is stored at offset 0x370 in CONFIG.DAT
            //name is limited to 8 characters only!
            WriteNullTerminatedString(*bufPntr, 0x370, currPlayer3Name, 8);

            //make sure at byte 9 after this offset with store a 0 character (string termination)
            (*bufPntr)[0x370 + 9] = 0;
            break;
         }

        case 4: {
            strcpy(this->currPlayer4Name, newName);

            //write new name also in the config file
            //data array
            //4th player name is stored at offset 0x390 in CONFIG.DAT
            //name is limited to 8 characters only!
            WriteNullTerminatedString(*bufPntr, 0x390, currPlayer4Name, 8);

            //make sure at byte 9 after this offset with store a 0 character (string termination)
            (*bufPntr)[0x390 + 9] = 0;
            break;
         }

        case 5: {
            strcpy(this->currPlayer5Name, newName);

            //write new name also in the config file
            //data array
            //5th player name is stored at offset 0x3B0 in CONFIG.DAT
            //name is limited to 8 characters only!
            WriteNullTerminatedString(*bufPntr, 0x3B0, currPlayer5Name, 8);

            //make sure at byte 9 after this offset with store a 0 character (string termination)
            (*bufPntr)[0x3B0 + 9] = 0;
            break;
         }

        case 6: {
            strcpy(this->currPlayer6Name, newName);

            //write new name also in the config file
            //data array
            //6th player name is stored at offset 0x3D0 in CONFIG.DAT
            //name is limited to 8 characters only!
            WriteNullTerminatedString(*bufPntr, 0x3D0, currPlayer6Name, 8);

            //make sure at byte 9 after this offset with store a 0 character (string termination)
            (*bufPntr)[0x3D0 + 9] = 0;
            break;
         }

        case 7: {
            strcpy(this->currPlayer7Name, newName);

            //write new name also in the config file
            //data array
            //7th player name is stored at offset 0x3F0 in CONFIG.DAT
            //name is limited to 8 characters only!
            WriteNullTerminatedString(*bufPntr, 0x3F0, currPlayer7Name, 8);

            //make sure at byte 9 after this offset with store a 0 character (string termination)
            (*bufPntr)[0x3F0 + 9] = 0;
            break;
         }

        case 8: {
            strcpy(this->currPlayer8Name, newName);

            //write new name also in the config file
            //data array
            //8th player name is stored at offset 0x410 in CONFIG.DAT
            //name is limited to 8 characters only!
            WriteNullTerminatedString(*bufPntr, 0x410, currPlayer8Name, 8);

            //make sure at byte 9 after this offset with store a 0 character (string termination)
            (*bufPntr)[0x410 + 9] = 0;
            break;
         }
    }
}

//only available in extended version of the game
void Assets::DecodeDeathMatchLives(char** bufPntr) {
    //current value for death match lives is stored at
    //0x34D in CONFIG.DAT
    //value 0
    //value 1
    //value 2
    //value 3
    //value 4 is the highest possible value
    this->currDeathMatchLives = (irr::u8)((*bufPntr)[0x34D]);
}

//only useful for extended version of the
//original game
irr::u8 Assets::GetCurrentDeathMatchLives() {
    return currDeathMatchLives;
}

//only available in extended version of the game
void Assets::DecodeHotSeatPlayers(char **bufPntr) {
    //current value for hot seat players is stored at
    //0x34E in CONFIG.DAT
    //value 2 min up to 8 max
    this->currHotSeatPlayers = (irr::u8)((*bufPntr)[0x34E]);
}

//only available in extended version of the game
void Assets::DecodeHotSeatRacingTime(char **bufPntr) {
    //current value for hot seat racing time is stored at
    //0x34F in CONFIG.DAT
    //value 1 min up to 5 max
    this->currHotSeatRacingTime = (irr::u8)((*bufPntr)[0x34F]);
}

char* Assets::GetNewMainPlayerName() {
    return this->currMainPlayerName;
}

std::string Assets::GetCraftModelName(char* craftName, irr::u8 selectedCraftColorScheme) {
    CraftInfoStruct* craft = nullptr;
    std::vector<CraftInfoStruct*>::iterator itCraft;
    std::string resultStr("");

    //search the craft with the right name
    for (itCraft = this->mCraftVec->begin(); itCraft != this->mCraftVec->end(); ++itCraft) {
        if (strcmp((*itCraft)->name, craftName) == 0) {
            //we found the right craft
            craft = (*itCraft);
            break;
        }
    }

    if (craft != nullptr) {
        char* meshFileName = craft->meshFileName;

        for (irr::u16 idx = 0; meshFileName[idx] != 0; idx++) {
            resultStr.push_back(meshFileName[idx]);
        }

        //add number for color scheme
        char number[5];
        sprintf(number, "%d", GetColorSchemeIndexNumberFromColorScheme(selectedCraftColorScheme));

        resultStr.push_back(number[0]);
        resultStr.append(".obj");
    }

   return resultStr;
}

//rotates through the available color schemes, so
//that every player has a different color scheme
irr::u8 Assets::RotateColorScheme(irr::u8 currentColorScheme) {
    irr::u8 nextColorScheme;

    switch (currentColorScheme) {
        case GAME_CRAFTCOLSCHEME_MADMEDICINE: {nextColorScheme = GAME_CRAFTCOLSCHEME_ASSASSINS; break;}
        case GAME_CRAFTCOLSCHEME_ASSASSINS: {nextColorScheme = GAME_CRAFTCOLSCHEME_GOREHOUNDS; break;}
        case GAME_CRAFTCOLSCHEME_GOREHOUNDS: {nextColorScheme = GAME_CRAFTCOLSCHEME_FOOFIGHTERS; break;}
        case GAME_CRAFTCOLSCHEME_FOOFIGHTERS: {nextColorScheme = GAME_CRAFTCOLSCHEME_DETHFEST; break;}
        case GAME_CRAFTCOLSCHEME_DETHFEST: {nextColorScheme = GAME_CRAFTCOLSCHEME_FIREPHREAKS; break;}
        case GAME_CRAFTCOLSCHEME_FIREPHREAKS: {nextColorScheme = GAME_CRAFTCOLSCHEME_STORMRIDERS; break;}
        case GAME_CRAFTCOLSCHEME_STORMRIDERS: {nextColorScheme = GAME_CRAFTCOLSCHEME_BULLFROG; break;}
        case GAME_CRAFTCOLSCHEME_BULLFROG: {nextColorScheme = GAME_CRAFTCOLSCHEME_MADMEDICINE; break;}
        default: {nextColorScheme = GAME_CRAFTCOLSCHEME_MADMEDICINE; break;} //default is MADMEDICINE
    }

    return (nextColorScheme);
}

//Helper function to implemented mechanism with possible
//changed player2 up to player8 names in extended original
//game version, valid range for variable currPlayerNr is 2 up to 8
char* Assets::GetCurrentAdditionalPlayerNames(irr::u8 currPlayerNr, char* defaultName) {
    //the non extended game always uses the default
    //player/pilot names
    if (!mGame->mExtendedGame) {
        return (defaultName);
    }

    if ((currPlayerNr < 2) || (currPlayerNr > 8)) {
        return (defaultName);
    }

    //for the extended game version we need to check if another player
    //name was specified in the current game config
    //if yes, then return the changed name from current game config,
    //if not, also return the default name
    switch (currPlayerNr) {
        case 2: {
           if (currPlayer2Name[0] == 0) {
              return (defaultName);
           } else {
               return (currPlayer2Name);
           }

           break;
        }

        case 3: {
           if (currPlayer3Name[0] == 0) {
              return (defaultName);
           } else {
               return (currPlayer3Name);
           }

           break;
        }

        case 4: {
           if (currPlayer4Name[0] == 0) {
              return (defaultName);
           } else {
               return (currPlayer4Name);
           }

           break;
        }

        case 5: {
           if (currPlayer5Name[0] == 0) {
              return (defaultName);
           } else {
               return (currPlayer5Name);
           }

           break;
        }

        case 6: {
           if (currPlayer6Name[0] == 0) {
              return (defaultName);
           } else {
               return (currPlayer6Name);
           }

           break;
        }

        case 7: {
           if (currPlayer7Name[0] == 0) {
              return (defaultName);
           } else {
               return (currPlayer7Name);
           }

           break;
        }

        case 8: {
           if (currPlayer8Name[0] == 0) {
              return (defaultName);
           } else {
               return (currPlayer8Name);
           }

           break;
        }

        default: {
             return (defaultName);
             break;
        }
    }
}

std::vector<PilotInfoStruct*> Assets::GetPilotInfoNextRace(bool addHumanPlayer, bool addComputerPlayers) {
    std::vector<PilotInfoStruct*> pilotInfo;

    pilotInfo.clear();

    irr::u8 currPilotNumber = 1;
    irr::u8 currentCpPlayerColorScheme;

    if (addHumanPlayer) {
        //add the human player
        PilotInfoStruct *newPlayer = new PilotInfoStruct();
        newPlayer->pilotNr = currPilotNumber;
        currPilotNumber++;

        strcpy(newPlayer->pilotName, GetNewMainPlayerName());

        newPlayer->humanPlayer = true;
        strcpy(newPlayer->defaultCraftName, mCraftVec->at(currMainPlayerCraftSelected)->name);
        newPlayer->currSelectedCraftColorScheme = currSelectedCraftColorScheme;

        currentCpPlayerColorScheme = RotateColorScheme(currSelectedCraftColorScheme);

        pilotInfo.push_back(newPlayer);
    } else {
        currentCpPlayerColorScheme = currSelectedCraftColorScheme;

        //if no human is involved, start pilot number with 2
        currPilotNumber = 2;
    }

    if (addComputerPlayers) {
        std::vector<PilotInfoStruct*>::iterator itPilot;

        for (itPilot = mPilotVec->begin(); itPilot != mPilotVec->end(); ++itPilot) {
            PilotInfoStruct *newPlayer = new PilotInfoStruct();
            newPlayer->pilotNr = currPilotNumber;
            currPilotNumber++;

            //function GetCurrentAdditionalPlayerNames takes care that we choose the correct names
            //for additional players, regardless if extended game version or not
            strcpy(newPlayer->pilotName, GetCurrentAdditionalPlayerNames(newPlayer->pilotNr, (*itPilot)->pilotName));
            newPlayer->humanPlayer = ((*itPilot)->humanPlayer);

            strcpy(newPlayer->defaultCraftName, (*itPilot)->defaultCraftName);

            //computer player craft color schemes seem to rotate through the available
            //game color schemes, if one restarts a race multiple times the choosen colors are always
            //the same for the same players, as long as the player selects the same color scheme and
            //player craft; so there seems to be nothing random about it
            newPlayer->currSelectedCraftColorScheme = currentCpPlayerColorScheme;

            //rotate to next available color scheme, so that all computer players use a
            //different craft color
            currentCpPlayerColorScheme = RotateColorScheme(currentCpPlayerColorScheme);

            pilotInfo.push_back(newPlayer);
        }
    }

    return pilotInfo;
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetCurrentChampionshipName(char* newName) {
    SetCurrentChampionshipName(newName, &currentConfigFileDataByteArray);
}

void Assets::SetCurrentChampionshipName(char* newName, char** bufPntr) {
    strcpy(this->currChampionshipName, newName);

    //write new name also in the config file
    //data array
    //current championship name is stored at offset 0x0 in CONFIG.DAT
    //name is limited to 12 characters only!
    WriteNullTerminatedString(*bufPntr, 0x0, currChampionshipName, 12);

    //make sure at byte 13 after this offset with store a 0 character (string termination)
    (*bufPntr)[0x0 + 13] = 0;
}

char* Assets::GetCurrentChampionshipName() {
    return this->currChampionshipName;
}

//returns nullptr in case of an unexpected error
std::vector<HighScoreEntryStruct*>* Assets::GetHighScoreTable() {
    //if we did not read config file before
    //we must return with problem
    if (!this->mCurrentConfigFileRead) {
       return nullptr;
    }

    return this->highScoreTableVec;
}

//this function writes to an existing original game
//config file that was read before
//returns true in case of success
//false otherwiese
bool Assets::WriteGameConfigFile(char* filename, char** sourceBuf, size_t inBufSize) {
  FILE* oFile;

  oFile = fopen(filename, "wb");

  size_t counter = 0;

  if (oFile != nullptr)
  {
      do {
          fputc((*sourceBuf)[counter], oFile);
          counter++;
      } while (counter < inBufSize);
      fclose(oFile);

      return true;
  }

  return false;
}

void Assets::DecodeHighScoreTable() {
  highScoreTableVec = new std::vector<HighScoreEntryStruct*>;
  highScoreTableVec->clear();

  //this are the highscore entry start offsets
  //there are more entries, but I read only the first 20 entries
  //right now, because the game limits highscore table drawing to first 19
  //entries
  //pos 1:  0x0994
  //pos 2:  0x09BB  => 39 bytes for each highscore entry
  //pos 3:  0x09E2
  //pos 4:  0x0A09
  //pos 5:  0x0A30
  //pos 6:  0x0A57
  //pos 7:  0x0A7E
  //pos 8:  0x0AA5
  //pos 9:  0x0ACC
  //pos 10: 0x0AF3
  //pos 11: 0x0B1A
  //pos 12: 0x0B41
  //pos 13: 0x0B68
  //pos 14: 0x0B8F
  //pos 15: 0x0BB6
  //pos 16: 0x0BDD
  //pos 17: 0x0C04
  //pos 18: 0x0C2B
  //pos 19: 0x0C52
  //pos 20: 0x0C79

  //data layout for each highscore table entry in config.dat seems to be
  //[1 byte for highscore value][3 bytes of unknown data][1 bytes for player assessement
  //string index][9 bytes for player name best lap, 9th byte = string termination char][25 bytes of
  //unknown data]

  HighScoreEntryStruct* newEntry;
  size_t dataOffset;

  for (irr::u8 cnt = 0; cnt < 19; cnt++) {
      newEntry = new HighScoreEntryStruct();

      dataOffset = 0x0994 + cnt * 39;

      newEntry->highscoreVal = (irr::u8)(currentConfigFileDataByteArray[dataOffset]);
      newEntry->playerAssessementVal = (irr::u8)(currentConfigFileDataByteArray[dataOffset + 4]);

      //player names in Hi-Octane are limited to max 8 characters
      char* pntr = &newEntry->namePlayer[0];
      ReadNullTerminatedString(currentConfigFileDataByteArray, dataOffset + 5, &pntr, 8);

      this->highScoreTableVec->push_back(newEntry);
  }
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetCurrentGameLanguage(irr::u8 newLanguage) {
    SetCurrentGameLanguage(newLanguage, &currentConfigFileDataByteArray);
}

void Assets::SetCurrentGameLanguage(irr::u8 newLanguage, char** bufPntr) {
     this->currSelectedGameLanguage = newLanguage;

     //write new selected game language also into
     //data array
     //selected language is stored at offset 0x2C7A in CONFIG.DAT
     //for LANGUAGE_ENGLISH 0x0
     //for LANGUAGE_GERMAN 0x1
     //for LANGUAGE_FRENCH 0x2
     //for LANGUAGE_SPANISH 0x3
     //for LANGUAGE_ITALIAN 0x4
     (*bufPntr)[0x2C7A] = char(newLanguage);
}

irr::u8 Assets::GetCurrentGameLanguage() {
    return this->currSelectedGameLanguage;
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetNewLastSelectedRaceTrack(int newLevelNumber) {
    SetNewLastSelectedRaceTrack(newLevelNumber, &currentConfigFileDataByteArray);
}

//levelNumber starts with index 0 for level 1!
void Assets::SetNewLastSelectedRaceTrack(int newLevelNumber, char** bufPntr) {
     this->currLevelSelected = newLevelNumber;

     //last selected Race track is stored at offset
     //0x2C76 in CONFIG.DAT
     //value 0 means level 1, value 1 means level 2 in the binary file and so on...
     (*bufPntr)[0x2C76] = char(newLevelNumber);
}

//levelNumber starts with index 0 for level 1!
irr::u8 Assets::GetLastSelectedRaceTrack() {
    return this->currLevelSelected;
}

irr::u8 Assets::GetMainPlayerSelectedCraft() {
   return this->currMainPlayerCraftSelected;
}

void Assets::DecodeCurrentGameLanguage() {
    //read new selected game language from
    //data array
    //selected language is stored at offset 0x2C7A in CONFIG.DAT
    //for LANGUAGE_ENGLISH 0x0
    //for LANGUAGE_GERMAN 0x1
    //for LANGUAGE_FRENCH 0x2
    //for LANGUAGE_SPANISH 0x3
    //for LANGUAGE_ITALIAN 0x4
    this->currSelectedGameLanguage = (irr::u8)(currentConfigFileDataByteArray[0x2C7A]);
}

void Assets::DecodeLastSelectedRaceTrack(char** bufPntr) {
    //last selected Race track is stored at offset
    //0x2C76 in CONFIG.DAT
    //value 0 means level 1, value 1 means level 2 in the binary file and so on...
    this->currLevelSelected = (irr::u8)((*bufPntr)[0x2C76]);
}

void Assets::DecodeLastSelectedCraft(char** bufPntr) {
    //last selected craft is stored at offset
    //0x94C in CONFIG.DAT
    //value 0 means KD1 Speeder (default selection at first start)
    //value 1 means Berserker
    //value 2 means Jugga
    //value 3 means Vampyr
    //value 4 means Outrider
    //value 5 means Flexiwing
    this->currMainPlayerCraftSelected = (irr::u8)((*bufPntr)[0x94C]);
}

void Assets::DecodeAudioVolumes() {
    //the current set music volume is stored at offset
    //0x97D in CONFIG.DAT
    //value 0 means volume off
    //1 - 199 all values inbetween monoton rising volume setting
    //value 200 means max volume
    this->currVolumeMusic = (irr::u8)(currentConfigFileDataByteArray[0x97D]);

    //the current set sound volume is stored at offset
    //0x981 in CONFIG.DAT
    //value 0 means volume off
    //1 - 199 all values inbetween monoton rising volume setting
    //value 200 means max volume
    this->currVolumeSound = (irr::u8)(currentConfigFileDataByteArray[0x981]);
}

irr::u8 Assets::ConvertVolumeProjectToHioctane(irr::f32 newVolumeProject) {
    //inside this project volume via SFML is handled
    //differently; as a float from 0 (off) up to 100 percent
    //of max volume
    //therefore convert below
    irr::f32 convVolume = (newVolumeProject / 100.0f) * 200.0f;
    irr::u8 truncVolume = (irr::u8)(convVolume);

    //plausi check
    if (truncVolume < 0)
        truncVolume = 0;

    if (truncVolume > 200)
        truncVolume = 200;

    return truncVolume;
}

irr::f32 Assets::ConvertVolumeHioctaneToProject(irr::u8 volumeHioctane) {
    //hioctane stores volume in config.dat file
    //as unsigned char with range 0 (off) up to 200 (max volume)
    //inside this project volume via SFML is handled
    //differently; as a float from 0 (off) up to 100 percent
    //of max volume
    //therefore convert below
    irr::f32 convVolume = (irr::f32)(volumeHioctane) / 2.0f;

    //plausi check
    if (convVolume < 0.0f)
        convVolume = 0.0f;

    if (convVolume > 100.0f)
        convVolume = 100.0f;

    return convVolume;
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetSoundVolume(irr::f32 newSoundVolume) {
    SetSoundVolume(newSoundVolume, &currentConfigFileDataByteArray);
}

void Assets::SetSoundVolume(irr::f32 newSoundVolume, char** bufPntr) {
    //inside this project volume via SFML is handled
    //differently; as a float from 0 (off) up to 100 percent
    //of max volume
    //therefore convert below
    irr::u8 truncVolume = ConvertVolumeProjectToHioctane(newSoundVolume);

    this->currVolumeSound = truncVolume;

    //the current set sound volume is stored at offset
    //0x981 in CONFIG.DAT
    //value 0 means volume off
    //1 - 199 all values inbetween monoton rising volume setting
    //value 200 means max volume
    (*bufPntr)[0x981] = char(truncVolume);
}

irr::f32 Assets::GetSoundVolume() {
    return ConvertVolumeHioctaneToProject(this->currVolumeSound);
}

irr::f32 Assets::GetMusicVolume() {
    return ConvertVolumeHioctaneToProject(this->currVolumeMusic);
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetMusicVolume(irr::f32 newMusicVolume) {
    SetMusicVolume(newMusicVolume, &currentConfigFileDataByteArray);
}

void Assets::SetMusicVolume(irr::f32 newMusicVolume, char** bufPntr) {
    //inside this project volume via SFML is handled
    //differently; as a float from 0 (off) up to 100 percent
    //of max volume
    //therefore convert below
    irr::u8 truncVolume = ConvertVolumeProjectToHioctane(newMusicVolume);

    this->currVolumeMusic = truncVolume;

    //the current set music volume is stored at offset
    //0x97D in CONFIG.DAT
    //value 0 means volume off
    //1 - 199 all values inbetween monoton rising volume setting
    //value 200 means max volume
    (*bufPntr)[0x97D] = char(truncVolume);
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetGameDifficulty(irr::u8 newDifficulty) {
    SetGameDifficulty(newDifficulty, &currentConfigFileDataByteArray);
}

void Assets::SetGameDifficulty(irr::u8 newDifficulty, char** bufPntr) {
    this->currGameDifficultyLevel = newDifficulty;

    //current game set difficulty level is stored at
    //0x977 in CONFIG.DAT
    //value 0 means easy
    //value 1 slightly higher difficulty
    //value 2 again slightly higher difficulty
    //value 3 means highest difficulty
    (*bufPntr)[0x977] = char(newDifficulty);
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetExtGameDeathMatchLives(irr::u8 newDeathMatchLives) {
    SetExtGameDeathMatchLives(newDeathMatchLives, &currentConfigFileDataByteArray);
}

void Assets::SetExtGameDeathMatchLives(irr::u8 newDeathMatchLives, char** bufPntr) {
    this->currDeathMatchLives = newDeathMatchLives;
    //current value for death match lives is stored at
    //0x34D in CONFIG.DAT
    //value 0
    //value 1
    //value 2
    //value 3
    //value 4 is the highest possible value
    (*bufPntr)[0x34D] = char(newDeathMatchLives);
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetExtGameHotSeatPlayers(irr::u8 newHotSetPlayers) {
    SetExtGameHotSeatPlayers(newHotSetPlayers, &currentConfigFileDataByteArray);
}

void Assets::SetExtGameHotSeatPlayers(irr::u8 newHotSetPlayers, char** bufPntr) {
    this->currHotSeatPlayers = newHotSetPlayers;

    //current value for hot seat players is stored at
    //0x34E in CONFIG.DAT
    //value from min 2 up to 8
    (*bufPntr)[0x34E] = char(newHotSetPlayers);
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetExtGameHotSeatRacingTime(irr::u8 newHotSeatRacingTime) {
    SetExtGameHotSeatRacingTime(newHotSeatRacingTime, &currentConfigFileDataByteArray);
}

void Assets::SetExtGameHotSeatRacingTime(irr::u8 newHotSeatRacingTime, char** bufPntr) {
    this->currHotSeatRacingTime = newHotSeatRacingTime;

    //current value for hot seat racing time is stored at
    //0x34F in CONFIG.DAT
    //value from min 1 up to 5
    (*bufPntr)[0x34F] = char(newHotSeatRacingTime);
}

//only useful for extended version of the
//original game
irr::u8 Assets::GetCurrentHotSeatPlayers() {
    return currHotSeatPlayers;
}

irr::u8 Assets::GetCurrentGameDifficulty() {
    return this->currGameDifficultyLevel;
}

irr::u8 Assets::GetCurrentTextureMappingQuality() {
    return this->mTextureMappingQuality;
}

bool Assets::GetSkyEnabled() {
    return this->mSkyEnabled;
}

bool Assets::GetShadingEnabled() {
    return this->mShadingEnabled;
}

void Assets::DecodeGameDifficultySetting(char** bufPntr) {
    //current game set difficulty level is stored at
    //0x977 in CONFIG.DAT
    //value 0 means easy
    //value 1 slightly higher difficulty
    //value 2 again slightly higher difficulty
    //value 3 means highest difficulty
    this->currGameDifficultyLevel = (irr::u8)((*bufPntr)[0x977]);
}

void Assets::DecodeGraphicSettings(char** bufPntr) {
    /* Offsets in CONFIG.DAT
    Offset 0x2996  Shading Off = 0, Shading On = 1 (Default On)
    Offset 0x299A  Sky Off = 0, Sky On = 1         (Default On)

    TextureMapping lowest setting:
      0x299E = 0,  0x29A2 = 0, 0x29A6 = 0

    TextureMapping 2nd lowest setting:
      0x299E = 0,  0x29A2 = 0, 0x29A6 = 1

    TextureMapping 3nd lowest setting:
      0x299E = 1,  0x29A2 = 1, 0x29A6 = 1

    TextureMapping 4th lowest setting:
      0x299E = 2,  0x29A2 = 2, 0x29A6 = 1

    TextureMapping 5th lowest setting:             (Default value)
      0x299E = 3,  0x29A2 = 3, 0x29A6 = 1

    TextureMapping highest setting:
      0x299E = 3,  0x29A2 = 4, 0x29A6 = 1 */

    irr::u8 currVal = (irr::u8)((*bufPntr)[0x2996]);
    if (currVal == 0) mShadingEnabled = false;
        else mShadingEnabled = true;

    currVal = (irr::u8)((*bufPntr)[0x299A]);
        if (currVal == 0) mSkyEnabled = false;
            else mSkyEnabled = true;


    currVal = (irr::u8)((*bufPntr)[0x299E]);
    irr::u8 currVal2 = (irr::u8)((*bufPntr)[0x29A2]);
    irr::u8 currVal3 = (irr::u8)((*bufPntr)[0x29A6]);

    if ((currVal == 0) && (currVal2 == 0) && (currVal3 == 0)) {
        mTextureMappingQuality = 0;
        return;
    }

    if ((currVal == 0) && (currVal2 == 0) && (currVal3 == 1)) {
        mTextureMappingQuality = 1;
        return;
    }

    if ((currVal == 1) && (currVal2 == 1) && (currVal3 == 1)) {
        mTextureMappingQuality = 2;
        return;
    }

    if ((currVal == 2) && (currVal2 == 2) && (currVal3 == 1)) {
        mTextureMappingQuality = 3;
        return;
    }

    if ((currVal == 2) && (currVal2 == 3) && (currVal3 == 1)) {
        mTextureMappingQuality = 4;
        return;
    }

    if ((currVal == 3) && (currVal2 == 4) && (currVal3 == 1)) {
        mTextureMappingQuality = 5;
        return;
    }

    //default value is second highest setting
     mTextureMappingQuality = 4;
}

void Assets::SetSkyEnabled(bool newSkyEnabled, char** bufPntr) {
    mSkyEnabled = newSkyEnabled;

    if (mSkyEnabled) {
        (*bufPntr)[0x299A] = 0x1;
    } else {
        (*bufPntr)[0x299A] = 0x0;
    }
}

void Assets::SetShadingEnabled(bool newShadingEnabled, char** bufPntr) {
    mShadingEnabled = newShadingEnabled;

    if (mShadingEnabled) {
        (*bufPntr)[0x2996] = 0x1;
    } else {
        (*bufPntr)[0x2996] = 0x0;
    }
}

void Assets::SetTextureMappingQuality(irr::u8 newTextureMappingQuality, char** bufPntr) {
    //only continue if newTextureMappingQuality value is valid
    //valid range is from 0 up to max 5
    if (newTextureMappingQuality > 5)
        return;

    mTextureMappingQuality = newTextureMappingQuality;

    switch (mTextureMappingQuality) {
        case 0: {
             (*bufPntr)[0x299E] = 0x0;
             (*bufPntr)[0x29A2] = 0x0;
             (*bufPntr)[0x29A6] = 0x0;
             break;
        }
        case 1: {
             (*bufPntr)[0x299E] = 0x0;
             (*bufPntr)[0x29A2] = 0x0;
             (*bufPntr)[0x29A6] = 0x1;
             break;
        }
        case 2: {
             (*bufPntr)[0x299E] = 0x1;
             (*bufPntr)[0x29A2] = 0x1;
             (*bufPntr)[0x29A6] = 0x1;
             break;
        }
        case 3: {
             (*bufPntr)[0x299E] = 0x2;
             (*bufPntr)[0x29A2] = 0x2;
             (*bufPntr)[0x29A6] = 0x1;
             break;
        }
        case 4: {
             (*bufPntr)[0x299E] = 0x2;
             (*bufPntr)[0x29A2] = 0x3;
             (*bufPntr)[0x29A6] = 0x1;
             break;
        }
        case 5: {
             (*bufPntr)[0x299E] = 0x3;
             (*bufPntr)[0x29A2] = 0x4;
             (*bufPntr)[0x29A6] = 0x1;
             break;
        }
        default: {
            break;
        }
     }
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetNewMainPlayerSelectedCraft(irr::u8 newSelectedCraftNr) {
    SetNewMainPlayerSelectedCraft(newSelectedCraftNr, &currentConfigFileDataByteArray);
}

void Assets::SetNewMainPlayerSelectedCraft(irr::u8 newSelectedCraftNr, char** bufPntr) {
    //last selected craft is stored at offset
    //0x94C in CONFIG.DAT
    //value 0 means KD1 Speeder (default selection at first start)
    //value 1 means Berserker
    //value 2 means Jugga
    //value 3 means Vampyr
    //value 4 means Outrider
    //value 5 means Flexiwing
    this->currMainPlayerCraftSelected = newSelectedCraftNr;

    (*bufPntr)[0x94C] = char(newSelectedCraftNr);
}

irr::u8 Assets::GetColorSchemeIndexNumberFromColorScheme(irr::u8 configFileValue) {
    irr::u8 result = 0;

    //first translate from config.dat file value to
    //number of color scheme we use in our project
    std::vector<irr::u8>::iterator it;

    for (it = this->mCraftColorSchemeConfigDatFileValue.begin(); it != this->mCraftColorSchemeConfigDatFileValue.end(); ++it) {
        if ((*it) == configFileValue) {
            return result;
        }

        result++;
    }

    //we did not find anything, return default
    //color scheme with value 0
    return 0;
}

irr::u8 Assets::GetCurrentCraftColorScheme() {
    //first translate from config.dat file value to
    //number of color scheme we use in our project
    irr::u8 result = GetColorSchemeIndexNumberFromColorScheme(this->currSelectedCraftColorScheme);

    return result;
}

//overloaded function to be called from outside, always works with config.dat
void Assets::SetCurrentCraftColorScheme(irr::u8 newCraftColorScheme) {
    SetCurrentCraftColorScheme(newCraftColorScheme, &currentConfigFileDataByteArray);
}

void Assets::SetCurrentCraftColorScheme(irr::u8 newCraftColorScheme, char** bufPntr) {
    irr::u8 result;

    //first we need to translate from color scheme numbering with use in our project
    //to the value used in the games config.dat file for this setting
    if (newCraftColorScheme >= this->mCraftColorSchemeConfigDatFileValue.size()) {
        //invalid value, just set default color scheme
        result = GAME_CRAFTCOLSCHEME_MADMEDICINE;
    } else {
        //valid value
        result = this->mCraftColorSchemeConfigDatFileValue.at(newCraftColorScheme);
    }

    this->currSelectedCraftColorScheme = result;

    //write currently selected craft color scheme into
    //data array
    //current selected craft color scheme is stored at offset 0x94B in CONFIG.DAT
    (*bufPntr)[0x94B] = char(result);
}

void Assets::DecodeCurrentCraftColorScheme(char** bufPntr) {
    //read currently selected craft color scheme
    //data array
    //current selected craft color scheme is stored at offset 0x94B in CONFIG.DAT
    this->currSelectedCraftColorScheme = (irr::u8)((*bufPntr)[0x94B]);
}

void Assets::AddCraft(char* nameCraft, char* meshFileName, irr::u8 statSpeed, irr::u8 statArmour,
                      irr::u8 statWeight, irr::u8 statFirePower) {
    CraftInfoStruct* newCraft= new CraftInfoStruct();
    newCraft->craftNr = currCraftNr;

    currCraftNr++;

    strcpy(newCraft->name, nameCraft);

    char fileName[40];

    irr::scene::IMesh* newMesh;
    newCraft->MeshCraft.clear();

    //load the mesh
    //lets loop to load all available ship color schemes
    for (int i = 0; i < 8; i++) {
        sprintf(fileName, "%s%d.obj", meshFileName, i);
        newMesh = mGame->mSmgr->getMesh(fileName);

        //add mesh to vector of available meshes (different color schemes)
        newCraft->MeshCraft.push_back(newMesh);
    }

    strcpy(newCraft->meshFileName, meshFileName);

    //set all stats
    newCraft->statSpeed = statSpeed;
    newCraft->statArmour = statArmour;
    newCraft->statWeight = statWeight;
    newCraft->statFirePower = statFirePower;

    //add to list of current available crafts
    mCraftVec->push_back(newCraft);
}

//Returns a new (default valued) race track struct
RaceTrackInfoStruct* Assets::CreateNewDefaultRaceTrackStats(irr::u8 levelNr, irr::u8 defaultNrLaps) {
   RaceTrackInfoStruct* newTrack = new RaceTrackInfoStruct();

   newTrack->levelNr = levelNr;

   //default init values, if there is no CONFIG.DAT
   //file yet
   strcpy(newTrack->bestPlayer, "BULLFROG");
   strcpy(newTrack->bestHighScorePlayer, "BULLFROG");
   newTrack->bestLapTime = 9999;
   newTrack->bestHighScore = 9999;

   //set the original default number of laps
   //defined by the original game
   newTrack->defaultNrLaps = defaultNrLaps;
   newTrack->currSelNrLaps = newTrack->defaultNrLaps;

   return newTrack;
}

void Assets::AddRaceTrack(char* nameTrack, char* meshFileName, irr::u8 defaultNrLaps) {
    RaceTrackInfoStruct* newTrack;

    //if possible, read current race track stats from original game configuration file
    if (this->mCurrentConfigFileRead) {
        newTrack = new RaceTrackInfoStruct();
        newTrack->levelNr = currLevelNr;
        newTrack->defaultNrLaps = defaultNrLaps;

        //take the current settings from config.dat config file
        DecodeCurrentRaceTrackStats(newTrack->levelNr, newTrack);
    } else {
        //default init values, if there is no CONFIG.DAT
        //file yet, create new track info with default game settings
        newTrack = CreateNewDefaultRaceTrackStats(currLevelNr, defaultNrLaps);
    }

    currLevelNr++;

    strcpy(newTrack->name, nameTrack);

    //load the mesh
    newTrack->MeshTrack = mGame->mSmgr->getMesh(meshFileName);

    strcpy(newTrack->meshFileName, meshFileName);

    //add to list of current available race tracks
    mRaceTrackVec->push_back(newTrack);
}

void Assets::InitCrafts() {
    //this is the order in which the game seems to handle the crafts
    //KD-1 SPEEDER is also the default selection when the game starts
    //without any config.dat file

    //Craft1
    AddCraft((char*)"KD-1 SPEEDER", (char*)("extract/models/car0-"), 6, 4, 5, 5);

    //Craft2
    AddCraft((char*)"BESERKER", (char*)("extract/models/tank0-"), 3, 5, 6, 6);

    //Craft3
    AddCraft((char*)"JUGGA", (char*)("extract/models/jugga0-"), 4, 5, 6, 5);

    //Craft4
    AddCraft((char*)"VAMPYR", (char*)("extract/models/jet0-"), 6, 4, 6, 4);

    //Craft5
    AddCraft((char*)"OUTRIDER", (char*)("extract/models/bike0-"), 8, 3, 4, 5);

    //Craft6
    AddCraft((char*)"FLEXIWING", (char*)("extract/models/skim0-"), 8, 4, 4, 4);

    //create vector with craft color scheme names
    char* schemeName;

    schemeName = new char [20];
    strcpy(schemeName, (char*)"MAD MEDICINE");
    mCraftColorSchemeNames.push_back(schemeName);
    mCraftColorSchemeConfigDatFileValue.push_back(GAME_CRAFTCOLSCHEME_MADMEDICINE);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"ASSASSINS");
    mCraftColorSchemeNames.push_back(schemeName);
    mCraftColorSchemeConfigDatFileValue.push_back(GAME_CRAFTCOLSCHEME_ASSASSINS);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"GOREHOUNDS");
    mCraftColorSchemeNames.push_back(schemeName);
    mCraftColorSchemeConfigDatFileValue.push_back(GAME_CRAFTCOLSCHEME_GOREHOUNDS);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"FOO FIGHTERS");
    mCraftColorSchemeNames.push_back(schemeName);
    mCraftColorSchemeConfigDatFileValue.push_back(GAME_CRAFTCOLSCHEME_FOOFIGHTERS);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"DETHFEST");
    mCraftColorSchemeNames.push_back(schemeName);
    mCraftColorSchemeConfigDatFileValue.push_back(GAME_CRAFTCOLSCHEME_DETHFEST);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"FIRE PHREAKS");
    mCraftColorSchemeNames.push_back(schemeName);
    mCraftColorSchemeConfigDatFileValue.push_back(GAME_CRAFTCOLSCHEME_FIREPHREAKS);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"STORM RIDERS");
    mCraftColorSchemeNames.push_back(schemeName);
    mCraftColorSchemeConfigDatFileValue.push_back(GAME_CRAFTCOLSCHEME_STORMRIDERS);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"BULLFROG");
    mCraftColorSchemeNames.push_back(schemeName);
    mCraftColorSchemeConfigDatFileValue.push_back(GAME_CRAFTCOLSCHEME_BULLFROG);
}

void Assets::InitRaceTracks() {
    //Track1
    AddRaceTrack((char*)("1. AMAZON DELTA TURNPIKE"), (char*)("extract/models/track0-0.obj"), GAME_DEFAULT_LAPS_TRACK1);

    //Track2
    AddRaceTrack((char*)("2. TRANS-ASIA INTERSTATE"), (char*)("extract/models/track0-1.obj"), GAME_DEFAULT_LAPS_TRACK2);

    //Track3
    AddRaceTrack((char*)("3. SHANGHAI DRAGON"), (char*)("extract/models/track0-2.obj"), GAME_DEFAULT_LAPS_TRACK3);

    //Track4
    AddRaceTrack((char*)("4. NEW CHERNOBYL CENTRAL"), (char*)("extract/models/track0-3.obj"), GAME_DEFAULT_LAPS_TRACK4);

    //Track5
    AddRaceTrack((char*)("5. SLAM CANYON"), (char*)("extract/models/track0-4.obj"), GAME_DEFAULT_LAPS_TRACK5);

    //Track6
    AddRaceTrack((char*)("6. THRAK CITY"), (char*)("extract/models/track0-5.obj"), GAME_DEFAULT_LAPS_TRACK6);

    //if we have the extended original game version available
    //also add the additional 3 race tracks
    if (mGame->mExtendedGame) {
        //Track7
        AddRaceTrack((char*)("7. ANCIENT MINE TOWN"), (char*)("extract/models/track0-6.obj"), GAME_DEFAULT_LAPS_TRACK7);

        //Track8
        AddRaceTrack((char*)("8. ARCTIC LAND"), (char*)("extract/models/track0-7.obj"), GAME_DEFAULT_LAPS_TRACK8);

        //Track9
        AddRaceTrack((char*)("9. DEATH MATCH ARENA"), (char*)("extract/models/track0-8.obj"), GAME_DEFAULT_LAPS_TRACK9);
    }
}

void Assets::AddPilot(char *pilotName, bool humanPlayer, char *defaultCraftName) {
    PilotInfoStruct* newPilot = new PilotInfoStruct();

    newPilot->pilotNr = currPilotNr;
    currPilotNr++;

    strcpy(newPilot->pilotName, pilotName);
    newPilot->humanPlayer = humanPlayer;
    strcpy(newPilot->defaultCraftName, defaultCraftName);

    //just take this color scheme as default
    currSelectedCraftColorScheme = GAME_CRAFTCOLSCHEME_MADMEDICINE;

    //add to my vector of pilots
    mPilotVec->push_back(newPilot);
}

void Assets::InitCpPilots() {
    //this players are all only used as computer players
    //do not change the order of this pilots below!
    AddPilot((char*)("BARNSY"), false, (char*)("BESERKER"));
   /* AddPilot((char*)("SHUNTLY"), false, (char*)("JUGGA"));
    AddPilot((char*)("COPSE"), false, (char*)("VAMPYR"));
    AddPilot((char*)("MANNY"), false, (char*)("OUTRIDER"));
    AddPilot((char*)("MCLALIN"), false, (char*)("FLEXIWING"));
    AddPilot((char*)("MAD"), false, (char*)("KD-1 SPEEDER"));
    AddPilot((char*)("ATROW"), false, (char*)("BESERKER"));*/
}

void Assets::SetComputerPlayersEnabled(bool enabled) {
    this->computerPlayersEnabled = enabled;
}

bool Assets::GetComputerPlayersEnabled() {
    return (this->computerPlayersEnabled);
}

//playerNr starting with index 0 for first player
irr::u16 Assets::ReadChampionShipOverallPointTableEntry(char** bufPntr, irr::u8 playerNr) {
    size_t offset = 0x2C7E + playerNr * 0x02;

    irr::u16 val = (irr::u16)((*bufPntr)[offset + 1]) << 8; //handle MSB
    val += (*bufPntr)[offset]; //add LSB

    return val;
}

//playerNr starting with index 0 for first player
void Assets::WriteChampionShipOverallPointTableEntry(char** bufPntr, irr::u8 playerNr, irr::u16 newPointsValue) {
    size_t offset = 0x2C7E + playerNr * 0x02;

    irr::u8 lsb = (irr::u8)(newPointsValue & 0x00FF);
    (*bufPntr)[offset] = lsb;

    irr::u16 hlper = (irr::u16)(newPointsValue & 0xFF00);
    hlper = hlper >> 8;

    irr::u8 msb = (irr::u8)(hlper);

    (*bufPntr)[offset + 1] = msb;
}

void Assets::WriteChampionShipOverallPointTable(char** bufPntr) {
    /*Current overall championship points are stored at the
    end of config.dat and the save files (if championship is
    saved),
    current points are an irr::u16 variable type (16 bits)

    Player 0 (Human) Championship Points  offset  0x2C7E LSB, 0x2C7F MSB
    BARNSY offset  0x2C80 LSB,  0x2C81 is the MSB
    SHUNTLY offset 0x2C82 LSB and so on
    COPSE offset 0x2C84 LSB
    MANNY offset 0x2C86 LSB
    MCLALIN offset  0x2C88 LSB
    MAD  offset  0x2C8A LSB
    ATROW offset  0x2C8C LSB*/

    //write points for all 8 players
    for (irr::u8 idx = 0; idx < 8; idx++) {
        WriteChampionShipOverallPointTableEntry(bufPntr, idx, mCurrChampionshipOverallPointVec.at(idx));
    }
}

void Assets::ResetChampionShipOverallPointTable() {
    mCurrChampionshipOverallPointVec.clear();

    for (irr::u8 idx = 0; idx < 8; idx++) {
        //set all points to 0
        mCurrChampionshipOverallPointVec.push_back(0);
    }
}

void Assets::DecodeChampionShipOverallPointTable(char** bufPntr) {
    /*Current overall championship points are stored at the
    end of config.dat and the save files (if championship is
    saved),
    current points are an irr::u16 variable type (16 bits)

    Player 0 (Human) Championship Points  offset  0x2C7E LSB, 0x2C7F MSB
    BARNSY offset  0x2C80 LSB,  0x2C81 is the MSB
    SHUNTLY offset 0x2C82 LSB and so on
    COPSE offset 0x2C84 LSB
    MANNY offset 0x2C86 LSB
    MCLALIN offset  0x2C88 LSB
    MAD  offset  0x2C8A LSB
    ATROW offset  0x2C8C LSB*/

    mCurrChampionshipOverallPointVec.clear();

    //read points for all 8 players
    for (irr::u8 idx = 0; idx < 8; idx++) {
        mCurrChampionshipOverallPointVec.push_back(ReadChampionShipOverallPointTableEntry(bufPntr, idx));
    }
}

std::vector<PointTableEntryStruct*>* Assets::GetLastRacePointsTable(std::vector<RaceStatsEntryStruct*>* lastRaceStats) {
    std::vector<PointTableEntryStruct*>* resultVec = new std::vector<PointTableEntryStruct*>();

    resultVec->clear();

    //for each player assign the points this player earned
    /* The points earned by each player in a race
       are very easily assigned by the original game
       It just depends on the order of the final positions
       at the race end;

       1st place player gets 20 points
       2nd place player gets 16 points
       3rd place player gets 13 points
       4th place player gets 11 points
       5th place player gets 9 points
       6th place player gets 7 points
       7th place player gets 6 points
       8th place player gets 5 points */

      std::vector<irr::u8> pointAssigementVec;
      pointAssigementVec.push_back(20);
      pointAssigementVec.push_back(16);
      pointAssigementVec.push_back(13);
      pointAssigementVec.push_back(11);
      pointAssigementVec.push_back(9);
      pointAssigementVec.push_back(7);
      pointAssigementVec.push_back(6);
      pointAssigementVec.push_back(5);

      std::vector<RaceStatsEntryStruct*>::iterator itEntry;
      PointTableEntryStruct* newPointTabEntry;

      for (itEntry = lastRaceStats->begin(); itEntry != lastRaceStats->end(); ++itEntry) {
           //create a new point table entry item
           newPointTabEntry = new PointTableEntryStruct();

           //create a new temporary buffer with playername
           //needs to be freed later! otherwise we have a memory leak
           newPointTabEntry->namePlayer = strdup((*itEntry)->playerName);

           //assign the earned points
           if (((*itEntry)->racePosition > 0) && ((*itEntry)->racePosition < 9)) {
                newPointTabEntry->pointVal = pointAssigementVec.at((*itEntry)->racePosition - 1);
           }

           resultVec->push_back(newPointTabEntry);
       }

      return resultVec;
}

void Assets::NewChampionship() {
    //if there was already a championship data array created before
    //free its memory! otherwise we have a memory leak
    if (currentChampionshipSaveGameDataByteArray != nullptr) {
        delete[] currentChampionshipSaveGameDataByteArray;
    }

    //create a new (default) config.dat file as a basis
    //for championship save game later
    CreateNewConfigFileContents(&currentChampionshipSaveGameDataByteArray, currentChampionshipSaveGameDataByteArrayLen);

    //let championship start at first level
    SetNewLastSelectedRaceTrack(0);

    //reset championship name
    strcpy(this->currChampionshipName, "");

    //reset the overall championship player point table to all
    //zero entries, but decoding the overall (new, empty) championship player point table
    //all current values need to be zero, so that we allow the player to initially
    //choose the player craft
    DecodeChampionShipOverallPointTable(&currentChampionshipSaveGameDataByteArray);
}

//Helper function to determine if player can currently select a craft
//during any point of the championship; In the original game the player
//craft can only be selected once, right at the start of a new
//championship
bool Assets::CanPlayerCurrentlySelectCraftDuringChampionship() {
    //allow selection, only if current level is first level
    //and if all point entries in championship player point table are
    //all zeroed out
    irr::u32 pointSum = 0;

    std::vector<irr::u16>::iterator it;

    for (it = mCurrChampionshipOverallPointVec.begin(); it != mCurrChampionshipOverallPointVec.end(); ++it) {
        pointSum += (*it);
    }

    if (pointSum != 0)
        return false;

    if (currLevelSelected != 0)
        return false;

    return true;
}

//This function does two things: Calculate new overall championship points for the currently active
//championship. Secondly the new result point table is returned in a dynamically allocated vector of PointTableEntryStruct
//structs
std::vector<PointTableEntryStruct*>* Assets::UpdateChampionshipOverallPointTable
                        (std::vector<PointTableEntryStruct*>* pointTableRace) {

    std::vector<PointTableEntryStruct*>::iterator it;

    irr::u8 idx = 0;

    //first create the sum and update the current overall point table
    for (it = pointTableRace->begin(); it != pointTableRace->end(); ++it) {
        this->mCurrChampionshipOverallPointVec.at(idx) += (*it)->pointVal;

        idx++;
    }

    std::vector<PointTableEntryStruct*>* resultVec = new std::vector<PointTableEntryStruct*>();

    resultVec->clear();

    std::vector<irr::u16>::iterator itSum;
    PointTableEntryStruct* newPointTabEntry;

    idx = 0;

    for (itSum = mCurrChampionshipOverallPointVec.begin(); itSum != mCurrChampionshipOverallPointVec.end(); ++itSum) {
         //create a new point table entry item
         newPointTabEntry = new PointTableEntryStruct();

         //create a new temporary buffer with playername
         //needs to be freed later! otherwise we have a memory leak
         newPointTabEntry->namePlayer = strdup(pointTableRace->at(idx)->namePlayer);

         //assign the earned points
         newPointTabEntry->pointVal = (*itSum);

         resultVec->push_back(newPointTabEntry);

         idx++;
     }

    return resultVec;
}

void Assets::CleanUpChampionshipSaveGameInfo() {
    std::vector<ChampionshipSaveGameInfoStruct*>::iterator it;
    ChampionshipSaveGameInfoStruct* pntr;

    for (it = mChampionshipSavegameInfoVec.begin(); it != mChampionshipSavegameInfoVec.end(); ) {
        pntr = (*it);

        it = mChampionshipSavegameInfoVec.erase(it);

        //free the string we created using strdup
        //function before
        free(pntr->championshipName);
    }
}

//whichSlotNr start with index 0, for first savegame slot
bool Assets::SearchChampionshipSaveGameSlot(irr::u8 whichSlotNr, char** championshipNameOutBuf) {
    //only load if slot number is valid
    if (whichSlotNr > 4)
        return false;

    strcpy(*championshipNameOutBuf, "EMPTY");

    //build save game file name for search
    char fname[20];

    //update file list for save directory
    //otherwise we do not find the latest added files
    if (!mGame->UpdateFileListSaveFolder())
        return false;

    //is this file existing?
    sprintf (fname, "save%0*u.dat", 2, whichSlotNr);

    //does this file exist? If not or there is
    //another problem simply exit
    irr::io::path foundFilePath =
            mGame->LocateFileInFileList(mGame->mOriginalGame->saveFolder, irr::core::string<fschar_t>(fname));

    if (foundFilePath.empty())
        return false;

    char* helperSaveGameDataByteArray = nullptr;
    size_t helperSaveGameDataByteArrayLen = 0;

    //file exists, read the savegame file
    //into a temporary helper buffer
    char* tempFileName = strdup(foundFilePath.c_str());

    if (!ReadGameConfigFile(tempFileName, &helperSaveGameDataByteArray, helperSaveGameDataByteArrayLen)) {
        free(tempFileName);
        return false;
    }

    free(tempFileName);

    DecodeCurrentChampionshipName(&helperSaveGameDataByteArray, championshipNameOutBuf);

    //cleanup the temporary buffer
    delete[] helperSaveGameDataByteArray;

    helperSaveGameDataByteArray = nullptr;

    //there was a savegame we successfully read
    return true;
}

void Assets::SearchChampionshipSaveGames() {
    //if there are already entries, delete them
    //and free the memory, before we search again
    if (mChampionshipSavegameInfoVec.size() > 0) {
       CleanUpChampionshipSaveGameInfo();
    }

    mChampionshipSavegameInfoVec.clear();

    //championship name helper buffer
    //length is limited to 12 characters in Hi-Octane!
    //plus 1 termination char + 1 char extra :)
    char helperChampionshipName[14];
    ChampionshipSaveGameInfoStruct* newStruct;

    for (irr::u8 idx = 0; idx < 5; idx++) {
         newStruct = new ChampionshipSaveGameInfoStruct();
         newStruct->saveGameAvail = false;

         char* pntr = &helperChampionshipName[0];

         if (SearchChampionshipSaveGameSlot(idx, &pntr)) {
            //we found a savegame in this slot
            newStruct->saveGameAvail = true;
         }

         newStruct->championshipName = strdup(helperChampionshipName);

         mChampionshipSavegameInfoVec.push_back(newStruct);
    }
}

void Assets::QuitCurrentChampionship() {
    if (currentChampionshipSaveGameDataByteArray == nullptr)
        return;

    //simply free dynamic allocated memory
    //for championship savegame data
    delete[] currentChampionshipSaveGameDataByteArray;

    currentChampionshipSaveGameDataByteArray = nullptr;
    currentChampionshipSaveGameDataByteArrayLen = 0;

    //set current selected level back to
    //first level
    SetNewLastSelectedRaceTrack(0);

    ResetChampionShipOverallPointTable();
}

//slot number starting with index 0
bool Assets::LoadChampionshipSaveGame(irr::u8 whichSlotNr) {
    //only load if slot number is valid
    if (whichSlotNr > 4)
        return false;

    //build save game file name to load
    char fname[20];

    //update file list for save directory
    if (!mGame->UpdateFileListSaveFolder())
        return false;

    //build file name
    sprintf (fname, "save%0*u.dat", 2, whichSlotNr);

    //does this file exist? If not or there is
    //another problem simply exit
    irr::io::path foundFilePath =
            mGame->LocateFileInFileList(mGame->mOriginalGame->saveFolder, irr::core::string<fschar_t>(fname));

    if (foundFilePath.empty())
        return false;

    //delete the data of a possible already loaded save game
    if (currentChampionshipSaveGameDataByteArray != nullptr) {
        delete[] currentChampionshipSaveGameDataByteArray;

        currentChampionshipSaveGameDataByteArray = nullptr;
    }

    //read the raw data of the championship save game file
    //into the special buffer for championship data
    char* tempFileName = strdup(foundFilePath.c_str());
    if (!ReadGameConfigFile(tempFileName, &currentChampionshipSaveGameDataByteArray, currentChampionshipSaveGameDataByteArrayLen)) {
        free(tempFileName);
        return false;
    }

    free(tempFileName);

    //decode raw data into my members
    //all of the settings below are now taken from the save game itself
    //the original game does seem to do the same
    DecodeMainPlayerName(&currentChampionshipSaveGameDataByteArray);

    //if we have the extended game read the player2 up to player8 names
    //back as well
    if (mGame->mExtendedGame) {
       DecodeExtGameAdditionalPlayerNames(&currentChampionshipSaveGameDataByteArray);
    }

    DecodeLastSelectedRaceTrack(&currentChampionshipSaveGameDataByteArray);
    DecodeLastSelectedCraft(&currentChampionshipSaveGameDataByteArray);
    DecodeGameDifficultySetting(&currentChampionshipSaveGameDataByteArray);
    DecodeCurrentCraftColorScheme(&currentChampionshipSaveGameDataByteArray);
    char* pntr = &this->currChampionshipName[0];
    DecodeCurrentChampionshipName(&currentChampionshipSaveGameDataByteArray, &pntr);

    //decode the overall championship player point table
    DecodeChampionShipOverallPointTable(&currentChampionshipSaveGameDataByteArray);

    //Next race track to continue championsship is stored at the same
    //offset location where the last selected racetrack is
    //saved in config.dat;

    //----------------------------------

    //Human Player (0) craft selection for the championship is also stored at the same offset location
    //for last selected craft as in config.dat, just in the save game files
    //---------------------------------

    //Human player (0) craft color scheme selection in Championship is exactly stored at the same offset
    //location as in config.dat file

    return true;
}

bool Assets::SaveChampionshipSaveGame(irr::u8 whichSlotNr) {
    //only save if slot number is valid
    if (whichSlotNr > 4)
        return false;

    //if there is no data to be save exit
    if (currentChampionshipSaveGameDataByteArray == nullptr)
        return false;

    //build save game file name for
    //writting the file
    char filename[100];
    char fname[20];

    strcpy(filename, mGame->mOriginalGame->saveFolder->getPath().c_str());
    sprintf (fname, "/save%0*u.dat", 2, whichSlotNr);

    strcat(filename, fname);

    //update all the data in the current savegame byte array

    //I guess it looks weird, but I need to copy string temporarily inside
    //another buffer, so that the strcpy function inside the routine SetNewMainPlayerName
    //does not copy from the same buffer location into the same buffer location again
    //I believe this would cause issues
    char* hlpBuf = strdup(this->currMainPlayerName);
    SetNewMainPlayerName(hlpBuf, &currentChampionshipSaveGameDataByteArray);
    free(hlpBuf);

    //if we have the extended game version, we need to also
    //write all other player names
    if (mGame->mExtendedGame) {
        hlpBuf = strdup(this->currPlayer2Name);
        SetNewExtGameAdditionalPlayerNames(2, hlpBuf, &currentChampionshipSaveGameDataByteArray);
        free(hlpBuf);

        hlpBuf = strdup(this->currPlayer3Name);
        SetNewExtGameAdditionalPlayerNames(3, hlpBuf, &currentChampionshipSaveGameDataByteArray);
        free(hlpBuf);

        hlpBuf = strdup(this->currPlayer4Name);
        SetNewExtGameAdditionalPlayerNames(4, hlpBuf, &currentChampionshipSaveGameDataByteArray);
        free(hlpBuf);

        hlpBuf = strdup(this->currPlayer5Name);
        SetNewExtGameAdditionalPlayerNames(5, hlpBuf, &currentChampionshipSaveGameDataByteArray);
        free(hlpBuf);

        hlpBuf = strdup(this->currPlayer6Name);
        SetNewExtGameAdditionalPlayerNames(6, hlpBuf, &currentChampionshipSaveGameDataByteArray);
        free(hlpBuf);

        hlpBuf = strdup(this->currPlayer7Name);
        SetNewExtGameAdditionalPlayerNames(7, hlpBuf, &currentChampionshipSaveGameDataByteArray);
        free(hlpBuf);

        hlpBuf = strdup(this->currPlayer8Name);
        SetNewExtGameAdditionalPlayerNames(8, hlpBuf, &currentChampionshipSaveGameDataByteArray);
        free(hlpBuf);
    }

    SetNewLastSelectedRaceTrack(this->currLevelSelected, &currentChampionshipSaveGameDataByteArray);
    SetNewMainPlayerSelectedCraft(this->currMainPlayerCraftSelected, &currentChampionshipSaveGameDataByteArray);
    SetCurrentCraftColorScheme(this->GetCurrentCraftColorScheme(), &currentChampionshipSaveGameDataByteArray);
    SetGameDifficulty(this->currGameDifficultyLevel, &currentChampionshipSaveGameDataByteArray);

    hlpBuf = strdup(this->currChampionshipName);
    SetCurrentChampionshipName(hlpBuf, &currentChampionshipSaveGameDataByteArray);
    free(hlpBuf);

    //update the championship overall player point table
    WriteChampionShipOverallPointTable(&currentChampionshipSaveGameDataByteArray);

    //write the data of the championship save game file
    //from the special buffer for championship data
    if (!WriteGameConfigFile(filename, &currentChampionshipSaveGameDataByteArray, currentChampionshipSaveGameDataByteArrayLen))
        return false;

    return true;
}

void Assets::CreateEmptyHighScoreTableEntry(char** targetBuf, irr::u8 entryNr) {
    size_t dataOffset;

    dataOffset = 0x0994 + entryNr * 39;

    memset(*targetBuf + dataOffset, 0, 39);

    (*targetBuf)[dataOffset + 4] = 0x0F; //set default player assessement

    char* dfltEntryName = strdup("BULLFROG");

    //player names in Hi-Octane are limited to max 8 characters
    WriteNullTerminatedString(*targetBuf, dataOffset + 5, dfltEntryName, (irr::u8)(strlen(dfltEntryName)));

    free(dfltEntryName);
}

void Assets::CreateEmptyHighScoreTable(char** targetBuf) {
    //there are 50 highscore table entries we
    //need to make
    for (int idx = 0; idx < 50; idx++) {
        CreateEmptyHighScoreTableEntry(targetBuf, idx);
    }
}

//nrRaceTrack = level number starting with 1
void Assets::WriteCurrentRaceTrackStats(irr::u32 nrRaceTrack, RaceTrackInfoStruct* inputInfoStruct, char** targetBuf) {
    //config file offsets found for the current
    //racetrack stats (each racetrack seems to have 74 bytes of stat
    //data available)
    //level 1 = 0x29AA = 10666 dec
    //level 2 = 0x29F4 = 10740 dec
    //level 3 = 0x2A3E = 10814 dec
    //level 4 = 0x2A88 = 10888 dec
    //level 5 = 0x2AD2 = 10962 dec
    //level 6 = 0x2B1C = 11036 dec
    //...

    //data layout for each race track stat in config.dat seems to be
    //[4 Bytes best lap value][9 bytes for player name best lap, 9th byte = string termination char][23 bytes of
    //unknown data][4 Bytes best race value][9 bytes for player name best race, 9th byte = string termination char]
    //[23 bytes of unknown data][1 Byte for current number laps set][1 Byte of unknown data]

    //calculate correct offset in file
    //for specified race track
    size_t dataOffset = 0x29AA + 74 * (nrRaceTrack - 1);

    //write best lap time
    ConvertInt32_ToByteArray(*targetBuf, dataOffset,  inputInfoStruct->bestLapTime);

    //player names in Hi-Octane are limited to max 8 characters
    WriteNullTerminatedString(*targetBuf, dataOffset + 4, inputInfoStruct->bestPlayer, 8);

    //write best highscore
    ConvertInt32_ToByteArray(*targetBuf, dataOffset + 36,  inputInfoStruct->bestHighScore);

    //player names in Hi-Octane are limited to max 8 characters
    WriteNullTerminatedString(*targetBuf, dataOffset + 40, inputInfoStruct->bestHighScorePlayer, 8);

    //write current number of set laps for this race track
    (*targetBuf)[dataOffset + 72] = (irr::u8)(inputInfoStruct->currSelNrLaps);
}

void Assets::WriteMeSomeBytesIDoNotKnowWhatTheMean(char** targetBuf, irr::u8 unknownValue, size_t writePosition) {
    for (int idx = 0; idx < 16; idx++) {
        (*targetBuf)[writePosition + idx] = unknownValue;
    }

    for (int idx = 0; idx < 8; idx++) {
        (*targetBuf)[writePosition + 16 + idx] = 0x0F;
    }
}

void Assets::WriteMe8UnknownBytes(char** targetBuf, irr::u8 unknownValue, size_t writePosition) {
    for (int idx = 0; idx < 8; idx++) {
        (*targetBuf)[writePosition + idx] = unknownValue;
    }
}

void Assets::CreateNewConfigFileContents(char** targetBuf, size_t &outNewSizeBytes) {
    //the default config.dat file has 11449 bytes
    //if we have the extended game version, we need to add
    //so much 0 bytes to it, until we have a config file
    //with a size of 137833 bytes at the end
    //I can only assume the additional space was reserved
    //for the additional extended original game game
    //settings (like you can change all player names), the
    //additional game modes and so on
    //Non extended version has a 11449 bytes sized config.dat
    //extended version has 137833 bytes
    //so we add 126384 bytes
    size_t size = CONFIG_DAT_SIZE_DEFAULT_GAME;

    if (mGame->mExtendedGame) {
        size = CONFIG_DAT_SIZE_EXTENDED_GAME;
    }

    outNewSizeBytes = size;

    *targetBuf = new char[size];

    //initially fill overall file with all zeros
    //size is in number of bytes to fill!
    memset(*targetBuf, 0, size);

    //now set the default settings
    highScoreTableVec = nullptr;

    //create an empty highscore table
    CreateEmptyHighScoreTable(targetBuf);

    SetCurrentGameLanguage(GAME_LANGUAGE_ENGLISH, targetBuf);

    //set default player name
    SetNewMainPlayerName((char*)"PLAYER", targetBuf);

    //if extended game version, set all player names from 2 up to 8
    //to empty strings
    if (mGame->mExtendedGame) {
        SetNewExtGameAdditionalPlayerNames(2, (char*)"", targetBuf);
        SetNewExtGameAdditionalPlayerNames(3, (char*)"", targetBuf);
        SetNewExtGameAdditionalPlayerNames(4, (char*)"", targetBuf);
        SetNewExtGameAdditionalPlayerNames(5, (char*)"", targetBuf);
        SetNewExtGameAdditionalPlayerNames(6, (char*)"", targetBuf);
        SetNewExtGameAdditionalPlayerNames(7, (char*)"", targetBuf);
        SetNewExtGameAdditionalPlayerNames(8, (char*)"", targetBuf);

        //also set Death match lives to default value of 0
        SetExtGameDeathMatchLives(0, targetBuf);

        //also set hot seat player value to default value of 2
        SetExtGameHotSeatPlayers(2, targetBuf);

        //set hot seat racing time default value of 1
        SetExtGameHotSeatRacingTime(1, targetBuf);
    }

    //there is currently no champhionship open
    SetCurrentChampionshipName((char*)"", targetBuf);

    //set default race track selection
    SetNewLastSelectedRaceTrack(0, targetBuf);     //is first level, as in the original game
    SetNewMainPlayerSelectedCraft(0, targetBuf);   //is KD1-SPEEDER, as in the original game
    SetCurrentCraftColorScheme(0, targetBuf);      //is MAD-MEDICINE, the default color scheme

    //if config.dat is missing, game always reverts to most
    //easy difficulty level
    SetGameDifficulty(0, targetBuf);

    //available range in config.dat game config file
    //is from 0 to 200 (max volume)
    //game sets it slightly below max volume when
    //config is not available
    SetMusicVolume(60, targetBuf);
    SetSoundVolume(60, targetBuf);

    //enable default shading and sky
    SetSkyEnabled(true, targetBuf);
    SetShadingEnabled(true, targetBuf);

    //default is second highest texture mapping
    //quality, range is from 0 (lowest) to 5 (highest)
    SetTextureMappingQuality(4, targetBuf);

    //create default race track stats
    RaceTrackInfoStruct* newTrack;

    newTrack = CreateNewDefaultRaceTrackStats(1, GAME_DEFAULT_LAPS_TRACK1);
    WriteCurrentRaceTrackStats(1, newTrack, targetBuf);
    delete newTrack;

    newTrack = CreateNewDefaultRaceTrackStats(2, GAME_DEFAULT_LAPS_TRACK2);
    WriteCurrentRaceTrackStats(2, newTrack, targetBuf);
    delete newTrack;

    newTrack = CreateNewDefaultRaceTrackStats(3, GAME_DEFAULT_LAPS_TRACK3);
    WriteCurrentRaceTrackStats(3, newTrack, targetBuf);
    delete newTrack;

    newTrack = CreateNewDefaultRaceTrackStats(4, GAME_DEFAULT_LAPS_TRACK4);
    WriteCurrentRaceTrackStats(4, newTrack, targetBuf);
    delete newTrack;

    newTrack = CreateNewDefaultRaceTrackStats(5, GAME_DEFAULT_LAPS_TRACK5);
    WriteCurrentRaceTrackStats(5, newTrack, targetBuf);
    delete newTrack;

    newTrack = CreateNewDefaultRaceTrackStats(6, GAME_DEFAULT_LAPS_TRACK6);
    WriteCurrentRaceTrackStats(6, newTrack, targetBuf);
    delete newTrack;

    //if we have the extended original game version
    //write also the information about the 3 additional
    //race tracks
    if (mGame->mExtendedGame) {
        newTrack = CreateNewDefaultRaceTrackStats(7, GAME_DEFAULT_LAPS_TRACK7);
        WriteCurrentRaceTrackStats(7, newTrack, targetBuf);
        delete newTrack;

        newTrack = CreateNewDefaultRaceTrackStats(8, GAME_DEFAULT_LAPS_TRACK8);
        WriteCurrentRaceTrackStats(8, newTrack, targetBuf);
        delete newTrack;

        newTrack = CreateNewDefaultRaceTrackStats(9, GAME_DEFAULT_LAPS_TRACK9);
        WriteCurrentRaceTrackStats(9, newTrack, targetBuf);
        delete newTrack;
    }

    //to be completely humble I do not know why we need to write all the bytes
    //below; I do not know currently what the mean for the game;
    //I just know we have to write a lot of additional BULLFROG strings
    //to be finished
    char* bullFrogStr = strdup("BULLFROG");
    irr::u8 maxLen = (irr::u8)(strlen(bullFrogStr));

    size_t startOff = 0x1152;

    for (int idx = 0; idx < 8; idx++) {
        WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x1262);

    startOff = 0x128A;

    //we need even more Bullfrog strings
    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMeSomeBytesIDoNotKnowWhatTheMean(targetBuf, 0x01, 0x138A);

    startOff = 0x13C2;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMeSomeBytesIDoNotKnowWhatTheMean(targetBuf, 0x02, 0x14C2);

    startOff = 0x14FA;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMeSomeBytesIDoNotKnowWhatTheMean(targetBuf, 0x03, 0x15FA);

    startOff = 0x1632;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMeSomeBytesIDoNotKnowWhatTheMean(targetBuf, 0x04, 0x1732);

    startOff = 0x176A;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMeSomeBytesIDoNotKnowWhatTheMean(targetBuf, 0x05, 0x186A);

    startOff = 0x18A2;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x06, 0x19A2);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x19B2);

    startOff = 0x19DA;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x07, 0x1ADA);
    WriteMe8UnknownBytes(targetBuf, 0x01, 0x1AE2);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x1AEA);

    startOff = 0x1B12;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x02, 0x1C1A);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x1C22);

    startOff = 0x1C4A;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x01, 0x1D4A);
    WriteMe8UnknownBytes(targetBuf, 0x03, 0x1D52);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x1D5A);

    startOff = 0x1D82;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x02, 0x1E82);
    WriteMe8UnknownBytes(targetBuf, 0x04, 0x1E8A);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x1E92);

    startOff = 0x1EBA;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x03, 0x1FBA);
    WriteMe8UnknownBytes(targetBuf, 0x05, 0x1FC2);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x1FCA);

    startOff = 0x1FF2;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x04, 0x20F2);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x2102);

    startOff = 0x212A;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x05, 0x222A);
    WriteMe8UnknownBytes(targetBuf, 0x01, 0x2232);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x223A);

    startOff = 0x2262;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x06, 0x2362);
    WriteMe8UnknownBytes(targetBuf, 0x02, 0x236A);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x2372);

    startOff = 0x239A;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x07, 0x249A);
    WriteMe8UnknownBytes(targetBuf, 0x03, 0x24A2);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x24AA);

    startOff = 0x24D2;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x04, 0x25DA);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x25E2);

    startOff = 0x260A;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x01, 0x270A);
    WriteMe8UnknownBytes(targetBuf, 0x05, 0x2712);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x271A);

    startOff = 0x2742;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x02, 0x2842);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x2852);

    startOff = 0x287A;

    for (int idx = 0; idx < 8; idx++) {
         WriteNullTerminatedString(*targetBuf, startOff + (idx * 0x20), bullFrogStr, maxLen);
    }

    WriteMe8UnknownBytes(targetBuf, 0x03, 0x297A);
    WriteMe8UnknownBytes(targetBuf, 0x01, 0x2982);
    WriteMe8UnknownBytes(targetBuf, 0x0F, 0x298A);

    //Final comment: One day I would really want to know for
    //what reason I had to write all of this random bytes...
    //but I fear I will never know :(

    free(bullFrogStr);

    //Joystick calibration values must be also somewhere
    //in the config.dat file, can not test this as I have
    //no Joystick available

    //write additional bytes where I do not know right now what
    //the mean, so that we get a correct config.dat file
    std::vector<std::pair<size_t, irr::u8>> byteVec;
    std::vector<std::pair<size_t, irr::u8>>::iterator it;

    byteVec.clear();

    //pair is offset adress of file config.dat, second
    //value is byte value to write
    byteVec.push_back(std::make_pair(0x32, 0x08));
    byteVec.push_back(std::make_pair(0x36, 0x01));
    byteVec.push_back(std::make_pair(0x4C, 0x01));
    byteVec.push_back(std::make_pair(0x4E, 0x08));
    byteVec.push_back(std::make_pair(0x50, 0x05));
    byteVec.push_back(std::make_pair(0x52, 0x50));

    //last byte in the level statistic state bytes must have value 0x0F
    //set correct value here, I do not know what this byte exactly does
    byteVec.push_back(std::make_pair(0x29F3, 0x0F));
    byteVec.push_back(std::make_pair(0x2A3D, 0x0F));
    byteVec.push_back(std::make_pair(0x2A87, 0x0F));
    byteVec.push_back(std::make_pair(0x2AD1, 0x0F));
    byteVec.push_back(std::make_pair(0x2B1B, 0x0F));
    byteVec.push_back(std::make_pair(0x2B65, 0x0F));

    if (mGame->mExtendedGame) {
       //add unknown value also for the additional 3 levels
       byteVec.push_back(std::make_pair(0x2BAF, 0x0F));
       byteVec.push_back(std::make_pair(0x2BF9, 0x0F));
       byteVec.push_back(std::make_pair(0x2C43, 0x0F));

       //30.03.2025: The extended game has 9 bytes with value 1 at the following locations,
       //but I was not able to find out what the do. Lets just write them right now, so that
       //we get a proper config.dat
       byteVec.push_back(std::make_pair(0x430, 0x01));
       byteVec.push_back(std::make_pair(0x431, 0x01));
       byteVec.push_back(std::make_pair(0x432, 0x01));
       byteVec.push_back(std::make_pair(0x433, 0x01));
       byteVec.push_back(std::make_pair(0x434, 0x01));
       byteVec.push_back(std::make_pair(0x435, 0x01));
       byteVec.push_back(std::make_pair(0x436, 0x01));
       byteVec.push_back(std::make_pair(0x437, 0x01));
       byteVec.push_back(std::make_pair(0x438, 0x01));
    }

    byteVec.push_back(std::make_pair(0x98C, 0x04));
    byteVec.push_back(std::make_pair(0x9E0, 0x01));
    byteVec.push_back(std::make_pair(0x9E1, 0x01));
    byteVec.push_back(std::make_pair(0xA07, 0x02));
    byteVec.push_back(std::make_pair(0xA08, 0x02));
    byteVec.push_back(std::make_pair(0xA2E, 0x03));
    byteVec.push_back(std::make_pair(0xA2F, 0x03));
    byteVec.push_back(std::make_pair(0xA55, 0x04));
    byteVec.push_back(std::make_pair(0xA56, 0x04));
    byteVec.push_back(std::make_pair(0xA7C, 0x05));
    byteVec.push_back(std::make_pair(0xA7D, 0x05));

    byteVec.push_back(std::make_pair(0xAA3, 0x06));
    byteVec.push_back(std::make_pair(0xACA, 0x07));
    byteVec.push_back(std::make_pair(0xACB, 0x01));
    byteVec.push_back(std::make_pair(0xAF2, 0x02));
    byteVec.push_back(std::make_pair(0xB18, 0x01));
    byteVec.push_back(std::make_pair(0xB19, 0x03));
    byteVec.push_back(std::make_pair(0xB3F, 0x02));
    byteVec.push_back(std::make_pair(0xB40, 0x04));
    byteVec.push_back(std::make_pair(0xB66, 0x03));
    byteVec.push_back(std::make_pair(0xB67, 0x05));
    byteVec.push_back(std::make_pair(0xB8D, 0x04));
    byteVec.push_back(std::make_pair(0xBB4, 0x05));
    byteVec.push_back(std::make_pair(0xBB5, 0x01));
    byteVec.push_back(std::make_pair(0xBDB, 0x06));
    byteVec.push_back(std::make_pair(0xBDC, 0x02));
    byteVec.push_back(std::make_pair(0xC02, 0x07));
    byteVec.push_back(std::make_pair(0xC03, 0x03));
    byteVec.push_back(std::make_pair(0xC2A, 0x04));
    byteVec.push_back(std::make_pair(0xC50, 0x01));
    byteVec.push_back(std::make_pair(0xC51, 0x05));
    byteVec.push_back(std::make_pair(0xC77, 0x02));
    byteVec.push_back(std::make_pair(0xC9E, 0x03));
    byteVec.push_back(std::make_pair(0xC9F, 0x01));
    byteVec.push_back(std::make_pair(0xCC5, 0x04));
    byteVec.push_back(std::make_pair(0xCC6, 0x02));
    byteVec.push_back(std::make_pair(0xCEC, 0x05));
    byteVec.push_back(std::make_pair(0xCED, 0x03));
    byteVec.push_back(std::make_pair(0xD13, 0x06));
    byteVec.push_back(std::make_pair(0xD14, 0x04));
    byteVec.push_back(std::make_pair(0xD3A, 0x07));
    byteVec.push_back(std::make_pair(0xD3B, 0x05));
    byteVec.push_back(std::make_pair(0xD88, 0x01));
    byteVec.push_back(std::make_pair(0xD89, 0x01));
    byteVec.push_back(std::make_pair(0xDAF, 0x02));
    byteVec.push_back(std::make_pair(0xDB0, 0x02));
    byteVec.push_back(std::make_pair(0xDD6, 0x03));
    byteVec.push_back(std::make_pair(0xDD7, 0x03));
    byteVec.push_back(std::make_pair(0xDFD, 0x04));
    byteVec.push_back(std::make_pair(0xDFE, 0x04));
    byteVec.push_back(std::make_pair(0xE24, 0x05));
    byteVec.push_back(std::make_pair(0xE25, 0x05));
    byteVec.push_back(std::make_pair(0xE4B, 0x06));
    byteVec.push_back(std::make_pair(0xE72, 0x07));
    byteVec.push_back(std::make_pair(0xE73, 0x01));
    byteVec.push_back(std::make_pair(0xE9A, 0x02));
    byteVec.push_back(std::make_pair(0xEC0, 0x01));
    byteVec.push_back(std::make_pair(0xEC1, 0x03));
    byteVec.push_back(std::make_pair(0xEE7, 0x02));
    byteVec.push_back(std::make_pair(0xEE8, 0x04));
    byteVec.push_back(std::make_pair(0xF0E, 0x03));
    byteVec.push_back(std::make_pair(0xF0F, 0x05));
    byteVec.push_back(std::make_pair(0xF35, 0x04));
    byteVec.push_back(std::make_pair(0xF5C, 0x05));
    byteVec.push_back(std::make_pair(0xF5D, 0x01));
    byteVec.push_back(std::make_pair(0xF83, 0x06));
    byteVec.push_back(std::make_pair(0xF84, 0x02));
    byteVec.push_back(std::make_pair(0xFAA, 0x07));
    byteVec.push_back(std::make_pair(0xFAB, 0x03));
    byteVec.push_back(std::make_pair(0xFD2, 0x04));
    byteVec.push_back(std::make_pair(0x101F, 0x02));
    byteVec.push_back(std::make_pair(0x1046, 0x03));
    byteVec.push_back(std::make_pair(0xFF8, 0x01));
    byteVec.push_back(std::make_pair(0xFF9, 0x05));
    byteVec.push_back(std::make_pair(0x1047, 0x01));
    byteVec.push_back(std::make_pair(0x106D, 0x04));
    byteVec.push_back(std::make_pair(0x106E, 0x02));
    byteVec.push_back(std::make_pair(0x1094, 0x05));
    byteVec.push_back(std::make_pair(0x1095, 0x03));
    byteVec.push_back(std::make_pair(0x10BB, 0x06));
    byteVec.push_back(std::make_pair(0x10BC, 0x04));
    byteVec.push_back(std::make_pair(0x10E2, 0x07));
    byteVec.push_back(std::make_pair(0x10E3, 0x05));
    byteVec.push_back(std::make_pair(0x1130, 0x01));
    byteVec.push_back(std::make_pair(0x1131, 0x01));

    for (it = byteVec.begin(); it != byteVec.end(); ++it) {
       (*targetBuf)[(*it).first] = (irr::u8)((*it).second);
    }
}
