
/*
 objectdatfile.cpp Source code to convert Bullfrog object data files to Wavefront Obj files
 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <limits.h>

#include "objectdatfile.h"

ObjectDatFile::ObjectDatFile(TABFILE* pntrModelTextureAtlasInfo, unsigned int texAtlasWidth,
                             unsigned int texAtlasHeight) {
    uvCoordVec = new std::vector<float>();
    this->ObjFileVertexVector = new std::vector<ObjVertex*>;
    this->ObjFileTriangleVector = new std::vector<ObjTriangle*>;
    this->ObjFileNormalsVector = new std::vector<ObjVertex*>;
    this->mAtlasTextureInfo = pntrModelTextureAtlasInfo;
    this->mTexAtlasWidth = texAtlasWidth;
    this->mTexAtlasHeight = texAtlasHeight;

    ConversionSuccesful = true;

    //check one time for correctness of BitConverterToInt16 function on this
    //system/environment/compiler
    if (!TestBitConverterToInt16()) {
        //there is a problem, can not continue
        ConversionSuccesful = false;
    }
}

ObjectDatFile::~ObjectDatFile() {
    delete uvCoordVec;

    std::vector<ObjVertex*>::iterator it;
    ObjVertex* pntr;

    //delete all existing vertices
    it = ObjFileVertexVector->begin();

    while (it != ObjFileVertexVector->end()) {
           pntr = (*it);
           it = ObjFileVertexVector->erase(it);
           delete pntr;
    }

    delete ObjFileVertexVector;

    //delete all existing normals
    it = ObjFileNormalsVector->begin();

    while (it != ObjFileNormalsVector->end()) {
           pntr = (*it);
           it = ObjFileNormalsVector->erase(it);
           delete pntr;
    }

    delete ObjFileNormalsVector;

    std::vector<ObjTriangle*>::iterator it2;
    ObjTriangle* pntr2;

    //delete all existing triangles
    it2 = ObjFileTriangleVector->begin();

    while (it2 != ObjFileTriangleVector->end()) {
           pntr2 = (*it2);
           it2 = ObjFileTriangleVector->erase(it2);
           delete pntr2;
    }

    delete ObjFileTriangleVector;
}

int ObjectDatFile::BitConverterToInt16(unsigned char* data, int index) {
   //only works for little endian machine!!!
   unsigned int val = (data[index + 1] << 8) + data[index];

   int value = (int16_t)(val);

   return value;
}

void ObjectDatFile::CalculateAndAddNormals(ObjVertex* v1, ObjVertex* v2, ObjVertex* v3) {
    irr::core::vector3df irrV1(v1->X, v1->Y, v1->Z);
    irr::core::vector3df irrV2(v2->X, v2->Y, v2->Z);
    irr::core::vector3df irrV3(v3->X, v3->Y, v3->Z);

    irr::core::vector3df dirVec1 = (irrV2 - irrV1);
    irr::core::vector3df dirVec2 = (irrV3 - irrV2);

    irr::core::vector3df normalVec = dirVec1.crossProduct(dirVec2).normalize();

    ObjVertex* newNormal = new ObjVertex;
    newNormal->X = (float)(normalVec.X);
    newNormal->Y = (float)(normalVec.Y);
    newNormal->Z = (float)(normalVec.Z);

    //Add to my vector with normals
    this->ObjFileNormalsVector->push_back(newNormal);
}

void ObjectDatFile::DebugWriteTriangleCsvFile(char* debugOutPutFileName, int triangleNr, int texID, TABFILE_ITEM* texItem) {
    if (debugFileOutputFailed)
        return;

    //do we need to still create the debug file?
    if (debugTriangleOutputFile == NULL) {
        char finalpath[50];

        //long dotPos = -1;

        strcpy(finalpath, debugOutPutFileName);
     /*
        //build new debug filename for text export
        for (ulong i = 0; i < strlen(finalpath); i++) {
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
         //just do this by writing NULL char to dot position
         finalpath[dotPos] = 0;

         strcat(finalpath, "-tab.txt");
     }*/

        //now we have our output filename
        debugTriangleOutputFile = fopen(finalpath, "w");
        if (debugTriangleOutputFile == NULL) {
            debugFileOutputFailed = true;
           return;
            }
        }

    //write the next entry
    fprintf(debugTriangleOutputFile, "%d;%d;%lu;%u;%u\n", triangleNr, texID, texItem->offset, texItem->width, texItem->height);
}

