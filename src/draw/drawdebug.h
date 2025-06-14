/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef DRAWDEBUG_H
#define DRAWDEBUG_H

#include <irrlicht.h>

class DrawDebug {

public:
    DrawDebug(irr::video::IVideoDriver *driver);
    ~DrawDebug();
    void DrawWorldCoordinateSystemArrows();
    void Draw3DArrow(irr::core::vector3df startPos, irr::core::vector3df arrowPosition, irr::video::SMaterial* color);
    void Draw3DLine(irr::core::vector3df startPos, irr::core::vector3df endPos, irr::video::SMaterial* color);
    void Draw3DTriangle(const irr::core::triangle3df *triangle, irr::video::SColor color);
    void Draw3DTriangleOutline(const irr::core::triangle3df *triangle, irr::video::SMaterial* color);
    void Draw3DRectangle(irr::core::vector3df v1, irr::core::vector3df v2, irr::core::vector3df v3, irr::core::vector3df v4,
                         irr::video::SMaterial* color);

    //colors for drawing debug arrows, lines etc
    irr::video::SMaterial *red = nullptr;
    irr::video::SMaterial *green = nullptr;
    irr::video::SMaterial *blue = nullptr;
    irr::video::SMaterial *white = nullptr;
    irr::video::SMaterial *pink = nullptr;
    irr::video::SMaterial *brown = nullptr;
    irr::video::SMaterial *cyan = nullptr;
    irr::video::SMaterial *orange = nullptr;

    irr::core::vector3df *origin = nullptr;

private:
    irr::video::IVideoDriver* myDriver = nullptr;

    irr::core::vector3df GetOrthogonalVector(irr::core::vector3df inVec);

    //coordinates for drawing 3D world coordinate system with arrows
    irr::core::vector3df *XAxis = nullptr;
    irr::core::vector3df *YAxis = nullptr;
    irr::core::vector3df *ZAxis = nullptr;
};

#endif // DRAWDEBUG_H
