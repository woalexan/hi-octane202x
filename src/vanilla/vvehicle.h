/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

//This source code was implemented by me based on the insight I gained into the original game. Some parts do more closely follow
//the Pseudo-C Code that is available to me, and other parts are heavily modified to hopefully work inside my existing project
//in the near future.
//The original game uses fixed point arithmetic for performance reasons (it had to run on 80486 after all), and is optimized everywhere you
//look at. I do not want to do the same, because the fixed point arithmetic makes it very difficult to read and understand. And performance should not
//be the problem nowadays. Therefore the biggest change which I have to do is to change all the logic to floating point arithmetic,
//while hopefully not breaking the concept behind everything.
//Additionally I will only try to reimplement the most important mechanisms and concepts from the original game where it benefits the overall project,
//but I will not change the overall project to use the same structs and data structures as the original. This would not work, and also does not
//make sense in my opinion.

//Important note: What makes this source code very difficult to handle is the fact, that my coordinate system in this existing project is
//completely different to the one in the original game. The original uses X and Y axis for the tile map, and Z is the height.
//For the levelfile and 2D map stuff I also use X any Y axis for the tile map most of the time.
//My 3D world setup (for rendering) using Irrlicht has X and Z for the tile map, and Y is the height. And to make things worse my Irrlicht vertice X and Y coordinates
//have a swapped sign (are negative) currently.

//I have decided to also use the original games coordinate system in all vanilla calculations. At the interface between
//original game calculations and Irrlicht 3D coordinate system I have then to convert from one coordinate system setup to the other.
//Thats the drawback I will have.

//I really want to thank aybe for giving me the opportunity to look much deeper into the original game inner workings as I was ever able before.
//Without this support I would not have been able to hopefully advance the current project more true to the original.

#ifndef VVEHICLE_H
#define VVEHICLE_H

#include <irrlicht.h>
#include <cstdint>
#include <vector>
#include <string>
#include "../vanilla/vcalc.h"
#include "../utils/path.h"
#include "SFML/Audio.hpp"

//This are player states I defined myself
//are not defined in the original game!
#define STATE_PLAYER_BEFORESTART 0
#define STATE_PLAYER_ONFIRSTWAYTOFINISHLINE 1
#define STATE_PLAYER_RACING 2
#define STATE_PLAYER_EMPTYFUEL 3
#define STATE_PLAYER_BROKEN 4
#define STATE_PLAYER_GRABEDBYRECOVERYVEHICLE 5

/************************
 * Forward declarations *
 ************************/

class Collectable;
class HUD;
class DustBelowCraft;
class VMGun;
class VMLauncher;
class Race;
struct MapTileRegionStruct;
struct VThing;

struct VehicleSensorPointStruct {
    irr::core::vector3df Position;
    int8_t CollideFlags;
    irr::f32 Zpos;
    irr::f32 ZposFloor;
    irr::f32 ZposDiff;
    //Important: Never remove initial value of 0.0f for
    //ZposDisplacement below, otherwise the craft will
    //show random/undefined behavior at the physics model start
    irr::f32 ZposDisplacement = 0.0f;
    irr::f32 Rebound;
    irr::f32 ReboundLimit;
};

struct VehicleControlFlagsStruct {
    bool Booster;
    bool Brake;
    bool Airbourn;
    bool BarrelRoll;
    bool AutoPilot;
    bool AutoRefuel;
    bool AutoRearm;
    bool AutoRepair;
    bool AutoStop;
    bool AutoPilotSet;
    bool AutoDrive;
    bool HealthDeath;
    bool FuelDeath;
    bool Reposition;
    //pad1 and pad2 seems to be used by
    //autopilot
    bool pad1;
    bool pad2;
};

struct VehicleFunctionFlagsStruct {
    bool Booster;
    bool Brake;
    bool BarrelRoll;
    bool Pad1;  //seems to be used for checkpoint processing logic
    bool Pad2;  //seems to be used for checkpoint processing logic
    bool Pad3;  //seems to be used for vehicle control logic
    bool Pad4;  //seems to be used for computer player control
    bool Pad5;  //seems to be used for computer player trigger of Rocket Gun
    bool Pad6;  //seems to be used during collision detection with vector collision
    bool Pad7;  //seems to be used for auto targeting system
    bool Pad8;  //seems to be used for auto targeting system
    bool Pad9;  //seems to be used for computer player control
    bool Pad11; //seems to be used for computer player trigger of MGun
    bool Pad12; //seems to be used for checkpoint processing logic
};

