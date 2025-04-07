/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

// Class to extract game data files
//

#include "preparedata.h"

#include "../../utils/logging.h"
#include "../intro/flifix.h"
#include <iomanip>

void UnpackDataFile(const char* packfile, const char* unpackfile);
void ExtractImagesfromDataFile(const char* datfname, const char* tabfname, unsigned char* palette, const char* outputDir);
std::vector<unsigned char> loadRawFile(const char *filename);

void PrepareData::CreatePalette() {
    logging::Info("Create Palette information...");

    //locate the palette file
    irr::io::path palFilePath =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("palet0-0.dat"));

    if (palFilePath.empty()) {
        //palette file not found!
         throw std::string("Could not locate the original games palette file palet0-0.dat");
    }

    //create memory for ingame palette
    palette=static_cast<unsigned char*>(malloc(768));

    char* fname = strdup(palFilePath.c_str());

    //read ingame palette
    ReadPaletteFile(fname, palette);
    free(fname);
}

//returns true if data preparation was succesfully
//finished
bool PrepareData::ExecuteNextStep() {
    //if we are done, return true
    if (mCurrentStep == PREP_DATA_FINISHED)
        return true;

    switch (mCurrentStep) {
        case PREP_DATA_EXTRACTGAMESCREENS: {
            ExtractGameScreens();
            mCurrentStep = PREP_DATA_EXTRACTFONTS;
            currentStepDescription.clear();
            currentStepDescription.append("GAME FONTS");
            break;
         }

        case PREP_DATA_EXTRACTFONTS: {
            ExtractFonts();
            mCurrentStep = PREP_DATA_EXTRACTHUD;
            currentStepDescription.clear();
            currentStepDescription.append("GAME HUD");
            break;
        }

        case PREP_DATA_EXTRACTHUD: {
             ExtractHuds();
             mCurrentStep = PREP_DATA_EXTRACTSKIES;
             currentStepDescription.clear();
             currentStepDescription.append("GAME SKIES");
             break;
        }

        case PREP_DATA_EXTRACTSKIES: {
             ExtractSkies();
             mCurrentStep = PREP_DATA_EXTRACTSPRITES;
             currentStepDescription.clear();
             currentStepDescription.append("GAME SPRITES");
             break;
        }

        case PREP_DATA_EXTRACTSPRITES: {
             ExtractSprites();
             mCurrentStep = PREP_DATA_EXTRACTMINIMAPS;
             currentStepDescription.clear();
             currentStepDescription.append("GAME MINIMAPS");
             break;
        }

        case PREP_DATA_EXTRACTMINIMAPS: {
             ExtractMiniMaps();
             mCurrentStep = PREP_DATA_EXTRACTTERRAINTEXTURES;
             currentStepDescription.clear();
             currentStepDescription.append("GAME TERRAIN TEXTURES");
             break;
        }

        case PREP_DATA_EXTRACTTERRAINTEXTURES: {
            ExtractTerrainTextures();
            mCurrentStep = PREP_DATA_EXTRACTLEVELS;
            currentStepDescription.clear();
            currentStepDescription.append("GAME LEVELS");
            break;
        }

        case PREP_DATA_EXTRACTLEVELS: {
            ExtractLevels();
            mCurrentStep = PREP_DATA_EXTRACTMISC;
            currentStepDescription.clear();
            currentStepDescription.append("GAME MISC");
            break;
        }

        case PREP_DATA_EXTRACTMISC: {
            ExtractEditor();
            ExtractCheatPuzzle();
            mCurrentStep = PREP_DATA_EXTRACTMODELS;
            currentStepDescription.clear();
            currentStepDescription.append("GAME MODELS");
            break;
        }

        case PREP_DATA_EXTRACTMODELS: {
            ExtractModels();
            mCurrentStep = PREP_DATA_EXTRACTINTRO;
            currentStepDescription.clear();
            currentStepDescription.append("GAME INTRO");
            break;
        }

        case PREP_DATA_EXTRACTINTRO: {
            ExtractIntro();
            mCurrentStep = PREP_DATA_EXTRACTAUDIO;
            currentStepDescription.clear();
            currentStepDescription.append("GAME AUDIO");
            break;
        }

        case PREP_DATA_EXTRACTAUDIO: {
            ExtractAudio();
            mCurrentStep = PREP_DATA_FINISHED;
            currentStepDescription.clear();
            break;
        }
    }

    //not finished yet
    return false;
}

void PrepareData::ExtractInitialData() {
    logging::Info("Extracting first game logo and font...");
    PrepareSubDir("extract/images");

    ExtractSelectionScreenSVGA();

    PrepareSubDir("extract/fonts");
    PrepareSubDir("extract/fonts/smallsvga");

    ExtractSmallFontSVGA();

    mCurrentStep = PREP_DATA_EXTRACTGAMESCREENS;
    currentStepDescription.clear();
    currentStepDescription.append("Game screens");
}

//returns true if game data is already available
//false otherwise if further extraction is needed
bool PrepareData::GameDataAvailable() {
    if (mCurrentStep == PREP_DATA_FINISHED)
        return true;

    return false;
}

PrepareData::PrepareData(InfrastructureBase* mInfraPntr) {
    mInfra = mInfraPntr;

    //load palette information needed for the later
    //data preparation steps
    CreatePalette();

    //only add next line temporarily to extract level textures for level 7 up to 9
    //from HiOctaneTools project
    //AddOtherLevelsHiOctaneTools();

    //check if extraction directory is already present
    //if not create this directory
    if (IsDirectoryPresent("extract") == 1) {
        mCurrentStep = PREP_DATA_FINISHED;
        return;
    }

    //need to create folder for data
    //extraction
    CreateDirectory("extract");

    //only extract the one image we want to
    //show while data is prepared, and one of the fonts
    ExtractInitialData();

   /* ExtractGameScreens();
    ExtractFonts();
    ExtractHuds();
    ExtractSkies();
    ExtractSprites();
    ExtractMiniMaps();
    ExtractTerrainTextures();
    ExtractLevels();
    ExtractEditor();
    ExtractCheatPuzzle();
    ExtractModels();
    ExtractIntro();
    ExtractAudio();*/

    //29.03.2025: instead of adding this data, lets
    //implement support for the extended version
    //of the game
    //ExtractUserdata();
}

PrepareData::~PrepareData() {
    free(palette);
}

void PrepareData::ExtractGameScreens() {
    logging::Info("Extracting game logos...");
    //PrepareSubDir("extract/images");

    ExtractGameLogoSVGA();
    ExtractIntroductoryScreen();
    ExtractLoadingScreenSVGA();
    //ExtractSelectionScreenSVGA();
}

void PrepareData::ExtractFonts() {
    //extract SVGA game logo data if not all exported files present
    logging::Info("Extracting game fonts...");
    //PrepareSubDir("extract/fonts");

    PrepareSubDir("extract/fonts/thinwhite");
    ExtractThinWhiteFontSVGA();

    //PrepareSubDir("extract/fonts/smallsvga");
    //ExtractSmallFontSVGA();

    PrepareSubDir("extract/fonts/smallsvgagreenish");
    //create greenish font for unselected items in menue (but based for smaller text size)
    CreateFontForUnselectedItemsInMenue(
        "extract/fonts/smallsvga/osfnt0-1-",
        "extract/fonts/smallsvgagreenish/green-osfnt0-1-",
        0, 241);

    PrepareSubDir("extract/fonts/large");
    ExtractLargeFontSVGA();

    PrepareSubDir("extract/fonts/largegreenish");
    //create greenish font for unselected items in menue
    //based on white SVGA font already extracted for game banner text font
    CreateFontForUnselectedItemsInMenue(
        "extract/fonts/large/olfnt0-1-",
        "extract/fonts/largegreenish/green-olfnt0-1-",
        0, 241);

    PrepareSubDir("extract/fonts/largegreen");
    ExtractLargeGreenFontSVGA();
}

void PrepareData::ExtractHuds() {
    logging::Info("Extracting 1 player HUD...");
    PrepareSubDir("extract/hud1player");
    ExtractHUD1PlayerSVGA();

    logging::Info("Extracting 2 player HUD...");
    PrepareSubDir("extract/hud2player");
    ExtractHUD2PlayersSVGA();
}

void PrepareData::ExtractSkies() {
    logging::Info("Extracting sky...");
    PrepareSubDir("extract/sky");

    for (char skyNr = '0'; skyNr <= '5'; skyNr++) {
        ExtractSky(skyNr);
    }
}

void PrepareData::ExtractSprites() {
    logging::Info("Extracting sprites...");
    PrepareSubDir("extract/sprites");
    ExtractTmaps();
}

void PrepareData::ExtractMiniMaps() {
    logging::Info("Extracting minimaps...");
    PrepareSubDir("extract/minimaps");
    ExtractMiniMapsSVGA();
    StitchMiniMaps();
}

void PrepareData::ExtractTerrainTextures() {
    logging::Info("Extracting terrain textures...");

    if (!mInfra->mExtendedGame) {
        for (char levelNr = '1'; levelNr <= '6'; levelNr++) {
            ExtractTerrainTexture(levelNr);
        }
    } else {
        for (char levelNr = '1'; levelNr <= '9'; levelNr++) {
            ExtractTerrainTexture(levelNr);
        }
    }
}

