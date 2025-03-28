/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "gametext.h"

void GameText::AddPixelToColorOccurenceList(std::vector<std::pair <irr::u8, irr::video::SColor>> &colorOccurenceList,
                                            irr::video::SColor newColor)
{
   //check if input color already exists in list
   std::vector<std::pair <irr::u8, irr::video::SColor>>::iterator it;

   for (it = colorOccurenceList.begin(); it != colorOccurenceList.end(); ++it) {
       if ((*it).second == newColor) {
           //color does already exist in list
           //just increase occurence
           (*it).first += 1;
           return;
       }
   }

   //color does not yet exist in list
   //add entry with occurence 1
   colorOccurenceList.push_back( std::make_pair(1, newColor));
}

//uses the 4 corner pixel of the character to derive the most
//likely transparent pixel color
//Returns true in case of success, false otherwise
bool GameText::DeriveTransparentColorForChar(GameTextCharacterInfo &character) {
    //we need to know the used pixel color format
     irr::video::ECOLOR_FORMAT format = character.texture->getColorFormat();

     //we can only handle this format right now
     if(irr::video::ECF_A8R8G8B8 == format)
       {
         //lock texture for just reading of pixel data
         irr::u8* datapntr = (irr::u8*)character.texture->lock(irr::video::ETLM_READ_ONLY);
         irr::u32 pitch = character.texture->getPitch();

         //the get the most likely transparent color of the font character
         //take the 4 corner pixels, and inspect which color the have
         //the color with the most occurence will most likey be the transparent
         //font character background color

         //declaring vector of pairs containing pixel color
         //and number of occurence of color
         std::vector< std::pair <irr::u8, irr::video::SColor> > vecColorOccurence;
         vecColorOccurence.clear();

         //get left upper pixel
         irr::video::SColor* texelTrans = (irr::video::SColor *)(datapntr + ((0 * pitch) + (0 * sizeof(irr::video::SColor))));

         AddPixelToColorOccurenceList(vecColorOccurence, *texelTrans);

         //get right upper pixel
         texelTrans = (irr::video::SColor *)(datapntr + ((0 * pitch) + ((character.sizeRawTex.Width - 1) * sizeof(irr::video::SColor))));

         AddPixelToColorOccurenceList(vecColorOccurence, *texelTrans);

         //get left lower pixel
         texelTrans = (irr::video::SColor *)(datapntr + (((character.sizeRawTex.Height - 1) * pitch) + (0 * sizeof(irr::video::SColor))));

         AddPixelToColorOccurenceList(vecColorOccurence, *texelTrans);

         //get right lower pixel
         texelTrans = (irr::video::SColor *)(datapntr + (((character.sizeRawTex.Height - 1) * pitch) + ((character.sizeRawTex.Width - 1) * sizeof(irr::video::SColor))));

         AddPixelToColorOccurenceList(vecColorOccurence, *texelTrans);

         //now sort list of pixel color occurences with falling number
         //of occurences, the color we want to find is then the one at the top

         //sort vector pairs in descending number of occurences
         std::sort(vecColorOccurence.rbegin(), vecColorOccurence.rend());

         //the most likely transparent color is now the one at the beginning of
         //the list
         character.transColor = vecColorOccurence.begin()->second;

         return true;
       } else {
         //unsupported pixel color format!
         return false;
     }
}