struct VehicleCollideControlStruct {
    irr::f32 SizeForward;
    irr::f32 SizeRear;
    irr::f32 SizeSideways;
    irr::f32 SizeSensorOffset;
    VehicleSensorPointStruct FrontLeft;
    VehicleSensorPointStruct FrontRight;
    VehicleSensorPointStruct RearLeft;
    VehicleSensorPointStruct RearRight;
    VehicleControlFlagsStruct Flag;
    VehicleFunctionFlagsStruct FunctionFlag;
    irr::f32 RideHeight;
    irr::f32 BrakePower;
};

struct VehicleStatsStruct {
    int16_t Behind;
    int16_t Fuel;
    int16_t Weapons;

    int16_t Health;
    irr::f32 Velocity;

    int16_t Weight;

    //11.09.2026: Invincable seems to be repurposed
    //as a counter variable that controls when
    //the players are able to start attacking after race start
    //As soon as the value is non zero the player can not attack
    //anymore
    int16_t Invincable;

    int16_t Invisible;
    int16_t VehicleHit;

    //player names in Hi-Octane are limited
    //to 8 characters, plus 1 termination char + 1 extra
    //char to be on the safe side :)
    char name[10];
};

struct VehicleSpecialMovesStruct {
    irr::f32 AngleXY;
    irr::f32 AngleZY;
    irr::f32 AngleXZ;
    int16_t Count;
};

struct VehicleBoosterStruct {
    irr::f32 InitialThrust;
    irr::f32 BurnThrust;
    int16_t BurnSetting;
    uint8_t BurnTime;
    uint8_t Burn;

    //variables moved here
    //from Thing (ThingWeapon)
    int16_t Trigger = 0;
    int16_t TriggerTime = 0;
    int16_t TriggerRestrictionCount = 0;
    int16_t Target = 0;

    //variables moved here
    //from Thing
    int16_t Upgrade = 0;
};

//all this values seem to be reset to 0
//inside this struct
//In the original game this struct is located
//in the Control related struct, but I moved it
//to vehicle
struct VehicleConditionsStruct {
    int32_t BumpAmount = 0;
    int32_t RocketsHit = 0;
    int32_t Bullets = 0;
    int32_t BulletsHit = 0;
    int32_t MiniGunHeatup = 0;

    //Note Deaths: In the original game implementation the Deaths array seems
    //to store the index to the ControlThing that did the frag. In my implementation
    //I will use the vehicle number right now instead!
    int32_t Deaths[8];

    int32_t DeathsCount = 0;
    int32_t Kills[8];
    int32_t KillsCount = 0;
    int32_t LapTimes[100];
    int32_t TotalTime = 0;
    int32_t LapCount = 0;
    int32_t FuelUsed = 0;
    int32_t HealthUsed = 0;
    int32_t WeaponsUsed = 0;

    int32_t RacePositionFinishShowTime = 0;

    bool FlagKill = false;
    bool FlagDeath = false;
    bool FlagNewLap = false;

    //not sure if the variables below
    //are needed for my purpose
 /*   int8_t FuelLowCounter = 0;
    int8_t FuelRechargeCounter = 0;
    int8_t FuelFullCounter = 0;
    int8_t WeaponsLowCounter = 0;
    int8_t WeaponsRechargeCounter = 0;
    int8_t WeaponsFullCounter = 0;
    int8_t HealthLowCounter = 0;
    int8_t HealthRechargeCounter = 0;
    int8_t HealthFullCounter = 0;*/

    int8_t FuelLowCounter = 0;
    int8_t WeaponsLowCounter = 0;
    int8_t HealthLowCounter = 0;
};

struct VehicleDamageStruct {
    uint16_t ShimmerCount = 0;
    uint16_t BulletCount = 0;
    uint16_t MissileCount = 0;
    uint16_t BulletHoles = 0;
};

struct VehicleComputerPlayerStruct {
    int16_t EnemyIndex;
    int16_t Count1;
    int16_t Count2;
    int16_t Count3;
    uint8_t Skill;
    uint8_t Param2;
    uint8_t Param3;
    uint8_t Param4;
};

struct VehicleAutoTargetStruct {
    uint16_t HitMeTotal[8];
    uint8_t HitMeCount[8];
    uint8_t HitMeTrigger[8];
    uint16_t PrimaryTarget;
    uint16_t ValidTargetCount;
};

struct VehicleMovementStatusStruct {
    irr::core::vector3df LastPosition;
    int16_t Count = 0;
};

