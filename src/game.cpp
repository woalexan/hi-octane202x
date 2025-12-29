/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "game.h"
#include "audio/music.h"
#include "resources/assets.h"
#include "draw/menue.h"
#include "draw/introplayer.h"
#include "utils/logger.h"
#include "audio/sound.h"
#include "definitions.h"
#include "draw/gametext.h"
#include "race.h"
#include "utils/tprofile.h"

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
    gameMusicPlayer = new MyMusicStream(this, audioSampleRate);
    if (!gameMusicPlayer->getInitOk()) {
        logging::Error("Music init failed!");
        return false;
    }

    //volumeMusic: 0 means no music, 100.0f means max volume
    //get configured volume from Assets class
    gameMusicPlayer->SetVolume(mGameAssets->GetMusicVolume());

    gameSoundEngine = new SoundEngine(this);

    gameSoundEngine->SetVolume(mGameAssets->GetSoundVolume());

    return true;
}

bool Game::LoadGameData() {
    //load all sound resource files
    gameSoundEngine->LoadSoundResources();

    if (!gameSoundEngine->getSoundResourcesLoadOk()) {
        logging::Error("Sound resource init failed!"); 
        return false;
    }

    //load all remaining game font
    mGameTexts->LoadFontsStep2();

    if (!mGameTexts->GameTextInitializedOk) {
        logging::Error("Second game font init operation failed!");
        return false;
    }

    //create the game menue
    MainMenue = new Menue(this, gameSoundEngine, mGameAssets);
    if (!MainMenue->MenueInitializationSuccess) {
        logging::Error("Game menue init operation failed!");
        return false;
    }

    return true;
}

//fully initializes the remaining game
//components
//before calling this function we need to be sure that all
//original game files were already extracted before and are
//available
bool Game::InitGameStep2() {
    /***********************************************************/
    /* Load and define game assets (Meshes, Tracks, Craft)     */
    /***********************************************************/
    mGameAssets = new Assets(this, keepConfigDataFileUpdated);

    //InitSFMLAudio needs to be called after
    //InitGameResources, because it needs the
    //assets already unpacked to load sounds etc..
    if (!InitSFMLAudio())
        return false;

    logging::Info("Audio init ok");

    if (DebugShowVariableBoxes) {
            //only for debugging
            dbgTimeProfiler = mGuienv->addStaticText(L"Location",
                   rect<s32>(100,150,300,200), false, true, nullptr, -1, true);

            dbgText = mGuienv->addStaticText(L"",
                   rect<s32>(100,250,300,350), false, true, nullptr, -1, true);

            /*dbgText2 = guienv->addStaticText(L"",
                   rect<s32>(350,200,450,300), false, true, nullptr, -1, true);*/
    }

    if (enableLightning) {
        mSmgr->addLightSceneNode(0, vector3df(0, 100, 100),
            video::SColorf(1.0f, 1.0f, 1.0f), 1000.0f, -1);
    }

    //set a minimum amount of light everywhere, to not have black areas
    //in the level
    if (enableLightning) {
       mSmgr->setAmbientLight(video::SColorf(0.4f, 0.4f, 0.4f));
    } else {
        //set max brightness everywhere
        mSmgr->setAmbientLight(video::SColorf(1.0f, 1.0f, 1.0f));
    }

    return true;
}

//First initialization step that is only important
//for the game, setup XEffects if needed, and
//load first resources
bool Game::InitGameStep1() {
   mUseXEffects = false;

   //if we use shadows we need to enable XEffects
   if (mGameConfig->enableShadows) {
       mUseXEffects = true;
       logging::Info("XEffects is turned on");

       if (!mGameConfig->useUpgradedSky) {
            //we need to use the upgraded sky, because the vanilla
            //sky does not work together with XEffects enabled
            mGameConfig->useUpgradedSky = true;

            logging::Warning("Switch to use upgraded sky because vanilla sky is not compatible to using XEffects");
       }
   }

   if (!mUseXEffects) {
       logging::Info("XEffects is not used");
   }

    //load the background image we need
    //for data extraction screen rendering and
    //main menue
    if (!LoadBackgroundImage()) {
        return false;
    }

    return true;
}

//This function allows to quickly enter a race for
//game debugging, and to skip the menue etc.
void Game::SetupDebugGame() {

    //which level should be directly entered?
    nextRaceLevelNr = 1;

    //set craft for main player
    //value 0 means KD1 Speeder (default selection at first start)
    //value 1 means Berserker
    //value 2 means Jugga
    //value 3 means Vampyr
    //value 4 means Outrider
    //value 5 means Flexiwing
    mGameAssets->SetNewMainPlayerSelectedCraft(0);

    //add computer players?
    mGameAssets->SetComputerPlayersEnabled(false);

    mGameState = DEF_GAMESTATE_INITRACE;
}

