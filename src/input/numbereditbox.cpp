/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "numbereditbox.h"
#include "../editor/editormode.h"
#include "../editorsession.h"
#include "../editor.h"
#include "../editor/uiconversion.h"
#include "../utils/logging.h"
#include "../definitions.h"

NumberEditBox::NumberEditBox(EditorMode* parentMode, irr::u8 nrDecimalPlaces, const irr::core::rect<irr::s32>& rectangle,
                             bool border, irr::gui::IGUIElement* parent) {

    mParentMode = parentMode;
    mParentWindow = parent;
    mNrDecimalPlaces = nrDecimalPlaces;

    //get next free GuiId number
    mGuiId = mParentMode->mParentSession->GetNextFreeGuiId();

    irr::core::stringw text("0");
    if (nrDecimalPlaces > 0) {
        text.append(L".0");
    }

    mEditBox = mParentMode->mParentSession->mParentEditor->mGuienv->addEditBox(text.c_str(),
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
bool NumberEditBox::SetValue(irr::f32 newValue) {
    //if wrong SetValue function is used for this type of numberEditBox exit with
    //error message for developer
    if (mNrDecimalPlaces == 0) {
       logging::Error("Float type SetValue method called for integer NumberEditBox, Skip, Check for program error");
       return false;
    }

    if (mValidRangeCheck) {
        if ((newValue < mMinValidValue) || (newValue > mMaxValidValue)) {
            //invalid value
            return false;
        }
    }

    mLastValidValue = newValue;

    //update text in EditBox
    stringw newEntryStr(newValue);

    //limit number of shown decimal places
    stringw newEntryStrLimited(
                mParentMode->mParentSession->mParentEditor->mUiConversion->NumberStringLimitDecimalPlaces(newEntryStr, mNrDecimalPlaces));

    mEditBox->setText(newEntryStrLimited.c_str());

    return true;
}

//returns true if the new value was
//succesfully assigned, False otherwise
//(for example when value outside of limits)
bool NumberEditBox::SetValue(irr::s32 newValue) {
    //if wrong SetValue function is used for this type of numberEditBox exit with
    //error message for developer
    if (mNrDecimalPlaces > 0) {
       logging::Error("Integer type SetValue method called for float NumberEditBox, Skip, Check for program error");
       return false;
    }

    if (mValidRangeCheck) {
        if ((newValue < (irr::s32)(mMinValidValue)) || (newValue > (irr::s32)(mMaxValidValue))) {
            //invalid value
            return false;
        }
    }

    //to prevent random rounding errors flipping the number
    //shift internal stored float number by 0.2f
    mLastValidValue = (irr::f32)(newValue) + (irr::f32)(sgn(newValue)) * 0.2f;

    //update text in EditBox
    stringw newEntryStr(newValue);

    //limit number of shown decimal places
    stringw newEntryStrLimited(
                mParentMode->mParentSession->mParentEditor->mUiConversion->NumberStringLimitDecimalPlaces(newEntryStr, mNrDecimalPlaces));

    mEditBox->setText(newEntryStrLimited.c_str());

    return true;
}

irr::u8 NumberEditBox::CalcMaxNumberDigitsIntPart(irr::s32 integerNr) {
    irr::s32 divresult = integerNr;
    irr::u8 nrDigits = 1;

    do {
        divresult = divresult / 10;
        if (divresult > 0) {
            nrDigits++;
        }
    } while (divresult > 0);

    return nrDigits;
}

void NumberEditBox::SetValueLimit(irr::f32 minValidValue, irr::f32 maxValidValue) {
    //if wrong SetValueLimit function is used for this type of numberEditBox exit with
    //error message for developer
    if (mNrDecimalPlaces == 0) {
       logging::Error("Float type SetValueLimit method called for integer NumberEditBox, Skip, Check for program error");
       return;
    }

    mMinValidValue = minValidValue;
    mMaxValidValue = maxValidValue;
    mValidRangeCheck = true;

    //how many number of digits do we need to enter
    //the number with the highest possible value
    //one digit is minimum of course
    irr::f32 highestVal = abs(maxValidValue);

    if (abs(minValidValue) > highestVal) {
        highestVal = abs(minValidValue);
    }

    irr::s32 divresult = (irr::s32)(highestVal);

    //maximum length of input text is max number of digits for integer parts
    //plus one char for decimal point plus number of allowed decimal places
    mMaxNrDigits = CalcMaxNumberDigitsIntPart(divresult) + 1 + mNrDecimalPlaces;

    mEditBox->setMax(mMaxNrDigits);
}

void NumberEditBox::SetValueLimit(irr::s32 minValidValue, irr::s32 maxValidValue) {
    //if wrong SetValue function is used for this type of numberEditBox exit with
    //error message for developer
    if (mNrDecimalPlaces > 0) {
       logging::Error("Integer SetValueLimit method called for float NumberEditBox, Skip, Check for program error");
       return;
    }

    //to prevent random internal rounding errors
    //shift numbers by 0.2f
    mMinValidValue = (irr::f32)(minValidValue) + (irr::f32)(sgn(minValidValue)) * 0.2f;
    mMaxValidValue = (irr::f32)(maxValidValue) + (irr::f32)(sgn(maxValidValue)) * 0.2f;
    mValidRangeCheck = true;

    //how many number of digits do we need to enter
    //the number with the highest possible value
    //one digit is minimum of course
    irr::s32 highestVal = (irr::s32)(abs(maxValidValue));

    if ((irr::s32)(abs(minValidValue)) > highestVal) {
        highestVal = (irr::s32)(abs(minValidValue));
    }

    irr::s32 divresult = (irr::s32)(highestVal);

    //maximum length of input text is max number of digits for integer part
    mMaxNrDigits = CalcMaxNumberDigitsIntPart(divresult);

    mEditBox->setMax(mMaxNrDigits);
}

//tries to read the current text of the EditBox
//and to get a valid number
//Returns true if success, False otherwise
bool NumberEditBox::InterpretValueInt(irr::s32& readNumber) {
    //Something is wrong: InterpretValueInt function is used for float numberEditBox, exit with
    //error message for developer
    if (mNrDecimalPlaces > 0) {
       logging::Error("Integer InterpretValueInt method called for float NumberEditBox, Skip, Check for program error");
       return false;
    }

   stringw text = mEditBox->getText();

   //Does text contain a number?
   if (!mParentMode->mParentSession->mParentEditor->mUiConversion->StringContainsNumber(text, false)) {
       //no, some other characters as well
       return false;
   }

   //we only have digits => good, try to get number out of it
   irr::s32 currValue;

   std::swscanf( text.c_str(), L"%d", &currValue);

   if (mValidRangeCheck) {
       if ((currValue < (irr::s32)(mMinValidValue)) || (currValue > (irr::s32)(mMaxValidValue))) {
           //invalid value
           return false;
       }
   }

   //we have a valid result
   readNumber = currValue;

   return true;
}

//tries to read the current text of the EditBox
//and to get a valid number
//Returns true if success, False otherwise
bool NumberEditBox::InterpretValueFloat(irr::f32& readNumber) {
    //Something is wrong: InterpretValueFloat function is used for integer numberEditBox, exit with
    //error message for developer
    if (mNrDecimalPlaces == 0) {
       logging::Error("Float InterpretValueFloat method called for integer NumberEditBox, Skip, Check for program error");
       return false;
    }

   stringw text = mEditBox->getText();

   //Does text contain a number?
   //Allow float number here
   if (!mParentMode->mParentSession->mParentEditor->mUiConversion->StringContainsNumber(text, true)) {
       //no, contains invalid characters
       return false;
   }

   //limit number decimal points
   stringw textLimitDec(mParentMode->mParentSession->mParentEditor->mUiConversion->NumberStringLimitDecimalPlaces(text, mNrDecimalPlaces));

   //we only have digits => good, try to get number out of it
   irr::f32 currValue;

   std::swscanf( textLimitDec.c_str(), L"%f", &currValue);

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
   if (mNrDecimalPlaces == 0) {
       //integer type of NumberInputBox
       irr::s32 newValueInt;

       if (InterpretValueInt(newValueInt)) {
           //we read a valid value

           //to prevent random internal rounding errors
           //shift numbers by 0.2f
           mLastValidValue = (irr::f32)(newValueInt) + (irr::f32)(sgn(newValueInt)) * 0.2f;
       }

       return;
   }

   //float type of NumberInputBox
   irr::f32 newValueFloat;

   if (InterpretValueFloat(newValueFloat)) {
       //we read a valid value
       mLastValidValue = newValueFloat;
   }
}

irr::s32 NumberEditBox::GetValueInt() {
    //Something is wrong: GetValueInt function is used for float numberEditBox, exit with
    //error message for developer
    if (mNrDecimalPlaces > 0) {
       logging::Error("Integer GetValueInt method called for float NumberEditBox, Return 0, Check for program error");
       return 0;
    }

    return (irr::s32)(mLastValidValue);
}

irr::f32 NumberEditBox::GetValueFloat() {
    //Something is wrong: GetValueFloat function is used for integer numberEditBox, exit with
    //error message for developer
    if (mNrDecimalPlaces == 0) {
       logging::Error("Float GetValueFloat method called for integer NumberEditBox, Return 0.0f, Check for program error");
       return 0.0f;
    }

    return mLastValidValue;
}

void NumberEditBox::UpdateWithNewValue() {
  //try to read new entered value
  if (mNrDecimalPlaces == 0) {
      //integer type numberEditBox
      irr::s32 newValueInt;

      if (InterpretValueInt(newValueInt)) {
           //we read a valid value

          //to prevent random internal rounding errors
          //shift numbers by 0.2f
          mLastValidValue = (irr::f32)(newValueInt) + (irr::f32)(sgn(newValueInt)) * 0.2f;

          //inform Parent EditorMode about the new
          //received value
          if (mParentMode != nullptr) {
               mParentMode->OnIntNumberEditBoxNewValue(this, newValueInt);
           }
      }

      //Update EditBox Text
      //If invalid new number was entered by the user
      //this restores the last valid entered number
      //in the editBox
      stringw validEntryStr((irr::s32)(mLastValidValue));
      mEditBox->setText(validEntryStr.c_str());

      return;
  }

  //float type numberEditBox
  irr::f32 newValueFloat;

  if (InterpretValueFloat(newValueFloat)) {
      //we read a valid value
      mLastValidValue = newValueFloat;

      //inform Parent EditorMode about the new
      //received value
      if (mParentMode != nullptr) {
           mParentMode->OnFloatNumberEditBoxNewValue(this, newValueFloat);
      }
  }

  //Update EditBox Text
  //If invalid new number was entered by the user
  //this restores the last valid entered number
  //in the editBox
  stringw validEntryStr(mLastValidValue);

  //limit number of shown decimal places
  stringw validEntryStrLimited(
             mParentMode->mParentSession->mParentEditor->mUiConversion->NumberStringLimitDecimalPlaces(validEntryStr, mNrDecimalPlaces));

  mEditBox->setText(validEntryStrLimited.c_str());
}

void NumberEditBox::OnEditBoxEditEndedEvent() {
   //user finished to enter/edit number (text)
   UpdateWithNewValue();
}