class VVehicle {
public:
    //playerNr starting with value 1 for first player, 8 for last player
    VVehicle(Race* mParentRace, uint8_t playerNr, std::string model, irr::core::vector3d<irr::f32> NewPosition,
             irr::core::vector3d<irr::f32> NewFrontAt, irr::u8 nrLaps, bool humanPlayer);
    ~VVehicle();

    uint8_t mPlayerNr;

    void Update(irr::f32 frameDeltaTime);

    bool AllAnimatorsDone();

    void DrawDebug();
    void TestCamera();

    bool KeyPressedTurnLeft = false;
    bool KeyPressedTurnRight = false;
    bool KeyPressedAccel = false;
    bool KeyPressedDeaccel = false;
    bool KeyPressedBooster = false;
    bool KeyPressedMachineGun = false;
    bool KeyPressedMissileLauncher = false;

    //Pointer to my thing
    VThing* ThingData = nullptr;

    Race* mRace = nullptr;
public:
    MomentumStruct Momentum;

    //for player input
    MovementStruct MovementInput;

    MovementStruct Increment;

    //flight model constants
    MovementStruct IncrementAdd;
    MovementStruct IncrementSub;
    MovementStruct IncrementLimit;

    VehicleCollideControlStruct FlightModel;
    irr::core::vector3df Displacement;
    irr::core::vector3df Slope;
    irr::core::vector3df Bump;

    VehicleSpecialMovesStruct Tumble;
    VehicleAutoTargetStruct AutoTarget;

    //Stats
    VehicleStatsStruct Stats;

    int16_t mThrustEffectiveness;
    irr::f32 mSideslipFriction = 0.0f;
    irr::f32 mSideslipToThrust = 0.0f;

    //is the distance of the currently closest missile
    //to this player vehicle
    irr::f32 ClosestMissile = 0.0f;

    //BumpDamage was not used at the end
    //at last in the Playstation version of the
    //game
    irr::f32 mBumpDamage = 0.0f;

    //flight model parameters
    irr::f32 mFriction = 0.0f;
    irr::f32 mFrictionLimit = 0.0f;
    irr::f32 mBounce = 0.0f;
    irr::f32 mMaximumZpos = 0.0f;

    VehicleBoosterStruct Booster;
    VehicleConditionsStruct Conditions;
    VehicleViewStruct View;
    VehicleDamageStruct Damage;
    VehicleMovementStatusStruct MovementStatus;

    irr::f32 mDeltaTimeFactor = 1.0f;

    irr::f32 mDbgColl;

    irr::scene::IMeshSceneNode* mCraftNode = nullptr;

    irr::core::aabbox3d<irr::f32> mBoundingBox;

    //Returns true if vehicle is controlled by
    //computer player
    bool IsControlledByComputer();

    bool ShouldAmmoBarBlink();
    bool ShouldFuelBarBlink();
    bool ShouldShieldBarBlink();

    bool AllowedToCollectPowerUp();

    void SetName(char* playerName);

    //is called when the player collected a collectable item of the
    //level
    bool CollectedCollectable(Collectable* whichCollectable);

    void SetCurrClosestWayPointLink(std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> newClosestWayPointLink);

    std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> currClosestWayPointLink;
    std::vector< std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> > currCloseWayPointLinks;

    //the following variable is only valid (and must only be used)
    //while currClosestWayPointLink is current != nullptr
    //Important note: This 3D coordinate is saved in the Irrlicht coordinate
    //system
    irr::core::vector3df projPlayerPositionClosestWayPointLink;

    //next checkpoints value we need to reach for
    //correct race progress
    irr::s32 nextCheckPointValue = 0;

    //stores the last crossed checkpoints; we need this
    //to differentiate first crossing of finish line at the start of the race
    //with the end of following laps
    irr::s32 lastCrossedCheckPointValue = 0;

    //the following variable is used for race position calculation
    irr::f32 remainingDistanceToNextCheckPoint = 0.0f;

    bool mHasFinishedRace = false;

    uint32_t ControlStatus = 1;

    //Important note: This 3D Vector is stored in the Irrlicht
    //coordinate system!
    irr::core::vector3df IrrWorldDirVecForward;

    //pointer to a connected HUD
    //nullptr if player is currently
    //not linked to any HUD
    HUD* mHUD = nullptr;

    void SetMyHUD(HUD* pntrHUD);
    HUD* GetMyHUD();

    void StartPlayingWarningSound();
    void StopPlayingWarningSound();

