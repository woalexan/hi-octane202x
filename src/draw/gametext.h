/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef GAMETEXT_H
#define GAMETEXT_H

#include "irrlicht.h"
#include <algorithm>
#include <vector>

#define WaitTimeBeforeNextBannerState 0.1f  //in seconds

 /************************
  * Forward declarations *
  ************************/

class InfrastructureBase;

typedef struct GameTextCharacterInfo {
      //contains the raw texture for the character
     irr::video::ITexture* texture = nullptr;

     //size in pixels of the raw texture for character
     irr::core::dimension2d<irr::s32> sizeRawTex;

     //contains "optimized" area of area, non useful empty black area removed
     //only 1 line of black area left on left and right side
     irr::core::rect<irr::s32> charRect;

     //contains the transparent color found for this
     //character
     irr::video::SColor transColor;
} GameTextCharacterInfo;

typedef struct GameTextFont {
    std::vector<GameTextCharacterInfo*> CharacterVector;
} GameTextFont;

class GameText {

private:
    InfrastructureBase* mInfra = nullptr;

    void LoadInitialFont();

    GameTextFont* LoadGameFont(char* fileName, const char* fileEnding, unsigned long numOffset, unsigned long numChars,
        std::vector<int> loadAddFileNr, bool addOutline, irr::video::SColor* outLineColor = nullptr);
    irr::core::rect<irr::s32> FindCharArea(GameTextCharacterInfo *character, bool &succesFlag);
    bool AddColoredOutline(GameTextCharacterInfo &character, irr::video::SColor *outLineColor);
    void FreeTextFont(GameTextFont &pntrFont);

    //uses the 4 corner pixel of the character to derive the most
    //likely transparent pixel color
    //Returns true in case of success, false otherwise
    bool DeriveTransparentColorForChar(GameTextCharacterInfo &character);
    void AddPixelToColorOccurenceList(std::vector<std::pair <irr::u8, irr::video::SColor>> &colorOccurenceList,
                                                irr::video::SColor newColor);

public:
    GameText(InfrastructureBase* infra);
    ~GameText();

    void LoadFontsStep2();

    //Renders specified 2D text using irrlicht at the specified location
    //Parameters:
    //  text = Text which should be rendered (null terminated!)
    //  whichFont = pointer to the font that should be used
    //  position = 2D position where text rendering should occur (leftmost character of text)
    //  stopAfterNrChars = optional parameter (default is -1 which means feature inactive)
    //                     If specified stops text rendering after specified number of chars
    void DrawGameText(char* text, GameTextFont *whichFont, irr::core::position2di position, irr::s16 stopAfterNrChars = -1);
    irr::u32 GetWidthPixelsGameText(char* text, GameTextFont *whichFont, irr::s16 stopAfterNrChars = -1);

    //delivers the maximum height in pixels of all characters found in the specified text
    //  text = Text which should be rendered (null terminated!)
    //  whichFont = pointer to the font that should be used
    //  stopAfterNrChars = optional parameter (default is -1 which means feature inactive)
    //                     If specified stops text rendering after specified number of chars
    irr::u32 GetHeightPixelsGameText(char* text, GameTextFont *whichFont, irr::s16 stopAfterNrChars = -1);

    //Renders specified 2D text (can only contain numbers from 0-9, and characters "." and "/") using irrlicht at the specified location
    //Parameters:
    //  numberText = Text which should be rendered (null terminated!)
    //  whichFont = pointer to the font that should be used
    //  position = 2D position where text rendering should occur (leftmost character of text)
    void DrawGameNumberText(char* numberText, GameTextFont *whichFont, irr::core::position2di position);

    //Renders specified 2D text (can only contain uppercase characters from A-Z) using irrlicht at the specified location
    //this fonts are used next to the Hud target symbol to name players
    //Parameters:
    //  numberText = Text which should be rendered (null terminated!)
    //  whichFont = pointer to the font that should be used
    //  position = 2D position where text rendering should occur (leftmost character of text)
    void DrawHudSmallText(char* alphanumericalText, GameTextFont *whichHudFont, irr::core::position2di position);

    irr::u32 GetWidthPixelsGameNumberText(char* numberText, GameTextFont *whichFont);

    //main use is in Hud banner text at the lower part of the screen
    GameTextFont* HudWhiteTextBannerFont = nullptr;

    //used in game menue for selected items
    GameTextFont* GameMenueSelectedItemFont = nullptr;

    //is the white smaller text font that is used in the menue for example
    //to give additional information about number of laps etc...
    GameTextFont* GameMenueWhiteTextSmallSVGA = nullptr;

    //is the green smaller text font that is used in the menue for example
    //to give additional information about number of laps etc...
    //is actually derived from loaded font GameMenueWhiteTextSmallSVGA, by just swapping color of
    //pixels from white to color "green"
    GameTextFont* GameMenueUnselectedTextSmallSVGA = nullptr;

    //main use is in the main menue of the page for entries that are not selected
    //is actually derived from loaded font HudWhiteTextBannerFont, by just swapping color of
    //pixels from white to color "green"
    GameTextFont* GameMenueUnselectedEntryFont = nullptr;

    //main use is text that is temporarily seen in the middle
    //of the Hud (for example Superman etc..)
    GameTextFont* HudBigGreenText = nullptr;

    //main use is to display the lap time number text in Hud (there is a red and grey variant of the characters)
    //important note: this font only contains numbers from 0 up to 9, "." and "/" character!
    //and for the red version of the text there is an additional special character ">" that can be used for
    //printing, which will draw the graphics symbol with the two red arrows that is used next to the current lap numbers
    //to print numbers text using this font we also have to use DrawGameNumberText routine instead
    //of method DrawGameText!
    GameTextFont* HudLaptimeNumberRed = nullptr;
    GameTextFont* HudLaptimeNumberGrey = nullptr;

    //main use is to diplay the kill count in the Hud with the bigger red number letters
    //there is also a special character ">" that will draw the red skull used next to the current player kill count
    GameTextFont* HudKillCounterNumberRed = nullptr;

    //the following two (very small fonts) are used
    //to describe target (opponents) names next
    //to the HUD target symbol
    //only characters from A up to Z are available!
    GameTextFont* HudTargetNameGreen = nullptr;
    GameTextFont* HudTargetNameRed = nullptr;

    //Not used until 03.01.2026; commented out
    //GameTextFont* ThinWhiteText = nullptr;

    bool GameTextInitializedOk = false;
};

#endif // GAMETEXT_H
