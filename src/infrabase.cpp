/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "infrabase.h"
#include "utils/logging.h"
#include "utils/logger.h"
#include <iostream>
#include <fstream>
#include "draw/gametext.h"
#include "input/input.h"
#include "utils/tprofile.h"
#include "utils/fileutils.h"
#include "draw/drawdebug.h"
#include "resources/readgamedata/bulcommn.h"
#include "utils/crc32.h"
#include <cwctype>
#include <sstream>
#include <iomanip>
#include "draw/attribution.h"
#include "font/font_manager.h"
#include "game.h"

//Returns true in case of success, False otherwise
bool InfrastructureBase::WriteGameConfigXmlFile(IrrlichtDevice *device) {
    std::string cfgFileName(mGameRootDir.c_str());
    cfgFileName.append("/hi-octane202x.xml");

    //create necessary XML writer
    irr::io::IXMLWriter* writer = device->getFileSystem()->createXMLWriter( cfgFileName.c_str() );
    if (!writer) {
         std::string overallMsg("Failed to create game Xml config file ");
         overallMsg.append(cfgFileName);
         overallMsg.append("!");
         logging::Error(overallMsg);

         return false;
     }

     //we need exactly one XML header
     writer->writeXMLHeader();

     writer->writeElement(L"hi-octane202x");
     writer->writeLineBreak();

     //section for config
     writer->writeElement(L"graphics");
     writer->writeLineBreak();

     writer->writeElement(L"setting",true, L"name", L"enableDoubleResolution" , L"value", BoolToXmlSettingStr(mGameConfig->enableDoubleResolution).c_str());
     writer->writeLineBreak();

     writer->writeElement(L"setting",true, L"name", L"enableVSync" , L"value", BoolToXmlSettingStr(mGameConfig->enableVSync).c_str());
     writer->writeLineBreak();

     writer->writeElement(L"setting",true, L"name", L"enableShadows" , L"value", BoolToXmlSettingStr(mGameConfig->enableShadows).c_str());
     writer->writeLineBreak();

     writer->writeElement(L"setting",true, L"name", L"useUpgradedSky" , L"value", BoolToXmlSettingStr(mGameConfig->useUpgradedSky).c_str());
     writer->writeLineBreak();

     //close config setup section
     writer->writeClosingTag(L"graphics");
     writer->writeLineBreak();

     //section for music setup
     writer->writeElement(L"intro");
     writer->writeLineBreak();

     writer->writeElement(L"setting",true, L"name", L"skipIntro" , L"value", BoolToXmlSettingStr(mGameConfig->skipIntro).c_str());
     writer->writeLineBreak();

     //close music setup section
     writer->writeClosingTag(L"intro");
     writer->writeLineBreak();

     //end of hi-octane202x config
     writer->writeClosingTag(L"hi-octane202x");

     //remove Xml writer
     writer->drop();

     return true;
}

//Returns true in case of success, False otherwise
bool InfrastructureBase::ReadGameConfigXmlFile(IrrlichtDevice *device) {
    //does this file exist?
    std::string cfgFileName(mGameRootDir.c_str());
    cfgFileName.append("/hi-octane202x.xml");
    if (!(FileExists(cfgFileName.c_str()) == 1)) {
        //file does not exist
        logging::Warning("Game configuration Xml file not found, recreate default configuration");

        //default configuration
        //use everything "vanilla"
        mGameConfig->enableVSync = true;
        mGameConfig->enableShadows = false;
        mGameConfig->useUpgradedSky = false;
        mGameConfig->enableDoubleResolution = false;
        mGameConfig->skipIntro = false;

        if (!WriteGameConfigXmlFile(device)) {
            logging::Error("Writing new game configuration Xml file failed!");
            return false;
        }

        return true;
    }

    //Config file exists, read the current settings
    //create the Xml Reader
    irr::io::IXMLReader* xmlReader = device->getFileSystem()->createXMLReader(cfgFileName.c_str());
    if (!xmlReader) {
        std::string overallMsg("Failed to read game Xml config file ");
        overallMsg.append(cfgFileName);
        overallMsg.append("!");
        logging::Error(overallMsg);

        return false;
    }

    const irr::core::stringw settingTag(L"setting");
    const irr::core::stringw graphicsSetupTag(L"graphics");
    const irr::core::stringw introSetupTag(L"intro");

    irr::core::stringw currentSection;

    irr::core::map<irr::core::stringw, irr::core::stringw> graphicsSetupMap;
    irr::core::map<irr::core::stringw, irr::core::stringw> introSetupMap;

    //while there is more to read
    while (xmlReader->read())
    {
        //check the node type
        switch (xmlReader->getNodeType())
        {
            //we found a new element
            case irr::io::EXN_ELEMENT:
            {
                 //we currently are in the empty or mapconfig section and find the sky setup tag so we set our current section to sky setup
                 if (currentSection.empty() && graphicsSetupTag.equals_ignore_case(xmlReader->getNodeName()))
                    {
                        currentSection = graphicsSetupTag;
                    }

                 //we are in the graphics setup section and we find a setting to parse
                 else if (currentSection.equals_ignore_case(graphicsSetupTag) && settingTag.equals_ignore_case(xmlReader->getNodeName()))
                    {
                       //read in the key
                        irr::core::stringw key = xmlReader->getAttributeValueSafe(L"name");
                        //if there actually is a key to set
                        if (!key.empty())
                        {
                            //set the setting in the map to the value,
                            //the [] operator overrides values if they already exist or inserts a new key value
                            //pair into the settings map if it was not defined yet
                            graphicsSetupMap[key] = xmlReader->getAttributeValueSafe(L"value");
                        }
                    }


                 //we currently are in the empty or mapconfig section and find the music setup tag so we set our current section to music setup
                 if (currentSection.empty() && introSetupTag.equals_ignore_case(xmlReader->getNodeName()))
                    {
                        currentSection = introSetupTag;
                    }

                 //we are in the intro setup section and we find a setting to parse
                 else if (currentSection.equals_ignore_case(introSetupTag) && settingTag.equals_ignore_case(xmlReader->getNodeName()))
                    {
                       //read in the key
                        irr::core::stringw key = xmlReader->getAttributeValueSafe(L"name");
                        //if there actually is a key to set
                        if (!key.empty())
                        {
                            //set the setting in the map to the value,
                            //the [] operator overrides values if they already exist or inserts a new key value
                            //pair into the settings map if it was not defined yet
                            introSetupMap[key] = xmlReader->getAttributeValueSafe(L"value");
                        }
                    }

                break;
          }

          //we found the end of an element
          case irr::io::EXN_ELEMENT_END: {
              //we were at the end of the current setup section so we reset our tag
                 currentSection=L"";
                 break;
          }
       }
   }

   // don't forget to delete the xml reader
   xmlReader->drop();

   irr::core::map<irr::core::stringw, irr::core::stringw>::Node* nodePntr;

   //get double resolution setting
   nodePntr = graphicsSetupMap.find(L"enableDoubleResolution");

   if (nodePntr == nullptr) {
       logging::Error("ReadGameConfigXmlFile: Missing enableDoubleResolution field in game config Xml file");
       return false;
   } else {
       bool outBool;
       if (!XmlSettingStrToBool(nodePntr->getValue(), outBool)) {
           logging::Error("ReadGameConfigXmlFile: Missformed field value for enableDoubleResolution in game config Xml file");
           return false;
       }

       mGameConfig->enableDoubleResolution = outBool;
   }

   //get enableVSync setting
   nodePntr = graphicsSetupMap.find(L"enableVSync");

   if (nodePntr == nullptr) {
       logging::Error("ReadGameConfigXmlFile: Missing enableVSync field in game config Xml file");
       return false;
   } else {
       bool outBool;
       if (!XmlSettingStrToBool(nodePntr->getValue(), outBool)) {
           logging::Error("ReadGameConfigXmlFile: Missformed field value for enableVSync in game config Xml file");
           return false;
       }

       mGameConfig->enableVSync = outBool;
   }

   //get enableShadows setting
   nodePntr = graphicsSetupMap.find(L"enableShadows");

   if (nodePntr == nullptr) {
       logging::Error("ReadGameConfigXmlFile: Missing enableShadows field in game config Xml file");
       return false;
   } else {
       bool outBool;
       if (!XmlSettingStrToBool(nodePntr->getValue(), outBool)) {
           logging::Error("ReadGameConfigXmlFile: Missformed field value for enableShadows in game config Xml file");
           return false;
       }

       mGameConfig->enableShadows = outBool;
   }

   //get useUpgradedSky setting
   nodePntr = graphicsSetupMap.find(L"useUpgradedSky");

   if (nodePntr == nullptr) {
       logging::Error("ReadGameConfigXmlFile: Missing useUpgradedSky field in game config Xml file");
       return false;
   } else {
       bool outBool;
       if (!XmlSettingStrToBool(nodePntr->getValue(), outBool)) {
           logging::Error("ReadGameConfigXmlFile: Missformed field value for useUpgradedSky in game config Xml file");
           return false;
       }

       mGameConfig->useUpgradedSky = outBool;
   }

   //get skipIntro setting
   nodePntr = introSetupMap.find(L"skipIntro");

   if (nodePntr == nullptr) {
       logging::Error("ReadGameConfigXmlFile: Missing skipIntro field in game config Xml file");
       return false;
   } else {
       bool outBool;
       if (!XmlSettingStrToBool(nodePntr->getValue(), outBool)) {
           logging::Error("ReadGameConfigXmlFile: Missformed field value for skipIntro in game config Xml file");
           return false;
       }

       mGameConfig->skipIntro = outBool;
   }

   return true;
}

