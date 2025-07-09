/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "irrmeshbuf.h"
#include "levelterrain.h"
#include "../resources/texture.h"
#include "column.h"

IrrMeshBuf::IrrMeshBuf(TextureLoader* texSource, bool enableLighning) {
    mTexSource = texSource;
    mEnableLightning = enableLighning;

    mAvailableTextureCount = mTexSource->NumLevelTextures;
}

IrrMeshBuf::~IrrMeshBuf() {
}

void IrrMeshBuf::InitializeMeshBufferInfoStructs(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec) {
     targetMeshBufVec.clear();

    //initial fill the MeshBufferInfoStruct vector
    //with empty MeshBufferInfroStructs, one for each possible
    //level texture Id
    for (int i = 0; i < mAvailableTextureCount; i++) {
        MeshBufferInfoStruct* newStruct = new MeshBufferInfoStruct;
        newStruct->textureId = i;

        targetMeshBufVec.push_back(newStruct);
    }
}

//adds an additional Meshbuffer for the specified textureId (material). Returns
//a pointer to the new added MeshBufferInfoStruct. In case something goes wrong
//returns nullptr
MeshBufferInfoStruct* IrrMeshBuf::AddAdditionalMeshBuffer(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId) {
    //plausi-check for forTextureId parameter
    if ((forTextureId < 0) || (forTextureId >= mAvailableTextureCount)) {
       //something wrong, exit!
       return nullptr;
    }

    //find out if there is already a Meshbuffer existing for this material?
    //we do not need to search in the vector, as I make sure that
    //there are always as many MeshBufferInfroStructs in this vector
    //as there are available terrain textures; index is textureId
    MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(forTextureId);

    MeshBufferInfoStruct* addToPntr = nullptr;

    //Meshbuffers for this Material are already existing?
    if (currPntr->meshBuf != nullptr) {
       //yes, it is
       addToPntr = currPntr;

       //loop through MeshbufferInfoStruct until we find the last
       //existing Meshbuffer Element
       //add a new element there
       while (addToPntr->nextPntr != nullptr) {
           addToPntr = addToPntr->nextPntr;
       }
    }

    SMeshBuffer* newBuf = new SMeshBuffer();

    //set texture/material for each SMeshBuffer
    newBuf->getMaterial().setTexture(0, this->mTexSource->levelTex[forTextureId]);
    newBuf->getMaterial().Lighting = mEnableLightning;
    newBuf->getMaterial().Wireframe = false;

    //newBuf->getMaterial().AntiAliasing = EAAM_QUALITY;
    newBuf->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX);

    //if this is the first MeshBuffer for this material
    //simply replace the nullptr in the initial element
    if (currPntr->meshBuf == nullptr) {
        currPntr->meshBuf = newBuf;
        currPntr->remainingIndices = 65535;

        return currPntr;
    } else {
        //for each additional Meshbuffer we need to add a new MeshBufferInfoStruct
        //to encapsulate the new Meshbuffer
        MeshBufferInfoStruct* newStruct = new MeshBufferInfoStruct();
        newStruct->textureId = forTextureId;
        newStruct->meshBuf = newBuf;
        newStruct->remainingIndices = 65535;

        //add pointer to new info struct
        //into the last existing info struct for
        //this texture Id (single linked list)
        addToPntr->nextPntr = newStruct;

        return newStruct;
    }
}

//finds the current (last in linked list) MeshBuffer info struct for a certain textureId
//returns nullptr in case of error
MeshBufferInfoStruct* IrrMeshBuf::FindLastMeshBufferInLinkedList(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId) {
    //vector index is textureId
    MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(forTextureId);

    if (currPntr == nullptr)
        return nullptr;

    //loop through MeshbufferInfoStruct until we find the last
    //existing Meshbuffer Element
    while (currPntr->nextPntr != nullptr) {
         currPntr = currPntr->nextPntr;
    }

    return currPntr;
}

