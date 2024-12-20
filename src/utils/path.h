/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PATH_H
#define PATH_H

#include "../race.h"
#include "../draw/drawdebug.h"

struct CheckPointInfoStruct {
    //pntr to checkpoint Irrlicht sceneNode
    irr::scene::IMeshSceneNode* SceneNode;
    //pntr to checkpoint Irrlich Mesh
    irr::scene::SMesh* Mesh;

    //value for checkpoint, is defined
    //within level file with increasing
    //integer values
    irr::s32 value = 0;

    EntityItem* pEntity;
    LineStruct* pLineStruct;

    //if we want to figure out if a player
    //craft flies in normal race direction
    //or reverse through the checkpoint
    //we want to figure out a forward
    //race direction pointer
    irr::core::vector3df RaceDirectionVec;
};

struct WayPointLinkInfoStruct {
    EntityItem* pStartEntity;
    EntityItem* pEndEntity;

    //3D line that links two waypoints together
    LineStruct* pLineStruct;

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
    //otherwise this pointer will be kept NULL
    CheckPointInfoStruct* pntrCheckPoint = NULL;

    //if there is a checkpoint crossing this WaypointLink
    //the following variable will hold the Distance
    //between Start of Waypoint location (also equal to pLineStruct->A
    //and the point where the checkpoint crosses the waypoint line
    irr::f32 distanceStartLinkToCheckpoint = 0.0f;

    //pointer to next WayPointLinkInfoStruct on the way
    //if a next element is existing
    //we can use this pointer for faster path search
    //during the game; if not existing stays NULL
    WayPointLinkInfoStruct *pntrPathNextLink = NULL;

    //if existing keep also direction vector of next element
    //so that we can adjust orientation of craft before we reach
    //next waypoint link
    //irr::core::vector3df PathNextLinkDirectionVec;

    //maximum possible offset waypoint link path shift
    //in positive direction (towards right side if looking in race
    //direction)
    irr::f32 maxOffsetShift;

    //minimum possible offset waypoint link path shift (negative)
    //tells us who far we can offset to the left of waypoint link
    irr::f32 minOffsetShift;
};

class Race;       //Forward declaration
class DrawDebug;  //Forward declaration
class Player;     //Forward declaration
class EntityItem; //Forward declaration

class Path {
public:
    Path(Race* parentRace, DrawDebug* pntrDrawDebug);

    EntityItem* FindNearestWayPointToLocation(irr::core::vector3df location);
    EntityItem* FindNearestWayPointToPlayer(Player* whichPlayer);

    std::vector<WayPointLinkInfoStruct*> FindWaypointLinksForWayPoint(EntityItem* wayPoint, bool whenStart, bool whenEnd);
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
        (std::vector<WayPointLinkInfoStruct*> path, irr::f32 distanceLowLimit, Player* checkForWhichPlayer);

    bool ProjectPositionAtWayPointLink(irr::core::vector3df position, WayPointLinkInfoStruct* link,
                                                            irr::core::vector3df &projPosition);

    bool DoesPathComeTooCloseToAnyOtherPlayer(std::vector<WayPointLinkInfoStruct*> path,
                                      Player* pathOfWhichPlayer);

private:
    Race* mRace;
    DrawDebug* mDrawDebug;

    void AddWayPointLinkToOccurenceList(std::vector<std::pair <irr::u8, WayPointLinkInfoStruct*>> &wayPointLinkOccurenceList,
                                                WayPointLinkInfoStruct* newWayPointLink);

    void AddWayPointLinkResultToVector(std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> &wayPointLinkList,
                                                WayPointLinkInfoStruct* newWayPointLink, irr::core::vector3df newProjectedPlayerPosition);

};

#endif // PATH_H
