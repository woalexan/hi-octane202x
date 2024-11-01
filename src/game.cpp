/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "game.h"

bool Game::InitIrrlicht() {
    /************************************************/
    /************** Init Irrlicht stuff *************/
    /************************************************/

    // create event receiver
    receiver = new MyEventReceiver();

    //set target screen resolution
    mGameScreenRes.set(640,480);
    //mGameScreenRes.set(1280,960);

    device = createDevice(video::EDT_OPENGL, mGameScreenRes, 16, false, false, false, receiver);

    if (device == 0) {
          cout << "Failed Irrlicht device creation!" << endl;
          return false;
    }

    //get pointer to video driver, Irrlicht scene manager
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();

    //get Irrlicht GUI functionality pointers
    guienv = device->getGUIEnvironment();

    return true;
}

bool Game::InitSFMLAudio() {
    /************************************************/
    /************** Init SFML for Audio use *********/
    /************************************************/

    uint32_t audioSampleRate = AUDIO_SAMPLERATE;

    //init music
    gameMusicPlayer = new MyMusicStream(audioSampleRate);
    if (!gameMusicPlayer->getInitOk()) {
        cout << "Music init failed!" << endl;
        return false;
    }

    gameMusicPlayer->SetVolume(volumeMusic);

    gameSoundEngine = new SoundEngine();
    if (!gameSoundEngine->getInitOk()) {
        cout << "Sound resource init failed!" << endl;
        return false;
    }

    gameSoundEngine->SetVolume(volumeSound);

    return true;
}

bool Game::InitGameAssets() {
    /***********************************************************/
    /* Extract game assets                                     */
    /***********************************************************/
    prepareData = new PrepareData(device, driver);
    if (!prepareData->PreparationOk) {
        cout << "Game assets preparation operation failed!" << endl;
        return false;
    }

    /***********************************************************/
    /* Load GameFonts                                          */
    /***********************************************************/
    GameTexts = new GameText(device, driver);

    if (!GameTexts->GameTextInitializedOk) {
        cout << "Game fonts init operation failed!" << endl;
        return false;
    }

    /***********************************************************/
    /* Load and define game assets (Meshes, Tracks, Craft)     */
    /***********************************************************/
    GameAssets = new Assets(device, driver, smgr, keepConfigDataFileUpdated);

    return true;
}

bool Game::InitGame() {
    if (!InitIrrlicht())
        return false;

    if (!InitGameAssets())
        return false;

    //InitSFMLAudio needs to be called after
    //InitGameAssets, because it needs the
    //assets already unpacked to load sounds etc..
    if (!InitSFMLAudio())
        return false;

    mTimeProfiler = new TimeProfiler();

    //create the games menue
    MainMenue = new Menue(device, driver, mGameScreenRes, GameTexts, receiver, smgr, gameSoundEngine,
                      gameMusicPlayer, GameAssets);
    if (!MainMenue->MenueInitializationSuccess) {
        cout << "Game menue init operation failed!" << endl;
        return false;
    }

    //only for debugging
    dbgTimeProfiler = guienv->addStaticText(L"Location",
           rect<s32>(100,150,300,200), false, true, NULL, -1, true);

    dbgText = guienv->addStaticText(L"",
           rect<s32>(100,250,300,350), false, true, NULL, -1, true);

    /*dbgText2 = guienv->addStaticText(L"",
           rect<s32>(350,200,450,300), false, true, NULL, -1, true);*/

    smgr->addLightSceneNode(0, vector3df(0, 100, 0),
            video::SColorf(1.0f, 1.0f, 1.0f), 1000.0f, -1);

    smgr->setAmbientLight(video::SColorf(255.0,255.0,255.0));

    return true;
}

//This function allows to quickly enter a race for
//game debugging, and to skip the menue etc.
void Game::DebugGame() {

    mDebugGame = true;

    int debugLevelNr = 1;

    //player wants to start the race
    if (this->CreateNewRace(debugLevelNr)) {
         mGameState = DEF_GAMESTATE_RACE;

         GameLoop();
    }
}

void Game::RunGame() {
    mDebugGame = false;

    if (!SkipGameIntro) {
        MainMenue->ShowIntro();
        mGameState = DEF_GAMESTATE_INTRO;
    } else {
      //go immediately to the game title screen
      MainMenue->ShowGameTitle();
      mGameState = DEF_GAMESTATE_GAMETITLE;

      //lastRaceStat = this->mCurrentRace->RetrieveFinalRaceStatistics();

      //MainMenue->ShowRaceStats(lastRaceStat);
      //mGameState = DEF_GAMESTATE_MENUE;
    }

    showTitleAbsTime = 0.0f;

    GameLoop();
}

