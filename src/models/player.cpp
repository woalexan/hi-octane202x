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
#include "particle.h"
#include "../audio/sound.h"
#include "../draw/hud.h"
#include "../utils/logger.h"
#include "../draw/drawdebug.h"
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

//delivers a random machine gun shoot location at the area of the
//player craft model if the shoot does hit the player (shootDoesHit = true)
//in case shoot does not hit, delivers a random target location around the player
//at the terrain
irr::core::vector3df Player::GetRandomMGunShootTargetLocation(bool shootDoesHit) {
    irr::core::vector3df randLocation;

    if (shootDoesHit) {
        //shoot does hit, return random location at player craft model
        // randLocation.set(mPlayerModelExtend.X * this->mRace->mGame->randFloat(),
        //                  mPlayerModelExtend.Y * this->mRace->mGame->randFloat(),
        //                  mPlayerModelExtend.Z * this->mRace->mGame->randFloat());

        randLocation -= mPlayerModelExtend * irr::core::vector3df(0.5f, 0.5f, 0.5f);
        randLocation += this->phobj->physicState.position;
    } else {
        //shoot does not hit, return random location around player at the terrain
        // randLocation.set(this->mRace->mGame->randFloat(), 0.0f, this->mRace->mGame->randFloat());

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
    //delete mMGun;
    //mMGun = nullptr;

    //free my missile launcher
    //delete mMissileLauncher;
    //mMissileLauncher = nullptr;

    delete dirtTexIdsVec;
    dirtTexIdsVec = nullptr;

    CleanUpBrokenGlas();
    delete this->brokenGlasVec;

    //delete my cpu player
    //delete mCpuPlayer;

    delete mMovingAvgPlayerLeaningAngleLeftRightCalc;
    //delete mMovingAvgPlayerPositionCalc;

    //delete my camera SceneNodes
    this->mIntCamera->remove();
    this->mThirdPersonCamera->remove();
    mSideLookingCamera->remove();
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
        //Human player is allowed to attack
        //Also the HUD is not shown yet
        case STATE_PLAYER_ONFIRSTWAYTOFINISHLINE: {
            this->mPlayerStats->mPlayerCanMove = true;
            this->mPlayerStats->mPlayerCanShoot = true;
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
    //return (this->mMGun->AllAnimationsFinished());
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
    //mCpuPlayer->CpTakeOverHuman();
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

        //original game in this case also refills fuel and ammo completely
        this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;
        this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

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
          //mCpuPlayer->FreedFromRecoveryVehicleAgain();
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
    //mCpuPlayer = new CpuPlayer(this);

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
    Player_node->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);

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

    //lower camera near value to prevent clipping into
    //the terrain
    mIntCamera->setNearValue(0.05f);

    //create my internal camera SceneNode for 3rd person
    mThirdPersonCamera = mRace->mGame->mSmgr->addCameraSceneNode(nullptr, NewPosition);

    mSideLookingCamera = mRace->mGame->mSmgr->addCameraSceneNode(nullptr, NewPosition);

    CalcCraftLocalFeatureCoordinates(NewPosition, NewFrontAt);

    //create my SmokeTrail particle system
    //mSmokeTrail = new SmokeTrail(mRace->mGame->mSmgr, mRace->mGame->mDriver, this, 20);

    //create my Dust cloud emitter particles system
   // mDustBelowCraft = new DustBelowCraft(mRace->mGame->mSmgr, mRace->mGame->mDriver, this, 100);

    //create my machinegun
    //mMGun = new MachineGun(this, mRace->mGame->mSmgr, mRace->mGame->mDriver);

    //create my missile launcher
    //mMissileLauncher = new MissileLauncher(this, mRace->mGame->mSmgr, mRace->mGame->mDriver);

    //create vector to store all the current broken Hud glas locations
    brokenGlasVec = new std::vector<HudDisplayPart*>();
    brokenGlasVec->clear();

    //create a moving average calculation helper for craft leaning angle left/right with average over
    //20 values
    mMovingAvgPlayerLeaningAngleLeftRightCalc = new MovingAverageCalculator(MVG_AVG_TYPE_IRRFLOAT32, 20);

    //create a moving average calculation helper for craft position over 10 samples
    //mMovingAvgPlayerPositionCalc = new MovingAverageCalculator(MVG_AVG_TYPE_IRRCOREVECT3DF, 10);
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
        //mCpuPlayer->SetCurrClosestWayPointLink(newClosestWayPointLink);
    }
}

