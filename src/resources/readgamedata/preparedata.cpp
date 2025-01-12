/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

// Class to extract game data files
//

#include "preparedata.h"

#include "../../utils/logging.h"
#include "../intro/flifix.h"


void UnpackDataFile(const char* packfile, const char* unpackfile);
void ExtractImagesfromDataFile(const char* datfname, const char* tabfname, unsigned char* palette, const char* outputDir);


PrepareData::PrepareData(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver) {
    myDevice = device;
    myDriver = driver;

    PreparationOk = true;

    //create memory for ingame palette
    palette=static_cast<unsigned char*>(malloc(768));

    char palfile[50];
    strcpy(&palfile[0], "originalgame/data/palet0-0.dat");

    //read ingame palette
    ReadPaletteFile(&palfile[0], palette);

    //only add next line temporarily to extract level textures for level 7 up to 9
    //from HiOctaneTools project
    //AddOtherLevelsHiOctaneTools();

    //check if extraction directory is already present
    //if not create this directory
    if (IsDirectoryPresent("extract") == -1) {
        //directory is not there
        CreateDirectory("extract");
        {
            //directory was created ok
            //now we need to extract all files

            //extract SVGA game logo data if not all exported files present
            logging::Info("Extracting game logos...");
            PrepareSubDir("extract/images");
            //export all game images
            ExtractGameLogoSVGA();
            PreparationOk = PreparationOk && ExtractIntroductoryScreen();
            PreparationOk = PreparationOk && ExtractLoadingScreenSVGA();
            PreparationOk = PreparationOk && ExtractSelectionScreenSVGA();

            //extract SVGA game logo data if not all exported files present
            logging::Info("Extracting game fonts...");
            PrepareSubDir("extract/fonts");

            PrepareSubDir("extract/fonts/thinwhite");
            PreparationOk = PreparationOk && ExtractThinWhiteFontSVGA();

            PrepareSubDir("extract/fonts/smallsvga");
            PreparationOk = PreparationOk && ExtractSmallFontSVGA();

            PrepareSubDir("extract/fonts/smallsvgagreenish");
            if (PreparationOk) {
                //create greenish font for unselected items in menue (but based for smaller text size)
                PreparationOk = PreparationOk && CreateFontForUnselectedItemsInMenue((char*)"extract/fonts/smallsvga/osfnt0-1-",
                             (char*)"extract/fonts/smallsvgagreenish/green-osfnt0-1-", 0, 241);
            }

            PrepareSubDir("extract/fonts/large");
            PreparationOk = PreparationOk && ExtractLargeFontSVGA();

            PrepareSubDir("extract/fonts/largegreenish");
            if (PreparationOk) {
                //create greenish font for unselected items in menue
                //based on white SVGA font already extracted for game banner text font
                PreparationOk = PreparationOk && CreateFontForUnselectedItemsInMenue((char*)"extract/fonts/large/olfnt0-1-",
                             (char*)"extract/fonts/largegreenish/green-olfnt0-1-", 0, 241);
            }

            PrepareSubDir("extract/fonts/largegreen");
            PreparationOk = PreparationOk && ExtractLargeGreenFontSVGA();

            logging::Info("Extracting 1 player HUD...");
            PrepareSubDir("extract/hud1player");
            PreparationOk = PreparationOk && ExtractHUD1PlayerSVGA();

            logging::Info("Extracting 2 player HUD...");
            PrepareSubDir("extract/hud2player");
            PreparationOk = PreparationOk && ExtractHUD2PlayersSVGA();

            logging::Info("Extracting sky...");
            PrepareSubDir("extract/sky");
            PreparationOk = PreparationOk && ExtractSky();

            logging::Info("Extracting sprites...");
            PrepareSubDir("extract/sprites");
            PreparationOk = PreparationOk && ExtractTmaps();

            logging::Info("Extracting minimaps...");
            PrepareSubDir("extract/minimaps");
            PreparationOk = PreparationOk && ExtractMiniMapsSVGA();
            PreparationOk = PreparationOk && StitchMiniMaps();

            logging::Info("Extracting terrain textures...");
            //for TerrainTextures: Still todo: Scale Tiles by factor of 2.0
            PreparationOk = PreparationOk && ExtractTerrainTextures();

            logging::Info("Extracting levels...");
            ExtractLevels();

            logging::Info("Extracting sounds...");
            PrepareSubDir("extract/sound");
            PreparationOk = PreparationOk && ExtractSounds();

            logging::Info("Extracting music...");
            PrepareSubDir("extract/music");
            PreparationOk = PreparationOk && ExtractMusic();

            logging::Info("Extracting editor...");
            PrepareSubDir("extract/editor");
            PreparationOk = PreparationOk && ExtractEditorItemsLarge();
            PreparationOk = PreparationOk && ExtractEditorItemsSmall();
            PreparationOk = PreparationOk && ExtractEditorCursors();

            logging::Info("Extracting puzzle...");
            PrepareSubDir("extract/puzzle");
            PreparationOk = PreparationOk && ExtractCheatPuzzle();

            logging::Info("Extracting models...");
            PrepareSubDir("extract/models");
             PreparationOk = PreparationOk && ExtractModelTextures();

             if (PreparationOk) {
                 PreparationOk = PreparationOk && Extra3DModels();
             }

            logging::Info("Extracting intro...");
            PrepareSubDir("extract/intro");
            PreparationOk = PreparationOk && PrepareIntro();

            //install other available assets user has copied
            //into folder userData from another source
            logging::Info("Extracting other stuff...");
            AddOtherLevelsHiOctaneTools();
        }
    }
}

PrepareData::~PrepareData() {
    free(palette);
}

//return true if all expected files are present
//return false if at least one file is missing
bool PrepareData::CheckForGameLogoSVGAFiles() {
    bool allFiles = true;

    char* file1 = strdup("extract/logo0-1-0000.bmp");
    char* file2 = strdup("extract/logo0-1-0001.bmp");
    char* file3 = strdup("extract/logo0-1-0002.bmp");
    char* file4 = strdup("extract/logo0-1-0003.bmp");
    char* file5 = strdup("extract/logo0-1-0004.bmp");
    char* file6 = strdup("extract/logo0-1-0005.bmp");
    allFiles = allFiles && (FileExists(&file1[0]) == 1);
    allFiles = allFiles && (FileExists(&file2[0]) == 1);
    allFiles = allFiles && (FileExists(&file3[0]) == 1);
    allFiles = allFiles && (FileExists(&file4[0]) == 1);
    allFiles = allFiles && (FileExists(&file5[0]) == 1);
    allFiles = allFiles && (FileExists(&file6[0]) == 1);

    return allFiles;
}

bool PrepareData::Extract3DModel(char* srcFilename, char* destFilename, char* objName) {

    ObjectDatFile* newConversion = new ObjectDatFile(this->modelsTabFileInfo, this->modelTexAtlasSize.Width,
                       this->modelTexAtlasSize.Height);

    if (!newConversion->LoadObjectDatFile(srcFilename)) {
        delete newConversion;
        return false;
    }

    if (!newConversion->WriteToObjFile(destFilename, objName)) {
        delete  newConversion;
        return false;
    }

    delete newConversion;
    return true;
}

