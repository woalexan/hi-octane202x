/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "particle.h"

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
    /*if (mLastVisible != mVisible) {
        if (mVisible) {
            //make the sprite again as solid as possible
            mCurrVerticeColor.set(255, 255, 255, 255);
            mSceneNode->setColor(mCurrVerticeColor);

            this->mSceneNode->setVisible(true);
        }
    }

    mLastVisible = mVisible;*/

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

SteamFountain::SteamFountain(Race* parentRace, EntityItem* entityItem, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, irr::core::vector3d<irr::f32> location,
                             irr::u32 nrMaxParticles) {
    mSmgr = smgr;
    mDriver = driver;
    mParentRace = parentRace;

    mPosition = location;
    mNrMaxParticles = nrMaxParticles;
    mEntityItem = entityItem;

    //get the cloud sprite from the game
    mSteamTex = mParentRace->mTexLoader->spriteTex.at(17);
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

//***************************************************
//*   SmokeParticle / SmokeTrail class              *
//***************************************************

void SmokeParticle::ResetParticle(irr::core::vector3df newStartLocation) {
    //let particle restart
    mParticlePos = newStartLocation;
    mLifeTimeSec = lifeTimeSecParam;
    mCurrParticleSize = mInitSize;

    //I want a random number between 0.8f and 1.2f
    int rNum = rand();
    irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX))  * 0.4f;

    mSceneNode->setPosition(mParticlePos);
    mSceneNode->setSize(mInitSize * rNumFloat);

    ShowParticle();
}

//Update returns true if particle still has remaining lifetime
//returns false if lifetime is over
bool SmokeParticle::Update(irr::f32 frameDeltaTime, irr::core::vector3df newStartLocation) {
   mLifeTimeSec -= frameDeltaTime;

   if (mLifeTimeSec > 0.0f) {
     //this particle has some time left
     int rNum = rand();
     irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX)) * 0.4f;

     mParticlePos.Y += 3.0f * rNumFloat * frameDeltaTime;

     rNum = rand();
     rNumFloat = (float(rNum - (RAND_MAX / 2)) / float (RAND_MAX));

     mParticlePos.X += 0.7f * rNumFloat * frameDeltaTime;

     rNum = rand();
     rNumFloat = (float(rNum - (RAND_MAX / 2)) / float (RAND_MAX));

     mParticlePos.Z += 0.7f * rNumFloat * frameDeltaTime;

     //calculate new size depending on remaining particle lifetime
     mCurrParticleSize = mInitSize + mEndLifeSize * (1.0f - (mLifeTimeSec / lifeTimeSecParam));

     mSceneNode->setPosition(mParticlePos);
     mSceneNode->setSize(mCurrParticleSize);

   } else {
       HideParticle();
   }

    SpriteParticle::Update(frameDeltaTime);

    if (mLifeTimeSec > 0.0f)
        return true;
    else return false;
}

SmokeParticle::~SmokeParticle() {
}

SmokeTrail::SmokeTrail(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, Player* parentPlayer,
                             irr::u32 nrMaxParticles) {
    mSmgr = smgr;
    mDriver = driver;
    mParentPlayer = parentPlayer;

    mNrMaxParticles = nrMaxParticles;

    //load the smoke sprite from the game
    //std::string spriteTexFile("extract/sprites/tmaps0013.png");

    mSmokeTex = parentPlayer->mRace->mTexLoader->spriteTex.at(13);
    mSmokeTexSize = mSmokeTex->getSize();

    mCurrSpriteVec = new std::vector<SmokeParticle*>();
}

