/*
 The source code of function createEntity in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself (and later modified by me)
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 The source code for function draw2DImage was taken from:  (most likely from user Lonesome Ducky)
 https://irrlicht.sourceforge.io/forum/viewtopic.php?t=43565
 https://irrlicht.sourceforge.io/forum//viewtopic.php?p=246138#246138

 Copyright (C) 2024-2025 Wolf Alexander
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code of function createEntity, later modified by me)
 
 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef RACE_H
#define RACE_H

#include "irrlicht.h"
#include <vector>
#include <list>
#include "resources/entityitem.h"
#include <string>
#include "xeffects/XEffects.h"
#include "scenenodes/CloudSceneNode.h"
#include "scenenodes/CLensFlareSceneNode.h"

using namespace std;

//after race is finished (all players finished last lap)
//the exit of race is delayed by this amount of time in seconds
const irr::f32 DEF_RACE_FINISHED_WAITTIME_SEC = 7.0f;

//maximum time the external camera is following a specific player
//in demo mode, until change to the next scene
const irr::f32 DEF_RACE_DEMOMODE_MAXTIMEFOLLOWPLAYER = 10.0f;

#define DEF_RACE_DAMAGETYPE_UNDEF 0
#define DEF_RACE_DAMAGETYPE_MGUN 1
#define DEF_RACE_DAMAGETYPE_MISSILE 2

#define DEF_RACE_PHASE_START 0
#define DEF_RACE_PHASE_FIRSTWAYTOWARDSFINISHLINE 1
#define DEF_RACE_PHASE_RACING 2
#define DEF_RACE_PHASE_WAITUNTIL_ANIMATORS_DONE 3

struct RaceStatsEntryStruct {
    //player names in Hi-Octane are limited
    //to 8 characters, plus 1 termination char + 1 extra
    //char to be on the safe side :)
    char playerName[10];

    //the hit accuracy
    irr::u8 hitAccuracy;

    //the number of kills
    irr::u8 nrKills;

    //the number of deaths
    irr::u8 nrDeaths;

    //the average lap time
    irr::u16 avgLapTime;

    //the best lap time
    irr::u16 bestLapTime;

    //the race time
    irr::u32 raceTime;

    //the rating number from 1 up to 20
    irr::u8 rating;

    //the position
    irr::u8 racePosition;
};

/************************
 * Forward declarations *
 ************************/

class Game;
class Ray;
class MyMusicStream;
class Player;
struct WayPointLinkInfoStruct;
struct CheckPointInfoStruct;
struct ExtendedRegionInfoStruct;
struct MapTileRegionStruct;
struct LineStruct;
class PhysicsObject;
class SoundEngine;
class Physics;
class Bezier;
class Cone;
class Recovery;
class ExplosionLauncher;
class ExplosionEntity;
class HUD;
class Timer;
class WorldAwareness;
class CollectableSpawner;
class Path;
class Camera;
class ChargingStation;
class SteamFountain;
class LevelTerrain;
class LevelBlocks;
class Morph;
class TextureLoader;
class DrawDebug;
class LevelFile;
class Collectable;
struct ColorStruct;
class ShaderCallBack;
struct MapConfigStruct;

class Race {
public:
    Race(Game* parentGame, MyMusicStream* gameMusicPlayerParam, SoundEngine* soundEngine,
         std::string levelRootPath, std::string levelName, irr::u8 nrLaps, bool demoMode, bool attributionActive,
         bool skipStart);

    ~Race();

    bool ready;

    void HandleInput(irr::f32 deltaTime);
    void HandleBasicInput();
    void HandleComputerPlayers(irr::f32 frameDeltaTime);
    void Render();
    void DrawHUD(irr::f32 frameDeltaTime);
    void DrawMiniMap(irr::f32 frameDeltaTime);
    void AdvanceTime(irr::f32 frameDeltaTime);
    void Init();
    void AddPlayer(bool humanPlayer, char* name, std::string player_model);
    void End();

    std::vector<RaceStatsEntryStruct*>* RetrieveFinalRaceStatistics();
    void CleanupRaceStatistics(std::vector<RaceStatsEntryStruct*>* pntr);

