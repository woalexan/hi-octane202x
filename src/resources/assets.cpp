/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "assets.h"

Assets::Assets(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::scene::ISceneManager* smgr,
               bool updateGameConfigFile) {
    this->myDevice = device;
    this->myDriver = driver;
    this->mySmgr = smgr;
    mUpdateGameConfigFile = updateGameConfigFile;

    mCurrentConfigFileRead = false;

    if (ReadGameConfigFile(&currentConfigFileDataByteArray, currentConfigFileDataByteArrayLen)) {
        mCurrentConfigFileRead = true;
    }

    //set main player name
    if (mCurrentConfigFileRead) {
        DecodeMainPlayerName();
        DecodeHighScoreTable();
        DecodeCurrentGameLanguage();
        DecodeLastSelectedRaceTrack();
        DecodeLastSelectedCraft();
        DecodeGameDifficultySetting();
        DecodeCurrentCraftColorScheme();
        DecodeCurrentChampionshipName();
        DecodeAudioVolumes();
    } else {
        //set default player name
        strcpy(currMainPlayerName, "PLAYER");
        strcpy(currChampionshipName, "");

        //set default race track selection
        currLevelSelected = 0;           //is first level, as in the original game
        currMainPlayerCraftSelected = 0; //is KD1-SPEEDER, as in the original game
        currSelectedCraftColorScheme = 0; //is MAD-MEDICINE, the default color scheme

        //if config.dat is missing, game always reverts to most
        //easy difficulty level
        currGameDifficultyLevel = 0;

        //available range in config.dat game config file
        //is from 0 to 200 (max volume)
        //game sets it slightly below max volume when
        //config is not available
        currVolumeMusic = 188;
        currVolumeSound = 188;
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
           this->mySmgr->getMeshCache()->removeMesh(pntrTrack->MeshTrack);

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
                this->mySmgr->getMeshCache()->removeMesh(pntrCraft->MeshCraft.at(j));
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

    char* compDataArrStr = NULL;
    size_t compDataLength;

    //reread current file contents in config.dat for
    //comparing with internal config data
    if (!ReadGameConfigFile(&compDataArrStr, compDataLength)) {
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
        WriteGameConfigFile();
    }

    delete[] compDataArrStr;
}

void Assets::ReadNullTerminatedString(char* bytes, size_t start_position, char* outString, irr::u8 maxStrLen) {
  if (!mCurrentConfigFileRead)
        return;

  irr::u8 currRemaining = maxStrLen;
  bool endFound = false;
  char currChar;
  size_t currPos = start_position;

  char helpstr[2];

  strcpy(outString, "");

  while (!endFound && currRemaining > 0) {
      currChar = bytes[currPos];

      if (currChar == 0) {
          endFound = true;
      } else {
          helpstr[0] = currChar;
          helpstr[1] = 0;

          strcat(outString, helpstr);

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
bool Assets::DecodeCurrentRaceTrackStats(irr::u32 nrRaceTrack, RaceTrackInfoStruct* targetInfoStruct) {
    if (!mCurrentConfigFileRead)
        return false;

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
    ReadNullTerminatedString(currentConfigFileDataByteArray, dataOffset + 4, targetInfoStruct->bestPlayer, 8);

    //read best highscore
    targetInfoStruct->bestHighScore = (irr::u32)(ConvertByteArray_ToInt32(currentConfigFileDataByteArray, dataOffset + 36));
    //player names in Hi-Octane are limited to max 8 characters
    ReadNullTerminatedString(currentConfigFileDataByteArray, dataOffset + 40, targetInfoStruct->bestHighScorePlayer, 8);

    //get current number of set laps for this race track
    targetInfoStruct->currSelNrLaps = (irr::u8)(currentConfigFileDataByteArray[dataOffset + 72]);

    return true;
}

//helper function which reads the current set main player (player1)
//name from CONFIG.DAT file
bool Assets::DecodeMainPlayerName() {
    if (!mCurrentConfigFileRead)
        return false;

    //current player 1 name is stored in CONFIG.DAT
    //at offset 0x094F
    //player names in Hi-Octane are limited to max 8 characters
    ReadNullTerminatedString(currentConfigFileDataByteArray, 0x094F, this->currMainPlayerName, 8);

    return true;
}

//helper function which reads the current championship
//name from CONFIG.DAT file
bool Assets::DecodeCurrentChampionshipName() {
    if (!mCurrentConfigFileRead)
        return false;

    //current championship name is stored in CONFIG.DAT
    //at offset 0x0
    //championship name in Hi-Octane is limited to max 12 characters
    ReadNullTerminatedString(currentConfigFileDataByteArray, 0x0, this->currChampionshipName, 12);

    return true;
}

int32_t Assets::ConvertByteArray_ToInt32(char* bytes, size_t start_position) {
    int32_t result;

    result = static_cast<int32_t>(  (bytes[start_position + 3] << 24) +
                                    (bytes[start_position + 2] << 16) +
                                    (bytes[start_position + 1] << 8) +
                                    (bytes[start_position]));
    return (result);
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
        //[23 bytes of unknown data][1 Byte for current number laps set][1 Byte of unknown data]

        //calculate correct offset in file
        //for specified race track
        size_t dataOffset = 0x29AA + 74 * (nrRaceTrack - 1);

        //set new current number of laps for this race track
        currentConfigFileDataByteArray[dataOffset + 72] = char(newNumberLaps);
    }
}

void Assets::AddDriverAssessementString(char* newString) {
    char* stringPntr;
    irr::u8 strLen = strlen(newString);

    //to be safe add one char extra :)
    stringPntr = new char[strLen + 2];
    strcpy(stringPntr, newString);

    driverAssessementStrings->push_back(stringPntr);
}

void Assets::InitDriverAssessementStrings() {
  driverAssessementStrings = new std::vector<char*>();
  driverAssessementStrings->clear();

  //Strings taken from Hi-Octane Exe file
  //With the last ones I am not sure anymore if the belong to
  //the driver assessement or not. I will need to find this out in
  //the future
  AddDriverAssessementString((char*)"IMMORTAL\0");
  AddDriverAssessementString((char*)"HARDWIRED\0");
  AddDriverAssessementString((char*)"OUTTA CONTROL\0");
  AddDriverAssessementString((char*)"DEMON\0");
  AddDriverAssessementString((char*)"DEADLY\0");
  AddDriverAssessementString((char*)"WRECKER\0");
  AddDriverAssessementString((char*)"DANGEROUS\0");
  AddDriverAssessementString((char*)"TAILGATER\0");
  AddDriverAssessementString((char*)"ANTISOCIAL\0");
  AddDriverAssessementString((char*)"DUST DEVIL\0");
  AddDriverAssessementString((char*)"UPWARDLY MOBILE\0");
  AddDriverAssessementString((char*)"MEDIOCRE\0");
  AddDriverAssessementString((char*)"TOO SLOW\0");
  AddDriverAssessementString((char*)"ENDANGERED SPECIES\0");
  AddDriverAssessementString((char*)"TARGET PRACTICE\0");
  AddDriverAssessementString((char*)"BACK MARKER\0");
  AddDriverAssessementString((char*)"CANYON KISSER\0");
  AddDriverAssessementString((char*)"VICTIM\0");
  AddDriverAssessementString((char*)"SCRAP\0");
  AddDriverAssessementString((char*)"SMEAR\0");
  //I believe the following string does not actually belong
  //to the driver assessement strings in Hi-Octance, because
  //the race stats page lists 20 different ratings, and not 21
  //AddDriverAssessementString((char*)"CHEATING SUCKS\0");
}

irr::u8 Assets::GetNumberDriverAssessementStrings() {
    return this->driverAssessementStrings->size();
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

      //delete string itself as well
      delete[] strPntr;
  }
}

//this function reads an existing original game
//config file
//returns true if a config file was read
//false otherwise
bool Assets::ReadGameConfigFile(char** targetBuf, size_t &outBufSize) {
  //is there already an existing subfolder "HIOCTANE.CD"?
  char dirName[40];
  char fileName[60];

  strcpy(dirName, "originalgame/HIOCTANE.CD/SAVE");
  strcpy(fileName, "originalgame/HIOCTANE.CD/SAVE/CONFIG.DAT");

  if (IsDirectoryPresent(dirName) != 1) {
      //originalgame/HIOCTANE.CD/SAVE directory not present
      return false;
  }

  //now check for existing file CONFIG.DAT
  if (FileExists(fileName) != 1) {
      //config.dat file does not exist
      return false;
  }

  //original game config file is there
  //read it
  FILE* iFile;

  iFile = fopen(fileName, "rb");
  fseek(iFile, 0L, SEEK_END);
  size_t size = ftell(iFile);
  fseek(iFile, 0L, SEEK_SET);

  size_t counter = 0;

  *targetBuf = new char[size];

  //remember amount of data for the next write
  outBufSize = size;

  if (iFile != NULL)
  {
      do {
          (*targetBuf)[counter] = fgetc(iFile);
          counter++;
      } while (counter < size);
      fclose(iFile);
  } else {
      delete[] *targetBuf;
      *targetBuf = NULL;

      return false;
  }

  return true;
}

void Assets::SetNewMainPlayerName(char* newName) {
    if (strcmp(newName, this->currMainPlayerName) != 0) {
        strcpy(this->currMainPlayerName, newName);

        //write new name also in the config file
        //data array
        //main player name is stored at offset 0x094F in CONFIG.DAT
        //name is limited to 8 characters only!
        WriteNullTerminatedString(currentConfigFileDataByteArray, 0x094F, currMainPlayerName, 8);

        //make sure at byte 9 after this offset with store a 0 character (string termination)
        currentConfigFileDataByteArray[0x094F + 9] = 0;
    }
}

char* Assets::GetNewMainPlayerName() {
    return this->currMainPlayerName;
}

std::string Assets::GetCraftModelName(char* craftName, irr::u8 selectedCraftColorScheme) {
    CraftInfoStruct* craft = NULL;
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

    if (craft != NULL) {
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

std::vector<PilotInfoStruct*> Assets::GetPilotInfoNextRace(bool addHumanPlayer, bool addComputerPlayers) {
    std::vector<PilotInfoStruct*> pilotInfo;
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
    }

    if (addComputerPlayers) {
        std::vector<PilotInfoStruct*>::iterator itPilot;

        for (itPilot = mPilotVec->begin(); itPilot != mPilotVec->end(); ++itPilot) {
            PilotInfoStruct *newPlayer = new PilotInfoStruct();
            newPlayer->pilotNr = currPilotNumber;
            currPilotNumber++;

            strcpy(newPlayer->pilotName, ((*itPilot)->pilotName));
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

void Assets::SetCurrentChampionshipName(char* newName) {
    if (strcmp(newName, this->currChampionshipName) != 0) {
        strcpy(this->currChampionshipName, newName);

        //write new name also in the config file
        //data array
        //current championship name is stored at offset 0x0 in CONFIG.DAT
        //name is limited to 12 characters only!
        WriteNullTerminatedString(currentConfigFileDataByteArray, 0x0, currChampionshipName, 12);

        //make sure at byte 13 after this offset with store a 0 character (string termination)
        currentConfigFileDataByteArray[0x0 + 13] = 0;
    }
}

char* Assets::GetCurrentChampionshipName() {
    return this->currChampionshipName;
}

//returns NULL in case of an unexpected error
std::vector<HighScoreEntryStruct*>* Assets::GetHighScoreTable() {
    //if we did not read config file before
    //we must return with problem
    if (!this->mCurrentConfigFileRead) {
       return NULL;
    }

    return this->highScoreTableVec;
}

//this function writes to an existing original game
//config file that was read before
//returns true in case of success
//false otherwiese
bool Assets::WriteGameConfigFile() {
  //if we did not read config file before
  //we must not write it!
  if (!this->mCurrentConfigFileRead) {
     return false;
  }

  char fileName[60];

  strcpy(fileName, "originalgame/HIOCTANE.CD/SAVE/CONFIG.DAT");

  //original game config file is there
  //read it
  FILE* oFile;

  oFile = fopen(fileName, "wb");

  size_t counter = 0;

  if (oFile != NULL)
  {
      do {
          fputc(currentConfigFileDataByteArray[counter], oFile);
          counter++;
      } while (counter < currentConfigFileDataByteArrayLen);
      fclose(oFile);

      return true;
  }

  return false;
}

bool Assets::DecodeHighScoreTable() {
  if (!mCurrentConfigFileRead) {
      highScoreTableVec = NULL;
      return false;
  }

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
      ReadNullTerminatedString(currentConfigFileDataByteArray, dataOffset + 5, newEntry->namePlayer, 8);

      this->highScoreTableVec->push_back(newEntry);
  }

  return true;
}

void Assets::SetCurrentGameLanguage(irr::u8 newLanguage) {
    if (newLanguage != this->currSelectedGameLanguage) {
        this->currSelectedGameLanguage = newLanguage;

        //write new selected game language also into
        //data array
        //selected language is stored at offset 0x2C7A in CONFIG.DAT
        //for LANGUAGE_ENGLISH 0x0
        //for LANGUAGE_GERMAN 0x1
        //for LANGUAGE_FRENCH 0x2
        //for LANGUAGE_SPANISH 0x3
        //for LANGUAGE_ITALIAN 0x4
        currentConfigFileDataByteArray[0x2C7A] = char(newLanguage);
    }
}

irr::u8 Assets::GetCurrentGameLanguage() {
    return this->currSelectedGameLanguage;
}

//levelNumber starts with index 0 for level 1!
void Assets::SetNewLastSelectedRaceTrack(int newLevelNumber) {
    if (newLevelNumber != this->currLevelSelected) {
        this->currLevelSelected = newLevelNumber;

        //last selected Race track is stored at offset
        //0x2C76 in CONFIG.DAT
        //value 0 means level 1, value 1 means level 2 in the binary file and so on...
        currentConfigFileDataByteArray[0x2C76] = char(newLevelNumber);
    }
}

//levelNumber starts with index 0 for level 1!
irr::u8 Assets::GetLastSelectedRaceTrack() {
    return this->currLevelSelected;
}

irr::u8 Assets::GetMainPlayerSelectedCraft() {
   return this->currMainPlayerCraftSelected;
}

bool Assets::DecodeCurrentGameLanguage() {
    if (!mCurrentConfigFileRead) {
        return false;
    }

    //read new selected game language from
    //data array
    //selected language is stored at offset 0x2C7A in CONFIG.DAT
    //for LANGUAGE_ENGLISH 0x0
    //for LANGUAGE_GERMAN 0x1
    //for LANGUAGE_FRENCH 0x2
    //for LANGUAGE_SPANISH 0x3
    //for LANGUAGE_ITALIAN 0x4
    this->currSelectedGameLanguage = (irr::u8)(currentConfigFileDataByteArray[0x2C7A]);

    return true;
}

bool Assets::DecodeLastSelectedRaceTrack() {
    if (!mCurrentConfigFileRead) {
        return false;
    }

    //last selected Race track is stored at offset
    //0x2C76 in CONFIG.DAT
    //value 0 means level 1, value 1 means level 2 in the binary file and so on...
    this->currLevelSelected = (irr::u8)(currentConfigFileDataByteArray[0x2C76]);

    return true;
}

bool Assets::DecodeLastSelectedCraft() {
    if (!mCurrentConfigFileRead) {
        return false;
    }

    //last selected craft is stored at offset
    //0x94C in CONFIG.DAT
    //value 0 means KD1 Speeder (default selection at first start)
    //value 1 means Berserker
    //value 2 means Jugga
    //value 3 means Vampyr
    //value 4 means Outrider
    //value 5 means Flexiwing
    this->currMainPlayerCraftSelected = (irr::u8)(currentConfigFileDataByteArray[0x94C]);

    return true;
}

bool Assets::DecodeAudioVolumes() {
    if (!mCurrentConfigFileRead) {
        return false;
    }

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

    return true;
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

void Assets::SetSoundVolume(irr::f32 newSoundVolume) {
    //inside this project volume via SFML is handled
    //differently; as a float from 0 (off) up to 100 percent
    //of max volume
    //therefore convert below
    irr::u8 truncVolume = ConvertVolumeProjectToHioctane(newSoundVolume);

    if (truncVolume != this->currVolumeSound) {
        this->currVolumeSound = truncVolume;

        //the current set sound volume is stored at offset
        //0x981 in CONFIG.DAT
        //value 0 means volume off
        //1 - 199 all values inbetween monoton rising volume setting
        //value 200 means max volume
        currentConfigFileDataByteArray[0x981] = char(truncVolume);
    }
}

irr::f32 Assets::GetSoundVolume() {
    return ConvertVolumeHioctaneToProject(this->currVolumeSound);
}

irr::f32 Assets::GetMusicVolume() {
    return ConvertVolumeHioctaneToProject(this->currVolumeMusic);
}

void Assets::SetMusicVolume(irr::f32 newMusicVolume) {
    //inside this project volume via SFML is handled
    //differently; as a float from 0 (off) up to 100 percent
    //of max volume
    //therefore convert below
    irr::u8 truncVolume = ConvertVolumeProjectToHioctane(newMusicVolume);

    if (truncVolume != this->currVolumeMusic) {
        this->currVolumeMusic = truncVolume;

        //the current set music volume is stored at offset
        //0x97D in CONFIG.DAT
        //value 0 means volume off
        //1 - 199 all values inbetween monoton rising volume setting
        //value 200 means max volume
        currentConfigFileDataByteArray[0x97D] = char(truncVolume);
    }
}

void Assets::SetGameDifficulty(irr::u8 newDifficulty) {
    if (newDifficulty != this->currGameDifficultyLevel) {
        this->currGameDifficultyLevel = newDifficulty;

        //current game set difficulty level is stored at
        //0x977 in CONFIG.DAT
        //value 0 means easy
        //value 1 slightly higher difficulty
        //value 2 again slightly higher difficulty
        //value 3 means highest difficulty
        currentConfigFileDataByteArray[0x977] = char(newDifficulty);
    }
}

irr::u8 Assets::GetCurrentGameDifficulty() {
    return this->currGameDifficultyLevel;
}

bool Assets::DecodeGameDifficultySetting() {
    if (!mCurrentConfigFileRead) {
        return false;
    }

    //current game set difficulty level is stored at
    //0x977 in CONFIG.DAT
    //value 0 means easy
    //value 1 slightly higher difficulty
    //value 2 again slightly higher difficulty
    //value 3 means highest difficulty
    this->currGameDifficultyLevel = (irr::u8)(currentConfigFileDataByteArray[0x977]);

    return true;
}

void Assets::SetNewMainPlayerSelectedCraft(irr::u8 newSelectedCraftNr) {
    if (newSelectedCraftNr != this->currMainPlayerCraftSelected) {
        this->currMainPlayerCraftSelected = newSelectedCraftNr;

        //last selected craft is stored at offset
        //0x94C in CONFIG.DAT
        //value 0 means KD1 Speeder (default selection at first start)
        //value 1 means Berserker
        //value 2 means Jugga
        //value 3 means Vampyr
        //value 4 means Outrider
        //value 5 means Flexiwing
        currentConfigFileDataByteArray[0x94C] = char(newSelectedCraftNr);
    }
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

void Assets::SetCurrentCraftColorScheme(irr::u8 newCraftColorScheme) {
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

    if (result != this->currSelectedCraftColorScheme) {
        this->currSelectedCraftColorScheme = result;

        //write currently selected craft color scheme into
        //data array
        //current selected craft color scheme is stored at offset 0x26 in CONFIG.DAT
        //this are the actual used values for each
        //color scheme in this file
        //MADMEDICINE 0x06
        //ASSASSINS 0xAA
        //GOREHOUNDS 0x4A
        //FOOFIGHTERS 0x10
        //DETHFEST 0x8C
        //FIREPHREAKS 0x16
        //STORMRIDERS 0x17
        //BULLFROG 0x8F
        currentConfigFileDataByteArray[0x26] = char(result);
    }
}

bool Assets::DecodeCurrentCraftColorScheme() {
    if (!mCurrentConfigFileRead) {
        return false;
    }

    //read currently selected craft color scheme
    //data array
    //current selected craft color scheme is stored at offset 0x26 in CONFIG.DAT
    //this are the actual used values for each
    //color scheme in this file
    //MADMEDICINE 0x06
    //ASSASSINS 0xAA
    //GOREHOUNDS 0x4A
    //FOOFIGHTERS 0x10
    //DETHFEST 0x8C
    //FIREPHREAKS 0x16
    //STORMRIDERS 0x17
    //BULLFROG 0x8F

    this->currSelectedCraftColorScheme = (irr::u8)(currentConfigFileDataByteArray[0x26]);

    return true;
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
        newMesh = this->mySmgr->getMesh(fileName);

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

void Assets::AddRaceTrack(char* nameTrack, char* meshFileName, irr::u8 defaultNrLaps) {
    RaceTrackInfoStruct* newTrack = new RaceTrackInfoStruct();
    newTrack->levelNr = currLevelNr;

    //if possible, read current race track stats from original game configuration file
    if (this->mCurrentConfigFileRead) {
        DecodeCurrentRaceTrackStats(newTrack->levelNr, newTrack);
    } else {
        //default init values, if there is no CONFIG.DAT
        //file yet
        strcpy(newTrack->bestPlayer, "BULLFROG");
        strcpy(newTrack->bestHighScorePlayer, "BULLFROG");
        newTrack->bestLapTime = 9999;
        newTrack->bestHighScore = 9999;

        newTrack->currSelNrLaps = newTrack->defaultNrLaps;
    }

    currLevelNr++;

    strcpy(newTrack->name, nameTrack);

    //load the mesh
    newTrack->MeshTrack = this->mySmgr->getMesh(meshFileName);

    strcpy(newTrack->meshFileName, meshFileName);
    newTrack->defaultNrLaps = defaultNrLaps;

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
    AddRaceTrack((char*)("1. AMAZON DELTA TURNPIKE"), (char*)("extract/models/track0-0.obj"), 11);

    //Track2
    AddRaceTrack((char*)("2. TRANS-ASIA INTERSTATE"), (char*)("extract/models/track0-1.obj"), 8);

    //Track3
    AddRaceTrack((char*)("3. SHANGHAI DRAGON"), (char*)("extract/models/track0-2.obj"), 9);

    //Track4
    AddRaceTrack((char*)("4. NEW CHERNOBYL CENTRAL"), (char*)("extract/models/track0-3.obj"), 8);

    //Track5
    AddRaceTrack((char*)("5. SLAM CANYON"), (char*)("extract/models/track0-4.obj"), 9);

    //Track6
    AddRaceTrack((char*)("6. THRAK CITY"), (char*)("extract/models/track0-5.obj"), 5);

    //Track7, TODO: fix race track 3D model if I get it one day, also fix to correct number of default laps (I do not know)
    //AddRaceTrack((char*)("7. ANCIENT MINE TOWN"), (char*)("extract/models/cone0-0.obj"), 5);

    //Track8, TODO: fix race track 3D model if I get it one day, also fix to correct number of default laps (I do not know)
    //AddRaceTrack((char*)("8. ARCTIC LAND"), (char*)("extract/models/cone0-0.obj"), 5);

    //Track9, TODO: fix race track 3D model if I get it one day, also fix to correct number of default laps (I do not know)
    //AddRaceTrack((char*)("9. DEATH MATCH ARENA"), (char*)("extract/models/cone0-0  irr::u8 currSelectedCraftColorScheme; = GAME_CRAFTCOLSCHEME_MADMEDICINE;.obj"), 5);
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
    AddPilot((char*)("MAD"), false, (char*)("KD-1 SPEEDER"));
    AddPilot((char*)("ATROW"), false, (char*)("BESERKER"));
    AddPilot((char*)("BARNSY"), false, (char*)("BESERKER"));
    AddPilot((char*)("SHUNTLY"), false, (char*)("JUGGA"));
    AddPilot((char*)("COPSE"), false, (char*)("VAMPYR"));
    AddPilot((char*)("MANNY"), false, (char*)("OUTRIDER"));
    AddPilot((char*)("MCLALIN"), false, (char*)("FLEXIWING"));
}

void Assets::SetComputerPlayersEnabled(bool enabled) {
    this->computerPlayersEnabled = enabled;
}

bool Assets::GetComputerPlayersEnabled() {
    return (this->computerPlayersEnabled);
}