//Takes the image from a font character, and defines a rect that contains the pixels of the character,
//while removing unnecessary transparent columns of pixels
//Parameters:
//  character = pointer to the character that should be "optimized"
//In case of an unexpected error this function returns succesFlag = false, True otherwise
irr::core::rect<irr::s32> GameText::FindCharArea(GameTextCharacterInfo *character, bool &succesFlag) {
 bool wholeColumnTrans;
 irr::s32 firstCharColumn = -1;
 irr::s32 lastCharColumn = -1;

 //we need to know the used pixel color format
  irr::video::ECOLOR_FORMAT format = character->texture->getColorFormat();

  //we can only handle this format right now
  if(irr::video::ECF_A8R8G8B8 == format)
    {
        //lock texture for just reading of pixel data
        irr::u8* datapntr = (irr::u8*)character->texture->lock(irr::video::ETLM_READ_ONLY);
        irr::u32 pitch = character->texture->getPitch();

        //all raw character pictures have the transparent color at the upper leftmost pixel (0,0)
        //get this value, as a reference where there is no pixel of the character itself
        //irr::video::SColor* texelTrans = (irr::video::SColor *)(datapntr + ((0 * pitch) + (0 * sizeof(irr::video::SColor))));
        irr::video::SColor* texelTrans = &character->transColor;

        //iterate through all colums from left to right
        for (irr::s32 x = 0; x < character->sizeRawTex.Width; x++) {
            //check current column from top to bottom to see if we only find
            //black (unused pixels)
            wholeColumnTrans = true;

            for (irr::s32 y = 0; y < character->sizeRawTex.Height; y++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeColumnTrans = false;
                }
            }

            if (!wholeColumnTrans) {
                if (firstCharColumn == -1)
                    firstCharColumn = x;
                break;
            }
        }

        //iterate through all colums from right to left
        for (irr::s32 x = (character->sizeRawTex.Width - 1); x > -1 ; x--) {
            //check current column from top to bottom to see if we only find
            //black (unused pixels)
            wholeColumnTrans = true;

            for (irr::s32 y = 0; y < character->sizeRawTex.Height; y++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeColumnTrans = false;
                }
            }

            if (!wholeColumnTrans) {
                if (lastCharColumn == -1)
                    lastCharColumn = x;
                break;
            }
        }

        //unlock texture again!
        character->texture->unlock();

        //if possible leave one fully transparent column left of the character data
       if (firstCharColumn > 0)
           firstCharColumn--;

       //if no character was found at all return "full" empty character
       if (firstCharColumn == -1)
           firstCharColumn = 0;

       if (lastCharColumn == -1)
           lastCharColumn = character->sizeRawTex.Width;

         //if possible leave one fully transparent column right of the character data
       if (lastCharColumn <  character->sizeRawTex.Width)
           lastCharColumn++;

        //return optimized size of character
        irr::core::rect<irr::s32> result(firstCharColumn, 0, lastCharColumn, character->sizeRawTex.Height);
        succesFlag = true;
        return result;
  } else {
      //unsupported pixel color format!
      //just return full characters, and set successFlag to False
      irr::core::rect<irr::s32> result(0, 0, character->sizeRawTex.Width, character->sizeRawTex.Height);
      succesFlag = false;
      return result;
  }
}

