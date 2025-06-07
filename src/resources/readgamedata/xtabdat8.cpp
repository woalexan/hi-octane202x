// converts picture data from bullfrog's DAT/TAB files into BMPs
// works with 8bpp files, needs a .dat and a .tab file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bulcommn.h"
#include "xtabdat8.h"
#include <inttypes.h>

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

 //Note 19.03.2025: In an attempt to reduce/remove the warnings due to type conversions, uninitialized variables and so on in Visual Studio,
 // I decided to modify more parts of the original code below. I wanted to use variable types now with fixed defined bit lengths.

//extracts all images within data file into outputDir
int ExtractImages (char* datfname, char* tabfname, unsigned char* palette, char* outputDir)
{
    //Reading DAT,TAB and extracting images
    IMAGELIST images;
    {
        int retcode=create_images_dattab_idx(&images,datfname,tabfname,1);
        if (retcode!=0) return retcode;
    }

    //Looping through images and extracting to files
    long picnum;
    for (picnum=0;picnum<images.count;picnum++)
    {
        //printf ("\rExtracting: picture number %*d", 4, picnum);
        char finalpath[80];
        char fname[20];
        strcpy(&finalpath[0], &outputDir[0]);
        sprintf (fname, "%0*ld.bmp", 4, picnum);
        strcat(finalpath, fname);
        IMAGEITEM *item=&(images.items[picnum]);
        if ((item->width*item->height)>0)
            write_bmp_idx(finalpath, item->width, item->height, palette, (char*)(item->data), 0, 1, 2, 4);
    }

    free_dattab_images(&images);
    return 0;
}

int create_images_dattab_idx(IMAGELIST* images,char* datfname,char* tabfname,int verbose)
{
    char hlpstr[500];
    std::string msg("");

    if (verbose) 
        logging::Info("Reading TAB file ...");
    //Opening TAB file
    TABFILE tabf;
    {
        int retcode=read_tabfile_data(&tabf,tabfname);
        if (retcode!=0)
          switch (retcode)
          {
          case 1:
              if (verbose) {
                  snprintf(hlpstr, 500, "Error - Cannot open TAB file: %s", tabfname);
                  msg.clear();
                  msg.append(hlpstr);
                  logging::Error(msg);
              }
             return 11;
          default:
              if (verbose) { 
                  snprintf(hlpstr, 500, "Error - Loading TAB file %s returned fail code %d", tabfname, retcode); 
                  msg.clear();
                  msg.append(hlpstr);
                  logging::Error(msg);
              }
             return 19;
          }
    }
    if (verbose) 
        logging::Info("Done.");

    if (verbose) 
        logging::Info("Reading DAT file ...");
    //Opening DAT file
    DATFILE datf;
    {
        int retcode=read_datfile_data(&datf,datfname);
        if (retcode!=0)
          switch (retcode)
          {
          case 1:
              if (verbose) { 
                  snprintf(hlpstr, 500, "Error - Cannot open DAT file: %s", datfname); 
                  msg.clear();
                  msg.append(hlpstr);
                  logging::Error(msg);
              }
             return 21;
          default:
              if (verbose) { 
                  snprintf(hlpstr, 500, "Error - Loading DAT file %s returned fail code %d", datfname, retcode); 
                  msg.clear();
                  msg.append(hlpstr);
                  logging::Error(msg);
              }
             return 29;
          }
    }
    if (verbose) logging::Info("Done.");

    if (verbose)
    {
        logging::Info("");
        snprintf (hlpstr, 500, "The TAB file informs of %lu pictures.", tabf.count);
        msg.clear();
        msg.append(hlpstr);
        logging::Info(msg);
        if (tabf.filelength!=(unsigned long)((tabf.count+1)*6))
        {
            logging::Error("Warning - the TAB file contains incomplete entry at end.");
            logging::Error(" The truncated entry will be skipped.");
        }
        if (datf.count==-1)
        {
            logging::Error("The DAT file informs of 4bpp content.");
            logging::Error("Warning - this is 8bpp extractor!");
        }
        else {
            snprintf(hlpstr, 500, "The DAT file informs of %ld pictures with 8bpp.", datf.count);
            msg.clear();
            msg.append(hlpstr);
            logging::Info(msg);
        }
    }
    
    if (verbose) logging::Info("Decoding images ...");
    unsigned long readcount=2;
    read_dattab_images(images,&readcount,&tabf,&datf,verbose);
    if (verbose) logging::Info("Done.");

    if (verbose)
    {
        snprintf(hlpstr, 500, "Processed %lu of %zu bytes of DAT file.",readcount,datf.filelength);
        msg.clear();
        msg.append(hlpstr);
    
        int32_t unused= (int32_t)(datf.filelength)-(int32_t)(readcount);
        if (unused >= 0) {
            snprintf(hlpstr, 500, "Bytes skipped: %" PRIu32 "\n", unused);
            msg.append(hlpstr);
        }
        else {
            snprintf(hlpstr, 500, "Bytes overlapping: %" PRId32 "\n", unused);
            msg.append(hlpstr);
        }

        logging::Info(msg);
    }
    free_tabfile_data(&tabf);
    free_datfile_data(&datf);
    return 0;
}