bool PrepareData::Extra3DModels() {
    char file[65];
    char file2[50];
    char helper[10];
    char objname[20];
    int idx;

    /******************************************
     *    Barel                               *
     ******************************************/

    for (idx = 0; idx < 3; idx++) {
        strcpy(file, "originalgame/objects/data/barel0-");
        strcpy(file2, "extract/models/barel0-");
        strcpy(objname, "barel0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Bikes                               *
     ******************************************/

    for (idx = 0; idx < 8; idx++) {
        strcpy(file, "originalgame/objects/data/bike0-");
        strcpy(file2, "extract/models/bike0-");
        strcpy(objname, "bike0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Car                                 *
     ******************************************/

    for (idx = 0; idx < 8; idx++) {
        strcpy(file, "originalgame/objects/data/car0-");
        strcpy(file2, "extract/models/car0-");
        strcpy(objname, "car0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Cone                                *
     ******************************************/

    for (idx = 0; idx < 2; idx++) {
        strcpy(file, "originalgame/objects/data/cone0-");
        strcpy(file2, "extract/models/cone0-");
        strcpy(objname, "cone0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Jet                                 *
     ******************************************/

    for (idx = 0; idx < 8; idx++) {
        strcpy(file, "originalgame/objects/data/jet0-");
        strcpy(file2, "extract/models/jet0-");
        strcpy(objname, "jet0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Jugga                               *
     ******************************************/

    for (idx = 0; idx < 8; idx++) {
        strcpy(file, "originalgame/objects/data/jugga0-");
        strcpy(file2, "extract/models/jugga0-");
        strcpy(objname, "jugga0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Marsh                               *
     ******************************************/

    for (idx = 0; idx < 1; idx++) {
        strcpy(file, "originalgame/objects/data/marsh0-");
        strcpy(file2, "extract/models/marsh0-");
        strcpy(objname, "marsh0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Recovery vehicle                    *
     ******************************************/

    for (idx = 0; idx < 1; idx++) {
        strcpy(file, "originalgame/objects/data/recov0-");
        strcpy(file2, "extract/models/recov0-");
        strcpy(objname, "recov0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Sign                                *
     ******************************************/

    for (idx = 0; idx < 1; idx++) {
        strcpy(file, "originalgame/objects/data/sign0-");
        strcpy(file2, "extract/models/sign0-");
        strcpy(objname, "sign0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Skim                                *
     ******************************************/

    for (idx = 0; idx < 8; idx++) {
        strcpy(file, "originalgame/objects/data/skim0-");
        strcpy(file2, "extract/models/skim0-");
        strcpy(objname, "skim0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Tank                                *
     ******************************************/

    for (idx = 0; idx < 8; idx++) {
        strcpy(file, "originalgame/objects/data/tank0-");
        strcpy(file2, "extract/models/tank0-");
        strcpy(objname, "tank0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    /******************************************
     *    Track                               *
     ******************************************/

    for (idx = 0; idx < 6; idx++) {
        strcpy(file, "originalgame/objects/data/track0-");
        strcpy(file2, "extract/models/track0-");
        strcpy(objname, "track0-");

        sprintf(helper, "%d", idx);
        strcat(file, helper);
        strcat(file, ".dat");
        strcat(file2, helper);
        strcat(file2, ".obj");
        strcat(objname, helper);

        if (!Extract3DModel(file, file2, objname))
            return false;
    }

    return true;
}


//extracts the level map files
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractLevels() {
    PrepareSubDir("extract/level0-1");
    UnpackDataFile("originalgame/maps/level0-1.dat", "extract/level0-1/level0-1-unpacked.dat");

    PrepareSubDir("extract/level0-2");
    UnpackDataFile("originalgame/maps/level0-2.dat", "extract/level0-2/level0-2-unpacked.dat");

    PrepareSubDir("extract/level0-3");
    UnpackDataFile("originalgame/maps/level0-3.dat", "extract/level0-3/level0-3-unpacked.dat");

    PrepareSubDir("extract/level0-4");
    UnpackDataFile("originalgame/maps/level0-4.dat", "extract/level0-4/level0-4-unpacked.dat");

    PrepareSubDir("extract/level0-5");
    UnpackDataFile("originalgame/maps/level0-5.dat", "extract/level0-5/level0-5-unpacked.dat");

    PrepareSubDir("extract/level0-6");
    UnpackDataFile("originalgame/maps/level0-6.dat", "extract/level0-6/level0-6-unpacked.dat");
}

//extracts the SVGA game logo data in data\logo0-1.dat and data\logo0-1.tab
//returns true in case of success, returns false in case of unexpected error
void PrepareData::ExtractGameLogoSVGA() {
    //read game logo in SVGA
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\logo0-1.dat
    //data\logo0-1.tab
    //Unknown format 	RNC-compressed = Yes 	Game logo (SVGA)

    UnpackDataFile("originalgame/data/logo0-1.dat", "extract/images/logo0-1-unpacked.dat");

    ExtractImagesfromDataFile("extract/images/logo0-1-unpacked.dat", "originalgame/data/logo0-1.tab", palette, "extract/images/logo0-1-");

    remove("extract/images/logo0-1-unpacked.dat");
}

//extracts the SVGA HUD for 1 Player in data\panel0-1.dat and data\panel0-1.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractHUD1PlayerSVGA() {
 //read HUD SVGA for 1 player
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\panel0-1.dat
 //data\panel0-1.tab
 //Unknown format 	RNC-compressed = No 	HUD 1-Player (SVGA)

 //unpack data file
 char unpackfile[35];
 char tabfile[35];
 strcpy(unpackfile, "originalgame/data/panel0-1.dat");
 strcpy(tabfile, "originalgame/data/panel0-1.tab");

 //is not RNC compressed, we can skip this step

 char palFile[35];
 char outputDir[50];
 strcpy(palFile, "originalgame/data/palet0-0.dat");
 strcpy(outputDir, "extract/hud1player/panel0-1-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfile, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 return true;
}

//extracts the SVGA Minimaps in data\track0-1.dat and data\track0-1.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractMiniMapsSVGA() {
 //data\track0-1.dat
 //data\track0-1.tab
 //Unknown format 	RNC-compressed = No 	MiniMaps

 //unpack data file
 char unpackfile[35];
 char tabfile[35];
 strcpy(unpackfile, "originalgame/data/track0-1.dat");
 strcpy(tabfile, "originalgame/data/track0-1.tab");

 //is not RNC compressed, we can skip this step

 char palFile[35];
 char outputDir[50];
 strcpy(palFile, "originalgame/data/palet0-0.dat");
 strcpy(outputDir, "extract/minimaps/track0-1-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfile, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 return true;
}

bool PrepareData::StitchMiniMaps() {
    /********************************************
     *  Map for level 1                         *
     * ******************************************/

    char filename[50];

    strcpy(filename, "extract/minimaps/track0-1-0000.bmp");

    //open image part1
    irr::io::IReadFile *file = myDevice->getFileSystem()->createAndOpenFile(filename);
    irr::video::IImage* part1 = myDriver->createImageFromFile(file);

    //close the part1 picture file
    file->drop();

    //get part 1 image dimension
    irr::core::dimension2d<irr::u32> part1Dimension = part1->getDimension();

    //lock texture for only reading of pixel data
    //irr::u8* datapntr = (irr::u8*)part1->lock();

    irr::video::SColor texel;

    //get games transparent color at the upper leftmost pixel (0,0)
    irr::video::SColor texelTrans = part1->getPixel(0,0);

    //unlock image again!
    //part1->unlock();

    strcpy(filename, "extract/minimaps/track0-1-0001.bmp");

    //open image part2
    file = myDevice->getFileSystem()->createAndOpenFile(filename);
    irr::video::IImage* part2 = myDriver->createImageFromFile(file);

    //close the part2 picture file
    file->drop();

    //get part 2 image dimension
    irr::core::dimension2d<irr::u32> part2Dimension = part2->getDimension();

    //stitch together
    //create the new empty image for the modified sky file
    irr::video::IImage* imgNew =
        myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(part1Dimension.Width, part1Dimension.Height
                                                                                                        + part2Dimension.Height - 1));

    //first make sure new image is filled completely
    //with the transparent color of the game
    imgNew->fill(texelTrans);

    part1->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                   irr::core::rect<irr::s32>(0, 0, part1Dimension.Width, part1Dimension.Height));

    part2->copyTo(imgNew, irr::core::vector2d<irr::s32>(6, 120),
                   irr::core::rect<irr::s32>(0, 0, part2Dimension.Width, part2Dimension.Height));

    part1->drop();
    part2->drop();

    strcpy(filename, "extract/minimaps/track0-1.png");

    //create new file for writting
    irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(filename, false);

    myDriver->writeImageToFile(imgNew, outputPic);

    //close output file
    outputPic->drop();
    imgNew->drop();

    /********************************************
     *  Map for level 2                         *
     * ******************************************/

    strcpy(filename, "extract/minimaps/track0-1-0002.bmp");

    //open image part1
    file = myDevice->getFileSystem()->createAndOpenFile(filename);
    part1 = myDriver->createImageFromFile(file);

    //close the part1 picture file
    file->drop();

    //get part 1 image dimension
    part1Dimension = part1->getDimension();

    strcpy(filename, "extract/minimaps/track0-1-0003.bmp");

    //open image part2
    file = myDevice->getFileSystem()->createAndOpenFile(filename);
    part2 = myDriver->createImageFromFile(file);

    //close the part2 picture file
    file->drop();

    //get part 2 image dimension
    part2Dimension = part2->getDimension();

    //open image part3

    strcpy(filename, "extract/minimaps/track0-1-0004.bmp");

    file = myDevice->getFileSystem()->createAndOpenFile(filename);
    irr::video::IImage* part3 = myDriver->createImageFromFile(file);

    //close the part3 picture file
    file->drop();

    //get part 3 image dimension
    irr::core::dimension2d<irr::u32> part3Dimension = part3->getDimension();

    //stitch together
    //create the new empty image for the modified sky file
    imgNew =
        myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(part1Dimension.Width + part2Dimension.Width, part1Dimension.Height
                                                                                                        + part3Dimension.Height - 1));

    //first make sure new image is filled completely
    //with the transparent color of the game
    imgNew->fill(texelTrans);

    part1->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                   irr::core::rect<irr::s32>(0, 0, part1Dimension.Width, part1Dimension.Height));

    part2->copyTo(imgNew, irr::core::vector2d<irr::s32>(120, 5),
                   irr::core::rect<irr::s32>(0, 0, part2Dimension.Width, part2Dimension.Height));

    part3->copyTo(imgNew, irr::core::vector2d<irr::s32>(74, 120),
                   irr::core::rect<irr::s32>(0, 0, part3Dimension.Width, part3Dimension.Height));

    strcpy(filename, "extract/minimaps/track0-2.png");

    //create new file for writting
    outputPic = myDevice->getFileSystem()->createAndWriteFile(filename, false);

    myDriver->writeImageToFile(imgNew, outputPic);

    //close output file
    outputPic->drop();
    imgNew->drop();

    /********************************************
     *  Map for level 3                         *
     * ******************************************/

    //is alreay finished in file track0-1-0005.bmp
    //no need for stitching

    /********************************************
     *  Map for level 4                         *
     * ******************************************/

    strcpy(filename, "extract/minimaps/track0-1-0006.bmp");

    //open image part1
    file = myDevice->getFileSystem()->createAndOpenFile(filename);
    part1 = myDriver->createImageFromFile(file);

    //close the part1 picture file
    file->drop();

    //get part 1 image dimension
    part1Dimension = part1->getDimension();

    strcpy(filename, "extract/minimaps/track0-1-0007.bmp");

    //open image part2
    file = myDevice->getFileSystem()->createAndOpenFile(filename);
    part2 = myDriver->createImageFromFile(file);

    //close the part2 picture file
    file->drop();

    //get part 2 image dimension
    part2Dimension = part2->getDimension();

    //stitch together
    //create the new empty image for the modified sky file
    imgNew =
        myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(part1Dimension.Width, part1Dimension.Height
                                                                                                        + part2Dimension.Height - 1));

    //first make sure new image is filled completely
    //with the transparent color of the game
    imgNew->fill(texelTrans);

    part1->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                   irr::core::rect<irr::s32>(0, 0, part1Dimension.Width, part1Dimension.Height));

    part2->copyTo(imgNew, irr::core::vector2d<irr::s32>(27, 120),
                   irr::core::rect<irr::s32>(0, 0, part2Dimension.Width, part2Dimension.Height));

    strcpy(filename, "extract/minimaps/track0-4.png");

    //create new file for writting
    outputPic = myDevice->getFileSystem()->createAndWriteFile(filename, false);

    myDriver->writeImageToFile(imgNew, outputPic);

    //close output file
    outputPic->drop();
    imgNew->drop();

    /********************************************
     *  Map for level 5                         *
     * ******************************************/

    strcpy(filename, "extract/minimaps/track0-1-0008.bmp");

    //open image part1
    file = myDevice->getFileSystem()->createAndOpenFile(filename);
    part1 = myDriver->createImageFromFile(file);

    //close the part1 picture file
    file->drop();

    //get part 1 image dimension
    part1Dimension = part1->getDimension();

    strcpy(filename, "extract/minimaps/track0-1-0009.bmp");

    //open image part2
    file = myDevice->getFileSystem()->createAndOpenFile(filename);
    part2 = myDriver->createImageFromFile(file);

    //close the part2 picture file
    file->drop();

    //get part 2 image dimension
    part2Dimension = part2->getDimension();

    //stitch together
    //create the new empty image for the modified sky file
    imgNew =
        myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(part1Dimension.Width, part1Dimension.Height
                                                                                                        + part2Dimension.Height - 1));

    //first make sure new image is filled completely
    //with the transparent color of the game
    imgNew->fill(texelTrans);

    part1->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                   irr::core::rect<irr::s32>(0, 0, part1Dimension.Width, part1Dimension.Height));

    part2->copyTo(imgNew, irr::core::vector2d<irr::s32>(18, 120),
                   irr::core::rect<irr::s32>(0, 0, part2Dimension.Width, part2Dimension.Height));

    strcpy(filename, "extract/minimaps/track0-5.png");

    //create new file for writting
    outputPic = myDevice->getFileSystem()->createAndWriteFile(filename, false);

    myDriver->writeImageToFile(imgNew, outputPic);

    //close output file
    outputPic->drop();
    imgNew->drop();

    /********************************************
     *  Map for level 6                         *
     * ******************************************/

    //is alreay finished in file track0-1-0010.bmp
    //no need for stitching

    return true;
}

//extracts the SVGA HUD for 2 Players in data\panel0-0.dat and data\panel0-0.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractHUD2PlayersSVGA() {
 //read HUD SVGA for 2 players
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\panel0-0.dat
 //data\panel0-0.tab
 //Unknown format 	RNC-compressed = No 	HUD 2-Player (SVGA)

 //unpack data file
 char unpackfile[35];
 char tabfile[35];
 strcpy(unpackfile, "originalgame/data/panel0-0.dat");
 strcpy(tabfile, "originalgame/data/panel0-0.tab");

 //is not RNC compressed, we can skip this step

 char palFile[35];
 char outputDir[50];
 strcpy(palFile, "originalgame/data/palet0-0.dat");
 strcpy(outputDir, "extract/hud2player/panel0-0-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfile, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 return true;
}

//extracts the SVGA Large Green font in data\pfont0-1.dat and data\pfont0-1.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractLargeGreenFontSVGA() {
 //read Large Green Font SVGA
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\pfont0-1.dat
 //data\pfont0-1.tab
 //Unknown format 	RNC-compressed = No 	Large Green Font (SVGA)

 //unpack data file
 char unpackfile[35];
 char tabfile[35];
 strcpy(unpackfile, "originalgame/data/pfont0-1.dat");
 strcpy(tabfile, "originalgame/data/pfont0-1.tab");

 //is not RNC compressed, we can skip this step

 char outputDir[50];
 strcpy(outputDir, "extract/fonts/largegreen/pfont0-1-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfile, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 return true;
}

//extracts the Cheat puzzle in data\puzzle.dat and data\puzzle.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractCheatPuzzle() {
 //read Cheat puzzle
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\puzzle.dat
 //data\puzzle.tab
 //Unknown format 	RNC-compressed = No 	Cheat puzzle  112x96

 //unpack data file
 char unpackfile[50];

 //is not RNC compressed, we can skip this step

 //now create final png picture out of raw video data
 //we also have to take game palette into account
 char palFile[35];
 strcpy(palFile, "originalgame/data/palet0-0.dat");

 char outputFile[50];
 strcpy(unpackfile, "originalgame/data/puzzle.dat");
 strcpy(outputFile, "extract/puzzle/puzzle.png");

 //scale puzzle by factor 4
 ConvertRawImageData(unpackfile, palette, 112, 96, outputFile, 4.0);

 return true;
}

//extracts the SVGA Large white font data in data\olfnt0-1.dat and data\olfnt0-1.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractLargeFontSVGA() {
 //read Large Fonts SVGA
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\olfnt0-1.dat
 //data\olfnt0-1.tab
 //Unknown format 	RNC-compressed = Yes 	Large white font (SVGA)

 //unpack data file
 char packfile[35];
 char unpackfile[60];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/olfnt0-1.dat");
 strcpy(tabfile, "originalgame/data/olfnt0-1.tab");
 strcpy(unpackfile, "extract/fonts/large/olfnt0-1-unpacked.dat");

 //RNC unpack Large Font file
 int unpack_res = main_unpack(&packfile[0], &unpackfile[0]);

 if (unpack_res != 0) {
     return false;
 }

 char unpackfileDat[50];
 char palFile[35];
 char outputDir[50];
 strcpy(unpackfileDat, "extract/fonts/large/olfnt0-1-unpacked.dat");
 strcpy(palFile, "originalgame/data/palet0-0.dat");
 strcpy(outputDir, "extract/fonts/large/olfnt0-1-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfileDat, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 remove(unpackfile);

 return true;
}

//extracts the SVGA Small white font data in data\osfnt0-1.dat and data\osfnt0-1.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractSmallFontSVGA() {
 //read Small Fonts SVGA
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\osfnt0-1.dat
 //data\osfnt0-1.tab
 //Unknown format 	RNC-compressed = Yes 	Small white font (SVGA)

 //unpack data file
 char packfile[35];
 char unpackfile[60];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/osfnt0-1.dat");
 strcpy(tabfile, "originalgame/data/osfnt0-1.tab");
 strcpy(unpackfile, "extract/fonts/smallsvga/osfnt0-1-unpacked.dat");

 //RNC unpack small Font file
 int unpack_res = main_unpack(&packfile[0], &unpackfile[0]);

 if (unpack_res != 0) {
     return false;
 }

 char unpackfileDat[50];
 char palFile[35];
 char outputDir[50];
 strcpy(unpackfileDat, "extract/fonts/smallsvga/osfnt0-1-unpacked.dat");
 strcpy(palFile, "originalgame/data/palet0-0.dat");
 strcpy(outputDir, "extract/fonts/smallsvga/osfnt0-1-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfileDat, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 remove(unpackfile);

 return true;
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
bool PrepareData::CreateFontForUnselectedItemsInMenue(char* sourceFntFileName, char* destFntFileName,
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
        irr::video::IImage* srcImg = myDriver->createImageFromFile(finalpathSrc);

        irr::core::dimension2d<irr::u32> srcDim(srcImg->getDimension().Width, srcImg->getDimension().Height);

        //problem opening the source image?
        if (srcImg == NULL)
            return false;

        //copy source image into new destination image
        irr::video::IImage* destImg =
                myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(srcDim.Width, srcDim.Height));

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
            //unexpected error, just return
            return false;
        }

        if (!this->ReplacePixelColor(destImg, detColor, newColor)) {
            //unexpected error, just return
            return false;
        }

        //now we have the font character with new color in image variable
        //save the result into a new image file
        irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(finalpathDest, false);
        myDriver->writeImageToFile(destImg, outputPic);

        //close output file
        outputPic->drop();
        destImg->drop();
    }

    return true;
}

//return true if all expected files are present
//return false if at least one file is missing
bool PrepareData::CheckForScreenSVGA() {
    bool allFiles = true;

    char* file1 = strdup("extract/onet0-1-unpacked.dat");

    allFiles = allFiles && (FileExists(&file1[0]) == 1);

    return allFiles;
}

//extracts the SVGA Loading Screen (is shown while game loads) in data\onet0-1.dat and data\onet0-1.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractLoadingScreenSVGA() {
 //read SVGA Loading Screen
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\onet0-1.dat
 //data\onet0-1.tab
 //Raw VGA image 	RNC-compressed = Yes 	Loading and selection screens

 //unpack data file
 char packfile[35];
 char unpackfile[50];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/onet0-1.dat");
 strcpy(tabfile, "originalgame/data/onet0-1.tab");
 strcpy(unpackfile, "extract/images/onet0-1-unpacked.dat");

 //RNC unpack Loading Screen
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //now create final png picture out of raw video data
 //we also have to take game palette into account

 char outputFile[50];
 strcpy(outputFile, "extract/images/onet0-1.png");

 //upscale image by a factor of 2.0, we then have an image of 1280 x 960
 //ConvertRawImageData(unpackfile, palette, 640, 480, outputFile, 2.0);

 ConvertRawImageData(unpackfile, palette, 640, 480, outputFile, 1.0);

 remove(unpackfile);

 return true;
}

//return true if selection screen (main menue background) is present
//return false if at least one file is missing
bool PrepareData::CheckForSelectionScreenSVGA() {
    bool allFiles = true;

    char* file1 = strdup("extract/oscr0-1-unpacked.dat");

    allFiles = allFiles && (FileExists(&file1[0]) == 1);

    return allFiles;
}

//extracts the SVGA Selection Screen (is the Main menue background picture) in data\oscr0-1.dat and data\oscr0-1.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractSelectionScreenSVGA() {
 //read SVGA Selection Screen
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\oscr0-1.dat
 //data\oscr0-1.tab
 //Raw VGA image 	RNC-compressed = Yes 	Loading and selection screens

 //unpack data file
 char packfile[35];
 char unpackfile[50];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/oscr0-1.dat");
 strcpy(tabfile, "originalgame/data/oscr0-1.tab");
 strcpy(unpackfile, "extract/images/oscr0-1-unpacked.dat");

 //RNC unpack Selection Screen
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //now create final png picture out of raw video data
 //we also have to take game palette into account
 char palFile[45];
 strcpy(palFile, "originalgame/data/palet0-0.dat");

 char outputFile[50];
 strcpy(outputFile, "extract/images/oscr0-1.png");

 //upscale image by a factor of 2.0, we then have an image of 1280 x 960
 //ConvertRawImageData(unpackfile, palette, 640, 480, outputFile, 2.0);

 ConvertRawImageData(unpackfile, palette, 640, 480, outputFile, 1.0);

 remove(unpackfile);

 return true;
}

//extracts the Sky images (in format 256x256) in data\sky0-*.dat and data\sky0-*.tab
//* is from 0 up to 5
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractSky() {
 //read all race track sky images
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //Raw VGA image 	RNC-compressed = Yes 	256x256 Sky images

 //unpack data file number 0
 char packfile[35];
 char unpackfile[50];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/sky0-0.dat");
 strcpy(tabfile, "originalgame/data/sky0-0.tab");
 strcpy(unpackfile, "extract/sky/sky0-0-unpacked.dat");

 //RNC unpack Selection Screen
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 1
 strcpy(packfile, "originalgame/data/sky0-1.dat");
 strcpy(tabfile, "originalgame/data/sky0-1.tab");
 strcpy(unpackfile, "extract/sky/sky0-1-unpacked.dat");

 //RNC unpack Selection Screen
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 2
 strcpy(packfile, "originalgame/data/sky0-2.dat");
 strcpy(tabfile, "originalgame/data/sky0-2.tab");
 strcpy(unpackfile, "extract/sky/sky0-2-unpacked.dat");

 //RNC unpack Selection Screen
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 3
 strcpy(packfile, "originalgame/data/sky0-3.dat");
 strcpy(tabfile, "originalgame/data/sky0-3.tab");
 strcpy(unpackfile, "extract/sky/sky0-3-unpacked.dat");

 //RNC unpack Selection Screen
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 4
 strcpy(packfile, "originalgame/data/sky0-4.dat");
 strcpy(tabfile, "originalgame/data/sky0-4.tab");
 strcpy(unpackfile, "extract/sky/sky0-4-unpacked.dat");

 //RNC unpack Selection Screen
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 5
 strcpy(packfile, "originalgame/data/sky0-5.dat");
 strcpy(tabfile, "originalgame/data/sky0-5.tab");
 strcpy(unpackfile, "extract/sky/sky0-5-unpacked.dat");

 //RNC unpack Selection Screen
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //now create final png picture out of raw video data
 //we also have to take game palette into account

 char outputFile[50];
 strcpy(unpackfile, "extract/sky/sky0-0-unpacked.dat");
 strcpy(outputFile, "extract/sky/sky0-0.png");

 //upscale image by a factor of 2.0, we then have an image of 512 x 512
 ConvertRawImageData(unpackfile, palette, 256, 256, outputFile, 2.0);

 remove(unpackfile);

 char modfile[50];
 strcpy(unpackfile, "extract/sky/sky0-0.png");
 strcpy(modfile, "extract/sky/modsky0-0.png");

 //create new modified sky image for us
 //for easier usage
 ModifySkyImage(unpackfile, modfile);

 strcpy(unpackfile, "extract/sky/sky0-1-unpacked.dat");
 strcpy(outputFile, "extract/sky/sky0-1.png");

  //upscale image by a factor of 2.0, we then have an image of 512 x 512
 ConvertRawImageData(&unpackfile[0], palette, 256, 256, outputFile, 2.0);

 remove(unpackfile);

 strcpy(unpackfile, "extract/sky/sky0-1.png");
 strcpy(modfile, "extract/sky/modsky0-1.png");

 //create new modified sky image for us
 //for easier usage
 ModifySkyImage(unpackfile, modfile);

 strcpy(unpackfile, "extract/sky/sky0-2-unpacked.dat");
 strcpy(outputFile, "extract/sky/sky0-2.png");

  //upscale image by a factor of 2.0, we then have an image of 512 x 512
 ConvertRawImageData(unpackfile, palette, 256, 256, outputFile, 2.0);

 remove(unpackfile);

 strcpy(unpackfile, "extract/sky/sky0-2.png");
 strcpy(modfile, "extract/sky/modsky0-2.png");

 //create new modified sky image for us
 //for easier usage
 ModifySkyImage(unpackfile, modfile);

 strcpy(unpackfile, "extract/sky/sky0-3-unpacked.dat");
 strcpy(outputFile, "extract/sky/sky0-3.png");

  //upscale image by a factor of 2.0, we then have an image of 512 x 512
 ConvertRawImageData(unpackfile, palette, 256, 256, outputFile, 2.0);

 remove(unpackfile);

 strcpy(unpackfile, "extract/sky/sky0-3.png");
 strcpy(modfile, "extract/sky/modsky0-3.png");

 //create new modified sky image for us
 //for easier usage
 ModifySkyImage(unpackfile, modfile);

 strcpy(unpackfile, "extract/sky/sky0-4-unpacked.dat");
 strcpy(outputFile, "extract/sky/sky0-4.png");

  //upscale image by a factor of 2.0, we then have an image of 512 x 512
 ConvertRawImageData(unpackfile, palette, 256, 256, outputFile, 2.0);

 remove(unpackfile);

 strcpy(unpackfile, "extract/sky/sky0-4.png");
 strcpy(modfile, "extract/sky/modsky0-4.png");

 //create new modified sky image for us
 //for easier usage
 ModifySkyImage(unpackfile, modfile);

 strcpy(unpackfile, "extract/sky/sky0-5-unpacked.dat");
 strcpy(outputFile, "extract/sky/sky0-5.png");

  //upscale image by a factor of 2.0, we then have an image of 512 x 512
 ConvertRawImageData(unpackfile, palette, 256, 256, outputFile, 2.0);

 remove(unpackfile);

 strcpy(unpackfile, "extract/sky/sky0-5.png");
 strcpy(modfile, "extract/sky/modsky0-5.png");

 //create new modified sky image for us
 //for easier usage
 ModifySkyImage(unpackfile, modfile);

 return true;
}

//This helper function modifies the original sky image so that we can use it easier
//in this project. The result is stored in another new image file
bool PrepareData::ModifySkyImage(char *origSkyFileName, char* outputModifiedSkyFileName) {
    //first open original sky image again
    irr::io::IReadFile *file = myDevice->getFileSystem()->createAndOpenFile(origSkyFileName);

    irr::video::IImage* origSky = myDriver->createImageFromFile(file);

    //get original sky image dimension
    irr::core::dimension2d<irr::u32> origDimension = origSky->getDimension();

    //we want to add in the modified sky image three copies of the original sky next to each other
    //the height should not change
    //calculate new image dimensions

    //create the new empty image for the modified sky file
    irr::video::IImage* imgNew =
        myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(3 * origDimension.Width, origDimension.Height - 1));

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
    irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(outputModifiedSkyFileName, false);

    myDriver->writeImageToFile(imgNew, outputPic);

    //close output file
    outputPic->drop();

    //close the original picture file
    file->drop();

    return true;
}

//the original Terrain texture Atlas stored within the game has all tiles
//in one column in y-direction; We can not use this complete Atlas for rendering, as
//this would lead to awful flickering lines on the border between different terrain tiles
//Therefore we need to read all tiles and export them to singulated picture files, which we can later
//use to load the textures
//Returns true in case of success, returns false in case of unexpected problem
bool PrepareData::ExportTerrainTextures(char* targetFile, char* exportDir, char* outputFileName) {
    //first open target image again
    irr::io::IReadFile *file = myDevice->getFileSystem()->createAndOpenFile(targetFile);

    irr::video::IImage* origAtlas = myDriver->createImageFromFile(targetFile);

    //get original atlas image dimension
    irr::core::dimension2d<irr::u32> origDimension = origAtlas->getDimension();

    //calculate overall number of tiles; Width of picture is tile size; Heigth of picture
    //divided by width equals the overall number of titles in the image
    int numberTiles = (origDimension.Height / origDimension.Width);

    PrepareSubDir(exportDir);

    //directory was created ok
    //now we need to extract all files

    char finalpath[50];
    char fname[20];

    //now export one tile after each other
    for (int tileIdx = 0; tileIdx < numberTiles; tileIdx++) {

        //create the new empty image for the next single tile
         irr::video::IImage* imgNew =
            myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(origDimension.Width, origDimension.Width));

            //process all the tiles by copying them to the new image at
            //the new position
            origAtlas->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                          irr::core::rect<irr::s32>(0, tileIdx * origDimension.Width, origDimension.Width, (tileIdx + 1) * origDimension.Width));

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

            strcpy(&finalpath[0], &exportDir[0]);
            strcat(&finalpath[0], &outputFileName[0]);
            sprintf (fname, "%0*d.png", 4, tileIdx);
            strcat(finalpath, fname);

            //create new file for writting
            irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(finalpath, false);

            //if (scaleFactor == 1.0) {
                   //write original image data
                  myDriver->writeImageToFile(imgNew, outputPic);
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

//the Terrain texture Atlas from https://github.com/movAX13h/HiOctaneTools
//has a different layout then the extracted texture atlas directly from the game files
//therefore to be able to use the levels 7 up to 9 data from their project
//we have to reorganize the atlas again (split it in single picture files)
//Returns true in case of success, returns false in case of unexpected problem
bool PrepareData::SplitHiOctaneToolsAtlas(char* targetFile, char* exportDir, char* outputFileName) {
    //first open target image again
    irr::io::IReadFile *file = myDevice->getFileSystem()->createAndOpenFile(targetFile);

    irr::video::IImage* origAtlas = myDriver->createImageFromFile(targetFile);

    //get HiOctaneTools atlas image dimension
    irr::core::dimension2d<irr::u32> origDimension = origAtlas->getDimension();

    //each tile is 64x64 pixels
    irr::f32 tilePixelSize = 64;

    //calculate overall number of tiles; each tile is tilePixelSize x tilePixelSize
    int numberTiles = (origDimension.Height / tilePixelSize) * (origDimension.Width / tilePixelSize);

    //create the target directory
    CreateDirectory(exportDir);

    //directory was created ok
    //now we need to extract all files

    char finalpath[50];
    char fname[20];
    int tileposx = 0;
    int tileposy = 0;

    //now export one tile after each other
    for (int tileIdx = 0; tileIdx < numberTiles; tileIdx++) {

        //create the new empty image for the next single tile
         irr::video::IImage* imgNew =
            myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(tilePixelSize, tilePixelSize));

            //process all the tiles by copying them to the new image at
            //the new position
            origAtlas->copyTo(imgNew, irr::core::vector2d<irr::s32>(0, 0),
                          irr::core::rect<irr::s32>(tileposx * tilePixelSize, tileposy * tilePixelSize, (tileposx + 1) * tilePixelSize, (tileposy +1) * tilePixelSize));

            tileposx++;

            if (tileposx > (origDimension.Width / tilePixelSize) - 1) {
                tileposx = 0;
                tileposy++;
            }

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
            irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(finalpath, false);

            //if (scaleFactor == 1.0) {
                   //write original image data
                  myDriver->writeImageToFile(imgNew, outputPic);
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

/*
//the original Terrain texture Atlas stored within the game has all tiles
//in one column in y-direction; Therefore the Atlas picture is not square at all;
//This is not good for rendering nowadays as textures should be square;
//Therefore lets rearrange tiles in original Atals, and save a new picture
//with 16 tiles in a row
void PrepareData::ReorganizeTerrainAtlas(char* targetFile, char* outputFileName) {
    int tilesInRow = 16;  //put 16 tiles in a row

    //first open target image again
    irr::io::IReadFile *file = myDevice->getFileSystem()->createAndOpenFile(targetFile);

    irr::video::IImage* origAtlas = myDriver->createImageFromFile(targetFile);

    //get original atlas image dimension
    irr::core::dimension2d<irr::u32> origDimension = origAtlas->getDimension();

    //calculate overall number of tiles; Width of picture is tile size; Heigth of picture
    //divided by width equals the overall number of titles in the image
    int numberTiles = (origDimension.Height / origDimension.Width);

    //how many rows do we need to hold all tiles?
    int numberRows = (numberTiles / tilesInRow);

    //create the new empty image for the reorganized Atals
    irr::video::IImage* imgNew =
            myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(origDimension.Width * tilesInRow, numberRows * origDimension.Width));

    //process all the tiles by copying them to the new image at
    //the new position
    int currRowNr = 0;
    int currColumnNr = 0;
    for (int tileIdx = 0; tileIdx < numberTiles; tileIdx++) {
        origAtlas->copyTo(imgNew, irr::core::vector2d<irr::s32>(currColumnNr * origDimension.Width, currRowNr * origDimension.Width),
                          irr::core::rect<irr::s32>(0, tileIdx * origDimension.Width, origDimension.Width, (tileIdx + 1) * origDimension.Width));

        currColumnNr++;

        //enough tiles in the current row?
        if (currColumnNr >= tilesInRow) {
            currColumnNr = 0;
            currRowNr++;
        }
    }
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

    //create new file for writting
/*    irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(outputFileName, false);

    //write image to file
   // if (scaleFactor == 1.0) {
       //write original image data
       myDriver->writeImageToFile(imgNew, outputPic);
   // } else {
        //write upscaled image data
    //   myDriver->writeImageToFile(imgUp, outputPic);
   // }

    //close output file
    outputPic->drop();

    //close the original picture file
    file->drop();
}
*/

//extracts the Terrain Textures (Texture Atlas) (in format 64×16384) in data\textu0-*.dat and data\textu0-*.tab
//* is from 0 up to 5
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractTerrainTextures() {
 //read all race track Terrain textures
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //Raw image 64×16384 	RNC-compressed = Yes 	64x64 Terrain Textures

 //unpack data file number 0
 char packfile[35];
 char unpackfile[100];
 char tabfile[35];
 char outputDirName[20];
 strcpy(packfile, "originalgame/data/textu0-0.dat");
 strcpy(tabfile, "originalgame/data/textu0-0.tab");
 strcpy(unpackfile, "extract/textu0-0-unpacked.dat");

 //RNC unpack file
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 1
 strcpy(packfile, "originalgame/data/textu0-1.dat");
 strcpy(tabfile, "originalgame/data/textu0-1.tab");
 strcpy(unpackfile, "extract/textu0-1-unpacked.dat");

 //RNC unpack file
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 2
 strcpy(packfile, "originalgame/data/textu0-2.dat");
 strcpy(tabfile, "originalgame/data/textu0-2.tab");
 strcpy(unpackfile, "extract/textu0-2-unpacked.dat");

 //RNC unpack file
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 3
 strcpy(packfile, "originalgame/data/textu0-3.dat");
 strcpy(tabfile, "originalgame/data/textu0-3.tab");
 strcpy(unpackfile, "extract/textu0-3-unpacked.dat");

 //RNC unpack file
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 4
 strcpy(packfile, "originalgame/data/textu0-4.dat");
 strcpy(tabfile, "originalgame/data/textu0-4.tab");
 strcpy(unpackfile, "extract/textu0-4-unpacked.dat");

 //RNC unpack file
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //unpack data file number 5
 strcpy(packfile, "originalgame/data/textu0-5.dat");
 strcpy(tabfile, "originalgame/data/textu0-5.tab");
 strcpy(unpackfile, "extract/textu0-5-unpacked.dat");

 //RNC unpack file
 unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 //now create final png picture out of raw video data
 //we also have to take game palette into account
 char outputFile[50];
 char finalFile[50];
 strcpy(unpackfile, "extract/textu0-0-unpacked.dat");
 strcpy(outputFile, "extract/textu0-0-orig.png");
 strcpy(finalFile, "/tex");

 ConvertRawImageData(unpackfile, palette, 64, 16384, outputFile);

 //reorganize Terrain Atlas format to be Square
 //ReorganizeTerrainAtlas(&outputFile[0], &finalFile[0]);
 strcpy(outputDirName, "extract/level0-1");
 ExportTerrainTextures(outputFile, outputDirName, finalFile);

 remove(unpackfile);
 remove(outputFile);

 strcpy(unpackfile, "extract/textu0-1-unpacked.dat");
 strcpy(outputFile, "extract/textu0-1-orig.png");
 strcpy(finalFile, "/tex");

 ConvertRawImageData(unpackfile, palette, 64, 16384, outputFile);

 //reorganize Terrain Atlas format to be Square
 //ReorganizeTerrainAtlas(&outputFile[0], &finalFile[0]);
 strcpy(outputDirName, "extract/level0-2");
 ExportTerrainTextures(outputFile, outputDirName, finalFile);

 remove(unpackfile);
 remove(outputFile);

 strcpy(unpackfile, "extract/textu0-2-unpacked.dat");
 strcpy(outputFile, "extract/textu0-2-orig.png");
 strcpy(finalFile, "/tex");

 ConvertRawImageData(unpackfile, palette, 64, 16384, outputFile);

 //reorganize Terrain Atlas format to be Square
 //ReorganizeTerrainAtlas(&outputFile[0], &finalFile[0]);
 strcpy(outputDirName, "extract/level0-3");
 ExportTerrainTextures(outputFile, outputDirName, finalFile);

 remove(unpackfile);
 remove(outputFile);

 strcpy(unpackfile, "extract/textu0-3-unpacked.dat");
 strcpy(outputFile, "extract/textu0-3-orig.png");
 strcpy(finalFile, "/tex");

 ConvertRawImageData(unpackfile, palette, 64, 16384, outputFile);

 //reorganize Terrain Atlas format to be Square
 //ReorganizeTerrainAtlas(&outputFile[0], &finalFile[0]);
 strcpy(outputDirName, "extract/level0-4");
 ExportTerrainTextures(outputFile, outputDirName, finalFile);

 remove(unpackfile);
 remove(outputFile);

 strcpy(unpackfile, "extract/textu0-4-unpacked.dat");
 strcpy(outputFile, "extract/textu0-4-orig.png");
 strcpy(finalFile, "/tex");

 ConvertRawImageData(unpackfile, palette, 64, 16384, outputFile);

 //reorganize Terrain Atlas format to be Square
 //ReorganizeTerrainAtlas(&outputFile[0], &finalFile[0]);
 strcpy(outputDirName, "extract/level0-5");
 ExportTerrainTextures(outputFile, outputDirName, finalFile);

 remove(unpackfile);
 remove(outputFile);

 strcpy(unpackfile, "extract/textu0-5-unpacked.dat");
 strcpy(outputFile, "extract/textu0-5-orig.png");
 strcpy(finalFile, "/tex");

 ConvertRawImageData(unpackfile, palette, 64, 16384, outputFile);

 //reorganize Terrain Atlas format to be Square
 //ReorganizeTerrainAtlas(&outputFile[0], &finalFile[0]);
 strcpy(outputDirName, "extract/level0-6");
 ExportTerrainTextures(outputFile, outputDirName, finalFile);

 remove(unpackfile);
 remove(outputFile);

 return true;
}

bool PrepareData::AddOtherLevelsHiOctaneTools() {
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

     return true;
}

bool PrepareData::ConvertRawImageData(char* rawDataFilename, unsigned char *palette, irr::u32 sizex, irr::u32 sizey,
                                      char* outputFilename, int scaleFactor, bool flipY) {
    FILE* iFile;

    iFile = fopen(rawDataFilename, "rb");
    fseek(iFile, 0L, SEEK_END);
    size_t size = ftell(iFile);
    fseek(iFile, 0L, SEEK_SET);

    if (size != (sizex*sizey)) {
        fclose(iFile);
        printf("\nError - Raw picture filesize does not fit with expectation! %s\n", rawDataFilename);
        return false;
    }

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
     unsigned char *arrR=static_cast<unsigned char*>(malloc(size));
     unsigned char *arrG=static_cast<unsigned char*>(malloc(size));
     unsigned char *arrB=static_cast<unsigned char*>(malloc(size));

     double arrIntR;
     double arrIntG;
     double arrIntB;
     unsigned char color;

     //use palette to derive RGB information for all pixels
     //loaded palette has 6 bits per color
     for (counter = 0; counter < size; counter++) {
         color = ByteArray[counter];
         arrIntR = palette[color * 3    ];
         arrIntG = palette[color * 3 + 1 ];
         arrIntB = palette[color * 3 + 2 ];

         arrIntR = (arrIntR * 255.0) / 63.0;
         arrIntG = (arrIntG * 255.0) / 63.0;
         arrIntB = (arrIntB * 255.0) / 63.0;

         arrR[counter] = (unsigned char)arrIntR;
         arrG[counter] = (unsigned char)arrIntG;
         arrB[counter] = (unsigned char)arrIntB;
     }

     //create an empty image
     irr::video::IImage* img =
             myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex, sizey));

     //draw the image
     for (irr::u32 posx = 0; posx < sizex; posx++) {
         for (irr::u32 posy = 0; posy < sizey; posy++) {
             if (!flipY) {
                 img->setPixel(posx, posy,  irr::video::SColor(255, arrR[posy * sizex + posx],
                           arrG[posy * sizex + posx], arrB[posy * sizex + posx]));
             } else {
                 //flip image vertically
                 img->setPixel(posx, posy,  irr::video::SColor(255, arrR[(sizey - posy) * sizex + posx],
                           arrG[(sizey - posy) * sizex + posx], arrB[(sizey - posy) * sizex + posx]));
             }
         }
     }

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
     }

     //create new file for writting
     irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(outputFilename, false);

     //write image to file
     if (scaleFactor == 1.0) {
        //write original image data
        myDriver->writeImageToFile(img, outputPic);
     } else {
         //write upscaled image data
        myDriver->writeImageToFile(imgUp, outputPic);
     }

     //close output file
     outputPic->drop();

    delete[] ByteArray;
    free(arrR);
    free(arrG);
    free(arrB);

    return true;
}

bool PrepareData::ConvertIntroFrame(char* ByteArray, flic::Colormap colorMap, irr::u32 sizex, irr::u32 sizey,
                                      char* outputFilename, int scaleFactor, bool flipY) {
    size_t size = sizex * sizey;

   //create arrays for color information
    unsigned char *arrR=static_cast<unsigned char*>(malloc(size));
    unsigned char *arrG=static_cast<unsigned char*>(malloc(size));
    unsigned char *arrB=static_cast<unsigned char*>(malloc(size));

    unsigned char color;

    size_t counter;

    //use palette to derive RGB information for all pixels
    //loaded palette has 6 bits per color
    for (counter = 0; counter < size; counter++) {
         color = ByteArray[counter];

         arrR[counter] = (unsigned char)colorMap[color].r;
         arrG[counter] = (unsigned char)colorMap[color].g;
         arrB[counter] = (unsigned char)colorMap[color].b;
     }

     //create an empty image
     irr::video::IImage* img =
             myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex, sizey));

     //draw the image
     for (irr::u32 posx = 0; posx < sizex; posx++) {
         for (irr::u32 posy = 0; posy < sizey; posy++) {
             if (!flipY) {
                 img->setPixel(posx, posy,  irr::video::SColor(255, arrR[posy * sizex + posx],
                           arrG[posy * sizex + posx], arrB[posy * sizex + posx]));
             } else {
                 //flip image vertically
                 img->setPixel(posx, posy,  irr::video::SColor(255, arrR[(sizey - posy) * sizex + posx],
                           arrG[(sizey - posy) * sizex + posx], arrB[(sizey - posy) * sizex + posx]));
             }
         }
     }

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
     }

     //create new file for writting
     irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(outputFilename, false);

     //write image to file
     if (scaleFactor == 1.0) {
        //write original image data
        myDriver->writeImageToFile(img, outputPic);
     } else {
         //write upscaled image data
        myDriver->writeImageToFile(imgUp, outputPic);
     }

    //close output file
    outputPic->drop();

    free(arrR);
    free(arrG);
    free(arrB);

    return true;
}

void PrepareData::ReadPaletteFile(char *palFile, unsigned char* paletteDataOut) {
    int retcode=read_palette_rgb(paletteDataOut,palFile,(unsigned int)(256));

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

bool PrepareData::ConvertObjectTexture(char* rawDataFilename, char* outputFilename, int scaleFactor) {
    FILE* iFile;

    iFile = fopen(rawDataFilename, "rb");
    fseek(iFile, 0L, SEEK_END);
    size_t size = ftell(iFile);
    fseek(iFile, 0L, SEEK_SET);

    irr::u16 sizex = 256;
    irr::u16 sizey = 768;

    //6 bytes of the file is needed for picture information above
/*    if ((size - 6) != (sizex*sizey)) {
        fclose(iFile);
        printf("\nError - Raw picture filesize does not fit with expectation! %s\n", rawDataFilename);
        return false;
    }*/

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

     unsigned int amountPixel = sizex*sizey;

     //use palette to derive RGB information for all pixels
     //loaded palette has 6 bits per color
     //raw pictue data starts at byte #7
     for (counter = 0; counter < amountPixel; counter++) {
         color = ByteArray[counter];
         arrIntR = palette[color * 3    ];
         arrIntG = palette[color * 3 + 1 ];
         arrIntB = palette[color * 3 + 2 ];

         arrIntR = (arrIntR * 255.0) / 63.0;
         arrIntG = (arrIntG * 255.0) / 63.0;
         arrIntB = (arrIntB * 255.0) / 63.0;

         arrR[counter] = (unsigned char)arrIntR;
         arrG[counter] = (unsigned char)arrIntG;
         arrB[counter] = (unsigned char)arrIntB;
     }

     //create an empty image
     irr::video::IImage* img =
             myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex, sizey));

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
         imgUp = myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex * scaleFactor, sizey * scaleFactor));

         //get the pointers to the raw image data
         uint32_t *imageDataUp = (uint32_t*)imgUp->lock();
         uint32_t *imageData = (uint32_t*)img->lock();

         xbrz::scale(scaleFactor, &imageData[0], &imageDataUp[0], sizex, sizey, xbrz::ColorFormat::ARGB, xbrz::ScalerCfg(), 0, sizey);

         //release the pointers, we do not need them anymore
         img->unlock();
         imgUp->unlock();
       }

     //create new file for writting
     irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(outputFilename, false);

     //write image to file
     if (scaleFactor == 1.0) {
           //write original image data
           myDriver->writeImageToFile(img, outputPic);
        } else {
            //write upscaled image data
           myDriver->writeImageToFile(imgUp, outputPic);
     }

     //close output file
     outputPic->drop();

    delete[] ByteArray;
    free(arrR);
    free(arrG);
    free(arrB);

    return true;
}