//This function allows to quickly enter a demo race for
//demo debugging, and to skip the menue etc.
void Game::SetupDebugDemo() {

    //which level should be directly entered
    //during demo mode
    nextRaceLevelNr = 1;

    //in demo mode computer players need to be enabled!
    mGameAssets->SetComputerPlayersEnabled(false);

    mGameState = DEF_GAMESTATE_INITDEMO;
}

void Game::RunGame() {
    //further game data needs to be extracted?
    if (!mPrepareData->GameDataAvailable()) {
        mGameState = DEF_GAMESTATE_EXTRACTDATA;
    } else {
        mGameState = DEF_GAMESTATE_INITSTEP2;
    }

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
        MainMenue->ShowChampionshipMenue(true);
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
        //next level number selected in race track selection page
        //at the end is handed over in pendingAction->currSetValue;
        nextRaceLevelNr = pendingAction->currSetValue;
        mGameState = DEF_GAMESTATE_INITRACE;
    }

    //take care of the special menue actions

    //is game intro playing finished or was it interrupted?
    /*if (pendingAction == MainMenue->ActIntroStop) {
             //yes, change to game title screen
             mGameState = DEF_GAMESTATE_GAMETITLE;
             MainMenue->ShowGameTitle();
    }*/

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
             lastRaceStat = nullptr;

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
        //point table, variable lastRacePointTable is not yet nullptr
        //in this case, we free this memory, and the continue to show the
        //overall championship point table
        //if lastRacePointTable is already nullptr we returned from showing
        //the overall championship point table; In this case we cleanup the
        //lastOverallChampionshipPointTable table, and then we return to the
        //current championship menue
        if (lastRacePointTable != nullptr) {
          //we just showed the race point results
          //start to show the overall championship point table

          //also cleanup the variable lastRacePointTable
          CleanUpPointTable(*lastRacePointTable);

          lastRacePointTable = nullptr;

          //now show the overall championship point table
          //Parameter needs to be set to true, so that the header text is correct!
          MainMenue->ShowPointsTablePage(lastOverallChampionshipPointTable, true);
        } else {
            //we returned from showing the overall championship point table
            //clean this table up
            CleanUpPointTable(*lastOverallChampionshipPointTable);

            lastOverallChampionshipPointTable = nullptr;

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
        irr::u8 slotNr = MainMenue->GetChampionShipSlotNrForLoadAction(pendingAction->currSetValue);

        //function expects value 0 up to 4
        mGameAssets->LoadChampionshipSaveGame(slotNr);

        //after loading of the save game
        //go back to the main championship menue
        MainMenue->ShowChampionshipMenue(true);
    }

    //user wants to save the current championship state?
    if (pendingAction == MainMenue->ActSaveChampionshipSlot) {
        //which slot we want to save to is handed over
        //in parameter currSetValue inside the action
        mSaveChampionShipToWhichSlot = MainMenue->GetChampionShipSlotNrForSaveAction(pendingAction->currSetValue);

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
        //pick a random level number for the demo
        nextRaceLevelNr = randRangeInt(1, (int)(mGameAssets->mRaceTrackVec->size()));

        mGameState = DEF_GAMESTATE_INITDEMO;
    }

    if (pendingAction == MainMenue->ActSetDoubleResolution) {
        if (pendingAction->currSetValue == 0) {
            mGameConfig->enableDoubleResolution = false;
        } else {
            mGameConfig->enableDoubleResolution = true;
        }
    }

    if (pendingAction == MainMenue->ActSetVSync) {
        if (pendingAction->currSetValue == 0) {
            mGameConfig->enableVSync = false;
        } else {
            mGameConfig->enableVSync = true;
        }
    }

    if (pendingAction == MainMenue->ActSetEnableShadows) {
        if (pendingAction->currSetValue == 0) {
            mGameConfig->enableShadows = false;
        } else {
            mGameConfig->enableShadows = true;
        }
    }

    if (pendingAction == MainMenue->ActSetUpgradedSky) {
        if (pendingAction->currSetValue == 0) {
            mGameConfig->useUpgradedSky = false;
        } else {
            mGameConfig->useUpgradedSky = true;
        }
    }

    if (pendingAction == MainMenue->ActSkipIntro) {
        if (pendingAction->currSetValue == 0) {
            mGameConfig->skipIntro = false;
        } else {
            mGameConfig->skipIntro = true;
        }

        //write the new updated configuration
        logging::Info("Updated configuration in game config Xml file");
        WriteGameConfigXmlFile(mDevice);
    }

    if (pendingAction == MainMenue->ActReturnFromDetailsMenue) {
        //write the new updated configuration
        logging::Info("Updated configuration in game config Xml file");
        WriteGameConfigXmlFile(mDevice);
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

bool Game::LoadAdditionalGameImages() {
     mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //load gameTitle
     if (!mGameConfig->enableDoubleResolution) {
        //load the image that was scaled by a factor of 2x before
        gameTitle = mDriver->getTexture("extract/images/title.png");
     } else {
        //load the image that was scaled by a factor of 4x before
        gameTitle = mDriver->getTexture("extract/images/title-x2.png");
     }

     if (gameTitle == nullptr) {
         //there was a texture loading error
         //just return with false
         return false;
     }

     gameTitleSize = gameTitle->getSize();
     //calculate position to draw gameTitle so that it is centered on the screen
     //because most likely target resolution does not fit with image resolution
     gameTitleDrawPos.X = (mScreenRes.Width - gameTitleSize.Width) / 2;
     gameTitleDrawPos.Y = (mScreenRes.Height - gameTitleSize.Height) / 2;

     //load race loading screen
     if (!mGameConfig->enableDoubleResolution) {
        raceLoadingScr = mDriver->getTexture("extract/images/onet0-1.png");
     } else {
        //load the image that was scaled by a factor of 2x before
        raceLoadingScr = mDriver->getTexture("extract/images/onet0-1-x2.png");
     }

     if (raceLoadingScr == nullptr) {
         //there was a texture loading error
         //just return with false
         return false;
     }

     raceLoadingScrSize = raceLoadingScr->getSize();
     //calculate position to draw race loading screen so that it is centered on the screen
     //because maybe target resolution does not fit with image resolution
     raceLoadingScrDrawPos.X = (mScreenRes.Width - raceLoadingScrSize.Width) / 2;
     raceLoadingScrDrawPos.Y = (mScreenRes.Height - raceLoadingScrSize.Height) / 2;

     mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

     return true;
}

bool Game::LoadBackgroundImage() {
    mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    //first load background image for menue
    if (!mGameConfig->enableDoubleResolution) {
        backgnd = mDriver->getTexture("extract/images/oscr0-1.png");
    } else {
        //load the image which was 2x scaled before
        backgnd = mDriver->getTexture("extract/images/oscr0-1-x2.png");
    }

    if (backgnd == nullptr) {
        //there was a texture loading error
        //just return with false
        return false;
    }

    irr::core::dimension2d<irr::u32> backgndSize;

    backgndSize = backgnd->getSize();
    if ((backgndSize.Width != mScreenRes.Width) ||
        (backgndSize.Height != mScreenRes.Height)) {
        //background texture size does not fit with selected screen resolution
        return false;
    }

    mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}

void Game::RenderDataExtractionScreen() {
    //first draw background picture
     mDriver->draw2DImage(backgnd, irr::core::vector2di(0, 0),
                         irr::core::recti(0, 0, mScreenRes.Width, mScreenRes.Height)
                         , 0, irr::video::SColor(255,255,255,255), true);

     char* infoText = strdup("EXTRACTING GAME DATA, PLEASE STANDBY...");
     char* currStepText = strdup(mPrepareData->currentStepDescription.c_str());

     irr::u32 textHeight = mGameTexts->GetHeightPixelsGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA);
     irr::u32 textWidth = mGameTexts->GetWidthPixelsGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA);

     irr::core::position2di txtPos;
     txtPos.X = mScreenRes.Width / 2 - textWidth / 2;
     txtPos.Y = mScreenRes.Height / 2 - textHeight / 2;

     //write info text, warning: at the data extraction stage of the game only single font GameMenueWhiteTextSmallSVGA is
     //available!
     mGameTexts->DrawGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA, txtPos);

     textHeight = mGameTexts->GetHeightPixelsGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA);
     textWidth = mGameTexts->GetWidthPixelsGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA);

     txtPos.X = mScreenRes.Width / 2 - textWidth / 2;
     txtPos.Y += 25;

     mGameTexts->DrawGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA, txtPos);

     free(infoText);
     free(currStepText);
}

