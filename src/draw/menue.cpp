/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "menue.h"
#include "../audio/sound.h"
#include "../game.h"
#include "../input/input.h"
#include "gametext.h"
#include "../resources/assets.h"
#include "../race.h"

MenueAction::MenueAction(irr::u32 actionNrParam) {
    actionNr = actionNrParam;
}

MenueAction::~MenueAction() {
}

MenuePage::~MenuePage() {
}

MenuePage::MenuePage(Menue* parentMenue, MenuePage* parentPage, irr::u8 pageNr, MenueAction* pageEscKeyAction) {
    pageNumber = pageNr;
    parentMenuePage = parentPage;
    pageEscKeyTriggerAction = pageEscKeyAction;
    mParentMenue = parentMenue;
};

MenueSingleEntry* MenuePage::AddDefaultMenueEntry(const char* text, bool itemSelectable, MenuePage* goToPage, MenueAction* triggerAction) {
    return(AddDefaultMenueEntry(text, itemSelectable, goToPage, triggerAction, mParentMenue->mGame->mGameTexts->GameMenueUnselectedEntryFont,
                                                      mParentMenue->mGame->mGameTexts->GameMenueSelectedItemFont));
}

MenueSingleEntry* MenuePage::AddDefaultMenueEntry(const char* text, bool itemSelectable, MenuePage* goToPage, MenueAction* triggerAction,
                                                  GameTextFont* unselectedFont, GameTextFont* selectedFont) {
    irr::u8 currNrEntries = (irr::u8)(this->pageEntryVec.size());

    if (currNrEntries < 255) {
          MenueSingleEntry* newEntry = new MenueSingleEntry();

          newEntry->mParentPage = this;
          newEntry->entryType = MENUE_ENTRY_TYPE_DEFAULT;
          //This entry has no checkbox/slider; Therefore maxValue = 0
          newEntry->maxValue = 0;
          newEntry->currValue = 0;
          newEntry->entryText = strdup(text);
          newEntry->entryNumber = currNrEntries;
          newEntry->nextMenuePage = goToPage;
          newEntry->triggerAction = triggerAction;
          newEntry->itemSelectable = itemSelectable;

          //set special used fonts
          newEntry->usedUnselectedTextFont = unselectedFont;
          newEntry->usedSelectedTextFont = selectedFont;

          pageEntryVec.push_back(newEntry);

          return newEntry;
    } else {
        logging::Error("Too much menue entries, Skip new entry creation!");
    }

    return nullptr;
}

MenueSingleEntry* MenuePage::AddEmptySpaceMenueEntry() {
    irr::u8 currNrEntries = (irr::u8)(this->pageEntryVec.size());

    if (currNrEntries < 255) {
          MenueSingleEntry* newEntry = new MenueSingleEntry();

          newEntry->mParentPage = this;
          newEntry->entryType = MENUE_ENTRY_TYPE_EMPTYSPACE;
          newEntry->maxValue = 0;
          newEntry->currValue = 0;
          newEntry->entryText = strdup("");
          newEntry->entryNumber = currNrEntries;
          newEntry->nextMenuePage = nullptr;
          newEntry->triggerAction = nullptr;
          newEntry->itemSelectable = false;

          //use the default menue fonts
          newEntry->usedUnselectedTextFont = mParentMenue->mGame->mGameTexts->GameMenueUnselectedEntryFont;
          newEntry->usedSelectedTextFont = mParentMenue->mGame->mGameTexts->GameMenueSelectedItemFont;

          pageEntryVec.push_back(newEntry);

          return newEntry;
    } else {
        logging::Error("Too much menue entries, Skip new entry creation!");
    }

    return nullptr;
}

MenueSingleEntry* MenuePage::AddSliderMenueEntry(const char* text, bool itemSelectable,
                                                 irr::u8 currValueParam, irr::u8 maxValueParam, irr::u8 nrBlocksParam,
                                                 irr::u8 checkBoxPixelPerBlockWidthParam, irr::u8 checkBoxPixelPerBlockHeightParam,
                                                 MenueAction* triggerAction) {
    irr::u8 currNrEntries = (irr::u8)(this->pageEntryVec.size());

    if (currNrEntries < 255) {
          MenueSingleEntry* newEntry = new MenueSingleEntry();

          newEntry->mParentPage = this;
          newEntry->entryType = MENUE_ENTRY_TYPE_SLIDER;
          //This entry has a checkbox/slider; We know this when
          //maxValue is set > 0
          newEntry->maxValue = maxValueParam;
          newEntry->currValue = currValueParam;
          newEntry->entryText = strdup(text);
          newEntry->entryNumber = currNrEntries;
          newEntry->nextMenuePage = nullptr;
          newEntry->triggerAction = triggerAction;
          newEntry->itemSelectable = itemSelectable;
          newEntry->checkBoxPixelPerBlockWidth = checkBoxPixelPerBlockWidthParam;
          newEntry->checkBoxPixelPerBlockHeight = checkBoxPixelPerBlockHeightParam;
          newEntry->checkBoxNrBlocks = nrBlocksParam;

          //use the default menue fonts
          newEntry->usedUnselectedTextFont = mParentMenue->mGame->mGameTexts->GameMenueUnselectedEntryFont;
          newEntry->usedSelectedTextFont = mParentMenue->mGame->mGameTexts->GameMenueSelectedItemFont;

          pageEntryVec.push_back(newEntry);

          return newEntry;
    } else {
        logging::Error("Too much menue entries, Skip new entry creation!");
    }

    return nullptr;
}

MenueSingleEntry* MenuePage::AddTextInputMenueEntry(char* initTextPntrParam, bool itemSelectable, MenueAction* triggerAction) {
    irr::u8 currNrEntries = (irr::u8)(this->pageEntryVec.size());

    if (currNrEntries < 255) {
          MenueSingleEntry* newEntry = new MenueSingleEntry();

          newEntry->mParentPage = this;
          newEntry->entryType = MENUE_ENTRY_TYPE_TEXTINPUTFIELD;
          //This entry has no checkbox/slider; Therefore maxValue = 0
          newEntry->maxValue = 0;
          newEntry->currValue = 0;
          newEntry->entryText = strdup(""); //start with empty text!
          newEntry->entryNumber = currNrEntries;
          newEntry->nextMenuePage = nullptr;
          newEntry->triggerAction = triggerAction;
          newEntry->itemSelectable = itemSelectable;
          newEntry->isTextEntryField = true;  //important!

          newEntry->initTextPntr = initTextPntrParam; //set initial text at the start

          //use the default menue fonts
          newEntry->usedUnselectedTextFont = mParentMenue->mGame->mGameTexts->GameMenueUnselectedEntryFont;
          newEntry->usedSelectedTextFont = mParentMenue->mGame->mGameTexts->GameMenueSelectedItemFont;

          pageEntryVec.push_back(newEntry);

          return newEntry;
    } else {
        logging::Error("Too much menue entries, Skip new entry creation!");
    }

   return nullptr;
}

void MenuePage::RealignMenueEntries(irr::core::recti newMenueSpace) {
    //we want to reorganize all menue entries in a way, that
    //all printed information is centered inside the currently
    //available menue selection space
    irr::u16 nrEntries = (irr::u16)(this->pageEntryVec.size());
    irr::u32 startAlignY;
    irr::u32 alignX = newMenueSpace.UpperLeftCorner.X + newMenueSpace.getWidth() / 2;

    if (nrEntries > 0) {
        irr::u32 height = newMenueSpace.getHeight();

        irr::u32 neededHeight = 0;

        std::vector<MenueSingleEntry*>::iterator it;
        for (it = pageEntryVec.begin(); it != pageEntryVec.end(); ++it) {
            if (((*it)->entryType != MENUE_ENTRY_TYPE_TEXTINPUTFIELD) &&
                ((*it)->entryType != MENUE_ENTRY_TYPE_EMPTYSPACE)) {
                neededHeight += (*it)->GetHeight();
            } else {
                //for text entry or empty space entry
                //use fixed height in pixels
                neededHeight += MENUE_ENTRY_EMPTYSPACE_HEIGHT_PIXELS;
            }

            neededHeight += MENUE_ENTRY_DISTANCE_PIXEL;
        }

        irr::u32 remainHeight = height - neededHeight;
        startAlignY = newMenueSpace.UpperLeftCorner.Y + remainHeight / 2;

        for (it = pageEntryVec.begin(); it != pageEntryVec.end(); ++it) {
           (*it)->drawTextScrPosition.X = alignX - (*it)->GetWidth() / 2;
           (*it)->drawTextScrPosition.Y = startAlignY;

            //if this element also has a slider, we also need to set its outline box
            if ((*it)->entryType == MENUE_ENTRY_TYPE_SLIDER) {
                (*it)->checkBoxOutline.UpperLeftCorner.X = alignX + (*it)->GetWidth() / 2 + MENUE_ENTRY_TOSLIDER_DISTANCE_PIXEL;
                (*it)->checkBoxOutline.UpperLeftCorner.Y = startAlignY;
                (*it)->checkBoxOutline.LowerRightCorner.X = (*it)->checkBoxOutline.UpperLeftCorner.X + (*it)->checkBoxNrBlocks * (*it)->checkBoxPixelPerBlockWidth;
                (*it)->checkBoxOutline.LowerRightCorner.Y = (*it)->checkBoxOutline.UpperLeftCorner.Y + (*it)->checkBoxPixelPerBlockHeight;
            }

            if (((*it)->entryType != MENUE_ENTRY_TYPE_TEXTINPUTFIELD) &&
                ((*it)->entryType != MENUE_ENTRY_TYPE_EMPTYSPACE)) {
                    startAlignY += (*it)->GetHeight() + MENUE_ENTRY_DISTANCE_PIXEL;
            } else {
                    startAlignY += MENUE_ENTRY_EMPTYSPACE_HEIGHT_PIXELS;
            }
        }
    }
}

//Gets the needed width in pixels
irr::u32 MenueSingleEntry::GetWidth() {
    //Get width of text in pixels for entry
    irr::u32 textWidth =
             mParentPage->mParentMenue->mGame->mGameTexts->GetWidthPixelsGameText(entryText,
                                                  this->usedUnselectedTextFont);

    //is also a checkbox/slider included?
    if (maxValue != 0) {
        //yes, it is
        textWidth += MENUE_ENTRY_TOSLIDER_DISTANCE_PIXEL;
        textWidth += checkBoxNrBlocks * checkBoxPixelPerBlockWidth;
    }

    return textWidth;
}

//Gets the needed height in pixels
irr::u32 MenueSingleEntry::GetHeight() {
    //Get height of text in pixels for entry
    irr::u32 textHeight =
             mParentPage->mParentMenue->mGame->mGameTexts->GetHeightPixelsGameText(entryText,
                                                  this->usedUnselectedTextFont);

    //is also a checkbox/slider included?
    if (maxValue != 0) {
        //yes, it is
        //if the slider blocks are heigher then the text itself
        //the set the needed height to the height of the slider
        //block itself
        if (checkBoxPixelPerBlockHeight > textHeight) {
            textHeight = checkBoxPixelPerBlockHeight;
        }
    }

    return textHeight;
}

void MenueSingleEntry::SetText(char* newText) {
    if (newText == nullptr)
        return;

    if (entryText != nullptr) {
        //delete old string at heap
        //we need to use free here, as we have strdup
        //text strings before!
        free(entryText);

        //set new output string
        entryText = newText;
    }
}

void Menue::RealignMenuePageItems() {
    if (currSelMenuePage == nullptr)
        return;

    irr::core::recti scaled;

    scaled.UpperLeftCorner.X = (currMenueSpace.UpperLeftCorner.X * mGame->mScreenRes.Width) / 640;
    scaled.LowerRightCorner.X = (currMenueSpace.LowerRightCorner.X * mGame->mScreenRes.Width) / 640;

    scaled.UpperLeftCorner.Y = (currMenueSpace.UpperLeftCorner.Y * mGame->mScreenRes.Height) / 400;
    scaled.LowerRightCorner.Y = (currMenueSpace.LowerRightCorner.Y * mGame->mScreenRes.Height) / 400;

    currSelMenuePage->RealignMenueEntries(scaled);
}

void Menue::PlayMenueSound(uint8_t sndResId) {
  if (mSoundEngine != nullptr) {
      //in menue only allow one sound to play at
      //the same time
      if (!mSoundEngine->IsAnySoundPlaying()) {
        mSoundEngine->PlaySound(sndResId);
      }
  }
}

void Menue::StopMenueSound() {
    if (mSoundEngine != nullptr) {
        mSoundEngine->StopAllSounds();
    }
}

bool Menue::InitMenueResources() {
    mGame->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    //load window graphic elements
    wndCornerElementUpperLeftTex = mGame->mDriver->getTexture("extract/hud1player/panel0-1-0299.bmp");
    if (wndCornerElementUpperLeftTex == nullptr)
        return false;
    //define transparency color
    mGame->mDriver->makeColorKeyTexture(wndCornerElementUpperLeftTex,
           irr::core::position2d<irr::s32>(wndCornerElementUpperLeftTex->getSize().Width - 1, wndCornerElementUpperLeftTex->getSize().Height - 1));

    wndCornerElementUpperRightTex = mGame->mDriver->getTexture("extract//hud1player/panel0-1-0300.bmp");
    if (wndCornerElementUpperRightTex == nullptr)
        return false;
    //define transparency color
    mGame->mDriver->makeColorKeyTexture(wndCornerElementUpperRightTex,
           irr::core::position2d<irr::s32>(0, wndCornerElementUpperRightTex->getSize().Height - 1));

    wndCornerElementLowerLeftTex  = mGame->mDriver->getTexture("extract//hud1player/panel0-1-0301.bmp");
    if (wndCornerElementLowerLeftTex == nullptr)
        return false;
    //define transparency color
    mGame->mDriver->makeColorKeyTexture(wndCornerElementLowerLeftTex,
           irr::core::position2d<irr::s32>(wndCornerElementLowerLeftTex->getSize().Width - 1, 0));

    wndCornerElementLowerRightTex = mGame->mDriver->getTexture("extract//hud1player/panel0-1-0302.bmp");
    if (wndCornerElementLowerRightTex == nullptr)
        return false;
    //define transparency color
    mGame->mDriver->makeColorKeyTexture(wndCornerElementLowerRightTex,
           irr::core::position2d<irr::s32>(0, 0));

    /**********************************
     * Load game logo images          *
     **********************************/

    std::string imgNamePrefix("");
    std::string imgName("");
    char nrStr[15];

    if (mGame->mGameConfig->enableDoubleResolution) {
        //use images that were upscaled by a factor of 2 before
        imgNamePrefix.append("extract/images/logo0-1-x2-");
    } else {
        imgNamePrefix.append("extract/images/logo0-1-");
    }

    MenueGraphicPart* GameLogoPiece;
    std::vector<irr::core::vector2d<irr::s32>> posDefinitionVector;

    //Define the game logo images draw positions to stich the logo properly
    //together
    if (!mGame->mGameConfig->enableDoubleResolution) {
        posDefinitionVector.push_back(irr::core::vector2di(90, 34));
        posDefinitionVector.push_back(irr::core::vector2di(220, 34));
        posDefinitionVector.push_back(irr::core::vector2di(347, 34));
        posDefinitionVector.push_back(irr::core::vector2di(474, 34));
        posDefinitionVector.push_back(irr::core::vector2di(91, 92));
        posDefinitionVector.push_back(irr::core::vector2di(218, 92));

        mLogoExtensionStrPos.set(368,92);
    } else {
        posDefinitionVector.push_back(irr::core::vector2di(90 * 2, 34 * 2));
        posDefinitionVector.push_back(irr::core::vector2di(220 * 2, 34 * 2));
        posDefinitionVector.push_back(irr::core::vector2di(347 * 2, 34 * 2));
        posDefinitionVector.push_back(irr::core::vector2di(474 * 2, 34 * 2));
        posDefinitionVector.push_back(irr::core::vector2di(91 * 2, 92 * 2));
        posDefinitionVector.push_back(irr::core::vector2di(218 * 2, 92 * 2));

        mLogoExtensionStrPos.set(368 * 2 + 40, 92 * 2);
    }

    for (long idx = 0; idx < 6; idx++) {
        imgName.clear();
        imgName.append(imgNamePrefix);

        sprintf (nrStr, "%0*ld.bmp", 4, idx);
        imgName.append(nrStr);

        GameLogoPiece = new MenueGraphicPart();
        GameLogoPiece->texture = mGame->mDriver->getTexture(imgName.c_str());
        if (GameLogoPiece->texture == nullptr) {
            return false;
        }
        mGame->mDriver->makeColorKeyTexture(GameLogoPiece->texture, irr::core::position2d<irr::s32>(0,0));
        GameLogoPiece->sizeTex = GameLogoPiece->texture->getSize();
        GameLogoPiece->drawScrPosition.set(posDefinitionVector.at(idx));

        GameLogo.push_back(GameLogoPiece);
    }

    mGame->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}

void Menue::AcceptedRaceSetup() {  
    //tell main program that we should start
    //the race
    //handover selected level number
    this->ActRace->currSetValue = currSelectedRaceTrack + 1;
    this->currActionToExecute = this->ActRace;
}

void Menue::InitActions() {
    //create the action for starting the race
    ActRace = new MenueAction(MENUE_ACTION_RACE);

    ActQuitToOS = new MenueAction(MENUE_ACTION_QUITTOOS);

    /****************************
     * Championship Actions     *
     ****************************/

    ActEnterChampionshipMenue = new MenueAction(MENUE_ACTION_CHAMPIONSHIP_ENTERMENUE);
    ActContinueChampionship = new MenueAction(MENUE_ACTION_CHAMPIONSHIP_CONTINUE);
    ActLoadChampionshipSlot = new MenueAction(MENUE_ACTION_CHAMPIONSHIP_LOADSLOT);
    ActSaveChampionshipSlot = new MenueAction(MENUE_ACTION_CHAMPIONSHIP_SAVESLOT);
    ActFinalizeChampionshipSaveSlot = new MenueAction(MENUE_ACTION_CHAMPIONSHIP_SAVESLOT_FINALIZE);
    ActQuitChampionship = new MenueAction(MENUE_ACTION_CHAMPIONSHIP_QUIT);

    /***************************
     * Other actions           *
     ***************************/

    //create an action for setting a new player name
    ActSetPlayerName = new MenueAction(MENUE_ACTION_SETPLAYERNAME);

    //a special action that signals the Main loop that race stat
    //page was closed by player
    ActCloseRaceStatPage = new MenueAction(MENUE_ACTION_CLOSERACESTATPAGE);

    //a special action that signals the Main loop that points table
    //page was closed by player
    ActClosePointsTablePage = new MenueAction(MENUE_ACTION_CLOSEPOINTSTABLEPAGE);

    //a special action that signals the Main loop that a demo
    //should be started
    ActStartDemo = new MenueAction(MENUE_ACTION_STARTDEMO);

    //a special action that signals the Main loop that the
    //highscore page should be shown
    ActShowHighScorePage = new MenueAction(MENUE_ACTION_SHOWHIGHSCOREPAGE);
    ActSetComputerPlayerEnable = new MenueAction(MENUE_ACTION_SETCOMPUTERPLAYERENA);
    ActSetDifficultyLevel = new MenueAction(MENUE_ACTION_SETDIFFICULTYLEVEL);
    ActSkipIntro = new MenueAction(MENUE_ACTION_SETSKIPINTRO);

    /***************************
     * Option actions          *
     ***************************/

    ActReturnFromDetailsMenue = new MenueAction(MENUE_ACTION_RETURNFROMDETAILSMENUE);
    ActSetSoundVolume = new MenueAction(MENUE_ACTION_SETSOUNDVOLUME);
    ActSetMusicVolume = new MenueAction(MENUE_ACTION_SETMUSICVOLUME);
    ActSetUpgradedSky = new MenueAction(MENUE_ACTION_SETUPGRADEDSKY);
    ActSetEnableShadows = new MenueAction(MENUE_ACTION_SETENABLESHADOW);
    ActSetVSync = new MenueAction(MENUE_ACTION_SETVSYNC);
    ActSetDoubleResolution = new MenueAction(MENUE_ACTION_SETDOUBLERESOLUTION);

    /***************************
     * Info actions            *
     ***************************/

    ActStartAttribution = new MenueAction(MENUE_ACTION_STARTATTRIBUTION);
}

