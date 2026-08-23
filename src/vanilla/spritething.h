/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef SPRITETHING_H
#define SPRITETHING_H

#include <irrlicht.h>
#include "vthing.h"
#include <vector>

//Forward declaration
class Race;

class SpriteThing {
public:
    //initPosition and initMovement are in the original games (vanilla) coordinate
    //system
    SpriteThing(irr::scene::ISceneManager* smgr, Race* race, irr::video::ITexture* spriteTex,
                irr::core::vector3d<irr::f32> initPosition, MovementStruct initMovement);

    virtual ~SpriteThing();

    //Update returns true if particle still has remaining lifetime
    //returns false if lifetime is over
    virtual bool Update(irr::f32 frameDeltaTime);

    VThing ThingData;

    //Is set to true if this SpriteThing
    //has reached End of Life and needs to be
    //deleted again
    bool IsExpired = false;

protected:
     irr::scene::ISceneManager* mSmgr = nullptr;
     Race* mRace = nullptr;

     irr::video::ITexture* mSpriteTex = nullptr;
     irr::core::dimension2d<irr::u32> mSpriteTexSize;

     irr::scene::IBillboardSceneNode* mSceneNode = nullptr;
     irr::video::SColor mCurrVerticeColor;

     void UpdateSceneNode();
};

class EffectVehicleThing: public SpriteThing {
public:
    //initPosition and initMovement are in the original games (vanilla) coordinate
    //system
    EffectVehicleThing(irr::scene::ISceneManager* smgr, Race* race, irr::video::ITexture* spriteTex,
                     irr::core::vector3d<irr::f32> initPosition, MovementStruct initMovement) :
                 SpriteThing(smgr, race, spriteTex, initPosition, initMovement) {
                 }

    ~EffectVehicleThing();

    //Update returns true if particle still has remaining lifetime
    //returns false if lifetime is over
    bool Update(irr::f32 frameDeltaTime);
};

#endif // SPRITETHING_H
