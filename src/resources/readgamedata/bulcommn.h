/*
 * bulcommn.h   define exported routines from bulcommn.c
 */

#ifndef BULL_BULCOMMN_H
#define BULL_BULCOMMN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <cstdint>
#include <string>
#include "../../utils/logging.h"

//source code taken from http://syndicate.lubiki.pl/downloads/bullfrog_utils_tabdat.zip
//for readme file please see bullfrog_utils_tabdat-readme.txt
//DK Utilities author (8bpp extraction code):
//Jon Skeet

//Syndicale level viewer author (4bpp extraction code):
//Andrew Sampson

//Dev-C++ IDE version, modifications:
//Tomasz Lis

 //Note 19.03.2025: In an attempt to reduce/remove the warnings due to type conversions, uninitialized variables and so on in Visual Studio,
 // I decided to modify more parts of the original code below. I wanted to use variable types now with fixed defined bit lengths.

// RNC compression magic identifier
#define RNC_SIGNATURE_STR "RNC\001"

// Routines

long file_length (char *path);
long file_length_opened (FILE *fp);
char *filename_from_path(char *pathname);

int32_t read_long_le_file (FILE *fp);
int32_t read_long_le_buf (unsigned char *buff);
uint16_t read_short_le_file (FILE *fp);
uint16_t read_short_le_buf (unsigned char *buff);

void write_short_le_file (FILE *fp, uint16_t x);
void write_short_le_buf (unsigned char *buff, uint16_t x);
void write_long_le_file (FILE *fp, uint32_t x);
void write_long_le_buf (unsigned char *buff, uint32_t x);

int32_t read_long_be_buf (unsigned char *buff);
int32_t read_long_le_buf2 (unsigned char *buff);

void write_short_be_buf (unsigned char *buff, uint16_t x);
void write_long_be_buf (unsigned char *buff, uint32_t x);

int nth_bit( unsigned char c, int n );
int nth_bit_fourbytes( unsigned char c[4], int n );

void write_bmp_idx (char *fname, uint16_t width, uint16_t height, unsigned char *pal,
		char *data, int16_t red, int16_t green, int16_t blue, int16_t mult);
void write_bmp_24b (char *fname, uint16_t width, uint16_t height, char *data);

int read_palette_rgb(unsigned char *palette, char *fname, uint16_t nColors);

int rnc_compressed_buf (unsigned char *buff);
int rnc_compressed_file (FILE *fp);

#endif