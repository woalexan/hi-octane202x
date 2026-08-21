/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

//This source code was implemented by me based on the insight I gained into the original game. Some parts do more closely follow
//the Pseudo-C Code that is available to me, and other parts are heavily modified to hopefully work inside my existing project
//in the near future.
//The original game uses fixed point arithmetic for performance reasons (it had to run on 80486 after all), and is optimized everywhere you
//look at. I do not want to do the same, because the fixed point arithmetic makes it very difficult to read and understand. And performance should not
//be the problem nowadays. Therefore the biggest change which I have to do is to change all the logic to floating point arithmetic,
//while hopefully not breaking the concept behind everything.
//Additionally I will only try to reimplement the most important mechanisms and concepts from the original game where it benefits the overall project,
//but I will not change the overall project to use the same structs and data structures as the original. This would not work, and also does not
//make sense in my opinion.

//Important note: What makes this source code very difficult to handle is the fact, that my coordinate system in this existing project is
//completely different to the one in the original game. The original uses X and Y axis for the tile map, and Z is the height.
//For the levelfile and 2D map stuff I also use X any Y axis for the tile map most of the time.
//My 3D world setup (for rendering) using Irrlicht has X and Z for the tile map, and Y is the height. And to make things worse my Irrlicht vertice X and Y coordinates
//have a swapped sign (are negative) currently.

//I have decided to also use the original games coordinate system in all vanilla calculations. At the interface between
//original game calculations and Irrlicht 3D coordinate system I have then to convert from one coordinate system setup to the other.
//Thats the drawback I will have.

//I really want to thank aybe for giving me the opportunity to look much deeper into the original game inner workings as I was ever able before.
//Without this support I would not have been able to hopefully advance the current project more true to the original.

#include "vmgun.h"
#include "vvehicle.h"
#include "vcalc.h"
#include "../race.h"
#include "../game.h"
#include "../resources/texture.h"
#include "../vanilla/vthing.h"

VMGun::VMGun(Race* parentRace, VVehicle* owner) {
    mParentRace = parentRace;
    mOwner = owner;

    mSpriteTex = mParentRace->mTexLoader->spriteTex.at(43);
    mSpriteTexSize = mSpriteTex->getSize();

    mBulletThings.clear();
}

VMGun::~VMGun() {
}

BulletThingStruct* VMGun::CreateBulletThing(irr::core::vector3df* position,
                                   irr::f32 angleXY, irr::f32 angleZY,
                                          irr::f32 angleXZ) {
    BulletThingStruct* newBulletThing = new BulletThingStruct;
    newBulletThing->ThingData.Action = 0;
    newBulletThing->ThingData.Movement.AngleXY = angleXY;
    newBulletThing->ThingData.Movement.AngleXZ = angleXZ;
    newBulletThing->ThingData.Movement.AngleZY = angleZY;
    newBulletThing->ThingData.Movement.SpeedActual = 0.0f;
    //mapwho_add(newBulletThing->ThingData, position)
    //alternative below
    newBulletThing->ThingData.Position = *position;
    newBulletThing->ThingData.Displacement.set(0.0f, 0.0f, 0.0f);

    //Life and CollideSize for bullet is defined in
    //initializeEFFECT_BULLET
    newBulletThing->ThingData.Life = 4;
    newBulletThing->ThingData.CollideSize.set(1.0f, 1.0f, 1.0f);
    newBulletThing->ThingData.ColideGroup = (1048 & 0xFFFE);
    newBulletThing->ThingData.AffectNumber = 60;
    newBulletThing->ThingData.AffectStatus |= 4;

    //Setup the Irrlicht SceneNode as well
    newBulletThing->mSceneNode = mParentRace->mGame->mSmgr->addBillboardSceneNode();
    newBulletThing->mSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
    newBulletThing->mSceneNode->setMaterialTexture(0, mSpriteTex);
    newBulletThing->mSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, true);
    newBulletThing->mSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);
    newBulletThing->mSceneNode->setSize(irr::core::dimension2df(0.2f, 0.2f));

    UpdateSceneNode(newBulletThing->mSceneNode, newBulletThing->ThingData.Position);

    mBulletThings.push_back(newBulletThing);

    return newBulletThing;
}

void VMGun::UpdateSceneNode(irr::scene::IBillboardSceneNode* whichNode, irr::core::vector3df vanPos) {
    irr::core::vector3df irrPos =
            mParentRace->mVCalc->VanillaToIrrlichtCoord(vanPos);

     whichNode->setPosition(irrPos);
}

