//source code taken from http://syndicate.lubiki.pl/downloads/bullfrog_utils_flifix.zip
//for readme file please see flifixer-readme.txt

/*Note: This source code was modified by me, and is not 100% the original code of the authors
anymore. Otherwise I would not have been able to compile it, or use it in my project. */

/*
    FLI Filetype functions
    written by Tomasz Lis, Gdansk, Poland 2004-2006
    this software is under GNU license
*/
#ifndef __FLIFile_Cpp
#define __FLIFile_Cpp

#include <stdio.h>
#include <stdlib.h>
#include "flifile.h"

//SaveBlockToFile("!debug.dta",ChunkPacketHdr,DataSize-ChunkPos);
//printf("Saved. Press N key \n");getch();

/*
===========================================================
Displaying FLI file information
===========================================================
*/

const char *getChunkTypeStr(unsigned int iType)
{
  switch (iType)
    {
    case FLI_COLOR256:return "FLI_COLOR256 (256-level color palette info)";
    case      FLI_SS2:return "FLI_SS2 (Word-oriented delta compression)";
    case    FLI_COLOR:return "FLI_COLOR (64-level color palette info)";
    case       FLI_LC:return "FLI_LC (Byte-oriented delta compression)";
    case    FLI_BLACK:return "FLI_BLACK (Set whole screen to color 0)";
    case     FLI_BRUN:return "FLI_BRUN (Bytewise run-length compression)";
    case     FLI_COPY:return "FLI_COPY (Indicates uncompressed full frame)";
    case   FLI_PSTAMP:return "FLI_PSTAMP (Postage stamp sized image)";
    default:          return "UNKNOWN (Not a standard FLI chunk type)";
    };
}

void displayMainHeaderInfo(FLIMainHeader *animHeader,ulong hStart, ulong fSize)
{
  printf("-------Main header starts on byte %lu ---------\n",hStart);
  printf("Filesize from header: %7lu    Real filesize:%lu\n",animHeader->size,fSize);
  printf("Magic number: %15u    Shall be:%u or %u\n",animHeader->magic,(int)(0x0af11),(int)(0x0af12));
  printf("Frames count: %15u    Value should be positive & big\n",animHeader->frames);
  printf("Width: %22u    Value should be positive & <1000\n",animHeader->width);
  printf("Height: %21u    Value should be positive & <800\n",animHeader->height);
}

void displayAddHeaderInfo(FLIAddHeader *animAddHeader,ulong hEnd)
{
  printf("------- Main Header - Additional informations ---------\n");
  printf("Color depth: %16u    Should be set to:8\n",animAddHeader->depth);
  printf("Flags: %22u    Usually: 3\n",animAddHeader->flags);
  printf("Frame speed: %16u    In sec/1024;usually between <4,350>\n",animAddHeader->speed);
  printf("Unknown value: %14lu    Usually 0;no estimations\n",animAddHeader->next);
  printf("Date/Time of creation:%7lu    No estimations\n",animAddHeader->frit);
  printf("Creator: %20lu    No estimations\n",animAddHeader->creator);
  printf("Date/Time of last change:%4lu    No estimations\n",animAddHeader->lastchange);
  printf("Serial number: %14lu    No estimations\n",animAddHeader->changerserial);
  printf("X-Aspect ratio: %13u    Can be 0;no estimations\n",animAddHeader->Xaspec);
  printf("Y-Aspect ratio: %13u    Can be 0;no estimations\n",animAddHeader->Yaspec);
  printf("Reserved(1st):");
  ulong n;
  for (n=0;n<(sizeof(animAddHeader->reserved1)/sizeof(animAddHeader->reserved1[0]));n++)
     printf("%u ",animAddHeader->reserved1[n]);
     printf("(no estims)\n");
  printf("Offset of frame 1: %10lu    (no estimations)\n",animAddHeader->frame1);
  printf("Offset of frame 2: %10lu    (no estimations)\n",animAddHeader->frame2);
  printf("Reserved(2nd):");
  for (n=0;n<(sizeof(animAddHeader->reserved2)/sizeof(animAddHeader->reserved2[0]));n++)
     printf("%u ",animAddHeader->reserved2[n]);
     printf("(no estims)\n");
  printf("-------Main header ends on byte %lu ---------\n",hEnd);
}

