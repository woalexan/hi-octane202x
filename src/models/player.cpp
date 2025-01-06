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

int Player::randRangeInt(int min, int max) {
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void Player::DamageGlas() {
    //with a certain probability damage glas if another player
    //shoots with the machine gun at me
    irr::s32 rNum = rand();
    irr::f32 rFloat = (float(rNum) / float (RAND_MAX));

    //with 10% probability damage glas of player HUD
    if (rFloat < 0.1f) {
       if (mHUD != NULL) {
        AddGlasBreak();
       }
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

        //if this is a computer player setup the path to the next
        //correct checkpoint
        /*if (!this->mHumanPlayer) {
            mRace->testPathResult = mRace->mPath->FindPathToNextCheckPoint(this);

            this->CpPlayerFollowPath(mRace->testPathResult);
        }*/
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

    CleanUpBrokenGlas();
    delete this->brokenGlasVec;
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

//after a computer player is freed from the recovery vehicle, our under
//some error cases where we could lose our current path progress we need
//to try to restart with a new clean path/path status, this is done by this routine
void Player::WorkaroundResetCurrentPath() {
    //which waypoint link is the closest one to us right now?
   std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> closeWaypointLinks =
           this->mRace->mPath->PlayerFindCloseWaypointLinks(this);

   std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> closestLink =
           this->mRace->mPath->PlayerDeriveClosestWaypointLink(closeWaypointLinks);

   if (closestLink.first == NULL) {
       //workaround, just do nothing! TODO: Maybe improve later
   } else {
       //setup new path for the race
       AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, closestLink.first);
   }

  computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
}

void Player::FreedFromRecoveryVehicleAgain() {
   if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_GRABEDBYRECOVERYVEHICLE) {
       mGrabedByThisRecoveryVehicle = NULL;

       SetNewState(STATE_PLAYER_RACING);

       mRecoveryVehicleCalled = false;
       mCpPlayerCurrentlyStuck = false;

       //additional logic for computer player
       if (!mHumanPlayer) {
         //renew the current path for computer
         //player
         WorkaroundResetCurrentPath();
       }
   }
}

irr::u32 Player::GetCurrentState() {
    return this->mPlayerStats->mPlayerCurrentState;
}

void Player::AdvanceDbgColor() {
    if (currDbgColor == this->mRace->mDrawDebug->red)  {
        currDbgColor = this->mRace->mDrawDebug->cyan;
    } else if (currDbgColor == this->mRace->mDrawDebug->cyan)  {
        currDbgColor = this->mRace->mDrawDebug->pink;
    } else if (currDbgColor == this->mRace->mDrawDebug->pink)  {
        currDbgColor = this->mRace->mDrawDebug->green;
    } else if (currDbgColor == this->mRace->mDrawDebug->green)  {
        currDbgColor = this->mRace->mDrawDebug->red;
    }
}

Player::Player(Race* race, std::string model, irr::core::vector3d<irr::f32> NewPosition,
               irr::core::vector3d<irr::f32> NewFrontAt, irr::scene::ISceneManager* smgr,
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

    mPathHistoryVec.clear();

    currDbgColor = this->mRace->mDrawDebug->red;

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
        Player_node->setMaterialFlag(irr::video::EMF_LIGHTING, this->mRace->mGame->enableLightning);

        if (this->mRace->mGame->enableShadows) {
            // add shadow
            PlayerNodeShadow = Player_node->addShadowVolumeSceneNode();
        }
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

    //create vector to store all the current broken Hud glas locations
    brokenGlasVec = new std::vector<HudDisplayPart*>();
    brokenGlasVec->clear();
}

void Player::AddCommand(uint8_t cmdType, EntityItem* targetEntity) {
    if (cmdType != CMD_FLYTO_TARGETENTITY || targetEntity == NULL)
        return;

    CheckAndRemoveNoCommand();

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = CMD_FLYTO_TARGETENTITY;
    newCmd->targetEntity = targetEntity;

    //create a new temporary waypoint link from computer player crafts current
    //position towards the entity we want to fly to
    WayPointLinkInfoStruct* newStruct = new WayPointLinkInfoStruct();

    LineStruct* newLineStr = new LineStruct();
    newLineStr->A = this->phobj->physicState.position;

    irr::core::vector3df posEntity = targetEntity->getCenter();
    //our universe is flipped for X axis
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

    //precalculate a direction vector which stands at a 90 degree
    //angle at the original waypoint direction vector, and always points
    //to the right direction when looking into race direction
    //this direction vector is later used during the game to offset the player
    //path sideways
    newStruct->offsetDirVec = newStruct->LinkDirectionVec.crossProduct(-*this->mRace->yAxisDirVector).normalize();

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

    CheckAndRemoveNoCommand();

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = CMD_FOLLOW_TARGETWAYPOINTLINK;
    newCmd->targetWaypointLink = targetWayPointLink;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void Player::AddCommand(uint8_t cmdType) {
    CheckAndRemoveNoCommand();

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = cmdType;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void Player::AddCommand(uint8_t cmdType, Collectable* whichCollectable) {
    CheckAndRemoveNoCommand();

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = cmdType;
    newCmd->targetCollectible = whichCollectable;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void Player::CheckAndRemoveNoCommand() {
    if (currCommand == NULL)
        return;

    if (currCommand->cmdType == CMD_NOCMD) {
        CPCOMMANDENTRY* oldCmd = currCommand;

        //if we set currCommand to NULL then the program
        //will pull the next available command in
        //RunComputerPlayerLogic
        currCommand = NULL;

        //delete old command struct
        delete oldCmd;
    }
}

void Player::RemoveAllPendingCommands() {
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

    currCommand = NULL;
}

void Player::SetCurrClosestWayPointLink(std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> newClosestWayPointLink) {
    if (newClosestWayPointLink.first != NULL) {
        this->currClosestWayPointLink = newClosestWayPointLink;
        this->projPlayerPositionClosestWayPointLink = newClosestWayPointLink.second;
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
    if (!DEF_PLAYERCANGOBACKWARDS) {
        //we can not go backwards in Hioctane
        //we can only add friction to brake
        this->phobj->AddFriction(1000.0f);

        if (mPlayerStats->throttleVal > 0)
            mPlayerStats->throttleVal--;
    } else {
        //if player can not move right now simply
        //exit
        if (!this->mPlayerStats->mPlayerCanMove)
            return;

            //go solution during debugging, for example testing collisions, it helps to be able to accelerate backwards
            this->phobj->AddLocalCoordForce(irr::core::vector3df(0.0f, 0.0f, 0.0f), irr::core::vector3df(0.0f, 0.0f, 50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

            if (mPlayerStats->throttleVal > 0)
                mPlayerStats->throttleVal--;
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

    this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, irr::core::vector3df(currentSideForce, 0.0f, 0.0f),
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

    this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, irr::core::vector3df(currentSideForce, 0.0f, 0.0f),
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

void Player::CpStuckDetection(irr::f32 deltaTime) {
    //for a human player simply return
    if (mHumanPlayer)
        return;

    //we can not execute stuck detection every frame, as the computer
    //player does not move a meaningful way distance in one frame
    //we want to check for a longer period of time
    mExecuteCpStuckDetectionTimeCounter += deltaTime;

    if (mExecuteCpStuckDetectionTimeCounter > CP_PLAYER_STUCKDETECTION_PERIOD_SEC) {
           irr::f32 addTime = mExecuteCpStuckDetectionTimeCounter;
           mExecuteCpStuckDetectionTimeCounter = 0.0f;

            //if computer player is charging currently at a station
            //somewhere its position is also not changing, we do not want to
            //detect this as stuck
            if ((this->mCurrChargingFuel) || (this->mCurrChargingAmmo) || (this->mCurrChargingShield)) {
                mCpPlayerTimeNotMovedSeconds = 0.0f;
                mCpPlayerLastPosition = this->phobj->physicState.position;
                return;
            }

            //if computer player has finished race already also
            //it is not stuck, or if its fuel is empty
            //to refuel fuel there is another logic in place, we do not
            //want to interrupt here
            //in both cases the player is not allowed to move
            if (!this->mPlayerStats->mPlayerCanMove){
               mCpPlayerTimeNotMovedSeconds = 0.0f;
               mCpPlayerLastPosition = this->phobj->physicState.position;
               return;
           }

            //if we have already called a recovery vehicle also leave
            if (mRecoveryVehicleCalled || mCpPlayerCurrentlyStuck) {
                mCpPlayerTimeNotMovedSeconds = 0.0f;
                mCpPlayerLastPosition = this->phobj->physicState.position;
                return;
           }

            //next line only for debugging
            //dbgStuckDet = (this->phobj->physicState.position - mCpPlayerLastPosition).getLengthSQ();

            //check if we have not moved since last frame
            if ((this->phobj->physicState.position - mCpPlayerLastPosition).getLengthSQ() < CP_PLAYER_STUCKDETECTION_MINDISTANCE_LIMIT) {
                this->mCpPlayerTimeNotMovedSeconds += addTime;

                //stuck for a long enough time?
                if (this->mCpPlayerTimeNotMovedSeconds > CP_PLAYER_STUCKDETECTION_THRESHOLD_SEC) {
                    //we seem to be stuck
                    //for a workaround call the Recovery vehicle to
                    //put us back properly at the next waypoint link at the track
                    this->mRace->CallRecoveryVehicleForHelp(this);
                    mRecoveryVehicleCalled = true;
                    mCpPlayerCurrentlyStuck = true;

                    mCpPlayerTimeNotMovedSeconds = 0.0f;
                }
            } else {
                //we have move enough, reset time counter again
                this->mCpPlayerTimeNotMovedSeconds = 0.0f;
            }

               dbgStuckDet = this->mCpPlayerTimeNotMovedSeconds;
             mCpPlayerLastPosition = this->phobj->physicState.position;
    }
}

void Player::TestCpForceControlLogicWithHumanPlayer() {
    if (!mHumanPlayer)
        return;

    if (this->currClosestWayPointLink.first != NULL) {

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

         irr::f32 angleDotProduct = this->currClosestWayPointLink.first->LinkDirectionVec.dotProduct(craftForwardDirVec);

        irr::f32 angleRad = acosf(angleDotProduct);
        mAngleError = (angleRad / irr::core::PI) * 180.0f;

        if (craftSidewaysToRightVec.dotProduct(this->currClosestWayPointLink.first->LinkDirectionVec) > 0.0f) {
            mAngleError = -mAngleError;
        }

        //irr::f32 currAngleVelocityCraft =  (mCurrentCraftOrientationAngle - mLastCurrentCraftOrientationAngle);

        irr::f32 currDistanceChangeRate = mCurrentCraftDistToWaypointLink - mLastCraftDistToWaypointLink;

        irr::f32 distError = (mCurrentCraftDistToWaypointLink - mCurrentCraftDistWaypointLinkTarget);

         dbgDistError = distError;
    }
}

void Player::CPForceController() {
    mLastCurrentCraftOrientationAngle = mCurrentCraftOrientationAngle;
    mLastAngleError = mAngleError;

    mLastCraftDistToWaypointLink = mCurrentCraftDistToWaypointLink;

    //fuel empty?
    if (this->mPlayerStats->gasolineVal <= 0.0f) {
        this->mPlayerStats->gasolineVal = 0.0f;
        computerPlayerTargetSpeed = 0.0f;
        SetNewState(STATE_PLAYER_EMPTYFUEL);
    }

    if ((computerPlayerTargetSpeed + mTargetSpeedAdjust) > computerPlayerCurrentSpeed) {
        computerPlayerCurrentSpeed += mCpCurrentAccelRate;
    } else if (computerPlayerCurrentSpeed > (computerPlayerTargetSpeed - mTargetSpeedAdjust)) {
        computerPlayerCurrentSpeed -= mCpCurrentDeaccelRate;
    }

    //is there a craft very close in front of us, then go slower
    if (this->mCraftDistanceAvailFront < 5.0f) {
        mTargetSpeedAdjust -= 0.2f;

        if (mTargetSpeedAdjust < 0.0f) {
            mTargetSpeedAdjust = 0.0f;
        }
    } else {
        if (mTargetSpeedAdjust < 0.0f) {
            mTargetSpeedAdjust += 0.2f;

            if (mTargetSpeedAdjust > 0.0f)
            {
                mTargetSpeedAdjust = 0.0f;
            }
        }
    }

    //control computer player speed
    if (this->phobj->physicState.speed < (computerPlayerCurrentSpeed * 0.9f))
    {
        //go faster
        this->CPForward();
    } else if (this->phobj->physicState.speed > (computerPlayerCurrentSpeed * 1.1f)) {
        //go slower
       this->CPBackward();
    }

    if (!this->mPlayerStats->mPlayerCanMove)
        return;

    if (this->cPCurrentFollowSeg != NULL) {
        //we need to project current computer player craft
        //position onto the current segment we follow
        //this recalculates the current projPlayerPositionClosestWayPointLink member
        //variable for the further calculation steps below
        ProjectPlayerAtCurrentSegments();

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

        mCurrentCraftOrientationAngle =
                this->mRace->GetAbsOrientationAngleFromDirectionVec(craftForwardDirVec);

         irr::f32 angleDotProduct = this->cPCurrentFollowSeg->LinkDirectionVec.dotProduct(craftForwardDirVec);

        irr::f32 angleRad = acosf(angleDotProduct);
        mAngleError = (angleRad / irr::core::PI) * 180.0f;

        if (craftSidewaysToRightVec.dotProduct(this->cPCurrentFollowSeg->LinkDirectionVec) > 0.0f) {
            mAngleError = -mAngleError;
        }

        //irr::f32 currAngleVelocityCraft =  (mCurrentCraftOrientationAngle - mLastCurrentCraftOrientationAngle);

        irr::f32 currDistanceChangeRate = mCurrentCraftDistToWaypointLink - mLastCraftDistToWaypointLink;

        /***************************************/
        /*  Control Craft absolute angle start */
        /***************************************/

        //best values before 30.12.2024
        /*irr::f32 corrForceOrientationAngle = 0.5f;
        irr::f32 corrDampingOrientationAngle = 2000.0f;

        irr::f32 corrForceAngle = mAngleError * corrForceOrientationAngle - currAngleVelocityCraft * corrDampingOrientationAngle;*/

        irr::f32 corrForceOrientationAngle = 500.0f;
        irr::f32 corrDampingOrientationAngle = 50.0f;

        irr::f32 angleVelocityCraftX = this->phobj->GetVelocityLocalCoordPoint(LocalCraftFrontPnt).X * corrDampingOrientationAngle;

        irr::f32 corrForceAngle = mAngleError * corrForceOrientationAngle - angleVelocityCraftX;

        //we need to limit max force, if force is too high just
        //set it zero, so that not bad physical things will happen!
        if (fabs(corrForceAngle) > 500.0f) {
            corrForceAngle = sgn(corrForceAngle) * 500.0f;
        }

        //depending on the angle error also dynamically set physics model angular damping
        //I saw no other solution, because if I statically set the angular damping in the physics
        //model so low, that I can also get through tight turns then the models start to "oscillate"
        //And if I increase the angular damping to a high static value the player models are more
        //stable, but I can not make a tight turn. By adjusting the angular damping of the computer
        //players physics model depending on the actual angle error I can achieve both goals at
        //the same time
        irr::f32 absAngleError = fabs(mAngleError);

        //below a preset minimum absolute angle error I apply constant max angular damping factor
        //between a min and max angle error I interpolate the damping between max and min value (linear interpolation)
        //above a preset maximum absolute angle error I keep applying the defined minimum anglular damping in the physics model
        if (absAngleError < CP_PLAYER_ANGULAR_DAMPING_ANGLEMIN) {
            this->phobj->mRotationalFrictionVal = CP_PLAYER_ANGULAR_DAMPINGMAX;
        } else if (absAngleError > CP_PLAYER_ANGULAR_DAMPING_ANGLEMAX) {
            this->phobj->mRotationalFrictionVal = CP_PLAYER_ANGULAR_DAMPINGMIN;
        } else {
            //interpolate between max and min value depending on the angle error
            mDbgRotationalFrictionVal = CP_PLAYER_ANGULAR_DAMPINGMAX -
                    ((CP_PLAYER_ANGULAR_DAMPINGMAX - CP_PLAYER_ANGULAR_DAMPINGMIN) / (CP_PLAYER_ANGULAR_DAMPING_ANGLEMAX - CP_PLAYER_ANGULAR_DAMPING_ANGLEMIN)) * absAngleError;

             this->phobj->mRotationalFrictionVal = mDbgRotationalFrictionVal;
        }

         mDbgForceAngle = corrForceAngle;
         mDbgAngleVelocityCraftZ = this->phobj->GetVelocityLocalCoordPoint(LocalCraftFrontPnt).Z;
         mDbgAngleVelocityCraftX = angleVelocityCraftX;

        this->phobj->AddLocalCoordForce(LocalCraftFrontPnt, irr::core::vector3df(corrForceAngle, 0.0f, 0.0f),
                                             PHYSIC_APPLYFORCE_ONLYROT);

        /****************************************************/
        /*  Control Craft distance to current waypoint link */
        /****************************************************/

        //best values until 30.12.2024
        irr::f32 corrForceDist = 100.0f;
        irr::f32 corrDampingDist = 2000.0f;

        irr::f32 distError = (mCurrentCraftDistToWaypointLink - mCurrentCraftDistWaypointLinkTarget);

         dbgDistError = distError;

         //best line until 30.12.2024
        irr::f32 corrForceDistance = distError * corrForceDist + currDistanceChangeRate * corrDampingDist;

        if (corrForceDistance > 100.0f) {
            corrForceDistance = 100.0f;
        } else if (corrForceDistance < -100.0f) {
            corrForceDistance = -100.0f;
        }

        mDbgFoceDistance = corrForceDistance;

        this->phobj->AddLocalCoordForce(LocalCraftOrigin, irr::core::vector3df(corrForceDistance, 0.0f, 0.0f),
                                            PHYSIC_APPLYFORCE_ONLYTRANS);
    } else {
        //no segment to follow, stop craft
        this->computerPlayerTargetSpeed = 0.0f;
    }
}

void Player::CpCheckCurrentPathForObstacles() {
    bool updatePath = false;

        irr::f32 missingSpace;

        //are we too close to the race track edge / available space runs out?
        if (this->mCraftDistanceAvailLeft < 1.0f) {
            //go more right
            //missingSpace = (1.5f - this->mCraftDistanceAvailLeft);
            missingSpace = 0.5f;

            if (mCpCurrPathOffset < (mCpFollowThisWayPointLink->maxOffsetShift - 0.5f - missingSpace)) {
                if (!(mCpCurrPathOffset + missingSpace > (0.5f))) {
                    mCpCurrPathOffset += missingSpace;
                    updatePath = true;
                }
            }
        }

        if (this->mCraftDistanceAvailRight < 1.0f) {
            //go more left
            //missingSpace = (1.5f - this->mCraftDistanceAvailRight);
             missingSpace = 0.5f;
            if (mCpCurrPathOffset > (mCpFollowThisWayPointLink->minOffsetShift + 0.5f + missingSpace)) {
                 if (!(mCpCurrPathOffset - missingSpace < (-0.5f))) {
                   mCpCurrPathOffset -= missingSpace;
                   updatePath = true;
                 }
            }
        }

        //only check possible collision with players we do actually see in front of us
        //otherwise we would report an possible collision with our path, when the other
        //player comes close to our current path behind us
        //and we do not want to detect this
        std::vector<Player*> playerISee;
        std::vector<RayHitInfoStruct>::iterator it;

        playerISee.clear();
        for (it = this->PlayerSeenList.begin(); it != this->PlayerSeenList.end(); ++it) {
            if ((*it).HitType == RAY_HIT_PLAYER) {
                playerISee.push_back((*it).HitPlayerPntr);
            }
        }
        if (playerISee.size() > 0) {
            if (!this->mRace->mPath->DoesPathComeTooCloseToAnyOtherPlayer(
                        this->mCurrentPathSeg, playerISee, dbgPlayerInMyWay)) {
            } else {
                //we have to react, and find another path
                //without any obstacle
                updatePath = true;
                updatePathCnter++;
            }
        }

    if (updatePath) {
         FollowPathDefineNextSegment(this->mCpFollowThisWayPointLink, mCpCurrPathOffset);
         /*if (mHUD != NULL) {
           this->mHUD->ShowBannerText((char*)"OBSTACLE", 4.0f);
         }*/

         //next line is only for debugging purposes
         //this->mRace->mGame->StopTime();
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

   bool foundCurrentProgress = false;

   for(WayPointLink_iterator = mCurrentPathSeg.begin(); WayPointLink_iterator != mCurrentPathSeg.end(); ++WayPointLink_iterator) {
       if ((*WayPointLink_iterator) == cPCurrentFollowSeg) {
           foundCurrentProgress = true;
           break;
       }

       mCurrentPathSegCurrSegmentNr++;
   }

   //in some cases we could loose our way through the current
   //segment path
   if (!foundCurrentProgress) {
       //to fix this situation, completely renew the current overall path
       WorkaroundResetCurrentPath();
   } else {
           //have we reached the end of the following path we follow?
           //if (mCurrentPathSegCurrSegmentNr >= (mCurrentPathSegNrSegments -1)) {
           if (mCurrentPathSegCurrSegmentNr >= (mCurrentPathSegNrSegments - 2)) {
              ReachedEndCurrentFollowingSegments();
           }
   }
}

WayPointLinkInfoStruct* Player::CpPlayerWayPointLinkSelectionLogic(std::vector<WayPointLinkInfoStruct*> availLinks) {
    std::vector<WayPointLinkInfoStruct*>::iterator it;
    WayPointLinkInfoStruct* linkForFuel = NULL;
    WayPointLinkInfoStruct* linkForShield = NULL;
    WayPointLinkInfoStruct* linkForAmmo = NULL;
    std::vector<WayPointLinkInfoStruct*> linksNothingSpecial;

    linksNothingSpecial.clear();

    EntityItem* ent;

    for (it = availLinks.begin(); it != availLinks.end(); ++it) {
        //what kind of waypoint is this?
        ent = (*it)->pStartEntity;

        switch (ent->getEntityType()) {
            case Entity::EntityType::WaypointFuel: {
                linkForFuel = (*it);
                break;
            }

            case Entity::EntityType::WaypointAmmo: {
                linkForAmmo = (*it);
                break;
            }

            case Entity::EntityType::WaypointShield: {
                linkForShield = (*it);
                break;
            }

            //need to go slower
            case Entity::EntityType::WaypointSlow: {
                linksNothingSpecial.push_back(*it);
                break;
            }

            //need to go faster
            case Entity::EntityType::WaypointFast: {
                linksNothingSpecial.push_back(*it);
                break;
            }

            case Entity::EntityType::WaypointSpecial1:
            case Entity::EntityType::WaypointSpecial2:
            case Entity::EntityType::WaypointSpecial3:
            case Entity::EntityType::WaypointShortcut: {
                linksNothingSpecial.push_back(*it);
                break;
            }

        }
    }

    //now handle stuff in terms of priority, most important stuff first

    //do we have link to shield?
    if (linkForShield) {
        //do we need shield? nothing more important!
        if (DoIWantToChargeShield()) {
            //with the current command implementation I have an issue:
            //if for example the computer player craft sees an Collectable it
            //wants to pickup up, (so there is a command for that active), and
            //then the player ship runs out of shield/ammo/fuel it adds
            //a second command for that below, this section command for charging will not
            //get pulled in the command processing routine, because this source code
            //will only see the older Collectable collection command; so charging does not
            //work properly. I should improve this whole concept in future. As a workaround now
            //I will just delete all active commands when we want to charge something. This will
            //remove possible pending collectable commands; Additional I have prevented somewhere else
            //that collectables are commanded to be picked up as soon as a charging command was added to the list
            RemoveAllPendingCommands();

            //I need shield  
            AddCommand(CMD_CHARGE_SHIELD);

            //we want to accel/deaccelerate computer player craft
            //now much quicker, so that if we reach the charging area
            //finally we stop fast enough and do not overshoot the
            //charging area itself
            mCpCurrentDeaccelRate = CP_PLAYER_ACCELDEACCEL_RATE_CHARGING;
            return (linkForShield);
        }
    }

    //do we have link to fuel?
    if (linkForFuel) {
        //do we need fuel?
        if (DoIWantToChargeFuel()) {
            //please see important comment in shield section above
            //why we do this
            RemoveAllPendingCommands();

            //I need fuel
            AddCommand(CMD_CHARGE_FUEL);

            //we want to accel/deaccelerate computer player craft
            //now much quicker, so that if we reach the charging area
            //finally we stop fast enough and do not overshoot the
            //charging area itself
            mCpCurrentDeaccelRate = CP_PLAYER_ACCELDEACCEL_RATE_CHARGING;
            return (linkForFuel);
        }
    }

    //do we have link to ammo?
    if (linkForAmmo) {
        //do we need ammo?
        if (DoIWantToChargeAmmo()) {
            //please see important comment in shield section above
            //why we do this
            RemoveAllPendingCommands();

            //I need ammo
            AddCommand(CMD_CHARGE_AMMO);

            //we want to accel/deaccelerate computer player craft
            //now much quicker, so that if we reach the charging area
            //finally we stop fast enough and do not overshoot the
            //charging area itself
            mCpCurrentDeaccelRate = CP_PLAYER_ACCELDEACCEL_RATE_CHARGING;
            return (linkForAmmo);
        }
    }

    //if there is only one waypoint link available
    //just return this one
    int nrWays = linksNothingSpecial.size();
    if (nrWays > 0) {
        //return (linksNothingSpecial.at(0));

        //choose available path random
        int rNum;
        rNum = randRangeInt(0, nrWays - 1);

        return linksNothingSpecial.at(rNum);
    }

    if (linkForFuel) {
      return (linkForFuel);
    }

    if (linkForAmmo) {
      return (linkForAmmo);
    }

    //we still have no way to go
    if (linkForShield) {
      return (linkForShield);
    }

    return NULL;
}

bool Player::DoIWantToChargeShield() {
     if (this->mPlayerStats->shieldVal < (0.4 * this->mPlayerStats->shieldMax))
         return true;

     return false;
}

bool Player::DoIWantToChargeFuel() {
    if (this->mPlayerStats->gasolineVal < (0.5 * this->mPlayerStats->gasolineMax))
       return true;

    return false;
}

bool Player::DoIWantToChargeAmmo() {
    if (this->mPlayerStats->ammoVal < (0.3 * this->mPlayerStats->ammoMax))
        return true;

    return false;
}

void Player::ReachedEndCurrentFollowingSegments() {
  //  if (this->currClosestWayPointLink.first != NULL) {

        //which waypoint options do we have at the end of
        //our closest waypoint link?
        irr::core::vector3df endPointLink;

        //endPointLink = this->currClosestWayPointLink.first->pEndEntity->get_Pos();
        endPointLink = this->mCpFollowThisWayPointLink->pEndEntity->getCenter();

        std::vector<EntityItem*> availWaypoints =
              this->mRace->mPath->FindAllWayPointsInArea(endPointLink, 2.0f);

        mDbgCpAvailWaypointNr = availWaypoints.size();

        if (mDbgCpAvailWaypointNr > 0) {
             std::vector<EntityItem*>::iterator it;
             mCpAvailWayPointLinks.clear();

              std::vector<WayPointLinkInfoStruct*> fndLinks;
              std::vector<WayPointLinkInfoStruct*>::iterator it2;

             for (it = availWaypoints.begin(); it != availWaypoints.end(); ++it) {
                 //if ((*it) != this->currClosestWayPointLink.first->pEndEntity) {
                     //important: we need to exclude the waypoint link from the search we
                     //came from! otherwise we can loop around to the link we came from (mCpFollowThisWayPointLink)
                     fndLinks = this->mRace->mPath->FindWaypointLinksForWayPoint((*it), true, false,
                                this->mCpFollowThisWayPointLink);

                     if (fndLinks.size() > 0) {
                         for (it2 = fndLinks.begin(); it2 != fndLinks.end(); ++it2) {
                            mCpAvailWayPointLinks.push_back(*it2);
                         }
                     }
                // }
             }

             mDbgCpAvailWayPointLinksNr = mCpAvailWayPointLinks.size();
        }

        //do we currently want to pickup a collectible, this has priority
        if (this->mCpTargetCollectableToPickUp != NULL) {
            //is the waypoint link next to the collectible one of the waypoint links in front
            //of me, if so select this one and add a new path to the collectible
            if (this->mCpWayPointLinkClosestToCollectable != NULL) {
               std::vector<WayPointLinkInfoStruct*>::iterator it3;
               for (it3 = mCpAvailWayPointLinks.begin(); it3 != mCpAvailWayPointLinks.end(); ++it3) {
                   if ((*it3) == mCpWayPointLinkClosestToCollectable) {
                       //yes, we found it
                       //define path through it
                       PickupCollectableDefineNextSegment(mCpTargetCollectableToPickUp);
                       this->mCpFollowThisWayPointLink = (*it3);
                       /*if (mHUD != NULL) {
                         this->mHUD->ShowBannerText((char*)"COLLECT", 4.0f);
                       }*/

                       Entity::EntityType entType = (*it3)->pStartEntity->getEntityType();

                       //we need to go slower?
                       if (entType == Entity::EntityType::WaypointSlow) {
                           this->computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
                       } else if (entType == Entity::EntityType::WaypointFast) {
                           this->computerPlayerTargetSpeed = CP_PLAYER_FAST_SPEED;
                       }

                       return;
                   }
               }
            }
        }

        WayPointLinkInfoStruct* nextLink = NULL;

        //ask computer player logic what to do
        nextLink = CpPlayerWayPointLinkSelectionLogic(mCpAvailWayPointLinks);
        this->mCpFollowThisWayPointLink = nextLink;

        if (nextLink != NULL) {
            Entity::EntityType entType = nextLink->pStartEntity->getEntityType();

            //we need to go slower?
            if (entType == Entity::EntityType::WaypointSlow) {
                this->computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
            } else if (entType == Entity::EntityType::WaypointFast) {
                this->computerPlayerTargetSpeed = CP_PLAYER_FAST_SPEED;
            }

            FollowPathDefineNextSegment(nextLink, mCpCurrPathOffset, true);
           /* if (mHUD != NULL) {
               this->mHUD->ShowBannerText((char*)"REACHED END", 4.0f);
             }*/
      }
  //  }
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

bool Player::DoISeeACertainCollectable(Collectable* whichItem) {
    std::vector<Collectable*>::iterator it;

    for (it = this->mCpCollectablesSeenByPlayer.begin(); it != this->mCpCollectablesSeenByPlayer.end(); ++it) {
        if ((*it) == whichItem)
            return true;
    }

    return false;
}

void Player::CpPlayerCollectableSelectionLogic() {
    //does this player want to pickup a collectable right now?
    if (mCpTargetCollectableToPickUp != NULL) {
        //verify that the player still sees the collectable in his view region, if not remove
        //current command again, and return to normal path
        if (!DoISeeACertainCollectable(mCpTargetCollectableToPickUp)) {
            //I do not see it anymore
            //change back to normal race path

            mCpTargetCollectableToPickUp = NULL;
            mCpWayPointLinkClosestToCollectable = NULL;

            //only remove current computer player command to pickup the collectable
            //if we really wanted to pickup this collectable in the first place
            //otherwise we get wrong player behavior afterwards
            if ((currCommand->cmdType == CMD_PICKUP_COLLECTABLE) &&
                    (currCommand->targetCollectible == mCpTargetCollectableToPickUp)) {

                this->CurrentCommandFinished();
            }

            AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, mCpFollowThisWayPointLink);
        }

        //continue to pickup current targeted collectable item
        return;
    }

    //player has no collectable target currently
    //and no other special command as well
    //do we see something we want to have?
    if (currCommand != NULL) {
        if ((currCommand->cmdType == CMD_NOCMD) || (currCommand->cmdType == CMD_FOLLOW_TARGETWAYPOINTLINK)) {
            std::vector<Collectable*>::iterator it;
            Collectable* wantPickup = NULL;

            for (it = this->mCpCollectablesSeenByPlayer.begin(); (it != this->mCpCollectablesSeenByPlayer.end() && (wantPickup == NULL)); ++it) {
                if ((*it)->GetIfVisible()) {
                    switch ((*it)->mEntityItem->getEntityType()) {
                        case Entity::EntityType::ExtraShield: {
                            break;
                        }

                        case Entity::EntityType::ShieldFull: {
                            break;
                        }

                        case Entity::EntityType::DoubleShield: {
                            break;
                        }

                        case Entity::EntityType::ExtraAmmo: {
                            break;
                        }

                        case Entity::EntityType::AmmoFull: {
                            break;
                        }

                        case Entity::EntityType::DoubleAmmo: {
                            break;
                        }

                        case Entity::EntityType::ExtraFuel: {
                            break;
                        }

                        case Entity::EntityType::FuelFull: {
                            break;
                        }

                        case Entity::EntityType::DoubleFuel: {
                            break;
                        }

                        case Entity::EntityType::MinigunUpgrade: {
                            //we only want to pick this up if minigun is not
                            //already at highest level
                            if (this->mPlayerStats->currMinigunUpgradeLevel < 3) {
                                wantPickup = (*it);
                            }

                            break;
                        }

                        case Entity::EntityType::MissileUpgrade: {
                            //we only want to pick this up if missile is not
                            //already at highest level
                            if (this->mPlayerStats->currRocketUpgradeLevel < 3) {
                                wantPickup = (*it);
                            }
                            break;
                        }

                        case Entity::EntityType::BoosterUpgrade: {
                            //we only want to pick this up if booster is not
                            //already at highest level
                            if (this->mPlayerStats->currBoosterUpgradeLevel < 3) {
                                wantPickup = (*it);
                            }
                            break;
                        }

                        case Entity::EntityType::UnknownShieldItem: {
                            break;
                        }

                        case Entity::EntityType::UnknownItem: {
                            break;
                        }

                    }
                }
            }

            if (wantPickup != NULL) {
                //we found something we want to have
                AddCommand(CMD_PICKUP_COLLECTABLE, wantPickup);
            }
        }
    }

    //we do not want to pickup anything
}

irr::core::vector2df Player::GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord) {
    threeDCoord = this->phobj->physicState.position;

    irr::core::vector2df result(this->phobj->physicState.position.X, this->phobj->physicState.position.Z);

    return result;
}

irr::core::vector2df Player::GetBezierCurvePlaningCoordMidPoint(irr::core::vector3df point1, irr::core::vector3df point2, irr::core::vector3df &threeDCoord) {
     irr::core::vector3df midPoint = (point2 - point1) * irr::core::vector3df(0.5f, 0.5f, 0.5f) +  point1;
     threeDCoord = midPoint;

    irr::core::vector2df result(midPoint.X, midPoint.Z);

    return result;
}

void Player::PickupCollectableDefineNextSegment(Collectable* whichCollectable) {
    //create bezier curve
    //start point is the current end point of the path
    //control point is the start point of the link
    //in the path with the specified number
    //end point is the end point of the link in the
    //path with the defined number
    irr::core::vector3df link1Start3D;
    irr::core::vector3df link1End3D;

    //current player position, is start point for bezier curve 1
    irr::core::vector2df bezierPnt1 = this->GetMyBezierCurvePlaningCoord(debugPathPnt1);

    //collectable => is the control point for bezier curve 1
    irr::core::vector2df bezierCntrlPnt1 = whichCollectable->GetMyBezierCurvePlaningCoord(debugPathPnt2);

    //end point for next link is needed to calculate midpoint
    //irr::core::vector2df link1End = nextLink->pEndEntity->GetMyBezierCurvePlaningCoord(link1End3D);

    //take endpoint of link closest to collectable, is the bezier curve 1 end point
    irr::core::vector2df bezierPnt2 =  mCpWayPointLinkClosestToCollectable->pEndEntity->GetMyBezierCurvePlaningCoord(debugPathPnt3);

    std::vector<WayPointLinkInfoStruct*> newPoints;
    newPoints.clear();

    this->AdvanceDbgColor();

    newPoints = mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, bezierPnt2, bezierCntrlPnt1,
                                                                    0.1f, currDbgColor);

    std::vector<WayPointLinkInfoStruct*>::iterator it;

    mCurrentPathSeg.clear();

    //add new waypoints to the existing ones
    for (it = newPoints.begin(); it != newPoints.end(); ++it) {
        mCurrentPathSeg.push_back(*it);
    }

    mCurrentPathSegNrSegments = mCurrentPathSeg.size();

    ProjectPlayerAtCurrentSegments();

    //this->mRace->mGame->StopTime();
}

void Player::FollowPathDefineNextSegment(WayPointLinkInfoStruct* nextLink, irr::f32 startOffsetWay,
                                         bool updatePathReachedEndWayPointLink) {

    bool freeWayFound = false;
    //irr::f32 currOffset = startOffsetWay;
    irr::f32 currOffset = 0.0f;

   /* if (this->currClosestWayPointLink.first != NULL) {
        if (nextLink == this->currClosestWayPointLink.first) {
            if (nextLink->pntrPathNextLink != NULL) {
                nextLink = nextLink->pntrPathNextLink;
            }
        }
    }*/

    std::vector<WayPointLinkInfoStruct*> newPoints;
    newPoints.clear();

    this->AdvanceDbgColor();

    irr::f32 freeSpaceLeftOfPath;
    irr::f32 freeSpaceRightOfPath;

    if (updatePathReachedEndWayPointLink) {

        //in which direction should we shift the new way to clear target?
        //in which direction do we have more free space available?
        freeSpaceLeftOfPath = currOffset - nextLink->minOffsetShift;
        freeSpaceRightOfPath = nextLink->maxOffsetShift - currOffset;

        //do we need to change offset, because in front of us there is not enough
        //space available?
        if (freeSpaceLeftOfPath < 1.0f) {
            currOffset = nextLink->minOffsetShift * 0.35f;
        }

        if (freeSpaceRightOfPath < 1.0f) {
            currOffset = nextLink->maxOffsetShift * 0.35f;
        }
    }

    freeSpaceLeftOfPath = currOffset - nextLink->minOffsetShift;
    freeSpaceRightOfPath = nextLink->maxOffsetShift - currOffset;

    bool goleft = false;
    bool leftFailed = false;
    bool rightFailed = false;
    bool lastCalc = false;

    if (freeSpaceLeftOfPath > freeSpaceRightOfPath) {
        //go left
        goleft = true;
    }

    irr::s32 iterationCnt = 0;
    irr::s32 maxIterations = 20;

    //for bezier curve sanity check we also need the 2D waypoint link race direction
    irr::core::vector2df raceDirection = this->mRace->mPath->WayPointLinkGetRaceDirection2D(nextLink);

    while (!freeWayFound && !lastCalc && (iterationCnt < maxIterations)) {
            iterationCnt++;

            //create bezier curve
            //start point is the current end point of the path
            //control point is the start point of the link
            //in the path with the specified number
            //end point is the end point of the link in the
            //path with the defined number
            irr::core::vector3df link1Start3D;
            irr::core::vector3df link1End3D;

            if ((rightFailed && leftFailed) || (iterationCnt >= maxIterations)) {
                lastCalc = true;
            }

            //current player position, is start point for bezier curve 1
            irr::core::vector2df bezierPnt1 = this->GetMyBezierCurvePlaningCoord(debugPathPnt1);

            //next link start point => is the control point for bezier curve 1
            irr::core::vector2df bezierCntrlPnt1 = nextLink->pStartEntity->GetMyBezierCurvePlaningCoord(link1Start3D);
            this->mRace->mPath->OffsetWayPointLinkCoordByOffset(bezierCntrlPnt1, link1Start3D, nextLink, currOffset);

            debugPathPnt2 = link1Start3D;

            //end point for next link is needed to calculate midpoint
            irr::core::vector2df link1End = nextLink->pEndEntity->GetMyBezierCurvePlaningCoord(link1End3D);
            this->mRace->mPath->OffsetWayPointLinkCoordByOffset(link1End, link1End3D, nextLink, currOffset);

            //calculate midpoint for next link, is the bezier curve 1 end point
            irr::core::vector2df bezierPnt2 = this->GetBezierCurvePlaningCoordMidPoint(link1Start3D, link1End3D, debugPathPnt3);

            if (this->mRace->mPath->SaniCheckBezierInputPoints(bezierPnt1,bezierCntrlPnt1, bezierPnt2, raceDirection)) {
                 newPoints = mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, bezierPnt2, bezierCntrlPnt1,
                                                                                 0.1f, currDbgColor);
             } else {
                //default does not work, could be that next link is the link at which we are currently located
                //try different order
                if (this->mRace->mPath->SaniCheckBezierInputPoints(bezierPnt1,bezierPnt2, link1End, raceDirection)) {
                    //other order works
                    newPoints = mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, link1End, bezierPnt2,
                                                                                    0.1f, currDbgColor);

                    debugPathPnt2 = debugPathPnt3;
                    debugPathPnt3 = link1End3D;
                } else {
                    //also does not work
                    //try different order
                    //here we need the midpoint of the next waypoint link in front of us
                    if (nextLink->pntrPathNextLink != NULL) {
                            WayPointLinkInfoStruct* pntrLinkAfterwards = nextLink->pntrPathNextLink;
                            irr::core::vector3df linkAfterwardsStart3D;
                            irr::core::vector2df bezierPntNextLinkStart =
                                    pntrLinkAfterwards->pStartEntity->GetMyBezierCurvePlaningCoord(linkAfterwardsStart3D);
                            this->mRace->mPath->OffsetWayPointLinkCoordByOffset(bezierPntNextLinkStart, linkAfterwardsStart3D, pntrLinkAfterwards, currOffset);

                            debugPathPnt2 = linkAfterwardsStart3D;

                            irr::core::vector3df linkAfterwardsEnd3D;

                            //end point for link afterwards is needed to calculate midpoint
                            irr::core::vector2df bezierPntNextLinkEnd = pntrLinkAfterwards->pEndEntity->GetMyBezierCurvePlaningCoord(linkAfterwardsEnd3D);
                            this->mRace->mPath->OffsetWayPointLinkCoordByOffset(bezierPntNextLinkEnd, linkAfterwardsEnd3D, pntrLinkAfterwards, currOffset);

                            //calculate midpoint for link afterwards, is the bezier curve 1 end point
                            irr::core::vector2df curveEndPoint =
                                    this->GetBezierCurvePlaningCoordMidPoint(linkAfterwardsStart3D, linkAfterwardsEnd3D, debugPathPnt3);

                            if (this->mRace->mPath->SaniCheckBezierInputPoints(bezierPnt1,bezierPntNextLinkStart, curveEndPoint, raceDirection)) {
                                //other order works
                                newPoints = mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, curveEndPoint, bezierPntNextLinkStart,
                                                                                                0.1f, currDbgColor);
                               // this->mRace->mGame->StopTime();
                            } else {
                                //also this does not work
                                //just give up
                            }
                    }
                }
            }

            /*else {
                 //does also not work

                if (this->mRace->mPath->SaniCheckBezierInputPoints(bezierCntrlPnt1,bezierPnt1, bezierPnt2, raceDirection)) {
                    //yes, works
                    newPoints = mRace->testBezier->QuadBezierCurveGetSegments( bezierCntrlPnt1, bezierPnt2, bezierPnt1,
                                                                                    0.1f, currDbgColor);
                } else {
                    //also this solution is not ok, try another one
                    newPoints = mRace->testBezier->QuadBezierCurveGetSegments( bezierCntrlPnt1, bezierPnt2, bezierPnt1,
                                                                                    0.1f, currDbgColor);
                  }
                //this->mRace->mGame->StopTime();
            } */

            //now check if the new "way" is free from other players

                //only check possible collision with players we do actually see in front of us
                //otherwise we would report an possible collision with our path, when the other
                //player comes close to our current path behind us
                //and we do not want to detect this
                std::vector<Player*> playerISee;
                std::vector<RayHitInfoStruct>::iterator it;

                playerISee.clear();
                for (it = this->PlayerSeenList.begin(); it != this->PlayerSeenList.end(); ++it) {
                    if ((*it).HitType == RAY_HIT_PLAYER) {
                        playerISee.push_back((*it).HitPlayerPntr);
                    }
                }
                if (playerISee.size() > 0) {

                        if (!this->mRace->mPath->DoesPathComeTooCloseToAnyOtherPlayer(newPoints, playerISee, dbgPlayerInMyWay)) {
                                freeWayFound = true;
                        } else {
                                //no free way found
                                updatePathCnter++;

                                if (goleft) {
                                    currOffset -= 0.75f;
                                    if (currOffset < (nextLink->minOffsetShift + 1.0f)) {
                                        leftFailed = true;
                                        goleft = false;
                                    }

                                } else {
                                    //go right
                                    currOffset += 0.75f;

                                    if (currOffset > (nextLink->maxOffsetShift - 1.0f)) {
                                        rightFailed = true;
                                        goleft = true;
                                    }
                                }
                        }
            } else freeWayFound = true;


            if ((leftFailed && rightFailed) || (iterationCnt >= (maxIterations - 1))) {
               // currOffset = 0.0f;
               // this->mRace->mGame->StopTime();
            }
    }

    std::vector<WayPointLinkInfoStruct*>::iterator it;

    mCurrentPathSeg.clear();
    irr::f32 angleDotProduct;
    irr::f32 angle;
    irr::f32 angleRad;

    //add new waypoints to the existing ones
    for (it = newPoints.begin(); it != newPoints.end(); ++it) {
        //final additional check:
        //sort all segments out that are looking in the opposite direction
        //as the player craft is currently oriented
        //to make sure we do not reverse direction accidently suddently
        angleDotProduct = (*it)->LinkDirectionVec.dotProduct(craftForwardDirVec);

        angleRad = acosf(angleDotProduct);
        angle = (angleRad / irr::core::PI) * 180.0f;

       if (craftSidewaysToRightVec.dotProduct((*it)->LinkDirectionVec) > 0.0f) {
           angle = -angle;
       }

        //if not in opposite direction add new segment path
        if (fabs(angle) < 90.0f) {
            mCurrentPathSeg.push_back(*it);
            mPathHistoryVec.push_back(*it);
        }
    }

    //update current player offset path value
    mCpCurrPathOffset = currOffset;

    mCurrentPathSegNrSegments = mCurrentPathSeg.size();

    ProjectPlayerAtCurrentSegments();

    //this->mRace->mGame->StopTime();
}

/*irr::core::vector3df Player::DeriveCurrentDirectionVector(WayPointLinkInfoStruct *currentWayPointLine, irr::f32 progressCurrWayPoint) {
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
}*/

void Player::ShowPlayerBigGreenHudText(char* text, irr::f32 timeDurationShowTextSec) {
    if (mHUD != NULL) {
        this->mHUD->ShowGreenBigText(text, timeDurationShowTextSec);
    }
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

        irr::core::vector3df entPos = currCommand->targetEntity->getCenter();

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

void Player::RunComputerPlayerLogic(irr::f32 deltaTime) {
    this->CpCurrMissionState = CP_MISSION_FINISHLAPS;

    if (currCommand == NULL) {
        currCommand = PullNextCommandFromCmdList();
    }

    switch (currCommand->cmdType) {
        case CMD_NOCMD: {
           /* if (mHUD != NULL) {
              this->mHUD->ShowBannerText((char*)"NO CMD", 4.0f);
            }*/
            break;
        }

        case CMD_FLYTO_TARGETENTITY: {
                FlyTowardsEntityRunComputerPlayerLogic(currCommand);
            break;
        }

        case CMD_FLYTO_TARGETPOSITION: {
            break;
        }

        case CMD_CHARGE_FUEL:
        case CMD_CHARGE_AMMO:
        case CMD_CHARGE_SHIELD: {
            CpHandleCharging();
            break;
        }

        case CMD_PICKUP_COLLECTABLE: {
            this->mCpTargetCollectableToPickUp = currCommand->targetCollectible;

            if (mCpWayPointLinkClosestToCollectable == NULL) {
                //figure out which target link is closest to this collectable
                std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> wayPointLinkCloseToCollectable =
                        this->mRace->mPath->FindClosestWayPointLinkToCollectible(mCpTargetCollectableToPickUp);

                if (wayPointLinkCloseToCollectable.first != NULL) {
                   mCpWayPointLinkClosestToCollectable = wayPointLinkCloseToCollectable.first;
                }
            }

            break;
        }

        case CMD_FOLLOW_TARGETWAYPOINTLINK: {
            mCpFollowThisWayPointLink = currCommand->targetWaypointLink;
            mCpLastFollowThisWayPointLink = currCommand->targetWaypointLink;
            //FollowPathDefineNextSegment(mCpLastFollowThisWayPointLink, mCpCurrPathOffset, true);
            FollowPathDefineNextSegment(mCpLastFollowThisWayPointLink, mCpCurrPathOffset, false);
            /*if (mHUD != NULL) {
              this->mHUD->ShowBannerText((char*)"FOLLOW", 4.0f);
            }*/
            computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;

            CurrentCommandFinished();
            break;
        }

    }

    //If true enables computer player stuck detection
    //turn off for testing computer player movement
    //performance and stabilitiy testing to keep computer
    //players stuck; Set true for final release to handle
    //random cases where computer players really get stuck, so that
    //races can for sure finish
    if (this->mRace->CpEnableStuckDetection) {
        CpStuckDetection(deltaTime);
    }

    CpPlayerCollectableSelectionLogic();

    //check for obstacles only every 300 ms
    mCpAbsCheckObstacleTimerCounter += deltaTime;

    if (mCpAbsCheckObstacleTimerCounter >= 0.3f) {
        mCpAbsCheckObstacleTimerCounter -= 0.3f;

        CpCheckCurrentPathForObstacles();
    }

    //CpPlayerHandleAttack();

    //for all computer players in this race we need to call the
    //CPForceController which has the job to control the crafts movement
    //so that the computer is following the currenty definded target path
    CPForceController();

    return;
}

void Player::CpHandleCharging() {
   //what charging do we need to do?
   switch(currCommand->cmdType) {
        case CMD_CHARGE_AMMO: {
          if (mCurrChargingAmmo) {
              if (this->mPlayerStats->ammoVal >= (0.95 * this->mPlayerStats->ammoMax)) {
                  //charging finished
                  mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;
                  CurrentCommandFinished();

                  //old lines before WorkaroundResetCurrentPath
                  AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
                  computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
              } else {
                  //lets stop computer player until
                  //charging is finished
                  computerPlayerTargetSpeed = 0.0f;
              }

              break;
          }
       }

       case CMD_CHARGE_SHIELD: {
         if (mCurrChargingShield) {
             if (this->mPlayerStats->shieldVal >= (0.95 * this->mPlayerStats->shieldMax)) {
                 //charging finished
                 mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;
                 CurrentCommandFinished();

                 //old lines before WorkaroundResetCurrentPath
                 AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
                 computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
             } else {
                 //lets stop computer player until
                 //charging is finished
                 computerPlayerTargetSpeed = 0.0f;
             }

             break;
         }
      }

       case CMD_CHARGE_FUEL: {
         if (mCurrChargingFuel) {
             if (this->mPlayerStats->gasolineVal >= (0.95 * this->mPlayerStats->gasolineMax)) {
                 //charging finished
                 mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;

                 CurrentCommandFinished();

               //  this->mRace->mGame->StopTime();

                 //old lines before WorkaroundResetCurrentPath
                 AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
                 computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
             } else {
                 //lets stop computer player until
                 //charging is finished
                 computerPlayerTargetSpeed = 0.0f;
             }

             break;
         }
        }
   }
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
        structPntrVec = mRace->mPath->FindWaypointLinksForWayPoint((*it), true, true, NULL);

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
   irr::f32 distStart = (this->phobj->physicState.position - currLink->pStartEntity->getCenter()).getLength();
   irr::f32 distEnd = (this->phobj->physicState.position - currLink->pEndEntity->getCenter()).getLength();

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

    //recalculate current 2D cell coordinates
    //where the player is currently located
    mCurrPosCellX = -(this->phobj->physicState.position.X / mRace->mLevelTerrain->segmentSize);
    mCurrPosCellY = (this->phobj->physicState.position.Z / mRace->mLevelTerrain->segmentSize);

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

    /*  if (maxh > 0.0f) {
          adjusth = maxh * 0.5f;
      } else adjusth = 0.0f;*/

    PlayerCraftHeightControl();

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
            if (!mEmptyFuelWarningAlreadyShown) {
                if (mHUD != NULL) {
                    this->mHUD->ShowBannerText((char*)"FUEL EMPTY", 4.0f, true);
                }
                mEmptyFuelWarningAlreadyShown = true;

                if (this->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_RACING) {
                    //change player state to empty fuel state
                    SetNewState(STATE_PLAYER_EMPTYFUEL);

                    //call a recovery vehicle to help us out
                    this->mRace->CallRecoveryVehicleForHelp(this);
                    mRecoveryVehicleCalled = true;
                }
            }
        } else if (mPlayerStats->gasolineVal < (mPlayerStats->gasolineMax * 0.25f)) {
            if (!mLowFuelWarningAlreadyShown) {
                if (mHUD != NULL) {
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

     if (mPlayerStats->gasolineVal > (mPlayerStats->gasolineMax * 0.25f)) {
          mLowFuelWarningAlreadyShown = false;
     }

    //TODO: check with actual game how ammo warnings and reduction works exactly
    if (mPlayerStats->ammoVal <= 0.0f) {
        if (!mEmptyAmmoWarningAlreadyShown) {
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"AMMO EMPTY", 4.0f, true);
            }
            mEmptyAmmoWarningAlreadyShown = true;
        }
    } else if (mPlayerStats->ammoVal < (mPlayerStats->ammoMax * 0.25f)) {
        if (!mLowAmmoWarningAlreadyShown) {
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"AMMO LOW", 4.0f, true);
            }
            mLowAmmoWarningAlreadyShown = true;
        }
    }

     if (mPlayerStats->ammoVal > 0.0f) {
         mEmptyAmmoWarningAlreadyShown = false;
     }

     if (mPlayerStats->ammoVal > (mPlayerStats->ammoMax * 0.25f)) {
          mLowAmmoWarningAlreadyShown = false;
     }

    //TestCpForceControlLogicWithHumanPlayer();

    CalcPlayerCraftLeaningAngle();

    if (!isnan(currPlayerCraftLeaningAngleDeg)) {

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
    }

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

    //check if player entered a craft trigger region
    CheckForTriggerCraftRegion();

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

void Player::PlayerCraftHeightControl() {
    //*****************************************************
    //* Hovercraft height control force calculation Start *  solution 1: with the 4 local points left, right, front and back of craft
    //*****************************************************

    //remember last distance in front of craft towards race track
    //needed for jump detection
    lastHeightFront = currHeightFront;
    lastHeightBack = currHeightBack;

    //establish height information of race track below player craft
    GetHeightRaceTrackBelowCraft(currHeightFront, currHeightBack, currHeightLeft, currHeightRight);

    //internal variable firstHeightControlLoop is used to prevent a
    //first unwanted JUMP detection when the first loop of PlayerCraftHeightControl is
    //executed at the start of the race (due to uninitialized variables)
    if (!firstHeightControlLoop) {
        //is craft currently jumping?
        if (!mCurrJumping) {
            //are we suddently start a jump?
            //we should be able to detect this when the front distance between craft
            //and race track below goes suddently much longer, but the distance from the craft
            //back to the racetrack is still similar then before
            if (currHeightFront < (lastHeightFront - CRAFT_JUMPDETECTION_THRES)) {
                if (currHeightBack > (lastHeightBack - CRAFT_JUMPDETECTION_THRES)) {
                    this->mCurrJumping = true;
                 /*   if (mHUD !=NULL) {
                        this->mHUD->ShowGreenBigText("JUMP", 4.0f);
                    }*/
                }
            }
        } else {
            //craft currently jumping
            //is the jump over again?
            //the jump is over when distance at craft front and
            //back towards the race track is similar enough again
            irr::f32 heightFrontJump = (WorldCoordCraftFrontPnt.Y - (currHeightFront + HOVER_HEIGHT));

            if (heightFrontJump < HOVER_HEIGHT) {
                this->mCurrJumping = false;
/*
                if (mHUD !=NULL) {
                    this->mHUD->ShowGreenBigText("JUMP END", 4.0f);
                }*/
            }
        }

        if (mCurrJumping) {
            //slowly move craft downwards while jumping, instead of the normal
            //craft height control below, while we jump we are disconnected from the
            //race track surface
            this->phobj->AddLocalCoordForce(LocalCraftOrigin, LocalCraftOrigin - irr::core::vector3df(0.0f, 50.0f, 0.0f), PHYSIC_APPLYFORCE_REAL,
                                            PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL);

            //when we jump exit, no more height control necessary
            return;
        }
    } else {
        //internal variables to prevent first unwanted
        //jump after start of race
        firstHeightControlLoop = false;
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

    //if craft is at all points higher than racetrack let it go towards racetrack slower (too allow something like a jump)
    if ((heightErrorFront > 0.0f) && (heightErrorBack > 0.0f) && (heightErrorLeft > 0.0f) && (heightErrorRight > 0.0f)) {
        corrForceHeight = 40.0f;
    }

    //original lines until 21.12.2024
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
            mRecoveryVehicleCalled = true;

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

void Player::CheckForTriggerCraftRegion() {
    //remember last trigger region before next update
    mLastCraftTriggerRegion = mCurrentCraftTriggerRegion;

    std::vector<MapTileRegionStruct*>::iterator itRegion;

    mCurrentCraftTriggerRegion = NULL;

    //check for each trigger region in level
    for (itRegion = this->mRace->mTriggerRegionVec.begin(); itRegion != this->mRace->mTriggerRegionVec.end(); ++itRegion) {
        //only check for regions which are a playercraft trigger region
        if ((*itRegion)->regionType == LEVELFILE_REGION_TRIGGERCRAFT) {
            //is the player inside this area?
            if (this->mRace->mLevelTerrain->CheckPosInsideRegion(mCurrPosCellX,
                    mCurrPosCellY, (*itRegion))) {

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
    if (mCurrentCraftTriggerRegion != NULL) {
        if (mCurrentCraftTriggerRegion != mLastCraftTriggerRegion) {
            //yes, we hit a new trigger region
            mRace->PlayerEnteredCraftTriggerRegion(this, mCurrentCraftTriggerRegion);
        }
    }
}

void Player::CheckForChargingStation() {
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

    //see if we are currently in an charging area with this player
    this->mRace->mLevelTerrain->CheckPosInsideChargingRegion(mCurrPosCellX, mCurrPosCellY,
                                                             cShield, cFuel, cAmmo);

    if (cShield) {
       mCurrChargingShield = true;

       if (mPlayerStats->shieldVal < mPlayerStats->shieldMax)
        {
           atCharger = true;

            this->mPlayerStats->shieldVal++;

            if (mHUD != NULL) {
                RepairGlasBreaks();

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

    if (cFuel) {
       mCurrChargingFuel = true;

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

    if (mCurrChargingFuel != mLastChargingFuel) {
        if (mCurrChargingFuel) {
            //charging fuel started
             if (atCharger) {
               if (mHUD != NULL) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"FUEL RECHARGING", -1.0f);
               }
             }

        } else {
            if (mHUD != NULL) {
                this->mHUD->CancelAllPermanentBannerTextMsg();
            }
            mBlockAdditionalFuelFullMsg = false;
        }
    }

    if (mCurrChargingAmmo != mLastChargingAmmo) {
        if (mCurrChargingAmmo) {
            //charging Ammo started
             if (atCharger) {
               if (mHUD != NULL) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"AMMO RECHARGING", -1.0f);
               }
             }

        } else {
            if (mHUD != NULL) {
                this->mHUD->CancelAllPermanentBannerTextMsg();
            }
            mBlockAdditionalAmmoFullMsg = false;
        }
    }

    if (mCurrChargingShield != mLastChargingShield) {
        if (mCurrChargingShield) {
            //charging shield started
             if (atCharger) {
               if (mHUD != NULL) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"SHIELD RECHARGING", -1.0f);
               }
             }

        } else {
            if (mHUD != NULL) {
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
    MapEntry* tilePntr = this->mRace->mLevelTerrain->GetMapEntry(mCurrPosCellX, mCurrPosCellY);
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

//is called when the player collected a collectable item of the
//level
void Player::CollectedCollectable(Collectable* whichCollectable) {
    //play sound
    if (this->mHumanPlayer) {
        this->mRace->mSoundEngine->PlaySound(SRES_GAME_PICKUP);
    }

    EntityItem entity = *whichCollectable->mEntityItem;

  //depending on the type of entity/collectable alter player stats
    Entity::EntityType type = whichCollectable->mEntityItem->getEntityType();

    //Alex TODO important: figure out what the items do exactly with the player stats and how
    //much the effect is, right now this stuff does not make much sense to me
    switch (type) {
        case Entity::EntityType::ExtraFuel:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"EXTRA FUEL", 4.0f);
            }

            this->mPlayerStats->gasolineVal += 10.0f;
            if (this->mPlayerStats->gasolineVal > this->mPlayerStats->gasolineMax)
                this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

            break;

        case Entity::EntityType::FuelFull:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"FUEL FULL", 4.0f);
            }

            this->mPlayerStats->gasolineVal += 100.0f;
            if (this->mPlayerStats->gasolineVal > this->mPlayerStats->gasolineMax)
                this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

            break;

        case Entity::EntityType::DoubleFuel:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"DOUBLE FULL", 4.0f);
            }

            this->mPlayerStats->gasolineVal += 200.0f;
            if (this->mPlayerStats->gasolineVal > this->mPlayerStats->gasolineMax)
                this->mPlayerStats->gasolineVal = this->mPlayerStats->gasolineMax;

            break;

        case Entity::EntityType::ExtraAmmo:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"EXTRA AMMO", 4.0f);
            }
            this->mPlayerStats->ammoVal += 10.0f;
            if (this->mPlayerStats->ammoVal > this->mPlayerStats->ammoMax)
                this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;

            break;

        case Entity::EntityType::AmmoFull:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"AMMO FULL", 4.0f);
            }
            this->mPlayerStats->ammoVal += 100.0f;
            if (this->mPlayerStats->ammoVal > this->mPlayerStats->ammoMax)
                this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;

            break;

        case Entity::EntityType::DoubleAmmo:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"DOUBLE AMMO", 4.0f);
            }

            this->mPlayerStats->ammoVal += 200.0f;
            if (this->mPlayerStats->ammoVal > this->mPlayerStats->ammoMax)
                this->mPlayerStats->ammoVal = this->mPlayerStats->ammoMax;

            break;

        case Entity::EntityType::ExtraShield:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"EXTRA SHIELD", 4.0f);
            }

            this->mPlayerStats->shieldVal += 10.0f;
            if (this->mPlayerStats->shieldVal > this->mPlayerStats->shieldMax)
                this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;

            break;

        case Entity::EntityType::ShieldFull:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"SHIELD FULL", 4.0f);
            }

            this->mPlayerStats->shieldVal += 100.0f;
            if (this->mPlayerStats->shieldVal > this->mPlayerStats->shieldMax)
                this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;

            break;

        case Entity::EntityType::DoubleShield:
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"DOUBLE SHIELD", 4.0f);
            }

            this->mPlayerStats->shieldVal += 200.0f;
            if (this->mPlayerStats->shieldVal > this->mPlayerStats->shieldMax)
                this->mPlayerStats->shieldVal = this->mPlayerStats->shieldMax;

            break;

        case Entity::EntityType::BoosterUpgrade:
            //upgrade players booster
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"BOOSTER UPGRADED", 4.0f);
            }

            if (this->mPlayerStats->currBoosterUpgradeLevel < 3)
                this->mPlayerStats->currBoosterUpgradeLevel++;

            break;

        case Entity::EntityType::MissileUpgrade:
            //upgrade players rocket
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"MISSILE UPGRADED", 4.0f);
            }

            if (this->mPlayerStats->currRocketUpgradeLevel < 3)
                this->mPlayerStats->currRocketUpgradeLevel++;

            break;

        case Entity::EntityType::MinigunUpgrade:
            //upgrade players mini-gun
            if (mHUD != NULL) {
                this->mHUD->ShowBannerText((char*)"MINIGUN UPGRADED", 4.0f);
            }

            if (this->mPlayerStats->currMinigunUpgradeLevel < 3)
                this->mPlayerStats->currMinigunUpgradeLevel++;

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

