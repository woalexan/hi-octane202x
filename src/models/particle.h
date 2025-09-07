/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <irrlicht.h>
#include <vector>
#include "spriteparticle.h"

const irr::f32 DEF_DUSTPARTICLELIFETIME = 2.0f;  //0.5f before debugging, reset back to this value!

/************************
 * Forward declarations *
 ************************/

class Player;
class Race;
class EntityItem;

class SmokeParticle: public SpriteParticle {
public:
    SmokeParticle(irr::scene::ISceneManager* smgr, irr::video::ITexture* spriteTex, irr::core::vector3d<irr::f32> startLocation,
                  irr::f32 lifeTimeSec, irr::core::dimension2d<irr::f32> initSize, irr::core::dimension2d<irr::f32> endLifeSize) :
        SpriteParticle(smgr, spriteTex, startLocation, lifeTimeSec, initSize, endLifeSize) {
    }

    ~SmokeParticle();

    //Update returns true if particle still has remaining lifetime
    //returns false if lifetime is over
    bool Update(irr::f32 frameDeltaTime, irr::core::vector3df newStartLocation);

    void ResetParticle(irr::core::vector3df newStartLocation);
};

class SmokeTrail {
public:
    SmokeTrail(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, Player* parentPlayer,
                  irr::u32 nrMaxParticles);

    ~SmokeTrail();

    void Update(irr::f32 frameDeltaTime);
    void Activate();
    void Deactivate();

private:
    bool mVisible;
    bool mActivated = false;

    Player* mParentPlayer = nullptr;

    irr::video::ITexture* mSmokeTex = nullptr;
    irr::core::dimension2d<irr::u32> mSmokeTexSize;

    irr::scene::ISceneManager* mSmgr = nullptr;
    irr::video::IVideoDriver* mDriver = nullptr;

    irr::u32 mNrMaxParticles;
    irr::u32 mCurrNrParticels = 0;

    irr::f32 absTimeSinceLastActivation;
    irr::f32 absTimeSinceLastUpdate = 0.0f;

    std::vector<SmokeParticle*>* mCurrSpriteVec = nullptr;

    void CreateParticle() {
    }
};

class DustParticle: public SpriteParticle {
public:
    DustParticle(irr::scene::ISceneManager* smgr, irr::video::ITexture* spriteTex, irr::core::vector3d<irr::f32> startLocation,
                  irr::f32 lifeTimeSec, irr::core::dimension2d<irr::f32> initSize, irr::core::dimension2d<irr::f32> endLifeSize) :
        SpriteParticle(smgr, spriteTex, startLocation, lifeTimeSec, initSize, endLifeSize) {
    }

    ~DustParticle();

    //Update returns true if particle still has remaining lifetime
    //returns false if lifetime is over
    bool Update(irr::f32 frameDeltaTime, irr::core::vector3df newStartLocation);

    void ResetParticle(irr::core::vector3df newStartLocation);
};

class DustBelowCraft {
public:
    DustBelowCraft(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, Player* parentPlayer,
                  irr::u32 nrMaxParticles);

    ~DustBelowCraft();

    void Update(irr::f32 frameDeltaTime);
    void Activate();
    void Deactivate();

private:
    bool mVisible;
    bool mActivated = false;

    Player* mParentPlayer = nullptr;

    irr::video::ITexture* mDustTex = nullptr;
    irr::core::dimension2d<irr::u32> mDustTexSize;

    irr::scene::ISceneManager* mSmgr = nullptr;
    irr::video::IVideoDriver* mDriver = nullptr;

    irr::u32 mNrMaxParticles;
    irr::u32 mCurrNrParticels = 0;

    irr::f32 absTimeSinceLastActivation;
    irr::f32 absTimeSinceLastUpdate = 0.0f;

    std::vector<DustParticle*>* mCurrSpriteVec = nullptr;

    void SetupVelocityParticle(DustParticle* particlePntr);
};

#endif // PARTICLE_H
