/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "string.h"
#include "stdlib.h"
#include "../utils/logging.h"

//-1 if item can not be accessed (unexpected error)
//0 if item is not a directory
//1 if item is a directory
int IsDirectoryPresent(const char* dirPath);

// throws an error message if the directory was not created
void CreateDirectory(const char *dirPath);

//-1 if item can not be accessed, means file does not exit
//1 if file exists
//0 specified element is not a file, but something else
int FileExists(const char *fname);

int copy_file(char* srcFileName, char* destFileName);

void PrepareSubDir(const char* dirName);

#endif // FILEUTILS_H
