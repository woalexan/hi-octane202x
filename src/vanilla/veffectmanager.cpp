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
#include "../audio/sound.h"
#include "../race.h"
#include "../game.h"
#include "../resources/texture.h"

VEffectManager::VEffectManager(Race* parentRace) {
    mParentRace = parentRace;

    LoadSprites();

    mSpriteTexSize = animTexListExplosion[0]->getSize();

    mActiveEffectVec.clear();
    mNewEffectVec.clear();
}

VEffectManager::~VEffectManager() {
    //cleanup remaining effects when deconstructing
    EffectInfoStruct* pntr;
    std::vector<EffectInfoStruct*>::iterator itEffect;

    for (itEffect = mActiveEffectVec.begin(); itEffect != mActiveEffectVec.end(); ) {
        pntr = (*itEffect);
        itEffect = mActiveEffectVec.erase(itEffect);
        CleanupEffect(pntr);
    }

    //also clean up the new effects vector
    for (itEffect = mNewEffectVec.begin(); itEffect != mNewEffectVec.end(); ) {
        pntr = (*itEffect);
        itEffect = mNewEffectVec.erase(itEffect);
        CleanupEffect(pntr);
    }

    if (mSmokeTex != nullptr) {
        //remove underlying texture
        mParentRace->mGame->mDriver->removeTexture(mSmokeTex);
    }
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

           animTexListExplosion.push_back(newTex);
   }

   //Texture for the SMOKE effect
   mSmokeTex = this->mParentRace->mTexLoader->spriteTex.at(47);

   if (mSmokeTex == nullptr)
       return false;

   return true;
}

uint16_t VEffectManager::GetNrCurrentlyActiveEffects() {
    uint16_t result = (uint16_t)(mActiveEffectVec.size()) +(uint16_t)(mNewEffectVec.size());
    return result;
}

void VEffectManager::InitSceneNode(EffectInfoStruct* whichInfoStruct, irr::video::ITexture* firstTexture, irr::core::dimension2df sizeSprite) {
    whichInfoStruct->sceneNode = mParentRace->mGame->mSmgr->addBillboardSceneNode();
    whichInfoStruct->sceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
    whichInfoStruct->sceneNode->setMaterialTexture(0, firstTexture);
    whichInfoStruct->sceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    whichInfoStruct->sceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);
    whichInfoStruct->sceneNode->setSize(sizeSprite);

    //make the sprite completely visible
    whichInfoStruct->currVerticeColor.set(255, 255, 255, 255);
    whichInfoStruct->sceneNode->setColor(whichInfoStruct->currVerticeColor);
}

void VEffectManager::UpdateSceneNode(EffectInfoStruct* whichInfoStruct, irr::video::ITexture* newTexture) {

    irr::core::vector3df irrPos =
            mParentRace->mVCalc->VanillaToIrrlichtCoord(whichInfoStruct->thingPntr->Position);

    whichInfoStruct->sceneNode->setPosition(irrPos);

    //do we need to swap the texture of the sprite?
    if (newTexture != nullptr) {
        whichInfoStruct->sceneNode->setMaterialTexture(0, newTexture);
    }
}