void Menue::InitMenuePages() {
    //define menue pages first, highest top page, has no parent
    TopMenuePage = new MenuePage(this, nullptr, MENUE_TOPENTRY);

    RaceMenuePage = new MenuePage(this, TopMenuePage, MENUE_SELECTIONRACE);
    OptionMenuePage = new MenuePage(this, TopMenuePage, MENUE_OPTION);

    //TODO: do we need a link to a parent page here? IMPORTANT!
    RaceTrackSelectionPage = new MenuePage(this, nullptr, MENUE_SELECTRACETRACK);
    //TODO: do we need a link to a parent page here? IMPORTANT!
    ShipSelectionPage = new MenuePage(this, nullptr, MENUE_SELECTSHIP);

    ChampionshipMenuePage = new MenuePage(this, RaceMenuePage, MENUE_CHAMPIONSHIP);

    ChampionshipSaveMenuePage = new MenuePage(this, ChampionshipMenuePage, MENUE_CHAMPIONSHIPSAVE);
    ChampionshipLoadMenuePage = new MenuePage(this, ChampionshipMenuePage, MENUE_CHAMPIONSHIPLOAD);

    //parent page of changeNamePage is option menue page, important!
    ChangeNamePage = new MenuePage(this, OptionMenuePage, MENUE_CHANGENAME);
    VideoDetailsPage = new MenuePage(this, OptionMenuePage, MENUE_VIDEOOPTIONS, ActReturnFromDetailsMenue);

    //parent page of sound option menue is Option menue page
    SoundOptionsPage = new MenuePage(this, OptionMenuePage, MENUE_SOUNDOPTIONS);

    //define dummy menue page for highscore screen
    //if we press ESC on the highscore page go to game menue
    gameHighscoreMenuePage = new MenuePage(this, nullptr, MENUE_HIGHSCORE);

    //define dummy menue page for race stats screen
    //if we press ESC on the race stats page go to game menue
    raceStatsMenuePage = new MenuePage(this, nullptr, MENUE_RACESTATS);

    //define dummy menue page for points table page
    pointsTablePage = new MenuePage(this, nullptr, MENUE_POINTSTABLE);

    //Info menue page
    InfoMenuePage = new MenuePage(this, TopMenuePage, MENUE_INFO);
}

void Menue::CreateMenueEntries() {
    /**************************
     * TOP Level Menue Page   *
     * ************************/

    Race = TopMenuePage->AddDefaultMenueEntry("RACE", true, RaceMenuePage, nullptr);
    TopMenuePage->AddDefaultMenueEntry("OPTIONS", true, OptionMenuePage, nullptr);
    TopMenuePage->AddDefaultMenueEntry("INFO", true, InfoMenuePage, nullptr);
    TopMenuePage->AddDefaultMenueEntry("QUIT TO OS", true, nullptr, ActQuitToOS);

    /******************************
    * SELECTION RACE Menue Page   *
    * *****************************/

    SelectChampionsship = RaceMenuePage->AddDefaultMenueEntry("CHAMPIONSHIP", true, nullptr, ActEnterChampionshipMenue);
    SelectSingleRace = RaceMenuePage->AddDefaultMenueEntry("SINGLE RACE", true, RaceTrackSelectionPage, nullptr);
    RaceMenuePage->AddDefaultMenueEntry("MAIN MENUE", true, TopMenuePage, nullptr);

    /*****************************
     * Championship Menue Page   *
     * ***************************/

    //default make next item not selectable!
    ContinueChampionshipEntry = ChampionshipMenuePage->AddDefaultMenueEntry("CONTINUE CHAMPIONSHIP", false, nullptr, ActContinueChampionship);
    NewChampionshipEntry = ChampionshipMenuePage->AddDefaultMenueEntry("NEW CHAMPIONSHIP", true, RaceTrackSelectionPage, nullptr);
    ChampionshipMenuePage->AddDefaultMenueEntry("LOAD CHAMPIONSHIP", true, ChampionshipLoadMenuePage, nullptr);
    //default make next item not selectable!
    SaveChampionshipEntry = ChampionshipMenuePage->AddDefaultMenueEntry("SAVE CHAMPIONSHIP", false, ChampionshipSaveMenuePage, nullptr);
    ChampionshipMenuePage->AddDefaultMenueEntry("QUIT CHAMPIONSHIP", true, nullptr, ActQuitChampionship);

    /*********************************
    * CHAMPIONSHIP SAVE Menue Page   *
    * ********************************/

    //This item is only a lable, make not selectable!
    ChampionshipSaveMenuePage->AddDefaultMenueEntry("SAVE GAME", false, nullptr, nullptr);
    ChampionshipSaveMenuePage->AddEmptySpaceMenueEntry();
    ChampionshipSaveSlot1 = ChampionshipSaveMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActSaveChampionshipSlot);
    ChampionshipSaveSlot2 = ChampionshipSaveMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActSaveChampionshipSlot);
    ChampionshipSaveSlot3 = ChampionshipSaveMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActSaveChampionshipSlot);
    ChampionshipSaveSlot4 = ChampionshipSaveMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActSaveChampionshipSlot);
    ChampionshipSaveSlot5 = ChampionshipSaveMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActSaveChampionshipSlot);
    ChampionshipSaveMenuePage->AddEmptySpaceMenueEntry();
    ChampionshipSaveMenuePage->AddDefaultMenueEntry("MAIN OPTIONS", true, ChampionshipMenuePage, nullptr);

    /*********************************
    * CHAMPIONSHIP LOAD Menue Page   *
    * ********************************/

    //This item is only a lable, make not selectable!
    ChampionshipLoadMenuePage->AddDefaultMenueEntry("LOAD GAME", false, nullptr, nullptr);
    ChampionshipLoadMenuePage->AddEmptySpaceMenueEntry();
    ChampionshipLoadSlot1 = ChampionshipLoadMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActLoadChampionshipSlot);
    ChampionshipLoadSlot2 = ChampionshipLoadMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActLoadChampionshipSlot);
    ChampionshipLoadSlot3 = ChampionshipLoadMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActLoadChampionshipSlot);
    ChampionshipLoadSlot4 = ChampionshipLoadMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActLoadChampionshipSlot);
    ChampionshipLoadSlot5 = ChampionshipLoadMenuePage->AddDefaultMenueEntry("EMPTY", true, nullptr, ActLoadChampionshipSlot);
    ChampionshipLoadMenuePage->AddEmptySpaceMenueEntry();
    ChampionshipLoadMenuePage->AddDefaultMenueEntry("MAIN OPTIONS", true, ChampionshipMenuePage, nullptr);

    /*********************************
    * Dummy Menue Entries            *
    * ********************************/

    gameHiscoreMenueDummyEntry = gameHighscoreMenuePage->AddDefaultMenueEntry("", true, nullptr, nullptr);
    raceStatsMenueDummyEntry = raceStatsMenuePage->AddDefaultMenueEntry("", true, nullptr, ActCloseRaceStatPage);
    pointsTableMenueDummyEntry = pointsTablePage->AddDefaultMenueEntry("", true, nullptr, ActClosePointsTablePage);

    /**********************
    * OPTION Menue Page   *
    * *********************/

    OptionMenuePage->AddDefaultMenueEntry("CHANGE NAME", true, ChangeNamePage, nullptr);
    OptionMenuePage->AddDefaultMenueEntry("DETAIL OPTIONS", true, VideoDetailsPage, nullptr);
    OptionMenuePage->AddDefaultMenueEntry("SOUND OPTIONS", true, SoundOptionsPage, nullptr);
    OptionMenuePage->AddDefaultMenueEntry("REINITIALIZE JOYSTICK", true, nullptr, nullptr);

    irr::u8 setValue;
    //computer players is a checkbox
    if (this->mGameAssets->GetComputerPlayersEnabled()) {
        setValue = 1;
    } else {
        setValue = 0;
    }

    OptionMenuePage->AddSliderMenueEntry("COMPUTER PLAYERS", true, setValue, 1, 1, 22, 14, ActSetComputerPlayerEnable);

    //difficultyLevel allows to set 4 different levels
    //get current difficulty level from assets class
    setValue = mGameAssets->GetCurrentGameDifficulty();

    OptionMenuePage->AddSliderMenueEntry("DIFFICULTY LEVEL", true, setValue, 3, 3, 22, 14, ActSetDifficultyLevel);

    //Skip intro
    if (mGame->mGameConfig->skipIntro) {
        setValue = 1;
    } else {
        setValue = 0;
    }

    OptionMenuePage->AddSliderMenueEntry("SKIP INTRO", true, setValue, 1, 1, 22, 14, ActSkipIntro);
    OptionMenuePage->AddDefaultMenueEntry("MAIN MENU", true, TopMenuePage, nullptr);

    /**********************************
    * CHANGE Player Name Menue Page   *
    * *********************************/
    //the first entry just is used to print label "ENTER NAME"
    //is not selectable
    ChangeNamePage->AddDefaultMenueEntry("ENTER NAME", false, nullptr, nullptr);

    //the second entry is used for player name input
    //initialize the input text field with current configured
    //main player name
    ChangeNamePage->AddTextInputMenueEntry(this->mGameAssets->GetNewMainPlayerName(), true, ActSetPlayerName);

    /************************************
    * Video Details Option Menue Page   *
    * ***********************************/

    //enable double resolution is a checkbox
    if (mGame->mGameConfig->enableDoubleResolution) {
        setValue = 1;
    } else {
        setValue = 0;
    }

    VideoDetailsPage->AddSliderMenueEntry("DOUBLE RESOLUTION", true, setValue, 1, 1, 22, 14, ActSetDoubleResolution);

    //enable vsync is a checkbox
    if (mGame->mGameConfig->enableVSync) {
        setValue = 1;
    } else {
        setValue = 0;
    }

    VideoDetailsPage->AddSliderMenueEntry("ENABLE VSYNC", true, setValue, 1, 1, 22, 14, ActSetVSync);

    //enable shadows is a checkbox
    if (mGame->mGameConfig->enableShadows) {
        setValue = 1;
    } else {
        setValue = 0;
    }

    VideoDetailsPage->AddSliderMenueEntry("ENABLE SHADOWS", true, setValue, 1, 1, 22, 14, ActSetEnableShadows);

    //option to use the upgraded sky is a checkbox
    if (mGame->mGameConfig->useUpgradedSky) {
        setValue = 1;
    } else {
        setValue = 0;
    }

    VideoDetailsPage->AddSliderMenueEntry("UPGRADED SKY", true, setValue, 1, 1, 22, 14, ActSetUpgradedSky);
    VideoDetailsPage->AddDefaultMenueEntry("MAIN OPTIONS", true, OptionMenuePage, ActReturnFromDetailsMenue);

    /*****************************
    * Sound Options Menue Page   *
    * ****************************/

    //music volume is a slider with 16 blocks (17 different possible settings)

    //get current music volume from GameAssets class
    irr::f32 currMusicVol = this->mGameAssets->GetMusicVolume();
    currMusicVol = (currMusicVol / 100.0f) * (irr::f32)(16);

    irr::u8 finalVal = (irr::u8)(currMusicVol);
    //plausi check
    if (finalVal < 0)
        finalVal = 0;

    if (finalVal > 16)
        finalVal = 16;

    SoundOptionsPage->AddSliderMenueEntry("MUSIC VOLUME", true, finalVal, 16, 16, 10, 14, ActSetMusicVolume);

    //effects volume is a slider with 16 blocks (17 different possible settings)

    //get current sound volume from GameAssets class
    irr::f32 currSoundVol = this->mGameAssets->GetSoundVolume();
    currSoundVol = (currSoundVol / 100.0f) * (irr::f32)(16);

    finalVal = (irr::u8)(currSoundVol);
    //plausi check
    if (finalVal < 0)
        finalVal = 0;

    if (finalVal > 16)
        finalVal = 16;

    SoundOptionsPage->AddSliderMenueEntry("EFFECTS VOLUME", true, finalVal, 16, 16, 10, 14, ActSetSoundVolume);
    SoundOptionsPage->AddDefaultMenueEntry("MAIN OPTIONS", true, OptionMenuePage, nullptr);

    //Race track selection page
    //Item must be selectable, Important!
    RaceTrackNameTitle = RaceTrackSelectionPage->AddDefaultMenueEntry("", true, nullptr, nullptr);

    if (mGame->mGameConfig->enableDoubleResolution) {
        RaceTrackSelectionPage->AddEmptySpaceMenueEntry();
        RaceTrackSelectionPage->AddEmptySpaceMenueEntry();
    }

    SelRaceTrackNrLapsLabel = RaceTrackSelectionPage->AddDefaultMenueEntry("", false, nullptr, nullptr, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA,
                                                                           mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA);

    SelRaceTrackBestLapLabel = RaceTrackSelectionPage->AddDefaultMenueEntry("", false, nullptr, nullptr, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA,
                                                                            mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA);

    SelRaceTrackBestRaceLabel = RaceTrackSelectionPage->AddDefaultMenueEntry("", false, nullptr, nullptr, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA,
                                                                             mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA);

    //Ship selection page
    //Item must be selectable, Important!
    ShipNameTitle = ShipSelectionPage->AddDefaultMenueEntry("", true, nullptr, nullptr);
    ShipSelectionPage->AddEmptySpaceMenueEntry();
    ShipSelectionPage->AddEmptySpaceMenueEntry();
    ShipSelectionPage->AddEmptySpaceMenueEntry();

    if (mGame->mGameConfig->enableDoubleResolution) {
        ShipSelectionPage->AddEmptySpaceMenueEntry();
        ShipSelectionPage->AddEmptySpaceMenueEntry();
    }

    /**********************
    * INFO Menue Page   *
    * *********************/

    InfoMenuePage->AddDefaultMenueEntry("ATTRIBUTION", true, nullptr, ActStartAttribution);
    InfoMenuePage->AddDefaultMenueEntry("MAIN MENU", true, TopMenuePage, nullptr);
}

void Menue::SetWindowAnimationVec() {
    //the following coordinates were derived by using the original game + Dosbox debugger
    //stepping frame by frame, Screenshots and simply measurements in picture editor :)
    //I simply put the measured coordinates here, and did not bother to further investigate
    //how the coordinates are most likely simply calculated in the original game :)
    //important note: This specified coordinates are based on the 640x400 resolution the
    //game seems to use in VGA mode; This is how I measured this coordinates
    //later in my code I will scale this values according to my resolution, so that I am
    //very flexible with resolution selection

    //first animation of menue -> from initial game start to Main Top Level menue open
    windowMenueAnimationStartGame = new MenueWindowAnimationVec();
    windowMenueAnimationStartGame->coordVec.clear();
    //this animation is done in 25 steps
    //coordinates are for left upper corner of drawn window / right lower corner of drawn window
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 382, 379, 473)); //Step 1
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 373, 379, 453)); //Step 2
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 363, 379, 433)); //Step 3
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 352, 379, 413)); //Step 4
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 330, 379, 393)); //Step 5
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 311, 379, 374)); //Step 6
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 290, 379, 352)); //Step 7
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 269, 379, 332)); //Step 8
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 250, 379, 312)); //Step 9
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(262, 226, 379, 298)); //Step 10
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(251, 222, 395, 295)); //Step 11
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(236, 217, 410, 301)); //Step 12
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(205, 206, 440, 313)); //Step 13
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(188, 198, 452, 319)); //Step 14
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(177, 193, 471, 324)); //Step 15
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(158, 186, 483, 329)); //Step 16
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(145, 181, 499, 338)); //Step 17
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(131, 175, 514, 342)); //Step 18
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(115, 169, 527, 349)); //Step 19
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti(102, 164, 544, 355)); //Step 20
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti( 84, 157, 556, 361)); //Step 21
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti( 73, 151, 575, 367)); //Step 22
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti( 54, 145, 587, 373)); //Step 23
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti( 41, 141, 602, 380)); //Step 24
    this->windowMenueAnimationStartGame->coordVec.push_back(irr::core::recti( 26, 131, 619, 384)); //Step 25 - final fully open window state

    //animation shown during transition from main menue to track selection page
    windowMenueAnimationBeforeTrackSelection = new MenueWindowAnimationVec();
    windowMenueAnimationBeforeTrackSelection->coordVec.clear();
    //this animation is done in 32 steps
    //coordinates are for left upper corner of drawn window / right lower corner of drawn window
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 25, 132, 618, 385)); //Step 1
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 40, 129, 604, 371)); //Step 2
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 53, 122, 587, 352)); //Step 3
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 69, 119, 572, 336)); //Step 4
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 84, 113, 557, 318)); //Step 5
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 99, 109, 543, 302)); //Step 6
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(115, 104, 528, 285)); //Step 7
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(131,  98, 514, 267)); //Step 8
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(143,  92, 497, 249)); //Step 9
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(159,  88, 484, 233)); //Step 10
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(175,  80, 470, 213)); //Step 11
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(187,  77, 453, 198)); //Step 12
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(204,  72, 439, 181)); //Step 13
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(217,  66, 422, 163)); //Step 14
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(233,  62, 409, 146)); //Step 15
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(247,  55, 393, 128)); //Step 16
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(250,  48, 394, 115)); //Step 17
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(263,  49, 378, 110)); //Step 18
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(234,  46, 407, 117)); //Step 19
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(206,  40, 439, 123)); //Step 20
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(189,  36, 453, 123)); //Step 21
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(175,  35, 469, 128)); //Step 22
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(160,  33, 485, 130)); //Step 23
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(144,  29, 499, 131)); //Step 24
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(129,  27, 510, 132)); //Step 25
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(114,  24, 526, 134)); //Step 26
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti(101,  23, 544, 139)); //Step 27
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 85,  19, 557, 141)); //Step 28
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 70,  17, 573, 143)); //Step 29
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 54,  15, 587, 147)); //Step 30
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 41,  13, 604, 149)); //Step 31
    this->windowMenueAnimationBeforeTrackSelection->coordVec.push_back(irr::core::recti( 24,   7, 617, 150)); //Step 32 - final fully open window state

    //is shown during transition from race track selection to main menue
    windowMenueAnimationQuitTrackSelection = new MenueWindowAnimationVec();
    windowMenueAnimationQuitTrackSelection->coordVec.clear();
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 25,  10, 618, 153));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 39,  23, 604, 161));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 53,  36, 588, 169));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 69,  49, 574, 178));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 99,  76, 544, 195));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(130, 103, 515, 212));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(156, 132, 483, 231));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(188, 157, 454, 246));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(201, 171, 438, 254));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(216, 185, 423, 264));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(233, 200, 410, 273));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(263, 237, 380, 290));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(234, 216, 409, 301));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(204, 205, 439, 314));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(174, 192, 469, 325));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(144, 181, 499, 338));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(112, 168, 526, 348));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti(100, 163, 543, 356));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 87, 156, 560, 361));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 72, 151, 575, 368));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 55, 144, 588, 373));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 42, 138, 605, 379));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 26, 133, 619, 386));
    this->windowMenueAnimationQuitTrackSelection->coordVec.push_back(irr::core::recti( 24, 132, 617, 385));

    //is shown during transition from race track selection to ship selection and vice versa
    windowMenueAnimationBetweenRaceAndShipSelection = new MenueWindowAnimationVec();
    windowMenueAnimationBetweenRaceAndShipSelection->coordVec.clear();
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 24,  7, 617, 151));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 35,  7, 601, 151));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 66,  14, 571, 144));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 126,  25, 511, 136));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 173,  33, 470, 128));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 203,  38, 441, 122));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 263,  49, 381, 115));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 218,  43, 424, 123));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 175,  37, 470, 130));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 113,  24, 527, 139));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 71,  17, 574, 145));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 40,  11, 605, 149));
    this->windowMenueAnimationBetweenRaceAndShipSelection->coordVec.push_back(irr::core::recti( 24,  7, 617, 151));
}

void Menue::RenderWindow(irr::core::recti position) {
    //correct current position information with actually selected game resolution
    //specified locations in parameter are always based on 640x400 resolution game uses in VGA mode
    position.UpperLeftCorner.X = (position.UpperLeftCorner.X * mGame->mScreenRes.Width) / 640;
    position.LowerRightCorner.X = (position.LowerRightCorner.X * mGame->mScreenRes.Width) / 640;

    position.UpperLeftCorner.Y = (position.UpperLeftCorner.Y * mGame->mScreenRes.Height) / 400;
    position.LowerRightCorner.Y = (position.LowerRightCorner.Y * mGame->mScreenRes.Height) / 400;

    irr::core::recti backRectPos;
    backRectPos.UpperLeftCorner.X = position.UpperLeftCorner.X + 8;
    backRectPos.UpperLeftCorner.Y = position.UpperLeftCorner.Y + 8;
    backRectPos.LowerRightCorner.X = position.LowerRightCorner.X - 10;
    backRectPos.LowerRightCorner.Y = position.LowerRightCorner.Y - 9;

    //draw window background (half transparent)
    mGame->mDriver->draw2DRectangle(irr::video::SColor(60,103,174,145), backRectPos);

    //draw left upper corner graphics element
    mGame->mDriver->draw2DImage(wndCornerElementUpperLeftTex, position.UpperLeftCorner,
          irr::core::rect<irr::s32>(0,0, wndCornerElementUpperLeftTex->getSize().Width, wndCornerElementUpperLeftTex->getSize().Height), 0,
          irr::video::SColor(255,255,255,255), true);

    //draw right upper corner graphics element
    irr::core::position2di coord;
    coord.X = position.LowerRightCorner.X - wndCornerElementUpperRightTex->getSize().Width;
    coord.Y = position.UpperLeftCorner.Y;

    mGame->mDriver->draw2DImage(wndCornerElementUpperRightTex, coord,
          irr::core::rect<irr::s32>(0,0, wndCornerElementUpperLeftTex->getSize().Width, wndCornerElementUpperLeftTex->getSize().Height), 0,
          irr::video::SColor(255,255,255,255), true);

    //draw right lower corner graphics element
    coord.X = position.LowerRightCorner.X - wndCornerElementLowerRightTex->getSize().Width;
    coord.Y = position.LowerRightCorner.Y - wndCornerElementLowerRightTex->getSize().Height;

    mGame->mDriver->draw2DImage(wndCornerElementLowerRightTex, coord,
          irr::core::rect<irr::s32>(0,0, wndCornerElementLowerRightTex->getSize().Width, wndCornerElementLowerRightTex->getSize().Height), 0,
          irr::video::SColor(255,255,255,255), true);

    //draw left lower corner graphics element
    coord.X = position.UpperLeftCorner.X;
    coord.Y = position.LowerRightCorner.Y - wndCornerElementLowerLeftTex->getSize().Height;

    mGame->mDriver->draw2DImage(wndCornerElementLowerLeftTex, coord,
          irr::core::rect<irr::s32>(0,0, wndCornerElementLowerLeftTex->getSize().Width, wndCornerElementLowerLeftTex->getSize().Height), 0,
          irr::video::SColor(255,255,255,255), true);

    //Draw upper and left line around window
    irr::video::SColor UpperLeftLineColor(255, 167, 191, 193);
    irr::core::position2di coord2;
    //upper line
    coord.X = position.UpperLeftCorner.X + wndCornerElementUpperLeftTex->getSize().Width;
    coord.Y = position.UpperLeftCorner.Y + 8;
    coord2.Y = coord.Y;
    coord2.X = position.LowerRightCorner.X - wndCornerElementUpperRightTex->getSize().Width - 1;
    mGame->mDriver->draw2DLine(coord, coord2, UpperLeftLineColor);
    //left line
    coord.X = position.UpperLeftCorner.X + 9;
    coord.Y = position.UpperLeftCorner.Y + wndCornerElementUpperLeftTex->getSize().Height;
    coord2.X = coord.X;
    coord2.Y = position.LowerRightCorner.Y - wndCornerElementLowerLeftTex->getSize().Height;
    mGame->mDriver->draw2DLine(coord, coord2, UpperLeftLineColor);

    //Draw lower and right line around window
    irr::video::SColor LowerRightLineColor(255, 79, 89, 83);
    //lower line
    coord.X = position.UpperLeftCorner.X + wndCornerElementLowerLeftTex->getSize().Width;
    coord.Y = position.LowerRightCorner.Y - 10;
    coord2.Y = coord.Y;
    coord2.X = position.LowerRightCorner.X - wndCornerElementLowerRightTex->getSize().Width - 1;
    mGame->mDriver->draw2DLine(coord, coord2, LowerRightLineColor);
    //right line
    coord.X = position.LowerRightCorner.X - 10;
    coord.Y = position.UpperLeftCorner.Y + 9;
    coord2.X = coord.X;
    coord2.Y = position.LowerRightCorner.Y - 10;
    mGame->mDriver->draw2DLine(coord, coord2, LowerRightLineColor);
}

