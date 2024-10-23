//source code taken from http://syndicate.lubiki.pl/downloads/bullfrog_utils_flifix.zip
//for readme file please see flifixer-readme.txt

/*Note: This source code was modified by me, and is not 100% the original code of the authors
anymore. Otherwise I would not have been able to compile it, or use it in my project. */

#include "flifix.h"

ulong const MAX_CHUNK_CUT_SIZE = 16777216;
const long DeltaRange=65535;

enum BestFrameChoise
  {
   cbfOldOffset,
   cbfFixedOffset,
   cbfNothing
  };

/*void sayHello()
{
  printf("\nAutodesk Animator FLI files fixer");
  printf("\n-------------------------------\n");
  printf("\nThis program can repair damaged or modified FLI files.\n");
  printf("%20s written by Tomasz Lis, Gdansk, Poland 2004-2006\n","");
  printf("%20s this software is under GNU license.\n\n","");
}*/

/*void showUsage(char *addText)
{
  printf("\n\n%s\n",addText);
  printf("\nUsage:\n");
  printf("%8s flifix [options] <srcFile> [destFile]\n","");
  printf("\nWhere:\n");
  printf("  <srcFile>  is a FLI file to fix\n");
  printf("  [destFile] is an output file, set to '%s', if no other name typed\n",DefDestFName);
  printf("  [options]  can be -v -m -f -p -n -b -c -a -l -u -r -k -s<num> -e<num>\n");
  printf("See readme file for options explanation.\n");
}*/

FILE *openSourceFLIFile(char *FName)
{
    FILE *filep;
    filep=fopen(FName,"rb");
    if (!filep) {
      showError(errFileOpen+errCritical,strerror(errno));
      return NULL;
    }

    return filep;
}

FILE *openDestinationFLIFile(char *FName)
{
    FILE *filep;
    filep=fopen(FName,"wb");
    if (!filep) {
      showError(errFileOpen+errCritical,strerror(errno));
      return NULL;
    }

    return filep;
}

/*void waitForKeypress(int *options)
{
  if (((*options) & poDisplayAllInfo)&&(!((*options) & poNeverWaitForKey)))
    {
    printf("\n (Press any key to continue, Q quits, ESC stops holding)\n");
    char X=getch();
    if (X==kbEscape) (*options)&=(0xffff-poDisplayAllInfo);
    if ((X=='q')||(X=='Q')) exit(1);
    };
}*/

void closeFLIFiles(FILE *animFile,FILE *destFile,int options)
{
  //printf("-------- Analyst complete - all frames has been processed ---------\n");
  fclose(animFile);
  fclose(destFile);
  /*if (!(options & poNeverWaitForKey))
    {
    printf("\n (Press any key to leave program)");
    getch();
    };
  printf("\n");*/
}

/*void askToDisplayInfo(int *options)
{
  if (!((*options) & poNeverWaitForKey))
    {
    printf(" (Press ENTER to display additional informations)\n");
    printf(" (Other key cancels full displaying)\n");
    char X=getch();
    if (X==kbEnter) (*options)|=poDisplayAllInfo; else (*options)&=(0xffff-poDisplayAllInfo);
    };
}*/

