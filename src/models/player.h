/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PLAYER_H
#define PLAYER_H

#include <irrlicht/irrlicht.h>
#include "levelterrain.h"
#include <iostream>
#include "../definitions.h"
#include "irrlicht/irrMath.h"
#include "../utils/physics.h"
#include "../draw/hud.h"
#include "collectable.h"
#include "../resources/entityitem.h"
#include "../race.h"
#include "SFML/Audio.hpp"
#include <queue>
#include "particle.h"
#include "mgun.h"
#include "missile.h"
#include "levelterrain.h"

//The target hover height of the craft above the race track
const irr::f32 HOVER_HEIGHT = 0.6f;

const irr::f32 MAX_PLAYER_SPEED = 17.0f;

const irr::f32 CRAFT_SIDEWAYS_BRAKING = 2.0f;

#define CRAFT_AIRFRICTION_NOTURBO 0.3f
#define CRAFT_AIRFRICTION_TURBO 0.2f

#define CRAFT_NOLEANING 0
#define CRAFT_LEANINGLEFT 1
#define CRAFT_LEANINGRIGHT 2

#define CP_TURN_NOTURN 0
#define CP_TURN_LEFT 1
#define CP_TURN_RIGHT 2

#define CMD_NOCMD 0
#define CMD_FLYTO_TARGETENTITY 1
#define CMD_FLYTO_TARGETPOSITION 2
#define CMD_FOLLOW_TARGETWAYPOINTLINK 3

#define STATE_HMAP_COLL_IDLE 0
#define STATE_HMAP_COLL_WATCH 1
#define STATE_HMAP_COLL_RESOLVE 2
#define STATE_HMAP_COLL_NOINTERSECTPNT 3
#define STATE_HMAP_COLL_FAILED 4

#define STATE_PLAYER_BEFORESTART 0
#define STATE_PLAYER_RACING 1
#define STATE_PLAYER_EMPTYFUEL 2
#define STATE_PLAYER_BROKEN 3
#define STATE_PLAYER_FINISHED 4
#define STATE_PLAYER_GRABEDBYRECOVERYVEHICLE 5

struct WayPointLinkInfoStruct; //Forward declaration
struct RayHitTriangleInfoStruct; //Forward declaration

typedef struct {
    uint8_t cmdType;
    EntityItem* targetEntity = NULL;
    irr::core::vector3df* targetPosition = NULL;
    WayPointLinkInfoStruct* targetWaypointLink = NULL;
} CPCOMMANDENTRY;

typedef struct {
    unsigned int lapNr;
    irr::u32 lapTimeMultiple100mSec;
} LAPTIMEENTRY;

typedef struct {
    irr::f32 speed;
    irr::f32 speedMax = MAX_PLAYER_SPEED;

    irr::f32 shieldVal;
    irr::f32 gasolineVal;
    irr::f32 ammoVal;
    irr::f32 shieldMax = 100.0;
    irr::f32 gasolineMax = 100.0;
    irr::f32 ammoMax = 100.0;
    irr::f32 mgHeatVal = 0.0;
    irr::f32 mgHeatMax = 100.0;

    irr::f32 boosterVal;
    irr::f32 boosterMax = 100.0;

    irr::f32 throttleVal;
    irr::f32 throttleMax = 100.0;

    //current equipment upgrade levels

    //minigun level, goes from 0 (no upgrade, to 3 upgrades)
    int currMinigunUpgradeLevel = 0;

    //rocket level, goes from 0 (no upgrade, to 3 upgrades)
    int currRocketUpgradeLevel = 0;

    //booster level, goes from 0 (no upgrade, to 3 upgrades)
    int currBoosterUpgradeLevel = 0;

    irr::u32 currLapTimeMultiple100mSec = 0;
    irr::f32 currLapTimeExact;
    std::vector <LAPTIMEENTRY> lapTimeList;
    LAPTIMEENTRY lastLap;
    LAPTIMEENTRY LapBeforeLastLap;

    int currRacePlayerPos = 0;
    int overallPlayerNumber = 0;

    int currLapNumber = 1;

    int raceNumberLaps = 6;

    int currKillCount = 0;

    char name[50];

    bool mPlayerCanMove;
    bool mPlayerCanShoot;

    irr::u32 mPlayerCurrentState;
} PLAYERSTATS;

