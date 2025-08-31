/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "numbereditbox.h"
#include "../editor/editormode.h"
#include "../editorsession.h"
#include "../editor.h"
#include <cwctype>
#include <cwchar>

NumberEditBox::NumberEditBox(EditorMode* parentMode, const wchar_t* text, const irr::core::rect<irr::s32>& rectangle,
                             bool border, irr::gui::IGUIElement* parent) {

    mParentMode = parentMode;
    mParentWindow = parent;

    //get next free GuiId number
    mGuiId = mParentMode->mParentSession->GetNextFreeGuiId();

    mEditBox = mParentMode->mParentSession->mParentEditor->mGuienv->addEditBox(text,
                              rectangle, border, mParentWindow, mGuiId);

    mEditBox->setMultiLine(false);
    mEditBox->setWordWrap(false);

    //Register myself in the Editor object
    //so that I get all Gui Events I need to function
    //correctly
    mParentMode->mParentSession->mParentEditor->RegisterNumberEditBox(this, mGuiId);
}

NumberEditBox::~NumberEditBox() {
    //before deleting myself I need to unregister myself from the
    //Editor object, so that we do not get further Gui Events
    //into no existing objects and crash
    mParentMode->mParentSession->mParentEditor->UnregisterNumberEditBox(mGuiId);

    mEditBox->remove();
    if (mLabel != nullptr) {
        mLabel->remove();
    }
}

void NumberEditBox::AddLabel(const wchar_t* text, const irr::core::rect<irr::s32>& rectangle) {
    mLabel = mParentMode->mParentSession->mParentEditor->mGuienv->addStaticText ( text,
                  rectangle, false, false, mParentWindow, -1, false );
}

void NumberEditBox::SetVisible(bool visible) {
    mEditBox->setVisible(visible);
    mEditBox->setEnabled(visible);
    if (mLabel != nullptr) {
        mLabel->setVisible(visible);
        mLabel->setEnabled(visible);
    }
}

void NumberEditBox::SetEnabled(bool enabled) {
    mEditBox->setEnabled(enabled);
}

//returns true if the new value was
//succesfully assigned, False otherwise
//(for example when value outside of limits)
bool NumberEditBox::SetValue(irr::s32 newValue) {
    if (mValidRangeCheck) {
        if ((newValue < mMinValidValue) || (newValue > mMaxValidValue)) {
            //invalid value
            return false;
        }
    }

    mLastValidValue = newValue;

    //update text in EditBox
    stringw newEntryStr(newValue);

    mEditBox->setText(newEntryStr.c_str());

    return true;
}

void NumberEditBox::SetValueLimit(irr::s32 minValidValue, irr::s32 maxValidValue) {
    mMinValidValue = minValidValue;
    mMaxValidValue = maxValidValue;
    mValidRangeCheck = true;

    //how many number of digits do we need to enter
    //the number with the highest possible value
    //one digit is minimum of course
    irr::s32 highestVal = abs(maxValidValue);

    if (abs(minValidValue) > highestVal) {
        highestVal = abs(minValidValue);
    }

    mMaxNrDigits = 1;

    irr::s32 divresult = highestVal;

    do {
        divresult = divresult / 10;
        if (divresult > 0) {
            mMaxNrDigits++;
        }
    } while (divresult > 0);

    mEditBox->setMax(mMaxNrDigits);
}

//tries to read the current text of the EditBox
//and to get a valid number
//Returns true if success, False otherwise
bool NumberEditBox::InterpretValue(irr::s32& readNumber) {
   stringw text = mEditBox->getText();

   irr::u32 textLen = text.size();

   wint_t currwChar;

   for (irr::u32 idx = 0; idx < textLen; idx++) {
       currwChar = (wint_t)(text[idx]);

       if (std::iswdigit(currwChar) == 0) {
           //non numeric char found! => invalid
           return false;
       }
   }

   //we only have digits => good, try to get number out of it
   irr::s32 currValue;

   std::swscanf( text.c_str(), L"%d", &currValue);

   if (mValidRangeCheck) {
       if ((currValue < mMinValidValue) || (currValue > mMaxValidValue)) {
           //invalid value
           return false;
       }
   }

   //we have a valid result
   readNumber = currValue;

   return true;
}

void NumberEditBox::OnElementFocused() {
   //When the user clicks into the EditBox
   //remember the current valid entered number
   irr::s32 newValue;

   if (InterpretValue(newValue)) {
       //we read a valid value
       mLastValidValue = newValue;
   }
}

irr::s32 NumberEditBox::GetValue() {
    return mLastValidValue;
}

void NumberEditBox::UpdateWithNewValue() {
  //try to read new entered value
   irr::s32 newValue;

   if (InterpretValue(newValue)) {
       //we read a valid value
       mLastValidValue = newValue;

       //inform Parent EditorMode about the new
       //received value
       if (mParentMode != nullptr) {
           mParentMode->OnNumberEditBoxNewValue(this, mLastValidValue);
       }
    } else {
       //user input is not valid
       //restore last entered valid EditBox Text
       //and do not modify last valid value
       stringw validEntryStr(mLastValidValue);

       mEditBox->setText(validEntryStr.c_str());
   }
}

void NumberEditBox::OnEditBoxEditEndedEvent() {
   //user finished to enter/edit number (text)
   UpdateWithNewValue();
}


