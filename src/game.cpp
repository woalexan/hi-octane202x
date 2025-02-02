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

    //we need to enable stencil buffers, otherwise volumentric shadows
    //will not work
    device = createDevice(video::EDT_OPENGL, mGameScreenRes, 16, fullscreen, enableShadows, false, receiver);

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

void Game::StopTime() {
    if (!mTimeStopped) {
        mTimeStopped = true;
    }
}

void Game::StartTime() {
    if (mTimeStopped) {
        mTimeStopped = false;
        mAdvanceFrameMode = false;
    }
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
    try {
        prepareData = new PrepareData(device, driver);
    }
    catch (const std::string &msg) {
        cout << "Game assets preparation operation failed!\n" << msg << endl;
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

    //log window left upper corner 100, 380
    //log window right lower corner 540, 460
    irr::core::rect logWindowPos(100, 380, 540, 460);

    //create my logger class
    mLogger = new Logger(guienv, logWindowPos);
    mLogger->HideWindow();

    if (!InitGameAssets())
        return false;

    mLogger->AddLogMessage((char*)"Game Assets initialized");

    //InitSFMLAudio needs to be called after
    //InitGameAssets, because it needs the
    //assets already unpacked to load sounds etc..
    if (!InitSFMLAudio())
        return false;

    mLogger->AddLogMessage((char*)"Audio init ok");

    mTimeProfiler = new TimeProfiler();

    //create the games menue
    MainMenue = new Menue(device, driver, mGameScreenRes, GameTexts, receiver, smgr, gameSoundEngine,
                      gameMusicPlayer, GameAssets);
    if (!MainMenue->MenueInitializationSuccess) {
        cout << "Game menue init operation failed!" << endl;
        return false;
    }

    if (DebugShowVariableBoxes) {
            //only for debugging
            dbgTimeProfiler = guienv->addStaticText(L"Location",
                   rect<s32>(100,150,300,200), false, true, NULL, -1, true);

            dbgText = guienv->addStaticText(L"",
                   rect<s32>(100,250,300,350), false, true, NULL, -1, true);

            /*dbgText2 = guienv->addStaticText(L"",
                   rect<s32>(350,200,450,300), false, true, NULL, -1, true);*/
    }

    if (enableLightning) {
        smgr->addLightSceneNode(0, vector3df(0, 100, 100),
            video::SColorf(1.0f, 1.0f, 1.0f), 1000.0f, -1);
    }

    //set a minimum amount of light everywhere, to not have black areas
    //in the level
    if (enableLightning) {
       smgr->setAmbientLight(video::SColorf(0.4f, 0.4f, 0.4f));
    } else {
        //set max brightness everywhere
        smgr->setAmbientLight(video::SColorf(1.0f, 1.0f, 1.0f));
    }

    if (enableShadows) {
       smgr->setShadowColor(video::SColor(150,0,0,0));
    }

    return true;
}

//This function allows to quickly enter a race for
//game debugging, and to skip the menue etc.
void Game::DebugGame() {
    mDebugGame = true;

    int debugLevelNr = 1;

    if (!runDemoMode) {
        //player wants to start the race
        if (this->CreateNewRace(debugLevelNr, mDebugGame)) {
             mGameState = DEF_GAMESTATE_RACE;

             GameLoop();
        }
    } else {
        //we want to start a demo
        if (this->RunDemoMode(debugLevelNr)) {
             mGameState = DEF_GAMESTATE_DEMORACE;

             GameLoop();
        }
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

void Game::AdvanceFrame(irr::s32 advanceFrameCount) {
    if (this->mTimeStopped) {
        mAdvanceFrameMode = true;
        mAdvanceFrameCnt = advanceFrameCount;
    }
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
            if (this->CreateNewRace(pendingAction->currSetValue, mDebugGame)) {
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

    mCurrentRace->HandleBasicInput();

    if (mAdvanceFrameMode) {   
        if (mAdvanceFrameCnt > 0) {
            mTimeStopped = false;
            mAdvanceFrameCnt--;
        } else {
            mTimeStopped = true;
            mAdvanceFrameMode = false;
        }
    }

    if (!this->mTimeStopped) {
        //advance race time, execute physics, move players...
        mCurrentRace->AdvanceTime(frameDeltaTime);
    }

    if (!this->mTimeStopped) {
        //handle main player keyboard input
        mCurrentRace->HandleInput();
    }

    mTimeProfiler->Profile(mTimeProfiler->tIntHandleInput);

    if (!this->mTimeStopped) {
        mCurrentRace->HandleComputerPlayers(frameDeltaTime);
    }

    mTimeProfiler->Profile(mTimeProfiler->tIntHandleComputerPlayers);

    if (DebugShowVariableBoxes) {

        wchar_t* text = new wchar_t[200];
        wchar_t* text2 = new wchar_t[400];

        mTimeProfiler->GetTimeProfileResultDescending(text, 200, 5);

       //mCurrentRace->player->GetHeightMapCollisionSensorDebugInfo(text2, 390);

        /*    irr::f32 deltah1 = mCurrentRace->player->cameraSensor->wCoordPnt1.Y - mCurrentRace->player->cameraSensor->wCoordPnt1.Y;
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
                     maxh);*/

           /* swprintf(text2, 390, L"camY: %lf\n camYTarget: %lf\n avg: %lf\n newCamHeight: %lf\n maxh: %lf\n minCeiling: %lf\n",
                        mCurrentRace->player->dbgCameraVal,
                      mCurrentRace->player->dbgCameraTargetVal,
                         mCurrentRace->player->dbgCameraAvgVAl,
                         mCurrentRace->player->dbgNewCameraVal,
                         mCurrentRace->player->dbgMaxh,
                         mCurrentRace->player->dbgMinCeilingFound);*/

            //irr::f32 absolutePlayerAngle = this->mCurrentRace->GetAbsOrientationAngleFromDirectionVec(this->mCurrentRace->player->craftForwardDirVec);

              /*swprintf(text2, 390, L"currDist = %lf\nTargetDist = %lf\ncurrAngle = %lf\ncurrTargetAngle = %lf\ntargetOffsetAngle = %lf\n",
                             this->mCurrentRace->player2->mCurrentCraftDistToWaypointLink,
                             this->mCurrentRace->player2->mCurrentCraftDistWaypointLinkTarget,
                             this->mCurrentRace->player2->mCurrentCraftOrientationAngle,
                             this->mCurrentRace->player2->mCurrentWaypointLinkAngle,
                             this->mCurrentRace->player2->mCurrentCraftTargetOrientationOffsetAngle);*/


            /*swprintf(text2, 390, L"nCurrSeg = %d\nSegments = %d\n",
                           this->mCurrentRace->player2->mCurrentPathSegCurrSegmentNr,
                           this->mCurrentRace->player2->mCurrentPathSegNrSegments);*/

           /* swprintf(text2, 390, L"nAvailWay = %d\n nAvailLinks = %d\n nCurrSeg = %d\nSegments = %d\n",
                           this->mCurrentRace->player2->mDbgCpAvailWaypointNr,
                           this->mCurrentRace->player2->mDbgCpAvailWayPointLinksNr,
                         this->mCurrentRace->player2->mCurrentPathSegCurrSegmentNr,
                          this->mCurrentRace->player2->mCurrentPathSegNrSegments);*/

            /*swprintf(text2, 390, L"nCollectable = %d\n",
                           this->mCurrentRace->player2->mCpCollectablesSeenByPlayer.size());*/


           /* swprintf(text2, 390, L"Angle Error = %lf\nDist error = %lf\n",
                           this->mCurrentRace->player2->mAngleError,
                           this->mCurrentRace->player2->dbgDistError);*/

            /*swprintf(text2, 390, L"x = %lf\n y = %lf\n z = %lf\n",
                              this->mCurrentRace->player->phobj->physicState.position.X,
                            this->mCurrentRace->player->phobj->physicState.position.Y,
                          this->mCurrentRace->player->phobj->physicState.position.Z);*/

           /* swprintf(text2, 390, L"front = %lf\n left = %lf\n right = %lf\n back = %lf\n currOffset = %lf\n",
                              this->mCurrentRace->player2->mCraftDistanceAvailFront,
                              this->mCurrentRace->player2->mCraftDistanceAvailLeft,
                              this->mCurrentRace->player2->mCraftDistanceAvailRight,
                              this->mCurrentRace->player2->mCraftDistanceAvailBack,
                              this->mCurrentRace->player2->mCpCurrPathOffset
                            );*/

           /* if (this->mCurrentRace->mPlayerVec.at(0)->mCurrJumping) {
                swprintf(text2, 390, L"Jumping %lf %lf\n %lf %lf\n", this->mCurrentRace->mPlayerVec.at(0)->lastHeightFront,
                         this->mCurrentRace->mPlayerVec.at(0)->currHeightFront,
                         this->mCurrentRace->mPlayerVec.at(0)->lastHeightBack,
                         this->mCurrentRace->mPlayerVec.at(0)->currHeightBack);
            } else {
               swprintf(text2, 390, L"%lf %lf\n %lf %lf\n" ,this->mCurrentRace->mPlayerVec.at(0)->lastHeightFront,
                        this->mCurrentRace->mPlayerVec.at(0)->currHeightFront,
                        this->mCurrentRace->mPlayerVec.at(0)->lastHeightBack,
                        this->mCurrentRace->mPlayerVec.at(0)->currHeightBack);
            }*/
/*
           swprintf(text2, 390, L"Curr Offset: %lf\n Left Avail: %lf\n Right Avail: %lf\n WALeftAvail: %lf\n WARightAvai: %lf\n NoClearLink: %d\n LostProgress: %d\n %d / %d Reverse: %d",
                     this->mCurrentRace->mPlayerVec.at(1)->mCpCurrPathOffset,
                      this->mCurrentRace->mPlayerVec.at(1)->mCpFollowedWayPointLinkCurrentSpaceLeftSide,
                     this->mCurrentRace->mPlayerVec.at(1)->mCpFollowedWayPointLinkCurrentSpaceRightSide,
                     this->mCurrentRace->mPlayerVec.at(1)->mCraftDistanceAvailLeft,
                     this->mCurrentRace->mPlayerVec.at(1)->mCraftDistanceAvailRight,
                      this->mCurrentRace->mPlayerVec.at(1)->mCPTrackMovementNoClearClosestLinkCnter,
                    this->mCurrentRace->mPlayerVec.at(1)->mCPTrackMovementLostProgressCnter,
                    this->mCurrentRace->mPlayerVec.at(1)->mCurrentPathSegCurrSegmentNr,
                    this->mCurrentRace->mPlayerVec.at(1)->mCurrentPathSegNrSegments,
                    this->mCurrentRace->mPlayerVec.at(1)->mCurrentPathSegSortedOutReverse.size()
                    );*/

          swprintf(text2, 390, L"");

           /*
          swprintf(text2, 390, L"Nr Lap = %d Next Checkpoint = %d Rem Dist = %lf\n", //Nr Lap = %d Next Checkpoint = %d Rem Dist = %lf\n",
                        this->mCurrentRace->mPlayerVec.at(0)->mPlayerStats->currLapNumber,
                        this->mCurrentRace->mPlayerVec.at(0)->nextCheckPointValue,
                        this->mCurrentRace->mPlayerVec.at(0)->remainingDistanceToNextCheckPoint

                   this->mCurrentRace->mPlayerVec.at(1)->mPlayerStats->currLapNumber,
                   this->mCurrentRace->mPlayerVec.at(1)->nextCheckPointValue,
                   this->mCurrentRace->mPlayerVec.at(1)->remainingDistanceToNextCheckPoint
                 //  );   */


            dbgTimeProfiler->setText(text);
            dbgText->setText(text2);

            delete[] text;
            delete[] text2;
    }

    driver->beginScene(true,true,
     video::SColor(255,100,101,140));

    //render scene: terrain, blocks, player craft, entities...
    mCurrentRace->Render();

    smgr->drawAll();

    mTimeProfiler->Profile(mTimeProfiler->tIntRender3DScene);

    //2nd draw HUD over the scene, needs to be done at the end
    mCurrentRace->DrawHUD(frameDeltaTime);
    mCurrentRace->DrawMiniMap(frameDeltaTime);

    //render log window
    mLogger->Render();

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

            case DEF_GAMESTATE_DEMORACE:
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

bool Game::CreateNewRace(int load_levelnr, bool debugRace) {
    if (mCurrentRace != NULL)
        return false;

    gameSoundEngine->StartEngineSound();

    //create a new Race
    mCurrentRace = new Race(device, driver, smgr, receiver, GameTexts, this, gameMusicPlayer, gameSoundEngine,
                           mTimeProfiler, this->mGameScreenRes, load_levelnr, false, debugRace, false);

    mCurrentRace->Init();

    //add first human player
    std::string pl1Model("extract/models/car0-0.obj");
    mCurrentRace->AddPlayer(true, (char*)"PLAYER", pl1Model);

    //std::string player_model("extract/models/jet0-0.obj");
   //    std::string player_model2("extract/models/bike0-0.obj");
   //    //std::string player_model("extract/models/car0-0.obj");
   //    //std::string player_model("extract/models/jugga0-0.obj");
   //    //std::string player_model("extract/models/marsh0-0.obj");
   //    //std::string player_model("extract/models/skim0-0.obj");

    //add computer player 1
    std::string pl2Model("extract/models/bike0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIE", pl2Model);

    //add computer player 2
    std::string pl3Model("extract/models/jugga0-3.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIZ", pl3Model);

    //add computer player 3
    std::string pl4Model("extract/models/skim0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KID", pl4Model);

    //add computer player 4
    std::string pl5Model("extract/models/bike0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIV", pl5Model);

    //add computer player 5
    std::string pl6Model("extract/models/marsh0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIF", pl6Model);

    //add computer player 6
    std::string pl7Model("extract/models/jet0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIS", pl7Model);

    //add computer player 7
    std::string pl8Model("extract/models/tank0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIA", pl8Model);

    mCurrentRace->currPlayerFollow = this->mCurrentRace->mPlayerVec.at(0);
    mCurrentRace->Hud1Player->SetMonitorWhichPlayer(mCurrentRace->mPlayerVec.at(0));

    if (!mCurrentRace->ready) {
        //there was a problem with Race initialization
        cout << "Race creation failed!" << endl;
        return false;
    }

    //StopTime();

    return true;
}

bool Game::RunDemoMode(int load_levelnr) {
    if (mCurrentRace != NULL)
        return false;

    gameSoundEngine->StartEngineSound();

    //create a new Race in Demo Mode
    mCurrentRace = new Race(device, driver, smgr, receiver, GameTexts, this, gameMusicPlayer, gameSoundEngine,
                           mTimeProfiler, this->mGameScreenRes, load_levelnr, true, false, false);

    mCurrentRace->Init();

    //add computer player 1
    std::string pl2Model("extract/models/bike0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIE", pl2Model);

    //add computer player 2
    std::string pl3Model("extract/models/jugga0-3.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIZ", pl3Model);

    //add computer player 3
    std::string pl4Model("extract/models/skim0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KID", pl4Model);

    //add computer player 4
    std::string pl5Model("extract/models/bike0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIV", pl5Model);

    //add computer player 5
    std::string pl6Model("extract/models/marsh0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIF", pl6Model);

    //add computer player 6
    std::string pl7Model("extract/models/jet0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIS", pl7Model);

    //add computer player 7
    std::string pl8Model("extract/models/tank0-0.obj");
    mCurrentRace->AddPlayer(false, (char*)"KIA", pl8Model);

    mCurrentRace->currPlayerFollow = this->mCurrentRace->mPlayerVec.at(0);
    mCurrentRace->Hud1Player->SetMonitorWhichPlayer(mCurrentRace->mPlayerVec.at(0));

    //StopTime();

    if (!mCurrentRace->ready) {
        //there was a problem with Race initialization
        cout << "Demo creation failed!" << endl;
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