void PrepareData::ExtractLevels() {
    logging::Info("Extracting levels...");

    //locate the needed input level file
    irr::io::path inputLevelFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-1.dat"));

    if (inputLevelFile.empty()) {
        //map file not found!
         throw std::string("Could not locate the original games map file level0-1.dat");
    }
    PrepareSubDir("extract/level0-1");
    UnpackDataFile(inputLevelFile.c_str(), "extract/level0-1/level0-1-unpacked.dat");

    inputLevelFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-2.dat"));

    if (inputLevelFile.empty()) {
        //map file not found!
         throw std::string("Could not locate the original games map file level0-2.dat");
    }
    PrepareSubDir("extract/level0-2");
    UnpackDataFile(inputLevelFile.c_str(), "extract/level0-2/level0-2-unpacked.dat");

    inputLevelFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-3.dat"));

    if (inputLevelFile.empty()) {
        //map file not found!
         throw std::string("Could not locate the original games map file level0-3.dat");
    }
    PrepareSubDir("extract/level0-3");
    UnpackDataFile(inputLevelFile.c_str(), "extract/level0-3/level0-3-unpacked.dat");

    inputLevelFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-4.dat"));

    if (inputLevelFile.empty()) {
        //map file not found!
         throw std::string("Could not locate the original games map file level0-4.dat");
    }
    PrepareSubDir("extract/level0-4");
    UnpackDataFile(inputLevelFile.c_str(), "extract/level0-4/level0-4-unpacked.dat");

    inputLevelFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-5.dat"));

    if (inputLevelFile.empty()) {
        //map file not found!
         throw std::string("Could not locate the original games map file level0-5.dat");
    }
    PrepareSubDir("extract/level0-5");
    UnpackDataFile(inputLevelFile.c_str(), "extract/level0-5/level0-5-unpacked.dat");

    inputLevelFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-6.dat"));

    if (inputLevelFile.empty()) {
        //map file not found!
         throw std::string("Could not locate the original games map file level0-6.dat");
    }
    PrepareSubDir("extract/level0-6");
    UnpackDataFile(inputLevelFile.c_str(), "extract/level0-6/level0-6-unpacked.dat");

    //if we have the extended game version we also need to extract
    //levels 7 up to 9
    if (mInfra->mExtendedGame) {
        //locate the needed input level file
        irr::io::path inputLevelFile =
                mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-7.dat"));

        if (inputLevelFile.empty()) {
            //map file not found!
             throw std::string("Could not locate the original games map file level0-7.dat");
        }
        PrepareSubDir("extract/level0-7");
        UnpackDataFile(inputLevelFile.c_str(), "extract/level0-7/level0-7-unpacked.dat");

        inputLevelFile =
                mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-8.dat"));

        if (inputLevelFile.empty()) {
            //map file not found!
             throw std::string("Could not locate the original games map file level0-8.dat");
        }
        PrepareSubDir("extract/level0-8");
        UnpackDataFile(inputLevelFile.c_str(), "extract/level0-8/level0-8-unpacked.dat");

        inputLevelFile =
                mInfra->LocateFileInFileList(mInfra->mOriginalGame->mapsFolder, irr::core::string<fschar_t>("level0-9.dat"));

        if (inputLevelFile.empty()) {
            //map file not found!
             throw std::string("Could not locate the original games map file level0-9.dat");
        }
        PrepareSubDir("extract/level0-9");
        UnpackDataFile(inputLevelFile.c_str(), "extract/level0-9/level0-9-unpacked.dat");
    }
}

void PrepareData::ExtractEditor() {
    logging::Info("Extracting editor...");
    PrepareSubDir("extract/editor");
    ExtractEditorItemsLarge();
    ExtractEditorItemsSmall();
    ExtractEditorCursors();
}

void PrepareData::ExtractCheatPuzzle() {
    logging::Info("Extracting puzzle...");
    PrepareSubDir("extract/puzzle");

    //read Cheat puzzle
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\puzzle.dat
    //Unknown format 	RNC-compressed = No 	Cheat puzzle  112x96

    //is not RNC compressed, we can skip this step

    //scale puzzle by factor 4
    //locate the needed input data file
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("puzzle.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file puzzle.dat");
    }
    ConvertRawImageData(inputDatFile.c_str(), 112, 96, "extract/puzzle/puzzle.png", 4);
}

void PrepareData::ExtractModels() {
    logging::Info("Extracting models...");
    PrepareSubDir("extract/models");
    ExtractModelTextures();
    Extra3DModels();
}

void PrepareData::ExtractIntro() {
    logging::Info("Extracting intro...");
    PrepareSubDir("extract/intro");
    PrepareIntro();
}

void PrepareData::ExtractAudio() {
    logging::Info("Extracting sounds...");
    PrepareSubDir("extract/sound");
    ExtractSounds();

    logging::Info("Extracting music...");
    PrepareSubDir("extract/music");
    ExtractMusic();
}

void PrepareData::ExtractUserdata() {
    //install other available assets user has copied
    //into folder userData from another source
    logging::Info("Extracting other stuff...");
    AddOtherLevelsHiOctaneTools();
}

void PrepareData::Extra3DModels() {
    ExtractNamed3DModel("barel0-", 3);
    ExtractNamed3DModel("bike0-", 8);
    ExtractNamed3DModel("car0-", 8);
    ExtractNamed3DModel("cone0-", 2);
    ExtractNamed3DModel("jet0-", 8);
    ExtractNamed3DModel("jugga0-", 8);
    ExtractNamed3DModel("marsh0-", 1);
    ExtractNamed3DModel("recov0-", 1);
    ExtractNamed3DModel("sign0-", 1);
    ExtractNamed3DModel("skim0-", 8);
    ExtractNamed3DModel("tank0-", 8);

    //if we have the extended version of the game we also need
    //to extract 3 more race track 3D models for the menue
    if (!mInfra->mExtendedGame) {
        ExtractNamed3DModel("track0-", 6);
    } else {
        ExtractNamed3DModel("track0-", 9);
    }
}

void PrepareData::ExtractNamed3DModel(const char* name, int n_models) {
    std::string obj_path = "extract/models/";
    irr::io::path inputDatFile;

    for (int idx = 0; idx < n_models; idx++) {
        std::string objname = std::string(name) + std::to_string(idx);
        std::string datfile = objname + ".dat";
        std::string objfile = obj_path + objname + ".obj";

        inputDatFile = mInfra->LocateFileInFileList(mInfra->mOriginalGame->objectsFolder,
                                             irr::core::string<fschar_t>(datfile.c_str()));

        if (inputDatFile.empty()) {
            //dat file not found!
             throw "Could not locate the original games object data file " + datfile;
        }

        Extract3DModel(inputDatFile.c_str(), objfile.c_str(), objname.c_str());
    }
}

void PrepareData::Extract3DModel(const char* srcFilename, const char* destFilename, const char* objName) {
    logging::Detail(std::string("Extracting 3D model \"") + objName + "\": " + srcFilename + " -> " + destFilename);

    if (this->modelsTabFileInfo == NULL) {
        throw std::string("Error: Model texture atlas not loaded");
    }

    ObjectDatFile* newConversion = new ObjectDatFile(this->modelsTabFileInfo, (unsigned int)(this->modelTexAtlasSize.Width),
                       (unsigned int)(this->modelTexAtlasSize.Height));

    if (!newConversion->LoadObjectDatFile(srcFilename)) {
        delete newConversion;
        throw "Error loading object file: " + std::string(srcFilename);
    }

    if (!newConversion->WriteToObjFile(destFilename, objName)) {
        delete  newConversion;
        throw "Error writing object file: " + std::string(destFilename);
    }

    delete newConversion;
}

//extracts the SVGA game logo data in data\logo0-1.dat and data\logo0-1.tab
//raises an error message in case of unexpected error
void PrepareData::ExtractGameLogoSVGA() {
    //read game logo in SVGA
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\logo0-1.dat
    //data\logo0-1.tab
    //Unknown format 	RNC-compressed = Yes 	Game logo (SVGA)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("logo0-1.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file logo0-1.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("logo0-1.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file logo0-1.tab");
    }
    ExtractCompressedImagesFromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), "extract/images/logo0-1-");
}

//extracts the SVGA HUD for 1 Player in data\panel0-1.dat and data\panel0-1.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractHUD1PlayerSVGA() {
    //read HUD SVGA for 1 player
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\panel0-1.dat
    //data\panel0-1.tab
    //Unknown format 	RNC-compressed = No 	HUD 1-Player (SVGA)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("panel0-1.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file panel0-1.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("panel0-1.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file panel0-1.tab");
    }
    ExtractImagesfromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), palette, "extract/hud1player/panel0-1-");
}

//extracts the SVGA Minimaps in data\track0-1.dat and data\track0-1.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractMiniMapsSVGA() {
    //data\track0-1.dat
    //data\track0-1.tab
    //Unknown format 	RNC-compressed = No 	MiniMaps (for the non-extended version of the game!)
    //Unknown format 	RNC-compressed = Yes! 	MiniMaps (for the extended version of the game!)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("track0-1.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file track0-1.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("track0-1.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file track0-1.tab");
    }

    //Important note: For the extended version of the game the minimap data file is RNC-compressed!
    //if we do not do this right, then the minimap extraction will fail
    if (mInfra->mExtendedGame) {
        UnpackDataFile(inputDatFile.c_str(), "extract/tmp-unpacked.dat");

        ExtractImagesfromDataFile("extract/tmp-unpacked.dat", inputTabFile.c_str(), palette, "extract/minimaps/track0-1-");

        remove("extract/tmp-unpacked.dat");
    } else {
        //non extended game version does not use RNC compression!
        ExtractImagesfromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), palette, "extract/minimaps/track0-1-");
    }
}

class MinimapStitcher {
public:
    MinimapStitcher(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver) {
        this->device = device;
        this->driver = driver;
        this->images = {};
    }

    ~MinimapStitcher() {
        for (auto image : images) {
            image->drop();
        }
    }

    void insert_image(irr::u32 x, irr::u32 y, const char* filename) {
        irr::io::IReadFile *file = device->getFileSystem()->createAndOpenFile(filename);
        irr::video::IImage* image = driver->createImageFromFile(file);
        file->drop();
        images.push_back(image);
        x_offsets.push_back(x);
        y_offsets.push_back(y);
    }

    void probe_transparent(const int img, int x, int y) {
        transparent = images[img]->getPixel(x, y);
    }

    irr::core::dimension2d<irr::u32> compute_resulting_dimension() {
        irr::u32 xmin = -1;
        irr::u32 ymin = -1;
        irr::u32 xmax = 0;
        irr::u32 ymax = 0;
        for (int i = 0; i < images.size(); i++) {
            irr::core::dimension2d<irr::u32> dim = images[i]->getDimension();
            xmin = std::min(xmin, x_offsets[i]);
            ymin = std::min(ymin, y_offsets[i]);
            xmax = std::max(xmax, x_offsets[i] + dim.Width);
            ymax = std::max(ymax, y_offsets[i] + dim.Height);
        }
        return irr::core::dimension2d(xmax - xmin, ymax - ymin);
    }

    void finalize(const char *filename) {
        auto size = compute_resulting_dimension();
        irr::video::IImage* imgNew = driver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, size);
        imgNew->fill(transparent);

        for (int i = 0; i < images.size(); i++) {
            images[i]->copyTo(imgNew, irr::core::vector2d<irr::s32>(x_offsets[i], y_offsets[i]));
        }

        irr::io::IWriteFile* outputPic = device->getFileSystem()->createAndWriteFile(filename, false);
        driver->writeImageToFile(imgNew, outputPic);
        outputPic->drop();
        imgNew->drop();
    }

