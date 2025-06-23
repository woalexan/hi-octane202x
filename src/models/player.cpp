/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "player.h"
#include "../game.h"
#include "../utils/physics.h"
#include "../utils/ray.h"
#include "levelterrain.h"
#include "levelblocks.h"
#include "cpuplayer.h"
#include "mgun.h"
#include "missile.h"
#include "particle.h"
#include "../audio/sound.h"
#include "../draw/hud.h"
#include "../utils/logger.h"
#include "camera.h"
#include "../utils/movingavg.h"
#include "../race.h"
#include "collectable.h"
#include "../resources/mapentry.h"

void Player::SetPlayerObject(PhysicsObject* phObjPtr) {
   this->phobj = phObjPtr;
}

void Player::DamageGlas() {
    //with a certain probability damage glas if another player
    //shoots with the machine gun at me
    irr::s32 rNum = rand();
    irr::f32 rFloat = (float(rNum) / float (RAND_MAX));

    //with 10% probability damage glas of player HUD
    if (rFloat < 0.1f) {
       if (mHUD != nullptr) {
        AddGlasBreak();
       }
    }
}

void Player::CrossedCheckPoint(irr::s32 valueCrossedCheckPoint, irr::s32 numberOfCheckpoints) {    
    //if this player has already finished the race ignore checkpoints
    if (mPlayerStats->mHasFinishedRace)
        return;

    //crossed checkpoint is the one we need to cross next?
    if (this->nextCheckPointValue == valueCrossedCheckPoint) {
        //did we cross the finish line the first time after start?
        //if so we need to advance the race state
        if ((lastCrossedCheckPointValue == 0) && (valueCrossedCheckPoint == 0)) {
            this->mRace->PlayerCrossesFinishLineTheFirstTime();
        }

        //did the cross the finish line?
        if ((lastCrossedCheckPointValue !=0) && (valueCrossedCheckPoint == 0)) {
            //we finished a complete lap
            this->FinishedLap();
        }

        //calculate next checkpoint target value
        nextCheckPointValue++;

        //if we exceed number of available waypoints
        //the next expected waypoint is the finish line again (with value 0)
        if (nextCheckPointValue > (numberOfCheckpoints - 1)) {
            nextCheckPointValue = 0;
        }

        //remember value of last crossed way point
        lastCrossedCheckPointValue = valueCrossedCheckPoint;
    }
}

//delivers a random machine gun shoot location at the area of the
//player craft model if the shoot does hit the player (shootDoesHit = true)
//in case shoot does not hit, delivers a random target location around the player
//at the terrain
irr::core::vector3df Player::GetRandomMGunShootTargetLocation(bool shootDoesHit) {
    irr::core::vector3df randLocation;

    if (shootDoesHit) {
        //shoot does hit, return random location at player craft model
        randLocation.set(mPlayerModelExtend.X * this->mRace->mGame->randFloat(),
                         mPlayerModelExtend.Y * this->mRace->mGame->randFloat(),
                         mPlayerModelExtend.Z * this->mRace->mGame->randFloat());

        randLocation -= mPlayerModelExtend * irr::core::vector3df(0.5f, 0.5f, 0.5f);
        randLocation += this->phobj->physicState.position;
    } else {
        //shoot does not hit, return random location around player at the terrain
        randLocation.set(this->mRace->mGame->randFloat(), 0.0f, this->mRace->mGame->randFloat());

        randLocation -= irr::core::vector3df(0.5f, 0.0f, 0.5f);
        randLocation += this->phobj->physicState.position;

        irr::core::vector2di outCell;

        //get final Y coord from Terrain at the random location
        randLocation.Y = this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(randLocation. X, randLocation.Z, outCell);
    }

    return randLocation;
}

//returns integer with value between 0 and 100
//percent
irr::u32 Player::GetMGunHitProbability() {
    //the hit probability in percent
    //does depend on the quality of the current
    //target lock of the target player
    irr::f32 probability = DEF_PLAYER_MGUN_MINHIT_PROB;

    //mTargetMissleLockProgr value goes from 22 down to 0 (full lock)
    probability += ((DEF_PLAYER_MGUN_MAXHIT_PROB - DEF_PLAYER_MGUN_MINHIT_PROB)/22.0f) * (irr::f32)(22 - mTargetMissleLockProgr);

    if (probability > 100.0f)
        probability = 100.0f;

    if (probability < 0.0f)
        probability = 0.0f;

    return (irr::u32)(probability);
}

//Get current weapon shooting target for this player
//Returns true if there was a target found, False otherwise
bool Player::GetWeaponTarget(RayHitTriangleInfoStruct &shotTarget) {
    //built a ray cast 3d line from physics object position to direction we want to go
    irr::core::vector3df startPnt(this->phobj->physicState.position);
    irr::core::vector3df endPnt(this->phobj->physicState.position + this->craftForwardDirVec * irr::core::vector3df(50.0f, 50.0f, 50.0f));

    std::vector<irr::core::vector3di> voxels;

    std::vector<RayHitTriangleInfoStruct*> allHitTriangles;

    //with ReturnOnlyClosestTriangles = true!
    allHitTriangles = this->mRace->mRay->ReturnTrianglesHitByRay( this->mRace->mRay->mRayTargetSelectors,
                                  startPnt, endPnt, 1, true);

    int vecSize = (int)(allHitTriangles.size());
    std::vector<RayHitTriangleInfoStruct*>::iterator it;

    if (vecSize < 1)
        return false;

    if (vecSize == 1) {
        it = allHitTriangles.begin();
        //only one target found, no shorting over distance necessary
        shotTarget = *(*it);

        //cleanup triangle hit information again
        //otherwise we have a memory leak!
        this->mRace->mRay->EmptyTriangleHitInfoVector(allHitTriangles);

        return true;
    }

    //we have more then one triangle, we need to find the closest one to the player
    irr::f32 minDistance;
    bool firstElement = true;
    irr::f32 currDist;
    RayHitTriangleInfoStruct* nearestTriangleHit;

    for (it = allHitTriangles.begin(); it != allHitTriangles.end(); ++it) {
        currDist = (*it)->distFromRayStartSquared;
        if (firstElement) {
            firstElement = false;
            minDistance = currDist;
            nearestTriangleHit = (*it);
        } else if (currDist < minDistance) {
            minDistance = currDist;
            nearestTriangleHit = (*it);
        }
    }

    shotTarget = *nearestTriangleHit;

    //cleanup triangle hit information again
    //otherwise we have a memory leak!
    this->mRace->mRay->EmptyTriangleHitInfoVector(allHitTriangles);

    return true;
}

Player::~Player() {
    //free memory of all player stats
    delete mPlayerStats;
    mPlayerStats = nullptr;

    delete mFinalPlayerStats;
    mFinalPlayerStats = nullptr;

    //Remove my Scenenode from
    //Scenemanager
    if (this->Player_node != nullptr) {
        Player_node->remove();
        Player_node = nullptr;
    }

    //Remove my player Mesh
    if (this->PlayerMesh != nullptr) {
       mRace->mGame->mSmgr->getMeshCache()->removeMesh(this->PlayerMesh);
       this->PlayerMesh = nullptr;
    }

    //free my SmokeTrail particle system
    delete mSmokeTrail;
    mSmokeTrail = nullptr;

    //free my Dust cloud emitter particles system
    delete mDustBelowCraft;
    mDustBelowCraft = nullptr;

    //free my machinegun
    delete mMGun;
    mMGun = nullptr;

    //free my missile launcher
    delete mMissileLauncher;
    mMissileLauncher = nullptr;

    delete dirtTexIdsVec;
    dirtTexIdsVec = nullptr;

    //free the height map collision stuff
    delete this->mHMapCollPntData.front;
    delete this->mHMapCollPntData.frontLeft45deg;
    delete this->mHMapCollPntData.frontRight45deg;
    delete this->mHMapCollPntData.left;
    delete this->mHMapCollPntData.right;
    delete this->mHMapCollPntData.backLeft45deg;
    delete this->mHMapCollPntData.backRight45deg;
    delete this->mHMapCollPntData.back;

    delete this->cameraSensor;
    delete this->cameraSensor2;
    delete this->cameraSensor3;
    delete this->cameraSensor4;
    delete this->cameraSensor5;
    delete this->cameraSensor6;
    delete this->cameraSensor7;

    CleanUpBrokenGlas();
    delete this->brokenGlasVec;

    //delete my cpu player
    delete mCpuPlayer;

    delete mMovingAvgPlayerLeaningAngleLeftRightCalc;
    //delete mMovingAvgPlayerPositionCalc;

    //delete my camera SceneNodes
    this->mIntCamera->remove();
    this->mThirdPersonCamera->remove();
}

void Player::SetNewState(irr::u32 newPlayerState) {
    this->mPlayerStats->mPlayerCurrentState = newPlayerState;

    switch (newPlayerState) {
        case STATE_PLAYER_BEFORESTART: {
            this->mPlayerStats->mPlayerCanMove = false;
            this->mPlayerStats->mPlayerCanShoot = false;
            break;
        }

        //This is the inbetween state after green light comes on
        //and the first time a player crosses the finish line
        //in this state the players move towards the start line, and
        //computer players do not seem to attack
        //Also the HUD is not shown yet
        case STATE_PLAYER_ONFIRSTWAYTOFINISHLINE: {
            this->mPlayerStats->mPlayerCanMove = true;
            this->mPlayerStats->mPlayerCanShoot = false;
            break;
        }

        case STATE_PLAYER_RACING: {
            this->mPlayerStats->mPlayerCanMove = true;
            this->mPlayerStats->mPlayerCanShoot = true;
            break;
        }

        case STATE_PLAYER_EMPTYFUEL: {
            this->mPlayerStats->mPlayerCanMove = false;
            this->mPlayerStats->mPlayerCanShoot = false;
            break;
        }

        case STATE_PLAYER_BROKEN: {
            this->mPlayerStats->mPlayerCanMove = false;
            this->mPlayerStats->mPlayerCanShoot = false;
            break;
        }

        case STATE_PLAYER_GRABEDBYRECOVERYVEHICLE: {
           this->mPlayerStats->mPlayerCanMove = false;
           this->mPlayerStats->mPlayerCanShoot = false;
           break;
        }
   }

    //in the finished state the player should be able to
    //move, but not shoot; the human player craft is taken
    //over in this state by the computer player control
    if (mPlayerStats->mHasFinishedRace) {
        this->mPlayerStats->mPlayerCanMove = true;
        this->mPlayerStats->mPlayerCanShoot = false;
    }

    //Update a connected HUD as well
    UpdateHUDState();
}

bool Player::AllAnimatorsDone() {
    return (this->mMGun->AllAnimationsFinished());
}

void Player::DeactivateAttack() {
    this->mPlayerStats->mPlayerCanShoot = false;
}

void Player::FinishedRace() {
    /* after the player is finished with the race
     * the game uses the external view, while a
     * computer player takes over controlling this craft */
    this->mCurrentViewMode = CAMERA_EXTERNALVIEW;

    mPlayerStats->mHasFinishedRace = true;

    //create a copy of the final player stats
    *mFinalPlayerStats = *mPlayerStats;

    //Update a connected HUD as well
    //so that change of mHasFinishedRace changes
    //the HUD state
    UpdateHUDState();

    LogMessage((char*)"I have finished the race");

    //also tell the race that I am finished
    mRace->PlayerHasFinishedLastLapOfRace(this);

    //if this is a human player we need to reconfigure it
    //as a computer player, so that the computer player
    //can continue moving it over the race track
    if (mHumanPlayer) {
        CpTakeOverHuman();
    }
}

void Player::CpTakeOverHuman() {
    //reconfigure physics model, so that computer player is able
    //to drive the craft
    this->mRace->SetupPhysicsObjectParameters(*this->phobj, false);

    //stop being a human player
    mHumanPlayer = false;

    LogMessage((char*)"Control handed over to computer");
    mCpuPlayer->CpTakeOverHuman();
}

void Player::SetGrabedByRecoveryVehicle(Recovery* whichRecoveryVehicle) {
    //lets repair (refuel) the player
    if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_EMPTYFUEL) {
        //our fuel is empty, add a little fuel (30% of max value)
        this->mPlayerStats->gasolineVal += 0.3f * this->mPlayerStats->gasolineMax;
    } else if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_BROKEN) {
        //we are broken down, fix the shield
        this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;
        //also fix broken glass
        RepairGlasBreaks();

        //remove killed by HUD message
        RemovePlayerPermanentGreenBigText();
    }

   LogMessage((char*)"The recovery vehicle grabed me");

   SetNewState(STATE_PLAYER_GRABEDBYRECOVERYVEHICLE);

   mGrabedByThisRecoveryVehicle = whichRecoveryVehicle;
}

void Player::FreedFromRecoveryVehicleAgain() {
   if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_GRABEDBYRECOVERYVEHICLE) {
       mGrabedByThisRecoveryVehicle = nullptr;

       LogMessage((char*)"I was dropped of by recovery vehicle again");

       //restore the correct viewmode again before craft
       //was destroyed
       mCurrentViewMode = mLastViewModeBeforeBrokenCraft;

       //this not only sets the player state to racing again
       //which means the player is allowed to move
       //but also enables drawing of HUD again
       //as long as the player has not finished the race yet
       SetNewState(STATE_PLAYER_RACING);

       //if this is a computer player, also inform
       //this cpu player about the fact that we were freed
       //from the recovery vehicle again
       if (!mHumanPlayer) {
          mCpuPlayer->FreedFromRecoveryVehicleAgain();
       }

       mRecoveryVehicleCalled = false;
   }
}

irr::u32 Player::GetCurrentState() {
    return this->mPlayerStats->mPlayerCurrentState;
}

Player::Player(Race* race, std::string model, irr::core::vector3d<irr::f32> NewPosition,
               irr::core::vector3d<irr::f32> NewFrontAt,
               irr::u8 nrLaps, bool humanPlayer) {

    //mFinalPlayerStats allows to make a copy of the
    //final player stats, when the player finishes the race
    mFinalPlayerStats = new PLAYERSTATS();

    mPlayerStats = new PLAYERSTATS();

    SetNewState(STATE_PLAYER_RACING);

    mPlayerStats->speed = 0.0f;
    mHumanPlayer = humanPlayer;

    mPlayerStats->shieldVal = mPlayerStats->shieldMax;
    mPlayerStats->gasolineVal = mPlayerStats->gasolineMax;

    //at the start the player has 6 missiles
    mPlayerStats->ammoVal = mPlayerStats->ammoMax;

    mPlayerStats->boosterVal = 0.0;
    mPlayerStats->throttleVal = 0.0f;

    //clear list of last lap times
    //integer value, each count equals to 100ms of time
    mPlayerStats->lapTimeList.clear();
    mPlayerStats->lastLap.lapNr = 0;
    mPlayerStats->lastLap.lapTimeMultiple40mSec = 0;
    mPlayerStats->LapBeforeLastLap.lapNr = 0;
    mPlayerStats->LapBeforeLastLap.lapTimeMultiple40mSec = 0;
    mPlayerStats->raceNumberLaps = nrLaps;

    mRace = race;

    //mPlayerStats->ammoVal = 0.0f;
    //mPlayerStats->shieldVal = 10.0f;

    //create my cpuPlayer
    mCpuPlayer = new CpuPlayer(this);

    //definition of dirt texture elements
    dirtTexIdsVec = new std::vector<irr::s32>{0, 1, 2, 60, 61, 62, 63, 64, 65, 66, 67, 79};

    //my Irrlicht coordinate system is swapped at the x axis; correct this issue
    //Position = NewPosition;
    //FrontDir = (NewFrontAt-Position).normalize(); //calculate direction vector

    PlayerMesh = mRace->mGame->mSmgr->getMesh(model.c_str());
    Player_node = mRace->mGame->mSmgr->addMeshSceneNode(PlayerMesh);

    //set player model initial orientation and position, later player craft is only moved by physics engine
    //also current change in Rotation of player craft model compared with this initial orientation is controlled by a
    //quaterion inside the physics engine object for this player craft as well
    Player_node->setRotation(((NewFrontAt-NewPosition).normalize()).getHorizontalAngle()+ irr::core::vector3df(0.0f, 180.0f, 0.0f));
    Player_node->setPosition(NewPosition);

    //Player_node->setDebugDataVisible(EDS_BBOX);
    Player_node->setDebugDataVisible(EDS_OFF);

    Player_node->setScale(irr::core::vector3d<irr::f32>(1,1,1));
    Player_node->setMaterialFlag(irr::video::EMF_LIGHTING, this->mRace->mGame->enableLightning);

    if (this->mRace->mGame->enableShadows) {
       // add shadow
       PlayerNodeShadow = Player_node->addShadowVolumeSceneNode();
    }

    //get player bounding box, to use later for machine gun targeting
    irr::core::aabbox3df Player_node_bbox = Player_node->getTransformedBoundingBox();

    mPlayerModelExtend = Player_node_bbox.getExtent();
    //reduce the extend by a factor of 2, so that we do not shoot at such
    //a wide range at the player model
    mPlayerModelExtend *= 0.5f;

    mCraftVisible = true;
    mCurrentViewMode = CAMERA_PLAYER_COCKPIT;

    //create my internal camera SceneNode for 1st person
    mIntCamera = mRace->mGame->mSmgr->addCameraSceneNode(nullptr, NewPosition);

    //create my internal camera SceneNode for 3rd person
    mThirdPersonCamera = mRace->mGame->mSmgr->addCameraSceneNode(nullptr, NewPosition);

    CalcCraftLocalFeatureCoordinates(NewPosition, NewFrontAt);

    //create my SmokeTrail particle system
    mSmokeTrail = new SmokeTrail(mRace->mGame->mSmgr, mRace->mGame->mDriver, this, 20);

    //create my Dust cloud emitter particles system
    mDustBelowCraft = new DustBelowCraft(mRace->mGame->mSmgr, mRace->mGame->mDriver, this, 100);

    //create my machinegun
    mMGun = new MachineGun(this, mRace->mGame->mSmgr, mRace->mGame->mDriver);

    //create my missile launcher
    mMissileLauncher = new MissileLauncher(this, mRace->mGame->mSmgr, mRace->mGame->mDriver);

    //create vector to store all the current broken Hud glas locations
    brokenGlasVec = new std::vector<HudDisplayPart*>();
    brokenGlasVec->clear();

    //create a moving average calculation helper for craft leaning angle left/right with average over
    //20 values
    mMovingAvgPlayerLeaningAngleLeftRightCalc = new MovingAverageCalculator(MVG_AVG_TYPE_IRRFLOAT32, 20);

    //create a moving average calculation helper for craft position over 10 samples
    //mMovingAvgPlayerPositionCalc = new MovingAverageCalculator(MVG_AVG_TYPE_IRRCOREVECT3DF, 10);

    dbgRecordFrontHeight = new std::vector<irr::f32>();
    dbgRecordBackHeight = new std::vector<irr::f32>();
    dbgRecordCurrJumping = new std::vector<irr::u8>();
    dbgRecordCurrCollision = new std::vector<irr::u8>();
}

