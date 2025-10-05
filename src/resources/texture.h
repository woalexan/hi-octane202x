/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "irrlicht.h"
#include <vector>

using namespace irr::core;

class TextureLoader {

public:
    int NumLevelTextures;
    int NumSpriteTextures;

    int NumEditorTextures;

    std::vector<irr::video::ITexture*> levelTex;
    std::vector<irr::video::ITexture*> spriteTex;
    std::vector<irr::video::ITexture*> editorTex;
    TextureLoader(irr::video::IVideoDriver* myDriver,  const char* levelTexFilePath,
                  const char* spriteTexFilePath, bool loadLevelEditorSprites = false);
    ~TextureLoader();

    void LoadLevelTextures(const char* filePath);
    void LoadSpriteTextures(const char* filePath, bool makeTransparent = false);
    void LoadEditorTextures();

    bool mLoadSuccess = true;

private:
    irr::video::IVideoDriver *m_driver = nullptr;

    void LoadEditorTexture(const char* fileName, bool makeTransparent = false);
};

#endif // TEXTURE_H
