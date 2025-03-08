/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "game.h"

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

    //volumeMusic: 0 means no music, 100.0f means max volume
    //get configured volume from Assets class
    gameMusicPlayer->SetVolume(mGameAssets->GetMusicVolume());

    gameSoundEngine = new SoundEngine();
    if (!gameSoundEngine->getInitOk()) {
        cout << "Sound resource init failed!" << endl;
        return false;
    }

    gameSoundEngine->SetVolume(mGameAssets->GetSoundVolume());

    return true;
}

bool Game::InitGameAssets() {
    /***********************************************************/
    /* Load and define game assets (Meshes, Tracks, Craft)     */
    /***********************************************************/
    mGameAssets = new Assets(mInfra->mDevice, mInfra->mDriver, mInfra->mSmgr, keepConfigDataFileUpdated);

    return true;
}

bool Game::InitGame() {
    dimension2d<u32> targetResolution;

    //set target screen resolution
    targetResolution.set(640,480);
    //targetResolution.set(1280,960);

    //create my infrastructure
    mInfra = new InfrastructureBase(targetResolution, fullscreen, enableShadows);
    if (!mInfra->GetInitOk())
        return false;

    InitGameAssets();

    //InitSFMLAudio needs to be called after
    //InitGameResources, because it needs the
    //assets already unpacked to load sounds etc..
    if (!InitSFMLAudio())
        return false;

    mInfra->mLogger->AddLogMessage((char*)"Audio init ok");

    //create the games menue
    MainMenue = new Menue(mInfra, gameSoundEngine,
                      gameMusicPlayer, mGameAssets);
    if (!MainMenue->MenueInitializationSuccess) {
        cout << "Game menue init operation failed!" << endl;
        return false;
    }

    if (DebugShowVariableBoxes) {
            //only for debugging
            dbgTimeProfiler = mInfra->mGuienv->addStaticText(L"Location",
                   rect<s32>(100,150,300,200), false, true, NULL, -1, true);

            dbgText = mInfra->mGuienv->addStaticText(L"",
                   rect<s32>(100,250,300,350), false, true, NULL, -1, true);

            /*dbgText2 = guienv->addStaticText(L"",
                   rect<s32>(350,200,450,300), false, true, NULL, -1, true);*/
    }

    if (enableLightning) {
        mInfra->mSmgr->addLightSceneNode(0, vector3df(0, 100, 100),
            video::SColorf(1.0f, 1.0f, 1.0f), 1000.0f, -1);
    }

    //set a minimum amount of light everywhere, to not have black areas
    //in the level
    if (enableLightning) {
       mInfra->mSmgr->setAmbientLight(video::SColorf(0.4f, 0.4f, 0.4f));
    } else {
        //set max brightness everywhere
        mInfra->mSmgr->setAmbientLight(video::SColorf(1.0f, 1.0f, 1.0f));
    }

    if (enableShadows) {
       mInfra->mSmgr->setShadowColor(video::SColor(150,0,0,0));
    }

    return true;
}

