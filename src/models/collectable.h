/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef COLLECTABLE_H
#define COLLECTABLE_H

#include "../resources/levelfile.h"
#include "../resources/mapentry.h"
#include "../race.h"

class Race; //Forward declaration

class Collectable {
public:
    Collectable(Race* race, EntityItem* entityItem, int number, vector3d<irr::f32> pos,
                irr::scene::ISceneManager* mSmgr, irr::video::IVideoDriver *driver);
    ~Collectable();

    vector3d<irr::f32> Position;

    irr::core::dimension2d<irr::u32> texturesize;
    vector3d<irr::f32> m_Size;

    irr::video::ITexture* collectable_tex;

    irr::scene::IBillboardSceneNode *billSceneNode;
    irr::core::aabbox3df boundingBox;

    bool GetIfVisible();
    void PickedUp();
    void Trigger();

    EntityItem* mEntityItem;

    irr::core::vector2df GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord);

private:
    Race* mRace;

    bool mEnableLightning;

protected:
    irr::video::IVideoDriver* m_driver;
    std::string m_texfile;
    irr::scene::ISceneManager *m_smgr;

    //default is non visible after
    //start of game, and before entity group
    //1 is triggered (group 1 is triggered once
    //at game start to make initial collectables visible)
    bool isVisible;
};

#endif // COLLECTABLE_H
