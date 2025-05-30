/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "chargingstation.h"

ChargingStation::ChargingStation(irr::scene::ISceneManager* smgr, Race* race, MapTileRegionStruct* regionStruct,
                    std::vector<WayPointLinkInfoStruct*> *allRaceWayPoints) {
    mSmgr = smgr;
    mRace = race;
    mRegionStruct = regionStruct;

    //create the mesh and SceneNode
    createRegionMeshData();

    mStandVec.clear();

    mIntersectingWayPointLinksVec.clear();

    //figure out which waypoint links intersect me
    std::vector<WayPointLinkInfoStruct*>::iterator it;

    //get bounding box of region SceneNode that was prepared before
    irr::core::aabbox3d bbox = mSceneNode->getTransformedBoundingBox();

    irr::core::line3df line3D;

    for (it = allRaceWayPoints->begin(); it != allRaceWayPoints->end(); ++it) {
        //create a 3D line along the specified waypoint link
        line3D.start = (*it)->pLineStruct->A;
        line3D.end = (*it)->pLineStruct->B;

        //does this waypoint link intersect the charging region bounding box?
        if (bbox.intersectsWithLine(line3D)) {
           //yes this waypoint link intersects us
           //keep it in my vector
           this->mIntersectingWayPointLinksVec.push_back(*it);
        }
    }

    //we need at least one intersecting waypoint link, so that
    //we know the directon of the craft traveling through
    if (mIntersectingWayPointLinksVec.size() > 0) {
        createChargingStands();
    }

    //create a temporary entityItem for entering
    //the charging station for computer players
    this->enterEntityItem = new EntityItem(this->enterEntityItemLocation);

    //additional helper entityItem for path to stall creation
    this->helperEntityItem = new EntityItem(this->helperEntityItemLocation);
}

irr::u8 ChargingStation::GetChargingStationType() {
    return this->mRegionStruct->regionType;
}

//creates a Mesh/Box around defined charger map region
//could be used later for different things, to control computer player craft for example
void ChargingStation::createRegionMeshData() {
    int vind = 0;
    int iind = 0;
    irr::f32 plane_h = 10.0f; //plane height

    video::SColor cubeColour4(255,255,255,255);

    irr::f32 segSize = mRace->mLevelTerrain->segmentSize;

    irr::f32 midPointHeight =
            -mRace->mLevelTerrain->GetHeightInterpolated(mRegionStruct->regionCenterTileCoord.X, mRegionStruct->regionCenterTileCoord.Y);

    //our X Irrlicht coordinate system is swapped in sign
    irr::core::vector3df corner1(-(mRegionStruct->tileXmin * segSize),  midPointHeight, mRegionStruct->tileYmin * segSize);
    irr::core::vector3df corner2(-(mRegionStruct->tileXmin * segSize),  midPointHeight, mRegionStruct->tileYmax * segSize);
    irr::core::vector3df corner3(-(mRegionStruct->tileXmax * segSize),  midPointHeight, mRegionStruct->tileYmax * segSize);
    irr::core::vector3df corner4(-(mRegionStruct->tileXmax * segSize),  midPointHeight, mRegionStruct->tileYmin * segSize);

    //create a region mesh scenenode for the specified map region
    //create a new mesh
    mMesh = new SMesh();

    //create a new MeshBuffer
    SMeshBuffer *Mbuf = new SMeshBuffer();
    Mbuf->Indices.set_used(6*2);
    Mbuf->Vertices.set_used(4*2);
    Mbuf->Material.Wireframe = true;

    Mbuf->Vertices[vind] = video::S3DVertex(corner2.X, corner2.Y - plane_h, corner2.Z, 0.0f, 0.0f, 1.0f, cubeColour4, 0.0f, 1.0f);
    Mbuf->Vertices[vind+1] = video::S3DVertex(corner3.X, corner3.Y - plane_h, corner3.Z, 0.0f, 0.0f, 1.0f, cubeColour4, 1.0f, 1.0f);
    Mbuf->Vertices[vind+2] = video::S3DVertex(corner3.X, corner3.Y + plane_h, corner3.Z, 0.0f, 0.0f, 1.0f, cubeColour4, 1.0f, 0.0f);
    Mbuf->Vertices[vind+3] = video::S3DVertex(corner2.X, corner2.Y + plane_h, corner2.Z, 0.0f, 0.0f, 1.0f, cubeColour4, 0.0f, 0.0f);

    Mbuf->Vertices[vind+4] = video::S3DVertex(corner1.X, corner1.Y - plane_h, corner1.Z, 0.0f, 0.0f, -1.0f, cubeColour4, 0.0f, 1.0f);
    Mbuf->Vertices[vind+5] = video::S3DVertex(corner4.X, corner4.Y - plane_h, corner4.Z, 0.0f, 0.0f, -1.0f, cubeColour4, 1.0f, 1.0f);
    Mbuf->Vertices[vind+6] = video::S3DVertex(corner4.X, corner4.Y + plane_h, corner4.Z, 0.0f, 0.0f, -1.0f, cubeColour4, 1.0f, 0.0f);
    Mbuf->Vertices[vind+7] = video::S3DVertex(corner1.X, corner1.Y + plane_h, corner1.Z, 0.0f, 0.0f, -1.0f, cubeColour4, 0.0f, 0.0f);

    //first triangle to form quad
    Mbuf->Indices[iind] = vind;
    Mbuf->Indices[iind+1] = vind+1;
    Mbuf->Indices[iind+2] = vind+3;

    //2nd triangle to form quad
    Mbuf->Indices[iind+3] = vind+3;
    Mbuf->Indices[iind+4] = vind+1;
    Mbuf->Indices[iind+5] = vind+2;

    //3rd triangle to form quad
    Mbuf->Indices[iind+6] = vind+4;
    Mbuf->Indices[iind+7] = vind+5;
    Mbuf->Indices[iind+8] = vind+7;

    //4nd triangle to form quad
    Mbuf->Indices[iind+9] = vind+7;
    Mbuf->Indices[iind+10] = vind+5;
    Mbuf->Indices[iind+11] = vind+6;

    Mbuf->recalculateBoundingBox();

    mMesh->addMeshBuffer(Mbuf);

    Mbuf->drop();

    mMesh->recalculateBoundingBox();

    //now create a MeshSceneNode
    mSceneNode = mSmgr->addMeshSceneNode(mMesh, 0);

    //hide the collision mesh that the player does not see it
    mSceneNode->setVisible(false);
    mSceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, true);
    //mSceneNode->setDebugDataVisible(EDS_BBOX);
    mSceneNode->setDebugDataVisible(EDS_OFF);
}