//helper function, I one time thought I would need it especially for Recovery craft, when dropping the
//player craft after repair, but it turned out I had anothe problem
//I kept code in case I need something like that one day; But I am not sure if the code below
//is even working correctly
/*void Player::AlignPlayerModelToTerrainBelow() {
    irr::f32 hFront;
    irr::f32 hBack;
    irr::f32 hLeft;
    irr::f32 hRight;

    //the following function does also update player class member variables
    //terrainTiltCraftLeftRightDeg and terrainTiltCraftFrontBackDeg we need below
    GetHeightRaceTrackBelowCraft(hFront, hBack, hLeft, hRight);

    irr::core::vector3df currRotation;
    currRotation = this->Player_node->getRotation();

    this->Player_node->setRotation(irr::core::vector3df(-terrainTiltCraftLeftRightDeg,
                                                        irr::core::radToDeg(currRotation.Y), -terrainTiltCraftFrontBackDeg));

    irr::core::vector3df pos_in_worldspace_originPos(LocalCraftOrigin);
    this->Player_node->updateAbsolutePosition();
    irr::core::matrix4 matr = this->Player_node->getAbsoluteTransformation();

    matr.transformVect(pos_in_worldspace_originPos);

    irr::core::vector2di outCellOrigin;
    irr::f32 origin = this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                pos_in_worldspace_originPos.X,
                pos_in_worldspace_originPos.Z,
                outCellOrigin);

    pos_in_worldspace_originPos.Y = origin + HOVER_HEIGHT;

    this->Player_node->setPosition(pos_in_worldspace_originPos);
}*/

void Player::SetCurrClosestWayPointLink(std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> newClosestWayPointLink) {
    if (newClosestWayPointLink.first != nullptr) {
        this->currClosestWayPointLink = newClosestWayPointLink;
        this->projPlayerPositionClosestWayPointLink = newClosestWayPointLink.second;

        //also remember this choice in case we do not find the correct link at any moment later
        this->lastClosestWayPointLink.first = newClosestWayPointLink.first;
        this->lastClosestWayPointLink.second = newClosestWayPointLink.second;

        //Update a copy of this value/variable in the cpuPlayer class
        //Do this also for human players, we will need this information
        //always up to date when the computer player takes over the craft at the
        //end of the race from the human player!
        mCpuPlayer->SetCurrClosestWayPointLink(newClosestWayPointLink);
    }
}

//calculate local coordinates for heightmap collision points of craft
void Player::CreateHMapCollisionPointData() {
    this->Player_node->updateAbsolutePosition();

    /*irr::f32 diag = sqrt(this->mRace->mLevelTerrain->segmentSize * this->mRace->mLevelTerrain->segmentSize
                         + this->mRace->mLevelTerrain->segmentSize * this->mRace->mLevelTerrain->segmentSize);*/

    irr::f32 diag = this->mRace->mLevelTerrain->segmentSize * 0.7f;

    //get the size of the craft 3D model bounding box
    irr::core::vector3df hlpVec = Player_node->getTransformedBoundingBox().getExtent();

    //straight forwards direction
    this->mHMapCollPntData.front = new HMAPCOLLSENSOR();
    mHMapCollPntData.front->localPnt1.set(0.0f, 0.0f, -0.5f * hlpVec.Z);
    mHMapCollPntData.front->localPnt2.set(0.0f, 0.0f, -0.5f * hlpVec.Z - this->mRace->mLevelTerrain->segmentSize);
    strcpy(mHMapCollPntData.front->sensorName, "Front");

    //forward 45deg to the left side direction
    this->mHMapCollPntData.frontLeft45deg = new HMAPCOLLSENSOR();
    this->mHMapCollPntData.frontLeft45deg->localPnt1.set(0.5f * hlpVec.X, 0.0f,-0.5f * hlpVec.Z);
    this->mHMapCollPntData.frontLeft45deg->localPnt2.set(0.5f * hlpVec.X + diag, 0.0f, -0.5f * hlpVec.Z - diag);
    strcpy(mHMapCollPntData.frontLeft45deg->sensorName, "FrontL");

    //forward 45deg to the right side direction
    this->mHMapCollPntData.frontRight45deg = new HMAPCOLLSENSOR();
    this->mHMapCollPntData.frontRight45deg->localPnt1.set(-0.5f * hlpVec.X, 0.0f, -0.5f * hlpVec.Z);
    this->mHMapCollPntData.frontRight45deg->localPnt2.set(-0.5f * hlpVec.X - diag, 0.0f, -0.5f * hlpVec.Z - diag);
    strcpy(mHMapCollPntData.frontRight45deg->sensorName, "FrontR");

    //left side of the player craft
    this->mHMapCollPntData.left = new HMAPCOLLSENSOR();
    this->mHMapCollPntData.left->localPnt1.set(0.5f * hlpVec.X, 0.0f, 0.0f);
    this->mHMapCollPntData.left->localPnt2.set(0.5f * hlpVec.X + this->mRace->mLevelTerrain->segmentSize, 0.0f, 0.0f);
    strcpy(mHMapCollPntData.left->sensorName, "Left");

    //right side of the player craft
    this->mHMapCollPntData.right = new HMAPCOLLSENSOR();
    this->mHMapCollPntData.right->localPnt1.set(-0.5f * hlpVec.X, 0.0f, 0.0f);
    this->mHMapCollPntData.right->localPnt2.set(-0.5f * hlpVec.X - this->mRace->mLevelTerrain->segmentSize, 0.0f, 0.0f);
    strcpy(mHMapCollPntData.right->sensorName, "Right");

    //back 45deg to the left side direction
    this->mHMapCollPntData.backLeft45deg = new HMAPCOLLSENSOR();
    this->mHMapCollPntData.backLeft45deg->localPnt1.set(0.5f * hlpVec.X, 0.0f, 0.5f * hlpVec.Z);
    this->mHMapCollPntData.backLeft45deg->localPnt2.set(0.5f * hlpVec.X + diag, 0.0f, 0.5f * hlpVec.Z + diag);
    strcpy(mHMapCollPntData.backLeft45deg->sensorName, "BackL");

    //back 45deg to the right side direction
    this->mHMapCollPntData.backRight45deg = new HMAPCOLLSENSOR();
    this->mHMapCollPntData.backRight45deg->localPnt1.set(-0.5f * hlpVec.X, 0.0f, 0.5f * hlpVec.Z);
    this->mHMapCollPntData.backRight45deg->localPnt2.set(-0.5f * hlpVec.X - diag, 0.0f, 0.5f * hlpVec.Z + diag);
    strcpy(mHMapCollPntData.backRight45deg->sensorName, "BackR");

    //straight backwards direction
    this->mHMapCollPntData.back = new HMAPCOLLSENSOR();
    mHMapCollPntData.back->localPnt1.set(0.0f, 0.0f, 0.5f * hlpVec.Z);
    mHMapCollPntData.back->localPnt2.set(0.0f, 0.0f, 0.5f * hlpVec.Z + this->mRace->mLevelTerrain->segmentSize);
    strcpy(mHMapCollPntData.back->sensorName, "Back");
}

void Player::ExecuteCpPlayerLogic(irr::f32 deltaTime) {
    if (mHumanPlayer)
        return;

    mCpuPlayer->RunPlayerLogic(deltaTime);
}

void Player::GetHeightMapCollisionSensorDebugInfo(wchar_t* outputText, int maxCharNr) {

    int remChars = maxCharNr;

    wcscpy(outputText, L"");

    wchar_t entry[200];
    size_t currLen;

    //front
    if (mHMapCollPntData.front->currState != STATE_HMAP_COLL_IDLE) {
        GetHeightMapCollisionSensorDebugInfoEntryText(mHMapCollPntData.front,
                                                  entry, 190);

        currLen = wcslen(entry);
        remChars -= (unsigned int)(currLen);

        if (remChars > 0) {
         wcscat(outputText, entry);
        } else
            return;
    }

    //front left
     if (mHMapCollPntData.frontLeft45deg->currState != STATE_HMAP_COLL_IDLE) {
        GetHeightMapCollisionSensorDebugInfoEntryText(mHMapCollPntData.frontLeft45deg,
                                                      entry, 190);

        currLen = wcslen(entry);
        remChars -= (unsigned int)currLen;

        if (remChars > 0) {
         wcscat(outputText, entry);
        } else
            return;
     }

    //front right
    if (mHMapCollPntData.frontRight45deg->currState != STATE_HMAP_COLL_IDLE) {
        GetHeightMapCollisionSensorDebugInfoEntryText(mHMapCollPntData.frontRight45deg,
                                                      entry, 190);

        currLen = wcslen(entry);
        remChars -= (unsigned int)currLen;

        if (remChars > 0) {
         wcscat(outputText, entry);
        } else
            return;
    }

    //Left
    if (mHMapCollPntData.left->currState != STATE_HMAP_COLL_IDLE) {
        GetHeightMapCollisionSensorDebugInfoEntryText(mHMapCollPntData.left,
                                                      entry, 190);

        currLen = wcslen(entry);
        remChars -= (unsigned int)currLen;

        if (remChars > 0) {
         wcscat(outputText, entry);
         } else
            return;
    }

    //Right
    if (mHMapCollPntData.right->currState != STATE_HMAP_COLL_IDLE) {
        GetHeightMapCollisionSensorDebugInfoEntryText(mHMapCollPntData.right,
                                                      entry, 190);

        currLen = wcslen(entry);
        remChars -= (unsigned int)currLen;

        if (remChars > 0) {
         wcscat(outputText, entry);
        } else
            return;
    }

    //Back left
    if (mHMapCollPntData.backLeft45deg->currState != STATE_HMAP_COLL_IDLE) {
        GetHeightMapCollisionSensorDebugInfoEntryText(mHMapCollPntData.backLeft45deg,
                                                    entry, 190);

        currLen = wcslen(entry);
        remChars -= (unsigned int)currLen;

        if (remChars > 0) {
          wcscat(outputText, entry);
        } else
         return;
    }

    //Back right
   if (mHMapCollPntData.backRight45deg->currState != STATE_HMAP_COLL_IDLE) {
        GetHeightMapCollisionSensorDebugInfoEntryText(mHMapCollPntData.backRight45deg,
                                                      entry, 190);

        currLen = wcslen(entry);
        remChars -= (unsigned int)currLen;

        if (remChars > 0) {
         wcscat(outputText, entry);
        } else
            return;
    }

    //back
    if (mHMapCollPntData.back->currState != STATE_HMAP_COLL_IDLE) {
        GetHeightMapCollisionSensorDebugInfoEntryText(mHMapCollPntData.back,
                                                      entry, 190);

        currLen = wcslen(entry);
        remChars -= (unsigned int)currLen;

        if (remChars > 0) {
         wcscat(outputText, entry);
        } else
            return;
    }
}

void Player::GetHeightMapCollisionSensorDebugStateName(HMAPCOLLSENSOR *collSensor, char **stateName) {
    *stateName = new char[20];

    strcpy(*stateName, "");

    if (collSensor != nullptr) {
        if (collSensor->currState == STATE_HMAP_COLL_IDLE) {
            strcpy(*stateName, "IDLE");
        } else if (collSensor->currState == STATE_HMAP_COLL_WATCH) {
            strcpy(*stateName, "WATCH");
        } else if (collSensor->currState == STATE_HMAP_COLL_RESOLVE) {
            strcpy(*stateName, "RESOLVE");
        } else if (collSensor->currState == STATE_HMAP_COLL_FAILED) {
            strcpy(*stateName, "FAILED");
       } else if (collSensor->currState == STATE_HMAP_COLL_NOINTERSECTPNT) {
            strcpy(*stateName, "NOINTERSECTPNT");
        }
    }
}

void Player::GetHeightMapCollisionSensorDebugInfoEntryText(
    HMAPCOLLSENSOR *collSensor, wchar_t* outputText, int maxCharNr) {

    wcscpy(outputText, L"");

    if (collSensor != nullptr) {
        char *stateName;

        //only create debug text if state is not idle
        GetHeightMapCollisionSensorDebugStateName(collSensor, &stateName);

        //22.03.2025: It seems Visual Studio automatically changes swprintf to instead using
        //safer function swprintf_s which would be fine for me
        //The problem is this function checks for validity of format strings, and simply %s as under GCC
        //is not valid when specifiying a normal non wide C-string, and as a result text output does not work (only
        //garbage is shown); To fix this we need to use special format string "%hs" under windows;
        //But because I am not sure if GCC will accept this under Linux, I will keep the original code under GCC
        //here
#ifdef _MSC_VER 
        swprintf(outputText, maxCharNr, L"%hs: %hs %lf %lf %u\n",
                     collSensor->sensorName,
                     stateName,
                     collSensor->stepness,
                     collSensor->distance,
                     collSensor->collCnt);
#endif
#ifdef __GNUC__
        swprintf(outputText, maxCharNr, L"%s: %s %lf %lf %u\n",
            collSensor->sensorName,
            stateName,
            collSensor->stepness,
            collSensor->distance,
            collSensor->collCnt);
#endif

          delete[] stateName;
        }
}

void Player::UpdateHMapCollisionSensorPointData(HMAPCOLLSENSOR &sensor) {
    sensor.wCoordPnt1 = this->phobj->ConvertToWorldCoord(sensor.localPnt1);
    sensor.wCoordPnt2 = this->phobj->ConvertToWorldCoord(sensor.localPnt2);
}

//calculate world coordinates from local coordinates for heightmap collision points of craft
void Player::UpdateHMapCollisionPointData() {
  UpdateHMapCollisionSensorPointData(*this->mHMapCollPntData.front);
  UpdateHMapCollisionSensorPointData(*this->mHMapCollPntData.frontLeft45deg);
  UpdateHMapCollisionSensorPointData(*this->mHMapCollPntData.frontRight45deg);
  UpdateHMapCollisionSensorPointData(*this->mHMapCollPntData.left);
  UpdateHMapCollisionSensorPointData(*this->mHMapCollPntData.right);
  UpdateHMapCollisionSensorPointData(*this->mHMapCollPntData.back);
  UpdateHMapCollisionSensorPointData(*this->mHMapCollPntData.backLeft45deg);
  UpdateHMapCollisionSensorPointData(*this->mHMapCollPntData.backRight45deg);
}