void Menue::RenderCursor(MenueSingleEntry* textEntryField) {
    //text cursor is simply a greenish filled rectangle on the correct position
    irr::core::recti cursorPos(0,0,0,0);

    //calculate correct position
    cursorPos.UpperLeftCorner.X = textEntryField->drawTextScrPosition.X +
            mGame->mGameTexts->GetWidthPixelsGameText(textEntryField->currTextInputFieldStr,
                     textEntryField->usedSelectedTextFont) + 3;

    cursorPos.UpperLeftCorner.Y = textEntryField->drawTextScrPosition.Y + 1;

    //make cursor 18 x 18 pixels
    cursorPos.LowerRightCorner.X = cursorPos.UpperLeftCorner.X + 18;
    cursorPos.LowerRightCorner.Y = cursorPos.UpperLeftCorner.Y + 18;

    //draw cursor with greenish color
    this->mGame->mDriver->draw2DRectangle(irr::video::SColor(255, 97, 165, 145), cursorPos);
}

irr::u32 Menue::GetNrofCharactersForMenueItem(MenueSingleEntry* whichMenueEntry) {
    if (whichMenueEntry != nullptr) {
        //is this a slider/checkbox object
        if (whichMenueEntry->maxValue != 0) {
            //yes is a checkbox/slider; Each block counts as one character
            //in the original game (number of blocks stored in checkBoxNrBlocks!)
            //we also have to add the items text length itself as well (see entryText)
            return ((irr::u32)(strlen(whichMenueEntry->entryText)) + whichMenueEntry->checkBoxNrBlocks);
        }

        //is a simple menue entry items
        return ((irr::u32)(strlen(whichMenueEntry->entryText)));
    }

    return 0;
}

irr::u32 Menue::GetNrOfCharactersOnMenuePage(MenuePage* whichMenuePage) {
    irr::u32 nrCharsResult = 0;
    std::vector<MenueSingleEntry*>::iterator it;

    if (whichMenuePage != nullptr) {
        //iterate through all menue entries on specified MenuePage
        for (it = whichMenuePage->pageEntryVec.begin(); it != whichMenuePage->pageEntryVec.end(); ++it) {
            //add items number of characters to overall number of characters for page
            nrCharsResult += GetNrofCharactersForMenueItem((*it));
        }

        return nrCharsResult;
    }

    return 0;
}

void Menue::PrintMenueEntries() {
    //is there currently a menue page selected?
    if (currSelMenuePage != nullptr) {
        //print text from currently selected menue page item by item
        std::vector<MenueSingleEntry*>::iterator it;
        irr::s32 printCharLeft = currNrCharsShownCnter;

        //if we do not use type write effect deactivate it
        //here for rendering
        if (!MENUE_ENABLETYPEWRITEREFFECT) {
            printCharLeft = -1;
        }

        //any entries here to print?
        if (currSelMenuePage->pageEntryVec.size() > 0) {
            for (it = currSelMenuePage->pageEntryVec.begin(); it != currSelMenuePage->pageEntryVec.end(); ++it) {
                //if current item to print is currently selected menue entry item print it in white text color
                if ((currSelMenueSingleEntry->entryNumber == (*it)->entryNumber) && ((*it)->isTextEntryField == false)) {
                    mGame->mGameTexts->DrawGameText((*it)->entryText, (*it)->usedSelectedTextFont, (*it)->drawTextScrPosition, printCharLeft);

                    if (MENUE_ENABLETYPEWRITEREFFECT) {
                        //decrease number of characters left for printing in this rendering run (type writer effect) of game
                        printCharLeft -= GetNrofCharactersForMenueItem((*it));

                        if (printCharLeft < 0)
                            printCharLeft = 0;
                    }

                    //do we need to additional render a checkbox/slider?
                    if ((*it)->maxValue != 0) {
                        //render selected item with black/white colors
                        RenderCheckBox((*it), (*it)->checkBoxOutline, irr::video::SColor(255, 254, 254, 250), irr::video::SColor(255, 4, 4, 8), (*it)->checkBoxNrBlocks, printCharLeft);
                    }
                } else {
                        //item is currently not selected, draw in "greenish" color, or is a text input field entry item (is also green in original game)
                        mGame->mGameTexts->DrawGameText((*it)->entryText, (*it)->usedUnselectedTextFont, (*it)->drawTextScrPosition, printCharLeft);

                        if (MENUE_ENABLETYPEWRITEREFFECT) {
                            //decrease number of characters left for printing in this rendering run (type writer effect) of game
                            printCharLeft -= GetNrofCharactersForMenueItem((*it));

                            if (printCharLeft < 0)
                                 printCharLeft = 0;
                        }

                        //do we need to additional render a checkbox/slider?
                        if ((*it)->maxValue != 0) {
                            //non selected item, render checkbox/slider with greenish/darkish colors
                            RenderCheckBox((*it), (*it)->checkBoxOutline, irr::video::SColor(255, 97, 165, 145), irr::video::SColor(255, 40, 65, 56), (*it)->checkBoxNrBlocks, printCharLeft);
                        }

                        //if this is in fact a text input entry field render cursor
                        if ((*it)->isTextEntryField && this->blinkTextCursorVisible) {
                          RenderCursor(*it);
                        }
                  }
            }
        }
    }
}

void Menue::PrintMenueEntriesShipSelection() {
    //is there currently a menue page selected?
    if (currSelMenuePage != nullptr) {
            irr::s32 printCharLeft = currNrCharsShownCnter;

            //if we do not use type write effect deactivate it
            //here for rendering
            if (!MENUE_ENABLETYPEWRITEREFFECT) {
                printCharLeft = -1;
            }

          mGame->mGameTexts->DrawGameText(currSelShipName, mGame->mGameTexts->GameMenueSelectedItemFont, this->ShipNameTitle->drawTextScrPosition, printCharLeft);
          printCharLeft -= (irr::u32)(strlen(currSelShipName));
          if (printCharLeft < 0)
              printCharLeft = 0;

          //print current name of selected ship color scheme
          mGame->mGameTexts->DrawGameText(currSelShipColorSchemeName, mGame->mGameTexts->GameMenueWhiteTextSmallSVGA, currSelectedShipColorSchemeTextPos, printCharLeft);
          printCharLeft -= (irr::u32)(strlen(currSelShipColorSchemeName));
          if (printCharLeft < 0)
              printCharLeft = 0;

          //render the current selected ship stats
          mGame->mGameTexts->DrawGameText(ShipStatSpeedLabel->text, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA, ShipStatSpeedLabel->drawPositionTxt, printCharLeft);
          printCharLeft -= (irr::u32)(strlen(ShipStatSpeedLabel->text));
          if (printCharLeft < 0)
              printCharLeft = 0;

          this->RenderShipStatBoxes(ShipStatSpeedLabel->statBoxOutline, irr::video::SColor(255, 97, 165, 145), irr::video::SColor(255, 4, 4, 8),
                                     ShipStatSpeedLabel->statNrBlocks, printCharLeft);
          printCharLeft -= ShipStatSpeedLabel->statNrBlocks;
          if (printCharLeft < 0)
              printCharLeft = 0;

          mGame->mGameTexts->DrawGameText(ShipStatArmourLabel->text, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA, ShipStatArmourLabel->drawPositionTxt, printCharLeft);
          printCharLeft -= (irr::u32)(strlen(ShipStatArmourLabel->text));
          if (printCharLeft < 0)
              printCharLeft = 0;

          this->RenderShipStatBoxes(ShipStatArmourLabel->statBoxOutline, irr::video::SColor(255, 97, 165, 145), irr::video::SColor(255, 4, 4, 8),
                                     ShipStatArmourLabel->statNrBlocks, printCharLeft);
          printCharLeft -= ShipStatArmourLabel->statNrBlocks;
          if (printCharLeft < 0)
              printCharLeft = 0;

          mGame->mGameTexts->DrawGameText(ShipStatWeightLabel->text, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA, ShipStatWeightLabel->drawPositionTxt, printCharLeft);
          printCharLeft -= (irr::u32)(strlen(ShipStatWeightLabel->text));
          if (printCharLeft < 0)
              printCharLeft = 0;

          this->RenderShipStatBoxes(ShipStatWeightLabel->statBoxOutline, irr::video::SColor(255, 97, 165, 145), irr::video::SColor(255, 4, 4, 8),
                                     ShipStatWeightLabel->statNrBlocks, printCharLeft);
          printCharLeft -= ShipStatWeightLabel->statNrBlocks;
          if (printCharLeft < 0)
              printCharLeft = 0;

          mGame->mGameTexts->DrawGameText(ShipStatFirePowerLabel->text, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA, ShipStatFirePowerLabel->drawPositionTxt, printCharLeft);
          printCharLeft -= (irr::u32)(strlen(ShipStatFirePowerLabel->text));
          if (printCharLeft < 0)
              printCharLeft = 0;

          this->RenderShipStatBoxes(ShipStatFirePowerLabel->statBoxOutline, irr::video::SColor(255, 97, 165, 145), irr::video::SColor(255, 4, 4, 8),
                                     ShipStatFirePowerLabel->statNrBlocks, printCharLeft);
          printCharLeft -= ShipStatFirePowerLabel->statNrBlocks;
          if (printCharLeft < 0)
              printCharLeft = 0;
    }
}

//  renderOnlyNumberBlocks = optional parameter (default is -1 which means feature inactive)
//                           Only renders specified number of blocks (this feature is needed
//                           and used for type writer effect in menue which also the original game has
void Menue::RenderCheckBox(MenueSingleEntry* entry, irr::core::recti position, irr::video::SColor colorRect, irr::video::SColor lineColor,
                           irr::u8 nrBlocks, irr::s8 renderOnlyNumberBlocks) {

    //if no blocks should be currently rendered just exit straight away
    if ((renderOnlyNumberBlocks > 0) || (renderOnlyNumberBlocks == -1)) {
        //calculate pixelDist for each block (amount of pixels per block)
        irr::f32 pixelDist = 
            ((irr::f32)(position.LowerRightCorner.X) - (irr::f32)(position.UpperLeftCorner.X)) / (irr::f32)(nrBlocks);
        irr::u8 drawNrBlocks = entry->checkBoxNrBlocks;

        //if we do not render all blocks (type writer effect) we need to shorten the outline as
        //well
        if ((renderOnlyNumberBlocks < entry->checkBoxNrBlocks) && (renderOnlyNumberBlocks != -1)) {
            position.LowerRightCorner.X -= 
                (irr::s32)(pixelDist * (irr::f32)(entry->checkBoxNrBlocks - renderOnlyNumberBlocks));
            drawNrBlocks = renderOnlyNumberBlocks;
        }

        //step 1: draw outline of whole element using lineColor
        mGame->mDriver->draw2DRectangleOutline(position, lineColor);

        //step 2: draw vertical lines along overall element so that
        //we seperate the element into nrSeperations pieces, also with lineColor

        irr::f32 currXcoordFloat = (irr::f32)(position.UpperLeftCorner.X) + pixelDist;
        irr::u32 currXcoord = (irr::u32)(currXcoordFloat);

        irr::core::vector2d<irr::s32> startPnt(0, position.UpperLeftCorner.Y);
        irr::core::vector2d<irr::s32> endPnt(0, position.LowerRightCorner.Y);

        for (irr::u8 idx = 0; idx < drawNrBlocks; idx++) {
            startPnt.X = currXcoord;
            endPnt.X = currXcoord;

            mGame->mDriver->draw2DLine(startPnt, endPnt, lineColor);
            currXcoordFloat += pixelDist;
            currXcoord = (irr::u32)(currXcoordFloat);
        }

        //step 3: how many blocks need to be filled out?
        //draw filled and unfilled blocks in one operation
        irr::f32 currXcoord2;

        irr::u8 filledBlocks = (irr::u8)((entry->currValue * nrBlocks) / (entry->maxValue));
        currXcoordFloat = (irr::f32)(position.UpperLeftCorner.X);
        currXcoord = (irr::u32)(currXcoordFloat);
        currXcoord2 = currXcoordFloat + pixelDist;
        startPnt.Y = position.UpperLeftCorner.Y + 1;
        endPnt.Y = position.LowerRightCorner.Y - 1;
        irr::core::recti fillPos(0,0,0,0);
        irr::u8 cntFilledBlocks = 0;

        for (irr::u8 idx = 0; idx < drawNrBlocks; idx++) {
            startPnt.X = currXcoord + 1;
            endPnt.X = (irr::s32)(currXcoord2) - 1;
            fillPos.UpperLeftCorner = startPnt;
            fillPos.LowerRightCorner = endPnt;

            if (cntFilledBlocks < filledBlocks) {
                //filled block
                mGame->mDriver->draw2DRectangle(colorRect, fillPos);
            } else {
                //unfilled block (only draw outline of rect)
                mGame->mDriver->draw2DRectangleOutline(fillPos, colorRect);
            }

            cntFilledBlocks++;

            currXcoordFloat += pixelDist;
            currXcoord = (irr::u32)(currXcoordFloat);
            currXcoord2 = currXcoordFloat + pixelDist;
        }
    }
}

//  renderOnlyNumberBlocks = optional parameter (default is -1 which means feature inactive)
//                           Only renders specified number of blocks (this feature is needed
//                           and used for type writer effect in menue which also the original game has
void Menue::RenderShipStatBoxes(irr::core::recti position, irr::video::SColor colorRect, irr::video::SColor lineColor,
                           irr::u8 nrBlocks, irr::s8 renderOnlyNumberBlocks) {

    //if no blocks should be currently rendered just exit straight away
    if ((renderOnlyNumberBlocks > 0) || (renderOnlyNumberBlocks == -1)) {
        //define pixelDist for each block (amount of pixels per block)
        irr::f32 pixelDist = 8.0f;
        irr::u8 drawNrBlocks = nrBlocks;

        //if we do not render all blocks (type writer effect) we need to shorten the outline as
        //well
        if ((renderOnlyNumberBlocks < drawNrBlocks) && (renderOnlyNumberBlocks != -1)) {
            position.LowerRightCorner.X -= (irr::s32)(pixelDist * (irr::f32)(drawNrBlocks - renderOnlyNumberBlocks));
            drawNrBlocks = renderOnlyNumberBlocks;
        }

        //at the beginning we need to recalculate the position from original game menue 640x400 resolution
        //to our set game screen resolution
      /*  position.UpperLeftCorner.X = (position.UpperLeftCorner.X * screenResolution.Width) / 640;
        position.UpperLeftCorner.Y = (position.UpperLeftCorner.Y * screenResolution.Height) / 400;
        position.LowerRightCorner.X = (position.LowerRightCorner.X * screenResolution.Width) / 640;
        position.LowerRightCorner.Y = (position.LowerRightCorner.Y * screenResolution.Height) / 400;*/

        //step 1: draw outline of whole element using lineColor
        mGame->mDriver->draw2DRectangleOutline(position, lineColor);

        //step 2: draw vertical lines along overall element so that
        //we seperate the element into nrSeperations pieces, also with lineColor

        irr::f32 currXcoordFloat = position.UpperLeftCorner.X + pixelDist;
        irr::u32 currXcoord = (irr::u32)(currXcoordFloat);

        irr::core::vector2d<irr::s32> startPnt(0, position.UpperLeftCorner.Y);
        irr::core::vector2d<irr::s32> endPnt(0, position.LowerRightCorner.Y);

        for (irr::u8 idx = 0; idx < drawNrBlocks; idx++) {
            startPnt.X = currXcoord;
            endPnt.X = currXcoord;

            mGame->mDriver->draw2DLine(startPnt, endPnt, lineColor);
            currXcoordFloat += pixelDist;
            currXcoord = (irr::u32)(currXcoordFloat);
        }

        //step 3: how many blocks need to be filled out?
        //draw filled and unfilled blocks in one operation
        irr::f32 currXcoord2;

        currXcoordFloat = (irr::f32)(position.UpperLeftCorner.X);
        currXcoord = (irr::u32)(currXcoordFloat);
        currXcoord2 = currXcoordFloat + pixelDist;
        startPnt.Y = position.UpperLeftCorner.Y + 1;
        endPnt.Y = position.LowerRightCorner.Y - 1;
        irr::core::recti fillPos(0,0,0,0);

        for (irr::u8 idx = 0; idx < drawNrBlocks; idx++) {
            startPnt.X = currXcoord + 1;
            endPnt.X = (irr::s32)(currXcoord2) - 1;
            fillPos.UpperLeftCorner = startPnt;
            fillPos.LowerRightCorner = endPnt;

            //here we want to fill all blocks anyway
            mGame->mDriver->draw2DRectangle(colorRect, fillPos);

            currXcoordFloat += pixelDist;
            currXcoord = (irr::u32)(currXcoordFloat);
            currXcoord2 = currXcoordFloat + pixelDist;
        }
    }
}

void Menue::AdvanceTime(irr::f32 frameDeltaTime) {
    //frameDeltaTime is in seconds

    //advance my time
    absoluteTime += frameDeltaTime;

    //logic for text input cursor (we do not care here if it is actually currently visible
    //on the screen)
    //time for new cursor state change?
    if (absoluteTime >= blinkTextCursorNextStateChangeAbsTime) {
        //calculate next time for the necessary change
        blinkTextCursorNextStateChangeAbsTime  = (absoluteTime + MENUE_TEXTENTRY_CURSORBLINKPERIODSEC);

        //change cursor state
        if (!blinkTextCursorVisible)
            blinkTextCursorVisible = true;
        else blinkTextCursorVisible = false;
    }

    //timing for menue typewriter effect
    //run this logic only in the correct window state
    if (currMenueState == MENUE_STATE_TYPETEXT) {
        if (absoluteTime > typeWriterEffectNextCharacterAbsTime) {
            //calculate next time for typing next character
            typeWriterEffectNextCharacterAbsTime  = (absoluteTime + MENUE_TYPEWRITEREFFECT_TYPEDELAY);

            if (currNrCharsShownCnter < finalNrChardsShownMenuePageFinished) {
                //we need to type faster, because otherwise menue print does
                //take way to long
                currNrCharsShownCnter = currNrCharsShownCnter + 3;

                PlayMenueSound(SRES_MENUE_TYPEWRITEREFFECT1);
            } else {
                //we are done with type writer effect
                //set menue state to next state
                currMenueState = MENUE_STATE_SELACTIVE;

                PlayMenueSound(SRES_MENUE_TYPEWRITEREFFECT2);
            }
        }
    }

    //handle 3D object updates
    if (currSelMenuePage != nullptr) {
        if ((currSelMenuePage == RaceTrackSelectionPage) || (currSelMenuePage == ShipSelectionPage)) {
           if (absoluteTime > last3DModelUpdateAbsTime) {
               last3DModelUpdateAbsTime = (absoluteTime + MENUE_3DMODEL_UPDATEPERIODSEC);

               if (currSelMenuePage == RaceTrackSelectionPage) {
                    //if race track model selection "wheel" is turning update
                    //model positions
                    Update3DModelRaceTrackWheelPosition();
               }

               if (currSelMenuePage == ShipSelectionPage) {
                   //if ship model selection "wheel" is turning update
                   //model positions
                   Update3DModelShipWheelPosition();
               }

               //update 3D Model positions + rotations (race track + ship models)
               Update3DModels();
           }
        }
    }
}