private:
    irr::video::IVideoDriver* driver;
    irr::IrrlichtDevice* device;
    std::vector<irr::video::IImage*> images;
    std::vector<irr::u32> x_offsets;
    std::vector<irr::u32> y_offsets;
    irr::video::SColor transparent;
};

void PrepareData::StitchMiniMaps() {
    MinimapStitcher track1(mInfra->mDevice, mInfra->mDriver);
    track1.insert_image(0, 0, "extract/minimaps/track0-1-0000.bmp");
    track1.insert_image(6, 120, "extract/minimaps/track0-1-0001.bmp");
    track1.probe_transparent(0, 0, 0);
    track1.finalize("extract/minimaps/track0-1.png");

    MinimapStitcher track2(mInfra->mDevice, mInfra->mDriver);
    track2.insert_image(0, 0, "extract/minimaps/track0-1-0002.bmp");
    track2.insert_image(120, 5, "extract/minimaps/track0-1-0003.bmp");
    track2.insert_image(74, 120, "extract/minimaps/track0-1-0004.bmp");
    track2.probe_transparent(0, 0, 0);
    track2.finalize("extract/minimaps/track0-2.png");

    // track3 minimap is fully contained in file track0-1-0005.bmp -> no need for stitching

    MinimapStitcher track4(mInfra->mDevice, mInfra->mDriver);
    track4.insert_image(0, 0, "extract/minimaps/track0-1-0006.bmp");
    track4.insert_image(27, 120, "extract/minimaps/track0-1-0007.bmp");
    track4.probe_transparent(0, 0, 0);
    track4.finalize("extract/minimaps/track0-4.png");

    MinimapStitcher track5(mInfra->mDevice, mInfra->mDriver);
    track5.insert_image(0, 0, "extract/minimaps/track0-1-0008.bmp");
    track5.insert_image(18, 120, "extract/minimaps/track0-1-0009.bmp");
    track5.probe_transparent(0, 0, 0);
    track5.finalize("extract/minimaps/track0-5.png");

    // track6 minimap is fully contained in file track0-1-0010.bmp -> no need for stitching

    //if we have the extended game version, we also need to stich the remaining
    //maps as well
    if (mInfra->mExtendedGame) {
        MinimapStitcher track7(mInfra->mDevice, mInfra->mDriver);
        track7.insert_image(0, 0, "extract/minimaps/track0-1-0011.bmp");
        track7.insert_image(6, 126, "extract/minimaps/track0-1-0012.bmp");
        track7.probe_transparent(0, 0, 0);
        track7.finalize("extract/minimaps/track0-7.png");

        // track8 minimap is fully contained in file track0-1-0013.bmp -> no need for stitching
        // track9 minimap is fully contained in file track0-1-0014.bmp -> no need for stitching
    }
}

//extracts the SVGA HUD for 2 Players in data\panel0-0.dat and data\panel0-0.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractHUD2PlayersSVGA() {
    //read HUD SVGA for 2 players
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\panel0-0.dat
    //data\panel0-0.tab
    //Unknown format 	RNC-compressed = No 	HUD 2-Player (SVGA)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("panel0-0.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file panel0-0.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("panel0-0.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file panel0-0.tab");
    }
    ExtractImagesfromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), palette, "extract/hud2player/panel0-0-");
}

//extracts the SVGA Large Green font in data\pfont0-1.dat and data\pfont0-1.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractLargeGreenFontSVGA() {
    //read Large Green Font SVGA
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\pfont0-1.dat
    //data\pfont0-1.tab
    //Unknown format 	RNC-compressed = No 	Large Green Font (SVGA)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("pfont0-1.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file pfont0-1.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("pfont0-1.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file pfont0-1.tab");
    }
    ExtractImagesfromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), palette, "extract/fonts/largegreen/pfont0-1-");
}

//extracts the SVGA Large white font data in data\olfnt0-1.dat and data\olfnt0-1.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractLargeFontSVGA() {
    //read Large Fonts SVGA
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\olfnt0-1.dat
    //data\olfnt0-1.tab
    //Unknown format 	RNC-compressed = Yes 	Large white font (SVGA)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("olfnt0-1.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file olfnt0-1.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("olfnt0-1.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file olfnt0-1.tab");
    }
    ExtractCompressedImagesFromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), "extract/fonts/large/olfnt0-1-");
}

//extracts the SVGA Small white font data in data\osfnt0-1.dat and data\osfnt0-1.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractSmallFontSVGA() {
    //read Small Fonts SVGA
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\osfnt0-1.dat
    //data\osfnt0-1.tab
    //Unknown format 	RNC-compressed = Yes 	Small white font (SVGA)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("osfnt0-1.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file osfnt0-1.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("osfnt0-1.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file osfnt0-1.tab");
    }
    ExtractCompressedImagesFromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), "extract/fonts/smallsvga/osfnt0-1-");
}

//Takes an image, and replaces one specified color with another specified color
//Parameters:
//  character = pointer to the image that should be modified
//  originalColor = color that should be replaced
//  newColor = new color for replacement of selected pixels
//In case of an unexpected error this function returns false, True otherwise
bool PrepareData::ReplacePixelColor(irr::video::IImage* image,
             irr::video::SColor originalColor, irr::video::SColor newColor) {

 //we need to know the used pixel color format
  //irr::video::ECOLOR_FORMAT format = image->getColorFormat();

  //we can only handle this format right now
  //if(irr::video::ECF_A8R8G8B8 == format)
  //  {
        irr::video::SColor texel;
        //lock texture for reading and writing of pixel data
        irr::u8* datapntr = (irr::u8*)image->lock();
        //irr::u32 pitch = image->getPitch();

        irr::core::position2d<irr::u32> imagSize = image->getDimension();

        //iterate through all pixels of the image
        for (irr::u32 x = 0; x < imagSize.X; x++) {
            for (irr::u32 y = 0; y < imagSize.Y; y++) {
                //texel is the pixel color at position x and y

                //irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                irr::video::SColor texel = image->getPixel(x, y);
                if (texel == originalColor) {
                    //we found the original Color, replace it with
                    //new color
                    //*texel = newColor;
                    image->setPixel(x,y, newColor);
                }
            }
         }

        //unlock texture again!
        image->unlock();

        return true;
  /*} else {
      //unsupported pixel color format!
      //do not modify image and just return false
      return false;
  }*/
}

//Takes an image, and returns the detected font character color
//First detected pixel color other then transparent color is returned
//Parameters:
//  character = pointer to the image of the character that should be investigated
//  detectedColor = pointer to SCOLOR with detected color
//In case of an unexpected error this function returns false, True otherwise
bool PrepareData::DetectFontCharacterColor(irr::video::IImage* image, irr::video::SColor* detectedColor) {

 //we need to know the used pixel color format
  //irr::video::ECOLOR_FORMAT format = image->getColorFormat();

  //we can only handle this format right now
  /*if(irr::video::ECF_A8R8G8B8 == format)
    {*/
        //lock texture for only reading of pixel data
        irr::u8* datapntr = (irr::u8*)image->lock();
        //irr::u32 pitch = image->getPitch();

        irr::video::SColor texel;

        irr::core::position2d<irr::u32> imagSize = image->getDimension();

        //all raw character pictures have the transparent color at the upper leftmost pixel (0,0)
        //get this value, as a reference where there is no pixel of the character itself
        //irr::video::SColor* texelTrans = (irr::video::SColor *)(datapntr + ((0 * pitch) + (0 * sizeof(irr::video::SColor))));
        irr::video::SColor texelTrans = image->getPixel(0,0);

        //iterate through all pixels of the image
        for (irr::u32 x = 0; x < imagSize.X; x++) {
            for (irr::u32 y = 0; y < imagSize.Y; y++) {
                //texel is the pixel color at position x and y

                //irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                texel = image->getPixel(x, y);

                if (texel != texelTrans) {
                    //we found the first pixel with non transparent color
                    //return this color
                    *detectedColor = texel;

                    //unlock image again!
                    image->unlock();

                    return true;
                }
            }
         }

        //unlock image again!
        //we did not find any color of font
        image->unlock();

        return true;
 /* } else {
      //unsupported pixel color format!
      //do not continue, just return with false
      return false;
  }*/
}

//this function takes an already existing font extracted before
//into single image files, duplicates this files, and changes the text character color
//for this new created image files
void PrepareData::CreateFontForUnselectedItemsInMenue(const char* sourceFntFileName, const char* destFntFileName,
          irr::u32 fileNameNumOffset, irr::u32 numberCharacters) {

    char finalpathSrc[70];
    char finalpathDest[70];
    char fname[70];
    irr::video::SColor detColor;

    //definition of new color for this font used in game menue
    irr::video::SColor newColor(255, 103,174,145);

    for (ulong idx = fileNameNumOffset; idx < (numberCharacters + fileNameNumOffset); idx++) {
        //build current filename
        strcpy(finalpathSrc, sourceFntFileName);
        strcpy(finalpathDest, destFntFileName);
        sprintf (fname, "%0*lu.bmp", 4, idx);
        strcat(finalpathSrc, fname);
        strcat(finalpathDest, fname);

        //now we have source and destination file names
        //load image with irrlicht
        irr::video::IImage* srcImg = mInfra->mDriver->createImageFromFile(finalpathSrc);

        irr::core::dimension2d<irr::u32> srcDim(srcImg->getDimension().Width, srcImg->getDimension().Height);

        if (srcImg == NULL)
            throw std::string("Error opening source image file: ") + finalpathSrc;

        //copy source image into new destination image
        irr::video::IImage* destImg =
                mInfra->mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(srcDim.Width, srcDim.Height));

        //copy pixel data
        srcImg->lock();
        destImg->lock();
        for (irr::u32 x = 0; x < srcDim.Width; x++) {
            for (irr::u32 y = 0; y < srcDim.Height; y++) {
                //copy pixel data
                destImg->setPixel(x, y, srcImg->getPixel(x, y));
            }
        }

        srcImg->unlock();
        srcImg->drop();

        destImg->unlock();

        //detect font color inside source image
        if (!this->DetectFontCharacterColor(destImg, &detColor)) {
            throw std::string("Error detecting font color in image file: ") + finalpathSrc;
        }

        if (!this->ReplacePixelColor(destImg, detColor, newColor)) {
            throw std::string("Error replacing pixel color in image file: ") + finalpathSrc;
        }

        //now we have the font character with new color in image variable
        //save the result into a new image file
        irr::io::IWriteFile* outputPic = mInfra->mDevice->getFileSystem()->createAndWriteFile(finalpathDest, false);
        mInfra->mDriver->writeImageToFile(destImg, outputPic);

        //close output file
        outputPic->drop();
        destImg->drop();
    }
}