//Returns true in case of success, false otherwise
bool InfrastructureBase::LoadLevelConfigData(std::string levelRootPath, MapConfigStruct** outMapTarget) {
    irr::io::path configFileName = GetMapConfigFileName(levelRootPath);

    if (FileExists(configFileName.c_str()) != 1) {
        //Problem with this file
        std::string logErr("Can not find file '");
        logErr.append(configFileName.c_str());
        logErr.append("'!");

        logging::Error(logErr);
        return false;
    }

    //Read complete level/map configuration from Xml file
    *outMapTarget = new MapConfigStruct();
    if (!ReadMapConfigFile(configFileName.c_str(), **outMapTarget)) {
        logging::Error("Failed to read map config file!");
        return false;
    }

    std::string logStr("Succesfully read file '");
    logStr.append(configFileName.c_str());
    logStr.append("'");

    logging::Info(logStr);

    return true;
}

bool InfrastructureBase::InitStage1() {
   //first initialization is done using the Nulldevice

   //first get native screen resolution
   //for this we can use the Null device
   IrrlichtDevice *nullDev = createDevice(video::EDT_NULL);

   if (nullDev == nullptr) {
       return false;
   }

   mNativeResolution = nullDev->getVideoModeList()->getDesktopResolution();

    //get game root dir, is an absolute path
    mGameRootDir = nullDev->getFileSystem()->getWorkingDirectory();

    if (mWriteLogFile) {
        //use the null Device to get the current system date and time
        ITimer::RealTimeDate dateTime = nullDev->getTimer()->getRealTimeAndDate();

        //create log file name based on current system date and time
        std::string logFileName("logfile-");

        std::ostringstream hlper;
        hlper << std::setfill('0') << std::setw(2) << dateTime.Year   << std::setfill('0') << std::setw(2) << dateTime.Month <<
                 std::setfill('0') << std::setw(2) << dateTime.Day    << std::setfill('0') << std::setw(2) << dateTime.Hour <<
                 std::setfill('0') << std::setw(2) << dateTime.Minute << std::setfill('0') << std::setw(2) << dateTime.Second << ".txt";

        logFileName.append(hlper.str());

        //start logging into logfile
        logging::StartLogFile(logFileName.c_str());
    }

    if (mRunningAs == INFRA_RUNNING_AS_GAME) {
        //read the game configuration file
        if (!ReadGameConfigXmlFile(nullDev)) {
            return false;
        }
    }

    //delete the null device again
    nullDev->drop();

    if (mCLIVec.size() > 0) {
        std::ostringstream executable;
        executable << "Executable: " << mCLIVec.at(0);

        logging::Info(executable.str());

        //if there were additional command line parameters also
        //list them here
        if (mCLIVec.size() > 1) {
            std::ostringstream parameters;
            parameters << "Command line parameters: ";

            std::vector<std::string>::iterator it;
            for (it = mCLIVec.begin() + 1; it != mCLIVec.end(); ++it) {
                parameters << (*it) << " ";
            }

            logging::Info(parameters.str());
        }
    }

    std::ostringstream nativeResolution;
    nativeResolution << "Native screen resolution is " << mNativeResolution.Width << " x " << mNativeResolution.Height;

    logging::Info(nativeResolution.str());

    // create event receiver
    mEventReceiver = new MyEventReceiver(this);

    mFontManager = new FontManager();

    return true;
}

bool InfrastructureBase::InitStage2() {
    /************************************************/
    /********** Init Irrlicht stuff Stage 2 *********/
    /************************************************/

    //Define the resolution to be used, according to the program type we execute as,
    //and the Game config Xml file configuration
    if (mRunningAs == INFRA_RUNNING_AS_GAME) {
        if (mGameConfig->enableDoubleResolution) {
            mScreenRes.set(1280,960);
        } else {
            mScreenRes.set(640,480);
        }

        mFullscreen = false;
        mDevice = createDevice(video::EDT_OPENGL, mScreenRes, 32, mFullscreen, false, mGameConfig->enableVSync, mEventReceiver);
    } else if (mRunningAs == INFRA_RUNNING_AS_EDITOR) {
        mScreenRes.set(1280,960);

        mFullscreen = false;
        mDevice = createDevice(video::EDT_OPENGL, mScreenRes, 32, mFullscreen, false, true, mEventReceiver);
    } else {
        //unknown program run mode, quit
        logging::Error("Unknown infrastructure program run mode, Exit!");
        return false;
    }

    if (mDevice == 0) {
        logging::Error("Failed Irrlicht device creation in InitStage2!");
        return false;
    }

    //do not allow window resizing in windowed mode
    mDevice->setResizable(false);

    //get pointer to video driver, Irrlicht scene manager
    mDriver = mDevice->getVideoDriver();
    mSmgr = mDevice->getSceneManager();

    //get Irrlicht GUI functionality pointers
    mGuienv = mDevice->getGUIEnvironment();

    //create a DrawDebug object
    mDrawDebug = new DrawDebug(mDriver);

    if ((mRunningAs == INFRA_RUNNING_AS_EDITOR) ||
            ((mRunningAs == INFRA_RUNNING_AS_GAME) && (mGameConfig->enableDoubleResolution))) {
            //higher resolution font size
            fontAndika = mFontManager->GetTtFont(mDriver, mDevice->getFileSystem(),
                             "media/andika/Andika-R.ttf", 12, true, true);
    } else {
        //lower resolution font size
        fontAndika = mFontManager->GetTtFont(mDriver, mDevice->getFileSystem(),
                         "media/andika/Andika-R.ttf", 10, true, true);
    }

    if (fontAndika) {
         mGuienv->getSkin()->setFont(fontAndika);
    }

    //"Random" seed for "random" number generator
    srand(mDevice->getTimer()->getRealTime());

    return true;
}

bool InfrastructureBase::InitStage3() {
    //create the predefined axis direction vectors
    xAxisDirVector = new irr::core::vector3df(1.0f, 0.0f, 0.0f);
    yAxisDirVector = new irr::core::vector3df(0.0f, 1.0f, 0.0f);
    zAxisDirVector = new irr::core::vector3df(0.0f, 0.0f, 1.0f);

    //Query if the graphics adapter is able to render to a target
    //we need this feature to create the block definition preview images
    //for the Gui Windows
    if (mDriver->queryFeature(video::EVDF_RENDER_TO_TARGET))
     {
        //we have this feature, we can enable block preview in LevelEditor
        mBlockPreviewEnabled = true;
     } else {
        logging::Warning("Graphics Adapter does not support Render To Target Feature: Block definition preview images not working");
        mBlockPreviewEnabled = false;
    }

    //detect the original game files
    //if some directory of original game is missing
    //exit here
    if (!LocateOriginalGame())
        return false;

    //search original game version data in
    //original game exe file
    if (!DetermineOriginalGameVersion())
        return false;

    //do we know this game version?
    if (!ProcessGameVersionDate())
        return false;

    if (this->mExtendedGame) {
        logging::Info("This game is the extended version");
    } else {
        logging::Info("This game is the non-extended version");
    }

    //log window left upper corner 100, 380
    //log window right lower corner 540, 460
    irr::core::rect logWindowPos(100, 380, 540, 460);

    //create my logger class
    mLogger = new Logger(mGuienv, logWindowPos);
    mLogger->HideWindow();

    //Initial most basic game assets to be able
    //to show a first graphical screen to the user
    //remaining data extraction/loading of assets is
    //done from main loop of the game
    if (!InitGameResourcesInitialStep())
        return false;

    logging::Info("Initial Game Resources initialized");

    mTimeProfiler = new TimeProfiler(mGuienv, rect<s32>(100,150,300,300));

    mCrc32 = new Crc32();

    if ((mRunningAs == INFRA_RUNNING_AS_EDITOR) ||
            ((mRunningAs == INFRA_RUNNING_AS_GAME) && (mGameConfig->enableDoubleResolution))) {
            mAttribution = new Attribution(this, 796, 0, true);
    } else {
            mAttribution = new Attribution(this, 412, -70, true);
    }

    return true;
}

//returns true if Gui Event should be canceled
bool InfrastructureBase::HandleGuiEvent(const irr::SEvent& event) {
    return false;
}

void InfrastructureBase::HandleMouseEvent(const irr::SEvent& event) {
}

irr::core::stringw InfrastructureBase::SColorToXmlSettingStr(irr::video::SColor* whichColor) {
   irr::core::stringw result(L"SColor(");

   result.append(irr::core::stringw(whichColor->getAlpha()));
   result.append(L",");
   result.append(irr::core::stringw(whichColor->getRed()));
   result.append(L",");
   result.append(irr::core::stringw(whichColor->getGreen()));
   result.append(L",");
   result.append(irr::core::stringw(whichColor->getBlue()));
   result.append(L")");

   return result;
}

irr::core::stringw InfrastructureBase::Vector3dfToXmlSettingStr(irr::core::vector3df* whichVector) {
   irr::core::stringw result(L"Vector3df(");

   result.append(irr::core::stringw(whichVector->X));
   result.append(L",");
   result.append(irr::core::stringw(whichVector->Y));
   result.append(L",");
   result.append(irr::core::stringw(whichVector->Z));
   result.append(L")");

   return result;
}

irr::core::stringw InfrastructureBase::Vector2diToXmlSettingStr(irr::core::vector2di* whichVector) {
   irr::core::stringw result(L"Vector2di(");

   result.append(irr::core::stringw(whichVector->X));
   result.append(L",");
   result.append(irr::core::stringw(whichVector->Y));
   result.append(L")");

   return result;
}

irr::core::stringw InfrastructureBase::BoolToXmlSettingStr(bool inputVal) {
   if (!inputVal) {
       return (irr::core::stringw("False"));
   }

   return (irr::core::stringw("True"));
}

