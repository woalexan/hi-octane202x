/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "mgun.h"
#include "../resources/texture.h"
#include "player.h"
#include "../utils/ray.h"
#include "../audio/sound.h"
#include "../utils/physics.h"
#include "../game.h"
#include "../race.h"

//Returns true in case of success
//False otherwise
bool MachineGun::LoadSprites() {
   irr::video::ITexture* newTex;

   //get pointer to preloaded texture
   newTex = this->mParent->mRace->mTexLoader->spriteTex.at(43);

   if (newTex == nullptr)
       return false;

   animTexList.push_back(newTex);

   newTex = this->mParent->mRace->mTexLoader->spriteTex.at(44);

   if (newTex == nullptr)
       return false;

   animTexList.push_back(newTex);

   newTex = this->mParent->mRace->mTexLoader->spriteTex.at(45);

   if (newTex == nullptr)
       return false;

   animTexList.push_back(newTex);

   newTex = this->mParent->mRace->mTexLoader->spriteTex.at(46);

   if (newTex == nullptr)
       return false;

   animTexList.push_back(newTex);

   return true;
}

//we can not remove scenenodes which have still
//an animations running when the race is over
//the race object uses this function to wait until
//all animations are done
bool MachineGun::AllAnimationsFinished() {
    std::vector<MachineGunBulletImpactStruct*>::iterator it;

    for (it = this->mBulletImpactVec.begin(); it != this->mBulletImpactVec.end(); ++it) {
        if ((*it)->animator != nullptr) {
           if (!(*it)->animator->hasFinished())
               return false;
        }
    }

    return true;
}

//returns nullptr if currently all 4 shoots are fired, and no impact
//struct (sprite scenenode and animator) is currently available
//otherwise returns a random picked available impact struct
//random to make sure that the impact does not occur always at the same tile
//location
MachineGunBulletImpactStruct* MachineGun::GetCurrentlyAvailableImpactStruct() {
  irr::u8 nrAvail = 0;

  std::vector<MachineGunBulletImpactStruct*>::iterator it;

  for (it = this->mBulletImpactVec.begin(); it != this->mBulletImpactVec.end(); ++it) {
      if (!(*it)->shooting) {
          nrAvail++;
      }
  }

  //if we currently fire all 4 machine guns at the same time
  //return nullptr
  if (nrAvail < 1)
      return nullptr;

  //now select one of the available impact structs randomly
  irr::f32 probability = 100.0f / irr::f32(nrAvail);
  int rNum;
  irr::f32 rNumFloat;

  while (1) {
    for (it = this->mBulletImpactVec.begin(); it != this->mBulletImpactVec.end(); ++it) {
      if (!(*it)->shooting) {
          //get random number between 0 and calculated probability value
           rNum = rand();
           rNumFloat = (irr::f32)((float(rNum) / float (RAND_MAX)) * 100.0f);

           if (rNumFloat < probability) {
               //this available impact struct was randomly selected
               return (*it);
           }
      }
    }
 }
}

//HelperFunction for computer player control
bool MachineGun::CoolDownNeeded() {
   return CpPlayerCurrentlyCoolDownNeeded;
}