//NewPosition = New position of player craft center of gravity (world coordinates)
//NewFrontAt = defines where player craft front is located at (world coordinates)
void Player::CalcCraftLocalFeatureCoordinates(irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt) {

    //simply set the craft original coodinates
    LocalCraftOrigin.set(0.0f, 0.0f, 0.0f);

    irr::core::vector3df pos_in_worldspace_frontPos(NewFrontAt);
    this->Player_node->updateAbsolutePosition();
    irr::core::matrix4 matr = this->Player_node->getAbsoluteTransformation();
    matr.makeInverse();

    matr.transformVect(pos_in_worldspace_frontPos);

    //calculates new local coordinate for front of craft
    LocalCraftFrontPnt = pos_in_worldspace_frontPos;

    irr::core::vector3df WCDirVecFrontToCOG = (NewPosition - NewFrontAt);
    irr::core::vector3df WCDirVecCOGtoBack = NewPosition + WCDirVecFrontToCOG;

    matr.transformVect(WCDirVecCOGtoBack);
    LocalCraftBackPnt = WCDirVecCOGtoBack;

    irr::core::vector3d<irr::f32> VectorUp(0.0f, 1.0f, 0.0f);
    WCDirVecFrontToCOG.normalize();
    irr::core::vector3df sideDirToLeft = WCDirVecFrontToCOG.crossProduct(VectorUp);

    //define a local craft coordinate that is independent of the craft model size,
    //so that for physics control the behavior of the craft does not depend on the model
    //otherwise models like the berserker are much more difficult to control
    LocalCraftForceCntrlPnt = LocalCraftOrigin - WCDirVecFrontToCOG * irr::core::vector3df(0.5f, 0.5f, 0.5f);

    sideDirToLeft.normalize();
    irr::core::vector3df WCDirVecCOGtoLeft = NewPosition - sideDirToLeft * WCDirVecFrontToCOG.getLength();

    matr.transformVect(WCDirVecCOGtoLeft);
    LocalCraftLeftPnt = WCDirVecCOGtoLeft;

    irr::core::vector3df WCDirVecCOGtoRight = NewPosition + sideDirToLeft * WCDirVecFrontToCOG.getLength();
    matr.transformVect(WCDirVecCOGtoRight);
    LocalCraftRightPnt = WCDirVecCOGtoRight;

    //LocalTopLookingCamPosPnt = DirVecCOGtoBack + irr::core::vector3df(0.0f, 1.0f, 0.4f);
    //LocalTopLookingCamTargetPnt = DirVecFrontToCOG + irr::core::vector3df(0.0f, 0.8f, 0.0f);

    //LocalTopLookingCamPosPnt = WCDirVecCOGtoBack + irr::core::vector3df(0.0f, 1.5f, 0.4f);      best value until 04.09.2024
    //LocalTopLookingCamTargetPnt = WCDirVecFrontToCOG + irr::core::vector3df(0.0f, 0.8f, 0.0f);   best value until 04.09.2024

    LocalTopLookingCamPosPnt = WCDirVecCOGtoBack + irr::core::vector3df(0.0f, 1.2f, 0.5f);     //attempt since 04.09.2024
    LocalTopLookingCamTargetPnt = WCDirVecFrontToCOG + irr::core::vector3df(0.0f, 1.1f, 0.0f); //attempt since 04.09.2024

    //Local1stPersonCamPosPnt = (LocalCraftFrontPnt) * irr::core::vector3df(0.0f, 0.0f, 0.5f);
    //Local1stPersonCamTargetPnt = Local1stPersonCamPosPnt + irr::core::vector3df(0.0f, 0.0f, -0.2f);

    Local1stPersonCamPosPnt = LocalCraftOrigin + irr::core::vector3df(0.0f, 0.2f, 0.4f);
    Local1stPersonCamTargetPnt = Local1stPersonCamPosPnt + irr::core::vector3df(0.0f, 0.0f, -0.2f);

    LocalCraftAboveCOGStabilizationPoint = irr::core::vector3df(0.0f, 1.0f, 0.0f);

    //define where from the craft the smoke emitts when the player
    //health is very low, let it emit from the backside of the player model
    irr::core::vector3df hlpVec = Player_node->getTransformedBoundingBox().getExtent();

    LocalCraftSmokePnt.Z = hlpVec.Z * 0.5f;
    LocalCraftSmokePnt.X = 0.0f;
    LocalCraftSmokePnt.Y = hlpVec.Y * 0.5f;

    //define where from the craft dust clouds are emitted, when hovering outside of the race
    //track
    LocalCraftDustPnt.X = 0.0f;
    LocalCraftDustPnt.Y = -hlpVec.Y * 0.3f;
    LocalCraftDustPnt.Z = 0.0f;

    CreateHMapCollisionPointData();

    LocalCraftFrontPnt = mHMapCollPntData.front->localPnt1;
    LocalCraftBackPnt = mHMapCollPntData.back->localPnt1;
    LocalCraftLeftPnt = mHMapCollPntData.left->localPnt1;
    LocalCraftRightPnt = mHMapCollPntData.right->localPnt1;

    //straight forwards direction camera Sensor to sense steepness over a longer
    //distance in front of the craft to prevent clipping of the camera at steep
    //hills
    this->cameraSensor = new HMAPCOLLSENSOR();
    this->cameraSensor->localPnt1.set(0.0f, 0.0f, -0.5f * hlpVec.Z + 2.0f * this->mRace->mLevelTerrain->segmentSize);
    this->cameraSensor->localPnt2.set(0.0f, 0.0f, -0.5f * hlpVec.Z + 1.0f * this->mRace->mLevelTerrain->segmentSize);
    strcpy(this->cameraSensor->sensorName, "CamSens");

    this->cameraSensor2 = new HMAPCOLLSENSOR();
    this->cameraSensor2->localPnt1.set(0.0f, 0.0f, -0.5f * hlpVec.Z + 1.0f * this->mRace->mLevelTerrain->segmentSize);
    this->cameraSensor2->localPnt2.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 0.0f * this->mRace->mLevelTerrain->segmentSize);
    strcpy(this->cameraSensor2->sensorName, "CamSens2");

    this->cameraSensor3 = new HMAPCOLLSENSOR();
    this->cameraSensor3->localPnt1.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 0.0f * this->mRace->mLevelTerrain->segmentSize);
    this->cameraSensor3->localPnt2.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 1.0f * this->mRace->mLevelTerrain->segmentSize);
    strcpy(this->cameraSensor3->sensorName, "CamSens3");

    this->cameraSensor4 = new HMAPCOLLSENSOR();
    this->cameraSensor4->localPnt1.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 1.0f * this->mRace->mLevelTerrain->segmentSize);
    this->cameraSensor4->localPnt2.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 2.0f * this->mRace->mLevelTerrain->segmentSize);
    strcpy(this->cameraSensor4->sensorName, "CamSens4");

    this->cameraSensor5 = new HMAPCOLLSENSOR();
    this->cameraSensor5->localPnt1.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 2.0f * this->mRace->mLevelTerrain->segmentSize);
    this->cameraSensor5->localPnt2.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 3.0f * this->mRace->mLevelTerrain->segmentSize);
    strcpy(this->cameraSensor5->sensorName, "CamSens5");

    this->cameraSensor6 = new HMAPCOLLSENSOR();
    this->cameraSensor6->localPnt1.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 3.0f * this->mRace->mLevelTerrain->segmentSize);
    this->cameraSensor6->localPnt2.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 4.0f * this->mRace->mLevelTerrain->segmentSize);
    strcpy(this->cameraSensor6->sensorName, "CamSens6");

    this->cameraSensor7 = new HMAPCOLLSENSOR();
    this->cameraSensor7->localPnt1.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 4.0f * this->mRace->mLevelTerrain->segmentSize);
    this->cameraSensor7->localPnt2.set(0.0f, 0.0f, -0.5f * hlpVec.Z - 5.0f * this->mRace->mLevelTerrain->segmentSize);
    strcpy(this->cameraSensor7->sensorName, "CamSens7");
}

void Player::DebugCraftLocalFeatureCoordinates() {
    pos_in_worldspace_frontPos = LocalCraftFrontPnt;
    pos_in_worldspace_backPos = LocalCraftBackPnt;
    pos_in_worldspace_leftPos = LocalCraftLeftPnt;
    pos_in_worldspace_rightPos = LocalCraftRightPnt;

    this->Player_node->updateAbsolutePosition();
    irr::core::matrix4 matr = this->Player_node->getAbsoluteTransformation();

    matr.transformVect(pos_in_worldspace_frontPos);
    matr.transformVect(pos_in_worldspace_backPos);
    matr.transformVect(pos_in_worldspace_leftPos);
    matr.transformVect(pos_in_worldspace_rightPos);
}

/* At one time I thought I need the code below, but later it turned out I had another
 * problem. Therefore I commented this code out again, in case I would need something like this
 * one day. But I am also not sure right now if the code below is correct
irr::core::quaternion Player::GetQuaternionFromPlayerModelRotation() {
    irr::core::quaternion result;

    Player_node->updateAbsolutePosition();
    irr::core::matrix4 corrEuler = Player_node->getAbsoluteTransformation();

    irr::core::vector3df inDeg = corrEuler.getRotationDegrees();
    irr::core::vector3df inRad;
    inRad.X = irr::core::degToRad(inDeg.X);
    inRad.Y = irr::core::degToRad(inDeg.Y);
    inRad.Z = irr::core::degToRad(inDeg.Z);

    result.set(inRad.X, inRad.Y, inRad.Z);
    result.normalize();

    return result;
}*/

