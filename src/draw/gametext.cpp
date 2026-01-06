/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "gametext.h"
#include "../infrabase.h"
#include "../utils/logging.h"

//Loads a game font from the extracted (many) character image files
//Parameters:
//  filename = beginning of image files filename and relative path
//  numOffset = value to tell the function offset of first number in filename for first
//              character to be loaded
//  numChars = value to tell the function how many characters need to be loaded
//             character image filenames contain incrementing number at the end
//  loadAddFileNr = allows to specify additional files for loading character images file offsets
//                  at the end
//In case of an unexpected error this function returns nullptr
GameTextFont* GameText::LoadGameFont(char* fileName, const char *fileEnding, unsigned long numOffset, 
    unsigned long numChars, std::vector<int> loadAddFileNr) {
  mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

  char finalpath[70];
  char fname[20];

  //first load font info Xml file
  //we get transparent pixel data from there
  std::string inputPath(fileName);
  size_t endPathPos = inputPath.find_last_of('/');

  if (endPathPos == std::string::npos) {
      logging::Error("LoadGameFont: Invalid path to font, font loading failed");
      return nullptr;
  }

  irr::video::SColor transColor;

  std::string path = inputPath.substr(0, endPathPos);
  if (!mInfra->ReadFontInfoXmlFile(mInfra->mDevice, irr::io::path(path.c_str()), transColor)) {
      logging::Error("LoadGameFont: Font Info Xml file reading failed");
      return nullptr;
  }

  //create the new font
  GameTextFont *newFont = new GameTextFont;
  newFont->CharacterVector.clear();

  std::vector<unsigned long> loadFileNumbers;
  loadFileNumbers.clear();

  for (unsigned long idx = numOffset; idx < (numChars + numOffset); idx++) {
      loadFileNumbers.push_back(idx);
  }

  std::vector<int>::iterator it;
  for (it = loadAddFileNr.begin(); it!=loadAddFileNr.end(); ++it) {
      loadFileNumbers.push_back(*it);
  }

  std::vector<unsigned long>::iterator it2;

  for (it2 = loadFileNumbers.begin(); it2 != loadFileNumbers.end(); ++it2) {
      //build current filename
      strcpy(finalpath, fileName);
      sprintf (fname, "%0*lu%s", 4, (*it2), fileEnding);
      strcat(finalpath, fname);

      //load a new character for the new font
      GameTextCharacterInfo* newCharInfo = new GameTextCharacterInfo;

      newCharInfo->texture = mInfra->mDriver->getTexture(finalpath);
    
      if (newCharInfo->texture == nullptr) {
          //there was a texture loading error
          //just return with nullptr
          return nullptr;
      }

      newCharInfo->sizeRawTex = newCharInfo->texture->getOriginalSize();
      newCharInfo->charRect.UpperLeftCorner.set(0, 0);
      newCharInfo->charRect.LowerRightCorner.set(newCharInfo->sizeRawTex.Width, newCharInfo->sizeRawTex.Height);
      newCharInfo->transColor = transColor;

      mInfra->mDriver->makeColorKeyTexture(newCharInfo->texture, newCharInfo->transColor);

      //add new character to our vector of characters for this font
      newFont->CharacterVector.push_back(newCharInfo);
  }

  mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

  return newFont;
}

//Deletes a loaded font, and does cleanup heap
void GameText::FreeTextFont(GameTextFont &pntrFont) {
   std::vector<GameTextCharacterInfo*>::iterator itChar;

   GameTextCharacterInfo* pntr;

   //delete all existing characters in this font
   itChar = pntrFont.CharacterVector.begin();

   while (itChar != pntrFont.CharacterVector.end()) {
          pntr = (*itChar);
          itChar = pntrFont.CharacterVector.erase(itChar);

          //remove underlying texture
          mInfra->mDriver->removeTexture(pntr->texture);

          //delete character object itself
          delete pntr;
   }

   //delete font object itself
   delete &pntrFont;
}