//the Race selection part of the menue is so special/different to the remaining menue
//that it is better to implement it by itself
void Menue::RenderRaceSelection() {
    //first draw background picture, no logo
    DrawBackground(false);

    //Render the currently selected window depending on the current
    //window state (fully open vs. transitioning)
    RenderWindow(currSelWindowAnimation->coordVec[currMenueWindowAnimationIdx]);

    //in case of current window transition calculate next animation index
    //and also check if transistion is finished
    if ((currMenueState == MENUE_STATE_TRANSITION) && ((absoluteTime - lastAnimationUpdateAbsTime) > MENUE_WINDOW_ANIMATION_PERIODTIME)) {
        currMenueWindowAnimationIdx++;

        //play sound that is active during window movement
        PlayMenueSound(SRES_MENUE_WINDOWMOVEMENT);

        if (currMenueWindowAnimationIdx > currMenueWindowAnimationFinalIdx) {
            //window animation finished, window fully open, now print text and allow user selection
            currMenueWindowAnimationIdx = currMenueWindowAnimationFinalIdx;

            StopMenueSound();

            //set current menue space
            currMenueSpace = currSelWindowAnimation->coordVec[currMenueWindowAnimationIdx];

            //Realign layout of menue page
            RealignMenuePageItems();

            if (currSelMenuePage == ShipSelectionPage) {
                FinalPositionShipSelectionWheelReached(currSelectedShip);

                RecalculateShipStatLabels();
                UpdateShipSelectionTypeWriterEffect();
            }

            if (currSelMenuePage == RaceTrackSelectionPage) {
                FinalPositionRaceTrackWheelReached(currSelectedRaceTrack);

            }

            currMenueState = MENUE_STATE_TYPETEXT;
        }

         lastAnimationUpdateAbsTime = absoluteTime;
    }

    //only print menue items if window is fully open right now
    if ((currMenueState == MENUE_STATE_SELACTIVE) || (currMenueState == MENUE_STATE_TYPETEXT)) {
        if (currSelMenuePage == RaceTrackSelectionPage)
            PrintMenueEntries();
        else if (currSelMenuePage == ShipSelectionPage)
           PrintMenueEntriesShipSelection();
    }

    //draw all 3D models
    this->smgrMenue->drawAll();
}

void Menue::SetRotation3DModelAroundYAxis(irr::scene::ISceneNode* sceneObj, irr::f32 rotDeg) {
    core::vector3df yaxis(0.0f, 1.0f, 0.0f); // yaxis in world space
    core::quaternion q;
    q.fromAngleAxis(rotDeg *core::DEGTORAD, yaxis);

    //rotate selected 3D model
    core::matrix4 matrix = sceneObj->getAbsoluteTransformation();
    matrix.rotateVect(yaxis); // yaxis in local space
    yaxis.normalize();

    core::vector3df rot = q.getMatrix().getRotationDegrees();
    sceneObj->setRotation(rot);
}

void Menue::Update3DModels() {
    //rotate race track and ship models around their Y-AXIS
    //objects are rotating counter clock wise around
    //Y axis
    curr3DModelRotationDeg -= 1.5f;

    if (curr3DModelRotationDeg > 360.0f)
        curr3DModelRotationDeg -= 360.0f;

    if (curr3DModelRotationDeg < 0.0f)
        curr3DModelRotationDeg += 360.0f;

    if (currSelMenuePage != nullptr) {
        if (currSelMenuePage == RaceTrackSelectionPage) {
            //rotate 3D selection models of all available race tracks
            for (irr::u8 i = 0; i < this->numRaceTracksAvailable; i++) {
                SetRotation3DModelAroundYAxis(this->ModelTrackSceneNodeVec.at(i), curr3DModelRotationDeg);
            }
        } else if (currSelMenuePage == ShipSelectionPage) {
            //update models for all colorschemes
            for (irr::u8 j = 0; j < this->numCraftsAvailable; j++) {
                for (unsigned long i = 0; i < this->mGameAssets->mCraftColorSchemeNames.size(); i++) {
                    SetRotation3DModelAroundYAxis(this->ModelCraftsSceneNodeVec[j]->at(i), curr3DModelRotationDeg);
                }
            }
        }
    }
}

void Menue::UnhideRaceTrackModels() {
    //unhide all race track 3D selection models
    //if we are not in championship mode, and can select all available race tracks
    if (!mChampionshipMode) {
        for (irr::u8 i = 0; i < this->numRaceTracksAvailable; i++) {
              this->ModelTrackSceneNodeVec.at(i)->setVisible(true);
        }
    } else {
        //in championshipMode we only unhide the one racetrack we should
        //actually see, all other racetracks we hide
        for (irr::u8 i = 0; i < this->numRaceTracksAvailable; i++) {
              this->ModelTrackSceneNodeVec.at(i)->setVisible(false);
        }

        this->ModelTrackSceneNodeVec.at(currSelectedRaceTrack)->setVisible(true);
   }
}

void Menue::HideRaceTrackModels() {
    //hide all race track 3D selection models
    for (irr::u8 i = 0; i < this->numRaceTracksAvailable; i++) {
          this->ModelTrackSceneNodeVec.at(i)->setVisible(false);
    }
}

void Menue::UnhideShipModels() {
    for (irr::u8 i = 0; i < this->numCraftsAvailable; i++) {
        this->ModelCraftsSceneNodeVec[i]->at(currSelectedShipColorScheme)->setVisible(true);
    }
}

void Menue::HideShipModels() {
    //hide all available ship models (we have different
    //ship color schemes)
    for (irr::u8 j = 0; j < this->numCraftsAvailable; j++) {
        for (unsigned long i = 0; i < this->mGameAssets->mCraftColorSchemeNames.size(); i++) {
            this->ModelCraftsSceneNodeVec[j]->at(i)->setVisible(false);
        }
    }
}

void Menue::ShowShipSelection() {
    //hide race track models, otherwise we would also see them while
    //player ship selection
    HideRaceTrackModels();

    //restore last selected main player craft from assets class
    this->currSelectedShip = this->mGameAssets->GetMainPlayerSelectedCraft();

    //restore last selected craft color scheme
    this->currSelectedShipColorScheme = this->mGameAssets->GetCurrentCraftColorScheme();

    //put selection "wheel" to correct position immediately, no animation => parameter 3
    Set3DModelShipWheelPositionIdx(currSelectedShip, 3);

    //update selected craft color scheme
    SetShipColorScheme(currSelectedShipColorScheme);

    //This call is also needed afterwards to immediately set new wheel position
    Update3DModelShipWheelPosition();

    //unhide player ship models, we need to see them
    UnhideShipModels();

    //initially activate menue animation for start of game
    currSelWindowAnimation = windowMenueAnimationBetweenRaceAndShipSelection;
    currMenueWindowAnimationIdx = 0;
    currMenueWindowAnimationFinalIdx = (irr::u8)(windowMenueAnimationBetweenRaceAndShipSelection->coordVec.size()) - 1;
    currMenueState = MENUE_STATE_TRANSITION;
    lastAnimationUpdateAbsTime = absoluteTime;
}

//if parameter championshipMode is true, then race selection screen only allows to select exactly
//one preselected race track. Which race track is shown and selected depends on
//call this->mGameAssets->GetLastSelectedRaceTrack(). Also the number of laps for the following race does not depend on the
//user/player selection, but on the default number of laps for this racetrack. As in the original game.
void Menue::ShowRaceSelection(MenueSingleEntry* callerItem, bool championshipMode = false) {
    //remember from which menue item we were called, important!
    //so that we can return to the correct menue page later when
    //interrupting race track selection prematuraliy
    RaceTrackSelectionCallerMenueEntry = callerItem;

    mChampionshipMode = championshipMode;

    //which race track was last selected, restore
    this->currSelectedRaceTrack = this->mGameAssets->GetLastSelectedRaceTrack();

    if (!championshipMode) {
        //we are not in championShip mode
        //get current configured number of laps from config.dat
        this->currentSelRaceLapNumberVal =
                this->mGameAssets->mRaceTrackVec->at(this->currSelectedRaceTrack)->currSelNrLaps;
    } else {
        //we are in championShip mode
        //we always use default number of laps for the
        //race
        this->currentSelRaceLapNumberVal =
                this->mGameAssets->mRaceTrackVec->at(this->currSelectedRaceTrack)->defaultNrLaps;
    }

    //put selection "wheel" to correct position immediately, no animation => parameter 3
    Set3DModelRaceTrackWheelPositionIdx(currSelectedRaceTrack, 3);
    //This call is also needed afterwards to immediately set new wheel position
    Update3DModelRaceTrackWheelPosition();

    //unhide Race track models, we need to see them
    UnhideRaceTrackModels();

    //Hide the player ship models, otherwise we would also see thim
    //in the race track selection screen
    HideShipModels();

    //change current page to race track selection
    currSelMenuePage = RaceTrackSelectionPage;
    currSelMenueSingleEntry = RaceTrackNameTitle;

    //activate window animation for change of main menue to race selection page
    currSelWindowAnimation = windowMenueAnimationBeforeTrackSelection;
    currMenueWindowAnimationIdx = 0;
    currMenueWindowAnimationFinalIdx = (irr::u8)(windowMenueAnimationBeforeTrackSelection->coordVec.size()) - 1;
    currMenueState = MENUE_STATE_TRANSITION;
    lastAnimationUpdateAbsTime = absoluteTime;
}

void Menue::InterruptRaceSelection() {
    //go back to page from where we have initially called the race
    //track selection page
    if (RaceTrackSelectionCallerMenueEntry == this->SelectSingleRace) {
        currSelMenuePage = RaceMenuePage;
        currSelMenueSingleEntry = SelectSingleRace;
    } else if (RaceTrackSelectionCallerMenueEntry == this->NewChampionshipEntry) {
        currSelMenuePage = ChampionshipMenuePage;
        currSelMenueSingleEntry = NewChampionshipEntry;
    } else if (RaceTrackSelectionCallerMenueEntry == this->ContinueChampionshipEntry) {
        currSelMenuePage = ChampionshipMenuePage;
        currSelMenueSingleEntry = ContinueChampionshipEntry;
    }

    //hide race track models again
    HideRaceTrackModels();

    //activate window animation for change of main menue to race selection page
    currSelWindowAnimation = windowMenueAnimationQuitTrackSelection;
    currMenueWindowAnimationIdx = 0;
    currMenueWindowAnimationFinalIdx = (irr::u8)(windowMenueAnimationQuitTrackSelection->coordVec.size()) - 1;
    currMenueState = MENUE_STATE_TRANSITION;
    lastAnimationUpdateAbsTime = absoluteTime;

    //restore last selected race track number
    currSelectedRaceTrack = this->mGameAssets->GetLastSelectedRaceTrack();
}

void Menue::HandleUserInactiveTimer(irr::f32 frameDeltaTime) {
    //just increase inactive time in top menue page
    if (currSelMenuePage != TopMenuePage)
        return;

    mMenueUserInactiveTimer += frameDeltaTime;

    //user was long enough inactive?
    if (mMenueUserInactiveTimer > MENUE_USERINACTIVETIME_THRESHOLD) {
        //yes, trigger next defined action automatically
        mMenueUserInactiveTimer = 0.0f;

        //create random number to decide between 0 = run demo, and
        //1 = show high score page
        int rNum;
        rNum = mGame->randRangeInt(0, 1);

        if (rNum == 0) {
            //we want to trigger a demo because player was inactive in the menue
            //for a longer time
            //set here currValue to 0, because this is not a checkbox/slider action
            ActStartDemo->currSetValue = 0;
            currActionToExecute = ActStartDemo;
        } else {
            if (rNum == 1) {
                //we want to show the highscore page because player was inactive in the menue
                //for a longer time
                //set here currValue to 0, because this is not a checkbox/slider action
                ActShowHighScorePage->currSetValue = 0;
                currActionToExecute = ActShowHighScorePage;
            }
        }
    }
}

void Menue::DrawBackground(bool addLogo) {
   //first draw background picture
   mGame->mDriver->draw2DImage(mGame->backgnd, irr::core::vector2di(0, 0),
                         irr::core::recti(0, 0, mGame->mScreenRes.Width, mGame->mScreenRes.Height)
                         , 0, irr::video::SColor(255,255,255,255), true);

   if (addLogo) {
       //draw game logo
       int sizeVec = (int)(GameLogo.size());

       for (int i = 0; i < sizeVec ; i++) {
           mGame->mDriver->draw2DImage(GameLogo[i]->texture, GameLogo[i]->drawScrPosition,
             irr::core::rect<irr::s32>(0,0, GameLogo[i]->sizeTex.Width, GameLogo[i]->sizeTex.Height), 0,
             irr::video::SColor(255,255,255,255), true);
       }

       //add my additional logo text
       mGame->mGameTexts->DrawGameText((char*)"202X", mGame->mGameTexts->HudBigGreenText, mLogoExtensionStrPos);
   }
}

void Menue::Render(irr::f32 frameDeltaTime) {
    //racetrack selection and ship selection menue is so different, render it independently
    if ((this->currSelMenuePage == RaceTrackSelectionPage) || (this->currSelMenuePage == ShipSelectionPage)) {
        //call racetrack and ship selection rendering source code
        RenderRaceSelection();
    } else if ((this->currSelMenuePage == gameHighscoreMenuePage) || (this->currSelMenuePage == raceStatsMenuePage)
               || (this->currSelMenuePage == pointsTablePage)) {
        RenderStatTextPage(frameDeltaTime);
    } else {
     //default menue rendering source code

     //first draw background picture, logo included
     DrawBackground(true);

     //Render the currently selected window depending on the current
     //window state (fully open vs. transitioning)
     RenderWindow(currSelWindowAnimation->coordVec[currMenueWindowAnimationIdx]);

     //in case of current window transition calculate next animation index
     //and also check if transistion is finished
     if ((currMenueState == MENUE_STATE_TRANSITION) && ((absoluteTime - lastAnimationUpdateAbsTime) > MENUE_WINDOW_ANIMATION_PERIODTIME)) {
           currMenueWindowAnimationIdx++;

            //play sound that is active during window movement
            PlayMenueSound(SRES_MENUE_WINDOWMOVEMENT);

            if (currMenueWindowAnimationIdx > currMenueWindowAnimationFinalIdx) {
                //window animation finished, window fully open, now print text and allow user selection
                currMenueWindowAnimationIdx = currMenueWindowAnimationFinalIdx;

                StopMenueSound();

                //set current menue space
                currMenueSpace = currSelWindowAnimation->coordVec[currMenueWindowAnimationIdx];

                //Realign layout of menue page
                RealignMenuePageItems();

                currMenueState = MENUE_STATE_TYPETEXT;
            }

             lastAnimationUpdateAbsTime = absoluteTime;
        }

        //only print menue items if window is fully open right now
        if ((currMenueState == MENUE_STATE_SELACTIVE) || (currMenueState == MENUE_STATE_TYPETEXT)) {
            PrintMenueEntries();
        }
    }
}

//this render function is used for the race finished stat page,
//the point table pages, and the highscore page
//(which contains just formated texts)
void Menue::RenderStatTextPage(irr::f32 frameDeltaTime) {
    //first draw background picture, no logo
    DrawBackground(false);

    if (currSelMenuePage == gameHighscoreMenuePage) {
        //Render the currently selected window depending on the current
        //window state (fully open vs. transitioning)
        RenderWindow(irr::core::recti(24, 23, 619, 360));
    } else if (currSelMenuePage == raceStatsMenuePage) {
        //Render the currently selected window depending on the current
        //window state (fully open vs. transitioning)
        RenderWindow(irr::core::recti(0, 0, 640, 395/*334*/));
    } else if (currSelMenuePage == pointsTablePage) {
        //Render the currently selected window depending on the current
        //window state (fully open vs. transitioning)
        RenderWindow(irr::core::recti(25, 18, 618, 360));
    }

    irr::s32 printCharLeft = currNrCharsShownCnter;

    //is the complete page already rendered, if so sum up overall
    //time this page is already shown to the user
    if (currMenueState == MENUE_STATE_SELACTIVE) {
        if (currSelMenuePage == gameHighscoreMenuePage) {

            absTimeElapsedAtHighScorePage += frameDeltaTime;

            //if we reach 10 seconds close this page again
            if (absTimeElapsedAtHighScorePage > 10.0f) {
                CleanupHighScorepage();
                return;
            }
        }
    }

    //if we do not use type write effect deactivate it
    //here for rendering
    if (!MENUE_ENABLETYPEWRITEREFFECT) {
        printCharLeft = -1;
    }

    //Draw overall text
    std::vector<MenueTextLabel*>::iterator it;
    std::vector<MenueTextLabel*>* lblVecPntr = nullptr;

    if (currSelMenuePage == gameHighscoreMenuePage) {
        lblVecPntr = highScorePageTextVec;
    } else if (currSelMenuePage == raceStatsMenuePage) {
        lblVecPntr = raceStatsPageTextVec;
    } else if (currSelMenuePage == pointsTablePage) {
        lblVecPntr = pointsTablePageTextVec;
    }

    if (lblVecPntr != nullptr) {
        for (it = lblVecPntr->begin(); it != lblVecPntr->end(); ++it) {
            mGame->mGameTexts->DrawGameText(
                        (*it)->text,
                        (*it)->whichFont,
                        (*it)->drawPositionTxt, printCharLeft);

            if (MENUE_ENABLETYPEWRITEREFFECT) {
                //decrease number of characters left for printing in this rendering run (type writer effect) of game
                printCharLeft -= (irr::u32)(strlen((*it)->text));

                if (printCharLeft < 0) {
                    printCharLeft = 0;
                }
            }
        }
   }
}

irr::u32 Menue::MaxMenueEntryNumber(MenuePage* inputPage) {
    irr::u32 maxEntryNum = 0;

    //only run this calculation when we have at least one item in the vector
    //otherwise we crash
    if (inputPage->pageEntryVec.size() > 0) {
        std::vector<MenueSingleEntry*>::iterator it;
        for (it = inputPage->pageEntryVec.begin(); it != inputPage->pageEntryVec.end(); ++it) {
            if ((*it)->entryNumber > maxEntryNum) {
                maxEntryNum = (*it)->entryNumber;
            }
        }
    } else
        return 0;

    return maxEntryNum;
}

//Returns nullptr if no applicable menue single entry object is found
MenueSingleEntry* Menue::GetMenueSingleEntryForEntryNr(MenuePage* newMenuePage, irr::u32 inputEntryNr) {
    //only run this operation when we have at least one item in the vector
    //otherwise we crash
    if (newMenuePage->pageEntryVec.size() > 0) {
        std::vector<MenueSingleEntry*>::iterator it;
        for (it = newMenuePage->pageEntryVec.begin(); it != newMenuePage->pageEntryVec.end(); ++it) {
            if ((*it)->entryNumber == inputEntryNr) {
                //we found the matching menue item, return with pointer
                return(*it);
            }
        }
    }

    return nullptr;
}

void Menue::SelectNewMenueEntryWithEntryNr(MenuePage* newMenuePage, irr::u32 nextSelEntryNr) {  
    //only run this operation when we have at least one item in the vector
    //otherwise we crash
    if (newMenuePage->pageEntryVec.size() > 0) {
        std::vector<MenueSingleEntry*>::iterator it;
        for (it = newMenuePage->pageEntryVec.begin(); it != newMenuePage->pageEntryVec.end(); ++it) {
            if ((*it)->entryNumber == nextSelEntryNr) {
                //we found the matching menue item, change selected item
                currSelMenuePage = newMenuePage;
                currSelMenueSingleEntry = (*it);

                //Realign layout of menue page
                RealignMenuePageItems();

                //if this menue page has a text entry field
                //preset input text
                if (currSelMenueSingleEntry->isTextEntryField && currSelMenueSingleEntry->initTextPntr != nullptr) {
                    //present text field string from source pointer location
                    SetInputTextField(currSelMenueSingleEntry, currSelMenueSingleEntry->initTextPntr);
                }
            }
        }
    }
}

//in case no selectable entry is found simply return the first item (but this should
//never occur, only in case of bug in program!)
irr::u32 Menue::MenuePageFindFirstSelectableEntryNr(MenuePage* newMenuePage) {
    //for the specified menue page find the first selectable menue entry
    //then return it
    std::vector<MenueSingleEntry*>::iterator it;

    if (newMenuePage->pageEntryVec.size() > 0) {
        for (it = newMenuePage->pageEntryVec.begin(); it != newMenuePage->pageEntryVec.end(); ++it) {
            if ((*it)->itemSelectable) {
                //we found first selectable item, return its entry number
                return((*it)->entryNumber);
            }
        }
    }

    //nothing found, simply return first item of menue page (should never occur!)
    //only if the programmer sets a menue dialog/window wrong up!
    it = newMenuePage->pageEntryVec.begin();
    return((*it)->entryNumber);
}

