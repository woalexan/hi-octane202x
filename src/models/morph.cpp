/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did translation to C++, and then modified and extended code for my project)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "morph.h"
#include "levelterrain.h"
#include "levelblocks.h"
#include "column.h"

float Morph::getProgress() {
    return progress;
}

void Morph::setProgress(irr::f32 newProgress) {
    LastProgress = progress;
    progress = newProgress;
}

void Morph::MorphColumns() {
    std::vector<Column*>::iterator it;

    for (it = this->Columns.begin(); it != this->Columns.end(); ++it) {
        (*it)->ApplyMorph(getProgress());
    }
}

void Morph::Trigger() {
   if (Permanent && mPermanentOnceFired)
       return;

   mCurrMorphing = true;

   if (Permanent && !mPermanentOnceFired) {
       mPermanentOnceFired = true;
   }
}

void Morph::Update(irr::f32 frameDeltaTime) {
    if (!mCurrMorphing)
        return;

    absTimeMorph += frameDeltaTime;
    setProgress((float)fmin(1.0f, fmax(0.0f, 0.5f + sin(absTimeMorph))));

    mLevelTerrain->ApplyMorph((*this));
    MorphColumns();

    if (mMorphDirectionUp && (progress >= 0.99999f)) {
        mCurrMorphing = false;
        mMorphDirectionUp = false;
    }

    if (!mMorphDirectionUp && (progress <= 0.0001f)) {
        mCurrMorphing = false;
        mMorphDirectionUp = true;
    }
}

Morph::Morph(int myEntityID, EntityItem* source, EntityItem* target, int width, int height,
             bool permanent, LevelTerrain* levelTerrain, LevelBlocks* levelblocks) {
    Source = source;
    Target = target;
    Width = width;
    Height = height;
    LastProgress = 0.0f;
    Permanent = permanent;
    myEntityId = myEntityID;
    mLevelTerrain = levelTerrain;
    mLevelBlocks = levelblocks;
    absTimeMorph = 0.0f;
}

Morph::~Morph() {
}