int processMainHeader(FILE *animFile,FILE *destFile,FLIMainHeader *animHeaderDest,int *options)
{
  //Local variables
  ulong increment=sizeof_FLIMainHeader;
  ulong startPos=ftell(animFile);

  //Allocating memory for source header
  FLIMainHeader *animHeaderSrc=(FLIMainHeader *)allocateMem(sizeof(FLIMainHeader)+1,errCritical|errMainHdr,1,*options);
  FLIAddHeader *animAddHeaderSrc=(FLIAddHeader *)allocateMem(sizeof(FLIAddHeader)+1,errCritical|errAddHdr,1,*options);
  //Reading the header
  loadMainHeader(animHeaderSrc,animAddHeaderSrc,animFile,*options);

  //creating fixed version of header
  FLIAddHeader *animAddHeaderDest=(FLIAddHeader *)allocateMem(sizeof(FLIAddHeader)+1,errCritical|errAddHdr,1,*options);
  fixMainHeader(animHeaderSrc,animAddHeaderSrc,animHeaderDest,animAddHeaderDest,filesize(animFile),*options);

  //Displaying header informations
  displayMainHeaderInfo(animHeaderSrc,startPos,filesize(animFile));
  //askToDisplayInfo(options);
  if ((*options) & poDisplayAllInfo)
    displayAddHeaderInfo(animAddHeaderSrc,ftell(animFile)-1);

  //Saving headers
  saveDataToFile(animHeaderDest,sizeof_FLIMainHeader,destFile);
  saveDataToFile(animAddHeaderDest,sizeof_FLIAddHeader,destFile);

  //Updating output variables
  if (hasAddHeader(animHeaderSrc))
    increment+=sizeof_FLIAddHeader;

  //Freeing memory
  free(animHeaderSrc);
  free(animAddHeaderSrc);
  free(animAddHeaderDest);

  return increment;
}

ulong processFrame(FILE *animFile,FILE *destFile,FLIFrameHeader *animFrameHdr,ulong framePos,ulong frameEnd,ulong frameNum,ulong totalFrames,int options)
{
  ulong increment=sizeof_FLIFrameHeader;
  loadFrameHeader(animFile,animFrameHdr,framePos,frameNum,totalFrames,options);
  fixFrameHeader(animFrameHdr,framePos,frameEnd,options);

  //Creating a copy of header in DestFrameHdr
  FLIFrameHeader *destFrameHdr=(FLIFrameHeader *)allocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr,1,options);
  memmove(destFrameHdr,animFrameHdr,sizeof(FLIFrameHeader));

  //After small modifications, saving the copy
  fixFrameHdrToAddChunks(destFrameHdr,frameNum,options);
  saveDataToFile(destFrameHdr,sizeof_FLIFrameHeader,destFile);
  addRequiredChunksAtFrameStart(destFile,frameNum,options);
  free(destFrameHdr);
  return increment;
}

long posInFrameTable(ulong offset,ulong *framePosTable,ulong tableSize)
{
  ulong elemPos=0;
  while ((elemPos<tableSize)&&(framePosTable[elemPos]<offset))
    elemPos++;
  //Checking if frame is not already in table
  if (framePosTable[elemPos]==offset)
    return elemPos;
   else
    return -1;
}

long insertFrameToTable(ulong *framePosTable,unsigned int frameCount,ulong *foundedFrames,ulong offset)
{
  //Finding position for new frame
  ulong insertPos=0;
  while ((insertPos<(*foundedFrames))&&(framePosTable[insertPos]<offset))
    {
    insertPos++;
    };
  //Checking if frame is not already in table
  if (framePosTable[insertPos]==offset)
    {
    return 0;
    };
  //If table if full - reallocating table memory
  if ((*foundedFrames)>frameCount)
    {
    return -1;//We don't need this for now - it shouldn't be required
    };
  //Moving all elements after pasting position
  long i;
  for (i=frameCount+1;i>=(long)insertPos;i--)
    {
//printf("Przenoszc el. %li by wstawi+ na poz. %lu\n",i,InsertPos);
    framePosTable[i+1]=framePosTable[i];
    };
  //Inserting our frame into created position
  framePosTable[insertPos]=offset;
  //The nomber of frames has increased
  (*foundedFrames)++;
  return insertPos+1;
}