void Player::Forward(irr::f32 deltaTime) {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

    //29.04.2025: for throttleVal calculation we need to take into accont
    //the current frame rate! otherwise the throttle change speed
    //depends heavily on the frame rate of the computer!
    irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

        //to accelerate player add force in craft forward direction
        this->phobj->AddLocalCoordForce(LocalCraftOrigin, irr::core::vector3df(0.0f, 0.0f, -50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

        if (mPlayerStats->throttleVal < mPlayerStats->throttleMax) {
            //+1.0f is for constant 60FPS
            mPlayerStats->throttleVal += 1.0f * speedFactor;
        }
}

void Player::Backward(irr::f32 deltaTime) {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

    //29.04.2025: for throttleVal calculation we need to take into accont
    //the current frame rate! otherwise the throttle change speed
    //depends heavily on the frame rate of the computer!
    irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

    if (!DEF_PLAYERCANGOBACKWARDS) {
        //we can not go backwards in Hioctane
        //we can only add friction to brake
        this->phobj->AddFriction(10.0f);

        if (mPlayerStats->throttleVal > 0) {
            //-1.0f is for constant 60FPS
            mPlayerStats->throttleVal -= 1.0f * speedFactor;
        }
    } else {
            //go solution during debugging, for example testing collisions, it helps to be able to accelerate backwards
            this->phobj->AddLocalCoordForce(LocalCraftOrigin, irr::core::vector3df(0.0f, 0.0f, 50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

            if (mPlayerStats->throttleVal > 0) {
                //-1.0f is for constant 60FPS
                mPlayerStats->throttleVal -= 1.0f * speedFactor;
            }
    }
}

//this function is only used for the human player
void Player::Left() {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

    currentSideForce += 30.0f;

    if (currentSideForce > 200.0f)
        currentSideForce = 200.0f;

    this->phobj->AddLocalCoordForce(LocalCraftForceCntrlPnt, irr::core::vector3df(currentSideForce, 0.0f, 0.0f),
                                     PHYSIC_APPLYFORCE_ONLYROT);
}

//this function is only used for the human player
void Player::Right() {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

    currentSideForce -= 30.0f;

    if (currentSideForce < -200.0f)
        currentSideForce = -200.0f;

    this->phobj->AddLocalCoordForce(LocalCraftForceCntrlPnt, irr::core::vector3df(currentSideForce, 0.0f, 0.0f),
                                     PHYSIC_APPLYFORCE_ONLYROT);
}

//this function is only used for the human player
void Player::NoTurningKeyPressed() {
    //stop turning sideways

    //add a counter force against the sideways movement
    //to prevent endlessly going sideways without any sideways key
    //pressed
    irr::f32 craftWorldSpeedSideWays = this->phobj->physicState.velocity.dotProduct(this->craftSidewaysToRightVec);
    irr::core::vector3df counterForceSideWaysMotion = irr::core::vector3df(craftWorldSpeedSideWays * CRAFT_SIDEWAYS_BRAKING, 0.0f, 0.0f);
    this->phobj->AddLocalCoordForce(this->LocalCraftOrigin, counterForceSideWaysMotion,
                                        PHYSIC_APPLYFORCE_ONLYTRANS);

    currentSideForce = 0.0f;
}

//returns true if player craft of a computer
//player is currently stuck
//returns always false when called for a human
//player, regardless what the human player currently
//does
bool Player::IsCurrentlyStuck() {
    if (mHumanPlayer)
        return false;

    return mCpuPlayer->IsCurrentlyStuck();
}


void Player::TestCpForceControlLogicWithHumanPlayer() {
    if (!mHumanPlayer)
        return;

    if (this->currClosestWayPointLink.first != nullptr) {

        irr::core::vector3df dirVecToLink = (this->currClosestWayPointLink.second - this->phobj->physicState.position);
        dirVecToLink.Y = 0.0f;

        irr::core::vector3df currDirVecCraftSide = craftSidewaysToRightVec;
        irr::core::vector3df currDirVecLink = dirVecToLink;
        currDirVecLink.normalize();

        currDirVecCraftSide.Y = 0.0f;
        currDirVecCraftSide.normalize();

        irr::f32 dotProd = currDirVecCraftSide.dotProduct(currDirVecLink);

        //define distance to current waypoint in a way, that if craft is left
        //of current waypoint link the distance is negative, and if the craft is right
        //of the current waypoint link the distance is positive
        if (dotProd < 0.0f) {
            mCurrentCraftDistToWaypointLink = dirVecToLink.getLength();
        } else {
            mCurrentCraftDistToWaypointLink = -dirVecToLink.getLength();
        }

        mCurrentCraftOrientationAngle =
                this->mRace->GetAbsOrientationAngleFromDirectionVec(craftForwardDirVec);

        //UpdateCurrentCraftOrientationAngleAvg();

         irr::f32 angleDotProduct = this->currClosestWayPointLink.first->LinkDirectionVec.dotProduct(craftForwardDirVec);

        irr::f32 angleRad = acosf(angleDotProduct);
        mAngleError = (angleRad / irr::core::PI) * 180.0f;

        if (craftSidewaysToRightVec.dotProduct(this->currClosestWayPointLink.first->LinkDirectionVec) > 0.0f) {
            mAngleError = -mAngleError;
        }

        //lets just pretend we always want to follow the line
        //exactly, without any offset
        irr::f32 mLocalOffset = 0.0f;

        irr::f32 distError = (mCurrentCraftDistToWaypointLink - mLocalOffset);

       // dbgDistError = distError;
    }
}
/*
void Player::UpdateCurrentCraftOrientationAngleAvg() {
    if (mCurrentCraftOrientationAngleSamples > 10) {
        this->mCurrentCraftOrientationAngleVec.pop_front();
        mCurrentCraftOrientationAngleSamples--;
    }

    this->mCurrentCraftOrientationAngleVec.push_back(mCurrentCraftOrientationAngle);
    mCurrentCraftOrientationAngleSamples++;

    std::list<irr::f32>::iterator itList;
    irr::f32 avgVal = 0.0f;

    for (itList = this->mCurrentCraftOrientationAngleVec.begin(); itList != this->mCurrentCraftOrientationAngleVec.end(); ++itList) {
        avgVal += (*itList);
    }

    mCurrentCraftOrientationAngleAvg = (avgVal / (irr::f32)(mCurrentCraftOrientationAngleSamples));
}*/

//returns true if player is currently in chargeing
//station and charges fuel, shield or ammo
bool Player::IsCurrentlyCharging() {
    return (mCurrChargingFuel || mCurrChargingAmmo || mCurrChargingShield);
}

bool Player::IsCurrentlyChargingFuel() {
    return mCurrChargingFuel;
}

bool Player::IsCurrentlyChargingShield() {
    return mCurrChargingShield;
}

bool Player::IsCurrentlyChargingAmmo() {
    return mCurrChargingAmmo;
}

void Player::LogMessage(char *msgTxt) {
    char* combinedMsg;

    size_t msgLen1 = strlen(this->mPlayerStats->name);
    size_t msgLen2 = strlen(msgTxt);

    combinedMsg = new char[msgLen1 + msgLen2 + 12];
    strcpy(combinedMsg, "Player ");
    strcat(combinedMsg, this->mPlayerStats->name);
    strcat(combinedMsg, ": ");
    strcat(combinedMsg, msgTxt);
    strcat(combinedMsg, "\0");

    mRace->mGame->mLogger->AddLogMessage(combinedMsg);

    delete[] combinedMsg;
}

void Player::StartDbgRecording() {
    if (mDbgCurrRecording)
        return;

    dbgRecordBackHeight->clear();
    dbgRecordFrontHeight->clear();
    dbgRecordCurrJumping->clear();
    dbgRecordCurrCollision->clear();

    mDbgCurrRecording = true;
}

void Player::EndDbgRecording() {
    if (!mDbgCurrRecording)
        return;

    //write output file

    //write the debugging output file
    FILE* outputFile;

    outputFile = fopen((char*)("collexport.txt"), "w");

    size_t maxIdx = dbgRecordFrontHeight->size();

    for (size_t currIdx = 0; currIdx < maxIdx; currIdx++) {

        fprintf(outputFile, "%lf;%lf;%u;%u\n",
                dbgRecordFrontHeight->at(currIdx),
                dbgRecordBackHeight->at(currIdx),
                dbgRecordCurrJumping->at(currIdx),
                dbgRecordCurrCollision->at(currIdx)
               );

    }

    fclose(outputFile);
}

void Player::ExecuteHeightMapCollisionDetection(irr::f32 deltaTime) {
     UpdateHMapCollisionPointData();

     //as a preparation to collision detection via heightmap
     //we need to do craft jump detection, so that we know
     //when we need to disable the detection
     JumpControlPhysicsLoop(deltaTime);

     //if player does jump currently do not execute heightmap
     //collision detection
     if (mCurrJumping) {

         if (mDbgCurrRecording) {
                dbgRecordCurrCollision->push_back(0);
         }
         return;
     }

     //Execute the terrain heightmap tile collision detection
     //only if we do this morphing will also have an effect
     //on the craft movements
     irr::u8 collval = 0;

     if (HeightMapCollision(*mHMapCollPntData.front)) {
         collval = 1;
     }

     if (HeightMapCollision(*mHMapCollPntData.frontLeft45deg)) {
         collval = 2;
     }

     if (HeightMapCollision(*mHMapCollPntData.frontRight45deg)) {
         collval = 3;
     }

     if (HeightMapCollision(*mHMapCollPntData.left)) {
         collval = 4;
     }

     if (HeightMapCollision(*mHMapCollPntData.right)) {
         collval = 5;
     }

     if (HeightMapCollision(*mHMapCollPntData.back)) {
         collval = 6;
     }

     if (HeightMapCollision(*mHMapCollPntData.backLeft45deg)) {
         collval = 7;
     }

     if (HeightMapCollision(*mHMapCollPntData.backRight45deg)) {
         collval = 8;
     }

     if (mDbgCurrRecording) {
         dbgRecordCurrCollision->push_back(collval);
     }

     //store heightmap collision detection debugging results for frame
     //StoreHeightMapCollisionDbgRecordingDataForFrame();
}

//in the original game the ship seems to collide with the
//marked Wallsegments which are contained within the enties
//of the level data. I do the same, as I create a collision mesh which
//contains an auto generated vertical wall at each wall segment location.
//Additional I create an collision mesh where the outline of (almost) every
//block in the game is also added. Only exception are the blocks above an tunnel,
//because otherwise the player often got stuck there in the ceiling.
//This collision mesh collisions are handled in Utils/physics.
//The problem is if I only apply this kind of collisions with the blocks and the
//wallsegment lines the player can fly through walls of Morphing-Regions, as the are
//not there at all. Also for example in Level 3 the player can fly upwards the walls
//of the massive hills around the starting point, but in the original game the
//player can not. Therefore the game seems to do additional collision checking based on the
//height of surrounding tiles. I need to do the same, which I will try to do here.
//I know it seems kind of messy to have different kind of collisions in the same
//game, but that is all I can do now
//returns true if a collision at this sensor was detected
bool Player::HeightMapCollision(HMAPCOLLSENSOR &collSensor) {

    irr::f32 dist;
    irr::core::vector3df collPlanePos1Coord;
    irr::core::vector3df collPlanePos2Coord;
    irr::core::vector3df collResolutionDirVector;
    irr::core::vector3df outIntersect;
    irr::core::vector3df delta;
    irr::core::vector3df deltaGround;
    irr::f32 step;
    irr::core::plane3df cplane;
    bool collDet = false;

    collSensor.stepness = 0.0f;
    collSensor.distance = 0.0f;
    collSensor.planePnt1.set(0.0f, 0.0f, 0.0f);
    collSensor.planePnt2.set(0.0f, 0.0f, 0.0f);
    collSensor.intersectionPnt.set(0.0f, 0.0f, 0.0f);

    bool diffTiles = false;
    bool noIntersectionPnt = false;

    collSensor.currState = STATE_HMAP_COLL_IDLE;

    collSensor.wCoordPnt1.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (collSensor.wCoordPnt1.X,
             collSensor.wCoordPnt1.Z,
             collSensor.cellPnt1);

    collSensor.wCoordPnt2.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (collSensor.wCoordPnt2.X,
             collSensor.wCoordPnt2.Z,
             collSensor.cellPnt2);

    if ((collSensor.cellPnt1.X != collSensor.cellPnt2.X) ||
        (collSensor.cellPnt1.Y != collSensor.cellPnt2.Y)) {
            diffTiles = true;
    }

    if (diffTiles) {
        //the two points are in different tiles, that means there is a intersection point at the edge
        //between the two tiles
        //there is potential for a collision, get "collision" plane between both cells
        this->mRace->mLevelTerrain->GetCollisionPlaneBetweenNeighboringTiles(collSensor.cellPnt1.X, collSensor.cellPnt1.Y,
                                                                             collSensor.cellPnt2.X, collSensor.cellPnt2.Y,
                               collPlanePos1Coord, collPlanePos2Coord,
                               collResolutionDirVector);

        collSensor.planePnt1 = collPlanePos1Coord;
        collSensor.planePnt2 = collPlanePos2Coord;

        //create a plane for further collision calculations
        cplane.setPlane(collPlanePos1Coord, collPlanePos2Coord, collPlanePos1Coord + irr::core::vector3df(0.0f, 1.0f, 0.0f));

        if (cplane.getIntersectionWithLimitedLine(collSensor.wCoordPnt1, collSensor.wCoordPnt2, outIntersect))
        {
            irr::core::vector2di outCell;
            //we need to correct Y coordinate of intersection point, so that it is at the height of the tile below
            outIntersect.Y =
                    this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(outIntersect.X, outIntersect.Z, outCell);

            collSensor.intersectionPnt = outIntersect;

            //steepness is calculated from pnt2 back to intersection point
            delta = (collSensor.wCoordPnt2 - outIntersect);

            deltaGround = delta;
            deltaGround.Y = 0.0f;

            //calculate steepness
            step = delta.Y / (deltaGround.getLength());
            collSensor.stepness = step;

        } else {
            //we did not find an intersection point
            noIntersectionPnt = true;

            collSensor.currState = STATE_HMAP_COLL_NOINTERSECTPNT;
        }
    } else {
        //both points are in the same tile
        //calculate steepness directly between pnt2 and pnt1
        delta = (collSensor.wCoordPnt2 - collSensor.wCoordPnt1);

        deltaGround = delta;
        deltaGround.Y = 0.0f;

        //calculate steepness
        step = delta.Y / (deltaGround.getLength());
        collSensor.stepness = step;
    }

    irr::f32 SteepThresh = DEF_LEVELTERRAIN_HEIGHTMAP_COLLISION_THRES;

    collSensor.deactivateHeightControl = false;

    //if steepness is higher then threshold we need to check for collision
    if (step > SteepThresh) {
        collSensor.deactivateHeightControl = true;
        collSensor.currState = STATE_HMAP_COLL_WATCH;

        if (diffTiles && !noIntersectionPnt) {
            //if pnt2 and pnt1 are again in different tiles, the remaining distance before
            //crash is from intersection point to pnt1
            irr::core::vector3df distVec = (outIntersect - collSensor.wCoordPnt1);

            dist = distVec.getLength();
            collSensor.distance = dist;

            if ((dist < 0.1f) && (dist > 0.0f)) {
                //we have a collision!
                //HeightMapCollisionResolve(cplane, collDataStruct->wCoordFront1, collDataStruct->wCoordFront2);

                //a collision should occur, play collision sound
                if (!this->mCurrJumping) {
                    collDet = true;
                    this->Collided();
                }

                collSensor.currState = STATE_HMAP_COLL_RESOLVE;

                irr::core::vector3df collResolutionDirVec = distVec.normalize();

                //immediately move the object so the collision is resolved
                //add a force that pushes the object away from the terrain tile we collided with
                this->phobj->physicState.position -= collResolutionDirVec * (0.1f - dist);

                this->phobj->AddWorldCoordForce(this->phobj->physicState.position,
                        this->phobj->physicState.position - collResolutionDirVec * 10.0f, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);

                collSensor.collCnt++;
            }

        } else if (!diffTiles) {
            //we most likely went to far over the cplane
            if (cplane.getIntersectionWithLimitedLine(this->phobj->physicState.position, collSensor.wCoordPnt1, outIntersect)) {
                irr::core::vector2di outCell;
                //we need to correct Y coordinate of intersection point, so that it is at the height of the tile below
                outIntersect.Y =
                        this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(outIntersect.X, outIntersect.Z, outCell);

                collSensor.intersectionPnt = outIntersect;

                //distance we went over the collision target is between
                //the point closes to the craft and the new intersection point
                irr::core::vector3df distVec = (outIntersect - collSensor.wCoordPnt1);

                dist = distVec.getLength();
                collSensor.distance = dist;

                //a collision should occur, play collision sound
                if (!this->mCurrJumping) {
                    collDet = true;
                    this->Collided();
                }

                collSensor.currState = STATE_HMAP_COLL_RESOLVE;

                irr::core::vector3df collResolutionDirVec = distVec.normalize();

                //immediately move the object so the collision is resolved
                //add a force that pushes the object away from the terrain tile we collided with
                this->phobj->physicState.position += collResolutionDirVec * dist;

                this->phobj->AddWorldCoordForce(this->phobj->physicState.position,
                        this->phobj->physicState.position + collResolutionDirVec * 10.0f, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);

                 collSensor.collCnt++;

            } else {
                //we did not find an intersection point
                noIntersectionPnt = true;

                collSensor.currState = STATE_HMAP_COLL_NOINTERSECTPNT;
            }

        }
    }

    return (collDet);
}

void Player::StoreHeightMapCollisionDbgRecordingDataForFrame() {
    //if currently no recording is done just return
    if (this->hMapCollDebugWhichSensor == nullptr)
        return;

    if (this->hMapCollDebugRecordingData == nullptr)
        return;

    HMAPCOLLSENSOR* newDataPoint = new HMAPCOLLSENSOR();
    //copy the data
    *newDataPoint = *hMapCollDebugWhichSensor;

    //store recording data from the current frame in the recording vector
    this->hMapCollDebugRecordingData->push_back(newDataPoint);
}

void Player::StartRecordingHeightMapCollisionDbgData(HMAPCOLLSENSOR *whichCollSensor) {
    //if we record already just return
    if (this->hMapCollDebugWhichSensor != nullptr)
        return;

    //if no valid sensor is specified also return
    if (whichCollSensor == nullptr)
        return;

    //create new recording data vector
    this->hMapCollDebugRecordingData = new std::vector<HMAPCOLLSENSOR*>();

    hMapCollDebugWhichSensor = whichCollSensor;
}

void Player::StopRecordingHeightMapCollisionDbgData(char* outputDbgFileName) {
    //if we do not record right now just return
    if (this->hMapCollDebugWhichSensor == nullptr)
        return;

    std::vector<HMAPCOLLSENSOR*>::iterator it;

    //write the debugging output file
    FILE* outputFile;

    outputFile = fopen(outputDbgFileName, "w");

    char *stateName;

    for (it = this->hMapCollDebugRecordingData->begin(); it != this->hMapCollDebugRecordingData->end(); ++it) {
        GetHeightMapCollisionSensorDebugStateName((*it), &stateName);

        fprintf(outputFile, "%s;%lf;%lf;%u\n",
                stateName,
                (*it)->stepness,
                (*it)->distance,
                (*it)->collCnt
               );

        delete[] stateName;
    }

    fclose(outputFile);

    HMAPCOLLSENSOR* pntr;

    //delete recorded data again
    for (it = this->hMapCollDebugRecordingData->begin(); it != this->hMapCollDebugRecordingData->end(); ) {
        pntr = (*it);

        it = this->hMapCollDebugRecordingData->erase(it);

        //delete object itself as well
        delete pntr;
    }

    delete this->hMapCollDebugRecordingData;
    this->hMapCollDebugRecordingData = nullptr;

    hMapCollDebugWhichSensor = nullptr;
}

void Player::Collided() {
    if (mHumanPlayer) {
           if (CollisionSound == nullptr) {
              CollisionSound = mRace->mSoundEngine->PlaySound(SRES_GAME_COLLISION, this->phobj->physicState.position, false);
           }
    }
}

void Player::AfterPhysicsUpdate() {
    if (CollisionSound != nullptr) {
        if (CollisionSound->getStatus() == CollisionSound->Stopped) {
            CollisionSound = nullptr;
        }
    }

    if (this->phobj->CollidedOtherObjectLastTime) {
        Collided();
    }

    //does another player have currently a missile
    //lock on us? if so play the warning sound
    if (this->mRace->currPlayerFollow == this) {
        if (mOtherPlayerHasMissleLockAtMe) {
            StartPlayingLockOnSound();
        } else {
            StopPlayingLockOnSound();
        }
    }
}

void Player::MaxTurboReached() {
    mMaxTurboActive = true;

    if (mMaxTurboActive != mLastMaxTurboActive) {
        if (mMaxTurboActive) {
            //we just reached max Turbo
            //now play booster sound
            mRace->mSoundEngine->PlaySound(SRES_GAME_BOOSTER, false);

            //give model speed boost for 20 mseconds
            mRemainingMaxTurboActiveTime = 0.02f;

            //reduce air friction to give player faster speeds with turbo
            this->phobj->SetAirFriction(CRAFT_AIRFRICTION_TURBO);
        } else {
            //we left max turbo again
        }
    }
}

//10.05.2025: Original Game observation
//Depending on the booster upgrade level the booster "charging" time to go from
//no booster value up to max booster level changes
//see ca. time measurement results from the original game below:
// no upgrade:   3.25 seconds
// 1st upgrade:  2.65 seconds
// 2nd upgrade:  2 seconds
// 3rd upgrade:  1.6 seconds
//to "discharge" booster from max value down to zero again takes always
//approx. 8.3 seconds;
void Player::IsSpaceDown(bool down, irr::f32 deltaTime) {
    mBoosterActive = down;

    //if this player can not move prevent
    //new activation of booster
    if (!mPlayerStats->mPlayerCanMove) {
        mBoosterActive = false;
    }

    //as long as booster "charge" value is not zero again after
    //it was charged once, a new charging/start of booster is prevented
    if (mBoosterRechargeCurrentlyLocked) {
       if (!(this->mPlayerStats->boosterVal > 0.0f)) {
         mBoosterRechargeCurrentlyLocked = false;
        } else {
            mBoosterActive = false;
        }
    }

    //in case max turbo/booster state was reached
    //a new turbo is not possible
    if (mMaxTurboActive) {
        mBoosterActive = false;

        //give the player model a speed boost for a defined time
        if (mRemainingMaxTurboActiveTime > 0.0f) {
            this->phobj->AddLocalCoordForce(irr::core::vector3df(0.0f, 0.0f, 0.0f), irr::core::vector3df(0.0f, 0.0f, -500.0f), PHYSIC_APPLYFORCE_REAL,
                                        PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

            mRemainingMaxTurboActiveTime -= deltaTime;
        }
    }

    //we need to scale the values below with the current frame rate
    //otherwise the booster charging/discharging speed depends on the current
    //computer FPS rate!
    irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

    if (mBoosterActive) {
           if (mLastBoosterActive != mBoosterActive) {
                //booster was activated!
                //keep a pointer to the sound source for the Turbo,
                //because maybe we need to interrupt the turbo sound if space key is released
                //by the player

                //the sound file for the developing turbo is not long enough
                //for the overall duration at lower booster upgrade levels
                //I am not sure what exactly the game does for solution, but what I
                //decided to do here right now, is to change the sound pitch (and
                //therefore the playback speed) of the turbo sound, so that the sound
                //is hopefully exactly over when the booster bar is fully charged
                //of course as a negative sideeffect this also changes the sound frequencies
                //of the booster playback sound
                irr::f32 soundPitch = 0.73f; //default pitch no booster upgrade

                switch (mPlayerStats->currBoosterUpgradeLevel) {
                   case 1: {
                        //1st upgrade level
                        soundPitch = 0.8f;
                        break;
                   }
                   case 2: {
                        //2nd upgrade level
                        soundPitch = 0.88f;
                        break;
                   }
                   case 3: {
                        //3rd upgrade level
                        soundPitch = 1.0f;
                        break;
                   }
               }

                TurboSound = mRace->mSoundEngine->PlaySound(SRES_GAME_TURBO, soundPitch, false);

                this->mPlayerStats->boosterVal = 0;
           }

           switch (mPlayerStats->currBoosterUpgradeLevel) {
               case 0: {
                    //no booster upgrade
                    this->mPlayerStats->boosterVal += 0.535f * speedFactor;
                    break;
               }
               case 1: {
                    //1st upgrade level
                    this->mPlayerStats->boosterVal += 0.658f * speedFactor;
                    break;
               }
               case 2: {
                    //2nd upgrade level
                    this->mPlayerStats->boosterVal += 0.866f * speedFactor;
                    break;
               }
               case 3: {
                    //3rd upgrade level
                    this->mPlayerStats->boosterVal += 1.083f * speedFactor;
                    break;
               }
           }

           if (this->mPlayerStats->boosterVal >= this->mPlayerStats->boosterMax) {
               //we reached max turbo level
               //make sure turbo sound is stopped
               //TurboSound->stop();
               if (TurboSound != nullptr) {
                   TurboSound->stop();
                   TurboSound = nullptr;
               }

               MaxTurboReached();
           }
        } else {

              if (mLastBoosterActive != mBoosterActive) {
                    //space key was released
                    //make sure turbo sound is stopped
                    //TurboSound->stop();
                    if (TurboSound != nullptr) {
                        TurboSound->stop();
                        TurboSound = nullptr;
                    }

                    //if space key is released the booster sound is played as well
                    mRace->mSoundEngine->PlaySound(SRES_GAME_BOOSTER, false);

                    mBoosterActive = false;
                    mBoosterRechargeCurrentlyLocked = true;
              }

              if (this->mPlayerStats->boosterVal > 0.0f)  {
                  //again scale booster value decrement value by current FPS of
                  //computer to get frame rate independent behavior!
                  this->mPlayerStats->boosterVal -= 0.2f * speedFactor;
              } else {
                  mMaxTurboActive = false;

                  //restore default air fricition without turbo active
                  phobj->SetAirFriction(CRAFT_AIRFRICTION_NOTURBO);
              }
        }
}

//when the craft goes into a curve and leans to its side
//we want to be able to draw the background sky also rotated
//to give the player the impression that the drawn sky is realistic
//for this we need to calculate the current angle of the craft relative to the
//Y-axis
void Player::CalcPlayerCraftLeaningAngle() {
    this->Player_node->updateAbsolutePosition();
    irr::core::vector3d<irr::f32> craftUpwardsVec =
            (WorldCoordCraftAboveCOGStabilizationPoint - this->Player_node->getAbsolutePosition()).normalize();

    irr::core::vector3d<irr::f32> distVec = (craftUpwardsVec - *mRace->mGame->yAxisDirVector);
    irr::f32 distVal = distVec.dotProduct(craftSidewaysToRightVec);

    //calculate angle between upVec and craftUpwardsVec
    //irr::f32 angleRad = acosf(craftUpwardsVec.dotProduct(upVec));

    irr::f32 angleRad = acosf(distVal);

    this->currPlayerCraftLeaningAngleDeg = (angleRad / irr::core::PI) * 180.0f - 90.0f + terrainTiltCraftLeftRightDeg;

    irr::core::vector3df leaningDirVec = craftUpwardsVec - *mRace->mGame->yAxisDirVector;
    irr::core::vector3df CraftRightDirVec = (WorldCoordCraftRightPnt - this->Player_node->getAbsolutePosition()).normalize();
    irr::f32 dotProductRightDir = leaningDirVec.dotProduct(CraftRightDirVec);

    if (dotProductRightDir > 0.1f) {
        currPlayerCraftLeaningOrientation = CRAFT_LEANINGRIGHT;
    } else if (dotProductRightDir < 0.1f) {
        currPlayerCraftLeaningOrientation = CRAFT_LEANINGLEFT;
    } else {
        //no leaning
        currPlayerCraftLeaningOrientation = CRAFT_NOLEANING;
    }

    //derive current craft forwards direction angle referenced to x-Axis
    //we need this number for computer player control
    //Important: Here do not correct angle calculation result outside of valid range!
   // mCurrentCraftOrientationAngle = mRace->GetAbsOrientationAngleFromDirectionVec(craftForwardDirVec, false);

    distVal = distVec.dotProduct(craftForwardDirVec);

    angleRad = acosf(distVal);

    this->currPlayerCraftForwardLeaningAngleDeg = (angleRad / irr::core::PI) * 180.0f - 90.0f + terrainTiltCraftFrontBackDeg;
}

//returns false if waypoint we want project player on is not sideways of player
//returns true otherwise
bool Player::ProjectOnWayPoint(WayPointLinkInfoStruct* projOnWayPointLink, irr::core::vector3df craftCoord, irr::core::vector3df *projPlayerPosition, irr::core::vector3d<irr::f32>* distanceVec,
                               irr::f32 *remainingDistanceToTravel) {
    irr::core::vector3df dA;
    irr::core::vector3df dB;

    irr::f32 projecteddA;
    irr::f32 projecteddB;
    irr::f32 projectedPl;

    //we want to find the waypoint link (line) to which the player is currently closest too (which the player currently tries to follow)
    //we also want to only find the line which is sideways of the player
    //first check if player is parallel to current line, or if the line is far away
    dA = craftCoord - projOnWayPointLink->pLineStruct->A;
    dB = craftCoord - projOnWayPointLink->pLineStruct->B;

    projecteddA = dA.dotProduct(projOnWayPointLink->LinkDirectionVec);
    projecteddB = dB.dotProduct(projOnWayPointLink->LinkDirectionVec);

    //if craft position is parallel (sideways) to current waypoint link the two projection
    //results need to have opposite sign; otherwise we are not sideways of this line, and need to ignore
    //this path segment
    if (sgn(projecteddA) != sgn(projecteddB)) {
        //this waypoint is interesting for further analysis
        //calculate distance from player position to this line, where connecting line meets path segment
        //in a 90° angle
        projectedPl =  dA.dotProduct(projOnWayPointLink->LinkDirectionVec);

        *projPlayerPosition = projOnWayPointLink->pLineStruct->A +
                irr::core::vector3df(projectedPl, projectedPl, projectedPl) * (projOnWayPointLink->LinkDirectionVec);

        *distanceVec = (*projPlayerPosition - craftCoord);

        *remainingDistanceToTravel = (projOnWayPointLink->pLineStruct->B - *projPlayerPosition).getLength();

        return true;
     }

    return false;
}

/*irr::core::vector3df Player::DeriveCurrentDirectionVector(WayPointLinkInfoStruct *currentWayPointLine, irr::f32 progressCurrWayPoint) {
    if (currentWayPointLine->pntrPathNextLink != nullptr) {
        //we have the next path link as well, we should be able to find out in which direction
        //we will have to go next
        irr::core::vector3df calcDirVec;

        irr::core::vector3df alpha;
        irr::core::vector3df invAlpha;
        irr::f32 inv = 1.0f - progressCurrWayPoint;

        alpha.set(progressCurrWayPoint, progressCurrWayPoint, progressCurrWayPoint);
        invAlpha.set(inv, inv, inv);

        calcDirVec = currentWayPointLine->LinkDirectionVec * invAlpha +
                currentWayPointLine->PathNextLinkDirectionVec * alpha;

        return calcDirVec;
    } else
        //if there is no link to the next element, return
        //direction vec of this current waypoint segment
        return currentWayPointLine->LinkDirectionVec;
}*/

//if showDurationSec is negative, the text will be shown until it is deleted
//with a call to function RemovePlayerPermanentGreenBigText
//if blinking is true text will blink (for example used for final lap text), If false
//text does not blink (as used when player died and waits for repair craft)
void Player::ShowPlayerBigGreenHudText(char* text, irr::f32 timeDurationShowTextSec, bool blinking) {
    if (mHUD != nullptr) {
        this->mHUD->ShowGreenBigText(text, timeDurationShowTextSec, blinking);
    }
}

void Player::RemovePlayerPermanentGreenBigText() {
    if (mHUD != nullptr) {
        this->mHUD->RemovePermanentGreenBigText();
    }
}

irr::core::vector2df Player::GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord) {
    threeDCoord = this->phobj->physicState.position;

    irr::core::vector2df result(this->phobj->physicState.position.X, this->phobj->physicState.position.Z);

    return result;
}

void Player::HideCraft() {
    if (mCraftVisible) {
        mCraftVisible = false;
        this->Player_node->setVisible(false);
    }
}

void Player::UnhideCraft() {
    if (!mCraftVisible) {
        mCraftVisible = true;
        this->Player_node->setVisible(true);
    }
}

bool Player::DoWeNeedHidePlayerModel() {
    if (mCurrentViewMode == CAMERA_PLAYER_COCKPIT)
        return true;

    return false;
}

irr::scene::ICameraSceneNode* Player::DeliverActiveCamera() {
    //are we on external view, and we have an external camera available?
    if (mCurrentViewMode == CAMERA_EXTERNALVIEW) {
        if (externalCamera != nullptr) {
            //return my external camera
            //lets update this external camera, so that it
            //does focus at us
            externalCamera->Update();

            //make sure player model is
            //visible again
            UnhideCraft();

            return externalCamera->mCamSceneNode;
        } else {
            //as a fallback return my cockpit view
            //make sure player model is hidden first
            HideCraft();

            return mIntCamera;
        }
    }

    if (mCurrentViewMode == CAMERA_PLAYER_COCKPIT) {
          return mIntCamera;
    }

    if (mCurrentViewMode == CAMERA_PLAYER_BEHINDCRAFT) {
         UnhideCraft();

         return mThirdPersonCamera;
    }

    //no valid view option, return nullptr
    return nullptr;
}

void Player::ChangeViewMode() {
    if (mCurrentViewMode == CAMERA_PLAYER_COCKPIT) {
         //unhide player craft model so that we can see
         //it again from the outside
         UnhideCraft();

         mCurrentViewMode = CAMERA_PLAYER_BEHINDCRAFT;
    } else if (mCurrentViewMode == CAMERA_PLAYER_BEHINDCRAFT) {
        //hide player craft model so that we do not see
        //it in our own camera
        HideCraft();

        mCurrentViewMode = CAMERA_PLAYER_COCKPIT;
    }
}

//returns true if the return parameter was modified, that means if a new minimum was
//found and set
bool Player::GetCurrentCeilingMinimumPositionHelper(HMAPCOLLSENSOR *sensor,
                                                    irr::core::vector3df &currMinPos, bool firstElement) {

   bool valSet = false;
   bool fElement = firstElement;

   irr::f32 height1;
   bool ceil1 =
           mRace->mLevelBlocks->GetCurrentCeilingHeightForTileCoord(sensor->cellPnt1, height1);

   irr::f32 height2;
   bool ceil2 = false;

   if (sensor->cellPnt1 != sensor->cellPnt2) {
       ceil2 = mRace->mLevelBlocks->GetCurrentCeilingHeightForTileCoord(sensor->cellPnt2, height2);
   }

   if (ceil1) {
      if ((height1 < currMinPos.Y) || (fElement)) {
           currMinPos.Y = height1;

           currMinPos.X = -sensor->cellPnt1.X * this->mRace->mLevelBlocks->segmentSize;
           currMinPos.Z = sensor->cellPnt1.Y * this->mRace->mLevelBlocks->segmentSize;
           valSet = true;
           fElement = false;
      }
   }

   if (ceil2) {
      if ((height2 < currMinPos.Y) || (fElement)) {
           currMinPos.Y = height2;

           currMinPos.X = -sensor->cellPnt2.X * this->mRace->mLevelBlocks->segmentSize;
           currMinPos.Z = sensor->cellPnt2.Y * this->mRace->mLevelBlocks->segmentSize;
           valSet = true;
      }
   }

   return valSet;
}

bool Player::GetCurrentCeilingMinimumPosition(irr::core::vector3df &currMinPos) {
    bool minValSet = false;

    irr::core::vector3df minPosVec(1000.0f, 1000.0f, 1000.0f);

    minValSet |= GetCurrentCeilingMinimumPositionHelper(this->cameraSensor, minPosVec, true);
    minValSet |= GetCurrentCeilingMinimumPositionHelper(this->cameraSensor2, minPosVec);
    minValSet |= GetCurrentCeilingMinimumPositionHelper(this->cameraSensor3, minPosVec);
    minValSet |= GetCurrentCeilingMinimumPositionHelper(this->cameraSensor4, minPosVec);
    minValSet |= GetCurrentCeilingMinimumPositionHelper(this->cameraSensor5, minPosVec);
    minValSet |= GetCurrentCeilingMinimumPositionHelper(this->cameraSensor6, minPosVec);
    minValSet |= GetCurrentCeilingMinimumPositionHelper(this->cameraSensor7, minPosVec);

    if (minValSet) {
        currMinPos = minPosVec;
        irr::core::vector2di outCell;

        currMinPos.Y += mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
        (minPosVec.X,
         minPosVec.Z,
            outCell);
    }

    return minValSet;
}

void Player::UpdateCameras() {
    //update cameraSensor to detect steepness over a wider distance
    //in front of the player craft in an attempt to prevent camera clipping
    //when moving fast over steep hills
    UpdateHMapCollisionSensorPointData(*this->cameraSensor);
    UpdateHMapCollisionSensorPointData(*this->cameraSensor2);
    UpdateHMapCollisionSensorPointData(*this->cameraSensor3);
    UpdateHMapCollisionSensorPointData(*this->cameraSensor4);
    UpdateHMapCollisionSensorPointData(*this->cameraSensor5);
    UpdateHMapCollisionSensorPointData(*this->cameraSensor6);
    UpdateHMapCollisionSensorPointData(*this->cameraSensor7);

    cameraSensor->wCoordPnt1.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor->wCoordPnt1.X,
             cameraSensor->wCoordPnt1.Z,
             cameraSensor->cellPnt1);

    cameraSensor->wCoordPnt2.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor->wCoordPnt2.X,
             cameraSensor->wCoordPnt2.Z,
             cameraSensor->cellPnt2);

    cameraSensor->stepness = cameraSensor->wCoordPnt2.Y - cameraSensor->wCoordPnt1.Y;

    cameraSensor2->wCoordPnt1.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor2->wCoordPnt1.X,
             cameraSensor2->wCoordPnt1.Z,
             cameraSensor2->cellPnt1);

    cameraSensor2->wCoordPnt2.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor2->wCoordPnt2.X,
             cameraSensor2->wCoordPnt2.Z,
             cameraSensor2->cellPnt2);

    cameraSensor2->stepness = cameraSensor2->wCoordPnt2.Y - cameraSensor2->wCoordPnt1.Y;

    cameraSensor3->wCoordPnt1.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor3->wCoordPnt1.X,
             cameraSensor3->wCoordPnt1.Z,
             cameraSensor3->cellPnt1);

    cameraSensor3->wCoordPnt2.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor3->wCoordPnt2.X,
             cameraSensor3->wCoordPnt2.Z,
             cameraSensor3->cellPnt2);

    cameraSensor3->stepness = cameraSensor3->wCoordPnt2.Y - cameraSensor3->wCoordPnt1.Y;

    cameraSensor4->wCoordPnt1.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor4->wCoordPnt1.X,
             cameraSensor4->wCoordPnt1.Z,
             cameraSensor4->cellPnt1);

    cameraSensor4->wCoordPnt2.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor4->wCoordPnt2.X,
             cameraSensor4->wCoordPnt2.Z,
             cameraSensor4->cellPnt2);

    cameraSensor4->stepness = cameraSensor4->wCoordPnt2.Y - cameraSensor4->wCoordPnt1.Y;

    cameraSensor5->wCoordPnt1.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor5->wCoordPnt1.X,
             cameraSensor5->wCoordPnt1.Z,
             cameraSensor5->cellPnt1);

    cameraSensor5->wCoordPnt2.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor5->wCoordPnt2.X,
             cameraSensor5->wCoordPnt2.Z,
             cameraSensor5->cellPnt2);

    cameraSensor5->stepness = cameraSensor5->wCoordPnt2.Y - cameraSensor5->wCoordPnt1.Y;

    cameraSensor6->wCoordPnt1.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor6->wCoordPnt1.X,
             cameraSensor6->wCoordPnt1.Z,
             cameraSensor6->cellPnt1);

    cameraSensor6->wCoordPnt2.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor6->wCoordPnt2.X,
             cameraSensor6->wCoordPnt2.Z,
             cameraSensor6->cellPnt2);

    cameraSensor6->stepness = cameraSensor6->wCoordPnt2.Y - cameraSensor6->wCoordPnt1.Y;

    cameraSensor7->wCoordPnt1.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor7->wCoordPnt1.X,
             cameraSensor7->wCoordPnt1.Z,
             cameraSensor7->cellPnt1);

    cameraSensor7->wCoordPnt2.Y =
            mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate
            (cameraSensor7->wCoordPnt2.X,
             cameraSensor7->wCoordPnt2.Z,
             cameraSensor7->cellPnt2);

    cameraSensor7->stepness = cameraSensor7->wCoordPnt2.Y - cameraSensor7->wCoordPnt1.Y;

    //*****************************************************
    //* Get minimum height above player craft
    //*****************************************************
    minCeilingFound = GetCurrentCeilingMinimumPosition(dbgCurrCeilingMinPos);

   // StabilizeCraft(frameDeltaTime);

    irr::f32 deltah1 = cameraSensor->wCoordPnt1.Y  - cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah2 = cameraSensor2->wCoordPnt1.Y - cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah3 = cameraSensor3->wCoordPnt1.Y - cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah4 = cameraSensor4->wCoordPnt1.Y - cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah5 = cameraSensor5->wCoordPnt1.Y - cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah6 = cameraSensor6->wCoordPnt1.Y - cameraSensor->wCoordPnt1.Y;
    irr::f32 deltah7 = cameraSensor7->wCoordPnt1.Y - cameraSensor->wCoordPnt1.Y;

    irr::f32 maxh = fmax(deltah1, deltah2);
    maxh = fmax(maxh, deltah3);
    maxh = fmax(maxh, deltah4);
    maxh = fmax(maxh, deltah5);
    maxh = fmax(maxh, deltah6);
    maxh = fmax(maxh, deltah7);

    //update 3rd person camera coordinates
    WorldTopLookingCamPosPnt = this->phobj->ConvertToWorldCoord(this->LocalTopLookingCamPosPnt);
    WorldTopLookingCamTargetPnt = this->phobj->ConvertToWorldCoord(this->LocalTopLookingCamTargetPnt);

    //1st person camera selected
    World1stPersonCamPosPnt = this->phobj->ConvertToWorldCoord(this->Local1stPersonCamPosPnt);
    World1stPersonCamTargetPnt = this->phobj->ConvertToWorldCoord(this->Local1stPersonCamTargetPnt);

    if (playerCamHeightListElementNr > 20) {
        this->playerCamHeightList.pop_front();
        playerCamHeightListElementNr--;
    }

    irr::f32 newCameraHeight = maxh * 0.5f;

    if (minCeilingFound) {
        if (World1stPersonCamPosPnt.Y > (dbgCurrCeilingMinPos.Y - 1.0f)) {
            newCameraHeight -= (World1stPersonCamPosPnt.Y - (dbgCurrCeilingMinPos.Y - 0.6f));
        }
    }

    this->playerCamHeightList.push_back(newCameraHeight);
    playerCamHeightListElementNr++;

    std::list<irr::f32>::iterator itList;
    irr::f32 avgVal = 0.0f;

    for (itList = this->playerCamHeightList.begin(); itList != this->playerCamHeightList.end(); ++itList) {
        avgVal += (*itList);
    }

    avgVal = (avgVal / (irr::f32)(playerCamHeightListElementNr));

    World1stPersonCamPosPnt.Y += avgVal;
    World1stPersonCamTargetPnt.Y += avgVal;

    dbgMaxh = maxh;
    dbgCameraVal = World1stPersonCamPosPnt.Y;
    dbgCameraTargetVal = World1stPersonCamTargetPnt.Y;
    dbgNewCameraVal = newCameraHeight;
    dbgCameraAvgVAl = avgVal;
    dbgMinCeilingFound = dbgCurrCeilingMinPos.Y;

    /*********************************************
     * Update my two internal cameras            *
     *********************************************/

    //3rd person camera
    mThirdPersonCamera->setPosition(this->WorldTopLookingCamPosPnt);
    mThirdPersonCamera->setTarget(this->WorldTopLookingCamTargetPnt);

    //1st person camera in cockpit
    mIntCamera->setPosition(this->World1stPersonCamPosPnt);
    mIntCamera->setTarget(this->World1stPersonCamTargetPnt);
    mIntCamera->setUpVector(this->craftUpwardsVec);
}

