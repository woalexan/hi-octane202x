/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef STEAMFOUNTAIN_H
#define STEAMFOUNTAIN_H

#include <irrlicht.h>
#include <vector>
#include "spriteparticle.h"

/************************
 * Forward declarations *
 ************************/

class EntityItem;

class SteamParticle: public SpriteParticle {
public:
    SteamParticle(irr::scene::ISceneManager* smgr, irr::video::ITexture* spriteTex, irr::core::vector3d<irr::f32> startLocation,
                  irr::f32 lifeTimeSec, irr::core::dimension2d<irr::f32> initSize, irr::core::dimension2d<irr::f32> endLifeSize) :
        SpriteParticle(smgr, spriteTex, startLocation, lifeTimeSec, initSize, endLifeSize) {
    }

    ~SteamParticle();

    //Update returns true if particle still has remaining lifetime
    //returns false if lifetime is over
    bool Update(irr::f32 frameDeltaTime);
    void ResetParticle();
};

class SteamFountain {
public:
    SteamFountain(irr::video::ITexture* steamTex, EntityItem* entityItem, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, irr::core::vector3d<irr::f32> location,
                  irr::u32 nrMaxParticles);

    ~SteamFountain();

    void TriggerUpdate(irr::f32 frameDeltaTime);
    void Activate();

    //if triggered the steam Fountain is
    //enabled
    void Trigger();

    EntityItem* mEntityItem = nullptr;

    void Hide();
    void Show();

private:
    irr::core::vector3d<irr::f32> mPosition;
    bool mVisible;
    bool mActivated = false;

    irr::video::ITexture* mSteamTex = nullptr;
    irr::core::dimension2d<irr::u32> mSteamTexSize;

    irr::scene::ISceneManager* mSmgr = nullptr;
    irr::video::IVideoDriver* mDriver = nullptr;

    irr::u32 mNrMaxParticles;
    irr::u32 mCurrNrParticels = 0;

    irr::f32 absTimeSinceLastActivation;
    irr::f32 absTimeSinceLastUpdate = 0.0f;

    std::vector<SteamParticle*>* mCurrSpriteVec = nullptr;
};

#endif // STEAMFOUNTAIN_H