void Player::ExecuteCpPlayerLogic(irr::f32 deltaTime) {
    if (mHumanPlayer)
        return;

    //mCpuPlayer->RunPlayerLogic(deltaTime);
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

    LocalSideLookingCamPosPnt = WCDirVecCOGtoRight + irr::core::vector3df(-1.5f, 0.4f, 0.0f);
    LocalSideLookingCamTargetPnt = irr::core::vector3df(0.0f, 0.0f, 0.0f);

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

    // LocalCraftFrontPnt = mHMapCollPntData.front->localPnt1;
    // LocalCraftBackPnt = mHMapCollPntData.back->localPnt1;
    // LocalCraftLeftPnt = mHMapCollPntData.left->localPnt1;
    // LocalCraftRightPnt = mHMapCollPntData.right->localPnt1;
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


//returns true if player craft of a computer
//player is currently stuck
//returns always false when called for a human
//player, regardless what the human player currently
//does
bool Player::IsCurrentlyStuck() {
    if (mHumanPlayer)
        return false;

    //return mCpuPlayer->IsCurrentlyStuck();
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

void Player::Collided() {
    if (mHumanPlayer) {
           if (CollisionSound == nullptr) {
              CollisionSound = mRace->mSoundEngine->PlaySound(SRES_GAME_COLLISION, this->phobj->physicState.position, false);
           }
    }
}

void Player::AfterPhysicsUpdate() {
    //TODO: Commented out, because does not compile anymore
    // if (CollisionSound != nullptr) {
    //     if (CollisionSound->getStatus() == sf::SoundSource::Status::Stopped) {
    //         CollisionSound = nullptr;
    //     }
    // }

    // if (this->phobj->CollidedOtherObjectLastTime) {
    //     Collided();
    // }

    // //does another player have currently a missile
    // //lock on us? if so play the warning sound
    // if (this->mRace->currPlayerFollow == this) {
    //     if (mOtherPlayerHasMissleLockAtMe) {
    //         StartPlayingLockOnSound();
    //     } else {
    //         StopPlayingLockOnSound();
    //     }
    // }
}

//when the craft goes into a curve and leans to its side
//we want to be able to draw the background sky also rotated
//to give the player the impression that the drawn sky is realistic
//for this we need to calculate the current angle of the craft relative to the
//Y-axis
// void Player::CalcPlayerCraftLeaningAngle() {
//     this->Player_node->updateAbsolutePosition();
//     irr::core::vector3d<irr::f32> craftUpwardsVec =
//             (WorldCoordCraftAboveCOGStabilizationPoint - this->Player_node->getAbsolutePosition()).normalize();

//     irr::core::vector3d<irr::f32> distVec = (craftUpwardsVec - *mRace->mGame->yAxisDirVector);
//     irr::f32 distVal = distVec.dotProduct(craftSidewaysToRightVec);

//     //calculate angle between upVec and craftUpwardsVec
//     //irr::f32 angleRad = acosf(craftUpwardsVec.dotProduct(upVec));

//     irr::f32 angleRad = acosf(distVal);

//     this->currPlayerCraftLeaningAngleDeg = (angleRad / irr::core::PI) * 180.0f - 90.0f + terrainTiltCraftLeftRightDeg;

//     irr::core::vector3df leaningDirVec = craftUpwardsVec - *mRace->mGame->yAxisDirVector;
//     irr::core::vector3df CraftRightDirVec = (WorldCoordCraftRightPnt - this->Player_node->getAbsolutePosition()).normalize();
//     irr::f32 dotProductRightDir = leaningDirVec.dotProduct(CraftRightDirVec);

//     if (dotProductRightDir > 0.1f) {
//         currPlayerCraftLeaningOrientation = CRAFT_LEANINGRIGHT;
//     } else if (dotProductRightDir < 0.1f) {
//         currPlayerCraftLeaningOrientation = CRAFT_LEANINGLEFT;
//     } else {
//         //no leaning
//         currPlayerCraftLeaningOrientation = CRAFT_NOLEANING;
//     }

//     //derive current craft forwards direction angle referenced to x-Axis
//     //we need this number for computer player control
//     //Important: Here do not correct angle calculation result outside of valid range!
//    // mCurrentCraftOrientationAngle = mRace->GetAbsOrientationAngleFromDirectionVec(craftForwardDirVec, false);

//     distVal = distVec.dotProduct(craftForwardDirVec);

//     angleRad = acosf(distVal);

//     this->currPlayerCraftForwardLeaningAngleDeg = (angleRad / irr::core::PI) * 180.0f - 90.0f + terrainTiltCraftFrontBackDeg;
// }

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
        if (mRace->mGame->mUseXEffects) {
            mRace->mGame->mEffect->removeShadowFromNode(this->phobj->sceneNode);
        }
    }
}