//struct for one heightmap collision "sensor" element
//which consists of 2 points for terrain stepness measurement
typedef struct {
    //local coordinate for the craft 3D Model of the sensor element point
    irr::core::vector3df localPnt1;
    irr::core::vector3df localPnt2;

    //latest world coordinate for the sensor element point
    irr::core::vector3df wCoordPnt1;
    irr::core::vector3df wCoordPnt2;

    //latest terrain cell coordinate for the sensor
    //element point
    irr::core::vector2di cellPnt1;
    irr::core::vector2di cellPnt2;

    //current plane for intersection
    irr::core::vector3df planePnt1;
    irr::core::vector3df planePnt2;

    //the current intersectionPnt
    irr::core::vector3df intersectionPnt;

    //current measured stepness;
    irr::f32 stepness;

    //the current distance to the
    //detected collision point
    irr::f32 distance;

    //if the craft is close to a high slope (collision) at this sensor element
    //stop using this point for craft leaning (height) control
    //if we would not do this then the craft would be able to climb
    //up the slope, and the collision detection does not stop us worst case
    bool deactivateHeightControl;

    irr::u32 collCnt = 0;

    irr::u32 currState;

    //for debugging print purposes
    char sensorName[10];
} HMAPCOLLSENSOR;

typedef struct {
    HMAPCOLLSENSOR* front;
    HMAPCOLLSENSOR* frontRight45deg;
    HMAPCOLLSENSOR* frontLeft45deg;
    HMAPCOLLSENSOR* right;
    HMAPCOLLSENSOR* left;
    HMAPCOLLSENSOR* backRight45deg;
    HMAPCOLLSENSOR* backLeft45deg;
    HMAPCOLLSENSOR* back;
} HMAPCOLLSTRUCT;

class HUD; //Forward declaration
class Race; //Forward declaration
class SmokeTrail; //Forward declaration
class DustBelowCraft; //Forward declaration
class MachineGun; //Forward declaration
class MissileLauncher; //Forward declaration
class PhysicsObject; //Forward declaration
class Recovery; //Forward declaration

class Player {
public:
    //Player(LevelTerrain *Terrain, std::string model, irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt, irr::scene::ISceneManager* smgr);
    Player(Race* race, std::string model, irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt, irr::scene::ISceneManager* smgr,
           bool humanPlayer);
    ~Player();

    HMAPCOLLSENSOR* cameraSensor;
    HMAPCOLLSENSOR* cameraSensor2;
    HMAPCOLLSENSOR* cameraSensor3;
    HMAPCOLLSENSOR* cameraSensor4;
    HMAPCOLLSENSOR* cameraSensor5;
    HMAPCOLLSENSOR* cameraSensor6;
    HMAPCOLLSENSOR* cameraSensor7;

    void SetPlayerObject(PhysicsObject* phObjPtr);
    void DamageGlas();

    void GetHeightRaceTrackBelowCraft(irr::f32 &front, irr::f32 &back, irr::f32 &left, irr::f32 &right);
    irr::f32 currHeightFront;
    irr::f32 currHeightBack;
    irr::f32 currHeightLeft;
    irr::f32 currHeightRight;

    irr::f32 dbgdeltaAngleTurningLeft;
    irr::f32 dbgdeltaAngleTurningRight;
    irr::f32 dbgForce;

    irr::f32 mCraftDistanceAvailRight;
    irr::f32 mCraftDistanceAvailLeft;
    irr::f32 mCraftDistanceAvailBack;
    irr::f32 mCraftDistanceAvailFront;

    irr::f32 debugMaxStep;
    MapEntry* currTileBelowPlayer;

