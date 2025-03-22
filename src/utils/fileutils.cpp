/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "fileutils.h"

#include <stdexcept>
#include <filesystem>

/*int copy_file(char* iname, char* oname)
{
    char *sysbuf;

    sysbuf = static_cast<char*>(malloc (strlen (iname)+strlen(oname)+6));
    if (!sysbuf)
    {
    fprintf (stderr, "Out of memory.\n");
    return 1;
    }

    strcpy (sysbuf, "cp ");
    strcat (sysbuf, iname);
    strcat (sysbuf, " ");
    strcat (sysbuf, oname);
    system (sysbuf);
    free (sysbuf);
    return 0;
}*/

//source code example taken from https://markaicode.com/how-to-copy-file-contents-in-c/
int copy_file(char* srcFileName, char* destFileName) {
    FILE* sourceFile;
    FILE* destFile;
    char buffer[4096];
    size_t bytesRead;

    sourceFile = fopen(srcFileName, "rb");
    if (sourceFile == NULL) {
        printf("Error opening source file!\n");
        return 1;
    }

    destFile = fopen(destFileName, "wb");
    if (destFile == NULL) {
        printf("Error creating destination file!\n");
        fclose(sourceFile);
        return 1;
    }

    while ((bytesRead = fread(buffer, 1, 4096, sourceFile)) > 0) {
        fwrite(buffer, 1, bytesRead, destFile);
    }

    fclose(sourceFile);
    fclose(destFile);

    return 0;
}

//-1 if item can not be accessed (unexpected error)
//0 if item is not a directory
//1 if item is a directory
int IsDirectoryPresent(const char* dirPath) {
    struct stat info;

    if( stat( dirPath, &info ) != 0 )
        return (-1); //can not access item
    else if( info.st_mode & S_IFDIR )
        return (1); //is a directory
    else
        return (0); //is no directory
}

//-1 if item can not be accessed, means file does not exit
//1 if file exists
//0 specified element is not a file, but something else
int FileExists(const char *fname)
{
    struct stat info;

    if( stat( fname, &info ) != 0 )
        return (0); //can not access item, file should be missing
    else if( info.st_mode & S_IFREG )
        return (1); //is a file, does exist
    else
        return (0); //is something else
}

//Returns 0 if directory was created succesfully
//returns 1 if directory was not created due to problem
void CreateDirectory(const char *dirPath) {
    //16.03.2025: mkdir not available under windows
    // therefore replaced
    //int check = mkdir(dirPath, 0777);

     // check if directory is created or not
    /*/if (check != 0) {
        throw "Error creating directory: " + std::string(dirPath);
    }*/

    namespace fs = std::filesystem;
    fs::create_directories(dirPath);

    // check if directory is now existing
    if (!(IsDirectoryPresent(dirPath) == 1)) {
        throw "Error creating directory: " + std::string(dirPath);
    }
}

void PrepareSubDir(const char* dirName) {
    //check if specified directory is already present
    //if not create this directory
    if (IsDirectoryPresent(dirName) == -1) {
        //directory is not there
        CreateDirectory(dirName);
    }
}
