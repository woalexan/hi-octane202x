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
#include "draw/introplayer.h"
#include "infrabase.h"

#define DEF_GAMESTATE_AFTERINIT 0
#define DEF_GAMESTATE_EXTRACTDATA 1
#define DEF_GAMESTATE_LOADDATA 2
#define DEF_GAMESTATE_INTRO 3
#define DEF_GAMESTATE_GAMETITLE 4
#define DEF_GAMESTATE_MENUE 5
#define DEF_GAMESTATE_RACE 6
#define DEF_GAMESTATE_DEMORACE 7
#define DEF_GAMESTATE_INITRACE 8
#define DEF_GAMESTATE_INITDEMO 9
#define DEF_GAMESTATE_ERROR 10

class Menue; //Forward declaration
struct RaceStatsEntryStruct; //Forward declaration
struct PointTableEntryStruct; //Forward declaration
struct PilotInfoStruct; //Forward declaration
struct MenueAction; //Forward declaration
class Logger; //Forward declaration
class SoundEngine; //Forward declaration
class Assets; //Forward declaration
class InfrastructureBase; //Forward declaration
class IntroPlayer; //Forward declaration
class MyMusicStream; //forward declaration

class Game {
private:
    //Irrlicht related, for debugging of game
    IGUIStaticText* dbgTimeProfiler = NULL;
    IGUIStaticText* dbgText = NULL;
    IGUIStaticText* dbgText2 = NULL;

    Assets* mGameAssets = NULL;

    //SFML related, Audio, Music
    MyMusicStream* gameMusicPlayer = NULL;
    SoundEngine* gameSoundEngine = NULL;

    //own game stuff
    Menue* MainMenue = NULL;
    IntroPlayer* gameIntroPlayer = NULL;
    InfrastructureBase* mInfra = NULL;

    //stores the current gamestate
    irr::u8 mGameState = DEF_GAMESTATE_AFTERINIT;

    bool ExitGame = false;

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

    void RenderDataExtractionScreen();
    bool LoadBackgroundImage();
    void GameLoopExtractData();
    void GameLoopTitleScreenLoadData();
    void GameLoopLoadRaceScreen();
    bool LoadAdditionalGameImages();

    bool LoadGameData();

    void GameLoopIntro(irr::f32 frameDeltaTime);

    void CleanUpRace();

    bool mAdvanceFrameMode = false;
    irr::s32 mAdvanceFrameCnt = 0;

    std::vector<PilotInfoStruct*> mPilotsNextRace;

    void CleanupPilotInfo(std::vector<PilotInfoStruct*> &pilotInfo);

    //special images for the game
    irr::video::ITexture* gameTitle;
    irr::core::vector2di gameTitleDrawPos;
    irr::core::dimension2d<irr::u32> gameTitleSize;

    irr::video::ITexture* raceLoadingScr;
    irr::core::vector2di raceLoadingScrDrawPos;
    irr::core::dimension2d<irr::u32> raceLoadingScrSize;

    //if true skips main menue, and jumps directly to
    //defined race for game mechanics debugging
    bool mDebugRace = false;
    bool mDebugDemoMode = true;

    int nextRaceLevelNr = 1;

public:
    irr::video::ITexture* backgnd;

    bool enableLightning = false;
    bool enableShadows = false;
    bool fullscreen = false;
    bool DebugShowVariableBoxes = false;

    bool computerPlayersAttack = true;

    //Returns true for success, false for error occured
    bool InitGameStep1();
    bool InitGameStep2();

    void RunGame();
    void SetupDebugGame();
    void SetupDebugDemo();

    void StopTime();
    void StartTime();
    void AdvanceFrame(irr::s32 advanceFrameCount);

    Game();
    ~Game();
};

#endif // GAME_H