//finds the first available MeshBuffer info struct for a certain textureId
//which has still space for 6 additional indices (an additional Quad)
//returns nullptr in case of error, or nothing available
MeshBufferInfoStruct* IrrMeshBuf::FindFirstMeshBufferForAdditionalQuad(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t forTextureId) {
    //vector index is textureId
    MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(forTextureId);

    if (currPntr == nullptr)
        return nullptr;

    if (currPntr->remainingIndices >= 6)
        return currPntr;

    //loop through MeshbufferInfoStruct until we find the next
    //Meshbuffer element with at least 6 free indices
    while (currPntr->nextPntr != nullptr) {
         currPntr = currPntr->nextPntr;

         if (currPntr->remainingIndices >= 6)
             return currPntr;
    }

    //no available meshbuffer found with at
    //least 6 free indices
    return nullptr;
}

//counts the number of existing Meshbuffers for all possible
//Texture Ids
std::vector<irr::u8> IrrMeshBuf::ReturnMeshBufferCntPerTextureId(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec) {
    std::vector<irr::u8> result;

    result.clear();
    irr::u8 cnt;

    for (int i = 0; i < mAvailableTextureCount; i++) {
         //vector index is textureId
         MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(i);
         cnt = 0;

        if (currPntr == nullptr) {
            result.push_back(cnt);
            continue;
        }

        if (currPntr->meshBuf != nullptr) {
           cnt++;
        }

        //loop through MeshbufferInfoStruct until we find the last
        //existing Meshbuffer Element
        while (currPntr->nextPntr != nullptr) {
             currPntr = currPntr->nextPntr;

             if (currPntr->meshBuf != nullptr) {
                cnt++;
             }
        }

        result.push_back(cnt);
    }

    return result;
}

std::vector<irr::scene::SMeshBuffer*> IrrMeshBuf::ReturnAllMeshBuffersForTextureId(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, int16_t textureId) {
    std::vector<irr::scene::SMeshBuffer*> result;
    result.clear();

    //vector index is textureId
    MeshBufferInfoStruct* currPntr = targetMeshBufVec.at(textureId);

    if (currPntr == nullptr)
        return result;

    if (currPntr->meshBuf != nullptr) {
        result.push_back(currPntr->meshBuf);
    }

    while (currPntr->nextPntr != nullptr) {
        if (currPntr->nextPntr->meshBuf != nullptr) {
            result.push_back(currPntr->nextPntr->meshBuf);
        }

        currPntr = currPntr->nextPntr;
    }

    //return list of found MeshBuffers
    return result;
}

void IrrMeshBuf::CleanupMeshBufferInfoStructs(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec) {
    std::vector<MeshBufferInfoStruct*> infoStructVec;
    std::vector<MeshBufferInfoStruct*>::iterator it;

    MeshBufferInfoStruct* pntrStruct;

    for (int i = 0; i < mAvailableTextureCount; i++) {
        pntrStruct = targetMeshBufVec.at(i);
        infoStructVec.clear();

        if (pntrStruct == nullptr)
            continue;

        infoStructVec.push_back(pntrStruct);

        while (pntrStruct->nextPntr != nullptr) {
            pntrStruct = pntrStruct->nextPntr;
            infoStructVec.push_back(pntrStruct);
        }

        for (it = infoStructVec.begin(); it != infoStructVec.end(); ) {
            pntrStruct = (*it);

            it = infoStructVec.erase(it);

            //delete MeshBufferInfoStruct
            delete pntrStruct;
        }
    }
}

