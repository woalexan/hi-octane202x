/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "attribution.h"
#include <fstream>
#include "../utils/fileutils.h"
#include "../utils/logging.h"
#include "../infrabase.h"

Attribution::Attribution(InfrastructureBase* infra) {
    mInfra = infra;
    mScreenRes = mInfra->mScreenRes;
    mWidthRender = 800;

    mTextLineCnt = 0;

    mAttrData.clear();
}

Attribution::~Attribution() {
}

void Attribution::Init() {
    mSuccessText = ReadAttributionInfo();

    if (mSuccessText) {
        mTextLineCnt = mAttrData.size();
    }

    mSuccessImages = LoadResourceImages();
}

//Returns nullptr in case of a texture loading problem
irr::video::ITexture* Attribution::LoadResourceImage(std::string filename, irr::s32& texWidth, irr::s32& texHeight) {
    bool problem = true;

    irr::video::ITexture* texPntr;

    if (FileExists(filename.c_str()) == 1) {
       //file exists, open it
       texPntr = mInfra->mDriver->getTexture(filename.c_str());

       if (texPntr != nullptr) {
            //make image transparent, take color at pixel coord 0,0 for transparency color
            mInfra->mDriver->makeColorKeyTexture(texPntr, irr::core::position2d<irr::s32>(0,0));

            texWidth = texPntr->getSize().Width;
            texHeight = texPntr->getSize().Height;

            problem = false;
       }
    }

    if (problem) {
        //Problem while opening file
        std::string logWarning("Can not read file '");
        logWarning.append(filename.c_str());
        logWarning.append("'");
        logging::Warning(logWarning.c_str());
        return nullptr;
    }

    //success, return pointer to loaded
    //texture
    return texPntr;
}

//Return true in case of success, False otherwise
bool Attribution::LoadResourceImages() {
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    std::string texFile1("extract/hud1player/panel0-1-0197.bmp");
    std::string texFile2("extract/hud1player/panel0-1-0199.bmp");
    std::string texFile3("extract/hud1player/panel0-1-0198.bmp");
    std::string texFileIrrlichtLogo("media/irrlichtlogo.bmp");
    std::string texFileSFMLLogo("media/sfml-logo-small.png");

    texSeperatorLine1 = LoadResourceImage(texFile1, texSeperatorLine1Width, texSeperatorLineHeight);

    if (texSeperatorLine1 == nullptr) {
        //There was an issue loading this texture
        mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

        return false;
    }

    texSeperatorLine2 = LoadResourceImage(texFile2, texSeperatorLine2Width, texSeperatorLineHeight);

    if (texSeperatorLine2 == nullptr) {
        //There was an issue loading this texture
        mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

        return false;
    }

    texSeperatorLine3 = LoadResourceImage(texFile3, texSeperatorLine3Width, texSeperatorLineHeight);

    if (texSeperatorLine3 == nullptr) {
        //There was an issue loading this texture
        mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

        return false;
    }

    texIrrlichtLogo = LoadResourceImage(texFileIrrlichtLogo, texIrrlichtLogoWidth, texIrrlichtLogoHeight);

    if (texIrrlichtLogo == nullptr) {
        //There was an issue loading this texture
        mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

        return false;
    }

    texSFMLLogo = LoadResourceImage(texFileSFMLLogo, texSFMLLogoWidth, texSFMLLogoHeight);

    if (texSFMLLogo == nullptr) {
        //There was an issue loading this texture
        mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

        return false;
    }

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}

void Attribution::AddTextElement(irr::core::rect<irr::s32> newPosition) {
    if (mNextTextLineNr >= mTextLineCnt) {
        //we are done, no more text lines available
        //simply return
        return;
    }

    if (!(mAttrData.at(mNextTextLineNr).find(L"<lineseperator>") == std::string::npos)) {
        //we did find string "lineseperator" in the next text line
        //add the image to seperate lines instead
        AddLineSeperator(newPosition);

        mNextTextLineNr++;
        return;
    }

    //add the next text line
    irr::gui::IGUIStaticText* nextStaticText = mInfra->mGuienv->addStaticText(mAttrData.at(mNextTextLineNr).c_str(),
                                          newPosition, false, true, nullptr, -1, true);

    nextStaticText->setOverrideColor(irr::video::SColor(255, 255, 255, 255));
    nextStaticText->setBackgroundColor(irr::video::SColor(230, 0, 0, 0));

    mTextElementVec.push_back(nextStaticText);

    mNextTextLineNr++;
}