EffectInfoStruct* VEffectManager::AddEffect(EffectType whichEffect, irr::core::vector3df location, irr::f32 angleXY,
                               irr::f32 angleZY, irr::f32 angleXZ, int16_t id) {

    EffectInfoStruct* newInfoStruct = new EffectInfoStruct();
    newInfoStruct->effectType = whichEffect;
    bool explosionSound = false;
    irr::core::vector3df irrPos;

    switch (whichEffect) {
        case EffectType::Smoke: {
            newInfoStruct->thingPntr = mParentRace->mThingManager->thing_initialise(location,
                                                               angleXY, angleZY, angleXZ,
                                                               2, 7, id);

            initialiseEFFECT_SMOKE(newInfoStruct->thingPntr);
            newInfoStruct->currDrawNr = 60;

            //I looked at all different Drawing numbers for the thing
            //sprite in the Playstation 1 version from 60 up to 69, and
            //the seem to be all the identical sprite;
            InitSceneNode(newInfoStruct, mSmokeTex, irr::core::dimension2df(1.0f, 1.0f));
            break;
        }
        case EffectType::SmokeFire: {
            newInfoStruct->thingPntr = mParentRace->mThingManager->thing_initialise(location,
                                                               angleXY, angleZY, angleXZ,
                                                               2, 8, id);

            initialiseEFFECT_SMOKE_FIRE(newInfoStruct->thingPntr);
            newInfoStruct->currDrawNr = 5;

            //I looked at all different Drawing numbers for the thing
            //sprite in the Playstation 1 version from 5 up to 20, and
            //the seem to be all the identical sprite;
            InitSceneNode(newInfoStruct, mSmokeTex, irr::core::dimension2df(0.5f, 0.5f));
            break;
        }
        case EffectType::ExplosionSmall: {
            newInfoStruct->thingPntr = mParentRace->mThingManager->thing_initialise(location,
                                                               angleXY, angleZY, angleXZ,
                                                               2, 0, id);

            initialiseEFFECT_EXPLOSION(newInfoStruct->thingPntr);
            newInfoStruct->currDrawNr = 0;

            InitSceneNode(newInfoStruct, animTexListExplosion[newInfoStruct->currDrawNr], irr::core::dimension2df(1.0f, 1.0f));
            explosionSound = true;
            break;
        }
        case EffectType::ExplosionMedium: {
            newInfoStruct->thingPntr = mParentRace->mThingManager->thing_initialise(location,
                                                               angleXY, angleZY, angleXZ,
                                                               2, 1, id);

            initialiseEFFECT_EXPLOSION_MEDIUM(newInfoStruct->thingPntr);

            explosionSound = true;

            //this effect has no assigned SceneNode itself
            break;
        }
        default: {
              delete newInfoStruct;
              return nullptr;
        }
    }

    irrPos = mParentRace->mVCalc->VanillaToIrrlichtCoord(newInfoStruct->thingPntr->Position);

    //not every effect has a SceneNode assigned to it!
    if (newInfoStruct->sceneNode != nullptr) {
        newInfoStruct->sceneNode->setPosition(irrPos);
    }

    //we need to add it to the new effects vector
    //instead of the ActiveEffect vector, because we can not
    //add effects to the vector here directly safely, before most of the time
    //when we are here we are in a for-loop of this vector; and when we do this
    //we damage the memory integrity
    mNewEffectVec.push_back(newInfoStruct);

    if (explosionSound) {
        mParentRace->mSoundEngine->PlaySound(SRES_GAME_EXPLODE, irrPos, false);
    }

    return newInfoStruct;
}

void VEffectManager::UpdateEffect(EffectInfoStruct* whichInfoStruct) {
        if (whichInfoStruct->thingPntr == nullptr)
            return;

        if ((whichInfoStruct->thingPntr->Status & 4) != 0) {
            mParentRace->mThingManager->thing_remove(whichInfoStruct->thingPntr);
            whichInfoStruct->thingPntr = nullptr;
            whichInfoStruct->readyForCleanup = true;
            return;
        }

    switch (whichInfoStruct->effectType) {
        case EffectType::Smoke: {
            processEFFECT_SMOKE(whichInfoStruct);
            if (!whichInfoStruct->readyForCleanup) {
                UpdateSceneNode(whichInfoStruct, nullptr);
            }
            break;
        }
        case EffectType::SmokeFire: {
            processEFFECT_SMOKE_FIRE(whichInfoStruct);
            if (!whichInfoStruct->readyForCleanup) {
                UpdateSceneNode(whichInfoStruct, nullptr);
            }
            break;
        }
        case EffectType::ExplosionSmall: {
            processEFFECT_EXPLOSION(whichInfoStruct);
            if (!whichInfoStruct->readyForCleanup) {
                UpdateSceneNode(whichInfoStruct, nullptr);
            }
            break;
        }
        case EffectType::ExplosionMedium: {
            processEFFECT_EXPLOSION_MEDIUM(whichInfoStruct);

            //This effect does not have a SceneNode assigned to it
            break;
        }
        default: {
        }
    }

    if (!whichInfoStruct->readyForCleanup) {
        //we need to update the TimeSlice variable in
        //this effect
        mParentRace->mThingManager->UpdateTimeSlice(whichInfoStruct->thingPntr);
    }
}

