/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "texture.h"

void TextureLoader::LoadLevelTextures(char* filePath) {
    int currTexIdx = 0;
    char finalpath[70];
    char fname[20];
    irr::video::ITexture* newTex;

    NumLevelTextures = 0;

    //there are 256 texture files to load
    for (currTexIdx = 0; currTexIdx < 256; currTexIdx++) {
        //create filename
        strcpy(finalpath, filePath);
        sprintf (fname, "%0*d.png", 4, currTexIdx);
        strcat(finalpath, fname);

        //loading the specified terrain texture file
        newTex = m_driver->getTexture(finalpath);

        //add new texture to texture vector
        this->levelTex.push_back(newTex);

        NumLevelTextures++;
    }
}

void TextureLoader::LoadSpriteTextures(char* filePath) {
    int currTexIdx = 0;
    char finalpath[70];
    char fname[20];
    irr::video::ITexture* newTex;

    NumSpriteTextures = 0;

    //there are 256 texture files to load
    for (currTexIdx = 0; currTexIdx < 48; currTexIdx++) {
        //create filename
        strcpy(finalpath, filePath);
        sprintf (fname, "%0*d.png", 4, currTexIdx);
        strcat(finalpath, fname);

        //loading the specified sprite texture file
        newTex = m_driver->getTexture(finalpath);

        //add new texture to sprite vector
        this->spriteTex.push_back(newTex);

        NumSpriteTextures++;
    }
}

TextureLoader::TextureLoader(irr::video::IVideoDriver* myDriver, char* levelTexFilePath,  char* spriteTexFilePath) {
   m_driver = myDriver;

   //load all level textures
   LoadLevelTextures(levelTexFilePath);

   //load all sprite textures
   LoadSpriteTextures(spriteTexFilePath);
}

TextureLoader::~TextureLoader() {
   //free all loaded textures again
   if (levelTex.size() > 0) {
       std::vector<irr::video::ITexture*>::iterator it;
       irr::video::ITexture* pntr;

       for (it = levelTex.begin(); it != levelTex.end(); ) {
           pntr = (*it);

           it = levelTex.erase(it);

           //free texture via driver
           m_driver->removeTexture(pntr);
       }
   }

   //free all loaded sprites again
   if (spriteTex.size() > 0) {
       std::vector<irr::video::ITexture*>::iterator it;
       irr::video::ITexture* pntr;

       for (it = spriteTex.begin(); it != spriteTex.end(); ) {
           pntr = (*it);

           it = spriteTex.erase(it);

           //free texture via driver
           m_driver->removeTexture(pntr);
       }
   }
}
