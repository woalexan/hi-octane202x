//source code taken from http://syndicate.lubiki.pl/downloads/bullfrog_utils_flifix.zip
//for readme file please see flifixer-readme.txt

/*
    Program tool structures
    written by Tomasz Lis, Gdansk, Poland 2004-2006
    this software is under GNU license
*/
#ifndef __PrgTools_H
#define __PrgTools_H

#include <stdio.h>

typedef unsigned long ulong;
typedef unsigned int uint;

enum ProgError
    {
    errFileRead=        0x0001,
    errFileWrite=       0x0002,
    errFileOpen=        0x0004,
    errMemAlloc=        0x0008,

    errMainHdr=         0x0010,
    errFrameHdr=        0x0020,
    errChunkHdr=        0x0040,
    errChunkData=       0x0080,
    errAddHdr=          0x0100,
    errFramePosTbl=     0x0200,
    errPlainData=       0x0400,

    errDataNotExist=    0x1000,
    errCannotComplete=  0x2000,
    errOnlyParsing=     0x4000,
    errCritical=        0x8000,
    errImpossible=      0xffff
    };

enum ProgOptions
    {
    poManualSeeking=     0x0001,//Alternate method of getting offsets in files
    poDisplayAllInfo=    0x0002,//Displays a lot of informatons - highly verbose mode
    poExpandPatette=     0x0004,//Internal-set option - changes palette types COLOR256 to COLOR
    poRecostructPatette= 0x0008,//Internal-set option - it enables when frame 1 has no color palette definition
    poFixMainHeader=     0x0010,//Determines if MainHeader will be corrected to real values
    poFixFramePositions= 0x0020,//Determines if frame positions should be adjusted by searching the file around given offset
    poUseFrameFinder=    0x0040,//FrameFinder allows to search for frames in file if on first parsing not all of them has been found
    poFixFrameHeaders=   0x0080,//Without this option, frame repair function just ends itself
    poRadicalFrameHdrFix=0x0100,//It decreases the range of values in FrameHeaders whitch are assumed proper
    poRemoveBadChunks=   0x0200,//If chunk seems to be destroyed, it is removed
    poFixChunkHeaders=   0x0400,//Determines if chunk headers should be corrected
    poSimpleFix=         0x0800,//For tests - only header is fixed, other data only copied
    poNeverWaitForKey=   0x1000,//Never asks for pressing a key to continue
    poIgnoreExceptions=  0x2000,//Internal-use option - will not show errors when they occure
    poNeverSkipFrames=   0x4000 //With this option, frames witch looks bad won't be removed
    };

#define kbEnter 0x0d
#define kbEscape 27

typedef struct
  {
     unsigned int low;
     unsigned int high;
  } longAsInt;

//No i funkcje

long filesize(FILE *fp);
void showError(int ErrNum, const char ErrText[]);
void loadPalette(const char *FName,void *Buf,ulong BufSize,int Options);
void saveBlockToNewFile(const char *FName,void *Buf,ulong BufSize);
void saveDataToFile(void *BufDest,ulong Size,FILE *DestFile);
int loadDataFromFile(FILE *File,void *Buf,ulong BytesToRead,int ErrNum,int Options);
void *allocateMem(ulong buffer_size,int ErrNum,int mem_clear,int Options);



#endif	// __PrgTools_H