void findLostFrames(ulong *framePosTable,FILE *animFile,unsigned int frameCount,ulong *foundedFrames,ulong fullFrameSize,int options)
{
  printf("    Some frames are missing. Parsing whole source file...\n");
  //Preparing buffers
  ulong bufSize=sizeof(FLIFrameHeader)+sizeof(FLIChunkHeader);
  void *buf=allocateMem(bufSize+1,errFrameHdr|errOnlyParsing,0,options);
  if (buf==NULL) return;
  FLIFrameHeader *frameHdr=(FLIFrameHeader *)&(((char *)buf)[0]);
  FLIChunkHeader *chunkHdr=(FLIChunkHeader *)&(((char *)buf)[sizeof_FLIFrameHeader]);
  //Searching offsets for frames
  ulong offset;
  for (offset=0;offset<=(filesize(animFile)-bufSize);offset++)
    {
    //Reading headers
    fseek(animFile,offset ,SEEK_SET);
    if (!loadDataFromFile(animFile,buf,bufSize,errFrameHdr|errOnlyParsing,options))
      {
      free(buf);
      return;
      };
    //Checking if header looks like frame
    if (strictValidFrame(frameHdr,fullFrameSize+2*MaxPalSize+sizeof_FLIFrameHeader+3*sizeof_FLIChunkHeader))
      {
      if ((frameHdr->chunks==0) || ((frameHdr->chunks>0) && validChunk(chunkHdr,fullFrameSize+sizeof_FLIChunkHeader)) )
        {
        if (options & poDisplayAllInfo)
          printf("    ->Founded valid frame at %9lu, ",offset);
        long insResult=insertFrameToTable(framePosTable,frameCount,foundedFrames,offset);
        if (options & poDisplayAllInfo)
          {
          if (insResult>0)  printf("added to frame table as frame %li.\n",insResult);
          if (insResult==0) printf("already is in frame table.\n");
          if (insResult<0)  printf("cannot add, frame table full.\n");
          };
        };
      };//end if (StrictValidFrame(...
    };//end for (ulong Offset...
  printf("    After searching, we have %lu frames (the file informed of %u frames)\n",*foundedFrames,frameCount+1);
  free(buf);
}

ulong findBestPosition(ulong *framePosTable,FILE *animFile,ulong lastFrame,ulong foundedFrames,ulong offset,int parseOptions,
                       int globOptions)
{
    signed long delta;
    signed long newDelta=0;
    int frameIsValid=0;
    ulong newOffset;
    FLIFrameHeader *animFrameHdr=(FLIFrameHeader *)allocateMem(sizeof(FLIFrameHeader)+1,
                                  errCritical|errFrameHdr,1,globOptions);
    do
    {
        delta=newDelta;
        //delta should be table: 0 -1 1 -2 2 -3 3 ....
        if (newDelta>=0)
        { newDelta++; newDelta*=(-1); }
        else
        { newDelta*=(-1); };
        //For each delta, we need to read FrameHeader
        //It has no matter if we compare (Offset-Delta) or (Offset+Delta)
        newOffset=offset+delta;
        if ( (newOffset>(sizeof_FLIMainHeader-2)) &&
             (newOffset<=(filesize(animFile)-sizeof_FLIFrameHeader)) &&
             (newOffset>framePosTable[lastFrame]) )
            loadFrameHeader(animFile,animFrameHdr,newOffset,foundedFrames+1,0,parseOptions);
        else
        {
            clearFLIFrameHdr(animFrameHdr);
            continue;
        }
        //if ((delta>0)&&((delta%20000)==0)) printf("Problems with finding frame %lu from offset %lu\n",foundedFrames+1,Offset);
        frameIsValid=validFrame(animFrameHdr,filesize(animFile)-framePosTable[lastFrame]);
	if ((frameIsValid)&&(posInFrameTable(newOffset,framePosTable,foundedFrames)<0))
          {
          free(animFrameHdr);
          return newOffset;
          };
    }
    while ((delta<DeltaRange));
    free(animFrameHdr);
    return offset;
}