//the Terrain texture Atlas from https://github.com/movAX13h/HiOctaneTools
//has a different layout then the extracted texture atlas directly from the game files
//therefore to be able to use the levels 7 up to 9 data from their project
//we have to reorganize the atlas again (split it in single picture files)
//Returns true in case of success, returns false in case of unexpected problem
bool PrepareData::DebugSplitModelTextureAtlasAndWriteSingulatedPictures(
        char *atlasFileName, char* exportDir, char* outputFileName, TABFILE *tabf) {
    //first open target image again
    irr::io::IReadFile *file = myDevice->getFileSystem()->createAndOpenFile(atlasFileName);

    irr::video::IImage* origAtlas = myDriver->createImageFromFile(atlasFileName);

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
            myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(itemPntr->width, itemPntr->height));

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
            irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(finalpath, false);

            //if (scaleFactor == 1.0) {
                   //write original image data
                  myDriver->writeImageToFile(imgNew, outputPic);
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
             myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex, sizey));

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
         imgUp = myDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(sizex * scaleFactor, sizey * scaleFactor));

         //get the pointers to the raw image data
         uint32_t *imageDataUp = (uint32_t*)imgUp->lock();
         uint32_t *imageData = (uint32_t*)img->lock();

         xbrz::scale(scaleFactor, &imageData[0], &imageDataUp[0], sizex, sizey, xbrz::ColorFormat::ARGB, xbrz::ScalerCfg(), 0, sizey);

         //release the pointers, we do not need them anymore
         img->unlock();
         imgUp->unlock();
       }

     //create new file for writting
     irr::io::IWriteFile* outputPic = myDevice->getFileSystem()->createAndWriteFile(outputFilename, false);

     //write image to file
     if (scaleFactor == 1.0) {
           //write original image data
           myDriver->writeImageToFile(img, outputPic);
        } else {
            //write upscaled image data
           myDriver->writeImageToFile(imgUp, outputPic);
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
bool PrepareData::ExtractIntroductoryScreen() {
 //read introductory screen
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //Raw image 320×200 	RNC-compressed = Yes 	320x200 Introductory screen

 //unpack data file number 0
 char packfile[35];
 char unpackfile[60];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/title.dat");
 strcpy(tabfile, "originalgame/data/title.tab");
 strcpy(unpackfile, "extract/images/title-unpacked.dat");

 //RNC unpack file
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

  char outputFile[50];
  strcpy(outputFile, "extract/images/title.png");

  //upscale original image data by a factor 4, we get an image with 1280 x 800
  //ConvertRawImageData(&unpackfile[0], palette, 320, 200, &outputFile[0], 4.0);

  //upscale original image data by a factor 2, we get an image with 640 x 480
  ConvertRawImageData(unpackfile, palette, 320, 200, outputFile, 2.0);

 //remove unnecessary files
 remove(unpackfile);

 return true;
}


//extracts the SVGA thin white font data in data\hfont0-0.dat and data\hfont0-0.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractThinWhiteFontSVGA() {
 //read thin white font SVGA
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\hfont0-0.dat
 //data\hfont0-0.tab
 //Unknown format 	RNC-compressed = Yes 	Thin white font (SVGA) (SVGA)

 //unpack data file
 char packfile[35];
 char unpackfile[60];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/hfont0-0.dat");
 strcpy(tabfile, "originalgame/data/hfont0-0.tab");
 strcpy(unpackfile, "extract/fonts/thinwhite/hfont0-0-unpacked.dat");

 //RNC unpack thin white font
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 char unpackfileDat[60];
 char outputDir[50];
 strcpy(unpackfileDat, "extract/fonts/thinwhite/hfont0-0-unpacked.dat");
 strcpy(outputDir, "extract/fonts/thinwhite/hfont0-0-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfileDat, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 //remove unnecessary files
 remove(unpackfileDat);

 return true;
}

//extracts the Editor cursors data in data\point0-0.dat and data\point0-0.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractEditorCursors() {
 //read Editor cursors
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\point0-0.dat
 //data\point0-0.tab
 //Unknown format 	RNC-compressed = Yes 	Editor cursors

 //unpack data file
 char packfile[35];
 char unpackfile[60];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/point0-0.dat");
 strcpy(tabfile, "originalgame/data/point0-0.tab");
 strcpy(unpackfile, "extract/editor/point0-0-unpacked.dat");

 //RNC unpack editor cursors
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 char unpackfileDat[60];
 char outputDir[60];
 strcpy(unpackfileDat, "extract/editor/point0-0-unpacked.dat");
 strcpy(outputDir, "extract/editor/point0-0-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfileDat, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 remove(unpackfileDat);

 return true;
}

//extracts the Editor items large in data\hspr0-0.dat and data\hspr0-0.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractEditorItemsLarge() {
 //read Editor items large
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\hspr0-0.dat
 //data\hspr0-0.tab
 //Unknown format 	RNC-compressed = Yes 	Editor icons (large)

 //unpack data file
 char packfile[35];
 char unpackfile[50];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/hspr0-0.dat");
 strcpy(tabfile, "originalgame/data/hspr0-0.tab");
 strcpy(unpackfile, "extract/editor/hspr0-0-unpacked.dat");

 //RNC unpack editor items large data
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 char unpackfileDat[50];
 char outputDir[50];
 strcpy(unpackfileDat, "extract/editor/hspr0-0-unpacked.dat");
 strcpy(outputDir, "extract/editor/hspr0-0-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfileDat, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 remove(unpackfileDat);

 return true;
}

//extracts the Editor items small in data\mspr0-0.dat and data\mspr0-0.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractEditorItemsSmall() {
 //read Editor items small
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\mspr0-0.dat
 //data\mspr0-0.tab
 //Unknown format 	RNC-compressed = Yes 	Editor icons (small)

 //unpack data file
 char packfile[35];
 char unpackfile[50];
 char tabfile[35];
 strcpy(packfile, "originalgame/data/mspr0-0.dat");
 strcpy(tabfile, "originalgame/data/mspr0-0.tab");
 strcpy(unpackfile, "extract/editor/mspr0-0-unpacked.dat");

 //RNC unpack editor items small data
 int unpack_res = main_unpack(packfile, unpackfile);

 if (unpack_res != 0) {
     return false;
 }

 char unpackfileDat[50];
 char outputDir[50];
 strcpy(unpackfileDat, "extract/editor/mspr0-0-unpacked.dat");
 strcpy(outputDir, "extract/editor/mspr0-0-");

 //extract images to BMP from DAT/TAB file
 int extract_res = ExtractImages (unpackfileDat, tabfile, palette, outputDir);

 if (extract_res != 0) {
     return false;
 }

 remove(unpackfile);

 return true;
}

