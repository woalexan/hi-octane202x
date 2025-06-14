/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "drawdebug.h"

DrawDebug::DrawDebug(irr::video::IVideoDriver *driver) {
    myDriver = driver;

    red = new irr::video::SMaterial();
    red->MaterialType = irr::video::EMT_SOLID;
    red->AmbientColor = irr::video::SColor(255,255,0,0);

    green = new irr::video::SMaterial();
    green->MaterialType = irr::video::EMT_SOLID;
    green->AmbientColor = irr::video::SColor(255,0,255,0);

    blue = new irr::video::SMaterial();
    blue->MaterialType = irr::video::EMT_SOLID;
    blue->AmbientColor = irr::video::SColor(255,0,0,255);

    white = new irr::video::SMaterial();
    white->MaterialType = irr::video::EMT_SOLID;
    white->AmbientColor = irr::video::SColor(255,255,255,255);

    pink = new irr::video::SMaterial();
    pink->MaterialType = irr::video::EMT_SOLID;
    pink->AmbientColor = irr::video::SColor(255, 255, 0, 255);

    brown = new irr::video::SMaterial();
    brown->MaterialType = irr::video::EMT_SOLID;
    brown->AmbientColor = irr::video::SColor(255, 128, 64, 0);

    cyan = new irr::video::SMaterial();
    cyan->MaterialType = irr::video::EMT_SOLID;
    cyan->AmbientColor = irr::video::SColor(255, 0, 255, 255);

    orange = new irr::video::SMaterial();
    orange->MaterialType = irr::video::EMT_SOLID;
    orange->AmbientColor = irr::video::SColor(255, 255, 128, 0);

    origin = new irr::core::vector3df(0.0f, 0.0f, 0.0f);
    XAxis = new irr::core::vector3df(10.0f, 0.0f, 0.0f);
    YAxis = new irr::core::vector3df(0.0f, 10.0f, 0.0f);
    ZAxis = new irr::core::vector3df(0.0f, 0.0f, 10.0f);
}

void DrawDebug::Draw3DTriangle(const irr::core::triangle3df *triangle, irr::video::SColor color) {
    myDriver->draw3DTriangle(*triangle, color);
}

void DrawDebug::Draw3DTriangleOutline(const irr::core::triangle3df *triangle, irr::video::SMaterial* color) {
    myDriver->setMaterial(*color);
    myDriver->draw3DLine(triangle->pointA, triangle->pointB);
    myDriver->draw3DLine(triangle->pointB, triangle->pointC);
    myDriver->draw3DLine(triangle->pointC, triangle->pointA);
}

void DrawDebug::Draw3DLine(irr::core::vector3df startPos, irr::core::vector3df endPos, irr::video::SMaterial* color) {
    myDriver->setMaterial(*color);
    myDriver->draw3DLine(startPos, endPos);
}

void DrawDebug::Draw3DRectangle(irr::core::vector3df v1, irr::core::vector3df v2, irr::core::vector3df v3, irr::core::vector3df v4,
                     irr::video::SMaterial* color) {
    myDriver->setMaterial(*color);
    myDriver->draw3DLine(v1, v2);
    myDriver->draw3DLine(v2, v3);
    myDriver->draw3DLine(v3, v4);
    myDriver->draw3DLine(v4, v1);
}

DrawDebug::~DrawDebug() {
    delete ZAxis;
    delete YAxis;
    delete XAxis;
    delete origin;
    delete white;
    delete blue;
    delete green;
    delete red;
    delete pink;
    delete brown;
    delete cyan;
    delete orange;
}

irr::core::vector3df DrawDebug::GetOrthogonalVector(irr::core::vector3df inVec) {
  irr::f32 x = inVec.X;
  irr::f32 y = inVec.Y;
  irr::f32 z = inVec.Z;
  const double s = std::sqrt(x*x + y*y + z*z);
  const double g = std::copysign(s, z);  // note s instead of 1
  const double h = z + g;
  return irr::core::vector3df((irr::f32)(g*h - x*x), (irr::f32)(- x * y), (irr::f32)(- x * h));
}

void DrawDebug::Draw3DArrow(irr::core::vector3df startPos, irr::core::vector3df arrowPosition, irr::video::SMaterial* color) {
    myDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);

    myDriver->setMaterial(*color);
    myDriver->draw3DLine(startPos, arrowPosition);

    irr::core::vector3df dirVec = (startPos - arrowPosition);
    dirVec.normalize();

    dirVec += arrowPosition;

    irr::core::vector3df oneNormal = GetOrthogonalVector(dirVec);
    oneNormal.normalize();

    irr::core::vector3df otherVec = dirVec.crossProduct(oneNormal);
    otherVec.normalize();

    myDriver->draw3DLine(arrowPosition, dirVec + oneNormal);
    myDriver->draw3DLine(arrowPosition, dirVec - oneNormal);
    myDriver->draw3DLine(arrowPosition, dirVec + otherVec);
    myDriver->draw3DLine(arrowPosition, dirVec - otherVec);
}

void DrawDebug::DrawWorldCoordinateSystemArrows(void) {
    Draw3DArrow(*origin, *XAxis, red);
    Draw3DArrow(*origin, *YAxis, green);
    Draw3DArrow(*origin, *ZAxis, blue);
}
