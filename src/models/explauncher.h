/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef EXPLAUNCHER_H
#define EXPLAUNCHER_H

#include "irrlicht.h"
#include <vector>
#include "../audio/sound.h"

/************************
 * Forward declarations *
 ************************/

class Race;
class Explosion;

class ExplosionLauncher {
    public:
        ExplosionLauncher(Race* myParentRace, irr::scene::ISceneManager* smgr,  irr::video::IVideoDriver *driver);
        ~ExplosionLauncher();

        bool ready;
        bool exploding = false;

        void Trigger(irr::core::vector3df targetLocation);
        void Update(irr::f32 DeltaTime);

        Race* mParentRace = nullptr;
        irr::scene::ISceneManager *mSmgr = nullptr;
        irr::video::IVideoDriver *mDriver = nullptr;

        irr::core::array<irr::video::ITexture*> animTexList;

    private:

        std::vector<Explosion*> mCurrentExplosionVec;

        //Returns true in case of success
        //False otherwise
        bool LoadSprites();

        irr::f32 timeAccu = 0.0f;
        irr::f32 coolOffTime = 0.0f;

        sf::Sound* mExplosionSound = nullptr;
};

#endif // EXPLAUNCHER_H