//extracts the Ingame Textures Atlas in data\tex0-0.dat and data\tex0-0.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractModelTextures() {
 //read Ingame textures atlas tex0-0
 //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
 //data\tex0-0.dat
 //data\tex0-0.tab
 //Is a Raw image file RNC-compressed = No 	In game textures 256x768

 //this is the defined size of the
 //texture Atlas in the Game
 modelTexAtlasSize.Width = 256;
 modelTexAtlasSize.Height = 768;

 char packfile[65];
 char tabfile[65];
 char outputfile[50];
 strcpy(packfile, "originalgame/objects/data/tex0-0.dat");
 strcpy(tabfile, "originalgame/objects/data/tex0-0.tab");
 strcpy(outputfile, "extract/models/tex0-0.png");

 ConvertRawImageData(packfile, palette, modelTexAtlasSize.Width, modelTexAtlasSize.Height,
                     outputfile, 1.0, false);

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
 int retcode=read_tabfile_data(this->modelsTabFileInfo, tabfile, false);
 if (retcode!=0)
   switch (retcode)
     {
       case 1:
          printf("\nError - Cannot open TAB file: %s\n", tabfile);
          return false;
       default:
          printf("\nError - Loading TAB file %s returned fail code %d\n",tabfile,retcode);
          return false;
     }

 /*char exportDir[50];
 char exportFileName[25];
 strcpy(exportDir, "extract/models/");
 strcpy(exportFileName, "dbgtex-");

 //the next debugging command allows to write a CSV file
 //which contains the listing of all contained pictures described
 //in the TAB file. For each picture the offset, width and height is
 //shown
 DebugWriteTabFileContentsCsvTable(tabfile, this->modelsTabFileInfo);

 //the next line allows to seperate the original texture Atlas
 //into seperate PNG images on the harddisc. This could be interesting
 //for debugging purposes.
 DebugSplitModelTextureAtlasAndWriteSingulatedPictures(outputfile, exportDir, exportFileName, this->modelsTabFileInfo);*/

 return true;
}

