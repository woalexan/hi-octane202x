/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "texture.h"
#include <string>
#include "../utils/logging.h"
#include "../infrabase.h"

void TextureLoader::LoadLevelTextures(const char* filePathLevelRootDir, const char* filePathBaseTextures) {
    int currTexIdx = 0;
    io::path finalPath;
    char fname[20];
    irr::video::ITexture* newTex;

    NumLevelTextures = 0;
    NumCustomLevelTextures = 0;

    //01.01.2026: Changed behavior: First build file name in level
    //root directory for each texture, to see if the user supplied
    //us with an alternative (replacement) texture file
    //if so, use this file instead for texture loading

    //if there is no replacement file in the level root dir,
    //use the default texture files in the specified texture base directory
    //from the map Xml config file

    //Create a list of files existing in specified level root dir
    irr::io::IFileList* fList = mInfra->CreateFileList(irr::io::path(filePathLevelRootDir));

    bool fallBackDefaultTex = false;

    if (fList == nullptr) {
        logging::Warning("LoadLevelTextures: Can not search for user specified texture files, Fallback to default textures");
        fallBackDefaultTex = true;
    }

    if (!mUseCustomTextures) {
        logging::Info("LoadLevelTextures: Custom textures disabled for this level, use default textures");
        fallBackDefaultTex = true;
    }

    io::path fileEnding;
    bool customTex;

    //there are 256 texture files to load
    for (currTexIdx = 0; currTexIdx < 256; currTexIdx++) {
        //create filename, let the file ending open
        //so that we can search for the texture file name
        //strcpy(finalpath, filePathLevelRootDir);
        sprintf (fname, "tex%0*d", 4, currTexIdx);
        //strcat(finalpath, fname);

        customTex = false;

        if (!fallBackDefaultTex) {
                //is there a file with this name?
                //See additional parameter true, we need to ignore the file ending!
                irr::io::path alternativeTexFile =
                        mInfra->LocateFileInFileList(fList, irr::core::string<fschar_t>(fname), true);

                if (!alternativeTexFile.empty()) {
                   //There is a possible alternative texture file to load, check further if file ending is usable
                   fileEnding = mInfra->GetFileEndingFromFileName(alternativeTexFile);

                   if (fileEnding.equals_ignore_case("png") || fileEnding.equals_ignore_case("jpg") ||
                           fileEnding.equals_ignore_case("bmp") || fileEnding.equals_ignore_case("tga")
                           || fileEnding.equals_ignore_case("pcx")) {
                       //file is usable, use this file instead
                       finalPath = alternativeTexFile;
                       NumCustomLevelTextures++;
                       customTex = true;
                   }
              }
        }

        if (!customTex) {
             finalPath = "";
             finalPath.append(filePathBaseTextures);
             finalPath.append("/");
             finalPath.append(fname);
             finalPath.append(".png");
        }

        //loading the specified terrain texture file
        newTex = mInfra->mDriver->getTexture(finalPath);

        if (newTex == nullptr) {
            char hlpstr[500];
            std::string msg("");

            //loading texture failed
            snprintf(hlpstr, 500, "Failed to load texture: %s", finalPath.c_str());
            msg.clear();
            msg.append(hlpstr);
            logging::Error(msg);

            mLoadSuccess = false;

            if (!fallBackDefaultTex) {
                //drop the file list again
                //not that we get a memory leak!
                fList->drop();
            }

            return;
        }

        //add new texture to texture vector
        this->levelTex.push_back(newTex);

        NumLevelTextures++;
    }

    char hlpstr[500];
    std::string msg("");

    snprintf(hlpstr, 500, "Loaded %d custom level textures", NumCustomLevelTextures);
    msg.clear();
    msg.append(hlpstr);
    logging::Info(msg);

    if (!fallBackDefaultTex) {
        //drop the file list again
        //not that we get a memory leak!
        fList->drop();
    }
}

void TextureLoader::LoadSpriteTextures(const char* filePath, bool makeTransparent) {
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
        newTex = mInfra->mDriver->getTexture(finalpath);

        if (newTex == nullptr) {
            char hlpstr[500];
            std::string msg("");

            //loading texture failed
            snprintf(hlpstr, 500, "Failed to load texture: %s", finalpath);
            msg.clear();
            msg.append(hlpstr);
            logging::Error(msg);

            mLoadSuccess = false;

            return;
        }

        if (makeTransparent) {
            //Take pixel 1,1 for transparent color
            mInfra->mDriver->makeColorKeyTexture(newTex, irr::core::vector2di(1,1));
        }

        //add new texture to sprite vector
        this->spriteTex.push_back(newTex);

        NumSpriteTextures++;
    }
}

void TextureLoader::LoadEditorTexture(const char* fileName, bool makeTransparent) {
    irr::video::ITexture* newTex;

    //load the specified texture file
    newTex = mInfra->mDriver->getTexture(fileName);

    if (newTex == nullptr) {
        char hlpstr[500];
        std::string msg("");

        //loading texture failed
        snprintf(hlpstr, 500, "Failed to load texture: %s", fileName);
        msg.clear();
        msg.append(hlpstr);
        logging::Error(msg);

        mLoadSuccess = false;

        return;
    }

    if (makeTransparent) {
        //Take pixel 1,1 for transparent color
        mInfra->mDriver->makeColorKeyTexture(newTex, irr::core::vector2di(1,1));
    }

    //add new texture to level editor sprite vector
    this->editorTex.push_back(newTex);

    NumEditorTextures++;
}

void TextureLoader::LoadEditorTextures() {
    NumEditorTextures = 0;

    LoadEditorTexture("media/editor/camera-small.png", true);
    LoadEditorTexture("media/editor/stopwatch-small.png", true);
    LoadEditorTexture("media/editor/speedo-slow-small.png", true);
    LoadEditorTexture("media/editor/speedo-fast-small.png", true);

    //image of original game: white arrow pointing to the right
    LoadEditorTexture("extract/hud1player/panel0-1-0308.png", true);

    LoadEditorTexture("media/editor/stopsign-small.png", true);
}

TextureLoader::TextureLoader(InfrastructureBase* infra, const char* filePathLevelRootDir, const char* filePathBaseTextures,
                             bool useCustomTextures, const char* spriteTexFilePath, bool loadLevelEditorSprites) {
   mInfra = infra;
   mUseCustomTextures = useCustomTextures;

   //load all level textures
   LoadLevelTextures(filePathLevelRootDir, filePathBaseTextures);

   //load all sprite textures
   //for level editor make the sprites transparent
   LoadSpriteTextures(spriteTexFilePath, loadLevelEditorSprites);

   if (loadLevelEditorSprites) {
       LoadEditorTextures();
   }
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
           mInfra->mDriver->removeTexture(pntr);
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
           mInfra->mDriver->removeTexture(pntr);
       }
   }

   //free all loaded editor sprites again
   if (editorTex.size() > 0) {
       std::vector<irr::video::ITexture*>::iterator it;
       irr::video::ITexture* pntr;

       for (it = editorTex.begin(); it != editorTex.end(); ) {
           pntr = (*it);

           it = editorTex.erase(it);

           //free texture via driver
           mInfra->mDriver->removeTexture(pntr);
       }
   }
}
