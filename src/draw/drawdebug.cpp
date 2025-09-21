/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "drawdebug.h"
#include <cmath>

DrawDebug::DrawDebug(irr::video::IVideoDriver *driver) {
    myDriver = driver;

    red = AddColor(255,255,0,0);
    green = AddColor(255,0,255,0);
    blue = AddColor(255,0,0,255);
    black = AddColor(255,0,0,0);
    white = AddColor(255,255,255,255);
    pink = AddColor(255, 255, 0, 255);
    brown = AddColor(255, 128, 64, 0);
    cyan = AddColor(255, 0, 255, 255);
    orange = AddColor(255, 255, 128, 0);
    grey = AddColor(255, 195, 194, 190);

    colorShieldCharger = AddColor(255, 24, 254, 56);
    colorFuelCharger = AddColor(255, 121, 65, 198);
    colorAmmoCharger = AddColor(255, 254, 222, 48);

    origin = new irr::core::vector3df(0.0f, 0.0f, 0.0f);
    XAxis = new irr::core::vector3df(10.0f, 0.0f, 0.0f);
    YAxis = new irr::core::vector3df(0.0f, 10.0f, 0.0f);
    ZAxis = new irr::core::vector3df(0.0f, 0.0f, 10.0f);
}

ColorStruct* DrawDebug::AddColor(irr::u32 alpha, irr::u32 r, irr::u32 g, irr::u32 b) {
    ColorStruct* newColorStruct = new ColorStruct();
    newColorStruct->color = new irr::video::SColor(alpha, r, g, b);

    newColorStruct->material = new irr::video::SMaterial();
    newColorStruct->material->MaterialType = irr::video::EMT_SOLID;
    newColorStruct->material->AmbientColor = *newColorStruct->color;

    return newColorStruct;
}

void DrawDebug::CleanUpColor(ColorStruct* whichColor) {
    if (whichColor != nullptr) {
        if (whichColor->material != nullptr) {
            delete whichColor->material;
        }

        if (whichColor->color != nullptr) {
            delete whichColor->color;
        }

        delete whichColor;
    }
}

void DrawDebug::Draw3DTriangle(const irr::core::triangle3df *triangle, ColorStruct* color) {
    myDriver->draw3DTriangle(*triangle, *color->color);
}

void DrawDebug::Draw3DTriangleOutline(const irr::core::triangle3df *triangle, ColorStruct* color) {
    myDriver->setMaterial(*color->material);
    myDriver->draw3DLine(triangle->pointA, triangle->pointB);
    myDriver->draw3DLine(triangle->pointB, triangle->pointC);
    myDriver->draw3DLine(triangle->pointC, triangle->pointA);
}

void DrawDebug::Draw3DLine(irr::core::vector3df startPos, irr::core::vector3df endPos, ColorStruct* color) {
    myDriver->setMaterial(*color->material);
    myDriver->draw3DLine(startPos, endPos);
}

void DrawDebug::Draw3DRectangle(irr::core::vector3df v1, irr::core::vector3df v2, irr::core::vector3df v3, irr::core::vector3df v4,
                     ColorStruct* color) {
    myDriver->setMaterial(*color->material);
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

    CleanUpColor(white);
    CleanUpColor(black);
    CleanUpColor(blue);
    CleanUpColor(green);
    CleanUpColor(red);
    CleanUpColor(pink);
    CleanUpColor(brown);
    CleanUpColor(cyan);
    CleanUpColor(orange);
    CleanUpColor(grey);

    CleanUpColor(colorShieldCharger);
    CleanUpColor(colorFuelCharger);
    CleanUpColor(colorAmmoCharger);
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

void DrawDebug::Draw3DArrow(irr::core::vector3df startPos, irr::core::vector3df arrowPosition, irr::f32 arrowOffset, ColorStruct* color, irr::f32 arrowSize) {
    myDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);

    myDriver->setMaterial(*color->material);
    myDriver->draw3DLine(startPos, arrowPosition);

    irr::core::vector3df dirVec = (startPos - arrowPosition);
    dirVec.normalize();

    irr::core::vector3df dp = arrowPosition + dirVec * arrowSize + dirVec * arrowOffset;

    irr::core::vector3df oneNormal = GetOrthogonalVector(dp);
    oneNormal.normalize();

    irr::core::vector3df otherVec = dp.crossProduct(oneNormal);
    otherVec.normalize();

    myDriver->draw3DLine(arrowPosition, dp + oneNormal * arrowSize);
    myDriver->draw3DLine(arrowPosition, dp - oneNormal * arrowSize);
    myDriver->draw3DLine(arrowPosition, dp + otherVec * arrowSize);
    myDriver->draw3DLine(arrowPosition, dp - otherVec * arrowSize);
}

void DrawDebug::DrawWorldCoordinateSystemArrows(void) {
    Draw3DArrow(*origin, *XAxis, 0.0f, red);
    Draw3DArrow(*origin, *YAxis, 0.0f, green);
    Draw3DArrow(*origin, *ZAxis, 0.0f, blue);
}

void DrawDebug::DrawAround3DBoundingBox(irr::core::aabbox3df* boundingBox, ColorStruct* color) {
    if (boundingBox == nullptr)
        return;

    myDriver->setMaterial(*color->material);
    myDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);

    myDriver->draw3DBox(*boundingBox);
}
