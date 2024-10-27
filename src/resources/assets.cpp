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

    if (ReadGameConfigFile()) {
        mCurrentConfigFileRead = true;
    }

    //set main player name
    if (mCurrentConfigFileRead) {
        DecodeMainPlayerName();
        DecodeHighScoreTable();
    } else {
        //set default player name
        strcpy(currMainPlayerName, "PLAYER");
    }

    InitDriverAssessementStrings();

    mRaceTrackVec = new std::vector<RaceTrackInfoStruct*>();
    mRaceTrackVec->clear();

    mCraftVec = new std::vector<CraftInfoStruct*>();
    mCraftVec->clear();

    //init all race tracks
    InitRaceTracks();

    //init all crafts
    InitCrafts();
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
void Assets::DecodeCurrentRaceTrackStats(irr::u32 nrRaceTrack, RaceTrackInfoStruct* targetInfoStruct) {
    if (!mCurrentConfigFileRead)
        return;

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
}

//helper function which reads the current set main player (player1)
//name from CONFIG.DAT file
void Assets::DecodeMainPlayerName() {
    if (!mCurrentConfigFileRead)
        return;

    //current player 1 name is stored in CONFIG.DAT
    //at offset 0x094F
    //player names in Hi-Octane are limited to max 8 characters
    ReadNullTerminatedString(currentConfigFileDataByteArray, 0x094F, this->currMainPlayerName, 8);
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

        if (mUpdateGameConfigFile) {
            //write modified default lap number into CONFIG.DAT file
            WriteGameConfigFile();
        }
    }
}

void Assets::AddDriverAssessementString(char* newString) {
    char* stringPntr;
    irr::u8 strLen = sizeof(newString);

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
  AddDriverAssessementString((char*)"IMMORTAL");
  AddDriverAssessementString((char*)"HARDWIRED");
  AddDriverAssessementString((char*)"OUTTA CONTROL");
  AddDriverAssessementString((char*)"DEMON");
  AddDriverAssessementString((char*)"DEADLY");
  AddDriverAssessementString((char*)"WRECKER");
  AddDriverAssessementString((char*)"DANGEROUS");
  AddDriverAssessementString((char*)"TAILGATER");
  AddDriverAssessementString((char*)"ANTISOCIAL");
  AddDriverAssessementString((char*)"DUST DEVIL");
  AddDriverAssessementString((char*)"UPWARDLY MOBILE");
  AddDriverAssessementString((char*)"MEDIOCRE");
  AddDriverAssessementString((char*)"TOO SLOW");
  AddDriverAssessementString((char*)"ENDANGERED SPECIES");
  AddDriverAssessementString((char*)"TARGET PRACTICE");
  AddDriverAssessementString((char*)"BACK MARKER");
  AddDriverAssessementString((char*)"CANYON KISSER");
  AddDriverAssessementString((char*)"VICTIM");
  AddDriverAssessementString((char*)"SCRAP");
  AddDriverAssessementString((char*)"SMEAR");
  AddDriverAssessementString((char*)"CHEATING SUCKS");
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
bool Assets::ReadGameConfigFile() {
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

  currentConfigFileDataByteArray = new char[size];

  //remember amount of data for the next write
  currentConfigFileDataByteArrayLen = size;

  if (iFile != NULL)
  {
      do {
          currentConfigFileDataByteArray[counter] = fgetc(iFile);
          counter++;
      } while (counter < size);
      fclose(iFile);
  } else {
      delete[] currentConfigFileDataByteArray;
      currentConfigFileDataByteArray = NULL;

      return false;
  }

  return true;
}

void Assets::SetNewMainPlayerName(char* newName) {
    strcpy(this->currMainPlayerName, newName);

    //write new name also in the config file
    //data array
    //main player name is stored at offset 0x094F in CONFIG.DAT
    //name is limited to 8 characters only!
    WriteNullTerminatedString(currentConfigFileDataByteArray, 0x094F, currMainPlayerName, 8);

    //make sure at byte 9 after this offset with store a 0 character (string termination)
    currentConfigFileDataByteArray[0x094F + 9] = 0;

    if (mUpdateGameConfigFile) {
        //write modified name into CONFIG.DAT file
        WriteGameConfigFile();
    }
}

char* Assets::GetNewMainPlayerName() {
    return this->currMainPlayerName;
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
  if (!ReadGameConfigFile()) {
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

void Assets::AddCraft(char* nameCraft, char* meshFileName, irr::u8 statSpeed, irr::u8 statArmour, irr::u8 statWeight, irr::u8 statFirePower) {
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

    schemeName = new char [20];
    strcpy(schemeName, (char*)"ASSASSINS");
    mCraftColorSchemeNames.push_back(schemeName);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"GOREHOUNDS");
    mCraftColorSchemeNames.push_back(schemeName);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"FOO FIGHTERS");
    mCraftColorSchemeNames.push_back(schemeName);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"DETHFEST");
    mCraftColorSchemeNames.push_back(schemeName);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"FIRE PHREAKS");
    mCraftColorSchemeNames.push_back(schemeName);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"STORM RIDERS");
    mCraftColorSchemeNames.push_back(schemeName);

    schemeName = new char [20];
    strcpy(schemeName, (char*)"BULLFROG");
    mCraftColorSchemeNames.push_back(schemeName);
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
    //AddRaceTrack((char*)("9. DEATH MATCH ARENA"), (char*)("extract/models/cone0-0.obj"), 5);
}
