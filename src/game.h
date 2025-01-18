/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef GAME_H
#define GAME_H

#include <irrlicht/irrlicht.h>
#include "utils/tprofile.h"
#include "input/input.h"
#include "resources/readgamedata/preparedata.h"
#include "draw/menue.h"
#include "race.h"
#include "draw/hud.h"
#include "utils/logger.h"

#define DEF_GAMESTATE_AFTERINIT 0
#define DEF_GAMESTATE_INTRO 1
#define DEF_GAMESTATE_GAMETITLE 2
#define DEF_GAMESTATE_MENUE 3
#define DEF_GAMESTATE_RACE 4
#define DEF_GAMESTATE_DEMORACE 5

using namespace std;

using namespace irr;
using namespace irr::core;
using namespace irr::video;
using namespace irr::scene;
using namespace irr::gui;

class Menue; //Forward declaration
struct RaceStatsEntryStruct; //Forward declaration
struct MenueAction; //Forward declaration
class Logger; //Forward declaration

class Game {
private:
    //Irrlicht stuff
    IrrlichtDevice*                 device;
    video::IVideoDriver*	        driver;
    scene::ISceneManager*           smgr;
    MyEventReceiver* receiver;
    IGUIEnvironment* guienv;

    //Irrlicht related, for debugging of game
    IGUIStaticText* dbgTimeProfiler;
    IGUIStaticText* dbgText;
    IGUIStaticText* dbgText2;

    //SFML related, Audio, Music
    MyMusicStream* gameMusicPlayer = NULL;
    SoundEngine* gameSoundEngine = NULL;

    float volumeSound = 60.0f; //60.0f;
    float volumeMusic = 90.0f; //90.0f;

    //own game stuff
    TimeProfiler* mTimeProfiler;
    PrepareData* prepareData;
    GameText* GameTexts;
    Menue* MainMenue;
    Assets* GameAssets;

    //stores the current gamestate
    irr::u8 mGameState;

    //counter to count how long we have showed
    //the games title
    irr::f32 showTitleAbsTime = 0.0f;

    bool ExitGame = false;
    bool mDebugGame = false;

    //set true to skip playing
    //game intro
    bool SkipGameIntro = true;

    //if true config.dat file is updated
    //with latest selected choices
    bool keepConfigDataFileUpdated = false;

    //Returns true for success, false for error occured
    bool InitIrrlicht();
    bool InitGameAssets();
    bool InitSFMLAudio();

    int lastFPS = -1;
    MenueAction* pendingAction = NULL;

    std::vector<RaceStatsEntryStruct*>* lastRaceStat;

    void GameLoop();
    void GameLoopMenue(irr::f32 frameDeltaTime);
    void GameLoopRace(irr::f32 frameDeltaTime);

    Race* mCurrentRace = NULL;
    bool mTimeStopped = false;

    bool CreateNewRace(int load_levelnr);
    bool RunDemoMode(int load_levelnr);

    void CleanUpRace();

    bool mAdvanceFrameMode = false;
    irr::s32 mAdvanceFrameCnt = 0;

public:
    dimension2d<u32> mGameScreenRes;
    Logger* mLogger;

    bool enableLightning = false;
    bool enableShadows = false;
    bool fullscreen = false;
    bool DebugShowVariableBoxes = false;

    bool computerPlayersAttack = true;
    bool runDemoMode = false;

    //Returns true for success, false for error occured
    bool InitGame();
    void RunGame();
    void DebugGame();

    void StopTime();
    void StartTime();
    void AdvanceFrame(irr::s32 advanceFrameCount);

    Game();
    ~Game();
};

#endif // GAME_H
