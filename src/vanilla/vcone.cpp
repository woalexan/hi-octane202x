/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "vcone.h"
#include "../race.h"
#include "../game.h"
#include "vcalc.h"
#include "vthing.h"
#include "vvehicle.h"

//initPosition is in the original games (vanilla) coordinate
//system
VCone::VCone(irr::scene::ISceneManager* smgr, Race* race, irr::core::vector3d<irr::f32> initPosition) {
    mSmgr = smgr;
    mRace = race;

    //Create the Thing I need
    ThingData =
         mRace->mThingManager->thing_initialise_member(initPosition, 0.0f, 0.0f, 0.0f, 3, 6, -1);

    ThingData->Position = initPosition;
    ThingData->CollideSize.X = 0.15625f;
    ThingData->CollideSize.Y = 0.15625f;
    ThingData->CollideSize.Z = 0.3125f;
    ThingData->Count = 0;

    //get my mesh, and create my SceneNode
    mConeMesh = smgr->getMesh("extract/models/cone0-0.obj");
    mConeNode = smgr->addMeshSceneNode(mConeMesh);

    irr::core::vector3df irrPos =
            mRace->mVCalc->VanillaToIrrlichtCoord(ThingData->Position);

    mConeNode->setPosition(irrPos);
    mConeNode->setScale(irr::core::vector3d<irr::f32>(0.7f,0.7f,0.7f));
    mConeNode->setMaterialFlag(irr::video::EMF_LIGHTING, mRace->mGame->enableLightning);
    mConeNode->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);

    mModelOrientation = new VehicleViewStruct();
}

void VCone::UpdateSceneNode() {
    irr::core::vector3df irrPos =
            mRace->mVCalc->VanillaToIrrlichtCoord(ThingData->Position);

    mConeNode->setPosition(irrPos);
}

//Update/initialize final height after
//Terrain geometry information is available
void VCone::InitializeHeight() {
    //10.10.2026: Original Z coordinate shift was +0.1953125f
    //Had to adjust this number so that the cones are not floating in the air
    ThingData->Position.Z =
            mRace->mVCalc->map_floor(ThingData->Position) + 0.1703125f;

    UpdateSceneNode();
}

VCone::~VCone() {
  //remove my SceneNode from the Scene
  this->mConeNode->remove();

  if (ThingData != nullptr) {
        mRace->mThingManager->thing_remove(ThingData);
        ThingData = nullptr;
  }

  if (mModelOrientation != nullptr) {
      delete mModelOrientation;
      mModelOrientation = nullptr;
  }
}

