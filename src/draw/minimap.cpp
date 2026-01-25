/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "../infrabase.h"
#include "minimap.h"
#include "../utils/fileutils.h"
#include "../utils/logging.h"

MiniMap::MiniMap(InfrastructureBase* infra, irr::core::vector2di lowerRightCorner,
                 MapConfigStruct* mapConfig, std::string levelRootDir) {
   mInfra = infra;
   mMapConfig = mapConfig;
   mLowerRightCorner = lowerRightCorner;
   mLevelRootDir = levelRootDir;

   //predefine mini map marker colors
   //for max 8 players
   mMiniMapMarkerColors.clear();
   mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 254, 254, 250)); //player 1 marker color (human player)
   mMiniMapMarkerColors.push_back(new irr::video::SColor(255,  70, 121,  99)); //player 2 marker color
   mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 114,  53,  60)); //player 3 marker color
   mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 235, 112,  46)); //player 4 marker color
   mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 251, 220,  56)); //player 5 marker color
   mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 182,  32, 130)); //player 6 marker color
   mMiniMapMarkerColors.push_back(new irr::video::SColor(255,  57,  77, 145)); //player 7 marker color
   mMiniMapMarkerColors.push_back(new irr::video::SColor(255,   4,   6,   8)); //player 8 marker color

   //use the original game supplied minimap
   //drawings
   mMiniMapInitOk = InitMiniMap();
}

MiniMap::~MiniMap() {
    CleanMiniMap();
}

bool MiniMap::GetInitOk() {
    return mMiniMapInitOk;
}

void MiniMap::CleanMiniMap() {
    if (baseMiniMap != nullptr) {
        //free this texture
        mInfra->mDriver->removeTexture(baseMiniMap);
        baseMiniMap = nullptr;
    }

    //also clean up all minimap marker colors
    std::vector<irr::video::SColor*>::iterator itColor;
    irr::video::SColor* pntrColor;

    for (itColor = this->mMiniMapMarkerColors.begin(); itColor != this->mMiniMapMarkerColors.end(); ) {
        pntrColor = (*itColor);

        itColor = mMiniMapMarkerColors.erase(itColor);

        delete pntrColor;
    }
}

void MiniMap::DrawMiniMap(irr::f32 frameDeltaTime, std::vector<irr::core::vector2di> coordVec,
                          bool player0IsHuman) {
    //if minimap initialization was not ok do not attempt
    //to draw it
    if (!mMiniMapInitOk)
        return;

    mInfra->mDriver->draw2DImage(baseMiniMap, miniMapDrawLocation,
             miniMapImageUsedArea, 0,
             irr::video::SColor(255,255,255,255), true);

    //we want to blink the human player location
    //marker
    miniMapAbsTime += frameDeltaTime * 1000.0f;
    if (miniMapAbsTime > 500.0f) {
        miniMapAbsTime = 0.0f;
        miniMapBlinkActive = !miniMapBlinkActive;
    }

    /**********************************
     * Draw MiniMap for all players   *
     * ******************************** */

    int playerIdx = 0;
    irr::core::dimension2di playerLocation;
    std::vector<irr::core::vector2di>::iterator itPlayer;

    for (itPlayer = coordVec.begin(); itPlayer != coordVec.end(); ++itPlayer) {
        //draw player position
        playerLocation = CalcPlayerMiniMapPosition(*itPlayer);

        //player 0 is the human player if one is in the race
        if (playerIdx == 0) {
            if (player0IsHuman) {
                //player 0 is human player, draw blinking cursor
                irr::video::SColor player1LocationFrameColor(255, 194, 189, 206);  //this color is drawn around the main white block to create a blinking effect

                //for blinking effect draw bigger frame block for player 1
                //only draw it for blinking effect
                if (miniMapBlinkActive) {
                    mInfra->mDriver->draw2DRectangle(player1LocationFrameColor,
                                         core::rect<s32>(playerLocation.Width - 5, playerLocation.Height -5,
                                                         playerLocation.Width + 5, playerLocation.Height + 5));
                }
            }
        }

        //draw the default marker for all available players
        mInfra->mDriver->draw2DRectangle(*mMiniMapMarkerColors.at(playerIdx),
                                 core::rect<s32>(playerLocation.Width - 3, playerLocation.Height - 3,
                                                 playerLocation.Width + 3, playerLocation.Height + 3));

        playerIdx++;
    }
}

