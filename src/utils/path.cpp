/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "path.h"

Path::Path(Race* parentRace, DrawDebug* pntrDrawDebug)
{
    mRace = parentRace;
    mDrawDebug = pntrDrawDebug;
}

EntityItem* Path::FindNearestWayPointToLocation(irr::core::vector3df location) {
   std::vector<EntityItem*>::iterator it;
   irr::f32 minDistance;
   bool firstElement = true;
   irr::f32 currDist;
   EntityItem* nearestWayPoint;

   irr::core::vector3df wPos;

   if (mRace->ENTWaypoints_List->size() <= 0)
       return NULL;

   for (it = mRace->ENTWaypoints_List->begin(); it != mRace->ENTWaypoints_List->end(); ++it) {
       wPos = (*it)->get_Center();
       //my X-axis is flipped
       wPos.X = -wPos.X;
       currDist = (wPos - location).getLengthSQ();
       if (firstElement) {
           firstElement = false;
           minDistance = currDist;
           nearestWayPoint = (*it);
       } else if (currDist < minDistance) {
           minDistance = currDist;
           nearestWayPoint = (*it);
       }
   }

   return nearestWayPoint;
}

std::vector<EntityItem*> Path::FindAllWayPointsInArea(irr::core::vector3df location, irr::f32 radius) {
    std::vector<EntityItem*>::iterator it;
    std::vector<EntityItem*> result;

    result.clear();

    irr::f32 currDist;

    irr::core::vector3df wPos;

    if (mRace->ENTWaypoints_List->size() <= 0)
        return result;

    for (it = mRace->ENTWaypoints_List->begin(); it != mRace->ENTWaypoints_List->end(); ++it) {
        wPos = (*it)->get_Center();
        //my X-axis is flipped
        wPos.X = -wPos.X;
        currDist = (wPos - location).getLength();
        if (currDist < radius) {
            result.push_back(*it);
        }
    }

    return result;
}

EntityItem* Path::FindNearestWayPointToPlayer(Player* whichPlayer) {
   if (whichPlayer == NULL)
       return NULL;

   irr::core::vector3df playerPos = whichPlayer->phobj->physicState.position;

   return FindNearestWayPointToLocation(playerPos);
}

std::vector<WayPointLinkInfoStruct*> Path::FindWaypointLinksForWayPoint(EntityItem* wayPoint) {
   std::vector<WayPointLinkInfoStruct*>::iterator it;

   std::vector<WayPointLinkInfoStruct*> res;

   if (mRace->wayPointLinkVec->size() <= 0)
       return res;

   for (it = mRace->wayPointLinkVec->begin(); it != mRace->wayPointLinkVec->end(); ++it) {
       if (((*it)->pStartEntity == wayPoint) || ((*it)->pEndEntity == wayPoint)) {
           res.push_back(*it);
       }
   }

   return res;
}