void displayFrameInfo(FLIFrameHeader *currFrameHdr,ulong frameNumber,ulong totalFrames,ulong hStart,ulong hEnd)
{
  printf("  -------Frame no %lu of %lu header starts on byte %lu ---------\n",frameNumber,totalFrames,hStart);
  printf("  Bytes in frame: %11lu    Shall be <framesize+palsize\n",currFrameHdr->size);
  printf("  Magic: %20u    Always %u\n",currFrameHdr->magic,0xf1fa);
  printf("  Chunks in frame:%11u    No estimations\n",currFrameHdr->chunks);
  printf("  Expand:");
  ulong n;
  for (n=0;n<(sizeof(currFrameHdr->expand)/sizeof(currFrameHdr->expand[0]));n++)
     printf("%u ",currFrameHdr->expand[n]);
     printf("  (no estims)\n");
  printf("  -------Frame %lu header ends on byte %lu ---------\n",frameNumber,hEnd);
}

void displayChunkInfo(FLIChunkHeader *currChunkHdr,ulong chunkNumber,ulong hStart)
{
  printf("    ==>Chunk no %lu, starts on byte %lu\n",chunkNumber,hStart);
  printf("    Bytes in chunk:%10lu    ( <%lu )\n",currChunkHdr->size,(unsigned long)(MaxAnimWidth*MaxAnimHeight));
  printf("    Type of chunk:%11u  %s\n",currChunkHdr->type,getChunkTypeStr(currChunkHdr->type));
}

/*
===========================================================
Clearing structures
===========================================================
*/
void clearNBufferBytes(char *buf,ulong N)
{
  if (buf==NULL) return;
  ulong i;
  for (i=0;i<N;i++)
    buf[i]=0;
}

void clearFLIChunkHdr(FLIChunkHeader *hdrPtr)
{
  char *buf=(char *)hdrPtr;
  clearNBufferBytes(buf,sizeof(FLIChunkHeader));
}

void clearFLIFrameHdr(FLIFrameHeader *hdrPtr)
{
  char *buf=(char *)hdrPtr;
  clearNBufferBytes(buf,sizeof(FLIFrameHeader));
}

void clearFLIAddHdr(FLIAddHeader *hdrPtr)
{
  char *buf=(char *)hdrPtr;
  clearNBufferBytes(buf,sizeof(FLIAddHeader));
}

void setFLIAddHdrDefaults(FLIAddHeader *hdrPtr)
{
  hdrPtr->depth=8;
  hdrPtr->flags=3;
  hdrPtr->speed=5;
  hdrPtr->next=0;
  hdrPtr->frit=0;
  hdrPtr->creator=0;
  hdrPtr->lastchange=0;
  hdrPtr->changerserial=0;
  hdrPtr->Xaspec=1;
  hdrPtr->Yaspec=1;
  ulong i;
  for (i=0;i<19;i++)
    hdrPtr->reserved1[i]=0;
  hdrPtr->frame1=0;
  hdrPtr->frame2=0;
  for (i=0;i<20;i++)
    hdrPtr->reserved2[i]=0;
}

/*
===========================================================
Reading structures
===========================================================
*/
int hasAddHeader(FLIMainHeader *animHeader)
{
  if ((animHeader->magic==44818)&&(animHeader->size==12))
    return 0;
  return 1;
}

void loadMainHeader(FLIMainHeader *animHeader,FLIAddHeader *animAddHeader,FILE *animFile,int options)
{
    if (options & poManualSeeking)
        fseek(animFile,0 ,SEEK_SET);
    loadDataFromFile(animFile,animHeader,sizeof_FLIMainHeader,errMainHdr|errCritical,options);
    if (hasAddHeader(animHeader))
    {
        loadDataFromFile(animFile,animAddHeader,sizeof_FLIAddHeader,errMainHdr,options);
    }
    else
    {
        setFLIAddHdrDefaults(animAddHeader);
//      if ((Options & poDisplayAllInfo))
        printf("-->Additional header doesn't exist - FIXED\n");
    };
}

void loadFrameHeader(FILE *animFile,FLIFrameHeader *animFrameHdr,ulong startOffset,ulong currFrame,ulong totalFrames,int options)
{
  clearFLIFrameHdr(animFrameHdr);
  fseek(animFile, startOffset, SEEK_SET);
  loadDataFromFile(animFile,animFrameHdr,sizeof_FLIFrameHeader,errFrameHdr|errCritical,options);
  if (options & poDisplayAllInfo) displayFrameInfo(animFrameHdr,currFrame,totalFrames,startOffset,ftell(animFile)-1);
   else
    if (!(options & poIgnoreExceptions)) printf("  Restoring frame no %8lu\n",currFrame);
}