irr::core::dimension2di MiniMap::CalcPlayerMiniMapPosition(irr::core::vector2di inputCoord) {
    irr::core::dimension2di miniMapLocation;

    irr::f32 posPlWidth = miniMapPixelPerCellW * (irr::f32)(inputCoord.X - mMapConfig->minimapCalStartVal.X);
    irr::f32 posPlHeight = miniMapPixelPerCellH * (irr::f32)(inputCoord.Y - mMapConfig->minimapCalStartVal.Y);

    miniMapLocation.Width = miniMapDrawLocation.X + (irr::s32)(posPlWidth);
    miniMapLocation.Height = miniMapDrawLocation.Y + (irr::s32)(posPlHeight);

    return miniMapLocation;
}

//Initializes the games original
//minimap
//returns true for success, false otherwise
bool MiniMap::InitMiniMap() {
    //first define correct filename to load image
    //of minimap
    std::string miniMapPicFile("");
    miniMapPicFile.append(mLevelRootDir.c_str());
    miniMapPicFile.append("minimap.png");

    if (FileExists(miniMapPicFile.c_str()) != 1) {
        //minimap image file does not exist!
        std::string warningMsg("InitMiniMap: Can not find minimap file '");
        warningMsg.append(miniMapPicFile);
        warningMsg.append("'! Will disable minimap");

        logging::Warning(warningMsg);
        return false;
    }

    //Minimap "calibration" values not set in mapConfig Xml
    //file, can not use minimap as well
    if (!mMapConfig->minimapCalSet) {
        logging::Warning("Map configuration Xml file does not supply minimap calibration values, Will disable minimap");
        return false;
    }

    //we need to establish the parameters for the level size
    //this is where we can reuse this function, even though we do not
    //need the autogenerated picture at the end
    //irr::video::IImage* baseMiniMapPic = mLevelTerrain->CreateMiniMapInfo(miniMapStartW, miniMapEndW, miniMapStartH, miniMapEndH);

    /*std::string miniMapCalFilename("");
    miniMapCalFilename.append(mLevelRootPath);
    miniMapCalFilename.append("minimapcalval.dat");

    if (!mGame->ReadMiniMapCalFile(miniMapCalFilename, miniMapStartW, miniMapEndW, miniMapStartH, miniMapEndH)) {
        return false;
    }*/

    //just drop picture again, we do not need it
    //baseMiniMapPic->drop();

    //load original games mini map
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    baseMiniMap = mInfra->mDriver->getTexture(miniMapPicFile.c_str());

    if (baseMiniMap == nullptr) {
        return false;
    }

    //for transparency
    mInfra->mDriver->makeColorKeyTexture(baseMiniMap, irr::core::position2d<irr::s32>(0,0));

    bool successUsedArea;
    miniMapImageUsedArea = FindMiniMapImageUsedArea(baseMiniMap, successUsedArea);

    if (!successUsedArea) {
        //something was wrong in FindMiniMapImageUsedArea
        logging::Error("Unexpected error in FindMiniMapImageUsedArea!");
        return false;
    }

    //miniMapSize = baseMiniMap->getSize();
    //miniMapDrawLocation.X = this->mGameScreenRes.Width - miniMapSize.Width;
    //miniMapDrawLocation.Y = this->mGameScreenRes.Height - miniMapSize.Height;

    miniMapSize.Width = miniMapImageUsedArea.getWidth();
    miniMapSize.Height = miniMapImageUsedArea.getHeight();

    miniMapDrawLocation.X = mLowerRightCorner.X - miniMapSize.Width;
    miniMapDrawLocation.Y = mLowerRightCorner.Y - miniMapSize.Height;

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    //do some precalculations so that we have to repeat them not unnecessary all the time
    //during the game
    //miniMapPixelPerCellW = (irr::f32)(miniMapSize.Width) / ((irr::f32)(miniMapEndW) - (irr::f32)(miniMapStartW));
    //miniMapPixelPerCellH = (irr::f32)(miniMapSize.Height) / ((irr::f32)(miniMapEndH) - (irr::f32)(miniMapStartH));

    miniMapPixelPerCellW =
            (irr::f32)(miniMapSize.Width) / ((irr::f32)(mMapConfig->minimapCalEndVal.X) - (irr::f32)(mMapConfig->minimapCalStartVal.X));
    miniMapPixelPerCellH =
            (irr::f32)(miniMapSize.Height) / ((irr::f32)(mMapConfig->minimapCalEndVal.Y) - (irr::f32)(mMapConfig->minimapCalStartVal.Y));

    //we were successfull
    return true;
}

