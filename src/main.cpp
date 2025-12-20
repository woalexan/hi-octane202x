/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */


/*#include "resources/levelfile.h"
#include "resources/entityitem.h"
#include "models/levelterrain.h"
#include "models/player.h"
#include "utils/boundingbox/collision.h"
#include "draw/hud.h"
#include "resources/texture.h"
#include "models/column.h"
#include "models/levelblocks.h"
#include "models/morph.h"
#include "utils/physics.h"
#include "draw/drawdebug.h"
#include "math.h"*/

#include "game.h"
#include "definitions.h"
#include <vector>
#include <iostream>
#include "wchar.h"
#include "string.h"
#include <cstdint>
#include <list>
#include <algorithm>

using namespace std;

using namespace irr;
using namespace irr::core;
using namespace irr::video;
using namespace irr::scene;

class Game; //Forward declaration

Game* mGame = nullptr;

int main(int argc, char **argv)
{
    //create new game object
    mGame = new Game(argc, argv);

    //try to init most basic
    //game components, so that we
    //can show a first graphical screen
    //04.05.2025: Do not use XEffects right now, I have problem with too dark lightning
    if (!mGame->InitGameStep1(false))
       return 1;

    //run the game
    mGame->RunGame();

    delete mGame;

    return 0;
}