    irr::core::vector3df dbgcollPlanePos1;
    irr::core::vector3df dbgcollPlanePos2;
    irr::core::vector3df dbgcollResolutionDirVec;
    irr::f32 dbgDistance;

    bool firstNoKeyPressed = false;

    //True means humanPlayer
    //False is for computer player
    bool mHumanPlayer;

    void Forward();
    void Backward();
    void Left();
    void Right();
    void NoTurningKeyPressed();

    void CollectedCollectable(Collectable* whichCollectable);

    void StartPlayingWarningSound();
    void StopPlayingWarningSound();

    std::list<CPCOMMANDENTRY*>* cmdList;
    void AddCommand(uint8_t cmdType, EntityItem* targetEntity);
    void AddCommand(uint8_t cmdType, irr::core::vector3df* targetLocation);
    void AddCommand(uint8_t cmdType, WayPointLinkInfoStruct* targetWayPointLink);

    //void buttonL();
    //void buttonR();

    void Update(irr::f32 frameDeltaTime);
    void AddTextureID(irr::s32 newTexId);

    void SetName(char* playerName);

    //returns TRUE if player reached below/equal 0 health (therefore if
    //player died); otherwise false is returned
    bool Damage(irr::f32 damage);

    void ShowPlayerBigGreenHudText(char* text, irr::f32 timeDurationShowTextSec);

    irr::f32 currentSideForce = 20.0f;

    irr::core::vector3d<irr::f32> LocalTopLookingCamPosPnt;
    irr::core::vector3d<irr::f32> LocalTopLookingCamTargetPnt;

    irr::core::vector3d<irr::f32> Local1stPersonCamPosPnt;
    irr::core::vector3d<irr::f32> Local1stPersonCamTargetPnt;

    bool mFirstPlayerCam = true;

    //a local coordinate system point defined on the players craft, for height control
    irr::core::vector3d<irr::f32> LocalCraftFrontPnt;
    irr::core::vector3d<irr::f32> LocalCraftBackPnt;
    irr::core::vector3d<irr::f32> LocalCraftLeftPnt;
    irr::core::vector3d<irr::f32> LocalCraftRightPnt;

    irr::core::vector3d<irr::f32> LocalCraftOrigin;

    irr::core::vector3d<irr::f32> LocalCraftFrontPnt2;

    //local position on the craft where smoke pours out if
    //player is damaged
    irr::core::vector3d<irr::f32> LocalCraftSmokePnt;

    //local position on the craft where dust clouds are created when
    //hovering above dusty tiles next to the race track
    irr::core::vector3d<irr::f32> LocalCraftDustPnt;

    //mainly for debugging
    irr::core::vector3d<irr::f32> WorldCoordCraftFrontPnt;
    irr::core::vector3d<irr::f32> WorldCoordCraftBackPnt;
    irr::core::vector3d<irr::f32> WorldCoordCraftLeftPnt;
    irr::core::vector3d<irr::f32> WorldCoordCraftRightPnt;

    irr::core::vector3d<irr::f32> WorldCoordCraftFrontPnt2;

    //world coordinate position on the craft where smoke pours out if
    //player is damaged
    irr::core::vector3d<irr::f32> WorldCoordCraftSmokePnt;

    //world coordinate position on the craft where dust clouds are created when
    //hovering above dusty tiles next to the race track
    irr::core::vector3d<irr::f32> WorldCraftDustPnt;

    irr::core::vector3df LocalCraftAboveCOGStabilizationPoint;
    irr::core::vector3df WorldCoordCraftAboveCOGStabilizationPoint;

    irr::core::vector3d<irr::f32> craftForwardDirVec;

    irr::f32 DbgShipUpAngle;

    irr::s32 currTextID;
    vector<irr::s32> textureIDlist;

    bool mPlayerModelSmoking = false;
    bool mLastPlayerModelSmoking = false;

    //if we currently target another player for attacking
    //this pointer points to the other target player
    //no target if NULL
    Player* mTargetPlayer = NULL;
    Player* mLastTargetPlayer = NULL;