void Game::GameLoopMenue(irr::f32 frameDeltaTime) {
    if (mGameState == DEF_GAMESTATE_GAMETITLE) {
        showTitleAbsTime += frameDeltaTime;

        if (showTitleAbsTime > 3.0f) {
            MainMenue->ShowMainMenue();

            //next line just for debugging/testing
            //of HighScore page
            //MainMenue->ShowHighscore();

            mGameState = DEF_GAMESTATE_MENUE;
        }
    }

    driver->beginScene(true,true,
                video::SColor(255,100,101,140));

    MainMenue->HandleInput();
    MainMenue->Render(frameDeltaTime);

    //advance menues absolute time (necessary
    //to control menue animations) and to trigger
    //time dependent menue actions triggers (start game demo,
    //show high score)
    MainMenue->AdvanceTime(frameDeltaTime);

    if (MainMenue->HandleActions(pendingAction)) {
        //user triggered an menue action
        if (pendingAction == MainMenue->ActQuitToOS) {
            //make sure config.dat is updated
            GameAssets->UpdateGameConfigFileExitGame();

            //user wants to quit the program
            ExitGame = true;
        }

        //this is how we handle a checkbox/slider value update
        if (pendingAction == MainMenue->ActSetDifficultyLevel) {
            //user wants to change difficulty level
            //newLevel = pendingAction->currSetValue;
        }

        //this is how we handle a playername string change
        if (pendingAction == MainMenue->ActSetPlayerName) {
            //copy new selected main player name into player name array
            char* newStr = pendingAction->newSetTextInputString;
            this->GameAssets->SetNewMainPlayerName(newStr);
        }

        if (pendingAction == MainMenue->ActSetMusicVolume) {
            //volumeMusic: 0 means no music, 100.0f means max volume
            volumeMusic = (float(pendingAction->currSetValue) / float(16)) * 100.0f;
            gameMusicPlayer->SetVolume(volumeMusic);
        }

        if (pendingAction == MainMenue->ActSetSoundVolume) {
            //volumeSound: 0 means no sound, 100.0f means max volume
            volumeSound = (float(pendingAction->currSetValue) / float(16)) * 100.0f;
            gameSoundEngine->SetVolume(volumeSound);
        }

        if (pendingAction == MainMenue->ActRace) {
            //MainMenue->ShowGameLoadingScreen();

            //player wants to start the race
            if (this->CreateNewRace(pendingAction->currSetValue)) {
                 mGameState = DEF_GAMESTATE_RACE;
            }
        }

        //take care of the special menue actions

        //is game intro playing finished or was it interrupted?
        if (pendingAction == MainMenue->ActIntroStop) {
                 //yes, change to game title screen
                 mGameState = DEF_GAMESTATE_GAMETITLE;
                 MainMenue->ShowGameTitle();
        }

        if (pendingAction == MainMenue->ActCloseRaceStatPage) {
                 //user pressed Return at race stat page
                 //cleanup race stat page and return back to
                 //main menue

                 //first cleanup data of race stats page
                 MainMenue->CleanupRaceStatsPage();

                 //do not forget to also cleanup last
                 //racestat struct memory!
                 mCurrentRace->CleanupRaceStatistics(lastRaceStat);

                 mGameState = DEF_GAMESTATE_MENUE;

                 //go back to main menue top page
                 MainMenue->ShowMainMenue();
        }
    }

    driver->endScene();
}

