/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "player.h"
#include "../definitions.h"

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
       this->mHUD->AddGlasBreak();
    }
}

void Player::CrossedCheckPoint(irr::s32 valueCrossedCheckPoint, irr::s32 numberOfCheckpoints) {    
    //if this player has already finished the race ignore checkpoints
    if (this->GetCurrentState() == STATE_PLAYER_FINISHED)
        return;

    //crossed checkpoint is the one we need to cross next?
    if (this->nextCheckPointValue == valueCrossedCheckPoint) {
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

    //if this is a computer player setup the path to the next
    //correct checkpoint
    if (!this->mHumanPlayer) {
        mRace->testPathResult = mRace->mPath->FindPathToNextCheckPoint(this);

        this->CpPlayerFollowPath(mRace->testPathResult);
    }
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
    allHitTriangles = this->mRace->mPhysics->ReturnTrianglesHitByRay( this->mRace->mPhysics->mRayTargetSelectors,
                                  startPnt, endPnt, true);

    int vecSize = allHitTriangles.size();
    std::vector<RayHitTriangleInfoStruct*>::iterator it;

    if (vecSize < 1)
        return false;

    if (vecSize == 1) {
        it = allHitTriangles.begin();
        //only one target found, no shorting over distance necessary
        shotTarget = *(*it);

        //cleanup triangle hit information again
        //otherwise we have a memory leak!
        this->mRace->mPhysics->EmptyTriangleHitInfoVector(allHitTriangles);

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
    this->mRace->mPhysics->EmptyTriangleHitInfoVector(allHitTriangles);

    return true;
}

Player::~Player() {
    //free memory of all player stats
    delete mPlayerStats;
    mPlayerStats = NULL;

    //delete/clean all stuff
    //linked to player commands
    CleanUpCommandList();

    //Remove my Scenenode from
    //Scenemanager
    if (this->Player_node != NULL) {
        Player_node->remove();
        Player_node = NULL;
    }

    //Remove my player Mesh
    if (this->PlayerMesh != NULL) {
       this->mRace->mSmgr->getMeshCache()->removeMesh(this->PlayerMesh);
       this->PlayerMesh = NULL;
    }

    //free my SmokeTrail particle system
    delete mSmokeTrail;
    mSmokeTrail = NULL;

    //free my Dust cloud emitter particles system
    delete mDustBelowCraft;
    mDustBelowCraft = NULL;

    //free my machinegun
    delete mMGun;
    mMGun = NULL;

    //free my missile launcher
    delete mMissileLauncher;
    mMissileLauncher = NULL;

    delete dirtTexIdsVec;
    dirtTexIdsVec = NULL;

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
}

void Player::SetNewState(irr::u32 newPlayerState) {
    this->mPlayerStats->mPlayerCurrentState = newPlayerState;

    switch (newPlayerState) {
        case STATE_PLAYER_BEFORESTART: {
            this->mPlayerStats->mPlayerCanMove = false;
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

        case STATE_PLAYER_FINISHED: {
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
}

void Player::SetGrabedByRecoveryVehicle(Recovery* whichRecoveryVehicle) {
    //lets repair (refuel) the player
    if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_EMPTYFUEL) {
        //our fuel is empty, add a little fuel (30% of max value)
        this->mPlayerStats->gasolineVal += 0.3f * this->mPlayerStats->gasolineMax;
    } else if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_BROKEN) {
        //we are broken down, fix the shield
        this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;
    }

   SetNewState(STATE_PLAYER_GRABEDBYRECOVERYVEHICLE);

   mGrabedByThisRecoveryVehicle = whichRecoveryVehicle;
}

void Player::FreedFromRecoveryVehicleAgain() {
   if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_GRABEDBYRECOVERYVEHICLE) {
       mGrabedByThisRecoveryVehicle = NULL;
       SetNewState(STATE_PLAYER_RACING);
   }
}

irr::u32 Player::GetCurrentState() {
    return this->mPlayerStats->mPlayerCurrentState;
}

Player::Player(Race* race, std::string model, irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt, irr::scene::ISceneManager* smgr,
               bool humanPlayer) {

    mPlayerStats = new PLAYERSTATS();

    SetNewState(STATE_PLAYER_RACING);

    mPlayerStats->speed = 0.0f;
    mHumanPlayer = humanPlayer;

    mPlayerStats->shieldVal = 100.0;
    mPlayerStats->gasolineVal = 100.0;
    mPlayerStats->ammoVal = 100.0;
    mPlayerStats->boosterVal = 0.0;
    mPlayerStats->throttleVal = 0.0f;

    //clear list of last lap times
    //integer value, each count equals to 100ms of time
    mPlayerStats->lapTimeList.clear();
    mPlayerStats->lastLap.lapNr = 0;
    mPlayerStats->lastLap.lapTimeMultiple100mSec = 0;
    mPlayerStats->LapBeforeLastLap.lapNr = 0;
    mPlayerStats->LapBeforeLastLap.lapTimeMultiple100mSec = 0;

    mRace = race;

    //create the player command list
    cmdList = new std::list<CPCOMMANDENTRY*>();

    //definition of dirt texture elements
    dirtTexIdsVec = new std::vector<irr::s32>{0, 1, 2, 60, 61, 62, 63, 64, 65, 66, 67, 79};

    //my Irrlicht coordinate system is swapped at the x axis; correct this issue
    //Position = NewPosition;
    //FrontDir = (NewFrontAt-Position).normalize(); //calculate direction vector

    if (DEF_INSPECT_LEVEL == true) {

        PlayerMesh = smgr->getMesh(model.c_str());
        Player_node = smgr->addMeshSceneNode(PlayerMesh);

        //set player model initial orientation and position, later player craft is only moved by physics engine
        //also current change in Rotation of player craft model compared with this initial orientation is controlled by a
        //quaterion inside the physics engine object for this player craft as well
        Player_node->setRotation(((NewFrontAt-NewPosition).normalize()).getHorizontalAngle()+ irr::core::vector3df(0.0f, 180.0f, 0.0f));
        Player_node->setPosition(NewPosition);

       // targetSteerDir = (NewFrontAt-NewPosition).normalize();
       // targetSteerAngle = 0;

        //Player_node->setDebugDataVisible(EDS_BBOX);
        Player_node->setDebugDataVisible(EDS_OFF);

        Player_node->setScale(irr::core::vector3d<irr::f32>(1,1,1));
        Player_node->setMaterialFlag(irr::video::EMF_LIGHTING, false);

        // add shadow
        //Player_node->addShadowVolumeSceneNode();
        //smgr->setShadowColor(video::SColor(150,0,0,0));
    }

    CalcCraftLocalFeatureCoordinates(NewPosition, NewFrontAt);

    //create my SmokeTrail particle system
    mSmokeTrail = new SmokeTrail(this->mRace->mSmgr, this->mRace->mDriver, this, 20);

    //create my Dust cloud emitter particles system
    mDustBelowCraft = new DustBelowCraft(this->mRace->mSmgr, this->mRace->mDriver, this, 7);

    //create my machinegun
    mMGun = new MachineGun(this, mRace->mSmgr, mRace->mDriver);

    //create my missile launcher
    mMissileLauncher = new MissileLauncher(this, mRace->mSmgr, mRace->mDriver);
}

void Player::AddCommand(uint8_t cmdType, EntityItem* targetEntity) {
    if (cmdType != CMD_FLYTO_TARGETENTITY || targetEntity == NULL)
        return;

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = CMD_FLYTO_TARGETENTITY;
    newCmd->targetEntity = targetEntity;

    //create a new temporary waypoint link from computer player crafts current
    //position towards the entity we want to fly to
    WayPointLinkInfoStruct* newStruct = new WayPointLinkInfoStruct();

    LineStruct* newLineStr = new LineStruct();
    newLineStr->A = this->phobj->physicState.position;

    irr::core::vector3df posEntity = targetEntity->get_Pos();
    //our universe is flipped for X axis
    posEntity.X = - posEntity.X;
    newLineStr->B = posEntity;

    //set white as default color
    newLineStr->color = this->mRace->mDrawDebug->white;
    newLineStr->name = new char[10];
    strcpy(newLineStr->name, "");

    newStruct->pLineStruct = newLineStr;
    irr::core::vector3df vec3D = (newLineStr->B - newLineStr->A);

    this->mRace->dbgCoord = newStruct->pLineStruct->B;

    //precalculate and store length as we will need this very often
    //during the game loop for race position update
    newStruct->length3D = vec3D.getLength();
    vec3D.normalize();

    newStruct->LinkDirectionVec = vec3D;
    newStruct->LinkDirectionVec.normalize();

    //also store this temporary waypointlink struct info
    //in the command, so that we can cleanup after this command was
    //executed
    newCmd->targetWaypointLink = newStruct;

    //mark waypoint link as temporary created, so that
    //we delete it again after command was fully executed
    newCmd->WayPointLinkTemporary = true;

    newStruct->pEndEntity = targetEntity;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void Player::AddCommand(uint8_t cmdType, WayPointLinkInfoStruct* targetWayPointLink) {
    if (cmdType != CMD_FOLLOW_TARGETWAYPOINTLINK || targetWayPointLink == NULL)
        return;

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = CMD_FOLLOW_TARGETWAYPOINTLINK;
    newCmd->targetWaypointLink = targetWayPointLink;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void Player::SetCurrClosestWayPointLink(WayPointLinkInfoStruct* newClosestWayPointLink) {
    this->currClosestWayPointLink = newClosestWayPointLink;

    //non human player start following first closest waypoint link
    //we see via race logic
    if (!mHumanPlayer) {
        if (computerCurrFollowWayPointLink == NULL) {
            computerCurrFollowWayPointLink = newClosestWayPointLink;
        } else {
            if (computerCurrFollowWayPointLink->pntrPathNextLink == newClosestWayPointLink) {
                computerCurrFollowWayPointLink = newClosestWayPointLink;
            }
        }
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
        remChars -= currLen;

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
        remChars -= currLen;

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
        remChars -= currLen;

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
        remChars -= currLen;

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
        remChars -= currLen;

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
        remChars -= currLen;

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
        remChars -= currLen;

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
        remChars -= currLen;

        if (remChars > 0) {
         wcscat(outputText, entry);
        } else
            return;
    }
}

void Player::GetHeightMapCollisionSensorDebugStateName(HMAPCOLLSENSOR *collSensor, char **stateName) {
    *stateName = new char[20];

    strcpy(*stateName, "");

    if (collSensor != NULL) {
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

    if (collSensor != NULL) {
        char *stateName;

        //only create debug text if state is not idle
        GetHeightMapCollisionSensorDebugStateName(collSensor, &stateName);

        swprintf(outputText, maxCharNr, L"%s: %s %lf %lf %u\n",
                     collSensor->sensorName,
                     stateName,
                     collSensor->stepness,
                     collSensor->distance,
                     collSensor->collCnt);

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

    Local1stPersonCamPosPnt = (LocalCraftFrontPnt) * irr::core::vector3df(0.0f, 0.0f, 0.5f);
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

void Player::Forward() {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

        //to accelerate player add force in craft forward direction
        this->phobj->AddLocalCoordForce(irr::core::vector3df(0.0f, 0.0f, 0.0f), irr::core::vector3df(0.0f, 0.0f, -50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

        if (mPlayerStats->throttleVal < mPlayerStats->throttleMax)
            mPlayerStats->throttleVal++;
}

void Player::CPForward() {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

        //to accelerate player add force in craft forward direction
        this->phobj->AddLocalCoordForce(irr::core::vector3df(0.0f, 0.0f, 0.0f), irr::core::vector3df(0.0f, 0.0f, -50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

        if (mPlayerStats->throttleVal < mPlayerStats->throttleMax)
            mPlayerStats->throttleVal++;
}

void Player::Backward() {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

    if (!DEF_PLAYERCANGOBACKWARDS) {
        //we can not go backwards in Hioctane
        //we can only add friction to brake
        this->phobj->AddFriction(10.0f);

        if (mPlayerStats->throttleVal > 0)
            mPlayerStats->throttleVal--;
    } else {
            //go solution during debugging, for example testing collisions, it helps to be able to accelerate backwards
            this->phobj->AddLocalCoordForce(irr::core::vector3df(0.0f, 0.0f, 0.0f), irr::core::vector3df(0.0f, 0.0f, 50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

            if (mPlayerStats->throttleVal > 0)
                mPlayerStats->throttleVal--;
    }
}

void Player::CPBackward() {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

    if (!DEF_PLAYERCANGOBACKWARDS) {
        //we can not go backwards in Hioctane
        //we can only add friction to brake
        this->phobj->AddFriction(10.0f);

        if (mPlayerStats->throttleVal > 0)
            mPlayerStats->throttleVal--;
    } else {
            //go solution during debugging, for example testing collisions, it helps to be able to accelerate backwards
            this->phobj->AddLocalCoordForce(irr::core::vector3df(0.0f, 0.0f, 0.0f), irr::core::vector3df(0.0f, 0.0f, 50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

            if (mPlayerStats->throttleVal > 0)
                mPlayerStats->throttleVal--;
    }
}

void Player::Left() {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

        currentSideForce += 30.0f;

        if (currentSideForce > 200.0f)
            currentSideForce = 200.0f;

   //original line 10.11.2024
  /*this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt + irr::core::vector3df(currentSideForce, 0.0f, 0.0f),
                                   PHYSIC_APPLYFORCE_ONLYROT);*/

  //experimental line
  this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt + irr::core::vector3df(currentSideForce, 0.0f, 0.0f),
                                         PHYSIC_APPLYFORCE_ONLYROT);
}

void Player::Right() {
    //if player can not move right now simply
    //exit
    if (!this->mPlayerStats->mPlayerCanMove)
        return;

    currentSideForce -= 30.0f;

    if (currentSideForce < -200.0f)
        currentSideForce = -200.0f;

    //original line 10.11.2024
    /*this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt + irr::core::vector3df(currentSideForce, 0.0f, 0.0f),
                                     PHYSIC_APPLYFORCE_ONLYROT);*/

    //experimental
    this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt + irr::core::vector3df(currentSideForce, 0.0f, 0.0f),
                                     PHYSIC_APPLYFORCE_ONLYROT);
}

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

void Player::CPForceController() {

    mLastCurrentCraftOrientationAngle = mCurrentCraftOrientationAngle;
    mLastCraftDistToWaypointLink = mCurrentCraftDistToWaypointLink;
    //cpLastFollowSeg = cPCurrentFollowSeg;

    //control computer player speed
    if (this->phobj->physicState.speed < (computerPlayerTargetSpeed * 0.9f))
    {
        //go faster
        this->CPForward();
         //this->mHUD->ShowBannerText((char*)"FORWARD", 4.0f);
    } else if (this->phobj->physicState.speed > (computerPlayerTargetSpeed * 1.1f)) {
        //go slower
       this->CPBackward();
         //this->mHUD->ShowBannerText((char*)"BACKWARD", 4.0f);
    }

    if (this->cPCurrentFollowSeg != NULL) {
        computerPlayerTargetSpeed = 2.0f;

        //we need to project current computer player craft
        //position onto the current segment we follow
        //this recalculates the current projPlayerPositionClosestWayPointLink member
        //variable for the further calculation steps below
        //ProjectPlayerAtCurrentSegment();
        ProjectPlayerAtCurrentSegments();
  /*
        if (cPCurrentFollowSeg != cpLastFollowSeg) {
            cpLastFollowSeg = cPCurrentFollowSeg;

            mLastCurrentCraftOrientationAngle = mCurrentCraftOrientationAngle;
            mLastCraftDistToWaypointLink = mCurrentCraftDistToWaypointLink;
        }*/

        irr::core::vector3df dirVecToLink = (this->projPlayerPositionFollowSeg - this->phobj->physicState.position);
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

        //what is our relation towards the currClosestWayPoint link?
        irr::f32 absAngleLinkForwardDir =
                this->mRace->GetAbsOrientationAngleFromDirectionVec(this->cPCurrentFollowSeg->LinkDirectionVec);

        mCurrentWaypointLinkAngle = absAngleLinkForwardDir;

        mCurrentCraftOrientationAngle =
                this->mRace->GetAbsOrientationAngleFromDirectionVec(craftForwardDirVec);

        irr::f32 angleError = absAngleLinkForwardDir - mCurrentCraftOrientationAngle + mCurrentCraftTargetOrientationOffsetAngle;
        irr::f32 currAngleVelocityCraft =  (mCurrentCraftOrientationAngle - mLastCurrentCraftOrientationAngle);

        irr::f32 currDistanceChangeRate = mCurrentCraftDistToWaypointLink - mLastCraftDistToWaypointLink;

        /***************************************/
        /*  Control Craft absolute angle start */
        /***************************************/

        irr::f32 corrForceOrientationAngle = 3.0f;
        irr::f32 corrDampingOrientationAngle = 2000.0f;

        irr::f32 corrForceAngle = angleError * corrForceOrientationAngle - currAngleVelocityCraft * corrDampingOrientationAngle;

        //we need to limit max force, if force is too high just
        //set it zero, so that not bad physical things will happen!
        if (fabs(corrForceAngle) > 1000.0f) {
            corrForceAngle = 0.0f;
        }

        this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt + irr::core::vector3df(corrForceAngle, 0.0f, 0.0f),
                                             PHYSIC_APPLYFORCE_ONLYROT);

        /****************************************************/
        /*  Control Craft distance to current waypoint link */
        /****************************************************/

        irr::f32 corrForceDist = 3.0f;
        irr::f32 corrDampingDist = 2000.0f;

        irr::f32 distError = (mCurrentCraftDistToWaypointLink - mCurrentCraftDistWaypointLinkTarget);

        irr::f32 corrForceDistance = distError * corrForceDist + currDistanceChangeRate * corrDampingDist;

        if (corrForceDistance > 1.0f) {
            corrForceDistance = 1.0f;
        } else if (corrForceDistance < -1.0f) {
            corrForceDistance = -1.0f;
        }

        this->phobj->AddLocalCoordForce(LocalCraftOrigin, LocalCraftOrigin + irr::core::vector3df(corrForceDistance, 0.0f, 0.0f),
                                            PHYSIC_APPLYFORCE_ONLYTRANS);
    } else {
        //no segment to follow, stop craft
        this->computerPlayerTargetSpeed = 0.0f;
    }
}

void Player::ProjectPlayerAtCurrentSegment() {
    irr::core::vector3df dA;
    irr::core::vector3df dB;

    irr::f32 projecteddA;
    irr::f32 projecteddB;
    irr::f32 projectedPl;
    //irr::f32 distance;

    irr::core::vector3df projPlayerPosition;

    dA = phobj->physicState.position - cPCurrentFollowSeg->pLineStruct->A;
    dB = phobj->physicState.position - cPCurrentFollowSeg->pLineStruct->B;

    projecteddA = dA.dotProduct(cPCurrentFollowSeg->LinkDirectionVec);
    projecteddB = dB.dotProduct(cPCurrentFollowSeg->LinkDirectionVec);

    //if craft position is parallel (sideways) to current waypoint link the two projection
    //results need to have opposite sign; otherwise we are not sideways of this line, and need to ignore
    //this path segment
    if (sgn(projecteddA) != sgn(projecteddB)) {
        //we seem to be still parallel to this segment => projection will still give useful results
        //calculate distance from player position to this line, where connecting line meets path segment
        //in a 90° angle
        projectedPl =  dA.dotProduct(cPCurrentFollowSeg->LinkDirectionVec);

        /*
        (*WayPointLink_iterator)->pLineStruct->debugLine = new irr::core::line3df((*WayPointLink_iterator)->pLineStruct->A +
                                                                                  projectedPl * (*WayPointLink_iterator)->LinkDirectionVec,
                                                                                    player->phobj->physicState.position);*/

        projPlayerPosition = (cPCurrentFollowSeg->pLineStruct->A +
                irr::core::vector3df(projectedPl, projectedPl, projectedPl) * (cPCurrentFollowSeg->LinkDirectionVec));

        //distance = (projPlayerPosition - phobj->physicState.position).getLength();

        projPlayerPositionFollowSeg = projPlayerPosition;
    }
}

void Player::ProjectPlayerAtCurrentSegments() {
    std::vector<WayPointLinkInfoStruct*>::iterator WayPointLink_iterator;
    irr::core::vector3df dA;
    irr::core::vector3df dB;

    irr::f32 projecteddA;
    irr::f32 projecteddB;
    irr::f32 projectedPl;
    irr::f32 distance;
    irr::f32 minDistance;
    bool firstElement = true;
    irr::core::vector3df projPlayerPosition;
    WayPointLinkInfoStruct* closestLink = NULL;
    WayPointLinkInfoStruct* LinkWithClosestStartEndPoint = NULL;
    irr::f32 minStartEndPointDistance;
    bool firstElementStartEndPoint = true;

    irr::f32 startPointDistHlper;
    irr::f32 endPointDistHlper;
    irr::core::vector3d<irr::f32> posH;

    //iterate through all player follow way segments
    for(WayPointLink_iterator = mCurrentPathSeg.begin(); WayPointLink_iterator != mCurrentPathSeg.end(); ++WayPointLink_iterator) {

        //for the workaround later (in case first waypoint link search does not work) also find in parallel the waypoint link that
        //has a start or end-point closest to the current player location
        posH = (*WayPointLink_iterator)->pLineStruct->A;
        startPointDistHlper = ((phobj->physicState.position - posH)).getLengthSQ();

        posH = (*WayPointLink_iterator)->pLineStruct->B;
        endPointDistHlper = ((phobj->physicState.position - posH)).getLengthSQ();

        if (endPointDistHlper < startPointDistHlper) {
             startPointDistHlper = endPointDistHlper;
         }

        if (firstElementStartEndPoint) {
           LinkWithClosestStartEndPoint = (*WayPointLink_iterator);
           minStartEndPointDistance = startPointDistHlper;
           firstElementStartEndPoint = false;
        } else {
            if (startPointDistHlper < minStartEndPointDistance) {
                //we have a new closest start/end point
                LinkWithClosestStartEndPoint = (*WayPointLink_iterator);
                minStartEndPointDistance = startPointDistHlper;
            }
        }

        //we want to find the waypoint link (line) to which the player is currently closest too (which the player currently tries to follow)
        //we also want to only find the line which is sideways of the player
        //first check if player is parallel to current line, or if the line is far away
        dA = phobj->physicState.position - (*WayPointLink_iterator)->pLineStruct->A;
        dB = phobj->physicState.position - (*WayPointLink_iterator)->pLineStruct->B;

        projecteddA = dA.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);
        projecteddB = dB.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);

        //if craft position is parallel (sideways) to current waypoint link the two projection
        //results need to have opposite sign; otherwise we are not sideways of this line, and need to ignore
        //this path segment
        if (sgn(projecteddA) != sgn(projecteddB)) {
            //this waypoint is interesting for further analysis
            //calculate distance from player position to this line, where connecting line meets path segment
            //in a 90° angle
            projectedPl =  dA.dotProduct((*WayPointLink_iterator)->LinkDirectionVec);

            /*
            (*WayPointLink_iterator)->pLineStruct->debugLine = new irr::core::line3df((*WayPointLink_iterator)->pLineStruct->A +
                                                                                      projectedPl * (*WayPointLink_iterator)->LinkDirectionVec,
                                                                                      player->phobj->physicState.position);*/

            projPlayerPosition = (*WayPointLink_iterator)->pLineStruct->A +
                    irr::core::vector3df(projectedPl, projectedPl, projectedPl) * ((*WayPointLink_iterator)->LinkDirectionVec);

            distance = (projPlayerPosition - phobj->physicState.position).getLength();

            //(*WayPointLink_iterator)->pLineStruct->color = mDrawDebug->pink;

            //prevent picking far away waypoint links
            //accidently (this happens especially when we are between
            //the end of the current waypoint link and the start
            //of the next one)
            if (distance < 10.0f) {

            if (firstElement) {
                minDistance = distance;
                closestLink = (*WayPointLink_iterator);
                projPlayerPositionFollowSeg = projPlayerPosition;
                firstElement = false;
            } else {
                if (distance < minDistance) {
                    minDistance = distance;
                    closestLink = (*WayPointLink_iterator);
                    projPlayerPositionFollowSeg = projPlayerPosition;
                 }
              }
          }
        }
    }

    //did we still not find the closest link? try some workaround
    if (closestLink == NULL) {
       //workaround, take the waypoint with either the closest
       //start or end entity
       closestLink = LinkWithClosestStartEndPoint;
    }

    /*if ((closestLink != NULL) && (whichPlayer == player)) {
        closestLink->pLineStruct->color = mDrawDebug->green;
    }*/

/*    if ((LinkWithClosestStartEndPoint != NULL) && (whichPlayer == player)) {
       LinkWithClosestStartEndPoint->pLineStruct->color = mDrawDebug->red;
    }*/

   cPCurrentFollowSeg = closestLink;

   //at which number of segment are we currently?
   mCurrentPathSegCurrSegmentNr = 0;

   for(WayPointLink_iterator = mCurrentPathSeg.begin(); WayPointLink_iterator != mCurrentPathSeg.end(); ++WayPointLink_iterator) {
       if ((*WayPointLink_iterator) == cPCurrentFollowSeg) {
           break;
       }

       mCurrentPathSegCurrSegmentNr++;
   }

   //have we reached the end of the following path we follow?
   if (mCurrentPathSegCurrSegmentNr >= (mCurrentPathSegNrSegments -1)) {
      ReachedEndCurrentFollowingSegments();
   }
}

void Player::ReachedEndCurrentFollowingSegments() {
    mFollowPathCurrentNrLink--;

    if (mFollowPathCurrentNrLink < 0) {
        //path has no more links
    } else {
        //create bezier curve for the next link
        //of the specified path
        FollowPathDefineNextSegment(mFollowPathCurrentNrLink);
    }
}

void Player::HeightMapCollisionResolve(irr::core::plane3df cplane, irr::core::vector3df pnt1, irr::core::vector3df pnt2) {
    irr::f32 dist;
    irr::core::vector3df outIntersect;
    irr::core::vector3df collResolutionDirVec;

    //allow max 10 iterations to resolve collision
    //to prevent endless loops
    for (int resolvCnt = 0; resolvCnt < 10; resolvCnt++) {
        if (cplane.getIntersectionWithLimitedLine(pnt1, pnt2, outIntersect)) {
            /*irr::core::vector2di outCell;
            //we need to correct Y coordinate of intersection point, so that it is at the height of the tile below
            outIntersect.Y =
                    this->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(outIntersect.X, outIntersect.Z, outCell);

            dbgInterset = outIntersect;*/

            collResolutionDirVec = (outIntersect - pnt1);
            dist = collResolutionDirVec.getLength();
        } else {
            //no more intersection
            break;
        }

        dbgDistance = dist;

        //if we have enough distance exit
        if (dist > 0.0f)
            break;

        if (resolvCnt == 0) {
            //a collision should occur, play collision sound
            this->Collided();
        }

        //prevent collision detection to modify Y coordinate, this should
        //eliminate all kind of unwanted effects, like moving the craft
        //upwards a hill during a continious collision situation
        //with the terrain etc.
        collResolutionDirVec.Y = 0.0f;

        //add a force that pushes the object away from the terrain tile we collided with
        this->phobj->physicState.position -= collResolutionDirVec * dist;
        this->phobj->AddWorldCoordForce(this->phobj->physicState.position,
                this->phobj->physicState.position - collResolutionDirVec * 2.0f, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);
     }

    //Also slow the craft down considerably by adding friction
    //because otherwise the player gets no penalty by colliding with walls
    this->phobj->AddFriction(2.0f);
}

void Player::ExecuteHeightMapCollisionDetection() {
     UpdateHMapCollisionPointData();

     //Execute the terrain heightmap tile collision detection
     //only if we do this morphing will also have an effect
     //on the craft movements
     HeightMapCollision(*mHMapCollPntData.front);
     HeightMapCollision(*mHMapCollPntData.frontLeft45deg);
     HeightMapCollision(*mHMapCollPntData.frontRight45deg);
     HeightMapCollision(*mHMapCollPntData.left);
     HeightMapCollision(*mHMapCollPntData.right);
     HeightMapCollision(*mHMapCollPntData.back);
     HeightMapCollision(*mHMapCollPntData.backLeft45deg);
     HeightMapCollision(*mHMapCollPntData.backRight45deg);

     //store heightmap collision detection debugging results for frame
     StoreHeightMapCollisionDbgRecordingDataForFrame();
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
void Player::HeightMapCollision(HMAPCOLLSENSOR &collSensor) {

    irr::f32 dist;
    irr::core::vector3df collPlanePos1Coord;
    irr::core::vector3df collPlanePos2Coord;
    irr::core::vector3df collResolutionDirVector;
    irr::core::vector3df outIntersect;
    irr::core::vector3df delta;
    irr::core::vector3df deltaGround;
    irr::f32 step;
    irr::core::plane3df cplane;

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
                this->Collided();

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
                this->Collided();

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
}

void Player::StoreHeightMapCollisionDbgRecordingDataForFrame() {
    //if currently no recording is done just return
    if (this->hMapCollDebugWhichSensor == NULL)
        return;

    if (this->hMapCollDebugRecordingData == NULL)
        return;

    HMAPCOLLSENSOR* newDataPoint = new HMAPCOLLSENSOR();
    //copy the data
    *newDataPoint = *hMapCollDebugWhichSensor;

    //store recording data from the current frame in the recording vector
    this->hMapCollDebugRecordingData->push_back(newDataPoint);
}

void Player::StartRecordingHeightMapCollisionDbgData(HMAPCOLLSENSOR *whichCollSensor) {
    //if we record already just return
    if (this->hMapCollDebugWhichSensor != NULL)
        return;

    //if no valid sensor is specified also return
    if (whichCollSensor == NULL)
        return;

    //create new recording data vector
    this->hMapCollDebugRecordingData = new std::vector<HMAPCOLLSENSOR*>();

    hMapCollDebugWhichSensor = whichCollSensor;
}

void Player::StopRecordingHeightMapCollisionDbgData(char* outputDbgFileName) {
    //if we do not record right now just return
    if (this->hMapCollDebugWhichSensor == NULL)
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
    this->hMapCollDebugRecordingData = NULL;

    hMapCollDebugWhichSensor = NULL;
}

void Player::Collided() {
    if (mHumanPlayer) {
           if (CollisionSound == NULL) {
              CollisionSound = mRace->mSoundEngine->PlaySound(SRES_GAME_COLLISION, false);
           }
    }
}

void Player::AfterPhysicsUpdate() {
    if (CollisionSound != NULL) {
        if (CollisionSound->getStatus() == CollisionSound->Stopped) {
            CollisionSound = NULL;
        }
    }

    if (this->phobj->CollidedOtherObjectLastTime) {
        Collided();
    }
}

void Player::MaxTurboReached() {
    mMaxTurboActive = true;

    if (mMaxTurboActive != mLastMaxTurboActive) {
        if (mMaxTurboActive) {
            //we just reached max Turbo
            //now play booster sound
            mRace->mSoundEngine->PlaySound(SRES_GAME_BOOSTER, false);

            //give the player model a speed boost
            this->phobj->AddLocalCoordForce(irr::core::vector3df(0.0f, 0.0f, 0.0f), irr::core::vector3df(0.0f, 0.0f, -500.0f), PHYSIC_APPLYFORCE_REAL,
                                        PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

            //reduce air friction to give player faster speeds with turbo
            this->phobj->SetAirFriction(CRAFT_AIRFRICTION_TURBO);
        } else {
            //we left max turbo again
        }
    }
}

void Player::IsSpaceDown(bool down) {
    mBoosterActive = down;

    //if this player can not move prevent
    //new activation of booster
    if (!mPlayerStats->mPlayerCanMove) {
        mBoosterActive = false;
    }

    //in case max turbo/booster state was reached
    //a new turbo is not possible
    if (mMaxTurboActive) {
        mBoosterActive = false;
    }

        if (mBoosterActive) {

           if (mLastBoosterActive != mBoosterActive) {
                //booster was activated!
                //keep a pointer to the sound source for the Turbo,
                //because maybe we need to interrupt the turbo sound if space key is released
                //by the player
                TurboSound = mRace->mSoundEngine->PlaySound(SRES_GAME_TURBO, false);

                this->mPlayerStats->boosterVal = 0;
           }

           this->mPlayerStats->boosterVal += 1.0;

           if (this->mPlayerStats->boosterVal >= this->mPlayerStats->boosterMax) {
               //we reached max turbo level
               //make sure turbo sound is stopped
               TurboSound->stop();

               MaxTurboReached();
           }

        } else {

              if (mLastBoosterActive != mBoosterActive) {

                //space key was released
                //make sure turbo sound is stopped
                TurboSound->stop();

                //if space key is released the booster sound is played as well
                mRace->mSoundEngine->PlaySound(SRES_GAME_BOOSTER, false);

                mBoosterActive = false;
              }

              if (this->mPlayerStats->boosterVal > 0)  {
                  this->mPlayerStats->boosterVal -= 0.2;
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
    irr::core::vector3d<irr::f32> craftUpwardsVec = (WorldCoordCraftAboveCOGStabilizationPoint - this->Player_node->getAbsolutePosition()).normalize();

    irr::core::vector3d<irr::f32> distVec = (craftUpwardsVec - *mRace->yAxisDirVector);
    irr::f32 distVal = distVec.dotProduct(craftSidewaysToRightVec);

    //calculate angle between upVec and craftUpwardsVec
    //irr::f32 angleRad = acosf(craftUpwardsVec.dotProduct(upVec));

    irr::f32 angleRad = acosf(distVal);

    this->currPlayerCraftLeaningAngleDeg = (angleRad / irr::core::PI) * 180.0f - 90.0f + terrainTiltCraftLeftRightDeg;

    irr::core::vector3df leaningDirVec = craftUpwardsVec - *mRace->yAxisDirVector;
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

void Player::FollowPathDefineNextSegment(irr::u32 nrCurrentLink) {
    //create bezier curve
    //start point is the current players position
    //control point is the start point of the link
    //in the path with the specified number
    //end point is the end point of the link in the
    //path with the defined number
    irr::core::vector2df bezierPnt1;
    //irr::core::vector3df bezierPnt13D = this->phobj->physicState.position;
    irr::core::vector3df bezierPnt13D = this->WorldCoordCraftBackPnt;

    bezierPnt1.X = bezierPnt13D.X;
    bezierPnt1.Y = bezierPnt13D.Z;

    irr::core::vector2df bezierPnt2;
    irr::core::vector3df bezierPnt23D = mFollowPath.at(nrCurrentLink)->pEndEntity->get_Pos();

    bezierPnt2.X = -bezierPnt23D.X;
    bezierPnt2.Y = bezierPnt23D.Z;

    irr::core::vector2df bezierPntcntrl;
    irr::core::vector3df bezierPntcntrl3D = mFollowPath.at(nrCurrentLink)->pStartEntity->get_Pos();

    bezierPntcntrl.X = -bezierPntcntrl3D.X;
    bezierPntcntrl.Y = bezierPntcntrl3D.Z;

    mCurrentPathSeg = mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, bezierPnt2, bezierPntcntrl,
                                                                    0.1f, mRace->mDrawDebug->blue);

    mCurrentPathSegNrSegments = mCurrentPathSeg.size();

    ProjectPlayerAtCurrentSegments();
}

void Player::CpPlayerFollowPath(std::vector<WayPointLinkInfoStruct*> path) {
    if (path.size() > 0) {
        mFollowPath = path;
        mFollowPathNrLinks = path.size();
        bool didFindPlayerInPath = false;

        //we need to make sure that the waypoint link of this path
        //where the start from lies completely in front of the player
        //otherwise the craft would fly backwards
        std::vector<WayPointLinkInfoStruct*>::iterator it;

        mFollowPathCurrentNrLink = mFollowPathNrLinks - 1;

        for (it = mFollowPath.begin(); it != mFollowPath.end(); ++it) {
            if (this->currClosestWayPointLink == (*it)) {
               if (mFollowPathCurrentNrLink > 0) {
                  mFollowPathCurrentNrLink--;
                }

               didFindPlayerInPath = true;

               break;
            }

            if (mFollowPathCurrentNrLink > 0) {
              mFollowPathCurrentNrLink--;
            }
        }

        //we did not find player, assume player
        //is slightly before path, first element is the
        //the fist waypoint link of the path
        if (!didFindPlayerInPath) {
             mFollowPathCurrentNrLink = mFollowPathNrLinks - 1;
        }

    } else {
        return;
    }

    //create bezier curve for the first link
    //of the specified path
    FollowPathDefineNextSegment(mFollowPathCurrentNrLink);
}

irr::core::vector3df Player::DeriveCurrentDirectionVector(WayPointLinkInfoStruct *currentWayPointLine, irr::f32 progressCurrWayPoint) {
    if (currentWayPointLine->pntrPathNextLink != NULL) {
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
}

void Player::ShowPlayerBigGreenHudText(char* text, irr::f32 timeDurationShowTextSec) {
    this->mHUD->ShowGreenBigText(text, timeDurationShowTextSec);
}

void Player::FlyTowardsEntityRunComputerPlayerLogic(CPCOMMANDENTRY* currCommand) {
    //if we run this method for human player
    //just exit

    if (mHumanPlayer)
        return;

    //is there really a target entity and waypoint link information?
     if ((currCommand->targetWaypointLink != NULL) && (currCommand->targetEntity != NULL)) {
        //yes, there is

        cPCurrentFollowSeg = currCommand->targetWaypointLink;

        irr::core::vector3df entPos = currCommand->targetEntity->get_Pos();
        entPos.X = -entPos.X;

        //have we reached the target yet?
        irr::f32 distToTarget = (this->phobj->physicState.position -
                                 entPos).getLength();

        if (distToTarget < 2.0f) {
            //we reached the target
            //this->mHUD->ShowBannerText((char*)"TARGET REACHED", 4.0f);

            //mark current command as finished, pull the next one
            CurrentCommandFinished();

            cPCurrentFollowSeg = NULL;

            return;
        }
    }
}

CPCOMMANDENTRY* Player::CreateNoCommand() {
    CPCOMMANDENTRY* newcmd = new CPCOMMANDENTRY();
    newcmd->cmdType = CMD_NOCMD;
    newcmd->targetEntity = NULL;
    newcmd->targetPosition = NULL;
    newcmd->targetWaypointLink = NULL;

    return newcmd;
}

CPCOMMANDENTRY* Player::PullNextCommandFromCmdList() {
    if (this->cmdList->size() <= 0) {
        //there is no other command in the queue

        //create a new command
        CpDefineNextAction();

        //if still no command available, create no command "cmd"
        if (this->cmdList->size() <= 0) {
            //create an empty command
            return CreateNoCommand();
        }
    }

    CPCOMMANDENTRY* cmd = (CPCOMMANDENTRY*)(this->cmdList->front());

    return cmd;
}

//Function to free all currently pending
//commands, and command list
void Player::CleanUpCommandList() {
    std::list<CPCOMMANDENTRY*>::iterator it;
    CPCOMMANDENTRY* pntrCmd;

    if (this->cmdList->size() > 0) {
       for (it = this->cmdList->begin(); it != cmdList->end(); ) {
           pntrCmd = (*it);

           it = this->cmdList->erase(it);

           //free command struct itself
           //as well
           if (pntrCmd->cmdType == CMD_FLYTO_TARGETENTITY) {
               //we have to do maybe more cleanup
               if (pntrCmd->targetWaypointLink != NULL) {
                   //if temporary waypoint link (created for a specific purpose,
                   //not part of level file), clean up again
                   if (pntrCmd->WayPointLinkTemporary) {
                       //clean up waypoint link structure again
                       //we need to clean up the LineStruct inside
                       LineStruct* pntrLineStruct = pntrCmd->targetWaypointLink->pLineStruct;

                       delete[] pntrLineStruct->name;
                       delete pntrLineStruct;

                       delete pntrCmd->targetWaypointLink;
                   }
               }
           }

           delete pntrCmd;
       }
    }

    //free cmdList object
    delete cmdList;
    cmdList = NULL;
}

void Player::CurrentCommandFinished() {
    CPCOMMANDENTRY* oldCmd = currCommand;
    this->cmdList->pop_front();

    //if we set currCommand to NULL then the program
    //will pull the next available command in
    //RunComputerPlayerLogic
    currCommand = NULL;

    if (oldCmd->cmdType == CMD_FLYTO_TARGETENTITY) {
        //we have to do maybe more cleanup
        if (oldCmd->targetWaypointLink != NULL) {
            //if temporary waypoint link (created for a specific purpose,
            //not part of level file), clean up again
            if (oldCmd->WayPointLinkTemporary) {
                //clean up waypoint link structure again
                //we need to clean up the LineStruct inside
                LineStruct* pntrLineStruct = oldCmd->targetWaypointLink->pLineStruct;

                delete[] pntrLineStruct->name;
                delete pntrLineStruct;

                delete oldCmd->targetWaypointLink;
            }
        }
    }

    //delete old command struct
    delete oldCmd;
}

void Player::RunComputerPlayerLogic() {
    this->CpCurrMissionState = CP_MISSION_FINISHLAPS;

    if (currCommand == NULL) {
        currCommand = PullNextCommandFromCmdList();
    }

    switch (currCommand->cmdType) {
    case CMD_NOCMD: {
          this->mHUD->ShowBannerText((char*)"NO CMD", 4.0f);
        break;
    }

    case CMD_FLYTO_TARGETENTITY: {
            FlyTowardsEntityRunComputerPlayerLogic(currCommand);
        break;
    }

    case CMD_FLYTO_TARGETPOSITION: {
        break;
    }

    case CMD_FOLLOW_TARGETWAYPOINTLINK: {
        FollowWayPointLink();
        break;
    }

    }

    return;
}

void Player::CpAddCommandTowardsNextCheckpoint() {
    //which waypoint is currently close to the player
    /*WayPointLinkInfoStruct* closest = this->mRace->PlayerFindClosestWaypointLink(this);

    std::vector<WayPointLinkInfoStruct*> foundLinks;

    foundLinks = this->mRace->FindWaypointLinksForWayPoint(cPCurrentFollowSeg->pStartEntity);
    std::vector<WayPointLinkInfoStruct*>::iterator it;

    if (foundLinks.size() > 0) {
        for (it = foundLinks.begin(); it < foundLinks.end(); ++it) {

        if (cPCurrentFollowSeg->pEndEntity == (*it)->pStartEntity) {
            AddCommand(CMD_FLYTO_TARGETENTITY, (*it)->pStartEntity);
            break;
        }
      }
    }*/

    //what waypoint entities do I see currently?
    std::vector<EntityItem*> wayPointAroundMeVec =
            this->mRace->mPath->FindAllWayPointsInArea(this->phobj->physicState.position, 10.0f);

    //if there is no waypoint, I do not know what to do, exit
    if (wayPointAroundMeVec.size() <= 0) {
        return;
    }

    std::vector<EntityItem*>::iterator it;
    std::vector<WayPointLinkInfoStruct*> structPntrVec;
    std::vector<WayPointLinkInfoStruct*> overallWaypointLinkList;

    std::vector<WayPointLinkInfoStruct*>::iterator it2;

    vector< pair <irr::f32, WayPointLinkInfoStruct*> > availWayPointLinksWithDistanceVecPair;
    availWayPointLinksWithDistanceVecPair.clear();
    overallWaypointLinkList.clear();

    for (it = wayPointAroundMeVec.begin(); it != wayPointAroundMeVec.end(); ++it) {
        structPntrVec = mRace->mPath->FindWaypointLinksForWayPoint(*it);

        for (it2 = structPntrVec.begin(); it2 != structPntrVec.end(); ++it2) {
            overallWaypointLinkList.push_back(*it2);
        }
    }

    //irr::core::vector3df dirVecStart;
    //irr::core::vector3df dirVecEnd;

    //irr::f32 dotProdStart;
    //irr::f32 dotProdEnd;

    //now we have a vector all close waypoint links available
    for (it2 = structPntrVec.begin(); it2 != structPntrVec.end(); ++it2) {

      /*  dirVecStart = (this->phobj->physicState.position - (*it2)->pStartEntity->get_Pos()).normalize();
        dirVecEnd = (this->phobj->physicState.position - (*it2)->pEndEntity->get_Pos()).normalize();

        dotProdStart = this->craftForwardDirVec.dotProduct(dirVecStart);
        dotProdEnd = this->craftForwardDirVec.dotProduct(dirVecEnd);

        if ((dotProdStart > 0.0f) && (dotProdEnd > 0.0f)) {*/
            availWayPointLinksWithDistanceVecPair.push_back(
                    //for each option calculate the distance until we hit the next checkpoint
                    make_pair( this->mRace->mPath->CalculateDistanceFromWaypointLinkToNextCheckpoint(*it2), (*it2)));
        //}
    }

    //sort vector pairs in descending value for remaining distance to next checkpoint
   std::sort(availWayPointLinksWithDistanceVecPair.rbegin(), availWayPointLinksWithDistanceVecPair.rend());

   //start with the last element in sorted vector (which is the waypoint link with the
   //least remaining distance to next checkpoint
   auto it4 = availWayPointLinksWithDistanceVecPair.rbegin();

   WayPointLinkInfoStruct* currLink;

   //take the waypoint link with the shortest distance
   currLink = (*it4).second;

   //are we closer to the start or end of this waypoint link?
   irr::f32 distStart = (this->phobj->physicState.position - currLink->pStartEntity->get_Pos()).getLength();
   irr::f32 distEnd = (this->phobj->physicState.position - currLink->pEndEntity->get_Pos()).getLength();

   if (distStart < distEnd) {
            //we are closer to the start point of this waypoint link
            //next command is to go to the end of this waypoint link
            AddCommand(CMD_FLYTO_TARGETENTITY, currLink->pEndEntity);
            return;
        } else {
            //we are closer to the end point of this waypoint link
            //next command is to go to the start to the following waypoint link
            AddCommand(CMD_FLYTO_TARGETENTITY, currLink->pntrPathNextLink->pStartEntity);
            return;
          }
}

void Player::CpDefineNextAction() {
    //depending on the current computer player logic
    //mission state, define the next players action
    switch (CpCurrMissionState) {
        case CP_MISSION_FINISHLAPS: {
            //This Mission means we are happy with fuel, shield, etc..
            //we simply want to finish laps as fast as possible
            //CpAddCommandTowardsNextCheckpoint();
            break;
        }
    }
}

void Player::FollowWayPointLink() {
    //if we run this method for human player
    //just exit

    if (mHumanPlayer)
        return;

    //do we already follow a Waypoint link?
    //if (computerCurrFollowWayPointLink != NULL) {
     if (computerCurrFollowWayPointLink != NULL) {
        //yes, we do
        /*
        //control computer player speed
        if (this->phobj->physicState.speed < (computerPlayerTargetSpeed * 0.9f))
        {
            //go faster
            this->Forward();
             //this->mHUD->ShowBannerText((char*)"FORWARD", 4.0f);
        } else if (this->phobj->physicState.speed > (computerPlayerTargetSpeed * 1.1f)) {
            //go slower
           this->Backward();
             //this->mHUD->ShowBannerText((char*)"BACKWARD", 4.0f);
        }*/

        irr::core::vector3d<irr::f32> mDirVecToMyRightSide = (WorldCoordCraftRightPnt - this->phobj->physicState.position);

        //control craft sideways
        //project player on waypoint link we currently
        //follow
        irr::core::vector3d<irr::f32> projPlayerPositionFrontCraft;
        irr::core::vector3d<irr::f32> distanceVecFrontCraft;
        irr::f32 remainingDistanceToTravelFront;

        irr::core::vector3d<irr::f32> projPlayerPositionBackCraft;
        irr::core::vector3d<irr::f32> distanceVecBackCraft;
        irr::f32 remainingDistanceToTravelBack;
        WayPointLinkInfoStruct* controlLink = computerCurrFollowWayPointLink;

        bool sideWaysOfWaypointFront = true;
        bool SideWaysofWayPointBack = true;

      //  if (ProjectOnWayPoint(computerCurrFollowWayPointLink, &projPlayerPosition, &distanceVec, &remainingDistanceToTravel)) {
        sideWaysOfWaypointFront = sideWaysOfWaypointFront && ProjectOnWayPoint(computerCurrFollowWayPointLink, this->WorldCoordCraftFrontPnt ,&projPlayerPositionFrontCraft, &distanceVecFrontCraft, &remainingDistanceToTravelFront);
        SideWaysofWayPointBack = SideWaysofWayPointBack && ProjectOnWayPoint(computerCurrFollowWayPointLink, this->WorldCoordCraftBackPnt ,&projPlayerPositionBackCraft, &distanceVecBackCraft, &remainingDistanceToTravelBack);

        //if we are not sideways of any waypoint link the
        //following calculations would lead to temporary false
        //computer player steer inputs, and would throw us of track
        //therefore in this case just exit, and hope the next time we get
        //an acceptable waypoint again
        if (!sideWaysOfWaypointFront || !SideWaysofWayPointBack) {
            if (!sideWaysOfWaypointFront && !SideWaysofWayPointBack) {
                this->mHUD->ShowBannerText((char*)"NO REF FRONT BACK", 0.1f);
             } else

            if (!sideWaysOfWaypointFront) {
                this->mHUD->ShowBannerText((char*)"NO REF FRONT", 0.1f);
             } else
            if (!SideWaysofWayPointBack) {
                this->mHUD->ShowBannerText((char*)"NO REF BACK", 0.1f);
            }

            if (computerCurrFollowWayPointLink->pntrTransitionLink != NULL)
                controlLink = computerCurrFollowWayPointLink->pntrTransitionLink;

            if (controlLink == NULL) {
                return;
            } else {
                //project again, this time on the alternative transition waypoint link
                sideWaysOfWaypointFront = sideWaysOfWaypointFront && ProjectOnWayPoint(controlLink, this->WorldCoordCraftFrontPnt ,&projPlayerPositionFrontCraft, &distanceVecFrontCraft, &remainingDistanceToTravelFront);
                SideWaysofWayPointBack = SideWaysofWayPointBack && ProjectOnWayPoint(controlLink, this->WorldCoordCraftBackPnt ,&projPlayerPositionBackCraft, &distanceVecBackCraft, &remainingDistanceToTravelBack);
            }
        }

        CPcurrWaypointLinkFinishedPerc = 100.0f - (remainingDistanceToTravelFront / (controlLink->length3D / 100.0f));

        irr::core::vector3df dirVectorFollow = DeriveCurrentDirectionVector(controlLink, CPcurrWaypointLinkFinishedPerc / 100.0f);

        //calculate angle between craft forward direction and current waypoint link direction
        //so that we know if we are parallel to the waypoint link or not
        irr::f32 angleRad = acosf(this->craftForwardDirVec.dotProduct(dirVectorFollow));
        computerPlayerCurrShipWayPointLinkSide = distanceVecFrontCraft.dotProduct(mDirVecToMyRightSide);

        computerPlayerCurrDistanceFromWayPointLinkFront = distanceVecFrontCraft.getLength();
        computerPlayerCurrDistanceFromWayPointLinkBack = distanceVecBackCraft.getLength();

        //define cPCurrRelativeAngle in a way, that a craft flying towards the current waypoint line
        //leads to a negative relative angle, and a craft flying aways from the current
        //refererence waypoint link leads to a positive result
        if (computerPlayerCurrShipWayPointLinkSide < 0) {
           cPCurrRelativeAngle = -(angleRad / irr::core::PI) * 180.0f;
        } else {
            cPCurrRelativeAngle = (angleRad / irr::core::PI) * 180.0f;
        }

        irr::core::vector3d<irr::f32> avgDistanceVec = (distanceVecFrontCraft + distanceVecBackCraft)*irr::core::vector3df(0.5f, 0.5f, 0.5f);
        computerPlayerCurrDistanceFromWayPointLinkAvg = avgDistanceVec.getLength();

        //I want to define distance in a way, that if player ship is left of current waypoint link, then distance is negative
        //and if player is right of current waypoint link the distance becomes positive
        if (computerPlayerCurrShipWayPointLinkSide < 0.0) {
            cPcurrDistance = computerPlayerCurrDistanceFromWayPointLinkAvg;
        } else {
            cPcurrDistance = -computerPlayerCurrDistanceFromWayPointLinkAvg;
        }

        cPTargetDistance = 0.0f;
        cPcurrDistanceErr = cPTargetDistance - cPcurrDistance;

        //define current

        //distance error is too high, we have to correct something
        if ((abs(cPcurrDistanceErr) > 0.3f)/* && (abs(cPcurrDistanceErr) < 2.0f)*/) {
            if (cPcurrDistanceErr > 0.0f) {
                //we are too far at the left
                //we have to place ourself more right on the race track
                //if we are left of the current waypoint line this means
                //we need a negative target steer angle to go towards the
                //waypoint line and more to the right
                cPTargetRelativeAngle = -5.0f;

                cPCorrectingPosErr = (abs(cPcurrDistanceErr) / 2.0f);
                cPCurrCorrectingPos = true;

                this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt + irr::core::vector3df(-3.0f, 0.0f, 0.0f),
                                              PHYSIC_APPLYFORCE_ONLYTRANS);

                 this->mHUD->ShowBannerText((char*)"DIST TOO LEFT", 0.1f);
             } else {
                //we are too far right on the race track
                //we have to go more to the left
                cPTargetRelativeAngle = 5.0f;

                cPCorrectingPosErr = (cPcurrDistanceErr / 2.0f);
                cPCurrCorrectingPos = true;

                this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt + irr::core::vector3df(+3.0f, 0.0f, 0.0f),
                                              PHYSIC_APPLYFORCE_ONLYTRANS);

                this->mHUD->ShowBannerText((char*)"DIST TOO RIGHT", 0.1f);
            }
        } else  {
            //just go straight
            cPTargetRelativeAngle = 0.0f;
            cPCurrCorrectingPos = false;

            this->mHUD->ShowBannerText((char*)"DIST OK", 0.1f);
            CpTriggerTurn(CP_TURN_NOTURN, 0.0f);
        }

        /*if (cPCurrCorrectingPos) {
            if (abs(cPcurrDistanceErr) < abs(cPCorrectingPosErr)) {
                //start turning in the other direction
                //to prevent craft reaction overshoot
                cPTargetRelativeAngle = -cPTargetRelativeAngle;
                cPCurrCorrectingPos = false;
                this->mHUD->ShowBannerText((char*)"GEGEN", 0.1f);
            }
        }*/

        cPCurrRelativeAngleErr = cPTargetRelativeAngle - cPCurrRelativeAngle;
        irr::f32 turnAngle = 2.0f;

        turnAngle = turnAngle * ((abs(cPcurrDistanceErr)) / 1.0f);

        if (turnAngle < 1.0f)
            turnAngle = 1.0f;

        if (turnAngle > 5.0f)
            turnAngle = 5.0f;

        //too high error on target relative angle
        //we need to do something
        if (abs(cPCurrRelativeAngleErr) > 3.0f) {
            //if (abs(cPCurrRelativeAngle) < 30.0f) {
            if (true) {

            if (cPCurrRelativeAngleErr > 0.0f) {
               CpTriggerTurn(CP_TURN_LEFT, turnAngle);
            } else {
               CpTriggerTurn(CP_TURN_RIGHT, turnAngle);
            }
            }
            else {

                this->NoTurningKeyPressed();
               /* if (cPCurrRelativeAngleErr > 0.0f) {
                   CpTriggerTurn(CP_TURN_RIGHT, 2.0f);
                } else {
                   CpTriggerTurn(CP_TURN_LEFT, 2.0f);
                }*/

            }
        } else {
            //just go straight
            //NoTurningKeyPressed();
            cPProbTurnLeft = 0;
            cPProbTurnRight = 0;
            CpTriggerTurn(CP_TURN_NOTURN, 0.0f);

        }

    } else {
         //we have no waypoint link anymore
  //       this->NoTurningKeyPressed();
          this->mHUD->ShowBannerText((char*)"NO WAY", 4.0f);
             CpTriggerTurn(CP_TURN_NOTURN, 0.0f);
     }

     CPForceController();
}

void Player::GetPlayerCameraDataFirstPerson(irr::core::vector3df &world1stPersonCamPosPnt, irr::core::vector3df &world1stPersonCamTargetPnt) {
    world1stPersonCamPosPnt = this->World1stPersonCamPosPnt;
    world1stPersonCamTargetPnt = this->World1stPersonCamTargetPnt;
}

void Player::GetPlayerCameraDataThirdPerson(irr::core::vector3df &worldTopLookingCamPosPnt, irr::core::vector3df &worldTopLookingCamTargetPnt) {
    worldTopLookingCamPosPnt = this->WorldTopLookingCamPosPnt;
    worldTopLookingCamTargetPnt = this->WorldTopLookingCamTargetPnt;
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

void Player::Update(irr::f32 frameDeltaTime) {
    if ((mPlayerStats->mPlayerCurrentState != STATE_PLAYER_FINISHED)
        && (mPlayerStats->mPlayerCurrentState != STATE_PLAYER_BEFORESTART)) {
            //advance current lap lap time, frameDeltaTime is in seconds
            mPlayerStats->currLapTimeExact += frameDeltaTime;
       }

    updateSlowCnter += frameDeltaTime;

    this->mPlayerStats->speed = this->phobj->physicState.speed;

    if (updateSlowCnter >= 0.1) {
        updateSlowCnter = 0.0f;

        if (mHumanPlayer) {
            this->mRace->mSoundEngine->SetPlayerSpeed(this->mPlayerStats->speed, this->mPlayerStats->speedMax);
        }

        if (mMaxTurboActive) {

        }

        //handle missle lock timing logic
        if (this->mTargetMissleLockProgr > 0) {
            mTargetMissleLockProgr--;

            if (mTargetMissleLockProgr == 0) {
                //we have achieved missle lock
                this->mTargetMissleLock = true;
            }
        }
    }

    //calculate lap time for Hud display
    mPlayerStats->currLapTimeMultiple100mSec = (mPlayerStats->currLapTimeExact * 10);

    //calculate player craft world coordinates
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

    //*****************************************************
    //* Hovercraft height control force calculation Start *  solution 1: with the 4 local points left, right, front and back of craft
    //*****************************************************

    //establish height information of race track below player craft
    GetHeightRaceTrackBelowCraft(currHeightFront, currHeightBack, currHeightLeft, currHeightRight);

    DbgCurrRaceTrackHeightFront = currHeightFront;
    DbgCurrRaceTrackHeightBack = currHeightBack;
    DbgCurrRaceTrackHeightLeft = currHeightLeft;
    DbgCurrRaceTrackHeightRight = currHeightRight;

    irr::f32 leftMore = 0.0f;
    irr::f32 rightMore = 0.0f;

    if (true) {
        if (abs(currentSideForce) > 40.0f) {
            //leftMore = 2.0f * (-currentSideForce / 100.0f);  original line, works best until 04.09.2024
            //rightMore = 2.0f * (currentSideForce / 100.0f);  original line, works best until 04.09.2024

            leftMore = 1.0f * (-currentSideForce / 200.0f);   //new attempt since 04.09.2024
            rightMore = 1.0f * (currentSideForce / 200.0f);    //new attempt since 04.09.2024


        if (leftMore < 0.0f)
            leftMore = 0.0f;

        if (rightMore < 0.0f)
            rightMore = 0.0f;
        }
    }

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

    irr::f32 adjusth;

  /*  if (maxh > 0.0f) {
        adjusth = maxh * 0.5f;
    } else adjusth = 0.0f;*/

    adjusth = 0.0f;

    irr::f32 heightErrorFront = (WorldCoordCraftFrontPnt.Y - (currHeightFront + HOVER_HEIGHT + adjusth));
    irr::f32 heightErrorBack = (WorldCoordCraftBackPnt.Y - (currHeightBack + HOVER_HEIGHT + adjusth));
    irr::f32 heightErrorLeft = (WorldCoordCraftLeftPnt.Y - (currHeightLeft + HOVER_HEIGHT + leftMore + adjusth));
    irr::f32 heightErrorRight = (WorldCoordCraftRightPnt.Y - (currHeightRight + HOVER_HEIGHT+ rightMore + adjusth));

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

    //if craft is at all points higher than racetrack let it go towards racetrack slower (too allow something like a jump)
    if ((heightErrorFront > 0.0f) && (heightErrorBack > 0.0f) && (heightErrorLeft > 0.0f) && (heightErrorRight > 0.0f)) {
        corrForceHeight = 40.0f;
    }

    irr::f32 corrForceFront = heightErrorFront * corrForceHeight + this->phobj->GetVelocityLocalCoordPoint(LocalCraftFrontPnt).Y * corrDampingHeight;
    this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, LocalCraftFrontPnt - irr::core::vector3df(0.0f, corrForceFront, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

    irr::f32 corrForceBack = heightErrorBack * corrForceHeight + this->phobj->GetVelocityLocalCoordPoint(LocalCraftBackPnt).Y * corrDampingHeight;
    this->phobj->AddLocalCoordForce(LocalCraftBackPnt, LocalCraftBackPnt - irr::core::vector3df(0.0f, corrForceBack, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

    irr::f32 corrForceLeft = heightErrorLeft * corrForceHeight + this->phobj->GetVelocityLocalCoordPoint(LocalCraftLeftPnt).Y * corrDampingHeight;
    this->phobj->AddLocalCoordForce(LocalCraftLeftPnt, LocalCraftLeftPnt - irr::core::vector3df(0.0f, corrForceLeft, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

    irr::f32 corrForceRight = heightErrorRight * corrForceHeight + this->phobj->GetVelocityLocalCoordPoint(LocalCraftRightPnt).Y * corrDampingHeight;
    this->phobj->AddLocalCoordForce(LocalCraftRightPnt, LocalCraftRightPnt - irr::core::vector3df(0.0f, corrForceRight, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

    /************ Update player camera stuff ***************/
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

    /************ Update player camera stuff end ************/

    //check if this player is located at a charging station (gasoline, ammo or shield)
    CheckForChargingStation();

    //remove some gasoline if we are moving fast enough
    //TODO: check with actual game how gasoline burning works exactly
    if (phobj->physicState.speed > 3.0f) {
        mPlayerStats->gasolineVal -= 0.02f;

        if (mPlayerStats->gasolineVal <= 0.0f) {
            if ((mHUD != NULL) && !mEmptyFuelWarningAlreadyShown) {
                this->mHUD->ShowBannerText((char*)"FUEL EMPTY", 4.0f, true);
                mEmptyFuelWarningAlreadyShown = true;

                if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_RACING) {
                    //change player state to empty fuel state
                    SetNewState(STATE_PLAYER_EMPTYFUEL);

                    //call a recovery vehicle to help us out
                    this->mRace->CallRecoveryVehicleForHelp(this);
                }
            }
        } else if (mPlayerStats->gasolineVal < (mPlayerStats->gasolineMax * 0.25f)) {
            if ((mHUD != NULL) && !mLowFuelWarningAlreadyShown) {
                this->mHUD->ShowBannerText((char*)"FUEL LOW", 4.0f, true);
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

     if (mPlayerStats->gasolineVal > (mPlayerStats->gasolineMax * 0.25f)) {
          mLowFuelWarningAlreadyShown = false;
     }

    //TODO: check with actual game how ammo warnings and reduction works exactly
    if (mPlayerStats->ammoVal <= 0.0f) {
        if ((mHUD != NULL) && !mEmptyAmmoWarningAlreadyShown) {
            this->mHUD->ShowBannerText((char*)"AMMO EMPTY", 4.0f, true);
            mEmptyAmmoWarningAlreadyShown = true;
        }
    } else if (mPlayerStats->ammoVal < (mPlayerStats->ammoMax * 0.25f)) {
        if ((mHUD != NULL) && !mLowAmmoWarningAlreadyShown) {
            this->mHUD->ShowBannerText((char*)"AMMO LOW", 4.0f, true);
            mLowAmmoWarningAlreadyShown = true;
        }
    }

     if (mPlayerStats->ammoVal > 0.0f) {
         mEmptyAmmoWarningAlreadyShown = false;
     }

     if (mPlayerStats->ammoVal > (mPlayerStats->ammoMax * 0.25f)) {
          mLowAmmoWarningAlreadyShown = false;
     }

    CalcPlayerCraftLeaningAngle();

    /* calculate player abs angle average list for sky rendering */
    if (playerAbsAngleSkytListElementNr > 20) {
        this->playerAbsAngleSkyList.pop_front();
        playerAbsAngleSkytListElementNr--;
    }

    this->playerAbsAngleSkyList.push_back(this->currPlayerCraftLeaningAngleDeg);
    playerAbsAngleSkytListElementNr++;

    irr::f32 avgSkyVal = 0.0f;

    for (itList = this->playerAbsAngleSkyList.begin(); itList != this->playerAbsAngleSkyList.end(); ++itList) {
        avgSkyVal += (*itList);
    }

    avgSkyVal = (avgSkyVal / (irr::f32)(playerAbsAngleSkytListElementNr));
    absSkyAngleValue = avgSkyVal;

    /* calculate player abs angle average list for sky rendering end */

    mLastBoosterActive = mBoosterActive;
    mLastMaxTurboActive = mMaxTurboActive;

    mPlayerModelSmoking = this->mPlayerStats->shieldVal < (0.3 * this->mPlayerStats->shieldMax);

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

    //*****************************************************
    //* Hovercraft height control force calculation End   *
    //*****************************************************

    //calculate local Wheel steering angle, starting point is orientation of ship
    //WheelSteerDir = this->phobj->physicState.orientation;

    //rotate WheelSteerDir further defined by current steering angle
    //irr::core::quaternion rotateFurther;
    //rotateFurther.fromAngleAxis((targetSteerAngle / 180.0f) * irr::core::PI, irr::core::vector3df(0.0f, 1.0f, 0.0f));

    //WheelSteerDir *= rotateFurther;

    //WorldCoordWheelDir = (WorldCoordCraftFrontPnt - WorldCoordCraftBackPnt).normalize();
    //WorldCoordCraftTravelDir = WorldCoordWheelDir;
    //LocalCoordWheelDir = (LocalCraftFrontPnt - LocalCraftBackPnt).normalize();

    //irr::core::matrix4 matrx;
    //rotateFurther.getMatrix_transposed(matrx);
    //matrx.rotateVect(WorldCoordWheelDir);
    //matrx.rotateVect(LocalCoordWheelDir);
    //WorldCoordWheelDir.normalize();
    //LocalCoordWheelDir.normalize();

}

void Player::SetName(char* playerName) {
    strcpy(this->mPlayerStats->name, playerName);
}

//returns TRUE if player reached below/equal 0 health (therefore if
//player died); otherwise false is returned
bool Player::Damage(irr::f32 damage) {
    //only deal positive damage!
    if ((damage > 0.0f) && (this->mPlayerStats->mPlayerCurrentState != STATE_PLAYER_BROKEN)) {
        this->mPlayerStats->shieldVal -= damage;
        if (this->mPlayerStats->shieldVal <= 0.0f) {
            this->mPlayerStats->shieldVal = 0.0f;
            playerCurrentlyDeath = true;

            //Player has now zero shield and is broken
            //Call recovery vehicle for help and set new
            //player state
            SetNewState(STATE_PLAYER_BROKEN);
            this->mRace->CallRecoveryVehicleForHelp(this);

            return true;
       }
    }

    return false;
}

void Player::SetMyHUD(HUD* pntrHUD) {
    mHUD = pntrHUD;
}

HUD* Player::GetMyHUD() {
    return mHUD;
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

//Helper function for finding charging stations in a second way
bool Player::CanIFindTextureIdAroundPlayer(int posX, int posY, int textureId) {
    //should return true if below or one tile around the player
    //we find the specified texture Id, false otherwise
    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX, posY, textureId))
        return true;

    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX - 1, posY, textureId))
        return true;

    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX + 1, posY, textureId))
        return true;

    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX, posY - 1, textureId))
        return true;

    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX, posY + 1, textureId))
        return true;

    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX - 1, posY - 1, textureId))
        return true;

    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX + 1, posY + 1, textureId))
        return true;

    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX + 1, posY - 1, textureId))
        return true;

    if (this->mRace->mLevelTerrain->levelRes->CheckTileForTextureId(posX - 1, posY + 1, textureId))
        return true;

    //nothing found, return false
    return false;
}

void Player::CheckForChargingStation() {
    bool currChargingFuel = false;
    bool currChargingAmmo = false;
    bool currChargingShield = false;

    //get information about current tile below player craft

    //calculate current cell below player
    int current_cell_calc_x, current_cell_calc_y;

    current_cell_calc_y = (this->phobj->physicState.position.Z / mRace->mLevelTerrain->segmentSize);
    current_cell_calc_x = -(this->phobj->physicState.position.X / mRace->mLevelTerrain->segmentSize);

    //MapEntry* tilePntr = this->mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);

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

    bool atCharger = false;

    bool cShield;
    bool cFuel;
    bool cAmmo;

    //see if we are currently in an charging area with this player
    this->mRace->mLevelTerrain->CheckPosInsideChargingRegion(current_cell_calc_x, current_cell_calc_y,
                                                             cShield, cFuel, cAmmo);

    //is this a shield recharge, we know via floor tile texture ID, workaround for level 4, 8 ...
    if (CanIFindTextureIdAroundPlayer(current_cell_calc_x, current_cell_calc_y, 51)) {
        cShield = true;
    }

    if (cShield) {
       currChargingShield = true;

       if (mPlayerStats->shieldVal < mPlayerStats->shieldMax)
        {
           atCharger = true;

            this->mPlayerStats->shieldVal++;
            this->mHUD->RepairGlasBreaks();

            if (mHUD != NULL) {
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

    //is this a ammo recharge, we know via floor tile texture ID, workaround for level 4, 8 ...
    if (CanIFindTextureIdAroundPlayer(current_cell_calc_x, current_cell_calc_y, 47)) {
        cAmmo = true;
    }

    if (cAmmo) {
       currChargingAmmo = true;

       if (mPlayerStats->ammoVal < mPlayerStats->ammoMax)
        {
            atCharger = true;

            this->mPlayerStats->ammoVal++;
            if (mHUD != NULL) {
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

    //is this a fuel recharge, we know via floor tile texture ID, workaround for level 4, 8 ...
    if (CanIFindTextureIdAroundPlayer(current_cell_calc_x, current_cell_calc_y, 43)) {
        cFuel = true;
    }

    if (cFuel) {
       currChargingFuel = true;

       if (mPlayerStats->gasolineVal < mPlayerStats->gasolineMax)
        {
            atCharger = true;

            this->mPlayerStats->gasolineVal++;

             if (mHUD != NULL) {
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

    if (currChargingFuel != mLastChargingFuel) {
        if (currChargingFuel) {
            //charging fuel started
             if (mHUD != NULL && atCharger) {
               //make this a permanent message by specification of showDurationSec = -1.0f
               this->mHUD->ShowBannerText((char*)"FUEL RECHARGING", -1.0f);
             }

        } else {
            this->mHUD->CancelAllPermanentBannerTextMsg();
            mBlockAdditionalFuelFullMsg = false;
        }
    }

    if (currChargingAmmo != mLastChargingAmmo) {
        if (currChargingAmmo) {
            //charging Ammo started
             if (mHUD != NULL && atCharger) {
               //make this a permanent message by specification of showDurationSec = -1.0f
               this->mHUD->ShowBannerText((char*)"AMMO RECHARGING", -1.0f);
             }

        } else {
            this->mHUD->CancelAllPermanentBannerTextMsg();
            mBlockAdditionalAmmoFullMsg = false;
        }
    }

    if (currChargingShield != mLastChargingShield) {
        if (currChargingShield) {
            //charging shield started
             if (mHUD != NULL && atCharger) {
               //make this a permanent message by specification of showDurationSec = -1.0f
               this->mHUD->ShowBannerText((char*)"SHIELD RECHARGING", -1.0f);
             }

        } else {
            this->mHUD->CancelAllPermanentBannerTextMsg();
            mBlockAdditionalShieldFullMsg = false;
        }
    }

    mLastChargingFuel = currChargingFuel;
    mLastChargingAmmo = currChargingAmmo;
    mLastChargingShield = currChargingShield;

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
                   mChargingSoundSource = NULL;
               }
       }

    }
}

void Player::StartPlayingWarningSound() {
   //already warning playing?
   if (mWarningSoundSource == NULL) {
       //no, start playing new warning
       //we need to keep a pntr to the looping sound source to be able to stop it
       //later again!
       if (mHumanPlayer) {
        mWarningSoundSource = this->mRace->mSoundEngine->PlaySound(SRES_GAME_WARNING, true);
       }
   }
}

void Player::StopPlayingWarningSound() {
   //warning really playing?
   if (mWarningSoundSource != NULL) {
       //yes, stop it
       if (mHumanPlayer) {
         this->mRace->mSoundEngine->StopLoopingSound(mWarningSoundSource);
         mWarningSoundSource = NULL;
       }
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
    //calculate current cell below player
    int current_cell_calc_x, current_cell_calc_y;

    current_cell_calc_y = (this->phobj->physicState.position.Z / mRace->mLevelTerrain->segmentSize);
    current_cell_calc_x = -(this->phobj->physicState.position.X / mRace->mLevelTerrain->segmentSize);

    MapEntry* tilePntr = this->mRace->mLevelTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y);
    irr::s32 texId = tilePntr->m_TextureId;

    mEmitDustCloud = false;

    //check if our texture ID is present in the dirt tex id list
    //if so then emit clouds
    for (std::vector<irr::s32>::iterator itTex = dirtTexIdsVec->begin(); itTex != dirtTexIdsVec->end(); ++itTex) {
        if ((*itTex) == texId) {
            mEmitDustCloud = true;
            break;
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

    irr::core::vector3df pos_in_worldspace_frontPos(LocalCraftFrontPnt);
    this->Player_node->updateAbsolutePosition();
    irr::core::matrix4 matr = this->Player_node->getAbsoluteTransformation();

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

    //currTileBelowPlayer = NULL;

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
}

void Player::CpTriggerTurn(uint8_t newTurnMode, irr::f32 turnAngle) {
   // if (cPCurrentTurnMode == CP_TURN_NOTURN) {
        //we are currently making no turn, we can start a
        //new turn
        cPCurrentTurnMode = newTurnMode;
        //cPStartTurnAngle = mCurrentCraftOrientationAngle;
        cPStartTurnOrientation = this->craftForwardDirVec;
        cPTargetTurnAngle = turnAngle;

/*        if (newTurnMode == CP_TURN_LEFT) {
            cpEndTurnAngle = cPStartTurnAngle + turnAngle;

        } else if (newTurnMode == CP_TURN_RIGHT) {
            //cpEndTurnAngle = cPStartTurnAngle - turnAngle;
            cPTargetTurnAngle = turnAngle;
        }*/

        if (cPTargetTurnAngle > 360.0f)
            cPTargetTurnAngle -= 360.0f;

        if (cPTargetTurnAngle < 0.0f) {
            cPTargetTurnAngle += 360.0f;
        }
   // }
}

void Player::CpInterruptTurn() {
    cPCurrentTurnMode = CP_TURN_NOTURN;
    currentSideForce = 0.0f;
}

/*void Player::Update(irr::f32 frameDeltaTime) {

   irr::core::vector3d<irr::f32> VectorUp(0.0f, 1.0f, 0.0f);

   currentLapTimeMultiple100mSec += (frameDeltaTime / 0.1);

   //we have currently some forces applied (appliedForceForwards and appliedForceSideways)
   //forces divided by mass is acceleration
   accelForward = (appliedForceForwards / shipMass);
   accelSideways = (appliedForceSideways / shipMass);

   velForward += accelForward;
   velSideways += accelSideways;

   irr::core::vector3d<irr::f32> lastPosition = Position;

   //FrontDir.X = speedForward;
   //FrontDir.Y = speedSideways;
   //speed = speedForward;

   SideDir = FrontDir.crossProduct(VectorUp);

   Position += FrontDir * velForward * MOVEMENT_SPEED * frameDeltaTime;
   Position += SideDir * velSideways * MOVEMENT_SPEED * frameDeltaTime;

   //FrontDir = Position - lastPosition;
   //FrontDir.normalize();

   //SideDir = FrontDir.crossProduct(VectorUp);

   //we need to control the height of the player according to the Terrain below
   irr::core::vector3d<irr::f32> SearchPosition;

   irr::f32 next_height;

   if (DEBUG_PLAYER_HEIGHT_CONTROL == true)
        debug_player_heightcalc_lines.clear();

   MapEntry* FrontTerrainTiles[3];
   MapEntry* BackTerrainTiles[3];
   MapEntry* RightTerrainTiles[3];
   MapEntry* LeftTerrainTiles[3];

   //calculate current cell below player
   int current_cell_calc_x, current_cell_calc_y;

   current_cell_calc_y = (Position.Z / MyTerrain->segmentSize);
   current_cell_calc_x = -(Position.X / MyTerrain->segmentSize);

   //*** search the Terrain cells in front of the player ***

   //search next cell in front of the player
   int next_cell_x, next_cell_y;
   SearchPosition = Position;
   int kmax = 1;

   for (int k = 0; k < kmax; k++) {
       FrontTerrainTiles[k] = NULL;
    //search next cell, but max 10 iterations to not lock up the game if something is wrong
    for (int j = 0; j < 10; j++) {
        next_cell_y = (SearchPosition.Z / MyTerrain->segmentSize);
        next_cell_x = -(SearchPosition.X / MyTerrain->segmentSize);

      if ((next_cell_x != current_cell_calc_x) or (next_cell_y != current_cell_calc_y)) {
          //we found the next cell
          next_height = MyTerrain->GetMapEntry(next_cell_x, next_cell_y)->m_Height;
          FrontTerrainTiles[k] = MyTerrain->GetMapEntry(next_cell_x, next_cell_y);
          j = 10;
      }

      SearchPosition += FrontDir;
   }

      //have not found the next cell?
     if ((next_cell_x == current_cell_calc_x) and (next_cell_y == current_cell_calc_y)) {
           //have not found another cell, just take the current cell height, thats the best we have
         next_height = MyTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y)->m_Height;
    }
  }

   //*** search the Terrain cells back of the player ***

   //search cells back of the player
   SearchPosition = Position;

   for (int k = 0; k < kmax; k++) {
       BackTerrainTiles[k] = NULL;
    //search cells behind the player, but max 10 iterations to not lock up the game if something is wrong
    for (int j = 0; j < 10; j++) {
        next_cell_y = (SearchPosition.Z / MyTerrain->segmentSize);
        next_cell_x = -(SearchPosition.X / MyTerrain->segmentSize);

      if ((next_cell_x != current_cell_calc_x) or (next_cell_y != current_cell_calc_y)) {
          //we found the next cell
          next_height = MyTerrain->GetMapEntry(next_cell_x, next_cell_y)->m_Height;
          BackTerrainTiles[k] = MyTerrain->GetMapEntry(next_cell_x, next_cell_y);
          j = 10;
      }

      SearchPosition -= FrontDir; //follow path behind the player
   }

      //have not found the next cell?
     if ((next_cell_x == current_cell_calc_x) and (next_cell_y == current_cell_calc_y)) {
           //have not found another cell, just take the current cell height, thats the best we have
         next_height = MyTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y)->m_Height;
    }
  }

   // search the Terrain cells in right of the player

   //search next cell right of the player
    SearchPosition = Position;


   for (int k = 0; k < kmax; k++) {
       RightTerrainTiles[k] = NULL;
    //search next cell, but max 10 iterations to not lock up the game if something is wrong
    for (int j = 0; j < 10; j++) {
        next_cell_y = (SearchPosition.Z / MyTerrain->segmentSize);
        next_cell_x = -(SearchPosition.X / MyTerrain->segmentSize);

      if ((next_cell_x != current_cell_calc_x) or (next_cell_y != current_cell_calc_y)) {
          //we found the next cell
          next_height = MyTerrain->GetMapEntry(next_cell_x, next_cell_y)->m_Height;
          RightTerrainTiles[k] = MyTerrain->GetMapEntry(next_cell_x, next_cell_y);
          j = 10;
      }

      SearchPosition += SideDir;
   }

      //have not found the next cell?
     if ((next_cell_x == current_cell_calc_x) and (next_cell_y == current_cell_calc_y)) {
           //have not found another cell, just take the current cell height, thats the best we have
         next_height = MyTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y)->m_Height;
    }
  }

   // search the Terrain cells in left of the player

   //search next cell right of the player
    SearchPosition = Position;

   for (int k = 0; k < kmax; k++) {
       LeftTerrainTiles[k] = NULL;
    //search next cell, but max 10 iterations to not lock up the game if something is wrong
    for (int j = 0; j < 10; j++) {
        next_cell_y = (SearchPosition.Z / MyTerrain->segmentSize);
        next_cell_x = -(SearchPosition.X / MyTerrain->segmentSize);

      if ((next_cell_x != current_cell_calc_x) or (next_cell_y != current_cell_calc_y)) {
          //we found the next cell
          next_height = MyTerrain->GetMapEntry(next_cell_x, next_cell_y)->m_Height;
          LeftTerrainTiles[k] = MyTerrain->GetMapEntry(next_cell_x, next_cell_y);
          j = 10;
      }

      SearchPosition -= SideDir;
   }

      //have not found the next cell?
     if ((next_cell_x == current_cell_calc_x) and (next_cell_y == current_cell_calc_y)) {
           //have not found another cell, just take the current cell height, thats the best we have
         next_height = MyTerrain->GetMapEntry(current_cell_calc_x, current_cell_calc_y)->m_Height;
    }
  }

   irr::u32 lastIdxLeft = 0;
   irr::u32 lastIdxRight = 0;
   irr::u32 lastIdxFront = 0;
   irr::u32 lastIdxBack = 0;

   for (irr::u32 tidx = 0; tidx < kmax; tidx++) {
       if (LeftTerrainTiles[tidx] != NULL) {
           lastIdxLeft = tidx;
       } else break;
    }

   for (irr::u32 tidx = 0; tidx < kmax; tidx++) {
       if (RightTerrainTiles[tidx] != NULL) {
           lastIdxRight = tidx;
       } else break;
    }

   for (irr::u32 tidx = 0; tidx < kmax; tidx++) {
       if (FrontTerrainTiles[tidx] != NULL) {
           lastIdxFront = tidx;
       } else break;
    }

   for (irr::u32 tidx = 0; tidx < kmax; tidx++) {
       if (BackTerrainTiles[tidx] != NULL) {
           lastIdxBack = tidx;
       } else break;
    }

   //irr::f32 SideSlope = (LeftTerrainTiles[lastIdxLeft]->m_Height - RightTerrainTiles[lastIdxRight]->m_Height) / (1.0f * (lastIdxLeft + lastIdxRight + 2));
   //irr::f32 DirSlope = (FrontTerrainTiles[lastIdxFront]->m_Height - BackTerrainTiles[lastIdxBack]->m_Height) / (1.0f * (lastIdxFront + lastIdxBack + 2));
   //calculate final vectors for current ship orientation

   irr::core::vector3d<irr::f32> debugpos;
   irr::core::vector3d<irr::f32> debugpos2;
   debugpos.Z =  FrontTerrainTiles[lastIdxFront]->get_Z()*MyTerrain->segmentSize;
   debugpos.X =  -FrontTerrainTiles[lastIdxFront]->get_X()*MyTerrain->segmentSize;
   debugpos.Y =  FrontTerrainTiles[lastIdxFront]->m_Height;

   debugpos2.Z =  BackTerrainTiles[lastIdxBack]->get_Z()*MyTerrain->segmentSize;
   debugpos2.X =  -BackTerrainTiles[lastIdxBack]->get_X()*MyTerrain->segmentSize;
   debugpos2.Y =  BackTerrainTiles[lastIdxBack]->m_Height;

   irr::core::vector3d<irr::f32> vec1 = debugpos - debugpos2;
   vec1.normalize();

    LineStruct* NewDebugLine = new LineStruct;

   NewDebugLine->A = debugpos;
   NewDebugLine->B = debugpos2;

   debug_player_heightcalc_lines.push_back(NewDebugLine);


   debugpos.Z =  LeftTerrainTiles[lastIdxLeft]->get_Z()*MyTerrain->segmentSize;
   debugpos.X =  -LeftTerrainTiles[lastIdxLeft]->get_X()*MyTerrain->segmentSize;
   debugpos.Y =  LeftTerrainTiles[lastIdxLeft]->m_Height;


   debugpos2.Z =  RightTerrainTiles[lastIdxRight]->get_Z()*MyTerrain->segmentSize;
   debugpos2.X =  -RightTerrainTiles[lastIdxRight]->get_X()*MyTerrain->segmentSize;
   debugpos2.Y =  RightTerrainTiles[lastIdxRight]->m_Height;

     irr::core::vector3d<irr::f32> vec2 = debugpos - debugpos2;
     vec2.normalize();

   NewDebugLine = new LineStruct;
   NewDebugLine->A = debugpos;
   NewDebugLine->B = debugpos2;

   debug_player_heightcalc_lines.push_back(NewDebugLine);


   irr::core::vector3d<irr::f32> shipZVec = vec2.crossProduct(vec1);
   shipZVec.normalize();

   NewDebugLine = new LineStruct;
   NewDebugLine->A = Position;
   NewDebugLine->B = Position + shipZVec;

   debug_player_heightcalc_lines.push_back(NewDebugLine);

   //FrontDir = -shipZVec.crossProduct(SideDir);


   irr::f32 maxh;
   maxh = -1000.0f;

   //irr::core::vector3d<irr::f32> debugpos;
   int maxk = 0;

   for (int k = 0; k < kmax; k++) {
       if (FrontTerrainTiles[k] != NULL) {

       /* if (DEBUG_PLAYER_HEIGHT_CONTROL == true) {
           debugpos.Z =  FrontTerrainTiles[k]->get_Z()*MyTerrain->segmentSize;
           debugpos.X =  -FrontTerrainTiles[k]->get_X()*MyTerrain->segmentSize;
           debugpos.Y =  FrontTerrainTiles[k]->m_Height;

           LineStruct* NewDebugLine = new LineStruct;
           NewDebugLine->A = Position;
           NewDebugLine->B = debugpos;

           debug_player_heightcalc_lines.push_back(NewDebugLine);
        }*//*

      if (FrontTerrainTiles[k]->m_Height > maxh)
           maxh = FrontTerrainTiles[k]->m_Height;
           maxk = k;
       }
   }

   for (int k = 0; k < kmax; k++) {
       if (BackTerrainTiles[k] != NULL) {
             /*  if (DEBUG_PLAYER_HEIGHT_CONTROL == true) {
                debugpos.Z =  BackTerrainTiles[k]->get_Z()*MyTerrain->segmentSize;
                debugpos.X =  -BackTerrainTiles[k]->get_X()*MyTerrain->segmentSize;
                debugpos.Y =  BackTerrainTiles[k]->m_Height;

                LineStruct* NewDebugLine = new LineStruct;
                NewDebugLine->A = Position;
                NewDebugLine->B = debugpos;

                debug_player_heightcalc_lines.push_back(NewDebugLine);
           }*/
/*
      if (BackTerrainTiles[k]->m_Height > maxh)
           maxh = BackTerrainTiles[k]->m_Height;
           maxk = k;
       }
   }

   for (int k = 0; k < kmax; k++) {
       if (RightTerrainTiles[k] != NULL) {
              /* if (DEBUG_PLAYER_HEIGHT_CONTROL == true) {
                debugpos.Z =  RightTerrainTiles[k]->get_Z()*MyTerrain->segmentSize;
                debugpos.X =  -RightTerrainTiles[k]->get_X()*MyTerrain->segmentSize;
                debugpos.Y =  RightTerrainTiles[k]->m_Height;

                LineStruct* NewDebugLine = new LineStruct;
                NewDebugLine->A = Position;
                NewDebugLine->B = debugpos;

                debug_player_heightcalc_lines.push_back(NewDebugLine);
           }*/
/*
      if (RightTerrainTiles[k]->m_Height > maxh)
           maxh = RightTerrainTiles[k]->m_Height;
           maxk = k;
       }
   }

   for (int k = 0; k < kmax; k++) {
       if (LeftTerrainTiles[k] != NULL) {
          /*     if (DEBUG_PLAYER_HEIGHT_CONTROL == true) {
                debugpos.Z =  LeftTerrainTiles[k]->get_Z()*MyTerrain->segmentSize;
                debugpos.X =  -LeftTerrainTiles[k]->get_X()*MyTerrain->segmentSize;
                debugpos.Y =  LeftTerrainTiles[k]->m_Height;

                LineStruct* NewDebugLine = new LineStruct;
                NewDebugLine->A = Position;
                NewDebugLine->B = debugpos;

                debug_player_heightcalc_lines.push_back(NewDebugLine);
           }*/
/*
      if (LeftTerrainTiles[k]->m_Height > maxh)
           maxh = LeftTerrainTiles[k]->m_Height;
           maxk = k;
       }
   }

  //debugpos.Z =  FrontTerrainTiles[maxk]->get_X()*MyTerrain->segmentSize;
  //debugpos.X =  -FrontTerrainTiles[maxk]->get_Z()*MyTerrain->segmentSize;
  //debugpos.Y =  FrontTerrainTiles[maxk]->m_Height;

  //how steep is the area in front of the craft?
  //take the maximum observed height for calculation
  //irr::core::vector3d<irr::f32> stepness;
  //stepness = -(debugpos-Position).normalize();

  //first_player_update is only true after creating player once, to make sure the player is immediately at the correct height after start of game
  //afterwards height changes are slowed down to prevent step response of player height when cell is changed
  if (first_player_Update == false) {

    //define new target height
    TargetHeight = maxh + HOVER_HEIGHT;

    irr::f32 absdiff;

    absdiff = abs(TargetHeight-current_Height);

    if ((TargetHeight > current_Height) && (absdiff > 1e-4))
        current_Height += (TargetHeight - current_Height) * SPEED_CLIMB * frameDeltaTime;

    if ((TargetHeight < current_Height) && (absdiff > 1e-4))
        current_Height += (TargetHeight - current_Height)* SPEED_FALL * frameDeltaTime;

    //Position.Y = current_Height;

  } else {
        first_player_Update = false;

        //right after start update player position immediately without any delay
        TargetHeight = maxh + HOVER_HEIGHT;
        current_Height = TargetHeight;

       // Position.Y = TargetHeight;
   }

  //we can later experiment with this line to let craft model have an angle when going hills up and down
  //to make this work also other calculations above need to be enabled!
  //FrontDir.Y = stepness.Y;

  if (DEF_INSPECT_LEVEL == true) {
    //Player_node->setPosition(Position);
    //Player_node->setRotation(FrontDir.getHorizontalAngle()+ irr::core::vector3df(0.0f, 180.0f, 0.0f) );

    //for the "neigung" of ship model in driving direction
   /* working code irr::core::vector3d othervector(0.0f, 0.0f, 1.0f);
    testAngle = acos((shipZVec.dotProduct(othervector)) / (shipZVec.getLength() * othervector.getLength()));*/
/*
    irr::core::vector3d othervector(0.0f, 0.0f, 1.0f);
    testAngle = acos((shipZVec.dotProduct(othervector)) / (shipZVec.getLength() * othervector.getLength()));

   //for the "neigung" of ship model in sideways direction
    irr::core::vector3d othervector2(-1.0f, 0.0f, 0.0f);
    irr::f32 testAngle2 = acos((shipZVec.dotProduct(othervector2)) / (shipZVec.getLength() * othervector2.getLength()));

    //for the "drehung" of ship model when player steers left or right
     irr::core::vector3d othervector3(1.0f, 0.0f, 0.0f);
     irr::f32 testAngle3 = acos((FrontDir.dotProduct(othervector3)) / (FrontDir.getLength() * othervector3.getLength()));

     //for the "drehung" of ship model when player steers left or right
      irr::core::vector3d othervector4(0.0f, 0.0f, 1.0f);
      irr::f32 testAngle4 = acos((FrontDir.dotProduct(othervector4)) / (FrontDir.getLength() * othervector4.getLength()));

    //rotate player model in the "correct" way according the terrain
   // Player_node->setRotation(irr::core::vector3df((testAngle / PI) * 180.0f - 90.0f, 0.0f, (testAngle2 / PI) * 180.0f - 90.0f));
    //Player_node->setRotation(irr::core::vector3df((testAngle / PI) * 180.0f - 90.0f, (testAngle3 / PI) * 180.0f, (testAngle2 / PI) * 180.0f - 90.0f));

       core::quaternion test;
       test.fromAngleAxis(testAngle - (PI/2), core::vector3df(1,0,0));
       test.normalize();

       core::quaternion test2;
       test2.fromAngleAxis(testAngle2 - (PI/2), core::vector3df(0,0,1));
       test2.normalize();

//       core::quaternion test3;
 //      test3.fromAngleAxis(testAngle3, core::vector3df(1,0,0));
  //     test3.normalize();

//       core::quaternion test5;
 //      test5.fromAngleAxis(testAngle4, core::vector3df(0,0,1));
  //     test5.normalize();

       core::quaternion test4 = test * test2;

       core::vector3df rot;
       test4.toEuler(rot);
     //  Player_node->setRotation(rot * core::RADTODEG);

  } else {
      irr::core::vector3d camUpVec = -shipZVec.normalize();
      Player_camera->setPosition(Position);
      Player_camera->setTarget(Position+FrontDir);
      Player_camera->setUpVector(camUpVec);
  }
}*/

//is called when the player collected a collectable item of the
//level
void Player::CollectedCollectable(Collectable* whichCollectable) {
    //play sound
    if (this->mHumanPlayer) {
        this->mRace->mSoundEngine->PlaySound(SRES_GAME_PICKUP);
    }

    EntityItem entity = *whichCollectable->mEntityItem;

  //depending on the type of entity/collectable alter player stats
    EntityItem::EntityType type = whichCollectable->mEntityItem->get_GameType();

    //Alex TODO important: figure out what the items do exactly with the player stats and how
    //much the effect is, right now this stuff does not make much sense to me
    switch (type) {
        case entity.EntityType::ExtraFuel:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"EXTRA FUEL", 4.0f);
            }

            this->mPlayerStats->gasolineVal += 10.0f;
            if (this->mPlayerStats->gasolineVal > this->mPlayerStats->gasolineMax)
                this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

            break;

        case entity.EntityType::FuelFull:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"FUEL FULL", 4.0f);
            }

            this->mPlayerStats->gasolineVal += 100.0f;
            if (this->mPlayerStats->gasolineVal > this->mPlayerStats->gasolineMax)
                this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

            break;

        case entity.EntityType::DoubleFuel:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"DOUBLE FULL", 4.0f);
            }

            this->mPlayerStats->gasolineVal += 200.0f;
            if (this->mPlayerStats->gasolineVal > this->mPlayerStats->gasolineMax)
                this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

            break;

        case entity.EntityType::ExtraAmmo:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"EXTRA AMMO", 4.0f);
            }
            this->mPlayerStats->ammoVal += 10.0f;
            if (this->mPlayerStats->ammoVal > this->mPlayerStats->ammoMax)
                this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;

            break;

        case entity.EntityType::AmmoFull:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"AMMO FULL", 4.0f);
            }
            this->mPlayerStats->ammoVal += 100.0f;
            if (this->mPlayerStats->ammoVal > this->mPlayerStats->ammoMax)
                this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;

            break;

        case entity.EntityType::DoubleAmmo:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"DOUBLE AMMO", 4.0f);
            }

            this->mPlayerStats->ammoVal += 200.0f;
            if (this->mPlayerStats->ammoVal > this->mPlayerStats->ammoMax)
                this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;

            break;

        case entity.EntityType::ExtraShield:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"EXTRA SHIELD", 4.0f);
            }

            this->mPlayerStats->shieldVal += 10.0f;
            if (this->mPlayerStats->shieldVal > this->mPlayerStats->shieldMax)
                this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;

            break;

        case entity.EntityType::ShieldFull:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"SHIELD FULL", 4.0f);
            }

            this->mPlayerStats->shieldVal += 100.0f;
            if (this->mPlayerStats->shieldVal > this->mPlayerStats->shieldMax)
                this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;

            break;

        case entity.EntityType::DoubleShield:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"DOUBLE SHIELD", 4.0f);
            }

            this->mPlayerStats->shieldVal += 200.0f;
            if (this->mPlayerStats->shieldVal > this->mPlayerStats->shieldMax)
                this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;

            break;

        case entity.EntityType::BoosterUpgrade:
            //upgrade players booster
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"BOOSTER UPGRADED", 4.0f);
            }

            if (this->mPlayerStats->currBoosterUpgradeLevel < 3)
                this->mPlayerStats->currBoosterUpgradeLevel++;

            break;

        case entity.EntityType::MissileUpgrade:
            //upgrade players rocket
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"MISSILE UPGRADED", 4.0f);
            }

            if (this->mPlayerStats->currRocketUpgradeLevel < 3)
                this->mPlayerStats->currRocketUpgradeLevel++;

            break;

        case entity.EntityType::MinigunUpgrade:
            //upgrade players mini-gun
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"MINIGUN UPGRADED", 4.0f);
            }

            if (this->mPlayerStats->currMinigunUpgradeLevel < 3)
                this->mPlayerStats->currMinigunUpgradeLevel++;

            break;

        case entity.EntityType::UnknownShieldItem:
            //uncomment the next 2 lines to show this items also to the player
            // collectable = new Collectable(41, entity.get_Center(), color, driver);
            // ENTCollectables_List.push_back(collectable);
            break;

        case entity.EntityType::UnknownItem:
        case entity.EntityType::Unknown:
            //uncomment the next 2 lines to show this items also to the player
            // collectable = new Collectable(50, entity.get_Center(), color, driver);
            // ENTCollectables_List.push_back(collectable);
            break;

         //all the other entities we do not care here
        default:
            break;
    }
}

