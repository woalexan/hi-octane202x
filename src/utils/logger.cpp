#include "logger.h"

Logger::Logger(irr::gui::IGUIEnvironment* guienvPntr, irr::core::rect<irr::s32> logWindowPos) {
    mGuiEnv = guienvPntr;
    mCurrentLogTextOutputWindowPos = logWindowPos;

    mCurrentLogEntries.clear();
    mCurrentLogMsgCount = 0;

    mWindowHidden = true;

    //create the log window
    this->logTextOutputWindow = mGuiEnv->addStaticText(L"",
           mCurrentLogTextOutputWindowPos, false, true, NULL, -1, true);

    //we need to deactivate word wrap because if we do not
    //there seems to be an Irrlicht bug or problem
    //that causes an unexpected malloc assert problems somehow
    this->logTextOutputWindow->setWordWrap(false);

    //how many lines of log messages can we max show at a time
    //depends on the current logWindow size, and font height
    irr::u32 fontHeight =
            this->logTextOutputWindow->getActiveFont()->getDimension(L"Lgq!").Height;

    mNumberOfLastLogLinesShown = logWindowPos.getHeight() / fontHeight;

    //initially hide window
    this->logTextOutputWindow->setVisible(false);
}

Logger::~Logger() {
    //cleanup all existing log messages
    std::vector<LogEntryStruct*>::iterator it;
    LogEntryStruct* pntr;

    for (it = this->mCurrentLogEntries.begin(); it != this->mCurrentLogEntries.end(); ++it) {
        pntr = (*it);

        it = this->mCurrentLogEntries.erase(it);

        //free text itself
        delete[] pntr->logText;

        //free LogEntryStruct
        delete pntr;
    }
}

bool Logger::IsWindowHidden() {
    return (mWindowHidden);
}

void Logger::HideWindow() {
    mWindowHidden = true;
    this->logTextOutputWindow->setVisible(false);
}

void Logger::ShowWindow() {
   mWindowHidden = false;
   this->logTextOutputWindow->setVisible(true);
}

void Logger::AddLogMessage(char* logText) {
   LogEntryStruct* newLogMsg = new LogEntryStruct();

   size_t textLen = strlen(logText) + 2;
   newLogMsg->logText = new wchar_t[textLen];

   mbstowcs(newLogMsg->logText, logText, textLen);
   wcscat(newLogMsg->logText, L"\n");

   newLogMsg->logTextLen = wcslen(newLogMsg->logText);

   //add new log message to the vector of current
   //log messages
   this->mCurrentLogEntries.push_back(newLogMsg);

   mCurrentLogMsgCount++;
}

void Logger::Render() {
    //if window is hidden, we do not need to render it
    if (mWindowHidden)
        return;

    if (mCurrentLogMsgCount < 1)
        return;

    std::vector<LogEntryStruct*>::iterator it;
    irr::u32 showNrLines;

    showNrLines = mCurrentLogMsgCount;

    if (showNrLines > mNumberOfLastLogLinesShown) {
        showNrLines = mNumberOfLastLogLinesShown;
    }

    std::vector<LogEntryStruct*>::iterator itStart =
            mCurrentLogEntries.end() - showNrLines;

    irr::u32 overallChars = 0;

    //how many characters do we have to output overall?
    for (it = itStart; it != this->mCurrentLogEntries.end(); ++it) {
      overallChars += (*it)->logTextLen;
    }

    wchar_t* outputText = new wchar_t[overallChars + 2];
    wcscpy(outputText, L"");

    //concate overall shown log text together
    for (it = itStart; it != this->mCurrentLogEntries.end(); ++it) {
      wcscat(outputText, (*it)->logText);
    }

    this->logTextOutputWindow->setText(outputText);

    //cleanup temporary text variable
    delete[] outputText;
}