//extracts the SVGA Loading Screen (is shown while game loads) in data\onet0-1.dat and data\onet0-1.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractLoadingScreenSVGA() {
    //read SVGA Loading Screen
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\onet0-1.dat
    //data\onet0-1.tab
    //Raw VGA image 	RNC-compressed = Yes 	Loading and selection screens
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("onet0-1.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file onet0-1.dat");
    }
    ConvertCompressedImageData(inputDatFile.c_str(), "extract/images/onet0-1.png", 640, 480, 1);
}

//extracts the SVGA Selection Screen (is the Main menue background picture) in data\oscr0-1.dat and data\oscr0-1.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractSelectionScreenSVGA() {
    //read SVGA Selection Screen
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\oscr0-1.dat
    //data\oscr0-1.tab
    //Raw VGA image 	RNC-compressed = Yes 	Loading and selection screens
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("oscr0-1.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file oscr0-1.dat");
    }
    ConvertCompressedImageData(inputDatFile.c_str(), "extract/images/oscr0-1.png", 640, 480, 1);
}

void PrepareData::ExtractSky(char skyNr) {
    //read all race track sky images
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //Raw VGA image 	RNC-compressed = Yes 	256x256 Sky images

    std::string packFile = std::string("sky0-") + skyNr + ".dat";
    std::string outputFile = std::string("extract/sky/sky0-") + skyNr + ".png";
    std::string modifiedFile = std::string("extract/sky/modsky0-") + skyNr + ".png";

    //locate the needed input file
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>(packFile.c_str()));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw "Could not locate the original games data file " + packFile;
    }

    ConvertCompressedImageData(inputDatFile.c_str(), outputFile.c_str(), 256, 256, 2);
    //create new modified sky image for easier usage
    ModifySkyImage(outputFile.c_str(), modifiedFile.c_str());
}

//This helper function modifies the original sky image so that we can use it easier
//in this project. The result is stored in another new image file
void PrepareData::ModifySkyImage(const char *origSkyFileName, const char* outputModifiedSkyFileName) {
    //first open original sky image again
    irr::io::IReadFile *file = mInfra->mDevice->getFileSystem()->createAndOpenFile(origSkyFileName);

    irr::video::IImage* origSky = mInfra->mDriver->createImageFromFile(file);

    //get original sky image dimension
    irr::core::dimension2d<irr::u32> origDimension = origSky->getDimension();

    //we want to add in the modified sky image three copies of the original sky next to each other
    //the height should not change
    //calculate new image dimensions

    //create the new empty image for the modified sky file
    irr::video::IImage* imgNew =
        mInfra->mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(3 * origDimension.Width, origDimension.Height - 1));

    //first change: take lower part of picture starting with line 148, and move it to the top of the new picture
    origSky->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                   irr::core::rect<irr::s32>(0, 148, origDimension.Width, origDimension.Height));

    //second change: take upper part of the picture until line 147 and add it to the modified new picture to the lower part
    origSky->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, (origDimension.Height - 148)),
                   irr::core::rect<irr::s32>(0, 0, origDimension.Width, 147));

    //now repeat this two more times, to add 2 copies next to it to fill the whole width of the new picture
    origSky->copyTo(imgNew, irr::core::vector2d<irr::s32>(origDimension.Width, 0),
                   irr::core::rect<irr::s32>(0, 148, origDimension.Width, origDimension.Height));

    origSky->copyTo(imgNew, irr::core::vector2d<irr::s32>(origDimension.Width, (origDimension.Height - 148)),
                  irr::core::rect<irr::s32>(0, 0, origDimension.Width, 147));

    origSky->copyTo(imgNew, irr::core::vector2d<irr::s32>(origDimension.Width * 2, 0),
                  irr::core::rect<irr::s32>(0, 148, origDimension.Width, origDimension.Height));

    origSky->copyTo(imgNew, irr::core::vector2d<irr::s32>(origDimension.Width * 2, (origDimension.Height - 148)),
                  irr::core::rect<irr::s32>(0, 0, origDimension.Width, 147));

    //create new file for writting
    irr::io::IWriteFile* outputPic = mInfra->mDevice->getFileSystem()->createAndWriteFile(outputModifiedSkyFileName, false);

    mInfra->mDriver->writeImageToFile(imgNew, outputPic);

    //close output file
    outputPic->drop();

    //close the original picture file
    file->drop();
}

//the original Terrain texture Atlas stored within the game has all tiles
//in one column in y-direction; We can not use this complete Atlas for rendering, as
//this would lead to awful flickering lines on the border between different terrain tiles
//Therefore we need to read all tiles and export them to singulated picture files, which we can later
//use to load the textures
void PrepareData::ExportTerrainTextures(const char* targetFile, const char* exportDir, const char* outputFileName) {
    //first open target image again
    irr::io::IReadFile *file = mInfra->mDevice->getFileSystem()->createAndOpenFile(targetFile);
    irr::video::IImage* origAtlas = mInfra->mDriver->createImageFromFile(targetFile);

    //get original atlas image dimension
    irr::core::dimension2d<irr::u32> origDimension = origAtlas->getDimension();

    //calculate overall number of tiles; Width of picture is tile size; Heigth of picture
    //divided by width equals the overall number of titles in the image
    int numberTiles = (origDimension.Height / origDimension.Width);

    int tileSize = origDimension.Width;

    //now export one tile after each other
    for (int tileIdx = 0; tileIdx < numberTiles; tileIdx++) {
        //create the new empty image for the next single tile
        irr::video::IImage* imgNew = mInfra->mDriver->createImage(
            irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
            irr::core::dimension2d<irr::u32>(origDimension.Width, origDimension.Width));

        //process all the tiles by copying them from their position in the atlas
        origAtlas->copyTo(imgNew, {0, 0},
            irr::core::rect<irr::s32>(0, tileIdx * tileSize, tileSize, (tileIdx + 1) * tileSize));

        std::stringstream fp;
        fp << exportDir << outputFileName << std::setw(4) << std::setfill('0') << tileIdx << ".png";
        std::string finalpath = fp.str();

        // write image to file
        irr::io::IWriteFile* outputPic = mInfra->mDevice->getFileSystem()->createAndWriteFile(finalpath.c_str(), false);
        mInfra->mDriver->writeImageToFile(imgNew, outputPic);
        outputPic->drop();
    }

    //close the original picture file
    file->drop();
}

//the Terrain texture Atlas from https://github.com/movAX13h/HiOctaneTools
//has a different layout then the extracted texture atlas directly from the game files
//therefore to be able to use the levels 7 up to 9 data from their project
//we have to reorganize the atlas again (split it in single picture files)
//Returns true in case of success, returns false in case of unexpected problem
void PrepareData::SplitHiOctaneToolsAtlas(char* targetFile, char* exportDir, char* outputFileName) {
    //first open target image again
    irr::io::IReadFile *file = mInfra->mDevice->getFileSystem()->createAndOpenFile(targetFile);
    irr::video::IImage* origAtlas = mInfra->mDriver->createImageFromFile(targetFile);

    //get HiOctaneTools atlas image dimension
    irr::core::dimension2d<irr::u32> origDimension = origAtlas->getDimension();

    //each tile is 64x64 pixels
    int tileSize = 64;

    //calculate overall number of tiles; each tile is tilePixelSize x tilePixelSize
    int numberTiles = (origDimension.Height / tileSize) * (origDimension.Width / tileSize);

    //create the target directory
    CreateDirectory(exportDir);

    int tileposx = 0;
    int tileposy = 0;

    //now export one tile after each other
    for (int tileIdx = 0; tileIdx < numberTiles; tileIdx++) {

        //create the new empty image for the next single tile
        irr::video::IImage* imgNew = mInfra->mDriver->createImage(
            irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
            irr::core::dimension2d<irr::u32>(tileSize, tileSize));

        //process all the tiles by copying them from their position in the atlas
        origAtlas->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                      irr::core::rect<irr::s32>(tileposx, tileposy, tileposx + tileSize, tileposy + tileSize));

        tileposx += tileSize;

        if ((irr::u32)(tileposx) >= origDimension.Width) {
            tileposx = 0;
            tileposy += tileSize;
        }

        std::stringstream fp;
        fp << exportDir << outputFileName << std::setw(4) << std::setfill('0') << tileIdx << ".png";
        std::string finalpath = fp.str();

        //create new file for writting
        irr::io::IWriteFile* outputPic = mInfra->mDevice->getFileSystem()->createAndWriteFile(finalpath.c_str(), false);
        mInfra->mDriver->writeImageToFile(imgNew, outputPic);
        outputPic->drop();
    }

    //close the original picture file
    file->drop();
}

void PrepareData::ExtractTerrainTexture(char levelNr) {
    //todo: Scale Tiles by factor of 2.0
    //read race track Terrain texture
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //Raw image 64×16384 	RNC-compressed = Yes 	64x64 Terrain Textures

    char texNr = levelNr - 1;  // looks like levels start with 1 but texture atlases start with 0

    std::string extract_dir = "extract/";

    std::string packfile("textu0-");
    std::string packfile2 = packfile + texNr + ".dat";

    std::string outputFile = extract_dir + "textu0-" + texNr + "-orig.png";
    std::string levelDir = extract_dir + "level0-" + levelNr + '/';
    PrepareSubDir(levelDir.c_str());

    //locate the needed input file
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>(packfile2.c_str()));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw "Could not locate the original games data file " + packfile2;
    }

    ConvertCompressedImageData(inputDatFile.c_str(), outputFile.c_str(), 64, 16384);
    //reorganize Terrain Atlas format to be Square
    //ReorganizeTerrainAtlas(&outputFile[0], &finalFile[0]);
    ExportTerrainTextures(outputFile.c_str(), levelDir.c_str(), "tex");
    remove(outputFile.c_str());
}

