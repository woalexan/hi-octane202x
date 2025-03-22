/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "collectablespawner.h"

CollectableSpawner::CollectableSpawner(Race* race, irr::core::vector3df spawnLocation, irr::scene::ISceneManager* smgr,
                                       irr::video::IVideoDriver *driver) {
    mSmgr = smgr;
    mRace = race;
    mDriver = driver;

    mPosition = spawnLocation;
    mSpawnedCollectablesVec.clear();
}

void CollectableSpawner::AddCollectibleToRaceVector(Collectable* collectibleToAdd) {
    //still add it to race vector of collectibles, so that it is properly
    //cleaned up from memory afterwards
    mRace->ENTCollectablesVec->push_back(collectibleToAdd);
}

void CollectableSpawner::Update(irr::f32 deltaTime) {
    //was spawning items triggered?
    if (mState == DEF_COLLECTABLE_SPAWNER_STATE_SPAWNING) {
          irr::f32 terrainHeight;
          int current_cell_calc_x, current_cell_calc_y;
          std::vector<SpawnedCollectableInfoStruct*>::iterator it;
          bool allCollectiblesReachedFinalLocation = true;
          SpawnedCollectableInfoStruct* pntrInfoStruct;

          //now update all the elements we spawned
          for (it = this->mSpawnedCollectablesVec.begin(); it != this->mSpawnedCollectablesVec.end(); ++it) {
              if (!(*it)->collectableReachedFinalLocation) {
                    allCollectiblesReachedFinalLocation = false;
                    //item is still moving, calculate next position
                  (*it)->pntrCollectable->UpdatePosition((*it)->pntrCollectable->Position + (*it)->currVelocity * deltaTime);
                  (*it)->currVelocity = (*it)->currVelocity + this->mRace->mPhysics->mGravityVec * deltaTime;

                  //check if sprite is currently moving towards ground, and is very close to race track ground (hits the ground)
                  //in this case stop the movement of the collectable, and fix it in position
                  //only check more if the sprite is currently falling towards the race track
                  if ((*it)->currVelocity.Y < 0.0f) {
                      //yes, sprite is falling down, now we need to calculate high about terrain tile below
                      //calculate current cell below sprite
                      current_cell_calc_y = (int)((*it)->pntrCollectable->Position.Z / mRace->mLevelTerrain->segmentSize);
                      current_cell_calc_x = -(int)((*it)->pntrCollectable->Position.X / mRace->mLevelTerrain->segmentSize);

                      MapEntry* mEntry = mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);

                      //is there actually an entry?
                      if (mEntry != NULL) {
                           terrainHeight = mRace->mLevelTerrain->pTerrainTiles[mEntry->get_X()][mEntry->get_Z()].currTileHeight;

                           //collectible too close to terrain, stop the collectible to continue further
                           if (((*it)->pntrCollectable->Position.Y - terrainHeight) < ((*it)->pntrCollectable->GetCollectableCenterHeight())) {
                               //to close to terrain, stop movement
                               (*it)->mHitTerrain = true;
                               (*it)->collectableReachedFinalLocation = true;
                           }
                      } else {
                          //we did not find a valid entry, let collectible disappear (hide it)
                          //because we set it to not visible the computer players will not see it
                          //and we can also not pick it up => no problem
                          (*it)->collectableReachedFinalLocation = true;
                          (*it)->pntrCollectable->SetVisible(false);
                      }
                  }
              } else {
                    //item reached the final location, not moving anymore
              }
         }

        //process all items that have reached final position at the race track surface
        for (it = this->mSpawnedCollectablesVec.begin(); it != this->mSpawnedCollectablesVec.end(); ) {
           if ((*it)->collectableReachedFinalLocation) {
               //add it to race vector of collectibles, so that it can be picked up by a player and is properly
               //cleaned up from memory afterwards
               AddCollectibleToRaceVector((*it)->pntrCollectable);

               pntrInfoStruct = (*it);

               //remove entry in my vector, not needed anymore
               it = mSpawnedCollectablesVec.erase(it);

               //also delete the info struct
               delete pntrInfoStruct;
           } else {
               //go to next element
               it++;
           }
        }

        //all collectibles have reached the final location
        if (allCollectiblesReachedFinalLocation) {
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
    Collectable* newCollectable = new Collectable(mRace, newEntityType, mPosition, mSmgr, mDriver);

    //create a new struct with information how to spawn the collectable
    SpawnedCollectableInfoStruct* newInfoStruct = new SpawnedCollectableInfoStruct();

    newInfoStruct->pntrCollectable = newCollectable;
    newInfoStruct->spawnPoint = mPosition;

    irr::f32 rNumFloat1;
    irr::f32 rNumFloat2;
    irr::f32 rNumFloat3;
    int rNum;

    //derive a random flying velocity direction vector for the collectables
    rNum = rand();
    rNumFloat1 = 0.5f + (float(rNum) / float (RAND_MAX))  * 0.5f;

    rNum = rand();
    rNumFloat2 = 0.5f + (float(rNum) / float (RAND_MAX))  * 0.5f;

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
    rNumFloat3 = 1.0f + (float(rNum) / float (RAND_MAX))  * 2.0f;

    //set initial velocity
    newInfoStruct->mVelocity.set(rNumFloat1 * 2.0f, rNumFloat3 * 2.0f, rNumFloat2 * 2.0f);
    newInfoStruct->currVelocity = newInfoStruct->mVelocity;

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
    //delete the remaining collectibles and collectible
    //spawning info structs that are not yet finished
    //otherwise we have a memory leak
    if (mSpawnedCollectablesVec.size() > 0) {
        std::vector<SpawnedCollectableInfoStruct*>::iterator it;
        SpawnedCollectableInfoStruct* pntrInfoStruct;
        Collectable* pntrCollectible;

        for (it = this->mSpawnedCollectablesVec.begin(); it != this->mSpawnedCollectablesVec.end(); ) {
           pntrInfoStruct = (*it);
           pntrCollectible = (*it)->pntrCollectable;

           //remove entry in my vector, not needed anymore
           it = mSpawnedCollectablesVec.erase(it);

           //delete Collectable itself
           //this frees SceneNode and texture inside
           //collectable implementation
           delete pntrCollectible;

           //also delete the info struct
           delete pntrInfoStruct;
       }
    }
}
