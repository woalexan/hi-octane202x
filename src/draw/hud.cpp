/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "hud.h"

//a negative altPanelTexNr input value means no alternative texture (image) is used
void HUD::Add1PlayerHudDisplayPart(std::vector<HudDisplayPart*>* addToWhichBar,
                                   irr::u16 panelTexNr, irr::u16 drawPosX, irr::u16 drawPosY, irr::s16 altPanelTexNr) {

    char fileName[50];
    char fname[20];

    //first build the name for the image file to load
    strcpy(fileName, "extract/hud1player/panel0-1-");
    sprintf (fname, "%0*u.bmp", 4, panelTexNr);
    strcat(fileName, fname);

    HudDisplayPart* newPart = new HudDisplayPart();
    newPart->texture = mInfra->mDriver->getTexture(fileName);
    //make image transparent, take color at pixel coord 0,0 for transparency color
    mInfra->mDriver->makeColorKeyTexture(newPart->texture, irr::core::position2d<irr::s32>(0,0));

    newPart->sizeTex = newPart->texture->getSize();
    newPart->drawScrPosition.set(drawPosX, drawPosY);

    //I decided to also prepare a member variable for the image
    //source rect, so that it is always already available
    //when we draw the Hud over and over again, so that we save CPU cycles
    newPart->sourceRect.UpperLeftCorner.X = 0;
    newPart->sourceRect.UpperLeftCorner.Y = 0;
    newPart->sourceRect.LowerRightCorner.X = newPart->sizeTex.Width;
    newPart->sourceRect.LowerRightCorner.Y = newPart->sizeTex.Height;

    if (altPanelTexNr < 0) {
        //no alternative image specified
        newPart->altTexture = NULL;
    } else {
        //alternative image specified
        char altFileName[50];

        //build the name for the alternative image file to load
        strcpy(altFileName, "extract/hud1player/panel0-1-");
        sprintf (fname, "%0*u.bmp", 4, (irr::u16)(altPanelTexNr));
        strcat(altFileName, fname);

        //load/add the alternative texture
        newPart->altTexture = mInfra->mDriver->getTexture(altFileName);
        mInfra->mDriver->makeColorKeyTexture(newPart->altTexture, irr::core::position2d<irr::s32>(0,0));
    }

    //add the new image/texture/piece to the
    //specified bar/image vector
    addToWhichBar->push_back(newPart);
}