WayPointLinkInfoStruct* Path::PlayerFindClosestWaypointLink(Player* whichPlayer) {
    std::vector<WayPointLinkInfoStruct*>::iterator WayPointLink_iterator;
    irr::core::vector3df dA;
    //irr::core::vector3df dB;
    irr::core::vector3df dASegmentLonger;
    irr::core::vector3df dBSegmentLonger;

    //irr::f32 projecteddA;
    //irr::f32 projecteddB;
    irr::f32 projecteddASegmentLonger;
    irr::f32 projecteddBSegmentLonger;
    irr::f32 projectedPl;
    irr::f32 distance;
    irr::f32 minDistance;
    bool firstElement = true;
    irr::core::vector3df projPlayerPosition;
    WayPointLinkInfoStruct* closestLink = NULL;
    WayPointLinkInfoStruct* LinkWithClosestStartEndPoint = NULL;
    irr::f32 minStartEndPointDistance;
    bool firstElementStartEndPoint = true;

    irr::f32 startPointDistHlper;
    irr::f32 endPointDistHlper;
    irr::core::vector3d<irr::f32> posH;

    if (whichPlayer == mRace->player) {

    //first reset colors of all waypoint links to white for debugging (white for unselected lines)
    /*for(WayPointLink_iterator = mRace->wayPointLinkVec->begin(); WayPointLink_iterator != mRace->wayPointLinkVec->end(); ++WayPointLink_iterator) {
        (*WayPointLink_iterator)->pLineStruct->color = mDrawDebug->white;
       // (*WayPointLink_iterator)->pLineStruct->debugLine = NULL;
    }*/
    }

    //iterate through all waypoint links
    for(WayPointLink_iterator = mRace->wayPointLinkVec->begin(); WayPointLink_iterator != mRace->wayPointLinkVec->end(); ++WayPointLink_iterator) {

        //for the workaround later (in case first waypoint link search does not work) also find in parallel the waypoint link that
        //has a start or end-point closest to the current player location
        posH = (*WayPointLink_iterator)->pStartEntity->get_Pos();
        posH.X = -posH.X;

         startPointDistHlper = ((whichPlayer->phobj->physicState.position - posH)).getLengthSQ();

         posH = (*WayPointLink_iterator)->pEndEntity->get_Pos();
         posH.X = -posH.X;

         endPointDistHlper = ((whichPlayer->phobj->physicState.position - posH)).getLengthSQ();

         if (endPointDistHlper < startPointDistHlper) {
             startPointDistHlper = endPointDistHlper;
         }

        if (firstElementStartEndPoint) {
           LinkWithClosestStartEndPoint = (*WayPointLink_iterator);
           minStartEndPointDistance = startPointDistHlper;
           firstElementStartEndPoint = false;
        } else {
            if (startPointDistHlper < minStartEndPointDistance) {
                //we have a new closest start/end point
                LinkWithClosestStartEndPoint = (*WayPointLink_iterator);
                minStartEndPointDistance = startPointDistHlper;
            }
        }

        //we want to find the waypoint link (line) to which the player is currently closest too (which the player currently tries to follow)
        //we also want to only find the line which is sideways of the player
        //first check if player is parallel to current line, or if the line is far away

        //important! add a little bit of length at the start and end of the waypoint link element,
        //too make sure the transitions between the waypoint links goes smooth
        dASegmentLonger = whichPlayer->phobj->physicState.position - ((*WayPointLink_iterator)->pLineStruct->A - (*WayPointLink_iterator)->LinkDirectionVec) * 1.0f;
        dBSegmentLonger = whichPlayer->phobj->physicState.position - ((*WayPointLink_iterator)->pLineStruct->B + (*WayPointLink_iterator)->LinkDirectionVec) * 1.0f;

        //projecteddA = dA.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);
        //projecteddB = dB.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);

        projecteddASegmentLonger = dASegmentLonger.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);
        projecteddBSegmentLonger = dBSegmentLonger.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);

        //if craft position is parallel (sideways) to current waypoint link the two projection
        //results need to have opposite sign; otherwise we are not sideways of this line, and need to ignore
        //this path segment
        if (sgn(projecteddASegmentLonger) != sgn(projecteddBSegmentLonger)) {
            dA = whichPlayer->phobj->physicState.position - (*WayPointLink_iterator)->pLineStruct->A;
            //dB = whichPlayer->phobj->physicState.position - (*WayPointLink_iterator)->pLineStruct->B;

            //this waypoint is interesting for further analysis
            //calculate distance from player position to this line, where connecting line meets path segment
            //in a 90° angle
            projectedPl =  dA.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);

            /*
            (*WayPointLink_iterator)->pLineStruct->debugLine = new irr::core::line3df((*WayPointLink_iterator)->pLineStruct->A +
                                                                                      projectedPl * (*WayPointLink_iterator)->LinkDirectionVec,
                                                                                      player->phobj->physicState.position);*/

            projPlayerPosition = (*WayPointLink_iterator)->pLineStruct->A +
                    irr::core::vector3df(projectedPl, projectedPl, projectedPl) * ((*WayPointLink_iterator)->LinkDirectionVec);

            distance = (projPlayerPosition - whichPlayer->phobj->physicState.position).getLength();

            //(*WayPointLink_iterator)->pLineStruct->color = mDrawDebug->pink;

            //prevent picking far away waypoint links
            //accidently (this happens especially when we are between
            //the end of the current waypoint link and the start
            //of the next one)
            if (distance < 10.0f) {

            if (firstElement) {
                minDistance = distance;
                closestLink = (*WayPointLink_iterator);
                whichPlayer->projPlayerPositionClosestWayPointLink = projPlayerPosition;
                firstElement = false;
            } else {
                if (distance < minDistance) {
                    minDistance = distance;
                    closestLink = (*WayPointLink_iterator);
                    whichPlayer->projPlayerPositionClosestWayPointLink = projPlayerPosition;
                 }
              }
          }
        }
    }

    //did we still not find the closest link? try some workaround
    if (closestLink == NULL) {
       //workaround, take the waypoint with either the closest
       //start or end entity
       closestLink = LinkWithClosestStartEndPoint;
    }

    /*if ((closestLink != NULL) && (whichPlayer == player)) {
        closestLink->pLineStruct->color = mDrawDebug->green;
    }*/

/*    if ((LinkWithClosestStartEndPoint != NULL) && (whichPlayer == player)) {
       LinkWithClosestStartEndPoint->pLineStruct->color = mDrawDebug->red;
    }*/

  return(closestLink);
}