    std::vector<irr::core::vector3df> mPlayerStartLocations;

    void HandleCraftHeightMapCollisions(irr::f32 deltaTime, PhysicsObject* whichObj);

    void CallRecoveryVehicleForHelp(Player* whichPlayer);

    //returns an available recovery vehicle for physics reset
    //if no recovery vehicle is available returns nullptr
    Recovery* FindRecoveryVehicleForPhysicsReset(irr::core::vector3df dropOffPointAfterReset);

    bool exitRace = false;

    //handles the height map terrain
    //of the level
    LevelTerrain *mLevelTerrain = nullptr;

    Bezier *testBezier = nullptr;

    Ray *mRay = nullptr;

    SoundEngine* mSoundEngine = nullptr;

    std::vector<LineStruct*> *ENTWallsegmentsLine_List = nullptr;

    irr::f32 GetAbsOrientationAngleFromDirectionVec(irr::core::vector3df dirVector, bool correctAngleOutsideRange = true);
    void CheckPlayerCrossedCheckPoint(Player* whichPlayer, irr::core::aabbox3d<irr::f32> playerBox);

    //attacker is the enemy player that does damage the player targetToHit
    //for damage that an entity does cause (for example steamFountain) attacker is set
    //to nullptr
    void DamagePlayer(Player* targetToHit, irr::f32 damageVal, irr::u8 damageType, Player* attacker = nullptr);

    Player* currPlayerFollow = nullptr;

    //handles the columns (made of blocks)
    //of the level
    LevelBlocks *mLevelBlocks = nullptr;

    //void TestVoxels();
    //int getNrTrianglesCollected();
    //int getNrHitTrianglesRay();

    //std::vector<RayHitTriangleInfoStruct*> TestRayTrianglesSelector;

    //my physics
    Physics *mPhysics = nullptr;

    //my explosion launcher
    ExplosionLauncher* mExplosionLauncher = nullptr;

    //vector of all available explosions in
    //this map
    std::vector<ExplosionEntity*> mExplosionEntityVec;

    HUD *Hud1Player = nullptr;

    bool DebugHitBreakpoint = false;

    //debugging function which allows to draw a rectangle around a selected
    //tile of the heightmap of the terrain level
    void DebugDrawHeightMapTileOutline(int x, int z, ColorStruct* color);
    void PlayerHasFinishedLastLapOfRace(Player *whichPlayer);

    //this list contains all players that have already
    //finished the race in the order how the have finished
    std::vector<Player*> playerRaceFinishedVec;

    //object for pathfinding and services
    Path* mPath = nullptr;

    irr::core::vector3df dbgCoord;

    std::vector<EntityItem*> *ENTWaypoints_List = nullptr;
    std::vector<WayPointLinkInfoStruct*> *wayPointLinkVec = nullptr;

    std::vector<WayPointLinkInfoStruct*> testPathResult;
    WayPointLinkInfoStruct* dbgFirstLink = nullptr;

    irr::core::vector3df topRaceTrackerPointerOrigin;

    Game* mGame = nullptr;

    std::vector<Collectable*> *ENTCollectablesVec = nullptr;

    //vector of players in this race
    std::vector<Player*> mPlayerVec;

    //vector of craft/missile trigger regions
    std::vector<MapTileRegionStruct*> mTriggerRegionVec;

    void PlayerEnteredCraftTriggerRegion(Player* whichPlayer, MapTileRegionStruct* whichRegion);
    void PlayerMissileHitMissileTrigger(Player* whichPlayer, MapTileRegionStruct* whichRegion);
    void TimedTriggerOccured(Timer* whichTimer);

    //irr::core::vector3df dbgMiniMapPnt1;
    //irr::core::vector3df dbgMiniMapPnt2;
    //irr::core::vector3df dbgMiniMapPnt3;
    //irr::core::vector3df dbgMiniMapPnt4;

    //function for debugging
    void DebugSelectPlayer(int whichPlayerNr);