void Menue::ItemLeft() {
    //item currently selected?
   if (currSelMenueSingleEntry != nullptr) {
       //item has checkbox/slider functionality?
       if (currSelMenueSingleEntry->maxValue != 0) {           
           //play sound for changing value of checkbox/slider item
           PlayMenueSound(SRES_MENUE_CHANGECHECKBOXVAL);

           if (currSelMenueSingleEntry->currValue > 0) {
            currSelMenueSingleEntry->currValue--;
           }

           //trigger action is available to submit new value
           //to main program main loop
           if (currSelMenueSingleEntry->triggerAction != nullptr) {
               //we want to trigger/execute an action by the user
               currSelMenueSingleEntry->triggerAction->currSetValue = currSelMenueSingleEntry->currValue;
               currActionToExecute = currSelMenueSingleEntry->triggerAction;
           }
       }
   }
}

void Menue::ItemRight() {
    //item currently selected?
   if (currSelMenueSingleEntry != nullptr) {
       //item has checkbox/slider functionality?
       if (currSelMenueSingleEntry->maxValue != 0) {
           //play sound for changing value of checkbox/slider item
           PlayMenueSound(SRES_MENUE_CHANGECHECKBOXVAL);

           if (currSelMenueSingleEntry->currValue < currSelMenueSingleEntry->maxValue) {
            currSelMenueSingleEntry->currValue++;
           }

           //trigger action is available to submit new value
           //to main program main loop
           if (currSelMenueSingleEntry->triggerAction != nullptr) {
               //we want to trigger/execute an action by the user
               currSelMenueSingleEntry->triggerAction->currSetValue = currSelMenueSingleEntry->currValue;
               currActionToExecute = currSelMenueSingleEntry->triggerAction;
           }
       }
   }
}

void Menue::ItemUp() {
  //find current menue item entry number
  irr::s32 currEntryNr = currSelMenueSingleEntry->entryNumber;
  irr::s32 startSelItem = currEntryNr;
  MenueSingleEntry* entry;

  bool selectAbleItemFound = false;
  bool interrupt = false;

  irr::s32 maxEntryNr  = MaxMenueEntryNumber(currSelMenuePage);

  //play sound for changing selected menue item
  PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

  //we need to find the next selectable item on the menue page
  //we jump to the next one we find, if we reach the starting item (current
  //selected item) again, there is nothing else to select
  while ((selectAbleItemFound == false) && (interrupt == false)) {
      //try next item
      currEntryNr--;

      if (currEntryNr < 0) {
        //find currently highest entry number as we want to
        //flip to the last item in the current menue page
        currEntryNr = maxEntryNr;
      }

      if (currEntryNr == startSelItem) {
          //we reached the first element again, we need to interrupt, there is no other item
          //to select
          interrupt = true;
      } else {
          //find current menue single entry
          entry = GetMenueSingleEntryForEntryNr(currSelMenuePage, currEntryNr);
          //can we select this item, if so use it, otherwise go to the next one
          if (entry->itemSelectable) {
              selectAbleItemFound = true;
          }
       }
    }

  //if we found something new to select, select it
  if (!interrupt) {
    //select next item after player interaction
    SelectNewMenueEntryWithEntryNr(currSelMenuePage, currEntryNr);
  }
}

void Menue::ItemDown() {
    //find current menue item entry number
    irr::s32 currEntryNr = currSelMenueSingleEntry->entryNumber;
    irr::s32 startSelItem = currEntryNr;
    MenueSingleEntry* entry;

    bool selectAbleItemFound = false;
    bool interrupt = false;

    irr::s32 maxEntryNr = MaxMenueEntryNumber(currSelMenuePage);

    //play sound for changing selected menue item
    PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

    //we need to find the next selectable item on the menue page
    //we jump to the next one we find, if we reach the starting item (current
    //selected item) again, there is nothing else to select
    while ((selectAbleItemFound == false) && (interrupt == false)) {
        //try next item
        currEntryNr++;

        if (currEntryNr > maxEntryNr) {
            //we jumped over the last item,
            //select first item of current menue page
            currEntryNr = 0;
        }

        if (currEntryNr == startSelItem) {
            //we reached the first element again, we need to interrupt, there is no other item
            //to select
            interrupt = true;
        } else {
            //find current menue single entry
            entry = GetMenueSingleEntryForEntryNr(currSelMenuePage, currEntryNr);
            //can we select this item, if so use it, otherwise go to the next one
            if (entry->itemSelectable) {
                selectAbleItemFound = true;
            }
        }
    }

    //if we found something new to select, select it
    if (!interrupt) {
        //select next item after player interaction
        SelectNewMenueEntryWithEntryNr(currSelMenuePage, currEntryNr);
    }
}

void Menue::ItemReturn() {
    //Does this menue item have an action we can trigger?
    //when pressing Return key
    if (currSelMenueSingleEntry->triggerAction != nullptr) {
           //we want to trigger/execute an action by the user
            //we can reuse currSetValue here to indicate the main program
            //for example which championship slot to load or save and so on
            currSelMenueSingleEntry->triggerAction->currSetValue = currSelMenueSingleEntry->entryNumber;
            currActionToExecute = currSelMenueSingleEntry->triggerAction;
    }

    //does this item allow to change to another sub menue page?
    //if so then change to the next page
    if (currSelMenueSingleEntry->nextMenuePage != nullptr) {

        //special change to race selection menuePage?
        if (currSelMenueSingleEntry->nextMenuePage == RaceTrackSelectionPage) {
            //special handling for change to race selection part of menue logic

            //if we are at the race menue page, this is a default new race request
            //if not we want to create a new championship
            if (currSelMenuePage == this->RaceMenuePage) {
                ShowRaceSelection(currSelMenueSingleEntry);
            } else {
                //reset variables for new championship
                mGameAssets->NewChampionship();

                //disable the continue championship menue entry
                ContinueChampionshipEntry->itemSelectable = false;

                //we need to set parameter championShipMode = true!
                ShowRaceSelection(currSelMenueSingleEntry, true);
            }
        } else {
          //default menue behavior

          //change to first selectable item of activated page (beginning at the top)
          currSelMenuePage = currSelMenueSingleEntry->nextMenuePage;

          //one special case, when we enter the championship savegame load or save page, we first need to search
          //and update the available save games
          if ((currSelMenuePage == ChampionshipLoadMenuePage) || (currSelMenuePage == ChampionshipSaveMenuePage)) {
              UpdateChampionshipSaveGames();
          }

          //do we use typewriter effect?
          if (MENUE_ENABLETYPEWRITEREFFECT) {
            //reset typewriter effect to build
            //next page correctly
            currNrCharsShownCnter = 0;
            finalNrChardsShownMenuePageFinished = GetNrOfCharactersOnMenuePage(currSelMenuePage);

            //for typewriter effect we need to set menue state again
            //to typewriter inbetween state
            currMenueState = MENUE_STATE_TYPETEXT;
          } else {
            //no skip this effect
            currMenueState = MENUE_STATE_SELACTIVE;
          }

        SelectNewMenueEntryWithEntryNr(currSelMenuePage, MenuePageFindFirstSelectableEntryNr(currSelMenuePage));
      }

      return;
    }
}

void Menue::RemoveInputTextFieldChar(MenueSingleEntry* textInputEntry) {
    mMenueUserInactiveTimer = 0.0f;

    //is there still a character to be removed?
    if (textInputEntry->currTextInputFieldStrLen > 0) {
        textInputEntry->currTextInputFieldStrLen--;

        //just shorten current string to setting new null termination in string
        textInputEntry->currTextInputFieldStr[textInputEntry->currTextInputFieldStrLen] = 0;

        PlayMenueSound(SRES_MENUE_TYPEWRITEREFFECT1);
    }

    //update menue item entry text, important (otherwise we do not see change reflected in Gui Dialog!
    textInputEntry->entryText = textInputEntry->currTextInputFieldStr;

    //if typewriter effect is used we need to remove one character to variable
    //finalNrChardsShownMenuePageFinished, otherwise variable value will be wrong afterwards
    if (MENUE_ENABLETYPEWRITEREFFECT) {
        finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);
        currNrCharsShownCnter = finalNrChardsShownMenuePageFinished;
    }
}

void Menue::AddInputTextFieldChar(MenueSingleEntry* textInputEntry, char newCharToAdd) {
    mMenueUserInactiveTimer = 0.0f;

    PlayMenueSound(SRES_MENUE_TYPEWRITEREFFECT1);

   //we want to add a new char at the end of the input text
   //if text already to long let new character overwrite the last one at the end (game does the same)
   textInputEntry->currTextInputFieldStrLen++;

   if (textInputEntry->currTextInputFieldStrLen > (MENUE_TEXTENTRY_MAXTEXTLEN - 1)) {
       //text is too long
       textInputEntry->currTextInputFieldStrLen = (MENUE_TEXTENTRY_MAXTEXTLEN - 1);
       textInputEntry->currTextInputFieldStr[textInputEntry->currTextInputFieldStrLen] = newCharToAdd;
   } else {
       char newTxtInput[2];
       newTxtInput[0] = newCharToAdd;
       newTxtInput[1] = 0; //null terminate string!

       //not too long yet, add new character
       strcat(textInputEntry->currTextInputFieldStr, newTxtInput);
   }

   //update menue item entry text, important (otherwise we do not see change reflected in Gui Dialog!
   textInputEntry->entryText = textInputEntry->currTextInputFieldStr;

   //if typewriter effect is used we need to update number of
   //finalNrChardsShownMenuePageFinished, otherwise not all characters will be rendered
   if (MENUE_ENABLETYPEWRITEREFFECT) {
       finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);
       currNrCharsShownCnter = finalNrChardsShownMenuePageFinished;
   }
}

void Menue::SetInputTextField(MenueSingleEntry* textInputEntry, char* newText) {
   strncpy(textInputEntry->currTextInputFieldStr, newText, sizeof (textInputEntry->currTextInputFieldStr));

   textInputEntry->currTextInputFieldStrLen = (irr::u8)(strlen(textInputEntry->currTextInputFieldStr));

   //update menue item entry text, important (otherwise we do not see change reflected in Gui Dialog!
   textInputEntry->entryText = textInputEntry->currTextInputFieldStr;

   //if typewriter effect is used we need to update number of
   //finalNrChardsShownMenuePageFinished, otherwise not all characters will be rendered
   if (MENUE_ENABLETYPEWRITEREFFECT) {
       finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);
       currNrCharsShownCnter = finalNrChardsShownMenuePageFinished;
   }
}

void Menue::ReturnToParentMenuePage(MenuePage* currentMenuePage) {
    //is there really a parent entry, if not we can not go up
    if (currentMenuePage->parentMenuePage != nullptr) {

        //is there an action that should be triggered when with exit this menue page
        //with ESC key?
        if (currentMenuePage->pageEscKeyTriggerAction != nullptr) {
             currActionToExecute = currentMenuePage->pageEscKeyTriggerAction;
        }

        if (MENUE_ENABLETYPEWRITEREFFECT) {
            finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currentMenuePage->parentMenuePage);
            currNrCharsShownCnter = 0;
            currMenueState = MENUE_STATE_TYPETEXT;
        }

        //change to parent page, and select first selectable menue item
         SelectNewMenueEntryWithEntryNr(currentMenuePage->parentMenuePage,
                                        MenuePageFindFirstSelectableEntryNr(currentMenuePage->parentMenuePage));
    }
}

//this function returns currently entered value (string) in the text input field
//while triggering a defined menue action, and then goes back to the caller
//menue page
void Menue::AcceptInputTextFieldValue(MenueSingleEntry* textInputEntry) {
    //copy newly entered string into action string data field
    strcpy(textInputEntry->triggerAction->newSetTextInputString, textInputEntry->currTextInputFieldStr);

    //trigger the action
    currActionToExecute = textInputEntry->triggerAction;

    //go back to the caller MenuePage (one level up in menue)
    ReturnToParentMenuePage(currSelMenuePage);
}

void Menue::HandleTextInputField(MenueSingleEntry* textInputEntry) {
  //following behavior is wanted, the original game does the same
  //ESC or RETURN = accept and return currently entered text, and return to menue page above
  //allow player to enter the following keys as text: 0-9, A-Z ; is automatically all upper case
  //the game also allows to enter the player certain other special characters, I will not implement this right now!
  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_0)) {
     AddInputTextFieldChar(textInputEntry, '0');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_1)) {
     AddInputTextFieldChar(textInputEntry, '1');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_2)) {
     AddInputTextFieldChar(textInputEntry, '2');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_3)) {
     AddInputTextFieldChar(textInputEntry, '3');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_4)) {
     AddInputTextFieldChar(textInputEntry, '4');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_5)) {
     AddInputTextFieldChar(textInputEntry, '5');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_6)) {
     AddInputTextFieldChar(textInputEntry, '6');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_7)) {
     AddInputTextFieldChar(textInputEntry, '7');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_8)) {
     AddInputTextFieldChar(textInputEntry, '8');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_9)) {
     AddInputTextFieldChar(textInputEntry, '9');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_A)) {
     AddInputTextFieldChar(textInputEntry, 'A');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_B)) {
     AddInputTextFieldChar(textInputEntry, 'B');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_C)) {
     AddInputTextFieldChar(textInputEntry, 'C');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_D)) {
     AddInputTextFieldChar(textInputEntry, 'D');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_E)) {
     AddInputTextFieldChar(textInputEntry, 'E');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_F)) {
     AddInputTextFieldChar(textInputEntry, 'F');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_G)) {
     AddInputTextFieldChar(textInputEntry, 'G');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_H)) {
     AddInputTextFieldChar(textInputEntry, 'H');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_I)) {
     AddInputTextFieldChar(textInputEntry, 'I');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_J)) {
     AddInputTextFieldChar(textInputEntry, 'J');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_K)) {
     AddInputTextFieldChar(textInputEntry, 'K');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_L)) {
     AddInputTextFieldChar(textInputEntry, 'L');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_M)) {
     AddInputTextFieldChar(textInputEntry, 'M');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_N)) {
     AddInputTextFieldChar(textInputEntry, 'N');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_O)) {
     AddInputTextFieldChar(textInputEntry, 'O');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_P)) {
     AddInputTextFieldChar(textInputEntry, 'P');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_Q)) {
     AddInputTextFieldChar(textInputEntry, 'Q');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_R)) {
     AddInputTextFieldChar(textInputEntry, 'R');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_S)) {
     AddInputTextFieldChar(textInputEntry, 'S');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_T)) {
     AddInputTextFieldChar(textInputEntry, 'T');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_U)) {
     AddInputTextFieldChar(textInputEntry, 'U');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_V)) {
     AddInputTextFieldChar(textInputEntry, 'V');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_W)) {
     AddInputTextFieldChar(textInputEntry, 'W');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_X)) {
     AddInputTextFieldChar(textInputEntry, 'X');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_Y)) {
     AddInputTextFieldChar(textInputEntry, 'Y');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_Z)) {
     AddInputTextFieldChar(textInputEntry, 'Z');
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_BACK)) {
     RemoveInputTextFieldChar(textInputEntry);
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_RETURN)) {
     AcceptInputTextFieldValue(textInputEntry);
  }

  if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
     AcceptInputTextFieldValue(textInputEntry);
  }

  RealignMenuePageItems();
}

void Menue::UpdateShipSelectionTypeWriterEffect() {
    if (MENUE_ENABLETYPEWRITEREFFECT) {
        currNrCharsShownCnter = 0;
        currMenueState = MENUE_STATE_TYPETEXT;
        finalNrChardsShownMenuePageFinished = (irr::u32)(strlen(currSelShipName)) + (irr::u32)(strlen(currSelShipColorSchemeName)) +
            (irr::u32)(strlen(ShipStatSpeedLabel->text)) + ShipStatSpeedLabel->statNrBlocks +
            (irr::u32)(strlen(ShipStatArmourLabel->text)) + ShipStatArmourLabel->statNrBlocks +
            (irr::u32)(strlen(ShipStatWeightLabel->text)) + ShipStatWeightLabel->statNrBlocks +
            (irr::u32)(strlen(ShipStatFirePowerLabel->text)) + ShipStatFirePowerLabel->statNrBlocks;
    }
}

void Menue::HandleInputRaceSelection() {
    //only allow menue input in case window is fully open
    if (currMenueState == MENUE_STATE_SELACTIVE) {
        //only allow to change the number of laps if we are not currently
        //in championship mode; the same is true for the track selection keys
        //in championship mode we can only see one racetrack, and no selection is possible
        if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_UP) && (!mChampionshipMode)) {
            mMenueUserInactiveTimer = 0.0f;

            //play sound for changing selected menue item
            PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

            //user wants to add one lap to race
            if (currentSelRaceLapNumberVal < 50)
                currentSelRaceLapNumberVal++;
            else
                currentSelRaceLapNumberVal = 1;

            RecalculateRaceTrackStatLabels();

            finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);
            currNrCharsShownCnter = finalNrChardsShownMenuePageFinished;
        }

        if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_DOWN) && (!mChampionshipMode)) {
            mMenueUserInactiveTimer = 0.0f;

            //play sound for changing selected menue item
            PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

            //user wants to remove one lap from race
            if (currentSelRaceLapNumberVal > 1)
                currentSelRaceLapNumberVal--;
            else
                currentSelRaceLapNumberVal = 50;

            RecalculateRaceTrackStatLabels();

            finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);
            currNrCharsShownCnter = finalNrChardsShownMenuePageFinished;
        }

        if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_RETURN)) {
            mMenueUserInactiveTimer = 0.0f;

            //store last selected race track in assets class
            this->mGameAssets->SetNewLastSelectedRaceTrack(currSelectedRaceTrack);

            //set new number of default laps for this race track
            this->mGameAssets->SetNewRaceTrackDefaultNrLaps(currSelectedRaceTrack, currentSelRaceLapNumberVal);

            //if we are in championship mode, and the player should not be able to select craft
            //anymore then skip ship selection completely
            if (mChampionshipMode && (!mGameAssets->CanPlayerCurrentlySelectCraftDuringChampionship())) {
                //skip ship selection
                mMenueUserInactiveTimer = 0.0f;

                //player accepted race setup
                AcceptedRaceSetup();
            } else {
                //change to Ship selection page
                currSelMenuePage = ShipSelectionPage;

                ShowShipSelection();
            }
        }

        if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_LEFT) && (!mChampionshipMode)) {
            mMenueUserInactiveTimer = 0.0f;

            //play sound for changing selected menue item
            PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

            if (currSelectedRaceTrack > 0)
                currSelectedRaceTrack--;
            else
                currSelectedRaceTrack = numRaceTracksAvailable - 1;

            //update selection "wheel" position
            Set3DModelRaceTrackWheelPositionIdx(currSelectedRaceTrack, 2);
        }

        if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_RIGHT) && (!mChampionshipMode)) {
            mMenueUserInactiveTimer = 0.0f;

            //play sound for changing selected menue item
            PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

            currSelectedRaceTrack++;
            if (currSelectedRaceTrack > (numRaceTracksAvailable - 1))
                currSelectedRaceTrack = 0;

            //update selection "wheel" position
            Set3DModelRaceTrackWheelPositionIdx(currSelectedRaceTrack, 1);
        }

        if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
            mMenueUserInactiveTimer = 0.0f;

            //interrupt and return back to default main menue
            InterruptRaceSelection();
        }
    }
}

void Menue::HandleInputShipSelection() {
    //only allow menue input in case window is fully open
    if (currMenueState == MENUE_STATE_SELACTIVE) {
        if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_UP)) {
            mMenueUserInactiveTimer = 0.0f;

            //play sound for changing selected menue item
            PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

            //user wants to change color of ship
            if (currSelectedShipColorScheme < (this->mGameAssets->mCraftColorSchemeNames.size() - 1))
                currSelectedShipColorScheme++;
            else
                currSelectedShipColorScheme = 0;

            //update ship color scheme
            SetShipColorScheme(currSelectedShipColorScheme);

            HideShipModels();
            UnhideShipModels();
        }

        if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_DOWN)) {
            mMenueUserInactiveTimer = 0.0f;

            //play sound for changing selected menue item
            PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

            //user wants to change color of ship
            if (currSelectedShipColorScheme > 0)
                currSelectedShipColorScheme--;
            else
                currSelectedShipColorScheme = (irr::u8)(this->mGameAssets->mCraftColorSchemeNames.size()) - 1;

            //update ship color scheme
            SetShipColorScheme(currSelectedShipColorScheme);

            HideShipModels();
            UnhideShipModels();
        }

        if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_RETURN)) {
            mMenueUserInactiveTimer = 0.0f;

            //set main player craft selection in assets class
            this->mGameAssets->SetNewMainPlayerSelectedCraft(this->currSelectedShip);

            //set new default craft color scheme
            this->mGameAssets->SetCurrentCraftColorScheme(this->currSelectedShipColorScheme);

            //player accepted race setup
            AcceptedRaceSetup();
        }

        if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_LEFT)) {
            mMenueUserInactiveTimer = 0.0f;

            //play sound for changing selected menue item
            PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

            if (currSelectedShip > 0)
                currSelectedShip--;
            else
                currSelectedShip = numCraftsAvailable - 1;

            //update selection "wheel" position
            Set3DModelShipWheelPositionIdx(currSelectedShip, 2);
        }

        if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_RIGHT)) {
            mMenueUserInactiveTimer = 0.0f;

            //play sound for changing selected menue item
            PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

            currSelectedShip++;
            if (currSelectedShip > (numCraftsAvailable - 1))
                currSelectedShip = 0;

            //update selection "wheel" position
            Set3DModelShipWheelPositionIdx(currSelectedShip, 1);
        }

        if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
            mMenueUserInactiveTimer = 0.0f;

            //interrupt and return back to race track selection menue page
            this->currSelMenuePage = RaceTrackSelectionPage;

            //hide player ship models again, otherwise we would also seem them
            //back in the race track selection screen again
            HideShipModels();

            //unhide race track models, we need to see them again
            UnhideRaceTrackModels();

            currSelWindowAnimation = windowMenueAnimationBetweenRaceAndShipSelection;
            currMenueWindowAnimationIdx = 0;
            currMenueWindowAnimationFinalIdx = 
                (irr::u8)(windowMenueAnimationBetweenRaceAndShipSelection->coordVec.size()) - 1;
            currMenueState = MENUE_STATE_TRANSITION;
            lastAnimationUpdateAbsTime = absoluteTime;
        }
    }
}