irr::u32 InfrastructureBase::CntNumberCharacterOccurence(irr::core::stringw* inputStr, wchar_t delimiter) {
    irr::u32 cnt = 0;
    irr::s32 currIdx;
    irr::s32 lastIdx = 0;
    irr::s32 nextIdx = -1;

    irr::s32 inputStrLen = (irr::s32)(inputStr->size());

    do {
        nextIdx = lastIdx + 1;
        if (!(nextIdx >= inputStrLen)) {
            currIdx = inputStr->findNext(delimiter, nextIdx);
            if (currIdx != -1) {
                cnt++;
                lastIdx = currIdx;
            }
        } else {
            currIdx = -1;
        }
    } while (currIdx != -1);

    return cnt;
}

void InfrastructureBase::SplitWStringAtDelimiterChar(irr::core::stringw* inputStr, wchar_t delimiter, std::vector<irr::core::stringw> &outWStrVec,
                                                     bool addEmptyStrings) {
    irr::s32 currIdx = -1;
    irr::s32 lastIdx = -1;
    irr::s32 nextIdx = -1;

    irr::core::stringw subStr;

    irr::s32 inputStrLen = (irr::s32)(inputStr->size());

    outWStrVec.clear();

    do {
        nextIdx = lastIdx + 1;
        if (!(nextIdx >= inputStrLen)) {
            currIdx = inputStr->findNext(delimiter, nextIdx);
            subStr = L"";
            if (currIdx != -1) {
                subStr = inputStr->subString(lastIdx + 1, (currIdx - lastIdx - 1));
                lastIdx = currIdx;
              } else {
                //add remaining part of string
                subStr = inputStr->subString(lastIdx + 1, (inputStrLen - lastIdx - 1));
             }

            if (subStr.size() > 0) {
                outWStrVec.push_back(subStr);
            } else if (addEmptyStrings) {
                outWStrVec.push_back(subStr);
            }
       } else {
            currIdx = -1;
        }
    } while (currIdx != -1);
}

//Returns false if input string is invalid (can not be parsed), True otherwise
//Output value is returned in second parameter
bool InfrastructureBase::XmlSettingStrToBool(irr::core::stringw inputStr, bool& outValue) {
    if (inputStr.make_lower().find(L"false") != -1) {
        outValue = false;
        return true;
    } else if (inputStr.make_lower().find(L"true") != -1) {
        outValue = true;
        return true;
    }

    return false;
}

//Returns true in case the input string contains a number, False if there are any other characters
//that are no digits;
bool InfrastructureBase::WStringContainsNumber(irr::core::stringw inputStr, bool allowDecimalNumber) {
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

//Returns false if Xml value payload string is missformed, True otherwise
//The output payload string is returned in the second parameter
bool InfrastructureBase::GetXmlValuePayload(irr::core::stringw inputStr, irr::core::stringw &payloadStr) {
    irr::s32 firstIdx = inputStr.findFirst(L'(');
    if (firstIdx == -1) {
        //( character not found
        return false;
    }

    irr::s32 possSecondStartSearch = firstIdx + 1;

    if (!(possSecondStartSearch >= (irr::s32)(inputStr.size()))) {
            irr::s32 possibleSecond = inputStr.findNext(L'(', possSecondStartSearch);

            if (possibleSecond != -1) {
                //we found another ( character, there should not be another
                //one => exit
                return false;
            }
    }

    irr::s32 secondIdx = inputStr.findNext(L')', firstIdx);

    if (secondIdx == -1) {
        //we did not find ) character, exit
        return false;
    }

    //now get substring between () characters
    irr::core::stringw subStr = inputStr.subString(firstIdx + 1, (secondIdx - firstIdx - 1));

    payloadStr = subStr;

    return true;
}

//Returns false if input string is invalid (can not be parsed), True otherwise
//Output value is returned in second parameter
bool InfrastructureBase::XmlSettingStrToVector3df(irr::core::stringw inputStr, irr::core::vector3df& outValue) {
    if (inputStr.make_lower().find(L"vector3df") == -1) {
         return false;
    }

    irr::core::stringw subStr;

    if (!GetXmlValuePayload(inputStr, subStr)) {
        //Xml value payload string is missformed
        return false;
    }

    if (CntNumberCharacterOccurence(&subStr, L',') != 2) {
        //wrong number of input fields for vector3df
        return false;
    }

    std::vector<irr::core::stringw> strVec;

    SplitWStringAtDelimiterChar(&subStr, L',', strVec, false);

    if (strVec.size() != 3) {
        //again wrong number of expected fields
        return false;
    }

    for (int i = 0; i < 3; i++) {
        //check if all fields are numbers
        if (!WStringContainsNumber(strVec.at(i), true)) {
            //first part string does also contain characters
            //that do not belong to a number!
            return false;
        }
    }

    irr::f32 parsedValue;

    //now parse the resulting numbers
    std::swscanf( strVec.at(0).c_str(), L"%f", &parsedValue);
    outValue.X = parsedValue;

    std::swscanf( strVec.at(1).c_str(), L"%f", &parsedValue);
    outValue.Y = parsedValue;

    std::swscanf( strVec.at(2).c_str(), L"%f", &parsedValue);
    outValue.Z = parsedValue;

    return true;
}

//Returns false if input string is invalid (can not be parsed), True otherwise
//Output value is returned in second parameter
bool InfrastructureBase::XmlSettingStrToSColor(irr::core::stringw inputStr, irr::video::SColor& outColor) {
    if (inputStr.make_lower().find(L"scolor") == -1) {
         return false;
    }

    irr::core::stringw subStr;

    if (!GetXmlValuePayload(inputStr, subStr)) {
        //Xml value payload string is missformed
        return false;
    }

    if (CntNumberCharacterOccurence(&subStr, L',') != 3) {
        //wrong number of input fields for SColor
        return false;
    }

    std::vector<irr::core::stringw> strVec;

    SplitWStringAtDelimiterChar(&subStr, L',', strVec, false);

    if (strVec.size() != 4) {
        //again wrong number of expected fields
        return false;
    }

    for (int i = 0; i < 4; i++) {
        //check if all fields are numbers
        if (!WStringContainsNumber(strVec.at(i), false)) {
            //first part string does also contain characters
            //that do not belong to a number!
            return false;
        }
    }

    irr::s32 parsedValue;

    //now parse the resulting numbers
    std::swscanf( strVec.at(0).c_str(), L"%d", &parsedValue);
    if ((parsedValue < 0) || (parsedValue > 255)) {
        //invalid number
        return false;
    }

    outColor.setAlpha(parsedValue);

    std::swscanf( strVec.at(1).c_str(), L"%d", &parsedValue);
    if ((parsedValue < 0) || (parsedValue > 255)) {
        //invalid number
        return false;
    }

    outColor.setRed(parsedValue);

    std::swscanf( strVec.at(2).c_str(), L"%d", &parsedValue);
    if ((parsedValue < 0) || (parsedValue > 255)) {
        //invalid number
        return false;
    }

    outColor.setGreen(parsedValue);

    std::swscanf( strVec.at(3).c_str(), L"%d", &parsedValue);
    if ((parsedValue < 0) || (parsedValue > 255)) {
        //invalid number
        return false;
    }

    outColor.setBlue(parsedValue);

    return true;
}

//Returns false if input string is invalid (can not be parsed), True otherwise
//Output value is returned in second parameter
bool InfrastructureBase::XmlSettingStrToVector2di(irr::core::stringw inputStr, irr::core::vector2di& outVector) {
    if (inputStr.make_lower().find(L"vector2di") == -1) {
         return false;
    }

    irr::core::stringw subStr;

    if (!GetXmlValuePayload(inputStr, subStr)) {
        //Xml value payload string is missformed
        return false;
    }

    if (CntNumberCharacterOccurence(&subStr, L',') != 1) {
        //wrong number of input fields for a 2 dimensional vector
        return false;
    }

    std::vector<irr::core::stringw> strVec;

    SplitWStringAtDelimiterChar(&subStr, L',', strVec, false);

    if (strVec.size() != 2) {
        //again wrong number of expected fields
        return false;
    }

    for (int i = 0; i < 2; i++) {
        //check if all fields are numbers
        if (!WStringContainsNumber(strVec.at(i), false)) {
            //first part string does also contain characters
            //that do not belong to a number!
            return false;
        }
    }

    irr::s32 parsedValue;

    //now parse the resulting numbers
    std::swscanf( strVec.at(0).c_str(), L"%d", &parsedValue);
    outVector.X = parsedValue;

    std::swscanf( strVec.at(1).c_str(), L"%d", &parsedValue);
    outVector.Y = parsedValue;

    return true;
}

//Returns true in case of success, False otherwise
bool InfrastructureBase::WriteMapConfigFile(const std::string fileName, MapConfigStruct* configStruct) {
    //create necessary XML writer
    irr::io::IXMLWriter* writer = mDevice->getFileSystem()->createXMLWriter( fileName.c_str() );
        if (!writer) {
            std::string overallMsg("Failed to create map config file ");
            overallMsg.append(fileName);
            overallMsg.append("!");
            logging::Error(overallMsg);

            return false;
        }

        //we need exactly one XML header
        writer->writeXMLHeader();

        writer->writeElement(L"mapconfig");
        writer->writeLineBreak();

        //section for sky setup
        writer->writeElement(L"sky");
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"vanillaSkyImage" , L"value", irr::core::stringw(configStruct->SkyImageFileVanilla.c_str()).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"upgradedSkyImage" , L"value", irr::core::stringw(configStruct->SkyImageFileUpgradedSky.c_str()).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorCenter1" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorCenter1).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorInner1" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorInner1).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorOuter1" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorOuter1).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorCenter2" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorCenter2).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorInner2" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorInner2).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorOuter2" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorOuter2).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorCenter3" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorCenter3).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorInner3" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorInner3).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"cloudColorOuter3" , L"value", SColorToXmlSettingStr(&configStruct->cloudColorOuter3).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"enableLensflare" , L"value", BoolToXmlSettingStr(configStruct->EnableLensFlare).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"locationLensflare" , L"value", Vector3dfToXmlSettingStr(&configStruct->lensflareLocation).c_str());
        writer->writeLineBreak();

        //close sky setup section
        writer->writeClosingTag(L"sky");
        writer->writeLineBreak();

        //section for music setup
        writer->writeElement(L"music");
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"musicFile" , L"value", irr::core::stringw(configStruct->MusicFile.c_str()).c_str());
        writer->writeLineBreak();

        //close music setup section
        writer->writeClosingTag(L"music");
        writer->writeLineBreak();

        //section for texture base location setup
        writer->writeElement(L"texturebase");
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"texturebaseLocation" , L"value", irr::core::stringw(configStruct->texBaseLocation.c_str()).c_str());
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"useCustomTextures" , L"value", BoolToXmlSettingStr(configStruct->useCustomTextures).c_str());
        writer->writeLineBreak();

        //close texture base location setup section
        writer->writeClosingTag(L"texturebase");
        writer->writeLineBreak();

        //section for minimap setup
        writer->writeElement(L"minimap");
        writer->writeLineBreak();

        writer->writeElement(L"setting",true, L"name", L"minimapCalSet" , L"value", BoolToXmlSettingStr(configStruct->minimapCalSet).c_str());
        writer->writeLineBreak();
        writer->writeElement(L"setting",true, L"name", L"minimapCalStartVal" , L"value", Vector2diToXmlSettingStr(&configStruct->minimapCalStartVal).c_str());
        writer->writeLineBreak();
        writer->writeElement(L"setting",true, L"name", L"minimapCalEndVal" , L"value", Vector2diToXmlSettingStr(&configStruct->minimapCalEndVal).c_str());
        writer->writeLineBreak();

        //close minimap setup section
        writer->writeClosingTag(L"minimap");
        writer->writeLineBreak();

        //end of mapconfig
        writer->writeClosingTag(L"mapconfig");

        //remove Xml writer
        writer->drop();

        return true;
}

