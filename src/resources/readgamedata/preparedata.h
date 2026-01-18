/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PREPAREDATA_H
#define PREPAREDATA_H

// Class to extract game data files
//

#include <irrlicht.h>
#include <cstdint>
#include <string>
#include <vector>
#include "../intro/flic.h"
#include "../readgamedata/xtabdat8.h"

#define PREP_DATA_INITSTATE 0
#define PREP_DATA_EXTRACTGAMESCREENS 1
#define PREP_DATA_EXTRACTFONTS 2
#define PREP_DATA_EXTRACTHUD 3
#define PREP_DATA_EXTRACTSKIES 4
#define PREP_DATA_EXTRACTSPRITES 5
#define PREP_DATA_EXTRACTTERRAINTEXTURES 6
#define PREP_DATA_EXTRACTLEVELS 7
#define PREP_DATA_EXTRACTMINIMAPS 8
#define PREP_DATA_EXTRACTMODELS 9
#define PREP_DATA_EXTRACTINTRO 10
#define PREP_DATA_EXTRACTAUDIO 11
#define PREP_DATA_EXTRACTMISC 12
#define PREP_DATA_FINISHED 13

#define OBJ_EXPORT_DEF_UVMAP_NORMAL 0
#define OBJ_EXPORT_DEF_UVMAP_FLIPU 1
#define OBJ_EXPORT_DEF_UVMAP_FLIPV 2
#define OBJ_EXPORT_DEF_UVMAP_FLIPUV 3

/************************
 * Forward declarations *
 ************************/

class InfrastructureBase;

class ObjTexModification {
public:
  ObjTexModification(int applyToTexIdParam, uint8_t texModTypeParam);

  int mApplyToTexId;
  uint8_t mTexModType;
};

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

typedef struct FontCharacterPreprocessInfo {
    //contains the raw image for the character
    irr::video::IImage* image = nullptr;

    //size in pixels of the raw image for character
    //as read from original game export data
    irr::core::dimension2d<irr::s32> sizeRawTex;

    //contains "optimized" area of character, non useful empty black area removed
    //only 1 line of black area left on left and right side
    irr::core::rect<irr::s32> charRect;

    //contains the transparent color found for this
    //character
    irr::video::SColor transColor;

    //contains the found character colors itself
    //are all colors that are not background colors
    //after upscaling multiple colors are possible
    std::vector<irr::video::SColor> charColorVec;
} FontCharacterPreprocessInfo;

class PrepareData {

public:
    PrepareData(InfrastructureBase* mInfraPntr);
    ~PrepareData();

    InfrastructureBase* mInfra = nullptr;

    //returns true if data preparation was succesfully
    //finished
    bool ExecuteNextStep();
    std::string currentStepDescription;

    //returns true if game data is already available
    //false otherwise if further extraction is needed
    bool GameDataAvailable();

    irr::u8 GetProgressBarNrBlocksFilled(irr::u8 overallNrBlocksBar);

    std::tuple<unsigned char, unsigned char, unsigned char> GetPaletteColor(unsigned char colorIndex);

    //void PrepareMapConfigDataFile(const char* targetFileName, const char* targetSkyFilePath, const char* targetMusicFilePath);

private:
    irr::u8 mCurrentStep = PREP_DATA_INITSTATE;

    //contains the ingame palette information
    unsigned char *palette = nullptr;

    void ReadPaletteFile(char *palFile, unsigned char* paletteDataOut);

    uint32_t read_uint32_le_file (FILE *fp);
    void ReadSoundFileEntry(FILE* inputFile, SOUNDFILEENTRY* entry);
    void ReadMusicFileEntry(FILE* inputFile, MUSICTABLEENTRY* entry);

    void CreatePalette();

    void ExtractImagesfromDataFile(const char* datfname, const char* tabfname, const char* outputDir);
    void UnpackDataFile(const char* packfile, const char* unpackfile);
    std::vector<unsigned char> loadRawFile(const char *filename);

    irr::u32 currSubStep;
    irr::u32 nrSubSteps;

    void ExtractInitialData();

    //The following methods return true if their work
    //is done, false otherwise
    bool ExtractGameScreens();
    bool ExtractFonts();
    bool ExtractHuds();
    bool ExtractSkies();
    bool ExtractSprites();
    bool ExtractMiniMaps();
    bool ExtractTerrainTextures();
    bool ExtractLevels();
    bool ExtractMisc();
    void ExtractCheatPuzzle();
    bool ExtractModels();
    bool ExtractIntro();
    bool ExtractAudio();
    void ExtractUserdata();

    //extracts sound files from sound.data
    bool ExtractSounds();

    void MoveIndexedFilesToNewLocation(const char* srcPath, const char* srcPrefix, irr::u16 srcStartIdx,
                                                    irr::u16 srcNrFiles, const char* targetPath, irr::u16 targetStartIdx);
    void PrepareHudFontsLocation();

    std::vector<ObjTexModification*> mModelTexModificationVec;

    //Tabfile information for model texture atlas file
    //this information is needed to be able to export
    //the objects in wavefront object file
    TABFILE *modelsTabFileInfo = nullptr;
    irr::core::dimension2d<irr::f32> modelTexAtlasSize;

    void Extract3DModel(const char* srcFilename, const char* destFilename, const char* objName);
    void ExtractNamed3DModel(const char* name, int n_models);

    void UpscaleExistingImageFile(const char* srcFile, const char* destFile, int scaleFactor);

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
    void ExportTerrainTextures(const char* targetFile, const char* exportDir, const char* outputFileName, int scaleFactor = 1);

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