//This function allows to quickly enter a race for
//game debugging, and to skip the menue etc.
void Game::DebugGame() {
    mDebugGame = true;

    int debugLevelNr = 1;

    //set craft for main player
    //value 0 means KD1 Speeder (default selection at first start)
    //value 1 means Berserker
    //value 2 means Jugga
    //value 3 means Vampyr
    //value 4 means Outrider
    //value 5 means Flexiwing
    mGameAssets->SetNewMainPlayerSelectedCraft(2);

    if (!runDemoMode) {
        //player wants to start the race
        mPilotsNextRace = mGameAssets->GetPilotInfoNextRace(true, false);

        if (this->CreateNewRace(debugLevelNr, mPilotsNextRace, false, mDebugGame)) {
             mGameState = DEF_GAMESTATE_RACE;
             CleanupPilotInfo(mPilotsNextRace);

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

void Game::AdvanceChampionship() {
    //if we have not reached the last available
    //level, advance championship forward
    //to next level
    irr::u8 currLevel = this->mGameAssets->GetLastSelectedRaceTrack();

    if (currLevel < (this->mGameAssets->mRaceTrackVec->size() - 1)) {
        //more levels available
        //continue
        this->mGameAssets->SetNewLastSelectedRaceTrack(currLevel + 1);

        //go back to the championship
        //menue
        MainMenue->ShowChampionshipMenue();
    } else {
        //no more levels available
        //return to top of main menue, as also
        //the original game does

        //at the same time set back to first level
        mGameAssets->SetNewLastSelectedRaceTrack(0);

        MainMenue->ShowMainMenue();
    }
}

void Game::AdvanceFrame(irr::s32 advanceFrameCount) {
    if (this->mTimeStopped) {
        mAdvanceFrameMode = true;
        mAdvanceFrameCnt = advanceFrameCount;
    }
}

void Game::HandleMenueActions() {
    //user triggered an menue action
    if (pendingAction == MainMenue->ActQuitToOS) {
        //make sure config.dat is updated
        mGameAssets->UpdateGameConfigFileExitGame();

        //user wants to quit the program
        ExitGame = true;
    }

    //this is how we handle a checkbox/slider value update
    if (pendingAction == MainMenue->ActSetDifficultyLevel) {
        //user wants to change difficulty level
        mGameAssets->SetGameDifficulty(pendingAction->currSetValue);
    }

    //this is how we handle a playername string change
    if (pendingAction == MainMenue->ActSetPlayerName) {
        //copy new selected main player name into player name array
        char* newStr = pendingAction->newSetTextInputString;
        mGameAssets->SetNewMainPlayerName(newStr);
    }

    if (pendingAction == MainMenue->ActSetMusicVolume) {
        //volumeMusic: 0 means no music, 100.0f means max volume
        //from the volume slider we get unsigned char between 0 up to 16 (convert!)
        irr::f32 volumeMusic = (float(pendingAction->currSetValue) / float(16)) * 100.0f;
        gameMusicPlayer->SetVolume(volumeMusic);

        mGameAssets->SetMusicVolume(volumeMusic);
    }

    if (pendingAction == MainMenue->ActSetSoundVolume) {
        //volumeSound: 0 means no sound, 100.0f means max volume
         //from the volume slider we get unsigned char between 0 up to 16 (convert!)
        irr::f32 volumeSound = (float(pendingAction->currSetValue) / float(16)) * 100.0f;
        gameSoundEngine->SetVolume(volumeSound);

        mGameAssets->SetSoundVolume(volumeSound);
    }

    if (pendingAction == MainMenue->ActSetComputerPlayerEnable) {
        //return value: currSetValue = 0: computer player off, 1: computer player on
        //important note: if computer player is on/off is not stored in config.dat file
        //in original game; game just resets to computer players on anytime you start game
        if (pendingAction->currSetValue == 0) {
            mGameAssets->SetComputerPlayersEnabled(false);
        } else {
            mGameAssets->SetComputerPlayersEnabled(true);
        }
    }

    if (pendingAction == MainMenue->ActRace) {
        //MainMenue->ShowGameLoadingScreen();

        //player wants to start the race
        mPilotsNextRace = mGameAssets->GetPilotInfoNextRace(true, mGameAssets->GetComputerPlayersEnabled());

        if (this->CreateNewRace(pendingAction->currSetValue, mPilotsNextRace, false, mDebugGame)) {
             mGameState = DEF_GAMESTATE_RACE;
             CleanupPilotInfo(mPilotsNextRace);
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

             //if we are in a championship right now, then
             //next step is too show the assigned/earned
             //points for each player for the last race
             if (MainMenue->mChampionshipMode) {
                 //get assigned points
                 lastRacePointTable = mGameAssets->GetLastRacePointsTable(lastRaceStat);
             }

             //cleanup race stat page and return back to
             //main menue

             //first cleanup data of race stats page
             MainMenue->CleanupRaceStatsPage();

             //do not forget to also cleanup last
             //racestat struct memory!
             mCurrentRace->CleanupRaceStatistics(lastRaceStat);
             lastRaceStat = NULL;

             mGameState = DEF_GAMESTATE_MENUE;

             if (!MainMenue->mChampionshipMode) {
                 //was just a normal single race
                 //go back to main menue top page
                 MainMenue->ShowMainMenue();
             } else {
                 //we are currently inside a championship

                 //first recalculate overall championship point
                 //table
                 lastOverallChampionshipPointTable =
                         mGameAssets->UpdateChampionshipOverallPointTable(lastRacePointTable);

                 //lets continue to show race point table
                 MainMenue->ShowPointsTablePage(lastRacePointTable, false);
             }
    }

    //this action will actually be triggered in two cases
    //if we have shown the user the race result point table, and
    //if we have shown the user the overall championship point table
    if (pendingAction == MainMenue->ActClosePointsTablePage) {
        //cleanup point table page text labels
        MainMenue->CleanupPointsTablePage();

        //if we showed the user currently the first race
        //point table, variable lastRacePointTable is not yet NULL
        //in this case, we free this memory, and the continue to show the
        //overall championship point table
        //if lastRacePointTable is already NULL we returned from showing
        //the overall championship point table; In this case we cleanup the
        //lastOverallChampionshipPointTable table, and then we return to the
        //current championship menue
        if (lastRacePointTable != NULL) {
          //we just showed the race point results
          //start to show the overall championship point table

          //also cleanup the variable lastRacePointTable
          CleanUpPointTable(*lastRacePointTable);

          lastRacePointTable = NULL;

          //now show the overall championship point table
          //Parameter needs to be set to true, so that the header text is correct!
          MainMenue->ShowPointsTablePage(lastOverallChampionshipPointTable, true);
        } else {
            //we returned from showing the overall championship point table
            //clean this table up
            CleanUpPointTable(*lastOverallChampionshipPointTable);

            lastOverallChampionshipPointTable = NULL;

            AdvanceChampionship();
        }
    }

    //user wants to enter championship menue?
    if (pendingAction == MainMenue->ActEnterChampionshipMenue) {
        MainMenue->ShowChampionshipMenue();
    }

    //user wants to continue the current championship?
    if (pendingAction == MainMenue->ActContinueChampionship) {
        MainMenue->ContinueChampionship();
    }

    //user wants to load a championship savegame?
    if (pendingAction == MainMenue->ActLoadChampionshipSlot) {
        //which slot we want to load is handed over
        //in parameter currSetValue inside the action
        //range of currSetValue is from 1 up to 5
        //function expects value 0 up to 4
        mGameAssets->LoadChampionshipSaveGame(pendingAction->currSetValue - 1);

        //after loading of the save game
        //go back to the main championship menue
        MainMenue->ShowChampionshipMenue();
    }

    //user wants to save the current championship state?
    if (pendingAction == MainMenue->ActSaveChampionshipSlot) {
        //which slot we want to save to is handed over
        //in parameter currSetValue inside the action
        //range of currSetValue is from 1 up to 5
        //function expects value 0 up to 4
        //store the slot number inside this member,
        //until we have a new championship name, and we can finally save
        mSaveChampionShipToWhichSlot = pendingAction->currSetValue - 1;

        //change the menue, so that the user can enter the new
        //championship name
        MainMenue->StartChampionshipNameInputAtSlot(mSaveChampionShipToWhichSlot);
    }

    //user finished entering new championship name at the end
    //of savegame saving process?
    if (pendingAction == MainMenue->ActFinalizeChampionshipSaveSlot) {
        //the entered new championship string is stored in action
        //variable newSetTextInputString
        mGameAssets->SetCurrentChampionshipName(pendingAction->newSetTextInputString);

        //finally create savegame file
        mGameAssets->SaveChampionshipSaveGame(mSaveChampionShipToWhichSlot);

        //now restore the menue item after championship
        //name input
        MainMenue->EndChampionshipNameInputAtSlot(mSaveChampionShipToWhichSlot);

        //after saving of the save game
        //go back to the main championship menue
        MainMenue->ShowChampionshipMenue();
    }

    //user wants to quit championship?
    if (pendingAction == MainMenue->ActQuitChampionship) {
        mGameAssets->QuitCurrentChampionship();

        //as in the original game go back to the
        //race menue
        MainMenue->ShowRaceMenue();
    }

    //show HighScorePage?
    if (pendingAction == MainMenue->ActShowHighScorePage) {
        MainMenue->ShowHighscore();
    }

    //start a demo?
    if (pendingAction == MainMenue->ActStartDemo) {
        //for the demo do not add a human player, but add computer players
        mPilotsNextRace = mGameAssets->GetPilotInfoNextRace(false, true);

        //pick a random level number for the demo
        int randLevelNr = this->mInfra->randRangeInt(1, 6);

        if (this->CreateNewRace(randLevelNr, mPilotsNextRace, true, mDebugGame)) {
             mGameState = DEF_GAMESTATE_RACE;
             CleanupPilotInfo(mPilotsNextRace);
        }
    }
}

void Game::CleanUpPointTable(std::vector<PointTableEntryStruct*> &tablePntr) {
    std::vector<PointTableEntryStruct*>::iterator it;
    PointTableEntryStruct* pntr;

    for (it = tablePntr.begin(); it != tablePntr.end(); ) {
        pntr = (*it);

        it = tablePntr.erase(it);

        //name of player was created using strdup function
        //we need to use free here!
        free(pntr->namePlayer);
    }

    //at the end also delete the vector
    delete &tablePntr;
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

    mInfra->mDriver->beginScene(true,true,
                video::SColor(255,100,101,140));

    MainMenue->HandleInput();
    MainMenue->Render(frameDeltaTime);

    //advance menues absolute time (necessary
    //to control menue animations) and to trigger
    //time dependent menue actions triggers (start game demo,
    //show high score)
    MainMenue->AdvanceTime(frameDeltaTime);
    MainMenue->HandleUserInactiveTimer(frameDeltaTime);

    if (MainMenue->HandleActions(pendingAction)) {
       //user triggered an menue action
       //handle it
       HandleMenueActions();
    }

    mInfra->mDriver->endScene();
}

void Game::GameLoopRace(irr::f32 frameDeltaTime) {

    mInfra->mTimeProfiler->StartOfGameLoop();

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

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntHandleInput);

    if (!this->mTimeStopped) {
        mCurrentRace->HandleComputerPlayers(frameDeltaTime);
    }

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntHandleComputerPlayers);

    if (DebugShowVariableBoxes) {

        wchar_t* text = new wchar_t[200];
        wchar_t* text2 = new wchar_t[400];

        mInfra->mTimeProfiler->GetTimeProfileResultDescending(text, 200, 5);

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

        /*   swprintf(text2, 390, L"%d %d %d %d %lf",
                       this->mCurrentRace->dbglocMovingWindow.UpperLeftCorner.X,
                       this->mCurrentRace->dbglocMovingWindow.UpperLeftCorner.Y,
                       this->mCurrentRace->dbglocMovingWindow.LowerRightCorner.X,
                       this->mCurrentRace->dbglocMovingWindow.LowerRightCorner.Y, mCurrentRace->currPlayerFollow->mCurrentAvgPlayerLeaningAngleLeftRightValue);*/

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

    mInfra->mDriver->beginScene(true,true,
     video::SColor(255,100,101,140));

    //render scene: terrain, blocks, player craft, entities...
    mCurrentRace->Render();

    mInfra->mSmgr->drawAll();

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntRender3DScene);

    //2nd draw HUD over the scene, needs to be done at the end
    mCurrentRace->DrawHUD(frameDeltaTime);
    mCurrentRace->DrawMiniMap(frameDeltaTime);

    //render log window
    mInfra->mLogger->Render();

    //last draw text debug output from Irrlicht
    mInfra->mGuienv->drawAll();

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntRender2D);

    mInfra->mDriver->endScene();

    //does the player want to end the race?
    if (mCurrentRace->exitRace) {
        mCurrentRace->End();

        this->lastRaceStat = NULL;

        //was the race finished, that means all players went through the finish
        //line and finished the last lap?
        if (mCurrentRace->GetWasRaceFinished()) {
            //yes it was, get race statistics
            this->lastRaceStat = mCurrentRace->RetrieveFinalRaceStatistics();
        }

        //clean up current race data
        delete mCurrentRace;
        mCurrentRace = NULL;

        if (mDebugGame) {
            ExitGame = true;
        } else {
            mGameState = DEF_GAMESTATE_MENUE;

            if (this->lastRaceStat != NULL) {
                //Show race statistics
                MainMenue->ShowRaceStats(lastRaceStat);
            } else {
                //there are no race statistics to show
                //simply return to the main menue
                MainMenue->ShowMainMenue();
            }
        }
    }
}

/* Fade In and Out example
 * //from https://github.com/XadillaX/irr-guide-examples/blob/master/Chapter%202/FadeInNFadeOut/main.cpp
 *
 *  f32 bg_r = 255.0f;
    f32 bg_g = 255.0f;
    f32 bg_b = 255.0f;
    bool fadeOut = true;
    int lastFPS = -1;

    u32 then = device->getTimer()->getTime();
    const f32 fadeRate = 0.1f;

    while(device->run())
    {
        const u32 now = device->getTimer()->getTime();
        const f32 frameDeltaTime = (f32)(now - then);
        then = now;

        if(bg_r <= 0.0f) fadeOut = false;
        else
        if(bg_r >= 255.0f) fadeOut = true;

        if(fadeOut)
        {
            bg_r -= fadeRate * frameDeltaTime;
            bg_g -= fadeRate * frameDeltaTime;
            bg_b -= fadeRate * frameDeltaTime;
        }
        else
        {
            bg_r += fadeRate * frameDeltaTime;
            bg_g += fadeRate * frameDeltaTime;
            bg_b += fadeRate * frameDeltaTime;
        }

        driver->beginScene(true, true, SColor(255, bg_r, bg_g, bg_b));
        driver->endScene();
    }

    */

void Game::GameLoop() {

    // In order to do framerate independent movement, we have to know
    // how long it was since the last frame
    u32 then = mInfra->mDevice->getTimer()->getTime();

    while (mInfra->mDevice->run() && (ExitGame == false)) {
       /*  if (device->isWindowActive())
            {*/

        //Work out a frame delta time.
        const u32 now = mInfra->mDevice->getTimer()->getTime();
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

        int fps = mInfra->mDriver->getFPS();

        if (lastFPS != fps) {
                         core::stringw tmp(L"Hi-Octane 202X [");
                         tmp += mInfra->mDriver->getName();
                         tmp += L"] Triangles drawn: ";
                         tmp += mInfra->mDriver->getPrimitiveCountDrawn();
                         tmp += " @ fps: ";
                         tmp += fps;

                         mInfra->mDevice->setWindowCaption(tmp.c_str());
                         lastFPS = fps;
                     }
            //}
   }

   //cleanup game assets
   delete mGameAssets;

   //delete sound/music
   delete gameSoundEngine;
   delete gameMusicPlayer;

   mInfra->mDriver->drop();

   //cleanup the infrastructure
   delete mInfra;
}

void Game::CleanupPilotInfo(std::vector<PilotInfoStruct*> &pilotInfo) {
    std::vector<PilotInfoStruct*>::iterator itPilot;
    PilotInfoStruct* pntr;

    //cleanup pilotInfo, we do not need it anymore
    for (itPilot = pilotInfo.begin(); itPilot != pilotInfo.end(); ) {
       pntr = (*itPilot);

       itPilot = pilotInfo.erase(itPilot);

       delete pntr;
    }
}

bool Game::CreateNewRace(int load_levelnr, std::vector<PilotInfoStruct*> pilotInfo, bool demoMode, bool debugRace) {
    if (mCurrentRace != NULL)
        return false;

    //create a new Race
    mCurrentRace = new Race(mInfra, this, gameMusicPlayer, gameSoundEngine, load_levelnr,
                            mGameAssets->mRaceTrackVec->at(load_levelnr-1)->currSelNrLaps, demoMode, debugRace, false);

    mCurrentRace->Init();

    //now add players according to pilotInfo
    std::vector<PilotInfoStruct*>::iterator itPilot;
    std::string modelName;

    for (itPilot = pilotInfo.begin(); itPilot != pilotInfo.end(); ++itPilot) {
        modelName = mGameAssets->GetCraftModelName((*itPilot)->defaultCraftName, (*itPilot)->currSelectedCraftColorScheme);

        //if there was a problem modelName is an empty string
        if (modelName == "")
            return false;

        //finally add the player to the race
        mCurrentRace->AddPlayer((*itPilot)->humanPlayer, (*itPilot)->pilotName, modelName);
    }

    //which player do we want to follow at the start
    //of the race
    mCurrentRace->DebugSelectPlayer(0);

    if (!mCurrentRace->ready) {
        //there was a problem with Race initialization
        cout << "Race creation failed!" << endl;
        return false;
    }

    return true;
}

bool Game::RunDemoMode(int load_levelnr) {
    if (mCurrentRace != NULL)
        return false;

    //gameSoundEngine->StartEngineSound();

    //create a new Race in Demo Mode
    mCurrentRace = new Race(mInfra, this, gameMusicPlayer, gameSoundEngine, load_levelnr,
                            mGameAssets->mRaceTrackVec->at(load_levelnr-1)->defaultNrLaps, true, false);

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