void PrepareData::AddOtherLevelsHiOctaneTools() {
     char outputDirName[50];
     char outputFile[50];
     char finalFile[50];
     char levelFile[50];

     strcpy(outputFile, "userdata/level0-7.png");
     strcpy(levelFile, "userdata/level0-7.dat");
     strcpy(finalFile, "/tex");

     //are level 7 files available in userdata folder?
     if ((FileExists(outputFile) == 1) && (FileExists(levelFile) == 1)) {
         //yes, install level 7 in extract folder
         strcpy(outputDirName, "extract/level0-7");
         SplitHiOctaneToolsAtlas(outputFile, outputDirName, finalFile);

         //copy level file to target dir
         strcpy(outputFile, "extract/level0-7/level0-7-unpacked.dat");
         copy_file(levelFile, outputFile);
     }

     strcpy(outputFile, "userdata/level0-8.png");
     strcpy(levelFile, "userdata/level0-8.dat");
     strcpy(finalFile, "/tex");

     //are level 8 files available in userdata folder?
     if ((FileExists(outputFile) == 1) && (FileExists(levelFile) == 1)) {
         //yes, install level 8 in extract folder
         strcpy(outputDirName, "extract/level0-8");
         SplitHiOctaneToolsAtlas(outputFile, outputDirName, finalFile);

         //copy level file to target dir
         strcpy(outputFile, "extract/level0-8/level0-8-unpacked.dat");
         copy_file(levelFile, outputFile);
     }

     strcpy(outputFile, "userdata/level0-9.png");
     strcpy(levelFile, "userdata/level0-9.dat");
     strcpy(finalFile, "/tex");

     //are level 9 files available in userdata folder?
     if ((FileExists(outputFile) == 1) && (FileExists(levelFile) == 1)) {
         //yes, install level 9 in extract folder
         strcpy(outputDirName, "extract/level0-9");
         SplitHiOctaneToolsAtlas(outputFile, outputDirName, finalFile);

         //copy level file to target dir
         strcpy(outputFile, "extract/level0-9/level0-9-unpacked.dat");
         copy_file(levelFile, outputFile);
     }
}

void PrepareData::ConvertRawImageData(const char* rawDataFilename, irr::u32 sizex, irr::u32 sizey,
                                      const char* outputFilename, int scaleFactor) {
    irr::video::IImage* img = loadRawImage(rawDataFilename, sizex, sizey);
    img = UpscaleImage(img, sizex, sizey, scaleFactor);
    saveIrrImage(outputFilename, img);
}

irr::video::IImage* PrepareData::UpscaleImage(irr::video::IImage *srcImg, irr::u32 sizex, irr::u32 sizey, int scaleFactor) {
    if (scaleFactor == 1) {
        return srcImg;
    }

    //create an empty image with upscaled dimension
    irr::video::IImage *upImg = mInfra->mDriver->createImage(
        irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
        irr::core::dimension2d<irr::u32>(sizex * scaleFactor, sizey * scaleFactor));

    //get the pointers to the raw image data
    uint32_t *imageDataUp = (uint32_t*)upImg->lock();
    uint32_t *imageData = (uint32_t*)srcImg->lock();

    xbrz::scale(scaleFactor, imageData, imageDataUp, sizex, sizey, xbrz::ColorFormat::ARGB, xbrz::ScalerCfg(), 0, sizey);

    //release the pointers, we do not need them anymore
    srcImg->unlock();
    upImg->unlock();

    return upImg;
}

void PrepareData::ConvertIntroFrame(unsigned char* ByteArray, flic::Colormap colorMap, irr::u32 sizex, irr::u32 sizey,
                                    char* outputFilename, int scaleFactor) {
     //create an empty image
     irr::video::IImage* img =
             mInfra->mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex, sizey));

    auto raw_buffer = (uint32_t*)img->lock();
    for (const unsigned char* src=ByteArray; src<ByteArray+sizex*sizey; src++) {
        flic::Color color = colorMap[*src];
        *raw_buffer++ = 0xFF000000 | (color.r << 16) | (color.g << 8) | color.b;
    }
    img->unlock();

    img = UpscaleImage(img, sizex, sizey, scaleFactor);
    saveIrrImage(outputFilename, img);
}

void PrepareData::ReadPaletteFile(char *palFile, unsigned char* paletteDataOut) {
    int retcode=read_palette_rgb(paletteDataOut,palFile,(uint16_t)(256));

    std::string msg = std::string();
    switch (retcode) {
        case 0:
            return;
        case 1:
            msg += "Cannot open PAL file: ";
            msg += palFile;
            throw msg;
        case 2:
            msg += "Cannot read 256 colors from PAL file: ";
            msg += palFile;
            throw msg;
        default:
            msg += "Unknown error reading PAL file: ";
            msg += palFile;
            throw msg;
    }

}

std::tuple<unsigned char, unsigned char, unsigned char> PrepareData::GetPaletteColor(unsigned char colorIndex) {
    int r = palette[colorIndex * 3];
    int g = palette[colorIndex * 3 + 1];
    int b = palette[colorIndex * 3 + 2];

    r = (r * 255) / 63;
    g = (g * 255) / 63;
    b = (b * 255) / 63;

    return {r, g, b};
}

void PrepareData::ConvertObjectTexture(char* rawDataFilename, char* outputFilename, int scaleFactor) {
    irr::u16 sizex = 256;
    irr::u16 sizey = 768;

    irr::video::IImage* img = loadRawImage(rawDataFilename, sizex, sizey);
    img = UpscaleImage(img, sizex, sizey, scaleFactor);
    saveIrrImage(outputFilename, img);
}

//the Terrain texture Atlas from https://github.com/movAX13h/HiOctaneTools
//has a different layout then the extracted texture atlas directly from the game files
//therefore to be able to use the levels 7 up to 9 data from their project
//we have to reorganize the atlas again (split it in single picture files)
//Returns true in case of success, returns false in case of unexpected problem
bool PrepareData::DebugSplitModelTextureAtlasAndWriteSingulatedPictures(
        char *atlasFileName, char* exportDir, char* outputFileName, TABFILE *tabf) {
    //first open target image again
    irr::io::IReadFile *file = mInfra->mDevice->getFileSystem()->createAndOpenFile(atlasFileName);

    irr::video::IImage* origAtlas = mInfra->mDriver->createImageFromFile(atlasFileName);

    //get original atlas image dimension
    irr::core::dimension2d<irr::u32> origDimension = origAtlas->getDimension();

    //size of each singulated picture in Atals is different, pictures and picture sizes
    //are stored within TABFILE input variable

    //calculate overall number of tiles; each tile is tilePixelSize x tilePixelSize
    int numberTiles = tabf->count;

    //now we need to extract all singulated files

    char finalpath[50];
    char fname[20];

    TABFILE_ITEM* itemPntr;
    unsigned int itemIdx = 0;
    unsigned int srcXLoc;
    unsigned int srcYLoc;

    //now export one picture defined in tab file after each other
    for (int tileIdx = 0; tileIdx < numberTiles; tileIdx++) {
        itemPntr = (TABFILE_ITEM*)(&tabf->items[itemIdx]);

        //create the new empty image for the next single tile
         irr::video::IImage* imgNew =
            mInfra->mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(itemPntr->width, itemPntr->height));

            //calculate start upper left corner of source rect in original Atlas, depending on Offset value for current picture
            srcYLoc = (itemPntr->offset / origDimension.Width);
            srcXLoc = itemPntr->offset - (srcYLoc * origDimension.Width);

            //process all the tiles by copying them to the new image at
            //the new position
            origAtlas->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                          irr::core::rect<irr::s32>(srcXLoc, srcYLoc, srcXLoc + itemPntr->width, srcYLoc + itemPntr->height));

            itemIdx++;

            /*
                 irr::video::IImage* imgUp;

                //should the picture be upscaled?
                if (scaleFactor != 1.0) {
                   //create an empty image with upscaled dimension
                   imgUp = myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex * scaleFactor, sizey * scaleFactor));

                   //get the pointers to the raw image data
                   uint32_t *imageDataUp = (uint32_t*)imgUp->lock();
                   uint32_t *imageData = (uint32_t*)img->lock();

                   xbrz::scale(scaleFactor, &imageData[0], &imageDataUp[0], sizex, sizey, xbrz::ColorFormat::ARGB, xbrz::ScalerCfg(), 0, sizey);

                   //release the pointers, we do not need them anymore
                   img->unlock();
                   imgUp->unlock();
                }*/

            strcpy(finalpath, exportDir);
            strcat(finalpath, outputFileName);
            sprintf (fname, "%0*d.png", 4, tileIdx);
            strcat(finalpath, fname);

            //create new file for writting
            irr::io::IWriteFile* outputPic = mInfra->mDevice->getFileSystem()->createAndWriteFile(finalpath, false);

            //if (scaleFactor == 1.0) {
                   //write original image data
                  mInfra->mDriver->writeImageToFile(imgNew, outputPic);
           // } else {
                    //write upscaled image data
                //   myDriver->writeImageToFile(imgUp, outputPic);
              //  }

            //close output file
            outputPic->drop();
    }

    //close the original picture file
    file->drop();

    return true;
}

