/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PREPAREDATA_H
#define PREPAREDATA_H

// Class to extract game data files
//

#include <string.h>
#include <vector>
#include "dernc.h"
#include "xtabdat8.h"
#include "stdlib.h"
#include <irrlicht/irrlicht.h>
#include "../../utils/fileutils.h"
#include "objectdatfile.h"
#include "../xbrz-1-8/xbrz.h"
#include "../intro/flic.h"

//Never remove packed attribute below, because
//otherwise read from File into this struct will
//fail with wrong data!
//this attribute prevents C struct padding!
typedef struct {
    char soundFilename[12];
    char padding1[6];
    unsigned int offsetTune;
    char padding2[4];
    unsigned int tuneLenBytes;
    short int unknown;
       } __attribute__((packed)) SOUNDFILEENTRY;

//Never remove packed attribute below, because
//otherwise read from File into this struct will
//fail with wrong data!
//this attribute prevents C struct padding!
typedef struct {
    unsigned int offTunes; //offset to the list of tunes
    unsigned int offTune1; //offset to the first tune
    unsigned int unknown;  //unknown (always 0xC0000000, or 192)
    unsigned int AllTunesLenBytes;
       } __attribute__((packed)) MUSICTABLEENTRY;

class PrepareData {

public:

    PrepareData(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver);
    ~PrepareData();

private:
    irr::video::IVideoDriver* myDriver;
    irr::IrrlichtDevice* myDevice;

    //contains the ingame palette information
    unsigned char *palette;

    void ReadPaletteFile(char *palFile, unsigned char* paletteDataOut);

    void ExtractLevels();

    //Tabfile information for model texture atlas file
    //this information is needed to be able to export
    //the objects in wavefront object file
    TABFILE *modelsTabFileInfo = NULL;
    irr::core::dimension2d<irr::f32> modelTexAtlasSize;

    void Extract3DModel(const char* srcFilename, const char* destFilename, const char* objName);
    void Extra3DModels();

    //return true if all expected files are present
    //return false if at least one file is missing
    bool CheckForGameLogoSVGAFiles();

    //extracts the SVGA game logo data in data\logo0-1.dat and data\logo0-1.tab
    //raises an error message in case of unexpected error
    void ExtractGameLogoSVGA();

    //extracts the SVGA Large white font data in data\olfnt0-1.dat and data\olfnt0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractLargeFontSVGA();

    //return true if all expected files are present
    //return false if at least one file is missing
    bool CheckForScreenSVGA();

    //extracts the SVGA Loading Screen in data\onet0-1.dat and data\onet0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractLoadingScreenSVGA();

    //extracts the SVGA HUD for 1 Player in data\panel0-1.dat and data\panel0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractHUD1PlayerSVGA();

    //extracts the SVGA HUD for 2 Players in data\panel0-0.dat and data\panel0-0.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractHUD2PlayersSVGA();

    //return true if selection screen (main menue background) is present
    //return false if at least one file is missing
    bool CheckForSelectionScreenSVGA();

    //extracts the SVGA Selection Screen in data\oscr0-1.dat and data\oscr0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractSelectionScreenSVGA();

    // Extract all sky images. Throws an error message in case of errors
    void ExtractSkies();

    // Extract a single sky image. Throws an error message in case of errors
    void ExtractSky(char skyNr);

    //This helper function modifies the original sky image so that we can use it easier
    //in this project. The result is stored in another new image file
    void ModifySkyImage(const char *origSkyFileName, const char* outputModifiedSkyFileName);

    //extracts the SVGA Minimaps in data\track0-1.dat and data\track0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractMiniMapsSVGA();

    //helper function to stich the original games minimap
    //parts together
    void StitchMiniMaps();

    //void ReorganizeTerrainAtlas(char* targetFile, char* outputFileName);

    //throws an error message in case of unexpected problem
    void ExportTerrainTextures(const char* targetFile, const char* exportDir, const char* outputFileName);

    //extracts the Terrain Textures (Texture Atlas) (in format 64×16384) in data\textu0-*.dat and data\textu0-*.tab
    //* is from 0 up to 5
    //returns true in case of success, returns false in case of unexpected error
    void ExtractTerrainTextures();

    // extract the Terrain Texture (Texture Atlas) for one level
    void ExtractTerrainTexture(char levelNr);

    //extracts the SVGA thin white font data in data\hfont0-0.dat and data\hfont0-0.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractThinWhiteFontSVGA();

    //extracts the Editor items large in data\hspr0-0.dat and data\hspr0-0.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractEditorItemsLarge();

    //extracts the Editor items small in data\mspr0-0.dat and data\mspr0-0.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractEditorItemsSmall();

    //extracts the SVGA Large Green font in data\pfont0-1.dat and data\pfont0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractLargeGreenFontSVGA();

    //extracts the Editor cursors data in data\point0-0.dat and data\point0-0.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractEditorCursors();

    //extracts the Cheat puzzle in data\puzzle.dat and data\puzzle.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractCheatPuzzle();

    //extracts the introductory screen (in format 320x200) in data\title.dat and data\title.tab
    //* is from 0 up to 5
    //throw error message in case of unexpected error
    void ExtractIntroductoryScreen();

    //extracts the Ingame Textures Atlas in data\tex0-0.dat and data\tex0-0.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractModelTextures();

    void ExtractSmallFontSVGA();

    bool ConvertObjectTexture(char* rawDataFilename, char* outputFilename, int scaleFactor);
    bool DebugSplitModelTextureAtlasAndWriteSingulatedPictures(
            char *atlasFileName, char* exportDir, char* outputFileName, TABFILE *tabf);

    //extracts sound files from sound.data
    void ExtractSounds();

    //further split sound files
    void SplitSoundFile(const char* filename, const char *ofilename, std::vector<SOUNDFILEENTRY> *entries);

    //Reads file format with sound file information, Returns all available SOUNDFILEENTRIES in entries
    //Returns true when successful, False otherwise
    void ReadSoundFileEntries(const char* filename, std::vector<SOUNDFILEENTRY> *entries);

    //writes raw video data into picture file, using specified palette file
    void ConvertRawImageData(const char* rawDataFilename, unsigned char *palette, irr::u32 sizex, irr::u32 sizey,
                             const char* outputFilename, int scaleFactor = 1.0, bool flipY = false);

    void ExtractTmaps();

    bool ConvertTMapImageData(char* rawDataFilename, char* outputFilename, int scaleFactor);

    void PrepareIntro();
    bool ConvertIntroFrame(char* ByteArray, flic::Colormap colorMap, irr::u32 sizex, irr::u32 sizey,
                                          char* outputFilename, int scaleFactor, bool flipY);

    void ExtractMusic();

    void ExtractMusicFiles(const char* outputNameStr, FILE *iFile,
           MUSICTABLEENTRY* VecMusicTableEntries,
           std::vector<SOUNDFILEENTRY> VecTuneInformation);

    bool ReplacePixelColor(irr::video::IImage* image, irr::video::SColor originalColor, irr::video::SColor newColor);
    bool DetectFontCharacterColor(irr::video::IImage* image, irr::video::SColor* detectedColor);

    void CreateFontForUnselectedItemsInMenue(const char* sourceFntFileName, const char* destFntFileName,
              irr::u32 fileNameNumOffset, irr::u32 numberCharacters);

    //splits additional level textures for levels 7 up to 9
    //Important note: This data is not extracted from the original
    //game files, as I do not have this data
    void AddOtherLevelsHiOctaneTools();
    void SplitHiOctaneToolsAtlas(char* targetFile, char* exportDir, char* outputFileName);
};

#endif // PREPAREDATA_H