void Game::GameLoopExtractData() {
    //execute next data extraction step
    //returns true if data extraction process is finished
    try {
        if (mPrepareData->ExecuteNextStep()) {
            //continue initialization step2 (Audio, ...)
            mGameState = DEF_GAMESTATE_INITSTEP2;
            return;
        }
    }
     catch (const std::string &msg) {
         std::string msgExt("Step 2 of game assets preparation operation failed: ");
         msgExt.append(msg);
         logging::Error(msgExt);
         mGameState = DEF_GAMESTATE_ERROR;
         return;
    }

    mDriver->beginScene(true,true,
    video::SColor(255,100,101,140));

    //update graphical image about current status
    RenderDataExtractionScreen();

    mDriver->endScene();
}

void Game::GameLoopTitleScreenLoadData() {
    //we need to load additional images
    if (!LoadAdditionalGameImages()) {
        logging::Error("Loading of game tile and race loading images failed");
        mGameState = DEF_GAMESTATE_ERROR;
        return;
    }

    mDriver->beginScene(true,true,
    video::SColor(255,100,101,140));

    //first draw a black rectangle over the whole screen to make sure that the parts of the
    //screen that are outside of the drawn image regions are black as well
    mDriver->draw2DRectangle(irr::video::SColor(255,0,0,0),
                   irr::core::rect<irr::s32>(0, 0, mScreenRes.Width, mScreenRes.Height));

    //draw game tile screen
    mDriver->draw2DImage(gameTitle, gameTitleDrawPos, irr::core::recti(0, 0,
                     gameTitleSize.Width, gameTitleSize.Height)
                     , 0, irr::video::SColor(255,255,255,255), true);

    mDriver->endScene();

    //now load data
    if (!LoadGameData()) {
        mGameState = DEF_GAMESTATE_ERROR;
        return;
    } else {
        if (!mDebugRace && !mDebugDemoMode) {
            if (mTestMapMode) {
                //set craft for main player
                //value 0 means KD1 Speeder (default selection at first start)
                //value 1 means Berserker
                //value 2 means Jugga
                //value 3 means Vampyr
                //value 4 means Outrider
                //value 5 means Flexiwing
                mGameAssets->SetNewMainPlayerSelectedCraft(0);

                //add computer players?
                mGameAssets->SetComputerPlayersEnabled(true);

                mGameState = DEF_GAMESTATE_INITRACE;
                return;
            }

            //was succesfull, now continue to main menue
            mGameState = DEF_GAMESTATE_MENUE;
            MainMenue->ShowMainMenue();
        } else if (mDebugRace) {
            //we want to directly create a race for debugging
            //of game mechanics and enter it
            SetupDebugGame();
        } else if (mDebugDemoMode) {
            //we want to directly create a demo for debugging
            SetupDebugDemo();
        }
    }
}