int chooseBestFrame(FILE *animFile,ulong maxFrameSize,ulong offset,ulong newOffset,int options)
// Returns    cbfOldOffset, cbfFixedOffset or  cbfNothing
{
  //Buffers for both frames
  FLIFrameHeader *newFrameHdr=(FLIFrameHeader *)allocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr|errOnlyParsing,1,options);
  FLIFrameHeader *oldFrameHdr=(FLIFrameHeader *)allocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr|errOnlyParsing,1,options);
  //Variables initialization
  int allowOld=1;
  int allowFix=1;
  int allowNot=1;
  //Loading frames
  loadFrameHeader(animFile,newFrameHdr,newOffset,0,0,options);
  loadFrameHeader(animFile,oldFrameHdr,   offset,0,0,options);
  //And doing tests - simple ones at start
  if (options & poNeverSkipFrames)
    allowNot=0;
  if ((offset==newOffset)||(!(options & poFixFramePositions)))
    allowFix=0;
  if (!validFrame(newFrameHdr,maxFrameSize))
    allowFix=0;
  if (allowFix || allowNot)
    if (!validFrame(oldFrameHdr,maxFrameSize))
      allowOld=0;
  //If we still not sure, do heavier tests
  if (allowOld||allowNot)
    if (newOffset>filesize(animFile)-sizeof_FLIFrameHeader)
      allowFix=0;
  if (allowFix || allowNot)
    if (offset>filesize(animFile)-sizeof_FLIFrameHeader)
      allowOld=0;
  if (allowFix || allowOld)
    if (validFrame(oldFrameHdr,maxFrameSize)||validFrame(newFrameHdr,maxFrameSize))
      allowNot=0;
  if (allowFix||allowNot)
    if (!strictValidFrame(oldFrameHdr,maxFrameSize))
      allowOld=0;
  if (allowOld || allowNot)
    if (!strictValidFrame(newFrameHdr,maxFrameSize))
      allowFix=0;
  if (allowFix && allowOld)
    if (newOffset<offset)
      allowOld=0;
     else
      allowFix=0;
  //Freeing frames
  free(newFrameHdr);
  free(oldFrameHdr);
  //And returning what we should return
  if (allowFix) return cbfFixedOffset;
  if (allowNot) return cbfNothing;
  return cbfOldOffset;
}

void fillFramePosTable(ulong *framePosTable,FILE *animFile,ulong *frameCount,ulong fullFrameSize,int options,int globOptions)
//  Searches where frames begin and saves results to table.
//  Needs frames count not increased by 1, just from header (FrameCount).
//  Assumes that current position in file is just after the header
{
  printf("Creating Frame-Position Table...\n");
  if (options & poFixFramePositions)
    printf("*Frames position fixing active.\n");
  //we need to remember where we're searching
  ulong offset=ftell(animFile);
  //and what we've already found
  ulong foundedFrames=0;
  signed long insResult=0;
  //Simplified options, for file parsing
  int parseOptions=(options & (0xffff - poDisplayAllInfo)) | poIgnoreExceptions;
  //Clearing table
  clearNBufferBytes((char *)framePosTable,((*frameCount)+2)*sizeof(ulong));
  //Frame header
  FLIFrameHeader *animFrameHdr=(FLIFrameHeader *)allocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFrameHdr,1,options);
  //Now let's browse through frames
  while (offset<filesize(animFile))
    {
    //Alternate frame start offset
    ulong fixedOffset=0;
	if (options & poDisplayAllInfo)
	  printf("  Searching for frames at%7lu...\n",offset);
    //Additional variable to check if we're overwriting last entry.
    //We need it because FramePosTable[lastFrame] must always be defined
    ulong lastFrame;
    if (foundedFrames>0) lastFrame=foundedFrames-1; else lastFrame=0;
    //The array is cleared, so now FramePosTable[foundedFrames]=0
    if (options & poFixFramePositions) fixedOffset=findBestPosition(framePosTable,animFile,lastFrame,foundedFrames,offset,parseOptions,
                                                                    globOptions);
    //Now we should choose one of them, or just increase "offset" and search for next
	if (options & poDisplayAllInfo)
	  printf("  Possible frame%5lu, ",(ulong)foundedFrames+1);
    switch (chooseBestFrame(animFile,fullFrameSize+2*MaxPalSize+sizeof_FLIFrameHeader+3*sizeof_FLIChunkHeader,offset,fixedOffset,parseOptions))
    {
      case cbfOldOffset:
      {
        if (options & poDisplayAllInfo)
            printf("found at expected offset%7lu, ",offset);
        loadFrameHeader(animFile,animFrameHdr,offset,0,0,parseOptions);
        insResult=insertFrameToTable(framePosTable,*frameCount,&foundedFrames,offset);
        offset+=animFrameHdr->size;
        break;
      }
      case cbfFixedOffset:
      {
        if (options & poDisplayAllInfo)
            printf("found at adjusted offset%7lu, ",fixedOffset);
        loadFrameHeader(animFile,animFrameHdr,fixedOffset,0,0,parseOptions);
        insResult=insertFrameToTable(framePosTable,*frameCount,&foundedFrames,fixedOffset);
        offset=fixedOffset+animFrameHdr->size;
        break;
      }
      case cbfNothing:
      {
        if (options & poDisplayAllInfo)
        printf("not founded near offset %7lu, ",offset);
        offset+=DeltaRange;
        insResult=-255;
        break;
	  }
      default:
	  showError(errImpossible,"");
    }
    if (options & poDisplayAllInfo)
      {
      if (insResult==-255) printf("skipped.\n");
       else
      if (insResult>0)  printf("added to frame table.\n");
       else
      if (insResult==0) printf("already in frame table.\n");
       else
      if (insResult<0)  printf("cannot add, frame table full.\n");
      };
    };
  if (foundedFrames<(*frameCount)+1)
    {
    if (options & poUseFrameFinder)
	{
        //FrameFindera will find the rest of frames
	findLostFrames(framePosTable,animFile,*frameCount,&foundedFrames,fullFrameSize,options);
        };
    };
  if (foundedFrames<(*frameCount)+1)
    {
    printf("-->Only %lu frames out of %lu founded. Frames count FIXED.\n",foundedFrames,(ulong)((*frameCount)+1));
    }
   else
    printf("  All frames listed in table.\n");
  framePosTable[(*frameCount)+2]=filesize(animFile);
  free(animFrameHdr);
}