void loadChunkHeader(FILE *animFile,FLIChunkHeader *animChunkHdr,ulong startOffset,ulong chunkNumber,int options)
{
  clearFLIChunkHdr(animChunkHdr);
  fseek(animFile, startOffset, SEEK_SET);
  loadDataFromFile(animFile,animChunkHdr,sizeof_FLIChunkHeader,errChunkHdr,options);
  if (options & poDisplayAllInfo) displayChunkInfo(animChunkHdr,chunkNumber,startOffset);
}

void loadChunkData(FILE *animFile,void *animChunkData,ulong size,int options)
{
  char *buf=(char *)animChunkData;
  clearNBufferBytes(buf,size);
  loadDataFromFile(animFile,animChunkData,size,errChunkData,options);
}

/*
===========================================================
Fixing structures
===========================================================
*/
int validChunk(FLIChunkHeader *animChunkHdr,ulong maxChunkSize)
{
  if (animChunkHdr==NULL) return 0;

  //First, let's check chunk sizes
  if ((animChunkHdr->size)>(unsigned long)(MaxAnimWidth*MaxAnimHeight)) return 0;
  if ( (animChunkHdr->type==FLI_COLOR256) || (animChunkHdr->type==FLI_COLOR) )
      if ((animChunkHdr->size) > MaxPalSize) return 0;
  if ((animChunkHdr->size)>(maxChunkSize)) return 0;

  //Now checking chunk type
    if ((animChunkHdr->size)>(sizeof_FLIChunkHeader-1))
    {
    if ( (animChunkHdr->type==FLI_COLOR256) ||
         (animChunkHdr->type==FLI_SS2)      ||
         (animChunkHdr->type==FLI_COLOR)    ||
         (animChunkHdr->type==FLI_LC)       ||
         (animChunkHdr->type==FLI_BLACK)    ||
         (animChunkHdr->type==FLI_BRUN)     ||
         (animChunkHdr->type==FLI_COPY)     ||
         (animChunkHdr->type==FLI_PSTAMP) )
    return 1;
    };
  return 0;
}

int validColor(void *chunkData,ulong dataSize)
{
    if (dataSize<sizeof_FLIColorHeader) return 0;
    ulong nAllColors=0;
    ulong nChangedColors=0;
    ulong chunkPos=sizeof_FLIColorHeader;
    FLIColorHeader *chunkDataHdr=(FLIColorHeader *)(chunkData);
    FLIColorPacketHeader *chunkPacketHdr;
    ulong nColorsInPack=0;

    //Browsing packets
    ulong i;
    for (i=0;i<chunkDataHdr->nPackets;i++)
    {
        if (dataSize<chunkPos)
        {
            return 0;
        }
        //Creating a pointer to packets' header
        //Note Wolf Alexander: The following line caused an "arithmetic on a pointer to void" error, and did not compile
        //therefore I commented the original source code line out, and replaced it with the line below
        //chunkPacketHdr=(FLIColorPacketHeader *)(chunkData+chunkPos);
        chunkPacketHdr=(FLIColorPacketHeader *)((char*)chunkData+chunkPos);
        //Now reading number of colors from header
        if (chunkPacketHdr->nChangeColors>0)
            nColorsInPack=chunkPacketHdr->nChangeColors;
        else
            nColorsInPack=256;
        nAllColors+=chunkPacketHdr->nSkipColors+nColorsInPack;
        nChangedColors+=nColorsInPack;
        if (nAllColors>256)
        {
            return 0;
        }
        chunkPos=sizeof_FLIColorHeader+(i+1)*sizeof_FLIColorPacketHeader+nChangedColors*sizeof_ColorDefinition;
    } //end for
    if ((nAllColors>0)&&(chunkPos<=dataSize)) return 1;
    return 0;
}

int validChunkData(unsigned int type,void *chunkData,ulong dataSize)
{
  if (chunkData==NULL) return 0;
  switch (type)
    {
    case FLI_COLOR256:return validColor(chunkData,dataSize);
    case      FLI_SS2:return 1;
    case    FLI_COLOR:return 1;
    case       FLI_LC:return 1;
    case    FLI_BLACK:return 1;
    case     FLI_BRUN:return 1;
    case     FLI_COPY:return 1;
    case   FLI_PSTAMP:return 1;
    };
  return 0;
}

