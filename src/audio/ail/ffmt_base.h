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

#ifndef FMBANKFORMATBASE_H
#define FMBANKFORMATBASE_H

#include <string>
#include "bank.h"
#include "ffmt_enums.h"

/*!
 * \brief Base class provides errors enum and commonly used headers
 */
class FmBankFormatBase
{
public:
    FmBankFormatBase();
    virtual ~FmBankFormatBase();

    virtual bool detect(const std::string &filePath, char* magic);
    virtual bool detectInst(const std::string &filePath, char* magic);

    virtual FfmtErrCode loadFile(std::string filePath, FmBank &bank);
    virtual FfmtErrCode saveFile(std::string filePath, FmBank &bank);

    virtual FfmtErrCode loadFileInst(std::string filePath, FmBank::Instrument &inst, bool *isDrum = 0);
    virtual FfmtErrCode saveFileInst(std::string filePath, FmBank::Instrument &inst, bool isDrum = false);

    virtual int         formatCaps() const;
    virtual std::string     formatName() const;
    virtual std::string     formatModuleName() const;
    virtual std::string     formatExtensionMask() const;
    virtual std::string     formatDefaultExtension() const;
    virtual BankFormats formatId() const;

    virtual int         formatInstCaps() const;
    virtual std::string     formatInstName() const;
    virtual std::string     formatInstModuleName() const;
    virtual std::string     formatInstExtensionMask() const;
    virtual std::string     formatInstDefaultExtension() const;
    virtual InstFormats formatInstId() const;
};

#endif // FMBANKFORMATBASE_H
