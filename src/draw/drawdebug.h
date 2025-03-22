/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef DRAWDEBUG_H
#define DRAWDEBUG_H

#include <irrlicht.h>
#include <vector>

class DrawDebug {

public:
    DrawDebug(irr::video::IVideoDriver *driver);
    ~DrawDebug();
    void DrawWorldCoordinateSystemArrows();
    void Draw3DArrow(irr::core::vector3df startPos, irr::core::vector3df arrowPosition, irr::video::SMaterial* color);
    void Draw3DLine(irr::core::vector3df startPos, irr::core::vector3df endPos, irr::video::SMaterial* color);
    void Draw3DTriangle(const irr::core::triangle3df *triangle, irr::video::SColor color);
    void Draw3DSphere(irr::core::vector3df pos, irr::f32 radius = 0.05f);
    void Draw3DRectangle(irr::core::vector3df v1, irr::core::vector3df v2, irr::core::vector3df v3, irr::core::vector3df v4,
                         irr::video::SMaterial* color);

    //colors for drawing debug arrows, lines etc
    irr::video::SMaterial *red;
    irr::video::SMaterial *green;
    irr::video::SMaterial *blue;
    irr::video::SMaterial *white;
    irr::video::SMaterial *pink;
    irr::video::SMaterial *brown;
    irr::video::SMaterial *cyan;
    irr::video::SMaterial *orange;

    irr::core::vector3df *origin;

private:
    irr::video::IVideoDriver* myDriver;

    irr::core::vector3df GetOrthogonalVector(irr::core::vector3df inVec);

    //coordinates for drawing 3D world coordinate system with arrows
    irr::core::vector3df *XAxis;
    irr::core::vector3df *YAxis;
    irr::core::vector3df *ZAxis;
};

#endif // DRAWDEBUG_H