int strictValidFrame(FLIFrameHeader *animFrameHdr,ulong maxLength)
{
  if (animFrameHdr==NULL) return 0;
  if (animFrameHdr->chunks > 3) return 0;
  ulong i;
  for (i=0;i<FLIFrameExpandSize;i++)
     {
     if (animFrameHdr->expand[i]!=0) return 0;
     };
  if ((animFrameHdr->size>(1229600))||(animFrameHdr->size>maxLength)) return 0;
  if ((animFrameHdr->magic==0xf1fa))
    return 1;
  return 0;
}

int validFrame(FLIFrameHeader *animFrameHdr,ulong maxLength)
{
  if (animFrameHdr == NULL) return 0;
  if (animFrameHdr->chunks > 64) return 0;
  if (animFrameHdr->size > 2*maxLength) return 0;
  if (animFrameHdr->size > (MaxAnimWidth*MaxAnimHeight+MaxPalSize+sizeof_FLIFrameHeader))
      return 0;
  if ((animFrameHdr->magic==0xf1fa)&&(animFrameHdr->chunks<7))
    return 1;
  return 0;
}

int palShallBeMultiplied(void *palData,ulong dataSize)
{
  int Result=1;
  if (!validColor(palData,dataSize)) return 0;

  ulong nChangedColors=0;
  ulong curChangedColors=0;
  ulong chunkPos=sizeof_FLIColorHeader;
  FLIColorHeader *chunkDataHdr=(FLIColorHeader *)palData;
  FLIColorPacketHeader *chunkPacketHdr;
  ColorDefinition *colorData;
  ulong i;
  for (i=0;i<chunkDataHdr->nPackets;i++)
  {
    if (dataSize<chunkPos) return 0;
    //At the beginning, let's set pointer positions
    //Note Wolf Alexander: The following two lines caused an "arithmetic on a pointer to void" error, and did not compile
    //therefore I commented the original source code lines out, and replaced it with the two lines below
    //chunkPacketHdr=(FLIColorPacketHeader *)(palData+chunkPos);
    //colorData=(ColorDefinition *)(palData+chunkPos+sizeof_FLIColorPacketHeader);
    chunkPacketHdr=(FLIColorPacketHeader *)((char*)palData+chunkPos);
    colorData=(ColorDefinition *)((char*)palData+chunkPos+sizeof_FLIColorPacketHeader);

    if (chunkPacketHdr->nChangeColors > 0)
        curChangedColors=chunkPacketHdr->nChangeColors;
     else
        curChangedColors=256;
    ulong k;
    for (k=0;k<curChangedColors;k++)
      {
      if (colorData[k].Red>64) Result=0;
      if (colorData[k].Green>64) Result=0;
      if (colorData[k].Blue>64) Result=0;
      nChangedColors++;
      }
    chunkPos=sizeof_FLIColorHeader+(i+1)*sizeof_FLIColorPacketHeader+nChangedColors*sizeof_ColorDefinition;
  }
  return Result;
}

void fixMainHeader(FLIMainHeader *animHeaderSrc,FLIAddHeader *animAddHeaderSrc,FLIMainHeader *animHeaderDest,FLIAddHeader *animAddHeaderDest,ulong fSize,int options)
{
  //MainHdr
  animHeaderDest->size  = animHeaderSrc->size;
  animHeaderDest->magic = animHeaderSrc->magic;
  animHeaderDest->frames= animHeaderSrc->frames;
  animHeaderDest->width = animHeaderSrc->width;
  animHeaderDest->height= animHeaderSrc->height;
  //AddHdr
  animAddHeaderDest->depth     = animAddHeaderSrc->depth;
  animAddHeaderDest->flags     = animAddHeaderSrc->flags;
  animAddHeaderDest->speed     = animAddHeaderSrc->speed;
  animAddHeaderDest->next      = animAddHeaderSrc->next;
  animAddHeaderDest->frit      = animAddHeaderSrc->frit;
  animAddHeaderDest->creator   = animAddHeaderSrc->creator;
  animAddHeaderDest->lastchange= animAddHeaderSrc->lastchange;
  animAddHeaderDest->changerserial= animAddHeaderSrc->changerserial;
  animAddHeaderDest->Xaspec    = animAddHeaderSrc->Xaspec;
  animAddHeaderDest->Yaspec    = animAddHeaderSrc->Yaspec;
  ulong i;
  for (i=0;i<sizeof(animAddHeaderDest->reserved1)/sizeof(animAddHeaderDest->reserved1[0]);i++)
      animAddHeaderDest->reserved1[i] = animAddHeaderSrc->reserved1[i];
  animAddHeaderDest->frame1 = animAddHeaderSrc->frame1;
  animAddHeaderDest->frame2 = animAddHeaderSrc->frame2;
  for (i=0;i<sizeof(animAddHeaderDest->reserved2)/sizeof(animAddHeaderDest->reserved2[0]);i++)
      animAddHeaderDest->reserved2[i]=animAddHeaderSrc->reserved2[i];
  if (options & poFixMainHeader)
    {
    animHeaderDest->size = fSize;
    animAddHeaderDest->depth = 8;
    };
}