void parseFileToSetOptions(ulong *framePosTable,FILE *animFile,ulong frameCount,int *options)
{
  printf("Parsing file to set optimal options...\n");
  int parseOptions=((*options) & (0xffff - poDisplayAllInfo)) | poIgnoreExceptions;
  int firstFrameHasPalette=0;
  int expandPalette=1;
  //Allocating memory from chunk frame and header
  FLIFrameHeader *frameHdr=(FLIFrameHeader *)malloc(sizeof(FLIFrameHeader)+1);
  FLIChunkHeader *animChunkHdr=(FLIChunkHeader *)malloc(sizeof(FLIChunkHeader)+1);
  if ((frameHdr==NULL)||(animChunkHdr==NULL))
    {
    showError(errMemAlloc,"Cannot allocate memory for parsing (to auto-configure). Options set do defaults.");
    free(animChunkHdr);
    free(frameHdr);
    return;
    };
  //and briefing frames with chunks
  ulong i;
  for (i=0;i<=frameCount;i++)
    {
    //Frame
    loadFrameHeader(animFile,frameHdr,framePosTable[i],i+1,frameCount+1,parseOptions);
    fixFrameHeader(frameHdr,framePosTable[i],framePosTable[i+1],parseOptions);
    //Now Chunks
    ulong chunkStartOffs;
    ulong k;
    for (k=0;k<(frameHdr->chunks);k++)
      {
      chunkStartOffs=ftell(animFile);
      //Adjusting Chunk offset
      //!!!!AdjustChunkOffset(AnimFile,,,Options);
      //Reading chunk header
      loadChunkHeader(animFile,animChunkHdr,chunkStartOffs,k+1,parseOptions);
      fixChunkHeader(animChunkHdr,framePosTable[i+1]-chunkStartOffs,k+1,i==frameCount,parseOptions);
// ulong hdsize=sizeof(FLIChunkHeader);
// printf("!parseFileToSetOptions frame %lu chunk %lu, header offset %lu headerSize %lu\n",i+1,k+1,chunkStartOffs,hdsize);
      //Creating a buffer to read chunk content
      ulong dataSize=animChunkHdr->size-sizeof_FLIChunkHeader;
      void *chunkData=malloc(dataSize+2);
      if (chunkData==NULL)
        {
        showError(errChunkHdr|errMemAlloc,"Cannot allocate memory for chunk data when parsing. Options set to defaults - detection failed.");
        free(animChunkHdr);
        free(frameHdr);
        return;
        };
      //Reading chunk content
// ulong tmpv=ftell(animFile);
// printf("!parseFileToSetOptions frame %lu chunk %lu, data offset %lu dataSize %lu\n",i+1,k+1,tmpv,dataSize);
      loadChunkData(animFile,chunkData,dataSize,parseOptions);
      fixChunkData(animChunkHdr,chunkData,&dataSize,parseOptions);
//      Time to check what this chunk means for us
//        We have: i - frame number         k - chunk number            FramePosTable[] - borders
//                 AnimChunkHdr             ChunkData                   FrameHdr
      //Option: shall we add patette
      if ((i==0)&&((animChunkHdr->type==FLI_COLOR)||(animChunkHdr->type==FLI_COLOR256)))
          firstFrameHasPalette=1;
      //Option: shall we change palette type
      if (animChunkHdr->type==FLI_COLOR256)
        {
        if (!palShallBeMultiplied(chunkData,dataSize))
          {
          if (expandPalette)
              printf("Resigned of multiplying pal*4 after chunk %lu from frame %lu\n",k+1,i+1);
          expandPalette=0;
          }
        }
      //End of options - freeing memory
      free(chunkData);
      }
    }
  //Now we can set right options
  if (!firstFrameHasPalette)
    {
    printf("*Patette reconstruction from .PAL file activated.\n");
    (*options)|=poRecostructPatette;
    };
  if ((firstFrameHasPalette)&&(expandPalette))
    {
    printf("*Palette type indicator correction activated.\n");
    (*options)|=poExpandPatette;
    };

  //All done - freeing memory
  free(animChunkHdr);
  free(frameHdr);
}