bool PrepareData::ConvertTMapImageData(char* rawDataFilename, char* outputFilename, int scaleFactor) {
    FILE* iFile;

    iFile = fopen(rawDataFilename, "rb");
    fseek(iFile, 0L, SEEK_END);
    size_t size = ftell(iFile);
    fseek(iFile, 0L, SEEK_SET);

    irr::u16 unknown;
    irr::u16 sizex;
    irr::u16 sizey;

    if (iFile != NULL)
    {
        //read picture information
        fread(&unknown, sizeof(unknown), 1, iFile);
        fread(&sizex, sizeof(sizex), 1, iFile);
        fread(&sizey, sizeof(sizey), 1, iFile);
    } else return false;

    //6 bytes of the file is needed for picture information above
    if ((size - 6) != (sizex*sizey)) {
        fclose(iFile);
        printf("\nError - Raw picture filesize does not fit with expectation! %s\n", rawDataFilename);
        return false;
    }

    fseek(iFile, 0L, SEEK_SET);

    size_t counter = 0;

    char* ByteArray;
    ByteArray = new char[size];
    if (iFile != NULL)
    {
        do {
            ByteArray[counter] = fgetc(iFile);
            counter++;
        } while (counter < size);
        fclose(iFile);
    } else {
        delete[] ByteArray;
        return false;
    }

    //create arrays for color information
     unsigned char *arrR=static_cast<unsigned char*>(malloc(size - 6));
     unsigned char *arrG=static_cast<unsigned char*>(malloc(size - 6));
     unsigned char *arrB=static_cast<unsigned char*>(malloc(size - 6));

     double arrIntR;
     double arrIntG;
     double arrIntB;
     unsigned char color;

     //use palette to derive RGB information for all pixels
     //loaded palette has 6 bits per color
     //raw pictue data starts at byte #7
     for (counter = 6; counter < size; counter++) {
         color = ByteArray[counter];
         arrIntR = palette[color * 3    ];
         arrIntG = palette[color * 3 + 1 ];
         arrIntB = palette[color * 3 + 2 ];

         arrIntR = (arrIntR * 255.0) / 63.0;
         arrIntG = (arrIntG * 255.0) / 63.0;
         arrIntB = (arrIntB * 255.0) / 63.0;

         arrR[counter - 6] = (unsigned char)arrIntR;
         arrG[counter - 6] = (unsigned char)arrIntG;
         arrB[counter - 6] = (unsigned char)arrIntB;
     }

     //create an empty image
     irr::video::IImage* img =
             mInfra->mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex, sizey));

     //draw the image
     for (irr::u32 posx = 0; posx < sizex; posx++) {
         for (irr::u32 posy = 0; posy < sizey; posy++) {
             img->setPixel(posx, posy,  irr::video::SColor(255, arrR[posy * sizex + posx],
                           arrG[posy * sizex + posx], arrB[posy * sizex + posx]));
         }
     }

     irr::video::IImage* imgUp;

     //should the picture be upscaled?
     if (scaleFactor != 1.0) {
          //create an empty image with upscaled dimension
         imgUp = mInfra->mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex * scaleFactor, sizey * scaleFactor));

         //get the pointers to the raw image data
         uint32_t *imageDataUp = (uint32_t*)imgUp->lock();
         uint32_t *imageData = (uint32_t*)img->lock();

         xbrz::scale(scaleFactor, &imageData[0], &imageDataUp[0], sizex, sizey, xbrz::ColorFormat::ARGB, xbrz::ScalerCfg(), 0, sizey);

         //release the pointers, we do not need them anymore
         img->unlock();
         imgUp->unlock();
       }

     //create new file for writting
     irr::io::IWriteFile* outputPic = mInfra->mDevice->getFileSystem()->createAndWriteFile(outputFilename, false);

     //write image to file
     if (scaleFactor == 1.0) {
           //write original image data
           mInfra->mDriver->writeImageToFile(img, outputPic);
        } else {
            //write upscaled image data
           mInfra->mDriver->writeImageToFile(imgUp, outputPic);
     }

     //close output file
     outputPic->drop();

    delete[] ByteArray;
    free(arrR);
    free(arrG);
    free(arrB);

    return true;
}

//extracts the introductory screen (in format 320x200) in data\title.dat and data\title.tab
//* is from 0 up to 5
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractIntroductoryScreen() {
    //read introductory screen
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //Raw image 320×200 	RNC-compressed = Yes 	320x200 Introductory screen
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("title.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file title.dat");
    }
    ConvertCompressedImageData(inputDatFile.c_str(), "extract/images/title.png", 320, 200, 2);
}

//extracts the SVGA thin white font data in data\hfont0-0.dat and data\hfont0-0.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractThinWhiteFontSVGA() {
    //read thin white font SVGA
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\hfont0-0.dat
    //data\hfont0-0.tab
    //Unknown format 	RNC-compressed = Yes 	Thin white font (SVGA) (SVGA)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("hfont0-0.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file hfont0-0.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("hfont0-0.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file hfont0-0.tab");
    }
    ExtractCompressedImagesFromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), "extract/fonts/thinwhite/hfont0-0-");
}

//extracts the Editor cursors data in data\point0-0.dat and data\point0-0.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractEditorCursors() {
    //read Editor cursors
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\point0-0.dat
    //data\point0-0.tab
    //Unknown format 	RNC-compressed = Yes 	Editor cursors
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("point0-0.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file point0-0.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("point0-0.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file point0-0.tab");
    }
    ExtractCompressedImagesFromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), "extract/editor/point0-0-");
}

//extracts the Editor items large in data\hspr0-0.dat and data\hspr0-0.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractEditorItemsLarge() {
    //read Editor items large
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\hspr0-0.dat
    //data\hspr0-0.tab
    //Unknown format 	RNC-compressed = Yes 	Editor icons (large)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("hspr0-0.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file hspr0-0.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("hspr0-0.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file hspr0-0.tab");
    }
    ExtractCompressedImagesFromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), "extract/editor/hspr0-0-");
}

//extracts the Editor items small in data\mspr0-0.dat and data\mspr0-0.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractEditorItemsSmall() {
    //read Editor items small
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\mspr0-0.dat
    //data\mspr0-0.tab
    //Unknown format 	RNC-compressed = Yes 	Editor icons (small)
    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("mspr0-0.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file mspr0-0.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("mspr0-0.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games data file mspr0-0.tab");
    }
    ExtractCompressedImagesFromDataFile(inputDatFile.c_str(), inputTabFile.c_str(), "extract/editor/mspr0-0-");
}

//extracts the Ingame Textures Atlas in data\tex0-0.dat and data\tex0-0.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractModelTextures() {
    //read Ingame textures atlas tex0-0
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\tex0-0.dat
    //data\tex0-0.tab
    //Is a Raw image file RNC-compressed = No 	In game textures 256x768

    //this is the defined size of the
    //texture Atlas in the Game
    modelTexAtlasSize.Width = 256;
    modelTexAtlasSize.Height = 768;

    //locate the needed input files
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->objectsFolder, irr::core::string<fschar_t>("tex0-0.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games object data file tex0-0.dat");
    }

    irr::io::path inputTabFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->objectsFolder, irr::core::string<fschar_t>("tex0-0.tab"));

    if (inputTabFile.empty()) {
        //tab file not found!
         throw std::string("Could not locate the original games object data file tex0-0.tab");
    }

    char outputfile[50];
    strcpy(outputfile, "extract/models/tex0-0.png");

    ConvertRawImageData(inputDatFile.c_str(),
                        (irr::u32)(modelTexAtlasSize.Width),
                        (irr::u32)(modelTexAtlasSize.Height), outputfile, 1);

    /********************************************
    * We also need the TAB file to know where  *
    * the pictures are located inside this     *
    * texture Atlas                            *
    ********************************************/

    this->modelsTabFileInfo = new TABFILE();

    //Very important: Do not remove the last boolean false parameter; For the
    //3D Model texture Atlas we HAVE to read the first TAB file entry as well,
    //because otherwise we are missing the first texture Atlas picture, and the
    //3D model texture mapping goes wrong
    int retcode=read_tabfile_data(this->modelsTabFileInfo, inputTabFile.c_str(), false);
    if (retcode!=0)
    switch (retcode) {
        case 1:
            throw std::string("Cannot open TAB file: ") + inputTabFile.c_str();
        default:
            throw std::string("Error - Loading TAB file ") + inputTabFile.c_str() + " returned fail code " + std::to_string(retcode);
    }

    /*char exportDir[50];
    char exportFileName[25];
    strcpy(exportDir, "extract/models/");
    strcpy(exportFileName, "dbgtex-");

    //the next debugging command allows to write a CSV file
    //which contains the listing of all contained pictures described
    //in the TAB file. For each picture the offset, width and height is
    //shown
    DebugWriteTabFileContentsCsvTable(inputTabFile.c_str(), this->modelsTabFileInfo);

    //the next line allows to seperate the original texture Atlas
    //into seperate PNG images on the harddisc. This could be interesting
    //for debugging purposes.
    DebugSplitModelTextureAtlasAndWriteSingulatedPictures(outputfile, exportDir, exportFileName, this->modelsTabFileInfo);*/
}

//extracts the Tmaps data in data\tmaps.dat and data\tmaps.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractTmaps() {
    //read Tmaps data (contains collectible items, powerups...)
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\tmaps.dat
    //data\tmaps.tab
    //RNC-compressed = Yes 	Tmaps data (contains collectible items, powerups...)

    //Information on https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //Many of the game's files are compressed using the Rob Northern Compression format;
    //some of them such as SOUND\SOUND.DAT are a concatenation of many RNC archives simply glued together.

    //locate the needed input file
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("tmaps.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games object data file tmaps.dat");
    }

    //first seperate data in multiple export files, new file always starts with "RNC" magic characters
    std::vector<unsigned char> ByteArray = loadRawFile(inputDatFile.c_str());

    //now seperate data into different new created file
    size_t counter = 0;
    unsigned long ofilenr = 0;
    FILE* oFile = NULL;

    while (counter < ByteArray.size()) {
        if (((ByteArray.size() - counter) > 4) && (ByteArray[counter] == 'R') && (ByteArray[counter+1] == 'N')
            && (ByteArray[counter+2] == 'C') && (ByteArray[counter+3] == 1))
        {
            //new file start found

            char finalpath[50];
            char fname[20];
            strcpy(finalpath, "extract/sprites/tmaps-packed");
            sprintf (fname, "%0*lu.dat", 4, ofilenr);
            strcat(finalpath, fname);
            ofilenr++;

            if (oFile != NULL) {
                //close last file
                fclose(oFile);
            }

            oFile = fopen(finalpath, "wb");
            if (oFile == NULL) {
                throw std::string("Cannot open file for writing: ") + finalpath;
            }
        }
        if (oFile != NULL) {
            fwrite(&ByteArray[counter], 1, 1, oFile);
        }
        counter++;
   }

   //close last file
   fclose(oFile);

   //RNC unpack all the sprite files
   unsigned long filecnt;

   for (filecnt = 0; filecnt < ofilenr; filecnt++) {
       char finalpath[50];
       char finalpathUnpacked[50];
       char fname[20];
       strcpy(finalpath, "extract/sprites/tmaps-packed");
       strcpy(finalpathUnpacked, "extract/sprites/tmaps-unpacked");
       sprintf (fname, "%0*lu.dat", 4, filecnt);
       strcat(finalpath, fname);
       strcat(finalpathUnpacked, fname);

       UnpackDataFile(finalpath, finalpathUnpacked);
   }

   //export raw video data to png pictures
    for (filecnt = 0; filecnt < ofilenr; filecnt++) {
        char finalpath[50];
        char finalpathUnpacked[50];
        char fname[20];
        char fname2[20];
        strcpy(finalpath, "extract/sprites/tmaps");
        strcpy(finalpathUnpacked, "extract/sprites/tmaps-unpacked");
        sprintf (fname, "%0*lu.dat", 4, filecnt);
        sprintf (fname2, "%0*lu.png", 4, filecnt);
        strcat(finalpath, fname2);
        strcat(finalpathUnpacked, fname);

        //upscale Tmaps by a factor of 2
        ConvertTMapImageData(finalpathUnpacked, finalpath, 2);
    }

    //cleanup unnecessary files
    for (filecnt = 0; filecnt < ofilenr; filecnt++) {
        char finalpath[50];
        char finalpathUnpacked[50];
        char fname[20];
        char fname2[20];
        strcpy(finalpath, "extract/sprites/tmaps-packed");
        strcpy(finalpathUnpacked, "extract/sprites/tmaps-unpacked");
        sprintf (fname, "%0*lu.dat", 4, filecnt);
        sprintf (fname2, "%0*lu.dat", 4, filecnt);
        strcat(finalpath, fname2);
        strcat(finalpathUnpacked, fname);

        remove(finalpathUnpacked);
        remove(finalpath);
    }
}

