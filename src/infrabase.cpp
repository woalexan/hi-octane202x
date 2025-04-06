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
    
    //22.03.2025: Direct3D does not work right now, at least at my wifes notebook, because
    //of "Could not lock DIRECT3D9 Texture." issue
    //mDevice = createDevice(video::EDT_DIRECT3D9, mScreenRes, 16, mFullscreen, mEnableShadows, false, mEventReceiver);

    if (mDevice == 0) {
          cout << "Failed Irrlicht device creation!" << endl;
          return false;
    }

    //get pointer to video driver, Irrlicht scene manager
    mDriver = mDevice->getVideoDriver();
    mSmgr = mDevice->getSceneManager();

    //get Irrlicht GUI functionality pointers
    mGuienv = mDevice->getGUIEnvironment();

    //get game root dir, is an absolute path
    mGameRootDir = mDevice->getFileSystem()->getWorkingDirectory();

    return true;
}

irr::io::IFileList* InfrastructureBase::CreateFileList(irr::io::path whichAbsPath) {
    //set current working directory
    if (!mDevice->getFileSystem()->changeWorkingDirectoryTo(whichAbsPath)) {
        return NULL;
    }

    //create list of files in the current working directory
    irr::io::IFileList* fileList = mDevice->getFileSystem()->createFileList();

    //restore original working dir for this project
    if (!mDevice->getFileSystem()->changeWorkingDirectoryTo(mGameRootDir)) {
        return NULL;
    }

    return fileList;
}

//if specified file is not found, returns empty path
irr::io::path InfrastructureBase::LocateFileInFileList(irr::io::IFileList* fileList, irr::core::string<fschar_t> fileName) {
    irr::u32 entriesFound = fileList->getFileCount();
    bool equals;

    for (irr::u32 idx = 0; idx < entriesFound; idx++) {
       //returns true if equal ignoring case
       equals = fileList->getFileName(idx).equals_ignore_case(fileName);

       if (equals) {
           return (fileList->getFullFileName(idx));
       }
    }

    irr::io::path empty("");

    //we did not find anything, return empty path
    return empty;
}

//Returns true in case of success, False otherwise
bool InfrastructureBase::DetermineOriginalGameVersion() {
    //First locate hioctane.exe file
    irr::io::path gameExeFile =
            LocateFileInFileList(mOriginalGame->execFolder, irr::core::string<fschar_t>("hioctane.exe"));

    if (gameExeFile.empty()) {
        //exe file not found!
         throw std::string("Could not locate the original game exe file hioctane.exe");
    }

    //open file and search for the version information
    bool foundStart = false;
    bool foundEnd = false;
    irr::u32 fndPosStart = 0;
    irr::u32 fndPosEnd = 0;

    /* try to open file to read */
    ifstream ifile;
    std::streampos fileSize;

    ifile.open(gameExeFile.c_str(), std::ifstream::binary);
       if(!ifile) {
         std::cout << "Could not open hioctane.exe file!" << endl;
         return false;
       }

     // get its size:
     ifile.seekg(0, std::ios::end);
     fileSize = ifile.tellg();
     ifile.seekg(0, std::ios::beg);

     std::vector<uint8_t> *fileData = new std::vector<uint8_t>(fileSize);

     ifile.read(reinterpret_cast<char*>(fileData->data()), fileData->size());

     if (!ifile) {
         std::cout << "hioctane.exe file read error: only " << ifile.gcount() << " bytes could be read!" << endl;
         delete fileData;
         return false;
     }

     std::vector<uint8_t>::iterator it;
     std::vector<uint8_t> compareStrStart ({0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E,
                                           0x20, 0x64, 0x61, 0x74, 0x65, 0x00, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E,
                                           0x20, 0x64, 0x61, 0x74, 0x65, 0x00});

     std::vector<uint8_t> compareStrEnd ({0x00, 0x25, 0x73, 0x20, 0x25, 0x73, 0x00,
                                         0x53, 0x75, 0x70, 0x70, 0x6C, 0x69, 0x65,
                                         0x64, 0x20, 0x74, 0x6F, 0x00, 0x53,
                                         0x75, 0x70, 0x70, 0x6C, 0x69, 0x65,
                                         0x64, 0x20, 0x74, 0x6F, 0x00});

     irr::u32 currCmpPosStart = 0;
     irr::u32 currCmpPosEnd = 0;
     irr::u32 idx = 0;

     for (it = fileData->begin(); it != fileData->end(); ++it) {

         if (!foundStart) {
             if ((*it) == compareStrStart.at(currCmpPosStart)) {
                 //first character found, check for the next one
                 currCmpPosStart++;

                 //did we find the overall search string?
                 if (currCmpPosStart >= compareStrStart.size()) {
                     foundStart = true;
                     fndPosStart = idx;
                 }
             } else {
                 //start again from first character
                 currCmpPosStart = 0;
             }
         }

         if (!foundEnd) {
             if ((*it) == compareStrEnd.at(currCmpPosEnd)) {
                 //first character found, check for the next one
                 currCmpPosEnd++;

                 //did we find the overall search string?
                 if (currCmpPosEnd >= compareStrEnd.size()) {
                     foundEnd = true;
                     fndPosEnd = idx;
                 }
             } else {
                 //start again from first character
                 currCmpPosEnd = 0;
             }
         }

         idx++;

         if (foundStart && foundEnd) {
             break;
         }
     }

    if (!foundStart || !foundEnd) {
        std::cout << "hioctane.exe version date not found!" << endl;
        delete fileData;
        return false;
    }

    mGameVersionDate.clear();

    std::vector<uint8_t>::iterator start = fileData->begin() + fndPosStart + 1;
    std::vector<uint8_t>::iterator end = fileData->begin() + fndPosEnd - (compareStrEnd.size() - 1);
    uint8_t currChar;

    for (it = start; it != end; ++it) {
        currChar = (*it);
        if (currChar == 0) {
            currChar = ' ';
        }

        mGameVersionDate.push_back(currChar);
    }

    delete fileData;

    return true;
}