void fixFrameHeader(FLIFrameHeader *animFrameHdr,ulong framePos,ulong frameEnd,int options)
{
  ulong nFixes=0;
  if (animFrameHdr==NULL) return;
  if (!(options & poFixFrameHeaders)) return;
  //Size
  if ((animFrameHdr->size>1229600) || (animFrameHdr->size<sizeof_FLIFrameHeader))
    {
    animFrameHdr->size=frameEnd-framePos;
    nFixes++;
    };
  if ((animFrameHdr->size>1229600))
    {
    animFrameHdr->size=65535;
    nFixes++;
    };
  //Now let's make chunk count - we will return to its size
  if (animFrameHdr->chunks>128)
    {
    animFrameHdr->chunks=3;
    nFixes++;
    };
  //And the size again
  if (animFrameHdr->size<sizeof_FLIFrameHeader)
    {
    if (animFrameHdr->chunks==0) animFrameHdr->size=sizeof_FLIFrameHeader;
     else animFrameHdr->size=sizeof_FLIFrameHeader+1024;
    nFixes++;
    };
  //Additional parameters
  int expandTmp=0;
  ulong i;
  for (i=0;i<FLIFrameExpandSize;i++)
     {
     if (animFrameHdr->expand[i]!=0)
       {
       animFrameHdr->expand[i]=0;
       expandTmp++;
       };
     };
  if (expandTmp) nFixes++;
  //Only Magic remain
  if ((animFrameHdr->magic!=0xf1fa)&&(nFixes>0))
    {
    animFrameHdr->magic=0xf1fa;
    nFixes++;
    };
  //If there is many errors, it is better not to search for content
  if ((nFixes>2)||(options & poRadicalFrameHdrFix))
    {
    animFrameHdr->chunks=0;
    animFrameHdr->size=sizeof_FLIFrameHeader;
    };
  if ((nFixes>0)&&(options & poDisplayAllInfo))
    printf("-->Frame header info incorrect - FIXED\n");
}

void fixChunkHeader(FLIChunkHeader *animChunkHdr,ulong maxSize,ulong chunkNum,int isLastFrame,int options)
{
  ulong nFixes=0;
  if (animChunkHdr==NULL) return;
  //Rozmiar
  if ((animChunkHdr->size > (unsigned long)(MaxAnimWidth*MaxAnimHeight)))
    {
    animChunkHdr->size=maxSize;
    nFixes++;
    };
  if (animChunkHdr->size<sizeof_FLIChunkHeader)
    {
    animChunkHdr->size=sizeof_FLIChunkHeader;
    nFixes++;
    };
  if (animChunkHdr->size>(unsigned long)(MaxAnimWidth*MaxAnimHeight))
    {
    animChunkHdr->size=(unsigned long)(MaxAnimWidth*MaxAnimHeight);
    nFixes++;
    };
  //Typ
  if ((animChunkHdr->type>18)||(animChunkHdr->type<1))
    {
    if ((animChunkHdr->type<1)&&(animChunkHdr->size==sizeof_FLIChunkHeader))
      animChunkHdr->type=FLI_BLACK;
     else
     animChunkHdr->type=FLI_SS2;
    nFixes++;
    };
  if ((nFixes>0)&&(options & poDisplayAllInfo))
  {
      if (isLastFrame)
        printf("-->Chunk no %lu header info for last frame incorrect - FIXED\n",chunkNum);
      else
        printf("-->Chunk no %lu header info incorrect - FIXED\n",chunkNum);
  }
}

void fixChunkData(FLIChunkHeader *animChunkHdr,void *chunkData,ulong *dataSize,int options)
{
  //Naprawiamy palety
  if ((animChunkHdr->type==FLI_COLOR256)||(animChunkHdr->type==FLI_COLOR))
    {
  //Czy to jest paleta do zmiany typu ?
    if ((options & poExpandPatette)&&(animChunkHdr->type==FLI_COLOR256))
      animChunkHdr->type=FLI_COLOR;


    };
//!!!!!!!!!!TODO

}