//Renders specified 2D text using irrlicht at the specified location
//Parameters:
//  text = Text which should be rendered (null terminated!)
//  whichFont = pointer to the font that should be used
//  position = 2D position where text rendering should occur (leftmost character of text)
//  stopAfterNrChars = optional parameter (default is -1 which means feature inactive)
//                     If specified stops text rendering after specified number of chars
void GameText::DrawGameText(char* text, GameTextFont *whichFont, irr::core::position2di position, irr::s16 stopAfterNrChars) {
  //only continue when fonts loaded ok
  if (GameTextInitializedOk && (whichFont != nullptr)) {
    char* pntr = &text[0];
    irr::core::vector2di correctCharPosition = position;
    irr::s16 charCnter = stopAfterNrChars;

    while ((*pntr != 0) && ((charCnter > 0) || (stopAfterNrChars == -1))) {
        //draw current character
        mInfra->mDriver->draw2DImage(whichFont->CharacterVector[*pntr]->texture, correctCharPosition,
              whichFont->CharacterVector[*pntr]->charRect, 0,
                  irr::video::SColor(255,255,255,255), true);

        //calculate next char position
        correctCharPosition.X += whichFont->CharacterVector[*pntr]->charRect.getWidth();

        pntr++;

        if (charCnter > 0)
            charCnter--;
     }
  }
}

//Calculates the width in pixels of a text message when rendered using the specified font and text
//Parameters:
//  text = Text which should be rendered (null terminated!)
//  whichFont = pointer to the font that should be used
//  stopAfterNrChars = optional parameter (default is -1 which means feature inactive)
//                     If specified stops text rendering after specified number of chars
irr::u32 GameText::GetWidthPixelsGameText(char* text, GameTextFont *whichFont, irr::s16 stopAfterNrChars) {
    irr::u32 width = 0;
    irr::s16 charCnter = stopAfterNrChars;

    //only continue when fonts loaded ok
    if (GameTextInitializedOk && (whichFont != nullptr)) {

        char* pntr = &text[0];

        while ((*pntr != 0) && ((charCnter > 0) || (stopAfterNrChars == -1))) {
            //calculate next char position
            width += whichFont->CharacterVector[*pntr]->charRect.getWidth();

            pntr++;

            if (charCnter > 0)
                charCnter--;
        }
    }

    return width;
}

//delivers the maximum height in pixels of all characters found in the specified text
//  text = Text which should be rendered (null terminated!)
//  whichFont = pointer to the font that should be used
//  stopAfterNrChars = optional parameter (default is -1 which means feature inactive)
//                     If specified stops text rendering after specified number of chars
irr::u32 GameText::GetHeightPixelsGameText(char* text, GameTextFont *whichFont, irr::s16 stopAfterNrChars) {
    irr::u32 maxHeight = 0;
    irr::u32 currHeight;
    irr::s16 charCnter = stopAfterNrChars;

    //only continue when fonts loaded ok
    if (GameTextInitializedOk && (whichFont != nullptr)) {

        char* pntr = &text[0];

        while ((*pntr != 0) && ((charCnter > 0) || (stopAfterNrChars == -1))) {
            //get height of current char
            currHeight = whichFont->CharacterVector[*pntr]->charRect.getHeight();

            if (currHeight > maxHeight) {
                maxHeight = currHeight;
            }

            pntr++;

            if (charCnter > 0)
                charCnter--;
        }
    }

    return maxHeight;
}

//Renders specified 2D text (can only contain uppercase characters from A-Z) using irrlicht at the specified location
//this fonts are used next to the Hud target symbol to name players
//Parameters:
//  numberText = Text which should be rendered (null terminated!)
//  whichFont = pointer to the font that should be used
//  position = 2D position where text rendering should occur (leftmost character of text)
void GameText::DrawHudSmallText(char* alphanumericalText, GameTextFont *whichHudFont, irr::core::position2di position) {
    //only continue when fonts loaded ok
    if (GameTextInitializedOk && (whichHudFont != nullptr)) {
      char* pntr = &alphanumericalText[0];
      char index;
      bool skipChar;

      irr::core::vector2di correctCharPosition = position;

      while (*pntr != 0) {
          //different to the other (full text font) text output render function DrawGameText for
          //this function/fonts the vector of characters only contains uppercase A-Z, and no
          //special characters
          //therefore we need to do the mapping of Ascii character to index of font vector
          skipChar = false;

          if ((*pntr >= 'A') && (*pntr <= 'Z')) {
              index = (*pntr - 65);
          } else {
              //found a non printable character in this (very limited) font
              //just skip it
              skipChar = true;
          }

          if (!skipChar) {
              //draw current character
              mInfra->mDriver->draw2DImage(whichHudFont->CharacterVector[index]->texture, correctCharPosition,
                whichHudFont->CharacterVector[index]->charRect, 0,
                    irr::video::SColor(255,255,255,255), true);

              //calculate next char position
              correctCharPosition.X += whichHudFont->CharacterVector[index]->charRect.getWidth();
          }

          pntr++;
      }
    }
}