//Searches for the used space inside the minimap picture
//while removing unnecessary transparent columns of pixels
//Parameters:
//  miniMapTexture = pointer to the minimap texture
//In case of an unexpected error this function returns succesFlag = false, True otherwise
irr::core::rect<irr::s32> MiniMap::FindMiniMapImageUsedArea(irr::video::ITexture* miniMapTexture, bool &succesFlag) {
 bool wholeColumnTrans;
 bool wholeLineTrans;
 irr::s32 firstUsedColumn = -1;
 irr::s32 lastUsedColumn = -1;
 irr::s32 firstUsedLine = -1;
 irr::s32 lastUsedLine = -1;

 //we need to know the used pixel color format
  irr::video::ECOLOR_FORMAT format = miniMapTexture->getColorFormat();

  //we can only handle this format right now
  if(irr::video::ECF_A8R8G8B8 == format)
    {
        //lock texture for just reading of pixel data
        irr::u8* datapntr = (irr::u8*)miniMapTexture->lock(irr::video::ETLM_READ_ONLY);
        irr::u32 pitch = miniMapTexture->getPitch();

        //all minimaps should have the transparent color at the upper leftmost pixel (0,0)
        //get this value, as a reference where there is no pixel of the actual minimap itself
        irr::video::SColor* texelTrans = (irr::video::SColor *)(datapntr + ((0 * pitch) + (0 * sizeof(irr::video::SColor))));

        irr::s32 texWidth = miniMapTexture->getSize().Width;
        irr::s32 texHeight = miniMapTexture->getSize().Height;

        //iterate through all colums from left to right
        for (irr::s32 x = 0; x < texWidth; x++) {
            //check current column from top to bottom to see if we only find
            //black (unused pixels)
            wholeColumnTrans = true;

            for (irr::s32 y = 0; y < texHeight; y++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeColumnTrans = false;
                }
            }

            if (!wholeColumnTrans) {
                if (firstUsedColumn == -1)
                    firstUsedColumn = x;
                break;
            }
        }

        //iterate through all colums from right to left
        for (irr::s32 x = (texWidth - 1); x > -1 ; x--) {
            //check current column from top to bottom to see if we only find
            //black (unused pixels)
            wholeColumnTrans = true;

            for (irr::s32 y = 0; y < texHeight; y++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeColumnTrans = false;
                }
            }

            if (!wholeColumnTrans) {
                if (lastUsedColumn == -1)
                    lastUsedColumn = x;
                break;
            }
        }

        //iterate through all lines from top to bottom
        for (irr::s32 y = 0; y < texHeight; y++) {
            //check current lines from left to right to see if we only find
            //black (unused pixels)
            wholeLineTrans = true;

            for (irr::s32 x = 0; x < texWidth; x++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeLineTrans = false;
                }
            }

            if (!wholeLineTrans) {
                if (firstUsedLine == -1)
                    firstUsedLine = y;
                break;
            }
        }

        //iterate through all lines from bottom to top
        for (irr::s32 y = (texHeight - 1); y > -1 ; y--) {
            //check current line from left to right to see if we only find
            //black (unused pixels)
            wholeLineTrans = true;

            for (irr::s32 x = 0; x < texWidth; x++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeLineTrans = false;
                }
            }

            if (!wholeLineTrans) {
                if (lastUsedLine == -1)
                    lastUsedLine = y;
                break;
            }
        }

        //unlock texture again!
        miniMapTexture->unlock();

       //if possible leave one fully transparent column left of the minimap texture data
       if (firstUsedColumn > 0)
           firstUsedColumn--;

       //if no minimap image was found at all return "full" empty minimap image
       if (firstUsedColumn == -1)
           firstUsedColumn = 0;

       if (lastUsedColumn == -1)
           lastUsedColumn = texWidth;

         //if possible leave one fully transparent column right of the minimap image
       if (lastUsedColumn <  texWidth)
           lastUsedColumn++;

       //if possible leave one fully transparent column left of the minimap texture data
       if (firstUsedLine > 0)
           firstUsedLine--;

       //if no minimap image was found at all return "full" empty minimap image
       if (firstUsedLine == -1)
           firstUsedLine = 0;

       if (lastUsedLine == -1)
           lastUsedLine = texHeight;

         //if possible leave one fully transparent column right of the minimap image
       if (lastUsedLine <  texHeight)
           lastUsedLine++;

        //return optimized size of minimap
        irr::core::rect<irr::s32> result(firstUsedColumn, firstUsedLine, lastUsedColumn, lastUsedLine);
        succesFlag = true;
        return result;
  } else {
      //unsupported pixel color format!
      //just return full minimap image, and set successFlag to False
      irr::s32 texWidth = miniMapTexture->getSize().Width;
      irr::s32 texHeight = miniMapTexture->getSize().Height;

      irr::core::rect<irr::s32> result(0, 0, texWidth - 1, texHeight - 1);
      succesFlag = false;
      return result;
  }
}

