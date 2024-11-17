/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include <iostream>
#include <irrlicht/irrlicht.h>
#include "resources/levelfile.h"
#include "resources/entityitem.h"
#include "models/levelterrain.h"

#include "models/player.h"
#include "definitions.h"
#include "utils/boundingbox/collision.h"
#include "string.h"

#include "draw/hud.h"
#include "resources/texture.h"
#include "models/column.h"
#include "models/levelblocks.h"
#include "models/morph.h"
#include "wchar.h"

#include "utils/physics.h"

#include "draw/drawdebug.h"

#include "math.h"
#include "game.h"

using namespace std;

using namespace irr;
using namespace irr::core;
using namespace irr::video;
using namespace irr::scene;

class Game; //Forward declaration

Game* mGame;

int main()
{
    //create new game object
    mGame = new Game();

    //try to init game
    if (!mGame->InitGame())
       return 1;

    //init all ok
    //run the game in default way
    //via menue with next line
    mGame->RunGame();

    //uncomment next line to skip menue and to
    //enter race directly
    //mGame->DebugGame();

    return 0;
}
