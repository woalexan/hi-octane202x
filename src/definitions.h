/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

//Please comment the following define out if you have problems with Freetype library, or you
//simply do not want to use it. Only drawback is that the level editor will use the built in
//font of Irrlicht Gui that is very small, and therefore difficult to read with the
//higher resolutions. But the software should still compile and run like this.
#define USE_FREETYPE

//User setup start

#define DEF_PLAYERCANGOBACKWARDS false

//Audio settings
#define AUDIO_SAMPLERATE 44100

//Length of all sides of the cubes in Terrain
//must be 1 for Hi-Octane !!
#define DEF_SEGMENTSIZE 1.0f

//User setup end

#include "irrlicht.h"

/************************
 * Forward declarations *
 ************************/
struct ColorStruct;

struct LineStruct {
  //std::string name;
  char* name = nullptr;
  irr::core::vector3d<irr::f32> A, B;
  ColorStruct *color = nullptr;

  irr::core::line3df irrLine;
};

enum {
    // I use this ISceneNode ID to indicate a scene node that is
    // not pickable by getSceneNodeAndCollisionPointFromRay()
    ID_IsNotPickable = 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be picked by ray selection.
    IDFlag_IsPickable = 1 << 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be highlighted.  In this example, the
    // homonids can be highlighted, but the level mesh can't.
    IDFlag_IsHighlightable = 1 << 1
};

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

#ifdef _MSC_VER 
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#endif // DEFINITIONS_H