bool ObjectDatFile::LoadObjectDatFile(const char* filename) {

    if (!ConversionSuccesful) {
        printf("\nError - ObjectDatFile: BitConverterToInt16 failed unit test on this computer!\n");
        return false;
    }

      //make sure file exists
      if (FileExists(filename) != 1) {
          //file does not exist, or item is actually no file
          printf("\nError - The following file does not exist: %s\n",filename);
          return false;
      }

      bool isAnimation = false;

      FILE* iFile;

      iFile = fopen(filename, "rb");
      fseek(iFile, 0L, SEEK_END);
      //get overall filesize
      size_t size = ftell(iFile);

      //seek back to start of file
      fseek(iFile, 0L, SEEK_SET);

      size_t counter = 0;

      //read overall file into memory
      char* ByteArray;
      ByteArray = new char[size + 1];
      if (iFile != NULL)
      {
          do {
              ByteArray[counter] = fgetc(iFile);
              counter++;
          } while (counter < size);
          fclose(iFile);
      } else {
          printf("\nError - The following file can not be opened: %s\n",filename);
          return false;
      }

      char headerbuf[30];
      //copy expected header string for comparison
      strcpy(headerbuf, OBJDATHEADER);
      int headerLen = strlen(headerbuf);

      //now validate header Start
      char headerRead[headerLen+1];

      //copy read header from file
      for (int d = 0; d < headerLen; d++) {
          headerRead[d] = ByteArray[d];
      }

      headerRead[headerLen] = 0;

      bool headerValid = true;

      if (strcmp(headerbuf, headerRead) != 0)
             headerValid = false;

      if (!headerValid) {
          printf("\nError - The header in the following file is invalid: %s\n",filename);
          return false;
      }

      headerValid = true;

       //4 zerobytes should follow, verify
      for (uint a = 0; a < 4; a++) {
        if (ByteArray[headerLen + a] != 0)
          headerValid = false;
      }

      //one of the 4 zeros after the header is missing, exit
      if (!headerValid)
      {
          printf("\nError - One of the 4 Zeros after the header was not found: %s\n",filename);
          return false;
      }

      //get numBytesTriDef
      int numBytesTriDef = (unsigned char)(ByteArray[headerLen + 4]) | ((unsigned char)(ByteArray[headerLen + 5]) << 8);
      if (numBytesTriDef == 0) {
          printf("\nError - File contains zero triangles: %s\n",filename);
          return false;
      }

      // 2 zerobytes (or 2 more bytes of numBytesTriDef maybe)
      if ((ByteArray[headerLen + 6] != 0) || (ByteArray[headerLen + 7] != 0))
      {
          printf("\nError - File does not contain 2 zerobytes at location headerLen + 6 and headerLen + 7: %s\n",filename);
          return false;
      }

      int numBytesVertexData = (unsigned char)(ByteArray[headerLen + 8]) | ((unsigned char)(ByteArray[headerLen + 9]) << 8);

      if (numBytesVertexData == 0) {
          printf("\nError - File contains zero VertexData: %s\n",filename);
          return false;
      }

      // 2 zerobytes (or 2 more bytes of numBytesVertexData maybe)
      if ((ByteArray[headerLen + 10] != 0) || (ByteArray[headerLen + 11] != 0))
      {
        printf("\nError - File does not contain 2 zerobytes at location headerLen + 10 and headerLen + 11: %s\n",filename);
        return false;
      }

      //num 3, same as numVertices
      int num3 = (unsigned char)(ByteArray[headerLen + 12]) | ((unsigned char)(ByteArray[headerLen + 13]) << 8);

      if (num3 == 0) {
          printf("\nError - Invalid num3 value found: %s\n",filename);
          return false;
        }

      // 2 zerobytes (or 2 more bytes of num 3 maybe)
      if ((ByteArray[headerLen + 14] != 0) || (ByteArray[headerLen + 15] != 0))
      {
        printf("\nError - File does not contain 2 zerobytes at location headerLen + 14 and headerLen + 15: %s\n",filename);
        return false;
      }

      //2 constants (05 and 01, same in all object files)
      if (ByteArray[headerLen + 16] != 5 && ByteArray[headerLen + 16] != 7) {
          printf("\nError - Constant 1 is %u, not 5 or 7 in file: %s\n",ByteArray[headerLen + 16], filename);
          return false;
      }

      //is it an Animation?
      if (ByteArray[headerLen + 16] == 7) isAnimation = true;

      //3 constants (01 and 04 and 20, same in all object files)
      if (ByteArray[headerLen + 17] != 1 && ByteArray[headerLen + 17] != 4 && ByteArray[headerLen + 17] != 20) {
          printf("\nError - Constant 2 is %u, not 1, 4 or 20 in file: %s\n",ByteArray[headerLen + 17], filename);
          return false;
      }

      //numTriangles
      int numTriangles = (unsigned char)(ByteArray[headerLen + 18]) | ((unsigned char)(ByteArray[headerLen + 19]) << 8);
      if (numTriangles == 0) {
          printf("\nError - Invalid numTriangles found: %s\n",filename);
          return false;
      }

      //numVertices
      int numVertices = (unsigned char)(ByteArray[headerLen + 20]) | ((unsigned char)(ByteArray[headerLen + 21]) << 8);
      if (numVertices == 0) {
          printf("\nError - Invalid numVertices found: %s\n",filename);
          return false;
      }

      //2 constants (4F and 00, same in all object files)
      if (ByteArray[headerLen + 22] != 0x4f) {
          printf("\nError - Constant 3 is %u, not 0x4F in file: %s\n",ByteArray[headerLen + 22], filename);
          return false;
      }

      if (ByteArray[headerLen + 23] != 0) {
          printf("\nError - Constant 4 is %u, not 0 in file: %s\n",ByteArray[headerLen + 23], filename);
          return false;
      }

      //start reading triangle definitions
      unsigned int offset = OBJDATHEADERLENGTH;

      TABFILE_ITEM* pntrItem;

      float texWidth;
      float texHeight;
      unsigned long texOffset;

      unsigned int srcXLoc;
      unsigned int srcYLoc;
      float uPosLocal;
      float vPosLocal;
      float uGlobal;
      float vGlobal;

      //char dbgFileName[70];
      //strcpy(dbgFileName, "extract/dbg_UV.txt");

      for(int i = 0; i < numTriangles; i++)
      {
          unsigned char type = (unsigned char)(ByteArray[offset]);

          // Animations have some zeros before actual triangle data.
          // I couldn't find a regularity in the number of bytes used.
          while (isAnimation && type == 0)
          {
              offset++;
              type = (unsigned char)(ByteArray[offset]);
          }

          if (type != 5 && type != 4)
          {
              printf("\nError - Triangle #%d has unknown type %u in file: %s\n", i+1, type, filename);
              return false;
          }

          offset++;
          offset++; // skip zero

          // 3 indices of the triangle
          ObjTriangle *newTriangle = new ObjTriangle();
          newTriangle->A = ByteArray[offset + 0];
          newTriangle->B = ByteArray[offset + 1];
          newTriangle->C = ByteArray[offset + 2];

          offset += 3;

          if (type == 5) {
              //first byte contains the texture ID of the
              //correct texture in the texture Atlas
              int texId = (unsigned char)(ByteArray[offset + 0]);

              //the next 6 bytes contain the UV coordinates
              int UV0 = ByteArray[offset + 1];
              int UV1 = ByteArray[offset + 2];
              int UV2 = ByteArray[offset + 3];
              int UV3 = ByteArray[offset + 4];
              int UV4 = ByteArray[offset + 5];
              int UV5 = ByteArray[offset + 6];

              //get Atlas texture info which belongs to this texture Id
              pntrItem = &this->mAtlasTextureInfo->items[texId];

              //we do not have a valid pointer, quit with error!
              if (pntrItem == NULL) {
                  return false;
              }

              texWidth = (float)(pntrItem->width);
              texHeight = (float)(pntrItem->height);
              texOffset = pntrItem->offset;

              //calculate start upper left corner of source rect in original Atlas, depending on
              //Offset value for current picture
              srcYLoc = (texOffset / this->mTexAtlasWidth);
              srcXLoc =  texOffset - (srcYLoc * this->mTexAtlasWidth);

              uPosLocal = srcXLoc + float(UV0);
              vPosLocal = srcYLoc + float(UV1);

              uGlobal = (uPosLocal / (float)(this->mTexAtlasWidth));
              vGlobal = (vPosLocal / (float)(this->mTexAtlasHeight));

              uvCoordVec->push_back(uGlobal);
              uvCoordVec->push_back(vGlobal);

              uPosLocal = srcXLoc + float(UV2);
              vPosLocal = srcYLoc + float(UV3);

              uGlobal = (uPosLocal / (float)(this->mTexAtlasWidth));
              vGlobal = (vPosLocal / (float)(this->mTexAtlasHeight));

              uvCoordVec->push_back(uGlobal);
              uvCoordVec->push_back(vGlobal);

              uPosLocal = srcXLoc + float(UV4);
              vPosLocal = srcYLoc + float(UV5);

              uGlobal = (uPosLocal / (float)(this->mTexAtlasWidth));
              vGlobal = (vPosLocal / (float)(this->mTexAtlasHeight));

              uvCoordVec->push_back(uGlobal);
              uvCoordVec->push_back(vGlobal);

              //DebugWriteTriangleCsvFile(dbgFileName, i, texId, pntrItem);

              offset += 7; // presumably 1 byte texture atlas id, 6 bytes tex coords
          }
            else {
              offset ++;

              uvCoordVec->push_back(0.0f);
              uvCoordVec->push_back(0.0f);
              uvCoordVec->push_back(0.0f);
              uvCoordVec->push_back(0.0f);
              uvCoordVec->push_back(0.0f);
              uvCoordVec->push_back(0.0f);
/*
              if (debugTriangleOutputFile != NULL) {
                 fprintf(debugTriangleOutputFile, "triangle != type 5\n");
              }*/

          }

          //add new triangle to file triangle vector
          ObjFileTriangleVector->push_back(newTriangle);
      }

      //read vertices
      for(int i = 0; i < numVertices; i++)
      {
          ObjVertex *newVertex = new ObjVertex();

          newVertex->X = BitConverterToInt16((unsigned char*)(&ByteArray[0]), offset) / (float)(1024.0f);
          offset += 2;
          newVertex->Y = BitConverterToInt16((unsigned char*)(&ByteArray[0]), offset) / (float)(1024.0f);
          offset += 2;
          newVertex->Z = BitConverterToInt16((unsigned char*)(&ByteArray[0]), offset) / (float)(1024.0f);
          offset += 2;
          ObjFileVertexVector->push_back(newVertex);
      }

      // we don't know what the rest does so we skip it

      //Bullfrog object files do not contain normal information
      //but we can calculate the normals ourself, and store them
      //in the wavefront file as well

      //calculate and add normals
      ObjVertex* pntrVert1;
      ObjVertex* pntrVert2;
      ObjVertex* pntrVert3;
      std::vector<ObjTriangle*>::iterator itTri;

      for (itTri = ObjFileTriangleVector->begin(); itTri != ObjFileTriangleVector->end(); ++itTri) {
             pntrVert1 = ObjFileVertexVector->at((*itTri)->A);
             pntrVert2 = ObjFileVertexVector->at((*itTri)->B);
             pntrVert3 = ObjFileVertexVector->at((*itTri)->C);

             //calculate next normal and add it to my vector of normals
             CalculateAndAddNormals(pntrVert1, pntrVert2, pntrVert3);
          }

      /*if (debugTriangleOutputFile != NULL) {
          fclose(debugTriangleOutputFile);
          debugTriangleOutputFile = NULL;
      }*/

      delete[] ByteArray;

      return true;
}

