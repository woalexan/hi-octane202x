/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PLAYER_H
#define PLAYER_H

#include "SFML/Audio.hpp"
#include <list>
#include <vector>
#include "irrlicht.h"
#include "../utils/path.h"
#include "../definitions.h"
#include "../utils/ray.h"
#include "../utils/worldaware.h"

//The target hover height of the craft above the race track
const irr::f32 HOVER_HEIGHT = 0.6f;  //0.6f

const irr::f32 MAX_PLAYER_SPEED = 17.0f;

const irr::f32 CRAFT_SIDEWAYS_BRAKING = 2.0f;

const irr::f32 CRAFT_JUMPDETECTION_THRES = 1.2f;

const irr::f32 DEF_PLAYER_MGUN_MINHIT_PROB = 25.0f;
const irr::f32 DEF_PLAYER_MGUN_MAXHIT_PROB = 90.0f;

#define CRAFT_AIRFRICTION_NOTURBO 0.3f
#define CRAFT_AIRFRICTION_TURBO 0.2f

#define CRAFT_NOLEANING 0
#define CRAFT_LEANINGLEFT 1
#define CRAFT_LEANINGRIGHT 2

#define STATE_HMAP_COLL_IDLE 0
#define STATE_HMAP_COLL_WATCH 1
#define STATE_HMAP_COLL_RESOLVE 2
#define STATE_HMAP_COLL_NOINTERSECTPNT 3
#define STATE_HMAP_COLL_FAILED 4

#define STATE_PLAYER_BEFORESTART 0
#define STATE_PLAYER_ONFIRSTWAYTOFINISHLINE 1
#define STATE_PLAYER_RACING 2
#define STATE_PLAYER_EMPTYFUEL 3
#define STATE_PLAYER_BROKEN 4
#define STATE_PLAYER_GRABEDBYRECOVERYVEHICLE 5

#define CAMERA_PLAYER_COCKPIT 0
#define CAMERA_PLAYER_BEHINDCRAFT 1
#define CAMERA_EXTERNALVIEW 2

/************************
 * Forward declarations *
 ************************/

struct WayPointLinkInfoStruct;
struct MapTileRegionStruct;
class Collectable;
class Camera;
class CpuPlayer;
class MovingAverageCalculator;
class MapEntry;
class EntityItem;

typedef struct {
    irr::u8 lapNr;
    irr::u32 lapTimeMultiple40mSec;
} LAPTIMEENTRY;

typedef struct PlayerStats {
    irr::f32 speed;
    irr::f32 speedMax = MAX_PLAYER_SPEED;

    irr::f32 shieldVal;
    irr::f32 gasolineVal;
    irr::f32 ammoVal;

    irr::f32 shieldMax = 50.0;
    irr::f32 gasolineMax = 50.0;

    //the player has ammo for 6 missiles
    //at the start
    irr::f32 ammoMax = 6.0;

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

    irr::u32 currLapTimeMultiple40mSec = 0;
    irr::f32 currLapTimeExact;
    std::vector <LAPTIMEENTRY> lapTimeList;
    LAPTIMEENTRY lastLap;
    LAPTIMEENTRY LapBeforeLastLap;

    irr::u8 currRacePlayerPos = 0;
    irr::u8 overallPlayerNumber = 0;

    irr::u8 currLapNumber = 1;

    irr::u8 raceNumberLaps = 6;

    bool mHasFinishedRace = false;

    //for the accuracy calculation later
    //in statistics
    irr::u32 shootsMissed = 0;
    irr::u32 shootsHit = 0;

    //the number of occurences where this player
    //has totaled another player completely
    irr::u8 currKillCount = 0;

    //the number of occurences where this player
    //was totaled, for whatever reason
    irr::u8 currDeathCount = 0;

    //player names in Hi-Octane are limited
    //to 8 characters, plus 1 termination char + 1 extra
    //char to be on the safe side :)
    char name[10];

    bool mPlayerCanMove;
    bool mPlayerCanShoot;

    irr::u8 mPlayerCurrentState;
} PLAYERSTATS;