void Game::GameLoopRace(irr::f32 frameDeltaTime) {

    mTimeProfiler->StartOfGameLoop();

    //handle main player keyboard input
    mCurrentRace->HandleInput();

    mTimeProfiler->Profile(mTimeProfiler->tIntHandleInput);

    mCurrentRace->HandleComputerPlayers();

    mTimeProfiler->Profile(mTimeProfiler->tIntHandleComputerPlayers);

    //advance race time, execute physics, move players...
    mCurrentRace->AdvanceTime(frameDeltaTime);

    wchar_t* text = new wchar_t[200];
    wchar_t* text2 = new wchar_t[400];

    mTimeProfiler->GetTimeProfileResultDescending(text, 200, 5);

    //mCurrentRace->player->GetHeightMapCollisionSensorDebugInfo(text2, 390);

    irr::f32 deltah1 = mCurrentRace->player->cameraSensor->wCoordPnt1.Y - mCurrentRace->player->cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah2 = mCurrentRace->player->cameraSensor2->wCoordPnt1.Y - mCurrentRace->player->cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah3 = mCurrentRace->player->cameraSensor3->wCoordPnt1.Y - mCurrentRace->player->cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah4 = mCurrentRace->player->cameraSensor4->wCoordPnt1.Y - mCurrentRace->player->cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah5 = mCurrentRace->player->cameraSensor5->wCoordPnt1.Y - mCurrentRace->player->cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah6 = mCurrentRace->player->cameraSensor6->wCoordPnt1.Y - mCurrentRace->player->cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah7 = mCurrentRace->player->cameraSensor7->wCoordPnt1.Y - mCurrentRace->player->cameraSensor->wCoordPnt1.Y;

    irr::f32 maxh = fmax(deltah1, deltah2);
    maxh = fmax(maxh, deltah3);
    maxh = fmax(maxh, deltah4);
    maxh = fmax(maxh, deltah5);
    maxh = fmax(maxh, deltah6);
    maxh = fmax(maxh, deltah7);


    irr::f32 maxStep = fmax(mCurrentRace->player->cameraSensor->stepness, mCurrentRace->player->cameraSensor2->stepness);
    maxStep = fmax(maxStep, mCurrentRace->player->cameraSensor3->stepness);
    maxStep = fmax(maxStep, mCurrentRace->player->cameraSensor4->stepness);
    maxStep = fmax(maxStep, mCurrentRace->player->cameraSensor5->stepness);

    swprintf(text2, 390, L"%lf\n %lf\n %lf\n %lf\n %lf\n %lf\n %lf\n %lf\n",
                 deltah1,
                 deltah2,
                 deltah3,
                 deltah4,
                 deltah5,
             deltah6,
             deltah7,
             maxh);

    dbgTimeProfiler->setText(text);
    dbgText->setText(text2);

    delete[] text;
    delete[] text2;

    driver->beginScene(true,true,
     video::SColor(255,100,101,140));

    //render scene: terrain, blocks, player craft, entities...
    mCurrentRace->Render();

    smgr->drawAll();

    mTimeProfiler->Profile(mTimeProfiler->tIntRender3DScene);

    //2nd draw HUD over the scene, needs to be done at the end
    mCurrentRace->DrawHUD(frameDeltaTime);
    mCurrentRace->DrawMiniMap(frameDeltaTime);

    //last draw text debug output from Irrlicht
    guienv->drawAll();

    mTimeProfiler->Profile(mTimeProfiler->tIntRender2D);

    driver->endScene();

    //does the player want to end the race?
    if (mCurrentRace->exitRace) {
        mCurrentRace->End();

        //clean up current race data
        delete mCurrentRace;
        mCurrentRace = NULL;

        if (mDebugGame) {
            ExitGame = true;
        } else {
            mGameState = DEF_GAMESTATE_MENUE;
            MainMenue->ShowMainMenue();
        }
    }
}

void Game::GameLoop() {

    // In order to do framerate independent movement, we have to know
    // how long it was since the last frame
    u32 then = device->getTimer()->getTime();

    while (device->run() && (ExitGame == false)) {
       /*  if (device->isWindowActive())
            {*/

        //Work out a frame delta time.
        const u32 now = device->getTimer()->getTime();
        f32 frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
        then = now;

        switch (mGameState) {
            case DEF_GAMESTATE_GAMETITLE:
            case DEF_GAMESTATE_INTRO:
            case DEF_GAMESTATE_MENUE: {
                GameLoopMenue(frameDeltaTime);
                break;
            }

            case DEF_GAMESTATE_RACE: {
                GameLoopRace(frameDeltaTime);
                break;
            }
        }

        int fps = driver->getFPS();

        if (lastFPS != fps) {
                         core::stringw tmp(L"Hi-Octane 202X [");
                         tmp += driver->getName();
                         tmp += L"] Triangles drawn: ";
                         tmp += driver->getPrimitiveCountDrawn();
                         tmp += " @ fps: ";
                         tmp += fps;

                         device->setWindowCaption(tmp.c_str());
                         lastFPS = fps;
                     }
            //}
   }

   //cleanup game texts
   delete GameTexts;

   //cleanup game assets
   delete GameAssets;

   //delete sound/music
   delete gameSoundEngine;
   delete gameMusicPlayer;

   delete mTimeProfiler;

   driver->drop();
}

bool Game::CreateNewRace(int load_levelnr) {
    if (mCurrentRace != NULL)
        return false;

    gameSoundEngine->StartEngineSound();

    //create a new Race
    mCurrentRace = new Race(device, driver, smgr, receiver, GameTexts, gameMusicPlayer, gameSoundEngine,
                           mTimeProfiler, this->mGameScreenRes, load_levelnr);

    mCurrentRace->Init();

    if (!mCurrentRace->ready) {
        //there was a problem with Race initialization
        cout << "Race creation failed!" << endl;
        return false;
    }

    return true;
}

void Game::CleanUpRace() {
    if (mCurrentRace == NULL)
        return;
}

Game::Game() {
}
