Bullfrog games DAT/TAB Graphics extractor
------------------------------

You need to specify a data/tab file and a pal file.
Specify both names wthout extensions.
A load of .bmp files will be created in the current directory.

Note that some files may be compressed, you must decompress
 them using DERNC.

Usage: xtabdat8 <datafile> <palfile>
or:    xtabdat4 <datafile> <palfile>

Dungeon Keeper example:
xtabdat8 editicn1 main

Syndicate Wars example:
xtabdat8 icons0-0 s-proj

Syndicate example:
xtabdat4 hspr-0 hpal01

The source code files comes with Dev-C++ project files, so you can
 easily rebuild the source using Dev-C++ IDE.

Note:
Program displays number of skipped bytes at end. If this number is
less than number of bitmaps+128, then everything is OK.
If program says that some bytes are overlapping, something is wrong.
Also, if program says "colour leak" - it means that some bitmaps
will be incorrect.

Bullfrog games JTY/TAB Graphics extractor
------------------------------

You need to specify a jty/tab file and a pal file.
Specify both names wthout extensions.
A load of .bmp files will be created in the current directory.

Note that some files may be compressed, you must decompress
 them using DERNC.

Usage: xtabjty <datafile> <palfile>

Dungeon Keeper example:
xtabjty creature palette

Bullfrog Syndicate Wars Level Sprites extractor
------------------------------
Extracts the whole level sprites. Requires INF file, works only with
 Syndicate Wars.
UNFINISHED!

Usage: xswlvspr <????>

Revision history for the utils
------------------------------

Version: 1.11
 Added JTY/TAB extractor for Dungeon Keeper

Version: 1.10
 Transparency support in 4bpp code

Version: 1.09
 RNC detection in 4bpp code

Version: 1.08
 RNC compression detection

Version: 1.07
 Level Sprites extractor updates

Version: 1.03
 Updated BULCOMMN unit

Version: 1.02
 Added Level Sprites extractor

Version: 1.00
 First working one


DK Utilities author (8bpp extraction code):
Jon Skeet

Syndicale level viewer author (4bpp extraction code):
Andrew Sampson

Dev-C++ IDE version, modifications:
Tomasz Lis
