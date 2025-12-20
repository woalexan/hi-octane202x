/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "editor.h"
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

class Editor; //Forward declaration

Editor* mEditor = nullptr;

int main(int argc, char **argv)
{
    //create new editor object
    mEditor = new Editor(argc, argv);

    //Init most basic stuff
    //using the Irrlicht Null device
    //Parse command line options
    //Start logging
    //Read also Game Xml config file
    if (!mEditor->InitStage1()) {
        //editor init failed
        return 1;
    }

    //Init final Irrlicht device
    if (!mEditor->InitStage2()) {
        //Stage 2 editor init failed
        return 1;
    }

    //Third initialization step
    //Locate original game
    //Define game directories
    //Make sure basic first game assets
    //are available
    if (!mEditor->InitStage3()) {
        //Stage 3 editor init failed
        return 1;
    }

    //try to init most basic
    //game components, so that we
    //can show a first graphical screen
    if (!mEditor->InitEditorStep1()) {
       delete mEditor;
       return 1;
    }

    //Continue initialization
    //and finally run the editor
    mEditor->RunEditor();

    delete mEditor;

    return 0;
}