void HUD::InitShieldBar() {
    //create dynamic variable
    shieldBar = new std::vector<HudDisplayPart*>;

    //initialize shield bar
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
    //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = -2;

    Add1PlayerHudDisplayPart(shieldBar, 71, 3 + Xoff   , 3);     //Piece 1
    Add1PlayerHudDisplayPart(shieldBar, 72, 41 + Xoff  , 3);     //Piece 2
    Add1PlayerHudDisplayPart(shieldBar, 73, 67 + Xoff  , 3);     //Piece 3
    Add1PlayerHudDisplayPart(shieldBar, 74, 75 + Xoff  , 3);     //Piece 4
    Add1PlayerHudDisplayPart(shieldBar, 75, 95 + Xoff  , 3);     //Piece 5
    Add1PlayerHudDisplayPart(shieldBar, 76, 113 + Xoff , 3);     //Piece 6

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitAmmoBar() {
    //create dynamic variable
    ammoBar = new std::vector<HudDisplayPart*>;

    //initialize ammo bar
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0

    int Xoff = + 2;
    irr::u16 scrSizeX = mInfra->mScreenRes.Width;

    Add1PlayerHudDisplayPart(ammoBar, 90, scrSizeX - 43 + Xoff   , 3);    //Piece 1
    Add1PlayerHudDisplayPart(ammoBar, 89, scrSizeX - 86 + Xoff   , 3);    //Piece 2
    Add1PlayerHudDisplayPart(ammoBar, 88, scrSizeX - 114 + Xoff  , 3);    //Piece 3
    Add1PlayerHudDisplayPart(ammoBar, 87, scrSizeX - 134 + Xoff  , 3);    //Piece 4
    Add1PlayerHudDisplayPart(ammoBar, 86, scrSizeX - 152 + Xoff  , 3);    //Piece 5
    Add1PlayerHudDisplayPart(ammoBar, 85, scrSizeX - 180 + Xoff  , 3);    //Piece 6

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitGasolineBar() {
    //create dynamic variable
    gasolineBar = new std::vector<HudDisplayPart*>;

    //initialize gasoline bar
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = -3;

    Add1PlayerHudDisplayPart(gasolineBar, 0  , 80 + Xoff    , 3);     //Piece 1
    Add1PlayerHudDisplayPart(gasolineBar, 1  , 135 + Xoff   , 3);     //Piece 2
    Add1PlayerHudDisplayPart(gasolineBar, 2  , 182 + Xoff   , 3);     //Piece 3
    Add1PlayerHudDisplayPart(gasolineBar, 3  , 217 + Xoff   , 3);     //Piece 4
    Add1PlayerHudDisplayPart(gasolineBar, 4  , 243 + Xoff   , 3);     //Piece 5
    Add1PlayerHudDisplayPart(gasolineBar, 5  , 268 + Xoff   , 3);     //Piece 6
    Add1PlayerHudDisplayPart(gasolineBar, 6  , 290 + Xoff   , 3);     //Piece 7
    Add1PlayerHudDisplayPart(gasolineBar, 7  , 311 + Xoff   , 3);     //Piece 8
    Add1PlayerHudDisplayPart(gasolineBar, 8  , 330 + Xoff   , 3);     //Piece 9
    Add1PlayerHudDisplayPart(gasolineBar, 9  , 348 + Xoff   , 3);     //Piece 10
    Add1PlayerHudDisplayPart(gasolineBar, 10 , 373 + Xoff   , 3);     //Piece 11
    Add1PlayerHudDisplayPart(gasolineBar, 11 , 398 + Xoff   , 3);     //Piece 12
    Add1PlayerHudDisplayPart(gasolineBar, 12 , 422 + Xoff   , 3);     //Piece 13
    Add1PlayerHudDisplayPart(gasolineBar, 13 , 448 + Xoff   , 3);     //Piece 14
    Add1PlayerHudDisplayPart(gasolineBar, 14 , 472 + Xoff   , 3);     //Piece 15
    Add1PlayerHudDisplayPart(gasolineBar, 15 , 497 + Xoff   , 3);     //Piece 16
    Add1PlayerHudDisplayPart(gasolineBar, 16 , 523 + Xoff   , 3);     //Piece 17

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitStartSignal() {
    //create dynamic variable
    startSignal = new std::vector<HudDisplayPart*>;

    //initialize start signal
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    /****************************
     * Start signal             *
     ****************************/

    //                                     signal not lit     alternative texture is the lit light
    //                                     |                  |
    Add1PlayerHudDisplayPart(startSignal, 280  , 50    , 11, 281);    //red light
    Add1PlayerHudDisplayPart(startSignal, 282  , 113   , 10, 283);    //yellow light
    Add1PlayerHudDisplayPart(startSignal, 284  , 176   , 10, 285);    //green light

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitBrokenGlas() {
    //initialize texture for broken glass
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    /****************************
     * Broken glas              *
     ****************************/

    //broken glas image
    brokenGlas = new HudDisplayPart();
    brokenGlas->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0229.bmp");
    mInfra->mDriver->makeColorKeyTexture(brokenGlas->texture, irr::core::position2d<irr::s32>(0,0));
    brokenGlas->sizeTex = brokenGlas->texture->getSize();
    brokenGlas->drawScrPosition.set(0, 0);
    brokenGlas->sourceRect.UpperLeftCorner.X = 0;
    brokenGlas->sourceRect.UpperLeftCorner.Y = 0;
    brokenGlas->sourceRect.LowerRightCorner.X = brokenGlas->sizeTex.Width;
    brokenGlas->sourceRect.LowerRightCorner.Y = brokenGlas->sizeTex.Height;

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitMGHeatBar() {
    //create dynamic variable
    mgHeatBar = new std::vector<HudDisplayPart*>;

    //initialize machine gun heat bar
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = 0;

    Add1PlayerHudDisplayPart(mgHeatBar, 91  , 506 + Xoff ,  29);     //Piece 1
    Add1PlayerHudDisplayPart(mgHeatBar, 92  , 521 + Xoff ,  29);     //Piece 2
    Add1PlayerHudDisplayPart(mgHeatBar, 93  , 527 + Xoff ,  29);     //Piece 3
    Add1PlayerHudDisplayPart(mgHeatBar, 94  , 533 + Xoff ,  29);     //Piece 4
    Add1PlayerHudDisplayPart(mgHeatBar, 95  , 539 + Xoff ,  29);     //Piece 5
    Add1PlayerHudDisplayPart(mgHeatBar, 96  , 545 + Xoff ,  29);     //Piece 6
    Add1PlayerHudDisplayPart(mgHeatBar, 97  , 551 + Xoff ,  29);     //Piece 7
    Add1PlayerHudDisplayPart(mgHeatBar, 98  , 557 + Xoff ,  29);     //Piece 8
    Add1PlayerHudDisplayPart(mgHeatBar, 99  , 569 + Xoff ,  29);     //Piece 9
    Add1PlayerHudDisplayPart(mgHeatBar, 100 , 581 + Xoff ,  29);     //Piece 10
    Add1PlayerHudDisplayPart(mgHeatBar, 101 , 589 + Xoff ,  29);     //Piece 11
    Add1PlayerHudDisplayPart(mgHeatBar, 102 , 597 + Xoff ,  29);     //Piece 12
    Add1PlayerHudDisplayPart(mgHeatBar, 103 , 605 + Xoff ,  29);     //Piece 13
    Add1PlayerHudDisplayPart(mgHeatBar, 104 , 613 + Xoff ,  29);     //Piece 14
    Add1PlayerHudDisplayPart(mgHeatBar, 105 , 621 + Xoff ,  29);     //Piece 15
    Add1PlayerHudDisplayPart(mgHeatBar, 106 , 629 + Xoff ,  29);     //Piece 16

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitThrottleBar() {
    //create dynamic variable
    throttleBar = new std::vector<HudDisplayPart*>;

    //initialize throttle bar
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = 0;

    Add1PlayerHudDisplayPart(throttleBar, 17  , 115 + Xoff ,  15,   36);     //Piece 1
    Add1PlayerHudDisplayPart(throttleBar, 18  , 139 + Xoff ,  15,   37);     //Piece 2
    Add1PlayerHudDisplayPart(throttleBar, 19  , 160 + Xoff ,  15,   38);     //Piece 3
    Add1PlayerHudDisplayPart(throttleBar, 20  , 185 + Xoff ,  15,   39);     //Piece 4
    Add1PlayerHudDisplayPart(throttleBar, 21  , 206 + Xoff ,  15,   40);     //Piece 5
    Add1PlayerHudDisplayPart(throttleBar, 22  , 229 + Xoff ,  15,   41);     //Piece 6
    Add1PlayerHudDisplayPart(throttleBar, 23  , 247 + Xoff ,  15,   42);     //Piece 7
    Add1PlayerHudDisplayPart(throttleBar, 24  , 268 + Xoff ,  15,   43);     //Piece 8
    Add1PlayerHudDisplayPart(throttleBar, 25  , 289 + Xoff ,  15,   44);     //Piece 9
    Add1PlayerHudDisplayPart(throttleBar, 26  , 308 + Xoff ,  15,   45);     //Piece 10
    Add1PlayerHudDisplayPart(throttleBar, 27  , 327 + Xoff ,  15,   46);     //Piece 11
    Add1PlayerHudDisplayPart(throttleBar, 28  , 346 + Xoff ,  15,   47);     //Piece 12
    Add1PlayerHudDisplayPart(throttleBar, 29  , 368 + Xoff ,  15,   48);     //Piece 13
    Add1PlayerHudDisplayPart(throttleBar, 30  , 394 + Xoff ,  15,   49);     //Piece 14
    Add1PlayerHudDisplayPart(throttleBar, 31  , 420 + Xoff ,  15,   50);     //Piece 15
    Add1PlayerHudDisplayPart(throttleBar, 32  , 445 + Xoff ,  15,   51);     //Piece 16
    Add1PlayerHudDisplayPart(throttleBar, 33  , 468 + Xoff ,  15,   52);     //Piece 17
    Add1PlayerHudDisplayPart(throttleBar, 34  , 491 + Xoff ,  15,   53);     //Piece 18
    Add1PlayerHudDisplayPart(throttleBar, 35  , 513 + Xoff ,  15,   54);     //Piece 19

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitSpeedBar() {
    //create dynamic variable
    speedBar = new std::vector<HudDisplayPart*>;

    //initialize booster bar
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = 0;

    Add1PlayerHudDisplayPart(speedBar, 55  , 125 + Xoff ,  27);     //Piece 1
    Add1PlayerHudDisplayPart(speedBar, 56  , 147 + Xoff ,  27);     //Piece 2
    Add1PlayerHudDisplayPart(speedBar, 57  , 168 + Xoff ,  27);     //Piece 3
    Add1PlayerHudDisplayPart(speedBar, 58  , 189 + Xoff ,  27);     //Piece 4
    Add1PlayerHudDisplayPart(speedBar, 59  , 210 + Xoff ,  27);     //Piece 5
    Add1PlayerHudDisplayPart(speedBar, 60  , 231 + Xoff ,  27);     //Piece 6
    Add1PlayerHudDisplayPart(speedBar, 61  , 249 + Xoff ,  27);     //Piece 7
    Add1PlayerHudDisplayPart(speedBar, 62  , 270 + Xoff ,  27);     //Piece 8
    Add1PlayerHudDisplayPart(speedBar, 63  , 291 + Xoff ,  27);     //Piece 9
    Add1PlayerHudDisplayPart(speedBar, 64  , 310 + Xoff ,  27);     //Piece 10
    Add1PlayerHudDisplayPart(speedBar, 65  , 327 + Xoff ,  27);     //Piece 11
    Add1PlayerHudDisplayPart(speedBar, 66  , 345 + Xoff ,  27);     //Piece 12
    Add1PlayerHudDisplayPart(speedBar, 67  , 361 + Xoff ,  27);     //Piece 13
    Add1PlayerHudDisplayPart(speedBar, 68  , 382 + Xoff ,  27);     //Piece 14
    Add1PlayerHudDisplayPart(speedBar, 69  , 421 + Xoff ,  27);     //Piece 15
    Add1PlayerHudDisplayPart(speedBar, 70  , 457 + Xoff ,  27);     //Piece 16

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitRacePosition() {
    //create dynamic variable
    currRacePlayerPosition = new std::vector<HudDisplayPart*>;
    numberPlayers = new std::vector<HudDisplayPart*>;

    //initialize race position characters
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0

    //**********************************************************
    //* First init characters for current player race position *
    //* This is the bigger grey character on the far left      *
    //**********************************************************
    int Xoff = 0;

    Add1PlayerHudDisplayPart(currRacePlayerPosition, 107  , 2 + Xoff ,  28);     //number 0
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 108  , 2 + Xoff ,  28);     //number 1
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 109  , 2 + Xoff ,  28);     //number 2
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 110  , 2 + Xoff ,  28);     //number 3
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 111  , 2 + Xoff ,  28);     //number 4
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 112  , 2 + Xoff ,  28);     //number 5
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 113  , 2 + Xoff ,  28);     //number 6
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 114  , 2 + Xoff ,  28);     //number 7
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 115  , 2 + Xoff ,  28);     //number 8
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 116  , 2 + Xoff ,  28);     //number 9
    Add1PlayerHudDisplayPart(currRacePlayerPosition, 137  , 53       ,  72);     //add the slash

    //*************************************************************
    //* 2nd init characters for overall number of players in race *
    //* This is the smaller grey character right from the slash   *
    //*************************************************************

    Xoff = 0;

    Add1PlayerHudDisplayPart(numberPlayers, 127  , 83 + Xoff ,  72);     //number 0
    Add1PlayerHudDisplayPart(numberPlayers, 128  , 83 + Xoff ,  72);     //number 1
    Add1PlayerHudDisplayPart(numberPlayers, 129  , 83 + Xoff ,  72);     //number 2
    Add1PlayerHudDisplayPart(numberPlayers, 130  , 83 + Xoff ,  72);     //number 3
    Add1PlayerHudDisplayPart(numberPlayers, 131  , 83 + Xoff ,  72);     //number 4
    Add1PlayerHudDisplayPart(numberPlayers, 132  , 83 + Xoff ,  72);     //number 5
    Add1PlayerHudDisplayPart(numberPlayers, 133  , 83 + Xoff ,  72);     //number 6
    Add1PlayerHudDisplayPart(numberPlayers, 134  , 83 + Xoff ,  72);     //number 7
    Add1PlayerHudDisplayPart(numberPlayers, 135  , 83 + Xoff ,  72);     //number 8
    Add1PlayerHudDisplayPart(numberPlayers, 136  , 83 + Xoff ,  72);     //number 9
    Add1PlayerHudDisplayPart(numberPlayers, 137  , 83 + Xoff ,  72);     //add the slash

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitUpgradeBar() {
    //create dynamic variable
    upgradeBar = new std::vector<HudDisplayPart*>;

    //initialize upgrade bar symbols
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    /****************************
     * Minigun                  *
     ****************************/

    Add1PlayerHudDisplayPart(upgradeBar, 267  , 149 ,  71);     //minigun symbol
    Add1PlayerHudDisplayPart(upgradeBar, 269  , 219 ,  71);     //minigun upgrade level 1 symbol
    Add1PlayerHudDisplayPart(upgradeBar, 269  , 233 ,  71);     //minigun upgrade level 2 symbol
    Add1PlayerHudDisplayPart(upgradeBar, 269  , 247 ,  71);     //minigun upgrade level 3 symbol

    /****************************
    * Missile                   *
    ****************************/

    Add1PlayerHudDisplayPart(upgradeBar, 270  , 265 ,  71);     //missile symbol
    Add1PlayerHudDisplayPart(upgradeBar, 269  , 318 ,  71);     //missile upgrade level 1 symbol
    Add1PlayerHudDisplayPart(upgradeBar, 269  , 332 ,  71);     //missile upgrade level 2 symbol
    Add1PlayerHudDisplayPart(upgradeBar, 269  , 346 ,  71);     //missile upgrade level 3 symbol

    /****************************
    * Booster                  *
    ****************************/

    Add1PlayerHudDisplayPart(upgradeBar, 273  , 364 ,  71);     //booster symbol
    Add1PlayerHudDisplayPart(upgradeBar, 274  , 415 ,  71);     //booster upgrade level 1 symbol
    Add1PlayerHudDisplayPart(upgradeBar, 274  , 429 ,  71);     //booster upgrade level 2 symbol
    Add1PlayerHudDisplayPart(upgradeBar, 274  , 443 ,  71);     //booster upgrade level 3 symbol

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::SetHUDState(irr::u8 newHUDState) {
    this->mHudState = newHUDState;
}

void HUD::CleanUpBannerMessage(BannerTextMessageStruct* msgToDeletePntr) {
    std::vector<BannerTextMessageStruct*>::iterator it;
    BannerTextMessageStruct* mPntr;

    if (this->bannerMessageVec->size() > 0 ) {
        //search for the message that should be deleted / cleaned up
        for (it = this->bannerMessageVec->begin(); it != this->bannerMessageVec->end();) {
            if ((*it) == msgToDeletePntr) {
                mPntr = (*it);
                //remove message struct from message vector
                it = bannerMessageVec->erase(it);

                //cleanup the message struct itself
                //first the dynamic allocated text array itself
                delete[]  mPntr->text;

                //now the struct itself
                delete mPntr;

                mPntr = NULL;
        } else ++it;
     }
  }
}

void HUD::RemoveAllPermanentBannerMessagesMsgVec() {
    std::vector<BannerTextMessageStruct*>::iterator it;
    BannerTextMessageStruct* mPntr;

    if (this->bannerMessageVec->size() > 0 ) {
        for (it = this->bannerMessageVec->begin(); it != this->bannerMessageVec->end();) {
            if ((*it)->permanentMsg) {
                mPntr = (*it);
                //remove message struct from message vector
                it = bannerMessageVec->erase(it);

                //cleanup the message struct itself
                //first the dynamic allocated text array itself
                delete[] mPntr->text;

                //now the struct itself
                delete mPntr;

                mPntr = NULL;
        } else ++it;
     }
  }
}

void HUD::CleanUpAllBannerMessages() {
    std::vector<BannerTextMessageStruct*>::iterator it;
    BannerTextMessageStruct* mPntr;

    if (this->bannerMessageVec->size() > 0 ) {
        //search for the message that should be deleted / cleaned up
        for (it = this->bannerMessageVec->begin(); it != this->bannerMessageVec->end();) {
                mPntr = (*it);
                //remove message struct from message vector
                it = bannerMessageVec->erase(it);

                //cleanup the message struct itself
                //first the dynamic allocated text array itself
                delete[] mPntr->text;

                //now the struct itself
                delete mPntr;

                mPntr = NULL;
        }
     }
}

//returns true if there is a next banner message
//available that can be shown, false otherwise
bool HUD::SelectNextBannerMessage() {
    if (this->bannerMessageVec->size() > 0) {
        //if there is a message with higher priority (warning)
        //select this one before
        std::vector<BannerTextMessageStruct*>::iterator it;

        //preselect the oldest message
        currShownBannerMsg = *bannerMessageVec->begin();

        //if there is a warning, take the oldest one and use it instead
        for (it = bannerMessageVec->begin(); it != bannerMessageVec->end(); ++it) {
            if ((*it)->isWarning) {
                currShownBannerMsg = (*it);
                break;
            }
        }

        //if we find a permanent message take this one instead, but start at the newest
        //message instead
        for (auto it2 = bannerMessageVec->rbegin(); it2 != bannerMessageVec->rend(); ++it2) {
            if ((*it2)->permanentMsg) {
                currShownBannerMsg = (*it2);
                break;
            }
        }

        //we have now the new message selected in currShownBanner Msg pointer
        return true;
    }

    return false;
}

void HUD::CancelAllPermanentBannerTextMsg() {
    if (currShownBannerMsg != NULL) {
        if (currShownBannerMsg->permanentMsg) {
            //remove and clean up current message
            CleanUpBannerMessage(currShownBannerMsg);

            //reset currently shown message pointer back to NULL
            //important, so that we do not crash during the next
            //banner text render operation
            currShownBannerMsg = NULL;

            RemoveAllPermanentBannerMessagesMsgVec();
        }
    }

    RemoveAllPermanentBannerMessagesMsgVec();
}

void HUD::BannerTextLogic(irr::f32 deltaTime) {
    bool newMsgSel = false;

    //is there a new message to be shown?
    if (currShownBannerMsg == NULL) {
       //verify if there is a new message to show
       newMsgSel = SelectNextBannerMessage();

       if (!newMsgSel && (currentBannerTextState > 0)) {
           //close banner again
           bannerDoesCurrentlyClose = true;
           bannerDoesCurrentlyOpen = false;
       }
    }

    if (newMsgSel || ((currentBannerTextState == 0) && (currShownBannerMsg != NULL))) {
        //we have a new message, depending on the current text banner state we have
        //to react correctly for correct rendering/animation of banner etc.

         //if we replace a current banner with warning active, but the new
         //one has no warning make sure that warning sound is turned off immediately
         if (mCurrentMessageWarningActive && !currShownBannerMsg->isWarning) {
                 this->monitorWhichPlayer->StopPlayingWarningSound();
                 mCurrentMessageWarningActive = false;
             }

         //depending on current banner state we have to react differently
         //case 1: currentBannerTextState == 7: just swap text
         if (currentBannerTextState < 7) {
             //does banner currently close? if so open again
             if (bannerDoesCurrentlyClose) {
                 //open banner again
                 bannerDoesCurrentlyClose = false;
                 bannerDoesCurrentlyOpen = true;
             } else if (!bannerDoesCurrentlyOpen)  {
                 currentBannerTextState = 1;
                 bannerDoesCurrentlyOpen = true;
                 bannerDoesCurrentlyClose = false;

                 //set negative timer time to execute next state change
                 //immediately
                 timerNextBannerState = -1.0f;
             }
         }
    }

    //if there is a banner text currently shown/should be shown render it
    if (currentBannerTextState > 0) {
        //first render graphical elements of banner (without the text)
        RenderTextBannerGraphics();

        timerNextBannerState -= deltaTime;

        //is banner text currently shown?
        if ((currentBannerTextState == 7) && (currShownBannerMsg != NULL)) {

            currShownBannerMsg->textAlreadyShownSec += deltaTime;

            if (!mCurrentMessageWarningActive && currShownBannerMsg->isWarning) {
                 if (this->monitorWhichPlayer->mRace->currPlayerFollow == this->monitorWhichPlayer) {
                    this->monitorWhichPlayer->StartPlayingWarningSound();
                 }
                 mCurrentMessageWarningActive = true;
             }

           //Draw the text
           //for Hud banner text we use HudWhiteTextBannerFont
           mInfra->mGameTexts->DrawGameText(currShownBannerMsg->text, mInfra->mGameTexts->HudWhiteTextBannerFont,
                                        currShownBannerMsg->textPosition);

           if (!currShownBannerMsg->permanentMsg) {
            currShownBannerMsg->textStillShownSec -= deltaTime;

            if (currShownBannerMsg->textStillShownSec <= 0) {
                if (mCurrentMessageWarningActive && currShownBannerMsg->isWarning) {
                    this->monitorWhichPlayer->StopPlayingWarningSound();
                    mCurrentMessageWarningActive = false;
                }

                //remove and clean up current message
                CleanUpBannerMessage(currShownBannerMsg);

                //reset currently shown message pointer back to NULL
                //important, so that we do not crash during the next
                //banner text render operation
                currShownBannerMsg = NULL;
             }
           }
        }

        if (timerNextBannerState < 0) {
            timerNextBannerState = WaitTimeBeforeNextBannerState;

            if (bannerDoesCurrentlyOpen) {
                currentBannerTextState++;

                if (currentBannerTextState >= 7) {
                    //if we need to play sound of warning start playing sound here
                    if (bannerDoesCurrentlyOpen) {
                        bannerDoesCurrentlyOpen = false;
                    }
                }
            } else if (bannerDoesCurrentlyClose) {
                currentBannerTextState--;

                if (currentBannerTextState <=0 ) {
                    currentBannerTextState = 0;
                    bannerDoesCurrentlyClose = false;
                }
            }
        }
    }
}

//0.. means no light lit
//with increasing value the start signal
//advances towards the final state
void HUD::SetStartSignalState(irr::u8 newSignalState) {
    mStartSignalState = newSignalState;
}

irr::u8 HUD::GetStartSignalState() {
    return mStartSignalState;
}

void HUD::DrawHUD1PlayerStartSignal(irr::f32 deltaTime) {

    bool redLit = false;
    bool yellowLit = false;
    bool greenLit = false;

    switch (this->mStartSignalState) {
        case DEF_HUD_STARTSIGNAL_RED_LIT: {
            redLit = true;
            break;
            }
    case DEF_HUD_STARTSIGNAL_YELLOW_LIT: {
            yellowLit = true;
            break;
          }
    case DEF_HUD_STARTSIGNAL_GREEN_LIT: {
        greenLit = true;
        break;
     }
    }

    if (redLit) {
         mInfra->mDriver->draw2DImage((*startSignal)[0]->altTexture, (*startSignal)[0]->drawScrPosition,
               (*startSignal)[0]->sourceRect, 0, *mColorSolid, true);
    } else {
        mInfra->mDriver->draw2DImage((*startSignal)[0]->texture, (*startSignal)[0]->drawScrPosition,
              (*startSignal)[0]->sourceRect, 0, *mColorSolid, true);
    }

    if (yellowLit) {
         mInfra->mDriver->draw2DImage((*startSignal)[1]->altTexture, (*startSignal)[1]->drawScrPosition,
               (*startSignal)[1]->sourceRect, 0, *mColorSolid, true);
    } else {
        mInfra->mDriver->draw2DImage((*startSignal)[1]->texture, (*startSignal)[1]->drawScrPosition,
              (*startSignal)[1]->sourceRect, 0, *mColorSolid, true);
    }

    if (greenLit) {
         mInfra->mDriver->draw2DImage((*startSignal)[2]->altTexture, (*startSignal)[2]->drawScrPosition,
               (*startSignal)[2]->sourceRect, 0, *mColorSolid, true);
    } else {
        mInfra->mDriver->draw2DImage((*startSignal)[2]->texture, (*startSignal)[2]->drawScrPosition,
              (*startSignal)[2]->sourceRect, 0, *mColorSolid, true);
    }
}

void HUD::DrawAmmoBar() {
    //the first bar is always visible, even when ammo
    //is empty
    int nrAddBars = ammoBar->size() - 1;

    //how many additional bars needs to be drawn?
    irr::f32 addBars = (monitorWhichPlayer->mPlayerStats->ammoVal / monitorWhichPlayer->mPlayerStats->ammoMax) * nrAddBars;

    int addBarsInt = (int)(irr::core::round32(addBars));

    if (addBarsInt < 0)
        addBarsInt = 0;

    if (addBarsInt > nrAddBars)
        addBarsInt = nrAddBars;

    for (int i = 0; i < (addBarsInt + 1); i++) {
        if (!mDrawAmmoBarTransparent) {
            mInfra->mDriver->draw2DImage((*ammoBar)[i]->texture, (*ammoBar)[i]->drawScrPosition,
                  (*ammoBar)[i]->sourceRect, 0, *mColorSolid, true);
        } else {
            mInfra->mDriver->draw2DImage((*ammoBar)[i]->texture, (*ammoBar)[i]->drawScrPosition,
                  (*ammoBar)[i]->sourceRect, 0, *mColorTransparent, true);
        }
    }
}

void HUD::DrawShieldBar() {
    int sizeVec = shieldBar->size();

    irr::f32 perc = (monitorWhichPlayer->mPlayerStats->shieldVal / monitorWhichPlayer->mPlayerStats->shieldMax) * sizeVec;

    int barsInt = (int)(irr::core::round32(perc));

    if (barsInt < 0)
        barsInt = 0;

    if (barsInt > sizeVec)
        barsInt = sizeVec;

    //draw as many shield bars as the current shield state of the ship allows
    for (int i = 0; i < barsInt ; i++) {
        if (!mDrawShieldBarTransparent) {
            mInfra->mDriver->draw2DImage((*shieldBar)[i]->texture, (*shieldBar)[i]->drawScrPosition,
                  (*shieldBar)[i]->sourceRect, 0, *mColorSolid, true);
        } else {
            mInfra->mDriver->draw2DImage((*shieldBar)[i]->texture, (*shieldBar)[i]->drawScrPosition,
                  (*shieldBar)[i]->sourceRect, 0, *mColorTransparent, true);
        }
    }
}

//the number of gasoline bars drawn in HUD does not go up
//linear with the amount of gasoline available
//first the bars are disappearing slower, and when the gasoline
//gets less the last bars disappear quicker
int HUD::GetNumberCurrentGasolineBars(irr::f32 gasolineVal) {
    if ((gasolineVal > 0.0f) && (gasolineVal <= 10.0f)) {
        if (gasolineVal < 2.0f)
            return 0;

        if (gasolineVal < 4.0f)
            return 1;

        if (gasolineVal < 6.0f)
            return 2;

        if (gasolineVal < 8.0f)
            return 3;

        if (gasolineVal < 10.0f)
            return 4;
    } else if ((gasolineVal > 10.0f) && (gasolineVal <= 20.0f)) {
        if (gasolineVal < 12.0f)
            return 5;

        if (gasolineVal < 14.0f)
            return 6;

        if (gasolineVal < 16.0f)
            return 7;

        if (gasolineVal < 18.0f)
            return 8;

        if (gasolineVal < 20.0f)
            return 9;
    } else if ((gasolineVal > 20.0f) && (gasolineVal <= 28.0f)) {
        if (gasolineVal < 22.0f)
            return 10;

        if (gasolineVal < 24.0f)
            return 11;

        if (gasolineVal < 26.0f)
            return 12;

        if (gasolineVal < 28.0f)
            return 13;
    } else if ((gasolineVal > 28.0f) && (gasolineVal <= 31.0f)) {
        return 14;
    } else if ((gasolineVal > 31.0f) && (gasolineVal <= 37.0f)) {
        return 15;
    }  else if ((gasolineVal > 37.0f) && (gasolineVal <= 43.0f)) {
        return 16;
    } else if (gasolineVal > 43.0f) {
        return 17;
    }

    return 0;
}

void HUD::DrawGasolineBar() {
    int sizeVec = gasolineBar->size();

    int nrBarsToDraw = GetNumberCurrentGasolineBars(monitorWhichPlayer->mPlayerStats->gasolineVal);

    if (nrBarsToDraw < 0)
        nrBarsToDraw = 0;

    if (nrBarsToDraw > sizeVec)
        nrBarsToDraw = sizeVec;

    for (int i = (sizeVec - nrBarsToDraw); i < sizeVec; i++) {
        if (!mDrawGasolineBarTransparent) {
            mInfra->mDriver->draw2DImage((*gasolineBar)[i]->texture, (*gasolineBar)[i]->drawScrPosition,
                  (*gasolineBar)[i]->sourceRect, 0, *mColorSolid, true);
        } else {
            mInfra->mDriver->draw2DImage((*gasolineBar)[i]->texture, (*gasolineBar)[i]->drawScrPosition,
                  (*gasolineBar)[i]->sourceRect, 0, *mColorTransparent, true);
        }
    }
}

void HUD::DrawHUD1PlayerRace(irr::f32 deltaTime) {
    if (monitorWhichPlayer != NULL) {

        //in case a hud bar is empty of a low warning
        //is present the bar does blink in the original
        //game
        blinkBarTimer += deltaTime;
        if (blinkBarTimer >= DEF_HUD_BARS_BLINKPERIODE) {
            blinkBarTimer = 0.0f;

            mDrawBarsTransparent = !mDrawBarsTransparent;

            if (monitorWhichPlayer->ShouldAmmoBarBlink()) {
                mDrawAmmoBarTransparent = mDrawBarsTransparent;
            } else {
                mDrawAmmoBarTransparent = false;
            }

            if (monitorWhichPlayer->ShouldGasolineBarBlink()) {
                mDrawGasolineBarTransparent = mDrawBarsTransparent;
            } else {
                mDrawGasolineBarTransparent = false;
            }

            if (monitorWhichPlayer->ShouldShieldBarBlink()) {
                mDrawShieldBarTransparent = mDrawBarsTransparent;
            } else {
                mDrawShieldBarTransparent = false;
            }
        }

        DrawFinishedPlayerList();

        //any broken glas?
        if (this->monitorWhichPlayer->brokenGlasVec->size() > 0) {
            std::vector<HudDisplayPart*>::iterator itGlasBreak;

            for (itGlasBreak = this->monitorWhichPlayer->brokenGlasVec->begin(); itGlasBreak != this->monitorWhichPlayer->brokenGlasVec->end(); ++itGlasBreak) {
                mInfra->mDriver->draw2DImage((*itGlasBreak)->texture, (*itGlasBreak)->drawScrPosition,
                      (*itGlasBreak)->sourceRect, 0, *mColorSolid, true);
            }
        }

        DrawShieldBar();
        DrawAmmoBar();
        DrawGasolineBar();

        int sizeVec = throttleBar->size();

        irr::f32 perc2 = (monitorWhichPlayer->mPlayerStats->throttleVal / monitorWhichPlayer->mPlayerStats->throttleMax) * sizeVec;
        irr::f32 perc = (monitorWhichPlayer->mPlayerStats->boosterVal / monitorWhichPlayer->mPlayerStats->boosterMax) * sizeVec;

        if (perc < 0)
            perc = 0;

        if (perc > sizeVec)
            perc = sizeVec;

        if (perc2 < 0)
            perc2 = 0;

        if (perc2 > sizeVec)
            perc2 = sizeVec;

        //according to current player throttle setting draw unlighted default colors
        for (int i = 0; i < perc2; i++) {
            mInfra->mDriver->draw2DImage((*throttleBar)[i]->texture, (*throttleBar)[i]->drawScrPosition,
                  (*throttleBar)[i]->sourceRect, 0, *mColorSolid, true);
        }

        //according to current booster state draw over it with lighted colors
        for (int i = 0; i < perc; i++) {
            mInfra->mDriver->draw2DImage((*throttleBar)[i]->altTexture, (*throttleBar)[i]->drawScrPosition,
                  (*throttleBar)[i]->sourceRect, 0, *mColorSolid, true);
        }

        sizeVec = speedBar->size();

        perc = (monitorWhichPlayer->mPlayerStats->speed / monitorWhichPlayer->mPlayerStats->speedMax) * sizeVec;

        if (perc < 0)
            perc = 0;

        if (perc > sizeVec)
            perc = sizeVec;

        for (int i = 0; i < perc; i++) {
            mInfra->mDriver->draw2DImage((*speedBar)[i]->texture, (*speedBar)[i]->drawScrPosition,
                  (*speedBar)[i]->sourceRect, 0, *mColorSolid, true);
        }

        //Draw machine gun heat bar
        sizeVec = mgHeatBar->size();

        perc = (monitorWhichPlayer->mPlayerStats->mgHeatVal / monitorWhichPlayer->mPlayerStats->mgHeatMax) * sizeVec;

        if (perc < 0)
            perc = 0;

        if (perc > sizeVec)
            perc = sizeVec;

        for (int i = 0; i < perc; i++) {
            mInfra->mDriver->draw2DImage((*mgHeatBar)[i]->texture, (*mgHeatBar)[i]->drawScrPosition,
                  (*mgHeatBar)[i]->sourceRect, 0, *mColorSolid, true);
        }

        //Draw upgrade bar
        //symbol number 0 is the minigun symbol itself (for basic upgrade level 0)
        //the next three symbols 1, 2 and 3 are for upgrade levels 1, 2 and 3
        for (int i = 0; i <= monitorWhichPlayer->mPlayerStats->currMinigunUpgradeLevel; i++) {
            mInfra->mDriver->draw2DImage((*upgradeBar)[i]->texture, (*upgradeBar)[i]->drawScrPosition,
                  (*upgradeBar)[i]->sourceRect, 0, *mColorSolid, true);
        }

        //symbol number 4 is the rocket symbol itself (for basic upgrade level 0)
        //the next three symbols 5, 6 and 7 are for upgrade levels 1, 2 and 3
        for (int i = 4; i <= (monitorWhichPlayer->mPlayerStats->currRocketUpgradeLevel + 4); i++) {
            mInfra->mDriver->draw2DImage((*upgradeBar)[i]->texture, (*upgradeBar)[i]->drawScrPosition,
                  (*upgradeBar)[i]->sourceRect, 0, *mColorSolid, true);
        }

        //symbol number 8 is the booster symbol itself (for basic upgrade level 0)
        //the next three symbols 9, 10 and 11 are for upgrade levels 1, 2 and 3
        for (int i = 8; i <= (monitorWhichPlayer->mPlayerStats->currBoosterUpgradeLevel + 8); i++) {
            mInfra->mDriver->draw2DImage((*upgradeBar)[i]->texture, (*upgradeBar)[i]->drawScrPosition,
                  (*upgradeBar)[i]->sourceRect, 0, *mColorSolid, true);
        }


        //draw current player race position
        int hlp = monitorWhichPlayer->mPlayerStats->currRacePlayerPos;

        if (hlp < 0)
            hlp = 0;

        if (hlp > 9)
            hlp = 9;

        mInfra->mDriver->draw2DImage((*currRacePlayerPosition)[hlp]->texture, (*currRacePlayerPosition)[hlp]->drawScrPosition,
              (*currRacePlayerPosition)[hlp]->sourceRect, 0, *mColorSolid, true);

        //draw the slash for player position (slash is #10 in the list)
        mInfra->mDriver->draw2DImage((*currRacePlayerPosition)[10]->texture, (*currRacePlayerPosition)[10]->drawScrPosition,
              (*currRacePlayerPosition)[10]->sourceRect, 0, *mColorSolid, true);

        //draw overall number of players in race
        hlp = monitorWhichPlayer->mPlayerStats->overallPlayerNumber;

        if (hlp < 0)
            hlp = 0;

        if (hlp > 9)
            hlp = 9;

        //draw the overall number of players in the race
        mInfra->mDriver->draw2DImage((*numberPlayers)[hlp]->texture, (*numberPlayers)[hlp]->drawScrPosition,
              (*numberPlayers)[hlp]->sourceRect, 0, *mColorSolid, true);

        //render current lap number information
        char lapNumStr[10];
        //first draw 2 red arrow symbol next to lap number display
        strcpy(&lapNumStr[0], ">");
        //RenderRedTextLapNumber(&lapNumStr[0], irr::core::vector2d<irr::s32>(128, 91));
        mInfra->mGameTexts->DrawGameNumberText(lapNumStr,
                                               mInfra->mGameTexts->HudLaptimeNumberRed, irr::core::vector2d<irr::s32>(128, 91));

        //first built string
        //important! pad with 2 leading zeros!
        sprintf(&lapNumStr[0], "%02d/%02d", monitorWhichPlayer->mPlayerStats->currLapNumber, monitorWhichPlayer->mPlayerStats->raceNumberLaps);
        //RenderRedTextLapNumber(&lapNumStr[0], irr::core::vector2d<irr::s32>(178, 93));
        mInfra->mGameTexts->DrawGameNumberText(lapNumStr,
                                               mInfra->mGameTexts->HudLaptimeNumberRed, irr::core::vector2d<irr::s32>(178, 93));

        //next draw red skull and current player kill count number
        char currKillCountStr[10];

        //first draw red skull
        //strcpy(&currKillCountStr[0], "s");
        //RenderRedTextKillCount(&currKillCountStr[0], irr::core::vector2d<irr::s32>(559, 62));
        strcpy(&currKillCountStr[0], ">");
        mInfra->mGameTexts->DrawGameNumberText(&currKillCountStr[0],
                mInfra->mGameTexts->HudKillCounterNumberRed, irr::core::vector2d<irr::s32>(559, 62));

        //now draw current kill number, dont forget the leading zeros!
        sprintf(&currKillCountStr[0], "%02d", monitorWhichPlayer->mPlayerStats->currKillCount);
        //RenderRedTextKillCount(&currKillCountStr[0], irr::core::vector2d<irr::s32>(592, 62));
        mInfra->mGameTexts->DrawGameNumberText(&currKillCountStr[0],
                mInfra->mGameTexts->HudKillCounterNumberRed, irr::core::vector2d<irr::s32>(592, 62));

        BannerTextLogic(deltaTime);

        //Draw list with current player lap times
        //is located in the left lower part of the screen
        RenderPlayerLapTimes();

/*
        int time = 218;
        char laptime[10];
        sprintf(&laptime[0], "%d>", time);
        RenderRedText(&laptime[0], irr::core::vector2d<irr::s32>(100, 400));*/

        RenderTargetSymbol(deltaTime);

        //Render big green text if currently one is shown
        RenderBigGreenText(deltaTime);
    }
}

void HUD::RenderBigGreenText(irr::f32 deltaTime) {
    //Render big green text if currently one is shown
    if (this->currentBigGreenTextState > 0) {
        //Draw the text
        //we use HudBigGreenText
        if ((this->currentBigGreenTextState == 2) || (!blinkBigGreenText)) {
            mInfra->mGameTexts->DrawGameText(currentBigGreenText,
                                             mInfra->mGameTexts->HudBigGreenText, currentBigGreenTextDrawPosition);
        }

        //check how long we still need to show this text
        if (!permanentBigGreenText) {
            currentBigGreenTextStillShownSec -= deltaTime;

            if (currentBigGreenTextStillShownSec < 0.0f) {
                //stop showing the text again
                currentBigGreenTextState = 0;
                currentBigGreenTextStillShownSec = 0.0f;

                //we don't need our allocated text array anymore
                //get rid of it, because otherwise we have a memory leak :(
                delete[] currentBigGreenText;
                currentBigGreenText = NULL;

                return;
            }
        }

        if (!blinkBigGreenText)
            return;

        currentBigGreenTextBlinkPeriod -= deltaTime;

        if ((currentBigGreenTextBlinkPeriod < 0.0f) && (currentBigGreenTextState != 0)) {
            irr::f32 blinkPeriod = DEF_HUD_BIGGREENTEXT_BLINKPERIODTIME;
            currentBigGreenTextBlinkPeriod = blinkPeriod;

            //toggle between state 1 and 2
            //state 0 = no big green text is shown currently
            //state 1 = big green text is still shown, but text render disabled right now because of blinking
            //state 2 = big green text is currently shown an rendered
            if (currentBigGreenTextState == 1) {
                currentBigGreenTextState = 2;
            } else if (currentBigGreenTextState == 2) {
                currentBigGreenTextState = 1;
            }
        }
    }
}

//RemovePermanentGreenBigText does only remove a permanent green text,
//not one with a specified duration
void HUD::RemovePermanentGreenBigText() {
    //only act if there is a permanent big green text shown
    //right now
    if (DoesHudShowPermanentGreenBigText()) {
        //stop showing the text again
        currentBigGreenTextState = 0;
        currentBigGreenTextStillShownSec = 0.0f;

        //we don't need our allocated text array anymore
        //get rid of it, because otherwise we have a memory leak :(
        delete[] currentBigGreenText;
        currentBigGreenText = NULL;
    }
}

bool HUD::DoesHudShowPermanentGreenBigText() {
   if ((currentBigGreenTextState > 0) && (permanentBigGreenText))
       return true;

   return false;
}

void HUD::DrawHUD1(irr::f32 deltaTime) {
    switch (this->mHudState) {
        case DEF_HUD_STATE_STARTSIGNAL: {
            DrawHUD1PlayerStartSignal(deltaTime);
            break;
        }

        case DEF_HUD_STATE_RACE: {
            DrawHUD1PlayerRace(deltaTime);
            break;
        }

        case DEF_HUD_STATE_BROKENPLAYER: {
            DrawHUD1PlayerBrokenPlayer(deltaTime);
            break;
        }

        default: {
            break;
        }
    }
}

void HUD::DrawHUD1PlayerBrokenPlayer(irr::f32 deltaTime) {
    if (monitorWhichPlayer != NULL) {
        DrawFinishedPlayerList();
        RenderBigGreenText(deltaTime);
    }
}

void HUD::SetMonitorWhichPlayer(Player* newPlayer) {
    //if we change to another player, make sure warning sound of
    //old player is stopped
    if (monitorWhichPlayer != NULL) {
        monitorWhichPlayer->StopPlayingWarningSound();
        monitorWhichPlayer->SetMyHUD(NULL);
    }

    //also make sure that any visible permanent
    //big green text is removed first
    if (DoesHudShowPermanentGreenBigText()) {
        RemovePermanentGreenBigText();
    }

    CleanUpAllBannerMessages();

    monitorWhichPlayer = newPlayer;
    newPlayer->SetMyHUD(this);
}

void HUD::InitHudBannerText() {
    //init dynamic variables
    bannerTextState1 = new std::vector<HudDisplayPart*>;
    bannerTextState2 = new std::vector<HudDisplayPart*>;
    bannerTextState3 = new std::vector<HudDisplayPart*>;
    bannerTextState4 = new std::vector<HudDisplayPart*>;
    bannerTextState5 = new std::vector<HudDisplayPart*>;
    bannerTextState6 = new std::vector<HudDisplayPart*>;
    bannerTextState7 = new std::vector<HudDisplayPart*>;

    //initialize graphics needed for bottom screen banner text view
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    irr::s32 posY = (mInfra->mScreenRes.Height - 39);

    /*************
     * State 1   * first/last symbols are seen from message banner
     * **********/

    //Left boundary block symbol
    HudDisplayPart* leftBoundarySymbState1 = new HudDisplayPart();
    leftBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
    leftBoundarySymbState1->altTexture = NULL;
    //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
    leftBoundarySymbState1->drawScrPosition.set(243, posY);

    //Right boundary block symbol
    HudDisplayPart* rightBoundarySymbState1 = new HudDisplayPart();
    rightBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
    rightBoundarySymbState1->altTexture = NULL;
   // myDriver->makeColorKeyTexture(rightBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    rightBoundarySymbState1->sizeTex = rightBoundarySymbState1->texture->getSize();
    rightBoundarySymbState1->drawScrPosition.set(348, posY);

    HudDisplayPart* middleWhiteSymbState1Symb;
    irr::s32 posX = 243 + leftBoundarySymbState1->sizeTex.Width;
    irr::s32 remainX = 348 - posX;

    //symbols needed to be drawn for banner state 1
    bannerTextState1->push_back(leftBoundarySymbState1);

    //create all middle while background symbols (before text appears)
    for (int idx = 0; idx < 3; idx++) {
        middleWhiteSymbState1Symb = new HudDisplayPart();
        middleWhiteSymbState1Symb->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
        middleWhiteSymbState1Symb->altTexture = NULL;
      //  myDriver->makeColorKeyTexture(middleWhiteSymbState1Symb->texture, irr::core::position2d<irr::s32>(0,0));
        middleWhiteSymbState1Symb->sizeTex = middleWhiteSymbState1Symb->texture->getSize();
        middleWhiteSymbState1Symb->drawScrPosition.set(posX, posY);
        posX += middleWhiteSymbState1Symb->sizeTex.Width;
        remainX -= middleWhiteSymbState1Symb->sizeTex.Width;

        if (remainX < 0) {
            //move last symbol as many pixels to the left so that its end
            //does exactly stop before the right symbol
            posX += remainX;
        }

        bannerTextState1->push_back(middleWhiteSymbState1Symb);
    }

    bannerTextState1->push_back(rightBoundarySymbState1);

    /*************
     * State 2   * further open/close banner
     * **********/

    //Left boundary block symbol
    leftBoundarySymbState1 = new HudDisplayPart();
    leftBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
    leftBoundarySymbState1->altTexture = NULL;
    //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
    leftBoundarySymbState1->drawScrPosition.set(210, posY);

    //Right boundary block symbol
    rightBoundarySymbState1 = new HudDisplayPart();
    rightBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
    rightBoundarySymbState1->altTexture = NULL;
    //myDriver->makeColorKeyTexture(rightBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    rightBoundarySymbState1->sizeTex = rightBoundarySymbState1->texture->getSize();
    rightBoundarySymbState1->drawScrPosition.set(380, posY);

    posX = 210 + leftBoundarySymbState1->sizeTex.Width;
    remainX = 380 - posX;

    //symbols needed to be drawn for banner state 2
    bannerTextState2->push_back(leftBoundarySymbState1);

    //create all middle while background symbols (before text appears)
    for (int idx = 0; idx < 5; idx++) {
        middleWhiteSymbState1Symb = new HudDisplayPart();
        middleWhiteSymbState1Symb->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
        middleWhiteSymbState1Symb->altTexture = NULL;
      //  myDriver->makeColorKeyTexture(middleWhiteSymbState1Symb->texture, irr::core::position2d<irr::s32>(0,0));
        middleWhiteSymbState1Symb->sizeTex = middleWhiteSymbState1Symb->texture->getSize();
        middleWhiteSymbState1Symb->drawScrPosition.set(posX, posY);
        posX += middleWhiteSymbState1Symb->sizeTex.Width;
        remainX -= middleWhiteSymbState1Symb->sizeTex.Width;

        if (remainX < 0) {
            //move last symbol as many pixels to the left so that its end
            //does exactly stop before the right symbol
            posX += remainX;
        }

        bannerTextState2->push_back(middleWhiteSymbState1Symb);
    }

   bannerTextState2->push_back(rightBoundarySymbState1);

   /*************
    * State 3   *  further open/close banner
    * **********/

   //Left boundary block symbol
   leftBoundarySymbState1 = new HudDisplayPart();
   leftBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
   leftBoundarySymbState1->altTexture = NULL;
   //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
   leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
   leftBoundarySymbState1->drawScrPosition.set(180, posY);

   //Right boundary block symbol
   rightBoundarySymbState1 = new HudDisplayPart();
   rightBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
   rightBoundarySymbState1->altTexture = NULL;
   //myDriver->makeColorKeyTexture(rightBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
   rightBoundarySymbState1->sizeTex = rightBoundarySymbState1->texture->getSize();
   rightBoundarySymbState1->drawScrPosition.set(410, posY);

   posX = 180 + leftBoundarySymbState1->sizeTex.Width;
   remainX = 410 - posX;

   //symbols needed to be drawn for banner state 3
   bannerTextState3->push_back(leftBoundarySymbState1);

   //create all middle while background symbols (before text appears)
   for (int idx = 0; idx < 8; idx++) {
       middleWhiteSymbState1Symb = new HudDisplayPart();
       middleWhiteSymbState1Symb->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
       middleWhiteSymbState1Symb->altTexture = NULL;
     //  myDriver->makeColorKeyTexture(middleWhiteSymbState1Symb->texture, irr::core::position2d<irr::s32>(0,0));
       middleWhiteSymbState1Symb->sizeTex = middleWhiteSymbState1Symb->texture->getSize();
       middleWhiteSymbState1Symb->drawScrPosition.set(posX, posY);
       posX += middleWhiteSymbState1Symb->sizeTex.Width;
       remainX -= middleWhiteSymbState1Symb->sizeTex.Width;

       if (remainX < 0) {
           //move last symbol as many pixels to the left so that its end
           //does exactly stop before the right symbol
           posX += remainX;
       }

       bannerTextState3->push_back(middleWhiteSymbState1Symb);
   }

  bannerTextState3->push_back(rightBoundarySymbState1);

  /*************
   * State 4   * further open/close banner
   * **********/

  //Left boundary block symbol
  leftBoundarySymbState1 = new HudDisplayPart();
  leftBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
  leftBoundarySymbState1->altTexture = NULL;
  //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
  leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
  leftBoundarySymbState1->drawScrPosition.set(151, posY);

  //Right boundary block symbol
  rightBoundarySymbState1 = new HudDisplayPart();
  rightBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
  rightBoundarySymbState1->altTexture = NULL;
  //myDriver->makeColorKeyTexture(rightBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
  rightBoundarySymbState1->sizeTex = rightBoundarySymbState1->texture->getSize();
  rightBoundarySymbState1->drawScrPosition.set(441, posY);

  posX = 151 + leftBoundarySymbState1->sizeTex.Width;
  remainX = 441 - posX;

  //symbols needed to be drawn for banner state 4
  bannerTextState4->push_back(leftBoundarySymbState1);

  //create all middle while background symbols (before text appears)
  for (int idx = 0; idx < 10; idx++) {
      middleWhiteSymbState1Symb = new HudDisplayPart();
      middleWhiteSymbState1Symb->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
      middleWhiteSymbState1Symb->altTexture = NULL;
    //  myDriver->makeColorKeyTexture(middleWhiteSymbState1Symb->texture, irr::core::position2d<irr::s32>(0,0));
      middleWhiteSymbState1Symb->sizeTex = middleWhiteSymbState1Symb->texture->getSize();
      middleWhiteSymbState1Symb->drawScrPosition.set(posX, posY);
      posX += middleWhiteSymbState1Symb->sizeTex.Width;
      remainX -= middleWhiteSymbState1Symb->sizeTex.Width;

      if (remainX < 0) {
          //move last symbol as many pixels to the left so that its end
          //does exactly stop before the right symbol
          posX += remainX;
      }

      bannerTextState4->push_back(middleWhiteSymbState1Symb);
  }

 bannerTextState4->push_back(rightBoundarySymbState1);

 /*************
  * State 5   * further open/close banner
  * **********/

 //Left boundary block symbol
 leftBoundarySymbState1 = new HudDisplayPart();
 leftBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
 leftBoundarySymbState1->altTexture = NULL;
 //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
 leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
 leftBoundarySymbState1->drawScrPosition.set(120, posY);

 //Right boundary block symbol
 rightBoundarySymbState1 = new HudDisplayPart();
 rightBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
 rightBoundarySymbState1->altTexture = NULL;
 //myDriver->makeColorKeyTexture(rightBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
 rightBoundarySymbState1->sizeTex = rightBoundarySymbState1->texture->getSize();
 rightBoundarySymbState1->drawScrPosition.set(470, posY);

 posX = 120 + leftBoundarySymbState1->sizeTex.Width;
 remainX = 470 - posX;

 //symbols needed to be drawn for banner state 5
 bannerTextState5->push_back(leftBoundarySymbState1);

 //create all middle while background symbols (before text appears)
 for (int idx = 0; idx < 13; idx++) {
     middleWhiteSymbState1Symb = new HudDisplayPart();
     middleWhiteSymbState1Symb->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
     middleWhiteSymbState1Symb->altTexture = NULL;
   //  myDriver->makeColorKeyTexture(middleWhiteSymbState1Symb->texture, irr::core::position2d<irr::s32>(0,0));
     middleWhiteSymbState1Symb->sizeTex = middleWhiteSymbState1Symb->texture->getSize();
     middleWhiteSymbState1Symb->drawScrPosition.set(posX, posY);
     posX += middleWhiteSymbState1Symb->sizeTex.Width;
     remainX -= middleWhiteSymbState1Symb->sizeTex.Width;

     if (remainX < 0) {
         //move last symbol as many pixels to the left so that its end
         //does exactly stop before the right symbol
         posX += remainX;
     }

     bannerTextState5->push_back(middleWhiteSymbState1Symb);
    }

    bannerTextState5->push_back(rightBoundarySymbState1);

    /*************
     * State 6   * further open/close banner
     * **********/

    //Left boundary block symbol
    leftBoundarySymbState1 = new HudDisplayPart();
    leftBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
    leftBoundarySymbState1->altTexture = NULL;
    //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
    leftBoundarySymbState1->drawScrPosition.set(91, posY);

    //Right boundary block symbol
    rightBoundarySymbState1 = new HudDisplayPart();
    rightBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
    rightBoundarySymbState1->altTexture = NULL;
    //myDriver->makeColorKeyTexture(rightBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    rightBoundarySymbState1->sizeTex = rightBoundarySymbState1->texture->getSize();
    rightBoundarySymbState1->drawScrPosition.set(501, posY);

    posX = 91 + leftBoundarySymbState1->sizeTex.Width;
    remainX = 501 - posX;

    //symbols needed to be drawn for banner state 6
    bannerTextState6->push_back(leftBoundarySymbState1);

    //create all middle while background symbols (before text appears)
    for (int idx = 0; idx < 15; idx++) {
        middleWhiteSymbState1Symb = new HudDisplayPart();
        middleWhiteSymbState1Symb->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
        middleWhiteSymbState1Symb->altTexture = NULL;
      //  myDriver->makeColorKeyTexture(middleWhiteSymbState1Symb->texture, irr::core::position2d<irr::s32>(0,0));
        middleWhiteSymbState1Symb->sizeTex = middleWhiteSymbState1Symb->texture->getSize();
        middleWhiteSymbState1Symb->drawScrPosition.set(posX, posY);
        posX += middleWhiteSymbState1Symb->sizeTex.Width;
        remainX -= middleWhiteSymbState1Symb->sizeTex.Width;

        if (remainX < 0) {
            //move last symbol as many pixels to the left so that its end
            //does exactly stop before the right symbol
            posX += remainX;
        }

        bannerTextState6->push_back(middleWhiteSymbState1Symb);
       }

       bannerTextState6->push_back(rightBoundarySymbState1);

       /*************
        * State 7   * while message text is visible, banner is fully opened; no white symbols visible
        * **********/

       //Left boundary block symbol
       leftBoundarySymbState1 = new HudDisplayPart();
       leftBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
       leftBoundarySymbState1->altTexture = NULL;
       //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
       leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
       leftBoundarySymbState1->drawScrPosition.set(91, posY);

       //Right boundary block symbol
       rightBoundarySymbState1 = new HudDisplayPart();
       rightBoundarySymbState1->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
       rightBoundarySymbState1->altTexture = NULL;
       //myDriver->makeColorKeyTexture(rightBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
       rightBoundarySymbState1->sizeTex = rightBoundarySymbState1->texture->getSize();
       rightBoundarySymbState1->drawScrPosition.set(501, posY);

       //symbols needed to be drawn for banner state 7
       bannerTextState7->push_back(leftBoundarySymbState1);
       bannerTextState7->push_back(rightBoundarySymbState1);

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    //current banner text state is 0 (means nothing is shown at all)
    currentBannerTextState = 0;

    bannerDoesCurrentlyOpen = false;
    bannerDoesCurrentlyClose = false;
}

void HUD::RenderTextBannerGraphics() {
    std::vector<HudDisplayPart*> *elementsToDraw;

    switch (currentBannerTextState) {
        case 1: {
            elementsToDraw = bannerTextState1;
            break;
        }

        case 2: {
            elementsToDraw = bannerTextState2;
            break;
        }

        case 3: {
            elementsToDraw = bannerTextState3;
            break;
        }

        case 4: {
            elementsToDraw = bannerTextState4;
            break;
        }

        case 5: {
            elementsToDraw = bannerTextState5;
            break;
        }

        case 6: {
            elementsToDraw = bannerTextState6;
            break;
        }

        case 7: {
            elementsToDraw = bannerTextState7;
            break;
        }

        default: {
            elementsToDraw = NULL;
            break;
        }
    }

        if (elementsToDraw != NULL) {
            //draw currently visible banner state
            for (ulong i = 0; i < elementsToDraw->size(); i++) {
                mInfra->mDriver->draw2DImage(elementsToDraw->at(i)->texture, elementsToDraw->at(i)->drawScrPosition,
                      irr::core::rect<irr::s32>(0,0, elementsToDraw->at(i)->sizeTex.Width, elementsToDraw->at(i)->sizeTex.Height), 0,
                      *mColorSolid, true);

        }

    }
}

void HUD::RenderPlayerLapTimes() {
    //Notes about LapTime rendering in Hi-Octane
    //all laptimes are shown in the lower left corner of the HUD
    //Hi-Octane shows a maximum of 4 sets of lap time
    //the lowest line always shows the laptime and lap number of the current lap in red text
    //the highest line always shows the laptime and lap number where the player was fastest (minimum) in grey test
    //the two entries inbetween show the laptime and lap number of the two last laps (regardless of the actuall lap times); in red color
    //if there are less then 4 entries in the table Hi-Octane starts rendering from the lowest line (most Y coordinate) upwards

    //what info to print when? some brainstorming...
    //amountFinishedLaps = 0, just print current lap info, can be print always
    //amountFinishedLaps = 1, if one lap is finished this lap is automatically fastest lap -> print current lap info and fastest lap info
    //amountFinishedLaps = 2, if two laps are finished, and the last lap is not the fastest lap -> print current lap info, last lap info, and fastest lap info
    //                        if two laps are finished, and the last lap is  the fastest lap -> print current lap info and fastest lap info
    //amountFinishedLaps = 3, if three laps are finished, and the last lap is not the fastest lap -> print current lap info, last lap info, and fastest lap info
    //                        if two laps are finished, and the last lap is  the fastest lap -> print current lap info and fastest lap info

    irr::u32 posY = (mInfra->mScreenRes.Height - 24);
    irr::u32 posX;
    irr::u32 txtWidth;

    //1. render current lap number in the lowest line
    char text[20];
    sprintf(&text[0], "%2d.", this->monitorWhichPlayer->mPlayerStats->currLapNumber);

    txtWidth = mInfra->mGameTexts->GetWidthPixelsGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed);
    posX = 38 - txtWidth;

    mInfra->mGameTexts->DrawGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed, irr::core::position2di(posX, posY));

    //2. render current lap time number in the lowest line
    // this text is written in a way that times are alignment on the right side
    //the length (width) of text to render afterwards
    sprintf(&text[0], "%4d", this->monitorWhichPlayer->mPlayerStats->currLapTimeMultiple40mSec);

    //txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
    //posX = 92 - txtWidth;

    mInfra->mGameTexts->DrawGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed, irr::core::position2di(56, posY));

    //for the next line decrease Y coordinate
    posY -= 23;

    irr::u8 amountFinishedLaps = (this->monitorWhichPlayer->mPlayerStats->currLapNumber - 1);

    //first lets see if there is already a fastest lap number & lap time available
    //get this info
    irr::u8 fastestLapNr = 0;
    irr::u32 fastestLapNrLapTimeMultiple40ms;

    if (amountFinishedLaps > 0) {
        //because player lap time table is a sorted vector, fastest lap is always first entry
        fastestLapNr = this->monitorWhichPlayer->mPlayerStats->lapTimeList[0].lapNr;
        fastestLapNrLapTimeMultiple40ms = this->monitorWhichPlayer->mPlayerStats->lapTimeList[0].lapTimeMultiple40mSec;
    }

    //if available print information about last lap, also important: only print this lap here as long this is not the fastest lap!
    if ((this->monitorWhichPlayer->mPlayerStats->lastLap.lapNr != 0) && (this->monitorWhichPlayer->mPlayerStats->lastLap.lapNr != fastestLapNr)) {
        sprintf(&text[0], "%2d.", this->monitorWhichPlayer->mPlayerStats->lastLap.lapNr);

        txtWidth = mInfra->mGameTexts->GetWidthPixelsGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed);
        posX = 38 - txtWidth;

        mInfra->mGameTexts->DrawGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed, irr::core::position2di(posX, posY));

        sprintf(&text[0], "%4d", this->monitorWhichPlayer->mPlayerStats->lastLap.lapTimeMultiple40mSec);

        //txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
        //posX = 92 - txtWidth;

        mInfra->mGameTexts->DrawGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed, irr::core::position2di(56, posY));

        //for the next line decrease Y coordinate
        posY -= 23;
    }

    //if available print information about lap before last lap, also important: only print this lap here as long this is not the fastest lap!
    if ((this->monitorWhichPlayer->mPlayerStats->LapBeforeLastLap.lapNr != 0) && (this->monitorWhichPlayer->mPlayerStats->LapBeforeLastLap.lapNr != fastestLapNr)) {
        sprintf(&text[0], "%2d.", this->monitorWhichPlayer->mPlayerStats->LapBeforeLastLap.lapNr);

        txtWidth = mInfra->mGameTexts->GetWidthPixelsGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed);
        posX = 38 - txtWidth;

        mInfra->mGameTexts->DrawGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed, irr::core::position2di(posX, posY));

        sprintf(&text[0], "%4d", this->monitorWhichPlayer->mPlayerStats->LapBeforeLastLap.lapTimeMultiple40mSec);

        //txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
        //posX = 92 - txtWidth;

        mInfra->mGameTexts->DrawGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberRed, irr::core::position2di(56, posY));

        //for the next line decrease Y coordinate
        posY -= 23;
    }

    //now finally print currently fastest lap info with grey text
    if (fastestLapNr != 0) {
        sprintf(&text[0], "%2d.", fastestLapNr);

        txtWidth = mInfra->mGameTexts->GetWidthPixelsGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberGrey);
        posX = 38 - txtWidth;

        mInfra->mGameTexts->DrawGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberGrey, irr::core::position2di(posX, posY));

        sprintf(&text[0], "%4d", fastestLapNrLapTimeMultiple40ms);

        //txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberGrey);
        //posX = 92 - txtWidth;

        mInfra->mGameTexts->DrawGameNumberText(&text[0], mInfra->mGameTexts->HudLaptimeNumberGrey, irr::core::position2di(56, posY));
    }
}