void Menue::HandleInput() {
    //racetrack selection and ship selection menue is so different, handle its input independently
    if (this->currSelMenuePage == RaceTrackSelectionPage) {
        //call racetrack selection input handling code
        HandleInputRaceSelection();
    } else if (this->currSelMenuePage == ShipSelectionPage) {
        //call ship selection input handling code
        HandleInputShipSelection();
    } else {
     //default menue input handling source code

     //only allow menue input in case window is fully open
     if (currMenueState == MENUE_STATE_SELACTIVE) {
        //case for normal menue entry item, handle UP/DOWN travel on menue page
        if (!currSelMenueSingleEntry->isTextEntryField) {
            //be sure to use function IsKeyDownSingleEvent, to prevent
            //multiple input events during one keypress!
            if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_UP)) {
                mMenueUserInactiveTimer = 0.0f;
                ItemUp();
            }

            if(mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_DOWN)) {
                mMenueUserInactiveTimer = 0.0f;
                ItemDown();
            }

            if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_RETURN)) {
                mMenueUserInactiveTimer = 0.0f;
                ItemReturn();
            }

            if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_LEFT)) {
                mMenueUserInactiveTimer = 0.0f;
                ItemLeft();
            }

            if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_RIGHT)) {
                mMenueUserInactiveTimer = 0.0f;
                ItemRight();
            }

            if (mGame->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
                mMenueUserInactiveTimer = 0.0f;

                //when we press ESC we want to go back to parent
                //menue page, as the original game
                //play sound for changing selected menue item
                PlayMenueSound(SRES_MENUE_SELECTOTHERITEM);

                ReturnToParentMenuePage(currSelMenuePage);
            }
       } else {
            //instead is text input field, handle completelty different
            HandleTextInputField(currSelMenueSingleEntry);
        }
      }
  }
}

//returns true if the user triggered an action in the menue, false
//otherwise. If a new action is pending the type of menue action
//is returned in variable pendingAction
bool Menue::HandleActions(MenueAction* &pendingAction) {
    //is there a new action that needs to be executed?
    if (currActionToExecute != nullptr) {
        //handover pending action to main program
        pendingAction = currActionToExecute;

        currActionToExecute = nullptr;
        return true;
    }

    return false;
}

//Function is called when selected ship position is reached
//at the front of the menue
void Menue::FinalPositionShipSelectionWheelReached(irr::u8 newPosition) {
    //first update entry name in window
    if ((newPosition >= 0) && (newPosition < this->mGameAssets->mCraftVec->size())) {
        //copy craft name and craft stats from game assets class
        strcpy(currSelShipName, this->mGameAssets->mCraftVec->at(newPosition)->name);
        currSelShipStatSpeed = this->mGameAssets->mCraftVec->at(newPosition)->statSpeed;
        currSelShipStatArmour = this->mGameAssets->mCraftVec->at(newPosition)->statArmour;
        currSelShipStatWeight = this->mGameAssets->mCraftVec->at(newPosition)->statWeight;
        currSelShipStatFirePower = this->mGameAssets->mCraftVec->at(newPosition)->statFirePower;
    } else {
        strcpy(currSelShipName, "UNDEFINED (ERROR!)");
        currSelShipStatSpeed = 0;
        currSelShipStatArmour = 0;
        currSelShipStatWeight = 0;
        currSelShipStatFirePower = 0;
    }

    RecalculateShipStatLabels();

    //recalculate X coordinate of ship name to center name!
    irr::u32 newCoord = (mGame->mScreenRes.Width / 2) -
            (mGame->mGameTexts->GetWidthPixelsGameText(currSelShipName, mGame->mGameTexts->GameMenueSelectedItemFont) / 2);
    this->ShipNameTitle->drawTextScrPosition.X = newCoord;

    UpdateShipSelectionTypeWriterEffect();
}

//Function is called when selected race track position is reached
//at the front of the menue
void Menue::FinalPositionRaceTrackWheelReached(irr::u8 newPosition) {
    //Important info: We need to use malloc here, instead of new char[]
    //Because we will free this memory later! Not that we get a delete[]
    //vs. free memory issue later
    char* currSelRaceTrackName = (char*)(malloc(50));

    //first update entry name in window
    if ((newPosition >= 0) && (newPosition < this->mGameAssets->mRaceTrackVec->size())) {   
       //copy name of race track from game assets class
       strcpy(currSelRaceTrackName, this->mGameAssets->mRaceTrackVec->at(newPosition)->name);

       //copy number of laps user set the last time for this race track from game assets class
       //this number is stored in the games config file for each track independently
       currentSelRaceLapNumberVal = this->mGameAssets->mRaceTrackVec->at(newPosition)->currSelNrLaps;
    } else
    {
       strcpy(currSelRaceTrackName, "UNDEFINED (ERROR!)");
       currentSelRaceLapNumberVal = 0;
    }

    RaceTrackNameTitle->SetText(currSelRaceTrackName);

    RecalculateRaceTrackStatLabels();
    RealignMenuePageItems();

    //update number of characters to print in type writer effect
    finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);

    //start type writer effect from beginning again
    currNrCharsShownCnter = 0;

    currMenueState = MENUE_STATE_TYPETEXT;
    lastAnimationUpdateAbsTime = absoluteTime;
}

//newPosition = number of race track model in the front of camera
//0 = RaceTrack 1
//1 = RaceTrack 2
//2 = RaceTrack 3...
//newWheelMoveState.
//0 = wheel not moving
//1 = wheel moving clock wise
//2 = wheel moving counter clock wise
//3 = immediatley update position (no rotation), used for restoring last state
void Menue::Set3DModelRaceTrackWheelPositionIdx(irr::u8 newPosition, irr::u8 newWheelMoveState) {
    //newPosition = 0 -> selects Track1 to be in the front
    //newPosition = 1 -> selects Track2 to be in the front
    //items rotate clockwise for next race track (position)

    //set new target value for wheel position angle
    //targetRaceTrackWheelAngleDeg = 270.0f + newPosition * 60.0f;
    targetRaceTrackWheelAngleDeg = 270.0f + newPosition * (360.0f / irr::f32(numRaceTracksAvailable));

    while (targetRaceTrackWheelAngleDeg > 360.0f) {
        targetRaceTrackWheelAngleDeg -= 360.0f;
    }

    while (targetRaceTrackWheelAngleDeg < 0.0f) {
        targetRaceTrackWheelAngleDeg += 360.0f;
    }

    RaceTrackWheelMoving = newWheelMoveState;
}

void Menue::SetShipColorScheme(irr::u8 newSelectedColorScheme) {
    //update name of selected ship color scheme
    if ((newSelectedColorScheme >= 0) && (newSelectedColorScheme < (this->mGameAssets->mCraftColorSchemeNames.size()))) {
        strcpy(currSelShipColorSchemeName, this->mGameAssets->mCraftColorSchemeNames.at(newSelectedColorScheme));
    } else {
        strcpy(currSelShipColorSchemeName, "UNDEFINED (ERROR!)");
    }

    //recalculate X coordinate of ship color scheme name to center name!
    irr::u32 newCoord = (mGame->mScreenRes.Width / 2) -
            (mGame->mGameTexts->GetWidthPixelsGameText(currSelShipColorSchemeName, mGame->mGameTexts->GameMenueWhiteTextSmallSVGA) / 2);

    irr::core::vector2di pos1(newCoord, 59);

    pos1.Y = (pos1.Y * mGame->mScreenRes.Height) / 400;

    currSelectedShipColorSchemeTextPos.X = pos1.X;
    currSelectedShipColorSchemeTextPos.Y = pos1.Y;
}

//newPosition = number of ship model in the front of camera
//0 = Ship 1
//1 = Ship 2
//2 = Ship 3...
//newWheelMoveState.
//0 = wheel not moving
//1 = wheel moving clock wise
//2 = wheel moving counter clock wise
//3 = immediatley update position (no rotation), used for restoring last state
void Menue::Set3DModelShipWheelPositionIdx(irr::u8 newPosition, irr::u8 newWheelMoveState) {
    //newPosition = 0 -> selects Ship1 to be in the front
    //newPosition = 1 -> selects Ship2 to be in the front
    //items rotate clockwise for next ship (position)

    //set new target value for wheel position angle
    //targetShipWheelAngleDeg = 270.0f + newPosition * 60.0f;
    targetShipWheelAngleDeg = 270.0f + newPosition * (360.0f / irr::f32(numCraftsAvailable));

    while (targetShipWheelAngleDeg > 360.0f) {
        targetShipWheelAngleDeg -= 360.0f;
    }

    while (targetShipWheelAngleDeg < 0.0f) {
        targetShipWheelAngleDeg += 360.0f;
    }

    ShipWheelMoving = newWheelMoveState;
}

void Menue::Update3DModelRaceTrackWheelPosition() {
    irr::f32 CircleRadius = 1.5f;
    irr::f32 currAngleDeg = 0.0f;
    irr::core::vector3df TrackCoord(0.0f, 0.0f, 0.0f);

    //0 = wheel not moving
    //1 = wheel moving clock wise
    //2 = wheel moving counter clock wise
    //3 = immediatley update position (no rotation), used for restoring last state
    if (RaceTrackWheelMoving == 1) {
          currRaceTrackWheelAngleDeg += 2.0f;

          if (targetRaceTrackWheelAngleDeg < currRaceTrackWheelAngleDeg) {
              currRaceTrackWheelAngleDeg -= 360.0f;
          }

          if ((targetRaceTrackWheelAngleDeg - currRaceTrackWheelAngleDeg) < 2.0f) {
            //turning wheel is finished, set final correct value of angle
            RaceTrackWheelMoving = 0;
            currRaceTrackWheelAngleDeg = targetRaceTrackWheelAngleDeg;

            //update menue text, we reached the correct selected next
            //race track
            FinalPositionRaceTrackWheelReached(currSelectedRaceTrack);
         }
    }

    if (RaceTrackWheelMoving == 2) {
        currRaceTrackWheelAngleDeg -= 2.0f;

        if (targetRaceTrackWheelAngleDeg > currRaceTrackWheelAngleDeg) {
            currRaceTrackWheelAngleDeg += 360.0f;
        }

        if ((currRaceTrackWheelAngleDeg - targetRaceTrackWheelAngleDeg) < 2.0f) {
            //turning wheel is finished, set final correct value of angle
            RaceTrackWheelMoving = 0;
            currRaceTrackWheelAngleDeg = targetRaceTrackWheelAngleDeg;

            //update menue text, we reached the correct selected next
            //race track
            FinalPositionRaceTrackWheelReached(currSelectedRaceTrack);
       }
      }

    //no animation, immediately set new location
    if (RaceTrackWheelMoving == 3) {
        RaceTrackWheelMoving = 0;
        currRaceTrackWheelAngleDeg = targetRaceTrackWheelAngleDeg;

        //update menue text, we reached the correct selected next
        //race track
        FinalPositionRaceTrackWheelReached(currSelectedRaceTrack);
    }

    currAngleDeg = currRaceTrackWheelAngleDeg;

    for (irr::u8 i = 0; i < this->numRaceTracksAvailable; i++) {
        if (!mChampionshipMode) {
            //Track number i
            TrackCoord.X = CircleRadius * sin((currAngleDeg / 180) * irr::core::PI);
            TrackCoord.Z = CircleRadius * cos((currAngleDeg / 180) * irr::core::PI);
            this->ModelTrackSceneNodeVec.at(i)->setPosition(TrackCoord);

            //restore default scaling of race track models
            this->ModelTrackSceneNodeVec.at(i)->setScale(irr::core::vector3df(1.0f, 1.0f, 1.0f));
        } else {
            //additional move the single racetrack model into the middle of the screen if
            //we are in championship mode
            this->ModelTrackSceneNodeVec.at(i)->setPosition(irr::core::vector3df(0.0f, 0.0f, 0.0f));

            //also scale model so that it is big enough
            this->ModelTrackSceneNodeVec.at(i)->setScale(irr::core::vector3df(2.0f, 2.0f, 2.0f));
        }

        currAngleDeg -= (360.0f / irr::f32((this->numRaceTracksAvailable)));
    }
}

void Menue::Update3DModelShipWheelPosition() {
    irr::f32 CircleRadius = 1.5f;
    irr::f32 currAngleDeg = 0.0f;
    irr::core::vector3df ShipCoord(0.0f, 0.0f, 0.0f);

    //0 = wheel not moving
    //1 = wheel moving clock wise
    //2 = wheel moving counter clock wise
    //3 = immediatley update position (no rotation), used for restoring last state
    if (ShipWheelMoving == 1) {
          currShipWheelAngleDeg += 2.0f;

          if (targetShipWheelAngleDeg < currShipWheelAngleDeg) {
              currShipWheelAngleDeg -= 360.0f;
          }

          if ((targetShipWheelAngleDeg - currShipWheelAngleDeg) < 2.0f) {
              //turning wheel is finished, set final correct value of angle
              ShipWheelMoving = 0;
              currShipWheelAngleDeg = targetShipWheelAngleDeg;
              FinalPositionShipSelectionWheelReached(currSelectedShip);
         }
    }

    if (ShipWheelMoving == 2) {
        currShipWheelAngleDeg -= 2.0f;

        if (targetShipWheelAngleDeg > currShipWheelAngleDeg) {
            currShipWheelAngleDeg += 360.0f;
        }

        if ((currShipWheelAngleDeg - targetShipWheelAngleDeg) < 2.0f) {
            //turning wheel is finished, set final correct value of angle
            ShipWheelMoving = 0;
            currShipWheelAngleDeg = targetShipWheelAngleDeg;
            FinalPositionShipSelectionWheelReached(currSelectedShip);
       }
      }

    //Immediately update position, is used to restore last
    //selected craft
    if (ShipWheelMoving == 3) {
        ShipWheelMoving = 0;
        currShipWheelAngleDeg = targetShipWheelAngleDeg;
        FinalPositionShipSelectionWheelReached(currSelectedShip);
    }

    currAngleDeg = currShipWheelAngleDeg;

    for (irr::u8 i = 0; i < this->numCraftsAvailable; i++) {
        //Ship number i
        ShipCoord.X = CircleRadius * sin((currAngleDeg / 180) * irr::core::PI);
        ShipCoord.Z = CircleRadius * cos((currAngleDeg / 180) * irr::core::PI);
        this->ModelCraftsSceneNodeVec[i]->at(currSelectedShipColorScheme)->setPosition(ShipCoord);

        currAngleDeg -= (360.0f / irr::f32((this->numCraftsAvailable)));
    }
}

void Menue::InitRaceTrackSceneNodes() {
    //create the menu 3D camera
    MenueCamera = smgrMenue->addCameraSceneNode();
    MenueCamera->setTarget(irr::core::vector3df(0.0f, 0.0f, 0.0f));
    MenueCamera->setPosition(irr::core::vector3df(-3.0f, 1.5f, 0.0f));

    smgrMenue->setActiveCamera(MenueCamera);

    //set up light
    //smgrMenue->setAmbientLight(video::SColorf(255.0,255.0,255.0));

    //create SceneNode for each available race track
    //we need this sceneNodes for racetrack selection model view
    for (irr::u8 i = 0; i < this->numRaceTracksAvailable; i++) {
        this->ModelTrackSceneNodeVec.push_back(smgrMenue->addMeshSceneNode(this->mGameAssets->mRaceTrackVec->at(i)->MeshTrack));
        this->ModelTrackSceneNodeVec.at(i)->setVisible(true);
        this->ModelTrackSceneNodeVec.at(i)->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    }

    //initialize 3D model positions
    Set3DModelRaceTrackWheelPositionIdx(0, 0);
    currRaceTrackWheelAngleDeg = 270.0f;
}

void Menue::InitVehicleModels() {
    //load all necessary vehicle models using irrlicht

    //irr::scene::IMesh* newMesh;
    irr::scene::IMeshSceneNode* newSceneNode;
    std::vector<irr::scene::IMeshSceneNode*>* vecSceneNodePntr;

    //lets loop to create each SceneNode (we have
    //multiple per craft, because each ship exists in
    //different color schemes)
    for (irr::u8 j = 0; j < numCraftsAvailable; j++) {
          vecSceneNodePntr = new std::vector<irr::scene::IMeshSceneNode*>();
          vecSceneNodePntr->clear();
          for (int i = 0; i < 8; i++) {
            newSceneNode = smgrMenue->addMeshSceneNode(this->mGameAssets->mCraftVec->at(j)->MeshCraft.at(i));
            newSceneNode->setVisible(true);
            newSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);

            vecSceneNodePntr->push_back(newSceneNode);
            //this->ModelShip1.push_back(newSceneNode);
          }

          this->ModelCraftsSceneNodeVec.push_back(vecSceneNodePntr);
    }

    //initialize 3D model positions
    Set3DModelShipWheelPositionIdx(0, 0);
    currShipWheelAngleDeg = 270.0f;
}

void Menue::InitStatLabels() {
    ShipStatSpeedLabel = new ShipStatLabel();
    ShipStatSpeedLabel->text = new char[7];

    strcpy(ShipStatSpeedLabel->text, "SPEED");
    //all the other variables will be setup later

    ShipStatArmourLabel = new ShipStatLabel();
    ShipStatArmourLabel->text = new char[8];

    strcpy(ShipStatArmourLabel->text, "ARMOUR");
    //all the other variables will be setup later

    ShipStatWeightLabel = new ShipStatLabel();
    ShipStatWeightLabel->text = new char[8];

    strcpy(ShipStatWeightLabel->text, "WEIGHT");
    //all the other variables will be setup later

    ShipStatFirePowerLabel = new ShipStatLabel();
    ShipStatFirePowerLabel->text = new char[10];

    strcpy(ShipStatFirePowerLabel->text, "FIREPOWER");
    //all the other variables will be setup later
}

void Menue::CleanUpStatLabels() {
    delete[] ShipStatSpeedLabel->text;
    delete ShipStatSpeedLabel;
    ShipStatSpeedLabel = nullptr;

    delete[] ShipStatArmourLabel->text;
    delete ShipStatArmourLabel;
    ShipStatArmourLabel = nullptr;

    delete[] ShipStatWeightLabel->text;
    delete ShipStatWeightLabel;
    ShipStatWeightLabel = nullptr;

    delete[] ShipStatFirePowerLabel->text;
    delete ShipStatFirePowerLabel;
    ShipStatFirePowerLabel = nullptr;
}

void Menue::CalcStatLabelHelper(irr::u8 currStatVal, ShipStatLabel &label, irr::core::vector2di centerCoord) {
    irr::u32 textWidth =
            mGame->mGameTexts->GetWidthPixelsGameText(label.text, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA);
    irr::u32 textHeight =
            mGame->mGameTexts->GetHeightPixelsGameText(label.text, mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA);

    //8 pixels with per stat block width,
    irr::u32 statBlockWidth = 8 * currStatVal;

    //23 pixel distance between stat text and stat blocks
    irr::u32 distPixelStatTextToStatBlocks = 23;

    irr::u32 overallWidth = textWidth + statBlockWidth + distPixelStatTextToStatBlocks;

    label.drawPositionTxt.X = centerCoord.X - (overallWidth / 2);
    label.drawPositionTxt.Y = centerCoord.Y - (textHeight / 2);

    label.statBoxOutline.UpperLeftCorner.X = centerCoord.X - (overallWidth / 2) + textWidth + distPixelStatTextToStatBlocks;
    label.statBoxOutline.UpperLeftCorner.Y = centerCoord.Y - (textHeight / 2);
    label.statBoxOutline.LowerRightCorner.X = centerCoord.X - (overallWidth / 2) + textWidth + distPixelStatTextToStatBlocks + statBlockWidth;
    label.statBoxOutline.LowerRightCorner.Y = centerCoord.Y + (textHeight / 2);

    label.statNrBlocks = currStatVal;
}

void Menue::RecalculateShipStatLabels() {
    irr::core::vector2di pos1(216, 97);

    pos1.X = (pos1.X * mGame->mScreenRes.Width) / 640;
    pos1.Y = (pos1.Y * mGame->mScreenRes.Height) / 400;

    //Center speed stat around coord 216, 97
    CalcStatLabelHelper(currSelShipStatSpeed, *ShipStatSpeedLabel, pos1);

    pos1.set(200, 113);

    pos1.X = (pos1.X * mGame->mScreenRes.Width) / 640;
    pos1.Y = (pos1.Y * mGame->mScreenRes.Height) / 400;

    //Center speed stat around coord 200, 113
    CalcStatLabelHelper(currSelShipStatArmour, *ShipStatArmourLabel, pos1);

    pos1.set(452, 97);

    pos1.X = (pos1.X * mGame->mScreenRes.Width) / 640;
    pos1.Y = (pos1.Y * mGame->mScreenRes.Height) / 400;

    //Center weight stat around coord 452, 97
    CalcStatLabelHelper(currSelShipStatWeight, *ShipStatWeightLabel, pos1);

    pos1.set(456, 113);

    pos1.X = (pos1.X * mGame->mScreenRes.Width) / 640;
    pos1.Y = (pos1.Y * mGame->mScreenRes.Height) / 400;

    //Center firepower stat around coord 456, 113
    CalcStatLabelHelper(currSelShipStatFirePower, *ShipStatFirePowerLabel, pos1);
}

