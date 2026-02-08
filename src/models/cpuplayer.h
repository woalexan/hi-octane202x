/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef CPUPLAYER_H
#define CPUPLAYER_H

#include <irrlicht.h>
#include <cstdint>
#include <vector>
#include <list>

#define CMD_NOCMD 0
#define CMD_FLYTO_TARGETENTITY 1
#define CMD_FLYTO_TARGETPOSITION 2
#define CMD_FOLLOW_TARGETWAYPOINTLINK 3
#define CMD_FOLLOW_PATH 4
#define CMD_CHARGE_SHIELD 5
#define CMD_CHARGE_FUEL 6
#define CMD_CHARGE_AMMO 7
#define CMD_PICKUP_COLLECTABLE 8
#define CMD_GOTO_CHARGINGSTATION 9
#define CMD_EXIT_CHARGINGSTATION 10

#define CP_MISSION_WAITFORRACESTART 0
#define CP_MISSION_FINISHLAPS 1
                                                //restore commented out speed values later!
const irr::f32 CP_PLAYER_FAST_SPEED = 5.0f;      //13.0f;
const irr::f32 CP_PLAYER_SLOW_SPEED = 4.0f;     //9.0f;

const irr::f32 CP_PLAYER_DEACCEL_RATE_DEFAULT = 0.1f;
const irr::f32 CP_PLAYER_ACCEL_RATE_DEFAULT = 0.07f;
const irr::f32 CP_PLAYER_ACCELDEACCEL_RATE_CHARGING = 2.0f;

//computer player stuck detection logic values
const irr::f32 CP_PLAYER_STUCKDETECTION_MINDISTANCE_LIMIT = 2.0f;
const irr::f32 CP_PLAYER_STUCKDETECTION_THRESHOLD_SEC = 3.0f;
const irr::f32 CP_PLAYER_STUCKDETECTION_PERIOD_SEC = 0.5f;

#define CP_PLAYER_WAS_STUCKUNDEFINED 0
#define CP_PLAYER_WAS_STUCKLEFTSIDE 1
#define CP_PLAYER_WAS_STUCKRIGHTSIDE 2

const irr::f32 CP_PLAYER_ANGULAR_DAMPINGMAX = 1500.0f;  //2000.0f
const irr::f32 CP_PLAYER_ANGULAR_DAMPINGMIN = 30.0f; //50.0f
const irr::f32 CP_PLAYER_ANGULAR_DAMPING_ANGLEMIN = 2.0f;  //5.0f
const irr::f32 CP_PLAYER_ANGULAR_DAMPING_ANGLEMAX = 35.0f; //35.0f

const irr::f32 CP_BEZIER_RESOLUTION =  0.1f;

/************************
 * Forward declarations *
 ************************/

struct RayHitInfoStruct;
struct WayPointLinkInfoStruct;
struct RayHitTriangleInfoStruct;
class Collectable;
class ChargingStation;
struct ChargerStoppingRegionStruct;
class EntityItem;
struct ColorStruct;

typedef struct CpCommandEntry {
    uint8_t cmdType;
    EntityItem* targetEntity = nullptr;
    irr::core::vector3df targetPosition;
    WayPointLinkInfoStruct* targetWaypointLink = nullptr;
    Collectable* targetCollectible = nullptr;
    ChargingStation* targetChargingStation = nullptr;

    //if true this was a temporary dynamically
    //created waypoint link for a specific purpose
    //and not present in the original level data
    //will be deleted again after command was executed
    //by player craft
    bool WayPointLinkTemporary = false;
} CPCOMMANDENTRY;

class Player; //Forward declaration

class CpuPlayer {
public:
    CpuPlayer(Player* myParentPlayer);
    ~CpuPlayer();

    void RunPlayerLogic(irr::f32 deltaTime);
    void WasDestroyed();
    void CpTakeOverHuman();
    void CpStop();
    void FreedFromRecoveryVehicleAgain();

    bool IsCurrentlyStuck();

    void SetupForRaceStart(irr::core::vector3df startPos);
    void StartSignalShowsGreen();

