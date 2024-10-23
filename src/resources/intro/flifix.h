
/*Note: This source code file is not from the original author, but I had to create it to be able to use the original authors existing
 * projects source code inside my own project. */

#ifndef __flifix_H
#define __flifix_H

//#include <cstdlib.h>
//#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
//#include <sys\stat.h>
//#include <mem.h>
#include <limits.h>
#include "prgtools.h"
#include "flifile.h"
#include <errno.h>

//I had to create the function prototypes here
FILE *openSourceFLIFile(char *FName);
FILE *openDestinationFLIFile(char *FName);
void closeFLIFiles(FILE *animFile,FILE *destFile,int options);
int processMainHeader(FILE *animFile,FILE *destFile,FLIMainHeader *animHeaderDest,int *options);
ulong processFrame(FILE *animFile,FILE *destFile,FLIFrameHeader *animFrameHdr,ulong framePos,ulong frameEnd,ulong frameNum,ulong totalFrames,int options);
long posInFrameTable(ulong offset,ulong *framePosTable,ulong tableSize);
long insertFrameToTable(ulong *framePosTable,unsigned int frameCount,ulong *foundedFrames,ulong offset);
void findLostFrames(ulong *framePosTable,FILE *animFile,unsigned int frameCount,ulong *foundedFrames,ulong fullFrameSize,int options);

ulong findBestPosition(ulong *framePosTable,FILE *animFile,ulong lastFrame,ulong foundedFrames,ulong offset,int parseOptions,
                       int globOptions);

int chooseBestFrame(FILE *animFile,ulong maxFrameSize,ulong offset,ulong newOffset,int options);
void fillFramePosTable(ulong *framePosTable,FILE *animFile,ulong *frameCount,ulong fullFrameSize,int options,int globOptions);
void parseFileToSetOptions(ulong *framePosTable,FILE *animFile,ulong frameCount,int *options);
void adjustChunkOffset(ulong chunkPos,FILE *animFile,ulong rangeStart,ulong rangeEnd,int options);
ulong processChunk(FILE *animFile,FILE *destFile,ulong chunkPos,ulong maxSize,ulong chunkNum,int isLastFrame,int options);
void processFrameChunks(FILE *animFile,FILE *destFile,FLIFrameHeader *frameHdr,ulong countedPos,ulong frameEnd,ulong frameNum,ulong frameCount,int options);
void processFLISimple(FILE *animFile,FILE *destFile,ulong startFrame,ulong endFrame,int *options);
void processFLIFile(FILE *animFile,FILE *destFile,ulong startFrame,ulong endFrame,int globOptions);

#endif	// __flifix_H