void Attribution::AddLineSeperator(irr::core::rect<irr::s32> newPosition) {
    //add multiple images to form a line seperator
    irr::core::vector2d<irr::s32> currPos = newPosition.UpperLeftCorner;

    irr::gui::IGUIImage* nextImage = mInfra->mGuienv->addImage(texSeperatorLine1, currPos, true,
                                                               0, -1, L"");

    mImageElementVec.push_back(nextImage);

    currPos.X += texSeperatorLine1Width;

    irr::s32 remPixel = mWidthRender - texSeperatorLine1Width - texSeperatorLine3Width;

    irr::u8 cntNr = remPixel / texSeperatorLine2Width;

    for (irr::u8 cnt = 0; cnt < cntNr; cnt++) {
        irr::gui::IGUIImage* nextImage2 = mInfra->mGuienv->addImage(texSeperatorLine2, currPos, true,
                                                               0, -1, L"");

        mImageElementVec.push_back(nextImage2);

        currPos.X += texSeperatorLine2Width;
    }

    irr::gui::IGUIImage* nextImage3 = mInfra->mGuienv->addImage(texSeperatorLine3, currPos, true,
                                                               0, -1, L"");

    mImageElementVec.push_back(nextImage3);

    mWaitForLineSeperatorEnd = true;
}

void Attribution::Start() {
    if ((mSuccessText == false) || (mSuccessImages == false))
        return;

    mScrolling = true;

    mAbsTime = 0.0f;

    mTextElementVec.clear();
    mImageElementVec.clear();

    mNextTextLineNr = 0;

    mScrollStep.set(0,-1);

    irr::s32 midX = mScreenRes.Width / 2;

    AddTextElement(irr::core::rect<irr::s32>(midX - mWidthRender / 2, mScreenRes.Height + 10, midX + mWidthRender / 2, mScreenRes.Height + 32));

    irr::s32 logoWidth = 120;
    irr::s32 logoHeight = 50;

    irr::core::rect<irr::s32> IrrlichtLogoPos(midX - mWidthRender / 2 - 20 - logoWidth,
                             mScreenRes.Height - 20 - logoHeight, midX - mWidthRender / 2 - 20,
                              mScreenRes.Height - 20);

    IrrlichtLogo = mInfra->mGuienv->addImage(IrrlichtLogoPos, 0, -1, L"", true);
    IrrlichtLogo->setImage(texIrrlichtLogo);
    IrrlichtLogo->setScaleImage(true);

    irr::core::rect<irr::s32> SFMLLogoPos(midX + mWidthRender / 2 + 20,
                             mScreenRes.Height - 20 - logoHeight, midX + mWidthRender / 2 + 20 + logoWidth,
                              mScreenRes.Height - 20);

    SFMLLogo = mInfra->mGuienv->addImage(SFMLLogoPos, 0, -1, L"", true);
    SFMLLogo->setImage(texSFMLLogo);
    SFMLLogo->setScaleImage(true);
}

void Attribution::CleanupItems() {
    if (IrrlichtLogo != nullptr) {
        IrrlichtLogo->remove();
    }

    if (SFMLLogo != nullptr) {
        SFMLLogo->remove();
    }

    std::vector<irr::gui::IGUIStaticText*>::iterator it;
    irr::gui::IGUIStaticText* txtPntr;

    for (it = mTextElementVec.begin(); it != mTextElementVec.end(); ) {
        txtPntr = (*it);

        it = mTextElementVec.erase(it);
        txtPntr->remove();
    }

    std::vector<irr::gui::IGUIImage*>::iterator it2;
    irr::gui::IGUIImage* imagePntr;

    for (it2 = mImageElementVec.begin(); it2 != mImageElementVec.end(); ) {
        imagePntr = (*it2);

        it2 = mImageElementVec.erase(it2);
        imagePntr->remove();
    }
}

void Attribution::Stop() {
    mScrolling = false;

    CleanupItems();
}

void Attribution::ControlTextFading(irr::gui::IGUIStaticText* whichTextElement) {
    irr::core::rect<irr::s32> currPos;

    irr::f32 alpaValText;
    irr::f32 alpaValBackground;

    irr::s32 fadeBarWidthPixels = 100;
    irr::f32 fadingFactor = 1.0f;
    irr::f32 minFadingFactor = 0.0f;

    currPos = whichTextElement->getAbsolutePosition();

    irr::s32 fadeBorder = (irr::s32)(mScreenRes.Height) - fadeBarWidthPixels;

    //are we in the bar at the bottom of the screen?
    if (currPos.UpperLeftCorner.Y >= fadeBorder) {
        //when text scrolls upwards from the bottom of the screen, it starts faded out with minFadingFactor,
        //and then fades in until it hits the upper end of the bar with height fadeBarWidthPixels pixels
        irr::f32 kFade = ((1.0f - minFadingFactor) / ((irr::f32)(fadeBarWidthPixels)));
        irr::f32 madeDistance = (irr::f32)(mScreenRes.Height - currPos.UpperLeftCorner.Y);

        fadingFactor = minFadingFactor + madeDistance * kFade;
    }

    fadeBorder = (irr::s32)(fadeBarWidthPixels);

    if (currPos.UpperLeftCorner.Y < 0) {
        fadingFactor = minFadingFactor;
    } else

    //are we in the bar at the top of the screen?
    if (currPos.UpperLeftCorner.Y <= fadeBorder) {
        //when text scrolls upwards at the top of the screen, it starts faded in completely,
        //and then fades out until it hits minFadingFactor at the top of the screen
        irr::f32 kFade = -((1.0f - minFadingFactor) / ((irr::f32)(fadeBarWidthPixels)));
        irr::f32 madeDistance = (irr::f32)(fadeBarWidthPixels - currPos.UpperLeftCorner.Y);

        fadingFactor = 1.0f + madeDistance * kFade;
    }

    alpaValText = 255.0f * fadingFactor;
    alpaValBackground = 230.0f * fadingFactor;

    whichTextElement->setOverrideColor(irr::video::SColor((irr::u32)(alpaValText), 255, 255, 255));
    whichTextElement->setBackgroundColor(irr::video::SColor((irr::u32)(alpaValBackground), 0, 0, 0));
}

