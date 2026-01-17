/*
 * xtabdat8.h   define exported routines from xtabdat8.c
 */

#ifndef BULL_XTABDAT8_H
#define BULL_XTABDAT8_H

#include "bulcommn.h"
#include "vector"
#include "irrlicht.h"

//source code taken from http://syndicate.lubiki.pl/downloads/bullfrog_utils_tabdat.zip
//for readme file please see bullfrog_utils_tabdat-readme.txt
//DK Utilities author (8bpp extraction code):
//Jon Skeet

//Syndicale level viewer author (4bpp extraction code):
//Andrew Sampson

//Dev-C++ IDE version, modifications:
//Tomasz Lis

//One own routine added for debugging (DebugWriteTabFileContentsCsvTable), and one small modification necessary
//in routine read_tabfile_data to make 3D Model export work correctly
//Therefore this source code is not full original anymore!

//Note 19.03.2025: After I ran into runtime issues in windows (using MSVC during compilation), and because I received many warnings
// about possible data loss due to type conversions, uninitialized variables and so on in Visual Studio, I decided to modify many parts of the
//original code below. I wanted to use variable types now with fixed defined bit lengths.

//Note 10.01.2026: I had issues again when trying to load the bitmap files first exported with this original source code
//(the are saved in color format A1R5G5B5 in a .bmp file initially) in  Irrlicht, and then using them in A8R8G8B8 color format
//(and storing them in .png format) afterwards. What happns is that getPixel function in Irrlicht of the loaded image then
//delivers wrong pixel color information. I also come more and more to the conclusion that the multiple image file saving/loading
//during data extraction, data processing and so on is just not practical.
//Because of this I finally decided to modify this source code here, so that instead of writing a bitmap file in the first
//place here, and then load it later again in Irrlicht from the image file, I create the Irrlicht image here immediately.

//Forward declaration
class PrepareData;

// Data types

typedef struct {
    uint32_t offset;
    uint8_t width;
    uint8_t height;
} TABFILE_ITEM;

typedef struct {
    long count;
    unsigned long filelength;
    TABFILE_ITEM* items;
} TABFILE;

typedef struct {
    int16_t count;
    size_t filelength;
    unsigned char* data;
} DATFILE;

typedef struct {
    uint16_t width;
    uint16_t height;
    unsigned char* data;
    unsigned char* alpha;
} IMAGEITEM;

typedef struct {
    long count;
    IMAGEITEM* items;
} IMAGELIST;

// Routines

int read_tabfile_data(TABFILE* tabf,const char* srcfname, bool skipFirstEntry = true);
void free_tabfile_data(TABFILE* tabf);
int read_datfile_data(DATFILE* datf,char* srcfname);
void free_datfile_data(DATFILE* datf);

int read_dattab_images(IMAGELIST* images,unsigned long* readcount,TABFILE* tabf,DATFILE* datf,int verbose);
void free_dattab_images(IMAGELIST* images);
int read_dat_image_idx(IMAGEITEM* image,unsigned long* readedsize,DATFILE* datf, uint32_t off, uint16_t width, uint16_t height);
int create_images_dattab_idx(IMAGELIST* images,char* datfname,char* tabfname,int verbose);

//10.01.2026: Commented out, replaced with ExtractToIrrlichtImages today
//extracts all images within data file into outputDir
//int ExtractImages(char* datfname, char* tabfname, unsigned char* palette, char* outputDir);

//Returns true in case of success, False otherwise
bool ExtractToIrrlichtImages(PrepareData* parent, char* datfname, char* tabfname,
                             std::vector<irr::u16> &imageIndex,
                             std::vector<irr::video::IImage*> &imageOutput);

void DebugWriteTabFileContentsCsvTable(char* tabFileName, TABFILE* tabf);

// Error returns

#define XTABDAT8_COLOUR_LEAK 2
#define XTABDAT8_ENDOFBUFFER 4
#define XTABDAT8_NOMEMORY 8

#endif