void DebugWriteTabFileContentsCsvTable(char* tabFileName, TABFILE* tabf) {
    char finalpath[50];

    long dotPos = -1;

    strcpy(finalpath, tabFileName);

    //build new debug filename for text export
    for (unsigned long i = 0; i < strlen(finalpath); i++) {
        if (finalpath[i] == '.') {
            dotPos = i;
        }
    }

    //there is no Dot at all?
    if (dotPos == -1) {
        //just add new extension to existing file
        strcat(finalpath, "-tab.txt");
    } else {
        //remove everything after last dot including the dot
        //just do this by writing termination char to dot position
        finalpath[dotPos] = 0;

        strcat(finalpath, "-tab.txt");
    }

    //now we have our output filename
    FILE* oFile = fopen(finalpath, "w");
    if (oFile == nullptr) {
       return;
    }

    //iterate through available pictures
    //and write debug text for each one
    int entrynum;
    for (entrynum=0;entrynum<tabf->count;entrynum++)
    {
        TABFILE_ITEM *curitm=&(tabf->items[entrynum]);
        fprintf(oFile, "%lu;%u;%u\n", curitm->offset, curitm->width, curitm->height);
    }

    //close file
    fclose(oFile);
}

int read_tabfile_data(TABFILE* tabf,const char* srcfname, bool skipFirstEntry)
{
        FILE* tabfp;
        tabfp = fopen (srcfname, "rb");
        if (!tabfp)
        	return 1;

        tabf->filelength=(unsigned long)(file_length_opened(tabfp));
        tabf->count=tabf->filelength/6 - 1;
        tabf->items=static_cast<TABFILE_ITEM*>(malloc(tabf->count*sizeof(TABFILE_ITEM)));

        if (!tabf->items) { 
            fclose(tabfp);
            return 2; 
        }

        unsigned char tabitm[6];
        //Note about the next two lines: In the original xtabdat8.cpp code the fread command 2 lines below
        //is always active to skip reading the first entry. This is necessary for most of the assets in Hioctane.
        //But at least for the Texture Atlas file of the 3D models skipping the first item is a problem, as we are then
        //missing the first picture from the texture Atlas
        //Therefore I had to introduce a new parameter that skips the first entry for most of the assets (line
        //in original code before), but for the 3D Model texture model I am then able to read the first entry as well
        //Skipping first entry (should be empty)
        if (skipFirstEntry) {
            fread (tabitm, 6, 1, tabfp);
        }

        long entrynum;
        for (entrynum=0;entrynum<tabf->count;entrynum++)
        {
            TABFILE_ITEM *curitm=&(tabf->items[entrynum]);
            size_t readed=fread (tabitm, 1, 6, tabfp);
            curitm->offset=(uint32_t)(read_long_le_buf(tabitm));
            curitm->width=(uint8_t)(tabitm[4]);
            curitm->height=(uint8_t)(tabitm[5]);
            if (readed < 6) 
                return 3;
        }

        fclose(tabfp);

        return 0;
}

void free_tabfile_data(TABFILE* tabf)
{
    free(tabf->items);
    tabf->items=nullptr;
    tabf->filelength=0;
    tabf->count=0;
}

int read_datfile_data(DATFILE* datf, char* srcfname)
{
    FILE* datfp;
    datfp = fopen (srcfname, "rb");
    if (!datfp) 
        return 1;

    datf->count=read_short_le_file(datfp)-1;
    datf->filelength=(size_t)(file_length_opened(datfp));
    datf->data=static_cast<unsigned char*>(malloc(datf->filelength));
    if (!datf->data) { 
        fclose(datfp);
        return 2;
    }

    fseek(datfp, 0, SEEK_SET);
    size_t readed=fread(datf->data, 1, datf->filelength, datfp);
    fclose(datfp);
    if (readed < datf->filelength)
        return 3;

    return 0;
}

void free_datfile_data(DATFILE* datf)
{
    free(datf->data);
    datf->data=nullptr;
    datf->filelength=0;
    datf->count=0;
}