//Returns true if original game found,
//False otherwise, resulting original
//game root dir is stored in mOriginalGameRootDir
bool InfrastructureBase::LocateOriginalGame() {
    //first find folder for original game files (original game root dir)
    irr::io::IFileList* fList = CreateFileList(mGameRootDir);

    if (fList == NULL) {
        return false;
    }

    irr::io::path origGameRootDirPath = LocateFileInFileList(fList, irr::core::string<fschar_t>("originalgame"));

    //drop the file list again
    //not that we get a memory leak!
    fList->drop();

    //folder not found?
    if (origGameRootDirPath.empty()) {
        cout << "I was not able to locate the original game files" << std::endl;
        return false;
    }

    cout << "Original Game located in " << origGameRootDirPath.c_str() << std::endl;

    this->mOriginalGame = new OriginalGameFolderInfoStruct();

    //make file list for original games root folder
    this->mOriginalGame->rootFolder = CreateFileList(origGameRootDirPath);

    if (this->mOriginalGame->rootFolder == NULL) {
        cout << "I was not able to create the file list for the original games root directory" << std::endl;
        return false;
    }

    /*****************************************************
     *  Data folder                                      *
     * ***************************************************/

    //now locate all the original game folders
    irr::io::path dataFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("data"));

    if (dataFolderPath.empty()) {
        cout << "I was not able to locate the original game data folder" << std::endl;
        return false;
    }

    this->mOriginalGame->dataFolder = CreateFileList(dataFolderPath);

    if (this->mOriginalGame->dataFolder == NULL) {
        cout << "I was not able to create the file list for the original games data directory" << std::endl;
        return false;
    }

    /*****************************************************
     *  Exec folder                                      *
     * ***************************************************/

    irr::io::path execFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("exec"));

    if (execFolderPath.empty()) {
        cout << "I was not able to locate the original game exec folder" << std::endl;
        return false;
    }

    this->mOriginalGame->execFolder = CreateFileList(execFolderPath);

    if (this->mOriginalGame->execFolder == NULL) {
        cout << "I was not able to create the file list for the original games exec directory" << std::endl;
        return false;
    }

    /*****************************************************
     *  Maps folder                                      *
     * ***************************************************/

    irr::io::path mapsFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("maps"));

    if (mapsFolderPath.empty()) {
        cout << "I was not able to locate the original game maps folder" << std::endl;
        return false;
    }

    this->mOriginalGame->mapsFolder = CreateFileList(mapsFolderPath);

    if (this->mOriginalGame->mapsFolder == NULL) {
        cout << "I was not able to create the file list for the original games maps directory" << std::endl;
        return false;
    }

    /*****************************************************
     *  Objects/data folder                              *
     * ***************************************************/

    irr::io::path objectsFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("objects"));

    if (objectsFolderPath.empty()) {
        cout << "I was not able to locate the original game objects folder" << std::endl;
        return false;
    }

    //under objects subdir there is another folder "data", for whatever reason
    irr::io::IFileList* helperList = CreateFileList(objectsFolderPath);

    if (helperList == NULL) {
        cout << "I was not able to create the file list for the original games objects directory" << std::endl;
        return false;
    }

    //now search for the data folder inside the objects folder
    irr::io::path objectsFolderDataPath = LocateFileInFileList(helperList, irr::core::string<fschar_t>("data"));
    helperList->drop();

    if (objectsFolderDataPath.empty()) {
        cout << "I was not able to locate the data directory inside the original games objects folder" << std::endl;
        return false;
    }

    this->mOriginalGame->objectsFolder = CreateFileList(objectsFolderDataPath);

    if (this->mOriginalGame->objectsFolder == NULL) {
        cout << "I was not able to create the file list for the original games objects/data directory" << std::endl;
        return false;
    }

    /*****************************************************
     *  HIOCTANE.CD/SAVE folder                              *
     * ***************************************************/

    irr::io::path hioctaneCdFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("HIOCTANE.CD"));

    if (hioctaneCdFolderPath.empty()) {
        cout << "No HIOCTANE.CD folder found => create it" << std::endl;
        try {
            char newDir[100];
            strcpy(newDir, mOriginalGame->rootFolder->getPath().c_str());
            strcat(newDir, (char*)("hioctane.cd"));
            CreateDirectory(newDir);

            //need to update root folder file list
            this->mOriginalGame->rootFolder->drop();
            this->mOriginalGame->rootFolder = CreateFileList(origGameRootDirPath);

            hioctaneCdFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("HIOCTANE.CD"));
        } catch(...) {
            cout << "Failed to create HIOCTANE.CD folder" << std::endl;
            return false;
        }
    }

    //under HIOCTANE.CD subdir there is another folder "save"
    helperList = CreateFileList(hioctaneCdFolderPath);

    if (helperList == NULL) {
        cout << "I was not able to create the file list for the original games HIOCTANE.CD directory" << std::endl;
        return false;
    }

    //now search for the SAVE folder inside the objects folder
    saveFolderDataPath = LocateFileInFileList(helperList, irr::core::string<fschar_t>("SAVE"));
    helperList->drop();

    if (saveFolderDataPath.empty()) {
        cout << "I did not find a save directory inside the original games HIOCTANE.CD folder => create it" << std::endl;
        try {
            char newDir[100];
            strcpy(newDir, hioctaneCdFolderPath.c_str());
            strcat(newDir, (char*)("/save"));
            CreateDirectory(newDir);

            //need to update file list
            //under HIOCTANE.CD subdir there is another folder "save"
            helperList = CreateFileList(hioctaneCdFolderPath);

            if (helperList == NULL) {
                cout << "I was not able to create the file list for the original games HIOCTANE.CD directory" << std::endl;
                return false;
            }

            saveFolderDataPath = LocateFileInFileList(helperList, irr::core::string<fschar_t>("SAVE"));
            helperList->drop();
        }  catch (...) {
            cout << "Failed to create save folder" << std::endl;
            return false;
        }
    }

    this->mOriginalGame->saveFolder = CreateFileList(saveFolderDataPath);

    if (this->mOriginalGame->saveFolder == NULL) {
        cout << "I was not able to create the file list for the original games HIOCTANE.CD/SAVE directory" << std::endl;
        return false;
    }

    /*****************************************************
     *  Sound folder                                     *
     * ***************************************************/

    irr::io::path soundFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("sound"));

    if (soundFolderPath.empty()) {
        cout << "I was not able to locate the original game sound folder" << std::endl;
        return false;
    }

    this->mOriginalGame->soundFolder = CreateFileList(soundFolderPath);

    if (this->mOriginalGame->soundFolder == NULL) {
        cout << "I was not able to create the file list for the original games sound directory" << std::endl;
        return false;
    }

    //all ok
    return true;
}

