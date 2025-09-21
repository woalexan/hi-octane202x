/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "uiconversion.h"
#include "../editor.h"

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
