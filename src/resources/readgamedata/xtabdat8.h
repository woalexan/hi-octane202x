/*
 * xtabdat8.h   define exported routines from xtabdat8.c
 */

#ifndef BULL_XTABDAT8_H
#define BULL_XTABDAT8_H

#include "bulcommn.h"

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

// Data types

typedef struct {
    unsigned long offset;
    unsigned int width;
    unsigned int height;
       } TABFILE_ITEM;

typedef struct {
    long count;
    unsigned long filelength;
    TABFILE_ITEM* items;
       } TABFILE;

typedef struct {
    long count;
    unsigned long filelength;
    unsigned char* data;
       } DATFILE;

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char* data;
    unsigned char* alpha;
       } IMAGEITEM;

typedef struct {
    unsigned long count;
    IMAGEITEM* items;
       } IMAGELIST;

// Routines

int read_tabfile_data(TABFILE* tabf,char* srcfname, bool skipFirstEntry = true);
void free_tabfile_data(TABFILE* tabf);
int read_datfile_data(DATFILE* datf,char* srcfname);
void free_datfile_data(DATFILE* datf);

int read_dattab_images(IMAGELIST* images,unsigned long* readcount,TABFILE* tabf,DATFILE* datf,int verbose);
void free_dattab_images(IMAGELIST* images);
int read_dat_image_idx(IMAGEITEM* image,unsigned long* readedsize,DATFILE* datf,unsigned long off,unsigned int width,unsigned int height);
int create_images_dattab_idx(IMAGELIST* images,char* datfname,char* tabfname,int verbose);

//extracts all images within data file into outputDir
int ExtractImages (char* datfname, char* tabfname, unsigned char* palette, char* outputDir);

void DebugWriteTabFileContentsCsvTable(char* tabFileName, TABFILE* tabf);

// Error returns

#define XTABDAT8_COLOUR_LEAK 2
#define XTABDAT8_ENDOFBUFFER 4
#define XTABDAT8_NOMEMORY 8

#endif