//    //if this is a computer player, was this item planed to be picked
//    //up? if so reset variables for collectable logic
//    if (!this->mHumanPlayer) {
//        if (this->mCpTargetCollectableToPickUp == whichCollectable) {
//            //yes, correct item picked up
//            //set back to NULL, so that player can lookup
//            //next item he wants to collect
//            mCpTargetCollectableToPickUp = NULL;
//            this->mCpWayPointLinkClosestToCollectable = NULL;

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

    //do we need to show HUD Message for "final lap"
    if (mPlayerStats->currLapNumber == mPlayerStats->raceNumberLaps) {
        if (this->mRace->currPlayerFollow != NULL) {
            if (this->mRace->currPlayerFollow == this) {
                if (mHUD != NULL) {
                    mHUD->ShowGreenBigText((char*)"FINAL LAP", 4.0f);
                }

                //play the yee-haw sound
                mRace->mSoundEngine->PlaySound(SRES_GAME_FINALLAP, false);
            }
        }
    }

    //reset current lap time
    mPlayerStats->currLapTimeExact = 0.0;
    mPlayerStats->currLapTimeMultiple100mSec = 0;
}

//adds a single random location glas break
void Player::AddGlasBreak() {
    irr::s32 rNum = rand();
    irr::f32 rWidthFloat = (float(rNum) / float (RAND_MAX)) * this->mRace->mGame->mGameScreenRes.Width;

    rNum = rand();
    irr::f32 rHeightFloat = (float(rNum) / float (RAND_MAX)) * this->mRace->mGame->mGameScreenRes.Height;

    HudDisplayPart* newGlasBreak = new HudDisplayPart();
    newGlasBreak->texture = this->mHUD->brokenGlas->texture;
    newGlasBreak->altTexture = this->mHUD->brokenGlas->altTexture;
    newGlasBreak->sizeTex = this->mHUD->brokenGlas->sizeTex;

    newGlasBreak->drawScrPosition.set(rWidthFloat, rHeightFloat);

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

            if (pntr->texture != NULL) {
                //remove underlying texture
                this->mRace->mDriver->removeTexture(pntr->texture);
            }

            if (pntr->altTexture != NULL) {
                //remove underlying texture
                this->mRace->mDriver->removeTexture(pntr->altTexture);
            }

            delete pntr;
        }
    }
}

void Player::CpPlayerHandleAttack() {
    //if I do not see any other player, simply return
    if (this->PlayerSeenList.size() < 1)
        return;

    //if we have no target player, just return
    if (mTargetPlayer == NULL)
        return;

    //if we have a (red) perfect lock on another player, and enough ammo
    //fire missile
    if (this->mPlayerStats->ammoVal > 0.0f) {
        if (this->mTargetMissleLock) {
            this->mMissileLauncher->Trigger();

            //fire one missile is enough
            //just exit
            return;
        }
    }

    //machine gun currently cool enough
    //if so, do we have a target right now?
    if (!this->mMGun->CoolDownNeeded()) {
            //yes, fire
            this->mMGun->Trigger();
        }
}