void fixFrameHdrToAddChunks(FLIFrameHeader *animFrameHdr,ulong frameNum,int options)
{
  if ((options & poRecostructPatette)&&(frameNum==1))
    {
    animFrameHdr->size+=256*3+sizeof_FLIChunkHeader+4;
    animFrameHdr->chunks++;
    };
}

/*
===========================================================
Writing structures
===========================================================
*/
void rewriteChunkHeader(FILE *destFile,ulong destLastChunkStart,FLIChunkHeader *chunkHdr)
{
  ulong lastFilePos=ftell(destFile);
  fseek(destFile,destLastChunkStart ,SEEK_SET);
  chunkHdr->size=lastFilePos-destLastChunkStart;
  saveDataToFile(chunkHdr,sizeof_FLIChunkHeader,destFile);
  fseek(destFile,lastFilePos,SEEK_SET);
}

void rewriteFrameHeader(FILE *destFile,ulong destLastFramePos,ulong frameNum,FLIFrameHeader *frameHdr,int options)
{
  ulong lastFilePos=ftell(destFile);
  fseek(destFile,destLastFramePos ,SEEK_SET);
  fixFrameHdrToAddChunks(frameHdr,frameNum,options);
  frameHdr->size=lastFilePos-destLastFramePos;
  saveDataToFile(frameHdr,sizeof_FLIFrameHeader,destFile);
  fseek(destFile,lastFilePos,SEEK_SET);
}

void rewriteMainHeader(FILE *destFile,ulong nFrames,FLIMainHeader *mainHdr)
{
  ulong lastFilePos=ftell(destFile);
  fseek(destFile,0,SEEK_SET);
  mainHdr->size=(uint32_t)(filesize(destFile));
  mainHdr->frames=(uint16_t)(nFrames);
  saveDataToFile(mainHdr,sizeof_FLIMainHeader,destFile);
  fseek(destFile,lastFilePos,SEEK_SET);
}

void addRequiredChunksAtFrameStart(FILE *destFile,ulong frameNumber,int options)
{
//This function only writes a chunk.
//The header modification should be done outside, in fixFrameHdrToAddChunks
  if ((options & poRecostructPatette)&&(frameNumber==1))
    {
    //Declaring headers
    ulong palDataSize=256*sizeof_ColorDefinition;
    FLIChunkHeader *currChkHdr=(FLIChunkHeader *)allocateMem(sizeof(FLIChunkHeader)+1,errChunkHdr,1,options);
    FLIColorHeader *currPalChkHdr=(FLIColorHeader *)allocateMem(sizeof(FLIColorHeader)+1,errChunkData,1,options);
    FLIColorPacketHeader *currPcktChkHdr=(FLIColorPacketHeader *)allocateMem(sizeof(FLIColorPacketHeader)+1,errChunkData,1,options);
    //Clearing headers content
    clearFLIChunkHdr(currChkHdr);
    clearNBufferBytes((char *)currPalChkHdr,sizeof(FLIColorHeader));
    clearNBufferBytes((char *)currPcktChkHdr,sizeof(FLIColorPacketHeader));
    //Now setting up content: 1 packet, all colors inside packet
    currChkHdr->size=palDataSize+sizeof_FLIChunkHeader+sizeof_FLIColorHeader+sizeof_FLIColorPacketHeader;
    currChkHdr->type=FLI_COLOR256;
    currPalChkHdr->nPackets=1;
    currPcktChkHdr->nSkipColors=0;
    currPcktChkHdr->nChangeColors=0;//it means 256
    //Now writing everything
    saveDataToFile(currChkHdr,sizeof_FLIChunkHeader,destFile);
    free(currChkHdr);
    saveDataToFile(currPalChkHdr,sizeof_FLIColorHeader,destFile);
    free(currPalChkHdr);
    saveDataToFile(currPcktChkHdr,sizeof_FLIColorPacketHeader,destFile);
    free(currPcktChkHdr);
    //Separately we take care of palette content
    void *currChkData=allocateMem(palDataSize+1,errChunkData,0,options);
    loadPalette("FLIFix.pal",currChkData,(size_t)(palDataSize),options);
    saveDataToFile(currChkData,palDataSize,destFile);
    free(currChkData);
    };
}

#endif	// __FLIFile_Cpp