void VCone::Update(irr::f32 frameDeltaTime) {
    irr::core::vector3df new_position;
    irr::core::vector3df displacement;
    irr::f32 xy_relative;
    irr::f32 xpos;
    irr::f32 ypos;
    irr::f32 v9;
    irr::f32 xy;
    irr::f32 v11;
    int8_t v13;
    int8_t v14;
    bool v15;
    int8_t v16;
    int8_t v17;
    irr::f32 v19;
    int8_t v20;
    irr::f32 v22;
    irr::f32 v24;
    irr::f32 v25;
    irr::f32 v26;
    irr::f32 zpos;
    irr::f32 v29;

    std::vector<VVehicle*>::iterator it;
    VVehicle* v4 = nullptr;

    if (!ThingData->Count) {
        //we loop through all the player vehicles
        it = mRace->mVanillaCraftVec.begin();

        while ((it != mRace->mVanillaCraftVec.end()) && ((ThingData->CollideSize.X + 0.390625f) <
               mRace->mVCalc->distance_get_rough_xy(ThingData->Position, (*it)->ThingData->Position))
               || ((*it)->ThingData->Position.Z >= (ThingData->Position.Z + ThingData->CollideSize.Z))) {

            ++it;

            if (it == mRace->mVanillaCraftVec.end()) {
                goto VConeUpdate_LABEL_7;
            }
        }

        if (it != mRace->mVanillaCraftVec.end()) {
            v4 = (*it);
        }
VConeUpdate_LABEL_7:
        if (v4 != nullptr) {
            ThingData->Displacement.X = v4->Momentum.DeltaX * 0.5f;
            ThingData->Displacement.Y = v4->Momentum.DeltaY * 0.5f;
            xy_relative = mRace->mVCalc->angle_get_xy_relative(ThingData->Displacement);
            xpos = ThingData->Displacement.X;
            ypos = ThingData->Displacement.Y;
            ThingData->Movement.AngleXY = xy_relative;
            ThingData->Displacement.Z = 0.12890625f;
            ThingData->Movement.SpeedActual = 71.109375f;
            v9 = xpos + ypos;
            if (v9 < 0.1953125f) {
                ThingData->Movement.SpeedActual = 364.0f * (ThingData->Displacement.X + ThingData->Displacement.Y);
                ThingData->Displacement.Z = 0.12890625f * (v9 / 0.1953125f);
            }
            ++ThingData->Count;
        }
    }

    if (ThingData->Count == 1) {
          xy = ThingData->Movement.AngleXY;
          v11 = ThingData->Movement.AngleZY + ((ThingData->Movement.SpeedActual / 256.0f) * 360.0f);
          ThingData->Movement.AngleZY = v11;
          mModelOrientation->AngleXY = xy;
          mModelOrientation->AngleZY = v11;
          mModelOrientation->AngleXZ = ThingData->Movement.AngleXZ;
          new_position = ThingData->Position;
          new_position += ThingData->Displacement;
          v13 = mRace->mVCalc->map_colide_direction(ThingData->Position, new_position);
          v14 = v13;
          v15 = (v13 == 0);
          v16 = (v13 & 1);
          if (v15) {
              ThingData->Displacement.Z -= 0.01953125f;
          } else {
             v15 = (v16 == 0);
             v17 = (v14 & 2);
             if (!v15) {
                if (ThingData->Displacement.X > 0.0f) {
                    v19 = (0.00390625f - ThingData->Displacement.X) * 0.5f;
                } else {
                    v19 = (-ThingData->Displacement.X) * 0.5f;
                }
                ThingData->Displacement.X = v19;
                new_position.X = ThingData->Position.X + v19;
                v17 = (v14 & 2);
             }
             v15 = (v17 == 0);
             v20 = (v14 & 4);
             if (!v15) {
                 if (ThingData->Displacement.Y > 0.0f) {
                     v22 = (0.00390625f - ThingData->Displacement.Y) * 0.5f;
                 } else {
                     v22 = (-ThingData->Displacement.Y) * 0.5f;
                 }
                 ThingData->Displacement.Y = v22;
                 new_position.Y = ThingData->Position.Y + v22;
                 v20 = (v14 & 4);
             }
             if (v20) {
                v24 = -0.46875f * ThingData->Displacement.Z;
                ThingData->Displacement.Z = v24;
                if (v24 < 0.0390625f) {
                    ThingData->Displacement.Z = 0.0f;
                }
                new_position.Z = ThingData->Displacement.Z +
                        mRace->mVCalc->map_floor(new_position);
                mRace->mVCalc->move_displacement_slope(ThingData->Position, displacement);
                if (displacement.X >= -1.0f) {
                  if (displacement.X >= 1.00390625f) {
                     displacement.X = 1.0f;
                  }
                } else {
                    displacement.X = -1.0f;
                }
                v25 = -1.0f;
                if (displacement.Y < -1.0f || (v25 = 1.0f, displacement.Y >= 1.00390625f)) {
                    displacement.Y = v25;
                }
                ThingData->Movement.SpeedActual *= 0.5f;
                ThingData->Displacement.X += (displacement.X / 16.0f);
                ThingData->Displacement.Y += (displacement.Y / 16.0f);
             }
          }
          mRace->mThingManager->mapwho_move(ThingData, new_position);
          mModelOrientation->Position = ThingData->Position;
          v26 = ThingData->Displacement.Y;
          ThingData->Displacement.X = 0.91796875f * ThingData->Displacement.X;
          ThingData->Displacement.Y = 0.91796875f * ThingData->Displacement.Y;
          zpos = ThingData->Displacement.Z;
          v29 = -1.0f;
          if (zpos < -1.0f || (v29 = 1.0f, zpos >= 1.00390625f)) {
              ThingData->Displacement.Z = v29;
          }
          if ((fabs(ThingData->Displacement.X) < 0.00390625f) &&
              (fabs(ThingData->Displacement.Y) < 0.00390625f) &&
              (fabs(ThingData->Displacement.Z) < 0.00390625f)) {
                  if (fabs(new_position.Z - mRace->mVCalc->map_floor(new_position)) < 0.00390625f) {
                      ThingData->Movement.AngleZY = 90.0f;
                      mModelOrientation->AngleXY = ThingData->Movement.AngleXY;
                      mModelOrientation->AngleZY = ThingData->Movement.AngleZY;
                      mModelOrientation->AngleXZ = ThingData->Movement.AngleXZ;
                      ThingData->Count = 0;
                      ThingData->Position.Z += 0.1171875f;
                      mModelOrientation->Position = ThingData->Position;
                  }
              }

          mRace->UpdateSceneNodeModel(mConeNode, mModelOrientation);
    }
}
