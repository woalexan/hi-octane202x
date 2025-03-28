/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef GAME_H
#define GAME_H

#include "draw/menue.h"
#include "race.h"
#include "draw/hud.h"
#include "infrabase.h"

#define DEF_GAMESTATE_AFTERINIT 0
#define DEF_GAMESTATE_INTRO 1
#define DEF_GAMESTATE_GAMETITLE 2
#define DEF_GAMESTATE_MENUE 3
#define DEF_GAMESTATE_RACE 4
#define DEF_GAMESTATE_DEMORACE 5

class Menue; //Forward declaration
struct RaceStatsEntryStruct; //Forward declaration
struct PointTableEntryStruct; //Forward declaration
struct PilotInfoStruct; //Forward declaration
struct MenueAction; //Forward declaration
class Logger; //Forward declaration
class SoundEngine; //Forward declaration
class Assets; //Forward declaration
class InfrastructureBase; //Forward declaration

class Game {
private:
    //Irrlicht related, for debugging of game
    IGUIStaticText* dbgTimeProfiler = NULL;
    IGUIStaticText* dbgText = NULL;
    IGUIStaticText* dbgText2 = NULL;

    Assets* mGameAssets = NULL;
    bool InitGameAssets();

    //SFML related, Audio, Music
    MyMusicStream* gameMusicPlayer = NULL;
    SoundEngine* gameSoundEngine = NULL;

    //own game stuff
    Menue* MainMenue = NULL;
    InfrastructureBase* mInfra = NULL;

    //stores the current gamestate
    irr::u8 mGameState = DEF_GAMESTATE_INTRO;

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

    bool InitSFMLAudio();

    int lastFPS = -1;
    MenueAction* pendingAction = NULL;

    std::vector<RaceStatsEntryStruct*>* lastRaceStat = NULL;
    std::vector<PointTableEntryStruct*>* lastRacePointTable = NULL;
    std::vector<PointTableEntryStruct*>* lastOverallChampionshipPointTable = NULL;

    irr::u8 mSaveChampionShipToWhichSlot = 0;

    void AdvanceChampionship();

    void CleanUpPointTable(std::vector<PointTableEntryStruct*> &tablePntr);

    void GameLoop();
    void GameLoopMenue(irr::f32 frameDeltaTime);
    void GameLoopRace(irr::f32 frameDeltaTime);

    void HandleMenueActions();

    Race* mCurrentRace = NULL;
    bool mTimeStopped = false;

    bool CreateNewRace(int load_levelnr, std::vector<PilotInfoStruct*> pilotInfo, bool demoMode, bool debugRace);
    bool RunDemoMode(int load_levelnr);

    void CleanUpRace();

    bool mAdvanceFrameMode = false;
    irr::s32 mAdvanceFrameCnt = 0;

    std::vector<PilotInfoStruct*> mPilotsNextRace;

    void CleanupPilotInfo(std::vector<PilotInfoStruct*> &pilotInfo);

public:

    bool enableLightning = false;
    bool enableShadows = false;
    bool fullscreen = false;
    bool DebugShowVariableBoxes = false;

    bool computerPlayersAttack = false;
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