irr::f32 Player::GetHoverHeight() {
    irr::f32 height = HOVER_HEIGHT;

    return (height);
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
        mPlayerStats->LapBeforeLastLap.lapTimeMultiple100mSec = mPlayerStats->lastLap.lapTimeMultiple100mSec;
    }

    if (mPlayerStats->currLapNumber > 0) {
        mPlayerStats->lastLap.lapNr = mPlayerStats->currLapNumber;
        mPlayerStats->lastLap.lapTimeMultiple100mSec = mPlayerStats->currLapTimeMultiple100mSec;
    }

    //make sure we have at least one laptime entry
    if (mPlayerStats->lapTimeList.size() > 0) {
        for(idx = mPlayerStats->lapTimeList.begin(); idx < mPlayerStats->lapTimeList.end(); idx++)
            {
                if (mPlayerStats->currLapTimeMultiple100mSec <  (*idx).lapTimeMultiple100mSec)
                    break;
            }
    } else idx = mPlayerStats->lapTimeList.end();

    LAPTIMEENTRY newEntry;
    newEntry.lapNr = mPlayerStats->currLapNumber;
    newEntry.lapTimeMultiple100mSec = mPlayerStats->currLapTimeMultiple100mSec;

    mPlayerStats->lapTimeList.insert(idx, newEntry);

    mPlayerStats->currLapNumber++;

    //has this player finished the last lap of this race?
    if (mPlayerStats->currLapNumber > mPlayerStats->raceNumberLaps) {
        SetNewState(STATE_PLAYER_FINISHED);

        //also tell the race that I am finished
        mRace->PlayerHasFinishedLastLapOfRace(this);
    }

    //reset current lap time
    mPlayerStats->currLapTimeExact = 0.0;
    mPlayerStats->currLapTimeMultiple100mSec = 0;
}