void adjustChunkOffset(ulong chunkPos,FILE *animFile,ulong rangeStart,ulong rangeEnd,int options)
{
}

ulong processChunk(FILE *animFile,FILE *destFile,ulong chunkPos,ulong maxSize,ulong chunkNum,int isLastFrame,int options)
{
  //Initial settings and tests
  ulong increment=sizeof_FLIChunkHeader;
  ulong destLastChunkStart=ftell(destFile);
  int shallRemoveChunk=0;

  //Adjusting chunk offset
  //!!!!adjustChunkOffset(AnimFile,,,Options);

  //Reading its header
  FLIChunkHeader *animChunkHdr=(FLIChunkHeader *)allocateMem(sizeof(FLIChunkHeader)+1,errCritical|errChunkHdr,1,options);
  if (chunkPos+sizeof(FLIChunkHeader) <= filesize(animFile))
    loadChunkHeader(animFile,animChunkHdr,chunkPos,chunkNum,options);
   else
    {
    clearFLIChunkHdr(animChunkHdr);
    shallRemoveChunk=1;
    };
  //We must remember if the chunk was correct before fixations
  if ((options & poRemoveBadChunks)&&(!shallRemoveChunk))
    shallRemoveChunk=(!validChunk(animChunkHdr,maxSize));
  if (options & poFixChunkHeaders)
    fixChunkHeader(animChunkHdr,maxSize,chunkNum,isLastFrame,options);

  //Creating buffer to read content
  ulong dataSize;
  if (animChunkHdr->size > sizeof_FLIChunkHeader)
      dataSize=animChunkHdr->size-sizeof_FLIChunkHeader;
  else
      dataSize=0;
  if (dataSize>MAX_CHUNK_CUT_SIZE)
  {
      printf("-->Chunk no %lu too large - CUTTED, FILE WILL BE INVALID\n",chunkNum);
      printf("            (YOU SHOULD SET PARAMETERS TO ALLOW FIXATIONS)\n");
      dataSize=MAX_CHUNK_CUT_SIZE;
  }
  void *chunkData=allocateMem(dataSize+2,errCritical|errChunkData,0,options);

  //Reading chunk content
  loadChunkData(animFile,chunkData,dataSize,options);
  fixChunkData(animChunkHdr,chunkData,&dataSize,options);

if ( (options & poRemoveBadChunks) &&
     ((!validChunkData(animChunkHdr->type,chunkData,dataSize))||shallRemoveChunk) )
    {
    increment=0;
//    if ((Options & poDisplayAllInfo))
      printf("-->Chunk no %lu looks suspicious - REMOVED\n",chunkNum);
    }
   else
    {
    //Save and make the values correct
    saveDataToFile(animChunkHdr,sizeof_FLIChunkHeader,destFile);
    saveDataToFile(chunkData,dataSize,destFile);
    rewriteChunkHeader(destFile,destLastChunkStart,animChunkHdr);
    };

  //Freeing memory
  free(chunkData);
  free(animChunkHdr);
  return increment;
}