//extracts the Tmaps data in data\tmaps.dat and data\tmaps.tab
//returns true in case of success, returns false in case of unexpected error
bool PrepareData::ExtractTmaps() {
    //read Tmaps data (contains collectible items, powerups...)
    //info please see https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //data\tmaps.dat
    //data\tmaps.tab
    //RNC-compressed = Yes 	Tmaps data (contains collectible items, powerups...)

   char ArchiveName[55];
   strcpy(ArchiveName, "originalgame/data/tmaps.dat");

   //Information on https://moddingwiki.shikadi.net/wiki/Hi_Octane
   //Many of the game's files are compressed using the Rob Northern Compression format;
   //some of them such as SOUND\SOUND.DAT are a concatenation of many RNC archives simply glued together.

   //first seperate data in multiple export files, new file always starts with "RNC" magic characters
   FILE* iFile;
   FILE* oFile = NULL;

   iFile = fopen(ArchiveName, "rb");
   fseek(iFile, 0L, SEEK_END);
   size_t size = ftell(iFile);
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
       return false;
   }

   //now seperate data into different new created file
   counter = 0;
   unsigned long ofilenr = 0;

   while (counter < size) {
     if (((size - counter) > 4) && (ByteArray[counter] == 'R') && (ByteArray[counter+1] == 'N')
           && (ByteArray[counter+2] == 'C') && (ByteArray[counter+3] == 1)) {

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
              delete[] ByteArray;
              return false;
           }
     }
      if (oFile != NULL) {
        fwrite(&ByteArray[counter], 1, 1, oFile);
      }
      counter++;
   }

   //close last file
   fclose(oFile);

   delete[] ByteArray;

   //RNC unpack all the sound files
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

       //RNC unpack data
       int unpack_res = main_unpack(finalpath, finalpathUnpacked);

       if (unpack_res != 0) {
           return false;
       }
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
        ConvertTMapImageData(finalpathUnpacked, finalpath, 2.0);
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

   return true;
}

