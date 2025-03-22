/*
 The main part of this source code was taken from Irrlicht Tutorial 4: Movement
 https://irrlicht.sourceforge.io/docu/example004.html
 Irrlicht Engine Documentation © 2003-2012 by Nikolaus Gebhardt.
 
 I afterwards added the IsKeyDownSingleEvent function myself, because I needed it :)
 
 Copyright (C) 2024 Wolf Alexander     (for IsKeyDownSingleEvent function)
 Irrlicht Engine Documentation © 2003-2012 by Nikolaus Gebhardt.

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef INPUT_H
#define INPUT_H

#include <irrlicht.h>

using namespace irr;

/*
To receive events like mouse and keyboard input, or GUI events like "the OK
button has been clicked", we need an object which is derived from the
irr::IEventReceiver object. There is only one method to override:
irr::IEventReceiver::OnEvent(). This method will be called by the engine once
when an event happens. What we really want to know is whether a key is being
held down, and so we will remember the current state of each key.
*/

class MyEventReceiver : public IEventReceiver
{
public:
    // This is the one method that we have to implement
    virtual bool OnEvent(const SEvent& event);

    // This is used to check whether a key is being held down
    virtual bool IsKeyDown(EKEY_CODE keyCode) const;

    // This function allows to check for a single event
    // keypress event. Another key press will only be generated
    // after the key was released first again
    bool IsKeyDownSingleEvent(EKEY_CODE keyCode);

    MyEventReceiver();

private:
    // We use this array to store the current state of each key
    bool KeyIsDown[KEY_KEY_CODES_COUNT];

    //we use this array to store which keys are currently locked out
    //for another activation, as the were already triggered and
    //need to be released first for another trigger possibility
    bool KeyIsLockedCurr[KEY_KEY_CODES_COUNT];
};

#endif // INPUT_H