void Menue::CalcMenueTextLabelHelper(MenueTextLabel &label, irr::core::vector2di centerCoord) {
    irr::u32 textWidth = mGame->mGameTexts->GetWidthPixelsGameText(label.text, label.whichFont);
    irr::u32 textHeight = mGame->mGameTexts->GetHeightPixelsGameText(label.text, label.whichFont);

    label.drawPositionTxt.X = centerCoord.X - (textWidth / 2);
    label.drawPositionTxt.Y = centerCoord.Y - (textHeight / 2);
}

void Menue::RecalculateRaceTrackStatLabels() {
    //update to latest text labels
    //Important info: We need to use malloc here, instead of new char[]
    //Because we will free this memory later! Not that we get a delete[]
    //vs. free memory issue later
    char* newNrLapsText = (char*)(malloc(70));

    sprintf(newNrLapsText, "NUMBER OF LAPS %d", currentSelRaceLapNumberVal);

    //Set the new text
    SelRaceTrackNrLapsLabel->SetText(newNrLapsText);

    char* newBestLapText = (char*)(malloc(70));

    //also need to center all of this text inside the text window
    sprintf(newBestLapText, "BEST LAP %d %s", this->mGameAssets->mRaceTrackVec->at(currSelectedRaceTrack)->bestLapTime,
            this->mGameAssets->mRaceTrackVec->at(currSelectedRaceTrack)->bestPlayer);

    SelRaceTrackBestLapLabel->SetText(newBestLapText);

    char* newBestRaceText = (char*)(malloc(70));

    sprintf(newBestRaceText, "BEST RACE %d %s", this->mGameAssets->mRaceTrackVec->at(currSelectedRaceTrack)->bestHighScore,
            this->mGameAssets->mRaceTrackVec->at(currSelectedRaceTrack)->bestHighScorePlayer);

    SelRaceTrackBestRaceLabel->SetText(newBestRaceText);
}

//Constructor, initialization of main menue
Menue::Menue(Game* game, SoundEngine* soundEngine, Assets *assets) {
    mSoundEngine = soundEngine;
    mGameAssets = assets;
    mGame = game;

    //how many race tracks and crafts are available
    //for selection from assets class
    numRaceTracksAvailable = (irr::u8)(mGameAssets->mRaceTrackVec->size());
    this->ModelTrackSceneNodeVec.clear();

    numCraftsAvailable = (irr::u8)(mGameAssets->mCraftVec->size());
    this->ModelCraftsSceneNodeVec.clear();

    //create a new sceneManager for race track/ship selection page based on the main one
    smgrMenue = mGame->mSmgr->createNewSceneManager();

    absoluteTime = 0.0f;

    MenueInitializationSuccess = true;

    if (!InitMenueResources())
        MenueInitializationSuccess = false;

    InitStatLabels();
    InitActions();
    InitMenuePages();
    CreateMenueEntries();
    SetWindowAnimationVec();
    InitRaceTrackSceneNodes();
    InitVehicleModels();

    last3DModelUpdateAbsTime = absoluteTime;

    blinkTextCursorVisible = false;
    blinkTextCursorNextStateChangeAbsTime = (absoluteTime + MENUE_TEXTENTRY_CURSORBLINKPERIODSEC);

    ShowMainMenue();
}

void Menue::ShowMainMenue() {
    mMenueUserInactiveTimer = 0.0f;

    //initially game shows main menue top page
    //with item Race selected
    currSelMenuePage = this->TopMenuePage;
    currSelMenueSingleEntry = this->Race;

    RealignMenuePageItems();

    //initial setup for type writer effect
    //of main menue
    if (MENUE_ENABLETYPEWRITEREFFECT) {
        typeWriterEffectNextCharacterAbsTime = absoluteTime;
        currNrCharsShownCnter = 0;
        finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);
    }

    //initially activate menue animation for start of game
    currSelWindowAnimation = windowMenueAnimationStartGame;
    currMenueWindowAnimationIdx = 0;
    currMenueWindowAnimationFinalIdx = (irr::u8)(windowMenueAnimationStartGame->coordVec.size()) - 1;
    currMenueState = MENUE_STATE_TRANSITION;
    lastAnimationUpdateAbsTime = absoluteTime;
}

void Menue::UpdateChampionshipLoadSlotMenueEntry(MenueSingleEntry &whichEntry, std::vector<ChampionshipSaveGameInfoStruct*>::iterator it) {
    //free the old entry string
    free(whichEntry.entryText);

    //create the new updated string
    whichEntry.entryText = strdup((*it)->championshipName);

    //if there is a savegame available in this slot, allow to
    //select it, otherwise do not
    whichEntry.itemSelectable = (*it)->saveGameAvail;
}

irr::u8 Menue::GetChampionShipSlotNrForSaveAction(irr::u8 whichEntryNumber) {
    if (ChampionshipSaveSlot1->entryNumber == whichEntryNumber) {
        //is slot 0 = Savegame 1
        return 0;
    } else if (ChampionshipSaveSlot2->entryNumber == whichEntryNumber) {
        //is slot 1 = Savegame 2
        return 1;
    } else if (ChampionshipSaveSlot3->entryNumber == whichEntryNumber) {
        //is slot 2 = Savegame 3
        return 2;
    } else if (ChampionshipSaveSlot4->entryNumber == whichEntryNumber) {
        //is slot 3 = Savegame 4
        return 3;
    } else if (ChampionshipSaveSlot5->entryNumber == whichEntryNumber) {
        //is slot 4 = Savegame 5
        return 4;
    }

    //if there is something wrong, return an invalid slot number
    //then save function will simply exit
    logging::Error("GetChampionShipSlotNrForSaveAction: Invalid entry number found! Skip save");
    return 5;
}

irr::u8 Menue::GetChampionShipSlotNrForLoadAction(irr::u8 whichEntryNumber) {
    if (ChampionshipLoadSlot1->entryNumber == whichEntryNumber) {
        //is slot 0 = Savegame 1
        return 0;
    } else if (ChampionshipLoadSlot2->entryNumber == whichEntryNumber) {
        //is slot 1 = Savegame 2
        return 1;
    } else if (ChampionshipLoadSlot3->entryNumber == whichEntryNumber) {
        //is slot 2 = Savegame 3
        return 2;
    } else if (ChampionshipLoadSlot4->entryNumber == whichEntryNumber) {
        //is slot 3 = Savegame 4
        return 3;
    } else if (ChampionshipLoadSlot5->entryNumber == whichEntryNumber) {
        //is slot 4 = Savegame 5
        return 4;
    }

    //if there is something wrong, return an invalid slot number
    //then load function will simply exit
    logging::Error("GetChampionShipSlotNrForLoadAction: Invalid entry number found! Skip save");
    return 5;
}

void Menue::StartChampionshipNameInputAtSlot(irr::u8 whichSlotNr) {
    if (whichSlotNr > 4)
        return;

    MenueSingleEntry* pntr;

    switch (whichSlotNr) {
        case 0: {
           pntr = ChampionshipSaveSlot1; break;
        }

        case 1: {
           pntr = ChampionshipSaveSlot2; break;
        }

        case 2: {
           pntr = ChampionshipSaveSlot3; break;
        }

        case 3: {
           pntr = ChampionshipSaveSlot4; break;
        }

        case 4: {
           pntr = ChampionshipSaveSlot5; break;
        }
    }

    //create temporary storage for text input string
    mNewChampionshipNameInputText = new char[30];

    //for start, copy existing name into input field
    strcpy(mNewChampionshipNameInputText, pntr->entryText);

    //we need to switch the menue item object at this location, into a text input
    //field, so that the user can select a new championship name
    pntr->isTextEntryField = true;

    //initialize the input text field with current
    //set input text
    pntr->initTextPntr = mNewChampionshipNameInputText;

    //free the current entryText, we loose pointer to it
    //during text input field anyway
    free(pntr->entryText);

    //present text field string from source pointer location
    SetInputTextField(pntr, pntr->initTextPntr);

    //temporarily also change action, so that we can get the entered
    //text in the game class, and we know when the championship
    //name was entered
    pntr->triggerAction = ActFinalizeChampionshipSaveSlot;
}

void Menue::EndChampionshipNameInputAtSlot(irr::u8 whichSlotNr) {
    if (whichSlotNr > 4)
        return;

    MenueSingleEntry* pntr;

    switch (whichSlotNr) {
        case 0: {
           pntr = ChampionshipSaveSlot1; break;
        }

        case 1: {
           pntr = ChampionshipSaveSlot2; break;
        }

        case 2: {
           pntr = ChampionshipSaveSlot3; break;
        }

        case 3: {
           pntr = ChampionshipSaveSlot4; break;
        }

        case 4: {
           pntr = ChampionshipSaveSlot5; break;
        }
    }

    //we need to create a new buffer for the
    //textEntry
    pntr->entryText = strdup(pntr->currTextInputFieldStr);

    //set back to normal menue item, not a text field anymore
    pntr->isTextEntryField = false;

    //nullptr the pointer again
    pntr->initTextPntr = nullptr;

    //restore the default action
    pntr->triggerAction = ActSaveChampionshipSlot;

    //delete temporary text storage again
    delete[] mNewChampionshipNameInputText;
}

void Menue::UpdateChampionshipSaveGames() {
    //execute a low level search for save games
    this->mGameAssets->SearchChampionshipSaveGames();

    //the search result is stored in mChampionshipSavegameInfoVec
    //member variable of Assets class
    //update menue entries with this information
    std::vector<ChampionshipSaveGameInfoStruct*>::iterator it;

    irr::u8 idx = 0;

    for (it = this->mGameAssets->mChampionshipSavegameInfoVec.begin(); it !=
              this->mGameAssets->mChampionshipSavegameInfoVec.end(); ++it) {

        switch(idx) {
            case 0: {
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipLoadSlot1, it);
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipSaveSlot1, it);

                //slots to save game are always selectable
                ChampionshipSaveSlot1->itemSelectable = true;
                break;
            }

            case 1: {
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipLoadSlot2, it);
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipSaveSlot2, it);

                //slots to save game are always selectable
                ChampionshipSaveSlot2->itemSelectable = true;
                break;
            }

            case 2: {
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipLoadSlot3, it);
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipSaveSlot3, it);

                //slots to save game are always selectable
                ChampionshipSaveSlot3->itemSelectable = true;
                break;
            }

            case 3: {
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipLoadSlot4, it);
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipSaveSlot4, it);

                //slots to save game are always selectable
                ChampionshipSaveSlot4->itemSelectable = true;
                break;
            }

            case 4: {
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipLoadSlot5, it);
                UpdateChampionshipLoadSlotMenueEntry(*ChampionshipSaveSlot5, it);

                //slots to save game are always selectable
                ChampionshipSaveSlot5->itemSelectable = true;
                break;
            }
        }

        idx++;
    }

    //cleanup the data transfer structs again
    //we do not need them anymore, and we do not want to
    //have a memory leak
    mGameAssets->CleanUpChampionshipSaveGameInfo();
}

void Menue::ContinueChampionship() {
    //we need to set parameter championShipMode = true!
    ShowRaceSelection(currSelMenueSingleEntry, true);
}

void Menue::ShowRaceMenue() {
    mMenueUserInactiveTimer = 0.0f;

    //show Race Menue
    //with item Race selected
    currSelMenuePage = this->RaceMenuePage;
    currSelMenueSingleEntry = this->SelectChampionsship;

    RealignMenuePageItems();

    //initial setup for type writer effect
    //of main menue
    if (MENUE_ENABLETYPEWRITEREFFECT) {
        typeWriterEffectNextCharacterAbsTime = absoluteTime;
        currNrCharsShownCnter = 0;
        finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);
    }

    currMenueState = MENUE_STATE_TYPETEXT;
    lastAnimationUpdateAbsTime = absoluteTime;
}

void Menue::ShowChampionshipMenue(bool enaWindowAnimation) {
    mMenueUserInactiveTimer = 0.0f;

    //when a championship race is finished
    //the original game returns to the championship
    //top menue
    currSelMenuePage = this->ChampionshipMenuePage;

    //if there is currently a championship active, then allow to select
    //the continue championship menue entry
    //if not then do not activate this entry, in this case the player
    //needs to first create a new championship, same is true for saving championship
    if (!mGameAssets->CanPlayerCurrentlySelectCraftDuringChampionship()) {
        //championship is already created and ongoing
        currSelMenueSingleEntry = this->ContinueChampionshipEntry;
        this->ContinueChampionshipEntry->itemSelectable = true;
        this->SaveChampionshipEntry->itemSelectable = true;
    } else {
        //player needs to first create a new championship
        currSelMenueSingleEntry = this->NewChampionshipEntry;
        this->ContinueChampionshipEntry->itemSelectable = false;
        this->SaveChampionshipEntry->itemSelectable = false;
    }

    //initial setup for type writer effect
    //of main menue
    if (MENUE_ENABLETYPEWRITEREFFECT) {
        typeWriterEffectNextCharacterAbsTime = absoluteTime;
        currNrCharsShownCnter = 0;
        finalNrChardsShownMenuePageFinished = this->GetNrOfCharactersOnMenuePage(currSelMenuePage);
    }

    RealignMenuePageItems();

    if (!enaWindowAnimation) {
        currMenueState = MENUE_STATE_TYPETEXT;
        lastAnimationUpdateAbsTime = absoluteTime;
    } else {
        //activate menue animation
        currSelWindowAnimation = windowMenueAnimationStartGame;
        currMenueWindowAnimationIdx = 0;
        currMenueWindowAnimationFinalIdx = (irr::u8)(windowMenueAnimationStartGame->coordVec.size()) - 1;
        currMenueState = MENUE_STATE_TRANSITION;
        lastAnimationUpdateAbsTime = absoluteTime;
    }
}

//parameter overallPoints just controls which header text is used
void Menue::ShowPointsTablePage(std::vector<PointTableEntryStruct*>* pointTable, bool overallPoints = false) {
   //if nullptr simply return so that we do
   //not crash
   if (pointTable == nullptr)
       return;

   //we want to output table sorted starting with highest number of points
   std::vector<std::pair<irr::u16, char*>> sortedList;
   sortedList.clear();

   std::vector<PointTableEntryStruct*>::iterator itEntry;

   for (itEntry = pointTable->begin(); itEntry != pointTable->end(); ++itEntry) {
       sortedList.push_back(std::make_pair((*itEntry)->pointVal, (*itEntry)->namePlayer));
   }

   //now sort the vector with decreasing points
   std::sort(sortedList.rbegin(), sortedList.rend());

   //prepare text for this page
   pointsTablePageTextVec = new std::vector<MenueTextLabel*>();
   pointsTablePageTextVec->clear();

   MenueTextLabel* newLabel;
   irr::u32 textYcoord;
   char* pointValStr;
   irr::u32 nrCharsOverall = 0;

   //first add menue page title text
   newLabel = new MenueTextLabel();

   if (!overallPoints) {
        newLabel->text = strdup("POINTS");
   } else {
        newLabel->text = strdup("CHAMPIONSHIP TABLE");
   }

   newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedEntryFont;

   //calculate header text position
   newLabel->drawPositionTxt.X = (this->mGame->mScreenRes.Width / 2) -
           this->mGame->mGameTexts->GetWidthPixelsGameText(newLabel->text, newLabel->whichFont) / 2;
   newLabel->drawPositionTxt.Y = 56;

   newLabel->drawPositionTxt.Y = (newLabel->drawPositionTxt.Y * mGame->mScreenRes.Height) / 400;

   //Reuse the visible bool variable in the MenueTextLabel struct
   //here for another purpose. If we set visible to true we know
   //that during the Cleanup process of variables of pointPageTable
   //in function CleanupPointsTablePage we need to free also the
   //pntr to the text variable; if visible is false, then we must not
   //free the text string pointer!
   newLabel->visible = true; //set to true, free text pointer afterwards!

   pointsTablePageTextVec->push_back(newLabel);

   nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   irr::u8 cnt = 0;

   //loop over all players we want to show
   for (auto it4 = sortedList.begin(); it4 != sortedList.end(); ++it4) {
       //first calculate textYcoord according to entry line
       textYcoord = 100 + cnt * 30;

       //for the player name
       newLabel = new MenueTextLabel();
       newLabel->drawPositionTxt.X = 79;
       newLabel->drawPositionTxt.Y = textYcoord;

       //use the player name
       newLabel->text = strdup((*it4).second);
       newLabel->visible = true; //we must free this text pointer!

       //for the first 3 entries use white font color
       //for the remaining entries use header color
       if (cnt < 3) {
           newLabel->whichFont = mGame->mGameTexts->GameMenueSelectedItemFont;
       } else {
           newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedEntryFont;
       }

       ScalePositionMenueTextLabel(*newLabel);

       pointsTablePageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));

       //the actual points value
       pointValStr = new char[5];
       //use the number of points
       sprintf(pointValStr, "%u", (*it4).first);

       newLabel = new MenueTextLabel();

       //for the first 3 entries use white font color
       //for the remaining entries use header color
       if (cnt < 3) {
           newLabel->whichFont = mGame->mGameTexts->GameMenueSelectedItemFont;
       } else {
           newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedEntryFont;
       }

       newLabel->drawPositionTxt.X = 494;
       newLabel->drawPositionTxt.Y = textYcoord;

       newLabel->text = strdup(pointValStr);

       delete[] pointValStr;

       newLabel->visible = true; //set to true, free text pointer afterwards!

       ScalePositionMenueTextLabel(*newLabel);

       pointsTablePageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));

       cnt++;
   }

   //now we have all the text prepared
   currSelMenuePage = this->pointsTablePage;

   //there is a non visible dummy menue entry
   //we need to use it, because we need at least one item
   currSelMenueSingleEntry = pointsTableMenueDummyEntry;

   //do we use typewriter effect?
   if (MENUE_ENABLETYPEWRITEREFFECT) {
     //reset typewriter effect to build
     //next page correctly
     currNrCharsShownCnter = 0;
     finalNrChardsShownMenuePageFinished = nrCharsOverall;
     typeWriterEffectNextCharacterAbsTime = absoluteTime;

     //for typewriter effect we need to set menue state again
     //to typewriter inbetween state
     currMenueState = MENUE_STATE_TYPETEXT;
   } else {
     //no skip this effect
     currMenueState = MENUE_STATE_SELACTIVE;
   }
}

void Menue::CleanupPointsTablePage() {
    std::vector<MenueTextLabel*>::iterator it;
    MenueTextLabel* labelPntr;

    for (it = this->pointsTablePageTextVec->begin(); it != this->pointsTablePageTextVec->end(); ) {
        labelPntr = (*it);

        it = this->pointsTablePageTextVec->erase(it);

        //Reuse the visible bool variable in the MenueTextLabel struct
        //here for another purpose. If we set visible to true we know
        //that during the Cleanup process of variables of pointsTable screen
        //in function CleanupPointsTablePage we need to free also the
        //pntr to the text variable; if visible is false, then we must not
        //free the text string pointer!
        if (labelPntr->visible) {
           //visible was set to true, free text pointer!
           //text was malloced, we NEED to use free!
           free(labelPntr->text);
        }

        //delete MenueTextLabel as well
        delete labelPntr;
    }

    //also delete the vector of MenueTextLabels we have used
    delete pointsTablePageTextVec;
}

void Menue::CleanupHighScorepage() {
    std::vector<MenueTextLabel*>::iterator it;
    MenueTextLabel* labelPntr;

    for (it = this->highScorePageTextVec->begin(); it != this->highScorePageTextVec->end(); ) {
        labelPntr = (*it);

        it = this->highScorePageTextVec->erase(it);

        //Reuse the visible bool variable in the MenueTextLabel struct
        //here for another purpose. If we set visible to true we know
        //that during the Cleanup process of variables of highscore screen
        //in function CleanupHighScorepage we need to delete also the
        //pntr to the text variable; if visible is false, then we must not
        //delete the text string pointer!
        if (labelPntr->visible) {
           //visible was set to true, free text pointer
           //we malloced the text, therefore we need to use
           //free here! not delete!!!
           free(labelPntr->text);
        }

        //delete MenueTextLabel as well
        delete labelPntr;
    }

    //also delete the vector of MenueTextLabels we have used
    delete highScorePageTextVec;

    //go back to main menue top page
    ShowMainMenue();
}

void Menue::ScalePositionMenueTextLabel(MenueTextLabel& whichLabel) {
    whichLabel.drawPositionTxt.X = (whichLabel.drawPositionTxt.X * mGame->mScreenRes.Width) / 640;
    whichLabel.drawPositionTxt.Y = (whichLabel.drawPositionTxt.Y * mGame->mScreenRes.Height) / 400;
}