//Renders specified 2D text (can only contain numbers from 0-9, and characters ".", "/" and ">") using irrlicht at the specified location
//and ">" will draw the graphics symbol with the two red arrows that is used next to the current lap numbers
//Parameters:
//  numberText = Text which should be rendered (null terminated!)
//  whichFont = pointer to the font that should be used
//  position = 2D position where text rendering should occur (leftmost character of text)
void GameText::DrawGameNumberText(char* numberText, GameTextFont *whichFont, irr::core::position2di position) {
  //only continue when fonts loaded ok
  if (GameTextInitializedOk && (whichFont != nullptr)) {
    char* pntr = &numberText[0];
    char index;
    bool skipChar;

    irr::core::vector2di correctCharPosition = position;

    while (*pntr != 0) {
        //different to the other (full text font) text output render function DrawGameText for
        //this function/fonts the vector of characters only contains numbers 0 up to 9, and special for the red text
        //type (HudLaptimeNumberRed) additional two special characters "." and "/"
        //therefore we need to do the mapping of Ascii character to index of font vector
        skipChar = false;

        if (whichFont == HudLaptimeNumberRed) {
            if (*pntr == '.')
                index = 10;
            else if (*pntr == '/')
                index = 11;
            else if (*pntr == '>')
                index = 12;
            else if ((*pntr >= '0') && (*pntr <= '9')) {
                index = (*pntr - 48);
            } else {
                //found a non printable character in this (very limited) font
                //just skip it
                skipChar = true;
            }
        } else  if (whichFont == HudLaptimeNumberGrey) {
            if (*pntr == '.')
                index = 10;
            else if (*pntr == '/')
                index = 11;
            else if ((*pntr >= '0') && (*pntr <= '9')) {
                index = (*pntr - 48);
            } else {
                //found a non printable character in this (very limited) font
                //just skip it
                skipChar = true;
            }
        } else  if (whichFont == HudKillCounterNumberRed) {
            if (*pntr == '>')
                index = 10;
            else if ((*pntr >= '0') && (*pntr <= '9')) {
                index = (*pntr - 48);
            } else {
                //found a non printable character in this (very limited) font
                //just skip it
                skipChar = true;
            }
        }

        if (!skipChar) {
            //draw current character
            mInfra->mDriver->draw2DImage(whichFont->CharacterVector[index]->texture, correctCharPosition,
              whichFont->CharacterVector[index]->charRect, 0,
                  irr::video::SColor(255,255,255,255), true);

            //calculate next char position
            correctCharPosition.X += whichFont->CharacterVector[index]->charRect.getWidth();
        }

        pntr++;
    }
  }
}

//Calculates the width in pixels of a number text (can only contain numbers from 0-9, and characters "." and "/", some special
//fonts can also contain a special symbol ">")
//message when rendered using the specified font and text
//Parameters:
//  numberText = Text which should be rendered (null terminated!)
//  whichFont = pointer to the font that should be used
irr::u32 GameText::GetWidthPixelsGameNumberText(char* numberText, GameTextFont *whichFont) {
    irr::u32 width = 0;

    //only continue when fonts loaded ok
    if (GameTextInitializedOk && (whichFont != nullptr)) {
        char* pntr = &numberText[0];
        char index;
        bool skipChar;

        while (*pntr != 0) {
            //different to the other (full text font) text output render function DrawGameText for
            //this function/fonts the vector of characters only contains numbers 0 up to 9, and two
            //special characters "." and "/"
            //therefore we need to do the mapping of Ascii character to index of font vector
            skipChar = false;

            if (whichFont == HudLaptimeNumberRed) {
                if (*pntr == '.')
                    index = 10;
                else if (*pntr == '/')
                    index = 11;
                else if (*pntr == '>')
                    index = 12;
                else if ((*pntr >= '0') && (*pntr <= '9')) {
                    index = (*pntr - 48);
                } else {
                    //found a non printable character in this (very limited) font
                    //just skip it
                    skipChar = true;
                }
            } else  if (whichFont == HudLaptimeNumberGrey) {
                if (*pntr == '.')
                    index = 10;
                else if (*pntr == '/')
                    index = 11;
                else if ((*pntr >= '0') && (*pntr <= '9')) {
                    index = (*pntr - 48);
                } else {
                    //found a non printable character in this (very limited) font
                    //just skip it
                    skipChar = true;
                }
            } else  if (whichFont == HudKillCounterNumberRed) {
                if (*pntr == '>')
                    index = 10;
                else if ((*pntr >= '0') && (*pntr <= '9')) {
                    index = (*pntr - 48);
                } else {
                    //found a non printable character in this (very limited) font
                    //just skip it
                    skipChar = true;
                }
            }

             if (!skipChar) {
                //calculate next char position
                width += whichFont->CharacterVector[index]->charRect.getWidth();
             }

            pntr++;
        }
    }

    return width;
}