void MachineGun::Trigger() {
    //if player can not shoot right now simply exit
    if (!this->mParent->mPlayerStats->mPlayerCanShoot)
        return;

    bool skipAnimation = false;

    if (!mCanShotAgain)
        return;

    if (mGunOverheated) {
            //shoot attempt that failed, because gun is to warm
            //only play this sound for a human player, for a computer player
            //this does not make sense, and only is disturbing
            if (mParent->mHumanPlayer) {
               if (mShotFailSound == nullptr) {
                    mShotFailSound = mParent->mRace->mSoundEngine->PlaySound(SRES_GAME_MGUN_SHOTFAILED,
                                                                             mParent->phobj->physicState.position, false);
               }
            }
      return;
    }

    //can we shoot one bullet more currently, or are already
    //all 4 possible shoots at the same time fired?

    //freeStruct will be returned as nullptr if we can not fire another shot currently
    MachineGunBulletImpactStruct* freeStruct = GetCurrentlyAvailableImpactStruct();

    if (freeStruct != nullptr) {
        //we can shoot again
        irr::core::vector3df shotTargetLoc;

        if (mParent->mTargetPlayer != nullptr) {
             //we have currently a player targeted, fire at the player
             //06.05.2025: Until now the computer players have a target accuracy of 100% in the final
             //race stats which is kind of unrealistic, and looks weird; Reason is that the computer players only fire
             //when the have a target, and if there is a target the gun always hits with 100% probability right now
             //to make everything more realistic from today on I want to introduce a random probability with which
             //we hit an available target. The probability of hitting the target player should be increased with better
             //target quality. If we hit the player we do the same as before, but if we do not hit we want to draw a
             //sprite randomly somewhere around the player on the terrain
             irr::u32 hitProbability = this->mParent->GetMGunHitProbability();

             //get a random number between 0 and 100
             irr::u32 randNum = this->mParent->mRace->mGame->randRangeInt(0, 100);

             //did we hit?
             if (randNum < hitProbability) {
                     //yes, we hit according to random number
                     //use a random shoot target location based on the player craft model
                     shotTargetLoc = mParent->mTargetPlayer->GetRandomMGunShootTargetLocation(true);
                     irr::f32 damage = DEF_MGUN_DAMAGE;
                     //mParent->mTargetPlayer->Damage(damage);
                     mParent->mRace->DamagePlayer(mParent->mTargetPlayer, damage, DEF_RACE_DAMAGETYPE_MGUN, mParent);
                     mParent->mTargetPlayer->DamageGlas();

                     //this was a hit, add to hit statistics for accuracy
                     mParent->mPlayerStats->shootsHit++;
             } else {
                 //we did not hit the player
                 shotTargetLoc = mParent->mTargetPlayer->GetRandomMGunShootTargetLocation(false);

                 mParent->mPlayerStats->shootsMissed++;
             }
         } else {
             //we do not target any player right now
             //the bullet should impact the terrain
             //shotTargetLoc = GetBulletImpactPoint();
             RayHitTriangleInfoStruct triangleHit;
             if (mParent->GetWeaponTarget(triangleHit)) {
                 //we found a valid target
                 //position impact sprite a little bit away from the triangle, to make it visible completely
                 irr::core::vector3df triangleNormal = triangleHit.hitTriangle.getNormal().normalize();
                 if (triangleNormal.dotProduct(triangleHit.rayDirVec) > 0.0f) {
                     //triangle normal has opposite direction as the ray that hit it
                     //this means most likely the triangle that was hit is seen by the ray/player from the backside
                     //we need to negate direction of triangleNormal, because otherwise we offset sprite location
                     //behind the triangle, and player will not be able to see it
                     triangleNormal = -triangleNormal;
                 }

                 shotTargetLoc = triangleHit.hitPointOnTriangle + triangleNormal * irr::core::vector3df(0.2f, 0.2f, 0.2f);

                 //this was a miss, add to statistics of shoots target missed
                 //we need this for later accuracy calculations
                 mParent->mPlayerStats->shootsMissed++;

             } else {
                 //did not find a shooting location target
                 skipAnimation = true;
                 mParent->mPlayerStats->shootsMissed++;
                 freeStruct->animatorActive = false;
             }
        }

         //only if we know where to draw the animation draw it
         //otherwise do not draw anything
         if (!skipAnimation) {
            //move animated sprite with bullet impact to
            //new target location, and start animation
            freeStruct->animSprite->setPosition(shotTargetLoc);
            freeStruct->animSprite->setVisible(true);
            freeStruct->animator = mSmgr->createTextureAnimator(animTexList, 50, false);
            freeStruct->animSprite->addAnimator(freeStruct->animator);
            freeStruct->animatorActive = true;
         }

         freeStruct->shooting = true;
         mCanShotAgain = false;
         shootAgainDelay = DEF_MGUN_REPETITION_DELAY;

     //if (mParent->mHumanPlayer) {
            freeStruct->mShotSound = mParent->mRace->mSoundEngine->PlaySound(SRES_GAME_MGUN_SINGLESHOT,
                                                                             mParent->phobj->physicState.position, false);
     //}
    }
}

