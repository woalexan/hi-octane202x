/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef GAME_H
#define GAME_H

#include "infrabase.h"
#include "xeffects/XEffects.h"

#define DEF_GAMESTATE_AFTERINIT 0
#define DEF_GAMESTATE_EXTRACTDATA 1
#define DEF_GAMESTATE_INITSTEP2 2
#define DEF_GAMESTATE_INTRO 3
#define DEF_GAMESTATE_GAMETITLE 4
#define DEF_GAMESTATE_MENUE 5
#define DEF_GAMESTATE_RACE 6
#define DEF_GAMESTATE_DEMORACE 7
#define DEF_GAMESTATE_INITRACE 8
#define DEF_GAMESTATE_INITDEMO 9
#define DEF_GAMESTATE_ERROR 10

/************************
 * Forward declarations *
 ************************/

class Menue;
struct RaceStatsEntryStruct;
struct PointTableEntryStruct;
struct PilotInfoStruct;
struct MenueAction;
class Logger;
class SoundEngine;
class Assets;
class IntroPlayer;
class MyMusicStream;
class Race;

class Game : public InfrastructureBase {
private:
    //Irrlicht related, for debugging of game
    irr::gui::IGUIStaticText* dbgTimeProfiler = nullptr;
    irr::gui::IGUIStaticText* dbgText = nullptr;
    irr::gui::IGUIStaticText* dbgText2 = nullptr;

    Assets* mGameAssets = nullptr;

    //SFML related, Audio, Music
    MyMusicStream* gameMusicPlayer = nullptr;
    SoundEngine* gameSoundEngine = nullptr;

    //own game stuff
    Menue* MainMenue = nullptr;
    IntroPlayer* gameIntroPlayer = nullptr;

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
    MenueAction* pendingAction = nullptr;

    std::vector<RaceStatsEntryStruct*>* lastRaceStat = nullptr;
    std::vector<PointTableEntryStruct*>* lastRacePointTable = nullptr;
    std::vector<PointTableEntryStruct*>* lastOverallChampionshipPointTable = nullptr;

    irr::u8 mSaveChampionShipToWhichSlot = 0;

    void AdvanceChampionship();

    void CleanUpPointTable(std::vector<PointTableEntryStruct*> &tablePntr);

    void GameLoop();
    void GameLoopMenue(irr::f32 frameDeltaTime);
    void GameLoopRace(irr::f32 frameDeltaTime);

    void HandleMenueActions();

    Race* mCurrentRace = nullptr;
    bool mTimeStopped = false;

    bool CreateNewRace(int load_levelnr, std::vector<PilotInfoStruct*> pilotInfo, irr::u8 nrLaps, bool demoMode, bool debugRace);
    bool CreateNewRace(std::string targetLevel, std::vector<PilotInfoStruct*> pilotInfo,
                             irr::u8 nrLaps, bool demoMode, bool debugRace);

    void RenderDataExtractionScreen();
    bool LoadBackgroundImage();
    void GameLoopExtractData();
    void GameLoopTitleScreenLoadData();
    void GameLoopLoadRaceScreen();
    bool LoadAdditionalGameImages();
    void GameLoopInitStep2();

    bool LoadGameData();

    void GameLoopIntro(irr::f32 frameDeltaTime);

    void CleanUpRace();

    bool mAdvanceFrameMode = false;
    irr::s32 mAdvanceFrameCnt = 0;

    std::vector<PilotInfoStruct*> mPilotsNextRace;

    void CleanupPilotInfo(std::vector<PilotInfoStruct*> &pilotInfo);

    //special images for the game
    irr::video::ITexture* gameTitle = nullptr;
    irr::core::vector2di gameTitleDrawPos;
    irr::core::dimension2d<irr::u32> gameTitleSize;

    irr::video::ITexture* raceLoadingScr = nullptr;
    irr::core::vector2di raceLoadingScrDrawPos;
    irr::core::dimension2d<irr::u32> raceLoadingScrSize;

    int nextRaceLevelNr = 1;

    //Returns false if game should exit, False otherwise
    bool ParseCommandLineForGame();

    bool mTestMapMode = false;
    std::string mTestTargetLevel;

public:
    irr::video::ITexture* backgnd = nullptr;

    bool mUseXEffects;

    EffectHandler* mEffect = nullptr;

    //ShadowMap settings
    E_FILTER_TYPE mShadowMapFilterType;
    irr::u32 mShadowMapResolution;

    //if true skips main menue, and jumps directly to
    //defined race for game mechanics debugging
    bool mDebugRace = false;
    bool mDebugDemoMode = false;

    bool enableLightning = false;
    bool enableShadows = false;
    bool fullscreen = false;
    bool DebugShowVariableBoxes = false;

    //Returns true for success, false for error occured
    bool InitGameStep1(bool useXEffects);
    bool InitGameStep2();

    void RunGame();
    void SetupDebugGame();
    void SetupDebugDemo();

    void StopTime();
    void StartTime();
    void AdvanceFrame(irr::s32 advanceFrameCount);

    Game(int argc, char **argv);
    virtual ~Game();
};

#endif // GAME_H