//Return true in case of success, false otherwise
bool InfrastructureBase::GetCloudColorValue(irr::core::map<irr::core::stringw, irr::core::stringw> *valueMap, irr::core::stringw keyName, irr::video::SColor &outColor) {
    irr::core::map<irr::core::stringw, irr::core::stringw>::Node* nodePntr;

    nodePntr = valueMap->find(keyName);

    if (nodePntr == nullptr) {
        std::string overallErrMsg("ReadMapConfigFile: Missing ");
        overallErrMsg.append(WStringToStdString(keyName.c_str()));
        overallErrMsg.append("field in Mapconfig Xml file");
        logging::Error(overallErrMsg);

        return false;
    } else {
       irr::video::SColor outCol;
       if (!XmlSettingStrToSColor(nodePntr->getValue(), outCol)) {
           std::string overallErrMsg("ReadMapConfigFile: Missformed field value for ");
           overallErrMsg.append(WStringToStdString(keyName.c_str()));
           overallErrMsg.append(" in Mapconfig Xml file");
           logging::Error(overallErrMsg);

           return false;
       }

       outColor = outCol;
    }

    return true;
}

//Returns true in case of success, False otherwise
//Read values are returned in second parameter
bool InfrastructureBase::ReadMapConfigFile(const std::string fileName, MapConfigStruct &configStruct) {
        //create the Xml Reader
        irr::io::IXMLReader* xmlReader = mDevice->getFileSystem()->createXMLReader(fileName.c_str());
        if (!xmlReader) {
            std::string overallMsg("Failed to read map config file ");
            overallMsg.append(fileName);
            overallMsg.append("!");
            logging::Error(overallMsg);

            return false;
        }

        const irr::core::stringw settingTag(L"setting");
        const irr::core::stringw skySetupTag(L"sky");
        const irr::core::stringw musicSetupTag(L"music");
        const irr::core::stringw minimapSetupTag(L"minimap");
        const irr::core::stringw texBaseSetupTag(L"texturebase");

        irr::core::stringw currentSection;

        irr::core::map<irr::core::stringw, irr::core::stringw> skySetupMap;
        irr::core::map<irr::core::stringw, irr::core::stringw> musicSetupMap;
        irr::core::map<irr::core::stringw, irr::core::stringw> minimapSetupMap;
        irr::core::map<irr::core::stringw, irr::core::stringw> texBaseSetupMap;

        //while there is more to read
        while (xmlReader->read())
        {
            //check the node type
            switch (xmlReader->getNodeType())
            {
                //we found a new element
                case irr::io::EXN_ELEMENT:
                {
                     //we currently are in the empty or mapconfig section and find the sky setup tag so we set our current section to sky setup
                     if (currentSection.empty() && skySetupTag.equals_ignore_case(xmlReader->getNodeName()))
                        {
                            currentSection = skySetupTag;
                        }

                     //we are in the sky setup section and we find a setting to parse
                     else if (currentSection.equals_ignore_case(skySetupTag) && settingTag.equals_ignore_case(xmlReader->getNodeName()))
                        {
                           //read in the key
                            irr::core::stringw key = xmlReader->getAttributeValueSafe(L"name");
                            //if there actually is a key to set
                            if (!key.empty())
                            {
                                //set the setting in the map to the value,
                                //the [] operator overrides values if they already exist or inserts a new key value
                                //pair into the settings map if it was not defined yet
                                skySetupMap[key] = xmlReader->getAttributeValueSafe(L"value");
                            }
                        }

                     //we currently are in the empty or mapconfig section and find the music setup tag so we set our current section to music setup
                     if (currentSection.empty() && musicSetupTag.equals_ignore_case(xmlReader->getNodeName()))
                        {
                            currentSection = musicSetupTag;
                        }

                     //we are in the music setup section and we find a setting to parse
                     else if (currentSection.equals_ignore_case(musicSetupTag) && settingTag.equals_ignore_case(xmlReader->getNodeName()))
                        {
                           //read in the key
                            irr::core::stringw key = xmlReader->getAttributeValueSafe(L"name");
                            //if there actually is a key to set
                            if (!key.empty())
                            {
                                //set the setting in the map to the value,
                                //the [] operator overrides values if they already exist or inserts a new key value
                                //pair into the settings map if it was not defined yet
                                musicSetupMap[key] = xmlReader->getAttributeValueSafe(L"value");
                            }
                        }

                     //we currently are in the empty or mapconfig section and find the tex base setup tag so we set our current section to tex base setup
                     if (currentSection.empty() && texBaseSetupTag.equals_ignore_case(xmlReader->getNodeName()))
                        {
                            currentSection = texBaseSetupTag;
                        }

                     //we are in the texture base setup section and we find a setting to parse
                     else if (currentSection.equals_ignore_case(texBaseSetupTag) && settingTag.equals_ignore_case(xmlReader->getNodeName()))
                        {
                           //read in the key
                            irr::core::stringw key = xmlReader->getAttributeValueSafe(L"name");
                            //if there actually is a key to set
                            if (!key.empty())
                            {
                                //set the setting in the map to the value,
                                //the [] operator overrides values if they already exist or inserts a new key value
                                //pair into the settings map if it was not defined yet
                                texBaseSetupMap[key] = xmlReader->getAttributeValueSafe(L"value");
                            }
                        }

                     //we currently are in the empty or another config section and find the minimap setup tag so we set our current section to minimap setup
                      if (currentSection.empty() && minimapSetupTag.equals_ignore_case(xmlReader->getNodeName()))
                             {
                                 currentSection = minimapSetupTag;
                             }

                     //we are in the minimap setup section and we find a setting to parse
                     else if (currentSection.equals_ignore_case(minimapSetupTag) && settingTag.equals_ignore_case(xmlReader->getNodeName()))
                            {
                                 //read in the key
                                 irr::core::stringw key = xmlReader->getAttributeValueSafe(L"name");
                                 //if there actually is a key to set
                                 if (!key.empty())
                                 {
                                   //set the setting in the map to the value,
                                   //the [] operator overrides values if they already exist or inserts a new key value
                                   //pair into the settings map if it was not defined yet
                                   minimapSetupMap[key] = xmlReader->getAttributeValueSafe(L"value");
                                 }
                            }
                }
                break;

                //we found the end of an element
                case irr::io::EXN_ELEMENT_END:
                     //we were at the end of the sky/music/minimap setup section so we reset our tag
                     currentSection=L"";
                     break;
                }
       }

       // don't forget to delete the xml reader
       xmlReader->drop();

       irr::core::map<irr::core::stringw, irr::core::stringw>::Node* nodePntr;

       //get name of vanillaSkyImage
       nodePntr = skySetupMap.find(L"vanillaSkyImage");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing vanillaSkyImage field in Mapconfig Xml file");
           return false;
       } else {
          configStruct.SkyImageFileVanilla.clear();
          configStruct.SkyImageFileVanilla.append(WStringToStdString(nodePntr->getValue().c_str()));
       }

       //get name of upgradedSkyImage
       nodePntr = skySetupMap.find(L"upgradedSkyImage");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing upgradedSkyImage field in Mapconfig Xml file");
           return false;
       } else {
          configStruct.SkyImageFileUpgradedSky.clear();
          configStruct.SkyImageFileUpgradedSky.append(WStringToStdString(nodePntr->getValue().c_str()));
       }

       //get value for enableLensflare
       nodePntr = skySetupMap.find(L"enableLensflare");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing enableLensflare field in Mapconfig Xml file");
           return false;
       } else {
          bool outBool;
          if (!XmlSettingStrToBool(nodePntr->getValue(), outBool)) {
              logging::Error("ReadMapConfigFile: Missformed field value for enableLensflare in Mapconfig Xml file");
              return false;
          }

          configStruct.EnableLensFlare = outBool;
       }

       //get value for locationLensflare
       nodePntr = skySetupMap.find(L"locationLensflare");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing locationLensflare field in Mapconfig Xml file");
           return false;
       } else {
          irr::core::vector3df outVector;
          if (!XmlSettingStrToVector3df(nodePntr->getValue(), outVector)) {
              logging::Error("ReadMapConfigFile: Missformed field value for locationLensflare in Mapconfig Xml file");
              return false;
          }

          configStruct.lensflareLocation = outVector;
       }

       //get value for cloudColorCenter1
       irr::video::SColor clColor;

       if (!GetCloudColorValue(&skySetupMap, L"cloudColorCenter1", clColor)) {
           return false;
       }

       configStruct.cloudColorCenter1 = clColor;

       //get value for cloudColorInner1
       if (!GetCloudColorValue(&skySetupMap, L"cloudColorInner1", clColor)) {
           return false;
       }

       configStruct.cloudColorInner1 = clColor;

       //get value for cloudColorOuter1
       if (!GetCloudColorValue(&skySetupMap, L"cloudColorOuter1", clColor)) {
           return false;
       }

       configStruct.cloudColorOuter1 = clColor;

       //get value for cloudColorCenter2
       if (!GetCloudColorValue(&skySetupMap, L"cloudColorCenter2", clColor)) {
           return false;
       }

       configStruct.cloudColorCenter2 = clColor;

       //get value for cloudColorInner2
       if (!GetCloudColorValue(&skySetupMap, L"cloudColorInner2", clColor)) {
           return false;
       }

       configStruct.cloudColorInner2 = clColor;

       //get value for cloudColorOuter2
       if (!GetCloudColorValue(&skySetupMap, L"cloudColorOuter2", clColor)) {
           return false;
       }

       configStruct.cloudColorOuter2 = clColor;

       //get value for cloudColorCenter3
       if (!GetCloudColorValue(&skySetupMap, L"cloudColorCenter3", clColor)) {
           return false;
       }

       configStruct.cloudColorCenter3 = clColor;

       //get value for cloudColorInner3
       if (!GetCloudColorValue(&skySetupMap, L"cloudColorInner3", clColor)) {
           return false;
       }

       configStruct.cloudColorInner3 = clColor;

       //get value for cloudColorOuter3
       if (!GetCloudColorValue(&skySetupMap, L"cloudColorOuter3", clColor)) {
           return false;
       }

       configStruct.cloudColorOuter3 = clColor;

       //get name of music file
       nodePntr = musicSetupMap.find(L"musicFile");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing musicFile field in Mapconfig Xml file");
           return false;
       } else {
          configStruct.MusicFile.clear();
          configStruct.MusicFile.append(WStringToStdString(nodePntr->getValue().c_str()));
       }

       //get path to textue base
       nodePntr = texBaseSetupMap.find(L"texturebaseLocation");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing texturebaseLocation field in Mapconfig Xml file");
           return false;
       } else {
          configStruct.texBaseLocation.clear();
          configStruct.texBaseLocation.append(WStringToStdString(nodePntr->getValue().c_str()));
       }

       //get value for useCustomTextures
       nodePntr = texBaseSetupMap.find(L"useCustomTextures");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing useCustomTextures field in Mapconfig Xml file");
           return false;
       } else {
          bool outBool;
          if (!XmlSettingStrToBool(nodePntr->getValue(), outBool)) {
              logging::Error("ReadMapConfigFile: Missformed field value for useCustomTextures in Mapconfig Xml file");
              return false;
          }

          configStruct.useCustomTextures = outBool;
       }

       //Minimap setup
       //get value for minimapCalSet
       nodePntr = minimapSetupMap.find(L"minimapCalSet");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing minimapCalSet field in Mapconfig Xml file");
           return false;
       } else {
          bool outBool;
          if (!XmlSettingStrToBool(nodePntr->getValue(), outBool)) {
              logging::Error("ReadMapConfigFile: Missformed field value for minimapCalSet in Mapconfig Xml file");
              return false;
          }

          configStruct.minimapCalSet = outBool;
       }

       //get value for minimapCalStartVal
       nodePntr = minimapSetupMap.find(L"minimapCalStartVal");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing minimapCalStartVal field in Mapconfig Xml file");
           return false;
       } else {
          irr::core::vector2di outVector;
          if (!XmlSettingStrToVector2di(nodePntr->getValue(), outVector)) {
              logging::Error("ReadMapConfigFile: Missformed field value for minimapCalStartVal in Mapconfig Xml file");
              return false;
          }

          configStruct.minimapCalStartVal = outVector;
       }

       //get value for minimapCalEndVal
       nodePntr = minimapSetupMap.find(L"minimapCalEndVal");

       if (nodePntr == nullptr) {
           logging::Error("ReadMapConfigFile: Missing minimapCalEndVal field in Mapconfig Xml file");
           return false;
       } else {
          irr::core::vector2di outVector;
          if (!XmlSettingStrToVector2di(nodePntr->getValue(), outVector)) {
              logging::Error("ReadMapConfigFile: Missformed field value for minimapCalEndVal in Mapconfig Xml file");
              return false;
          }

          configStruct.minimapCalEndVal = outVector;
       }

       return true;
}