void MachineGun::Update(irr::f32 DeltaTime) {
    std::vector<MachineGunBulletImpactStruct*>::iterator it;

    mLastFiring = mCurrentFiring;

    mCurrentFiring = false;

    //are we currently shooting?
    for (it = this->mBulletImpactVec.begin(); it != this->mBulletImpactVec.end(); ++it) {
         if ((*it)->shooting) {
             mCurrentFiring = true;
             break;
         }
    }

    for (it = this->mBulletImpactVec.begin(); it != this->mBulletImpactVec.end(); ++it) {
            if ((*it)->shooting) {
                if ((*it)->animatorActive) {
                        if ((*it)->animator->hasFinished()) {
                            (*it)->animSprite->removeAnimator((*it)->animator);
                            (*it)->animator->drop();
                            (*it)->animator = nullptr;
                            (*it)->shooting = false;
                            (*it)->animSprite->setVisible(false);
                        }
                 } else {
                   (*it)->shooting = false;
                 }
            }

            if ((*it)->mShotSound != nullptr) {
                if ((*it)->mShotSound->getStatus() == sf::SoundSource::Status::Stopped) {
                    (*it)->mShotSound = nullptr;
                }
            }
    }

    if (mShotFailSound != nullptr) {
            if (mShotFailSound->getStatus() == sf::SoundSource::Status::Stopped) {
                mShotFailSound = nullptr;
            }
    }

    //we started or stopped firing
    if (mLastFiring != mCurrentFiring) {
        if (!mCurrentFiring) {
            //we stopped firing right now
            //calculate a new cooloff time, this time depends on the heat
            //we had before stopping firing
            //as higher the heat was the longer the cooloff time takes
            //worst case for full heat is approx 1 second, for not much heat
            //it is more like 200ms
            coolOffTime = 0.05f + 0.7f * (this->mParent->mPlayerStats->mgHeatVal / this->mParent->mPlayerStats->mgHeatMax);

            reductionValue = (this->mParent->mPlayerStats->mgHeatVal / (coolOffTime / 0.05f));
        }
    }

    timeAccu += DeltaTime;

    if (timeAccu > 0.05) {
        timeAccu = 0.0f;

        if (mCurrentFiring) {
            //we are currently firing, increase mgun heat value
            //from lowest temp up to max temp it seems to take around 5 seconds in the
            //original game
            this->mParent->mPlayerStats->mgHeatVal += 2.5f;
        } else {
            //we are not firing right now, we can cooloff
            if (this->mParent->mPlayerStats->mgHeatVal > 0.0f) {
                //we still have heat to cool-off
                //speed of cool-off depends on current value of variable coolOffTime
                this->mParent->mPlayerStats->mgHeatVal -= reductionValue;

                if (this->mParent->mPlayerStats->mgHeatVal < 0.0f) {
                    this->mParent->mPlayerStats->mgHeatVal = 0.0f;
                }
            }
        }

        if (this->mParent->mPlayerStats->mgHeatVal >= (0.95f * this->mParent->mPlayerStats->mgHeatMax)) {
            this->mParent->mPlayerStats->mgHeatVal = this->mParent->mPlayerStats->mgHeatMax;

            mGunOverheated = true;
            CpPlayerCurrentlyCoolDownNeeded = true;
         } else {
            //we are not overheated anymore
            mGunOverheated = false;
        }

        if (this->mParent->mPlayerStats->mgHeatVal < (0.3f * this->mParent->mPlayerStats->mgHeatMax)) {
            CpPlayerCurrentlyCoolDownNeeded = false;
        }
    }

    if (!mCanShotAgain) {
        shootAgainDelay -= DeltaTime;

        if (shootAgainDelay < 0.0f) {
            mCanShotAgain = true;
        }
    }
}

MachineGun::MachineGun(Player* myParentPlayer, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver *driver) {
    mParent = myParentPlayer;
    mSmgr = smgr;
    mDriver = driver;

    ready = true;

    if (!LoadSprites()) {
        ready = false;
    } else {

        //create the 4 machine gun impact structs
        for (int i = 0; i < 4; i++) {
            MachineGunBulletImpactStruct* newImpactStruct = new MachineGunBulletImpactStruct();

            newImpactStruct->animSprite = mSmgr->addBillboardSceneNode();
            newImpactStruct->animSprite->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );

            newImpactStruct->animSprite->setMaterialFlag(irr::video::EMF_LIGHTING, true);
            newImpactStruct->animSprite->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

            newImpactStruct->animSprite->setPosition(irr::core::vector3df(0.0f, 0.0f, 0.0f));
            newImpactStruct->animSprite->setSize(irr::core::dimension2d<irr::f32>(0.2f, 0.2f));

            newImpactStruct->animSprite->setVisible(false);

            this->mBulletImpactVec.push_back(newImpactStruct);
        }
    }
}

MachineGun::~MachineGun() {
    //cleanup my SceneNodes
    std::vector<MachineGunBulletImpactStruct*>::iterator it;
    MachineGunBulletImpactStruct* pntr;

    for (it = this->mBulletImpactVec.begin(); it != this->mBulletImpactVec.end(); ) {
        pntr = (*it);

        it = this->mBulletImpactVec.erase(it);

        //remove all animators from this SceneNode
        pntr->animSprite->removeAnimators();

        if (pntr->mShotSound != nullptr) {
            pntr->mShotSound->stop();
        }

        //remove this SceneNode from the
        //SceneManager
        pntr->animSprite->remove();
    }
}