SmokeTrail::~SmokeTrail() {
    //remove all currently existing particles
    std::vector<SmokeParticle*>::iterator itParticle;

    SmokeParticle* pntr;

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

void SmokeTrail::Activate() {
    absTimeSinceLastActivation = 0.0f;

    mActivated = true;
    absTimeSinceLastUpdate = 0.0f;
}

void SmokeTrail::Deactivate() {
    absTimeSinceLastActivation = 0.0f;

    mActivated = false;
    absTimeSinceLastUpdate = 0.0f;
}

void SmokeTrail::Update(irr::f32 frameDeltaTime) {
    absTimeSinceLastActivation += frameDeltaTime;

    absTimeSinceLastUpdate += frameDeltaTime;

        //only update sprites every 0.01 seconds
        if (absTimeSinceLastUpdate > 0.01) {

            if (mActivated) {
                    //we can create more particles
                    if (mCurrNrParticels < mNrMaxParticles) {
                       //old reusable particle left?
                       std::vector<SmokeParticle*>::iterator it;
                       SmokeParticle* pntr = NULL;
                       for (it = this->mCurrSpriteVec->begin(); it != this->mCurrSpriteVec->end(); it++) {
                           if (!((*it)->mLifeTimeSec > 0.0f)) {
                               pntr = (*it);
                               break;
                           }
                       }

                      if (pntr != NULL) {
                          pntr->ResetParticle(this->mParentPlayer->WorldCoordCraftSmokePnt);
                      } else {
                           //nothing available anymore, create a new particle
                           SmokeParticle* newParticle = new SmokeParticle(mSmgr, mSmokeTex,
                                     this->mParentPlayer->WorldCoordCraftSmokePnt, 0.6f,
                                                                       irr::core::dimension2d<irr::f32>(0.2f, 0.2f), irr::core::dimension2d<irr::f32>(0.4f, 0.4f));

                         //add new particle to my particle list
                         this->mCurrSpriteVec->push_back(newParticle);
                      }

                      mCurrNrParticels++;
                  }
            }

          std::vector<SmokeParticle*>::iterator it;

          //update all currently existing particles
          for (it = mCurrSpriteVec->begin(); it != mCurrSpriteVec->end(); ++it) {
              if (!(*it)->Update(absTimeSinceLastUpdate,
                            this->mParentPlayer->WorldCoordCraftSmokePnt)) {
                  //current particle stopped to exist (no lifetime anymore)
                  if (mCurrNrParticels > 0) {
                      mCurrNrParticels--;
                  }
              }
          }

            absTimeSinceLastUpdate = 0.0f;
      }
}

//***************************************************
//*   DustParticle / DustBelowCraft class           *
//***************************************************

void DustParticle::ResetParticle(irr::core::vector3df newStartLocation) {
    //let particle restart
    mParticlePos = newStartLocation;
    mLifeTimeSec = lifeTimeSecParam;
    mCurrParticleSize = mInitSize;

    //I want a random number between 0.8f and 1.2f
    int rNum = rand();
    irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX))  * 0.4f;

    mSceneNode->setPosition(mParticlePos);
    mSceneNode->setSize(mInitSize * rNumFloat);

    //unhide existing sceneNode of particles
    //again
    ShowParticle();
}

//Update returns true if particle still has remaining lifetime
//returns false if lifetime is over
bool DustParticle::Update(irr::f32 frameDeltaTime, irr::core::vector3df newStartLocation) {
   mLifeTimeSec -= frameDeltaTime;

   if (mLifeTimeSec > 0.0f) {
     //this particle has some time left
     int rNum = rand();
     irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX)) * 0.4f;

     mParticlePos.Y += 0.03f * rNumFloat * frameDeltaTime;

     rNum = rand();
     rNumFloat = (float(rNum - (RAND_MAX / 2)) / float (RAND_MAX));

     mParticlePos.X += 2.0f * rNumFloat * frameDeltaTime;

     rNum = rand();
     rNumFloat = (float(rNum - (RAND_MAX / 2)) / float (RAND_MAX));

     mParticlePos.Z += 2.0f * rNumFloat * frameDeltaTime;

     //calculate new size depending on remaining particle lifetime
     mCurrParticleSize = mInitSize + mEndLifeSize * (1.0f - (mLifeTimeSec / lifeTimeSecParam));

     mSceneNode->setPosition(mParticlePos);
     mSceneNode->setSize(mCurrParticleSize);

   } else {
       if (!mIsReset) {
        //this particles time is over
        ResetParticle(newStartLocation);
        mIsReset = true;
       }
   }

    SpriteParticle::Update(frameDeltaTime);

    if (mLifeTimeSec > 0.0f)
        return true;
    else return false;
}

DustParticle::~DustParticle() {
}

DustBelowCraft::DustBelowCraft(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, Player* parentPlayer,
                             irr::u32 nrMaxParticles) {
    mSmgr = smgr;
    mDriver = driver;
    mParentPlayer = parentPlayer;

    mNrMaxParticles = nrMaxParticles;

    //get the cloud sprite from the game
    mDustTex = mParentPlayer->mRace->mTexLoader->spriteTex.at(17);
    mDustTexSize = mDustTex->getSize();

    mCurrSpriteVec = new std::vector<DustParticle*>();
}