void GameText::LoadInitialFont() {
    std::vector<int> addFileOffs = {};

    //load white Hud banner text font smaller (SVGA), 241 characters need to be loaded
    GameMenueWhiteTextSmallSVGA = LoadGameFont((char*)"extract/fonts/smallsvga/pre-osfnt0-1-", ".png", 0, 241, addFileOffs);

    //was there are problem loading the text font?
    if (GameMenueWhiteTextSmallSVGA == nullptr) {
        GameTextInitializedOk = false;
    }
}

//this function handles loading fonts step 2
//from the mainloop of the game
void GameText::LoadFontsStep2() {
    std::vector<int> addFileOffs = {};

    //load white Hud banner text font, 241 characters need to be loaded
    HudWhiteTextBannerFont = LoadGameFont((char*)"extract/fonts/large/pre-olfnt0-1-", ".png", 0, 241, addFileOffs);

    //was there are problem loading the text font?
    if (HudWhiteTextBannerFont == nullptr) {
        GameTextInitializedOk = false;
    }

    //load white text font used in game menue, 241 characters need to be loaded
    if (!mInfra->mGameConfig->enableDoubleResolution) {
        GameMenueSelectedItemFont = LoadGameFont((char*)"extract/fonts/large/pre-olfnt0-1-", ".png", 0, 241, addFileOffs);
    }
    else {
        //for double resolution use font upscaled by factor of 2
        GameMenueSelectedItemFont = LoadGameFont((char*)"extract/fonts/large-x2/pre-olfnt0-1-", ".png", 0, 241, addFileOffs);
    }

    //was there are problem loading the text font?
    if (GameMenueSelectedItemFont == nullptr) {
        GameTextInitializedOk = false;
    }

    //load big green Hud text font, 241 characters need to be loaded
    HudBigGreenText = LoadGameFont((char*)"extract/fonts/largegreen/pre-pfont0-1-", ".png", 0, 241, addFileOffs);

    //was there are problem loading the text font?
    if (HudBigGreenText == nullptr) {
        GameTextInitializedOk = false;
    }

    //load Hud laptime number fonts in red, only 12 characters need to be loaded
    //also add graphical symbol with the 2 red arrows (which is used next to the current lap numbers)
    std::vector<int> addFileOffs2 = {226};
    //TODO 04.01.2026: Images still need to be preoptimized in PrepareData!
    HudLaptimeNumberRed = LoadGameFont((char*)"extract/hud1player/panel0-1-", ".bmp", 138, 12, addFileOffs2 /*, false*/);

    //was there are problem loading the text font?
    if (HudLaptimeNumberRed == nullptr) {
        GameTextInitializedOk = false;
    }

    //load Hud laptime number fonts in grey, only 12 characters need to be loaded
    //TODO 04.01.2026: Images still need to be preoptimized in PrepareData!
    HudLaptimeNumberGrey = LoadGameFont((char*)"extract/hud1player/panel0-1-", ".bmp", 150, 12, addFileOffs /*, false*/);

    //was there are problem loading the text font?
    if (HudLaptimeNumberGrey == nullptr) {
        GameTextInitializedOk = false;
    }

    //add also the skull graphical symbol
    std::vector<int> addFileOffs3 = {227};
    //TODO 04.01.2026: Images still need to be preoptimized in PrepareData!
    HudKillCounterNumberRed = LoadGameFont((char*)"extract/hud1player/panel0-1-", ".bmp", 117, 10, addFileOffs3 /*, false*/);

    //load font we created ourself before for unselected items in mainmenue
    if (!mInfra->mGameConfig->enableDoubleResolution) {
        GameMenueUnselectedEntryFont = LoadGameFont((char*)"extract/fonts/largegreenish/pre-green-olfnt0-1-", ".png", 0, 241, addFileOffs);
    }
    else {
        //for double resolution use font upscaled by factor of 2
        GameMenueUnselectedEntryFont = LoadGameFont((char*)"extract/fonts/largegreenish-x2/pre-green-olfnt0-1-", ".png", 0, 241, addFileOffs);
    }

    //was there are problem loading the text font?
    if (GameMenueUnselectedEntryFont == nullptr) {
        GameTextInitializedOk = false;
    }

    //load font we created ourself before for unselected items in mainmenue (based on smaller text size)
    if (!mInfra->mGameConfig->enableDoubleResolution) {
        GameMenueUnselectedTextSmallSVGA = LoadGameFont((char*)"extract/fonts/smallsvgagreenish/pre-green-osfnt0-1-", ".png", 0, 241, addFileOffs);
    }
    else {
        //for double resolution use font upscaled by factor of 2
        GameMenueUnselectedTextSmallSVGA = LoadGameFont((char*)"extract/fonts/smallsvgagreenish-x2/pre-green-osfnt0-1-", ".png", 0, 241, addFileOffs);
    }

    //was there are problem loading the text font?
    if (GameMenueUnselectedTextSmallSVGA == nullptr) {
        GameTextInitializedOk = false;
    }

    //load very small green target description font of HUD
    //TODO 04.01.2026: Images still need to be preoptimized in PrepareData!
    HudTargetNameGreen = LoadGameFont((char*)"extract/hud1player/panel0-1-", ".bmp", 231, 26, addFileOffs /*, false*/);

    //was there are problem loading the text font?
    if (HudTargetNameGreen == nullptr) {
        GameTextInitializedOk = false;
    }

    //load very small red target description font of HUD
    //TODO 04.01.2026: Images still need to be preoptimized in PrepareData!
    HudTargetNameRed = LoadGameFont((char*)"extract/hud1player/panel0-1-", ".bmp", 200, 26, addFileOffs /*, false*/);

    //was there are problem loading the text font?
    if (HudTargetNameRed == nullptr) {
        GameTextInitializedOk = false;
    }

    //load white thin font
    ThinWhiteText = LoadGameFont((char*)"extract/fonts/thinwhite/pre-hfont0-0-", ".png", 0, 127, addFileOffs);

    //was there are problem loading the text font?
    if (ThinWhiteText == nullptr) {
        GameTextInitializedOk = false;
    }
}

