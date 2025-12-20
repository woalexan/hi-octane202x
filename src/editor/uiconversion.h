/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef UICONVERSION_H
#define UICONVERSION_H

#include "irrlicht.h"
#include "../definitions.h"

/************************
 * Forward declarations *
 ************************/

class Editor;

class UiConversion {
public:
    UiConversion(Editor* parentEditor);
    ~UiConversion();

    //Returns true if conversion was succesfull, False otherwise
    //The value is returned in reference parameter value
    bool GetUIntFromTableEntry(irr::gui::IGUITable* table, irr::s32 rowIdx, irr::s32 columnIdx, irr::u32 &value);

    //For any unexpected error (for example string is not a number and contains other characters as well) the
    //this function will return an empty string
    irr::core::stringw NumberStringLimitDecimalPlaces(irr::core::stringw inputStr, irr::u8 nrDecimalPlaces);
 
private:
    Editor* mParentEditor = nullptr;
};

#endif // UICONVERSION_H