    irr::core::vector3df IrrCoordGetDustEmitterPosition();

    void vehicle_setup_tumble();
    void vehicle_set_autodrive_on();
    uint8_t vehicle_set_autopilot_on();

    uint8_t vehicle_race_positions_compare(VVehicle* vehicle2);

    //this three variables need to be public, so that the Race
    //can derive the current players race position
    uint16_t CheckPoint = 0;
    int16_t LapCounter = 0;
    int16_t RaceLaps = 0;
    irr::f32 DistanceToNextCheckpoint = 0.0f;

    int16_t RacePosition = 0;
    int16_t RacePlayerCount = 0;
    int16_t RacePositionFinish = 0;
    uint16_t CurrentWaypoint = 0;

    int32_t FastestLapNr = 0;
    int32_t FastestLapTicks = 0;
    int32_t LapTicks = 0;
    int32_t LastLapTicks = 0;

    //This variables seems to have something to do with
    //checkpoint handling
    int16_t Counter[8];

    void vehicle_set_camera();

    void FinishedRace();
    void TriggerRaceStart();

    //My Weapons
    VMGun* mMGun = nullptr;
    VMLauncher* mMLauncher = nullptr;

    uint32_t GetControlOrigin();
    void SetGameDifficultyLevel(uint8_t newDifficultyLevel);

    int16_t ControlViewType = 0;

    //void TestBigExplosion();

private:
    uint32_t ControlOrigin = 1; //activates the human player
    uint16_t LastWayPoint = 0;

    int32_t TotalRaceTicks = 0;
    int32_t TotalRaceTicksFinished = 0;

    irr::f32 mAbsTimeIntegrator = 0.0f;

    irr::f32 mUpdateVehicleTimeIntegrator = 0.0f;

    uint8_t PlayerDifficultyLevel = 0;

    bool mRaceTriggered = false;

    //the mesh for the Irrlicht SceneNode model
    irr::scene::IAnimatedMesh* mCraftMesh = nullptr;

    bool mUpdateEngineSound = false;

    void SetupFlightModelConstants();

    void vehicle_execute_action0x0_initialize();
    void vehicle_execute_action0x1_defaultracing();
    void vehicle_execute_action0x9_beforeexploding();
    void vehicle_execute_action0x11_spawnpowerups();
    void vehicle_execute_action0x13_exploding();
    void vehicle_execute_action0x14_callAndWaitForRecoveryVehicle();
    void vehicle_execute_action0x16();
    void vehicle_execute_action0x17_rescue();
    void vehicle_execute_action0x18();
    void vehicle_execute_action0x19_reset();

    void vehicle_do_action();

    VehicleComputerPlayerStruct ComputerPlayer;
    uint8_t vehicle_check_vehicle_movement_status();
    void vehicle_setup_computer_character();

    void vehicle_control();
    void vehicle_control_from_player();
    uint8_t vehicle_control_from_autopilot();
    uint8_t vehicle_set_autopilot_off();
    void vehicle_set_autodrive_off();

    //Returns true if BarrelRoll is not yet fininshed, False
    //if BarrelRoll has ended
    bool vehicle_do_tumble();

    void vehicle_get_track_friction();
    void vehicle_calculate_angle();
    void vehicle_calculate_thrust(irr::core::vector3df& delta);
    void vehicle_calculate_momentum(irr::core::vector3df& delta);
    void vehicle_calculate_movement_delta(irr::core::vector3df& delta);
    void vehicle_move_altitude(irr::core::vector3df& delta);
    void vehicle_move_tilt(irr::core::vector3df& delta);
    void vehicle_move_roll(irr::core::vector3df& delta);
    void vehicle_sensor_point_projection(irr::core::vector3df& delta);
    void vehicle_sensor_point_process(VehicleSensorPointStruct& sensor, irr::core::vector3df& slope, int8_t terrain);
    void vehicle_colide_map(irr::core::vector3df& delta);
    void vehicle_colide_vectors(irr::core::vector3df& delta);
    void vehicle_terrain_effect(irr::core::vector3df delta);
    uint8_t vehicle_colide(std::vector<VVehicle*> &vehicleVec, irr::core::vector3df& delta);
    uint8_t vehicle_colide_final_check_sean(std::vector<VVehicle*> &vehicleVec, irr::core::vector3df& delta);
    void vehicle_move_mapwho(irr::core::vector3df& delta);
    void vehicle_post_process();

