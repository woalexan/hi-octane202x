/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef COLLECTABLESPAWNER_H
#define COLLECTABLESPAWNER_H

#include <irrlicht/irrlicht.h>
#include "../race.h"
#include "collectable.h"

#define DEF_COLLECTABLE_SPAWNER_STATE_INITIAL 0
#define DEF_COLLECTABLE_SPAWNER_STATE_SPAWNING 1
#define DEF_COLLECTABLE_SPAWNER_STATE_DONE 2

class Race; //Forward declaration

//struct for keeping all the data of a spawned collectable entity
//that is emitted from a spawnPoint
struct SpawnedCollectableInfoStruct {
    //pointer to the spawned new collectable object
    Collectable* pntrCollectable;

    irr::core::vector3df currVelocity;
    irr::core::vector3df spawnPoint;

    irr::core::vector3df mVelocity;

    bool mHitTerrain = false;

    bool collectableReachedFinalLocation = false;
};

class CollectableSpawner {
public:
    CollectableSpawner(Race* race, irr::core::vector3df spawnLocation, irr::scene::ISceneManager* smgr,
                       irr::video::IVideoDriver *driver);
    ~CollectableSpawner();

    void Update(irr::f32 deltaTime);
    bool CanBeCleanedUp();

    //spawns a type 2 collectable (spawned temporary item, for example when player craft breaks down)
    void AddCollectableToSpawn(Entity::EntityType newEntityType);
    void Trigger();

private:
    irr::core::vector3df mPosition;
    irr::scene::ISceneManager* mSmgr;
    irr::video::IVideoDriver* mDriver;

    std::vector<SpawnedCollectableInfoStruct*> mSpawnedCollectablesVec;

    Race *mRace;
    irr::u8 mState = DEF_COLLECTABLE_SPAWNER_STATE_INITIAL;

    void AddCollectibleToRaceVector(Collectable* collectibleToAdd);
};

#endif // COLLECTABLESPAWNER_H