    //value from 22 downto 0 which describes how far we are
    //with the missle lock, 0 means lock achieved, 22 means
    //just began with missle targeting
    irr::u32 mTargetMissleLockProgr = 22;

    //if we have a missile lock as well this bool
    //is true. When this happens in the original game
    //the symbol becomes red
    bool mTargetMissleLock = false;

    void SetTarget(Player* newTarget);

    //irr::core::vector3d<irr::f32> Position;  //current player position
    //irr::core::vector3d<irr::f32> FrontDir;  //is direction vector which points forward of the craft
    //irr::core::vector3d<irr::f32> SideDir; //direction vector to right side of craft

    /*
    irr::core::quaternion WheelSteerDir;
    irr::core::vector3df WorldCoordWheelDir;
    irr::core::vector3df LocalCoordWheelDir;
    irr::core::vector3df targetSteerDir;
    irr::f32 targetSteerAngle;
    irr::core::vector3df WorldCoordCraftTravelDir;*/

    irr::core::vector3df craftUpwardsVec;
    irr::core::vector3df craftSidewaysToRightVec;

    uint8_t cPCurrentTurnMode = CP_TURN_NOTURN;
    irr::f32 mCurrentCraftOrientationAngle;
    irr::f32 mLastCurrentCraftOrientationAngle;
    irr::f32 cPStartTurnAngle;
    irr::f32 cpEndTurnAngle;
    irr::f32 cPAngleTurned;
    irr::f32 cPTargetTurnAngle;
    irr::core::vector3df cPStartTurnOrientation;

    void CpTriggerTurn(uint8_t newTurnMode, irr::f32 turnAngle);
    void CpInterruptTurn();

    PhysicsObject* phobj;

    irr::scene::IMeshSceneNode* Player_node;

    PLAYERSTATS *mPlayerStats;

    irr::f32 terrainTiltCraftLeftRightDeg;

    //next checkpoints value we need to reach for
    //correct race progress
    irr::s32 nextCheckPointValue = 0;

    //stores the last crossed checkpoints; we need this
    //to differentiate first crossing of finish line at the start of the race
    //with the end of following laps
    irr::s32 lastCrossedCheckPointValue = 0;

    void CrossedCheckPoint(irr::s32 valueCrossedCheckPoint, irr::s32 numberOfCheckpoints);

    Race *mRace;

    //is derived in function GetHeightRaceTrackBelowCraft
    //if not available is NULL
    MapEntry* mNextNeighboringTileInFrontOfMe;
    MapEntry* mNextNeighboringTileRightOfMe;
    MapEntry* mNextNeighboringTileLeftOfMe;
    MapEntry* mNextNeighboringTileBehindOfMe;

    //debug height calculation variables
    LineStruct debug;
    std::list<LineStruct*> debug_player_heightcalc_lines;

    irr::f32 TargetHeight;

    irr::f32 testAngle;

    bool playerCurrentlyDeath = false;

    bool ProjectOnWayPoint(WayPointLinkInfoStruct* projOnWayPointLink, irr::core::vector3df craftCoord,
                           irr::core::vector3df *projPlayerPosition,
                           irr::core::vector3d<irr::f32>* distanceVec, irr::f32 *remainingDistanceToTravel);

    WayPointLinkInfoStruct* currClosestWayPointLink = NULL;
    WayPointLinkInfoStruct* computerCurrFollowWayPointLink = NULL;

    void SetCurrClosestWayPointLink(WayPointLinkInfoStruct* newClosestWayPointLink);
    irr::core::vector3df DeriveCurrentDirectionVector(WayPointLinkInfoStruct *currentWayPointLine, irr::f32 progressCurrWayPoint);

    irr::f32 GetHoverHeight();


    //computer player stuff
    irr::f32 computerPlayerTargetSpeed = 1.3f; //3.0f; //10.0f

    irr::f32 cPTargetRelativeAngle = 0.0f; //0.0f means go straight in parallel to current followed
                                                    //waypoint link
    irr::f32 cPCurrRelativeAngle;
    irr::f32 cPCurrRelativeAngleErr;