void IrrMeshBuf::AddMeshBufferTile(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, TerrainTileData* tilePntr, int16_t textureId, MeshObjectStatsStruct &statpntr) {
    //what is the current Meshbuffer for the textureId
    //of the new tile
    MeshBufferInfoStruct* nextBufInfo = FindFirstMeshBufferForAdditionalQuad(targetMeshBufVec, textureId);

    //if routine returns nullptr something is wrong,
    //or no free meshbuffer currently available to add
    //new tile (quad)
    if (nextBufInfo == nullptr) {
        //no, create an additional MeshBuffer for this texture Id
        nextBufInfo = AddAdditionalMeshBuffer(targetMeshBufVec, textureId);

        //something wrong?
        if (nextBufInfo == nullptr)
            return;
    }

    //nextBufInfo contains now the Meshbuffer where we
    //want to add the additional tile (4 vertices, 6 indices)
    nextBufInfo->meshBuf->grab();

    irr::u16 firstIndexNewQuad = nextBufInfo->meshBuf->getVertexCount();

    //add the 4 existing vertices for this tile into the
    //meshbuffer vertice array
    nextBufInfo->meshBuf->Vertices.push_back(*tilePntr->vert1);
    nextBufInfo->meshBuf->Vertices.push_back(*tilePntr->vert2);
    nextBufInfo->meshBuf->Vertices.push_back(*tilePntr->vert3);
    nextBufInfo->meshBuf->Vertices.push_back(*tilePntr->vert4);

    //at the same time store in tile which index the vertices
    //have in the meshbuffer vertices array; we need this information later
    //for morphing
    tilePntr->myMeshBufVertexId1.push_back(firstIndexNewQuad);

    // add indices for the 2 new tris of the new
    //quad (this are 6 new indices)
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 1);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 3);

    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 1);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 2);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 3);

    //decrease remaining number of available indices in this Meshbuffer
    //for this textureId
    nextBufInfo->remainingIndices -= 6;

    nextBufInfo->meshBuf->drop();

    //also keep info which Meshbuffers we have used for this
    //tile; is also necessary for morphing later
    //current used Meshbuffer already stored for this tile?
    std::vector<irr::scene::SMeshBuffer*>::iterator it;
    bool found = false;

    for (it = tilePntr->myMeshBuffers.begin(); it != tilePntr->myMeshBuffers.end(); ++it) {
        if ((*it) == nextBufInfo->meshBuf) {
            found = true;
            break;
        }
    }

    if (!found) {
        //not yet part of vector, add also this (new) MeshBuffer
        tilePntr->myMeshBuffers.push_back(nextBufInfo->meshBuf);
    }

    //increase statistical values
    statpntr.numUVs += 4;
    statpntr.numVertices += 4;
    statpntr.numNormals += 4;
    statpntr.numIndices += 6;
}

//returns nullptr in case appropriate MeshBufferInfoStruct is not found
MeshBufferInfoStruct* IrrMeshBuf::FindMeshBufferInfoStructForMeshBuffer(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, irr::scene::SMeshBuffer* meshBufToFind) {
   std::vector<MeshBufferInfoStruct*>::iterator it;

   for (it = targetMeshBufVec.begin(); it != targetMeshBufVec.end(); ++it) {
       if ((*it)->meshBuf == meshBufToFind) {
           //found, return pointer
           return (*it);
       }
   }

   //not found
   return nullptr;
}

void IrrMeshBuf::RemoveMeshBufferTile(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, TerrainTileData* tilePntr, MeshObjectStatsStruct &statpntr) {
    //if this tile is optimized away, exit here
    if (!tilePntr->m_draw_in_mesh)
        return;

    std::vector<irr::scene::SMeshBuffer*>::iterator it;
    irr::scene::SMeshBuffer* meshBufPntr;

    irr::u32 vert1Idx;
    irr::u32 vert2Idx;
    irr::u32 vert3Idx;
    irr::u32 vert4Idx;

    irr::u32 nrIndices;
    bool dirty;

    std::vector<irr::u32>::iterator itVertId1 = tilePntr->myMeshBufVertexId1.begin();

    MeshBufferInfoStruct* pInfoStruct;

    /******************************************************************
     * Part 1: Remove existing tile from Meshbuffers and Mesh         *
     ******************************************************************/

    //iterate through all meshbuffers where this tile is included, and
    //see if we find any of the indices of the vertices of this tile included
    for (it = tilePntr->myMeshBuffers.begin(); it != tilePntr->myMeshBuffers.end(); ) {
        //what indices do my vertices have in this meshbuffer?
        vert1Idx = (*itVertId1);
        vert2Idx = (*itVertId1) + 1;
        vert3Idx = (*itVertId1) + 2;
        vert4Idx = (*itVertId1) + 3;

        meshBufPntr = (*it);

        //we need to adjust the number of free indices in the meshbuffer
        //in case we delete indices later
        //Therefore we need to find the correct MeshBufferInfoStruct for
        //this Meshbuffer
        pInfoStruct = FindMeshBufferInfoStructForMeshBuffer(targetMeshBufVec, meshBufPntr);

        meshBufPntr->grab();

        nrIndices = meshBufPntr->getIndexCount();
        dirty = false;

        //loop from backwards to forwards, so that deleting elements
        //does not throw of looping index
        for (irr::s32 idxCnt = (irr::s32)(nrIndices - 1); idxCnt >= 0; idxCnt--) {
            //if we find any indices of the tile to remove, remove it from the indices
            //array
            if ((meshBufPntr->Indices[idxCnt] == vert1Idx) ||
               (meshBufPntr->Indices[idxCnt] == vert2Idx) ||
               (meshBufPntr->Indices[idxCnt] == vert3Idx) ||
               (meshBufPntr->Indices[idxCnt] == vert4Idx)) {
                    meshBufPntr->Indices.erase(idxCnt);

                    dirty = true;

                    //nrIndices--;
                    statpntr.numIndices--;

                    if (pInfoStruct != nullptr) {
                        //There is one more available index
                        //again in indices array of Meshbuffer
                        pInfoStruct->remainingIndices++;
                    }
              }
        }

        meshBufPntr->drop();

        if (dirty) {
            meshBufPntr->setDirty(EBT_VERTEX_AND_INDEX);

            //we also remove this meshBuffer index from the myMeshBuffers
            //vector
            it = tilePntr->myMeshBuffers.erase(it);

            itVertId1 = tilePntr->myMeshBufVertexId1.erase(itVertId1);
        } else {
            //advance to the next myMeshBuffers position
            it++;

            itVertId1++;
        }
    }
}