void Player::UnhideCraft() {
    if (!mCraftVisible) {
        mCraftVisible = true;
        this->Player_node->setVisible(true);
        if (mRace->mGame->mUseXEffects) {
            mRace->mGame->mEffect->addShadowToNode(this->phobj->sceneNode, E_FILTER_TYPE::EFT_12PCF);
        }
    }
}

bool Player::DoWeNeedHidePlayerModel() {
    if (mCurrentViewMode == CAMERA_PLAYER_COCKPIT)
        return true;

    return false;
}

irr::scene::ICameraSceneNode* Player::DeliverActiveCamera() {
    // //are we on external view, and we have an external camera available?
    // if (mCurrentViewMode == CAMERA_EXTERNALVIEW) {
    //     if (externalCamera != nullptr) {
    //         //return my external camera
    //         //lets update this external camera, so that it
    //         //does focus at us
    //         externalCamera->Update();

    //         //make sure player model is
    //         //visible again
    //         UnhideCraft();

    //         return externalCamera->mCamSceneNode;
    //     } else {
    //         //as a fallback return my cockpit view
    //         //make sure player model is hidden first
    //         HideCraft();

    //         return mIntCamera;
    //     }
    // }

    // if (mCurrentViewMode == CAMERA_PLAYER_COCKPIT) {
    //       return mIntCamera;
    // }

    // if (mCurrentViewMode == CAMERA_PLAYER_BEHINDCRAFT) {
    //      UnhideCraft();

    //      return mThirdPersonCamera;
    // }

    // if (mCurrentViewMode == CAMERA_PLAYER_SIDELOOKING) {
    //     UnhideCraft();

    //     return mSideLookingCamera;
    // }

    // //no valid view option, return nullptr
    // return nullptr;
}

void Player::ChangeViewMode() {
    if (mCurrentViewMode == CAMERA_PLAYER_COCKPIT) {
         //unhide player craft model so that we can see
         //it again from the outside
         UnhideCraft();

         mCurrentViewMode = CAMERA_PLAYER_BEHINDCRAFT;
    } /*else if (mCurrentViewMode == CAMERA_PLAYER_BEHINDCRAFT) {
        mCurrentViewMode = CAMERA_PLAYER_SIDELOOKING;
    } */ else if (mCurrentViewMode == CAMERA_PLAYER_BEHINDCRAFT) {
        //hide player craft model so that we do not see
        //it in our own camera
        HideCraft();

        mCurrentViewMode = CAMERA_PLAYER_COCKPIT;
    }
}