//if showDurationSec is negative a permanent banner text message is created instead of the
//default time limited one
void HUD::ShowBannerText(char* text, irr::f32 showDurationSec, bool warningSoundActive) {
    //create new banner text message struct for the new message
    BannerTextMessageStruct* newMsg = new BannerTextMessageStruct();

    int textLen = strlen(text);
    newMsg->text = new char[textLen + 1];
    strcpy(newMsg->text, text);

    newMsg->isWarning = warningSoundActive;

    if (showDurationSec < 0.0f) {
        newMsg->permanentMsg = true;
        newMsg->textStillShownSec = 0.0f;
    } else  {
        newMsg->permanentMsg = false;
        newMsg->textStillShownSec = showDurationSec;
    }

    newMsg->textAlreadyShownSec = 0.0f;

    //for Hud banner text we use HudWhiteTextBannerFont
    irr::u32 currentTextWidthPixels = mInfra->mGameTexts->GetWidthPixelsGameText(newMsg->text,
                                                                                 mInfra->mGameTexts->HudWhiteTextBannerFont);

    //calculate text position in a way that the new text is centered in the middle of the banner
    newMsg->textPosition.Y = (mInfra->mScreenRes.Height - 39);
    newMsg->textPosition.X = (mInfra->mScreenRes.Width / 2) - (currentTextWidthPixels / 2.0);

    //add this new message to our current message vector
    this->bannerMessageVec->push_back(newMsg);

    //if there are now more then one message in the buffer
    //make the time the messages are shown shorten, to speed up
    //message viewing
    irr::f32 minShowDuration = DEF_HUD_BANNERTEXT_MINSHOWTIME;

    if (bannerMessageVec->size() > 1) {
       std::vector<BannerTextMessageStruct*>::iterator it;
       for (it = this->bannerMessageVec->begin(); it != this->bannerMessageVec->end(); ++it) {
           if (!(*it)->permanentMsg) {
             (*it)->textStillShownSec = minShowDuration - (*it)->textAlreadyShownSec;
           }
       }
    }
}