//struct for one heightmap collision "sensor" element
//which consists of 2 points for terrain stepness measurement
typedef struct HMapCollSensor {
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
struct HudDisplayPart; //Forward declaration

class Player {
public:
    Player(Race* race, std::string model, irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt,
           irr::u8 nrLaps, bool humanPlayer);
    ~Player();

    HMAPCOLLSENSOR* cameraSensor = nullptr;
    HMAPCOLLSENSOR* cameraSensor2 = nullptr;
    HMAPCOLLSENSOR* cameraSensor3 = nullptr;
    HMAPCOLLSENSOR* cameraSensor4 = nullptr;
    HMAPCOLLSENSOR* cameraSensor5 = nullptr;
    HMAPCOLLSENSOR* cameraSensor6 = nullptr;
    HMAPCOLLSENSOR* cameraSensor7 = nullptr;

    void SetPlayerObject(PhysicsObject* phObjPtr);
    void DamageGlas();

    //returns true if player craft of a computer
    //player is currently stuck
    //returns always false when called for a human
    //player, regardless what the human player currently
    //does
    bool IsCurrentlyStuck();

    void ExecuteCpPlayerLogic(irr::f32 deltaTime);

    void GetHeightRaceTrackBelowCraft(irr::f32 &front, irr::f32 &back, irr::f32 &left, irr::f32 &right);
    irr::f32 currHeightFront;
    irr::f32 currHeightBack;
    irr::f32 currHeightLeft;
    irr::f32 currHeightRight;

    //set high reset values, so that the values have no
    //influence until first worldAware calculation has been done
    irr::f32 mCraftDistanceAvailRight = 100.0f;
    irr::f32 mCraftDistanceAvailLeft = 100.0f;
    irr::f32 mCraftDistanceAvailBack = 100.0f;
    irr::f32 mCraftDistanceAvailFront = 100.0f;

    irr::f32 debugMaxStep;
    MapEntry* currTileBelowPlayer = nullptr;

    irr::core::vector3df dbgcollPlanePos1;
    irr::core::vector3df dbgcollPlanePos2;
    irr::core::vector3df dbgcollResolutionDirVec;
    irr::f32 dbgDistance;

    irr::f32 dbgMaxh;
    irr::f32 dbgNewCameraVal;
    irr::f32 dbgCameraAvgVAl;
    irr::f32 dbgMinCeilingFound;
    irr::f32 dbgCameraVal;
    irr::f32 dbgCameraTargetVal;

    bool mDbgCurrRecording = false;

    std::vector<irr::f32> *dbgRecordFrontHeight = nullptr;
    std::vector<irr::f32> *dbgRecordBackHeight = nullptr;
    std::vector<irr::u8> *dbgRecordCurrJumping = nullptr;
    std::vector<irr::u8> *dbgRecordCurrCollision = nullptr;

    void StartDbgRecording();
    void EndDbgRecording();

    bool firstNoKeyPressed = false;

    //True means humanPlayer
    //False is for computer player
    bool mHumanPlayer;

    //the following control functions are only
    //valid to be used with a human player!
    void Forward(irr::f32 deltaTime);
    void Backward(irr::f32 deltaTime);
    void Left();
    void Right();
    void NoTurningKeyPressed();

    void ChangeViewMode();

    int mCurrPosCellX;
    int mCurrPosCellY;

    bool mRecoveryVehicleCalled = false;

    std::vector<Collectable*> mCollectablesSeenByPlayer;

    std::vector<RayHitInfoStruct> PlayerSeenList;
    std::vector<irr::f32> PlayerSeenAngleList;

    std::vector<Player*> dbgPlayerInMyWay;

    //Returns true if the player actually picked the item up
    //false otherwise; For example the extra ammo item is only picked
    //up by players that have not already full ammo currenty
    bool CollectedCollectable(Collectable* whichCollectable);

    void StartPlayingWarningSound();
    void StopPlayingWarningSound();

    void Update(irr::f32 frameDeltaTime);
    void AddTextureID(irr::s32 newTexId);

    void SetName(char* playerName);

    //returns TRUE if player reached below/equal 0 health (therefore if
    //player died); otherwise false is returned
    bool Damage(irr::f32 damage, irr::u8 damageType);

    //if showDurationSec is negative, the text will be shown until it is deleted
    //with a call to function RemovePlayerPermanentGreenBigText
    //if blinking is true text will blink (for example used for final lap text), If false
    //text does not blink (as used when player died and waits for repair craft)
    void ShowPlayerBigGreenHudText(char* text, irr::f32 timeDurationShowTextSec, bool blinking);
    void RemovePlayerPermanentGreenBigText();

    irr::f32 currentSideForce = 0.0f;

    irr::core::vector3d<irr::f32> LocalTopLookingCamPosPnt;
    irr::core::vector3d<irr::f32> LocalTopLookingCamTargetPnt;

    irr::core::vector3d<irr::f32> Local1stPersonCamPosPnt;
    irr::core::vector3d<irr::f32> Local1stPersonCamTargetPnt;

    irr::core::vector3df WorldTopLookingCamPosPnt;
    irr::core::vector3df WorldTopLookingCamTargetPnt;

    irr::core::vector3df World1stPersonCamPosPnt;
    irr::core::vector3df World1stPersonCamTargetPnt;

    //local coordinate for control of craft
    irr::core::vector3d<irr::f32> LocalCraftForceCntrlPnt;

    //a local coordinate system point defined on the players craft, for height control
    irr::core::vector3d<irr::f32> LocalCraftFrontPnt;
    irr::core::vector3d<irr::f32> LocalCraftBackPnt;
    irr::core::vector3d<irr::f32> LocalCraftLeftPnt;
    irr::core::vector3d<irr::f32> LocalCraftRightPnt;

    irr::core::vector3d<irr::f32> LocalCraftOrigin;

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

    //world coordinate position on the craft where smoke pours out if
    //player is damaged
    irr::core::vector3d<irr::f32> WorldCoordCraftSmokePnt;

    //world coordinate position on the craft where dust clouds are created when
    //hovering above dusty tiles next to the race track
    irr::core::vector3d<irr::f32> WorldCraftDustPnt;

    irr::core::vector3df LocalCraftAboveCOGStabilizationPoint;
    irr::core::vector3df WorldCoordCraftAboveCOGStabilizationPoint;

    irr::core::vector3d<irr::f32> craftForwardDirVec;

    //only for debugging
    irr::f32 mCurrentCraftDistToWaypointLink;
    irr::f32 mAngleError;

    irr::f32 DbgShipUpAngle;

    irr::s32 currTextID;
    std::vector<irr::s32> textureIDlist;

    bool mPlayerModelSmoking = false;
    bool mLastPlayerModelSmoking = false;

    //if we currently target another player for attacking
    //this pointer points to the other target player
    //no target if nullptr
    Player* mTargetPlayer = nullptr;
    Player* mLastTargetPlayer = nullptr;

    //value from 22 downto 0 which describes how far we are
    //with the missle lock, 0 means lock achieved, 22 means
    //just began with missle targeting
    irr::u32 mTargetMissleLockProgr = 22;

    //if we have a missile lock as well this bool
    //is true. When this happens in the original game
    //the symbol becomes red
    bool mTargetMissleLock = false;

    void SetTarget(Player* newTarget);

    irr::core::vector3df craftUpwardsVec;
    irr::core::vector3df craftSidewaysToRightVec;

    irr::f32 mCurrentCraftOrientationAngle;
    /*irr::f32 mCurrentCraftOrientationAngleAvg;
    std::list<irr::f32> mCurrentCraftOrientationAngleVec;
    irr::u8 mCurrentCraftOrientationAngleSamples = 0;
    void UpdateCurrentCraftOrientationAngleAvg();*/

    //the players physics object
    PhysicsObject* phobj = nullptr;

    //the players Irrlicht SceneNode for the
    //craft model
    irr::scene::IMeshSceneNode* Player_node = nullptr;

    //is prepared value to later find random
    //shooting target for mgun at player model quickly
    irr::core::vector3df mPlayerModelExtend;

    //my player internal camera for cockpit
    irr::scene::ICameraSceneNode* mIntCamera = nullptr;
    //my 3rd person camera from behind the craft
    irr::scene::ICameraSceneNode* mThirdPersonCamera = nullptr;

    void UpdateCameras();

    //player craft shadow SceneNode
    irr::scene::IShadowVolumeSceneNode* PlayerNodeShadow = nullptr;

    //the players stats
    PLAYERSTATS *mPlayerStats = nullptr;

    //copy of the player stats when player
    //crosses finish line the last time
    PLAYERSTATS *mFinalPlayerStats = nullptr;

    irr::f32 terrainTiltCraftLeftRightDeg;
    irr::f32 terrainTiltCraftFrontBackDeg;

    //next checkpoints value we need to reach for
    //correct race progress
    irr::s32 nextCheckPointValue = 0;

    //stores the last crossed checkpoints; we need this
    //to differentiate first crossing of finish line at the start of the race
    //with the end of following laps
    irr::s32 lastCrossedCheckPointValue = 0;

    void CrossedCheckPoint(irr::s32 valueCrossedCheckPoint, irr::s32 numberOfCheckpoints);

    //the parent race for this player
    Race *mRace = nullptr;

    //debug height calculation variables
    LineStruct debug;
    std::list<LineStruct*> debug_player_heightcalc_lines;

    irr::f32 TargetHeight;

    irr::f32 testAngle;

    //irr::core::quaternion GetQuaternionFromPlayerModelRotation();

    bool ProjectOnWayPoint(WayPointLinkInfoStruct* projOnWayPointLink, irr::core::vector3df craftCoord,
                           irr::core::vector3df *projPlayerPosition,
                           irr::core::vector3d<irr::f32>* distanceVec, irr::f32 *remainingDistanceToTravel);

    std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> currClosestWayPointLink;
    std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> lastClosestWayPointLink;

    std::vector< std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> > currCloseWayPointLinks;

    void SetCurrClosestWayPointLink(std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> newClosestWayPointLink);
    //irr::core::vector3df DeriveCurrentDirectionVector(WayPointLinkInfoStruct *currentWayPointLine, irr::f32 progressCurrWayPoint);

    irr::f32 GetHoverHeight();
    //void AlignPlayerModelToTerrainBelow();

    void SetupComputerPlayerForStart(irr::core::vector3df startPos);

    void CraftHeightControl();
    void JumpControl(irr::f32 deltaTime);

    void SetupForStart();
    void SetupToSkipStart();
    void SetupForFirstWayToFinishLine();
    void DeactivateAttack();

    bool AllAnimatorsDone();

    EntityItem* cPTargetEntity = nullptr;

    //the following variable is only valid (and must only be used)
    //while currClosestWayPointLink is current != nullptr
    irr::core::vector3df projPlayerPositionClosestWayPointLink;

    //the following variable is used for race position calculation
    irr::f32 remainingDistanceToNextCheckPoint = 0.0f;

    //the current calculate angle between craft upwards direction vector
    //and the world Y-axis. Is used to draw the sky background image realistic
    irr::f32 currPlayerCraftLeaningAngleDeg = 0.0f;
    irr::u8 currPlayerCraftLeaningOrientation = CRAFT_NOLEANING;

    irr::f32 currPlayerCraftForwardLeaningAngleDeg = 0.0f;

    //void ProjectPlayerAtCurrentSegment();

    void FinishedLap();

    irr::core::vector3df pos_in_worldspace_frontPos;
    irr::core::vector3df pos_in_worldspace_backPos;
    irr::core::vector3df pos_in_worldspace_leftPos;
    irr::core::vector3df pos_in_worldspace_rightPos;

    /*irr::f32 DbgCurrRaceTrackHeightFront;
    irr::f32 DbgCurrRaceTrackHeightBack;
    irr::f32 DbgCurrRaceTrackHeightLeft;
    irr::f32 DbgCurrRaceTrackHeightRight;*/

    void DebugCraftLocalFeatureCoordinates();

    void SetMyHUD(HUD* pntrHUD);
    HUD* GetMyHUD();

    bool mBoosterActive = false;
    bool mBoosterRechargeCurrentlyLocked = false;
    bool mLastBoosterActive = false;
    sf::Sound* TurboSound = nullptr;

    void IsSpaceDown(bool down, irr::f32 deltaTime);
    void MaxTurboReached();

    bool mMaxTurboActive = false;
    bool mLastMaxTurboActive = false;
    irr::f32 mRemainingMaxTurboActiveTime;

    void Collided();
    void AfterPhysicsUpdate();
    sf::Sound* CollisionSound = nullptr;

    MachineGun* mMGun = nullptr;
    MissileLauncher* mMissileLauncher = nullptr;

    //Get current weapon shooting target for this player
    //Returns true if there was a target found, False otherwise
    bool GetWeaponTarget(RayHitTriangleInfoStruct &shotTarget);

    HMAPCOLLSTRUCT mHMapCollPntData;

    void ExecuteHeightMapCollisionDetection(irr::f32 deltaTime);

    void GetHeightMapCollisionSensorDebugInfo(wchar_t* outputText, int maxCharNr);
    void GetHeightMapCollisionSensorDebugInfoEntryText(HMAPCOLLSENSOR *collSensor,
                                                       wchar_t* outputText,
                                                       int maxCharNr);

    void StartRecordingHeightMapCollisionDbgData(HMAPCOLLSENSOR *whichCollSensor);
    void StopRecordingHeightMapCollisionDbgData(char* outputDbgFileName);

    void SetGrabedByRecoveryVehicle(Recovery* whichRecoveryVehicle);
    void FreedFromRecoveryVehicleAgain();

    irr::core::vector3df dbgCurrCeilingMinPos;
    bool minCeilingFound;

    irr::core::vector2df GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord);

    //vector with this players HUD broken glas
    //positions must be stored within the player, as each
    //player has a different state of the glass
    std::vector<HudDisplayPart*>* brokenGlasVec = nullptr;

    //true if player craft is currently jumping
    bool mCurrJumping = false;
    irr::f32 mCurrInAirTime;

    bool firstHeightControlLoop = true;

    //needed for player jump detection
    //logic
    irr::f32 lastHeightFront;
    irr::f32 lastHeightBack;

    irr::f32 lastDistCraftTerrainBack;
    irr::f32 currDistCraftTerrainFront;
    irr::f32 currDistCraftTerrainBack;

    irr::u8 playerAbsAngleSkytListElementNr = 0;

    MovingAverageCalculator* mMovingAvgPlayerLeaningAngleLeftRightCalc = nullptr;
    irr::f32 mCurrentAvgPlayerLeaningAngleLeftRightValue;
    irr::f32 nextLeaningAngleUpdate = -1.0f;

    //MovingAverageCalculator* mMovingAvgPlayerPositionCalc;
    //irr::core::vector3df mCurrentAvgPlayerPosition;

    void DebugSelectionBox(bool boundingBoxVisible);

    void TestCpForceControlLogicWithHumanPlayer();

    //current best camera for this players external view
    Camera* externalCamera = nullptr;

    irr::u8 mCurrentViewMode;
    irr::u8 mLastViewModeBeforeBrokenCraft;

    irr::scene::ICameraSceneNode* DeliverActiveCamera();

    bool mOtherPlayerHasMissleLockAtMe = false;

    void HideCraft();
    void UnhideCraft();

    bool mCraftVisible;

    bool DoWeNeedHidePlayerModel();
    bool IsCurrentlyValidTarget();

    bool ShouldAmmoBarBlink();
    bool ShouldGasolineBarBlink();
    bool ShouldShieldBarBlink();

    irr::core::vector3df GetRandomMGunShootTargetLocation(bool shootDoesHit);

    //returns integer with value between 0 and 100
    //percent
    irr::u32 GetMGunHitProbability();

    void LogMessage(char *msgTxt);

    //returns true if player is currently in chargeing
    //station and charges fuel, shield or ammo
    bool IsCurrentlyCharging();

    bool IsCurrentlyChargingFuel();
    bool IsCurrentlyChargingShield();
    bool IsCurrentlyChargingAmmo();

private:
    //the mesh for the Irrlicht SceneNode model
    irr::scene::IAnimatedMesh* PlayerMesh = nullptr;

    //for recording HeightMap Collision data for debugging
    std::vector<HMAPCOLLSENSOR*>* hMapCollDebugRecordingData = nullptr;

    //if nullptr then no recording is taking place right now
    HMAPCOLLSENSOR *hMapCollDebugWhichSensor = nullptr;

    //from which recovery vehicle are we currently grabbed?
    Recovery* mGrabedByThisRecoveryVehicle = nullptr;

    void SetNewState(irr::u32 newPlayerState);

    irr::u32 GetCurrentState();

    void GetHeightMapCollisionSensorDebugStateName(HMAPCOLLSENSOR *collSensor, char **stateName);
    void StoreHeightMapCollisionDbgRecordingDataForFrame();

    void UpdateHMapCollisionPointData();

    //returns true if a collision at this sensor was detected
    bool HeightMapCollision(HMAPCOLLSENSOR &collSensor);

    void JumpControlPhysicsLoop(irr::f32 deltaTime);

    bool GetCurrentCeilingMinimumPosition(irr::core::vector3df &currMinPos);
    bool GetCurrentCeilingMinimumPositionHelper(HMAPCOLLSENSOR *sensor,
                                                irr::core::vector3df &currMinPos, bool firstElement = false);

    float updateSlowCnter = 0.0f;

    void CalcCraftLocalFeatureCoordinates(irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt);

    void CheckForChargingStation(irr::f32 deltaTime);
    void CheckForTriggerCraftRegion();
    void CalcPlayerCraftLeaningAngle();

    //void HeightMapCollisionResolve(irr::core::plane3df cplane, irr::core::vector3df pnt1, irr::core::vector3df pnt2);
    void UpdateHMapCollisionSensorPointData(HMAPCOLLSENSOR &sensor);

    void CreateHMapCollisionPointData();

    std::list<irr::f32> playerCamHeightList;
    irr::u8 playerCamHeightListElementNr = 0;

    std::list<irr::f32> playerAbsAngleSkyList;

    //variables to remember if during the last
    //gameloop this player did any charging
    bool mLastChargingFuel = false;
    bool mLastChargingShield = false;
    bool mLastChargingAmmo = false;

    //variables to know if we do currently
    //charging
    bool mCurrChargingFuel = false;
    bool mCurrChargingShield = false;
    bool mCurrChargingAmmo = false;

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
    sf::Sound* mChargingSoundSource = nullptr;
    sf::Sound* mWarningSoundSource = nullptr;
    sf::Sound* mLockOnSoundSource = nullptr;
    
    //each player has a particle system for the
    //case the craft is heavily damager
    //smoke is relased behind the craft
    SmokeTrail* mSmokeTrail = nullptr;

    //each player has a particle system for the
    //case the craft is hovering above a dusty tile
    //to emit dust clouds below the craft
    //Is the original game does
    DustBelowCraft* mDustBelowCraft = nullptr;

    void CheckDustCloudEmitter();

    bool mEmitDustCloud = false;
    bool mLastEmitDustCloud = false;

    //definition of dirt texture elements vector
    std::vector<irr::s32> *dirtTexIdsVec = nullptr;

    //is unequal to NULL if player craft is currently inside
    //a craft trigger area defined in the level during
    //the current player update
    MapTileRegionStruct* mCurrentCraftTriggerRegion = nullptr;

    //is unequal to NULL if player craft was inside
    //a craft trigger area defined in the level during
    //the last player update
    MapTileRegionStruct* mLastCraftTriggerRegion = nullptr;

    void AddGlasBreak();
    void RepairGlasBreaks();
    void CleanUpBrokenGlas();

    void StartPlayingLockOnSound();
    void StopPlayingLockOnSound();

    void WasDestroyed();

    void UpdateHUDState();
    void FinishedRace();
    void CpTakeOverHuman();

    irr::u8 mCurrentRiccosSound = 0;
    void PlayMGunShootsAtUsSound();

    void HandleFuel(irr::f32 deltaTime);
    void HandleAmmo();
    void HandleShield();

    void UpdateInternalCoordVariables();

public:

    //every player needs a CpuPlayer that is able
    //to control the craft, at least after the race
    //is finished for the player
    CpuPlayer* mCpuPlayer = nullptr;

public:
    //pointer to a connected HUD
    //nullptr if player is currently
    //not linked to any HUD
    HUD* mHUD = nullptr;
};

#endif // PLAYER_H