//returns true if usable area was succesfully found
//returns false in case of error
bool ChargingStation::IdentifyUsableArea() {
    //Some predefined charging stations in the maps extend outside
    //of the usable race track, this would lead to stalls that are not
    //reachable by craft, we need to figure out which area is usable first,
    //and remove the other areas
    usableTileXmin = mRegionStruct->tileXmin;
    usableTileXmax = mRegionStruct->tileXmax;
    usableTileYmin = mRegionStruct->tileYmin;
    usableTileYmax = mRegionStruct->tileYmax;

    irr::u16 currYcoord;
    irr::u16 currXcoord;
    irr::u16 startLineY = (usableTileYmax - usableTileYmin) / 2 + usableTileYmin;
    irr::u16 startLineX = (usableTileXmax - usableTileXmin) / 2 + usableTileXmin;
    irr::u16 endLineY = usableTileYmax;
    irr::u16 endLineX = usableTileXmax;
    bool startLineYFnd = false;
    bool startLineXFnd = false;
    int16_t texture;
    irr::u16 tileCntCharging = 0;
    irr::u16 tileCnt = 0;
    irr::f32 percentCharging;
    //bool noColumnThere;
    MapEntry* mapEntry;

    if (this->mWidthInZDir) {
        /*************************************************************
         * First identify available "width" of charging station      *
         *************************************************************/

        //first we need to find a "line" of Ycoord tiles that are at least >75% charging tiles
        for (currYcoord = mRegionStruct->tileYmin; currYcoord <= mRegionStruct->tileYmax; currYcoord++) {
            tileCnt = 0;
            tileCntCharging = 0;

            for (currXcoord = mRegionStruct->tileXmin; currXcoord <= mRegionStruct->tileXmax; currXcoord++) {
                tileCnt++;
                mapEntry = this->mRace->mLevelTerrain->GetMapEntry(currXcoord, currYcoord);
                texture = mapEntry->m_TextureId;

                //we also want to exclude cells where there is a column
                //noColumnThere = (mapEntry->get_Column() == NULL);

                if (this->mRace->mLevelTerrain->IsChargingStationTexture(texture)) {
                    tileCntCharging++;
                }
            }

            percentCharging = ((irr::f32)(tileCntCharging) / (irr::f32)(tileCnt));

            if (percentCharging > 0.75) {
                //we are happy
                startLineY = currYcoord;
                startLineYFnd = true;
                break;
            }
        }

        //we did not find start, exit with failure
        if (!startLineYFnd)
            return false;

        //continue to search for end
        //means last line where we have > 75% charging tiles
        for (currYcoord = startLineY; currYcoord <= mRegionStruct->tileYmax; currYcoord++) {
            tileCnt = 0;
            tileCntCharging = 0;

            for (currXcoord = mRegionStruct->tileXmin; currXcoord <= mRegionStruct->tileXmax; currXcoord++) {
                tileCnt++;
                mapEntry = this->mRace->mLevelTerrain->GetMapEntry(currXcoord, currYcoord);
                texture = mapEntry->m_TextureId;

                //we also want to exclude cells where there is a column
                //noColumnThere = (mapEntry->get_Column() == NULL);

                if (this->mRace->mLevelTerrain->IsChargingStationTexture(texture)) {
                    tileCntCharging++;
                }
            }

            percentCharging = ((irr::f32)(tileCntCharging) / (irr::f32)(tileCnt));

            if (percentCharging < 0.75) {
                //we need to stop
                break;
            } else {
                //charging region not yet finished, extend
                endLineY = currYcoord;
            }
        }

        usableTileYmin = startLineY;
        usableTileYmax = endLineY;

        /*************************************************************
         * Identify available "length" of charging station           *
         *************************************************************/

        //first we need to find a "line" of Xcoord tiles that are at least >75% charging tiles
        for (currXcoord = mRegionStruct->tileXmin; currXcoord <= mRegionStruct->tileXmax; currXcoord++) {
            tileCnt = 0;
            tileCntCharging = 0;

            for (currYcoord = usableTileYmin; currYcoord <= usableTileYmax; currYcoord++) {
                tileCnt++;
                mapEntry = this->mRace->mLevelTerrain->GetMapEntry(currXcoord, currYcoord);
                texture = mapEntry->m_TextureId;

                //we also want to exclude cells where there is a column
                //noColumnThere = (mapEntry->get_Column() == NULL);

                if (this->mRace->mLevelTerrain->IsChargingStationTexture(texture)) {
                    tileCntCharging++;
                }
            }

            percentCharging = ((irr::f32)(tileCntCharging) / (irr::f32)(tileCnt));

            if (percentCharging > 0.75) {
                //we are happy
                startLineX = currXcoord;
                startLineXFnd = true;
                break;
            }
        }

        //we did not find start, exit with failure
        if (!startLineXFnd)
            return false;

        //continue to search for end
        //means last line where we have > 75% charging tiles
        for (currXcoord = startLineX; currXcoord <= mRegionStruct->tileXmax; currXcoord++) {
            tileCnt = 0;
            tileCntCharging = 0;

            for (currYcoord = usableTileYmin; currYcoord <= usableTileYmax; currYcoord++) {
                tileCnt++;
                mapEntry = this->mRace->mLevelTerrain->GetMapEntry(currXcoord, currYcoord);
                texture = mapEntry->m_TextureId;

                //we also want to exclude cells where there is a column
                //noColumnThere = (mapEntry->get_Column() == NULL);

                if (this->mRace->mLevelTerrain->IsChargingStationTexture(texture)) {
                    tileCntCharging++;
                }
            }

            percentCharging = ((irr::f32)(tileCntCharging) / (irr::f32)(tileCnt));

            if (percentCharging < 0.75) {
                //we need to stop
                break;
            } else {
                //charging region not yet finished, extend
                endLineX = currXcoord;
            }
        }

        usableTileXmin = startLineX;
        usableTileXmax = endLineX;

        return true;
    }

    //Here we have the case !this->mWidthInZDir below

    /*************************************************************
    * First identify available "width" of charging station      *
    *************************************************************/

    //first we need to find a "line" of Xcoord tiles that are at least >75% charging tiles
    for (currXcoord = mRegionStruct->tileXmin; currXcoord <= mRegionStruct->tileXmax; currXcoord++) {
       tileCnt = 0;
       tileCntCharging = 0;

       for (currYcoord = mRegionStruct->tileYmin; currYcoord <= mRegionStruct->tileYmax; currYcoord++) {
           tileCnt++;
           mapEntry = this->mRace->mLevelTerrain->GetMapEntry(currXcoord, currYcoord);
           texture = mapEntry->m_TextureId;

           //we also want to exclude cells where there is a column
           //noColumnThere = (mapEntry->get_Column() == NULL);

           if (this->mRace->mLevelTerrain->IsChargingStationTexture(texture)) {
               tileCntCharging++;
           }
       }

       percentCharging = ((irr::f32)(tileCntCharging) / (irr::f32)(tileCnt));

       if (percentCharging > 0.75) {
           //we are happy
           startLineX = currXcoord;
           startLineXFnd = true;
           break;
       }
    }

    //we did not find start, exit with failure
    if (!startLineXFnd)
        return false;

    //continue to search for end
    //means last line where we have > 75% charging tiles
    for (currXcoord = startLineX; currXcoord <= mRegionStruct->tileXmax; currXcoord++) {
        tileCnt = 0;
        tileCntCharging = 0;

        for (currYcoord = mRegionStruct->tileYmin; currYcoord <= mRegionStruct->tileYmax; currYcoord++) {
            tileCnt++;
            mapEntry = this->mRace->mLevelTerrain->GetMapEntry(currXcoord, currYcoord);
            texture = mapEntry->m_TextureId;

            //we also want to exclude cells where there is a column
            //noColumnThere = (mapEntry->get_Column() == NULL);

            if (this->mRace->mLevelTerrain->IsChargingStationTexture(texture)) {
                tileCntCharging++;
            }
        }

        percentCharging = ((irr::f32)(tileCntCharging) / (irr::f32)(tileCnt));

        if (percentCharging < 0.75) {
             //we need to stop
             break;
        } else {
                 //charging region not yet finished, extend
                 endLineX = currXcoord;
               }
    }

    usableTileXmin = startLineX;
    usableTileXmax = endLineX;

    /*************************************************************
     * Identify available "length" of charging station           *
     *************************************************************/

    //first we need to find a "line" of Ycoord tiles that are at least >75% charging tiles
    for (currYcoord = mRegionStruct->tileYmin; currYcoord <= mRegionStruct->tileYmax; currYcoord++) {
        tileCnt = 0;
        tileCntCharging = 0;

        for (currXcoord = usableTileXmin; currXcoord <= usableTileXmax; currXcoord++) {
            tileCnt++;
            mapEntry = this->mRace->mLevelTerrain->GetMapEntry(currXcoord, currYcoord);
            texture = mapEntry->m_TextureId;

            //we also want to exclude cells where there is a column
            //noColumnThere = (mapEntry->get_Column() == NULL);

            if (this->mRace->mLevelTerrain->IsChargingStationTexture(texture)) {
                tileCntCharging++;
            }
        }

        percentCharging = ((irr::f32)(tileCntCharging) / (irr::f32)(tileCnt));

        if (percentCharging > 0.75) {
            //we are happy
            startLineY = currYcoord;
            startLineYFnd = true;
            break;
        }
    }

    //we did not find start, exit with failure
    if (!startLineYFnd)
        return false;

    //continue to search for end
    //means last line where we have > 75% charging tiles
    for (currYcoord = startLineY; currYcoord <= mRegionStruct->tileYmax; currYcoord++) {
        tileCnt = 0;
        tileCntCharging = 0;

        for (currXcoord = usableTileXmin; currXcoord <= usableTileXmax; currXcoord++) {
            tileCnt++;
            mapEntry = this->mRace->mLevelTerrain->GetMapEntry(currXcoord, currYcoord);
            texture = mapEntry->m_TextureId;

            //we also want to exclude cells where there is a column
            //noColumnThere = (mapEntry->get_Column() == NULL);

            if (this->mRace->mLevelTerrain->IsChargingStationTexture(texture)) {
                tileCntCharging++;
            }
        }

        percentCharging = ((irr::f32)(tileCntCharging) / (irr::f32)(tileCnt));

        if (percentCharging < 0.75) {
            //we need to stop
            break;
        } else {
            //charging region not yet finished, extend
            endLineY = currYcoord;
        }
    }

    usableTileYmin = startLineY;
    usableTileYmax = endLineY;

    return true;
}

