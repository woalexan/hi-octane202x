/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef SPRITEPARTICLE_H
#define SPRITEPARTICLE_H

#include <irrlicht.h>
#include <vector>

/************************
 * Forward declarations *
 ************************/

class SpriteParticle {
public:
    SpriteParticle(irr::scene::ISceneManager* smgr, irr::video::ITexture* spriteTex, irr::core::vector3d<irr::f32> startLocation,
                   irr::f32 lifeTimeSec, irr::core::dimension2d<irr::f32> initSize, irr::core::dimension2d<irr::f32> endLifeSize);

    virtual ~SpriteParticle();

    //Update returns true if particle still has remaining lifetime
    //returns false if lifetime is over
    virtual bool Update(irr::f32 frameDeltaTime);

    //allows to setup movement velocity for this particle
    void SetMovemenVelocity(irr::core::vector3df initialVelocity, irr::core::vector3df finalVelocity);

    void ShowParticle();
    void HideParticle();

    irr::f32 mLifeTimeSec;
    bool mIsReset = true;

    protected:
     irr::scene::ISceneManager* mSmgr = nullptr;
     bool mVisible = false;
     bool mLastVisible = false;

     irr::core::vector3d<irr::f32> mParticlePos;
     irr::video::ITexture* mSpriteTex = nullptr;
     irr::core::dimension2d<irr::u32> mSpriteTexSize;

     irr::scene::IBillboardSceneNode* mSceneNode = nullptr;
     irr::video::SColor mCurrVerticeColor;

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

     //allows to set an initial movement velocity for the
     //particle at the beginning of the life time
     irr::core::vector3df mInitialMovementVelocity;

     //allows to set a final movement velocity for the
     //particle at the end of the life time, for times inbetween
     //the movementDir will be interpolated
     irr::core::vector3df mFinalMovementVelocity;

     //default we do not use movement velocity option
     bool mUseMovementVelocity = false;
};

#endif // SPRITEPARTICLE_H
