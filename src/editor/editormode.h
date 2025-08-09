/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef EDITORMODE_H
#define EDITORMODE_H

#include "irrlicht.h"
#include "../definitions.h"
#include <vector>
#include <cstdint>

//! Macro for save Dropping an Element
#define dropElement(x)	if (x) { x->remove(); x = 0; }

/************************
 * Forward declarations *
 ************************/

class EditorSession;
struct CurrentlySelectedEditorItemInfoStruct;

class EditorMode {
private:
    bool mWindowHidden = false;

    virtual void CreateWindow();
    
public:
    EditorMode(EditorSession* parentSession);
    virtual ~EditorMode();

    irr::gui::IGUIWindow* Window = nullptr;
    irr::s32 mGuiWindowId;

    bool IsWindowOpen();
    irr::core::rect<irr::s32> GetWindowPosition();
    
    //we need the following two Gui events
    virtual void OnElementHovered(irr::s32 hoveredGuiId);
    virtual void OnElementLeft(irr::s32 leftGuiId);
    virtual void OnButtonClicked(irr::s32 buttonGuiId);
    virtual void OnCheckBoxChanged(irr::s32 checkboxId);

    virtual void OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem);
    virtual void OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem);
    
    virtual void OnLeftMouseButtonDown();

    virtual void OnNewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected);

    void HideWindow();
    void ShowWindow();

    //is called when the editor mode
    //is exited. Can be used to disable features
    //etc. that should only be active during this
    //mode is active
    virtual void OnExitMode();

    //is called when the editor mode
    //is entered (becomes active)
    virtual void OnEnterMode();
    
    EditorSession* mParentSession = nullptr;
};

#endif // EDITORMODE_H