void IrrMeshBuf::ResetMeshStats(MeshObjectStatsStruct* statPntr) {
    statPntr->numVertices = 0;
    statPntr->numIndices = 0;
    statPntr->numUVs = 0;
    statPntr->numNormals = 0;
}

int IrrMeshBuf::GetNrTextures() {
    return mAvailableTextureCount;
}

 void IrrMeshBuf::AddMeshBufferCubeFace(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, BlockFaceInfoStruct* blockFaceInfo, MeshObjectStatsStruct &statpntr) {
    //what is the current Meshbuffer for the textureId
    //of this new cube face
    MeshBufferInfoStruct* nextBufInfo = FindFirstMeshBufferForAdditionalQuad(targetMeshBufVec, blockFaceInfo->textureId);

    //if routine returns nullptr something is wrong,
    //or no free meshbuffer currently available to add
    //new tile (quad)
    if (nextBufInfo == nullptr) {
        //no, create an additional MeshBuffer for this texture Id
        nextBufInfo = AddAdditionalMeshBuffer(targetMeshBufVec, blockFaceInfo->textureId);

        //something wrong?
        if (nextBufInfo == nullptr)
            return;
    }

    //nextBufInfo contains now the Meshbuffer where we
    //want to add the additional cube face (4 vertices, 6 indices)
    nextBufInfo->meshBuf->grab();

    irr::u16 firstIndexNewQuad = nextBufInfo->meshBuf->getVertexCount();

    //add the 4 existing vertices for this cube face into the
    //meshbuffer vertice array
    nextBufInfo->meshBuf->Vertices.push_back(*blockFaceInfo->vert1);
    nextBufInfo->meshBuf->Vertices.push_back(*blockFaceInfo->vert2);
    nextBufInfo->meshBuf->Vertices.push_back(*blockFaceInfo->vert3);
    nextBufInfo->meshBuf->Vertices.push_back(*blockFaceInfo->vert4);

    //at the same time store in cube face which index the vertices
    //have in the meshbuffer vertices array; we need this information later
    //for morphing
    blockFaceInfo->myMeshBufVertexId.push_back(firstIndexNewQuad);

    // add indices for the 2 new tris of the new
    //quad (this are 6 new indices)
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 1);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 3);

    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 1);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 2);
    nextBufInfo->meshBuf->Indices.push_back(firstIndexNewQuad + 3);

    //decrease remaining number of available indices in this Meshbuffer
    //for this textureId
    nextBufInfo->remainingIndices -= 6;

    nextBufInfo->meshBuf->drop();

    //also keep info which Meshbuffers we have used for this
    //cube face; is also necessary for morphing later
    //current used Meshbuffer already stored for this cube face?
    std::vector<irr::scene::SMeshBuffer*>::iterator it;
    bool found = false;

    for (it = blockFaceInfo->myMeshBuffers.begin(); it != blockFaceInfo->myMeshBuffers.end(); ++it) {
        if ((*it) == nextBufInfo->meshBuf) {
            found = true;
            break;
        }
    }

    if (!found) {
        //not yet part of vector, add also this (new) MeshBuffer
        blockFaceInfo->myMeshBuffers.push_back(nextBufInfo->meshBuf);
    }

    //increase statistical values
    statpntr.numUVs += 4;
    statpntr.numVertices += 4;
    statpntr.numNormals += 4;
    statpntr.numIndices += 6;
}