//The original game executable contains the Credits for the overall game development team; Unfortunetly when writting this
//credit information a mistake was made, and instead of a single 0 value byte between two names at one location two zero
//value bytes in a row were used. Unfortunetly this throws my simpler parsing solution off, and some names get lost
//To fix this issue this workaround method below makes sure that even though this happens we still get all the names
//of the team
//Workaround is to check if between the first two detected 0 value bytes in a row, and the next two 0 value bytes in a row
//we also find the necessary ':' character at the end of the role string; If not clearly no new role was started, and we have
//the mistake in front of us (which we then ignore). If we find the ':' character inbetween we know a new role was
//started, and everything is ok
//To get also the last role/name (this one is missing the ':' character at the end), we can use the '!'
//right at the end, to also accept this one
//Method below returns true in case a ':' character was found inbetween, which means a new role was started
//Returns false in case no ':' character was found inbetween, we have the mistake in front of use, and need to continue
//collecting names
bool InfrastructureBase::ParseOriginalGameCreditsWorkaround(std::vector<uint8_t> mGameCreditsInformation, size_t startIdx) {
    size_t byteVal0Cnter = 0;
    size_t currIdx;
    size_t lastIdx = mGameCreditsInformation.size();

    for (currIdx = startIdx; currIdx < lastIdx; currIdx++) {
        if ((mGameCreditsInformation.at(currIdx) == ':') || (mGameCreditsInformation.at(currIdx) == '!')) {
            return true;
        }

        //search for next two 0x0 bytes in a row
        if (mGameCreditsInformation.at(currIdx) == 0x0) {
            byteVal0Cnter++;

            if (byteVal0Cnter == 2) {
                if (mGameCreditsInformation.at(currIdx + 1) == '!') {
                    //so that we also accept the final role/name
                    return true;
                }

                return false;
            }
        } else {
            byteVal0Cnter = 0;
        }
    }

    return false;
}