irr::f32 Path::CalculateDistanceFromWaypointLinkToNextCheckpoint(WayPointLinkInfoStruct* startWaypointLink) {
    irr::f32 sumDistance = 0.0f;
    WayPointLinkInfoStruct* currLink;

    currLink = startWaypointLink;

    //create sum of all distances

    //The next line is for debugging
    //currLink->pLineStruct->color = mDrawDebug->green;

    //calculate length of vector from current 3D Player projected position on waypoint line to start of line

    sumDistance += (currLink->length3D);

    //now follow the waypoint links forward until we hit the next checkpoint
    while (currLink->pntrCheckPoint == NULL) {  //follow one link after another until we hit the next checkpoint
        currLink = currLink->pntrPathNextLink;

        if (currLink != NULL) {
             if (currLink->pntrCheckPoint == NULL) {
                    //The next line is for debugging
                    //currLink->pLineStruct->color = mDrawDebug->blue;

                    //for this links add up the whole length
                    sumDistance += currLink->length3D;
                } else {
                    //there is a checkpoint within this waypoint link
                    //for this one add only the distance from the start point until the checkpoint location
                    sumDistance += currLink->distanceStartLinkToCheckpoint;
                    //currLink->pLineStruct->color = mDrawDebug->red;
                    break;
                }
            }
        }

    return sumDistance;
}

EntityItem* Path::FindFirstWayPointAfterRaceStartPoint() {
    //get the player start positions
    std::vector<irr::core::vector3df> playerStartLocations =
            this->mRace->mLevelTerrain->GetPlayerRaceTrackStartLocations();

    //just take player 1 start position as the reference
    irr::core::vector3df Startpos = playerStartLocations.at(0);

    EntityItem* item = FindNearestWayPointToLocation(Startpos);

    return (item);
}

//recursive helper function to follow a path backwards
//to search for a specified player
bool Path::ContinuePathSearchForPlayer(WayPointLinkInfoStruct* startLink,
                                             WayPointLinkInfoStruct* wayPointLinkNearPlayer,
                                             std::vector<WayPointLinkInfoStruct*> &resultPath, WayPointLinkInfoStruct *interruptLink,
                                             bool firstLink) {

    //startLink->pLineStruct->color = mDrawDebug->pink;
    //interruptLink->pLineStruct->color = mDrawDebug->red;

    resultPath.push_back(startLink);

    if (wayPointLinkNearPlayer == startLink) {
            //player is next to this link
            return true;
    }

    std::vector<WayPointLinkInfoStruct*> linksToFollowBackwards;
    linksToFollowBackwards.clear();

    std::vector<WayPointLinkInfoStruct*>::iterator itLink;

    //which links go into this current link
    for (itLink = this->mRace->wayPointLinkVec->begin(); itLink != this->mRace->wayPointLinkVec->end(); ++itLink) {
        if ((*itLink)->pntrPathNextLink == startLink) {
            //yes, this is a possible candidate
            linksToFollowBackwards.push_back(*itLink);
        }
    }

    //go and follow the next waypoint link backwards that leads to this
    //start link
    if (linksToFollowBackwards.size() < 1) {
        //no more links to follow backwards
        return false;
    }

    bool playerFound;

    //follow each element backwards, one after another
       for (itLink = linksToFollowBackwards.begin(); itLink != linksToFollowBackwards.end(); ++itLink) {


            if ((startLink != interruptLink) || (firstLink)) {
                playerFound = ContinuePathSearchForPlayer((*itLink), wayPointLinkNearPlayer, resultPath, interruptLink, false);

               if (playerFound == true) {
                 return true;
               } else {
                   //remove the last added element again
                   resultPath.pop_back();
               }
            }
        }

        return false;
}

std::vector<WayPointLinkInfoStruct*> Path::FindPathToNextCheckPoint(Player *whichPlayer) {
    std::vector<WayPointLinkInfoStruct*> result;
    result.clear();

    //which is the next checkpoint to reach for
    //this player
    irr::s32 nextCheckPointValue = whichPlayer->nextCheckPointValue;

    //find the waypoint link to crosses this checkpoint
    std::vector<WayPointLinkInfoStruct*>::iterator it;
    WayPointLinkInfoStruct* linkPntr = NULL;

    for (it = this->mRace->wayPointLinkVec->begin(); it != this->mRace->wayPointLinkVec->end(); ++it) {
        if ((*it)->pntrCheckPoint != NULL) {
            if ((*it)->pntrCheckPoint->value == nextCheckPointValue) {
                //we found the correct waypoint link
                linkPntr = (*it);
                break;
            }
        }
    }

    //if we did not find the first waypoint link under
    //the next checkpoint for this player, then simply exit
    //with empty path
    if (linkPntr == NULL)
        return result;

    WayPointLinkInfoStruct* linkNearPlayer = this->PlayerFindClosestWaypointLink(whichPlayer);

    if (linkNearPlayer == NULL)
        return result;

    bool playerFound = false;

    //actually start with the link after the checkpoint so the craft can fly
    //a defined path through the next checkpoint
    if (linkPntr->pntrPathNextLink != NULL) {
        linkPntr = linkPntr->pntrPathNextLink;
    }

    //start recursive search backwards for player
    playerFound = ContinuePathSearchForPlayer(linkPntr, linkNearPlayer, result, linkPntr, true);

    return result;
}

