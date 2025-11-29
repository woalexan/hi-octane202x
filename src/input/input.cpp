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

#include "input.h"
#include "../utils/logging.h"

// This is the one method that we have to implement
bool MyEventReceiver::OnEvent(const SEvent& event)
{
    //is this a mouse event? Is only really used
    //in level editor, and not the game itself
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
    {
        //Forward this event to my parent
        //Infrastructure object
        mInfra->HandleMouseEvent(event);
    }

    // Remember whether each key is down or up
    if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
        KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

        //is the key currently locked, and the key was released?
        //if so we need to clear current locked condition
        if (KeyIsLockedCurr[event.KeyInput.Key]) {
            if (!event.KeyInput.PressedDown) {
                //unlock key again
                KeyIsLockedCurr[event.KeyInput.Key] = false;
            }
        }
    }

    //we want to process Irrlicht logging events in our own
    //logger class
    if (event.EventType == irr::EET_LOG_TEXT_EVENT) {
        switch (event.LogEvent.Level) {
                case irr::ELOG_LEVEL::ELL_ERROR: {
                    logging::Error(event.LogEvent.Text);
                    break;
                }
                case irr::ELOG_LEVEL::ELL_WARNING: {
                    logging::Warning(event.LogEvent.Text);
                    break;
                }

                case irr::ELOG_LEVEL::ELL_DEBUG: {
                    logging::Debug(event.LogEvent.Text);
                    break;
                }

                case irr::ELOG_LEVEL::ELL_INFORMATION:
                default: {
                    logging::Message("", logging::NORMAL, false, event.LogEvent.Text);
                    break;
                }
        }

        return true;
    }

    //is this a GUIElement event? Is only really used
    //in level editor, and not the game itself
    if (event.EventType == EET_GUI_EVENT) {
         //Forward this event to my parent
         //Infrastructure object

        //returns true if Gui Event should be canceled
        if (mInfra->HandleGuiEvent(event)) {
             //this Gui event should be canceled
             //this is done by returning true here
             return true;
         }
    }

    return false;
}

// This is used to check whether a key is being held down
bool MyEventReceiver::IsKeyDown(EKEY_CODE keyCode) const
{
    return KeyIsDown[keyCode];
}

MyEventReceiver::MyEventReceiver(InfrastructureBase* infra)
{
    mInfra = infra;

    for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i) {
        KeyIsDown[i] = false;
        KeyIsLockedCurr[i] = false;
    }
}

bool MyEventReceiver::IsKeyDownSingleEvent(EKEY_CODE keyCode) {
    //Key is already locked, do not generate another event
    //until key was released inbetween
    if (KeyIsLockedCurr[keyCode])
        return false;

    //key is not locked currently
    if (KeyIsDown[keyCode]) {
        //lock key
        KeyIsLockedCurr[keyCode] = true;
        return true;
    }

    return false;
}