//Returns true in case of success, False otherwise
bool InfrastructureBase::ParseOriginalGameCredits(std::vector<OriginalGameCreditStruct*>& originalGameCredits) {
    //First locate hioctane.exe file
    irr::io::path gameExeFile =
            LocateFileInFileList(mOriginalGame->execFolder, irr::core::string<fschar_t>("hioctane.exe"));

    if (gameExeFile.empty()) {
        //exe file not found!
         throw std::string("Could not locate the original game exe file hioctane.exe");
    }

    //open file and search for the credits information
    bool foundStart = false;
    bool foundEnd = false;
    irr::u32 fndPosStart = 0;
    irr::u32 fndPosEnd = 0;

    /* try to open file to read */
    ifstream ifile;
    std::streampos fileSize;

    ifile.open(gameExeFile.c_str(), std::ifstream::binary);
       if(!ifile) {
           logging::Error("Could not open hioctane.exe file!");
           return false;
       }

     // get its size:
     ifile.seekg(0, std::ios::end);
     fileSize = ifile.tellg();
     ifile.seekg(0, std::ios::beg);

     std::vector<uint8_t> *fileData = new std::vector<uint8_t>(fileSize);

     ifile.read(reinterpret_cast<char*>(fileData->data()), fileData->size());

     if (!ifile) {
         size_t fileReadsizet = ifile.gcount();
         char hlpstr[500];
         std::string msg("hioctane.exe file read error: only ");
         snprintf(hlpstr, 500, "%zu", fileReadsizet);
         msg.append(hlpstr);
         msg.append(" bytes could be read!");
         logging::Error(msg);

         delete fileData;
         return false;
     }

     std::vector<uint8_t>::iterator it;
     std::vector<uint8_t> compareStrStart ({0x46, 0x4F, 0x52, 0x20, 0x42, 0x55, 0x4C,
                                           0x4C, 0x46, 0x52, 0x4F, 0x47, 0x20, 0x50, 0x52, 0x4F, 0x44, 0x55, 0x43, 0x54,
                                           0x49, 0x4F, 0x4E, 0x53, 0x20, 0x4C, 0x54, 0x44, 0x2E, 0x00});

     std::vector<uint8_t> compareStrEnd ({0x24, 0x53, 0x31, 0x20, 0x25, 0x73, 0x25, 0x73, 0x00,
                                         0x25, 0x73, 0x25, 0x73, 0x00});

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
        logging::Error("hioctane.exe credits information not found!");
        delete fileData;
        return false;
    }

    std::vector<uint8_t> mGameCreditsInformation;

    mGameCreditsInformation.clear();

    std::vector<uint8_t>::iterator start = fileData->begin() + fndPosStart + 1;
    std::vector<uint8_t>::iterator end = fileData->begin() + fndPosEnd - (compareStrEnd.size() - 1);
    uint8_t currChar;

    for (it = start; it != end; ++it) {
        currChar = (*it);
    /*    if (currChar == 0) {
            currChar = ' ';
        }*/

        mGameCreditsInformation.push_back(currChar);
    }

    delete fileData;

    //now begin to further parse the collected
    //credits information
    originalGameCredits.clear();

    size_t currIdx = 0;
    size_t lastIdx = mGameCreditsInformation.size();
    size_t byteVal0Cnter = 0;
    std::string currRole("");
    std::string currName("");

    std::vector<std::string> currNames;

    bool roleStarted = false;
    bool namesStarted = false;

    //Before each new role the executable information contains two
    //Bytes with value 0x0; Therefore we simply search for two 0x0 bytes
    //in a row, and then we can start to collect a new role; Each role
    //ends with a ':' character, after the role there are the names,
    //each seperated with a single byte with value 0x0
    for (currIdx = 0; currIdx < lastIdx; currIdx++) {
        if (namesStarted && (mGameCreditsInformation.at(currIdx) != 0x0)) {
            currName.push_back(mGameCreditsInformation.at(currIdx));
        }

        if (roleStarted) {
            if (mGameCreditsInformation.at(currIdx) == ':') {
                roleStarted = false;
                namesStarted = true;
                currName.clear();
                currNames.clear();
            }

            currRole.push_back(mGameCreditsInformation.at(currIdx));
        }

        //search for two 0x0 bytes in a row
        if (mGameCreditsInformation.at(currIdx) == 0x0) {
            byteVal0Cnter++;

            if ((namesStarted) && (byteVal0Cnter == 1)) {
                //a new name is ready
                if (currName.size() > 0) {
                    currNames.push_back(currName);
                }

                currName.clear();
            }

            if ((byteVal0Cnter == 2) && ParseOriginalGameCreditsWorkaround(mGameCreditsInformation, currIdx)) {
                if (namesStarted) {
                    //a role was completely finished
                    //start working at the next role
                    OriginalGameCreditStruct* newStruct = new OriginalGameCreditStruct();
                    newStruct->role = currRole;
                    newStruct->individualsVec = currNames;

                    //store the new gained information
                    originalGameCredits.push_back(newStruct);

                    namesStarted = false;
                }

                //we found the beginning of a new role
                currRole.clear();
                roleStarted = true;
            }
        } else {
            byteVal0Cnter = 0;
        }
    }

    return true;
}

irr::io::IFileList* InfrastructureBase::CreateFileList(irr::io::path whichAbsPath) {
    //set current working directory
    if (!mDevice->getFileSystem()->changeWorkingDirectoryTo(whichAbsPath)) {
        return nullptr;
    }

    //create list of files in the current working directory
    irr::io::IFileList* fileList = mDevice->getFileSystem()->createFileList();

    //restore original working dir for this project
    if (!mDevice->getFileSystem()->changeWorkingDirectoryTo(mGameRootDir)) {
        return nullptr;
    }

    return fileList;
}

io::path InfrastructureBase::RemoveFileEndingFromFileName(io::path fileName) {
    io::path result("");

    //Returns -1 if not found
    irr::s32 dotPos = fileName.findLast('.');

    if (dotPos != -1) {
        //get rid of the file ending for comparison
        result = fileName.subString(0, dotPos);
    } else {
        result = fileName;
    }

    return result;
}

io::path InfrastructureBase::GetFileEndingFromFileName(io::path fileName) {
    io::path result("");

    //Returns -1 if not found
    irr::s32 dotPos = fileName.findLast('.');

    if (dotPos != -1) {
        //derive file ending
        result = fileName.subString((irr::u32)(dotPos) + 1, fileName.size() - dotPos - 1);
    } else {
        result = fileName;
    }

    return result;
}

//if specified file is not found, returns empty path
irr::io::path InfrastructureBase::LocateFileInFileList(irr::io::IFileList* fileList, irr::core::string<fschar_t> fileName,
                                                       bool ignoreFileEnding) {
    irr::u32 entriesFound = fileList->getFileCount();
    bool equals;
    io::path currName;

    for (irr::u32 idx = 0; idx < entriesFound; idx++) {
       currName = fileList->getFileName(idx);

       if (ignoreFileEnding) {
          currName = RemoveFileEndingFromFileName(currName);
       }

       //returns true if equal ignoring case
       equals = currName.equals_ignore_case(fileName);

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
           logging::Error("Could not open hioctane.exe file!");
           return false;
       }

     // get its size:
     ifile.seekg(0, std::ios::end);
     fileSize = ifile.tellg();
     ifile.seekg(0, std::ios::beg);

     std::vector<uint8_t> *fileData = new std::vector<uint8_t>(fileSize);

     ifile.read(reinterpret_cast<char*>(fileData->data()), fileData->size());

     if (!ifile) {
         size_t fileReadsizet = ifile.gcount();
         char hlpstr[500];
         std::string msg("hioctane.exe file read error: only ");
         snprintf(hlpstr, 500, "%zu", fileReadsizet);
         msg.append(hlpstr);
         msg.append(" bytes could be read!");
         logging::Error(msg);

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
        logging::Error("hioctane.exe version date not found!");
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

    if (fList == nullptr) {
        return false;
    }

    irr::io::path origGameRootDirPath = LocateFileInFileList(fList, irr::core::string<fschar_t>("originalgame"));

    //drop the file list again
    //not that we get a memory leak!
    fList->drop();

    //folder not found?
    if (origGameRootDirPath.empty()) {
        logging::Error("I was not able to locate the original game files");
        return false;
    }

    std::string msg("Original Game located in ");
    msg.append(origGameRootDirPath.c_str());
    logging::Info(msg);

    this->mOriginalGame = new OriginalGameFolderInfoStruct();

    //make file list for original games root folder
    this->mOriginalGame->rootFolder = CreateFileList(origGameRootDirPath);

    if (this->mOriginalGame->rootFolder == nullptr) {
        logging::Error("I was not able to create the file list for the original games root directory");
        return false;
    }

    /*****************************************************
     *  Data folder                                      *
     * ***************************************************/

    //now locate all the original game folders
    irr::io::path dataFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("data"));

    if (dataFolderPath.empty()) {
        logging::Error("I was not able to locate the original game data folder");
        return false;
    }

    this->mOriginalGame->dataFolder = CreateFileList(dataFolderPath);

    if (this->mOriginalGame->dataFolder == nullptr) {
        logging::Error("I was not able to create the file list for the original games data directory");
        return false;
    }

    /*****************************************************
     *  Exec folder                                      *
     * ***************************************************/

    irr::io::path execFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("exec"));

    if (execFolderPath.empty()) {
        logging::Error("I was not able to locate the original game exec folder");
        return false;
    }

    this->mOriginalGame->execFolder = CreateFileList(execFolderPath);

    if (this->mOriginalGame->execFolder == nullptr) {
        logging::Error("I was not able to create the file list for the original games exec directory");
        return false;
    }

    /*****************************************************
     *  Maps folder                                      *
     * ***************************************************/

    irr::io::path mapsFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("maps"));

    if (mapsFolderPath.empty()) {
        logging::Error("I was not able to locate the original game maps folder");
        return false;
    }

    this->mOriginalGame->mapsFolder = CreateFileList(mapsFolderPath);

    if (this->mOriginalGame->mapsFolder == nullptr) {
        logging::Error("I was not able to create the file list for the original games maps directory");
        return false;
    }

    /*****************************************************
     *  Objects/data folder                              *
     * ***************************************************/

    irr::io::path objectsFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("objects"));

    if (objectsFolderPath.empty()) {
        logging::Error("I was not able to locate the original game objects folder");
        return false;
    }

    //under objects subdir there is another folder "data", for whatever reason
    irr::io::IFileList* helperList = CreateFileList(objectsFolderPath);

    if (helperList == nullptr) {
        logging::Error("I was not able to create the file list for the original games objects directory");
        return false;
    }

    //now search for the data folder inside the objects folder
    irr::io::path objectsFolderDataPath = LocateFileInFileList(helperList, irr::core::string<fschar_t>("data"));
    helperList->drop();

    if (objectsFolderDataPath.empty()) {
        logging::Error("I was not able to locate the data directory inside the original games objects folder");
        return false;
    }

    this->mOriginalGame->objectsFolder = CreateFileList(objectsFolderDataPath);

    if (this->mOriginalGame->objectsFolder == nullptr) {
        logging::Error("I was not able to create the file list for the original games objects/data directory");
        return false;
    }

    /*****************************************************
     *  HIOCTANE.CD/SAVE folder                              *
     * ***************************************************/

    irr::io::path hioctaneCdFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("HIOCTANE.CD"));

    if (hioctaneCdFolderPath.empty()) {
        logging::Info("No HIOCTANE.CD folder found => create it");
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
            logging::Error("Failed to create HIOCTANE.CD folder");
            return false;
        }
    }

    //under HIOCTANE.CD subdir there is another folder "save"
    helperList = CreateFileList(hioctaneCdFolderPath);

    if (helperList == nullptr) {
        logging::Error("I was not able to create the file list for the original games HIOCTANE.CD directory");
        return false;
    }

    //now search for the SAVE folder inside the objects folder
    saveFolderDataPath = LocateFileInFileList(helperList, irr::core::string<fschar_t>("SAVE"));
    helperList->drop();

    if (saveFolderDataPath.empty()) {
        logging::Info("I did not find a save directory inside the original games HIOCTANE.CD folder => create it");
        try {
            char newDir[100];
            strcpy(newDir, hioctaneCdFolderPath.c_str());
            strcat(newDir, (char*)("/save"));
            CreateDirectory(newDir);

            //need to update file list
            //under HIOCTANE.CD subdir there is another folder "save"
            helperList = CreateFileList(hioctaneCdFolderPath);

            if (helperList == nullptr) {
                logging::Error("I was not able to create the file list for the original games HIOCTANE.CD directory");
                return false;
            }

            saveFolderDataPath = LocateFileInFileList(helperList, irr::core::string<fschar_t>("SAVE"));
            helperList->drop();
        }  catch (...) {
            logging::Error("Failed to create save folder");
            return false;
        }
    }

    this->mOriginalGame->saveFolder = CreateFileList(saveFolderDataPath);

    if (this->mOriginalGame->saveFolder == nullptr) {
        logging::Error("I was not able to create the file list for the original games HIOCTANE.CD/SAVE directory");
        return false;
    }

    /*****************************************************
     *  Sound folder                                     *
     * ***************************************************/

    irr::io::path soundFolderPath = LocateFileInFileList(mOriginalGame->rootFolder, irr::core::string<fschar_t>("sound"));

    if (soundFolderPath.empty()) {
        logging::Error("I was not able to locate the original game sound folder");
        return false;
    }

    this->mOriginalGame->soundFolder = CreateFileList(soundFolderPath);

    if (this->mOriginalGame->soundFolder == nullptr) {
        logging::Error("I was not able to create the file list for the original games sound directory");
        return false;
    }

    //all ok
    return true;
}

