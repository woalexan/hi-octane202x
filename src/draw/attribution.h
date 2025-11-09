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

class InfrastructureBase; //Forward declaration

class Attribution {
public:
    Attribution(InfrastructureBase* infra);
    ~Attribution();

    void Init();

    void Update(irr::f32 frameDeltaTime);

    void Start();
    void Stop();
   
private:
    InfrastructureBase* mInfra = nullptr;

    irr::core::dimension2d<irr::u32> mScreenRes;
    irr::s32 mWidthRender;

    //Return true in case of success, False otherwise
    bool ReadAttributionInfo();

    //Returns nullptr in case of a texture loading problem
    irr::video::ITexture* LoadResourceImage(std::string filename, irr::s32& texWidth, irr::s32& texHeight);

    //Return true in case of success, False otherwise
    bool LoadResourceImages();

    void AddTextElement(irr::core::rect<irr::s32> newPosition);
    irr::u32 mNextTextLineNr;
    irr::u32 mTextLineCnt;

    std::vector<irr::core::stringw> mAttrData;

    void ControlTextFading(irr::gui::IGUIStaticText* whichTextElement);
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

    irr::video::ITexture* texIrrlichtLogo = nullptr;
    irr::s32 texIrrlichtLogoWidth;
    irr::s32 texIrrlichtLogoHeight;

    irr::video::ITexture* texSFMLLogo = nullptr;
    irr::s32 texSFMLLogoWidth;
    irr::s32 texSFMLLogoHeight;

    irr::gui::IGUIImage* IrrlichtLogo;
    irr::gui::IGUIImage* SFMLLogo;

    void CleanupItems();
};

#endif // ATTRIBUTION_H
