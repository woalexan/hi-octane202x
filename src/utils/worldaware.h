/*
 ----------------------------------------------------------------------------------------------------------------------------
 Function CastRayDDA was initially taken from the great project/tutorial (and later modified slightly by me)
 For license also see worldaware-CastRayDDA-OLC3-LICENCE.md
 
 Fast Ray Casting Using DDA
//  "Itchy Eyes... Not blinking enough..." - javidx9

 Video: https://youtu.be/NbSee-XM7WA

 License (OLC-3)
 ~~~~~~~~~~~~~~~

    Copyright 2018 - 2021 OneLoneCoder.com

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions or derivations of source code must retain the above
    copyright notice, this list of conditions and the following disclaimer.

    2. Redistributions or derivative works in binary form must reproduce
    the above copyright notice. This list of conditions and the following
    disclaimer must be reproduced in the documentation and/or other
    materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Links
    ~~~~~
    YouTube:	https://www.youtube.com/javidx9
                https://www.youtube.com/javidx9extra
    Discord:	https://discord.gg/WhwHUMV
    Twitter:	https://www.twitter.com/javidx9
    Twitch:		https://www.twitch.tv/javidx9
    GitHub:		https://www.github.com/onelonecoder
    Homepage:	https://www.onelonecoder.com

    Author
    ~~~~~~
    David Barr, aka javidx9, ©OneLoneCoder 2019, 2020, 2021
    
 ----------------------------------------------------------------------------------------------------------------------------
 Function DrawLine source code taken from great article from Josh Beam
 https://joshbeam.com/articles/simple_line_drawing/
 He also has a GitHub repo for this
 https://github.com/joshb/linedrawing
 
 For the license please see the following file:
 worldaware-DrawLine-README.md
 
 ----------------------------------------------------------------------------------------------------------------------------
 
 Other source code in this file:
    
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef WORLDAWARE_H
#define WORLDAWARE_H

#include "irrlicht.h"
#include "stdint.h"
#include <vector>

#define RAY_HIT_NOTHING 0
#define RAY_HIT_TERRAIN 1
#define RAY_HIT_PLAYER 2

//if true allows world aware debugging
//(means a debugging picture is created in the background)
#define WA_ALLOW_DEBUGGING false

const irr::f32 WA_CP_PLAYER_NAVIGATIONAREASAFETYDISTANCE = 0.0f;

/************************
 * Forward declarations *
 ************************/

class Race;
class Player;

struct RayHitInfoStruct {
    uint8_t HitType = RAY_HIT_NOTHING;
    irr::f32 HitDistance;

    //in case RAY_HIT_PLAYER the following pntr
    //contains the pointer to the player that was hit
    //by the ray, or if multiple players were hit by rays, contains
    //the pointer to the player that should be selected by the players
    //target system
    Player* HitPlayerPntr = nullptr;
};

class WorldAwareness {

private:
    Race* mRace = nullptr;
    //my Irrlicht video driver
    irr::video::IVideoDriver *mDriver = nullptr;

    //my Irrlicht device
    irr::IrrlichtDevice* mDevice = nullptr;

    irr::video::IImage* staticWorld = nullptr;
    irr::video::IImage* wayPointLinkWorld = nullptr;
    irr::video::IImage* dynamicWorld = nullptr;
    irr::video::IImage* debugWorld = nullptr;

    //irr::core::plane3d<irr::f32>* XZPlane;

    void CreateStaticWorld();
    void CreateWayPointLinkWorld();
    void CreateDynamicWorld(Player* whichPlayer);

    irr::u32 worldSizeX;
    irr::u32 worldSizeY;

   /* irr::core::vector2d<irr::f32> Project3DPointTo2DPlane(
            irr::core::vector3d<irr::f32> Point3D, irr::core::plane3d<irr::f32> Plane2D);*/

    void DrawLine(irr::video::IImage &image, irr::video::SColor &color, irr::f32 x1, irr::f32 y1,
                         irr::f32 x2, irr::f32 y2);
    void DrawRectangle(irr::video::IImage &image, irr::video::SColor &color, irr::f32 x1, irr::f32 y1,
                         irr::f32 x2, irr::f32 y2);

    void DebugSavePicture(char* fileName, irr::video::IImage* image);
    void DrawPlayer(irr::video::IImage &image, irr::video::SColor &color, Player* whichPlayer);
    RayHitInfoStruct CastRay(irr::video::IImage &image, irr::core::vector3df startPos, irr::core::vector3df dirVec);
    RayHitInfoStruct CastRayDDA(irr::video::IImage &image, irr::core::vector3df startPos,
                                irr::core::vector3df dirVec, irr::f32 maxRange,
                                std::vector<irr::core::vector2di> &visitedCells);

    void SetPixelDynamicWorldMap(int playerNr, irr::f32 x1, irr::f32 y1);
    void DrawLineIntoDynamicWorldMap(int playerNr, irr::f32 x1, irr::f32 y1,
                         irr::f32 x2, irr::f32 y2);
    void DrawRectangleDynamicWorldMap(int playerNr, irr::f32 x1, irr::f32 y1,
                         irr::f32 x2, irr::f32 y2);
    void DrawPlayerDynamicWorldMap(int playerNr, Player* whichPlayer);

    //how many pixels we want per unit length in
    //3D world
    uint8_t PixelScaleFactor = 5;

    irr::video::SColor* colorEmptySpace = nullptr;
    irr::video::SColor* colorRed = nullptr;

    //predefined vector with colors for max 8 players
    std::vector<irr::video::SColor*> mColorPlayerVec;

    //static world map contains info about static
    //Terrain and Cubes; 0 means in the tile there is no obstacle
    //1 means there is an obstacle
    std::vector<uint8_t>* mStaticWorldMap = nullptr;

    //dynamic world map contains info about moving players
    //0 means there is no obstacle (no player)
    //if integer above 0, the number means the number of detected
    //player in this tile;
    std::vector<uint8_t>* mDynamicWorldMap = nullptr;

    //World map contains info about static
    //Terrain and Cubes + road curbside blocks. 0 means in the tile there is no obstacle
    //1 means there is an obstacle
    std::vector<uint8_t>* mWayPointLinksWorldMap = nullptr;

    void CreateStaticWorldMap();
    void UpdateDynamicWorldMap(Player* whichPlayer);

    //returns true if a track curbside was identified, False otherwise
    bool FindTrackCurbsideAlongCastRay(std::vector<irr::core::vector2di> cells,
                                  irr::core::vector3df rayStartPoint3D, irr::f32 &distanceToEnd);
    
public:
    WorldAwareness(irr::IrrlichtDevice* device, irr::video::IVideoDriver *driver, Race* race);
    ~WorldAwareness();

    void Analyse(Player *whichPlayer);

    void PreAnalyzeWaypointLinksOffsetRange();

    bool WriteOneDbgPic = false;
};

#endif // WORLDAWARE_H
