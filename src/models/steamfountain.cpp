/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "steamfountain.h"
#include "../resources/texture.h"
#include "spriteparticle.h"

//***************************************************
//*   SteamParticle / SteamFountain class           *
//***************************************************

void SteamParticle::ResetParticle() {
    //let particle restart
    mParticlePos = startLocationParam;
    mLifeTimeSec = lifeTimeSecParam;
    mCurrParticleSize = mInitSize;

    //I want a random number between 0.8f and 1.2f
    int rNum = rand();
    irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX))  * 0.4f;

    mSceneNode->setPosition(mParticlePos);
    mSceneNode->setSize(mInitSize * rNumFloat);

   //show particle again
   ShowParticle();
}

//Update returns true if particle still has remaining lifetime
//returns false if lifetime is over
bool SteamParticle::Update(irr::f32 frameDeltaTime) {
   mLifeTimeSec -= frameDeltaTime;

   if (mLifeTimeSec > 0.0f) {
     //this particle has some time left
     int rNum = rand();
     irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX)) * 0.4f;

     mParticlePos.Y += 3.0f * rNumFloat * frameDeltaTime;

     rNum = rand();
     rNumFloat = (float(rNum - (RAND_MAX / 2)) / float (RAND_MAX));

     mParticlePos.X += 0.4f * rNumFloat * frameDeltaTime;

     rNum = rand();
     rNumFloat = (float(rNum - (RAND_MAX / 2)) / float (RAND_MAX));

     mParticlePos.Z += 0.4f * rNumFloat * frameDeltaTime;

     //calculate new size depending on remaining particle lifetime
     mCurrParticleSize = mInitSize + mEndLifeSize * (1.0f - (mLifeTimeSec / lifeTimeSecParam));

     mSceneNode->setPosition(mParticlePos);
     mSceneNode->setSize(mCurrParticleSize);

   } else {
       //this particles time is over
       ResetParticle();
   }

   SpriteParticle::Update(frameDeltaTime);

   if (mLifeTimeSec > 0.0f)
       return true;
   else return false;
}

SteamParticle::~SteamParticle() {
}

//only used by the level Editor
void SteamFountain::Hide() {
    mActivated = false;

    std::vector<SteamParticle*>::iterator itParticle;

    for (itParticle = mCurrSpriteVec->begin(); itParticle != mCurrSpriteVec->end(); ++itParticle) {
        (*itParticle)->HideParticle();
    }
}

//only used by the level Editor
void SteamFountain::Show() {
    Activate();
}

SteamFountain::SteamFountain(irr::video::ITexture* steamTex, EntityItem* entityItem, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, irr::core::vector3d<irr::f32> location,
                             irr::u32 nrMaxParticles) {
    mSmgr = smgr;
    mDriver = driver;
    
    mPosition = location;
    mNrMaxParticles = nrMaxParticles;
    mEntityItem = entityItem;

    mSteamTex = steamTex;
    mSteamTexSize = mSteamTex->getSize();

    mCurrSpriteVec = new std::vector<SteamParticle*>();
}

SteamFountain::~SteamFountain() {
    //remove all currently existing particles
    std::vector<SteamParticle*>::iterator itParticle;

    SteamParticle* pntr;

    if (mCurrSpriteVec->size() > 0) {

        //delete all existing particles
        itParticle = mCurrSpriteVec->begin();

        while (itParticle != mCurrSpriteVec->end()) {
           pntr = (*itParticle);
           itParticle = mCurrSpriteVec->erase(itParticle);

           //remove also particle itself
           delete pntr;
        }
    }

    delete mCurrSpriteVec;
}

void SteamFountain::Activate() {
    absTimeSinceLastActivation = 0.0f;

    mCurrNrParticels = 0;

    mActivated = true;
    absTimeSinceLastUpdate = 0.0f;
}

void SteamFountain::Trigger() {
    if (!mActivated) {
        Activate();
    }
}

void SteamFountain::TriggerUpdate(irr::f32 frameDeltaTime) {
    absTimeSinceLastActivation += frameDeltaTime;

    if (mActivated) {
        absTimeSinceLastUpdate += frameDeltaTime;

        //only update sprites every 10 mSeconds
        if (absTimeSinceLastUpdate > 0.01) {

            //we can create more particles
            if (mCurrNrParticels < mNrMaxParticles) {
              SteamParticle* newParticle = new SteamParticle(mSmgr, mSteamTex, mPosition, 1.2f,
                                                               irr::core::dimension2d<irr::f32>(0.1f, 0.1f), irr::core::dimension2d<irr::f32>(0.4f, 0.4f));

              //add new particle to my particle list
              this->mCurrSpriteVec->push_back(newParticle);

              mCurrNrParticels++;
          }

          std::vector<SteamParticle*>::iterator it;

          //update all currently existing particles
          for (it = mCurrSpriteVec->begin(); it != mCurrSpriteVec->end(); ++it) {
              (*it)->Update(absTimeSinceLastUpdate);
          }

            absTimeSinceLastUpdate = 0.0f;
      }
    }
}