//if showDurationSec is negative, the text will be shown until it is deleted
//with a call to function RemoveGreenBigText
//if blinking is true text will blink (for example used for final lap text), If false
//text does not blink (as used when player died and waits for repair craft)
void HUD::ShowGreenBigText(char* text, irr::f32 showDurationSec, bool blinking) {
    //we need to allocate or own array of char and copy the data there for us later.
    //because we can not rely on a pntr to an array we got via
    //parameter; The array that we point to could disappear afterwards
    //because it was local in a function or similar problem.
    //Don't ask me why I know :)
    int textLen = strlen(text);
    currentBigGreenText = new char[textLen + 1];
    strcpy(currentBigGreenText, text);

    blinkBigGreenText = blinking;
    if (showDurationSec < 0.0f) {
        permanentBigGreenText = true;
    } else {
        permanentBigGreenText = false;
    }

    currentBigGreenTextStillShownSec = showDurationSec;

    currentBigGreenTextState = 2;
    irr::f32 blinkPeriod = DEF_HUD_BIGGREENTEXT_BLINKPERIODTIME;
    currentBigGreenTextBlinkPeriod = blinkPeriod;

    //calculate big green text draw position
    //we use HudBigGreenText font
    irr::u32 currentTextWidthPixels = mInfra->mGameTexts->GetWidthPixelsGameText(currentBigGreenText,
                                                                                 mInfra->mGameTexts->HudBigGreenText);
    irr::u32 currentTextHeightPixels = mInfra->mGameTexts->GetHeightPixelsGameText(currentBigGreenText,
                                                                                   mInfra->mGameTexts->HudBigGreenText);

    //calculate text position in a way that the new text is centered in the middle of the player screen
    currentBigGreenTextDrawPosition.Y = (mInfra->mScreenRes.Height / 2) - (currentTextHeightPixels / 2.0);
    currentBigGreenTextDrawPosition.X = (mInfra->mScreenRes.Width / 2) - (currentTextWidthPixels / 2.0);
}