void Game::GameLoopLoadRaceScreen() {
    mDriver->beginScene(true,true,
    video::SColor(255,100,101,140));

    //first draw a black rectangle over the whole screen to make sure that the parts of the
    //screen that are outside of the drawn image regions are black as well
    mDriver->draw2DRectangle(irr::video::SColor(255,0,0,0),
                   irr::core::rect<irr::s32>(0, 0, mScreenRes.Width, mScreenRes.Height));

    //draw load race screen
    mDriver->draw2DImage(raceLoadingScr, raceLoadingScrDrawPos, irr::core::recti(0, 0, raceLoadingScrSize.Width, raceLoadingScrSize.Height)
                     , 0, irr::video::SColor(255,255,255,255), true);

    char loadingTxt[25];
    strcpy(loadingTxt, "LOADING LEVEL");

    irr::u32 txtWidth = mGameTexts->GetWidthPixelsGameText(loadingTxt, mGameTexts->HudWhiteTextBannerFont);
    irr::u32 txtHeight = mGameTexts->GetHeightPixelsGameText(loadingTxt, mGameTexts->HudWhiteTextBannerFont);

    irr::core::position2di txtDrawPos;
    txtDrawPos.X = mScreenRes.Width / 2 - txtWidth / 2;
    txtDrawPos.Y = mScreenRes.Height / 2 - txtHeight / 2;

    mGameTexts->DrawGameText(loadingTxt, mGameTexts->HudWhiteTextBannerFont, txtDrawPos);

    mDriver->endScene();

    if (mGameState == DEF_GAMESTATE_INITRACE) {
        if (!mTestMapMode) {
            //Default game mode

            //player wants to start the race
            mPilotsNextRace = mGameAssets->GetPilotInfoNextRace(true, mGameAssets->GetComputerPlayersEnabled());

            if (this->CreateNewRace(nextRaceLevelNr, mPilotsNextRace, mGameAssets->mRaceTrackVec->at(nextRaceLevelNr-1)->currSelNrLaps,
                                    false, mDebugRace)) {
                 mGameState = DEF_GAMESTATE_RACE;
                 CleanupPilotInfo(mPilotsNextRace);
            } else {
                CleanupPilotInfo(mPilotsNextRace);

                mGameState = DEF_GAMESTATE_MENUE;

                //there was an error while creating the race
                //Go back to top of main menue
                MainMenue->ShowMainMenue();
            }
        } else {
            //Test map mode via Command-line

            //let the command line parameters define if computer players are added or not
            mPilotsNextRace = mGameAssets->GetPilotInfoNextRace(true, !mTestMapModeNoCpuPlayers);

            if (this->CreateNewRace(mTestTargetLevel, mPilotsNextRace, 10, false, mDebugRace)) {
                 mGameState = DEF_GAMESTATE_RACE;
                 CleanupPilotInfo(mPilotsNextRace);
            } else {
                CleanupPilotInfo(mPilotsNextRace);

                mGameState = DEF_GAMESTATE_MENUE;

                //there was an error while creating the race
                //Go back to top of main menue
                MainMenue->ShowMainMenue();
            }
        }
    } else if (mGameState == DEF_GAMESTATE_INITDEMO) {
        //for the demo do not add a human player, but add computer players
        mPilotsNextRace = mGameAssets->GetPilotInfoNextRace(false, true);

        if (this->CreateNewRace(nextRaceLevelNr, mPilotsNextRace, mGameAssets->mRaceTrackVec->at(nextRaceLevelNr-1)->currSelNrLaps,
                                true, mDebugRace)) {
             mGameState = DEF_GAMESTATE_RACE;
             CleanupPilotInfo(mPilotsNextRace);
        } else {
            CleanupPilotInfo(mPilotsNextRace);

            mGameState = DEF_GAMESTATE_MENUE;

            //there was an error while creating the race
            //Go back to top of main menue
            MainMenue->ShowMainMenue();
        }
    }
}