    irr::f32 cPTargetDistance = 10.0f;
    irr::f32 cPcurrDistanceErr;
    irr::f32 cPcurrDistance;

    irr::f32 CPcurrWaypointLinkFinishedPerc = 0.0f;

    bool cPCurrCorrectingPos = false;
    irr::f32 cPCorrectingPosErr;

    irr::f32 computerPlayerCurrShipWayPointLinkSide;
    irr::f32 computerPlayerCurrDistanceFromWayPointLinkFront;
    irr::f32 computerPlayerCurrDistanceFromWayPointLinkBack;
    irr::f32 computerPlayerCurrDistanceFromWayPointLinkAvg;
    irr::f32 computerPlayerCurrSteerForce = 10.0f;
    irr::u8 steerLeftPanic = 0;
    irr::u8 steerRightPanic = 0;

    irr::s32 computerPlayerCurrSteering = 0; //0 means straight (no steering applied, forward direction)
                                             //positive numbers mean left turn, negative numbers mean right turn applied

    void RunComputerPlayerLogic();
    void FollowWayPointLink();
    void FlyTowardsEntityRunComputerPlayerLogic(EntityItem* targetEntity);
    CPCOMMANDENTRY* PullNextCommandFromCmdList();
    CPCOMMANDENTRY* CreateNoCommand();
    void CurrentCommandFinished();
    void CleanUpCommandList();

    CPCOMMANDENTRY* currCommand = NULL;

    EntityItem* cPTargetEntity = NULL;

    //the following variable is only valid (and must only be used)
    //while currClosestWayPointLink is current != NULL
    irr::core::vector3df projPlayerPositionClosestWayPointLink;

    //the following variable is used for race position calculation
    irr::f32 remainingDistanceToNextCheckPoint = 0.0f;

    //the current calculate angle between craft upwards direction vector
    //and the world Y-axis. Is used to draw the sky background image realistic
    irr::f32 currPlayerCraftLeaningAngleDeg = 0.0f;
    irr::u8 currPlayerCraftLeaningOrientation = CRAFT_NOLEANING;

    irr::f32 targetPlayerCraftLeaningAngle = 0.0f;
    irr::u8 targetPlayerCraftLeaningOrientation = CRAFT_NOLEANING;

    void CPForceController();

    void FinishedLap();

    irr::core::vector3df pos_in_worldspace_frontPos;
    irr::core::vector3df pos_in_worldspace_backPos;
    irr::core::vector3df pos_in_worldspace_leftPos;
    irr::core::vector3df pos_in_worldspace_rightPos;

    irr::f32 DbgCurrRaceTrackHeightFront;
    irr::f32 DbgCurrRaceTrackHeightBack;
    irr::f32 DbgCurrRaceTrackHeightLeft;
    irr::f32 DbgCurrRaceTrackHeightRight;

    void DebugCraftLocalFeatureCoordinates();

    void SetMyHUD(HUD* pntrHUD);
    HUD* GetMyHUD();

    uint8_t cPCntrlCycle = 0;
    uint8_t cPProbTurnLeft = 0;
    uint8_t cPProbTurnRight = 0;

    bool mBoosterActive = false;
    bool mLastBoosterActive = false;
    sf::Sound* TurboSound;

    void IsSpaceDown(bool down);
    void MaxTurboReached();

    bool mMaxTurboActive = false;
    bool mLastMaxTurboActive = false;

    void Collided();
    void AfterPhysicsUpdate();
    sf::Sound* CollisionSound = NULL;

    MachineGun* mMGun;
    MissileLauncher* mMissileLauncher;

    //Get current weapon shooting target for this player
    //Returns true if there was a target found, False otherwise
    bool GetWeaponTarget(RayHitTriangleInfoStruct &shotTarget);

    HMAPCOLLSTRUCT mHMapCollPntData;

    void ExecuteHeightMapCollisionDetection();