void processFrameChunks(FILE *animFile,FILE *destFile,FLIFrameHeader *frameHdr,ulong countedPos,ulong frameEnd,ulong frameNum,ulong frameCount,int options)
{
  ulong chunkStartOffs=0;
  ulong realChunksNumber=0;
  ulong chunkSize;
  ulong k;
  for (k=0;k<(frameHdr->chunks);k++)
    {
    if (options & poManualSeeking)
      chunkStartOffs=countedPos;
     else
      chunkStartOffs=ftell(animFile);
    chunkSize=processChunk(animFile,destFile,chunkStartOffs,frameEnd-chunkStartOffs,k+1,frameNum==frameCount-1,options);
    if (chunkSize>0)
      {
      countedPos+=chunkSize;
      realChunksNumber++;
      }
    //printf(">>Pozycje: AnimFile %lu DestFile %lu\n",ftell(AnimFile),tell(DestFile));
    };
  //Some frames needs little correction
  frameHdr->chunks=realChunksNumber;
  if ((frameHdr->chunks>1)&&((frameHdr->size%2)>0))
    {
    fseek(destFile,-1 ,SEEK_CUR);
    };
}

void processFLISimple(FILE *animFile,FILE *destFile,ulong startFrame,ulong endFrame,int *options)
//  Simplified version of ProcessFLIFile - made for tests
{
  ulong countedPos=0;//Ten parametr jest u¾ywany do ManualSeeking

  //Processing the header
  FLIMainHeader *animMainHeader=(FLIMainHeader *)allocateMem(sizeof(FLIMainHeader)+1,errCritical|errMainHdr,1,*options);
  countedPos+=processMainHeader(animFile,destFile,animMainHeader,options);

  //Po prostu przepisujemy zawarto_+ pliku do DestFile
  ulong bufSize=4096;
  void *buf=allocateMem(bufSize+1,errPlainData|errCritical,0,*options);
  unsigned int nRead=1;
  while (nRead!=0)
    {
    nRead=fread(buf,bufSize,1,animFile);
    if (nRead!=0)
      saveDataToFile(buf,nRead,destFile);
    };
  //Na koniec poprawiamy MainHeader
  if (startFrame>animMainHeader->frames) startFrame=animMainHeader->frames;
  if (endFrame>animMainHeader->frames) endFrame=animMainHeader->frames;
  if (startFrame>endFrame) endFrame=startFrame;
  rewriteMainHeader(destFile,endFrame-startFrame,animMainHeader);

  //To wszystko - jeszcze zwalniamy pamic+
  free(animMainHeader);
}