bool InfrastructureBase::InitGameResourcesInitialStep() {
    /***********************************************************/
    /* Extract first game assets needed to show a first        */
    /* graphical screen to the user while the original games   */
    /* data is extracted                                       */
    /* Most of the data extraction happens later from the      */
    /* games main loop, so that rendering is not               */
    /* completely blocked                                      */
    /***********************************************************/
    try {
        mPrepareData = new PrepareData(this);
    }
    catch (const std::string &msg) {
        std::string msgExt("Initial part of game assets preparation operation failed: ");
        msgExt.append(msg);
        logging::Error(msgExt);
        return false;
    }

    /***********************************************************/
    /* Load the first initial GameFont, so that we can show    */
    /* a first graphical screen                                */
    /***********************************************************/
    mGameTexts = new GameText(mDevice, mDriver);

    if (!mGameTexts->GameTextInitializedOk) {
        logging::Error("First Game font init operation failed!");
        return false;
    }

    //All preparations succesfull to show a first
    //graphical screen
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

//returns true if the original game version date is known,
//and we can use it, False otherwise
bool InfrastructureBase::ProcessGameVersionDate() {
    std::string versionStr(this->mGameVersionDate.begin(), this->mGameVersionDate.end());

    std::string msg("Original game version date = ");
    msg.append(versionStr);
    logging::Info(msg);

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

    logging::Error("Unknown game version date!");

    return false;
}

bool InfrastructureBase::UpdateFileListSaveFolder() {
    if (this->mOriginalGame->saveFolder != nullptr) {
        this->mOriginalGame->saveFolder->drop();
    }

    //recreate the file list
    this->mOriginalGame->saveFolder = CreateFileList(saveFolderDataPath);

    if (this->mOriginalGame->saveFolder == nullptr) {
        logging::Error("I was not able to create the file list for the original games HIOCTANE.CD/SAVE directory");
        return false;
    }

    return true;
}

//Returns true if specified character is valid for a levelname
//False otherwise
//valid are a-z, A-Zm 0-9, _, - characters
bool InfrastructureBase::IsValidCharForLevelName(wchar_t* whichChar) {
   //any alphanumeric character is valid
   if (std::iswalpha(*whichChar) != 0)
       return true;

   //any number is valid
   if (std::iswdigit(*whichChar) != 0)
       return true;

   if ((*whichChar == L'_') || (*whichChar == L'-'))
       return true;

   //all other characters are invalid
   return false;
}

//Returns true if specified level name is valid
//means only a-z, A-Zm 0-9, _, - characters
//False otherwise
bool InfrastructureBase::CheckForValidLevelName(std::wstring levelName) {
   size_t strLen = levelName.size();
   wchar_t ch;

   for (size_t index = 0; index < strLen; index++) {
       ch = levelName.at(index);
       if (!IsValidCharForLevelName(&ch)) {
           //found an invalid character
           //stop and return invalid result
           return false;
       }
   }

   //all characters valid
   return true;
}

std::string InfrastructureBase::GetDescriptionLevel(io::path levelFilename) {
    //unfortunetly we need to read the whole level data again to be able to
    //calculate the Crc
    ifstream ifile;
    std::streampos fileSize;

    ifile.open(levelFilename.c_str(), std::ifstream::binary);

    // get its size:
    ifile.seekg(0, std::ios::end);
    fileSize = ifile.tellg();
    ifile.seekg(0, std::ios::beg);

    std::vector<uint8_t> *m_bytes = new std::vector<uint8_t>();

    //read the data
    m_bytes->resize(fileSize);
    ifile.read(reinterpret_cast<char*>(m_bytes->data()), m_bytes->size());

    ifile.close();

    //calculate the Crc32 checksum of the level file to be able
    //to detect which level we load from the original game levels
    //or to see if we have a custom/user modified level
    uint32_t cs = mCrc32->ComputeChecksum(*m_bytes);

    //use crc to find out which map was loaded
    std::string description("");
    if (mExtendedGame) {
        switch (cs) {
          case 3308913189: description.append("Amazon Delta Turnpike"); break;
          case 2028380229: description.append("Trans-Asia Interstate"); break;
          case 3087166776: description.append("Shanghai Dragon"); break;
          case 1401140937: description.append("New Chernobyl Central"); break;
          case 4215346212: description.append("Slam Canyon"); break;
          case 3809451489: description.append("Thrak City"); break;
          case 3062313907: description.append("Ancient Mine Town"); break;
          case 3081898808: description.append("Arctic Land"); break;
          case 540505443: description.append("Death Match Arena"); break;
          default: description.append("Custom (modified)"); break;
        }
    } else {
        //non extended game version, here we have other Crc32 values
        //because the game levels were also modified/extended in the
        //extended game version, so the Crc32 checksum of the files has changed
        //as well
        switch (cs) {
          case 2413984012: description.append("Amazon Delta Turnpike"); break;
          case 641962188: description.append("Trans-Asia Interstate"); break;
          case 4059358834: description.append("Shanghai Dragon"); break;
          case 2106663236: description.append("New Chernobyl Central"); break;
          case 2386808163: description.append("Slam Canyon"); break;
          case 3362410748: description.append("Thrak City"); break;
          default: description.append("Custom (modified)"); break;
        }
    }

    delete m_bytes;

    return description;
}

//Returns a vector of existing levels in a specified root directory
void InfrastructureBase::GetExistingLevelInfo(std::string rootDir, bool markAsCustomLevel,
                                              std::vector<LevelFolderInfoStruct*> &levelInfoVec) {
    levelInfoVec.clear();

    irr::io::IFileList* extractFolder = nullptr;
    irr::io::path rootPath(rootDir.c_str());

    extractFolder = CreateFileList(rootPath);

    if (extractFolder == nullptr)
        return;

    irr::u32 listEntryCnt = extractFolder->getFileCount();
    irr::io::path fname;
    irr::io::path expectedLevelFileName;
    irr::io::path levelBaseDir;
    std::string description("");

    for (irr::u32 idx = 0; idx < listEntryCnt; idx++) {
        if (extractFolder->isDirectory(idx)) {
            fname = extractFolder->getFileName(idx);

            while (!levelBaseDir.empty()) {
                levelBaseDir.erase(0);
            }

            levelBaseDir.append(rootDir.c_str());
            levelBaseDir.append("/");
            levelBaseDir.append(fname);
            levelBaseDir.append("/");

            //if this is a valid level directory (containing a map)
            //we should find a file inside with the correct name
            //"[DIRNAME]-unpacked.dat"
            //First create the expected filename
            while (!expectedLevelFileName.empty()) {
                expectedLevelFileName.erase(0);
            }

            expectedLevelFileName.append(levelBaseDir);
            expectedLevelFileName.append(fname);
            expectedLevelFileName.append("-unpacked.dat");

            description.clear();

            //does this file exist, and is it a valid level file?
            if (IsUnpackedLevelFileValid(expectedLevelFileName)) {
                //find description for the level we found
                description.append(GetDescriptionLevel(expectedLevelFileName));

                //yes, is a valid level file, add to our vector
                LevelFolderInfoStruct* newEntry = new LevelFolderInfoStruct();
                newEntry->levelName = std::string(fname.c_str());
                newEntry->levelFileName = expectedLevelFileName;
                newEntry->isCustomLevel = (markAsCustomLevel || (description.compare("Custom") == 0));
                newEntry->levelBaseDir = levelBaseDir;
                newEntry->description = description;

                levelInfoVec.push_back(newEntry);
            }
        }
    }

    //cleanup the fileList object again
    extractFolder->drop();
}

irr::io::path InfrastructureBase::GetMiniMapFileName(LevelFolderInfoStruct* whichLevel) {
    irr::io::path resultPath("");

    if (whichLevel == nullptr)
        return resultPath;

    resultPath.append(whichLevel->levelBaseDir);
    resultPath.append("minimap.bmp");
    return resultPath;
}

irr::io::path InfrastructureBase::GetMiniMapFileName(std::string levelRootPath) {
    irr::io::path resultPath("");

    resultPath.append(levelRootPath.c_str());
    resultPath.append("minimap.bmp");
    return resultPath;
}

irr::io::path InfrastructureBase::GetMapConfigFileName(LevelFolderInfoStruct* whichLevel) {
    irr::io::path resultPath("");

    if (whichLevel == nullptr)
        return resultPath;

    resultPath.append(whichLevel->levelBaseDir);
    resultPath.append("mapconfig.xml");
    return resultPath;
}

irr::io::path InfrastructureBase::GetMapConfigFileName(std::string levelRootPath) {
    irr::io::path resultPath("");

    resultPath.append(levelRootPath.c_str());
    resultPath.append("mapconfig.xml");
    return resultPath;
}

//Returns true if specified unpacked level file seems to be valid
//False otherwise
bool InfrastructureBase::IsUnpackedLevelFileValid(io::path levelFilename) {
    if (FileExists(levelFilename.c_str()) == 1) {
          //this file does exist
          //check if size is valid
          size_t fileSize = GetFileSizeBytes(levelFilename.c_str());

          if (fileSize == INFRA_LEVEL_FILE_VALIDSIZE_BYTES) {
              //level file size is valid
              return true;
          }
    }

    return false;
}

void InfrastructureBase::CleanupAllSceneNodes() {
    core::array<scene::ISceneNode*> outNodes;

    //get list of all existing sceneNodes
    mSmgr->getSceneNodesFromType(ESCENE_NODE_TYPE::ESNT_ANY, outNodes, 0);

    //remove all existing SceneNodes, only the root sceneNode is not removed
    irr::u32 nodeCnt = outNodes.size();

    for (irr::u32 idx = 0; idx < nodeCnt; idx++) {
        //remove this sceneNode from the SceneManager
        outNodes[idx]->remove();
    }
}

//Returns the number of bytes per pixel for a certain ECOLOR_FORMAT
//returns 0 for an undefined ECOLOR_FORMAT
irr::u32 InfrastructureBase::ReturnBytesPerPixel(irr::video::ECOLOR_FORMAT colFormat) {
    switch(colFormat) {
        //! 16 bit color format used by the software driver.
        case ECF_A1R5G5B5:
        //! Standard 16 bit color format.
        case ECF_R5G6B5:

        /** Floating Point formats. The following formats may only be used for render target textures. */
        //! 16 bit floating point format using 16 bits for the red channel.
        case ECF_R16F:
        {
           return 2;
        }

        //! 24 bit color, no alpha channel, but 8 bit for red, green and blue
        case ECF_R8G8B8:  {
           return 3;
        }

        //! Default 32 bit color format. 8 bits are used for every component: red, green, blue and alpha.
        case ECF_A8R8G8B8:
        //! 32 bit floating point format using 16 bits for the red channel and 16 bits for the green channel.
        case ECF_G16R16F:
        //! 32 bit floating point format using 32 bits for the red channel.
        case ECF_R32F:
        {
           return 4;
        }

        //! 64 bit floating point format 16 bits are used for the red, green, blue and alpha channels.
        case ECF_A16B16G16R16F:
        //! 64 bit floating point format using 32 bits for the red channel and 32 bits for the green channel.
        case ECF_G32R32F:
        {
           return 8;
        }

        //! 128 bit floating point format. 32 bits are used for the red, green, blue and alpha channels.
        case ECF_A32B32G32R32F:
        {
           return 16;
        }

        //! Unknown color format:
        case ECF_UNKNOWN:
        default: {
            return 0;
        }
    };
}

void InfrastructureBase::CopyTexture(irr::video::ITexture* source, irr::video::ITexture* target) {
    if ((source == nullptr) || (target == nullptr))
        return;

    if (source->getSize() != target->getSize())
        return;

    //25.07.2025: Normally I would like to compare Pitch too, but I had the problem
    //that the reported pitch from a render to texture texture in Irrlicht is always
    //return as 0, according to a Google Search this could be a old unsolved bug
    //Therefore I will not compare pitch; But because the size is identical, and I verify that the
    //color format is also identical there should be no problem because of this; I assume if the other
    //two things are indentical also the Pitch should be the same
    //irr:u32 srcPitch = source->getPitch();
    //irr::u32 targetPitch = target->getPitch();
    if (source->getColorFormat() != target->getColorFormat())
        return;

    //see comment above
    /*if (source->getPitch() != target->getPitch())
        return;*/

    //how many bytes to copy
    irr::u32 copyBytes = source->getSize().Width * source->getSize().Height * ReturnBytesPerPixel(source->getColorFormat());

    //if no pixels to copy, or undefined/unknown color format => exit, so that we do no harm
    if (copyBytes == 0)
        return;

    //we can simply copy the memory, lock the textures
    void* srcPntr = source->lock(E_TEXTURE_LOCK_MODE::ETLM_READ_ONLY);
    void* targetPntr = target->lock(E_TEXTURE_LOCK_MODE::ETLM_WRITE_ONLY);

    memcpy(targetPntr, srcPntr, copyBytes);

    //unlock the textures again
    target->unlock();
    source->unlock();
}

void InfrastructureBase::FillTexture(irr::video::ITexture* target, unsigned char fillvalue) {
    if (target == nullptr)
        return;

    //how many bytes to write
    irr::u32 writeBytes = target->getSize().Width * target->getSize().Height * ReturnBytesPerPixel(target->getColorFormat());

    //if no pixels to fill (bytes to write), or undefined/unknown color format => exit, so that we do no harm
    if (writeBytes == 0)
        return;

    //we can simply fill the memory, lock the textures
    void* targetPntr = target->lock(E_TEXTURE_LOCK_MODE::ETLM_WRITE_ONLY);

    memset(targetPntr, fillvalue, writeBytes);

    //unlock the textures again
    target->unlock();
}

InfrastructureBase::InfrastructureBase(int pArgc, char **pArgv, irr::u8 runningAsVal) {
    ParseCommandLineInformation(pArgc, pArgv);

    mRunningAs = runningAsVal;

    //if we are running as a game, create my game
    //configuration
    if (runningAsVal == INFRA_RUNNING_AS_GAME) {
        mGameConfig = new GameConfigStruct();
    }
}

void InfrastructureBase::ParseCommandLineInformation(int pArgc, char **pArgv) {
    mCLIVec.clear();

    mWriteLogFile = false;

    for (int idx = 0; idx < pArgc; idx++) {
        std::string newString(pArgv[idx]);

        //make string lower case
        std::transform(newString.begin(), newString.end(), newString.begin(),
                         [](unsigned char c){ return std::tolower(c); });

        mCLIVec.push_back(newString);
    }

    std::vector<std::string>::iterator it;
    std::string substr("logfile");

    for (it = mCLIVec.begin(); it != mCLIVec.end(); ++it) {
        //if one parameter contains substring "logfile" lets
        //enable writing of log file
        if ((*it).find(substr) != std::string::npos) {
            mWriteLogFile = true;
        }
    }
}

std::string InfrastructureBase::WStringToStdString(std::wstring inputStr) {
    std::string resultStr("");

    //How many bytes does MultiByte Char string need?
    size_t newStrLen = wcstombs(nullptr, inputStr.c_str(), 0) + 1;

    //create new buffer for multibyte string
    char* buffer = new char[newStrLen];

    //convert to multibyte char string
    wcstombs(buffer, inputStr.c_str(), newStrLen);

    // Creating std::string from char buffer
    resultStr.append(buffer);

    //delete buffer again
    delete[] buffer;

    return resultStr;
}

InfrastructureBase::~InfrastructureBase() {
    //cleanup game texts
    delete mGameTexts;

    delete mTimeProfiler;

    delete mDrawDebug;

    delete mCrc32;

    //cleanup the original game folder information
    if (mOriginalGame != nullptr) {
        if (mOriginalGame->dataFolder != nullptr) {
            mOriginalGame->dataFolder->drop();
            mOriginalGame->dataFolder = nullptr;
        }

        if (mOriginalGame->execFolder != nullptr) {
            mOriginalGame->execFolder->drop();
            mOriginalGame->execFolder = nullptr;
        }

        if (mOriginalGame->mapsFolder != nullptr) {
            mOriginalGame->mapsFolder->drop();
            mOriginalGame->mapsFolder = nullptr;
        }

        if (mOriginalGame->rootFolder != nullptr) {
            mOriginalGame->rootFolder->drop();
            mOriginalGame->rootFolder = nullptr;
        }

        if (mOriginalGame->saveFolder != nullptr) {
            mOriginalGame->saveFolder->drop();
            mOriginalGame->saveFolder = nullptr;
        }

        if (mOriginalGame->soundFolder != nullptr) {
            mOriginalGame->soundFolder->drop();
            mOriginalGame->soundFolder = nullptr;
        }

        if (mOriginalGame->objectsFolder != nullptr) {
            mOriginalGame->objectsFolder->drop();
            mOriginalGame->objectsFolder = nullptr;
        }

        delete mOriginalGame;
        mOriginalGame = nullptr;
    }

    if (mFontManager != nullptr) {
        delete mFontManager;
        mFontManager = nullptr;
    }

    if (mAttribution != nullptr) {
        delete mAttribution;
        mAttribution = nullptr;
    }

    //delete my axis direction vectors
    delete xAxisDirVector;
    delete yAxisDirVector;
    delete zAxisDirVector;

    if (mGameConfig != nullptr) {
        delete mGameConfig;
        mGameConfig = nullptr;
    }

    //if we did logging into a log file before
    //stop it
    if (mWriteLogFile) {
        logging::StopLogFile();
    }
}