void PrepareData::ExtractSounds() {
    //Information on https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //Many of the game's files are compressed using the Rob Northern Compression format;
    //some of them such as SOUND\SOUND.DAT are a concatenation of many RNC archives simply glued together.

    //locate the needed input file
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->soundFolder, irr::core::string<fschar_t>("sound.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games sound data file sound.dat");
    }

    std::vector<unsigned char> ByteArray = loadRawFile(inputDatFile.c_str());

    //first seperate data in multiple export files, new file always starts with "RNC" magic characters
    size_t counter = 0;
    unsigned long ofilenr = 0;
    FILE* oFile = NULL;

    while (counter < ByteArray.size()) {
        if (((ByteArray.size() - counter) > 4) && (ByteArray[counter] == 'R') && (ByteArray[counter+1] == 'N')
           && (ByteArray[counter+2] == 'C') && (ByteArray[counter+3] == 1))
        {
            //new file start found
            char finalpath[50];
            char fname[20];
            strcpy(finalpath, "extract/sound/sound-packed");
            sprintf (fname, "%0*lu.dat", 4, ofilenr);
            strcat(finalpath, fname);
            ofilenr++;

            if (oFile != NULL) {
                //close previous file
                fclose(oFile);
            }

            oFile = fopen(finalpath, "wb");
            if (oFile == NULL) {
                throw std::string("Cannot open file for writing: ") + finalpath;
            }
        }

        fwrite(&ByteArray[counter], 1, 1, oFile);
        counter++;
    }

    //close last file
    fclose(oFile);

    //RNC unpack all the sound files
    unsigned long filecnt;

    for (filecnt = 0; filecnt < ofilenr; filecnt++) {
        char finalpath[50];
        char finalpathUnpacked[50];
        char fname[20];
        strcpy(finalpath, "extract/sound/sound-packed");
        strcpy(finalpathUnpacked, "extract/sound/sound-unpacked");
        sprintf (fname, "%0*lu.dat", 4, filecnt);
        strcat(finalpath, fname);
        strcat(finalpathUnpacked, fname);

        UnpackDataFile(finalpath, finalpathUnpacked);
    }

    //****************************
    // Sound package 1
    //****************************

    //read file infos for first sound package
    char infofilename1[50];
    strcpy(infofilename1, "extract/sound/sound-unpacked0001.dat");

    std::vector<SOUNDFILEENTRY> list;
    ReadSoundFileEntries(infofilename1, &list);

    char splitfilename1[50];
    strcpy(splitfilename1, "extract/sound/sound-unpacked0000.dat");
    char ofilename1[50];
    strcpy(ofilename1, "extract/sound/sound0-");

    SplitSoundFile(splitfilename1, ofilename1, &list);

    //****************************
    // Sound package 2
    //****************************

    strcpy(infofilename1, "extract/sound/sound-unpacked0003.dat");
    list.clear();

    ReadSoundFileEntries(infofilename1, &list);

    strcpy(splitfilename1, "extract/sound/sound-unpacked0002.dat");
    strcpy(ofilename1, "extract/sound/sound2-");

    SplitSoundFile(splitfilename1, ofilename1, &list);

    //****************************
    // Sound package 3
    //****************************

    strcpy(infofilename1, "extract/sound/sound-unpacked0005.dat");
    list.clear();

    ReadSoundFileEntries(infofilename1, &list);

    strcpy(splitfilename1, "extract/sound/sound-unpacked0004.dat");
    strcpy(ofilename1, "extract/sound/sound4-");

    SplitSoundFile(splitfilename1, ofilename1, &list);

    //****************************
    // Sound package 4
    //****************************
    /*
    strcpy(splitfilename1, "extract/sound/sound-unpacked0006.dat");
    strcpy(ofilename1, "extract/sound/sound6-");

    SplitSoundFile(splitfilename1, ofilename1);

    strcpy(splitfilename1, "extract/sound/sound-unpacked0007.dat");
    strcpy(ofilename1, "extract/sound/sound7-");

    SplitSoundFile(splitfilename1, ofilename1);*/

    //cleanup unnecessary files
    for (filecnt = 0; filecnt < ofilenr; filecnt++) {
        char finalpath[50];
        char finalpathUnpacked[50];
        char fname[20];
        strcpy(finalpath, "extract/sound/sound-packed");
        strcpy(finalpathUnpacked, "extract/sound/sound-unpacked");
        sprintf (fname, "%0*lu.dat", 4, filecnt);
        strcat(finalpath, fname);
        strcat(finalpathUnpacked, fname);

        remove(finalpath);
        remove(finalpathUnpacked);
    }
}

uint32_t PrepareData::read_uint32_le_file (FILE *fp)
{
    uint32_t l = 0;
    l = fgetc (fp);
    l += fgetc (fp)<<8;
    l += fgetc (fp)<<16;
    l += fgetc (fp)<<24;
    return l;
}

void PrepareData::ReadSoundFileEntry(FILE* inputFile, SOUNDFILEENTRY* entry) {
    fread(entry->soundFilename, sizeof(entry->soundFilename), 1, inputFile);
    fread(entry->padding1, sizeof (entry->padding1), 1, inputFile);
    entry->offsetTune = read_uint32_le_file(inputFile);
    fread(entry->padding2, sizeof (entry->padding2), 1, inputFile);
    entry->tuneLenBytes = read_uint32_le_file(inputFile);
    entry->unknown = static_cast<int16_t>(read_short_le_file(inputFile));
}

//Reads file format with sound file information, Returns all available SOUNDFILEENTRIES in entries
//Returns true when successful, False otherwise
void PrepareData::ReadSoundFileEntries(const char* filename, std::vector<SOUNDFILEENTRY> *entries) {
    //Information on https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //see under MUSIC.DAT, structure seems to also apply to sound files

    FILE* iFile;
    unsigned int EntryNumber = 0;

    iFile = fopen(filename, "rb");
    fseek(iFile, 0L, SEEK_END);
    long size = ftell(iFile);
    fseek(iFile, 0L, SEEK_SET);

    //calculate amount of items to read from file
    //each SOUNDFILEENTRY has overall 32 bytes
    unsigned long itemnr = size / 32;

    SOUNDFILEENTRY *newItem;

    if (iFile == NULL)
    {
        entries = NULL;
        throw std::string("Error - Cannot open file for reading: ") + filename;
    }
    do {
        newItem = (SOUNDFILEENTRY*)malloc(sizeof(SOUNDFILEENTRY));
        ReadSoundFileEntry(iFile, newItem);
        entries->push_back(*newItem);
        free(newItem);
        EntryNumber++;
    } while (EntryNumber < itemnr);
    fclose(iFile);
}

//further split sound files
void PrepareData::SplitSoundFile(const char* filename, const char *ofilename, std::vector<SOUNDFILEENTRY> *entries) {
    //after ExtractSounds routine executed we get files which contain multiple WAV files
    //glued together, we need to split them further
    std::vector<unsigned char> ByteArray = loadRawFile(filename);

    //first seperate data in multiple export files, new file always starts with "RIFF" magic characters
    FILE* oFile = NULL;
    size_t counter = 0;

    std::vector<SOUNDFILEENTRY>::iterator it;
    unsigned long lastpos;

    //go through all files
   for(it = entries->begin(); it != entries->end(); ++it) {
       //skip the empty files with name "NULL.WAV" and ""
          if ((strcmp((*it).soundFilename, "NULL.WAV") != 0) && (strcmp((*it).soundFilename, "") != 0)) {
                char finalpath[50];
                strcpy(finalpath, ofilename);

                //next 2 outcommented lines if we simply want wave files
                //names with increasing number in them
                /*sprintf (fname, "%0*lu.wav", 4, ofilenr);
                strcat(finalpath, fname);*/

                //next two lines if we want wave files with official names in them
                strcat(finalpath, (*it).soundFilename);

                oFile = fopen(finalpath, "wb");
                if (oFile == NULL) {
                    throw std::string("Error - Cannot open file for writting: ") + finalpath;
                }

                lastpos = (*it).offsetTune + (*it).tuneLenBytes;

                for (counter = (*it).offsetTune; counter < lastpos; counter++) {
                    fwrite(&ByteArray[counter], 1, 1, oFile);
                }

                //close last file
                fclose(oFile);
            }
    }
}

void PrepareData::ExtractMusicFiles(const char* outputNameStr, FILE *iFile,
                                    MUSICTABLEENTRY *VecMusicTableEntries,
                                    std::vector<SOUNDFILEENTRY> VecTuneInformation)
{
    //now extract all the available song files
    std::vector<SOUNDFILEENTRY>::iterator it2;

    FILE *oFile = NULL;
    unsigned long dataCnt;

     for(it2 = VecTuneInformation.begin(); it2 != VecTuneInformation.end(); ++it2) {
         char finalpath[50];
         strcpy(finalpath, outputNameStr);
         strcat(finalpath, "/");
         strcat(finalpath, (*it2).soundFilename);

         oFile = fopen(finalpath, "wb");
         if (oFile == NULL) {
             throw std::string("Error - Cannot open file for writting: ") + finalpath;
         }

         //go to start of song file data
         fseek(iFile, VecMusicTableEntries->offTune1 + (*it2).offsetTune, SEEK_SET);

         //get length of this tune
         dataCnt = (*it2).tuneLenBytes;

         unsigned char *buffer = static_cast<unsigned char*>(malloc(dataCnt));

         if (buffer == NULL) {
             throw std::string("Error - cannot allocate ") + std::to_string(dataCnt) + " bytes of memory.";
         }

         //copy data
         fread(&buffer[0], dataCnt, 1, iFile);
         fwrite(&buffer[0], dataCnt, 1, oFile);

         //close current output file
         fclose(oFile);

         free(buffer);
     }
}

void PrepareData::ReadMusicFileEntry(FILE* inputFile, MUSICTABLEENTRY* entry) {
    entry->offTunes = read_uint32_le_file(inputFile);
    entry->offTune1 = read_uint32_le_file(inputFile);
    entry->unknown = read_uint32_le_file(inputFile);
    entry->AllTunesLenBytes = read_uint32_le_file(inputFile);
}