void Player::UpdateCameras() {
    //update 3rd person camera coordinates
    WorldTopLookingCamPosPnt = this->phobj->ConvertToWorldCoord(this->LocalTopLookingCamPosPnt);
    WorldTopLookingCamTargetPnt = this->phobj->ConvertToWorldCoord(this->LocalTopLookingCamTargetPnt);

    //1st person camera selected
    World1stPersonCamPosPnt = this->phobj->ConvertToWorldCoord(this->Local1stPersonCamPosPnt);
    World1stPersonCamTargetPnt = this->phobj->ConvertToWorldCoord(this->Local1stPersonCamTargetPnt);

    //Side looking camera (for debugging)
    WorldSideLookingCamPosPnt = this->phobj->ConvertToWorldCoord(this->LocalSideLookingCamPosPnt);
    WorldSideLookingCamTargetPnt = this->phobj->ConvertToWorldCoord(this->LocalSideLookingCamTargetPnt);

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

    mSideLookingCamera->setPosition(this->WorldSideLookingCamPosPnt);
    mSideLookingCamera->setTarget(this->WorldSideLookingCamTargetPnt);
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

irr::f32 Player::GetHoverHeight() {
    return HOVER_HEIGHT;
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
        //CraftHeightControl();
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

    //CalcPlayerCraftLeaningAngle();

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
    //TODO: Commented out, does not compile anymore
    // if (this->mRace->currPlayerFollow == this) {
    //     this->mRace->mSoundEngine->SetPlayerSpeed(this, this->mPlayerStats->speed, this->mPlayerStats->speedMax);
    // } else {
    //     //is not the main player (player that we follow right now)
    //     //use spatial engine sound
    //     this->mRace->mSoundEngine->SetPlayerSpeed(this, this->mPlayerStats->speed, this->mPlayerStats->speedMax,
    //                                                      this->phobj->physicState.position);
    // }

    mLastBoosterActive = mBoosterActive;
    mLastMaxTurboActive = mMaxTurboActive;

    //I "measured" it in the original game, around the point of ~33% of the max health a player
    //starts to have a "smoke trail"
    mPlayerModelSmoking = this->mPlayerStats->shieldVal < (0.33f * this->mPlayerStats->shieldMax);

    if (mPlayerModelSmoking != mLastPlayerModelSmoking) {
        if (mPlayerModelSmoking) {
            //player model start smoking
     //       this->mSmokeTrail->Activate();
        } else {
            //player model stop smoking again
       //     this->mSmokeTrail->Deactivate();
        }
    }

    //mSmokeTrail->Update(frameDeltaTime);

    mLastPlayerModelSmoking = mPlayerModelSmoking;

    CheckDustCloudEmitter();

    mDustBelowCraft->Update(frameDeltaTime);

    //mMGun->Update(frameDeltaTime);

    //mMissileLauncher->Update(frameDeltaTime);

    //check if player entered a craft trigger region
    //CheckForTriggerCraftRegion();

    //do we have currently a missile lock at another player
    //if so set flag for warning sound in the other player
    if ((mTargetPlayer != nullptr) && (mTargetMissleLock)) {
        mTargetPlayer->mOtherPlayerHasMissleLockAtMe = true;
    }
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
    //TODO: commented out, does not compile anymore
    // //if someone shoots with machine gun at us, and we are selected
    // //as the player to follow play the riccos sound
    // if ((damageType == DEF_RACE_DAMAGETYPE_MGUN) &&
    //        (this->mRace->currPlayerFollow == this)) {
    //            //Play machine gun shoots at us sounds
    //            PlayMGunShootsAtUsSound();
    // }

    // //only deal positive damage!
    // if ((damage > 0.0f) && (this->mPlayerStats->mPlayerCurrentState != STATE_PLAYER_BROKEN)) {
    //     this->mPlayerStats->shieldVal -= damage;
    //     if (this->mPlayerStats->shieldVal <= 0.0f) {
    //         this->mPlayerStats->shieldVal = 0.0f;

    //         this->WasDestroyed();

    //         return true;
    //    }
    // }

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
       //mCpuPlayer->WasDestroyed();
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
}

void Player::SetupForStart() {
    this->SetNewState(STATE_PLAYER_BEFORESTART);
}

void Player::SetupComputerPlayerForStart(irr::core::vector3df startPos) {
    //make sure we only execute this command for non human
    //players!
    if (!mHumanPlayer) {
        //mCpuPlayer->SetupForRaceStart(startPos);
    }
}

void Player::SetupToSkipStart() {
    this->SetNewState(STATE_PLAYER_RACING);

    //if this is a computer player, set its first target
    //speed
    if (!mHumanPlayer) {
        //mCpuPlayer->StartSignalShowsGreen();
    }
}

void Player::SetupForFirstWayToFinishLine() {
    this->SetNewState(STATE_PLAYER_ONFIRSTWAYTOFINISHLINE);

    //if this is a computer player, set its first target
    //speed
    if (!mHumanPlayer) {
        //mCpuPlayer->StartSignalShowsGreen();
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
    //TODO: commented out, does not compile anymore
    // if (mPlayerStats->currLapNumber == mPlayerStats->raceNumberLaps) {
    //     if (this->mRace->currPlayerFollow != nullptr) {
    //         if (this->mRace->currPlayerFollow == this) {
    //             if (mHUD != nullptr) {
    //                 mHUD->ShowGreenBigText((char*)"FINAL LAP", 4.0f, true);
    //             }

    //             //in demo mode prevent the yee-haw sound
    //             //from playing
    //             if (!mRace->mDemoMode) {
    //                 //play the yee-haw sound
    //                 mRace->mSoundEngine->PlaySound(SRES_GAME_FINALLAP, false);
    //             }
    //         }
    //     }
    // }

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

void Player::SetDebugFlag(irr::u8 debugFlag, bool enable) {
  switch (debugFlag) {
      case DEF_PLAYER_DBG_ALL: {
          mDebugDrawCurrWayPointLink = enable;
          mDebugDrawActingForces = enable;
          mDebugDrawCPUCurrSegment = enable;
          mDebugDrawCPUPathHistory = enable;
          mDebugDrawFreeSpace = enable;
          break;
      }
      case DEF_PLAYER_DBG_CURRWAYPOINTLINK: {
          mDebugDrawCurrWayPointLink = enable;
          break;
      }

      case DEF_PLAYER_DBG_ACTINGFORCES: {
          mDebugDrawActingForces = enable;
          break;
      }

      case DEF_PLAYER_DBG_CPU_CURRSEGMENT: {
          mDebugDrawCPUCurrSegment = enable;
          break;
      }

      case DEF_PLAYER_DBG_CPU_PATHHISTORY: {
          mDebugDrawCPUPathHistory = enable;
          break;
      }

      case DEF_PLAYER_DBG_FREESPACE: {
          mDebugDrawFreeSpace = enable;
          break;
      }

      default: {
          break;
      }
  }
}

bool Player::GetDebugFlag(irr::u8 debugFlag) {
    switch (debugFlag) {
        case DEF_PLAYER_DBG_CURRWAYPOINTLINK: {
            return (mDebugDrawCurrWayPointLink);
        }

        case DEF_PLAYER_DBG_ACTINGFORCES: {
            return (mDebugDrawActingForces);
        }

        case DEF_PLAYER_DBG_CPU_CURRSEGMENT: {
            return (mDebugDrawCPUCurrSegment);
        }

        case DEF_PLAYER_DBG_CPU_PATHHISTORY: {
            return (mDebugDrawCPUPathHistory);
        }

        case DEF_PLAYER_DBG_FREESPACE: {
            return (mDebugDrawFreeSpace);
        }

        default: {
            return (false);
        }
    }
}

void Player::DebugDrawFreeSpace() {
    irr::core::vector3df midPnt = phobj->physicState.position;
    irr::core::vector3df leftPnt = midPnt - craftSidewaysToRightVec * mCraftDistanceAvailLeft;

    //increase Y coordinate slightly, too not get flicker effect between
    //drawn lines and terrain tiles itself
    irr::core::vector2di cell;
    leftPnt.Y = mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(leftPnt.X, leftPnt.Z, cell) + 0.1f;

    mRace->mGame->mDrawDebug->Draw3DLine(midPnt, leftPnt, mRace->mGame->mDrawDebug->red);

    irr::core::vector3df rightPnt = midPnt + craftSidewaysToRightVec * mCraftDistanceAvailRight;

    rightPnt.Y = mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(rightPnt.X, rightPnt.Z, cell) + 0.1f;

    mRace->mGame->mDrawDebug->Draw3DLine(midPnt, rightPnt, mRace->mGame->mDrawDebug->red);

    irr::core::vector3df frontPnt = midPnt + craftForwardDirVec * mCraftDistanceAvailFront;

    frontPnt.Y = mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(frontPnt.X, frontPnt.Z, cell) + 0.1f;

    mRace->mGame->mDrawDebug->Draw3DLine(midPnt, frontPnt, mRace->mGame->mDrawDebug->red);

    irr::core::vector3df backPnt = midPnt - craftForwardDirVec * mCraftDistanceAvailBack;

    backPnt.Y = mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(backPnt.X, backPnt.Z, cell) + 0.1f;

    mRace->mGame->mDrawDebug->Draw3DLine(midPnt, backPnt, mRace->mGame->mDrawDebug->red);
}

void Player::DebugDraw() {
    if (mDebugDrawCurrWayPointLink) {
        if (currClosestWayPointLink.first != nullptr) {
             mRace->mGame->mDrawDebug->Draw3DLine(phobj->physicState.position, currClosestWayPointLink.first->pLineStruct->A,
                                   mRace->mGame->mDrawDebug->cyan);

             mRace->mGame->mDrawDebug->Draw3DLine(phobj->physicState.position, currClosestWayPointLink.first->pLineStruct->B,
                                   mRace->mGame->mDrawDebug->red);

             mRace->mGame->mDrawDebug->Draw3DLine(phobj->physicState.position, currClosestWayPointLink.second,
                                   mRace->mGame->mDrawDebug->blue);
        }
    }

    if (mDebugDrawActingForces) {
        //draw currently active world coordinate forces on player ship
        phobj->DebugDrawCurrentWorldCoordForces(mRace->mGame->mDrawDebug,
                                                mRace->mGame->mDrawDebug->green, PHYSIC_DBG_FORCETYPE_GENERICALL);
    }

    if (mDebugDrawFreeSpace) {
        DebugDrawFreeSpace();
    }

    /*if (!mHumanPlayer && (mCpuPlayer != nullptr)) {
       if (mDebugDrawCPUPathHistory) {
           mCpuPlayer->DebugDrawPathHistory();
       }

       if (mDebugDrawCPUCurrSegment) {
           mCpuPlayer->DebugDrawCurrentSegment();
       }
    }*/
}