//Takes the image from a font character, and adds an single pixel wide outline around it with a specified
//color; The game seems to do the same to improve the contrast of the text
//Parameters:
//  character = pointer to the input character
//In case of an unexpected error this function returns succesFlag = false, True otherwise
bool GameText::AddColoredOutline(GameTextCharacterInfo &character, irr::video::SColor *outLineColor) {

  irr::video::SColor* texelTrans;
  irr::video::SColor texelTextColor;
  bool textColFound = false;

 //we need to know the used pixel color format
  irr::video::ECOLOR_FORMAT format = character.texture->getColorFormat();

  //we can only handle this format right now
  if(irr::video::ECF_A8R8G8B8 == format)
    {
      irr::core::vector2d<irr::u32> texSize = character.texture->getSize();

      //create a helper texture in which we store the initial character pixel info
      irr::video::ITexture* hTex = this->myDriver->addTexture(texSize, irr::io::path("helperTex"), format);

        //lock texture for just reading of pixel data
        irr::u8* datapntr = (irr::u8*)character.texture->lock(irr::video::ETLM_READ_ONLY);
        irr::u8* datapntr2 = (irr::u8*)hTex->lock(irr::video::ETLM_READ_WRITE);
        irr::u32 pitch = character.texture->getPitch();

        //all raw character pictures have the transparent color at the upper leftmost pixel (0,0)
        //get this value, as a reference where there is no pixel of the character itself
        //texelTrans = (irr::video::SColor *)(datapntr + ((0 * pitch) + (0 * sizeof(irr::video::SColor))));
        texelTrans = &character.transColor;

        irr::video::SColor* texel;
        irr::video::SColor* wTexel;

        //iterate through all pixels and copy them into the helper texture
        for (irr::s32 x = 0; x < character.sizeRawTex.Width; x++) {
            for (irr::s32 y = 0; y < character.sizeRawTex.Height; y++) {
                //texel is the pixel color at position x and y

                texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                wTexel = (irr::video::SColor *)(datapntr2 + ((y * pitch) + (x * sizeof(irr::video::SColor))));

                //copy original picture into the helper texture
                *wTexel = *texel;

                //try to establish text color
                if (textColFound == false) {
                    if (*texel != *texelTrans) {
                        //remember text color we found
                        texelTextColor = *texel;
                        textColFound = true;
                    }
                }
            }
        }

        //unlock original texture again!
        character.texture->unlock();

        //if we did not find textcolor exit here
        //handle it as pass
        if (!textColFound) {
            hTex->unlock();
            return true;
        }

        //now lock original texture for write access
        datapntr = (irr::u8*)character.texture->lock(irr::video::ETLM_WRITE_ONLY);
        irr::video::SColor* texel1;
        irr::video::SColor* texel2;
        irr::video::SColor* texel3;
        irr::video::SColor* texel4;
        irr::video::SColor* texel5;
        irr::video::SColor* texel6;
        irr::video::SColor* texel7;
        irr::video::SColor* texel8;
        irr::video::SColor* texelp;
        bool outLine;

        //now iterate again, create outline pixel by pixel, and store result in original texture
        for (irr::s32 x = 1; x < character.sizeRawTex.Width - 1; x++) {
            for (irr::s32 y = 1; y < character.sizeRawTex.Height -1 ; y++) {
                //texel is the pixel color at position x and y
                 irr::video::SColor* wTexel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));

                texel1 = (irr::video::SColor *)(datapntr2 + (((y-1) * pitch) + ((x-1) * sizeof(irr::video::SColor))));
                texel2 = (irr::video::SColor *)(datapntr2 + (((y-1) * pitch) + ((x) * sizeof(irr::video::SColor))));
                texel3 = (irr::video::SColor *)(datapntr2 + (((y-1) * pitch) + ((x+1) * sizeof(irr::video::SColor))));
                texel4 = (irr::video::SColor *)(datapntr2 + (((y) * pitch) + ((x-1) * sizeof(irr::video::SColor))));
                texel5 = (irr::video::SColor *)(datapntr2 + (((y) * pitch) + ((x + 1) * sizeof(irr::video::SColor))));
                texel6 = (irr::video::SColor *)(datapntr2 + (((y+1) * pitch) + ((x-1) * sizeof(irr::video::SColor))));
                texel7 = (irr::video::SColor *)(datapntr2 + (((y+1) * pitch) + ((x) * sizeof(irr::video::SColor))));
                texel8 = (irr::video::SColor *)(datapntr2 + (((y+1) * pitch) + ((x+1) * sizeof(irr::video::SColor))));

                texelp = (irr::video::SColor *)(datapntr2 + (((y) * pitch) + ((x) * sizeof(irr::video::SColor))));

                //an outline pixel must have at least one neighboring pixel that is not transparent colored
                outLine = ((*texel1 != *texelTrans ) || (*texel2 != *texelTrans ) || (*texel3 != *texelTrans ) ||
                        (*texel4 != *texelTrans ) || (*texel5 != *texelTrans ) || (*texel6 != *texelTrans ) || (*texel7 != *texelTrans )
                        || (*texel8 != *texelTrans ));

                //and an outline pixel can not have the text color itself!
                outLine &= (*texelp != texelTextColor);

                //if this pixel belongs to an outline, set the outline color there
                if (outLine) {
                    *wTexel = *outLineColor;
                }
            }
        }

        //unlock textures again!
        character.texture->unlock();
        hTex->unlock();

        return true;
  } else {
      //unsupported pixel color format!
      return false;
  }
}