void Player::UpdateInternalCoordVariables() {
    WorldCoordCraftFrontPnt = this->phobj->ConvertToWorldCoord(LocalCraftFrontPnt);
    WorldCoordCraftBackPnt = this->phobj->ConvertToWorldCoord(LocalCraftBackPnt);
    WorldCoordCraftLeftPnt = this->phobj->ConvertToWorldCoord(LocalCraftLeftPnt);
    WorldCoordCraftRightPnt = this->phobj->ConvertToWorldCoord(LocalCraftRightPnt);

    WorldCoordCraftSmokePnt = this->phobj->ConvertToWorldCoord(LocalCraftSmokePnt);
    WorldCraftDustPnt = this->phobj->ConvertToWorldCoord(LocalCraftDustPnt);

    WorldCoordCraftAboveCOGStabilizationPoint = this->phobj->ConvertToWorldCoord(LocalCraftAboveCOGStabilizationPoint);

    craftUpwardsVec = (WorldCoordCraftAboveCOGStabilizationPoint - this->Player_node->getAbsolutePosition()).normalize();

    //calculate craft forward direction vector
    craftForwardDirVec = (WorldCoordCraftFrontPnt - this->phobj->physicState.position).normalize();
    craftSidewaysToRightVec = (WorldCoordCraftRightPnt - this->phobj->physicState.position).normalize();

    //recalculate current 2D cell coordinates
    //where the player is currently located
    mCurrPosCellX = -(int)(this->phobj->physicState.position.X / mRace->mLevelTerrain->segmentSize);
    mCurrPosCellY = (int)(this->phobj->physicState.position.Z / mRace->mLevelTerrain->segmentSize);
}