//returns true in case of success, False otherwise
/*bool Race::InitMiniMap(irr::u32 levelNr) {
    //we need to write base minimap image to disk so that we can
    //load it as texture again. As stupid as this sounds
    char outputFilename[50];
    strcpy(outputFilename, "extract/");

    char dirName[30];

    sprintf (dirName, "level0-%d", levelNr);
    strcat(outputFilename, dirName);
    strcat(outputFilename, "/baseminimap.bmp");

    //see if the auto generated minimap is already present
    //if so we do not need to rewrite it again

    //create minimap from level data
    //is not perfect, but better than nothing
    irr::video::IImage* baseMiniMapPic =
        mLevelTerrain->CreateMiniMapInfo(miniMapStartW, miniMapEndW, miniMapStartH, miniMapEndH);

    //file does not exist?
    if (FileExists(outputFilename) != 1) {
       //does not exist yet, create it

        //create new file for writting
        irr::io::IWriteFile* outputPic = mGame->mDevice->getFileSystem()->createAndWriteFile(outputFilename, false);

        mGame->mDriver->writeImageToFile(baseMiniMapPic, outputPic);

        //close output file
        outputPic->drop();
    }

    //drop the autogenerated minimap
    //we do not need it anymore
    baseMiniMapPic->drop();

    //minimap does exist already
    //just load it

    mGame->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    baseMiniMap = mGame->mDriver->getTexture(outputFilename);

    if (baseMiniMap == nullptr) {
        return false;
    }

    //for transparency
    mGame->mDriver->makeColorKeyTexture(baseMiniMap, irr::core::position2d<irr::s32>(0,0));

    bool successUsedArea;
    miniMapImageUsedArea = FindMiniMapImageUsedArea(baseMiniMap, successUsedArea);

    if (!successUsedArea) {
        //something was wrong in FindMiniMapImageUsedArea
        logging::Error("Unexpected error in FindMiniMapImageUsedArea!");
        return false;
    }

    //miniMapSize = baseMiniMap->getSize();
    miniMapSize.Width = miniMapImageUsedArea.getWidth();
    miniMapSize.Height = miniMapImageUsedArea.getHeight();

    miniMapDrawLocation.X = mGame->mScreenRes.Width - miniMapSize.Width;
    miniMapDrawLocation.Y = mGame->mScreenRes.Height - miniMapSize.Height;

    mGame->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    //do some precalculations so that we have to repeat them not unnecessary all the time
    //during the game
    miniMapPixelPerCellW = (irr::f32)(miniMapSize.Width) / ((irr::f32)(miniMapEndW) - (irr::f32)(miniMapStartW));
    miniMapPixelPerCellH = (irr::f32)(miniMapSize.Height) / ((irr::f32)(miniMapEndH) - (irr::f32)(miniMapStartH));

    //minimap init was succesfull
    return true;
}*/




