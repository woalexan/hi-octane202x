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
  std::string msg("");
  msg.append("  !!! There was an error during program execution !!!\n");
 if (ErrNum==errImpossible)
  {
  msg.append("IMPOSSIBLE EVENT HAS HAPPEN.\n  PROGRAM IS BADLY MODIFIED OR YOUR COMPUTER IS UNSTABLE.\n");
  }
 else
  {
  msg.append("    When ");
  if (ErrNum & errFileRead)
    msg.append("reading from a file");
   else
  if (ErrNum & errFileWrite)
    msg.append("writing to a file");
   else
  if (ErrNum & errFileOpen)
    msg.append("opening a file");
   else
  if (ErrNum & errMemAlloc)
    msg.append("allocating memory required");
   else
    msg.append("processings");

  if (ErrNum & 0x0ff0)
    {
    msg.append(" the element: ");
    if (ErrNum & errMainHdr)
      msg.append("Main FLI header");
     else
    if (ErrNum & errFrameHdr)
      msg.append("a frame header");
     else
    if (ErrNum & errChunkHdr)
      msg.append("a Chunk (part of frame) header");
     else
    if (ErrNum & errChunkData)
      msg.append("a Chunk (part of frame) data");
     else
    if (ErrNum & errAddHdr)
      msg.append("Main FLI header additional segments");
     else
    if (ErrNum & errFramePosTbl)
      msg.append("a Frame Position Table");
     else
    if (ErrNum & errPlainData)
      msg.append("a plain data");
     else
      msg.append("unknown");
    };
  msg.append(",\n    the exception has occured:\n");
  char hlpstr[500];
  snprintf(hlpstr, 500, "    %s\n",ErrText);
  msg.append(hlpstr);
  if (ErrNum & errDataNotExist)
    msg.append("    The expected data couldn't be found.\n");
  if (ErrNum & errCannotComplete)
    msg.append("    The operation couldn't be completed.\n");
  if (ErrNum & errOnlyParsing)
    msg.append("    This happen on parsing only, not on final processing.\n    Parsing cancelled.\n");
  }; //end if (ErrNum==errImposs...

if (ErrNum & errCritical)
  {
   msg.append("   With this error, program is unable to continue.\n");
   msg.append("   Program will now exit. Bye.\n");
   logging::Error(msg);
   //printf(" (Press any key to quit)\n");
   //getch();
   exit(EXIT_FAILURE);
  }
  else
  {
   msg.append("   Program will now try to continue, but some data may be invalid.\n\n");
   logging::Error(msg);
  };
}

void loadPalette(const char *pal_file_name,void *palette_buffer,size_t BufSize,int Options)
//Reads palette file, allocates buffer and returns it
{
    FILE *palfp;
    palfp = fopen (pal_file_name, "rb");
    if (!palfp)
    { showError(errFileOpen,strerror(errno)); return; };
    size_t ReadOK=fread(palette_buffer, BufSize, 1, palfp);
    if ((ReadOK!=BufSize) && !(Options & poIgnoreExceptions))
        showError(errFileRead,"I couldn't read enought bytes of palette file to reconstruct chunk.");
    if (Options & poDisplayAllInfo)
        logging::Info("    ==>Palette chunk constructed.");
    fclose(palfp);
}

void saveBlockToNewFile(const char *FName,void *Buf,size_t BufSize)
{
    FILE *File;
    File = fopen (FName, "wb");
    size_t nWritten=fwrite(Buf,1,BufSize,File);
    if (nWritten<BufSize) showError(errFileWrite,"SaveBlock function couldn't finish operation.");
    fclose(File);
}

void saveDataToFile(void *BufDest,size_t Size,FILE *DestFile)
{
  size_t nWritten=fwrite(BufDest,1,Size,DestFile);
  if (nWritten<Size) showError(errFileWrite+errCritical,strerror(errno));
}

int loadDataFromFile(FILE *File,void *Buf,size_t BytesToRead,int ErrNum,int Options)
/*
 Reads BytesToRead from file to the preallocated buffer.
*/
{
  if (BytesToRead==0) return 1;
  size_t ReadOK=fread(Buf,1,BytesToRead,File);
  if ((ReadOK!=BytesToRead) && !(Options & poIgnoreExceptions))
    {
    char *ErrMsg=(char *)malloc(256);
    if (ErrMsg!=nullptr)
      {
      snprintf(ErrMsg,256, "Couldn't read more than %zu bytes, %zu needed. EOF or read error.",ReadOK,BytesToRead);
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
    char hlpstr[500];
    std::string msg("");

    snprintf(hlpstr, 500, "!AllocateMem starting, %d\n",buffer_size);
    msg.append(hlpstr);
    logging::Info(msg);

    char *Data=(char*)malloc(buffer_size);
//    printf("!AllocateMem done\n");
    if ((Data==nullptr) && !(Options & poIgnoreExceptions))
    {
        logging::Error("!AllocateMem error occured");
        char *ErrMsg=(char *)malloc(128);
        if (ErrMsg!=nullptr)
        {
          if (buffer_size<8192)
            snprintf(ErrMsg, 128, "Cannot allocate %lu bytes. Whole memory has been allocated before.",buffer_size);
          else
            snprintf(ErrMsg, 128, "Cannot allocate %lu bytes. Maybe U should modify FLI file to have less frames.",buffer_size);
        showError(errMemAlloc|ErrNum,ErrMsg);
        free(ErrMsg);
        }
        else
        showError(errMemAlloc|ErrNum,"Cannot allocate any memory. Try reboot your machine.");
    }
    if ((mem_clear)&&(Data!=nullptr))
    {
        ulong num;
        for (num=0;num<buffer_size;num++)
                Data[num]=0;
    }
    return Data;
}

#endif	// __PrgTools_Cpp
