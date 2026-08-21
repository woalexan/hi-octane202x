/*
 Copyright (C) 2025-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "collectablespawner.h"
#include "../vanilla/vcalc.h"
#include "../race.h"
#include "../models/collectable.h"
#include "../game.h"
#include "../resources/texture.h"
#include "../definitions.h"
#include "../vanilla/vthing.h"

//Important: input parameters use the vanilla (original games) coordinate system!
CollectableSpawner::CollectableSpawner(Race* race, irr::core::vector3df vanillaSpawnLocation,
                                       irr::scene::ISceneManager* smgr, irr::video::IVideoDriver *driver) {
    mSmgr = smgr;
    mRace = race;
    mDriver = driver;

    mVanillaSpawnLocation = vanillaSpawnLocation;
    mSpawnedCollectablesVec.clear();
}

//Contains the implementation from powerup_move function
//from original game
//Important note: Works with vanilla coordinate system
//of original game!
int8_t CollectableSpawner::UpdatePosition(irr::f32 deltaTime, VThing* whichThing) {
   irr::core::vector3df intPosition;
   irr::core::vector3df intDisplacement;
   irr::f32 XPos;
   irr::f32 YPos;
   irr::f32 ZPos;

   bool isNoCollision;
   bool Flag1Set;
   bool Flag2Set;
   bool Flag4Set;

   if ((whichThing->Status & 0x200u) != 0) {
      intPosition.X = whichThing->Position.X;
      intPosition.Y = whichThing->Position.Y;
      intPosition.Z = whichThing->Position.Z;

      XPos = whichThing->Displacement.X;
      irr::f32 v9 = XPos + (float)(7.0f / 256.0f);
      if (v9 < 0.0f) {
          v9 += (float)(7.0f / 256.0f);
      }

      whichThing->Displacement.X -= (v9 / 8.0f);

      YPos = whichThing->Displacement.Y;
      irr::f32 v11 = YPos + (float)(7.0f / 256.0f);
      if (v11 < 0.0f) {
          v11 += (float)(7.0f / 256.0f);
      }

      whichThing->Displacement.Y -= (v11 / 8.0f);

      ZPos = whichThing->Displacement.Z;
      ZPos -= (float)(8.0f / 256.0f);

      if (ZPos < (float)(-100.0f / 256.0f)) {
          ZPos = (float)(-100.0f / 256.0f);
      }

      whichThing->Displacement.Z = ZPos;

      mRace->mVCalc->move_displacement_xyz(intPosition, whichThing->Displacement, 1);

      int8_t collideResult = mRace->mVCalc->map_colide_direction(whichThing->Position, intPosition);

      isNoCollision = (collideResult == 0);
      Flag1Set = ((collideResult & 1) != 0);

      if (!isNoCollision) {
          //there is a collision
          isNoCollision = !Flag1Set;
          Flag2Set = ((collideResult & 2) != 0);

          if (!isNoCollision) {
              irr::f32 v18 = 0.0f;
              if (whichThing->Displacement.X > 0.0f) {
                  v18 = (float)(1.0f / 256.0f);
              }

              v18 -= whichThing->Displacement.X;
              v18 = v18 * 0.5f;

              whichThing->Displacement.X = v18;
              intPosition.X = whichThing->Position.X + v18;
          }

           isNoCollision = !Flag2Set;
           Flag4Set = ((collideResult & 4) != 0);

           if (!isNoCollision) {
               irr::f32 v21 = 0.0f;
               if (whichThing->Displacement.Y > 0.0f) {
                   v21 = (float)(1.0f / 256.0f);
               }

               v21 -= whichThing->Displacement.Y;
               v21 = v21 * 0.5f;

               whichThing->Displacement.Y = v21;
               intPosition.Y = whichThing->Position.Y + v21;
           }

           if (Flag4Set) {
               irr::f32 v23 = (float)((-120.0f / 256.0f)) * whichThing->Displacement.Z;
               whichThing->Displacement.Z = v23;

               if (v23 < (float)(10.0f / 256.0f)) {
                   whichThing->Displacement.Z = 0.0f;
               }

              intPosition.Z = whichThing->Displacement.Z + mRace->mVCalc->map_floor(intPosition);
              mRace->mVCalc->move_displacement_slope(whichThing->Position, intDisplacement);

              irr::f32 v24 = -1.0f;
              if ((intDisplacement.X < -1.0f) ||
                      (v24 = 1.0f, intDisplacement.X >= (float)(257.0f / 256.0f))) {
                  intDisplacement.X = v24;
              }

              irr::f32 v25 = -1.0f;
              if ((intDisplacement.Y < -1.0f) ||
                      (v25 = 1.0f, intDisplacement.Y >= (float)(257.0f / 256.0f))) {
                  intDisplacement.Y = v25;
              }

              whichThing->Displacement.X += intDisplacement.X / 16.0f;
              whichThing->Displacement.Y += intDisplacement.Y / 16.0f;
           }
      }

      mRace->mThingManager->mapwho_move(whichThing, intPosition);
      //whichThing->Position = intPosition;
      return 1;
   }

   return 0;
}

void CollectableSpawner::Update(irr::f32 deltaTime) {
    irr::core::vector3df irrCoordPos;

    //was spawning items triggered?
    if (mState == DEF_COLLECTABLE_SPAWNER_STATE_SPAWNING) {
          std::vector<SpawnedCollectableInfoStruct*>::iterator it;
          bool allCollectiblesReachedEndOfLife = true;
          SpawnedCollectableInfoStruct* pntrInfoStruct;

          //now update all the elements we spawned
          for (it = this->mSpawnedCollectablesVec.begin(); it != this->mSpawnedCollectablesVec.end(); ++it) {
               //do nothing if end of life already reached
              if ((*it)->endOfLifeReached) {
                  continue;
              }

              //Update and verify remaining lifetime
              (*it)->deltaTimeAcc += deltaTime;

              //reduce collectable remaining life time count
              //every 50ms, independent of period time we
              //run this function otherwise
              if ((*it)->deltaTimeAcc > 0.05) {
                 (*it)->deltaTimeAcc = 0.0f;
                 (*it)->pntrCollectable->ThingData->Life -= 1;

                 if ((*it)->pntrCollectable->ThingData->Life < 0) {
                     //hide the type 2 collectable, so that it can
                     //not be picked up anymore
                     (*it)->pntrCollectable->SetVisible(false);
                     (*it)->endOfLifeReached = true;
                     continue;
                 }

                 //check if any temporary collectables were picked up, and therefore
                 //reached end of life
                 //we know this when the were spawned before, but are not visible
                 //anymore
                 if ((*it)->spawned && (!(*it)->pntrCollectable->GetIfVisible())) {
                     (*it)->endOfLifeReached = true;
                     continue;
                 }
              }

              allCollectiblesReachedEndOfLife = false;

              if (!(*it)->spawned) {
                  (*it)->pntrCollectable->SetVisible(true);
                  (*it)->spawned = true;

                  //add it to race vector of collectibles, so that it can be picked up by a player
                  mRace->RegisterTemporaryCollectible((*it)->pntrCollectable);
              }

              //item is still existing, calculate next position
              UpdatePosition(deltaTime, (*it)->pntrCollectable->ThingData);

              //convert from vanilla to my Irrlicht coordinate system
              irrCoordPos = mRace->mVCalc->VanillaToIrrlichtCoord((*it)->pntrCollectable->ThingData->Position);

              //updates position of SceneNode, Boundingsbox etc...
              (*it)->pntrCollectable->UpdatePosition(irrCoordPos);
          }

          //all collectibles have reached end-of-life?
          if (allCollectiblesReachedEndOfLife) {
            //first unregister all temporary Collectibles from Race
            //so that this objects can not be used anymore
            for (it = this->mSpawnedCollectablesVec.begin(); it != this->mSpawnedCollectablesVec.end(); ++it) {
                mRace->UnregisterTemporaryCollectible((*it)->pntrCollectable);
            }

            //mark spawner to be done with spawning
            //so that this spawner object can be deleted by the
            //race object afterwards
            mState = DEF_COLLECTABLE_SPAWNER_STATE_DONE;
        }
    }
}

bool CollectableSpawner::CanBeCleanedUp() {
    if (mState == DEF_COLLECTABLE_SPAWNER_STATE_DONE)
        return true;

    return false;
}

//spawns a type 2 collectable (spawned temporary item, for example when player craft breaks down)
void CollectableSpawner::AddCollectableToSpawn(Entity::EntityType newEntityType) {
    //only allow items to spawn in the initial state
    if (mState != DEF_COLLECTABLE_SPAWNER_STATE_INITIAL)
        return;

    //create a new type 2 collectable
    irr::u16 spriteNr = mRace->GetCollectableSpriteNumber(newEntityType);

    irr::core::vector3df irrCoordPos;

    //convert from vanilla to my Irrlicht coordinate system
    irrCoordPos = mRace->mVCalc->VanillaToIrrlichtCoord(this->mVanillaSpawnLocation);

    int8_t groupVal;
    int8_t memberVal;

    RevIdentifyEntity(newEntityType, groupVal, memberVal);

    //create a new struct with information how to spawn the collectable
    SpawnedCollectableInfoStruct* newInfoStruct = new SpawnedCollectableInfoStruct();

    //This creates the collectable SceneNode in Irrlicht, but also hides it immediately
    //so that first it is not visible
    Collectable* newCollectable = new Collectable(mRace->mGame, newEntityType, irrCoordPos,
                                                  mRace->mTexLoader->spriteTex.at(spriteNr), this->mRace->mGame->enableLightning);

    //keep a pointer to the sceneNode
    newInfoStruct->pntrCollectable = newCollectable;

    //get my thing
    newInfoStruct->pntrCollectable->ThingData =
        mRace->mThingManager->thing_initialise_member(mVanillaSpawnLocation, 0.0f, 0.0f, 0.0f,
                                    5, memberVal, -1);

    newInfoStruct->pntrCollectable->ThingData->Status |= 0x200u;

    //we also need to fill out the ThingData struct
    newInfoStruct->pntrCollectable->ThingData->Position = mVanillaSpawnLocation;
    newInfoStruct->pntrCollectable->ThingData->CollideSize.set(1.0f, 1.0f, 1.0f);
    newInfoStruct->pntrCollectable->ThingData->Position.Z = mRace->mVCalc->map_floor(newInfoStruct->pntrCollectable->ThingData->Position);
    newInfoStruct->deltaTimeAcc = 0.0f;
    newInfoStruct->endOfLifeReached = false;
    newInfoStruct->spawned = false;

    irr::f32 rNumFloat1;
    irr::f32 rNumFloat2;
    irr::f32 rNumFloat3;
    int rNum;

    //derive a random flying speed, the random number range below taken also from
    //powerup_init function of original game
    rNum = rand();
    rNumFloat1 = 0.3125f + (float(rNum) / float (RAND_MAX)) * 0.3125f;

    newInfoStruct->pntrCollectable->ThingData->Life = 200;
    newInfoStruct->pntrCollectable->ThingData->Movement.SpeedActual = rNumFloat1;


    //create another random number for Movement.Angle.ZY
    rNum = rand();
    rNumFloat2 = 19.9951171875f + (float(rNum) / float (RAND_MAX)) * 69.9993896484375f;

    //create another random number for Movement.Angle.XY
    //I was not able to find out the original implementation for this random number generation
    //Therefore I ran multiple iterations in the emulator, and observed the resulting random
    //numbers; With this information I created this code below
    rNum = rand();
    rNumFloat3 = -180.0f + (float(rNum) / float (RAND_MAX)) * 360.0f;

    newInfoStruct->pntrCollectable->ThingData->Movement.AngleXY = rNumFloat3;
    newInfoStruct->pntrCollectable->ThingData->Movement.AngleZY = -rNumFloat2;

    mRace->mVCalc->move_displacement_set(newInfoStruct->pntrCollectable->ThingData->Displacement, newInfoStruct->pntrCollectable->ThingData->Movement.AngleXY,
                                         newInfoStruct->pntrCollectable->ThingData->Movement.AngleZY,
                                         newInfoStruct->pntrCollectable->ThingData->Movement.SpeedActual);

    //add to my vector of items to spawn
    mSpawnedCollectablesVec.push_back(newInfoStruct);
}

void CollectableSpawner::Trigger() {
    //only allow to trigger spawning items in the initial state
    if (mState != DEF_COLLECTABLE_SPAWNER_STATE_INITIAL)
        return;

    //is there even something to spawn?
    if (mSpawnedCollectablesVec.size() > 0) {
        mState = DEF_COLLECTABLE_SPAWNER_STATE_SPAWNING;
    }
}

CollectableSpawner::~CollectableSpawner() {
    std::vector<SpawnedCollectableInfoStruct*>::iterator it;

    //delete all temporary created stuff
    if (mSpawnedCollectablesVec.size() > 0) {
        //make sure to unregister all temporary Collectibles from Race
        //so that this objects can not be used anymore
        for (it = this->mSpawnedCollectablesVec.begin(); it != this->mSpawnedCollectablesVec.end(); ++it) {
            mRace->UnregisterTemporaryCollectible((*it)->pntrCollectable);
        }

        SpawnedCollectableInfoStruct* pntrInfoStruct;
        Collectable* pntrCollectible;

        for (it = this->mSpawnedCollectablesVec.begin(); it != this->mSpawnedCollectablesVec.end(); ) {
           pntrInfoStruct = (*it);
           pntrCollectible = (*it)->pntrCollectable;

           //remove entry in my vector, not needed anymore
           it = mSpawnedCollectablesVec.erase(it);

           //delete Collectable itself
           //this frees the SceneNode
           delete pntrCollectible;

           //Signal that the underlying Thing can be
           //deleted as well
           mRace->mThingManager->thing_delete(pntrInfoStruct->pntrCollectable->ThingData);

           //also delete the info struct
           delete pntrInfoStruct;
       }
    }
}
