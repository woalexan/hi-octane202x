/*
 Copyright (C) 2025-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "attribution.h"
#include <fstream>
#include "../utils/fileutils.h"
#include "../utils/logging.h"
#include "../infrabase.h"

Attribution::Attribution(InfrastructureBase* infra, irr::s32 widthRenderPixel, irr::s32 offsXRender, bool enableFading) {
    mInfra = infra;
    mScreenRes = mInfra->mScreenRes;
    mWidthRender = widthRenderPixel;
    mFading = enableFading;
    mOffsXRender = offsXRender;

    texSeperatorLine1Width = 0;
    texSeperatorLine2Width = 0;
    texSeperatorLine3Width = 0;
    texSeperatorLineHeight = 0;

    mTextLineCnt = 0;
    mAbsTime = 0.0f;

    mNextTextLineNr = 0;

    mNoMoreTextAvailable = false;
    mNoMoreImagesAvailable = false;

    mScrollStep.set(0, -1);
}

Attribution::~Attribution() {
    if (mScrolling) {
        Stop();
    }
}

void Attribution::SetFadingParameters(irr::s32 fadeBarHeightPixels, irr::f32 minFadingFactor, irr::f32 maxFadingFactor) {
    mMinFadingFactor = minFadingFactor;
    mMaxFadingFactor = maxFadingFactor;
    mFadeBarHeightPixels = fadeBarHeightPixels;
}

void Attribution::SetFading(bool enable) {
    mFading = enable;
}

void Attribution::SetScrollSpeed(irr::u32 speedValue) {
    mScrollStep.set(0, -(irr::s32)(speedValue));
}

void Attribution::Init() {
    mAttrData.clear();
    mEmbeddedImageVec.clear();

    mTextLineCnt = 0;

    mSuccessText = ReadAttributionInfo();
    mSuccessText &= AddOriginalGameCredits();

    if (mSuccessText) {
        mTextLineCnt = mAttrData.size();
    }

    mSuccessImages = LoadLineSeperatorImages();

    if (mSuccessText && mSuccessImages) {
        mState = DEF_ATTR_STATE_READY;
    } else {
        mState = DEF_ATTR_STATE_INITERROR;
    }
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
bool Attribution::LoadLineSeperatorImages() {
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    std::string texFile1("extract/hud1player/panel0-1-0197.png");
    std::string texFile2("extract/hud1player/panel0-1-0199.png");
    std::string texFile3("extract/hud1player/panel0-1-0198.png");

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

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}

void Attribution::CheckForNewEmbeddedImage(irr::core::rect<irr::s32> newPosition) {
    std::vector<AttrEmbeddedImageStruct*>::iterator it;

    for (it = mEmbeddedImageVec.begin(); it != mEmbeddedImageVec.end(); ++it) {
        if ((*it)->InsertAtLineNr == mNextTextLineNr) {
            //if image was not properly loaded skip this item
            if (!(*it)->imageTexLoaded) {
                continue;
            }

            //define new image location
            (*it)->ImagePos.UpperLeftCorner.X = newPosition.UpperLeftCorner.X + mWidthRender + 20;
            (*it)->ImagePos.UpperLeftCorner.Y = newPosition.UpperLeftCorner.Y;
            (*it)->ImagePos.LowerRightCorner.X = (*it)->ImagePos.UpperLeftCorner.X + (*it)->TargetImageSize.Width;
            (*it)->ImagePos.LowerRightCorner.Y = (*it)->ImagePos.UpperLeftCorner.Y + (*it)->TargetImageSize.Height;

            (*it)->imageAlreadyPresented = true;

            //yes, there is a new image we need to show
            irr::gui::IGUIImage* newEmbeddedImage =
                 mInfra->mGuienv->addImage((*it)->ImagePos, 0, -1, 0, true);

            newEmbeddedImage->setScaleImage(true);
            newEmbeddedImage->setImage((*it)->imageTex);

            mImageElementVec.push_back(newEmbeddedImage);
        }
    }

    //have we presented all images now?
    bool allPicturesDone = true;

    for (it = mEmbeddedImageVec.begin(); it != mEmbeddedImageVec.end(); ++it) {
        if (!(*it)->imageAlreadyPresented) {
            allPicturesDone = false;
            break;
        }
    }

    mNoMoreImagesAvailable = allPicturesDone;
}

void Attribution::AddTextElement(irr::core::rect<irr::s32> newPosition) {
    //verify if we need to add an additional embedded image
    //at this line number
    CheckForNewEmbeddedImage(newPosition);

    if (mNextTextLineNr >= mTextLineCnt) {
        //we are done, no more text lines available
        //simply return
        mNoMoreTextAvailable = true;

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
    if ((mState != DEF_ATTR_STATE_READY) && (mState != DEF_ATTR_STATE_PRESENTATIONDONE))
        return;

    mScrolling = true;
    mState = DEF_ATTR_STATE_PRESENTING;

    mAbsTime = 0.0f;

    mTextElementVec.clear();
    mImageElementVec.clear();

    mNextTextLineNr = 0;

    irr::s32 midX = (mScreenRes.Width / 2) + mOffsXRender;

    std::vector<AttrEmbeddedImageStruct*>::iterator it;

    for (it = mEmbeddedImageVec.begin(); it != mEmbeddedImageVec.end(); ++it) {
        (*it)->imageAlreadyPresented = false;
    }

    mNoMoreTextAvailable = false;
    mNoMoreImagesAvailable = false;

    AddTextElement(irr::core::rect<irr::s32>(midX - mWidthRender / 2, mScreenRes.Height + 10, midX + mWidthRender / 2, mScreenRes.Height + 32));
}

void Attribution::CleanupItems() {
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

    if (mState != DEF_ATTR_STATE_PRESENTATIONDONE) {
        mState = DEF_ATTR_STATE_READY;
    }

    CleanupItems();
}

irr::u8 Attribution::GetState() {
    return mState;
}

irr::f32 Attribution::GetFadingFactor(irr::core::rect<irr::s32> currPos) {
    irr::f32 fadingFactor = mMaxFadingFactor;

    irr::s32 fadeBorder = (irr::s32)(mScreenRes.Height) - mFadeBarHeightPixels;

    //are we in the bar at the bottom of the screen?
    if (currPos.UpperLeftCorner.Y >= fadeBorder) {
        //when the image scrolls upwards from the bottom of the screen, it starts faded out with minFadingFactor,
        //and then fades in until it hits the upper end of the bar with height fadeBarWidthPixels pixels
        irr::f32 kFade = ((mMaxFadingFactor - mMinFadingFactor) / ((irr::f32)(mFadeBarHeightPixels)));
        irr::f32 madeDistance = (irr::f32)(mScreenRes.Height - currPos.UpperLeftCorner.Y);

        fadingFactor = mMinFadingFactor + madeDistance * kFade;
    }

    fadeBorder = (irr::s32)(mFadeBarHeightPixels);

    if (currPos.UpperLeftCorner.Y < 0) {
        fadingFactor = mMinFadingFactor;
    } else

    //are we in the bar at the top of the screen?
    if (currPos.UpperLeftCorner.Y <= fadeBorder) {
        //when the image scrolls upwards at the top of the screen, it starts faded in completely,
        //and then fades out until it hits minFadingFactor at the top of the screen
        irr::f32 kFade = -((mMaxFadingFactor - mMinFadingFactor) / ((irr::f32)(mFadeBarHeightPixels)));
        irr::f32 madeDistance = (irr::f32)(mFadeBarHeightPixels - currPos.UpperLeftCorner.Y);

        fadingFactor = mMaxFadingFactor + madeDistance * kFade;
    }

    return (fadingFactor);
}

void Attribution::ControlTextFading(irr::gui::IGUIStaticText* whichTextElement) {
    irr::core::rect<irr::s32> currPos;

    irr::f32 alpaValText;
    irr::f32 alpaValBackground;

    currPos = whichTextElement->getAbsolutePosition();

    irr::f32 fadingFactor = GetFadingFactor(currPos);

    alpaValText = 255.0f * fadingFactor;
    alpaValBackground = 230.0f * fadingFactor;

    whichTextElement->setOverrideColor(irr::video::SColor((irr::u32)(alpaValText), 255, 255, 255));
    whichTextElement->setBackgroundColor(irr::video::SColor((irr::u32)(alpaValBackground), 0, 0, 0));
}

void Attribution::ControlImageFading(irr::gui::IGUIImage* whichImage) {
    irr::core::rect<irr::s32> currPos;

    irr::f32 alpaValText;
    irr::f32 alpaValBackground;

    currPos = whichImage->getAbsolutePosition();

    irr::f32 fadingFactor = GetFadingFactor(currPos);

    alpaValText = 255.0f * fadingFactor;
    alpaValBackground = 230.0f * fadingFactor;

    whichImage->setColor(irr::video::SColor((irr::u32)(alpaValText), 255, 255, 255));
}

void Attribution::UpdateTexts() {
    std::vector<irr::gui::IGUIStaticText*>::iterator it;
    core::rect<s32> mCurrRect;

    irr::gui::IGUIStaticText* txtPntr;

    for (it = mTextElementVec.begin(); it != mTextElementVec.end(); ++it) {
        (*it)->move(mScrollStep);

        if (mFading) {
            ControlTextFading(*it);
        }
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

        if (mFading) {
            ControlImageFading(*it);
        }
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

                irr::s32 midX = (mScreenRes.Width / 2) + mOffsXRender;

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

            //are we done with presentation?
            if (mNoMoreTextAvailable && mNoMoreImagesAvailable) {
                if ((mTextElementVec.size() == 0) && (mImageElementVec.size() == 0)) {
                    //we are completely done, call stop
                    mState = DEF_ATTR_STATE_PRESENTATIONDONE;
                    Stop();
                }
            }
        }
    }
}

void Attribution::ProcessEmbeddedImageData(size_t currLineNr, std::string& inputLine) {
    //does the current input line contain
    //an embededded image command?
    size_t startPos = inputLine.find("<image>");

    if (startPos == std::string::npos) {
        //no added image command string found
        //just return, do not modify inputLine
        return;
    }

    size_t endPos = inputLine.find("</image>");

    if (endPos == std::string::npos) {
        //something is wrong, we did find a start
        //command for an embededd image, but no end
        //ignore this image command, and output warning
        std::string::iterator startRemove = inputLine.begin() + startPos;
        std::string::iterator endRemove = inputLine.end();

        inputLine.erase(startRemove, endRemove);
        logging::Warning("Attribution: Ignore malformed image input command");

        return;
    }

    //copy imageCmd String out
    std::string imageCmd("");
    imageCmd.append(inputLine.substr(startPos, endPos - startPos));

    //remove image command from the inputLine
    std::string::iterator startRemove = inputLine.begin() + startPos;
    std::string::iterator endRemove = inputLine.end();

    inputLine.erase(startRemove, endRemove);

    //now process the image command string data
    //remove the <image> part
    startPos = imageCmd.find('<');
    endPos = imageCmd.find('>');

    if ((startPos == std::string::npos) || (endPos == std::string::npos)) {
        //something went wrong, stop command processing
        logging::Warning("Attribution: Ignore malformed image input command, can not remove <image>");
        return;
    }

    startRemove = imageCmd.begin() + startPos;
    endRemove = imageCmd.begin() + endPos + 1;

    imageCmd.erase(startRemove, endRemove);

    //find image path by finding first ';' character
    endPos = imageCmd.find(';');

    if (endPos == std::string::npos) {
        logging::Warning("Attribution: Ignore malformed image input command, can not find first ';' character");
        return;
    }

    std::string imagePath("");
    imagePath.append(imageCmd.substr(0, endPos));

    //try to load this image
    irr::s32 newImageWidth;
    irr::s32 newImageHeight;

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    //Returns nullptr in case of a texture loading problem
    irr::video::ITexture* newImagePntr = LoadResourceImage(imagePath, newImageWidth, newImageHeight);

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    if (newImagePntr == nullptr) {
        //image loading issue, interrupt here
        return;
    }

    std::string logIngo("Succesfully loaded image '");
    logIngo.append(imagePath.c_str());
    logIngo.append("'");
    logging::Info(logIngo.c_str());

    std::string targetSizeStr("");
    targetSizeStr.append(imageCmd.substr(endPos + 1, imageCmd.size() - endPos));

    //now parse specified target size information
    //there needs to be another ';' character in there
    startPos = targetSizeStr.find(';');

    if (startPos == std::string::npos) {
        logging::Warning("Attribution: Ignore malformed image input command, can not find ';' character in target image size string");
        return;
    }

    std::string xSizeStr("");
    xSizeStr.append(targetSizeStr.substr(0, startPos));

    std::string ySizeStr("");
    ySizeStr.append(targetSizeStr.substr(startPos + 1, targetSizeStr.size() - startPos));

    irr::u32 targetXsize;
    irr::u32 targetYsize;

    std::sscanf(xSizeStr.c_str(), "%u", &targetXsize);
    std::sscanf(ySizeStr.c_str(), "%u", &targetYsize);

    //image data parsing succesfull
    AttrEmbeddedImageStruct* newImageInfo = new AttrEmbeddedImageStruct();
    newImageInfo->InsertAtLineNr = currLineNr;
    newImageInfo->imageTexLoaded = true;
    newImageInfo->TargetImageSize.set(targetXsize, targetYsize);
    newImageInfo->imageTex = newImagePntr;

    mEmbeddedImageVec.push_back(newImageInfo);
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
    irr::u32 currLineNr = 0;

    if (attrFile.is_open()) {
          while (getline(attrFile, line)) {
              //check for data of embedded images
              //process and then remove this additional
              //file info text
              ProcessEmbeddedImageData(currLineNr, line);

              mAttrData.push_back(irr::core::stringw(line.c_str()));

              currLineNr++;
          }

          //close the file again
          attrFile.close();

          logging::Info("Succesfully read file 'media/attribution.txt'");
          return true;
     }

     logging::Warning("Unable to open file 'media/attribution.txt'");
     return false;
}

//Returns true in case of success, False otherwise
bool Attribution::AddOriginalGameCredits() {
  std::vector<OriginalGameCreditStruct*> originalGameCredits;

  bool success =
      mInfra->ParseOriginalGameCredits(originalGameCredits);

  if (!success) {
      return false;
  }

  mAttrData.push_back("<lineseperator>");
  mAttrData.push_back("Contributors to the original game at 'Bullfrog Productions Ltd':");
  mAttrData.push_back("");

  std::string imageStr("  <image>extract/puzzle/puzzle.png;112;96</image>");
  ProcessEmbeddedImageData(mAttrData.size(), imageStr);

  std::vector<OriginalGameCreditStruct*>::iterator it;
  std::vector<std::string>::iterator itStr;

  for (it = originalGameCredits.begin(); it != originalGameCredits.end(); ++it) {
      if ((*it)->role.find("ELECTRONIC") != std::string::npos) {
          ++it;
          break;
      }

      mAttrData.push_back((*it)->role.c_str());
      mAttrData.push_back("");

      for (itStr = (*it)->individualsVec.begin(); itStr != (*it)->individualsVec.end(); ++itStr) {
            mAttrData.push_back((*itStr).c_str());
      }

      mAttrData.push_back("");
  }

  mAttrData.push_back("");
  mAttrData.push_back("<lineseperator>");
  mAttrData.push_back("Contributors to the original game at 'Electronic Arts Ltd':");
  mAttrData.push_back("");

  for ( ; it != originalGameCredits.end(); ++it) {
      mAttrData.push_back((*it)->role.c_str());
      mAttrData.push_back("");

      for (itStr = (*it)->individualsVec.begin(); itStr != (*it)->individualsVec.end(); ++itStr) {
            mAttrData.push_back((*itStr).c_str());
      }

      mAttrData.push_back("");
  }

  mAttrData.push_back("");
  mAttrData.push_back("<lineseperator>");
  mAttrData.push_back("And last but not least thank you very much for your interest in this project.");
  mAttrData.push_back("It really means a lot to me.");
  mAttrData.push_back("<lineseperator>");
  mAttrData.push_back("");

  return true;
}
