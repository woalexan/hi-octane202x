/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

//This source code was implemented by me based on the insight I gained into the original game. Some parts do more closely follow
//the Pseudo-C Code that is available to me, and other parts are heavily modified to hopefully work inside my existing project
//in the near future.
//The original game uses fixed point arithmetic for performance reasons (it had to run on 80486 after all), and is optimized everywhere you
//look at. I do not want to do the same, because the fixed point arithmetic makes it very difficult to read and understand. And performance should not
//be the problem nowadays. Therefore the biggest change which I have to do is to change all the logic to floating point arithmetic,
//while hopefully not breaking the concept behind everything.
//Additionally I will only try to reimplement the most important mechanisms and concepts from the original game where it benefits the overall project,
//but I will not change the overall project to use the same structs and data structures as the original. This would not work, and also does not
//make sense in my opinion.

//Important note: What makes this source code very difficult to handle is the fact, that my coordinate system in this existing project is
//completely different to the one in the original game. The original uses X and Y axis for the tile map, and Z is the height.
//For the levelfile and 2D map stuff I also use X any Y axis for the tile map most of the time.
//My 3D world setup (for rendering) using Irrlicht has X and Z for the tile map, and Y is the height. And to make things worse my Irrlicht vertice X and Y coordinates
//have a swapped sign (are negative) currently.

//I have decided to also use the original games coordinate system in all vanilla calculations. At the interface between
//original game calculations and Irrlicht 3D coordinate system I have then to convert from one coordinate system setup to the other.
//Thats the drawback I will have.

//I really want to thank aybe for giving me the opportunity to look much deeper into the original game inner workings as I was ever able before.
//Without this support I would not have been able to hopefully advance the current project more true to the original.

#ifndef VEFFECTMANAGER_H
#define VEFFECTMANAGER_H

#include "irrlicht.h"
#include <cstdint>
#include <vector>

/************************
 * Forward declarations *
 ************************/

class Race;
struct VThing;

enum EffectType {
          Undefined, Smoke, ExplosionSmall, ExplosionMedium, SmokeFire, Flare, ExplosionBig
};

struct EffectInfoStruct {
    EffectType effectType = Undefined;
    VThing* thingPntr = nullptr;
    irr::scene::IBillboardSceneNode* sceneNode = nullptr;
    size_t currDrawNr = 0;
    irr::video::SColor currVerticeColor;
    bool readyForCleanup = false;
};

class VEffectManager {

private:
    Race* mParentRace = nullptr;

    void InitSceneNode(EffectInfoStruct* whichInfoStruct, irr::video::ITexture* firstTexture, irr::core::dimension2df sizeSprite);
    void UpdateSceneNode(EffectInfoStruct* whichInfoStruct, irr::video::ITexture* newTexture);

    void UpdateEffect(EffectInfoStruct* whichInfoStruct);
    void CleanupEffect(EffectInfoStruct* whichInfoStruct);

    void initialiseEFFECT_EXPLOSION(VThing* whichThing);
    void processEFFECT_EXPLOSION(EffectInfoStruct* whichInfoStruct);

    void initialiseEFFECT_EXPLOSION_MEDIUM(VThing* whichThing);
    void processEFFECT_EXPLOSION_MEDIUM(EffectInfoStruct* whichInfoStruct);

    void initialiseEFFECT_SMOKE(VThing* whichThing);
    void processEFFECT_SMOKE(EffectInfoStruct* whichInfoStruct);

    void initialiseEFFECT_SMOKE_FIRE(VThing* whichThing);
    void processEFFECT_SMOKE_FIRE(EffectInfoStruct* whichInfoStruct);

    void initialiseEFFECT_FLARE(VThing* whichThing);
    void processEFFECT_FLARE(EffectInfoStruct* whichInfoStruct);

    irr::core::vector3df* search_start_level(int32_t start, int32_t stop);
    irr::core::vector3df* search_next();

    void initialiseEFFECT_EXPLOSION_BIG(VThing* whichThing);
    void processEFFECT_EXPLOSION_BIG(EffectInfoStruct* whichInfoStruct);

    irr::video::ITexture* mSmokeTex = nullptr;

    irr::video::ITexture* mSpriteTex = nullptr;
    irr::core::dimension2d<irr::u32> mSpriteTexSize;

    irr::core::array<irr::video::ITexture*> animTexListExplosion;

    bool LoadSprites();

    std::vector<EffectInfoStruct*> mActiveEffectVec;
    std::vector<EffectInfoStruct*> mNewEffectVec;

    irr::f32 mAbsTimeAcc = 0.0f;

public:
    VEffectManager(Race* parentRace);
    ~VEffectManager();

    VThing* Explosion = nullptr;

    //Returns a pointer to the Thing for the Effect
    EffectInfoStruct* AddEffect(EffectType whichEffect, irr::core::vector3df location, irr::f32 angleXY,
                   irr::f32 angleZY, irr::f32 angleXZ, int16_t id);

    void Update(irr::f32 frameDeltaTime);
    uint16_t GetNrCurrentlyActiveEffects();
};

#endif // VEFFECTMANAGER_H
