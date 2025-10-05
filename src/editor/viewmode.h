/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef VIEWMODE_H
#define VIEWMODE_H

#include "irrlicht.h"
#include "editormode.h"
#include <vector>
#include <cstdint>

/************************
 * Forward declarations *
 ************************/

class EditorSession;

/* GUI Elements for ViewMode
*/
struct GUIViewMode
{
    GUIViewMode ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUICheckBox* RunMorphsCheckbox;
    irr::gui::IGUICheckBox* FogCheckbox;
    irr::gui::IGUICheckBox* IlluminationCheckBox;
};

class ViewMode : public EditorMode {
private:
    virtual void CreateWindow();

public:
    ViewMode(EditorSession* parentSession);
    virtual ~ViewMode();

    //is called when the editor mode
    //is exited. Can be used to disable features
    //etc. that should only be active during this
    //mode is active
    virtual void OnExitMode();

    //is called when the editor mode
    //is entered (becomes active)
    virtual void OnEnterMode();

    virtual void OnCheckBoxChanged(irr::s32 checkboxId);

    GUIViewMode mGuiViewMode;
};

#endif // VIEWMODE_H