//create my available stands that allows
//player crafts to "stand" during charging
void ChargingStation::createChargingStands() {
    //in which coordinate direction will the craft
    //fly through us, we can derive this information
    //from a waypoint link that goes through us
    irr::f32 dotProductinX = this->mIntersectingWayPointLinksVec.at(0)->LinkDirectionVec.dotProduct(*mRace->xAxisDirVector);
    irr::f32 dotProductinZ = this->mIntersectingWayPointLinksVec.at(0)->LinkDirectionVec.dotProduct(*mRace->zAxisDirVector);

    irr::f32 signInX = -1.0f;

    if (dotProductinX >= 0.0f) {
        signInX = 1.0f;
    }

    irr::f32 signInZ = -1.0f;

    if (dotProductinZ >= 0.0f) {
        signInZ = 1.0f;
    }

    irr::f32 absDotProductinX = fabs(dotProductinX);
    irr::f32 absDotProductinZ = fabs(dotProductinZ);

    //define the width of the charging station as the width
    //perpendicular to the direction the craft will fly through
    //the charging station
    //define the length of the charging station as the length
    //in direction of the Waypoint link
    irr::f32 width;
    irr::f32 length;
    mWidthInZDir = false;

    irr::f32 segSize = mRace->mLevelTerrain->segmentSize;

    if (absDotProductinX > absDotProductinZ) {
        //it seems craft go through us in X direction
        //therefore width is in Z direction
        mWidthInZDir = true;
    }

    //identify "really" usable area for charging
    //TBD: read return boolean value, and react in case of
    //fail
    IdentifyUsableArea();

    if (absDotProductinX > absDotProductinZ) {
        //it seems craft go through us in X direction
        //therefore width is in Z direction
        //width = (mRegionStruct->tileYmax - mRegionStruct->tileYmin) * segSize;
        //length = (mRegionStruct->tileXmax - mRegionStruct->tileXmin) * segSize;
        width = (usableTileYmax - usableTileYmin) * segSize;
        length = (usableTileXmax - usableTileXmin) * segSize;
    } else {
        //it seems craft go through us in Z direction
        //width = (mRegionStruct->tileXmax - mRegionStruct->tileXmin) * segSize;
        //length = (mRegionStruct->tileYmax - mRegionStruct->tileYmin) * segSize;
        width = (usableTileXmax - usableTileXmin) * segSize;
        length = (usableTileYmax - usableTileYmin) * segSize;
    }

    //here we need to round down to nearest integer (or lets simply truncate!)
    nrStallsWidth = (irr::u8)(width / DEF_CHARGING_STATION_STAND_MINSIDELEN);
    nrStallsLength = (irr::u8)(length / DEF_CHARGING_STATION_STAND_MINSIDELEN);

    nrStalls = nrStallsWidth * nrStallsLength;

    //25.05.2025: I have again a level design issue with some existing charging stations
    //in the map, where the defined region goes under blocks not accessible by crafts
    //The initial source code which just uses all space of the region would put a stall there,
    //which no craft can reach. To fix this I decided to limit the max number of stalls to create
    //to 8 (we never have more players anyway), and start locating them centered in the region;
    //this should prevent unreachable stalls
    if (nrStalls > 8) {
        nrStalls = 8; //limit to 8

        //we also have to rebalance width and length
        //most likely width is more then 2
        if (nrStallsWidth > 2) {
            //can we still do it with 2 stalls next to each other
            //this will be the solution in most cases
            irr::u8 newLen = (nrStalls / 2);

            if (newLen <= nrStallsLength) {
                //yes it works
                nrStallsWidth = 2;
                nrStallsLength = newLen;
            } else {
                //no does not, try 3 in width
                newLen = (nrStalls / 3);
                if (newLen <= nrStallsLength) {
                    //yes it works
                    nrStallsWidth = 3;
                    nrStallsLength = newLen;
                } else {
                    //still does not work
                    //do not modify len and width
                }
            }
        } else {
            //calculate new length for less number
            //of stalls
            nrStallsLength = (nrStalls / nrStallsWidth);
        }
    }

    this->mStandVec.clear();

    //move Y-coord slightly upwards, so that 3D lines are always better
    //visible when using drawDebug
    irr::f32 midPointHeight =
            -mRace->mLevelTerrain->GetHeightInterpolated(mRegionStruct->regionCenterTileCoord.X, mRegionStruct->regionCenterTileCoord.Y) + 0.1f;

    //our X Irrlicht coordinate system is swapped in sign
    corner1.set(-(usableTileXmin * segSize),            midPointHeight,     usableTileYmin * segSize);
    corner2.set(-(usableTileXmin * segSize),            midPointHeight,     usableTileYmax * segSize + segSize);
    corner3.set(-(usableTileXmax * segSize + segSize),  midPointHeight,     usableTileYmax * segSize + segSize);
    corner4.set(-(usableTileXmax * segSize + segSize),  midPointHeight,     usableTileYmin * segSize);

    //now create all available stalls
    if (nrStalls > 0) {
        irr::u8 stallIndx = 0;
        irr::u8 idxWidth = 0;
        irr::u8 idxLen = 0;
        irr::core::vector3df stallLocation(0.0f, midPointHeight, 0.0f);
        irr::core::vector3df startingCorner(0.0f, midPointHeight, 0.0f);

        irr::core::vector3df middleCoordinate(0.0f, midPointHeight, 0.0f);
        enterEntityItemLocation.Y = midPointHeight;
        helperEntityItemLocation.Y = midPointHeight;

        //calculate middle pos
        if (mWidthInZDir) {
            middleCoordinate.Z = (corner2.Z - corner4.Z) * 0.5f + corner4.Z;
            enterEntityItemLocation.Z = middleCoordinate.Z;

            helperEntityItemLocation.Z = middleCoordinate.Z;

            if (signInX > 0.0f) {
                middleCoordinate.X = (-usableTileXmin * segSize) - DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;
                enterEntityItemLocation.X = (-usableTileXmax * segSize) + DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;

                helperEntityItemLocation.X = (-usableTileXmin * segSize) + DEF_CHARGING_STATION_STAND_MINSIDELEN * 4.0f;
            } else {
                middleCoordinate.X = (-usableTileXmax * segSize) + DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;
                enterEntityItemLocation.X = (-usableTileXmin * segSize) - DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;

                helperEntityItemLocation.X = (-usableTileXmax * segSize) - DEF_CHARGING_STATION_STAND_MINSIDELEN * 4.0f;
            }
        } else {
               middleCoordinate.X = (corner4.X - corner2.X) * 0.5f + corner2.X;
               enterEntityItemLocation.X = middleCoordinate.X;

               helperEntityItemLocation.X = middleCoordinate.X;

               if (signInZ > 0.0f) {
                    middleCoordinate.Z = usableTileYmax * segSize - DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;
                    enterEntityItemLocation.Z =  usableTileYmin * segSize + DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;

                    helperEntityItemLocation.Z = usableTileYmax * segSize + DEF_CHARGING_STATION_STAND_MINSIDELEN * 4.0f;
               } else {
                   middleCoordinate.Z = usableTileYmin * segSize + DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;
                   enterEntityItemLocation.Z = usableTileYmax * segSize - DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;

                   helperEntityItemLocation.Z = usableTileYmin * segSize - DEF_CHARGING_STATION_STAND_MINSIDELEN * 4.0f;
               }     
        }

        irr::f32 wFactor = (irr::f32)(nrStallsWidth - 1) / 2.0f;

        //calculate starting pos for first stall based
        //on middle location
        if (mWidthInZDir) {
            startingCorner.X = middleCoordinate.X;

            if (signInX > 0.0f) {
                   startingCorner.Z = middleCoordinate.Z + DEF_CHARGING_STATION_STAND_MINSIDELEN * wFactor;
            } else {
                   startingCorner.Z = middleCoordinate.Z - DEF_CHARGING_STATION_STAND_MINSIDELEN * wFactor;
            }
        } else {
               startingCorner.Z = middleCoordinate.Z;

               if (signInZ > 0.0f) {
                    startingCorner.X = middleCoordinate.X - DEF_CHARGING_STATION_STAND_MINSIDELEN * wFactor;
               } else {
                   startingCorner.X = middleCoordinate.X + DEF_CHARGING_STATION_STAND_MINSIDELEN * wFactor;
               }
        }

        middlePos = middleCoordinate;
        startPos = startingCorner;

        while (stallIndx < nrStalls) {

            if (mWidthInZDir) {
                if (signInX > 0.0f) {
                    stallLocation.Z = startingCorner.Z - idxWidth * DEF_CHARGING_STATION_STAND_MINSIDELEN;
                    stallLocation.X = startingCorner.X - idxLen * DEF_CHARGING_STATION_STAND_MINSIDELEN;
                } else {
                    stallLocation.Z = startingCorner.Z + idxWidth * DEF_CHARGING_STATION_STAND_MINSIDELEN;
                    stallLocation.X = startingCorner.X + idxLen * DEF_CHARGING_STATION_STAND_MINSIDELEN;
                }
            } else {
                   if (signInZ > 0.0f) {
                        stallLocation.Z = startingCorner.Z - idxLen * DEF_CHARGING_STATION_STAND_MINSIDELEN;
                        stallLocation.X = startingCorner.X + idxWidth * DEF_CHARGING_STATION_STAND_MINSIDELEN;
                   } else {
                       stallLocation.Z = startingCorner.Z + idxLen * DEF_CHARGING_STATION_STAND_MINSIDELEN;
                       stallLocation.X = startingCorner.X - idxWidth * DEF_CHARGING_STATION_STAND_MINSIDELEN;
                   }
            }

            //create the new stall
            ChargerStoppingRegionStruct* newStall = new ChargerStoppingRegionStruct();
            newStall->reservedForPlayerPntr = NULL;

            //create an temporary (not stored in game level file) internal entity
            //item for this stall, so that computer player control functions
            //can work with it
            EntityItem* newItem = new EntityItem(stallLocation);
            newStall->entityItem = newItem;

            //TBD: correct value of offset!
            newStall->offsetFromWayPointLink = 0.0f;

            //generate also the debugging draw coordinates, so that
            //we can draw rectangles around the locations later for
            //debugging purposes
            irr::f32 halfDist = DEF_CHARGING_STATION_STAND_MINSIDELEN * 0.5f;

            newStall->dbgDrawVertex1.set(stallLocation.X + halfDist, stallLocation.Y, stallLocation.Z - halfDist);
            newStall->dbgDrawVertex2.set(stallLocation.X + halfDist, stallLocation.Y, stallLocation.Z + halfDist);
            newStall->dbgDrawVertex3.set(stallLocation.X - halfDist, stallLocation.Y, stallLocation.Z + halfDist);
            newStall->dbgDrawVertex4.set(stallLocation.X - halfDist, stallLocation.Y, stallLocation.Z - halfDist);

            //store in my list of available stalls
            this->mStandVec.push_back(newStall);

            idxWidth++;

            if (idxWidth >= nrStallsWidth) {
                idxWidth = 0;
                idxLen++;
            }

            stallIndx++;
        }
    }
}

