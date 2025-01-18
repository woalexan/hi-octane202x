
/*
 objectdatfile.h Source code to convert Bullfrog object data files to Wavefront Obj files
 
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024 Wolf Alexander       (I did translation to C++, I later added the normals calculation
                                          and texture support as well.)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)
 
 A big thank you to movAX13h and srtuss for their great work on reverese engineering Hi-Octane!

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef BULL_OBJECTDATFILE_H
#define BULL_OBJECTDATFILE_H

#include "../../utils/fileutils.h"
#include "xtabdat8.h"
#include <vector>
#include <sstream>
#include "irrlicht/irrlicht.h"

#define OBJDATHEADER "BULLFROG OBJECT DATA"
#define OBJDATHEADERLENGTH 58

typedef struct {
     int A;
     int B;
     int C;
}  __attribute__((packed)) ObjTriangle;

typedef struct {
     float X;
     float Y;
     float Z;
   }  __attribute__((packed)) ObjVertex;

class ObjectDatFile {

private:
    std::vector<ObjTriangle*> *ObjFileTriangleVector;
    std::vector<ObjVertex*> *ObjFileVertexVector;
    std::vector<float> *uvCoordVec;
    std::vector<ObjVertex*> *ObjFileNormalsVector;

    TABFILE* mAtlasTextureInfo;
    unsigned int mTexAtlasWidth;
    unsigned int mTexAtlasHeight;
    FILE* debugTriangleOutputFile = NULL;
    bool debugFileOutputFailed = false;

    int BitConverterToInt16(unsigned char* data, int index);
    bool CreateMtlFile(char* outFilename);
    void CalculateAndAddNormals(ObjVertex* v1, ObjVertex* v2, ObjVertex* v3);
    void DebugWriteTriangleCsvFile(char* debugOutPutFileName, int triangleNr, int texID, TABFILE_ITEM* texItem);

public:
    ObjectDatFile(TABFILE* pntrModelTextureAtlasInfo, unsigned int texAtlasWidth,
                  unsigned int texAtlasHeight);
    ~ObjectDatFile();

    //Routines
    bool LoadObjectDatFile(const char* filename);
    bool WriteToObjFile(const char* filename, const char* objectname);
    bool TestBitConverterToInt16();

    bool ConversionSuccesful;
};

#endif