void Menue::ShowHighscore() {
   //request pointer to highscore table values
   std::vector<HighScoreEntryStruct*>* pntrTable;

   pntrTable = this->mGameAssets->GetHighScoreTable();

   //if function returns nullptr there was an
   //unexpected issue
   if (pntrTable == nullptr)
       return;

   //prepare text for this page
   highScorePageTextVec = new std::vector<MenueTextLabel*>();
   highScorePageTextVec->clear();

   MenueTextLabel* newLabel;
   irr::u32 textYcoord;
   char* highScoreValStr;
   char* assessementStr;
   irr::u32 nrCharsOverall = 0;

   //first add menue page title text
   newLabel = new MenueTextLabel();
   newLabel->drawPositionTxt.X = 264;
   newLabel->drawPositionTxt.Y = 45;
   newLabel->text = strdup("HISCORES");
   newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;

   ScalePositionMenueTextLabel(*newLabel);

   //Reuse the visible bool variable in the MenueTextLabel struct
   //here for another purpose. If we set visible to true we know
   //that during the Cleanup process of variables of highscore screen
   //in function CleanupHighScorepage we need to free also the
   //pntr to the text variable; if visible is false, then we must not
   //free the text string pointer!
   newLabel->visible = true; //set to true, free text pointer afterwards!

   highScorePageTextVec->push_back(newLabel);

   nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   //loop over all 19 entries we want to show
   for (irr::u8 cnt = 0; cnt < 19; cnt++) {
       //first calculate textYcoord according to entry line
       textYcoord = 75 + cnt * 14;

       //for the player name
       newLabel = new MenueTextLabel();
       newLabel->drawPositionTxt.X = 56;
       newLabel->drawPositionTxt.Y = textYcoord;
       newLabel->text = strdup(pntrTable->at(cnt)->namePlayer);
       newLabel->visible = true; //we must free this text pointer!
       newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;

       ScalePositionMenueTextLabel(*newLabel);

       highScorePageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));

       //for the highscore value
       highScoreValStr = new char[5];
       sprintf(highScoreValStr, "%u", pntrTable->at(cnt)->highscoreVal);

       newLabel = new MenueTextLabel();
       newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
       newLabel->drawPositionTxt.X =
               313 - mGame->mGameTexts->GetWidthPixelsGameText(
                   highScoreValStr,
                   newLabel->whichFont) / 2;

       newLabel->drawPositionTxt.Y = textYcoord;
       newLabel->text = strdup(highScoreValStr);

       delete[] highScoreValStr;

       newLabel->visible = true; //set to true, free text pointer afterwards!

       ScalePositionMenueTextLabel(*newLabel);

       highScorePageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));

       //for the player assessement string
       assessementStr =
               this->mGameAssets->GetDriverAssessementString(
                   pntrTable->at(cnt)->playerAssessementVal);

       newLabel = new MenueTextLabel();
       newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
       newLabel->drawPositionTxt.X =
               592 - mGame->mGameTexts->GetWidthPixelsGameText(
                   assessementStr,
                   newLabel->whichFont);

       newLabel->drawPositionTxt.Y = textYcoord;
       newLabel->text = strdup(assessementStr);
       newLabel->visible = true; //we must free this text pointer!

       ScalePositionMenueTextLabel(*newLabel);

       highScorePageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));
   }

   //now we have all the text prepared
   currSelMenuePage = this->gameHighscoreMenuePage;

   //there is a non visible dummy menue entry
   //we need to use it, because we need at least one item
   currSelMenueSingleEntry = gameHiscoreMenueDummyEntry;

   absTimeElapsedAtHighScorePage = 0.0f;

   //do we use typewriter effect?
   if (MENUE_ENABLETYPEWRITEREFFECT) {
     //reset typewriter effect to build
     //next page correctly
     currNrCharsShownCnter = 0;
     finalNrChardsShownMenuePageFinished = nrCharsOverall;
     typeWriterEffectNextCharacterAbsTime = absoluteTime;

     //for typewriter effect we need to set menue state again
     //to typewriter inbetween state
     currMenueState = MENUE_STATE_TYPETEXT;
   } else {
     //no skip this effect
     currMenueState = MENUE_STATE_SELACTIVE;
   }
}

void Menue::CleanupRaceStatsPage() {
    std::vector<MenueTextLabel*>::iterator it;
    MenueTextLabel* labelPntr;

    for (it = this->raceStatsPageTextVec->begin(); it != this->raceStatsPageTextVec->end(); ) {
        labelPntr = (*it);

        it = this->raceStatsPageTextVec->erase(it);

        //Reuse the visible bool variable in the MenueTextLabel struct
        //here for another purpose. If we set visible to true we know
        //that during the Cleanup process of variables of highscore screen
        //in function CleanupRaceStatsPage we need to free also the
        //pntr to the text variable; if visible is false, then we must not
        //free the text string pointer!
        if ((*it)->visible) {
           //visible was set to true, free text pointer!
           //text was malloced, we NEED to use free!
           free(labelPntr->text);
        }

        //delete MenueTextLabel as well
        delete labelPntr;
    }

    //also delete the vector of MenueTextLabels we have used
    delete raceStatsPageTextVec;
}

void Menue::ShowRaceStats(std::vector<RaceStatsEntryStruct*>* finalRaceStatistics) {
   //prepare text for this page
   raceStatsPageTextVec = new std::vector<MenueTextLabel*>();
   raceStatsPageTextVec->clear();

   MenueTextLabel* newLabel;
   irr::u32 columnXcoord;
   irr::u32 rowYCoord;
   irr::u32 nrCharsOverall = 0;

   //******************** Draw title ****************

   //first add menue page title text
   newLabel = new MenueTextLabel();
   newLabel->drawPositionTxt.X = 225;
   newLabel->drawPositionTxt.Y = 25;
   newLabel->text = strdup("STATISTICS");
   newLabel->whichFont = mGame->mGameTexts->GameMenueSelectedItemFont;

   ScalePositionMenueTextLabel(*newLabel);

   //Reuse the visible bool variable in the MenueTextLabel struct
   //here for another purpose. If we set visible to true we know
   //that during the Cleanup process of variables of highscore screen
   //in function CleanupHighScorepage we need to free also the
   //pntr to the text variable; if visible is false, then we must not
   //free the text string pointer!
   newLabel->visible = true; //set to true, free text pointer afterwards!

   raceStatsPageTextVec->push_back(newLabel);

   nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   // ****************** Write player names ***********

   irr::u8 nrPlayers = (irr::u8)(finalRaceStatistics->size());

   //loop over all available entries we want to show
   for (irr::u8 cnt = 0; cnt < nrPlayers; cnt = cnt + 2) {
        //first calculate columnXcoord according to entry column from
        //left of screen to right
        columnXcoord = 50 + cnt * 78;

        //for the player name
        newLabel = new MenueTextLabel();
        newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;
        newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                    finalRaceStatistics->at(cnt)->playerName, newLabel->whichFont) / 2;

        //even player entries draw in the upper name line
        newLabel->drawPositionTxt.Y = 59;

        newLabel->text = strdup(finalRaceStatistics->at(cnt)->playerName);
        newLabel->visible = true; //set to true, free text pointer afterwards!

        ScalePositionMenueTextLabel(*newLabel);

        raceStatsPageTextVec->push_back(newLabel);

        nrCharsOverall += (irr::u32)(strlen(newLabel->text));
   }

   //loop over all available entries we want to show
   for (irr::u8 cnt = 1; cnt < nrPlayers; cnt = cnt + 2) {
        //first calculate columnXcoord according to entry column from
        //left of screen to right
        columnXcoord = 50 + cnt * 78;

        //for the player name
        newLabel = new MenueTextLabel();
        newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;
        newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                    finalRaceStatistics->at(cnt)->playerName, newLabel->whichFont) / 2;

        //uneven player entries, draw the name in lower line
        newLabel->drawPositionTxt.Y = 75;

        newLabel->text = strdup(finalRaceStatistics->at(cnt)->playerName);
        newLabel->visible = true; //set to true, free text pointer afterwards!

        ScalePositionMenueTextLabel(*newLabel);

        raceStatsPageTextVec->push_back(newLabel);

        nrCharsOverall += (irr::u32)(strlen(newLabel->text));
   }

   // ****************** HIT ACCURACY ***********

   newLabel = new MenueTextLabel();
   newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
   newLabel->drawPositionTxt.X = 235;
   newLabel->drawPositionTxt.Y = 91;

   newLabel->text = strdup("HIT ACCURACY");
   newLabel->visible = true; //we must free this text pointer!

   ScalePositionMenueTextLabel(*newLabel);

   raceStatsPageTextVec->push_back(newLabel);

   nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   //loop over all available entries we want to show
   for (irr::u8 cnt = 0; cnt < nrPlayers; cnt++) {
       //first calculate columnXcoord according to entry column from
       //left of screen to right
       columnXcoord = 50 + cnt * 78;
       rowYCoord = 113 + 0 * 32;

       char* newText = new char[20];

       newLabel = new MenueTextLabel();
       newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;

       sprintf(newText, "%u", finalRaceStatistics->at(cnt)->hitAccuracy);
       strcat(newText, "%");

       newLabel->text = strdup(newText);

       delete[] newText;

       newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                 newLabel->text, newLabel->whichFont) / 2;

       newLabel->drawPositionTxt.Y = rowYCoord - mGame->mGameTexts->GetHeightPixelsGameText(
       newLabel->text, newLabel->whichFont) / 2;

       newLabel->visible = true; //we must free this text pointer!

       ScalePositionMenueTextLabel(*newLabel);

       raceStatsPageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));
   }

   // ****************** KILLS ***********

   newLabel = new MenueTextLabel();
   newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
   newLabel->drawPositionTxt.X = 287;
   newLabel->drawPositionTxt.Y = 123;

   newLabel->text = strdup("KILLS");
   newLabel->visible = true; //we must free this text pointer!

   ScalePositionMenueTextLabel(*newLabel);

   raceStatsPageTextVec->push_back(newLabel);

   nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   //loop over all available entries we want to show
   for (irr::u8 cnt = 0; cnt < nrPlayers; cnt++) {
       //first calculate columnXcoord according to entry column from
       //left of screen to right
       columnXcoord = 50 + cnt * 78;
       rowYCoord = 113 + 1 * 32;

       char* newText = new char[20];

       newLabel = new MenueTextLabel();
       newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;

       sprintf(newText, "%u", finalRaceStatistics->at(cnt)->nrKills);

       newLabel->text = strdup(newText);

       delete[] newText;

       newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                 newLabel->text, newLabel->whichFont) / 2;

       newLabel->drawPositionTxt.Y = rowYCoord - mGame->mGameTexts->GetHeightPixelsGameText(
       newLabel->text, newLabel->whichFont) / 2;

       newLabel->visible = true; //we must free this text pointer!

       ScalePositionMenueTextLabel(*newLabel);

       raceStatsPageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));
   }

   // ****************** DEATHS ***********

   newLabel = new MenueTextLabel();
   newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
   newLabel->drawPositionTxt.X = 278;
   newLabel->drawPositionTxt.Y = 155;

   newLabel->text = strdup("DEATHS");
   newLabel->visible = true; //we must free this text pointer!

   ScalePositionMenueTextLabel(*newLabel);

   raceStatsPageTextVec->push_back(newLabel);

   nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   //loop over all available entries we want to show
   for (irr::u8 cnt = 0; cnt < nrPlayers; cnt++) {
       //first calculate columnXcoord according to entry column from
       //left of screen to right
       columnXcoord = 50 + cnt * 78;
       rowYCoord = 113 + 2 * 32;

       char* newText = new char[20];

       newLabel = new MenueTextLabel();
       newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;

       sprintf(newText, "%u", finalRaceStatistics->at(cnt)->nrDeaths);

       newLabel->text = strdup(newText);

       delete[] newText;

       newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                 newLabel->text, newLabel->whichFont) / 2;

       newLabel->drawPositionTxt.Y = rowYCoord - mGame->mGameTexts->GetHeightPixelsGameText(
       newLabel->text, newLabel->whichFont) / 2;

       newLabel->visible = true; //we must free this text pointer!

       ScalePositionMenueTextLabel(*newLabel);

       raceStatsPageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));
   }

   // ****************** AVERAGE LAP ***********

   newLabel = new MenueTextLabel();
   newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
   newLabel->drawPositionTxt.X = 241;
   newLabel->drawPositionTxt.Y = 187;

   newLabel->text = strdup("AVERAGE LAP");
   newLabel->visible = true; //we must free this text pointer!

   ScalePositionMenueTextLabel(*newLabel);

   raceStatsPageTextVec->push_back(newLabel);

   nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   //loop over all available entries we want to show
   for (irr::u8 cnt = 0; cnt < nrPlayers; cnt++) {
       //first calculate columnXcoord according to entry column from
       //left of screen to right
       columnXcoord = 50 + cnt * 78;
       rowYCoord = 113 + 3 * 32;

       char* newText = new char[20];

       newLabel = new MenueTextLabel();
       newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;

       sprintf(newText, "%u", finalRaceStatistics->at(cnt)->avgLapTime);

       newLabel->text = strdup(newText);

       delete[] newText;

       newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                 newLabel->text, newLabel->whichFont) / 2;

       newLabel->drawPositionTxt.Y = rowYCoord - mGame->mGameTexts->GetHeightPixelsGameText(
       newLabel->text, newLabel->whichFont) / 2;

       newLabel->visible = true; //we must free this text pointer!

       ScalePositionMenueTextLabel(*newLabel);

       raceStatsPageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));
   }

   // ****************** BEST LAP ***********

    newLabel = new MenueTextLabel();
    newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
    newLabel->drawPositionTxt.X = 263;
    newLabel->drawPositionTxt.Y = 219;

    newLabel->text = strdup("BEST LAP");
    newLabel->visible = true; //we must free this text pointer!

    ScalePositionMenueTextLabel(*newLabel);

    raceStatsPageTextVec->push_back(newLabel);

    nrCharsOverall += (irr::u32)(strlen(newLabel->text));

    //loop over all available entries we want to show
    for (irr::u8 cnt = 0; cnt < nrPlayers; cnt++) {
        //first calculate columnXcoord according to entry column from
        //left of screen to right
        columnXcoord = 50 + cnt * 78;
        rowYCoord = 113 + 4 * 32;

        char* newText = new char[20];

        newLabel = new MenueTextLabel();
        newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;

        sprintf(newText, "%u", finalRaceStatistics->at(cnt)->bestLapTime);

        newLabel->text = strdup(newText);

        delete[] newText;

        newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                  newLabel->text, newLabel->whichFont) / 2;

        newLabel->drawPositionTxt.Y = rowYCoord - mGame->mGameTexts->GetHeightPixelsGameText(
        newLabel->text, newLabel->whichFont) / 2;

        newLabel->visible = true; //we must free this text pointer!

        ScalePositionMenueTextLabel(*newLabel);

        raceStatsPageTextVec->push_back(newLabel);

        nrCharsOverall += (irr::u32)(strlen(newLabel->text));
    }

    // ****************** RACE TIME ***********
    newLabel = new MenueTextLabel();
    newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
    newLabel->drawPositionTxt.X = 255;
    newLabel->drawPositionTxt.Y = 251;

    newLabel->text = strdup("RACE TIME");
    newLabel->visible = true; //we must free this text pointer!

    ScalePositionMenueTextLabel(*newLabel);

    raceStatsPageTextVec->push_back(newLabel);

    nrCharsOverall += (irr::u32)(strlen(newLabel->text));

    //loop over all available entries we want to show
    for (irr::u8 cnt = 0; cnt < nrPlayers; cnt++) {
        //first calculate columnXcoord according to entry column from
        //left of screen to right
        columnXcoord = 50 + cnt * 78;
        rowYCoord = 113 + 5 * 32;

        char* newText = new char[20];

        newLabel = new MenueTextLabel();
        newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;

        sprintf(newText, "%u", finalRaceStatistics->at(cnt)->raceTime);

        newLabel->text = strdup(newText);

        delete[] newText;

        newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                  newLabel->text, newLabel->whichFont) / 2;

        newLabel->drawPositionTxt.Y = rowYCoord - mGame->mGameTexts->GetHeightPixelsGameText(
        newLabel->text, newLabel->whichFont) / 2;

        newLabel->visible = true; //we must free this text pointer!

        ScalePositionMenueTextLabel(*newLabel);

        raceStatsPageTextVec->push_back(newLabel);

        nrCharsOverall += (irr::u32)(strlen(newLabel->text));
    }

    // ****************** RATING ***********
    newLabel = new MenueTextLabel();
    newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
    newLabel->drawPositionTxt.X = 281;
    newLabel->drawPositionTxt.Y = 283;

    newLabel->text = strdup("RATING");
    newLabel->visible = true; //we must free this text pointer!

    ScalePositionMenueTextLabel(*newLabel);

    raceStatsPageTextVec->push_back(newLabel);

    nrCharsOverall += (irr::u32)(strlen(newLabel->text));

    //loop over all available entries we want to show
    for (irr::u8 cnt = 0; cnt < nrPlayers; cnt++) {
        //first calculate columnXcoord according to entry column from
        //left of screen to right
        columnXcoord = 50 + cnt * 78;
        rowYCoord = 113 + 6 * 32;

        char* newText = new char[20];

        newLabel = new MenueTextLabel();
        newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;

        sprintf(newText, "%u", finalRaceStatistics->at(cnt)->rating);
        strcat(newText, "/20");

        newLabel->text = strdup(newText);

        delete[] newText;

        newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                  newLabel->text, newLabel->whichFont) / 2;

        newLabel->drawPositionTxt.Y = rowYCoord - mGame->mGameTexts->GetHeightPixelsGameText(
        newLabel->text, newLabel->whichFont) / 2;

        newLabel->visible = true; //we must free this text pointer!

        ScalePositionMenueTextLabel(*newLabel);

        raceStatsPageTextVec->push_back(newLabel);

        nrCharsOverall += (irr::u32)(strlen(newLabel->text));
    }

    // ****************** POSITION ***********
    newLabel = new MenueTextLabel();
    newLabel->whichFont = mGame->mGameTexts->GameMenueUnselectedTextSmallSVGA;
    newLabel->drawPositionTxt.X = 269;
    newLabel->drawPositionTxt.Y = 315;

    newLabel->text = strdup("POSITION");
    newLabel->visible = true; //we must free this text pointer!

    ScalePositionMenueTextLabel(*newLabel);

    raceStatsPageTextVec->push_back(newLabel);

    nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   //loop over all available entries we want to show
   for (irr::u8 cnt = 0; cnt < nrPlayers; cnt++) {
       //first calculate columnXcoord according to entry column from
       //left of screen to right
       columnXcoord = 50 + cnt * 78;
       rowYCoord = 113 + 7 * 32;

       char* newText = new char[20];

       newLabel = new MenueTextLabel();
       newLabel->whichFont = mGame->mGameTexts->GameMenueWhiteTextSmallSVGA;

       sprintf(newText, "%u", finalRaceStatistics->at(cnt)->racePosition);

       newLabel->text = strdup(newText);

       delete[] newText;

       newLabel->drawPositionTxt.X = columnXcoord - mGame->mGameTexts->GetWidthPixelsGameText(
                 newLabel->text, newLabel->whichFont) / 2;

       newLabel->drawPositionTxt.Y = rowYCoord - mGame->mGameTexts->GetHeightPixelsGameText(
       newLabel->text, newLabel->whichFont) / 2;

       newLabel->visible = true; //we must free this text pointer!

       ScalePositionMenueTextLabel(*newLabel);

       raceStatsPageTextVec->push_back(newLabel);

       nrCharsOverall += (irr::u32)(strlen(newLabel->text));
   }

   // ****************** MAIN PLAYER ASSESSEMENT ***********
   char* playerAssessement;
   playerAssessement = this->mGameAssets->GetDriverAssessementString(
               this->mGameAssets->GetNumberDriverAssessementStrings() - finalRaceStatistics->at(0)->rating);

   newLabel = new MenueTextLabel();
   newLabel->whichFont = mGame->mGameTexts->GameMenueSelectedItemFont;
   newLabel->drawPositionTxt.X = 320 - mGame->mGameTexts->GetWidthPixelsGameText(
               playerAssessement, newLabel->whichFont) / 2;

   newLabel->drawPositionTxt.Y = 363;

   newLabel->text = strdup(playerAssessement);

   newLabel->visible = true; //we must free this text pointer!

   ScalePositionMenueTextLabel(*newLabel);

   raceStatsPageTextVec->push_back(newLabel);

   nrCharsOverall += (irr::u32)(strlen(newLabel->text));

   //now we have all the text prepared
   currSelMenuePage = this->raceStatsMenuePage;

   //there is a non visible dummy menue entry
   //we need to use it, because we need at least one item
   currSelMenueSingleEntry = this->raceStatsMenueDummyEntry;

   //do we use typewriter effect?
   if (MENUE_ENABLETYPEWRITEREFFECT) {
     //reset typewriter effect to build
     //next page correctly
     currNrCharsShownCnter = 0;
     finalNrChardsShownMenuePageFinished = nrCharsOverall;
     typeWriterEffectNextCharacterAbsTime = absoluteTime;

     //for typewriter effect we need to set menue state again
     //to typewriter inbetween state
     currMenueState = MENUE_STATE_TYPETEXT;
   } else {
     //no skip this effect
     currMenueState = MENUE_STATE_SELACTIVE;
   }
}

Menue::~Menue() {
    CleanUpStatLabels();
}