void Game::GameLoopMenue(irr::f32 frameDeltaTime) {
    mDriver->beginScene(true,true,
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

    mDriver->endScene();
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
        mCurrentRace->HandleInput(frameDeltaTime);
    }

    mTimeProfiler->Profile(mTimeProfiler->tIntHandleInput);

    if (!this->mTimeStopped) {
        mCurrentRace->HandleComputerPlayers(frameDeltaTime);
    }

    mTimeProfiler->Profile(mTimeProfiler->tIntHandleComputerPlayers);

    //Update Time Profiler results
    mTimeProfiler->UpdateWindow();

    if (DebugShowVariableBoxes) {

        wchar_t* text2 = new wchar_t[400];

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

         /* swprintf(text2, 390, L"currDist = %lf\ncurrAngle = %lf\ncurrDist = %lf\ncurrAngle = %lf\n",
                       this->mCurrentRace->mPlayerVec.at(0)->mCurrentCraftDistToWaypointLink,
                        this->mCurrentRace->mPlayerVec.at(0)->mAngleError,
                       this->mCurrentRace->mPlayerVec.at(1)->mCpuPlayer->mCurrentCraftDistToWaypointLink,
                       this->mCurrentRace->mPlayerVec.at(1)->mCpuPlayer->mAngleError);*/

        /*    swprintf(text2, 390, L"currCharger = %d\currPos = %d\n",
                           this->mCurrentRace->currCharger,
                           this->mCurrentRace->currPos);*/

       /* swprintf(text2, 390, L"next Checkpoint = %d\n",
                                 this->mCurrentRace->mPlayerVec.at(0)->nextCheckPointValue);*/

           /* swprintf(text2, 390, L"nAvailWay = %d\n nAvailLinks = %d\n nCurrSeg = %d\nSegments = %d\n",
                           this->mCurrentRace->player2->mDbgCpAvailWaypointNr,
                           this->mCurrentRace->player2->mDbgCpAvailWayPointLinksNr,
                         this->mCurrentRace->player2->mCurrentPathSegCurrSegmentNr,
                          this->mCurrentRace->player2->mCurrentPathSegNrSegments);*/

            /*swprintf(text2, 390, L"nCollectable = %d\n",
                           this->mCurrentRace->player2->mCpCollectablesSeenByPlayer.size());*/

              /*   swprintf(text2, 390, L"DistSQ = %lf\n",
                                this->mCurrentRace->mPlayerVec.at(1)->mCpuPlayer->dbgDistVec);*/

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

       /* swprintf(text2, 390, L"Rot: %lf \n MoveXFloat %lf\n MoveXInt %d \n Wnd: %d %d %d %d\n %d %d \n", this->mCurrentRace->dbgSkyRotation, this->mCurrentRace->dbgSkyMoveXfloat,
                 this->mCurrentRace->dbgSkyMoveXInt, this->mCurrentRace->dbgSkyMovingWindow.UpperLeftCorner.X,
                 this->mCurrentRace->dbgSkyMovingWindow.UpperLeftCorner.Y, this->mCurrentRace->dbgSkyMovingWindow.LowerRightCorner.X,
                 this->mCurrentRace->dbgSkyMovingWindow.LowerRightCorner.Y, this->mCurrentRace->dbgSkyMiddlePos.X, this->mCurrentRace->dbgSkyMiddlePos.Y);*/

        /*swprintf(text2, 390, L"Vert1: %lf %lf %lf \n Vert2: %lf %lf %lf \n Vert3: %lf %lf %lf \n Vert4: %lf %lf %lf \n", mCurrentRace->dbgSkyVertice1.Pos.X,
                 mCurrentRace->dbgSkyVertice1.Pos.Y, mCurrentRace->dbgSkyVertice1.Pos.Z, mCurrentRace->dbgSkyVertice2.Pos.X,
                 mCurrentRace->dbgSkyVertice2.Pos.Y, mCurrentRace->dbgSkyVertice2.Pos.Z,
                 mCurrentRace->dbgSkyVertice3.Pos.X,
                                  mCurrentRace->dbgSkyVertice3.Pos.Y, mCurrentRace->dbgSkyVertice3.Pos.Z,
                 mCurrentRace->dbgSkyVertice4.Pos.X,
                 mCurrentRace->dbgSkyVertice4.Pos.Y, mCurrentRace->dbgSkyVertice4.Pos.Z);*/

        /*swprintf(text2, 390, L"UV1: %lf %lf \n UV2: %lf %lf \n UV3: %lf %lf \n UV4: %lf %lf \n", mCurrentRace->dbgSkyVertice1.TCoords.X,
                 mCurrentRace->dbgSkyVertice1.TCoords.X, mCurrentRace->dbgSkyVertice2.TCoords.X, mCurrentRace->dbgSkyVertice2.TCoords.Y,
                 mCurrentRace->dbgSkyVertice3.TCoords.X, mCurrentRace->dbgSkyVertice3.TCoords.Y,
                 mCurrentRace->dbgSkyVertice4.TCoords.X, mCurrentRace->dbgSkyVertice4.TCoords.Y);*/

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

            dbgText->setText(text2);

            delete[] text2;
    }

    mDriver->beginScene(true,true,
     video::SColor(255,100,101,140));

    //render scene: terrain, blocks, player craft, entities...
    mCurrentRace->Render();

    //if we do not use XEffects, we run the normal
    //Irrlicht drawAll command, otherwise we run XEffects update
    if (!mUseXEffects) {
        mSmgr->drawAll();
    } else {
        // EffectHandler->update() replaces smgr->drawAll(). It handles all
        // of the shadow maps, render targets switching, post processing, etc.
        mEffect->update();
    }

    mTimeProfiler->Profile(mTimeProfiler->tIntRender3DScene);

    //2nd draw HUD over the scene, needs to be done at the end
    mCurrentRace->DrawHUD(frameDeltaTime);
    mCurrentRace->DrawMiniMap(frameDeltaTime);

    //render log window
    mLogger->Render();

    //last draw text debug output from Irrlicht
    mGuienv->drawAll();

    mTimeProfiler->Profile(mTimeProfiler->tIntRender2D);

    mDriver->endScene();

    //does the player want to end the race?
    if (mCurrentRace->exitRace) {
        mCurrentRace->End();

        this->lastRaceStat = nullptr;

        //was the race finished, that means all players went through the finish
        //line and finished the last lap?
        if (mCurrentRace->GetWasRaceFinished()) {
            //yes it was, get race statistics
            this->lastRaceStat = mCurrentRace->RetrieveFinalRaceStatistics();
        }

        //clean up current race data
        delete mCurrentRace;
        mCurrentRace = nullptr;

        //cleanup XEffects if used
        if (mUseXEffects) {
            delete mEffect;
            mEffect = nullptr;
        }

        //if we were in game debugging mode or map test mode simply skip
        //main menue, and exit game immediately
        if (mDebugRace || mDebugDemoMode || mTestMapMode) {
            ExitGame = true;
        } else {
            mGameState = DEF_GAMESTATE_MENUE;

            if (this->lastRaceStat != nullptr) {
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

void Game::GameLoopIntro(irr::f32 frameDeltaTime) {
    //if we want to skip the intro, do it now
    if (mGameConfig->skipIntro) {
        mGameState = DEF_GAMESTATE_GAMETITLE;
        return;
    }

    //we want to play the intro
    //first time we need to initially the
    //intro player
    if (gameIntroPlayer == nullptr) {
        gameIntroPlayer = new IntroPlayer(this, gameSoundEngine, gameMusicPlayer);
        if (!gameIntroPlayer->Init()) {
            //init of intro failed => skip intro
            mGameState = DEF_GAMESTATE_GAMETITLE;
            return;
        }

        //start playing intro
        gameIntroPlayer->Play();
    } else {
            mDriver->beginScene(true,true,
                        video::SColor(255,100,101,140));

            if (gameIntroPlayer->introPlaying) {
                    gameIntroPlayer->RenderIntro(frameDeltaTime);
                    gameIntroPlayer->HandleInput();
            }

            mDriver->endScene();

            if (gameIntroPlayer->introFinished) {
                    //intro playing finished
                    //delete introplayer again
                    delete gameIntroPlayer;

                    mGameState = DEF_GAMESTATE_GAMETITLE;
            }
    }
}

void Game::GameLoopInitStep2() {
    if (!InitGameStep2()) {
        mGameState = DEF_GAMESTATE_ERROR;
        return;
    } else {
        if (!ParseCommandLineForGame()) {
           mGameState = DEF_GAMESTATE_ERROR;
           return;
        }

        mGameState = DEF_GAMESTATE_INTRO;
    }
}

void Game::GameLoop() {

    // In order to do framerate independent movement, we have to know
    // how long it was since the last frame
    u32 then = mDevice->getTimer()->getTime();

    while (mDevice->run() && (ExitGame == false)) {
       /*  if (device->isWindowActive())
            {*/

        //Work out a frame delta time.
        const u32 now = mDevice->getTimer()->getTime();
        f32 frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
        then = now;

        switch (mGameState) {
            case DEF_GAMESTATE_EXTRACTDATA: {
                GameLoopExtractData();
                break;
            }

            case DEF_GAMESTATE_INITSTEP2: {
               GameLoopInitStep2();
               break;
            }

            case DEF_GAMESTATE_INTRO: {
                GameLoopIntro(frameDeltaTime);
                break;
            }

            //shows game title, loads game data
            case DEF_GAMESTATE_GAMETITLE: {
                   GameLoopTitleScreenLoadData();
                   break;
            }
            case DEF_GAMESTATE_MENUE: {
                GameLoopMenue(frameDeltaTime);
                break;
            }

            case DEF_GAMESTATE_INITDEMO:
            case DEF_GAMESTATE_INITRACE: {
                   GameLoopLoadRaceScreen();
                   break;
            }

            case DEF_GAMESTATE_DEMORACE:
            case DEF_GAMESTATE_RACE: {
                GameLoopRace(frameDeltaTime);
                break;
            }

            case DEF_GAMESTATE_ERROR: {
                //there was an error, exit game
                ExitGame = true;
                break;
            }
        }

        int fps = mDriver->getFPS();

        if (lastFPS != fps) {
                         core::stringw tmp(L"Hi-Octane 202X [");
                         tmp += mDriver->getName();
                         tmp += L"] Triangles drawn: ";
                         tmp += mDriver->getPrimitiveCountDrawn();
                         tmp += " @ fps: ";
                         tmp += fps;

                         mDevice->setWindowCaption(tmp.c_str());
                         lastFPS = fps;
                     }
            //}
   }

   //cleanup game assets
   delete mGameAssets;

   //delete sound/music
   delete gameSoundEngine;
   delete gameMusicPlayer;

   mDriver->drop();
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

bool Game::CreateNewRace(std::string targetLevel, std::vector<PilotInfoStruct*> pilotInfo,
                         irr::u8 nrLaps, bool demoMode, bool debugRace) {
    if (mCurrentRace != nullptr)
        return false;

    std::string levelRootDir("");
    std::string levelName("");

    size_t splitCharPos = targetLevel.find_last_of("/");

    if (splitCharPos == std::string::npos) {
        //split char not found, something wrong!
        logging::Error("Specified target level string invalid, no '/' found, race creation failed!");
        return false;
    }

    levelRootDir.append(targetLevel);
    levelRootDir.append("/");

    levelName.append(targetLevel.substr(splitCharPos + 1, targetLevel.size() - splitCharPos));

    if (mUseXEffects) {
         // Initialise the EffectHandler, pass it the working Irrlicht device and the screen buffer resolution.
         // Shadow map resolution setting has been moved to SShadowLight for more flexibility.
         // (The screen buffer resolution need not be the same as the screen resolution.)
         // The second to last parameter enables VSM filtering, see example 6 for more information.
         // The last parameter enables soft round spot light masks on our shadow lights.
         mEffect = new EffectHandler(mDevice, mDriver->getScreenSize(), false, true, false);
         mEffect->setClearColour(SColor(255, 0, 0, 0));
         mEffect->setAmbientColor(SColor(255, 0, 0, 0));
     }

    //create a new Race
    mCurrentRace = new Race(this, gameMusicPlayer, gameSoundEngine, levelRootDir, levelName, nrLaps, demoMode, debugRace);

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

    //is there at least one player?
    if (mCurrentRace->mPlayerVec.size() == 0) {
        //no player in race, we need to interrupt
        //no race possible
        logging::Error("Not a single player in race, interrupt race creation");
        return false;
    }

    //which player do we want to follow at the start
    //of the race
    mCurrentRace->DebugSelectPlayer(0);

    if (!mCurrentRace->ready) {
        //there was a problem with Race initialization
        logging::Error("Race creation failed!");
        return false;
    }

    return true;
}

bool Game::CreateNewRace(int load_levelnr, std::vector<PilotInfoStruct*> pilotInfo, irr::u8 nrLaps, bool demoMode, bool debugRace) {
    if (mCurrentRace != nullptr)
        return false;

    if (mExtendedGame) {
        if ((load_levelnr < 1) || (load_levelnr > 9)) {
            logging::Error("CreateNewRace: Level number only possible from 1 up to 9!");
            return false;
        }
    } else {
        if ((load_levelnr < 1) || (load_levelnr > 6)) {
            logging::Error("CreateNewRace: Level number only possible from 1 up to 6!");
            return false;
        }
    }

    std::string targetLevel("extract/level0-");
    char nrStr[10];

    snprintf(nrStr, 9, "%d", load_levelnr);
    targetLevel.append(nrStr);

    return (CreateNewRace(targetLevel, pilotInfo, nrLaps, demoMode, debugRace));
}

void Game::CleanUpRace() {
    if (mCurrentRace == nullptr)
        return;
}

//Returns false if game should exit, False otherwise
bool Game::ParseCommandLineForGame() {
    std::vector<std::string>::iterator it;
    std::string substr("test");
    std::string subStr2("nocpu");
    irr::u8 currIdx = 0;

    for (it = mCLIVec.begin(); it != mCLIVec.end(); ++it) {
        //if one parameter contains substring "test" lets
        //enable map test mode which skips menue and
        //everything, and directly goes into a specified target
        //map
        if ((*it).find(substr) != std::string::npos && ((*it).size() == substr.size())) {
            mTestMapMode = true;
            //in this case we need another command line field that
            //specifies the target level/map we should enter
            if ((currIdx + 1) < mCLIVec.size()) {
                mTestTargetLevel.clear();
                mTestTargetLevel.append(mCLIVec.at(currIdx + 1));

                std::string logMessage("Activate map test mode with map ");
                logMessage.append(mTestTargetLevel);

                logging::Info(logMessage);

                //In this special testMode only print
                //Warning/Errors to the log window
                logging::PrintOnlyIssues = true;
            } else {
                //we are missing the target level information
                logging::Error("Command Line parameter 'test' additional needs target map information! Exit game");
                return false;
            }
        }

        //if one parameter contains substring "nocpu" lets
        //disable computer players during test map mode
        if ((*it).find(subStr2) != std::string::npos && ((*it).size() == subStr2.size())) {
               mTestMapModeNoCpuPlayers = true;
               logging::Info("Disabled Cpu Players during map test mode");
        }

        currIdx++;
    }

    return true;
}

Game::Game(int argc, char **argv) : InfrastructureBase(argc, argv, INFRA_RUNNING_AS_GAME) {
}

Game::~Game() {
    //cleanup background images
    if (backgnd != nullptr) {
        backgnd->drop();
        backgnd = nullptr;
    }

    if (gameTitle != nullptr) {
        gameTitle->drop();
        gameTitle = nullptr;
    }

    if (raceLoadingScr != nullptr) {
        raceLoadingScr->drop();
        raceLoadingScr = nullptr;
    }
}
