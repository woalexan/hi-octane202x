/*
 * dernc.h   define exported routines from dernc.c
 */

//source code taken from http://syndicate.lubiki.pl/synd/rework/synd_rework_rnc.php
/* For the original readme file please see bullfrog-utils-rnc-readme.txt

Author:
Jon Skeet

Dev-C++ IDE version, fixations:
Tomasz Lis

Note: This source code was slightly modified by me, and is not 100% the original code of the authors
anymore   */

#ifndef RNC_DERNC_H
#define RNC_DERNC_H

/*
 * Routines
 */

//function prototypes
long rnc_ulen (void *packed);
int main_unpack (char* iname, char* oname);
long rnc_unpack (unsigned char *packed, unsigned char *unpacked, unsigned int flags, long *leeway);
char *rnc_error (long errcode);
long rnc_crc (void *data, unsigned long len);

/*
 * Error returns
 */
#define RNC_FILE_IS_NOT_RNC    -1
#define RNC_HUF_DECODE_ERROR   -2
#define RNC_FILE_SIZE_MISMATCH -3
#define RNC_PACKED_CRC_ERROR   -4
#define RNC_UNPACKED_CRC_ERROR -5
#define RNC_HEADER_VAL_ERROR   -6
#define RNC_HUF_EXCEEDS_RANGE  -7

/*
 * Flags to ignore errors
 */
#define RNC_IGNORE_FILE_IS_NOT_RNC    0x0001
#define RNC_IGNORE_HUF_DECODE_ERROR   0x0002
#define RNC_IGNORE_FILE_SIZE_MISMATCH 0x0004
#define RNC_IGNORE_PACKED_CRC_ERROR   0x0008
#define RNC_IGNORE_UNPACKED_CRC_ERROR 0x0010
#define RNC_IGNORE_HEADER_VAL_ERROR   0x0020
#define RNC_IGNORE_HUF_EXCEEDS_RANGE  0x0040

/*
 * The compressor needs this define
 */
#define RNC_SIGNATURE 0x524E4301       /* "RNC\001" */

#endif
