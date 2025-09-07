/*
 I implemented this source code in this file based on the following article and source code examples from Keith Peters
 https://www.bit-101.com/2017/2022/12/coding-curves-08-bezier-curves/

 Copyright (C) 2024 Wolf Alexander              (for the put together source code based on the Keith Peters article and his source code examples)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

 */

#ifndef BEZIER_H
#define BEZIER_H

#include "irrlicht.h"
#include <vector>

/************************
 * Forward declarations *
 ************************/

class LevelTerrain;
class DrawDebug;
struct WayPointLinkInfoStruct;
struct ColorStruct;

class Bezier
{

public:
    Bezier(LevelTerrain* terrain, DrawDebug* pntrDrawDebug);

    irr::core::vector2df QuadBezierPoint(irr::core::vector2df startPnt,
                                            irr::core::vector2df endPnt, irr::core::vector2df cntrlPoint, irr::f32 t);

    void QuadBezierCurveDrawAtTerrain(irr::core::vector2df startPnt, irr::core::vector2df endPnt,
                                        irr::core::vector2df cntrlPoint, irr::f32 resolution, ColorStruct *color, bool goThrough = true);

    std::vector<WayPointLinkInfoStruct*> QuadBezierCurveGetSegments(irr::core::vector2df startPnt, irr::core::vector2df endPnt,
                                        irr::core::vector2df cntrlPoint, irr::f32 resolution, ColorStruct *color, bool goThrough = true);

    //if this function is used the bezier curve is forced to go exactly through
    //the control point, and is not simply bend towards the control point
    irr::core::vector2df QuadBezierPointThrough(irr::core::vector2df startPnt, irr::core::vector2df endPnt, irr::core::vector2df cntrlPoint, irr::f32 t);

    irr::core::vector2df GetBezierCurvePlaningCoordMidPoint(irr::core::vector3df point1, irr::core::vector3df point2, irr::core::vector3df &threeDCoord);

private:
    //pointer to the levelTerrain object
    //we need this to be able to derive the height of the tiles
    //of the terrain
    LevelTerrain *mLevelTerrain = nullptr;
    DrawDebug *mDrawDebug = nullptr;
};

#endif // BEZIER_H
