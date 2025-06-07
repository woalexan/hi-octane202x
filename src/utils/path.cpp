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
       return nullptr;

   //we need to ignore the Y-coordinate, we also want to find
   //waypoints that are slightly different in terrain elevation!
   location.Y = 0.0f;

   for (it = mRace->ENTWaypoints_List->begin(); it != mRace->ENTWaypoints_List->end(); ++it) {
       wPos = (*it)->getCenter();

       //we need to ignore the Y-coordinate, we also want to find
       //waypoints that are slightly different in terrain elevation!
       wPos.Y = 0.0f;

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

//returns false if sani check of bezier input points is "crazy", means one of the points is "not" in line
//the player
bool Path::SaniCheckBezierInputPoints(irr::core::vector2df startPnt, irr::core::vector2df cntrlPnt,
                                      irr::core::vector2df endPnt, irr::core::vector2df raceDirection) {

   irr::core::vector2df dirCntrlPnt = (cntrlPnt - startPnt).normalize();
   irr::core::vector2df dirEndPnt = (endPnt - startPnt).normalize();

   irr::f32 result = dirCntrlPnt.dotProduct(dirEndPnt);

   if (result < 0.0f)
       return false;

   result = dirCntrlPnt.dotProduct(raceDirection);

   //both direction vectors must be in "race Direction" and not opposite
   if (result < 0.0f)
       return false;

   result = dirEndPnt.dotProduct(raceDirection);

   if (result < 0.0f)
       return false;

   //all ok
   return true;
}

//Runs a plausi check to see if two 3D coordinates for waypoint planing in front of the player (which position and
//and forward direction is also taken into account) make sense according to their order, or if the two waypoint
//locations actually needs to be swapped
//returns true if order is correct, returns false if order is not correct
bool Path::SaniCheck3DPointOrder(irr::core::vector3df point1, irr::core::vector3df point2, Player* player) {
    irr::core::vector3df playerPos = player->phobj->physicState.position;

    irr::core::vector3df dirPlayerP2 = (point2 - playerPos);
    dirPlayerP2.Y = 0.0f;
    dirPlayerP2.normalize();

    irr::core::vector3df dirP1ToP2 = (point2 - point1);
    dirP1ToP2.Y = 0.0f;
    dirP1ToP2.normalize();

    irr::f32 dotProduct = dirPlayerP2.dotProduct(dirP1ToP2);

    //if the dot Product is negative (means the two direction vectors
    //are opposite sign), then we know position of point1 and point2 is swapped
    //in this case the Sani check fails, and this two points should be swapped
    if (dotProduct < 0.0f)
        return false;

    //point order makes sense, "sani check" passes
    return true;
}

//Helper function which takes a WayPointLink, and returns StartEntity pointer in case it is also in
//front of current player location, or if not returns the endEntity pointer
//If advanceForOneEntity parameter is true, the function does not return the first match for entity, but instead returns
//the next one on the path
EntityItem* Path::GetWayPointLinkEntityItemInFrontOfPlayer(WayPointLinkInfoStruct* whichWayPointLink, bool advanceForOneEntity, Player* whichPlayer) {
    irr::core::vector3df toPointA = (whichWayPointLink->pLineStruct->A - whichPlayer->phobj->physicState.position);
    toPointA.Y = 0.0f;
    toPointA.normalize();

    irr::core::vector3df playerFwdDir = whichPlayer->craftForwardDirVec;
    playerFwdDir.Y = 0.0f;
    playerFwdDir.normalize();

    irr::f32 dotProd = toPointA.dotProduct(playerFwdDir);

    if (dotProd > 0.0f) {
        //starting point seems to be in front of player, return this entity
        if (!advanceForOneEntity) {
            return whichWayPointLink->pStartEntity;
        } else {
            //we want to advance one entity
            return whichWayPointLink->pEndEntity;
        }
    }

    //waypoint A is behind the player
    //return end entity
    if (!advanceForOneEntity) {
        return whichWayPointLink->pEndEntity;
    }

    //we want to return the next entity on the path
    //hopefully there is one :(
    //lets return the endpoint of the next one,
    //as the startpoint of the next one does not really help,
    //as it is at the same location as the endpoint of the inital entity
    return whichWayPointLink->pntrPathNextLink->pEndEntity;
}

irr::core::vector2df Path::WayPointLinkGetRaceDirection2D(WayPointLinkInfoStruct* whichWayPointLink) {
  irr::core::vector2df result;

  result.X = whichWayPointLink->LinkDirectionVec.X;
  result.Y = whichWayPointLink->LinkDirectionVec.Z;

  return result;
}

void Path::OffsetWayPointLinkCoordByOffset(irr::core::vector2df &coord2D,
                                           irr::core::vector3df &coord3D, WayPointLinkInfoStruct* waypoint, irr::f32 offset) {

    //use the precalculated "offset" direction vector
    coord3D = coord3D + offset * waypoint->offsetDirVec;

    irr::core::vector2df dirVec2D(waypoint->offsetDirVec.X, waypoint->offsetDirVec.Z);
    dirVec2D.normalize();

    coord2D = coord2D + offset * dirVec2D;
}

std::vector<EntityItem*> Path::FindAllWayPointsInArea(irr::core::vector3df location, irr::f32 radius) {
    std::vector<EntityItem*>::iterator it;
    std::vector<EntityItem*> result;

    result.clear();

    irr::f32 currDist;

    irr::core::vector3df wPos;

    if (mRace->ENTWaypoints_List->size() <= 0)
        return result;

    //we need to ignore the Y-coordinate, we also want to find
    //waypoints that are slightly different in terrain elevation!
    location.Y = 0.0f;

    for (it = mRace->ENTWaypoints_List->begin(); it != mRace->ENTWaypoints_List->end(); ++it) {
        wPos = (*it)->getCenter();

        //we need to ignore the Y-coordinate, we also want to find
        //waypoints that are slightly different in terrain elevation!
        wPos.Y = 0.0f;

        currDist = (wPos - location).getLength();
        if (currDist < radius) {
            result.push_back(*it);
        }
    }

    return result;
}

std::vector<WayPointLinkInfoStruct*> Path::DeliverAllWayPointLinksThatLeadIntpSpecifiedToWayPointLink(
        WayPointLinkInfoStruct* inputWayPointLink) {
    std::vector<WayPointLinkInfoStruct*> result;

    std::vector<WayPointLinkInfoStruct*>::iterator it;

    for (it = mRace->wayPointLinkVec->begin(); it != mRace->wayPointLinkVec->end(); ++it) {
       if ((*it)->pntrPathNextLink == inputWayPointLink) {
           result.push_back(*it);
       }
    }

    return result;
}

//is used within Recovery vehicle to be able to execute a workaround in case we do not know the location
//anymore where to put a player back after its physics reset
std::vector<WayPointLinkInfoStruct*> Path::DeliverAllWayPointLinksThatLeadIntoPlayersNextExpectedCheckpoint(Player* player) {
    std::vector<WayPointLinkInfoStruct*> result;
    result.clear();

    if (player == nullptr)
        return result;

    //which waypoint link lays below the players next
    //expected checkpoint
    irr::s32 nextCheckPoint = player->nextCheckPointValue;

    std::vector<WayPointLinkInfoStruct*>::iterator it;
    WayPointLinkInfoStruct* checkPointLink = nullptr;

    for (it = mRace->wayPointLinkVec->begin(); it != mRace->wayPointLinkVec->end(); ++it) {
       //there is a checkpoint at this waypoint link?
       if ((*it)->pntrCheckPoint != nullptr) {
           //we found it?
           if ((*it)->pntrCheckPoint->value == nextCheckPoint) {
               checkPointLink = (*it);
               break;
           }
       }
    }

    if (checkPointLink == nullptr)
        return result;

    result = DeliverAllWayPointLinksThatLeadIntpSpecifiedToWayPointLink(checkPointLink);

    return result;
}

EntityItem* Path::FindNearestWayPointToPlayer(Player* whichPlayer) {
   if (whichPlayer == nullptr)
       return nullptr;

   irr::core::vector3df playerPos = whichPlayer->phobj->physicState.position;

   return FindNearestWayPointToLocation(playerPos);
}

std::vector<WayPointLinkInfoStruct*> Path::FindWaypointLinksForWayPoint(EntityItem* wayPoint, bool whenStart, bool whenEnd,
                                                                        WayPointLinkInfoStruct* excludeWhichLink) {
   std::vector<WayPointLinkInfoStruct*>::iterator it;

   std::vector<WayPointLinkInfoStruct*> res;

   if (mRace->wayPointLinkVec->size() <= 0)
       return res;

   bool alreadyAdded;

   for (it = mRace->wayPointLinkVec->begin(); it != mRace->wayPointLinkVec->end(); ++it) {
       alreadyAdded = false;

       //we want to be able to exclude one
       //choosen link
       if ((*it) != excludeWhichLink) {
           if (whenStart) {
            if ((*it)->pStartEntity == wayPoint) {
                res.push_back(*it);
                alreadyAdded = true;
            }
           }

           if ((!alreadyAdded) && whenEnd) {
            if ((*it)->pEndEntity == wayPoint) {
                res.push_back(*it);
            }
           }
     }
   }

   return res;
}

void Path::AddWayPointLinkToOccurenceList(std::vector<std::pair <irr::u8, WayPointLinkInfoStruct*>> &wayPointLinkOccurenceList,
                                            WayPointLinkInfoStruct* newWayPointLink)
{
   //check if input waypoint link already exists in list
   std::vector<std::pair <irr::u8, WayPointLinkInfoStruct*>>::iterator it;

   for (it = wayPointLinkOccurenceList.begin(); it != wayPointLinkOccurenceList.end(); ++it) {
       if ((*it).second == newWayPointLink) {
           //waypoint link does already exist in list
           //just increase occurence
           (*it).first += 1;
           return;
       }
   }

   //waypoint link does not yet exist in list
   //add entry with occurence 1
   wayPointLinkOccurenceList.push_back( std::make_pair(1, newWayPointLink));
}

void Path::AddWayPointLinkResultToVector(std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> &wayPointLinkList,
                                            WayPointLinkInfoStruct* newWayPointLink, irr::core::vector3df newProjectedPlayerPosition)
{
   //check if input waypoint link already exists in vector
   std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>>::iterator it;

   for (it = wayPointLinkList.begin(); it != wayPointLinkList.end(); ++it) {
       if ((*it).first == newWayPointLink) {
           //waypoint link does already exist in list
           return;
       }
   }

   //waypoint link does not yet exist in list
   //add entry with new projected player position
   wayPointLinkList.push_back( std::make_pair(newWayPointLink, newProjectedPlayerPosition));
}

std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> Path::PlayerDeriveClosestWaypointLink(
        std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>>

                                                                                                inputWayPointLinkVector) {

    WayPointLinkInfoStruct* nullLink = nullptr;

    if (inputWayPointLinkVector.size() < 1)
        return ( std::make_pair(nullLink, irr::core::vector3df(0.0f, 0.0f, 0.0f)));

    //make a majority decision
    std::vector< std::pair <irr::u8, WayPointLinkInfoStruct*> > vecWayPointLinkOccurence;
    vecWayPointLinkOccurence.clear();

    std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>>::iterator it;

    for (it = inputWayPointLinkVector.begin(); it != inputWayPointLinkVector.end(); ++it) {
       AddWayPointLinkToOccurenceList(vecWayPointLinkOccurence, (*it).first);
    }

    //now sort list of waypoint link occurences with falling number
    //of occurences, the waypoint link we want to find is then the one at the top

    //sort vector pairs in descending number of occurences
    std::sort(vecWayPointLinkOccurence.rbegin(), vecWayPointLinkOccurence.rend());

    WayPointLinkInfoStruct* closestLink = vecWayPointLinkOccurence.begin()->second;

    for (it = inputWayPointLinkVector.begin(); it != inputWayPointLinkVector.end(); ++it) {
       if (closestLink == (*it).first) {
           return (*it);
       }
    }

    return ( std::make_pair(nullLink, irr::core::vector3df(0.0f, 0.0f, 0.0f)));
}

std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> Path::FindClosestWayPointLinkToCollectible(Collectable* whichCollectable) {
   std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> result;

   irr::core::vector3df targetPos = whichCollectable->Position;
   irr::core::vector3df projPos(0.0f, 0.0f, 0.0f);

   result.first = PlayerFindClosestWaypointLinkHelper(targetPos, projPos);
   result.second = projPos;

   return result;
}

//returns all waypoint links of a defined input path that come closer to a defined player than a distance of distanceLowLimit
std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> Path::WhereDoesPathComeCloseToPlayer(std::vector<WayPointLinkInfoStruct*> path,
                                                                                         irr::f32 distanceLowLimit, Player* checkForWhichPlayer,
                                                                                        std::vector<irr::f32> &whichDistanceVec) {
    std::vector<WayPointLinkInfoStruct*>::iterator it;
    std::vector<irr::f32> inWhichDistanceVec;
    inWhichDistanceVec.clear();

    irr::f32 currDistance = 0.0f;

    std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> result;
    result.clear();

    irr::core::vector3df playerLoc = checkForWhichPlayer->phobj->physicState.position;
    irr::core::vector3df projPos;
    irr::f32 distance;

    for (it = path.begin(); it != path.end(); ++it) {
        if (this->mRace->mPath->ProjectPositionAtWayPointLink(playerLoc, (*it), projPos)) {
            distance = (playerLoc - projPos).getLength();

            if (distance < distanceLowLimit) {
                result.push_back(std::make_pair((*it), projPos));

                //add additional distance from A to projPos
                inWhichDistanceVec.push_back(currDistance + (projPos - (*it)->pLineStruct->A).getLength());
            }
        }

        currDistance += (*it)->length3D;
    }

    whichDistanceVec = inWhichDistanceVec;
    return result;
}

bool Path::DoesPathComeTooCloseToAnyOtherPlayer(std::vector<WayPointLinkInfoStruct*> path,
                                  std::vector<Player*> checkCollisionForWhichPlayers,
                                                std::vector<Player*> &playersInWay) {

    //Note 18.01.2025: This function causes me more problems right now as it solves
    //I am not sure if I will use it at the end. I will leave the computer players like the
    //are right now, because I am already working on this stuff for at least several months
    //I am getting really bored now by it
    return false;

   std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> result;
   std::vector<Player*>::iterator itPlayer;
   std::vector<irr::f32> inWhichDistanceVec;
   std::vector<Player*>::iterator it2;
   bool playerFound = false;

   for (itPlayer = checkCollisionForWhichPlayers.begin(); itPlayer != checkCollisionForWhichPlayers.end(); ++itPlayer) {

       result = WhereDoesPathComeCloseToPlayer(path, 2.0f, (*itPlayer), inWhichDistanceVec);

       bool playerClose = false;

       std::vector<irr::f32>::iterator it;

       for (it = inWhichDistanceVec.begin(); it != inWhichDistanceVec.end(); ++it) {
           if ((*it) < 2.0f) {
               playerClose = true;
           }
       }

         if ((result.size() > 0) && playerClose) {
             //other player comes close to our path
             //return true
             //is this player already in our list, if so do not add it again
             //if not add it
             for (it2 = playersInWay.begin(); it2 != playersInWay.end(); ++it2) {
                if ((*it2) == (*itPlayer)) {
                    playerFound = true;
                    break;
                }
             }

             if (!playerFound) {
                playersInWay.push_back((*itPlayer));
             }
         }
   }

   if (playersInWay.size() > 0)
       return true;

   //no collision problem, return false
   return false;
}

//this function returns (if more are existing) multiple close waypoint links to the player craft
std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> Path::PlayerFindCloseWaypointLinks(Player* whichPlayer) {

    irr::core::vector3df projPlayerPositionFront;
   // WayPointLinkInfoStruct* frontLink = PlayerFindClosestWaypointLinkHelper(whichPlayer->WorldCoordCraftFrontPnt, projPlayerPositionFront);

    irr::core::vector3df projPlayerPositionMid;
    WayPointLinkInfoStruct* midLink = PlayerFindClosestWaypointLinkHelper(whichPlayer->phobj->physicState.position, projPlayerPositionMid);

    irr::core::vector3df projPlayerPositionBack;
   // WayPointLinkInfoStruct* backLink = PlayerFindClosestWaypointLinkHelper(whichPlayer->WorldCoordCraftBackPnt, projPlayerPositionBack);

    std::vector< std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> > vecWayPointLinkResult;
    vecWayPointLinkResult.clear();

    /*if (frontLink != nullptr) {
        AddWayPointLinkResultToVector(vecWayPointLinkResult, frontLink, projPlayerPositionFront);
    }*/
    if (midLink != nullptr) {
        AddWayPointLinkResultToVector(vecWayPointLinkResult, midLink, projPlayerPositionMid);
    }
   /* if (backLink != nullptr) {
        AddWayPointLinkResultToVector(vecWayPointLinkResult, backLink, projPlayerPositionBack);
    }*/

    return vecWayPointLinkResult;
}

//returns true if the position seems to be next to the specified waypoint link, false otherwise
//the resulting projected position is returned in projPosition reference
bool Path::ProjectPositionAtWayPointLink(irr::core::vector3df position, WayPointLinkInfoStruct* link,
                                                         irr::core::vector3df &projPosition) {
    irr::core::vector3df dA;
    irr::core::vector3df dB;

    irr::f32 projecteddA;
    irr::f32 projecteddB;
    irr::f32 projectedPl;
    //irr::f32 distance;

    dA = position - link->pLineStruct->A;
    dB = position - link->pLineStruct->B;

    projecteddA = dA.dotProduct(link->LinkDirectionVec);
    projecteddB = dB.dotProduct(link->LinkDirectionVec);

    //if craft position is parallel (sideways) to current waypoint link the two projection
    //results need to have opposite sign; otherwise we are not sideways of this line, and need to ignore
    //this path segment
    if (sgn(projecteddA) != sgn(projecteddB))
    {
        //we seem to be still parallel to this segment => projection will still give useful results
        //calculate distance from player position to this line, where connecting line meets path segment
        //in a 90° angle
        projectedPl =  dA.dotProduct(link->LinkDirectionVec);

        /*
        (*WayPointLink_iterator)->pLineStruct->debugLine = new irr::core::line3df((*WayPointLink_iterator)->pLineStruct->A +
                                                                                  projectedPl * (*WayPointLink_iterator)->LinkDirectionVec,
                                                                                    player->phobj->physicState.position);*/

        projPosition = (link->pLineStruct->A +
                irr::core::vector3df(projectedPl, projectedPl, projectedPl) * (link->LinkDirectionVec));

        return true;
    }

    return false;
}

WayPointLinkInfoStruct* Path::PlayerFindClosestWaypointLinkHelper(irr::core::vector3df inputPosition, irr::core::vector3df
                                                                  &projectedPlayerPosition) {
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
    WayPointLinkInfoStruct* closestLink = nullptr;
    WayPointLinkInfoStruct* LinkWithClosestStartEndPoint = nullptr;
    irr::f32 minStartEndPointDistance;
    bool firstElementStartEndPoint = true;

    irr::f32 startPointDistHlper;
    irr::f32 endPointDistHlper;
    irr::core::vector3d<irr::f32> posH;
    irr::core::vector3df distVec;
    irr::core::vector3df linkVec;

    //iterate through all waypoint links
    for(WayPointLink_iterator = mRace->wayPointLinkVec->begin(); WayPointLink_iterator != mRace->wayPointLinkVec->end(); ++WayPointLink_iterator) {

        //for the workaround later (in case first waypoint link search does not work) also find in parallel the waypoint link that
        //has a start or end-point closest to the current player location
        //posH = (*WayPointLink_iterator)->pStartEntity->getCenter();
        posH = (*WayPointLink_iterator)->pLineStruct->A;

        distVec = inputPosition - posH;
        distVec.Y = 0.0f;

        startPointDistHlper = distVec.getLengthSQ();
        //posH = (*WayPointLink_iterator)->pEndEntity->getCenter();
        posH = (*WayPointLink_iterator)->pLineStruct->B;

        distVec = inputPosition - posH;
        distVec.Y = 0.0f;

        endPointDistHlper = distVec.getLengthSQ();

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
        //dASegmentLonger = inputPosition - ((*WayPointLink_iterator)->pLineStruct->A - (*WayPointLink_iterator)->LinkDirectionVec) * 1.0f;
        //dBSegmentLonger = inputPosition - ((*WayPointLink_iterator)->pLineStruct->B + (*WayPointLink_iterator)->LinkDirectionVec) * 1.0f;

        dASegmentLonger = inputPosition - (*WayPointLink_iterator)->pLineStructExtended->A;
        dBSegmentLonger = inputPosition - (*WayPointLink_iterator)->pLineStructExtended->B;

        dASegmentLonger.Y = 0.0f;
        dBSegmentLonger.Y = 0.0f;

        //projecteddA = dA.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);
        //projecteddB = dB.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);

        linkVec = (*WayPointLink_iterator)->LinkDirectionVec;
        linkVec.Y = 0.0f;

        projecteddASegmentLonger = dASegmentLonger.dotProduct(linkVec);
        projecteddBSegmentLonger = dBSegmentLonger.dotProduct(linkVec);

        //if craft position is parallel (sideways) to current waypoint link the two projection
        //results need to have opposite sign; otherwise we are not sideways of this line, and need to ignore
        //this path segment
        if (sgn(projecteddASegmentLonger) != sgn(projecteddBSegmentLonger)) {
            dA = inputPosition - (*WayPointLink_iterator)->pLineStructExtended->A;
            //dB = whichPlayer->phobj->physicState.position - (*WayPointLink_iterator)->pLineStruct->B;
            dA.Y = 0.0f;

            //this waypoint is interesting for further analysis
            //calculate distance from player position to this line, where connecting line meets path segment
            //in a 90° angle
            projectedPl =  dA.dotProduct(linkVec);

            /*
            (*WayPointLink_iterator)->pLineStruct->debugLine = new irr::core::line3df((*WayPointLink_iterator)->pLineStruct->A +
                                                                                      projectedPl * (*WayPointLink_iterator)->LinkDirectionVec,
                                                                                      player->phobj->physicState.position);*/

            projPlayerPosition = (*WayPointLink_iterator)->pLineStructExtended->A +
                    irr::core::vector3df(projectedPl, projectedPl, projectedPl) * (linkVec);

            distance = (projPlayerPosition - inputPosition).getLength();

            //(*WayPointLink_iterator)->pLineStruct->color = mDrawDebug->pink;

            //prevent picking far away waypoint links
            //accidently (this happens especially when we are between
            //the end of the current waypoint link and the start
            //of the next one)

            //Note 12.01.2025: It seems original 10.0f distance could be not
            //enough, increased it experiment wise to 100.0f limit
            //if (distance < 10.0f) {
            if (distance < 50.0f) {
                if (firstElement) {
                    minDistance = distance;
                    closestLink = (*WayPointLink_iterator);
                    projectedPlayerPosition = projPlayerPosition;
                    firstElement = false;
                } else {
                    if (distance < minDistance) {
                        minDistance = distance;
                        closestLink = (*WayPointLink_iterator);
                        projectedPlayerPosition = projPlayerPosition;
                     }
                  }
          }
        }
    }

    //did we still not find the closest link? try some workaround
    if (closestLink == nullptr) {
       //workaround, take the waypoint with either the closest
       //start or end entity
       closestLink = LinkWithClosestStartEndPoint;
    }

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
    while (currLink->pntrCheckPoint == nullptr) {  //follow one link after another until we hit the next checkpoint
        currLink = currLink->pntrPathNextLink;

        if (currLink != nullptr) {
             if (currLink->pntrCheckPoint == nullptr) {
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
    WayPointLinkInfoStruct* linkPntr = nullptr;

    for (it = this->mRace->wayPointLinkVec->begin(); it != this->mRace->wayPointLinkVec->end(); ++it) {
        if ((*it)->pntrCheckPoint != nullptr) {
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
    if (linkPntr == nullptr)
        return result;

    //WayPointLinkInfoStruct* linkNearPlayer = this->PlayerFindClosestWaypointLink(whichPlayer);
    whichPlayer->currCloseWayPointLinks = mRace->mPath->PlayerFindCloseWaypointLinks(whichPlayer);
    whichPlayer->SetCurrClosestWayPointLink(mRace->mPath->PlayerDeriveClosestWaypointLink(whichPlayer->currCloseWayPointLinks));

    WayPointLinkInfoStruct* linkNearPlayer = whichPlayer->currClosestWayPointLink.first;

    if (linkNearPlayer == nullptr)
        return result;

    bool playerFound = false;

    //actually start with the link after the checkpoint so the craft can fly
    //a defined path through the next checkpoint
    if (linkPntr->pntrPathNextLink != nullptr) {
        linkPntr = linkPntr->pntrPathNextLink;
    }

    if (linkPntr->pntrPathNextLink != nullptr) {
        linkPntr = linkPntr->pntrPathNextLink;
    }

    //start recursive search backwards for player
    playerFound = ContinuePathSearchForPlayer(linkPntr, linkNearPlayer, result, linkPntr, true);

    return result;
}

//returns a vector containing all charging stations
//which a certain defined waypoint link intersects
std::vector<ChargingStation*> Path::WhichChargingStationsDoesAWayPointLinkIntersect(WayPointLinkInfoStruct* whichLink) {
    std::vector<ChargingStation*> result;

    result.clear();
    if (whichLink == nullptr)
        return result;

    std::vector<ChargingStation*>::iterator it;
    irr::core::line3df line3D;

    //create a 3D line along the specified waypoint link
    line3D.start = whichLink->pLineStruct->A;
    line3D.end = whichLink->pLineStruct->B;

    for (it = this->mRace->mChargingStationVec->begin(); it != this->mRace->mChargingStationVec->end(); ++it) {
        //get bounding box of region mesh that was prepared before
        irr::core::aabbox3d bbox = (*it)->mSceneNode->getTransformedBoundingBox();

        //does this link intersect the charging region bounding box?
        if (bbox.intersectsWithLine(line3D)) {
           //yes, we found a charging station
           //add it to result vector
           result.push_back(*it);
        }
    }

    return result;
}

//Returns nullptr if within the next 5 waypoint links the specified
//charger type was not found, but with proper level design this
//should never happen
ChargingStation* Path::GetChargingStationAhead(WayPointLinkInfoStruct* startAtWhichLink, irr::u8 chargerTypeToFind) {
    if (startAtWhichLink == nullptr)
        return nullptr;

    WayPointLinkInfoStruct* currLink = startAtWhichLink;
    bool chargerFound = false;
    irr::u8 iteration = 0;
    std::vector<ChargingStation*> intersectedChargingStations;
    std::vector<ChargingStation*>::iterator it2;

    ChargingStation* foundCharger = nullptr;

    do {
        iteration++;
        if (currLink != nullptr) {
            //does this link intersect any of the charger regions?
            //if yes, is this the type of charger we expect to find?
            intersectedChargingStations = WhichChargingStationsDoesAWayPointLinkIntersect(currLink);

            if (intersectedChargingStations.size() > 0) {
                //is it the type of charger we expected?
                for (it2 = intersectedChargingStations.begin(); it2 != intersectedChargingStations.end(); ++it2) {
                    if ((*it2)->GetChargingStationType() == chargerTypeToFind) {
                        //yes, we have a match
                        foundCharger = (*it2);
                        chargerFound = true;
                        break;
                    }
                }
            }

            //we did not find the match yet, continue with next waypointlink
            currLink = currLink->pntrPathNextLink;
        }
    } while ((!chargerFound) && (iteration < 5));

    return foundCharger;
}
