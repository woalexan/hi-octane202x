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

#include "veffectmanager.h"
#include "vthing.h"
#include "vcalc.h"
#include "../race.h"
#include "../game.h"
#include "../resources/texture.h"

VEffectManager::VEffectManager(Race* parentRace) {
    mParentRace = parentRace;

    LoadSprites();

    mSpriteTexSize = animTexList[0]->getSize();

    mSceneNode = mParentRace->mGame->mSmgr->addBillboardSceneNode();
    mSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
    mSceneNode->setMaterialTexture(0, animTexList[0]);
    mSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    mSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);
    mSceneNode->setVisible(false);

    //make the sprite completely visible
    mCurrVerticeColor.set(255, 255, 255, 255);
    mSceneNode->setColor(mCurrVerticeColor);
}

VEffectManager::~VEffectManager() {
}

//Returns true in case of success
//False otherwise
bool VEffectManager::LoadSprites() {
   irr::video::ITexture* newTex;

   //get pointer to preloaded texture
   for (size_t idx = 0; idx < 13; idx++) {
           newTex = this->mParentRace->mTexLoader->spriteTex.at(idx);

           if (newTex == nullptr)
               return false;

           animTexList.push_back(newTex);
   }

   return true;
}

void VEffectManager::TestExplosion(irr::core::vector3df location, irr::f32 angleXY, irr::f32 angleZY, irr::f32 angleXZ, int16_t id) {
    if (Explosion != nullptr)
        return;

    Explosion = mParentRace->mThingManager->thing_initialise(location, angleXY,
                                           angleZY, angleXZ, 2, 1, id);

    initialiseEFFECT_EXPLOSION(Explosion);

    irr::core::vector3df irrPos =
            mParentRace->mVCalc->VanillaToIrrlichtCoord(location);

    mSceneNode->setPosition(irrPos);
    mSceneNode->setSize(irr::core::dimension2df(1.0f, 1.0f));
    currDrawNr = 0;
    mSceneNode->setMaterialTexture(0, animTexList[currDrawNr]);
    mSceneNode->setVisible(true);
}

void VEffectManager::UpdateTestExplosion() {
    if (Explosion == nullptr)
        return;

    processEFFECT_EXPLOSION(Explosion);

    irr::core::vector3df irrPos =
            mParentRace->mVCalc->VanillaToIrrlichtCoord(Explosion->Position);

    mSceneNode->setPosition(irrPos);
}

void VEffectManager::initialiseEFFECT_EXPLOSION(VThing* whichThing) {
    irr::f32 xy;
    irr::f32 zy;
    uint32_t status;

    //I skipped some code here, do we need it?

    whichThing->Life = 12;
    //thing_set_draw(v4, 139);
    whichThing->CollideSize.set(1.0f, 1.0f, 1.0f);
    whichThing->ColideGroup = (1048 & 0xFFFE);
    xy = whichThing->Movement.AngleXY;
    zy = whichThing->Movement.AngleZY;
    status = whichThing->AffectStatus;
    whichThing->AffectNumber = 200;
    whichThing->AffectStatus = (status | 3);
    mParentRace->mVCalc->move_displacement_set(
                whichThing->Displacement, xy, zy, 0.5859375f);
}

void VEffectManager::processEFFECT_EXPLOSION(VThing* whichThing) {
    irr::core::vector3df position;
    irr::f32 v13;
    irr::f32 v14;
    int8_t action;
    int rNum;
    int rNum2;
    int16_t v5;
    int16_t v6;
    size_t number;

    action = whichThing->Action;
    if (action == 1) {
 processEFFECT_EXPLOSION_LABEL_13:
        whichThing->Life--;
        if (whichThing->Life < 0) {
            whichThing->Action = 0x14;
            return;
        }
        if ((fabs(whichThing->Displacement.X) >= 0.00390625f) ||
           (fabs(whichThing->Displacement.Y) >= 0.00390625f)) {
                position = whichThing->Position;
                mParentRace->mVCalc->move_displacement_xyz(position, whichThing->Displacement, 1);
                whichThing->Displacement.X -= whichThing->Displacement.X / 8.0f;
                whichThing->Displacement.Y -= whichThing->Displacement.Y / 8.0f;
                if ((mParentRace->mVCalc->map_colide(position) & 0x10) != 0) {
                    whichThing->Displacement.X = 0.0f;
                    whichThing->Displacement.Y = 0.0f;
                } else {
                    mParentRace->mThingManager->mapwho_move(whichThing, position);
                }
        }

        number = currDrawNr;
        currDrawNr = number + 1;

        if (whichThing->Status & 0x200) {
            whichThing->Life--;
            currDrawNr = number + 2;
        }

        mSceneNode->setMaterialTexture(0, animTexList[currDrawNr]);

        v13 = whichThing->Position.Z + ((irr::f32)(whichThing->Count) / 256.0f);
        whichThing->Position.Z = v13;
        v14 = mParentRace->mVCalc->map_floor(whichThing->Position);
        if (v13 < v14) {
            whichThing->Position.Z = v14;
        }
        whichThing->Count = (11 * whichThing->Count) / 16;
        return;
    }

    if (action >= 2) {
      if (action < 21) {
          if (action < 19) {
              return;
          }
          mParentRace->mThingManager->thing_delete(whichThing);
      }
      return;
    }

    if (!whichThing->Action) {
       whichThing->Action = 1;
       if (mParentRace->mVCalc->map_colide(whichThing->Position)) {
            mParentRace->mThingManager->thing_remove(whichThing);
            return;
       }
       if (whichThing->ColideGroup) {
           mParentRace->mThingManager->thing_touching_anything(whichThing);
           mParentRace->mThingManager->affect_thing(whichThing);
       }

       //sample_play(v4, 20);
       rNum = rand();
       v5 = (int16_t)((rNum % 0x9Du));
       rNum2 = rand();
       v6 = (int16_t)((rNum2 % 0x64u) + 100);
       whichThing->Count = (2 * (v5 / 0x4Fu) - 1) * v6;
       goto processEFFECT_EXPLOSION_LABEL_13;
    }
}
