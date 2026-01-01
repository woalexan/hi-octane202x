/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2026 Wolf Alexander       (I did translation to C++, and then modified and extended code for my project)
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

    EntityItem* Source = nullptr;
    EntityItem* Target = nullptr;

    //A permanent morph means the morph continiously
    //is running in the game; does never stop
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

    //if a morph is triggered, it will morph
    //the terrain/columns
    //The permament morph will start running initially
    //when triggered first and then run forever
    //A MorphOnce will move to the other state once when triggered, and then
    //wait again for the next trigger. Then it will move back
    void Trigger();

    void Update(irr::f32 frameDeltaTime);

    irr::f32 absTimeMorph;

    bool mCurrMorphing = false;
    bool mMorphDirectionUp = true;
   
private:
    irr::f32 progress = 0.0f;

    int myEntityId;

    LevelTerrain* mLevelTerrain = nullptr;
    LevelBlocks* mLevelBlocks = nullptr;
};

#endif // MORPH_H
