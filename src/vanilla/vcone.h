/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef VCONE_H
#define VCONE_H

#include <irrlicht.h>
#include <vector>

//Forward declaration
class Race;
class VThing;
struct VehicleViewStruct;

class VCone {
public:
    //initPosition is in the original games (vanilla) coordinate
    //system
    VCone(irr::scene::ISceneManager* smgr, Race* race, irr::core::vector3d<irr::f32> initPosition);

    ~VCone();

    //Update/initialize final height after
    //Terrain geometry information is available
    void InitializeHeight();

    void Update(irr::f32 frameDeltaTime);

private:
     VThing* ThingData = nullptr;

     irr::scene::IAnimatedMesh*  mConeMesh = nullptr;
     irr::scene::IMeshSceneNode* mConeNode = nullptr;

     irr::scene::ISceneManager* mSmgr = nullptr;
     Race* mRace = nullptr;

     VehicleViewStruct* mModelOrientation = nullptr;

     void UpdateSceneNode();
};

#endif // VCONE_H
