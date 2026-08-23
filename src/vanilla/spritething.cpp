/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "spritething.h"
#include "../race.h"
#include "../vanilla/vcalc.h"
#include "../resources/texture.h"

//***************************************************
//*   SpriteThing Base class                        *
//***************************************************

//initPosition and initMovement are in the original games (vanilla) coordinate
//system
SpriteThing::SpriteThing(irr::scene::ISceneManager* smgr, Race* race, irr::video::ITexture* spriteTex,
                               irr::core::vector3d<irr::f32> initPosition, MovementStruct initMovement) {
    mSmgr = smgr;
    mRace = race;
    mSpriteTex = spriteTex;
    mSpriteTexSize = mSpriteTex->getSize();

    mSceneNode = smgr->addBillboardSceneNode();
    mSceneNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR );
    mSceneNode->setMaterialTexture(0, mSpriteTex);
    mSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    mSceneNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    //mSceneNode->setDebugDataVisible(irr::scene::EDS_BBOX);

    //I want a random number between 0.8f and 1.2f
    //int rNum = rand();
    //irr::f32 rNumFloat = 0.8f + (float(rNum) / float (RAND_MAX))  * 0.4f;

    ThingData.Position = initPosition;
    ThingData.Movement = initMovement;
    ThingData.Displacement.set(0.0f, 0.0f, 0.0f);
    ThingData.TimeSlice = 0;
    ThingData.AffectStatus = 0;
    ThingData.Life = 0;

    irr::core::vector3df irrPos =
            mRace->mVCalc->VanillaToIrrlichtCoord(initPosition);

    mSceneNode->setPosition(irrPos);
    mSceneNode->setSize(irr::core::dimension2df(1.0f, 1.0f));

    //make the sprite completely visible
    mCurrVerticeColor.set(255, 255, 255, 255);
    mSceneNode->setColor(mCurrVerticeColor);
}

void SpriteThing::UpdateSceneNode() {
    irr::core::vector3df irrPos =
            mRace->mVCalc->VanillaToIrrlichtCoord(ThingData.Position);

    mSceneNode->setPosition(irrPos);
    //mSceneNode->setSize(mInitSize * rNumFloat);

    //make the sprite completely visible
    mCurrVerticeColor.set(255, 255, 255, 255);
    mSceneNode->setColor(mCurrVerticeColor);
}

SpriteThing::~SpriteThing() {
  //remove my SceneNode from the Scene
  this->mSceneNode->remove();
}

//Update returns true if particle still has remaining lifetime
//returns false if lifetime is over
bool SpriteThing::Update(irr::f32 frameDeltaTime) {
    IsExpired = true;
    return false;
}

//***************************************************
//* EffectVehicleThing class                        *
//***************************************************

//Update returns true if particle still has remaining lifetime
//returns false if lifetime is over
bool EffectVehicleThing::Update(irr::f32 frameDeltaTime) {
  irr::core::vector3df position;
  irr::f32 Ypos;
  int8_t v4;
  int8_t v5;
  int8_t v7;
  bool v6;

  position = ThingData.Position;
  mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY + 180.0f,
                          ThingData.Movement.AngleZY, ThingData.Movement.SpeedActual);
  mRace->mVCalc->move_displacement_xyz(position, ThingData.Displacement, 1);
  Ypos = ThingData.Displacement.Y;
  ThingData.Displacement.X -= (ThingData.Displacement.X / 64.0f);
  ThingData.Displacement.Y = Ypos - (Ypos / 64.0f);
  v4 = mRace->mVCalc->map_colide(position);
  v5 = v4;
  v6 = (v4 == 0);
  v7 = (v4 & 0x10);
  if (v6) {
      position.Z += 0.01953125f;
  } else {
      if (v7) {
        //Thing not needed anymore, recycle
        IsExpired = true;
        return 0;
      }

      if ((v5 & 8) != 0) {
          position.Z = mRace->mVCalc->map_floor(position);
          ThingData.Movement.AngleZY = 0.0f;
      }
  }

  ThingData.Life--;
  if (ThingData.Life < 0) {
      //Thing not needed anymore, recycle
      IsExpired = true;
      return 0;
  } else {
      ThingData.Movement.SpeedActual *= 0.75f;
      //mapwho_move(thing, &position);
      //Instead currently:
      ThingData.Position = position;
  }

  UpdateSceneNode();

  return 1;
}

EffectVehicleThing::~EffectVehicleThing() {
}