    //method call for the mechanism to keep copy of currClosestWayPointLink variable
    //in this class up to date all the time
    //we will also do this for human players, so that we do not get trouble when human player
    //is switched over to computer player control at the end of the race
    void SetCurrClosestWayPointLink(std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> newClosestWayPointLink);

    //variables for debugging purposes of cpu craft control
    //etc
    irr::core::vector3df debugPathPnt1;
    irr::core::vector3df debugPathPnt2;
    irr::core::vector3df debugPathPnt3;

    irr::f32 mAngleError;
    irr::f32 dbgDistError;

    std::vector<WayPointLinkInfoStruct*> mFailedLinks;

    irr::f32 mDbgRotationalFrictionVal;

    irr::u32 mCPTrackMovementNoClearClosestLinkCnter = 0;
    irr::u32 mCPTrackMovementLostProgressCnter = 0;

    irr::u32 updatePathCnter = 0;

    irr::f32 mDbgForceAngle;
    irr::f32 mDbgFoceDistance;
    irr::f32 mDbgAngleVelocityCraftX;
    irr::f32 mDbgAngleVelocityCraftZ;

    irr::f32 mCpFollowedWayPointLinkCurrentSpaceRightSide;
    irr::f32 mCpFollowedWayPointLinkCurrentSpaceLeftSide;

    irr::f32 dbgDistVec = 0.0f;

    void DebugDrawCurrentSegment();
    void DebugDrawPathHistory();

    irr::core::vector3df mLocationChargingStall;

    irr::f32 mLocalOffset = 0.0f;

private:
    Player* mParentPlayer = nullptr;

    //the next 2 values (currently closest waypoint link this player craft is
    //next too) belongs more into the player class, and I want to keep it there
    //but because this value/variable is so often used during computer control
    //calculations I do not want for every variable access to grab the value from
    //the player class due to performance reasons
    //therefore I decided to keep a copy of this variable here also in this class,
    //which is kept update by a mechanism in the player class itself
    std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> currClosestWayPointLink;
    //the following variable is only valid (and must only be used)
    //while currClosestWayPointLink is current != nullptr
    irr::core::vector3df projPlayerPositionClosestWayPointLink;

    irr::f32 mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;
    irr::f32 mCpCurrentAccelRate = CP_PLAYER_ACCEL_RATE_DEFAULT;

    std::list<CPCOMMANDENTRY*>* cmdList = nullptr;
    void AddCommand(uint8_t cmdType, irr::core::vector3df targetPosition);
    void AddCommand(uint8_t cmdType, EntityItem* targetEntity);
    void AddCommand(uint8_t cmdType, WayPointLinkInfoStruct* targetWayPointLink);
    void AddCommand(uint8_t cmdType);
    void AddCommand(uint8_t cmdType, Collectable* whichCollectable);
    void AddCommand(uint8_t cmdType, ChargingStation* whichChargingStation, ChargerStoppingRegionStruct* whichStall);

    void CurrentCommandFinished();
    void CleanUpCommandList();

    void CheckAndRemoveNoCommand();
    void CpWaitForChargingFinished();
    void RemoveAllPendingCommands();

    CPCOMMANDENTRY* PullNextCommandFromCmdList();
    CPCOMMANDENTRY* CreateNoCommand();

    irr::u32 CpCurrMissionState = CP_MISSION_WAITFORRACESTART;

    CPCOMMANDENTRY* currCommand = nullptr;

    bool DoIWantToChargeShield();
    bool DoIWantToChargeFuel();
    bool DoIWantToChargeAmmo();

    ChargerStoppingRegionStruct* mAssignedChargingStall = nullptr;
    ChargingStation* mAssignedChargingStation = nullptr;

    bool mReachedChargingStation = false;
    bool mReachedChargingStall = false;
    bool mSetupPathToChargingStation = false;
    bool mSetupPathToChargingStall = false;
    bool mSetupPathToExitChargingStation = false;

    void CpWaitForChargingStallReached();
    void CpCommandPlayerToChargingStall(ChargingStation* whichChargingStation, ChargerStoppingRegionStruct* whichStall);
    void CpCommandPlayerToExitChargingStall(ChargingStation* whichChargingStation);