//extracts sound files from sound.data
bool PrepareData::ExtractSounds() {
   char ArchiveName[55];
   strcpy(ArchiveName, "originalgame/sound/sound.dat");

   //Information on https://moddingwiki.shikadi.net/wiki/Hi_Octane
   //Many of the game's files are compressed using the Rob Northern Compression format;
   //some of them such as SOUND\SOUND.DAT are a concatenation of many RNC archives simply glued together.

   //first seperate data in multiple export files, new file always starts with "RNC" magic characters
   FILE* iFile;
   FILE* oFile = NULL;

   iFile = fopen(ArchiveName, "rb");
   fseek(iFile, 0L, SEEK_END);
   size_t size = ftell(iFile);
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
       return false;
   }

   //now seperate data into different new created file
   counter = 0;
   unsigned long ofilenr = 0;

   while (counter < size) {
     if (((size - counter) > 4) && (ByteArray[counter] == 'R') && (ByteArray[counter+1] == 'N')
           && (ByteArray[counter+2] == 'C') && (ByteArray[counter+3] == 1)) {

         //new file start found

           char finalpath[50];
           char fname[20];
           strcpy(finalpath, "extract/sound/sound-packed");
           sprintf (fname, "%0*lu.dat", 4, ofilenr);
           strcat(finalpath, fname);
           ofilenr++;

           if (oFile != NULL) {
               //close last file
               fclose(oFile);
           }

           oFile = fopen(finalpath, "wb");
           if (oFile == NULL) {
              delete[] ByteArray;
              return false;
           }
     }

      fwrite(&ByteArray[counter], 1, 1, oFile);
      counter++;
   }

   //close last file
   fclose(oFile);

   delete[] ByteArray;

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

       //RNC unpack data
       int unpack_res = main_unpack(finalpath, finalpathUnpacked);

       if (unpack_res != 0) {
           return false;
       }
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

   return true;
}