//This function mostly implements the functionality
//of the function "processEFFECT_BULLET" of the original game
uint8_t VMGun::UpdateBulletThing(BulletThingStruct* whichBulletThing) {
    irr::f32 v2;

    v2 = whichBulletThing->ThingData.Movement.AngleXY + 11.375f;
    whichBulletThing->ThingData.Life--;

    whichBulletThing->ThingData.Movement.AngleXY = v2;
    if (whichBulletThing->ThingData.Life < 0) {
        //thing_delete(thing)
        //setting Flag 0x4 means the thing is not needed anymore
        //and should be deleted
        whichBulletThing->ThingData.Status |= 0x4;
         return 1;
    } else {
        if ((whichBulletThing->ThingData.Status & 8) == 0) {
            if (whichBulletThing->ThingData.ColideGroup) {
                //effect_affect_vehicle_exclusive(whichBulletThing);
                //affect_thing(thing);
            }
            whichBulletThing->ThingData.Status |= 8u;
        }
    }

    return 0;
}

//This function mostly implements the functionality
//of the function "processWEAPON_MINI_GUN" of the
//original game
void VMGun::Update(irr::f32 frameDeltaTime) {
    int32_t v5;
    int32_t v10;
    int16_t triggerRestrictionCount;
    irr::f32 angleXY;
    irr::f32 angleZY;
    irr::f32 angleXZ;
    VVehicle* targetVehicle = nullptr;
    irr::core::vector3df* p_Position;
    irr::core::vector3df position;
    uint32_t v13;
    irr::f32 v14;
    int16_t v18;
    BulletThingStruct* v12;

    //first update all existing bullet objects
    std::vector<BulletThingStruct*>::iterator it;
    for (it = mBulletThings.begin(); it != mBulletThings.end(); ++it) {
        UpdateBulletThing((*it));
    }

    if (mOwner != nullptr) {
      v5 = 0;
      if (Trigger) {
         if (TriggerTime < 100) {
             v5 = 1;
         } else {
            triggerRestrictionCount = TriggerRestrictionCount;
            TriggerRestrictionCount = triggerRestrictionCount - 1;
            if (triggerRestrictionCount) {
                //sample_stop(v6, 15)
                //sample_play(v6, 14);
            } else {
                v5 = 1;
                TriggerRestrictionCount = 10;
            }
            TriggerTime = 100;
         }
      if (v5) {
        angleXY = mOwner->View.AngleXY;
        angleZY = mOwner->View.AngleZY;
        angleXZ = mOwner->View.AngleXZ;
        if (Target) {
            targetVehicle = mParentRace->GetVehicleWithId((size_t)(Target));
            angleXY = mParentRace->mVCalc->angle_get_xy(mOwner->ThingData->Position,
                                                  targetVehicle->ThingData->Position);
            angleZY = mParentRace->mVCalc->angle_get_zy(mOwner->ThingData->Position,
                                                  targetVehicle->ThingData->Position);
        }
        v10 = 1;
        p_Position = &mOwner->ThingData->Position;
        do {
          //v12 = thing_initialise(p_Position, &angle, 6, 0, v6->Id);
          v12 = CreateBulletThing(p_Position, angleXY, angleZY, angleXZ);
          if (v12 != nullptr) {
              ++mOwner->Conditions.Bullets;
              //sample_play(v6, 15);
              v13 = mOwner->ThingData->Status ^ 0x10;
              mOwner->ThingData->Status = v13;
              v14 = -90.0f;
              if ((v13 & 0x10) != 0) {
                  v14 = 90.0f;
              }
              position.X = v12->ThingData.Position.X;
              position.Y = v12->ThingData.Position.Y;
              position.Z = v12->ThingData.Position.Z - 0.03125f;
              mParentRace->mVCalc->move_xyz(position, v14 + v12->ThingData.Movement.AngleXY, 0.0f, 0.125f);
              //mapwho_move(v12, &position);
              v12->ThingData.Position = position;
              UpdateSceneNode(v12->mSceneNode, v12->ThingData.Position);
              TriggerTime += 2;
              if (TriggerTime >= 100) {
                  ++mOwner->Conditions.MiniGunHeatup;
              }
              //v12->ThingData.Upgrade = this->Upgrade;
          }
          --v10;
          p_Position = &mOwner->ThingData->Position;
        } while (v10);
      }
      Trigger = 0;
      return;
    }
    if (Upgrade == 1) {
        v18 = TriggerTime - 17;
    } else if (Upgrade >= 2) {
      if (Upgrade == 2) {
          v18 = TriggerTime - 25;
      } else
      {
          if (Upgrade != 3) {
              goto processWEAPON_MINI_GUN_LABEL36;
          }
          v18 = TriggerTime - 33;
      }
    } else
    {
      if (Upgrade) {
               goto processWEAPON_MINI_GUN_LABEL36;
      }
      v18 = TriggerTime - 10;
    }
    TriggerTime = v18;
processWEAPON_MINI_GUN_LABEL36:
    if (TriggerTime >= 0) {
        return;
    }
    TriggerTime = 0;
  }
}