void VEffectManager::CleanupEffect(EffectInfoStruct* whichInfoStruct) {
    if (whichInfoStruct == nullptr)
        return;

    //give back the thing, but only if this was not done before
    //in the effect PROCESS functions
    if (whichInfoStruct->thingPntr != nullptr) {
        mParentRace->mThingManager->thing_remove(whichInfoStruct->thingPntr);
        whichInfoStruct->thingPntr = nullptr;
    }

    //remove the SceneNode as well
    //some effects do not have an own sceneNode assigned
    //to them, so check for Nullptr!
    if (whichInfoStruct->sceneNode != nullptr) {
        whichInfoStruct->sceneNode->remove();
    }

    //delete the struct itself
    delete whichInfoStruct;
}

void VEffectManager::Update(irr::f32 frameDeltaTime) {
    std::vector<EffectInfoStruct*>::iterator itEffect;
    std::vector<EffectInfoStruct*>::iterator itEffectNew;
    EffectInfoStruct* pntr2;

    //add delta time up to see when we need to update
    //the slower parts of the code
    mAbsTimeAcc += frameDeltaTime;

    if (mAbsTimeAcc >= 0.05f) {
        mAbsTimeAcc = 0.0f;

        //add the new effects of the last iteration to the main effects vector
        for (itEffectNew = mNewEffectVec.begin(); itEffectNew != mNewEffectVec.end(); ) {
            pntr2 = (*itEffectNew);

            //add to main effects vector list
            mActiveEffectVec.push_back(pntr2);

            itEffectNew = mNewEffectVec.erase(itEffectNew);
        }

        std::vector<EffectInfoStruct*>::reverse_iterator itRevEffect;

        //Update all current active effects
        //30.08.2026: We need to iterate in reverse order so that the underlying Things
        //stuff with Parents and Childs works. At least it seems so.
        for (itRevEffect = mActiveEffectVec.rbegin(); itRevEffect != mActiveEffectVec.rend(); ++itRevEffect) {
             UpdateEffect((*itRevEffect));
        }

        EffectInfoStruct* pntr;

        //Do we need to cleanup older now unused stuff?
        for (itEffect = mActiveEffectVec.begin(); itEffect != mActiveEffectVec.end(); ) {
            if ((*itEffect)->readyForCleanup) {
                pntr = (*itEffect);

                itEffect = mActiveEffectVec.erase(itEffect);

                CleanupEffect(pntr);
            } else {
                ++itEffect;
            }
        }
    }
}

void VEffectManager::initialiseEFFECT_SMOKE(VThing* whichThing) {

    //I skipped some code here, do we need it?

    //thing_set_draw(v4, 60);
    whichThing->CollideSize.set(0.5f, 0.5f, 0.5f);
    whichThing->Life = 22;
}

