/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PATH_H
#define PATH_H

#include "irrlicht.h"
#include <vector>
#include "../definitions.h"

/************************
 * Forward declarations *
 ************************/

class Race;
class DrawDebug;
class Player;
class EntityItem;
class Collectable;
class ChargingStation;

struct CheckPointInfoStruct {
    //pntr to checkpoint Irrlicht sceneNode
    irr::scene::IMeshSceneNode* SceneNode = nullptr;
    //pntr to checkpoint Irrlich Mesh
    irr::scene::SMesh* Mesh = nullptr;

    //value for checkpoint, is defined
    //within level file with increasing
    //integer values
    irr::s32 value = 0;

    EntityItem* pEntity = nullptr;
    LineStruct* pLineStruct = nullptr;

    //if we want to figure out if a player
    //craft flies in normal race direction
    //or reverse through the checkpoint
    //we want to figure out a forward
    //race direction pointer
    irr::core::vector3df RaceDirectionVec;
};

struct WayPointLinkInfoStruct {
    EntityItem* pStartEntity = nullptr;
    EntityItem* pEndEntity = nullptr;

    //3D line that links two waypoints together (as defined in the map file
    //via waypoint entities)
    LineStruct* pLineStruct = nullptr;

    //Idea: extend the lines a little bit further outwards at
    //both ends, so that when we project the players position on
    //the different segments later we always find a valid segment
    LineStruct* pLineStructExtended = nullptr;

    //normalized link direction vector
    irr::core::vector3df LinkDirectionVec;

    //the 3D length from Start entity center to
    //end entity center
    irr::f32 length3D;

    //precalculate new direction vector which stands 90 deg onto the link direction vector,
    //so that it points always in a 90 degree angle into direction right of waypoint link
    //we use this during game to offset computer player paths sideways
    irr::core::vector3df offsetDirVec;

    //if a checkpoint is crossing this waypoint line
    //segment add a pointer to this checkpoint here
    //otherwise this pointer will be kept nullptr
    CheckPointInfoStruct* pntrCheckPoint = nullptr;

    //if there is a checkpoint crossing this WaypointLink
    //the following variable will hold the Distance
    //between Start of Waypoint location (also equal to pLineStruct->A
    //and the point where the checkpoint crosses the waypoint line
    irr::f32 distanceStartLinkToCheckpoint = 0.0f;

    //pointer to next WayPointLinkInfoStruct on the way
    //if a next element is existing
    //we can use this pointer for faster path search
    //during the game; if not existing stays nullptr
    WayPointLinkInfoStruct *pntrPathNextLink = nullptr;

    //if existing keep also direction vector of next element
    //so that we can adjust orientation of craft before we reach
    //next waypoint link
    //irr::core::vector3df PathNextLinkDirectionVec;

    //maximum possible offset waypoint link path shift
    //in positive direction (towards right side if looking in race
    //direction) at start entity
    irr::f32 maxOffsetShiftStart;

    //minimum possible offset waypoint link path shift (negative)
    //tells us who far we can offset to the left of waypoint link
    //at start entity
    irr::f32 minOffsetShiftStart;

    //maximum possible offset waypoint link path shift
    //in positive direction (towards right side if looking in race
    //direction) at end entity
    irr::f32 maxOffsetShiftEnd;

    //minimum possible offset waypoint link path shift (negative)
    //tells us who far we can offset to the left of waypoint link
    //at end entity
    irr::f32 minOffsetShiftEnd;
};

class Path {
public:
    Path(Race* parentRace, DrawDebug* pntrDrawDebug);

    EntityItem* FindNearestWayPointToLocation(irr::core::vector3df location);
    EntityItem* FindNearestWayPointToPlayer(Player* whichPlayer);

    std::vector<WayPointLinkInfoStruct*> FindWaypointLinksForWayPoint(EntityItem* wayPoint, bool whenStart, bool whenEnd,
                                                                      WayPointLinkInfoStruct* excludeWhichLink);
    irr::f32 CalculateDistanceFromWaypointLinkToNextCheckpoint(WayPointLinkInfoStruct* startWaypointLink);

    std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> PlayerDeriveClosestWaypointLink(std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>>
                                                            inputWayPointLinkVector);
    std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> PlayerFindCloseWaypointLinks(Player* player);

    std::vector<EntityItem*> FindAllWayPointsInArea(irr::core::vector3df location, irr::f32 radius);

    EntityItem* FindFirstWayPointAfterRaceStartPoint();