void Attribution::UpdateTexts() {
    std::vector<irr::gui::IGUIStaticText*>::iterator it;
    core::rect<s32> mCurrRect;

    irr::gui::IGUIStaticText* txtPntr;

    for (it = mTextElementVec.begin(); it != mTextElementVec.end(); ++it) {
        (*it)->move(mScrollStep);

        ControlTextFading(*it);
    }

    for (it = mTextElementVec.begin(); it != mTextElementVec.end(); ) {
        mCurrRect = (*it)->getAbsolutePosition();
        if (mCurrRect.LowerRightCorner.Y < 0) {
            //text element is completely invisible above the top
            //of the screen; Remove this text element again
            txtPntr = (*it);

            it = mTextElementVec.erase(it);
            txtPntr->remove();
        } else {
            //Advance to the next text element
            ++it;
        }
    }

    if (!mWaitForLineSeperatorEnd) {
            //check current rect (position) of last text element, is it time to
            //add a new text element below?
            if (mCurrRect.UpperLeftCorner.Y < (irr::s32)(mScreenRes.Height)) {
                //time for a new text element
                irr::core::rect<irr::s32> nextPos = mCurrRect;
                nextPos.UpperLeftCorner.Y += mCurrRect.getHeight();
                nextPos.LowerRightCorner.Y += mCurrRect.getHeight();

                AddTextElement(nextPos);
            }
    }
}

void Attribution::UpdateImages() {
    std::vector<irr::gui::IGUIImage*>::iterator it;
    core::rect<s32> mCurrRect;

    irr::gui::IGUIImage* imagePntr;

    for (it = mImageElementVec.begin(); it != mImageElementVec.end(); ++it) {
        (*it)->move(mScrollStep);
    }

    for (it = mImageElementVec.begin(); it != mImageElementVec.end(); ) {
        mCurrRect = (*it)->getAbsolutePosition();
        if (mCurrRect.LowerRightCorner.Y < 0) {
            //image element is completely invisible above the top
            //of the screen; Remove this text element again
            imagePntr = (*it);

            it = mImageElementVec.erase(it);
            imagePntr->remove();
        } else {
            //Advance to the next image element
            ++it;
        }
    }

    if (mWaitForLineSeperatorEnd) {
            //check current rect (position) of last line seperator image element, is it time to
            //add a new text element below?
            if (mCurrRect.UpperLeftCorner.Y < (irr::s32)(mScreenRes.Height)) {
                //time for a new text element
                mWaitForLineSeperatorEnd = false;

                irr::core::rect<irr::s32> nextPos = mCurrRect;

                irr::s32 midX = mScreenRes.Width / 2;

                nextPos.UpperLeftCorner.Y += texSeperatorLineHeight;
                nextPos.UpperLeftCorner.X = midX - mWidthRender / 2;
                nextPos.LowerRightCorner.Y += texSeperatorLineHeight;
                nextPos.LowerRightCorner.X = midX + mWidthRender / 2;

                AddTextElement(nextPos);
            }
    }
}

void Attribution::Update(irr::f32 frameDeltaTime) {
    if (mScrolling) {
        mAbsTime += frameDeltaTime;
        if (mAbsTime > 0.02f) {
            mAbsTime = 0.0f;

            UpdateTexts();
            UpdateImages();
        }
    }
}

//Return true in case of success, False otherwise
bool Attribution::ReadAttributionInfo() {
    if (FileExists("media/attribution.txt") != 1) {
        //Problem with this file
        logging::Warning("Can not read file 'media/attribution.txt'");
        return false;
    }

    //open attribution data text file
    //and read line by line
    std::ifstream attrFile("media/attribution.txt");

    std::string line;
    if (attrFile.is_open()) {
          while (getline(attrFile, line)) {
              mAttrData.push_back(irr::core::stringw(line.c_str()));
          }

          //close the file again
          attrFile.close();

          logging::Info("Succesfully read file 'media/attribution.txt'");
          return true;
     }

     logging::Warning("Unable to open file 'media/attribution.txt'");
     return false;
}