    int32_t vehicle_get_checkpoint();
    uint8_t vehicle_process_checkpoint(int16_t cp_colide);
    int16_t vehicle_checkpoint_find_next(int16_t forCheckPointIdx);
    void vehicle_checkpoint_next_lap();

    //Returns a possible vehicle target within a specified
    //angle of view; If no target is found returns 0 value
    //Otherwise it returns the index into the mRace->mVanillaCraftVec
    //vector for the selected player + 1
    uint16_t vehicle_target(irr::f32 angle);
    void vehicle_targetting_system();
    void vehicle_process_autotarget();
    uint8_t vehicle_computer_set_no_shoot();
    void vehicle_calculate_behind_factor();

    void UpdateEngineSound();

    bool OrientedBBoxCollision(VVehicle* vehicle1, VVehicle* vehicle2,
                                        irr::core::vector3df& collNormal, irr::f32& depth);

    uint8_t vehicle_colide_my_attempt(std::vector<VVehicle*> &vehicleVec, irr::core::vector3df& delta);
    bool VehiclesCheckForCollision(VVehicle* vehicle1, VVehicle* vehicle2,
               irr::core::vector3df& collNormal, irr::f32& depth);

    //Below are my functions and Members I need
    //a local coordinate system point defined on the players craft
    irr::core::vector3d<irr::f32> IrrLocalCraftFrontPnt;
    irr::core::vector3d<irr::f32> IrrLocalCraftBackPnt;
    irr::core::vector3d<irr::f32> IrrLocalCraftLeftPnt;
    irr::core::vector3d<irr::f32> IrrLocalCraftRightPnt;
    irr::core::vector3d<irr::f32> IrrLocalCraftOrigin;
    irr::core::vector3d<irr::f32> IrrLocalCraftTriggerSensor;

    //local position on the craft where dust clouds are created when
    //hovering above dusty tiles next to the race track
    irr::core::vector3d<irr::f32> IrrLocalCraftDustPnt;

    irr::core::vector3d<irr::f32> IrrWorldCraftFrontPnt;
    irr::core::vector3d<irr::f32> IrrWorldCraftBackPnt;
    irr::core::vector3d<irr::f32> IrrWorldCraftLeftPnt;
    irr::core::vector3d<irr::f32> IrrWorldCraftRightPnt;
    irr::core::vector3d<irr::f32> IrrWorldCraftOrigin;
    irr::core::vector3d<irr::f32> IrrWorldCraftTriggerSensor;

    //world coordinate position on the craft where dust clouds are created when
    //hovering above dusty tiles next to the race track
    irr::core::vector3d<irr::f32> IrrWorldCraftDustPnt;

    sf::Sound* mWarningSoundSource = nullptr;

    void CalcCraftLocalFeatureCoordinates(irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt);

    void CheckForTriggerCraftRegion();
    void CheckForChargingStation();

    //is unequal to NULL if player craft is currently inside
    //a craft trigger area defined in the level during
    //the current player update
    MapTileRegionStruct* mCurrentCraftTriggerRegion = nullptr;

    //is unequal to NULL if player craft was inside
    //a craft trigger area defined in the level during
    //the last player update
    MapTileRegionStruct* mLastCraftTriggerRegion = nullptr;

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

    bool mPlayerCurrentlyCharging = false;
    sf::Sound* mChargingSoundSource = nullptr;

    //variables to remember if certain banner texts
    //were already shown to the player once, to prevent
    //to trigger them continiously
    bool mLowFuelWarningAlreadyShown = false;
    bool mEmptyFuelWarningAlreadyShown = false;
    bool mLowAmmoWarningAlreadyShown = false;
    bool mLowShieldWarningAlreadyShown = false;

    bool mBlockAdditionalFuelFullMsg = true;
    bool mBlockAdditionalShieldFullMsg = true;
    bool mBlockAdditionalAmmoFullMsg = true;

    //each player has a particle system for the
    //case the craft is hovering above a dusty tile
    //to emit dust clouds below the craft
    //Is the original game does
    DustBelowCraft* mDustBelowCraft = nullptr;

    sf::Sound* TurboSound = nullptr;
    sf::Sound* CollisionSound = nullptr;

    void CheckDustCloudEmitter();

    //definition of dirt texture elements vector
    std::vector<irr::s32> *dirtTexIdsVec = nullptr;

    bool mEmitDustCloud = false;
    bool mLastEmitDustCloud = false;

    void FinishedLap();

    void UpdateCoordinates();

    void UpdateSceneNode();

    void processWeaponBooster();
};

#endif // VVEHICLE_H