    void SetupPhysicsObjectParameters(PhysicsObject &phyObj, bool humanPlayer);

    //in demo mode the following bool
    //is true, in normal game mode the bool
    //is false
    bool mDemoMode;

    void PlayerCrossesFinishLineTheFirstTime();

    bool RaceAllowsPlayersToAttack();

    /*void SetPlayerLocationAndAlignToTrackHeight(Player* player, irr::core::vector3df newLocation,
                                                irr::core::vector3df newFrontDirVec);*/

    //returns filename of sprite file for collectable
    //invalid entity types will revert to sprite number 42
    irr::u16 GetCollectableSpriteNumber(Entity::EntityType mEntityType);

    bool GetWasRaceFinished();

    //my texture loader
    TextureLoader *mTexLoader = nullptr;

    //stores the detected minimap race track positions
    //referenced to Terrain tile coordinates from level
    //file. We need this information later to be able
    //to convert player level coordinates to 2D minimap
    //positions
   /* irr::u32 miniMapStartW;
    irr::u32 miniMapEndW;
    irr::u32 miniMapStartH;
    irr::u32 miniMapEndH;*/

    irr::u8 mPlayersInTrouble;

    std::vector<ExtendedRegionInfoStruct*>* mExtRegionVec = nullptr;

    //a vector of my existing charging stations
    std::vector<ChargingStation*>* mChargingStationVec = nullptr;

    ShaderCallBack* shaderCallBack;

    //Is public, because game loop needs to be
    //able to stop the race in attribution mode
    void InitiateExitRace();

private:
    std::string mLevelRootPath;
    std::string mLevelName;

    bool mMiniMapInitOk;

    MapConfigStruct* mMapConfig = nullptr;

    void HandleExitRace();

    //Returns true in case of success, False
    //otherwise
    bool SetupSky();

    irr::s32 shaderMaterial1;

    //ShadowMap settings
    E_FILTER_TYPE mShadowMapFilterType;
    irr::u32 mShadowMapResolution;

    SShadowLight* mShadowLight = nullptr;

    //the image for the base of the minimap
    //without the player location dots
    irr::video::ITexture* baseMiniMap = nullptr;
    irr::core::dimension2di miniMapSize;
    irr::core::vector2d<irr::s32> miniMapDrawLocation;
    MyMusicStream* mMusicPlayer = nullptr;

    irr::u8 mRaceNumberOfLaps;

    void SetupTopRaceTrackPointerOrigin();

    //holds the pixels coordinates inside the minimap texture
    //image which are actually used by non transparent pixels
    irr::core::rect<irr::s32> miniMapImageUsedArea;

    //precalculated value for miniMap so that we safe
    //unnecessary calculations during game
    irr::f32 miniMapPixelPerCellW;
    irr::f32 miniMapPixelPerCellH;

    irr::f32 miniMapAbsTime = 0.0f;
    bool miniMapBlinkActive = false;

    irr::core::dimension2di CalcPlayerMiniMapPosition(Player* whichPlayer);

    //Initializes the games original
    //minimap
    //returns true for success, false otherwise
    bool InitMiniMap();

    //Searches for the used space inside the minimap picture
    //while removing unnecessary transparent columns of pixels
    //Parameters:
    //  miniMapTexture = pointer to the minimap texture
    //In case of an unexpected error this function returns succesFlag = false, True otherwise
    irr::core::rect<irr::s32> FindMiniMapImageUsedArea(irr::video::ITexture* miniMapTexture, bool &succesFlag);

    //handles the file data structure of the
    //level
    LevelFile *mLevelRes = nullptr;

    //my sky image for the level background
    irr::video::ITexture* mSkyImage = nullptr;

    //class for world awareness functions
    //which are needed by computer player control functions
    WorldAwareness* mWorldAware = nullptr;

    //the main player object

    //Player physics object
    PhysicsObject* playerPhysicsObj = nullptr;

    //vector for player physic objects
    std::vector<PhysicsObject*> mPlayerPhysicObjVec;

    std::vector<Player*> playerRanking;

    irr::scene::IMeshSceneNode* testcube = nullptr;