//Extract music files
//Returns true when successful, False otherwise
void PrepareData::ExtractMusic() {
    //Information on https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //see under MUSIC.DAT

    char outputNameStr[50];

    //locate the needed input file
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->soundFolder, irr::core::string<fschar_t>("music.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games sound data file music.dat");
    }

    strcpy(outputNameStr, "extract/music");

    FILE* iFile;

    iFile = fopen(inputDatFile.c_str(), "rb");
    fseek(iFile, 0L, SEEK_END);
    long size = ftell(iFile);

    //first we need to read INT32LE at EOF, and seek to
    //this position
    fseek(iFile, size - 4, SEEK_SET);

    // unsigned long is larger than the 4 bytes read. initializing it to 0 avoids
    // the upper bytes containing garbage.
    unsigned long seekPos = 0;

    fread(&seekPos, 4, 1, iFile);
    fseek(iFile, seekPos, SEEK_SET);

    short int readVal = 0;
    unsigned int N = 0;
    long lastSeekPos;

    //now at this location count the number of INT16LE with value 01;
    //lets call the amount of repetitions N
    do {
        lastSeekPos = ftell(iFile);
        fread(&readVal, 2, 1, iFile);
        if (readVal == 0x01)
            N++;
    } while (readVal == 0x01);

    if (N == 0)
        throw std::string("Error - No music data found");

    //go back to file position before last read
    fseek(iFile, lastSeekPos, SEEK_SET);

    std::vector<MUSICTABLEENTRY> VecMusicTableEntries;

    MUSICTABLEENTRY *newTableEntry;

    //now read N times the following struct with tune offset information
    for (unsigned int cnt = 0; cnt < N; cnt++) {
         newTableEntry = (MUSICTABLEENTRY*)malloc(sizeof(MUSICTABLEENTRY));
         if (newTableEntry == NULL) {
             fclose(iFile);
             throw std::string("ExtractMusic - Out of Memory");
         }

         ReadMusicFileEntry(iFile, newTableEntry);
         VecMusicTableEntries.push_back(*newTableEntry);
         free(newTableEntry);
    }

    //now use every of this tables, seek to the offTunes position, and read the next struct
    //with music tune information there; music tune information struct is identical
    //to the struct that was used for sound files as well
    std::vector<MUSICTABLEENTRY>::iterator it;

    std::vector<SOUNDFILEENTRY> VecTuneInformation;

    SOUNDFILEENTRY *newTuneInformation;
    unsigned long targetLenTuneSum;

    for(it = VecMusicTableEntries.begin(); it != VecMusicTableEntries.end(); ++it) {
        newTuneInformation = (SOUNDFILEENTRY*)malloc(sizeof(SOUNDFILEENTRY));

        if (newTuneInformation == NULL) {
            fclose(iFile);
            throw std::string("ExtractMusic - Out of Memory");
        }

        fseek(iFile, (it)->offTunes, SEEK_SET);
        ReadSoundFileEntry(iFile, newTuneInformation);

        //in newTuneInformation we received now a value for lenTune
        //now we continue reading this kind of struct, until the sum of all of
        //their lenTune fields is identical to the first read one before
        targetLenTuneSum = newTuneInformation->tuneLenBytes;
        free(newTuneInformation);

        while (targetLenTuneSum > 0) {
            newTuneInformation = (SOUNDFILEENTRY*)malloc(sizeof(SOUNDFILEENTRY));
            if (newTuneInformation == NULL) {
                fclose(iFile);
                throw std::string("ExtractMusic - Out of Memory");
            }

            ReadSoundFileEntry(iFile, newTuneInformation);
            VecTuneInformation.push_back(*newTuneInformation);
            targetLenTuneSum -= newTuneInformation->tuneLenBytes;
            free(newTuneInformation);
        }

        //extract all available tune files
        ExtractMusicFiles(outputNameStr, iFile, &(*it), VecTuneInformation);

        //clear list again to make room for next file round
        VecTuneInformation.clear();
     }

    fclose(iFile);
}

//The following routine uses the flifix source code,
//repairs the original games intro.dat file, and then
//extracts the intro in single frames that we can then
//load and play using Irrlicht
void PrepareData::PrepareIntro() {
    //Variables initiation
    FILE *animFile;
    FILE *destFile;
    //Frames are numbered from 0
    ulong startFrame=0;
    ulong endFrame=ULONG_MAX;
    //Options description is avaible in the file where they are defined
    int globOptions = 0;

    //locate the needed input file
    irr::io::path inputDatFile =
            mInfra->LocateFileInFileList(mInfra->mOriginalGame->dataFolder, irr::core::string<fschar_t>("intro.dat"));

    if (inputDatFile.empty()) {
        //dat file not found!
         throw std::string("Could not locate the original games data file intro.dat");
    }

    char *srcFName = strdup(inputDatFile.c_str());
    char *destFName = strdup("extract/intro/intro.fli");

    //we need this option to succesfully repair
    //the original games fli movie file
    globOptions |= poFixMainHeader;

    //Opening the files
    animFile = openSourceFLIFile(srcFName);
    if (animFile == NULL)
        throw std::string("Error opening file: ") + srcFName;

    destFile = openDestinationFLIFile(destFName);
    if (destFile == NULL) {
        fclose(animFile);
        throw std::string("Error opening file: ") + destFName;
    }

    //File analyst
    if (globOptions & poSimpleFix)
        processFLISimple(animFile,destFile,startFrame,endFrame, & globOptions);
    else
        processFLIFile(animFile,destFile,startFrame,endFrame, globOptions);

    //Closing files
    closeFLIFiles(animFile,destFile,globOptions);

    //now load the FLI file and create for each frame a texture file for later
    FILE* f = std::fopen("extract/intro/intro.fli", "rb");
    flic::StdioFileInterface file(f);
    flic::Decoder decoder(&file);
    flic::Header header;
    if (!decoder.readHeader(header)) {
        throw std::string("Error reading FLI header in file extract/intro/intro.fli");
    }

    std::vector<uint8_t> buffer(header.width * header.height);
    flic::Frame frame;
    frame.pixels = &buffer[0];
    frame.rowstride = header.width;

    char outFrameFileName[50];
    char fname[20];

    for (long i = 0; i < header.frames; ++i) {
    if (!decoder.readFrame(frame)) {
        throw std::string("Error reading frame ") + std::to_string(i) + " in file extract/intro/intro.fli";
    }
       //process the current decoded frame data in buffer

       //create the filename for the picture output file
       strcpy(outFrameFileName, "extract/intro/frame");
       sprintf (fname, "%0*lu.png", 4, i);
       strcat(outFrameFileName, fname);

       //original frame size is 320x200, scale with factor of 2 to get frames with 640 x 400
       ConvertIntroFrame(buffer.data(), frame.colormap, header.width, header.height, outFrameFileName, 2);
    }

    free(srcFName);
    free(destFName);
}

void PrepareData::ConvertCompressedImageData(const char* packfile, const char* outfile, irr::u32 sizex, irr::u32 sizey, int scaleFactor) {
    UnpackDataFile(packfile, "extract/tmp-unpacked.dat");

    // upscale original image data if necessary
    ConvertRawImageData("extract/tmp-unpacked.dat", sizex, sizey, outfile, scaleFactor);

    remove("extract/tmp-unpacked.dat");
}

void PrepareData::ExtractCompressedImagesFromDataFile(const char* datFileName, const char* tabFileName, const char* outdir) {
    UnpackDataFile(datFileName, "extract/tmp-unpacked.dat");

    ExtractImagesfromDataFile(
        "extract/tmp-unpacked.dat",
        tabFileName,
        palette,
        outdir);

    remove("extract/tmp-unpacked.dat");
}

irr::video::IImage* PrepareData::loadRawImage(const char* rawDataFilename, irr::u32 sizex, irr::u32 sizey) {
    std::vector<unsigned char> ByteArray = loadRawFile(rawDataFilename);
    if (ByteArray.size() != sizex * sizey) {
        throw std::string("Error - Raw picture filesize does not fit with expectation! ") + rawDataFilename;
    }

    //create an empty image
    irr::video::IImage* img =
            mInfra->mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex, sizey));

    auto readbuf = ByteArray.data();
    auto raw_buffer = (uint32_t*)img->lock();
    for (const unsigned char* src=readbuf; src<readbuf+sizex*sizey; src++) {
        unsigned char r, g, b;
        std::tie(r, g, b) = GetPaletteColor(*src);
        *raw_buffer++ = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
    img->unlock();

    return img;
}

void PrepareData::saveIrrImage(const char* outputFilename, irr::video::IImage* img) {
    irr::io::IWriteFile* outputPic = mInfra->mDevice->getFileSystem()->createAndWriteFile(outputFilename, false);
    mInfra->mDriver->writeImageToFile(img, outputPic);
    outputPic->drop();
    img->drop();
}

// wrap ExtractImages to take const char* arguments and throw an exception on error
void ExtractImagesfromDataFile(const char* datfname, const char* tabfname, unsigned char* palette, const char* outputDir) {
    // we need to temporarily copy the strings because ExtractImages takes char* arguments.
    char* _datfname = strdup(datfname);
    char* _tabfname = strdup(tabfname);
    char* _outputDir = strdup(outputDir);

    int extract_res = ExtractImages(_datfname, _tabfname, palette, _outputDir);

    free(_datfname);
    free(_tabfname);
    free(_outputDir);

    if (extract_res != 0) {
        throw std::string("Error extracting images from ") + datfname + " and " + tabfname;
    }
}

// wrap main_unpack to take const char* arguments and throw an exception on error
void UnpackDataFile(const char* packfile, const char* unpackfile) {
    logging::Detail(std::string("unpacking ") + packfile);

    // we need to temporarily copy the strings because main_unpack takes char* arguments.
    char* _packfile = strdup(packfile);
    char* _unpackfile = strdup(unpackfile);

    int unpack_res = main_unpack(_packfile, _unpackfile);

    free(_packfile);
    free(_unpackfile);

    if (unpack_res != 0) {
        throw std::string("Error unpacking file: ") + packfile;
    }
}

std::vector<unsigned char> loadRawFile(const char *filename) {
    FILE* iFile = fopen(filename, "rb");
    if (iFile == NULL)
    {
        throw std::string("Cannot open file: ") + filename;
    }
    fseek(iFile, 0L, SEEK_END);
    size_t size = ftell(iFile);
    fseek(iFile, 0L, SEEK_SET);

    std::vector<unsigned char> buffer(size);
    fread(buffer.data(), 1, size, iFile);
    fclose(iFile);
    return buffer;
}