//Returns true if request is granted, False otherwise
//if success is granted, pointer to stall that was reserved for this player
//is returned in variable reservedStall
bool ChargingStation::RequestCharging(Player *whichPlayer, ChargerStoppingRegionStruct *& reservedStall) {
    if (whichPlayer == NULL)
        return false;

    //find next free (unreserved) stall for requesting
    //player to charge at
    ChargerStoppingRegionStruct* useStall = GetNextFreeStall();

    //if currently there is no stall free for this
    //request, return request not granted
    if (useStall == NULL)
        return false;

    //reserve stall for this player
    useStall->reservedForPlayerPntr = whichPlayer;

    reservedStall = useStall;

    //grant request
    return true;
}

//Returns true if player is close to the entry of the charging station
//Which means the current waypoint link the player is located at (parameter currWayPointLink)
//is identical to one of the waypoint links that intersected the region of this charging station
bool ChargingStation::ReachedEntryOfChargingStation(WayPointLinkInfoStruct* currWayPointLink) {
    std::vector<WayPointLinkInfoStruct*>::iterator it;

    for (it = this->mIntersectingWayPointLinksVec.begin(); it != this->mIntersectingWayPointLinksVec.end(); ++it) {
        if ((*it) == currWayPointLink) {
            return true;
        }
    }

    return false;
}

