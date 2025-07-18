/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did translation to C++, and then modified and extended code for my project)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MORPH_H
#define MORPH_H

#include <irrlicht.h>
#include <vector>

/************************
 * Forward declarations *
 ************************/

class Column;
class LevelTerrain;
class LevelBlocks;
class EntityItem;
class LevelTerrain;
class LevelBlocks;

class Morph {
public:
    Morph(int myEntityID, EntityItem* source, EntityItem* target, int width, int height, bool permanent,
          LevelTerrain* levelTerrain, LevelBlocks* levelblocks);
    ~Morph();

    bool Enabled = true;

    EntityItem* Source = nullptr;
    EntityItem* Target = nullptr;

    bool Permanent;
    bool UVSFromSource = false;

    std::vector<Column*> Columns;

    irr::f32 LastProgress;

    float getProgress();
    void setProgress(irr::f32 newProgress);

    //width and height from 2D top view
    int Width;
    int Height;

    void MorphColumns();

    //if morph is triggered, it will morph
    //the terrain/columns
    void Trigger();

    void Update(irr::f32 frameDeltaTime);

    irr::f32 absTimeMorph;

    bool mCurrMorphing = false;
    bool mMorphDirectionUp = true;

    bool mPermanentOnceFired = false;
   
private:
    irr::f32 progress = 0.0f;

    int myEntityId;

    LevelTerrain* mLevelTerrain = nullptr;
    LevelBlocks* mLevelBlocks = nullptr;
};

#endif // MORPH_H