    void FlyTowardsEntityRunComputerPlayerLogic(CPCOMMANDENTRY* currCommand);
    WayPointLinkInfoStruct* CpPlayerWayPointLinkSelectionLogic(std::vector<WayPointLinkInfoStruct*> availLinks);

    void CpPlayerCollectableSelectionLogic();
    void CpPlayerHandleAttack();

    void FollowPathDefineNextSegment(WayPointLinkInfoStruct* nextLink, irr::f32 startOffsetWay, bool updatePathReachedEndWayPointLink = false);
    void DefineSegmentTowardsWayPointLink(WayPointLinkInfoStruct* whichLink);

    void CreateNewSegmentPath(irr::core::vector3df start3DPnt, irr::core::vector3df cntrl3DPnt, irr::core::vector3df end3DPnt, bool goThrough);

    irr::f32 computerPlayerTargetSpeed = 0.0f;
    irr::f32 computerPlayerCurrentSpeed = 0.0f;
    irr::f32 mTargetSpeedAdjust = 0.0f;

    irr::u32 mCurrentPathSegNrSegments;
    irr::u32 mCurrentPathSegCurrSegmentNr;

    WayPointLinkInfoStruct* mCpFollowThisWayPointLink = nullptr;

    irr::s32 mDbgCpAvailWaypointNr = 0;
    std::vector<WayPointLinkInfoStruct*> mCpAvailWayPointLinks;
    irr::s32 mDbgCpAvailWayPointLinksNr = 0;

    void CPForceController(irr::f32 deltaTime);

    void CPForward(irr::f32 deltaTime);
    void CPBackward(irr::f32 deltaTime);
    void CPTrackMovement();

    void ReachedEndCurrentFollowingSegments();

    WayPointLinkInfoStruct* cPCurrentFollowSeg = nullptr;

    std::vector<WayPointLinkInfoStruct*> mPathHistoryVec;
    std::vector<WayPointLinkInfoStruct*> mCurrentPathSeg;
    std::vector<WayPointLinkInfoStruct*> mCurrentPathSegSortedOutReverse;

    //computer player stuck detection logic
    //just a workaround to save computer players
    //that can not move anymore, for whatever reason
    irr::core::vector3df mCpPlayerLastPosition;
    irr::f32 mCpPlayerTimeNotMovedSeconds;
    bool mCpPlayerCurrentlyStuck = false;
    irr::u8 mCpPlayerStuckAtSide = CP_PLAYER_WAS_STUCKUNDEFINED;

    irr::f32 mExecuteCpStuckDetectionTimeCounter = 0.0f;
    irr::f32 mCpAbsCheckObstacleTimerCounter = 0.0f;

    void CpStuckDetection(irr::f32 deltaTime);

    void WorkaroundResetCurrentPath();
    void CpCheckCurrentPathForObstacles();

    void PickupCollectableDefineNextSegment(Collectable* whichCollectable);

    //the current set global offset to the path
    irr::f32 mCpCurrPathOffset = 0.0f;

    void CpDefineNextAction();
    void CpAddCommandTowardsNextCheckpoint();
    void CpHandleSeperation(irr::f32 deltaTime);

    bool mHandleSeperation = true;

    Collectable* mCpTargetCollectableToPickUp = nullptr;
    WayPointLinkInfoStruct* mCpWayPointLinkClosestToCollectable = nullptr;
    bool DoISeeACertainCollectable(Collectable* whichItem);

    ColorStruct* currDbgColor = nullptr;
    void AdvanceDbgColor();

    irr::f32 mLastCraftDistToWaypointLink = 0.0f;
    irr::f32 mCurrentCraftDistToWaypointLink = 0.0f;

    irr::core::vector3df projPlayerPositionFollowSeg;

    //If true enables computer player stuck detection
    //turn off for testing computer player movement
    //performance and stabilitiy testing to keep computer
    //players stuck; Set true for final release to handle
    //random cases where computer players really get stuck, so that
    //races can for sure finish
    bool CpEnableStuckDetection = false;

    bool computerPlayersAttack = false;

    bool mLockedToWayPoints = false;
};

#endif // CPUPLAYER_H

