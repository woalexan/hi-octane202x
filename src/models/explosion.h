/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "irrlicht.h"
#include <vector>
#include "../audio/sound.h"

//defines for the animated exploding fire balls
#define DEF_EXPLOSION_NRDETONATIONS 12
#define DEF_EXPLOSION_COORDINATE_RNDVAL 2.0f
#define DEF_EXPLOSION_DETONATION_DELAY_RNDVAL 0.02f

//defines for the non animated but flying explosion
//debris objects
#define DEF_EXPLOSION_NRDEBRIS 4
#define DEF_EXPLOSION_DEBRIS_DISTANCENEXTSPRITE 0.2f
#define DEF_EXPLOSION_DEBRIS_SPRITESIZE 1.0f
#define DEF_EXPLOSION_NRDEBRISSPRITES 18
#define DEF_EXPLOSION_DEBRISSPRITELIFETIME 2.0f  //1.0f
#define DEF_EXPLOSION_DETONATION_DELAY_DEBRIS_RNDVAL 0.1f

//struct for keeping all the data for a single animated
//exploding fire ball
struct ExplosionDetonationStruct {
    irr::core::vector3d<irr::f32> currDetonationPos;
    irr::scene::IBillboardSceneNode* mSceneNodeDetonation = nullptr;
    irr::scene::ISceneNodeAnimator *animator = nullptr;

    irr::f32 detonationDelay;

    bool currDetonating = false;
    bool detonated = false;

    //initial sprite size after creation
    irr::core::dimension2d<irr::f32> mInitSize;

    //max sprite size at the end of life
    irr::core::dimension2d<irr::f32> mEndLifeSize;

    irr::core::dimension2d<irr::f32> mCurrSpriteSize;
};

//struct for keeping all the data for a single non-animated
//flying explosion debris, each flying debris consists of multiple
//Scenenode sprites following each other on a certain path ejected
//away from the explosion
struct ExplosionFlyingDebrisStruct {
    std::vector<irr::scene::IBillboardSceneNode*> mSceneNodeDebrisSprite;
    std::vector<irr::core::vector3d<irr::f32>> currSpritePos;

    std::vector<irr::core::dimension2d<irr::f32>> mCurrSpriteSize;
    std::vector<irr::core::vector3df> currVelocity;
    std::vector<irr::f32> remainingLifeTime;

    irr::core::vector3df spawnPoint;

    irr::core::vector3df mVelocity;

    bool mHitTerrainAgain = false;

    irr::f32 detonationDelay;

    unsigned int currTexNrFlyingDebris = 0;

    irr::f32 timerAddNextSprite;

    bool currDetonating = false;
    bool detonated = false;
};

/************************
 * Forward declarations *
 ************************/

class ExplosionLauncher;

class Explosion {

        private:
            irr::core::vector3df targetLocation;

            //vector which holds all animated exploding fire balls at the
            //explosion location
            std::vector<ExplosionDetonationStruct*> mExplosionSpriteVec;

            //vector which holds all non animated but flying explosion debris
            //objects
            std::vector<ExplosionFlyingDebrisStruct*> mExplosionDebrisVec;

            ExplosionLauncher* mParentExplosionLauncher = nullptr;

            sf::Sound* mExplodeSound = nullptr;
            bool exploded = false;
            bool exploding = false;

            //adds a new sprite to a debris that is currently flying
            void AddNewSpriteToDebris(ExplosionFlyingDebrisStruct &debrisPntr, irr::core::vector3df newLocation, irr::core::vector3df currVel,
                                      irr::core::dimension2d<irr::f32> newSpriteSize, irr::f32 lifeTime);

        public:
            Explosion(irr::core::vector3df targetLoc, ExplosionLauncher* parentExpLauncher);
            ~Explosion();

            void UpdateDetonations(irr::f32 DeltaTime);
            void UpdateDebris(irr::f32 DeltaTime);

            bool objToBeDeleted = false;
};

#endif // EXPLOSION_H