void Player::Update(irr::f32 frameDeltaTime) {
    if ((!mPlayerStats->mHasFinishedRace)
        && (mPlayerStats->mPlayerCurrentState != STATE_PLAYER_BEFORESTART)
        && (mPlayerStats->mPlayerCurrentState != STATE_PLAYER_ONFIRSTWAYTOFINISHLINE)) {
            //advance current lap lap time, frameDeltaTime is in seconds
            mPlayerStats->currLapTimeExact += frameDeltaTime;
       }

    updateSlowCnter += frameDeltaTime;

    this->mPlayerStats->speed = this->phobj->physicState.speed;

    //very special case: if the human player craft broke down (player died) we want
    //to stop the vehicle actively by adding friction
    if ((mHumanPlayer) && ((mPlayerStats->mPlayerCurrentState == STATE_PLAYER_BROKEN) ||
                           (mPlayerStats->mPlayerCurrentState == STATE_PLAYER_EMPTYFUEL))
            && (this->phobj->physicState.speed > 0.0f)) {
          //add friction to stop the craft
          this->phobj->AddFriction(10.0f);
    }

    if (updateSlowCnter >= 0.1) {
        updateSlowCnter = 0.0f;

        if (mMaxTurboActive) {

        }

        //only allow advancing missile lock state after race has fully
        //started, that means the first player has reached the finish line the
        //first time; we also know this, because here also the player state changed
        //to racing state
        if ((GetCurrentState() == STATE_PLAYER_RACING) && (!mPlayerStats->mHasFinishedRace)) {
            //handle missle lock timing logic
            if (this->mTargetMissleLockProgr > 0) {
                mTargetMissleLockProgr--;

                if (mTargetMissleLockProgr == 0) {
                    //we have achieved missile lock
                    this->mTargetMissleLock = true;
                }
            }
        } else {
            //player is not racing right now, or has finished the race
            this->mTargetMissleLock = false;
        }
    }

    //calculate lap time for Hud display
    //the number of lap time in HUD seems to indicate the time in
    //multiple of 40mSec, so every 40 mSec the number is increased by one integer
    mPlayerStats->currLapTimeMultiple40mSec = (irr::u32)(mPlayerStats->currLapTimeExact * 25.0f);

    //calculate player craft world coordinates
    UpdateInternalCoordVariables();

    //make sure this variable also gets updated for human
    //players as well
    if (mHumanPlayer) {
        mCurrentCraftOrientationAngle =
                this->mRace->GetAbsOrientationAngleFromDirectionVec(craftForwardDirVec);

        //UpdateCurrentCraftOrientationAngleAvg();
    }

    //we must prevent running player height control while
    //we are attached to recovery vehicle, because here physics
    //model is not active and otherwise we get weird behavior
    //when craft is freed again
    if (this->mGrabedByThisRecoveryVehicle == nullptr) {
        CraftHeightControl();
    }

    /************ Update player camera stuff ***************/
    UpdateCameras();
    /************ Update player camera stuff end ************/

    //check if this player is located at a charging station (gasoline, ammo or shield)
    CheckForChargingStation(frameDeltaTime);

    //execute code for fuel consumption, create low fuel
    //warnings, and change player state in case fuel is empty
    HandleFuel(frameDeltaTime);

    //execute source code to create low/empty ammo
    //warnings
    HandleAmmo();

    //create low shield warnings
    HandleShield();

    //next line is only for debugging
    //TestCpForceControlLogicWithHumanPlayer();

    CalcPlayerCraftLeaningAngle();

    nextLeaningAngleUpdate -= frameDeltaTime;

    if (nextLeaningAngleUpdate < 0.0f) {
        //update avg leaning angle every 16 ms
        nextLeaningAngleUpdate = 0.016f;

        if (!isnan(currPlayerCraftLeaningAngleDeg)) {
                //update average value for craft leaning angle left/right
                //is needed to rotate sky image
                mCurrentAvgPlayerLeaningAngleLeftRightValue = mMovingAvgPlayerLeaningAngleLeftRightCalc->AddNewValue(currPlayerCraftLeaningAngleDeg);
        }
    }

    //update moving average of current player position
    //value is needed to update spatial sound source location for player craft
    //if we do not use a moving average of the position then the player engine sound
    //has a weird amplitude modulation effect on it, because of craft position "jitter" from frame to frame
    //mCurrentAvgPlayerPosition = mMovingAvgPlayerPositionCalc->AddNewValue(this->phobj->physicState.position);

   // mCurrentAvgPlayerPosition = mMovingAvgPlayerPositionCalc->AddNewValue(World1stPersonCamPosPnt);

    //if this players camera is currently selected to be followed
    //set engine sound to be non spatial, otherwise we get a weird directional
    //sound effect when the player rotates around his axis
    if (this->mRace->currPlayerFollow == this) {
        this->mRace->mSoundEngine->SetPlayerSpeed(this, this->mPlayerStats->speed, this->mPlayerStats->speedMax);
    } else {
        //is not the main player (player that we follow right now)
        //use spatial engine sound
        this->mRace->mSoundEngine->SetPlayerSpeed(this, this->mPlayerStats->speed, this->mPlayerStats->speedMax,
                                                         this->phobj->physicState.position);
    }

    mLastBoosterActive = mBoosterActive;
    mLastMaxTurboActive = mMaxTurboActive;

    //I "measured" it in the original game, around the point of ~33% of the max health a player
    //starts to have a "smoke trail"
    mPlayerModelSmoking = this->mPlayerStats->shieldVal < (0.33f * this->mPlayerStats->shieldMax);

    if (mPlayerModelSmoking != mLastPlayerModelSmoking) {
        if (mPlayerModelSmoking) {
            //player model start smoking
            this->mSmokeTrail->Activate();
        } else {
            //player model stop smoking again
            this->mSmokeTrail->Deactivate();
        }
    }

    mSmokeTrail->Update(frameDeltaTime);

    mLastPlayerModelSmoking = mPlayerModelSmoking;

    CheckDustCloudEmitter();

    mDustBelowCraft->Update(frameDeltaTime);

    mMGun->Update(frameDeltaTime);

    mMissileLauncher->Update(frameDeltaTime);

    //check if player entered a craft trigger region
    CheckForTriggerCraftRegion();

    //do we have currently a missile lock at another player
    //if so set flag for warning sound in the other player
    if ((mTargetPlayer != nullptr) && (mTargetMissleLock)) {
        mTargetPlayer->mOtherPlayerHasMissleLockAtMe = true;
    }
}

void Player::JumpControlPhysicsLoop(irr::f32 deltaTime) {
    this->Player_node->updateAbsolutePosition();
    irr::core::matrix4 matr = this->Player_node->getAbsoluteTransformation();

    irr::core::vector3df frontCraftPoint(LocalCraftFrontPnt);
    matr.transformVect(frontCraftPoint);

    irr::core::vector2di outCellFront;
    irr::f32 frontTerrainHeight = this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                frontCraftPoint.X,
                frontCraftPoint.Z,
                outCellFront);

    /*irr::core::vector3df backCraftPoint(LocalCraftBackPnt);
    matr.transformVect(backCraftPoint);

    irr::core::vector2di outCellBack;
    irr::f32 backTerrainHeight = this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                backCraftPoint.X,
                backCraftPoint.Z,
                outCellBack);*/

    //update current distanced based on the heightmap collision
    //data
    currDistCraftTerrainFront = frontCraftPoint.Y - frontTerrainHeight;
    //currDistCraftTerrainBack =  backCraftPoint.Y - backTerrainHeight;

    //internal variable firstHeightControlLoop is used to prevent a
    //first unwanted JUMP detection when the first loop of PlayerCraftHeightControl is
    //executed at the start of the race (due to uninitialized variables)
    if (!firstHeightControlLoop) {
        //is craft currently jumping?
        if (!mCurrJumping) {
            //are we suddently start a jump?
            //we should be able to detect this when the front distance between craft
            //and race track below is suddently much bigger (because of the hole in the front
            //of the craft that is opening)
            if ((currDistCraftTerrainFront - HOVER_HEIGHT) > CRAFT_JUMPDETECTION_THRES ) {
                  this->mCurrJumping = true;

                  //reset in air time
                  mCurrInAirTime = 0.0f;

                  //message for debugging
                  /*if (mHUD !=nullptr) {
                         this->mHUD->ShowGreenBigText((char*)("JUMP"), 0.5f, false);
                  }*/
            }
        } else {
            //craft currently jumping
            //is the jump over again?
            //the jump is over when distance between craft front and race track
            //falls below normal hover height
            if (currDistCraftTerrainFront < HOVER_HEIGHT) {
                this->mCurrJumping = false;

                //message for debugging
                /*if (mHUD !=nullptr) {
                    this->mHUD->ShowGreenBigText((char*)("JUMP END"), 0.1f, false);
                }*/
            }
        }

         if (mCurrJumping) {
           mCurrInAirTime += deltaTime;
         }

      /*  if (mCurrJumping) {
            //slowly move craft downwards while jumping, instead of the normal
            //craft height control below, while we jump we are disconnected from the
            //race track surface

            //make the force downwards dependent on in air time
            //that means shorter jumps are easier for the player keeping
            //the downwards force first lower
            //but then with increasing air time the downward force is increased
            //so that the player does not fly to long through the air
            mCurrInAirTime += deltaTime;

            irr::core::vector3df downwardForce(0.0f, 0.0f, 0.0f);

            //14.04.2025: For the first level 6 jump and more jumps in level 8 we need a little bit of help to be able
            //to make the jump without using the booster; otherwise we can not fly over the wall on
            //the other side. Therefore let player model fly upwards at the beginning
            //feels weird, but there is no other solution right now :(
            if (mCurrInAirTime < 0.3f) {
                downwardForce.Y = -10.0f + 33.3f * mCurrInAirTime;
            } else if (mCurrInAirTime > 0.3f) {
                downwardForce.Y = 0.0f + 10.0f * (mCurrInAirTime - 0.3f);
            }

            //put limit at maximum force downwards
            if (downwardForce.Y > 25.0f)
                downwardForce.Y = 25.0f;

            //Apply force to the players model
            this->phobj->AddLocalCoordForce(LocalCraftOrigin, LocalCraftOrigin - downwardForce, PHYSIC_APPLYFORCE_REAL,
                                            PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);
        }*/

        /*if (mDbgCurrRecording) {
            dbgRecordFrontHeight->push_back(frontTerrainHeight);
            dbgRecordBackHeight->push_back(backTerrainHeight);
            if (mCurrJumping) {
                dbgRecordCurrJumping->push_back(1);
            } else {
                   dbgRecordCurrJumping->push_back(0);
            }
        }*/

    } else {
        //internal variables to prevent first unwanted
        //jump after start of race
        firstHeightControlLoop = false;
    }
}

