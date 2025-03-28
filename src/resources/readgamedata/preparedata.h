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
#include <irrlicht.h>
#include "../../utils/fileutils.h"
#include "objectdatfile.h"
#include "../xbrz-1-8/xbrz.h"
#include "../intro/flic.h"
#include <cstdint>

//each SOUNDFILEENTRY has 32 bytes
typedef struct {
        char soundFilename[12];
        char padding1[6];
        uint32_t offsetTune;
        char padding2[4];
        uint32_t tuneLenBytes;
        int16_t unknown;
     } SOUNDFILEENTRY;

//each MUSICTABLEENTRY has 16 bytes
typedef struct {
        uint32_t offTunes; //offset to the list of tunes
        uint32_t offTune1;   //offset to the first tune
        uint32_t unknown;  //unknown (always 0xC0000000, or 192)
        uint32_t AllTunesLenBytes;
     } MUSICTABLEENTRY;

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
    std::tuple<unsigned char, unsigned char, unsigned char> GetPaletteColor(unsigned char colorIndex);

    uint32_t read_uint32_le_file (FILE *fp);
    void ReadSoundFileEntry(FILE* inputFile, SOUNDFILEENTRY* entry);
    void ReadMusicFileEntry(FILE* inputFile, MUSICTABLEENTRY* entry);

    void ExtractGameScreens();
    void ExtractFonts();
    void ExtractHuds();
    void ExtractSkies();
    void ExtractSprites();
    void ExtractMiniMaps();
    void ExtractTerrainTextures();
    void ExtractLevels();
    void ExtractEditor();
    void ExtractCheatPuzzle();
    void ExtractModels();
    void ExtractIntro();
    void ExtractAudio();
    void ExtractUserdata();

    //Tabfile information for model texture atlas file
    //this information is needed to be able to export
    //the objects in wavefront object file
    TABFILE *modelsTabFileInfo = NULL;
    irr::core::dimension2d<irr::f32> modelTexAtlasSize;

    void Extract3DModel(const char* srcFilename, const char* destFilename, const char* objName);
    void ExtractNamed3DModel(const char* name, int n_models);
    void Extra3DModels();

    //extracts the SVGA game logo data in data\logo0-1.dat and data\logo0-1.tab
    //raises an error message in case of unexpected error
    void ExtractGameLogoSVGA();

    //extracts the SVGA Large white font data in data\olfnt0-1.dat and data\olfnt0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractLargeFontSVGA();

    //extracts the SVGA Loading Screen in data\onet0-1.dat and data\onet0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractLoadingScreenSVGA();

    //extracts the SVGA HUD for 1 Player in data\panel0-1.dat and data\panel0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractHUD1PlayerSVGA();

    //extracts the SVGA HUD for 2 Players in data\panel0-0.dat and data\panel0-0.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractHUD2PlayersSVGA();

    //extracts the SVGA Selection Screen in data\oscr0-1.dat and data\oscr0-1.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractSelectionScreenSVGA();

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

    //extracts the introductory screen (in format 320x200) in data\title.dat and data\title.tab
    //* is from 0 up to 5
    //throw error message in case of unexpected error
    void ExtractIntroductoryScreen();

    //extracts the Ingame Textures Atlas in data\tex0-0.dat and data\tex0-0.tab
    //returns true in case of success, returns false in case of unexpected error
    void ExtractModelTextures();

    void ExtractSmallFontSVGA();

    void ConvertObjectTexture(char* rawDataFilename, char* outputFilename, int scaleFactor);
    bool DebugSplitModelTextureAtlasAndWriteSingulatedPictures(
            char *atlasFileName, char* exportDir, char* outputFileName, TABFILE *tabf);

    //extracts sound files from sound.data
    void ExtractSounds();

    //further split sound files
    void SplitSoundFile(const char* filename, const char *ofilename, std::vector<SOUNDFILEENTRY> *entries);

    //Reads file format with sound file information, Returns all available SOUNDFILEENTRIES in entries
    //Returns true when successful, False otherwise
    void ReadSoundFileEntries(const char* filename, std::vector<SOUNDFILEENTRY> *entries);

    void ExtractCompressedImagesFromDataFile(const char* basename, const char* outdir);
    void ConvertCompressedImageData(const char* packfile, const char* outfile, irr::u32 sizex, irr::u32 sizey, int scaleFactor=1);

    //writes raw video data into picture file, using specified palette file
    void ConvertRawImageData(const char* rawDataFilename, irr::u32 sizex, irr::u32 sizey,
                             const char* outputFilename, int scaleFactor = 1);

    void ExtractTmaps();

    bool ConvertTMapImageData(char* rawDataFilename, char* outputFilename, int scaleFactor);

    void PrepareIntro();
    void ConvertIntroFrame(unsigned char* ByteArray, flic::Colormap colorMap, irr::u32 sizex, irr::u32 sizey,
                           char* outputFilename, int scaleFactor);

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

    irr::video::IImage* UpscaleImage(irr::video::IImage *srcImg, irr::u32 sizex, irr::u32 sizey, int scaleFactor);

    irr::video::IImage* loadRawImage(const char* rawDataFilename, irr::u32 sizex, irr::u32 sizey);
    void saveIrrImage(const char* outputFilename, irr::video::IImage* img);
};

#endif // PREPAREDATA_H
