/*
 I implemented this source code in this file based on the following article and source code examples from Keith Peters
 https://www.bit-101.com/2017/2022/12/coding-curves-08-bezier-curves/

 Copyright (C) 2024 Wolf Alexander              (for the put together source code based on the Keith Peters article and his source code examples)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

 */

#include "bezier.h"
#include "../models/levelterrain.h"
#include "../draw/drawdebug.h"

Bezier::Bezier(LevelTerrain* terrain, DrawDebug* pntrDrawDebug)
{
    mLevelTerrain = terrain;
    mDrawDebug = pntrDrawDebug;
}

void Bezier::QuadBezierCurveDrawAtTerrain(irr::core::vector2df startPnt, irr::core::vector2df endPnt,
                                          irr::core::vector2df cntrlPoint, irr::f32 resolution, ColorStruct *color, bool goThrough) {
  irr::core::vector3df drawPnt1(startPnt.X, 0.0f, startPnt.Y);
  irr::core::vector3df drawPnt2;
  irr::core::vector2df helper2D;
  irr::core::vector2di outCell;

  //get the correct Terrain Y coordinate from terrain
  drawPnt1.Y = mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
          (drawPnt1.X,
           drawPnt1.Z,
           outCell);

  for (irr::f32 t = resolution; t < 1.0f; t += resolution) {
      if (!goThrough) {
         helper2D = QuadBezierPoint(startPnt, endPnt, cntrlPoint, t);
      } else {
        helper2D = QuadBezierPointThrough(startPnt, endPnt, cntrlPoint, t);
      }
    drawPnt2.X = helper2D.X;
    drawPnt2.Z = helper2D.Y;

    //get the correct Terrain Y coordinate from terrain
    drawPnt2.Y = mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (drawPnt2.X,
             drawPnt2.Z,
             outCell);

    //draw this line in 3D space at the terrain
    mDrawDebug->Draw3DLine(drawPnt1, drawPnt2, color);

    //update drawPnt1, next line starts where the end point
    //of the last drawn line was
    drawPnt1 = drawPnt2;
  }
}

std::vector<WayPointLinkInfoStruct*> Bezier::QuadBezierCurveGetSegments(irr::core::vector2df startPnt, irr::core::vector2df endPnt,
                                          irr::core::vector2df cntrlPoint, irr::f32 resolution, ColorStruct *color,
                                              bool goThrough) {
  irr::core::vector3df drawPnt1(startPnt.X, 0.0f, startPnt.Y);
  irr::core::vector3df drawPnt2;
  irr::core::vector2df helper2D;
  irr::core::vector2di outCell;
  std::vector<WayPointLinkInfoStruct*> finalVec;
  finalVec.clear();

  //get the correct Terrain Y coordinate from terrain
  drawPnt1.Y = mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
          (drawPnt1.X,
           drawPnt1.Z,
           outCell);

  for (irr::f32 t = resolution; t <= 1.25f; t += resolution) {
      if (!goThrough) {
         helper2D = QuadBezierPoint(startPnt, endPnt, cntrlPoint, t);
      } else {
        helper2D = QuadBezierPointThrough(startPnt, endPnt, cntrlPoint, t);
      }
    drawPnt2.X = helper2D.X;
    drawPnt2.Z = helper2D.Y;

    //get the correct Terrain Y coordinate from terrain
    drawPnt2.Y = mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (drawPnt2.X,
             drawPnt2.Z,
             outCell);

    //create a new temporary waypointlink to control the movement
    //of the player
    WayPointLinkInfoStruct* newStruct = new WayPointLinkInfoStruct();

    LineStruct* newLineStr = new LineStruct();
    newLineStr->A = drawPnt1;
    newLineStr->B = drawPnt2;
    newLineStr->irrLine.setLine(newLineStr->A, newLineStr->B);

    //set white as default color
    newLineStr->color = color;
    newLineStr->name = new char[10];
    strcpy(newLineStr->name, "");

    newStruct->pLineStruct = newLineStr;
    irr::core::vector3df vec3D = (newLineStr->B - newLineStr->A);

    //precalculate and store length as we will need this very often
    //during the game loop for race position update
    newStruct->length3D = vec3D.getLength();
    vec3D.normalize();

    newStruct->LinkDirectionVec = vec3D;
    newStruct->LinkDirectionVec.normalize();

    //Idea: extend the lines a little bit further outwards at
    //both ends, so that when we project the players position on
    //the different segments later we always find a valid segment
    LineStruct* newLineExtStr = new LineStruct();
    newLineExtStr->A = drawPnt1;
    newLineExtStr->B = drawPnt2;

    //set white as default color
    newLineExtStr->color = color;
    newLineExtStr->name = new char[10];
    strcpy(newLineExtStr->name, "");

    newLineExtStr->A -= newStruct->LinkDirectionVec * 0.5f;
    newLineExtStr->B += newStruct->LinkDirectionVec * 0.5f;

    newStruct->pLineStructExtended = newLineExtStr;

    //add new segement to the vector of segments
    finalVec.push_back(newStruct);

    //update drawPnt1, next line starts where the end point
    //of the last drawn line was
    drawPnt1 = drawPnt2;
  }

  return finalVec;
}

irr::core::vector2df Bezier::QuadBezierPoint(irr::core::vector2df startPnt, irr::core::vector2df endPnt, irr::core::vector2df cntrlPoint, irr::f32 t) {
  irr::f32 m = (1.0f - t);
  irr::f32 a = m * m;
  irr::f32 b = 2 * m * t;
  irr::f32 c = t * t;
  //x = a * x0 + b * x1 + c * x2
  //y = a * y0 + b * y1 + c * y2

  irr::core::vector2df out = a * startPnt + b * cntrlPoint + c * endPnt;
  return out;
}

//if this function is used the bezier curve is forced to go exactly through
//the control point, and is not simply bend towards the control point
irr::core::vector2df Bezier::QuadBezierPointThrough(irr::core::vector2df startPnt, irr::core::vector2df endPnt, irr::core::vector2df cntrlPoint, irr::f32 t) {
  irr::f32 m = (1.0f - t);
  irr::f32 a = m * m;
  irr::f32 b = 2 * m * t;
  irr::f32 c = t * t;
  //x = a * x0 + b * x1 + c * x2
  //y = a * y0 + b * y1 + c * y2

  //calculate a new "control point" that is set in a way
  //that the final curve goes throw the specified control point
  irr::core::vector2df cntrlNew;
  //xc = x1 * 2 - x0 / 2 - x2 / 2
  //yc = y1 * 2 - y0 / 2 - y2 / 2

  cntrlNew = cntrlPoint * 2.0f - startPnt * 0.5f - endPnt * 0.5f;

  irr::core::vector2df out = a * startPnt + b * cntrlNew + c * endPnt;
  return out;
}

irr::core::vector2df Bezier::GetBezierCurvePlaningCoordMidPoint(irr::core::vector3df point1, irr::core::vector3df point2, irr::core::vector3df &threeDCoord) {
     irr::core::vector3df midPoint = (point2 - point1) * irr::core::vector3df(0.5f, 0.5f, 0.5f) +  point1;
     threeDCoord = midPoint;

    irr::core::vector2df result(midPoint.X, midPoint.Z);

    return result;
}