int read_dattab_images(IMAGELIST* images,unsigned long* readcount,TABFILE* tabf,DATFILE* datf,int verbose)
{
    char hlpstr[500];
    std::string msg("");

    images->count=tabf->count;
    images->items=static_cast<IMAGEITEM*>(malloc(sizeof(IMAGEITEM)*(images->count)));
    if (!images->items)
    {
        if (verbose) {
            snprintf(hlpstr, 500, "Error - cannot allocate %lu bytes of memory.", (unsigned long)(sizeof(IMAGEITEM) * images->count)); 
            msg.clear();
            msg.append(hlpstr);
            logging::Error(msg);
        }
        return 1;
    }
    //Looping through images
    long picnum;
    unsigned long errnum=0;
    unsigned long skipnum=0;
    for (picnum=0;picnum<images->count;picnum++)
    {
        TABFILE_ITEM *tabitem=&(tabf->items[picnum]);
        IMAGEITEM *item=&(images->items[picnum]);
        item->width=0;
        item->height=0;
        item->data=nullptr;
        item->alpha=nullptr;
        if (verbose) { 
            snprintf(hlpstr, 500, "Preparing picture%6lu from %06lx, %ux%u...", picnum, tabitem->offset, tabitem->width, tabitem->height); 
            msg.clear();
            msg.append(hlpstr);
            logging::Info(msg);
        }
        if (tabitem->offset >= datf->filelength)
            {
            if (verbose) { 
                logging::Error(" Skipped - Picture offset out of DAT filesize.");
            }
            skipnum++;
            continue;
            }
        if ((tabitem->width*tabitem->height) < 1)
            {
            if (verbose) { 
                logging::Error(" Skipped - Picture dimensions are invalid."); 
            }
            skipnum++;
            continue;
            }
        unsigned long readedsize;
        int retcode;
        retcode=read_dat_image_idx(item,&readedsize,datf,tabitem->offset,tabitem->width,tabitem->height);
        *readcount+=readedsize;
        if ((retcode&XTABDAT8_COLOUR_LEAK))
        {  
            if (verbose) { 
                logging::Error(" Error - colour leak out of picture size.");
            }
            errnum++;
        }
        else if ((retcode&XTABDAT8_ENDOFBUFFER))
        {  
            if (verbose) { 
                logging::Error(" Error - end of DAT buffer, picture truncated."); 
            }
            errnum++;
        }
    }
    if (verbose) { 
        snprintf(hlpstr, 500, "Images decoded, %lu skipped, %lu errors.", skipnum, errnum);
        msg.clear();
        msg.append(hlpstr);
        logging::Info(msg);
    }
    return errnum;
}

void free_dattab_images(IMAGELIST* images)
{
    long picnum;
    for (picnum=0;picnum<images->count;picnum++)
    {
        IMAGEITEM* item=&(images->items[picnum]);
        free(item->data);
        free(item->alpha);
    }
    free(images->items);
    images->items=nullptr;
    images->count=0;
}

int read_dat_image_idx(IMAGEITEM* image,unsigned long* readedsize,DATFILE* datf, uint32_t off, uint16_t width, uint16_t height)
{
    //Filling image structure
    {
        image->width=width;
        image->height=height;
        uint32_t imgsize=width*height;
        image->data=static_cast<unsigned char*>(malloc(imgsize));
        image->alpha=static_cast<unsigned char*>(malloc(imgsize));
        if ((image->data==nullptr)||(image->alpha==nullptr))
            return XTABDAT8_NOMEMORY;
        uint32_t i;
        for (i=0;i<imgsize;i++)
        {
            // Select color index when transparent
            image->data[i]=0;
            image->alpha[i]=255;
        }
    }
    //Code of error, if any occured
    int errorcode=0;
    //Counter of readed bytes
    uint32_t endoff=off; //Note 19.04.2024: geändert von long auf unsigned long, falls ab diesem Tag etwas nicht funktioniert
    //Position in buffer on height
    uint16_t row=0;
    //position in buffer on width
	uint16_t col=0;
    char g;

    //Time to decode picture
    while (row < height)
    {

        if (endoff < datf->filelength)
            g = (char)(datf->data[endoff]);
        else
            {g = 0;errorcode|=XTABDAT8_ENDOFBUFFER;}
	    endoff++;
	    if (g < 0)
	    {
           	col-=g;
        } else
        if (!g)
       	{
            col=0;
           	row++;
        } else //being here means that g>0
        {
            int i;
        	for (i=0; i < g; i++)
    		{
    		    if ((row >= height))
                {
                    //Colour leak on height - time to finish the work
                    errorcode|=XTABDAT8_COLOUR_LEAK;
                    break;
                } else
    		    if ((col > width))
                {
                    //Colour leak on width - going to next line
                    row++;col=0;
                    errorcode|=XTABDAT8_COLOUR_LEAK;
                } else
                if (col == width)
                {
                    //Do nothing - the error is small
                    errorcode|=XTABDAT8_COLOUR_LEAK;
                } else
    		    {
                    //No leak error
                    if (endoff < datf->filelength)
                    {
                        image->data[(width*row)+col]=datf->data[endoff];
                        image->alpha[(width*row)+col]=0;
                    }
                    else
                    {
                        errorcode|=XTABDAT8_ENDOFBUFFER;
                    }
                    endoff++;
                    col++;
                }
		    } //for (i=...
    	}
    }
	*readedsize=endoff-off;
	return errorcode;
}
