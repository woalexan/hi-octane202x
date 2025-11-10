/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef ATTRIBUTION_H
#define ATTRIBUTION_H

#include <irrlicht.h>
#include <vector>
#include <string>

#define DEF_ATTR_STATE_UNINITIALIZED 0
#define DEF_ATTR_STATE_INITERROR 1
#define DEF_ATTR_STATE_READY 2
#define DEF_ATTR_STATE_PRESENTING 3
#define DEF_ATTR_STATE_PRESENTATIONDONE 4

class InfrastructureBase; //Forward declaration

struct AttrEmbeddedImageStruct {
    irr::u32 InsertAtLineNr;
    irr::core::dimension2d<irr::u32> TargetImageSize;
    irr::core::rect<irr::s32> ImagePos;
    irr::video::ITexture* imageTex = nullptr;
    bool imageTexLoaded = false;
    bool imageAlreadyPresented = false;
};

class Attribution {
public:
    Attribution(InfrastructureBase* infra, irr::s32 widthRenderPixel, bool enableFading);
    ~Attribution();

    void Init();

    void SetFadingParameters(irr::s32 fadeBarHeightPixels, irr::f32 minFadingFactor, irr::f32 maxFadingFactor);
    void SetFading(bool enable);

    void SetScrollSpeed(irr::u32 speedValue);

    void Update(irr::f32 frameDeltaTime);

    void Start();
    void Stop();

    irr::u8 GetState();

private:
    InfrastructureBase* mInfra = nullptr;

    irr::u8 mState = DEF_ATTR_STATE_UNINITIALIZED;

    irr::core::dimension2d<irr::u32> mScreenRes;
    irr::s32 mWidthRender;

    //Return true in case of success, False otherwise
    bool ReadAttributionInfo();

    //Return true in case of success, False otherwise
    bool AddOriginalGameCredits();

    void ProcessEmbeddedImageData(irr::u32 currLineNr, std::string& inputLine);

    //Returns nullptr in case of a texture loading problem
    irr::video::ITexture* LoadResourceImage(std::string filename, irr::s32& texWidth, irr::s32& texHeight);

    //Return true in case of success, False otherwise
    bool LoadLineSeperatorImages();

    void AddTextElement(irr::core::rect<irr::s32> newPosition);
    irr::u32 mNextTextLineNr;
    irr::u32 mTextLineCnt;

    std::vector<irr::core::stringw> mAttrData;

    irr::f32 GetFadingFactor(irr::core::rect<irr::s32> currPos);
    void ControlTextFading(irr::gui::IGUIStaticText* whichTextElement);
    void ControlImageFading(irr::gui::IGUIImage* whichImage);

    void UpdateTexts();

    bool mSuccessText = false;
    bool mSuccessImages = false;
    bool mScrolling = false;

    irr::f32 mAbsTime;

    std::vector<irr::gui::IGUIStaticText*> mTextElementVec;

    void AddLineSeperator(irr::core::rect<irr::s32> newPosition);
    std::vector<irr::gui::IGUIImage*> mImageElementVec;

    void UpdateImages();

    irr::core::position2d<irr::s32> mScrollStep;

    //Images needed as resources
    irr::video::ITexture* texSeperatorLine1 = nullptr;
    irr::video::ITexture* texSeperatorLine2 = nullptr;
    irr::video::ITexture* texSeperatorLine3 = nullptr;

    irr::s32 texSeperatorLine1Width;
    irr::s32 texSeperatorLine2Width;
    irr::s32 texSeperatorLine3Width;

    irr::s32 texSeperatorLineHeight;

    bool mWaitForLineSeperatorEnd = false;

    void CheckForNewEmbeddedImage(irr::core::rect<irr::s32> newPosition);

    std::vector<AttrEmbeddedImageStruct*> mEmbeddedImageVec;

    void CleanupItems();

    //if true text and images are fading in and out at
    //the bottom/top of the screen
    bool mFading;

    //we following variables control the fading
    irr::f32 mMinFadingFactor = 0.0f;
    irr::f32 mMaxFadingFactor = 1.0f;

    //height of "fading" bar in pixels in which
    //fade in/out occurs
    irr::s32 mFadeBarHeightPixels = 200;

    bool mNoMoreTextAvailable;
    bool mNoMoreImagesAvailable;
};

#endif // ATTRIBUTION_H