//Loads a game font from the extracted (many) character image files
//Parameters:
//  filename = beginning of image files filename and relative path
//  numOffset = value to tell the function offset of first number in filename for first
//              character to be loaded
//  numChars = value to tell the function how many characters need to be loaded
//             character image filenames contain incrementing number at the end
//  loadAddFileNr = allows to specify additional files for loading character images file offsets
//                  at the end
//In case of an unexpected error this function returns NULL
GameTextFont* GameText::LoadGameFont(char* fileName, unsigned long numOffset, unsigned long numChars, std::vector<int> loadAddFileNr,
                                     bool addOutline, irr::video::SColor* outLineColor) {
  myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

  char finalpath[70];
  char fname[20];
  bool successFlag;

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

  //for (ulong idx = numOffset; idx < (numChars + numOffset); idx++) {
  for (it2 = loadFileNumbers.begin(); it2 != loadFileNumbers.end(); ++it2) {
      //build current filename
      strcpy(finalpath, fileName);
      sprintf (fname, "%0*lu.bmp", 4, (*it2));
      strcat(finalpath, fname);

      //load a new character for the new font
      GameTextCharacterInfo* newCharInfo = new GameTextCharacterInfo;

      newCharInfo->texture = myDriver->getTexture(finalpath);

      if (newCharInfo->texture == NULL) {
          //there was a texture loading error
          //just return with NULL
          return NULL;
      }

      newCharInfo->sizeRawTex = newCharInfo->texture->getSize();

      //myDriver->makeColorKeyTexture(newCharInfo->texture, irr::core::dimension2d(0,0));

      //try to establish most likely transparent color of char
      DeriveTransparentColorForChar(*newCharInfo);

      //should we add an outline to the character?
      if (addOutline && (outLineColor != NULL)) {
          if (!AddColoredOutline(*newCharInfo, outLineColor)) {
              return NULL;
          }
      }

      //find out where in the image the character is actually,
      //to remove transparent additional columns of pixels
      //the game seems to do the same somehow (in screenshots there is
      //always only a line of transparent pixels between characters)
      newCharInfo->charRect = FindCharArea(newCharInfo, successFlag);

      myDriver->makeColorKeyTexture(newCharInfo->texture, newCharInfo->transColor);

      if (successFlag != true) {
        //we had an unexpected error in FindCharArea, just return NULL and stop
        return NULL;
      }

      //add new character to our vector of characters for this font
      newFont->CharacterVector.push_back(newCharInfo);
  }

  myDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

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
          this->myDriver->removeTexture(pntr->texture);

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
  if (GameTextInitializedOk && (whichFont != NULL)) {
    char* pntr = &text[0];
    irr::core::vector2di correctCharPosition = position;
    irr::s16 charCnter = stopAfterNrChars;

    while ((*pntr != 0) && ((charCnter > 0) || (stopAfterNrChars == -1))) {
        //draw current character
        myDriver->draw2DImage(whichFont->CharacterVector[*pntr]->texture, correctCharPosition,
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
    if (GameTextInitializedOk && (whichFont != NULL)) {

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
    if (GameTextInitializedOk && (whichFont != NULL)) {

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
    if (GameTextInitializedOk && (whichHudFont != NULL)) {
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
              myDriver->draw2DImage(whichHudFont->CharacterVector[index]->texture, correctCharPosition,
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
  if (GameTextInitializedOk && (whichFont != NULL)) {
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
            myDriver->draw2DImage(whichFont->CharacterVector[index]->texture, correctCharPosition,
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
    if (GameTextInitializedOk && (whichFont != NULL)) {
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

//Constructor, initialization of all available and needed GameText fonts
//Parameters:
//  device = pointer to Irrlicht graphics device
//  driver = pointer to Irrlicht Video driver
// In case of unexpected initialization issue this constructor sets public
// member GameTextInitializedOk to false, and to true otherwise.
GameText::GameText(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver) {
    myDriver = driver;
    myDevice = device;
    this->GameTextInitializedOk = true;

    irr::video::SColor* outLineColor = new irr::video::SColor(255, 4, 4, 8);
    std::vector<int> addFileOffs = {};

    //load white Hud banner text font, 241 characters need to be loaded
    HudWhiteTextBannerFont = LoadGameFont((char*)"extract/fonts/large/olfnt0-1-", 0, 241, addFileOffs, true, outLineColor);

    //was there are problem loading the text font?
    if (HudWhiteTextBannerFont == NULL) {
        GameTextInitializedOk = false;
    }

    //load white Hud banner text font smaller (SVGA), 241 characters need to be loaded
    GameMenueWhiteTextSmallSVGA = LoadGameFont((char*)"extract/fonts/smallsvga/osfnt0-1-", 0, 241, addFileOffs, true, outLineColor);

    //was there are problem loading the text font?
    if (GameMenueWhiteTextSmallSVGA == NULL) {
        GameTextInitializedOk = false;
    }

    //load big green Hud text font, 241 characters need to be loaded
    HudBigGreenText = LoadGameFont((char*)"extract/fonts/largegreen/pfont0-1-", 0, 241, addFileOffs, false);

    //was there are problem loading the text font?
    if (HudBigGreenText == NULL) {
        GameTextInitializedOk = false;
    }

    //load Hud laptime number fonts in red, only 12 characters need to be loaded
    //also add graphical symbol with the 2 red arrows (which is used next to the current lap numbers)
    std::vector<int> addFileOffs2 = {226};
    HudLaptimeNumberRed = LoadGameFont((char*)"extract/hud1player/panel0-1-", 138, 12, addFileOffs2, false);

    //was there are problem loading the text font?
    if (HudLaptimeNumberRed == NULL) {
        GameTextInitializedOk = false;
    }

    //load Hud laptime number fonts in grey, only 12 characters need to be loaded
    HudLaptimeNumberGrey = LoadGameFont((char*)"extract/hud1player/panel0-1-", 150, 12, addFileOffs, false);

    //was there are problem loading the text font?
    if (HudLaptimeNumberGrey == NULL) {
        GameTextInitializedOk = false;
    }

    //add also the skull graphical symbol
    std::vector<int> addFileOffs3 = {227};
    HudKillCounterNumberRed = LoadGameFont((char*)"extract/hud1player/panel0-1-", 117, 10, addFileOffs3, false);

    irr::video::SColor* outLineColor2 = new irr::video::SColor(255, 40, 65, 56);

    //load font we created ourself before for unselected items in mainmenue
    GameMenueUnselectedEntryFont = LoadGameFont((char*)"extract/fonts/largegreenish/green-olfnt0-1-", 0, 241, addFileOffs, true, outLineColor2);

    //was there are problem loading the text font?
    if (GameMenueUnselectedEntryFont == NULL) {
        GameTextInitializedOk = false;
    }

    //load font we created ourself before for unselected items in mainmenue (based on smaller text size)
    GameMenueUnselectedTextSmallSVGA = LoadGameFont((char*)"extract/fonts/smallsvgagreenish/green-osfnt0-1-", 0, 241, addFileOffs, true, outLineColor2);

    //was there are problem loading the text font?
    if (GameMenueUnselectedTextSmallSVGA == NULL) {
        GameTextInitializedOk = false;
    }

    //load very small green target description font of HUD
    HudTargetNameGreen = LoadGameFont((char*)"extract/hud1player/panel0-1-", 231, 26, addFileOffs, false);

    //was there are problem loading the text font?
    if (HudTargetNameGreen == NULL) {
        GameTextInitializedOk = false;
    }

    //load very small red target description font of HUD
    HudTargetNameRed = LoadGameFont((char*)"extract/hud1player/panel0-1-", 200, 26, addFileOffs, false);

    //was there are problem loading the text font?
    if (HudTargetNameRed == NULL) {
        GameTextInitializedOk = false;
    }

    //load white thin font
    ThinWhiteText = LoadGameFont((char*)"extract/fonts/thinwhite/hfont0-0-", 0, 127, addFileOffs, false);

    //was there are problem loading the text font?
    if (ThinWhiteText == NULL) {
        GameTextInitializedOk = false;
    }

    delete outLineColor;
    delete outLineColor2;
}

GameText::~GameText() {
    //cleanup all fonts
    FreeTextFont(*HudWhiteTextBannerFont);
    HudWhiteTextBannerFont = NULL;

    FreeTextFont(*HudBigGreenText);
    HudBigGreenText = NULL;

    FreeTextFont(*HudLaptimeNumberRed);
    HudLaptimeNumberRed = NULL;

    FreeTextFont(*HudLaptimeNumberGrey);
    HudLaptimeNumberGrey = NULL;

    FreeTextFont(*HudKillCounterNumberRed);
    HudKillCounterNumberRed = NULL;

    FreeTextFont(*GameMenueUnselectedEntryFont);
    GameMenueUnselectedEntryFont = NULL;

    FreeTextFont(*HudTargetNameGreen);
    HudTargetNameGreen = NULL;

    FreeTextFont(*HudTargetNameRed);
    HudTargetNameRed = NULL;

    FreeTextFont(*ThinWhiteText);
    ThinWhiteText = NULL;

    FreeTextFont(*GameMenueWhiteTextSmallSVGA);
    GameMenueWhiteTextSmallSVGA = NULL;

    FreeTextFont(*GameMenueUnselectedTextSmallSVGA);
    GameMenueUnselectedTextSmallSVGA = NULL;
}
