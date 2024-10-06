/*
 * OPL Bank Editor by Wohlstand, a free tool for music bank editing
 * Copyright (c) 2016-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef AIL2_GTL_K
#define AIL2_GTL_K

#include "ffmt_base.h"

/**
 * @brief Reader and Writer of the Audio Interfaces Library Global Timbre Library format
 */
class AIL_GTL final : public FmBankFormatBase
{
public:
    bool detect(const std::string &filePath, char* magic) override;
    FfmtErrCode loadFile(std::string filePath, FmBank &bank) override;
    FfmtErrCode saveFile(std::string filePath, FmBank &bank) override;
    int  formatCaps() const override;
    std::string formatName() const override;
    std::string formatExtensionMask() const override;
    std::string formatDefaultExtension() const override;
    BankFormats formatId() const override;
};

#endif // AIL2_GTL_K