void Player::CraftHeightControl() {
    //*****************************************************
    //* Hovercraft height control force calculation Start *  solution 1: with the 4 local points left, right, front and back of craft
    //*****************************************************

    //remember last distance in front of craft towards race track
    //needed for jump detection

    lastHeightFront = currHeightFront;
    lastHeightBack = currHeightBack;

    //establish height information of race track below player craft
    GetHeightRaceTrackBelowCraft(currHeightFront, currHeightBack, currHeightLeft, currHeightRight);

    currDistCraftTerrainFront = WorldCoordCraftFrontPnt.Y - currHeightFront;
    currDistCraftTerrainBack = WorldCoordCraftBackPnt.Y - currHeightBack;

    //when we jump run different code then in default
    //height control
    if (mCurrJumping) {
            //slowly move craft downwards while jumping, instead of the normal
            //craft height control below, while we jump we are disconnected from the
            //race track surface

            //make the force downwards dependent on in air time
            //that means shorter jumps are easier for the player keeping
            //the downwards force first lower
            //but then with increasing air time the downward force is increased
            //so that the player does not fly to long through the air
            //mCurrInAirTime += deltaTime;

            irr::core::vector3df downwardForce(0.0f, 0.0f, 0.0f);

            //14.04.2025: For the first level 6 jump and more jumps in level 8 we need a little bit of help to be able
            //to make the jump without using the booster; otherwise we can not fly over the wall on
            //the other side. Therefore let player model fly upwards at the beginning
            //feels weird, but there is no other solution right now :(
            if (mCurrInAirTime < 0.3f) {
                downwardForce.Y = -70.0f + 140.3f * mCurrInAirTime;
            } else if (mCurrInAirTime > 0.3f) {
                downwardForce.Y = 0.0f + 140.0f * (mCurrInAirTime - 0.3f);
            }

            //put limit at maximum force downwards
            if (downwardForce.Y > 100.0f)
                downwardForce.Y = 100.0f;

            //Apply force to the players model
            this->phobj->AddLocalCoordForce(LocalCraftOrigin, LocalCraftOrigin - downwardForce, PHYSIC_APPLYFORCE_REAL,
                                            PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

        return;
    }

    /*DbgCurrRaceTrackHeightFront = currHeightFront;
    DbgCurrRaceTrackHeightBack = currHeightBack;
    DbgCurrRaceTrackHeightLeft = currHeightLeft;
    DbgCurrRaceTrackHeightRight = currHeightRight;*/

    /*****************************************
     * Starting from here we control craft   *
     * height for normal flight (no jump)    *
     * Craft corners are controlled based    *
     * on racetrack height below             *
     * *************************************** */

    irr::f32 heightErrorFront = (WorldCoordCraftFrontPnt.Y - (currHeightFront + HOVER_HEIGHT));
    irr::f32 heightErrorBack = (WorldCoordCraftBackPnt.Y - (currHeightBack + HOVER_HEIGHT));
    irr::f32 heightErrorLeft = (WorldCoordCraftLeftPnt.Y - (currHeightLeft + HOVER_HEIGHT));
    irr::f32 heightErrorRight = (WorldCoordCraftRightPnt.Y - (currHeightRight + HOVER_HEIGHT));

    //if we are close to terrain heightmap collision stop the height control loop
    //because otherwise we are pulled upwards of steep slopes etc.
    if (this->mHMapCollPntData.front->currState != STATE_HMAP_COLL_IDLE) {
        heightErrorFront = 0.0f;
        heightErrorBack = 0.0f;
    }

    if (this->mHMapCollPntData.back->currState != STATE_HMAP_COLL_IDLE) {
        heightErrorFront = 0.0f;
        heightErrorBack = 0.0f;
    }

    if (this->mHMapCollPntData.left->currState != STATE_HMAP_COLL_IDLE) {
        heightErrorLeft = 0.0f;
        heightErrorRight = 0.0f;
    }

    if (this->mHMapCollPntData.right->currState != STATE_HMAP_COLL_IDLE) {
        heightErrorLeft = 0.0f;
        heightErrorRight = 0.0f;
    }

    //best values until now 01.08.2024
    irr::f32 corrForceHeight = 100.0f;
    irr::f32 corrDampingHeight = 10.0f;

    irr::f32 preventFlip = craftUpwardsVec.dotProduct(*mRace->mGame->yAxisDirVector);

    //original lines until 21.12.2024
    irr::f32 currVelFront =  this->phobj->GetVelocityLocalCoordPoint(LocalCraftFrontPnt).Y;
    irr::f32 corrForceFront = heightErrorFront * corrForceHeight + currVelFront * corrDampingHeight;

    //this line prevents flipping over the player model on the roof
    corrForceFront = corrForceFront * preventFlip;

    this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt - irr::core::vector3df(0.0f, corrForceFront, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                            PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

    irr::f32 currVelBack = this->phobj->GetVelocityLocalCoordPoint(LocalCraftBackPnt).Y;
    irr::f32 corrForceBack = heightErrorBack * corrForceHeight + currVelBack * corrDampingHeight;

    //this line prevents flipping over the player model on the roof
    corrForceBack = corrForceBack * preventFlip;

    this->phobj->AddLocalCoordForce(LocalCraftBackPnt, LocalCraftBackPnt - irr::core::vector3df(0.0f, corrForceBack, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

    irr::f32 currVelLeft = this->phobj->GetVelocityLocalCoordPoint(LocalCraftLeftPnt).Y;
    irr::f32 corrForceLeft = heightErrorLeft * corrForceHeight + currVelLeft * corrDampingHeight;

    //this line prevents flipping over the player model on the roof
    corrForceLeft = corrForceLeft * preventFlip;

    this->phobj->AddLocalCoordForce(LocalCraftLeftPnt, LocalCraftLeftPnt - irr::core::vector3df(0.0f, corrForceLeft, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

    irr::f32 currVelRight = this->phobj->GetVelocityLocalCoordPoint(LocalCraftRightPnt).Y;
    irr::f32 corrForceRight = heightErrorRight * corrForceHeight + currVelRight * corrDampingHeight;

    //this line prevents flipping over the player model on the roof
    corrForceRight = corrForceRight * preventFlip;

    this->phobj->AddLocalCoordForce(LocalCraftRightPnt, LocalCraftRightPnt - irr::core::vector3df(0.0f, corrForceRight, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);
}

void Player::SetName(char* playerName) {
    strcpy(this->mPlayerStats->name, playerName);
}

void Player::PlayMGunShootsAtUsSound() {
    switch (mCurrentRiccosSound) {
        case 0: {
             mRace->mSoundEngine->PlaySound(SRES_GAME_RICCO1, false);
             break;
        }

        case 1: {
             mRace->mSoundEngine->PlaySound(SRES_GAME_RICCO2, false);
             break;
        }

        case 2: {
             mRace->mSoundEngine->PlaySound(SRES_GAME_RICCO3, false);
             break;
        }
    }

    mCurrentRiccosSound++;

    if (mCurrentRiccosSound > 2)
        mCurrentRiccosSound = 0;
}

//returns TRUE if player reached below/equal 0 health (therefore if
//player died); otherwise false is returned
bool Player::Damage(irr::f32 damage, irr::u8 damageType) {
    //if someone shoots with machine gun at us, and we are selected
    //as the player to follow play the riccos sound
    if ((damageType == DEF_RACE_DAMAGETYPE_MGUN) &&
           (this->mRace->currPlayerFollow == this)) {
               //Play machine gun shoots at us sounds
               PlayMGunShootsAtUsSound();
    }

    //only deal positive damage!
    if ((damage > 0.0f) && (this->mPlayerStats->mPlayerCurrentState != STATE_PLAYER_BROKEN)) {
        this->mPlayerStats->shieldVal -= damage;
        if (this->mPlayerStats->shieldVal <= 0.0f) {
            this->mPlayerStats->shieldVal = 0.0f;

            this->WasDestroyed();

            return true;
       }
    }

    return false;
}

void Player::WasDestroyed() {
    //remember current viewmode so that we can restore
    //the correct one afterwards again after craft
    //repair
    mLastViewModeBeforeBrokenCraft = mCurrentViewMode;

    //when player craft is broken (player was killed)
    //the game uses an outside view
    this->mCurrentViewMode = CAMERA_EXTERNALVIEW;

    if (!mHumanPlayer) {
       mCpuPlayer->WasDestroyed();
    }

    //increase my death count
    this->mPlayerStats->currDeathCount++;

    //Player has now zero shield and is broken
    //Call recovery vehicle for help and set new
    //player state
    //This also updates the HUD and current
    //viewmode to be external view
    SetNewState(STATE_PLAYER_BROKEN);

    LogMessage((char*)"I have broken down (died), I call recovery vehicle for help");
    this->mRace->CallRecoveryVehicleForHelp(this);
    mRecoveryVehicleCalled = true;
}

void Player::UpdateHUDState() {
    if (mHUD == nullptr)
        return;

    irr::u32 state = this->GetCurrentState();

    //there is one exception, if we are in demo mode
    //do not draw the normal HUD, only before start
    if (this->mRace->mDemoMode) {
        if ((state != STATE_PLAYER_BEFORESTART) && (state != STATE_PLAYER_ONFIRSTWAYTOFINISHLINE)) {
            mHUD->SetHUDState(DEF_HUD_STATE_NOTDRAWN);
            return;
        }
    }

    //make sure the HUD state if correct for us
    switch (state) {
        case STATE_PLAYER_BEFORESTART:
        case STATE_PLAYER_ONFIRSTWAYTOFINISHLINE:
        {
            mHUD->SetHUDState(DEF_HUD_STATE_STARTSIGNAL);
            break;
        }
    case STATE_PLAYER_EMPTYFUEL:
    case STATE_PLAYER_RACING: {
            //19.04.2025: If the player has already finished the race
            //then do not draw HUD anymore, otherwise draw it again
            if (!mPlayerStats->mHasFinishedRace) {
                mHUD->SetHUDState(DEF_HUD_STATE_RACE);
            } else {
                mHUD->SetHUDState(DEF_HUD_STATE_BROKENPLAYER);
            }
            break;
        }

    case STATE_PLAYER_GRABEDBYRECOVERYVEHICLE:
    case STATE_PLAYER_BROKEN:  {
        //if there is a connected HUD we need to disable
        //its drawing, because if the player is destroyed there
        //is an outside view at the craft, and for an outside view
        //there is no HUD visible
        mHUD->SetHUDState(DEF_HUD_STATE_BROKENPLAYER);
        break;
    }
  }
}

void Player::SetupForStart() {
    this->SetNewState(STATE_PLAYER_BEFORESTART);
}

void Player::SetupComputerPlayerForStart(irr::core::vector3df startPos) {
    //make sure we only execute this command for non human
    //players!
    if (!mHumanPlayer) {
        mCpuPlayer->SetupForRaceStart(startPos);
    }
}

void Player::SetupToSkipStart() {
    this->SetNewState(STATE_PLAYER_RACING);

    //if this is a computer player, set its first target
    //speed
    if (!mHumanPlayer) {
        mCpuPlayer->StartSignalShowsGreen();
    }
}

void Player::SetupForFirstWayToFinishLine() {
    this->SetNewState(STATE_PLAYER_ONFIRSTWAYTOFINISHLINE);

    //if this is a computer player, set its first target
    //speed
    if (!mHumanPlayer) {
        mCpuPlayer->StartSignalShowsGreen();
    }
}

void Player::SetMyHUD(HUD* pntrHUD) {
    mHUD = pntrHUD;

    //I got a new HUD connected
    //we need to tell the HUD the correct
    //HUD state we want for the current player
    //state we have
    UpdateHUDState();
}

HUD* Player::GetMyHUD() {
    return mHUD;
}

bool Player::IsCurrentlyValidTarget() {
    //player is only a valid attack target
    //when actively in racing state, and if not yet
    //finished the race
    if ((this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_RACING) &&
        (!mPlayerStats->mHasFinishedRace))
        return true;

    return false;
}

void Player::SetTarget(Player* newTarget) {
    if (newTarget != mLastTargetPlayer) {
        //reset missle lock progress to max value
        this->mTargetMissleLockProgr = 22;

        mTargetPlayer = newTarget;
        mTargetMissleLock = false;
    }

    mLastTargetPlayer = mTargetPlayer;
}

void Player::CheckForTriggerCraftRegion() {
    //remember last trigger region before next update
    mLastCraftTriggerRegion = mCurrentCraftTriggerRegion;

    std::vector<MapTileRegionStruct*>::iterator itRegion;

    mCurrentCraftTriggerRegion = nullptr;

    //16.05.2025: There is a (hidden) shortcut in level 2 that is opened by "driving" into the
    //level wall. Problem is if we use the ships (origin middle) position to calculate the cell for craft trigger (which I did at the beginning),
    //this point does not reach into the trigger area of the shortcut (because the heightmap collision detection and prevention
    //prevents this middle coordinate to penetrate deep enough into the wall), and like this the way only opens when trying a lot of times,
    //and with a lot of luck. It works but not acceptable.
    //To make it work much better I decided to instead use a craft coordinate much further in the front of the craft, so that it can
    //penetrate deep enough, and cause the craft trigger to fire much much easier. I decided to reuse a height map collision "sensor"
    //coordinate for this trigger as well.
    int mTrigCurrPosCellX = -(int)(this->mHMapCollPntData.front->wCoordPnt2.X / mRace->mLevelTerrain->segmentSize);
    int mTrigCurrPosCellY = (int)(this->mHMapCollPntData.front->wCoordPnt2.Z / mRace->mLevelTerrain->segmentSize);

    //check for each trigger region in level
    for (itRegion = this->mRace->mTriggerRegionVec.begin(); itRegion != this->mRace->mTriggerRegionVec.end(); ++itRegion) {
        //only check for regions which are a playercraft trigger region
        if ((*itRegion)->regionType == LEVELFILE_REGION_TRIGGERCRAFT) {
            //is the player inside this area?
            if (this->mRace->mLevelTerrain->CheckPosInsideRegion(mTrigCurrPosCellX,
                    mTrigCurrPosCellY, (*itRegion))) {

                //assume craft can only be in one region at a certain time
                //craft trigger regions should not overlap!
                mCurrentCraftTriggerRegion = (*itRegion);
                break;
            }
        }
    }

    //did we enter a new trigger region?
    //if so we need to trigger the trigger event and tell the race
    //about it
    if (mCurrentCraftTriggerRegion != nullptr) {
        if (mCurrentCraftTriggerRegion != mLastCraftTriggerRegion) {
            //yes, we hit a new trigger region

            //is this a one time trigger only trigger?
            if (((*itRegion)->mOnlyTriggerOnce && (!(*itRegion)->mAlreadyTriggered))
                    || (!(*itRegion)->mOnlyTriggerOnce)) {
                       if ((*itRegion)->mOnlyTriggerOnce) {
                           (*itRegion)->mAlreadyTriggered = true;
                       }

                       mRace->PlayerEnteredCraftTriggerRegion(this, mCurrentCraftTriggerRegion);
            }
        }
    }
}

void Player::CheckForChargingStation(irr::f32 deltaTime) {
    mCurrChargingFuel = false;
    mCurrChargingAmmo = false;
    mCurrChargingShield = false;

    //Note 18.08.2024: Today I finally figured out how the game knows where the charging regions are
    //Texture solution of ground tiles only worked for some charging locations, but there are levels where
    //chargers do not use charger textures at the location, and so this solution does not work 100%
    //Therefore today I change to the correct solution using the charging region information stored in the
    //level file itself.

    //Additional note 18.08.2024: It is some hours later, and what I wrote above is incorrect again. It seems Hi Octane does things
    //in each level different. For level 4 and 8 for example this all does not work, and there seems to be no start
    //position in Table at offset 247264 where we find it as in the other levels. Even when I zero out the table at offset
    //247264 and table at offset 358222 with all bytes all 0 values, the original game still finds the starting location
    //and charger positions. The only explaination right now I have for this is that maybe the game for this different maps
    //still looks at the texture IDs of the tile below the player to find the start location and chargers and so on.
    //Therefore I now decided to implement both variants here, and I hope that this will work for every available Hi Octane
    //map out there. What a mess this is.

    //Final note regarding charging stations 26.12.2024: Unfortunetly in level 5 and 6 the two variants written above do not work
    //for all charging stations in this level. There are really charging areas without an entry in the "region" data table of the
    //level file, and without the correct texture IDs we expect in the height map of the Terrain map (because there is snow). Therefore
    //today I implemented yet another variant of charger detection: I search for overhead columns with certain charger station
    //face symbols. This works now also for the final chargers out there. So to make chargers work everywhere: At the end I implemented 3 different
    //variants to find them in the existing levels.

    bool atCharger = false;

    bool cShield;
    bool cFuel;
    bool cAmmo;

    //29.04.2025: for charging speed we need to take into account
    //the current frame rate! otherwise the charging rate
    //depends heavily on the frame rate of the computer!
    irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

    //see if we are currently in an charging area with this player
    this->mRace->mLevelTerrain->CheckPosInsideChargingRegion(mCurrPosCellX, mCurrPosCellY,
                                                             cShield, cFuel, cAmmo);

    if (cShield) {
       mCurrChargingShield = true;

       if (mPlayerStats->shieldVal < mPlayerStats->shieldMax)
        {
           atCharger = true;

           //shield charging from completely empty to completely
           //full does take approx. 4 seconds in the original game
           //the value 0.22f below was defined to be correct at constant 60FPS
           //(Vsync on). If we have different FPS rate we need to scale value!
            this->mPlayerStats->shieldVal += 0.22f * speedFactor;

            RepairGlasBreaks();

            if (mHUD != nullptr) {
                if (mPlayerStats->shieldVal >= mPlayerStats->shieldMax) {
                    mPlayerStats->shieldVal = mPlayerStats->shieldMax;
                    if (!this->mBlockAdditionalShieldFullMsg) {
                        mHUD->CancelAllPermanentBannerTextMsg();
                        this->mHUD->ShowBannerText((char*)"SHIELD FULL", 4.0f);
                        mBlockAdditionalShieldFullMsg = true;
                    }
                }
            }
        }
    }

    if (cAmmo) {
       mCurrChargingAmmo = true;

       if (mPlayerStats->ammoVal < mPlayerStats->ammoMax)
        {
            atCharger = true;

            //ammo charging from completely empty to completely
            //full does take approx. 4 seconds in the original game
            //value below is for constant 60 FPS, scale with FPS!
            this->mPlayerStats->ammoVal += 0.02f * speedFactor;
            if (mHUD != nullptr) {
                if (mPlayerStats->ammoVal >= mPlayerStats->ammoMax) {
                    mPlayerStats->ammoVal = mPlayerStats->ammoMax;
                    if (!mBlockAdditionalAmmoFullMsg) {
                        mHUD->CancelAllPermanentBannerTextMsg();
                        this->mHUD->ShowBannerText((char*)"AMMO FULL", 4.0f);
                        mBlockAdditionalAmmoFullMsg = true;
                    }
                }
           }
        }
    }

    if (cFuel) {
       mCurrChargingFuel = true;

       if (mPlayerStats->gasolineVal < mPlayerStats->gasolineMax)
        {
            atCharger = true;

            //fuel charging from completely empty to completely
            //full does take approx. 4 seconds in the original game
            //value 0.15f below is correct for constant 60FPS
            //we need to scale with correct current FPS value to get
            //frame rate independent correct charging speed
            this->mPlayerStats->gasolineVal += 0.15f * speedFactor;

             if (mHUD != nullptr) {
                if (mPlayerStats->gasolineVal >= mPlayerStats->gasolineMax) {
                    mPlayerStats->gasolineVal = mPlayerStats->gasolineMax;
                    if (!mBlockAdditionalFuelFullMsg) {
                        mHUD->CancelAllPermanentBannerTextMsg();
                        this->mHUD->ShowBannerText((char*)"FUEL FULL", 4.0f);
                        mBlockAdditionalFuelFullMsg = true;
                    }
                }
             }
        }
    }

    if (mCurrChargingFuel != mLastChargingFuel) {
        if (mCurrChargingFuel) {
            //charging fuel started
             if (atCharger) {
               if (mHUD != nullptr) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"FUEL RECHARGING", -1.0f);
               }
             }

        } else {
            if (mHUD != nullptr) {
                this->mHUD->CancelAllPermanentBannerTextMsg();
            }
            mBlockAdditionalFuelFullMsg = false;
        }
    }

    if (mCurrChargingAmmo != mLastChargingAmmo) {
        if (mCurrChargingAmmo) {
            //charging Ammo started
             if (atCharger) {
               if (mHUD != nullptr) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"AMMO RECHARGING", -1.0f);
               }
             }

        } else {
            if (mHUD != nullptr) {
                this->mHUD->CancelAllPermanentBannerTextMsg();
            }
            mBlockAdditionalAmmoFullMsg = false;
        }
    }

    if (mCurrChargingShield != mLastChargingShield) {
        if (mCurrChargingShield) {
            //charging shield started
             if (atCharger) {
               if (mHUD != nullptr) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"SHIELD RECHARGING", -1.0f);
               }
             }

        } else {
            if (mHUD != nullptr) {
                this->mHUD->CancelAllPermanentBannerTextMsg();
            }
            mBlockAdditionalShieldFullMsg = false;
        }
    }

    mLastChargingFuel = mCurrChargingFuel;
    mLastChargingAmmo = mCurrChargingAmmo;
    mLastChargingShield = mCurrChargingShield;

    if (atCharger) {
         if (mPlayerCurrentlyCharging == false) {
                mPlayerCurrentlyCharging = true;

                //play sound
                if (this->mHumanPlayer) {
                    //we need to keep a pntr to the looping sound source to be able to stop it
                    //later again!
                    mChargingSoundSource = this->mRace->mSoundEngine->PlaySound(SRES_GAME_REFUEL, true);
                }
        }
    } else {
        if (mPlayerCurrentlyCharging == true) {
               mPlayerCurrentlyCharging = false;

               //stop playing sound from looping sound source
               if (this->mHumanPlayer) {
                   this->mRace->mSoundEngine->StopLoopingSound(mChargingSoundSource);
                   mChargingSoundSource = nullptr;
               }
       }

    }
}

void Player::StartPlayingWarningSound() {
   //already warning playing?
   if (mWarningSoundSource == nullptr) {
       //no, start playing new warning
       //we need to keep a pntr to the looping sound source to be able to stop it
       //later again!
       mWarningSoundSource = this->mRace->mSoundEngine->PlaySound(SRES_GAME_WARNING, true);
   }
}

void Player::StopPlayingWarningSound() {
   //warning really playing?
   if (mWarningSoundSource != nullptr) {
       //yes, stop it
       this->mRace->mSoundEngine->StopLoopingSound(mWarningSoundSource);
       mWarningSoundSource = nullptr;
   }
}

void Player::StartPlayingLockOnSound() {
   //already lockon sound playing?
   if (mLockOnSoundSource == nullptr) {
       //no, start playing new lockon sound
       //we need to keep a pntr to the looping sound source to be able to stop it
       //later again!
        mLockOnSoundSource = this->mRace->mSoundEngine->PlaySound(SRES_GAME_LOCKON, true);
   }
}

void Player::StopPlayingLockOnSound() {
   //lock on sound really playing?
   if (mLockOnSoundSource != nullptr) {
       //yes, stop it
         this->mRace->mSoundEngine->StopLoopingSound(mLockOnSoundSource);
         mLockOnSoundSource = nullptr;
   }
}

void Player::AddTextureID(irr::s32 newTexId) {
    bool nrFound = false;

    std::vector<irr::s32>::iterator it;
    for (it = this->textureIDlist.begin(); it!=this->textureIDlist.end(); ++it) {
        if ((*it) == newTexId) {
            nrFound = true;
            break;
        }
    }

    //not found, add to list
    if (!nrFound) {
        textureIDlist.push_back(newTexId);
    }
}

//checks if current player should emit dust clouds below the craft
//this is the case if the player is above a "dusty" tile next to the race track
void Player::CheckDustCloudEmitter() {
    MapEntry* tilePntr = this->mRace->mLevelTerrain->GetMapEntry(mCurrPosCellX, mCurrPosCellY);
    irr::s32 texId = tilePntr->m_TextureId;

    mEmitDustCloud = false;

    //if craft is close enough to the terrain below
    //(and can) technically emit dust, continue checking for
    //texture Id, otherwise we will not emit Dust
    if ((this->phobj->physicState.position.Y - tilePntr->m_Height) < (3 * HOVER_HEIGHT)) {
        //check if our texture ID is present in the dirt tex id list
        //if so then emit clouds
        for (std::vector<irr::s32>::iterator itTex = dirtTexIdsVec->begin(); itTex != dirtTexIdsVec->end(); ++itTex) {
            if ((*itTex) == texId) {
                mEmitDustCloud = true;
                break;
            }
        }
    }

    if (mEmitDustCloud != mLastEmitDustCloud) {
        if (mEmitDustCloud) {
            this->mDustBelowCraft->Activate();
        } else {
            this->mDustBelowCraft->Deactivate();
        }
    }

    mLastEmitDustCloud = mEmitDustCloud;
}

void Player::GetHeightRaceTrackBelowCraft(irr::f32 &front, irr::f32 &back, irr::f32 &left, irr::f32 &right) {
    //we need to control the height of the player according to the Terrain below
    //irr::core::vector3d<irr::f32> SearchPosition;

    //calculate craft travel direction
    //the commented line below which calculate frontDir vom Velocity vector is bad, because at the start
    //for example we have no velocity, and therefore frontDir is not defined; Therefore now height calculcation is
    //possible for terrain below the craft
    //irr::core::vector3df frontDir = (this->phobj->physicState.velocity / this->phobj->physicState.speed);

    /*irr::core::vector3df frontDir;
    irr::core::vector3df leftDir;
    irr::core::vector3df rightDir;
    irr::core::vector3df backDir;*/

    this->Player_node->updateAbsolutePosition();
    irr::core::matrix4 matr = this->Player_node->getAbsoluteTransformation();

    irr::core::vector3df pos_in_worldspace_frontPos(LocalCraftFrontPnt);
    matr.transformVect(pos_in_worldspace_frontPos);

    irr::core::vector2di outCellFront;
    front = this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                pos_in_worldspace_frontPos.X,
                pos_in_worldspace_frontPos.Z,
                outCellFront);

    irr::core::vector3df pos_in_worldspace_RightPos(LocalCraftRightPnt);
    matr.transformVect(pos_in_worldspace_RightPos);

    irr::core::vector2di outCellRight;
    right = this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                pos_in_worldspace_RightPos.X,
                pos_in_worldspace_RightPos.Z,
                outCellRight);

    irr::core::vector3df pos_in_worldspace_LeftPos(LocalCraftLeftPnt);
    matr.transformVect(pos_in_worldspace_LeftPos);

    irr::core::vector2di outCellLeft;
    left = this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                pos_in_worldspace_LeftPos.X,
                pos_in_worldspace_LeftPos.Z,
                outCellLeft);

    irr::core::vector3df pos_in_worldspace_backPos(LocalCraftBackPnt);
    matr.transformVect(pos_in_worldspace_backPos);

    irr::core::vector2di outCellBack;
    back = this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                pos_in_worldspace_backPos.X,
                pos_in_worldspace_backPos.Z,
                outCellBack);

    //currTileBelowPlayer = nullptr;

    //calculate current cell below player
    //int current_cell_calc_x, current_cell_calc_y;

    //current_cell_calc_y = (this->phobj->physicState.position.Z / mRace->mLevelTerrain->segmentSize);
    //current_cell_calc_x = -(this->phobj->physicState.position.X / mRace->mLevelTerrain->segmentSize);

    //currTileBelowPlayer = this->mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);

    //just for debugging texture IDs
    //currTextID = this->mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y)->m_TextureId;
    //AddTextureID(currTextID);

    //*** search the Terrain cells in front of the player ***

    //calculate terrain tilt from craft left to right
    irr::f32 hdiff = left - right;
    irr::f32 vdiff = (pos_in_worldspace_LeftPos - pos_in_worldspace_RightPos).getLength();

    irr::f32 terrainTiltCraftLeftRightRad = asinf(hdiff/vdiff) ;
    terrainTiltCraftLeftRightDeg = (terrainTiltCraftLeftRightRad / irr::core::PI) * 180.0f;

    //calculate terrain tilt from craft front to back
    hdiff = front - back;
    vdiff = (pos_in_worldspace_frontPos - pos_in_worldspace_backPos).getLength();

    irr::f32 terrainTiltCraftFrontBackRad = asinf(hdiff/vdiff) ;
    terrainTiltCraftFrontBackDeg = (terrainTiltCraftFrontBackRad / irr::core::PI) * 180.0f;
}

