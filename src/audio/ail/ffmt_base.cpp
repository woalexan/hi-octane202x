/*
 * OPL Bank Editor by Wohlstand, a free tool for music bank editing
 * Copyright (c) 2016-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * OPN2 Bank Editor by Wohlstand, a free tool for music bank editing
 * Copyright (c) 2017-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Note Wolf Alexander: The source code below is not 100% the original code from the
 * initial author anymore, as I had to make changes to it to be able to compile it
 * on my system. */

#include "ffmt_base.h"

FmBankFormatBase::FmBankFormatBase() {}

FmBankFormatBase::~FmBankFormatBase()
{}

bool FmBankFormatBase::detect(const std::string&, char*)
{
    return false;
}

bool FmBankFormatBase::detectInst(const std::string &, char *)
{
    return false;
}

FfmtErrCode FmBankFormatBase::loadFile(std::string, FmBank &)
{
    return FfmtErrCode::ERR_NOT_IMPLEMENTED;
}

FfmtErrCode FmBankFormatBase::saveFile(std::string, FmBank &)
{
    return FfmtErrCode::ERR_NOT_IMPLEMENTED;
}

FfmtErrCode FmBankFormatBase::loadFileInst(std::string, FmBank::Instrument &, bool *)
{
    return FfmtErrCode::ERR_NOT_IMPLEMENTED;
}

FfmtErrCode FmBankFormatBase::saveFileInst(std::string, FmBank::Instrument &, bool)
{
    return FfmtErrCode::ERR_NOT_IMPLEMENTED;
}

int FmBankFormatBase::formatCaps() const
{
    return (int)FormatCaps::FORMAT_CAPS_NOTHING;
}

int FmBankFormatBase::formatInstCaps() const
{
    return (int)FormatCaps::FORMAT_CAPS_NOTHING;
}

std::string FmBankFormatBase::formatInstName() const
{
    return "Unknown format";
}

std::string FmBankFormatBase::formatInstModuleName() const
{
    return formatInstName();
}

std::string FmBankFormatBase::formatInstExtensionMask() const
{
    return "*.*";
}

std::string FmBankFormatBase::formatInstDefaultExtension() const
{
    return std::string();
}

std::string FmBankFormatBase::formatName() const
{
    return "Unknown format";
}

std::string FmBankFormatBase::formatModuleName() const
{
    return formatName();
}

std::string FmBankFormatBase::formatExtensionMask() const
{
    return "*.*";
}

std::string FmBankFormatBase::formatDefaultExtension() const
{
    return std::string();
}

BankFormats FmBankFormatBase::formatId() const
{
    return BankFormats::FORMAT_UNKNOWN;
}

InstFormats FmBankFormatBase::formatInstId() const
{
    return InstFormats::FORMAT_INST_UNKNOWN;
}