//This function is executed every ~50ms for each of the current existing Smoke-Clouds
void VEffectManager::processEFFECT_SMOKE(EffectInfoStruct* whichInfoStruct) {
   int8_t action;
   size_t v6;
   bool v7;
   irr::core::vector3df v8;

   VThing* whichThing = whichInfoStruct->thingPntr;

   action = whichThing->Action;
   if (action == 1) {
       goto processEFFECT_SMOKE_LABEL9;
   }
   if (action < 2) {
      if (whichThing->Action) {
          return;
      }
      whichThing->Action = 1;
      mParentRace->mVCalc->move_displacement_set(whichThing->Displacement, whichThing->Movement.AngleXY,
                                                 whichThing->Movement.AngleZY,
                                                 whichThing->Movement.SpeedActual);
processEFFECT_SMOKE_LABEL9:
      whichThing->Life--;
      if (whichThing->Life < 0) {
          whichThing->Action = 0x14;
          return;
      }
      v8 = whichThing->Position;
      mParentRace->mVCalc->move_displacement_xyz(v8, whichThing->Displacement, 1);
      if (mParentRace->mVCalc->map_colide(v8)) {
          mParentRace->mThingManager->thing_remove(whichThing);
          whichInfoStruct->thingPntr = nullptr;
          whichInfoStruct->readyForCleanup = true;
          return;
      }
      mParentRace->mThingManager->mapwho_move(whichThing, v8);

      //26.08.2026: I looked at all different Drawing numbers for the thing
      //sprite in the Playstation 1 version from 60 up to 69 (for the Smoke), and
      //the seem to be all the identical sprite; So I keep counting the
      //variable up for timing purposes, but I will only use a single sprite
      v6 = whichInfoStruct->currDrawNr + 1;
      whichInfoStruct->currDrawNr = v6;
      v7 = (v6 < 70);
      if (!v7) {
          whichInfoStruct->currDrawNr = 69;
          return;
      }
      return;
   }
   if (action == 0x13) {
       return;
   }
   if (action == 0x14) {
       mParentRace->mThingManager->thing_delete(whichThing);
       whichInfoStruct->sceneNode->setVisible(false);
       return;
   }
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

//This function is executed every ~50ms for each of the current existing Explosions
void VEffectManager::processEFFECT_EXPLOSION(EffectInfoStruct* whichInfoStruct) {
    irr::core::vector3df position;
    irr::f32 v13;
    irr::f32 v14;
    int8_t action;
    uint16_t v5;
    uint16_t v6;
    size_t number;

    VThing* whichThing = whichInfoStruct->thingPntr;

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

        number = whichInfoStruct->currDrawNr;
        whichInfoStruct->currDrawNr = number + 1;

        if (whichThing->Status & 0x200) {
            whichThing->Life--;
            whichInfoStruct->currDrawNr = number + 2;
        }

        //change to the next sprite texture
        whichInfoStruct->sceneNode->setMaterialTexture(0, animTexListExplosion[whichInfoStruct->currDrawNr]);

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
            whichInfoStruct->thingPntr = nullptr;
            whichInfoStruct->readyForCleanup = true;
            return;
       }
       if (whichThing->ColideGroup) {
           mParentRace->mThingManager->thing_touching_anything(whichThing);
           mParentRace->mThingManager->affect_thing(whichThing);
       }

       //sample_play(v4, 20);
       v5 = (uint16_t)((whichThing->Seed % 0x9D));
       whichThing->Seed = 9377 * whichThing->Seed + 9439;
       v6 = (uint16_t)((whichThing->Seed % 0x64) + 100);
       whichThing->Seed = 9377 * whichThing->Seed + 9439;
       whichThing->Count = ((2 * (v5 / 0x4F) - 1) * v6);
       goto processEFFECT_EXPLOSION_LABEL_13;
    }
}

void VEffectManager::initialiseEFFECT_EXPLOSION_MEDIUM(VThing* whichThing) {
    uint32_t status;

    whichThing->Life = 3;
    whichThing->CollideSize.set(1.0f, 1.0f, 1.0f);
    whichThing->ColideGroup = (1048 & 0xFFFE);
    status = whichThing->AffectStatus;
    whichThing->AffectNumber = 1500;
    whichThing->AffectStatus = (status | 3);
}

