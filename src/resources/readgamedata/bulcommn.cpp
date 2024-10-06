/*
 * common procedures for Bullfrog game tools
 * 
 *
 * This is just a module without main().
 */

//source code taken from http://syndicate.lubiki.pl/downloads/bullfrog_utils_tabdat.zip
//for readme file please see bullfrog_utils_tabdat-readme.txt
//DK Utilities author (8bpp extraction code):
//Jon Skeet

//Syndicale level viewer author (4bpp extraction code):
//Andrew Sampson

//Dev-C++ IDE version, modifications:
//Tomasz Lis

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include "bulcommn.h"

void write_short_le_file (FILE *fp, unsigned short x)
{
    fputc ((int) (x&255), fp);
    fputc ((int) ((x>>8)&255), fp);
}

void write_short_le_buf (unsigned char *buff, unsigned short x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
}

void write_long_le_file (FILE *fp, unsigned long x)
{
    fputc ((int) (x&255), fp);
    fputc ((int) ((x>>8)&255), fp);
    fputc ((int) ((x>>16)&255), fp);
    fputc ((int) ((x>>24)&255), fp);
}

void write_long_le_buf (unsigned char *buff, unsigned long x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
    buff[2]=((x>>16)&255);
    buff[3]=((x>>24)&255);
}

long read_long_le_file (FILE *fp)
{
    long l;
    l = fgetc (fp);
    l += fgetc (fp)<<8;
    l += fgetc (fp)<<16;
    l += fgetc (fp)<<24;
    return l;
}

long read_long_le_buf (unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    l += buff[2]<<16;
    l += buff[3]<<24;
    return l;
}


long read_long_le_buf2 (unsigned char *buff)
{
    long l;
    l = buff[4];
    l += buff[5]<<8;
    l += buff[6]<<16;
    l += buff[7]<<24;
    return l;
}

unsigned short read_short_le_buf (unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    return l;
}

unsigned short read_short_le_file (FILE *fp)
{
    unsigned short l;
    l = fgetc (fp);
    l += fgetc (fp)<<8;
    return l;
}

long read_long_be_buf (unsigned char *buff)
{
    long l;
    l =  buff[3];
    l += buff[2]<<8;
    l += buff[1]<<16;
    l += buff[0]<<24;
    return l;
}

void write_short_be_buf (unsigned char *buff, unsigned short x)
{
    buff[1]=(x&255);
    buff[0]=((x>>8)&255);
}

void write_long_be_buf (unsigned char *buff, unsigned long x)
{
    buff[3]=(x&255);
    buff[2]=((x>>8)&255);
    buff[1]=((x>>16)&255);
    buff[0]=((x>>24)&255);
}

void write_bmp_idx (char *fname, int width, int height, 
		unsigned char *pal, char *data,
		int red, int green, int blue, int mult)
{
    long l;
    int i, j;
    FILE *out;
    
    out = fopen (fname, "wb");
    if (!out)
    {
	printf ("\nCan't open file %s. Aborting.\n", fname);
	exit (1);
    }
    
    l = width*height;
    fprintf (out, "BM");
    write_long_le_file (out, l+0x436);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0x436);
    write_long_le_file (out, 40);
    write_long_le_file (out, width);
    write_long_le_file (out, height);
    write_short_le_file (out, 1);
    write_short_le_file (out, 8);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    
    for (i=0; i < 256; i++)
    {
	fputc (pal[i*3+blue]*mult, out);
	fputc (pal[i*3+green]*mult, out);
	fputc (pal[i*3+red]*mult, out);
	fputc (0, out);
    }
    
    for (i=1; i <= height; i++)
    {
	fwrite (data+(height-i)*width, width, 1, out);
	if (width & 3)
	    for (j=0; j < 4-(width&3); j++)
		fputc (0, out);
    }
    
    fclose (out);
}

void write_bmp_24b (char *fname, int width, int height, char *data)
{
    long l;
    int i, j;
    FILE *out;
    
    out = fopen (fname, "wb");
    if (!out)
    {
	printf ("\nCan't open file %s. Aborting.\n", fname);
	exit (1);
    }
    
    l = width*height;
    fprintf (out, "BM");
    write_long_le_file (out, 3*l+0x036);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0x036);
    write_long_le_file (out, 40);
    write_long_le_file (out, width);
    write_long_le_file (out, height);
    write_short_le_file (out, 1);
    write_short_le_file (out, 24);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    write_long_le_file (out, 0);
    
    int datawidth=width*3;
    for (i=1; i <= height; i++)
    {
        fwrite (data+(height-i)*datawidth, datawidth, 1, out);
        int padding_size=4-(datawidth&3);
        if ((padding_size&3) > 0)
        {
            int cntr;
            for (cntr=0;cntr<padding_size;cntr++)
    	        fputc (0, out);
        }
    }
    
    fclose (out);
}


long file_length (char *path)
{
    FILE *fp;
    long ret;
    
    fp = fopen (path, "rb");
    if (!fp)
	return -1;
    fseek (fp, 0, SEEK_END);
    ret = ftell (fp);
    fclose (fp);
    return ret;
}

long file_length_opened (FILE *fp)
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

char *filename_from_path(char *pathname)
{
    char *fname = NULL;
    if (pathname)
    {
        fname = strrchr (pathname, '/') + 1;
        if (!fname)
            fname = strrchr (pathname, '\\') + 1;
    }
    if (!fname)
        fname=pathname;
return fname;
}

int nth_bit( unsigned char c, int n ) {
    // returns the nth bit of c
    // either 1 or 0
    // ...hooray for cse271...
    
    unsigned char one = 1;
    
    if( n < 0 || n > 7 )
        return 0;
    
    c = c>>n;
    return (int)(c & one);
    
}

int nth_bit_fourbytes( unsigned char c[4], int n ) {
    // returns the nth bit of c
    // either 1 or 0
    // goes like this:
    // [31 30 29 28 27 26 25 24][...][...][7 6 5 4 3 2 1 0]
    
    if( n < 0 || n > 32 )
        return 0;
    
    if( n < 8 )    // bits 0 - 7
        return nth_bit( c[3], n );
    else if( n < 16 )
        return nth_bit( c[2], n%8 );
    else if( n < 24 )
        return nth_bit( c[1], n%8 );
    else
        return nth_bit( c[0], n%8 );
    
}

int read_palette_rgb(unsigned char *palette, char *fname, unsigned int nColors)
{
    FILE *palfp;
    palfp = fopen (fname, "rb");
    if (!palfp) return 1;
    int palSize=3*nColors;
    int readed=fread (palette, 1, palSize, palfp);
    fclose(palfp);
    if (palSize!=readed) return 2;
    return 0;
}

// Returns 0 if buff seems not to be RNC compressed,
// or RNC version (positive number) if buff is compressed.
// Requies buff to be at least 4 bytes long.
int rnc_compressed_buf (unsigned char *buff)
{
    if (strncmp((char*)(buff),RNC_SIGNATURE_STR,3)!=0)
        return 0;
    int rncver=buff[3];
    if (rncver==0) rncver=255;
    return rncver;
}

int rnc_compressed_file (FILE *fp)
{
    unsigned char buff[5];
    long lastpos = ftell (fp);
    int readed=fread (buff, 1, 4, fp);
    fseek (fp, lastpos, SEEK_SET);
    if (readed<4) return 0;
    return rnc_compressed_buf(buff);
}