    void PrepareMapConfigData();

    void ConvertObjectTexture(char* rawDataFilename, char* outputFilename, int scaleFactor);
    bool DebugSplitModelTextureAtlasAndWriteSingulatedPictures(
            char *atlasFileName, char* exportDir, char* outputFileName, TABFILE *tabf);

    //further split sound files
    void SplitSoundFile(const char* filename, const char *ofilename, std::vector<SOUNDFILEENTRY> *entries);

    //Reads file format with sound file information, Returns all available SOUNDFILEENTRIES in entries
    //Returns true when successful, False otherwise
    void ReadSoundFileEntries(const char* filename, std::vector<SOUNDFILEENTRY> *entries);

    void ExtractCompressedImagesFromDataFile(const char* datFileName, const char* tabFileName, const char* outdir);
    void ConvertCompressedImageData(const char* packfile, const char* outfile, irr::u32 sizex, irr::u32 sizey, int scaleFactor=1);

    //writes raw video data into picture file, using specified palette file
    void ConvertRawImageData(const char* rawDataFilename, irr::u32 sizex, irr::u32 sizey,
                             const char* outputFilename, int scaleFactor = 1);

    void UpscaleAllImagesInDirectory(const char* srcDir, const char* srcFilePrefix, const char* targetDir, int scaleFactor);

    void ExtractTmaps();

    bool ConvertTMapImageData(char* rawDataFilename, char* outputFilename, int scaleFactor);

    //Variables for Intro
    //Processing
    flic::StdioFileInterface* mFLIFileInterface = nullptr;
    flic::Decoder* mFLIDecoder = nullptr;
    flic::Header* mFLIHeader = nullptr;
    flic::Frame* mFLIFrame = nullptr;
    std::vector<uint8_t>* mFLIBuffer = nullptr;

    void InitFLIProcessing(const char* inputFile);
    void CleanupFLIProcessing();
    void RepairFLI(const char* outputFLIFileName);
    void ProcessFrame(long currFrameNr);
    void ConvertIntroFrame(unsigned char* ByteArray, flic::Colormap colorMap, irr::u32 sizex, irr::u32 sizey,
                           char* outputFilename, int scaleFactor);

    void SplitSoundDatFile();
    unsigned long mNrSoundFiles;
    void RNCUnpackSoundData();

    void ExtractMusic();

    void ExtractMusicFiles(const char* outputNameStr, FILE *iFile,
           MUSICTABLEENTRY* VecMusicTableEntries,
           std::vector<SOUNDFILEENTRY> VecTuneInformation);

    bool ReplacePixelColor(irr::video::IImage* image, irr::video::SColor originalColor, irr::video::SColor newColor);
    bool DetectFontCharacterColor(irr::video::IImage* image, irr::video::SColor* detectedColor);

    void CreateFontForUnselectedItemsInMenue(const char* sourceFntFileName, const char* destFntFileName,
              irr::u32 fileNameNumOffset, irr::u32 numberCharacters, bool cleanUp = true);

    //splits additional level textures for levels 7 up to 9
    //Important note: This data is not extracted from the original
    //game files, as I do not have this data
    void AddOtherLevelsHiOctaneTools();
    void SplitHiOctaneToolsAtlas(char* targetFile, char* exportDir, char* outputFileName);

    irr::video::IImage* UpscaleImage(irr::video::IImage *srcImg, irr::u32 sizex, irr::u32 sizey, int scaleFactor);

    irr::video::IImage* loadRawImage(const char* rawDataFilename, irr::u32 sizex, irr::u32 sizey);
    void saveIrrImage(const char* outputFilename, irr::video::IImage* img);

    void ModifyPixelForYellowSky(irr::f32 &red, irr::f32 &green, irr::f32 &blue);
    void ModifyPixelForGreySky(irr::f32 &red, irr::f32 &green, irr::f32 &blue);
    void ModifyPixelForBlackSky(irr::f32 &red, irr::f32 &green, irr::f32 &blue);

    //Returns true in case of success, False otherwise
    bool CreateMapConfigFile(const char* filename, irr::u8 levelNr);

    //Creates other colored sky dome images (needed for improved sky, not vanilla)
    //In case of an unexpected error this function returns false, True otherwise
    //colorSelector == 1 => creates yellow sky
    bool CreateColoredSkydomeImage(const char* srcImagePath, const char* targetImagePath,
                                 int colorSelector);

    void PrepareUpgradedSkyData();

    //Stuff for font preprocessing
    void FindCharArea(FontCharacterPreprocessInfo* character);
    bool AddColoredOutline(FontCharacterPreprocessInfo& character, irr::video::SColor* outLineColor);

    irr::video::SColor DeriveTransparentColorForChar(FontCharacterPreprocessInfo& character);
    void AddPixelToColorOccurenceList(std::vector<std::pair <irr::u8, irr::video::SColor>>& colorOccurenceList,
        irr::video::SColor newColor);
    void DeriveCharColorsForChar(FontCharacterPreprocessInfo& character);

    //Returns true in case of success, False otherwise
    bool StorePreProcessedFontCharacter(FontCharacterPreprocessInfo& character, const char* outFileName);
    void PreProcessFontDirectory(const char* fontDirName, const char* srcFilePrefix, bool addOutline,
                                 irr::video::SColor* outLineColor, bool cleanUp = true);

    irr::video::SColor* fontOutLineColor;
    irr::video::SColor* fontOutLineColor2;
};

#endif // PREPAREDATA_H
