/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef NUMBEREDITBOX_H
#define NUMBEREDITBOX_H

#include <irrlicht.h>

using namespace irr;

/************************
 * Forward declarations *
 ************************/

class EditorMode;

class NumberEditBox
{
public:
    NumberEditBox(EditorMode* parentMode, const wchar_t* text, const irr::core::rect<irr::s32>& rectangle,
                  bool border=true, irr::gui::IGUIElement* parent=0);
    ~NumberEditBox();

    void AddLabel(const wchar_t* text, const irr::core::rect<irr::s32>& rectangle);

    void SetValueLimit(irr::s32 minValidValue, irr::s32 maxValidValue);
    irr::s32 GetValue();

    //returns true if the new value was
    //succesfully assigned, False otherwise
    //(for example when value outside of limits)
    bool SetValue(irr::s32 newValue);

    void OnEditBoxEditEndedEvent();
    void OnElementFocused();

    void SetVisible(bool visible);
    void SetEnabled(bool enabled);

private:
    irr::gui::IGUIEditBox* mEditBox = nullptr;
    irr::s32 mGuiId;

    irr::gui::IGUIElement* mParentWindow = nullptr;

    //has also an optional built in label
    irr::gui::IGUIStaticText* mLabel = nullptr;

    EditorMode* mParentMode = nullptr;

    irr::s32 mLastValidValue;

    irr::s32 mMinValidValue;
    irr::s32 mMaxValidValue;
    bool mValidRangeCheck = false;

    irr::u8 mMaxNrDigits;

    //tries to read the current text of the EditBox
    //and to get a valid number
    //Returns true if success, False otherwise
    bool InterpretValue(irr::s32& readNumber);

    void UpdateWithNewValue();
};

#endif // NUMBEREDITBOX_H
