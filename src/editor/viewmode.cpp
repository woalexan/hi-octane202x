/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "viewmode.h"
#include "../editorsession.h"
#include "../editor.h"
#include <iostream>

ViewMode::ViewMode(EditorSession* parentSession) : EditorMode(parentSession) {
     mModeNameStr.append(L"Viewmode");
}

ViewMode::~ViewMode() {
    //make sure my window is hidden at the end
    HideWindow();

    if (mGuiViewMode.RunMorphsCheckbox != nullptr) {
        mGuiViewMode.RunMorphsCheckbox->remove();
    }

    if (mGuiViewMode.FogCheckbox != nullptr) {
        mGuiViewMode.FogCheckbox->remove();
    }

    if (mGuiViewMode.IlluminationCheckBox != nullptr) {
        mGuiViewMode.IlluminationCheckBox->remove();
    }

    if (Window != nullptr) {
        //remove the window of this Mode object
        Window->remove();
    }
}

void ViewMode::CreateWindow() {
    irr::core::dimension2d<irr::u32> dim ( 150, 150 );

    //finally create the window
    Window = mParentSession->mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"View Mode", 0, mGuiWindowId);

    //create the checkbox which allows to disable/enable running
    //Morphs
    mGuiViewMode.RunMorphsCheckbox = mParentSession->mParentEditor->mGuienv->addCheckBox(true, rect<s32> ( 10, 30, dim.Width - 10, 55),
                                                                                         Window, GUI_ID_VIEWMODEWINDOW_RUNMORPH_CHECKBOX, L"Run Morphs");

    mGuiViewMode.FogCheckbox = mParentSession->mParentEditor->mGuienv->addCheckBox(false, rect<s32> ( 10, 65, dim.Width - 10, 90),
                                                                                         Window, GUI_ID_VIEWMODEWINDOW_FOG_CHECKBOX, L"Fog");

    mGuiViewMode.IlluminationCheckBox = mParentSession->mParentEditor->mGuienv->addCheckBox(true, rect<s32> ( 10, 100, dim.Width - 10, 125),
                                                                                            Window, GUI_ID_VIEWMODEWINDOW_ILLUMINATION_CHECKBOX, L"Illumination");

    //move window to a better start location
    Window->move(irr::core::vector2d<irr::s32>(1080,200));
}

void ViewMode::OnExitMode() {
    //make sure to turn off morphs
    //and reset Mesh of level to default
    //state
    mParentSession->DeactivateMorphs();
}

//is called when the editor mode
//is entered (becomes active)
void ViewMode::OnEnterMode() {
   //when this mode is activated the first
   //time create the window
   if (Window == nullptr) {
       CreateWindow();
   } else {
       //make sure existing window is not
       //hidden
       this->ShowWindow();
   }

   //run morphs in ViewMode?
   if (mGuiViewMode.RunMorphsCheckbox->isChecked()) {
     if (!mParentSession->IsMorphingRunning()) {
           //run morphs in ViewMode
           //if not deactivated by the user
           //in the dialog window
           mParentSession->ActivateMorphs();
     }
   }

   //enable Fog?
   mParentSession->SetFog(mGuiViewMode.FogCheckbox->isChecked());

   if (!mGuiViewMode.IlluminationCheckBox->isChecked()) {
       mParentSession->SetIllumination(false);
   } else {
       if (!mParentSession->IsIlluminationEnabled()) {
           mParentSession->SetIllumination(true);
       }
   }
}

void ViewMode::OnCheckBoxChanged(irr::s32 checkboxId) {
    if (checkboxId == GUI_ID_VIEWMODEWINDOW_RUNMORPH_CHECKBOX) {
        //is the checkbox now checked or not?
        if (mGuiViewMode.RunMorphsCheckbox->isChecked()) {
            //make sure morphing is enabled
            if (!mParentSession->IsMorphingRunning()) {
                mParentSession->ActivateMorphs();
            }
        } else {
            //morphs were disabled by the user
            //in the checkbox
            mParentSession->DeactivateMorphs();
        }
    } else if (checkboxId == GUI_ID_VIEWMODEWINDOW_FOG_CHECKBOX) {
        //Fog enabled or disabled?
        bool fogEnabled = mGuiViewMode.FogCheckbox->isChecked();

        mParentSession->SetFog(fogEnabled);
    } else if (checkboxId == GUI_ID_VIEWMODEWINDOW_ILLUMINATION_CHECKBOX) {
        //Illumination enabled or disabled?
        bool illuminationEnabled = mGuiViewMode.IlluminationCheckBox->isChecked();

        mParentSession->SetIllumination(illuminationEnabled);
    }
}