void IrrMeshBuf::AddMeshBufferBlock(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, BlockInfoStruct* blockInfo, MeshObjectStatsStruct &statpntr) {
     //to add a whole new cube, add 6 faces of this cube
     AddMeshBufferCubeFace(targetMeshBufVec, blockInfo->fN, statpntr);
     AddMeshBufferCubeFace(targetMeshBufVec, blockInfo->fE, statpntr);
     AddMeshBufferCubeFace(targetMeshBufVec, blockInfo->fS, statpntr);
     AddMeshBufferCubeFace(targetMeshBufVec, blockInfo->fW, statpntr);
     AddMeshBufferCubeFace(targetMeshBufVec, blockInfo->fT, statpntr);
     AddMeshBufferCubeFace(targetMeshBufVec, blockInfo->fB, statpntr);
}

void IrrMeshBuf::RemoveMeshBufferCubeFace(std::vector<MeshBufferInfoStruct*> &targetMeshBufVec, BlockFaceInfoStruct* blockFaceInfo, MeshObjectStatsStruct &statpntr) {
     if (blockFaceInfo == nullptr)
         return;

     std::vector<irr::scene::SMeshBuffer*>::iterator it;
     irr::scene::SMeshBuffer* meshBufPntr;

     irr::u32 vert1Idx;
     irr::u32 vert2Idx;
     irr::u32 vert3Idx;
     irr::u32 vert4Idx;

     irr::u32 nrIndices;
     bool dirty;

     std::vector<irr::u32>::iterator itVertId1 = blockFaceInfo->myMeshBufVertexId.begin();

     MeshBufferInfoStruct* pInfoStruct;

     /******************************************************************
      * Part 1: Remove existing block face from Meshbuffers and Mesh   *
      ******************************************************************/

     //iterate through all meshbuffers where this cube face is included, and
     //see if we find any of the indices of the vertices of this cube face included
     for (it = blockFaceInfo->myMeshBuffers.begin(); it != blockFaceInfo->myMeshBuffers.end(); ) {
         //what indices do my vertices have in this meshbuffer?
         vert1Idx = (*itVertId1);
         vert2Idx = (*itVertId1) + 1;
         vert3Idx = (*itVertId1) + 2;
         vert4Idx = (*itVertId1) + 3;

         meshBufPntr = (*it);

         //we need to adjust the number of free indices in the meshbuffer
         //in case we delete indices later
         //Therefore we need to find the correct MeshBufferInfoStruct for
         //this Meshbuffer
         pInfoStruct = FindMeshBufferInfoStructForMeshBuffer(targetMeshBufVec, meshBufPntr);

         meshBufPntr->grab();

         nrIndices = meshBufPntr->getIndexCount();
         dirty = false;

         //loop from backwards to forwards, so that deleting elements
         //does not throw of looping index
         for (irr::s32 idxCnt = (irr::s32)(nrIndices - 1); idxCnt >= 0; idxCnt--) {
             //if we find any indices of the cube face to remove, remove it from the indices
             //array
             if ((meshBufPntr->Indices[idxCnt] == vert1Idx) ||
                (meshBufPntr->Indices[idxCnt] == vert2Idx) ||
                (meshBufPntr->Indices[idxCnt] == vert3Idx) ||
                (meshBufPntr->Indices[idxCnt] == vert4Idx)) {
                     meshBufPntr->Indices.erase(idxCnt);

                     dirty = true;

                     //nrIndices--;
                     statpntr.numIndices--;

                     if (pInfoStruct != nullptr) {
                         //There is one more available index
                         //again in indices array of Meshbuffer
                         pInfoStruct->remainingIndices++;
                     }
               }
         }

         meshBufPntr->drop();

         if (dirty) {
             meshBufPntr->setDirty(EBT_VERTEX_AND_INDEX);

             //we also remove this meshBuffer index from the myMeshBuffers
             //vector
             it = blockFaceInfo->myMeshBuffers.erase(it);

             itVertId1 = blockFaceInfo->myMeshBufVertexId.erase(itVertId1);
         } else {
             //advance to the next myMeshBuffers position
             it++;

             itVertId1++;
         }
     }
 }

