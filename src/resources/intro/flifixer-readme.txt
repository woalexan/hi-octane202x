Autodesk Animator FLI files fixer
------------------------------
    written by Tomasz Lis, Gdansk, Poland 2004-2006
    this software is under GNU license

This little tool can analyze FLI file headers and fix
 informations in them. It can be used to repair any
 damaged FLI file.

I've made this one to repair very specific files,
 but my solution is very common and can be used in
 many cases. My specific case was to repair FLI
 files used in Bullfrog games. These files were in
 FLI format, but the header was cutted to avoid opening
 them in FLI viewers.

This program fixes all headers, but animation data
 is not changed. It means that after using this tool,
 there is still a chanse that file has errors.

Usage:  flifix [options] <srcFile> [destFile]

Where
  <srcFile>  is a FLI file to fix
  [destFile] is an output file, set to 'repaired.fli' if no other name typed
  [options]  can be -v -m -f -p -n -b -c -a -l -u -r -k -s<num> -e<num>

Options:
-v      poDisplayAllInfo     Displays a lot of informatons - highly
                              verbose mode

-m      poFixMainHeader      Determines if MainHeader will be corrected
                              to real values

-f      poFixFrameHeaders    Without this option, frame repair function
                              just ends itself

-p      poFixFramePositions  Determines if frame positions should be
                              adjusted by searching the file around given offset

-n      poUseFrameFinder     FrameFinder allows to search for frames in file
                              if on first parsing not all of them has been found

-b      poRemoveBadChunks    If chunk seems to be destroyed, it is removed

-c      poFixChunkHeaders    Determines if chunk headers should be corrected

-a      poNeverWaitForKey    Never asks for pressing a key to continue

-l      poSimpleFix          Only header is fixed, other data only copied.
                              Some other options won't work with this.

-u      poManualSeeking      Alternate method of getting offsets in files

-r      poRadicalFrameHdrFix It decreases the range of values in FrameHeaders
                              whitch are assumed proper

-k      poNeverSkipFrames    With this option, frames witch looks bad won't be removed

-s<num> startFrame           Allows to write to output only a range of frames

-e<num> endFrame             Allows to write to output only a range of frames


Example of executing program to fix Syndicate FLI file:
(file must be decompressed with DERNC first)
flifix -v -m -f -p -n -b -c -a mendlose.dat mendlosefix.fli

Example of executing program to fix Syndicate Wars FLI file:
flifix -v -m -f -p -n -b -c -a mod-01.fli mod-01fix.fli

If you don't know whitch options to use, try those from example.
If you run program without options, just filename, it will not
 work properly. It requires at least -p -n to find frames.

If the program won't find palette information in first frame of animation,
 it will load palette from FLIFix.pal file.

The source code files comes with Dev-C++ project files, so you can
 easily rebuild the source using Dev-C++ IDE. 


Version: 1.02
 Corrected fixChunkHeader and validFrame so program works on MacSyndicate animations

Version: 1.01
 Corrected validFrame for empty frames
 Fixed some memory allocation problems
 Converted from Borland C 3.1 to GCC and Dev-C++
 Added parameters, but startFrame and endFrame unfinished yet

Version: 1.00
 First working one

Author:
Tomasz Lis
