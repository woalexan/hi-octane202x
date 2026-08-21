/*
 Copyright (C) 2025-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef COLLECTABLESPAWNER_H
#define COLLECTABLESPAWNER_H

#include <irrlicht.h>
#include <vector>
#include "../resources/entityitem.h"
#include "../vanilla/vbase.h"
#include "../vanilla/vthing.h"

#define DEF_COLLECTABLE_SPAWNER_STATE_INITIAL 0
#define DEF_COLLECTABLE_SPAWNER_STATE_SPAWNING 1
#define DEF_COLLECTABLE_SPAWNER_STATE_DONE 2

/************************
 * Forward declarations *
 ************************/

class Race;
class Collectable;

//struct for keeping all the data of a spawned collectable entity
//that is emitted from a spawnPoint
struct SpawnedCollectableInfoStruct {
    //pointer to the spawned new collectable object
    Collectable* pntrCollectable = nullptr;

    //Important: state contains the coordinates in vanilla
    //(original game) coordinate system
    VThing* state = nullptr;

    irr::f32 deltaTimeAcc;

    bool endOfLifeReached;
    bool spawned;
};

class CollectableSpawner {
public:
    //Important: input parameters use the vanilla (original games) coordinate system!
    CollectableSpawner(Race* race, irr::core::vector3df vanillaSpawnLocation,
                       irr::scene::ISceneManager* smgr, irr::video::IVideoDriver *driver);
    ~CollectableSpawner();

    void Update(irr::f32 deltaTime);
    bool CanBeCleanedUp();

    //spawns a type 2 collectable (spawned temporary item, for example when player craft breaks down)
    void AddCollectableToSpawn(Entity::EntityType newEntityType);
    void Trigger();

private:
    irr::core::vector3df mVanillaSpawnLocation;

    irr::scene::ISceneManager* mSmgr = nullptr;
    irr::video::IVideoDriver* mDriver = nullptr;

    std::vector<SpawnedCollectableInfoStruct*> mSpawnedCollectablesVec;

    Race *mRace = nullptr;
    irr::u8 mState = DEF_COLLECTABLE_SPAWNER_STATE_INITIAL;

    void RegisterTemporaryCollectible(Collectable* collectibleToAdd);

    int8_t UpdatePosition(irr::f32 deltaTime, VThing* whichThing);
};

#endif // COLLECTABLESPAWNER_H