//Maybe on another computer this code will not work anymore
//therefore I wanted to include this "unit" test
bool ObjectDatFile::TestBitConverterToInt16() {
    unsigned char ByteArray[11];

    //This test example comes from
    //https://learn.microsoft.com/de-de/dotnet/api/system.bitconverter.toint16?view=net-8.0

    //Testdata
    ByteArray[0] = 15;
    ByteArray[1] = 0;
    ByteArray[2] = 0;
    ByteArray[3] = 128;
    ByteArray[4] = 16;
    ByteArray[5] = 39;
    ByteArray[6] = 240;
    ByteArray[7] = 216;
    ByteArray[8] = 241;
    ByteArray[9] = 255;
    ByteArray[10] = 127;

    int Res = BitConverterToInt16(&ByteArray[0], 1);
    if (Res != 0)
        return false;

    Res = BitConverterToInt16(&ByteArray[0], 0);
    if (Res != 15)
        return false;

    Res = BitConverterToInt16(&ByteArray[0], 8);
    if (Res != -15)
        return false;

    Res = BitConverterToInt16(&ByteArray[0], 4);
    if (Res != 10000)
        return false;

    Res = BitConverterToInt16(&ByteArray[0], 6);
    if (Res != -10000)
        return false;

    Res = BitConverterToInt16(&ByteArray[0], 9);
    if (Res != 32767)
        return false;

    Res = BitConverterToInt16(&ByteArray[0], 2);
    if (Res != -32768)
        return false;

    //all ok, return true
    return true;
}

