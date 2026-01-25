/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MINIMAP_H
#define MINIMAP_H

#include "irrlicht.h"
#include <vector>
#include <string>

class InfrastructureBase; //Forward declaration
struct MapConfigStruct; //Forward declaration

class MiniMap {
public:
    MiniMap(InfrastructureBase* infra, irr::core::vector2di lowerRightCorner, MapConfigStruct* mapConfig, std::string levelRootDir);
    ~MiniMap();

    void DrawMiniMap(irr::f32 frameDeltaTime, std::vector<irr::core::vector2di> coordVec,
                     bool player0IsHuman);

    bool GetInitOk();

private:
    //the image for the base of the minimap
    //without the player location dots
    irr::video::ITexture* baseMiniMap = nullptr;
    irr::core::dimension2di miniMapSize;
    irr::core::vector2d<irr::s32> miniMapDrawLocation;

    irr::core::vector2di mLowerRightCorner;

    std::string mLevelRootDir;

    std::vector<irr::video::SColor*> mMiniMapMarkerColors;

    void CleanMiniMap();

    //Initializes the games original
    //minimap
    //returns true for success, false otherwise
    bool InitMiniMap();

    //holds the pixels coordinates inside the minimap texture
    //image which are actually used by non transparent pixels
    irr::core::rect<irr::s32> miniMapImageUsedArea;

    //precalculated value for miniMap so that we safe
    //unnecessary calculations during game
    irr::f32 miniMapPixelPerCellW;
    irr::f32 miniMapPixelPerCellH;

    irr::f32 miniMapAbsTime = 0.0f;
    bool miniMapBlinkActive = false;

    InfrastructureBase* mInfra = nullptr;
    MapConfigStruct* mMapConfig = nullptr;

    bool mMiniMapInitOk;

    //Searches for the used space inside the minimap picture
    //while removing unnecessary transparent columns of pixels
    //Parameters:
    //  miniMapTexture = pointer to the minimap texture
    //In case of an unexpected error this function returns succesFlag = false, True otherwise
    irr::core::rect<irr::s32> FindMiniMapImageUsedArea(irr::video::ITexture* miniMapTexture, bool &succesFlag);

    irr::core::dimension2di CalcPlayerMiniMapPosition(irr::core::vector2di inputCoord);
};


#endif // MINIMAP_H
