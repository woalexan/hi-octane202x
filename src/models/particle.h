/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <irrlicht.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "EDebugSceneTypes.h"
#include "player.h"

class Player; //Forward declaration

class SpriteParticle {
public:
    SpriteParticle(irr::scene::ISceneManager* smgr, irr::video::ITexture* spriteTex, irr::core::vector3d<irr::f32> startLocation,
                   irr::f32 lifeTimeSec, irr::core::dimension2d<irr::f32> initSize, irr::core::dimension2d<irr::f32> endLifeSize);

    virtual ~SpriteParticle();

    //Update returns true if particle still has remaining lifetime
    //returns false if lifetime is over
    virtual bool Update(irr::f32 frameDeltaTime);

    void ShowParticle();
    void HideParticle();

    irr::f32 mLifeTimeSec;
    bool mIsReset = false;

    protected:
     irr::scene::ISceneManager* mSmgr;
     bool mVisible = false;
     bool mLastVisible = false;

     irr::core::vector3d<irr::f32> mParticlePos;
     irr::video::ITexture* mSpriteTex;
     irr::core::dimension2d<irr::u32> mSpriteTexSize;

     irr::scene::IBillboardSceneNode* mSceneNode;

     //initial sprite size after creation
     irr::core::dimension2d<irr::f32> mInitSize;

     //max sprite size at the end of life
     irr::core::dimension2d<irr::f32> mEndLifeSize;

     irr::core::dimension2d<irr::f32> mCurrParticleSize;
     irr::f32 lifeTimeSecParam;

     //to be able to reset particle
     //we need to store initial settings
     irr::core::vector3d<irr::f32> startLocationParam;
     virtual void ResetParticle();
};

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
    SteamFountain(EntityItem* entityItem, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, irr::core::vector3d<irr::f32> location,
                  irr::u32 nrMaxParticles);

    ~SteamFountain();

    void TriggerUpdate(irr::f32 frameDeltaTime);
    void Activate();

    //if triggered the steam Fountain is
    //enabled
    void Trigger();

    EntityItem* mEntityItem;

private:
    irr::core::vector3d<irr::f32> mPosition;
    bool mVisible;
    bool mActivated = false;

    irr::video::ITexture* mSteamTex;
    irr::core::dimension2d<irr::u32> mSteamTexSize;

    irr::scene::ISceneManager* mSmgr;
    irr::video::IVideoDriver* mDriver;

    irr::u32 mNrMaxParticles;
    irr::u32 mCurrNrParticels = 0;

    irr::f32 absTimeSinceLastActivation;
    irr::f32 absTimeSinceLastUpdate = 0.0f;

    std::vector<SteamParticle*>* mCurrSpriteVec;

    void CreateParticle() {

    }
};

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

    Player* mParentPlayer;

    irr::video::ITexture* mSmokeTex;
    irr::core::dimension2d<irr::u32> mSmokeTexSize;

    irr::scene::ISceneManager* mSmgr;
    irr::video::IVideoDriver* mDriver;

    irr::u32 mNrMaxParticles;
    irr::u32 mCurrNrParticels = 0;

    irr::f32 absTimeSinceLastActivation;
    irr::f32 absTimeSinceLastUpdate = 0.0f;

    std::vector<SmokeParticle*>* mCurrSpriteVec;

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

    Player* mParentPlayer;

    irr::video::ITexture* mDustTex;
    irr::core::dimension2d<irr::u32> mDustTexSize;

    irr::scene::ISceneManager* mSmgr;
    irr::video::IVideoDriver* mDriver;

    irr::u32 mNrMaxParticles;
    irr::u32 mCurrNrParticels = 0;

    irr::f32 absTimeSinceLastActivation;
    irr::f32 absTimeSinceLastUpdate = 0.0f;

    std::vector<DustParticle*>* mCurrSpriteVec;

    void CreateParticle() {
    }
};

#endif // PARTICLE_H