bool InfrastructureBase::InitGameResources() {
    /***********************************************************/
    /* Extract game assets                                     */
    /***********************************************************/
    try {
        mPrepareData = new PrepareData(this);
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

//get a random float value in the range of 0.0 up to 1.0
float InfrastructureBase::randFloat() {
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    return r;
}

bool InfrastructureBase::GetInitOk() {
    return mInitOk;
}

//returns true if the original game version date is known,
//and we can use it, False otherwise
bool InfrastructureBase::ProcessGameVersionDate() {
    std::string versionStr(this->mGameVersionDate.begin(), this->mGameVersionDate.end());

    std::cout << "Original game version date = " << versionStr << std::endl;

    if (versionStr.compare("Jun 10 1995 17:45:48") == 0) {
        //this version date belongs to the non extended
        //game release
        mExtendedGame = false;
        return true;
    }

    if (versionStr.compare("Oct 23 1995 15:57:32") == 0) {
        //this version date belongs to the extended
        //game release
        mExtendedGame = true;
        return true;
    }

    std::cout << "Unknown game version date!" << std::endl;

    return false;
}

bool InfrastructureBase::UpdateFileListSaveFolder() {
    if (this->mOriginalGame->saveFolder != NULL) {
        this->mOriginalGame->saveFolder->drop();
    }

    //recreate the file list
    this->mOriginalGame->saveFolder = CreateFileList(saveFolderDataPath);

    if (this->mOriginalGame->saveFolder == NULL) {
        cout << "I was not able to create the file list for the original games HIOCTANE.CD/SAVE directory" << std::endl;
        return false;
    }

    return true;
}

InfrastructureBase::InfrastructureBase(dimension2d<u32> resolution, bool fullScreen, bool enableShadows) {
    mScreenRes = resolution;
    mFullscreen = fullScreen;
    mEnableShadows = enableShadows;

    if (!InitIrrlicht()) {
        return;
    }

    //detect the original game files
    //if some directory of original game is missing
    //exit here
    if (!LocateOriginalGame())
        return;

    //search original game version data in
    //original game exe file
    if (!DetermineOriginalGameVersion())
        return;

    //do we know this game version?
    if (!ProcessGameVersionDate())
        return;

    if (this->mExtendedGame) {
        std::cout << "This game is the extended version" << std::endl;
    } else {
        std::cout << "This game is the non-extended version" << std::endl;
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

    mTimeProfiler = new TimeProfiler(mGuienv, rect<s32>(100,150,300,200));

    mInitOk = true;
}

InfrastructureBase::~InfrastructureBase() {
    //cleanup game texts
    delete mGameTexts;

    delete mTimeProfiler;

    //cleanup the original game folder information
    if (mOriginalGame != NULL) {
        if (mOriginalGame->dataFolder != NULL) {
            mOriginalGame->dataFolder->drop();
            mOriginalGame->dataFolder = NULL;
        }

        if (mOriginalGame->execFolder != NULL) {
            mOriginalGame->execFolder->drop();
            mOriginalGame->execFolder = NULL;
        }

        if (mOriginalGame->mapsFolder != NULL) {
            mOriginalGame->mapsFolder->drop();
            mOriginalGame->mapsFolder = NULL;
        }

        if (mOriginalGame->rootFolder != NULL) {
            mOriginalGame->rootFolder->drop();
            mOriginalGame->rootFolder = NULL;
        }

        if (mOriginalGame->saveFolder != NULL) {
            mOriginalGame->saveFolder->drop();
            mOriginalGame->saveFolder = NULL;
        }

        if (mOriginalGame->soundFolder != NULL) {
            mOriginalGame->soundFolder->drop();
            mOriginalGame->soundFolder = NULL;
        }

        if (mOriginalGame->objectsFolder != NULL) {
            mOriginalGame->objectsFolder->drop();
            mOriginalGame->objectsFolder = NULL;
        }

        delete mOriginalGame;
        mOriginalGame = NULL;
    }
}
