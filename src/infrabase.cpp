/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "infrabase.h"

bool InfrastructureBase::InitIrrlicht() {
    /************************************************/
    /************** Init Irrlicht stuff *************/
    /************************************************/

    // create event receiver
    mEventReceiver = new MyEventReceiver();

    //we need to enable stencil buffers, otherwise volumentric shadows
    //will not work
    mDevice = createDevice(video::EDT_OPENGL, mScreenRes, 16, mFullscreen, mEnableShadows, false, mEventReceiver);

    if (mDevice == 0) {
          cout << "Failed Irrlicht device creation!" << endl;
          return false;
    }

    //get pointer to video driver, Irrlicht scene manager
    mDriver = mDevice->getVideoDriver();
    mSmgr = mDevice->getSceneManager();

    //get Irrlicht GUI functionality pointers
    mGuienv = mDevice->getGUIEnvironment();

    return true;
}

bool InfrastructureBase::InitGameResources() {
    /***********************************************************/
    /* Extract game assets                                     */
    /***********************************************************/
    try {
        mPrepareData = new PrepareData(mDevice, mDriver);
    }
    catch (const std::string &msg) {
        cout << "Game assets preparation operation failed!\n" << msg << endl;
        return false;
    }

    /***********************************************************/
    /* Load GameFonts                                          */
    /***********************************************************/
    mGameTexts = new GameText(mDevice, mDriver);

    if (!mGameTexts->GameTextInitializedOk) {
        cout << "Game fonts init operation failed!" << endl;
        return false;
    }

    return true;
}

//get a random int in the range between min and max
int InfrastructureBase::randRangeInt(int min, int max) {
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

bool InfrastructureBase::GetInitOk() {
    return mInitOk;
}

InfrastructureBase::InfrastructureBase(dimension2d<u32> resolution, bool fullScreen, bool enableShadows) {
    mScreenRes = resolution;
    mFullscreen = fullScreen;
    mEnableShadows = enableShadows;

    if (!InitIrrlicht()) {
        return;
    }

    //log window left upper corner 100, 380
    //log window right lower corner 540, 460
    irr::core::rect logWindowPos(100, 380, 540, 460);

    //create my logger class
    mLogger = new Logger(mGuienv, logWindowPos);
    mLogger->HideWindow();

    if (!InitGameResources())
        return;

    mLogger->AddLogMessage((char*)"Game Resources initialized");

    mTimeProfiler = new TimeProfiler();

    mInitOk = true;
}

InfrastructureBase::~InfrastructureBase() {
    //cleanup game texts
    delete mGameTexts;

    delete mTimeProfiler;
}
