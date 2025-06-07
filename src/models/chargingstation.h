/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef CHARGINGSTATION_H
#define CHARGINGSTATION_H

#include <irrlicht.h>
#include "../race.h"
#include "../models/player.h"
#include "../utils/path.h"

//defines a rotation in clockwise direction
//for the exit of the charging station
//as far as I am aware will only be used one single time in
//level 6 of the original game (for the annyoing weirdly placed ammo charger :( )
#define ROT_EXIT_0DEG 0
#define ROT_EXIT_90DEG 1
#define ROT_EXIT_180DEG 2
#define ROT_EXIT_270DEG 3

//defines the minimum side len that is necessary for every
//charging station stand, is used to decide how many stands are
//possible during creation of a charging station
const irr::f32 DEF_CHARGING_STATION_STAND_MINSIDELEN = 1.0f;

class Race; //Forward declaration
class Player; //Forward declaration
struct WayPointLinkInfoStruct; //Forward declaration

//struct which holds information about a possible
//position for stopping inside a charger region
//that can be utilized by a player craft
struct ChargerStoppingRegionStruct {
    //the relative offset from the waypoint link going through
    //the charger, negative means a location left of the link
    //positive means right of the link
    irr::f32 offsetFromWayPointLink = 0.0f;

    //create a dynamical (temporary) entityitem
    //for this stall, so that we can route computer
    //players to it
    EntityItem* entityItem = nullptr;

    Player* reservedForPlayerPntr = nullptr;

    //only used for debugging drawing the stalls
    irr::core::vector3df dbgDrawVertex1;
    irr::core::vector3df dbgDrawVertex2;
    irr::core::vector3df dbgDrawVertex3;
    irr::core::vector3df dbgDrawVertex4;
};

class ChargingStation {
public:
    ChargingStation(irr::scene::ISceneManager* smgr, Race* race, MapTileRegionStruct* regionStruct,
                    std::vector<WayPointLinkInfoStruct*> *allRaceWayPoints);
    ~ChargingStation();

    void Update();

    //Lets also assign a SceneNode (Box)
    //that spans this region, could be used
    //for different things later
    //pntr to charger region Irrlicht sceneNode
    irr::scene::IMeshSceneNode* mSceneNode = nullptr;

    irr::u8 GetChargingStationType();
    std::vector<irr::core::vector3df> GetStallLocations();

    //debugging draw function for game testing
    void DebugDraw();

    irr::core::vector3df middlePos;
    irr::core::vector3df startPos;

    irr::core::vector3df corner1;
    irr::core::vector3df corner2;
    irr::core::vector3df corner3;
    irr::core::vector3df corner4;

    //Returns true if request is granted, False otherwise
    //if success is granted, pointer to stall that was reserved for this player
    //is returned in variable reservedStall
    bool RequestCharging(Player *whichPlayer, ChargerStoppingRegionStruct *& reservedStall);
    bool ReachedEntryOfChargingStation(WayPointLinkInfoStruct* currWayPointLink);
    bool PassedExitOfChargingStation(WayPointLinkInfoStruct* currWayPointLink);

    void ChargingFinished(Player *whichPlayer);

    EntityItem* enterEntityItem = nullptr;
    EntityItem* enterHelperEntityItem = nullptr;

    EntityItem* exitEntityItem = nullptr;

    WayPointLinkInfoStruct* exitWayPointLink = nullptr;

private:
    irr::scene::ISceneManager* mSmgr = nullptr;

    Race *mRace = nullptr;

    //the map region data struct that is behind
    //this charging station
    MapTileRegionStruct* mRegionStruct = nullptr;

    //we want to keep a list of all WayPointLinks that
    //intersect with this charging station
    //first we need this info to be able to know in which
    //direction the craft will pass us during charging
    std::vector<WayPointLinkInfoStruct*> mIntersectingWayPointLinksVec;

    //a vector of possible craft stopping regions
    //inside this region (charger)
    std::vector<ChargerStoppingRegionStruct*> mStandVec;

    //pntr to charger region Irrlich Mesh
    irr::scene::SMesh* mMesh = nullptr;

    //creates a Mesh/Box around defined charger map region
    //could be used later for different things, to control
    //computer player craft for example
    void createRegionMeshData();

    //create my available stands that allows
    //player crafts to "stand" during charging
    void createChargingStands();

    void DetermineOrientation();

    void DetectExitWayPointLink();

    //to store number of available stalls
    irr::u8 nrStallsWidth = 0;
    irr::u8 nrStallsLength = 0;

    irr::u8 nrStalls = 0;

    //internal coordinate necessary to create temporary
    //entityItem for entering the charging station for
    //computer players
    irr::core::vector3df enterEntityItemLocation;

    irr::core::vector3df enterHelperEntityItemLocation;

    //internal coordinate necessary to create temporary
    //entityItem for exiting the charging station for
    //computer players
    irr::core::vector3df exitEntityItemLocation;

    //Some predefined charging stations in the maps extend outside
    //of the usable race track, this would lead to stalls that are not
    //reachable by craft, we need to figure out which area is usable first,
    //and remove the other areas
    //store this result here for later usage
    irr::u16 usableTileXmin;
    irr::u16 usableTileXmax;
    irr::u16 usableTileYmin;
    irr::u16 usableTileYmax;

    //define the width of the charging station as the width
    //perpendicular to the direction the craft will fly through
    //the charging station
    //define the length of the charging station as the length
    //in direction of the Waypoint link
    irr::f32 width;
    irr::f32 length;

    //returns true if usable area was succesfully found
    //returns false in case of error
    bool IdentifyUsableArea();

    //if true the craft fly along the charging station
    //in 3D worlds X axis direction;
    //if false the craft fly along the 3D worlds Z axis
    //direction
    bool mWidthInZDir = false;

    //also used to determine the orientation of this
    //charging station
    irr::f32 signInX;
    irr::f32 signInZ;

    //If there is no free stall right now for
    //service returns nullptr
    ChargerStoppingRegionStruct* GetNextFreeStall();

    irr::u8 mExitRotation = ROT_EXIT_0DEG;
};

#endif // CHARGINGSTATION_H
