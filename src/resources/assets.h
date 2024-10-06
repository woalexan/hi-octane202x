/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ASSETS_H
#define ASSETS_H

#include "irrlicht/irrlicht.h"
#include <vector>

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
    char bestPlayer[50];

    //current best high score
    //for this race track
    irr::u32 bestHighScore;

    //name of current best high score
    //player
    char bestHighScorePlayer[50];

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

class Assets {
public:
    Assets(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::scene::ISceneManager* smgr);
    ~Assets();

    std::vector<RaceTrackInfoStruct*> *mRaceTrackVec;
    std::vector<CraftInfoStruct*> *mCraftVec;
    std::vector<char*> mCraftColorSchemeNames;

private:
    irr::video::IVideoDriver* myDriver;
    irr::IrrlichtDevice* myDevice;
    irr::scene::ISceneManager* mySmgr;

    irr::u8 currLevelNr = 1;
    irr::u8 currCraftNr = 1;

    void InitRaceTracks();
    void AddRaceTrack(char* nameTrack, char* meshFileName, irr::u8 defaultNrLaps);
    void InitCrafts();
    void AddCraft(char* nameCraft, char* meshFileName, irr::u8 statSpeed, irr::u8 statArmour, irr::u8 statWeight, irr::u8 statFirePower);
};

#endif // ASSETS_H