void processFLIFile(FILE *animFile,FILE *destFile,ulong startFrame,ulong endFrame,int globOptions)
//  Reads AnimFile (opened before) and rewrites it to DestFile (opened before)
{
//  printf("!ProcessFLIFile is starting\n");
  ulong countedPos=0;//This parameter is used for ManualSeeking
  ulong destLastFramePos=0;

  //Processing the header
  FLIMainHeader *animMainHeader=(FLIMainHeader *)allocateMem(sizeof(FLIMainHeader)+1,errCritical|errMainHdr,1,globOptions);
  countedPos+=processMainHeader(animFile,destFile,animMainHeader,&globOptions);

  //Creating the table containing positions of frames in file
//  printf("!ProcessFLIFile creating FramePosTable\n");
  ulong frames_count=animMainHeader->frames;
  ulong *framePosTable=(ulong *)allocateMem(sizeof(ulong)*(frames_count+3),errCritical|errFramePosTbl,1,globOptions);
  fillFramePosTable(framePosTable,animFile,&frames_count,((ulong)animMainHeader->width)*((ulong)animMainHeader->height),globOptions,globOptions);
  parseFileToSetOptions(framePosTable,animFile,frames_count,&globOptions);
  animMainHeader->frames=frames_count;
  FLIFrameHeader *frameHdr=(FLIFrameHeader *)allocateMem(sizeof(FLIFrameHeader)+1,errCritical|errFramePosTbl,1,globOptions);

//  printf("!ProcessFLIFile processing FramePosTable\n");
  //processing frames and chunks
  if (startFrame>animMainHeader->frames) startFrame=animMainHeader->frames;
  if (endFrame>animMainHeader->frames) endFrame=animMainHeader->frames;
  if (startFrame>endFrame) endFrame=startFrame;
  ulong i;
  for (i=startFrame;i<=endFrame;i++)
    {
    destLastFramePos=ftell(destFile);
    countedPos+=processFrame(animFile,destFile,frameHdr,framePosTable[i],framePosTable[i+1],i+1,animMainHeader->frames+1,globOptions);
    processFrameChunks(animFile,destFile,frameHdr,countedPos,framePosTable[i+1],i,endFrame+1,globOptions);
    rewriteFrameHeader(destFile,destLastFramePos,i+1,frameHdr,globOptions);
    //waitForKeypress(&globOptions);
    };

  //At the end, fixing MainHeader
  rewriteMainHeader(destFile,endFrame-startFrame,animMainHeader);
  //That's all - freeing memory
  free(animMainHeader);
  free(frameHdr);
  free(framePosTable);
}
/*
int main(int argc, char *argv[])
{
  //Variables initiation
  FILE *animFile;
  FILE *destFile;
  //Frames are numbered from 0
  ulong startFrame=0;
  ulong endFrame=ULONG_MAX;
  //Options description is avaible in the file where they are defined
  globOptions=0;
  //Welcome message
  sayHello();
  //Analyst of input parameters
  char *srcFName=NULL;
  char *destFName=DefDestFName;
  int num;
  if (argc>1)
  for (num=1;num<argc;num++)
  {
      char *param=argv[num];
      if ((param[0]=='-')&&(strlen(param)>1))
      {
          switch (param[1])
          {
          case 'v': globOptions|=poDisplayAllInfo;break;
          case 'm': globOptions|=poFixMainHeader;break;
          case 'f': globOptions|=poFixFrameHeaders;break;
          case 'p': globOptions|=poFixFramePositions;break;
          case 'n': globOptions|=poUseFrameFinder;break;
          case 'b': globOptions|=poRemoveBadChunks;break;
          case 'c': globOptions|=poFixChunkHeaders;break;
          case 'a': globOptions|=poNeverWaitForKey;break;
          case 'l': globOptions|=poSimpleFix;break;
          case 'u': globOptions|=poManualSeeking;break;
          case 'r': globOptions|=poRadicalFrameHdrFix;break;
          case 'k': globOptions|=poNeverSkipFrames;break;
          case 's': startFrame=100;break;
          case 'e': endFrame=100;break;
          }
      }
      else
      if (srcFName==NULL)
          srcFName=param;
      else
          destFName=param;
  }
  if (srcFName==NULL)
      { showUsage("U did not specified source FLI filename.");return 1; }

  //Opening the files
  animFile=openSourceFLIFile(srcFName);
  destFile=openDestinationFLIFile(destFName);

  //File analyst
  if (globOptions & poSimpleFix)
    processFLISimple(animFile,destFile,startFrame,endFrame,&globOptions);
   else
    processFLIFile(animFile,destFile,startFrame,endFrame);

  //Closing files
  closeFLIFiles(animFile,destFile,globOptions);
  return 0;
}*/
