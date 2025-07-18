/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MISSILE_H
#define MISSILE_H

#include <irrlicht.h>
#include <vector>
#include <SFML/Audio.hpp>

/************************
 * Forward declarations *
 ************************/

class Player;
class MissileLauncher;

#define DEF_MISSILE_SPEED 20.0f
#define DEF_MISSILE_SMOKEDIST 1.0f
#define DEF_MISSILE_SMOKESPRITEINITSIZE 0.2f
#define DEF_MISSILE_SMOKESPRITELIFETIMESEC 2.0f

//defines within what range (distance) from the target player
//the explosion must occur to cause damage to the target
//enemy player
#define DEF_MISSILE_DEALDAMAGE_DISTRANGE 4.0f

struct MissileSpriteStruct {
    irr::core::vector3d<irr::f32> mSpritePos;
    irr::video::ITexture* mSpriteTex = nullptr;
    irr::core::dimension2d<irr::u32> mSpriteTexSize;

    irr::video::SColor mCurrVerticeColor;

    irr::scene::IBillboardSceneNode* mSceneNode = nullptr;

    //initial sprite size after creation
    irr::core::dimension2d<irr::f32> mInitSize;

    //max sprite size at the end of life
    irr::core::dimension2d<irr::f32> mEndLifeSize;

    irr::core::dimension2d<irr::f32> mCurrSpriteSize;
    irr::f32 lifeTimeSecParam;
};

class Missile {

private:
    MissileLauncher* mParentLauncher = nullptr;
    irr::core::vector3df launchLocation;
    irr::core::vector3df targetLocation;
    irr::core::vector3df currentLocation;
    irr::core::vector3df travelDir;

    irr::scene::IBillboardSceneNode* mSceneNodeMissile = nullptr;

    std::vector<MissileSpriteStruct*> mMissileSpriteVec;

    irr::f32 flightTime = 0.0f;
    irr::f32 remainingDistToNextSmoke = DEF_MISSILE_SMOKEDIST;

    bool targetStillLocked;

    //Contains a pointer in case this missile is still
    //locked at an enemy player
    //nullptr if there is no lock, or the lock was lost
    //by the player that shoot the missle initially
    Player* mLockedPlayer = nullptr;

    sf::Sound* mExplodeSound = nullptr;
    bool exploded = false;

    void UpdateSmokeSprites(irr::f32 DeltaTime);
    void AddNewSmokeSprite(irr::core::vector3df newLocation);
    void RemoveSmokeSprite(MissileSpriteStruct* spriteToRemove);
    bool AreAllSmokeSpritesGone();
    void CheckForHitOfMissileTrigger(irr::core::vector3df explodedAtLocation);

public:
    Missile(MissileLauncher* mParentLauncher, irr::core::vector3df launchLoc, irr::core::vector3df targetLoc, bool targetLocked, Player* lockedPlayer);
    ~Missile();

    void Update(irr::f32 DeltaTime);

    bool objToBeDeleted = false;
};

class MissileLauncher {
public:
    MissileLauncher(Player* myParentPlayer, irr::scene::ISceneManager* smgr,  irr::video::IVideoDriver *driver);
    ~MissileLauncher();

    bool ready;
    bool shooting = false;

    void Trigger();
    void Update(irr::f32 DeltaTime);

    Player* mParent = nullptr;
    irr::scene::ISceneManager *mSmgr = nullptr;
    irr::video::IVideoDriver *mDriver = nullptr;

    irr::video::ITexture* mMissileTex = nullptr;
    irr::video::ITexture* mSmokeTex = nullptr;

    irr::core::dimension2d<irr::u32> mMissileTexSize;
    irr::core::dimension2d<irr::u32> mSmokeTexSize;

private:

    std::vector<Missile*> mCurrentMissilesVec;

    std::vector<irr::core::vector3df> GetMissileLaunchLocation(bool fireTwoMissiles);

    //Returns true in case of success
    //False otherwise
    bool LoadSprites();

    irr::f32 timeAccu = 0.0f;
    irr::f32 coolOffTime = 0.0f;

    sf::Sound* mShotSound = nullptr;
};

#endif // MISSILE_H
