/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MGUN_H
#define MGUN_H

#include <irrlicht.h>
#include "player.h"
#include "../audio/sound.h"
#include "../race.h"

//the machine gun damage should be choosen that a
//craft with full ammo needs continious machine gun for
//approx. 20 seconds, until it has no shield anymore
#define DEF_MGUN_DAMAGE 0.33f

const irr::f32 DEF_MGUN_REPETITION_DELAY = 0.1f;

class Player; //Forwards declaration

//the original game seems to allow to shoot maximum
//4 machine gun bullets at the same time
//therefore we need 4 SceneNodes for the impact
//sprites and 4 animators for the sprite animation
//each struct holds the info for one of the possible
//4 shoots at the same time
struct MachineGunBulletImpactStruct {
    irr::scene::IBillboardSceneNode* animSprite = nullptr;
    irr::scene::ISceneNodeAnimator *animator = nullptr;

    bool shooting = false;
    bool animatorActive = false;

    sf::Sound* mShotSound = nullptr;
};

class MachineGun {
public:
    MachineGun(Player* myParentPlayer, irr::scene::ISceneManager* smgr,  irr::video::IVideoDriver *driver);
    ~MachineGun();

    bool ready;

    void Trigger();
    void Update(irr::f32 DeltaTime);

    bool CoolDownNeeded();

    //we can not remove scenenodes which have still
    //an animations running when the race is over
    //the race object uses this function to wait until
    //all animations are done
    bool AllAnimationsFinished();

private:
    Player* mParent = nullptr;
    irr::scene::ISceneManager *mSmgr = nullptr;
    irr::video::IVideoDriver *mDriver = nullptr;

    std::vector<MachineGunBulletImpactStruct*> mBulletImpactVec;

    //returns nullptr if currently all 4 shoots are fired, and no impact
    //struct (sprite scenenode and animator) is currently available
    //otherwise returns a random picked available impact struct
    //random to make sure that the impact does not occur always at the same tile
    //location
    MachineGunBulletImpactStruct* GetCurrentlyAvailableImpactStruct();

    //Returns true in case of success
    //False otherwise
    bool LoadSprites();

    sf::Sound* mShotFailSound = nullptr;

    irr::core::array<irr::video::ITexture*> animTexList;

    irr::f32 timeAccu = 0.0f;
    irr::f32 coolOffTime = 0.0f;
    irr::f32 reductionValue;

    bool mCanShotAgain = true;
    irr::f32 shootAgainDelay;

    bool mCurrentFiring = false;
    bool mLastFiring = false;

    bool mGunOverheated = false;

    bool CpPlayerCurrentlyCoolDownNeeded = false;
};

#endif // MGUN_H
