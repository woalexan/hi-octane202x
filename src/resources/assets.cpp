/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "assets.h"

Assets::Assets(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::scene::ISceneManager* smgr) {
    this->myDevice = device;
    this->myDriver = driver;
    this->mySmgr = smgr;

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

    currLevelNr++;

    strcpy(newTrack->name, nameTrack);

    //load the mesh
    newTrack->MeshTrack = this->mySmgr->getMesh(meshFileName);
    strcpy(newTrack->bestPlayer, "BULLFROG");
    strcpy(newTrack->bestHighScorePlayer, "BULLFROG");
    newTrack->bestLapTime = 9999;
    newTrack->bestHighScore = 9999;

    strcpy(newTrack->meshFileName, meshFileName);
    newTrack->defaultNrLaps = defaultNrLaps;
    newTrack->currSelNrLaps = newTrack->defaultNrLaps;

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
    AddRaceTrack((char*)("7. ANCIENT MINE TOWN"), (char*)("extract/models/cone0-0.obj"), 5);

    //Track8, TODO: fix race track 3D model if I get it one day, also fix to correct number of default laps (I do not know)
    AddRaceTrack((char*)("8. ARCTIC LAND"), (char*)("extract/models/cone0-0.obj"), 5);

    //Track9, TODO: fix race track 3D model if I get it one day, also fix to correct number of default laps (I do not know)
    AddRaceTrack((char*)("9. DEATH MATCH ARENA"), (char*)("extract/models/cone0-0.obj"), 5);
}
