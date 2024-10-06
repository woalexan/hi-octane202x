/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "mgun.h"

//Returns true in case of success
//False otherwise
bool MachineGun::LoadSprites() {
   ITexture* newTex;
   newTex = mDriver->getTexture("extract/sprites/tmaps0043.png");

   if (newTex == NULL)
       return false;

   animTexList.push_back(newTex);

   newTex = mDriver->getTexture("extract/sprites/tmaps0044.png");

   if (newTex == NULL)
       return false;

   animTexList.push_back(newTex);

   newTex = mDriver->getTexture("extract/sprites/tmaps0045.png");

   if (newTex == NULL)
       return false;

   animTexList.push_back(newTex);

   newTex = mDriver->getTexture("extract/sprites/tmaps0046.png");

   if (newTex == NULL)
       return false;

   animTexList.push_back(newTex);

   return true;
}

//returns NULL if currently all 4 shoots are fired, and no impact
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
  //return NULL
  if (nrAvail < 1)
      return NULL;

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
               //this availalbe impact struct was randomly selected
               return (*it);
           }
      }
    }
 }
}

void MachineGun::Trigger() {
    bool skipAnimation = false;

    //can we shoot one bullet more currently, or are already
    //all 4 possible shoots at the same time fired?

    //freeStruct will be returned as NULL if we can not fire another shot currently
    MachineGunBulletImpactStruct* freeStruct = GetCurrentlyAvailableImpactStruct();

    if ((freeStruct != NULL) && (coolOffTime < 0.1f)) {

     this->mParent->mPlayerStats->mgHeatVal += 2.5f;

     if (this->mParent->mPlayerStats->mgHeatVal >
             (0.6 * this->mParent->mPlayerStats->mgHeatMax)) {
             //gun gets hot
             coolOffTime = 0.2f + this->mParent->mPlayerStats->mgHeatVal * 0.10f;
    } else coolOffTime = 0.2f;

     irr::core::vector3df shotTargetLoc;

     if (mParent->mTargetPlayer != NULL) {
         //we have currently a player targeted, fire at the player
         shotTargetLoc = mParent->mTargetPlayer->phobj->physicState.position;
         irr::f32 damage = DEF_MGUN_DAMAGE;
         //mParent->mTargetPlayer->Damage(damage);
         mParent->mRace->DamagePlayer(mParent->mTargetPlayer, damage, mParent);
         mParent->mTargetPlayer->DamageGlas();
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

         } else {
             //did not find a shooting location target
             skipAnimation = true;
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

        freeStruct->shooting = true;
     }

     if (mParent->mHumanPlayer) {
            freeStruct->mShotSound = mParent->mRace->mSoundEngine->PlaySound(SRES_GAME_MGUN_SINGLESHOT, false);
     }

    } else if ((freeStruct != NULL) && (coolOffTime > 0.1f)) {
        //shoot attempt that failed, because gun is to warm
        if (mParent->mHumanPlayer) {
           if (mShotFailSound == NULL) {
                mShotFailSound = mParent->mRace->mSoundEngine->PlaySound(SRES_GAME_MGUN_SHOTFAILED, false);
           }
        }
    }
}

void MachineGun::Update(irr::f32 DeltaTime) {
    std::vector<MachineGunBulletImpactStruct*>::iterator it;

    for (it = this->mBulletImpactVec.begin(); it != this->mBulletImpactVec.end(); ++it) {

        if ((*it)->shooting && (*it)->animator->hasFinished()) {
            (*it)->animator->drop();
            (*it)->shooting = false;
            (*it)->animSprite->setVisible(false);
            }

    if ((*it)->mShotSound != NULL) {
            if ((*it)->mShotSound->getStatus() == (*it)->mShotSound->Stopped) {
                (*it)->mShotSound = NULL;
            }
        }
    }

    if (mShotFailSound != NULL) {
            if (mShotFailSound->getStatus() == mShotFailSound->Stopped) {
                mShotFailSound = NULL;
            }
        }

    timeAccu += DeltaTime;

    if (coolOffTime > 0.0f) {
        coolOffTime -= DeltaTime;

        if (coolOffTime < 0.0f)
            coolOffTime = 0.0f;
    }

    if (timeAccu > 0.1) {
        timeAccu = 0.0f;
        if (this->mParent->mPlayerStats->mgHeatVal > 0.0f) {
            this->mParent->mPlayerStats->mgHeatVal -= 0.5f;

            if (this->mParent->mPlayerStats->mgHeatVal < 0.0f)
                this->mParent->mPlayerStats->mgHeatVal = 0.0f;
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
            //newImpactStruct->animSprite->setMaterialTexture(0, spriteTex);
            newImpactStruct->animSprite->setMaterialFlag(irr::video::EMF_LIGHTING, true);
            newImpactStruct->animSprite->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

            newImpactStruct->animSprite->setPosition(irr::core::vector3df(0.0f, 0.0f, 0.0f));
            newImpactStruct->animSprite->setSize(irr::core::dimension2d<irr::f32>(0.2f, 0.2f));

            newImpactStruct->animSprite->setVisible(false);

            this->mBulletImpactVec.push_back(newImpactStruct);
        }
    }
}