    //my camera
    irr::scene::ICameraSceneNode* mCamera = nullptr;

    //stores the currently active camera
    irr::scene::ICameraSceneNode* currActiveCamera = nullptr;

    //if all morphs should be executed
    //set to true
    bool runMorph = false;
    irr::f32 absTimeMorph = 0.0f;

    //if true the camera is inside player1 craft
    //if false we have a free-moving camera for debugging and
    //development
    bool playerCamera = true;

    //variables to switch different debugging functions on and off
    bool DebugShowWaypoints = false;
    bool DebugShowFreeMovementSpace = false;

    bool DebugShowWallCollisionMesh = false;
    bool DebugShowCheckpoints = false;
    bool DebugShowWallSegments = false;
    bool DebugShowRegionsAndPointOfInterest = false;
    bool DebugShowTriggerRegions = false;
    bool DebugShowTriggerEvents = false;
    bool AllowStartMorphsPerKey = false;

    void createEntity(EntityItem *p_entity, LevelFile *levelRes, LevelTerrain *levelTerrain, LevelBlocks* levelBlocks, irr::video::IVideoDriver *driver);
    bool LoadSkyImage(irr::video::IVideoDriver* driver, irr::core::dimension2d<irr::u32> screenResolution);
    bool LoadLevel();
    void createLevelEntities();

    void DrawSky();
    void DrawTestShape();

    void draw2DImage(irr::video::IVideoDriver *driver, irr::video::ITexture* texture ,
         irr::core::rect<irr::s32> sourceRect, irr::core::position2d<irr::s32> position,
         irr::core::position2d<irr::s32> rotationPoint, irr::f32 rotation, irr::core::vector2df scale, bool useAlphaChannel, irr::video::SColor color);

    void removePlayerTest();

    void IrrlichtStats(char* text);

    void AddCheckPoint(EntityItem entity);
    void CheckPointPostProcessing();

    void AddExplosionEntity(EntityItem *entity);

    void AddWayPoint(EntityItem *entity, EntityItem *next);

    std::list<EntityItem*> *ENTWallsegments_List = nullptr;
    std::list<EntityItem*> *ENTTriggers_List = nullptr;

    //holds a list of all available level morphs
    std::list<Morph*> Morphs;

    void createWallCollisionData();
    void createFinalCollisionData();

    void DebugResetColorAllWayPointLinksToWhite();

    //holds a generated Mesh for wall collision detection
    irr::scene::SMesh* wallCollisionMesh = nullptr;

    //holds the OctreeSceneNode for the wall collision detection
    //we use this for trianglePicking later
    irr::scene::ISceneNode *wallCollisionMeshSceneNode = nullptr;

    //the necessary triangle selectors for craft collision detection
    irr::scene::ITriangleSelector* triangleSelectorWallCollision = nullptr;
    irr::scene::ITriangleSelector* triangleSelectorColumnswCollision = nullptr;
    irr::scene::ITriangleSelector* triangleSelectorColumnswoCollision = nullptr;

    //necessary triangle selector for raycasting onto terrain
    irr::scene::ITriangleSelector* triangleSelectorStaticTerrain = nullptr;
    irr::scene::ITriangleSelector* triangleSelectorDynamicTerrain = nullptr;

    void createCheckpointMeshData(CheckPointInfoStruct &newStruct);
    std::vector<CheckPointInfoStruct*> *checkPointVec = nullptr;

    void UpdatePlayerDistanceToNextCheckpoint(Player* whichPlayer);
    void UpdatePlayerRacePositionRanking();
    void UpdatePlayerRacePositionRankingHelper2(std::vector< pair <irr::s32, Player*> > vecNextCheckPointExpected);
    void UpdatePlayerRacePositionRankingHelper3(std::vector< pair <irr::f32, Player*> > vecRemainingDistanceToNextCheckpoint);

    void CheckPlayerCollidedCollectible(Player* player, irr::core::aabbox3d<irr::f32> playerBox);

    //my vector of SteamFountains
    std::vector<SteamFountain*>* steamFountainVec = nullptr;