//Constructor, initialization of all available and needed GameText fonts
// In case of unexpected initialization issue this constructor sets public
// member GameTextInitializedOk to false, and to true otherwise.
GameText::GameText(InfrastructureBase* infra) {
    mInfra = infra;
    this->GameTextInitializedOk = true;

    //only load an initial font so that we can
    //continue to the main loop of the game
    //so that we do not block the rendering
    LoadInitialFont();
}

GameText::~GameText() {
    //cleanup all fonts
    //06.04.2025: Because font loading happens now in 2 steps
    //make sure we do not try to unload font that was not
    //loaded yet, especially if we delete object again before
    //second load step was executed. Therefore adding nullptr
    //check before attempting to free font
    if (HudWhiteTextBannerFont != nullptr) {
        FreeTextFont(*HudWhiteTextBannerFont);
        HudWhiteTextBannerFont = nullptr;
    }

    if (GameMenueSelectedItemFont != nullptr) {
        FreeTextFont(*GameMenueSelectedItemFont);
        GameMenueSelectedItemFont = nullptr;
    }

    if (HudBigGreenText != nullptr) {
        FreeTextFont(*HudBigGreenText);
        HudBigGreenText = nullptr;
    }

    if (HudLaptimeNumberRed != nullptr) {
        FreeTextFont(*HudLaptimeNumberRed);
        HudLaptimeNumberRed = nullptr;
    }

    if (HudLaptimeNumberGrey != nullptr) {
        FreeTextFont(*HudLaptimeNumberGrey);
        HudLaptimeNumberGrey = nullptr;
    }

    if (HudKillCounterNumberRed != nullptr) {
        FreeTextFont(*HudKillCounterNumberRed);
        HudKillCounterNumberRed = nullptr;
    }

    if (GameMenueUnselectedEntryFont != nullptr) {
        FreeTextFont(*GameMenueUnselectedEntryFont);
        GameMenueUnselectedEntryFont = nullptr;
    }

    if (HudTargetNameGreen != nullptr) {
        FreeTextFont(*HudTargetNameGreen);
        HudTargetNameGreen = nullptr;
    }

    if (HudTargetNameRed != nullptr) {
        FreeTextFont(*HudTargetNameRed);
        HudTargetNameRed = nullptr;
    }
    
    if (ThinWhiteText != nullptr) {
        FreeTextFont(*ThinWhiteText);
        ThinWhiteText = nullptr;
    }

    if (GameMenueWhiteTextSmallSVGA != nullptr) {
        FreeTextFont(*GameMenueWhiteTextSmallSVGA);
        GameMenueWhiteTextSmallSVGA = nullptr;
    }

    if (GameMenueUnselectedTextSmallSVGA != nullptr) {
        FreeTextFont(*GameMenueUnselectedTextSmallSVGA);
        GameMenueUnselectedTextSmallSVGA = nullptr;
    }
}