//If there is no free stall right now for
//service returns NULL
ChargerStoppingRegionStruct* ChargingStation::GetNextFreeStall() {
    //lets iterate the stalls in reserve, so that we assign
    //the next player that wants to charge a stall that is physically
    //located closer to the entry area of the charger, so that
    //the new player does not need to physically overtake a currently
    //charging one
    //in case the charging area is currently completely empty we will come up
    //with the first charger at the far end of the charging area anyway,
    //so also in the empty case everything works out fine
    irr::u8 nrStalls = this->mStandVec.size();
    ChargerStoppingRegionStruct* pntrStall = NULL;

    if (nrStalls > 0) {
        for (irr::s16 idx = (nrStalls - 1); idx >= 0; idx--) {
            if (mStandVec.at(idx)->reservedForPlayerPntr != NULL) {
                //first stall that is reserved right now
                break;
            }

            pntrStall = mStandVec.at(idx);
        }
    }

    return pntrStall;
}

//debugging draw function for game testing
void ChargingStation::DebugDraw() {
    std::vector<ChargerStoppingRegionStruct*>::iterator it;

    //draw region for charging station
    mRace->mDrawDebug->Draw3DRectangle(corner1, corner2, corner3, corner4, mRace->mDrawDebug->pink);

    /*mRace->mDrawDebug->Draw3DLine(mRace->topRaceTrackerPointerOrigin, middlePos, mRace->mDrawDebug->blue);
    mRace->mDrawDebug->Draw3DLine(mRace->topRaceTrackerPointerOrigin, startPos, mRace->mDrawDebug->red);*/

    //draw the waypoint links that intersect me
   /* std::vector<WayPointLinkInfoStruct*>::iterator it2;

    for (it2 = this->mIntersectingWayPointLinksVec.begin(); it2 != this->mIntersectingWayPointLinksVec.end(); ++it2) {
       mRace->mDrawDebug->Draw3DLine((*it2)->pLineStruct->A, (*it2)->pLineStruct->B, mRace->mDrawDebug->cyan);
    }*/

    //draw all charging stalls
    for (it = this->mStandVec.begin(); it != this->mStandVec.end(); ++it) {
        //if stand is currently not reserved for a player draw
        //stand location in white, red otherwise
        if ((*it)->reservedForPlayerPntr == NULL) {
            mRace->mDrawDebug->Draw3DRectangle((*it)->dbgDrawVertex1, (*it)->dbgDrawVertex2, (*it)->dbgDrawVertex3, (*it)->dbgDrawVertex4, mRace->mDrawDebug->white);
        } else {
            mRace->mDrawDebug->Draw3DRectangle((*it)->dbgDrawVertex1, (*it)->dbgDrawVertex2, (*it)->dbgDrawVertex3, (*it)->dbgDrawVertex4, mRace->mDrawDebug->red);
        }
    }
}

