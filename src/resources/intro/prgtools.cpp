//source code taken from http://syndicate.lubiki.pl/downloads/bullfrog_utils_flifix.zip
//for readme file please see flifixer-readme.txt

/*Note: This source code was modified by me, and is not 100% the original code of the authors
anymore. Otherwise I would not have been able to compile it, or use it in my project. */

/*
    Program tool functions
    written by Tomasz Lis, Gdansk, Poland 2004-2006
    this software is under GNU license
*/
#ifndef __PrgTools_Cpp
#define __PrgTools_Cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prgtools.h"
#include <errno.h>

long filesize(FILE *fp)
{
    long length;
    long lastpos;
    
    if (!fp)
	return -1;
    lastpos = ftell (fp);
    fseek (fp, 0, SEEK_END);
    length = ftell (fp);
    fseek (fp, lastpos, SEEK_SET);
    return length;
}

void showError(int ErrNum, const char ErrText[])
{
  printf("  !!! There was an error during program execution !!!\n");
 if (ErrNum==errImpossible)
  {
  printf("\n    IMPOSSIBLE EVENT HAS HAPPEN.\n  PROGRAM IS BADLY MODIFIED OR YOUR COMPUTER IS UNSTABLE.\n");
  }
 else
  {
  printf("    When ");
  if (ErrNum & errFileRead)
    printf("reading from a file");
   else
  if (ErrNum & errFileWrite)
    printf("writing to a file");
   else
  if (ErrNum & errFileOpen)
    printf("opening a file");
   else
  if (ErrNum & errMemAlloc)
    printf("allocating memory required");
   else
    printf("processings");
  if (ErrNum & 0x0ff0)
    {
    printf(" the element: ");
    if (ErrNum & errMainHdr)
      printf("Main FLI header");
     else
    if (ErrNum & errFrameHdr)
      printf("a frame header");
     else
    if (ErrNum & errChunkHdr)
      printf("a Chunk (part of frame) header");
     else
    if (ErrNum & errChunkData)
      printf("a Chunk (part of frame) data");
     else
    if (ErrNum & errAddHdr)
      printf("Main FLI header additional segments");
     else
    if (ErrNum & errFramePosTbl)
      printf("a Frame Position Table");
     else
    if (ErrNum & errPlainData)
      printf("a plain data");
     else
      printf("unknown");
    };
  printf(",\n    the exception has occured:\n");
  printf("    %s\n",ErrText);
  if (ErrNum & errDataNotExist)
    printf("    The expected data couldn't be found.\n");
  if (ErrNum & errCannotComplete)
    printf("    The operation couldn't be completed.\n");
  if (ErrNum & errOnlyParsing)
    printf("    This happen on parsing only, not on final processing.\n    Parsing cancelled.\n");
  }; //end if (ErrNum==errImposs...

if (ErrNum & errCritical)
  {
   printf("   With this error, program is unable to continue.\n");
   printf("   Program will now exit. Bye.\n");
   //printf(" (Press any key to quit)\n");
   //getch();
   exit(EXIT_FAILURE);
  }
  else
  {
   printf("   Program will now try to continue, but some data may be invalid.\n\n");
  };
}

void loadPalette(const char *pal_file_name,void *palette_buffer,ulong BufSize,int Options)
//Reads palette file, allocates buffer and returns it
{
    FILE *palfp;
    palfp = fopen (pal_file_name, "rb");
    if (!palfp)
    { showError(errFileOpen,strerror(errno)); return; };
    ulong ReadOK=fread(palette_buffer, BufSize, 1, palfp);
    if ((ReadOK!=BufSize) && !(Options & poIgnoreExceptions))
        showError(errFileRead,"I couldn't read enought bytes of palette file to reconstruct chunk.");
    if (Options & poDisplayAllInfo)
        printf("    ==>Palette chunk constructed.\n");
    fclose(palfp);
}

void saveBlockToNewFile(const char *FName,void *Buf,ulong BufSize)
{
    FILE *File;
    File = fopen (FName, "wb");
    ulong nWritten=fwrite(Buf,1,BufSize,File);
    if (nWritten<BufSize) showError(errFileWrite,"SaveBlock function couldn't finish operation.");
    fclose(File);
}

void saveDataToFile(void *BufDest,ulong Size,FILE *DestFile)
{
  ulong nWritten=fwrite(BufDest,1,Size,DestFile);
  if (nWritten<Size) showError(errFileWrite+errCritical,strerror(errno));
}

int loadDataFromFile(FILE *File,void *Buf,ulong BytesToRead,int ErrNum,int Options)
/*
 Reads BytesToRead from file to the preallocated buffer.
*/
{
  if (BytesToRead==0) return 1;
  unsigned int ReadOK=fread(Buf,1,BytesToRead,File);
  if ((ReadOK!=BytesToRead) && !(Options & poIgnoreExceptions))
    {
    char *ErrMsg=(char *)malloc(256);
    if (ErrMsg!=NULL)
      {
      sprintf(ErrMsg,"Couldn't read more than %u bytes, %lu needed. EOF or read error.",ReadOK,BytesToRead);
      showError(errFileRead|ErrNum,ErrMsg);
      free(ErrMsg);
      }
     else
      showError(errFileRead|ErrNum,"Couldn't read enought bytes, also some memory faults.");
    };
  if (ReadOK!=BytesToRead)
    return 0;
   else
    return 1;
}

void *allocateMem(ulong buffer_size,int ErrNum,int mem_clear,int Options)
{
//    printf("!AllocateMem starting, %d\n",buffer_size);
    char *Data=(char*)malloc(buffer_size);
//    printf("!AllocateMem done\n");
    if ((Data==NULL) && !(Options & poIgnoreExceptions))
    {
//        printf("!AllocateMem error occured\n");
        char *ErrMsg=(char *)malloc(128);
        if (ErrMsg!=NULL)
        {
          if (buffer_size<8192)
            sprintf(ErrMsg,"Cannot allocate %lu bytes. Whole memory has been allocated before.",buffer_size);
          else
            sprintf(ErrMsg,"Cannot allocate %lu bytes. Maybe U should modify FLI file to have less frames.",buffer_size);
        showError(errMemAlloc|ErrNum,ErrMsg);
        free(ErrMsg);
        }
        else
        showError(errMemAlloc|ErrNum,"Cannot allocate any memory. Try reboot your machine.");
    }
    if ((mem_clear)&&(Data!=NULL))
    {
        ulong num;
        for (num=0;num<buffer_size;num++)
                Data[num]=0;
    }
    return Data;
}

#endif	// __PrgTools_Cpp