    void GetHeightMapCollisionSensorDebugInfo(wchar_t* outputText, int maxCharNr);
    void GetHeightMapCollisionSensorDebugInfoEntryText(HMAPCOLLSENSOR *collSensor,
                                                       wchar_t* outputText,
                                                       int maxCharNr);

    void StartRecordingHeightMapCollisionDbgData(HMAPCOLLSENSOR *whichCollSensor);
    void StopRecordingHeightMapCollisionDbgData(char* outputDbgFileName);

    void SetGrabedByRecoveryVehicle(Recovery* whichRecoveryVehicle);
    void FreedFromRecoveryVehicleAgain();

private:
    irr::scene::IAnimatedMesh*  PlayerMesh;

    //for recording HeightMap Collision data for debugging
    std::vector<HMAPCOLLSENSOR*>* hMapCollDebugRecordingData;

    //if NULL then no recording is taking place right now
    HMAPCOLLSENSOR *hMapCollDebugWhichSensor = NULL;

    //from which recovery vehicle are we currently grabbed?
    Recovery* mGrabedByThisRecoveryVehicle = NULL;

    void SetNewState(irr::u32 newPlayerState);
    irr::u32 GetCurrentState();

    void GetHeightMapCollisionSensorDebugStateName(HMAPCOLLSENSOR *collSensor, char **stateName);
    void StoreHeightMapCollisionDbgRecordingDataForFrame();

    void UpdateHMapCollisionPointData();
    void HeightMapCollision(HMAPCOLLSENSOR &collSensor);

    float updateSlowCnter = 0.0f;

    bool CanIFindTextureIdAroundPlayer(int posX, int posY, int textureId);
    void CalcCraftLocalFeatureCoordinates(irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt);
    void ApplyWheelForces(irr::f32 deltaTime);
    void GetHeightRaceTrackBelowWheels(irr::f32 &frontLeft, irr::f32 &frontRight, irr::f32 &backLeft, irr::f32 &backRight);
    //void StabilizeCraft(irr::f32 deltaTime);

    void CheckForChargingStation();
    void CalcPlayerCraftLeaningAngle();

    void HeightMapCollisionResolve(irr::core::plane3df cplane, irr::core::vector3df pnt1, irr::core::vector3df pnt2);
    void UpdateHMapCollisionSensorPointData(HMAPCOLLSENSOR &sensor);

    void CreateHMapCollisionPointData();

    //variables to remember if during the last
    //gameloop this player did any charging
    bool mLastChargingFuel = false;
    bool mLastChargingShield = false;
    bool mLastChargingAmmo = false;

    //variables to remember if certain banner texts
    //were already shown to the player once, to prevent
    //to trigger them continiously
    bool mLowFuelWarningAlreadyShown = false;
    bool mEmptyFuelWarningAlreadyShown = false;
    bool mLowAmmoWarningAlreadyShown = false;
    bool mEmptyAmmoWarningAlreadyShown = false;
    bool mLowShieldWarningAlreadyShown = false;
    bool mBlockAdditionalFuelFullMsg = false;
    bool mBlockAdditionalShieldFullMsg = false;
    bool mBlockAdditionalAmmoFullMsg = false;

    bool mPlayerCurrentlyCharging = false;
    sf::Sound* mChargingSoundSource = NULL;
    sf::Sound* mWarningSoundSource = NULL;
    
    //each player has a particle system for the
    //case the craft is heavily damager
    //smoke is relased behind the craft
    SmokeTrail* mSmokeTrail;

    //each player has a particle system for the
    //case the craft is hovering above a dusty tile
    //to emit dust clouds below the craft
    //Is the original game does
    DustBelowCraft* mDustBelowCraft;

    void CheckDustCloudEmitter();

    bool mEmitDustCloud = false;
    bool mLastEmitDustCloud = false;

    //definition of dirt texture elements vector
    std::vector<irr::s32> *dirtTexIdsVec;

public:
       HUD* mHUD;
};

#endif // PLAYER_H
