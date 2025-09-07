/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "spriteparticle.h"
#include "../resources/texture.h"

//***************************************************
//*   SpriteParticle Base class                     *
//***************************************************

SpriteParticle::SpriteParticle(irr::scene::ISceneManager* smgr, irr::video::ITexture* spriteTex, irr::core::vector3d<irr::f32> startLocation,
                   irr::f32 lifeTimeSec, irr::core::dimension2d<irr::f32> initSize, irr::core::dimension2d<irr::f32> endLifeSize) {
    mSmgr = smgr;
    mSpriteTex = spriteTex;

    mSpriteTexSize = mSpriteTex->getSize();
    mParticlePos = startLocation;
    mLifeTimeSec = lifeTimeSec;

    mInitSize = initSize;
    mEndLifeSize = endLifeSize;
    mCurrParticleSize = mInitSize;

    //store parameters also inside for later particle reset
    lifeTimeSecParam = lifeTimeSec;
    startLocationParam = startLocation;

    mSceneNode = smgr->addBillboardSceneNode();
    mSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
    mSceneNode->setMaterialTexture(0, mSpriteTex);
    mSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    mSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    //mSceneNode->setDebugDataVisible(irr::scene::EDS_BBOX);

    //I want a random number between 0.8f and 1.2f
    int rNum = rand();
    irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX))  * 0.4f;

    mSceneNode->setPosition(mParticlePos);
    mSceneNode->setSize(mInitSize * rNumFloat);

    //default set movementVelocity option to all 0
    //(=not used)
    mInitialMovementVelocity.set(0.0f, 0.0f, 0.0f);
    mFinalMovementVelocity.set(0.0f, 0.0f, 0.0f);

    //make the sprite completely visible
    mCurrVerticeColor.set(255, 255, 255, 255);
    mSceneNode->setColor(mCurrVerticeColor);
}

//allows to setup movement velocity for this particle
void SpriteParticle::SetMovemenVelocity(irr::core::vector3df initialVelocity, irr::core::vector3df finalVelocity) {
    mInitialMovementVelocity = initialVelocity;
    mFinalMovementVelocity = finalVelocity;

    //we use the movement velocity option now
    mUseMovementVelocity = true;
}

SpriteParticle::~SpriteParticle() {
  //remove my SceneNode from the Scene
  this->mSceneNode->remove();
}

void SpriteParticle::ResetParticle() {
}

//Update returns true if particle still has remaining lifetime
//returns false if lifetime is over
bool SpriteParticle::Update(irr::f32 frameDeltaTime) {
    if (mLifeTimeSec > 0.0f) {
        irr::f32 lifeTimeAlphaVal = ((this->lifeTimeSecParam - mLifeTimeSec) / lifeTimeSecParam);

        if (lifeTimeAlphaVal > 1.0f)
            lifeTimeAlphaVal = 1.0f;

        if (lifeTimeAlphaVal < 0.0f)
            lifeTimeAlphaVal = 0.0f;

        //if movement velocity option is used,
        //apply it
        if (mUseMovementVelocity) {
            irr::core::vector3df currVelocityEffect = irr::core::lerp(mInitialMovementVelocity, mFinalMovementVelocity, lifeTimeAlphaVal);

            this->mParticlePos += currVelocityEffect * frameDeltaTime;
        }

        //at the end of the lifetime fade the particle
        //out
        if (mVisible) {
            if (lifeTimeAlphaVal > 0.5f) {
                irr::u8 fadeVal = (irr::u8)(255.0f * (1.0f - (lifeTimeAlphaVal - 0.5f)/0.5f));

                //if we want to fade out (make the billBoardSceneNode more transparent)
                //we just need to set the Vertices colors more black
                mCurrVerticeColor.set(255, fadeVal, fadeVal, fadeVal);
                this->mSceneNode->setColor(mCurrVerticeColor);
            }
        }

        return true;
    }
    else return false;
}

void SpriteParticle::ShowParticle() {
    mVisible = true;
    mCurrVerticeColor.set(255, 255, 255, 255);
    mSceneNode->setColor(mCurrVerticeColor);
    this->mSceneNode->setVisible(true);
}

void SpriteParticle::HideParticle() {
    this->mSceneNode->setVisible(false);
    mLifeTimeSec = 0.0f;
    mVisible = false;
}
