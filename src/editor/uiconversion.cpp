/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "uiconversion.h"
#include "../editor.h"
#include <cwctype>
#include <cwchar>

UiConversion::UiConversion(Editor* parentEditor) {
    mParentEditor = parentEditor;
}

UiConversion::~UiConversion() {
}

//Returns true if conversion was succesfull, False otherwise
bool UiConversion::GetUIntFromTableEntry(irr::gui::IGUITable* table, irr::s32 rowIdx, irr::s32 columnIdx, irr::u32 &value) {
    if (table == nullptr)
        return false;

    wchar_t* stopChar;

    irr::s32 nrRows = table->getRowCount();
    irr::s32 nrColumns = table->getColumnCount();

    if ((rowIdx < 0) || (rowIdx >= nrRows) || (columnIdx < 0) || (columnIdx >= nrColumns))
        return false;

    irr::core::stringw cellTxt = table->getCellText(rowIdx, columnIdx);

    //we use base 10!
    unsigned long i = std::wcstoul(cellTxt.c_str(), &stopChar, 10);

    //return the value
    value = (irr::u32)(i);

    return true;
}

//Returns true in case the input string contains a number, False if there are any other characters
//that are no digits;
bool UiConversion::StringContainsNumber(irr::core::stringw inputStr, bool allowDecimalNumber) {
    irr::u32 textLen = inputStr.size();

    wint_t currwChar;
    bool radixFound = false;

    for (irr::u32 idx = 0; idx < textLen; idx++) {
        currwChar = (wint_t)(inputStr[idx]);

        if (std::iswdigit(currwChar) == 0) {
            //non numeric char found! => invalid
            //could be a float?
            if (!allowDecimalNumber) {
                //for a non decimal number
                //we already can exit with invalid result
                return false;
            }

            if (currwChar == L'.') {
                //if we find a radix the second time we can
                //also exit
                if (radixFound) {
                    return false;
                }

                //one radix is ok, continue search
                radixFound = true;
            }
        }
    }

    return true;
}

//For any unexpected error (for example string is not a number and contains other characters as well) the
//this function will return an empty string
irr::core::stringw UiConversion::NumberStringLimitDecimalPlaces(irr::core::stringw inputStr, irr::u8 nrDecimalPlaces) {
    irr::core::stringw resultStr("");

    //if we have a missformed input string
    //simply return with empty string
    if (!StringContainsNumber(inputStr, true)) {
        return resultStr;
    }

    irr::s32 radixPos = inputStr.findLast(L'.');
    if (radixPos == -1) {
        //did not find radix, this number
        //has no decimal places, nothing to truncate
        //return full number
        return inputStr;
    }

    size_t lastCharPos = (size_t)(radixPos) + (size_t)(nrDecimalPlaces) + 1;

    //is the input string long enough?
    if (lastCharPos >= inputStr.size()) {
        //no, its not, return empty string
        return resultStr;
    }

    resultStr = inputStr.subString(0, lastCharPos);
    return resultStr;
}