    //my vector of recovery vehicles
    std::vector<Recovery*>* recoveryVec = nullptr;

    //vector of players that currently need help of
    //a recovery vehicle
    std::vector<Player*>* mPlayerWaitForRecoveryVec = nullptr;

    void UpdateRecoveryVehicles(irr::f32 deltaTime);

    //my vector of cones
    std::vector<Cone*>* coneVec = nullptr;

    std::vector<irr::video::SColor*> mMiniMapMarkerColors;

    void IndicateTriggerRegions();

    void AddTrigger(EntityItem *entity);

    void UpdateParticleSystems(irr::f32 frameDeltaTime);
    void UpdateMorphs(irr::f32 frameDeltaTime);
    void UpdateTimers(irr::f32 frameDeltaTime);
    void UpdateCones(irr::f32 frameDeltaTime);
    void UpdateExternalCameras();

    std::vector<Timer*> mTimerVec;
    void AddTimer(EntityItem *entity);

    //holds currently pending trigger target group events (when some
    //entity reports that something should be triggered it
    //is stored in this list), until the next Race update is done
    std::vector<int16_t> mPendingTriggerTargetGroups;

    //vector of predefined camera locations for demo mode
    //positions are stored inside the level files
    std::vector<Camera*> mCameraVec;

    void AddCamera(EntityItem* entity);
    void SetExternalViewAtPlayer();
    void ManagePlayerCamera();

    Player* mFollowPlayerDemoMode = nullptr;
    irr::f32 mFollowPlayerDemoModeTimeCounter = 0.0f;

    void ManageCameraDemoMode(irr::f32 deltaTime);
    void FindNextPlayerToFollowInDemoMode();
    void UpdateSoundListener();

    void ProcessPendingTriggers();

    void CheckRaceFinished(irr::f32 deltaTime);

    //Audio related stuff
    void StopMusic();
    void StopAllSounds();

    void DebugShowAllObstaclePlayers();

    //cleanup-stuff
    void CleanUpAllCheckpoints();
    void CleanUpWayPointLinks(std::vector<WayPointLinkInfoStruct*> &vec);
    void CleanUpSteamFountains();
    void CleanUpEntities();
    void CleanUpRecoveryVehicles();
    void CleanUpCones();
    void CleanUpMorphs();
    void CleanUpSky();
    void CleanMiniMap();
    void CleanUpTriggers();
    void CleanUpTimers();
    void CleanUpExplosionEntities();
    void CleanUpCameras();
    void CleanUpCollectableSpawners();
    void CleanupChargingStations();

    void DebugDrawWayPointLinks(bool drawFreeMovementSpace = false);

    irr::f32 mRaceFinishedWaitTimeCnter = 0.0f;
    bool mRaceWasFinished = false;

    irr::u8 mCurrentPhase;

    void ControlStartPhase(irr::f32 frameDeltaTime);
    irr::f32 mStartPhaseTimeCounter = 0.0f;

    std::vector<CollectableSpawner*> mCollectableSpawnerVec;
    void UpdateCollectableSpawners(irr::f32 frameDeltaTime);
    void SpawnCollectiblesForPlayer(Player* player);

    void UpdateType2Collectables(irr::f32 frameDeltaTime);

    std::vector<Collectable*> mType2CollectableForCleanupLater;

    void CreateChargingStations();

    //for improved sky
    irr::scene::ISceneNode* skydomeNode = nullptr;

    irr::scene::CLensFlareSceneNode* flare = nullptr;

    irr::scene::CCloudSceneNode* cloudLayer1 = nullptr;
    irr::scene::CCloudSceneNode* cloudLayer2 = nullptr;
    irr::scene::CCloudSceneNode* cloudLayer3 = nullptr;

    void UpdateShadowLights();

    //if true attribution mode is enabled
    //This means the race (demo) is only interrupted
    //by either the user (with Escape key), or by the game
    //loop itself (when attribution is finished); The
    //finished race does not end the demo (attribution mode)
    bool mAttributionMode;
};

#endif // RACE_H
