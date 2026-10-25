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

class HUD; //Forward declaration
class Race; //Forward declaration
class SmokeTrail; //Forward declaration
class DustBelowCraft; //Forward declaration
class MachineGun; //Forward declaration
class MissileLauncher; //Forward declaration
class PhysicsObject; //Forward declaration
struct HudDisplayPart; //Forward declaration

class Player {
public:
    Player(Race* race, std::string model, irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt,
           irr::u8 nrLaps, bool humanPlayer);
    ~Player();


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

    bool firstNoKeyPressed = false;

    //True means humanPlayer
    //False is for computer player
    bool mHumanPlayer;

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

    void AddTextureID(irr::s32 newTexId);

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

    //the parent race for this player
    Race *mRace = nullptr;

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

    //void AlignPlayerModelToTerrainBelow();

    bool AllAnimatorsDone();

    EntityItem* cPTargetEntity = nullptr;

    //the following variable is only valid (and must only be used)
    //while currClosestWayPointLink is current != nullptr
    irr::core::vector3df projPlayerPositionClosestWayPointLink;

    //void ProjectPlayerAtCurrentSegment();

    irr::core::vector3df pos_in_worldspace_frontPos;
    irr::core::vector3df pos_in_worldspace_backPos;
    irr::core::vector3df pos_in_worldspace_leftPos;
    irr::core::vector3df pos_in_worldspace_rightPos;

    /*irr::f32 DbgCurrRaceTrackHeightFront;
    irr::f32 DbgCurrRaceTrackHeightBack;
    irr::f32 DbgCurrRaceTrackHeightLeft;
    irr::f32 DbgCurrRaceTrackHeightRight;*/

    //Get current weapon shooting target for this player
    //Returns true if there was a target found, False otherwise
    bool GetWeaponTarget(RayHitTriangleInfoStruct &shotTarget);

    //MovingAverageCalculator* mMovingAvgPlayerPositionCalc;
    //irr::core::vector3df mCurrentAvgPlayerPosition;

    void DebugSelectionBox(bool boundingBoxVisible);

private:
    //the mesh for the Irrlicht SceneNode model
    irr::scene::IAnimatedMesh* PlayerMesh = nullptr;

/*    bool GetCurrentCeilingMinimumPosition(irr::core::vector3df &currMinPos);
    bool GetCurrentCeilingMinimumPositionHelper(HMAPCOLLSENSOR *sensor,
                                                irr::core::vector3df &currMinPos, bool firstElement = false);*/

    float updateSlowCnter = 0.0f;

    void CalcCraftLocalFeatureCoordinates(irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt);

    void CheckForTriggerCraftRegion();

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

    irr::u8 mCurrentRiccosSound = 0;
    void PlayMGunShootsAtUsSound();

    bool mDebugDrawCurrWayPointLink = false;
    bool mDebugDrawActingForces = false;
    bool mDebugDrawCPUCurrSegment = false;
    bool mDebugDrawCPUPathHistory = false;
    bool mDebugDrawFreeSpace = false;

public:


public:
    //pointer to a connected HUD
    //nullptr if player is currently
    //not linked to any HUD
    HUD* mHUD = nullptr;
};

#endif // PLAYER_H