void HUD::InitTargetStuff() {
    //initialize target other player ships stuff
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    targetSymbol = new HudDisplayPart();
    //green target symbol
    targetSymbol->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0257.bmp");

    //red target symbol
    targetSymbol->altTexture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0258.bmp");

    mInfra->mDriver->makeColorKeyTexture(targetSymbol->texture, irr::core::position2d<irr::s32>(0,0));
    mInfra->mDriver->makeColorKeyTexture(targetSymbol->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetSymbol->sizeTex = targetSymbol->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetSymbol->drawScrPosition.set(0,0);

    //arrow left of target symbol
    targetArrowLeft = new HudDisplayPart();
    //green arrow left
    targetArrowLeft->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0263.bmp");

    //red arrow left
    targetArrowLeft->altTexture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0264.bmp");

    mInfra->mDriver->makeColorKeyTexture(targetArrowLeft->texture, irr::core::position2d<irr::s32>(0,0));
    mInfra->mDriver->makeColorKeyTexture(targetArrowLeft->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetArrowLeft->sizeTex = targetArrowLeft->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetArrowLeft->drawScrPosition.set(0,0);

    //arrow right of target symbol
    targetArrowRight = new HudDisplayPart();
    //green arrow right
    targetArrowRight->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0265.bmp");

    //red arrow right
    targetArrowRight->altTexture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0266.bmp");

    mInfra->mDriver->makeColorKeyTexture(targetArrowRight->texture, irr::core::position2d<irr::s32>(0,0));
    mInfra->mDriver->makeColorKeyTexture(targetArrowRight->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetArrowRight->sizeTex = targetArrowRight->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetArrowRight->drawScrPosition.set(0,0);

    //arrow above of target symbol
    targetArrowAbove = new HudDisplayPart();
    //green arrow above
    targetArrowAbove->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0259.bmp");

    //red arrow above
    targetArrowAbove->altTexture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0260.bmp");

    mInfra->mDriver->makeColorKeyTexture(targetArrowAbove->texture, irr::core::position2d<irr::s32>(0,0));
    mInfra->mDriver->makeColorKeyTexture(targetArrowAbove->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetArrowAbove->sizeTex = targetArrowAbove->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetArrowAbove->drawScrPosition.set(0,0);

    //arrow below of target symbol
    targetArrowBelow = new HudDisplayPart();
    //green arrow below
    targetArrowBelow->texture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0261.bmp");

    //red arrow below
    targetArrowBelow->altTexture = mInfra->mDriver->getTexture("extract/hud1player/panel0-1-0262.bmp");

    mInfra->mDriver->makeColorKeyTexture(targetArrowBelow->texture, irr::core::position2d<irr::s32>(0,0));
    mInfra->mDriver->makeColorKeyTexture(targetArrowBelow->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetArrowBelow->sizeTex = targetArrowBelow->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetArrowBelow->drawScrPosition.set(0,0);

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

//Just taken from Irrlicht source code ISceneCollisionManager
//And then modified
//! Calculates 2d screen position from a 3d position.
core::position2d<s32> HUD::getScreenCoordinatesFrom3DPosition(
    const core::vector3df & pos3d, ICameraSceneNode* camera)
{

    irr::u32 Width = mInfra->mScreenRes.Width;
    irr::u32 Height = mInfra->mScreenRes.Height;

    Width /= 2;
    Height /= 2;

    core::matrix4 trans = camera->getProjectionMatrix();
    trans *= camera->getViewMatrix();

    f32 transformedPos[4] = { pos3d.X, pos3d.Y, pos3d.Z, 1.0f };

    trans.multiplyWith1x4Matrix(transformedPos);

    if (transformedPos[3] < 0)
        return core::position2d<s32>(-10000,-10000);

    const f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
        core::reciprocal(transformedPos[3]);

    return core::position2d<s32>(
            Width + core::round32(Width * (transformedPos[0] * zDiv)),
            Height - core::round32(Height * (transformedPos[1] * zDiv)));
}

void HUD::RenderTargetSymbol(irr::f32 deltaTime) {
      //does player target currently other opponent?
      if (monitorWhichPlayer->mTargetPlayer != NULL) {
          //yes, player has currently a target
          //we need to draw target symbol

          this->targetNameBlinkTimer += deltaTime;

          if (targetNameBlinkTimer > DEF_HUD_TARGETNAME_BLINKPERIODE) {
              this->targetNameBlinkTimer = 0.0f;
              currShowTargetName = !currShowTargetName;
          }

          //first we need to figure out where to draw target symbol on the 2D screen, so that the symbol is around
          //the targets player craft
          ICameraSceneNode* actCamera = mInfra->mSmgr->getActiveCamera();

          irr::core::vector3df targetPlayerPos = monitorWhichPlayer->mTargetPlayer->phobj->physicState.position;

          irr::core::vector2di targetPos = getScreenCoordinatesFrom3DPosition(targetPlayerPos, actCamera);

          irr::u32 targetSymBHalfWidth = targetSymbol->sizeTex.Width / 2;
          irr::u32 targetSymBHalfHeight = targetSymbol->sizeTex.Height / 2;

          targetSymbol->drawScrPosition.X = targetPos.X - targetSymBHalfWidth;
          targetSymbol->drawScrPosition.Y = targetPos.Y - targetSymBHalfHeight;

          irr::u32 lockProgress = this->monitorWhichPlayer->mTargetMissleLockProgr;

          targetArrowLeft->drawScrPosition.X =
                  targetPos.X - targetSymBHalfWidth - lockProgress - targetArrowLeft->sizeTex.Width;
          targetArrowLeft->drawScrPosition.Y = targetPos.Y - targetArrowLeft->sizeTex.Height / 2.0;

          targetArrowRight->drawScrPosition.X = targetPos.X + targetSymBHalfWidth + lockProgress;
          targetArrowRight->drawScrPosition.Y = targetArrowLeft->drawScrPosition.Y;

          targetArrowAbove->drawScrPosition.X = targetPos.X - targetArrowAbove->sizeTex.Width / 2.0;
          targetArrowAbove->drawScrPosition.Y =
                  targetPos.Y - targetSymBHalfHeight - lockProgress  - targetArrowAbove->sizeTex.Height;

          targetArrowBelow->drawScrPosition.X = targetArrowAbove->drawScrPosition.X;
          targetArrowBelow->drawScrPosition.Y = targetPos.Y + targetSymBHalfHeight + lockProgress;

          if (!monitorWhichPlayer->mTargetMissleLock) {
             //no missle lock, green symbol and green text
             mInfra->mDriver->draw2DImage(targetSymbol->texture, targetSymbol->drawScrPosition,
                targetSymbol->sourceRect, 0, *mColorSolid, true);

               if (currShowTargetName) {
                    //write player name next to target symbol
                    mInfra->mGameTexts->DrawHudSmallText(monitorWhichPlayer->mTargetPlayer->mPlayerStats->name,
                        mInfra->mGameTexts->HudTargetNameGreen,
                            irr::core::position2di(targetPos.X + targetSymBHalfWidth + 2,
                                            targetSymbol->drawScrPosition.Y));
               }

             //left green arrow
             mInfra->mDriver->draw2DImage(targetArrowLeft->texture, targetArrowLeft->drawScrPosition,
                targetArrowLeft->sourceRect, 0, *mColorSolid, true);

             //right green arrow
             mInfra->mDriver->draw2DImage(targetArrowRight->texture, targetArrowRight->drawScrPosition,
                targetArrowRight->sourceRect, 0, *mColorSolid, true);

             //above green arrow
             mInfra->mDriver->draw2DImage(targetArrowAbove->texture, targetArrowAbove->drawScrPosition,
                targetArrowAbove->sourceRect, 0, *mColorSolid, true);

             //below green arrow
             mInfra->mDriver->draw2DImage(targetArrowBelow->texture, targetArrowBelow->drawScrPosition,
                targetArrowBelow->sourceRect, 0, *mColorSolid, true);
          } else {
              //we also have missile lock, red symbol and red text
              mInfra->mDriver->draw2DImage(targetSymbol->altTexture, targetSymbol->drawScrPosition,
                 targetSymbol->sourceRect, 0, *mColorSolid, true);

              if (currShowTargetName) {
                //write player name next to target symbol
                mInfra->mGameTexts->DrawHudSmallText(monitorWhichPlayer->mTargetPlayer->mPlayerStats->name,
                        mInfra->mGameTexts->HudTargetNameRed,
                        irr::core::position2di(targetPos.X + targetSymBHalfWidth + 2,
                                             targetSymbol->drawScrPosition.Y));
              }

              //left red arrow
              mInfra->mDriver->draw2DImage(targetArrowLeft->altTexture, targetArrowLeft->drawScrPosition,
                 targetArrowLeft->sourceRect, 0, *mColorSolid, true);

              //right red arrow
              mInfra->mDriver->draw2DImage(targetArrowRight->altTexture, targetArrowRight->drawScrPosition,
                 targetArrowRight->sourceRect, 0, *mColorSolid, true);

              //above red arrow
              mInfra->mDriver->draw2DImage(targetArrowAbove->altTexture, targetArrowAbove->drawScrPosition,
                 targetArrowAbove->sourceRect, 0, *mColorSolid, true);

              //below red arrow
              mInfra->mDriver->draw2DImage(targetArrowBelow->altTexture, targetArrowBelow->drawScrPosition,
                 targetArrowBelow->sourceRect, 0, *mColorSolid, true);
          }
     }
}

HUD::HUD(InfrastructureBase* infra) {
    mInfra = infra;

    monitorWhichPlayer = NULL;

    //create this colors as the are used all the time
    //to save cpu cycles
    mColorSolid = new irr::video::SColor(255,255,255,255);
    mColorTransparent = new irr::video::SColor(100,255,255,255);

    InitShieldBar();
    InitAmmoBar();
    InitGasolineBar();
    InitThrottleBar();
    InitSpeedBar();
    InitMGHeatBar();
    InitRacePosition();
    InitUpgradeBar();
    InitHudBannerText();
    InitTargetStuff();
    InitStartSignal();
    InitBrokenGlas();

    //create vector for all the banner text messages we receive
    bannerMessageVec = new std::vector<BannerTextMessageStruct*>();
    bannerMessageVec->clear();
}

void HUD::CleanUpHudDisplayPartVector(std::vector<HudDisplayPart*> &pntrVector) {
    std::vector<HudDisplayPart*>::iterator itHudDisplayPart;
    HudDisplayPart* pntr;

    //delete all existing HudDisplayParts
    itHudDisplayPart = pntrVector.begin();

    while (itHudDisplayPart != pntrVector.end()) {
           pntr = (*itHudDisplayPart);
           itHudDisplayPart = pntrVector.erase(itHudDisplayPart);

           if (pntr->texture != NULL) {
               //remove underlying texture
               mInfra->mDriver->removeTexture(pntr->texture);
           }

           if (pntr->altTexture != NULL) {
               //remove underlying texture
               mInfra->mDriver->removeTexture(pntr->altTexture);
           }

           delete pntr;
    }
}

HUD::~HUD() {
    //clean up HUD stuff
    CleanUpHudDisplayPartVector(*shieldBar);
    shieldBar = NULL;

    CleanUpHudDisplayPartVector(*ammoBar);
    ammoBar = NULL;

    CleanUpHudDisplayPartVector(*gasolineBar);
    gasolineBar = NULL;

    CleanUpHudDisplayPartVector(*throttleBar);
    throttleBar = NULL;

    CleanUpHudDisplayPartVector(*speedBar);
    speedBar = NULL;

    CleanUpHudDisplayPartVector(*currRacePlayerPosition);
    currRacePlayerPosition = NULL;

    CleanUpHudDisplayPartVector(*numberPlayers);
    numberPlayers = NULL;

    CleanUpHudDisplayPartVector(*mgHeatBar);
    mgHeatBar = NULL;

    CleanUpHudDisplayPartVector(*upgradeBar);
    upgradeBar = NULL;

    CleanUpHudDisplayPartVector(*bannerTextState1);
    bannerTextState1 = NULL;

    CleanUpHudDisplayPartVector(*bannerTextState2);
    bannerTextState2 = NULL;

    CleanUpHudDisplayPartVector(*bannerTextState3);
    bannerTextState3 = NULL;

    CleanUpHudDisplayPartVector(*bannerTextState4);
    bannerTextState4 = NULL;

    CleanUpHudDisplayPartVector(*bannerTextState5);
    bannerTextState5 = NULL;

    CleanUpHudDisplayPartVector(*bannerTextState6);
    bannerTextState6 = NULL;

    CleanUpHudDisplayPartVector(*bannerTextState7);
    bannerTextState7 = NULL;

    CleanUpHudDisplayPartVector(*startSignal);
    startSignal = NULL;

    if (brokenGlas->texture != NULL) {
        //remove underlying texture
        mInfra->mDriver->removeTexture(brokenGlas->texture);
    }

    if (brokenGlas->altTexture != NULL) {
        //remove underlying texture
        mInfra->mDriver->removeTexture(brokenGlas->altTexture);
    }

    delete brokenGlas;
    brokenGlas = NULL;

    CleanUpAllBannerMessages();
    delete this->bannerMessageVec;
    this->bannerMessageVec = NULL;

    //if there is currently a big green HUD text
    //shown deallocate it
    if (currentBigGreenText != NULL) {
        delete[] currentBigGreenText;
        currentBigGreenText = NULL;
    }

    delete mColorSolid;
    delete mColorTransparent;
}

void HUD::DrawFinishedPlayerList() {
    irr::u8 nrFinishedPlayers = this->monitorWhichPlayer->mRace->playerRaceFinishedVec.size();

    if (nrFinishedPlayers > 0) {
        irr::u32 posXNr = 3;
        irr::u32 posXPlayerName = 25;
        irr::u32 posYPlayerName = 125;
        irr::u32 posY = 120;

        char posStr[5];

        //at least one player has finished race already
        //therefore we need to show position list in HUD
        for (irr::u8 position = 1; position <= nrFinishedPlayers; position++) {

            //first draw 2 red arrow symbol next to lap number display
            sprintf(posStr, "%d", position);

            mInfra->mGameTexts->DrawGameNumberText(posStr, mInfra->mGameTexts->HudLaptimeNumberGrey,
                                               irr::core::vector2d<irr::s32>(posXNr, posY));

            mInfra->mGameTexts->DrawHudSmallText(
                        this->monitorWhichPlayer->mRace->playerRaceFinishedVec.at(position - 1)->mPlayerStats->name,
                        this->mInfra->mGameTexts->HudTargetNameRed,  irr::core::vector2d<irr::s32>(posXPlayerName, posYPlayerName));

            posY += 24;
            posYPlayerName += 24;
        }
    }
}