//is called when the player collected a collectable item of the
//level
bool Player::CollectedCollectable(Collectable* whichCollectable) {
    //depending on the type of entity/collectable alter player stats
    Entity::EntityType type = whichCollectable->GetCollectableType();

    //Note 08.03.2025: I was able to figure out the fuel and ammo items by comparing the effects
    //in the original game with the effects in my project approx.
    switch (type) {
        case Entity::EntityType::ExtraFuel:
            //Fuel item can only be picked up by the player, if fuel is currently
            //not at max
            if (!(this->mPlayerStats->gasolineVal < (this->mPlayerStats->gasolineMax * 0.98f))) {
                //fuel is pretty maxed out, return without picking item up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"EXTRA FUEL", 4.0f);
            }

            //after collecting an extra fuel when almost empty in the original game
            //7 fuel bars where shown in the Hud afterwards, which is a fuel value between
            //12f and 14f; so adding 12.0f should be approx. correct
            this->mPlayerStats->gasolineVal += 12.0f;
            if (this->mPlayerStats->gasolineVal > this->mPlayerStats->gasolineMax)
                this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

            break;

        case Entity::EntityType::FuelFull:
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"FUEL FULL", 4.0f);
            }

            //the fuel full item sets the current fuel level to
            //max possible fuel level; the max possible fuel level is not modified

            this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

            break;

        case Entity::EntityType::DoubleFuel:
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"DOUBLE FUEL", 4.0f);
            }

            //the double fuel item sets the current gasoline level
            //to twice the max possible value
            //the max possible value is not modified;
            //that means the first half possible distance the player
            //takes on the course afterwards does not reduce the number of
            //indicated fuel available bars in the Hud. This is also in the original
            //game

            this->mPlayerStats->gasolineVal = 2.0f * this->mPlayerStats->gasolineMax;

            break;

        case Entity::EntityType::ExtraAmmo:
            //the extra ammo item increases the number of ammo
            //(missiles) by exactly one missile. But this item can only be picked
            //up by the player, if there is space for an additional missile
            //if ammo is already at ammo max this item can not be picked up

            if (!(this->mPlayerStats->ammoVal < this->mPlayerStats->ammoMax)) {
                //ammo already full, collectible is not picked up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"EXTRA AMMO", 4.0f);
            }

            //add one missile
            this->mPlayerStats->ammoVal += 1.0f;

            if (this->mPlayerStats->ammoVal > this->mPlayerStats->ammoMax)
                this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;

            break;

        case Entity::EntityType::AmmoFull:
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"AMMO FULL", 4.0f);
            }

            //the ammo full item sets the number
            //of available ammo (missiles) to max value
            //the max possible number of ammo stays at 6
            this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;

            break;

        case Entity::EntityType::DoubleAmmo:
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"DOUBLE AMMO", 4.0f);
            }

            //the double ammo item sets the number
            //of available ammo (missiles) to twice the max possible value
            //the max possible number of ammo stays at 6
            //that means the first 6 missiles that are fired, do not
            //reduce the number of indicated bars in the Hud
            this->mPlayerStats->ammoVal = 2.0f * this->mPlayerStats->ammoMax;
            break;

        case Entity::EntityType::ExtraShield: 
            //the extra shield item increases the number of shield
            //hud bars by approx. 2 bars. But this item can only be picked
            //up by the player, if the player has not already full shield
            if (!(this->mPlayerStats->shieldVal < this->mPlayerStats->shieldMax)) {
                //shield already full, collectible is not picked up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"EXTRA SHIELD", 4.0f);
            }

            this->mPlayerStats->shieldVal += 15.0f;
            if (this->mPlayerStats->shieldVal > this->mPlayerStats->shieldMax)
                this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;

            break;

        case Entity::EntityType::ShieldFull:
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"SHIELD FULL", 4.0f);
            }

            //based on how the fuel and ammo full item works (observed in original
            //game), I guessed that the shield item should do the same. I did not actually
            //fact check this with the original game

            this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;

            break;

        case Entity::EntityType::DoubleShield:
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"DOUBLE SHIELD", 4.0f);
            }

            //based on how the fuel and ammo double item works (observed in original
            //game), I guessed that the shield item should do the same. I did not actually
            //fact check this with the original game

            this->mPlayerStats->shieldVal = 2.0f * this->mPlayerStats->shieldMax;

            break;

        case Entity::EntityType::BoosterUpgrade:
            //can only be picked up if booster upgrade level is not already
            //at max
            if (this->mPlayerStats->currBoosterUpgradeLevel < 3) {
                this->mPlayerStats->currBoosterUpgradeLevel++;

                //upgrade players booster
                if (mHUD != nullptr) {
                    this->mHUD->ShowBannerText((char*)"BOOSTER UPGRADED", 4.0f);
                }
            } else {
                return false;
            }

            break;

        case Entity::EntityType::MissileUpgrade:
            //can only be picked up if missile upgrade level is not already
            //at max
            if (this->mPlayerStats->currRocketUpgradeLevel < 3) {
                this->mPlayerStats->currRocketUpgradeLevel++;

                if (mHUD != nullptr) {
                    this->mHUD->ShowBannerText((char*)"MISSILE UPGRADED", 4.0f);
                }
            } else {
                return false;
            }

            break;

        case Entity::EntityType::MinigunUpgrade:
            //can only be picked up if mini-gun upgrade level is not already
            //at max
            if (this->mPlayerStats->currMinigunUpgradeLevel < 3) {
                this->mPlayerStats->currMinigunUpgradeLevel++;

                //upgrade players mini-gun
                if (mHUD != nullptr) {
                    this->mHUD->ShowBannerText((char*)"MINIGUN UPGRADED", 4.0f);
                }
            } else {
                return false;
            }

            break;

        case Entity::EntityType::UnknownShieldItem:
            //uncomment the next 2 lines to show this items also to the player
            // collectable = new Collectable(41, entity.get_Center(), color, driver);
            // ENTCollectables_List.push_back(collectable);
            break;

        case Entity::EntityType::UnknownItem:
        case Entity::EntityType::Unknown:
            //uncomment the next 2 lines to show this items also to the player
            // collectable = new Collectable(50, entity.get_Center(), color, driver);
            // ENTCollectables_List.push_back(collectable);
            break;

         //all the other entities we do not care here
        default:
            break;
    }

    //play sound
    if (this->mHumanPlayer) {
        this->mRace->mSoundEngine->PlaySound(SRES_GAME_PICKUP);
    }

    //collectible was picked up
    return true;

//    //if this is a computer player, was this item planed to be picked
//    //up? if so reset variables for collectable logic
//    if (!this->mHumanPlayer) {
//        if (this->mCpTargetCollectableToPickUp == whichCollectable) {
//            //yes, correct item picked up
//            //set back to nullptr, so that player can lookup
//            //next item he wants to collect
//            mCpTargetCollectableToPickUp = nullptr;
//            this->mCpWayPointLinkClosestToCollectable = nullptr;

//            CurrentCommandFinished();
//        }
//    }
}

irr::f32 Player::GetHoverHeight() {
    irr::f32 height = HOVER_HEIGHT;

    return (height);
}

void Player::DebugSelectionBox(bool boundingBoxVisible) {
    if (boundingBoxVisible) {
        this->Player_node->setDebugDataVisible(E_DEBUG_SCENE_TYPE::EDS_BBOX);
    } else {
        this->Player_node->setDebugDataVisible(E_DEBUG_SCENE_TYPE::EDS_OFF);
    }
}

void Player::FinishedLap() {
    //add new lap time to lap time list
    //add the item in a way so that the list remains sorted
    std::vector<LAPTIMEENTRY>::iterator idx;

    //remember data from the last two laps, we want to access this information
    //quickly from the Hud without searching in the lap time vector
    //as we need this data every time we want to render a frame!
    if (mPlayerStats->currLapNumber > 1) {
        mPlayerStats->LapBeforeLastLap.lapNr = mPlayerStats->lastLap.lapNr;
        mPlayerStats->LapBeforeLastLap.lapTimeMultiple40mSec = mPlayerStats->lastLap.lapTimeMultiple40mSec;
    }

    if (mPlayerStats->currLapNumber > 0) {
        mPlayerStats->lastLap.lapNr = mPlayerStats->currLapNumber;
        mPlayerStats->lastLap.lapTimeMultiple40mSec = mPlayerStats->currLapTimeMultiple40mSec;
    }

    //make sure we have at least one laptime entry
    if (mPlayerStats->lapTimeList.size() > 0) {
        for(idx = mPlayerStats->lapTimeList.begin(); idx < mPlayerStats->lapTimeList.end(); idx++)
            {
                if (mPlayerStats->currLapTimeMultiple40mSec <  (*idx).lapTimeMultiple40mSec)
                    break;
            }
    } else idx = mPlayerStats->lapTimeList.end();

    LAPTIMEENTRY newEntry;
    newEntry.lapNr = mPlayerStats->currLapNumber;
    newEntry.lapTimeMultiple40mSec = mPlayerStats->currLapTimeMultiple40mSec;

    mPlayerStats->lapTimeList.insert(idx, newEntry);

    mPlayerStats->currLapNumber++;

    LogMessage((char*)"I have finished the current lap");

    //has this player finished the last lap of this race?
    if (mPlayerStats->currLapNumber > mPlayerStats->raceNumberLaps) {
        FinishedRace();
    }

    //do we need to show HUD Message for "final lap"
    if (mPlayerStats->currLapNumber == mPlayerStats->raceNumberLaps) {
        if (this->mRace->currPlayerFollow != nullptr) {
            if (this->mRace->currPlayerFollow == this) {
                if (mHUD != nullptr) {
                    mHUD->ShowGreenBigText((char*)"FINAL LAP", 4.0f, true);
                }

                //in demo mode prevent the yee-haw sound
                //from playing
                if (!mRace->mDemoMode) {
                    //play the yee-haw sound
                    mRace->mSoundEngine->PlaySound(SRES_GAME_FINALLAP, false);
                }
            }
        }
    }

    //reset current lap time
    mPlayerStats->currLapTimeExact = 0.0;
    mPlayerStats->currLapTimeMultiple40mSec = 0;
}

//adds a single random location glas break
void Player::AddGlasBreak() {
    irr::s32 rNum = rand();
    irr::f32 rWidthFloat = (float(rNum) / float (RAND_MAX)) * mRace->mGame->mScreenRes.Width;

    rNum = rand();
    irr::f32 rHeightFloat = (float(rNum) / float (RAND_MAX)) * mRace->mGame->mScreenRes.Height;

    HudDisplayPart* newGlasBreak = new HudDisplayPart();
    newGlasBreak->texture = this->mHUD->brokenGlas->texture;
    newGlasBreak->altTexture = this->mHUD->brokenGlas->altTexture;
    newGlasBreak->sizeTex = this->mHUD->brokenGlas->sizeTex;

    //I decided to also prepare a member variable for the image
    //source rect, so that it is always already available
    //when we draw the Hud over and over again, so that we save CPU cycles
    newGlasBreak->sourceRect.UpperLeftCorner.X = 0;
    newGlasBreak->sourceRect.UpperLeftCorner.Y = 0;
    newGlasBreak->sourceRect.LowerRightCorner.X = newGlasBreak->sizeTex.Width;
    newGlasBreak->sourceRect.LowerRightCorner.Y = newGlasBreak->sizeTex.Height;

    newGlasBreak->drawScrPosition.set((irr::s32)(rWidthFloat), (irr::s32)(rHeightFloat));

    this->brokenGlasVec->push_back(newGlasBreak);
}

//repairs all current glas breaks
void Player::RepairGlasBreaks() {

    if (this->brokenGlasVec->size() > 0) {
        std::vector<HudDisplayPart*>::iterator it;
        HudDisplayPart* pntr;

        for (it = brokenGlasVec->begin(); it != brokenGlasVec->end();) {
            pntr = (*it);

            it = brokenGlasVec->erase(it);

            delete pntr;
        }
    }
}

//deletes all broken glas stuff from heap
void Player::CleanUpBrokenGlas() {
    std::vector<HudDisplayPart*>::iterator it;
    HudDisplayPart* pntr;

    if (this->brokenGlasVec->size() > 0) {
        for (it = brokenGlasVec->begin(); it != brokenGlasVec->end();) {
            pntr = (*it);

            it = brokenGlasVec->erase(it);

            if (pntr->texture != nullptr) {
                //remove underlying texture
                mRace->mGame->mDriver->removeTexture(pntr->texture);
            }

            if (pntr->altTexture != nullptr) {
                //remove underlying texture
                mRace->mGame->mDriver->removeTexture(pntr->altTexture);
            }

            delete pntr;
        }
    }
}

void Player::HandleFuel(irr::f32 deltaTime) {
    //remove some gasoline if we are moving fast enough
    //TODO: check with actual game how gasoline burning works exactly
    if (phobj->physicState.speed > 3.0f) {

        //29.04.2025: for gasoline burn we need to take into accont
        //the current frame rate! otherwise the fuel consumption rate
        //depends heavily on the frame rate of the computer!
        irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

        //the value 0.012f below was determined at my computer at constant
        //60FPS to be correct
        mPlayerStats->gasolineVal -= 0.012f * speedFactor;

        if (mPlayerStats->gasolineVal <= 0.0f) {
            mPlayerStats->gasolineVal = 0.0f;
            if (!mEmptyFuelWarningAlreadyShown) {
                if (mHUD != nullptr) {
                    this->mHUD->ShowBannerText((char*)"FUEL EMPTY", 4.0f, true);
                }
                mEmptyFuelWarningAlreadyShown = true;

                if ((this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_RACING) ||
                (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_ONFIRSTWAYTOFINISHLINE)) {
                    //change player state to empty fuel state
                    SetNewState(STATE_PLAYER_EMPTYFUEL);

                    LogMessage((char*)"I have empty fuel, I call recovery vehicle for help");
                    //call a recovery vehicle to help us out
                    this->mRace->CallRecoveryVehicleForHelp(this);
                    mRecoveryVehicleCalled = true;
                }
            }
        } else if (mPlayerStats->gasolineVal <= 25.0f) {
            if (!mLowFuelWarningAlreadyShown) {
                if (mHUD != nullptr) {
                    this->mHUD->ShowBannerText((char*)"FUEL LOW", 4.0f, true);
                }
                mLowFuelWarningAlreadyShown = true;
            }
        }
    }

    if (mPlayerStats->gasolineVal > 0.0f) {
          mEmptyFuelWarningAlreadyShown = false;

          if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_EMPTYFUEL) {
                //change player state to racing again
                SetNewState(STATE_PLAYER_RACING);
          }
    }

     if (mPlayerStats->gasolineVal > 25.0f) {
          mLowFuelWarningAlreadyShown = false;
     }
}

bool Player::ShouldAmmoBarBlink() {
    if (mLowAmmoWarningAlreadyShown)
        return true;

    if (mEmptyAmmoWarningAlreadyShown)
        return true;

    return false;
}

bool Player::ShouldGasolineBarBlink() {
    if (mLowFuelWarningAlreadyShown)
        return true;

    if (mEmptyFuelWarningAlreadyShown)
        return true;

    return false;
}

bool Player::ShouldShieldBarBlink() {
    if (mLowShieldWarningAlreadyShown)
        return true;

    return false;
}

void Player::HandleAmmo() {
    //low ammo warning is activated if only 2 ammo (missile) or
    //less are left available
    if (mPlayerStats->ammoVal <= 0.9f) {
        if (!mEmptyAmmoWarningAlreadyShown) {
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"AMMO EMPTY", 4.0f, true);
            }
            mEmptyAmmoWarningAlreadyShown = true;
        }
    } else if (mPlayerStats->ammoVal < 2.5f) {
        if (!mLowAmmoWarningAlreadyShown) {
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"AMMO LOW", 4.0f, true);
            }
            mLowAmmoWarningAlreadyShown = true;
        }
    }

     if (mPlayerStats->ammoVal >= 1.0f) {
         mEmptyAmmoWarningAlreadyShown = false;
     }

     if (mPlayerStats->ammoVal > 3.0f) {
          mLowAmmoWarningAlreadyShown = false;
     }
}

void Player::HandleShield() {
    //low shield warning is activated if only 3 shield bars
    //are remaining
    if (mPlayerStats->shieldVal < (mPlayerStats->shieldMax * 0.5f)) {
        if (!mLowShieldWarningAlreadyShown) {
            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"SHIELD LOW", 4.0f, true);
            }
            mLowShieldWarningAlreadyShown = true;
        }
    }

    if (mPlayerStats->shieldVal >= (mPlayerStats->shieldMax * 0.5f)) {
          mLowShieldWarningAlreadyShown = false;
    }
}