//This function is executed every ~50ms for each of the current existing medium explosions
void VEffectManager::processEFFECT_EXPLOSION_MEDIUM(EffectInfoStruct* whichInfoStruct) {
    irr::core::vector3df position;
    irr::f32 angleXY;
    irr::f32 angleZY;
    irr::f32 angleXZ;
    int8_t action;
    int32_t v5;
    int32_t v7;
    irr::f32 v7Float;
    irr::f32 v8;
    uint16_t v10;
    irr::f32 v10Float;
    VThing* v9 = nullptr;
    EffectInfoStruct* infoStruct = nullptr;
    VThing* v12 = nullptr;

    VThing* whichThing = whichInfoStruct->thingPntr;

    action = whichThing->Action;
    if (action != 1) {
       if (action >= 2) {
          if (action < 0x15) {
             if (action < 0x13) {
                 return;
             }
             mParentRace->mThingManager->thing_delete(whichThing);
          }
          return;
       }
       if (whichThing->Action) {
           return;
       }
       whichThing->Action = 1;
       //sample_play(thing, 20);
       if (whichThing->ColideGroup) {
            mParentRace->mThingManager->thing_touching_anything(whichThing);
            mParentRace->mThingManager->affect_thing(whichThing);
       }
    }
    whichThing->Life--;
    v5 = 0;
    if (whichThing->Life < 0) {
        mParentRace->mThingManager->thing_delete(whichThing);
        return;
    }
    do {
       position = whichThing->Position;
       angleXY = whichThing->Movement.AngleXY;
       angleZY = whichThing->Movement.AngleZY;
       angleXZ = whichThing->Movement.AngleXZ;
       whichThing->Seed = 9377 * whichThing->Seed + 9439;
       v8 = mParentRace->mVCalc->VanillaRawAngleToMyFloatingAngle(whichThing->Seed);
       //Note: I moved the location for the 2nd seeding compared to the original
       //implementation I saw, for the original it seems the same random number is used
       //twice for v7 and v8; not sure if this makes sense, therefore moved it
       whichThing->Seed = 9377 * whichThing->Seed + 9439;
       v7 = static_cast<uint8_t>((-95 * static_cast<int16_t>(whichThing->Seed) - 33));
       v7Float = mParentRace->mVCalc->FixedPointToFloat8D8((int16_t)(v7));
       mParentRace->mVCalc->move_xyz(position, v8, 0.0f, v7Float);
       infoStruct = AddEffect(EffectType::ExplosionSmall, position, angleXY,
                       angleZY, angleXZ, whichThing->Id);

       ++v5;

       if (infoStruct != nullptr) {
           v9 = infoStruct->thingPntr;
            if (v9 != nullptr) {
                v10 = 9377 * whichThing->Seed + 9439;
                whichThing->Seed = v10;
                v10Float = mParentRace->mVCalc->VanillaRawAngleToMyFloatingAngle(v10);
                mParentRace->mVCalc->move_displacement_set(whichThing->Displacement, v10Float,
                                                    whichThing->Movement.AngleZY, 0.5859375f);
                v9->Action = 1;
                v9->ColideGroup = 0;
           }
        }
    } while (v5 < 6);

    //v12 = Create an Effect Flare!
    if (v12 != nullptr) {
        v12->Action = 1;
        v12->ColideGroup = 0;
        return;
    }
}

void VEffectManager::initialiseEFFECT_SMOKE_FIRE(VThing* whichThing) {

    //I skipped some code here, do we need it?

    whichThing->Life = 22;
    //thing_set_draw(v4, 5);
    whichThing->CollideSize.set(0.5f, 0.5f, 0.5f);
}

void VEffectManager::processEFFECT_SMOKE_FIRE(EffectInfoStruct* whichInfoStruct) {
    irr::core::vector3df position;
    int8_t action;
    irr::f32 zPos;
    uint16_t v8;
    bool v10;
    uint16_t zPosFixed;
    int16_t addZPosFixed;
    irr::f32 addZPosFloat;

    VThing* whichThing = whichInfoStruct->thingPntr;

    action = whichThing->Action;
    if (action == 1) {
        goto processEFFECT_SMOKE_FIRE_LABEL_9;
    }
    if (action < 2) {
       if (whichThing->Action) {
           return;
       }
       whichThing->Action = 1;
processEFFECT_SMOKE_FIRE_LABEL_9:
       whichThing->Life--;
       if (whichThing->Life < 0) {
           whichThing->Action = 0x14;
           return;
       }
       position = whichThing->Position;
       position.Z += whichThing->Displacement.Z;
       if (whichThing->Displacement.Z < 0.078125f) {
            zPos = whichThing->CollideSize.Z;
            zPosFixed = (uint16_t)(mParentRace->mVCalc->FloatToFixedPoint8D8(zPos));
            /*v8 = 9377 * whichThing->Seed + 9439;
            whichThing->Seed = v8;*/
            v8 = mParentRace->mGame->randRangeInt(0, 65535);
            addZPosFixed = v8 % (zPosFixed / 8) + zPosFixed / 8;
            addZPosFloat = mParentRace->mVCalc->FixedPointToFloat8D8(addZPosFixed);
            whichThing->Displacement.Z += addZPosFloat;
       }
       mParentRace->mThingManager->mapwho_move(whichThing, position);
       if ((whichThing->TimeSlice & 1) != 0) {
            whichInfoStruct->currDrawNr++;
            v10 = (whichInfoStruct->currDrawNr < 21);
            if (!v10) {
                whichInfoStruct->currDrawNr = 20;
                return;
            }
       }
       return;
    }
    if (action == 0x13) {
        return;
    }
    if (action == 0x14) {
        mParentRace->mThingManager->thing_delete(whichThing);
        whichInfoStruct->sceneNode->setVisible(false);
        return;
    }
}
