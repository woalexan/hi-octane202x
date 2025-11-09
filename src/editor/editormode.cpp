/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "../editorsession.h"
#include "../editor.h"
#include "editormode.h"
#include "itemselector.h"

EditorMode::EditorMode(EditorSession* parentSession) {
    mParentSession = parentSession;
}

EditorMode::~EditorMode() {
}

bool EditorMode::IsWindowOpen() {
    if ((Window != nullptr) && !mWindowHidden)
        return true;

    return false;
}

irr::core::rect<irr::s32> EditorMode::GetWindowPosition() {
   if (Window != nullptr) {
    return Window->getAbsolutePosition();
   } else
    return (irr::core::rect<irr::s32>(0, 0, 0, 0));
}

void EditorMode::HideWindow() {
   if (Window == nullptr)
       return;

   Window->setEnabled(false);
   Window->setVisible(false);

   mWindowHidden = true;
}

void EditorMode::ShowWindow() {
    //does window already exist?
    //if not create it
    if (Window == nullptr) {
        CreateWindow();
    } else {
        if (mWindowHidden) {
            //unhide the existing window
            Window->setEnabled(true);
            Window->setVisible(true);
        }
    }

    mWindowHidden = false;
}

void EditorMode::CreateWindow() {
      dropElement ( Window );

      //get a unique Gui Id for my future window
      //in case I create one
      mGuiWindowId = this->mParentSession->GetNextFreeGuiId();
}

//when called the EditorMode is able to
//draw primitives via DrawDebug
void EditorMode::OnDraw() {
}

void EditorMode::OnTableSelected(irr::s32 elementId) {
}

void EditorMode::OnButtonClicked(irr::s32 buttonGuiId) {
}

void EditorMode::OnNewLevelItemSelected(CurrentlySelectedEditorItemInfoStruct newItemSelected) {
}

void EditorMode::OnLeftMouseButtonDown() {
}

void EditorMode::OnElementHovered(irr::s32 hoveredGuiId) {
}

void EditorMode::OnElementLeft(irr::s32 leftGuiId) {
}

void EditorMode::OnDrawSelectedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrSelectedItem) {
}

void EditorMode::OnDrawHighlightedLevelItem(CurrentlySelectedEditorItemInfoStruct* mCurrHighlightedItem) {
}

void EditorMode::OnCheckBoxChanged(irr::s32 checkboxId) {
}

void EditorMode::OnEditBoxEnterEvent(irr::s32 editBoxId) {
}

void EditorMode::OnElementFocusLost(irr::s32 elementId) {
}

void EditorMode::OnKeyPressedInWindow(irr::EKEY_CODE whichKeyPressed) {
}

void EditorMode::OnIntNumberEditBoxNewValue(NumberEditBox* whichBox, irr::s32& newValue) {
}

void EditorMode::OnFloatNumberEditBoxNewValue(NumberEditBox* whichBox, irr::f32& newValue) {
}

//is called when the editor mode
//is exited. Can be used to disable features
//etc. that should only be active during this
//mode is active
void EditorMode::OnExitMode() {
}

//is called when the editor mode
//is entered (becomes active)
void EditorMode::OnEnterMode() {
}