bool ObjectDatFile::CreateMtlFile(char* outFilename) {
    FILE* oFile;

    oFile = fopen(outFilename, "w");
    if (oFile == NULL)
        return false;

    //create a "default" mtl file that links to our
    //Texture Atlas
    fprintf(oFile, "newmtl material_0\n");
    fprintf(oFile, "Ns 0.000000\n");
    fprintf(oFile, "Ka 0.200000 0.200000 0.200000\n");
    fprintf(oFile, "Kd 1.000000 1.000000 1.000000\n");
    fprintf(oFile, "Ks 1.000000 1.000000 1.000000\n");
    fprintf(oFile, "Ke 0.000000 0.000000 0.000000\n");
    fprintf(oFile, "Ni 1.000000\n");
    fprintf(oFile, "d 1.000000\n");
    fprintf(oFile, "illum 2\n");
    fprintf(oFile, "map_Kd tex0-0.png\n");

    fclose(oFile);
    return true;
}

bool ObjectDatFile::WriteToObjFile(const char* filename, const char* objectname) {
    if (!ConversionSuccesful) {
        printf("\nError - ObjectDatFile: BitConverterToInt16 failed unit test on this computer!\n");
        return false;
    }

    char mtlFilename[60];
    char objFilename[60];

    long dotPos = -1;

    //build final export filenames
    for (ulong i = 0; i < strlen(filename); i++) {
        if (filename[i] == '.') {
            dotPos = i;
        }
    }

    strcpy(objFilename, filename);
    strcpy(mtlFilename, filename);

    //there is no Dot at all?
    if (dotPos == -1) {
        //just add the correct extensions
        strcat(mtlFilename, ".mtl");
        strcat(objFilename, ".obj");
    } else {
        //remove everything after last dot including the dot
        //just do this by writing NULL char to dot position
        char filename_base[128];
        strcpy(filename_base, filename);
        filename_base[dotPos] = 0;

        strcpy(objFilename, filename_base);
        strcpy(mtlFilename, filename_base);

        strcat(mtlFilename, ".mtl");
        strcat(objFilename, ".obj");
    }

    FILE *oFile;

    oFile = fopen(objFilename, "w");
    if (oFile == NULL) {
        return false;
    }

    //when we write the Mtl file name in the first line
    //we additional need to remove all characters until the last
    // "/", to have a local filename
    char mtlNameFirstLine[60];
    strcpy(mtlNameFirstLine, "");

    dotPos = -1;

    //build final export filenames
    for (ulong i = 0; i < strlen(mtlFilename); i++) {
        if (mtlFilename[i] == '/') {
            dotPos = i;
        }
    }

    char addStr[2];

    //we found a slash, we need
    //to remove something
    if (dotPos != -1) {
       for (uint j = (dotPos + 1); j < strlen(mtlFilename); j++) {
           addStr[0] = mtlFilename[j];
           addStr[1] = 0;
           strcat(mtlNameFirstLine, addStr);
       }
    } else {
        //nothing to remove, just copy whole name
        strcpy(mtlNameFirstLine, mtlFilename);
    }

    //first add reference in obj file to additional
    //necessary MTL file
    fprintf(oFile, "mtllib ");
    fprintf(oFile, "%s", mtlNameFirstLine);
    fprintf(oFile, "\n");

    fprintf(oFile, "o %s \n", objectname);

    ObjVertex* pntrVert;
    std::vector<ObjTriangle*>::iterator itTri;
    std::vector<ObjVertex*>::iterator itNormals;
    std::vector<float>::iterator itUVCoord;

    //original line before change to textured version
    /*for (itVert = ObjFileVertexVector->begin(); itVert != ObjFileVertexVector->end(); ++itVert) {
       fprintf(oFile, "v %.6f %.6f %.6f\n", -(*itVert)->X, (*itVert)->Z, (*itVert)->Y);
    }*/

    for (itTri = ObjFileTriangleVector->begin(); itTri != ObjFileTriangleVector->end(); ++itTri) {
           pntrVert = ObjFileVertexVector->at((*itTri)->A);
           //fprintf(oFile, "v %.6f %.6f %.6f\n", -pntrVert->X, pntrVert->Z, pntrVert->Y);
           fprintf(oFile, "v %f %f %f\n", -pntrVert->X, pntrVert->Z, pntrVert->Y);

           pntrVert = ObjFileVertexVector->at((*itTri)->B);
           //fprintf(oFile, "v %.6f %.6f %.6f\n", -pntrVert->X, pntrVert->Z, pntrVert->Y);
           fprintf(oFile, "v %f %f %f\n", -pntrVert->X, pntrVert->Z, pntrVert->Y);

           pntrVert = ObjFileVertexVector->at((*itTri)->C);
           //fprintf(oFile, "v %.6f %.6f %.6f\n", -pntrVert->X, pntrVert->Z, pntrVert->Y);
           fprintf(oFile, "v %f %f %f\n", -pntrVert->X, pntrVert->Z, pntrVert->Y);
        }


    for (itUVCoord = this->uvCoordVec->begin(); itUVCoord != this->uvCoordVec->end(); ++itUVCoord) {
       //fprintf(oFile, "vt %.6f ", (*itUVCoord));
       fprintf(oFile, "vt %f ", (*itUVCoord));
       ++itUVCoord;
       //fprintf(oFile, "%.6f\n", (*itUVCoord));
       fprintf(oFile, "%f\n", -(*itUVCoord));
    }

    //original line before change to textured version
    /*for (itTri = ObjFileTriangleVector->begin(); itTri != ObjFileTriangleVector->end(); ++itTri) {
       fprintf(oFile, "f %d %d %d\n", ((*itTri)->A + 1), ((*itTri)->B + 1), ((*itTri)->C + 1));
    }*/

    //write my vector of calculated normals
    for (itNormals = ObjFileNormalsVector->begin(); itNormals != ObjFileNormalsVector->end(); ++itNormals) {
           //we need to write each normal 3 times, as each triangle has 3 vertices with the same normal
           //fprintf(oFile, "vn %.6f %.6f %.6f\n", -(*itNormals)->X, (*itNormals)->Z, (*itNormals)->Y);
           //fprintf(oFile, "vn %.6f %.6f %.6f\n", -(*itNormals)->X, (*itNormals)->Z, (*itNormals)->Y);
           //fprintf(oFile, "vn %.6f %.6f %.6f\n", -(*itNormals)->X, (*itNormals)->Z, (*itNormals)->Y);

           fprintf(oFile, "vn %f %f %f\n", -(*itNormals)->X, (*itNormals)->Z, (*itNormals)->Y);
           fprintf(oFile, "vn %f %f %f\n", -(*itNormals)->X, (*itNormals)->Z, (*itNormals)->Y);
           fprintf(oFile, "vn %f %f %f\n", -(*itNormals)->X, (*itNormals)->Z, (*itNormals)->Y);
    }

    //the next two lines are important to use the
    //correct material (texture atlas)
    fprintf(oFile, "usemtl material_0\n");
    fprintf(oFile, "s 1\n");

    int idxFace;
    int idx = ObjFileTriangleVector->size() * 3;

    fprintf(oFile, "f ");

    for (idxFace = 1 ; idxFace < (idx + 1); idxFace++) {
           fprintf(oFile, "%d/%d/%d ", idxFace, idxFace, idxFace);
           if (idxFace % 3 == 0) {
              fprintf(oFile, "\n");
              if (idxFace != idx) {
                fprintf(oFile, "f ");
              }
           }

        }

    //close last file
    fclose(oFile);

    //create additional necessary mtl file
    if (!CreateMtlFile(mtlFilename)) {
        return false;
    }

    return true;
}
