/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "irrlicht.h"
#include <vector>

using namespace irr::core;

/************************
 * Forward declarations *
 ************************/

class InfrastructureBase;

class TextureLoader {

public:
    int NumLevelTextures;
    int NumSpriteTextures;
    int NumCustomLevelTextures;

    int NumEditorTextures;

    std::vector<irr::video::ITexture*> levelTex;
    std::vector<irr::video::ITexture*> spriteTex;
    std::vector<irr::video::ITexture*> editorTex;
    TextureLoader(InfrastructureBase* infra, const char* filePathLevelRootDir, const char* filePathBaseTextures,
                  bool useCustomTextures, const char* spriteTexFilePath, bool loadLevelEditorSprites = false);
    ~TextureLoader();

    void LoadLevelTextures(const char* filePathLevelRootDir, const char* filePathBaseTextures);
    void LoadSpriteTextures(const char* filePath, bool makeTransparent = false);
    void LoadEditorTextures();

    bool mLoadSuccess = true;

private:
    InfrastructureBase* mInfra = nullptr;

    bool mUseCustomTextures;

    void LoadEditorTexture(const char* fileName, bool makeTransparent = false);
};

#endif // TEXTURE_H
