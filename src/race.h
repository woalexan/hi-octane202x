/*
 The source code of function createEntity in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself (and later modified by me)
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 The source code for function draw2DImage was taken from:  (most likely from user Lonesome Ducky)
 https://irrlicht.sourceforge.io/forum/viewtopic.php?t=43565
 https://irrlicht.sourceforge.io/forum//viewtopic.php?p=246138#246138

 Copyright (C) 2024 Wolf Alexander
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code of function createEntity, later modified by me)
 
 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef RACE_H
#define RACE_H

#include "models/cone.h"
#include "models/morph.h"
#include "models/recovery.h"
#include "draw/drawdebug.h"
#include "models/player.h"
#include "draw/gametext.h"
#include "input/input.h"
#include "audio/music.h"
#include "audio/sound.h"
#include "utils/worldaware.h"
#include "utils/tprofile.h"
#include "utils/path.h"
#include "game.h"
#include "models/explauncher.h"
#include "models/timer.h"
#include "models/expentity.h"
#include "models/camera.h"

using namespace std;

//after race is finished (all players finished last lap)
//the exit of race is delayed by this amount of time in seconds
const irr::f32 DEF_RACE_FINISHED_WAITTIME_SEC = 7.0f;

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

    //the rating number up to 20
    irr::u8 rating;

    //the position
    irr::u8 racePosition;
};

class Player; //Forward declaration
class HUD; //Forward declaration
class WorldAwareness; //Forward declaration
class SteamFountain; //Forward declaration
class LevelTerrain; //Forward declaration
class LevelBlocks; //Forward declaration
class Physics; //Forward declaration
class PhysicsObject; //Forward declaration
class Recovery; //Forward declaration
class Bezier; //Forward declaration
class Path; //Forward declaration
struct CheckPointInfoStruct; //Forward declaration
class Game; //Forward declaration
struct WayPointLinkInfoStruct; //Forward declaration
class ExplosionLauncher;  //Forward declaration
class Morph;   //Forward declaration
class Timer;   //Forward declaration
class ExplosionEntity; //Forward declaration
class Cone; //Forward declaration
class Camera; //Forward declaration

class Race {
public:
    Race(irr::IrrlichtDevice* device, irr::video::IVideoDriver *driver, irr::scene::ISceneManager* smgr, MyEventReceiver* eventReceiver,
         GameText* gameText, Game* mParentGame, MyMusicStream* gameMusicPlayerParam, SoundEngine* soundEngine, TimeProfiler* timeProfiler,
         dimension2d<u32> gameScreenRes, int loadLevelNr, bool demoMode, bool useAutoGenMiniMapParam = false);

    ~Race();

    bool ready;

    void HandleInput();
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

    void HandleCraftHeightMapCollisions();

    void CallRecoveryVehicleForHelp(Player* whichPlayer);

    bool exitRace = false;

    //handles the height map terrain
    //of the level
    LevelTerrain *mLevelTerrain;

    Bezier *testBezier;

    SoundEngine* mSoundEngine;

    std::vector<LineStruct*> *ENTWallsegmentsLine_List;

    //lets create and store a direction vector for
    //later use (calculations), so that we do not have to do this
    //over and over again
    irr::core::vector3d<irr::f32>* xAxisDirVector;
    irr::core::vector3d<irr::f32>* yAxisDirVector;
    irr::core::vector3d<irr::f32>* zAxisDirVector;

    irr::f32 GetAbsOrientationAngleFromDirectionVec(irr::core::vector3df dirVector, bool correctAngleOutsideRange = true);
    void CheckPlayerCrossedCheckPoint(Player* whichPlayer, irr::core::aabbox3d<f32> playerBox);

    //attacker is the enemy player that does damage the player targetToHit
    //for damage that an entity does cause (for example steamFountain) attacker is set
    //to NULL
    void DamagePlayer(Player* targetToHit, irr::f32 damageVal, Player* attacker = NULL);

    Player* currPlayerFollow = NULL;

    //my Irrlicht video driver
    irr::video::IVideoDriver *mDriver;

    //my Irrlicht scene manager
    scene::ISceneManager *mSmgr;

    //my Irrlicht device
    irr::IrrlichtDevice* mDevice;

    //handles the columns (made of blocks)
    //of the level
    LevelBlocks *mLevelBlocks;

    //void TestVoxels();
    //int getNrTrianglesCollected();
    //int getNrHitTrianglesRay();

    //std::vector<RayHitTriangleInfoStruct*> TestRayTrianglesSelector;

    //my physics
    Physics *mPhysics;

    //my explosion launcher
    ExplosionLauncher* mExplosionLauncher;

    //vector of all available explosions in
    //this map
    std::vector<ExplosionEntity*> mExplosionEntityVec;

    HUD *Hud1Player;

    bool DebugHitBreakpoint = false;

    //debugging function which allows to draw a rectangle around a selected
    //tile of the heightmap of the terrain level
    void DebugDrawHeightMapTileOutline(int x, int z, irr::video::SMaterial* color);
    void PlayerHasFinishedLastLapOfRace(Player *whichPlayer);

    //this list contains all players that have already
    //finished the race in the order how the have finished
    std::vector<Player*> playerRaceFinishedVec;

    //my drawDebug object
    DrawDebug *mDrawDebug;

    //object for pathfinding and services
    Path* mPath;

    irr::core::vector3df dbgCoord;

    std::vector<EntityItem*> *ENTWaypoints_List;
    std::vector<WayPointLinkInfoStruct*> *wayPointLinkVec;

    std::vector<WayPointLinkInfoStruct*> testPathResult;
    WayPointLinkInfoStruct* dbgFirstLink = NULL;

    irr::core::vector3df topRaceTrackerPointerOrigin;

    Game* mGame;

    std::vector<Collectable*> *ENTCollectablesVec;

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

    //If true enables computer player stuck detection
    //turn off for testing computer player movement
    //performance and stabilitiy testing to keep computer
    //players stuck; Set true for final release to handle
    //random cases where computer players really get stuck, so that
    //races can for sure finish
    bool CpEnableStuckDetection = true;

    //function for debugging
    void DebugSelectPlayer(int whichPlayerNr);

    void SetupPhysicsObjectParameters(PhysicsObject &phyObj, bool humanPlayer);

private:
    int levelNr;
    bool useAutoGenMinimap;

    //the image for the base of the minimap
    //without the player location dots
    irr::video::ITexture* baseMiniMap;
    irr::core::dimension2di miniMapSize;
    irr::core::vector2d<irr::s32> miniMapDrawLocation;
    MyMusicStream* mMusicPlayer;

    void SetupTopRaceTrackPointerOrigin();

    //stores the detected minimap race track positions
    //referenced to Terrain tile coordinates from level
    //file. We need this information later to be able
    //to convert player level coordinates to 2D minimap
    //positions
    irr::u32 miniMapStartW;
    irr::u32 miniMapEndW;
    irr::u32 miniMapStartH;
    irr::u32 miniMapEndH;

    //holds the pixels coordinates inside the minimap texture
    //image which are actually used by non transparent pixels
    irr::core::rect<irr::s32> miniMapImageUsedArea;

    //precalculated value for miniMap so that we safe
    //unnecessary calculations during game
    irr::f32 miniMapPixelPerCellW;
    irr::f32 miniMapPixelPerCellH;

    irr::f32 miniMapAbsTime = 0.0f;
    bool miniMapBlinkActive = false;

    void InitMiniMap(irr::u32 levelNr);
    irr::core::dimension2di CalcPlayerMiniMapPosition(Player* whichPlayer);

    //Initializes the games original
    //minimap
    void InitMiniMapOriginal(irr::u32 levelNr);

    //Searches for the used space inside the minimap picture
    //while removing unnecessary transparent columns of pixels
    //Parameters:
    //  miniMapTexture = pointer to the minimap texture
    //In case of an unexpected error this function returns succesFlag = false, True otherwise
    irr::core::rect<irr::s32> FindMiniMapImageUsedArea(irr::video::ITexture* miniMapTexture, bool &succesFlag);

    //handles the file data structure of the
    //level
    LevelFile *mLevelRes;

    TimeProfiler* mTimeProfiler;

    //my Irrlicht event receiver
    MyEventReceiver* mEventReceiver;

    //my game Text object pointer
    GameText* mGameText;

    //the game screen resolution
    dimension2d<u32> mGameScreenRes;

    //my sky image for the level background
    irr::video::ITexture* mSkyImage = NULL;

    //my texture loader
    TextureLoader *mTexLoader;

    //class for world awareness functions
    //which are needed by computer player control functions
    WorldAwareness* mWorldAware;

    //the main player object

    //Player physics object
    PhysicsObject* playerPhysicsObj;

    //vector for player physic objects
    std::vector<PhysicsObject*> mPlayerPhysicObjVec;

    std::vector<Player*> playerRanking;

    //my camera
    scene::ICameraSceneNode* mCamera;

    //stores the currently active camera
    scene::ICameraSceneNode* currActiveCamera = NULL;

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
    bool LoadSkyImage(int levelNr, irr::video::IVideoDriver* driver, irr::core::dimension2d<irr::u32> screenResolution);
    bool LoadLevel(int loadLevelNr);
    void createLevelEntities();
    void getPlayerStartPosition(int levelNr, irr::core::vector3d<irr::f32> &startPos, irr::core::vector3d<irr::f32> &startDirection);

    void DrawSky();
    void DrawTestShape();

    void IndicateMapRegions();

    void draw2DImage(irr::video::IVideoDriver *driver, irr::video::ITexture* texture ,
         irr::core::rect<irr::s32> sourceRect, irr::core::position2d<irr::s32> position,
         irr::core::position2d<irr::s32> rotationPoint, irr::f32 rotation, irr::core::vector2df scale, bool useAlphaChannel, irr::video::SColor color);

    void removePlayerTest();

    void IrrlichtStats(char* text);

    void AddCheckPoint(EntityItem entity);
    void CheckPointPostProcessing();

    void AddExplosionEntity(EntityItem *entity);

    void AddWayPoint(EntityItem *entity, EntityItem *next);

    std::list<EntityItem*> *ENTWallsegments_List;
    std::list<EntityItem*> *ENTTriggers_List;

    //holds a list of all available level morphs
    std::list<Morph*> Morphs;

    void createWallCollisionData();
    void createFinalCollisionData();

    void DebugResetColorAllWayPointLinksToWhite();

    //holds a generated Mesh for wall collision detection
    irr::scene::SMesh* wallCollisionMesh;

    //holds the OctreeSceneNode for the wall collision detection
    //we use this for trianglePicking later
    irr::scene::ISceneNode *wallCollisionMeshSceneNode;

    //the necessary triangle selectors for craft collision detection
    irr::scene::ITriangleSelector* triangleSelectorWallCollision = NULL;
    irr::scene::ITriangleSelector* triangleSelectorColumnswCollision = NULL;
    irr::scene::ITriangleSelector* triangleSelectorColumnswoCollision = NULL;

    //necessary triangle selector for raycasting onto terrain
    irr::scene::ITriangleSelector* triangleSelectorStaticTerrain = NULL;
    irr::scene::ITriangleSelector* triangleSelectorDynamicTerrain = NULL;

    void createCheckpointMeshData(CheckPointInfoStruct &newStruct);
    std::vector<CheckPointInfoStruct*> *checkPointVec;

    void UpdatePlayerDistanceToNextCheckpoint(Player* whichPlayer);
    void UpdatePlayerRacePositionRanking();
    void UpdatePlayerRacePositionRankingHelper2(std::vector< pair <irr::s32, Player*> > vecNextCheckPointExpected);
    void UpdatePlayerRacePositionRankingHelper3(std::vector< pair <irr::f32, Player*> > vecRemainingDistanceToNextCheckpoint);

    void CheckPlayerCollidedCollectible(Player* player, irr::core::aabbox3d<f32> playerBox);

    //my vector of SteamFountains
    std::vector<SteamFountain*>* steamFountainVec;

    //my vector of recovery vehicles
    std::vector<Recovery*>* recoveryVec;

    //vector of players that currently need help of
    //a recovery vehicle
    std::vector<Player*>* mPlayerWaitForRecoveryVec;

    void UpdateRecoveryVehicles(irr::f32 deltaTime);

    //my vector of cones
    std::vector<Cone*>* coneVec;

    std::vector<irr::video::SColor*> mMiniMapMarkerColors;

    void IndicateTriggerRegions();

    void AddTrigger(EntityItem *entity);

    void UpdateParticleSystems(irr::f32 frameDeltaTime);
    void UpdateMorphs(irr::f32 frameDeltaTime);
    void UpdateTimers(irr::f32 frameDeltaTime);
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

    //in demo mode the following bool
    //is true, in normal game mode the bool
    //is false
    bool mDemoMode;

    void AddCamera(EntityItem* entity);
    void SetExternalViewAtPlayer();
    void ManagePlayerCamera();

    void ProcessPendingTriggers();

    void CheckRaceFinished(irr::f32 deltaTime);

    //Audio related stuff
    void DeliverMusicFileName(unsigned int levelNr, char *musicFileName);
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

    void DebugDrawWayPointLinks(bool drawFreeMovementSpace = false);

    irr::f32 mRaceFinishedWaitTimeCnter = 0.0f;
    bool mRaceWasFinished = false;
};

#endif // RACE_H