//Reads file format with sound file information, Returns all available SOUNDFILEENTRIES in entries
//Returns true when successful, False otherwise
bool PrepareData::ReadSoundFileEntries(char* filename, std::vector<SOUNDFILEENTRY> *entries) {
    //Information on https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //see under MUSIC.DAT, structure seems to also apply to sound files

    FILE* iFile;
    unsigned int EntryNumber = 0;

    iFile = fopen(filename, "rb");
    fseek(iFile, 0L, SEEK_END);
    size_t size = ftell(iFile);
    fseek(iFile, 0L, SEEK_SET);

    //calculate amount of items to read from file
    unsigned long itemnr = size / sizeof(SOUNDFILEENTRY);

    SOUNDFILEENTRY *newItem;

    if (iFile != NULL)
    {
        do {
           newItem = (SOUNDFILEENTRY*)malloc(sizeof(SOUNDFILEENTRY));
           fread(newItem, sizeof(SOUNDFILEENTRY), 1, iFile);
           entries->push_back(*newItem);
           free(newItem);
           EntryNumber++;
        } while (EntryNumber < itemnr);
        fclose(iFile);
    } else {
        entries = NULL;
        return false;
    }

    return true;
}

//further split sound files
bool PrepareData::SplitSoundFile(char* filename, char *ofilename, std::vector<SOUNDFILEENTRY> *entries) {
   //after ExtractSounds routine executed we get files which contain multiple WAV files
   //glued together, we need to split them further

   //first seperate data in multiple export files, new file always starts with "RIFF" magic characters
   FILE* iFile;
   FILE* oFile = NULL;

   iFile = fopen(filename, "rb");
   fseek(iFile, 0L, SEEK_END);
   size_t size = ftell(iFile);
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
       return false;
   }

   //now seperate data into different new created file
   counter = 0;

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
                    delete[] ByteArray;
                    return false;
                }

                lastpos = (*it).offsetTune + (*it).tuneLenBytes;

                for (counter = (*it).offsetTune; counter < lastpos; counter++) {
                    fwrite(&ByteArray[counter], 1, 1, oFile);
                }

                //close last file
                fclose(oFile);
            }
    }

   delete[] ByteArray;

   return true;
}

