/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PATH_H
#define PATH_H

#include "../race.h"
#include "../draw/drawdebug.h"

class Race;       //Forward declaration
class DrawDebug;  //Forward declaration

class Path {
public:
    Path(Race* parentRace, DrawDebug* pntrDrawDebug);

private:
    Race* mRace;
    DrawDebug* mDrawDebug;

};

#endif // PATH_H
