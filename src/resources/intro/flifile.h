//source code taken from http://syndicate.lubiki.pl/downloads/bullfrog_utils_flifix.zip
//for readme file please see flifixer-readme.txt

/*Note: This source code was modified by me, and is not 100% the original code of the authors
anymore. Otherwise I would not have been able to compile it, or use it in my project. */

/*
    FLI Filetype declaration
    written by Tomasz Lis, Gdansk, Poland 2004-2006
    this software is under GNU license
*/

#ifndef __FLIFile_H
#define __FLIFile_H

#include "prgtools.h"
#include "stdint.h"

//18.05.2025: Added for better integrated game logging
#include "../../utils/logging.h"

enum ChunkType
  {
    FLI_COLOR256=   4,
    FLI_SS2=        7,
    FLI_COLOR=     11,
    FLI_LC =       12,
    FLI_BLACK=     13,
    FLI_BRUN =     15,
    FLI_COPY=      16,
    FLI_PSTAMP=    18
  };

typedef struct
  {
     uint32_t size;
     uint16_t magic;
     uint16_t frames;
     uint16_t width;
     uint16_t height;
  } FLIMainHeader;

#define sizeof_FLIMainHeader 12

typedef struct
  {
     uint16_t depth;
     uint16_t flags;
     uint16_t speed;
     uint32_t next;
     uint32_t frit;
     //End of importand stuff - irrelevant stuff here (offset 26 dec)
     uint32_t creator;
     uint32_t lastchange;
     uint32_t changerserial;
     uint16_t Xaspec;
     uint16_t Yaspec;
     uint16_t reserved1[19];
     uint32_t frame1;
     uint32_t frame2;
     uint16_t reserved2[20];
  } FLIAddHeader;
//####
#define sizeof_FLIAddHeader 116

#define FLIFrameExpandSize 4

typedef struct
  {
     uint32_t size;
     uint16_t magic;
     uint16_t chunks;
     uint16_t expand[FLIFrameExpandSize];
  } FLIFrameHeader;

#define sizeof_FLIFrameHeader 16

typedef struct
  {
     uint32_t size;
     uint16_t type;
  } FLIChunkHeader;
//####
#define sizeof_FLIChunkHeader 6

typedef struct
  {
     uint16_t nPackets;
  } FLIColorHeader;

#define sizeof_FLIColorHeader 2

typedef struct
  {
     unsigned char nSkipColors;
     unsigned char nChangeColors; //If this number is 0,  (zero), it means 256
  } FLIColorPacketHeader;

#define sizeof_FLIColorPacketHeader 2

typedef struct
  {
     unsigned char Red;
     unsigned char Green;
     unsigned char Blue;
  } ColorDefinition;

#define sizeof_ColorDefinition 3

#define MaxPalSize sizeof_FLIChunkHeader+sizeof_FLIColorHeader+6*sizeof_FLIColorPacketHeader+256*sizeof_ColorDefinition

#define MaxAnimWidth 1024
#define MaxAnimHeight 768

//Functions
const char *getChunkTypeStr(unsigned int iType);
void displayMainHeaderInfo(FLIMainHeader *animHeader,ulong hStart, ulong fSize);
void displayAddHeaderInfo(FLIAddHeader *animAddHeader,ulong hEnd);
void displayFrameInfo(FLIFrameHeader *currFrameHdr,ulong frameNumber,ulong totalFrames,ulong hStart,ulong hEnd);
void displayChunkInfo(FLIChunkHeader *currChunkHdr,ulong chunkNumber,ulong hStart);
//Clearing structures
void clearNBufferBytes(char *buf,ulong N);
void clearFLIChunkHdr(FLIChunkHeader *hdrPtr);
void clearFLIFrameHdr(FLIFrameHeader *hdrPtr);
void clearFLIAddHdr(FLIAddHeader *hdrPtr);
void setFLIAddHdrDefaults(FLIAddHeader *hdrPtr);
//Reading structures
int hasAddHeader(FLIMainHeader *animHeader);
void loadMainHeader(FLIMainHeader *animHeader,FLIAddHeader *animAddHeader,FILE *animFile,int options);
void loadFrameHeader(FILE *animFile,FLIFrameHeader *animFrameHdr,ulong startOffset,ulong currFrame,ulong totalFrames,int options);
void loadChunkHeader(FILE *animFile,FLIChunkHeader *animChunkHdr,ulong startOffset,ulong chunkNumber,int options);
void loadChunkData(FILE *animFile,void *animChunkData,ulong size,int options);
//Fixing structures
int validChunk(FLIChunkHeader *animChunkHdr,ulong maxChunkSize);
int validColor(void *chunkData,ulong dataSize);
int validChunkData(unsigned int type,void *chunkData,ulong dataSize);
int strictValidFrame(FLIFrameHeader *animFrameHdr,ulong maxLength);
int validFrame(FLIFrameHeader *animFrameHdr,ulong maxLength);
int palShallBeMultiplied(void *palData,ulong dataSize);
void fixMainHeader(FLIMainHeader *animHeaderSrc,FLIAddHeader *animAddHeaderSrc,FLIMainHeader *animHeaderDest,FLIAddHeader *animAddHeaderDest,ulong fSize,int options);
void fixFrameHeader(FLIFrameHeader *animFrameHdr,ulong framePos,ulong frameEnd,int options);
void fixChunkHeader(FLIChunkHeader *animChunkHdr,ulong maxSize,ulong chunkNum,int isLastFrame,int options);
void fixChunkData(FLIChunkHeader *animChunkHdr,void *chunkData,ulong *dataSize,int options);
void fixFrameHdrToAddChunks(FLIFrameHeader *animFrameHdr,ulong frameNum,int options);
//Writing structures
void rewriteChunkHeader(FILE *destFile,ulong destLastChunkStart,FLIChunkHeader *chunkHdr);
void rewriteFrameHeader(FILE *destFile,ulong destLastFramePos,ulong frameNum,FLIFrameHeader *frameHdr,int options);
void rewriteMainHeader(FILE *destFile,ulong nFrames,FLIMainHeader *mainHdr);
void addRequiredChunksAtFrameStart(FILE *destFile,ulong frameNumber,int options);

#endif	// __FLIFile_H