bool PrepareData::ExtractMusicFiles(char* outputNameStr, FILE *iFile,
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
             return false;
         }

         //go to start of song file data
         fseek(iFile, VecMusicTableEntries->offTune1 + (*it2).offsetTune, SEEK_SET);

         //get length of this tune
         dataCnt = (*it2).tuneLenBytes;

         unsigned char *buffer = static_cast<unsigned char*>(malloc(dataCnt));

         if (buffer == NULL) {
             printf(" Error - cannot allocate %lu bytes of memory.\n",(unsigned long)(dataCnt));
             return false;
         }

         //copy data
         fread(&buffer[0], dataCnt, 1, iFile);
         fwrite(&buffer[0], dataCnt, 1, oFile);

         //close current output file
         fclose(oFile);

         free(buffer);
     }

     return true;
}

//Extract music files
//Returns true when successful, False otherwise
bool PrepareData::ExtractMusic() {
    //Information on https://moddingwiki.shikadi.net/wiki/Hi_Octane
    //see under MUSIC.DAT

    char filename[65];
    char outputNameStr[50];

    strcpy(filename, "originalgame/sound/music.dat");
    strcpy(outputNameStr, "extract/music");

    FILE* iFile;

    iFile = fopen(filename, "rb");
    fseek(iFile, 0L, SEEK_END);
    size_t size = ftell(iFile);

    //first we need to read INT32LE at EOF, and seek to
    //this position
    fseek(iFile, size - 4, SEEK_SET);

    unsigned long seekPos;

    fread(&seekPos, sizeof(seekPos), 1, iFile);
    fseek(iFile, seekPos, SEEK_SET);

    short int readVal;
    unsigned int N = 0;
    size_t lastSeekPos;

    //now at this location count the number of INT16LE with value 01;
    //lets call the amount of repetitions N
    do {
        lastSeekPos = ftell(iFile);
        fread(&readVal, sizeof(readVal), 1, iFile);
        if (readVal == 0x01)
            N++;
    } while (readVal == 0x01);

    if (N == 0)
        return false;

    //go back to file position before last read
    fseek(iFile, lastSeekPos, SEEK_SET);

    std::vector<MUSICTABLEENTRY> VecMusicTableEntries;

    MUSICTABLEENTRY *newTableEntry;

    //now read N times the following struct with tune offset information
    for (unsigned int cnt = 0; cnt < N; cnt++) {
         newTableEntry = (MUSICTABLEENTRY*)malloc(sizeof(MUSICTABLEENTRY));
         fread(newTableEntry, sizeof(MUSICTABLEENTRY), 1, iFile);
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

        fseek(iFile, (it)->offTunes, SEEK_SET);
        fread(newTuneInformation, sizeof(SOUNDFILEENTRY), 1, iFile);

        //in newTuneInformation we received now a value for lenTune
        //now we continue reading this kind of struct, until the sum of all of
        //their lenTune fields is identical to the first read one before
        targetLenTuneSum = newTuneInformation->tuneLenBytes;
        free(newTuneInformation);

        while (targetLenTuneSum > 0) {
            newTuneInformation = (SOUNDFILEENTRY*)malloc(sizeof(SOUNDFILEENTRY));
            fread(newTuneInformation, sizeof(SOUNDFILEENTRY), 1, iFile);
            VecTuneInformation.push_back(*newTuneInformation);
            targetLenTuneSum -= newTuneInformation->tuneLenBytes;
            free(newTuneInformation);
        }

        //extract all available tune files
        if (!ExtractMusicFiles(outputNameStr, iFile, &(*it), VecTuneInformation))
            return false;

        //clear list again to make room for next file round
        VecTuneInformation.clear();
     }

    return true;
}

//The following routine uses the flifix source code,
//repairs the original games intro.dat file, and then
//extracts the intro in single frames that we can then
//load and play using Irrlicht
bool PrepareData::PrepareIntro() {
    //Variables initiation
    FILE *animFile;
    FILE *destFile;
    //Frames are numbered from 0
    ulong startFrame=0;
    ulong endFrame=ULONG_MAX;
    //Options description is avaible in the file where they are defined
    int globOptions = 0;

    char filename[65];
    char outputNameStr[50];

    strcpy(filename, "originalgame/data/intro.dat");
    strcpy(outputNameStr, "extract/intro/intro.fli");

    char *srcFName = filename;
    char *destFName = outputNameStr;

    //we need this option to succesfully repair
    //the original games fli movie file
    globOptions |= poFixMainHeader;

    //Opening the files
    animFile = openSourceFLIFile(srcFName);
    if (animFile == NULL)
        return false;

    destFile = openDestinationFLIFile(destFName);
    if (destFile == NULL) {
        fclose(animFile);
        return false;
    }

    //File analyst
    if (globOptions & poSimpleFix)
      processFLISimple(animFile,destFile,startFrame,endFrame, & globOptions);
     else
      processFLIFile(animFile,destFile,startFrame,endFrame, globOptions);

    //Closing files
    closeFLIFiles(animFile,destFile,globOptions);

    //now load the FLI file and create for each frame a texture file for later
    FILE* f = std::fopen(outputNameStr, "rb");
    flic::StdioFileInterface file(f);
    flic::Decoder decoder(&file);
    flic::Header header;
    if (!decoder.readHeader(header)) {
       return false;
    }

     std::vector<uint8_t> buffer(header.width * header.height);
     flic::Frame frame;
     frame.pixels = &buffer[0];
     frame.rowstride = header.width;

     char outFrameFileName[50];
     char fname[20];

     for (long i = 0; i < header.frames; ++i) {
       if (!decoder.readFrame(frame)) {
         //return 3;
         return false;
       } else {
           //process the current decoded frame data in buffer
           char* frameData = new char[buffer.size()];
           std::copy(buffer.begin(),buffer.end(), frameData);

           //create the filename for the picture output file
           strcpy(outFrameFileName, "extract/intro/frame");
           sprintf (fname, "%0*lu.png", 4, i);
           strcat(outFrameFileName, fname);

           //original frame size is 320x200, scale with factor of 2 to get frames with 640 x 400
           if (!ConvertIntroFrame(frameData, frame.colormap, header.width, header.height, outFrameFileName, 2.0, false)) {
               delete [] frameData;

               //there was an error
               return false;
           }

           delete[] frameData;
       }
     }

    return true;
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