    std::vector<WayPointLinkInfoStruct*> FindPathToNextCheckPoint(Player *whichPlayer);
    bool ContinuePathSearchForPlayer(WayPointLinkInfoStruct *startLink,
                WayPointLinkInfoStruct* wayPointLinkNearPlayer, std::vector<WayPointLinkInfoStruct*> &resultPath,
                                     WayPointLinkInfoStruct* interruptLink, bool firstLink);

    WayPointLinkInfoStruct* PlayerFindClosestWaypointLinkHelper(irr::core::vector3df inputPosition,
                                                                irr::core::vector3df &projectedPlayerPosition);

    std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>
        FindClosestWayPointLinkToCollectible(Collectable* whichCollectable);

    void OffsetWayPointLinkCoordByOffset(irr::core::vector2df &coord2D,
                                           irr::core::vector3df &coord3D, WayPointLinkInfoStruct* waypoint, irr::f32 offset);

    //returns all waypoint links of a defined input path that come closer to a defined player than a distance of distanceLowLimit
    std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> WhereDoesPathComeCloseToPlayer
        (std::vector<WayPointLinkInfoStruct*> path, irr::f32 distanceLowLimit, Player* checkForWhichPlayer,
         std::vector<irr::f32> &whichDistanceVec);

    bool ProjectPositionAtWayPointLink(irr::core::vector3df position, WayPointLinkInfoStruct* link,
                                                            irr::core::vector3df &projPosition);

    bool DoesPathComeTooCloseToAnyOtherPlayer(std::vector<WayPointLinkInfoStruct*> path,
                                      std::vector<Player*> checkCollisionForWhichPlayers,
                                          std::vector<Player*> &playersInWay);

    //returns false if sani check of bezier input points is "crazy", means one of the points is "behind"
    //the player
    bool SaniCheckBezierInputPoints(irr::core::vector2df startPnt, irr::core::vector2df cntrlPnt,
                                    irr::core::vector2df endPnt, irr::core::vector2df raceDirection);

    //Runs a plausi check to see if two 3D coordinates for waypoint planing in front of the player (which position and
    //and forward direction is also taken into account) make sense according to their order, or if the two waypoint
    //locations actually needs to be swapped
    //returns true if order is correct, returns false if order is not correct
    bool SaniCheck3DPointOrder(irr::core::vector3df point1, irr::core::vector3df point2, Player* player);

    //Helper function which takes a WayPointLink, and returns StartEntity pointer in case it is also in
    //front of current player location, or if not returns the endEntity pointer
    //If advanceForOneEntity parameter is true, the function does not return the first match for entity, but instead returns
    //the next one on the path
    EntityItem* GetWayPointLinkEntityItemInFrontOfPlayer(WayPointLinkInfoStruct* whichWayPointLink, bool advanceForOneEntity, Player* whichPlayer);

    irr::core::vector2df WayPointLinkGetRaceDirection2D(WayPointLinkInfoStruct* whichWayPointLink);

    std::vector<WayPointLinkInfoStruct*> DeliverAllWayPointLinksThatLeadIntpSpecifiedToWayPointLink(
            WayPointLinkInfoStruct* inputWayPointLink);

    //is used within Recovery vehicle to be able to execute a workaround in case we do not know the location
    //anymore where to put a player back after its physics reset
    std::vector<WayPointLinkInfoStruct*> DeliverAllWayPointLinksThatLeadIntoPlayersNextExpectedCheckpoint(Player* player);

    //Returns nullptr if within the next 5 waypoint links the specified
    //charger type was not found, but with proper level design this
    //should never happen
    ChargingStation* GetChargingStationAhead(WayPointLinkInfoStruct* startAtWhichLink, irr::u8 chargerTypeToFind);

    //returns a vector containing all charging stations
    //which a certain defined waypoint link intersects
    std::vector<ChargingStation*> WhichChargingStationsDoesAWayPointLinkIntersect(WayPointLinkInfoStruct* whichLink);

private:
    Race* mRace = nullptr;
    DrawDebug* mDrawDebug = nullptr;

    void AddWayPointLinkToOccurenceList(std::vector<std::pair <irr::u8, WayPointLinkInfoStruct*>> &wayPointLinkOccurenceList,
                                                WayPointLinkInfoStruct* newWayPointLink);

    void AddWayPointLinkResultToVector(std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> &wayPointLinkList,
                                                WayPointLinkInfoStruct* newWayPointLink, irr::core::vector3df newProjectedPlayerPosition);

};

#endif // PATH_H
