/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "hud.h"

void HUD::InitShieldBar() {
    //create dynamic variable
    shieldBar = new std::vector<HudDisplayPart*>;

    //initialize shield bar
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = -2;

    //Piece 1
    HudDisplayPart* shieldBarPiece = new HudDisplayPart();
    shieldBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0071.bmp");
    shieldBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(shieldBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    shieldBarPiece->sizeTex = shieldBarPiece->texture->getSize();
    shieldBarPiece->drawScrPosition.set(3+Xoff, 3);

    shieldBar->push_back(shieldBarPiece);

    //Piece 2
    shieldBarPiece = new HudDisplayPart();
    shieldBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0072.bmp");
    shieldBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(shieldBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    shieldBarPiece->sizeTex = shieldBarPiece->texture->getSize();
    shieldBarPiece->drawScrPosition.set(41+Xoff, 3);

    shieldBar->push_back(shieldBarPiece);

    //Piece 3
    shieldBarPiece = new HudDisplayPart();
    shieldBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0073.bmp");
    shieldBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(shieldBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    shieldBarPiece->sizeTex = shieldBarPiece->texture->getSize();
    shieldBarPiece->drawScrPosition.set(67+Xoff, 3);

    shieldBar->push_back(shieldBarPiece);

    //Piece 4
    shieldBarPiece = new HudDisplayPart();
    shieldBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0074.bmp");
    shieldBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(shieldBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    shieldBarPiece->sizeTex = shieldBarPiece->texture->getSize();
    shieldBarPiece->drawScrPosition.set(75+Xoff, 3);

    shieldBar->push_back(shieldBarPiece);

    //Piece 5
    shieldBarPiece = new HudDisplayPart();
    shieldBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0075.bmp");
    shieldBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(shieldBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    shieldBarPiece->sizeTex = shieldBarPiece->texture->getSize();
    shieldBarPiece->drawScrPosition.set(95+Xoff, 3);

    shieldBar->push_back(shieldBarPiece);

    //Piece 6
    shieldBarPiece = new HudDisplayPart();
    shieldBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0076.bmp");
    shieldBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(shieldBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    shieldBarPiece->sizeTex = shieldBarPiece->texture->getSize();
    shieldBarPiece->drawScrPosition.set(113+Xoff, 3);

    shieldBar->push_back(shieldBarPiece);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitAmmoBar() {
    //create dynamic variable
    ammoBar = new std::vector<HudDisplayPart*>;

    //initialize ammo bar
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0

     int Xoff = + 2;
     irr::u32 scrSizeX = this->screenResolution.Width;

    //Piece 1
    HudDisplayPart* ammoBarPiece = new HudDisplayPart();
    ammoBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0090.bmp");
    ammoBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(ammoBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    ammoBarPiece->sizeTex = ammoBarPiece->texture->getSize();
    ammoBarPiece->drawScrPosition.set(scrSizeX - 43 + Xoff, 3);

    ammoBar->push_back(ammoBarPiece);

    //Piece 2
    ammoBarPiece = new HudDisplayPart();
    ammoBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0089.bmp");
    ammoBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(ammoBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    ammoBarPiece->sizeTex = ammoBarPiece->texture->getSize();
    ammoBarPiece->drawScrPosition.set(scrSizeX - 86 + Xoff, 3);

    ammoBar->push_back(ammoBarPiece);

    //Piece 3
    ammoBarPiece = new HudDisplayPart();
    ammoBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0088.bmp");
    ammoBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(ammoBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    ammoBarPiece->sizeTex = ammoBarPiece->texture->getSize();
    ammoBarPiece->drawScrPosition.set(scrSizeX - 114 + Xoff, 3);

    ammoBar->push_back(ammoBarPiece);

    //Piece 4
    ammoBarPiece = new HudDisplayPart();
    ammoBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0087.bmp");
    ammoBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(ammoBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    ammoBarPiece->sizeTex = ammoBarPiece->texture->getSize();
    ammoBarPiece->drawScrPosition.set(scrSizeX - 134 + Xoff, 3);

    ammoBar->push_back(ammoBarPiece);

    //Piece 5
    ammoBarPiece = new HudDisplayPart();
    ammoBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0086.bmp");
    ammoBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(ammoBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    ammoBarPiece->sizeTex = ammoBarPiece->texture->getSize();
    ammoBarPiece->drawScrPosition.set(scrSizeX - 152 + Xoff, 3);

    ammoBar->push_back(ammoBarPiece);

    //Piece 6
    ammoBarPiece = new HudDisplayPart();
    ammoBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0085.bmp");
    ammoBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(ammoBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    ammoBarPiece->sizeTex = ammoBarPiece->texture->getSize();
    ammoBarPiece->drawScrPosition.set(scrSizeX - 180 + Xoff, 3);

    ammoBar->push_back(ammoBarPiece);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitGasolineBar() {
    //create dynamic variable
    gasolineBar = new std::vector<HudDisplayPart*>;

    //initialize gasoline bar
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = -3;

    //Piece 1
    HudDisplayPart* gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0000.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(80+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 2
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0001.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(135+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 3
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0002.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(182+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 4
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0003.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(217+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 5
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0004.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(243+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 6
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0005.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(268+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 7
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0006.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(290+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 8
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0007.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(311+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 9
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0008.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(330+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 10
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0009.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(348+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 11
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0010.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(373+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 12
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0011.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(398+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 13
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0012.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(422+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 14
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0013.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(448+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 15
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0014.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(472+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 16
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0015.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(497+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    //Piece 17
    gasolineBarPiece = new HudDisplayPart();
    gasolineBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0016.bmp");
    gasolineBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(gasolineBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    gasolineBarPiece->sizeTex = gasolineBarPiece->texture->getSize();
    gasolineBarPiece->drawScrPosition.set(523+Xoff, 3);

    gasolineBar->push_back(gasolineBarPiece);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitStartSignal() {
    //create dynamic variable
    startSignal = new std::vector<HudDisplayPart*>;

    //initialize start signal
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    /****************************
     * Start signal             *
     ****************************/

    //red light
    HudDisplayPart* symbol = new HudDisplayPart();
    //signal not lit
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0280.bmp");
    //alternative texture is the lit light
    symbol->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0281.bmp");
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(symbol->altTexture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(50, 11);

    startSignal->push_back(symbol);

    //yellow light
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0282.bmp");
    symbol->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0283.bmp");
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(symbol->altTexture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(113, 10);

    startSignal->push_back(symbol);

    //green light
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0284.bmp");
    symbol->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0285.bmp");
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(symbol->altTexture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(176, 10);

    startSignal->push_back(symbol);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitBrokenGlas() {
    //initialize texture for broken glass
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    /****************************
     * Broken glas              *
     ****************************/

    //broken glas image
    brokenGlas = new HudDisplayPart();
    brokenGlas->texture = myDriver->getTexture("extract/hud1player/panel0-1-0229.bmp");
    myDriver->makeColorKeyTexture(brokenGlas->texture, irr::core::position2d<irr::s32>(0,0));
    brokenGlas->sizeTex = brokenGlas->texture->getSize();
    brokenGlas->drawScrPosition.set(0, 0);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitMGHeatBar() {
    //create dynamic variable
    mgHeatBar = new std::vector<HudDisplayPart*>;

    //initialize machine gun heat bar
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = 0;

    //Piece 1
    HudDisplayPart* mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0091.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(506+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 2
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0092.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(521+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 3
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0093.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(527+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 4
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0094.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(533+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 5
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0095.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(539+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 6
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0096.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(545+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 7
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0097.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(551+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 8
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0098.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(557+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 9
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0099.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(569+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 10
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0100.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(581+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 11
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0101.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(589+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 12
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0102.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(597+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 13
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0103.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(605+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 14
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0104.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(613+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 15
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0105.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(621+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    //Piece 16
    mgHeatBarPiece = new HudDisplayPart();
    mgHeatBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0106.bmp");
    mgHeatBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(mgHeatBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    mgHeatBarPiece->sizeTex = mgHeatBarPiece->texture->getSize();
    mgHeatBarPiece->drawScrPosition.set(629+Xoff, 29);

    mgHeatBar->push_back(mgHeatBarPiece);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitThrottleBar() {
    //create dynamic variable
    throttleBar = new std::vector<HudDisplayPart*>;

    //initialize throttle bar
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = 0;

    //Piece 1
    HudDisplayPart* throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0017.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0036.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(115+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 2
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0018.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0037.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(139+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 3
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0019.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0038.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(160+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 4
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0020.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0039.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(185+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 5
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0021.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0040.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(206+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 6
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0022.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0041.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(229+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 7
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0023.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0042.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(247+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 8
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0024.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0043.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(268+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 9
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0025.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0044.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(289+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 10
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0026.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0045.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(308+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 11
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0027.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0046.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(327+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 12
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0028.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0047.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(346+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 13
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0029.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0048.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(368+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 14
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0030.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0049.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(394+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 15
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0031.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0050.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(420+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 16
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0032.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0051.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(445+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 17
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0033.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0052.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(468+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 18
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0034.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0053.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(491+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    //Piece 19
    throttleBarPiece = new HudDisplayPart();
    throttleBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0035.bmp");
    throttleBarPiece->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0054.bmp");
    myDriver->makeColorKeyTexture(throttleBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(throttleBarPiece->altTexture, irr::core::position2d<irr::s32>(0,0));
    throttleBarPiece->sizeTex = throttleBarPiece->texture->getSize();
    throttleBarPiece->drawScrPosition.set(513+Xoff, 15);

    throttleBar->push_back(throttleBarPiece);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitSpeedBar() {
    //create dynamic variable
    speedBar = new std::vector<HudDisplayPart*>;

    //initialize booster bar
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0
    int Xoff = 0;

    //Piece 1
    HudDisplayPart* speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0055.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(125+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 2
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0056.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(147+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 3
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0057.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(168+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 4
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0058.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(189+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 5
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0059.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(210+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 6
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0060.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(231+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 7
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0061.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(249+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 8
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0062.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(270+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 9
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0063.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(291+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 10
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0064.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(310+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 11
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0065.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(327+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 12
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0066.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(345+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 13
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0067.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(361+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 14
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0068.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(382+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 15
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0069.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(421+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    //Piece 16
    speedBarPiece = new HudDisplayPart();
    speedBarPiece->texture = myDriver->getTexture("extract/hud1player/panel0-1-0070.bmp");
    speedBarPiece->altTexture = NULL;
    myDriver->makeColorKeyTexture(speedBarPiece->texture, irr::core::position2d<irr::s32>(0,0));
    speedBarPiece->sizeTex = speedBarPiece->texture->getSize();
    speedBarPiece->drawScrPosition.set(457+Xoff, 27);

    speedBar->push_back(speedBarPiece);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitRacePosition() {
    //create dynamic variable
    currRacePlayerPosition = new std::vector<HudDisplayPart*>;
    numberPlayers = new std::vector<HudDisplayPart*>;

    //initialize race position characters
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
     //driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    //for all shield images we can get the alpha channel color information from
    //pixel 0,0

    //**********************************************************
    //* First init characters for current player race position *
    //* This is the bigger grey character on the far left      *
    //**********************************************************
    int Xoff = 0;

    //number 0
    HudDisplayPart* playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0107.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 1
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0108.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 2
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0109.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 3
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0110.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 4
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0111.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 5
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0112.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 6
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0113.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 7
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0114.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 8
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0115.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //number 9
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0116.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(2+Xoff, 28);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //add the slash
    playerRacePositionChar = new HudDisplayPart();
    playerRacePositionChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0137.bmp");
    playerRacePositionChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(playerRacePositionChar->texture, irr::core::position2d<irr::s32>(0,0));
    playerRacePositionChar->sizeTex = playerRacePositionChar->texture->getSize();
    playerRacePositionChar->drawScrPosition.set(53, 72);

    currRacePlayerPosition->push_back(playerRacePositionChar);

    //*************************************************************
    //* 2nd init characters for overall number of players in race *
    //* This is the smaller grey character right from the slash   *
    //*************************************************************

    Xoff = 0;

    //number 0
    HudDisplayPart* numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0127.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 1
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0128.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 2
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0129.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 3
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0130.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 4
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0131.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 5
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0132.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 6
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0133.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 7
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0134.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 8
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0135.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //number 9
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0136.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    //add the slash
    numberPlayerChar = new HudDisplayPart();
    numberPlayerChar->texture = myDriver->getTexture("extract/hud1player/panel0-1-0137.bmp");
    numberPlayerChar->altTexture = NULL;
    myDriver->makeColorKeyTexture(numberPlayerChar->texture, irr::core::position2d<irr::s32>(0,0));
    numberPlayerChar->sizeTex = numberPlayerChar->texture->getSize();
    numberPlayerChar->drawScrPosition.set(83+Xoff, 72);

    numberPlayers->push_back(numberPlayerChar);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

void HUD::InitUpgradeBar() {
    //create dynamic variable
    upgradeBar = new std::vector<HudDisplayPart*>;

    //initialize upgrade bar symbols
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    /****************************
     * Minigun                  *
     ****************************/

    //minigun symbol
    HudDisplayPart* symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0267.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(149, 71);

    upgradeBar->push_back(symbol);

    //minigun upgrade level 1 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0269.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(219, 71);

    upgradeBar->push_back(symbol);

    //minigun upgrade level 2 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0269.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(233, 71);

    upgradeBar->push_back(symbol);

    //minigun upgrade level 3 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0269.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(247, 71);

    upgradeBar->push_back(symbol);

    /****************************
    * Missile                   *
    ****************************/

    //missile symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0270.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(265, 71);

    upgradeBar->push_back(symbol);

    //missile upgrade level 1 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0269.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(318, 71);

    upgradeBar->push_back(symbol);

    //missile upgrade level 2 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0269.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(332, 71);

    upgradeBar->push_back(symbol);

    //missile upgrade level 3 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0269.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(346, 71);

    upgradeBar->push_back(symbol);

    /****************************
    * Booster                  *
    ****************************/

    //booster symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0273.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(364, 71);

    upgradeBar->push_back(symbol);

    //booster upgrade level 1 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0274.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(415, 71);

    upgradeBar->push_back(symbol);

    //booster upgrade level 2 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0274.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(429, 71);

    upgradeBar->push_back(symbol);

    //booster upgrade level 3 symbol
    symbol = new HudDisplayPart();
    symbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0274.bmp");
    symbol->altTexture = NULL;
    myDriver->makeColorKeyTexture(symbol->texture, irr::core::position2d<irr::s32>(0,0));
    symbol->sizeTex = symbol->texture->getSize();
    symbol->drawScrPosition.set(443, 71);

    upgradeBar->push_back(symbol);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
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
                 this->monitorWhichPlayer->StartPlayingWarningSound();
                 mCurrentMessageWarningActive = true;
             }

           //Draw the text
           //for Hud banner text we use HudWhiteTextBannerFont
           myTextRenderer->DrawGameText(currShownBannerMsg->text, myTextRenderer->HudWhiteTextBannerFont, currShownBannerMsg->textPosition);

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
         myDriver->draw2DImage((*startSignal)[0]->altTexture, (*startSignal)[0]->drawScrPosition,
               irr::core::rect<irr::s32>(0,0, (*startSignal)[0]->sizeTex.Width, (*startSignal)[0]->sizeTex.Height), 0,
               irr::video::SColor(255,255,255,255), true);
    } else {
        myDriver->draw2DImage((*startSignal)[0]->texture, (*startSignal)[0]->drawScrPosition,
              irr::core::rect<irr::s32>(0,0, (*startSignal)[0]->sizeTex.Width, (*startSignal)[0]->sizeTex.Height), 0,
              irr::video::SColor(255,255,255,255), true);
    }

    if (yellowLit) {
         myDriver->draw2DImage((*startSignal)[1]->altTexture, (*startSignal)[1]->drawScrPosition,
               irr::core::rect<irr::s32>(0,0, (*startSignal)[1]->sizeTex.Width, (*startSignal)[1]->sizeTex.Height), 0,
               irr::video::SColor(255,255,255,255), true);
    } else {
        myDriver->draw2DImage((*startSignal)[1]->texture, (*startSignal)[1]->drawScrPosition,
              irr::core::rect<irr::s32>(0,0, (*startSignal)[1]->sizeTex.Width, (*startSignal)[1]->sizeTex.Height), 0,
              irr::video::SColor(255,255,255,255), true);
    }

    if (greenLit) {
         myDriver->draw2DImage((*startSignal)[2]->altTexture, (*startSignal)[2]->drawScrPosition,
               irr::core::rect<irr::s32>(0,0, (*startSignal)[2]->sizeTex.Width, (*startSignal)[2]->sizeTex.Height), 0,
               irr::video::SColor(255,255,255,255), true);
    } else {
        myDriver->draw2DImage((*startSignal)[2]->texture, (*startSignal)[2]->drawScrPosition,
              irr::core::rect<irr::s32>(0,0, (*startSignal)[2]->sizeTex.Width, (*startSignal)[2]->sizeTex.Height), 0,
              irr::video::SColor(255,255,255,255), true);
    }
}

void HUD::DrawHUD1PlayerRace(irr::f32 deltaTime) {
    if (monitorWhichPlayer != NULL) {

        DrawFinishedPlayerList();

        //any broken glas?
        if (this->monitorWhichPlayer->brokenGlasVec->size() > 0) {
            std::vector<HudDisplayPart*>::iterator itGlasBreak;

            for (itGlasBreak = this->monitorWhichPlayer->brokenGlasVec->begin(); itGlasBreak != this->monitorWhichPlayer->brokenGlasVec->end(); ++itGlasBreak) {
                myDriver->draw2DImage((*itGlasBreak)->texture, (*itGlasBreak)->drawScrPosition,
                      irr::core::rect<irr::s32>(0,0, (*itGlasBreak)->sizeTex.Width, (*itGlasBreak)->sizeTex.Height), 0,
                      irr::video::SColor(255,255,255,255), true);
            }
        }

        int sizeVec = shieldBar->size();

        irr::f32 perc = (monitorWhichPlayer->mPlayerStats->shieldVal / monitorWhichPlayer->mPlayerStats->shieldMax) * sizeVec;

        if (perc < 0)
            perc = 0;

        if (perc > sizeVec)
            perc = sizeVec;

        //draw as many shield bars as the current shield state of the ship allows
        for (int i = 0; i < perc ; i++) {
            myDriver->draw2DImage((*shieldBar)[i]->texture, (*shieldBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*shieldBar)[i]->sizeTex.Width, (*shieldBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        sizeVec = ammoBar->size();

        perc = (monitorWhichPlayer->mPlayerStats->ammoVal / monitorWhichPlayer->mPlayerStats->ammoMax) * sizeVec;

        if (perc < 0)
            perc = 0;

        if (perc > sizeVec)
            perc = sizeVec;

        for (int i = 0; i < perc; i++) {
            myDriver->draw2DImage((*ammoBar)[i]->texture, (*ammoBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*ammoBar)[i]->sizeTex.Width, (*ammoBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        sizeVec = gasolineBar->size();

        perc = (monitorWhichPlayer->mPlayerStats->gasolineVal / monitorWhichPlayer->mPlayerStats->gasolineMax) * sizeVec;

        if (perc < 0)
            perc = 0;

        if (perc > sizeVec)
            perc = sizeVec;

        for (int i = (sizeVec - perc); i < sizeVec; i++) {
            myDriver->draw2DImage((*gasolineBar)[i]->texture, (*gasolineBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*gasolineBar)[i]->sizeTex.Width, (*gasolineBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        sizeVec = throttleBar->size();

        irr::f32 perc2 = (monitorWhichPlayer->mPlayerStats->throttleVal / monitorWhichPlayer->mPlayerStats->throttleMax) * sizeVec;
        perc = (monitorWhichPlayer->mPlayerStats->boosterVal / monitorWhichPlayer->mPlayerStats->boosterMax) * sizeVec;

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
            myDriver->draw2DImage((*throttleBar)[i]->texture, (*throttleBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*throttleBar)[i]->sizeTex.Width, (*throttleBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        //according to current booster state draw over it with lighted colors
        for (int i = 0; i < perc; i++) {
            myDriver->draw2DImage((*throttleBar)[i]->altTexture, (*throttleBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*throttleBar)[i]->sizeTex.Width, (*throttleBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        sizeVec = speedBar->size();

        perc = (monitorWhichPlayer->mPlayerStats->speed / monitorWhichPlayer->mPlayerStats->speedMax) * sizeVec;

        if (perc < 0)
            perc = 0;

        if (perc > sizeVec)
            perc = sizeVec;

        for (int i = 0; i < perc; i++) {
            myDriver->draw2DImage((*speedBar)[i]->texture, (*speedBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*speedBar)[i]->sizeTex.Width, (*speedBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        //Draw machine gun heat bar
        sizeVec = mgHeatBar->size();

        perc = (monitorWhichPlayer->mPlayerStats->mgHeatVal / monitorWhichPlayer->mPlayerStats->mgHeatMax) * sizeVec;

        if (perc < 0)
            perc = 0;

        if (perc > sizeVec)
            perc = sizeVec;

        for (int i = 0; i < perc; i++) {
            myDriver->draw2DImage((*mgHeatBar)[i]->texture, (*mgHeatBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*mgHeatBar)[i]->sizeTex.Width, (*mgHeatBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        //Draw upgrade bar
        //symbol number 0 is the minigun symbol itself (for basic upgrade level 0)
        //the next three symbols 1, 2 and 3 are for upgrade levels 1, 2 and 3
        for (int i = 0; i <= monitorWhichPlayer->mPlayerStats->currMinigunUpgradeLevel; i++) {
            myDriver->draw2DImage((*upgradeBar)[i]->texture, (*upgradeBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*upgradeBar)[i]->sizeTex.Width, (*upgradeBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        //symbol number 4 is the rocket symbol itself (for basic upgrade level 0)
        //the next three symbols 5, 6 and 7 are for upgrade levels 1, 2 and 3
        for (int i = 4; i <= (monitorWhichPlayer->mPlayerStats->currRocketUpgradeLevel + 4); i++) {
            myDriver->draw2DImage((*upgradeBar)[i]->texture, (*upgradeBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*upgradeBar)[i]->sizeTex.Width, (*upgradeBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }

        //symbol number 8 is the booster symbol itself (for basic upgrade level 0)
        //the next three symbols 9, 10 and 11 are for upgrade levels 1, 2 and 3
        for (int i = 8; i <= (monitorWhichPlayer->mPlayerStats->currBoosterUpgradeLevel + 8); i++) {
            myDriver->draw2DImage((*upgradeBar)[i]->texture, (*upgradeBar)[i]->drawScrPosition,
                  irr::core::rect<irr::s32>(0,0, (*upgradeBar)[i]->sizeTex.Width, (*upgradeBar)[i]->sizeTex.Height), 0,
                  irr::video::SColor(255,255,255,255), true);
        }


        //draw current player race position
        int hlp = monitorWhichPlayer->mPlayerStats->currRacePlayerPos;

        if (hlp < 0)
            hlp = 0;

        if (hlp > 9)
            hlp = 9;

        myDriver->draw2DImage((*currRacePlayerPosition)[hlp]->texture, (*currRacePlayerPosition)[hlp]->drawScrPosition,
              irr::core::rect<irr::s32>(0,0, (*currRacePlayerPosition)[hlp]->sizeTex.Width, (*currRacePlayerPosition)[hlp]->sizeTex.Height), 0,
              irr::video::SColor(255,255,255,255), true);

        //draw the slash for player position (slash is #10 in the list)
        myDriver->draw2DImage((*currRacePlayerPosition)[10]->texture, (*currRacePlayerPosition)[10]->drawScrPosition,
              irr::core::rect<irr::s32>(0,0, (*currRacePlayerPosition)[10]->sizeTex.Width, (*currRacePlayerPosition)[10]->sizeTex.Height), 0,
              irr::video::SColor(255,255,255,255), true);

        //draw overall number of players in race
        hlp = monitorWhichPlayer->mPlayerStats->overallPlayerNumber;

        if (hlp < 0)
            hlp = 0;

        if (hlp > 9)
            hlp = 9;

        //draw the overall number of players in the race
        myDriver->draw2DImage((*numberPlayers)[hlp]->texture, (*numberPlayers)[hlp]->drawScrPosition,
              irr::core::rect<irr::s32>(0,0, (*numberPlayers)[hlp]->sizeTex.Width, (*numberPlayers)[hlp]->sizeTex.Height), 0,
              irr::video::SColor(255,255,255,255), true);

        //render current lap number information
        char lapNumStr[10];
        //first draw 2 red arrow symbol next to lap number display
        strcpy(&lapNumStr[0], ">");
        //RenderRedTextLapNumber(&lapNumStr[0], irr::core::vector2d<irr::s32>(128, 91));
        myTextRenderer->DrawGameNumberText(lapNumStr, myTextRenderer->HudLaptimeNumberRed, irr::core::vector2d<irr::s32>(128, 91));

        //first built string
        //important! pad with 2 leading zeros!
        sprintf(&lapNumStr[0], "%02d/%02d", monitorWhichPlayer->mPlayerStats->currLapNumber, monitorWhichPlayer->mPlayerStats->raceNumberLaps);
        //RenderRedTextLapNumber(&lapNumStr[0], irr::core::vector2d<irr::s32>(178, 93));
        myTextRenderer->DrawGameNumberText(lapNumStr, myTextRenderer->HudLaptimeNumberRed, irr::core::vector2d<irr::s32>(178, 93));

        //next draw red skull and current player kill count number
        char currKillCountStr[10];

        //first draw red skull
        //strcpy(&currKillCountStr[0], "s");
        //RenderRedTextKillCount(&currKillCountStr[0], irr::core::vector2d<irr::s32>(559, 62));
        strcpy(&currKillCountStr[0], ">");
        myTextRenderer->DrawGameNumberText(&currKillCountStr[0], myTextRenderer->HudKillCounterNumberRed, irr::core::vector2d<irr::s32>(559, 62));

        //now draw current kill number, dont forget the leading zeros!
        sprintf(&currKillCountStr[0], "%02d", monitorWhichPlayer->mPlayerStats->currKillCount);
        //RenderRedTextKillCount(&currKillCountStr[0], irr::core::vector2d<irr::s32>(592, 62));
        myTextRenderer->DrawGameNumberText(&currKillCountStr[0], myTextRenderer->HudKillCounterNumberRed, irr::core::vector2d<irr::s32>(592, 62));

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
            myTextRenderer->DrawGameText(currentBigGreenText, myTextRenderer->HudBigGreenText, currentBigGreenTextDrawPosition);
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
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    irr::s32 posY = (this->screenResolution.Height - 39);

    /*************
     * State 1   * first/last symbols are seen from message banner
     * **********/

    //Left boundary block symbol
    HudDisplayPart* leftBoundarySymbState1 = new HudDisplayPart();
    leftBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
    leftBoundarySymbState1->altTexture = NULL;
    //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
    leftBoundarySymbState1->drawScrPosition.set(243, posY);

    //Right boundary block symbol
    HudDisplayPart* rightBoundarySymbState1 = new HudDisplayPart();
    rightBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
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
        middleWhiteSymbState1Symb->texture = myDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
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
    leftBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
    leftBoundarySymbState1->altTexture = NULL;
    //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
    leftBoundarySymbState1->drawScrPosition.set(210, posY);

    //Right boundary block symbol
    rightBoundarySymbState1 = new HudDisplayPart();
    rightBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
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
        middleWhiteSymbState1Symb->texture = myDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
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
   leftBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
   leftBoundarySymbState1->altTexture = NULL;
   //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
   leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
   leftBoundarySymbState1->drawScrPosition.set(180, posY);

   //Right boundary block symbol
   rightBoundarySymbState1 = new HudDisplayPart();
   rightBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
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
       middleWhiteSymbState1Symb->texture = myDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
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
  leftBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
  leftBoundarySymbState1->altTexture = NULL;
  //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
  leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
  leftBoundarySymbState1->drawScrPosition.set(151, posY);

  //Right boundary block symbol
  rightBoundarySymbState1 = new HudDisplayPart();
  rightBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
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
      middleWhiteSymbState1Symb->texture = myDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
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
 leftBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
 leftBoundarySymbState1->altTexture = NULL;
 //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
 leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
 leftBoundarySymbState1->drawScrPosition.set(120, posY);

 //Right boundary block symbol
 rightBoundarySymbState1 = new HudDisplayPart();
 rightBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
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
     middleWhiteSymbState1Symb->texture = myDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
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
    leftBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
    leftBoundarySymbState1->altTexture = NULL;
    //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
    leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
    leftBoundarySymbState1->drawScrPosition.set(91, posY);

    //Right boundary block symbol
    rightBoundarySymbState1 = new HudDisplayPart();
    rightBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
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
        middleWhiteSymbState1Symb->texture = myDriver->getTexture("extract/hud1player/panel0-1-0199.bmp");
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
       leftBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0197.bmp");
       leftBoundarySymbState1->altTexture = NULL;
       //myDriver->makeColorKeyTexture(leftBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
       leftBoundarySymbState1->sizeTex = leftBoundarySymbState1->texture->getSize();
       leftBoundarySymbState1->drawScrPosition.set(91, posY);

       //Right boundary block symbol
       rightBoundarySymbState1 = new HudDisplayPart();
       rightBoundarySymbState1->texture = myDriver->getTexture("extract/hud1player/panel0-1-0198.bmp");
       rightBoundarySymbState1->altTexture = NULL;
       //myDriver->makeColorKeyTexture(rightBoundarySymbState1->texture, irr::core::position2d<irr::s32>(0,0));
       rightBoundarySymbState1->sizeTex = rightBoundarySymbState1->texture->getSize();
       rightBoundarySymbState1->drawScrPosition.set(501, posY);

       //symbols needed to be drawn for banner state 7
       bannerTextState7->push_back(leftBoundarySymbState1);
       bannerTextState7->push_back(rightBoundarySymbState1);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

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
                myDriver->draw2DImage(elementsToDraw->at(i)->texture, elementsToDraw->at(i)->drawScrPosition,
                      irr::core::rect<irr::s32>(0,0, elementsToDraw->at(i)->sizeTex.Width, elementsToDraw->at(i)->sizeTex.Height), 0,
                      irr::video::SColor(255,255,255,255), true);

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

    irr::u32 posY = (this->screenResolution.Height - 24);
    irr::u32 posX;
    irr::u32 txtWidth;

    //1. render current lap number in the lowest line
    char text[20];
    sprintf(&text[0], "%2d.", this->monitorWhichPlayer->mPlayerStats->currLapNumber);

    txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
    posX = 38 - txtWidth;

    this->myTextRenderer->DrawGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed, irr::core::position2di(posX, posY));

    //2. render current lap time number in the lowest line
    // this text is written in a way that times are alignment on the right side
    //the length (width) of text to render afterwards
    sprintf(&text[0], "%4d", this->monitorWhichPlayer->mPlayerStats->currLapTimeMultiple100mSec);

    //txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
    //posX = 92 - txtWidth;

    this->myTextRenderer->DrawGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed, irr::core::position2di(56, posY));

    //for the next line decrease Y coordinate
    posY -= 23;

    irr::u8 amountFinishedLaps = (this->monitorWhichPlayer->mPlayerStats->currLapNumber - 1);

    //first lets see if there is already a fastest lap number & lap time available
    //get this info
    irr::u8 fastestLapNr = 0;
    irr::u32 fastestLapNrLapTimeMultiple100ms;

    if (amountFinishedLaps > 0) {
        //because player lap time table is a sorted vector, fastest lap is always first entry
        fastestLapNr = this->monitorWhichPlayer->mPlayerStats->lapTimeList[0].lapNr;
        fastestLapNrLapTimeMultiple100ms = this->monitorWhichPlayer->mPlayerStats->lapTimeList[0].lapTimeMultiple100mSec;
    }

    //if available print information about last lap, also important: only print this lap here as long this is not the fastest lap!
    if ((this->monitorWhichPlayer->mPlayerStats->lastLap.lapNr != 0) && (this->monitorWhichPlayer->mPlayerStats->lastLap.lapNr != fastestLapNr)) {
        sprintf(&text[0], "%2d.", this->monitorWhichPlayer->mPlayerStats->lastLap.lapNr);

        txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
        posX = 38 - txtWidth;

        this->myTextRenderer->DrawGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed, irr::core::position2di(posX, posY));

        sprintf(&text[0], "%4d", this->monitorWhichPlayer->mPlayerStats->lastLap.lapTimeMultiple100mSec);

        //txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
        //posX = 92 - txtWidth;

        this->myTextRenderer->DrawGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed, irr::core::position2di(56, posY));

        //for the next line decrease Y coordinate
        posY -= 23;
    }

    //if available print information about lap before last lap, also important: only print this lap here as long this is not the fastest lap!
    if ((this->monitorWhichPlayer->mPlayerStats->LapBeforeLastLap.lapNr != 0) && (this->monitorWhichPlayer->mPlayerStats->LapBeforeLastLap.lapNr != fastestLapNr)) {
        sprintf(&text[0], "%2d.", this->monitorWhichPlayer->mPlayerStats->LapBeforeLastLap.lapNr);

        txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
        posX = 38 - txtWidth;

        this->myTextRenderer->DrawGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed, irr::core::position2di(posX, posY));

        sprintf(&text[0], "%4d", this->monitorWhichPlayer->mPlayerStats->LapBeforeLastLap.lapTimeMultiple100mSec);

        //txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed);
        //posX = 92 - txtWidth;

        this->myTextRenderer->DrawGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberRed, irr::core::position2di(56, posY));

        //for the next line decrease Y coordinate
        posY -= 23;
    }

    //now finally print currently fastest lap info with grey text
    if (fastestLapNr != 0) {
        sprintf(&text[0], "%2d.", fastestLapNr);

        txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberGrey);
        posX = 38 - txtWidth;

        this->myTextRenderer->DrawGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberGrey, irr::core::position2di(posX, posY));

        sprintf(&text[0], "%4d", fastestLapNrLapTimeMultiple100ms);

        //txtWidth = this->myTextRenderer->GetWidthPixelsGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberGrey);
        //posX = 92 - txtWidth;

        this->myTextRenderer->DrawGameNumberText(&text[0], this->myTextRenderer->HudLaptimeNumberGrey, irr::core::position2di(56, posY));
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
    irr::u32 currentTextWidthPixels = myTextRenderer->GetWidthPixelsGameText(newMsg->text, myTextRenderer->HudWhiteTextBannerFont);

    //calculate text position in a way that the new text is centered in the middle of the banner
    newMsg->textPosition.Y = (this->screenResolution.Height - 39);
    newMsg->textPosition.X = (this->screenResolution.Width / 2) - (currentTextWidthPixels / 2.0);

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
    irr::u32 currentTextWidthPixels = myTextRenderer->GetWidthPixelsGameText(currentBigGreenText, myTextRenderer->HudBigGreenText);
    irr::u32 currentTextHeightPixels = myTextRenderer->GetHeightPixelsGameText(currentBigGreenText, myTextRenderer->HudBigGreenText);

    //calculate text position in a way that the new text is centered in the middle of the player screen
    currentBigGreenTextDrawPosition.Y = (this->screenResolution.Height / 2) - (currentTextHeightPixels / 2.0);
    currentBigGreenTextDrawPosition.X = (this->screenResolution.Width / 2) - (currentTextWidthPixels / 2.0);
}

void HUD::InitTargetStuff() {
    //initialize target other player ships stuff
    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    targetSymbol = new HudDisplayPart();
    //green target symbol
    targetSymbol->texture = myDriver->getTexture("extract/hud1player/panel0-1-0257.bmp");

    //red target symbol
    targetSymbol->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0258.bmp");

    myDriver->makeColorKeyTexture(targetSymbol->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(targetSymbol->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetSymbol->sizeTex = targetSymbol->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetSymbol->drawScrPosition.set(0,0);

    //arrow left of target symbol
    targetArrowLeft = new HudDisplayPart();
    //green arrow left
    targetArrowLeft->texture = myDriver->getTexture("extract/hud1player/panel0-1-0263.bmp");

    //red arrow left
    targetArrowLeft->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0264.bmp");

    myDriver->makeColorKeyTexture(targetArrowLeft->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(targetArrowLeft->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetArrowLeft->sizeTex = targetArrowLeft->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetArrowLeft->drawScrPosition.set(0,0);

    //arrow right of target symbol
    targetArrowRight = new HudDisplayPart();
    //green arrow right
    targetArrowRight->texture = myDriver->getTexture("extract/hud1player/panel0-1-0265.bmp");

    //red arrow right
    targetArrowRight->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0266.bmp");

    myDriver->makeColorKeyTexture(targetArrowRight->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(targetArrowRight->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetArrowRight->sizeTex = targetArrowRight->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetArrowRight->drawScrPosition.set(0,0);

    //arrow above of target symbol
    targetArrowAbove = new HudDisplayPart();
    //green arrow above
    targetArrowAbove->texture = myDriver->getTexture("extract/hud1player/panel0-1-0259.bmp");

    //red arrow above
    targetArrowAbove->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0260.bmp");

    myDriver->makeColorKeyTexture(targetArrowAbove->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(targetArrowAbove->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetArrowAbove->sizeTex = targetArrowAbove->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetArrowAbove->drawScrPosition.set(0,0);

    //arrow below of target symbol
    targetArrowBelow = new HudDisplayPart();
    //green arrow below
    targetArrowBelow->texture = myDriver->getTexture("extract/hud1player/panel0-1-0261.bmp");

    //red arrow below
    targetArrowBelow->altTexture = myDriver->getTexture("extract/hud1player/panel0-1-0262.bmp");

    myDriver->makeColorKeyTexture(targetArrowBelow->texture, irr::core::position2d<irr::s32>(0,0));
    myDriver->makeColorKeyTexture(targetArrowBelow->altTexture, irr::core::position2d<irr::s32>(0,0));

    targetArrowBelow->sizeTex = targetArrowBelow->texture->getSize();

    //just set the target position to 0,0; we do not really need this variable
    //here in this struct
    targetArrowBelow->drawScrPosition.set(0,0);

    myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
}

//Just taken from Irrlicht source code ISceneCollisionManager
//And then modified
//! Calculates 2d screen position from a 3d position.
core::position2d<s32> HUD::getScreenCoordinatesFrom3DPosition(
    const core::vector3df & pos3d, ICameraSceneNode* camera)
{

    irr::u32 Width = this->screenResolution.Width;
    irr::u32 Height = this->screenResolution.Height;

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
          ICameraSceneNode* actCamera = this->monitorWhichPlayer->mRace->mSmgr->getActiveCamera();

          irr::core::vector3df targetPlayerPos = monitorWhichPlayer->mTargetPlayer->phobj->physicState.position;

          irr::core::vector2di targetPos = getScreenCoordinatesFrom3DPosition(targetPlayerPos, actCamera);

          irr::u32 targetSymBHalfWidth = targetSymbol->sizeTex.Width / 2;
          irr::u32 targetSymBHalfHeight = targetSymbol->sizeTex.Height / 2;

          targetSymbol->drawScrPosition.X = targetPos.X - targetSymBHalfWidth;
          targetSymbol->drawScrPosition.Y = targetPos.Y - targetSymBHalfHeight;

          irr::u32 lockProgress = this->monitorWhichPlayer->mTargetMissleLockProgr;

          targetArrowLeft->drawScrPosition.X = targetPos.X - targetSymBHalfWidth - lockProgress - targetArrowLeft->sizeTex.Width;
          targetArrowLeft->drawScrPosition.Y = targetPos.Y - targetArrowLeft->sizeTex.Height / 2.0;

          targetArrowRight->drawScrPosition.X = targetPos.X + targetSymBHalfWidth + lockProgress;
          targetArrowRight->drawScrPosition.Y = targetArrowLeft->drawScrPosition.Y;

          targetArrowAbove->drawScrPosition.X = targetPos.X - targetArrowAbove->sizeTex.Width / 2.0;
          targetArrowAbove->drawScrPosition.Y = targetPos.Y - targetSymBHalfHeight - lockProgress  - targetArrowAbove->sizeTex.Height;

          targetArrowBelow->drawScrPosition.X = targetArrowAbove->drawScrPosition.X;
          targetArrowBelow->drawScrPosition.Y = targetPos.Y + targetSymBHalfHeight + lockProgress;

          if (!monitorWhichPlayer->mTargetMissleLock) {
             //no missle lock, green symbol and green text
             myDriver->draw2DImage(targetSymbol->texture, targetSymbol->drawScrPosition,
                irr::core::rect<irr::s32>(0,0, targetSymbol->sizeTex.Width, targetSymbol->sizeTex.Height), 0,
                irr::video::SColor(255,255,255,255), true);

               if (currShowTargetName) {
                    //write player name next to target symbol
                    this->myTextRenderer->DrawHudSmallText(monitorWhichPlayer->mTargetPlayer->mPlayerStats->name,
                        this->myTextRenderer->HudTargetNameGreen,
                            irr::core::position2di(targetPos.X + targetSymBHalfWidth + 2,
                                            targetSymbol->drawScrPosition.Y));
               }

             //left green arrow
             myDriver->draw2DImage(targetArrowLeft->texture, targetArrowLeft->drawScrPosition,
                irr::core::rect<irr::s32>(0,0, targetArrowLeft->sizeTex.Width, targetArrowLeft->sizeTex.Height), 0,
                irr::video::SColor(255,255,255,255), true);

             //right green arrow
             myDriver->draw2DImage(targetArrowRight->texture, targetArrowRight->drawScrPosition,
                irr::core::rect<irr::s32>(0,0, targetArrowRight->sizeTex.Width, targetArrowRight->sizeTex.Height), 0,
                irr::video::SColor(255,255,255,255), true);

             //above green arrow
             myDriver->draw2DImage(targetArrowAbove->texture, targetArrowAbove->drawScrPosition,
                irr::core::rect<irr::s32>(0,0, targetArrowAbove->sizeTex.Width, targetArrowAbove->sizeTex.Height), 0,
                irr::video::SColor(255,255,255,255), true);

             //below green arrow
             myDriver->draw2DImage(targetArrowBelow->texture, targetArrowBelow->drawScrPosition,
                irr::core::rect<irr::s32>(0,0, targetArrowBelow->sizeTex.Width, targetArrowBelow->sizeTex.Height), 0,
                irr::video::SColor(255,255,255,255), true);
          } else {
              //we also have missile lock, red symbol and red text
              myDriver->draw2DImage(targetSymbol->altTexture, targetSymbol->drawScrPosition,
                 irr::core::rect<irr::s32>(0,0, targetSymbol->sizeTex.Width, targetSymbol->sizeTex.Height), 0,
                 irr::video::SColor(255,255,255,255), true);

              if (currShowTargetName) {
                //write player name next to target symbol
                this->myTextRenderer->DrawHudSmallText(monitorWhichPlayer->mTargetPlayer->mPlayerStats->name,
                        this->myTextRenderer->HudTargetNameRed,
                        irr::core::position2di(targetPos.X + targetSymBHalfWidth + 2,
                                             targetSymbol->drawScrPosition.Y));
              }

              //left red arrow
              myDriver->draw2DImage(targetArrowLeft->altTexture, targetArrowLeft->drawScrPosition,
                 irr::core::rect<irr::s32>(0,0, targetArrowLeft->sizeTex.Width, targetArrowLeft->sizeTex.Height), 0,
                 irr::video::SColor(255,255,255,255), true);

              //right red arrow
              myDriver->draw2DImage(targetArrowRight->altTexture, targetArrowRight->drawScrPosition,
                 irr::core::rect<irr::s32>(0,0, targetArrowRight->sizeTex.Width, targetArrowRight->sizeTex.Height), 0,
                 irr::video::SColor(255,255,255,255), true);

              //above red arrow
              myDriver->draw2DImage(targetArrowAbove->altTexture, targetArrowAbove->drawScrPosition,
                 irr::core::rect<irr::s32>(0,0, targetArrowAbove->sizeTex.Width, targetArrowAbove->sizeTex.Height), 0,
                 irr::video::SColor(255,255,255,255), true);

              //below red arrow
              myDriver->draw2DImage(targetArrowBelow->altTexture, targetArrowBelow->drawScrPosition,
                 irr::core::rect<irr::s32>(0,0, targetArrowBelow->sizeTex.Width, targetArrowBelow->sizeTex.Height), 0,
                 irr::video::SColor(255,255,255,255), true);
          }
     }
}

HUD::HUD(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver,  dimension2d<u32> screenRes, GameText* TextRenderer) {
    myDriver = driver;
    myDevice = device;
    myTextRenderer = TextRenderer;
    screenResolution = screenRes;

    monitorWhichPlayer = NULL;

    InitShieldBar();
    InitAmmoBar();
    InitGasolineBar();
    InitThrottleBar();
    InitSpeedBar();
    InitMGHeatBar();
    InitRacePosition();
    //InitRedTextLapNumber();
    //InitRedTextKillCounter();
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
               this->myDriver->removeTexture(pntr->texture);
           }

           if (pntr->altTexture != NULL) {
               //remove underlying texture
               this->myDriver->removeTexture(pntr->altTexture);
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
        this->myDriver->removeTexture(brokenGlas->texture);
    }

    if (brokenGlas->altTexture != NULL) {
        //remove underlying texture
        this->myDriver->removeTexture(brokenGlas->altTexture);
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

            myTextRenderer->DrawGameNumberText(posStr, myTextRenderer->HudLaptimeNumberGrey,
                                               irr::core::vector2d<irr::s32>(posXNr, posY));

            myTextRenderer->DrawHudSmallText(
                        this->monitorWhichPlayer->mRace->playerRaceFinishedVec.at(position - 1)->mPlayerStats->name,
                        this->myTextRenderer->HudTargetNameRed,  irr::core::vector2d<irr::s32>(posXPlayerName, posYPlayerName));

            posY += 24;
            posYPlayerName += 24;
        }
    }
}

