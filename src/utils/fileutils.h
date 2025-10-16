/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <stdlib.h>

//-1 if item can not be accessed (unexpected error)
//0 if item is not a directory
//1 if item is a directory
int IsDirectoryPresent(const char* dirPath);

// throws an Exception if the directory was not created
void CreateDirectory(const char *dirPath);

//-1 if item can not be accessed, means file does not exit
//1 if file exists
//0 specified element is not a file, but something else
int FileExists(const char *fname);

//Returns file size in bytes, Returns 0 in case file does
//not exist or can not be opened
std::size_t GetFileSizeBytes(const char *fname);

//Returns 1 in case of unexpected error, 0 for success
int copy_file(const char* srcFileName, const char* destFileName);

void PrepareSubDir(const char* dirName);

#endif // FILEUTILS_H