DustBelowCraft::~DustBelowCraft() {
    //remove all currently existing particles
    std::vector<DustParticle*>::iterator itParticle;

    DustParticle* pntr;

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

void DustBelowCraft::Activate() {
    absTimeSinceLastActivation = 0.0f;

    mCurrNrParticels = 0;

    mActivated = true;
    absTimeSinceLastUpdate = 0.0f;
}

void DustBelowCraft::Deactivate() {
    absTimeSinceLastActivation = 0.0f;

    mCurrNrParticels = 0;

    mActivated = false;
    absTimeSinceLastUpdate = 0.0f;
}

void DustBelowCraft::SetupVelocityParticle(DustParticle* particlePntr) {
    if (particlePntr == NULL)
        return;

    int rNum;

    //first get a random direction for velocity in X-Z plane, so that
    //the particles fly away in all directions from the craft
    irr::core::vector3df initialVelocity;

    //derive a random flying velocity direction vector for the debris
    rNum = rand();
    irr::f32 rNumFloat1 = 0.5f + (float(rNum) / float (RAND_MAX))  * 0.5f;

    rNum = rand();
    irr::f32 rNumFloat2 = 0.5f + (float(rNum) / float (RAND_MAX))  * 0.5f;

    rNum = rand();
    if (float(rNum) > (float (RAND_MAX) * 0.5f)) {
        rNumFloat1 = -rNumFloat1;
    }

    rNum = rand();
    if (float(rNum) > (float (RAND_MAX) * 0.5f)) {
        rNumFloat2 = -rNumFloat2;
    }

    rNum = rand();
    //velocity for Y axis only upwards, not that the object is shoot into the ground
    irr::f32 rNumFloat3 = 1.0f + (float(rNum) / float (RAND_MAX))  * 2.0f;

    initialVelocity.X = rNumFloat1 * 0.3f;
    initialVelocity.Y = rNumFloat3 * 0.05f;
    initialVelocity.Z = rNumFloat2 * 0.3f;

    //for the final velocity reduce sideways
    //velocity quite a lot, as the craft is not moving the
    //dust particles so much anymore in this greater distance
    //but increase the speed upwards in Y direction
    irr::core::vector3df finalVel;
    finalVel.X = initialVelocity.X * 0.25f;
    finalVel.Z = initialVelocity.Z * 0.25f;
    finalVel.Y = initialVelocity.Y * 2.0f;

    particlePntr->SetMovemenVelocity(initialVelocity, finalVel);
}

void DustBelowCraft::Update(irr::f32 frameDeltaTime) {
    absTimeSinceLastActivation += frameDeltaTime;

    absTimeSinceLastUpdate += frameDeltaTime;

    //only update sprites every 0.01 seconds
    if (absTimeSinceLastUpdate > 0.01) {
            //are we currently above dirt, and need to create more particles?
            if (mActivated) {
                    //we can create more particles
                    if (mCurrNrParticels < mNrMaxParticles) {
                       //old reusable particle left?
                       std::vector<DustParticle*>::iterator it;
                       DustParticle* pntr = NULL;
                       for (it = this->mCurrSpriteVec->begin(); it != this->mCurrSpriteVec->end(); it++) {
                           if (!((*it)->mLifeTimeSec > 0.0f)) {
                               pntr = (*it);
                               break;
                           }
                       }

                      if (pntr != NULL) {
                          pntr->mIsReset = false;

                          //setup new particle velocities
                          SetupVelocityParticle(pntr);

                      } else {
                           //nothing available anymore, create a new particle
                           DustParticle* newParticle = new DustParticle(mSmgr, mDustTex,
                                     this->mParentPlayer->WorldCraftDustPnt, DEF_DUSTPARTICLELIFETIME,
                                                                       irr::core::dimension2d<irr::f32>(0.3f, 0.3f), irr::core::dimension2d<irr::f32>(0.4f, 0.4f));

                           //setup new particle velocities
                           SetupVelocityParticle(newParticle);

                           //add new particle to my particle list
                           this->mCurrSpriteVec->push_back(newParticle);
                      }

                      mCurrNrParticels++;
                  }
            }

          std::vector<DustParticle*>::iterator it;

          //update all currently existing particles
          for (it = mCurrSpriteVec->begin(); it != mCurrSpriteVec->end(); ++it) {
              if (!(*it)->Update(absTimeSinceLastUpdate,
                            this->mParentPlayer->WorldCraftDustPnt)) {
                  //current particle stopped to exist (no lifetime anymore)
                  (*it)->HideParticle();
                  if (mCurrNrParticels > 0) {
                      mCurrNrParticels--;
                  }
              }
          }

            absTimeSinceLastUpdate = 0.0f;
      }

}
