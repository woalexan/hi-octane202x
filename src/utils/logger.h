/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef LOGGER_H
#define LOGGER_H

#include "irrlicht.h"
#include <vector>
//#include "SFML/System.hpp"
//#include <algorithm>
//#include <cstdint>
#include "wchar.h"
#include "../models/player.h"

struct LogEntryStruct {
    wchar_t* logText = nullptr;
    irr::u32 logTextLen;
};

class Logger {
    
public:
    Logger(irr::gui::IGUIEnvironment* guienvPntr, irr::core::rect<irr::s32> logWindowPos);
    ~Logger();

    void Render();
    void AddLogMessage(char* logText);
    void HideWindow();
    void ShowWindow();
    bool IsWindowHidden();
    
private:
    irr::core::rect<irr::s32> mCurrentLogTextOutputWindowPos;

    irr::gui::IGUIEnvironment* mGuiEnv = nullptr;
    irr::gui::IGUIStaticText* logTextOutputWindow = nullptr;

    std::vector<LogEntryStruct*> mCurrentLogEntries;

    irr::u32 mNumberOfLastLogLinesShown;
    irr::u32 mCurrentLogMsgCount;

    bool mWindowHidden;
};

#endif // LOGGER_H