void ChargingStation::ChargingFinished(Player *whichPlayer) {
    if (whichPlayer == NULL)
        return;

    //search for Player reservation in my stalls
    std::vector<ChargerStoppingRegionStruct*>::iterator it;

    for (it = mStandVec.begin(); it != mStandVec.end(); ++it) {
        if ((*it)->reservedForPlayerPntr == whichPlayer) {
            //delete reservation
            (*it)->reservedForPlayerPntr = NULL;
        }
    }
}

std::vector<irr::core::vector3df> ChargingStation::GetStallLocations() {
    std::vector<irr::core::vector3df> result;
    result.clear();

    std::vector<ChargerStoppingRegionStruct*>::iterator it;

    for (it = mStandVec.begin(); it != mStandVec.end(); ++it) {
        result.push_back((*it)->entityItem->getCenter());
    }

    return result;
}

void ChargingStation::Update() {
}

ChargingStation::~ChargingStation() {
    //remove the SceneNode from SceneManager
    mSceneNode->remove();

    //cleanup the Mesh
    mSmgr->getMeshCache()->removeMesh(mMesh);

    //cleanup my list of available stalls
    std::vector<ChargerStoppingRegionStruct*>::iterator it;
    ChargerStoppingRegionStruct* pntr;

    if (mStandVec.size() > 0) {
        for (it = mStandVec.begin(); it != mStandVec.end(); ) {
            pntr = (*it);

            it = mStandVec.erase(it);

            //delete the temporary/internal
            //entityItem behind it
            if (pntr->entityItem != NULL) {
                delete pntr->entityItem;
            }

            //delete the struct itself
            delete pntr;
        }
    }

    //delete the temporary entityItem for entering
    //the charging station for computer players
    if (this->enterEntityItem != NULL) {
        delete enterEntityItem;
        enterEntityItem = NULL;
    }

    if (this->helperEntityItem != NULL) {
        delete helperEntityItem;
        helperEntityItem = NULL;
    }
}
