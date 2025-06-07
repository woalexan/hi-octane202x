/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "cpuplayer.h"

CpuPlayer::CpuPlayer(Player* myParentPlayer) {
   mParentPlayer = myParentPlayer;

   mPathHistoryVec.clear();

   currDbgColor = mParentPlayer->mRace->mDrawDebug->red;

   //create the player command list
   cmdList = new std::list<CPCOMMANDENTRY*>();
}

CpuPlayer::~CpuPlayer() {
   //delete/clean all stuff
   //linked to player commands
   CleanUpCommandList();
}

bool CpuPlayer::IsCurrentlyStuck() {
    return mCpPlayerCurrentlyStuck;
}

void CpuPlayer::WasDestroyed() {
    computerPlayerCurrentSpeed = 0.0f;
    computerPlayerTargetSpeed = 0.0f;
}

void CpuPlayer::StartSignalShowsGreen() {
    computerPlayerTargetSpeed = CP_PLAYER_FAST_SPEED;
}

void CpuPlayer::SetupForRaceStart(irr::core::vector3df startPos) {
    //for a computer player additional add first player command
    EntityItem* entItem = mParentPlayer->mRace->mPath->FindFirstWayPointAfterRaceStartPoint();
    if (entItem != NULL) {
       //get waypoint link for this waypoint
        std::vector<WayPointLinkInfoStruct*> foundLinks;

       foundLinks = mParentPlayer->mRace->mPath->FindWaypointLinksForWayPoint(entItem, true, false, NULL);

       if (foundLinks.size() > 0) {
           AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, foundLinks.at(0));

           //we also want to preset the waypoint link follow logic offset
           //to prevent collision of all the player craft at the tight start situation
           //preset by X coordinate difference between player origin and found waypoint link start entity
           irr::core::vector3df startWayPoint = foundLinks.at(0)->pStartEntity->getCenter();

           irr::core::vector3df deltaVec = startPos - startWayPoint;
           //deltaVec.Y = 0.0f;
           //deltaVec.Z = 0.0f;

           mCpCurrPathOffset = -deltaVec.X;
       }
    }
}

void CpuPlayer::SetCurrClosestWayPointLink(std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> newClosestWayPointLink) {
    if (newClosestWayPointLink.first != NULL) {
        this->currClosestWayPointLink = newClosestWayPointLink;
        this->projPlayerPositionClosestWayPointLink = newClosestWayPointLink.second;
    }
}

void CpuPlayer::FreedFromRecoveryVehicleAgain() {
    //if we are a computer player, and we got stuck and the stuck
    //detection save us, make sure that this will not happen anymore, by changing
    //current path offset
    if (mCpPlayerCurrentlyStuck) {
        if (mCpPlayerStuckAtSide == CP_PLAYER_WAS_STUCKLEFTSIDE) {
              //move current path offset a little bit to the right
              mCpCurrPathOffset += 0.5f;
        } else if (mCpPlayerStuckAtSide == CP_PLAYER_WAS_STUCKRIGHTSIDE) {
              //move current path offset a little bit to the left
              mCpCurrPathOffset -= 0.5f;
        }

        mCpPlayerStuckAtSide = CP_PLAYER_WAS_STUCKUNDEFINED;
        mCpPlayerCurrentlyStuck = false;
    }

    //renew the current path for computer
    //player
    WorkaroundResetCurrentPath();
}

void CpuPlayer::CpTakeOverHuman() {
    this->computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;

    mCpCurrPathOffset = 0.0f;

    //"jump start" computer player speed, so that craft does not immediately
    //stop when computer player takes over
    computerPlayerCurrentSpeed = mParentPlayer->phobj->physicState.speed;

    if (computerPlayerCurrentSpeed < CP_PLAYER_SLOW_SPEED) {
        computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
    }

    WorkaroundResetCurrentPath();
}

void CpuPlayer::AddCommand(uint8_t cmdType, EntityItem* targetEntity) {
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
    newLineStr->A = mParentPlayer->phobj->physicState.position;

    irr::core::vector3df posEntity = targetEntity->getCenter();
    //our universe is flipped for X axis
    newLineStr->B = posEntity;

    //set white as default color
    newLineStr->color = mParentPlayer->mRace->mDrawDebug->white;
    newLineStr->name = new char[10];
    strcpy(newLineStr->name, "");

    newStruct->pLineStruct = newLineStr;
    irr::core::vector3df vec3D = (newLineStr->B - newLineStr->A);

    //precalculate and store length as we will need this very often
    //during the game loop for race position update
    newStruct->length3D = vec3D.getLength();
    vec3D.normalize();

    newStruct->LinkDirectionVec = vec3D;
    newStruct->LinkDirectionVec.normalize();

    //Idea: extend the lines a little bit further outwards at
    //both ends, so that when we project the players position on
    //the different segments later we always find a valid segment
    LineStruct *lineExt = new LineStruct;

    lineExt->A = newLineStr->A - newStruct->LinkDirectionVec * 0.5f;
    lineExt->B = newLineStr->B + newStruct->LinkDirectionVec * 0.5f;

    lineExt->name = new char[100];
    sprintf(lineExt->name, "");

    //set white as default color
    lineExt->color = mParentPlayer->mRace->mDrawDebug->white;

    newStruct->pLineStructExtended = lineExt;

    newStruct->minOffsetShiftEnd = -10.0f;
    newStruct->maxOffsetShiftEnd = 10.0f;
    newStruct->minOffsetShiftStart = -10.0f;
    newStruct->maxOffsetShiftStart = 10.0f;

    //precalculate a direction vector which stands at a 90 degree
    //angle at the original waypoint direction vector, and always points
    //to the right direction when looking into race direction
    //this direction vector is later used during the game to offset the player
    //path sideways
    newStruct->offsetDirVec = newStruct->LinkDirectionVec.crossProduct(-*mParentPlayer->mRace->yAxisDirVector).normalize();

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

void CpuPlayer::AddCommand(uint8_t cmdType, WayPointLinkInfoStruct* targetWayPointLink) {
    if (cmdType != CMD_FOLLOW_TARGETWAYPOINTLINK || targetWayPointLink == NULL)
        return;

    CheckAndRemoveNoCommand();

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = CMD_FOLLOW_TARGETWAYPOINTLINK;
    newCmd->targetWaypointLink = targetWayPointLink;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void CpuPlayer::AddCommand(uint8_t cmdType) {
    CheckAndRemoveNoCommand();

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = cmdType;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void CpuPlayer::AddCommand(uint8_t cmdType, ChargingStation* whichChargingStation, ChargerStoppingRegionStruct* whichStall) {
    if (cmdType != CMD_GOTO_CHARGINGSTATION)
        return;

    if ((whichChargingStation == NULL) || (whichStall == NULL))
        return;

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = cmdType;
    newCmd->targetChargingStation = whichChargingStation;

    mReachedChargingStation = false;
    mReachedChargingStall = false;
    mSetupPathToChargingStation = false;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void CpuPlayer::CpCommandPlayerToChargingStall(ChargingStation* whichChargingStation, ChargerStoppingRegionStruct* whichStall) {
    if ((whichChargingStation == NULL) || (whichStall == NULL))
        return;

        //create a new temporary waypoint link from computer player crafts current
        //position towards the charging station we want to fly to
        WayPointLinkInfoStruct* newStruct = new WayPointLinkInfoStruct();

        LineStruct* newLineStr = new LineStruct();
        //newLineStr->A = whichChargingStation->enterEntityItem->getCenter();
        //newLineStr->B = whichStall->entityItem->getCenter();

        newLineStr->A = whichStall->entityItem->getCenter();
        newLineStr->B = whichChargingStation->enterHelperEntityItem->getCenter();

        //set white as default color
        newLineStr->color = mParentPlayer->mRace->mDrawDebug->white;
        newLineStr->name = new char[10];
        strcpy(newLineStr->name, "");

        newStruct->pLineStruct = newLineStr;
        irr::core::vector3df vec3D = (newLineStr->B - newLineStr->A);

        //precalculate and store length as we will need this very often
        //during the game loop for race position update
        newStruct->length3D = vec3D.getLength();
        vec3D.normalize();

        newStruct->LinkDirectionVec = vec3D;
        newStruct->LinkDirectionVec.normalize();

        //Idea: extend the lines a little bit further outwards at
        //both ends, so that when we project the players position on
        //the different segments later we always find a valid segment
        LineStruct *lineExt = new LineStruct;

        lineExt->A = newLineStr->A - newStruct->LinkDirectionVec * 0.5f;
        lineExt->B = newLineStr->B + newStruct->LinkDirectionVec * 0.5f;

        lineExt->name = new char[100];
        sprintf(lineExt->name, "");

        //set white as default color
        lineExt->color = mParentPlayer->mRace->mDrawDebug->white;

        newStruct->pLineStructExtended = lineExt;

        //newStruct->pStartEntity = whichChargingStation->enterEntityItem;
        newStruct->pStartEntity = whichStall->entityItem;

        newStruct->minOffsetShiftEnd = -10.0f;
        newStruct->maxOffsetShiftEnd = 10.0f;
        newStruct->minOffsetShiftStart = -10.0f;
        newStruct->maxOffsetShiftStart = 10.0f;

       // mParentPlayer->mRace->dbgCoord = newStruct->pLineStruct->B;

        //precalculate a direction vector which stands at a 90 degree
        //angle at the original waypoint direction vector, and always points
        //to the right direction when looking into race direction
        //this direction vector is later used during the game to offset the player
        //path sideways
        newStruct->offsetDirVec = newStruct->LinkDirectionVec.crossProduct(-*mParentPlayer->mRace->yAxisDirVector).normalize();
        //newStruct->pEndEntity = whichStall->entityItem;

        newStruct->pEndEntity = whichChargingStation->enterHelperEntityItem;

        mLocationChargingStall = whichStall->entityItem->getCenter();

        CheckAndRemoveNoCommand();

        CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
        newCmd->cmdType = CMD_GOTO_CHARGINGSTATION;

        //also store this temporary waypointlink struct info
        //in the command, so that we can cleanup after this command was
        //executed
        newCmd->targetWaypointLink = newStruct;

        //mark waypoint link as temporary created, so that
        //we delete it again after command was fully executed
        newCmd->WayPointLinkTemporary = true;

        //add the new command to the end of the command list
        this->cmdList->push_back(newCmd);

        mSetupPathToChargingStall = false;
}

void CpuPlayer::CpCommandPlayerToExitChargingStall(ChargingStation* whichChargingStation) {
    if (whichChargingStation == NULL)
        return;

        //create a new temporary waypoint link from computer player crafts current
        //position via charging station exit point and via exitWayPointLink

        //if we have no exitWayPointLink in this charging station, we can not exit
        //in this case, just return
        if (whichChargingStation->exitWayPointLink == NULL)
            return;

        //do we use start or exit entity of the exitWayPointLink to exit the charging
        //station? this depends on if startEntity is still in front of the player or not
        //do not advance entity in this first case
        EntityItem* nextEntityInFrontOfPlayer =
                this->mParentPlayer->mRace->mPath->GetWayPointLinkEntityItemInFrontOfPlayer(whichChargingStation->exitWayPointLink, false, this->mParentPlayer);

        //construct the "exit" route by combining current player location, then charging station exit point, and
        //the next entity of exitWayPointLink of charging station in front of player
        WayPointLinkInfoStruct* newStruct = new WayPointLinkInfoStruct();

        LineStruct* newLineStr = new LineStruct();

        newLineStr->A = whichChargingStation->exitEntityItem->getCenter();
        newLineStr->B = nextEntityInFrontOfPlayer->getCenter();

        //if we have bad luck it can happen that we select the proper nextEntityInFrontOfPlayer, but
        //this entity is placed between the player and the chargerStation helperEntityItem,
        //in which case the computer player control setup would fail
        //in this case lets take the next entity on the path out of the charging station,
        //by advancing entities by one (see function parameter below which is set to true);
        bool swap3DPointOrder =
                !this->mParentPlayer->mRace->mPath->SaniCheck3DPointOrder(newLineStr->A, newLineStr->B, this->mParentPlayer);

        if (swap3DPointOrder) {
            //we need to take the next waypoint entity on the exit path
            nextEntityInFrontOfPlayer =
                            this->mParentPlayer->mRace->mPath->GetWayPointLinkEntityItemInFrontOfPlayer(whichChargingStation->exitWayPointLink, true, this->mParentPlayer);

            //update value for B with advanced entity
            newLineStr->B = nextEntityInFrontOfPlayer->getCenter();
        }

        //set white as default color
        newLineStr->color = mParentPlayer->mRace->mDrawDebug->white;
        newLineStr->name = new char[10];
        strcpy(newLineStr->name, "");

        newStruct->pLineStruct = newLineStr;
        irr::core::vector3df vec3D = (newLineStr->B - newLineStr->A);

        //precalculate and store length as we will need this very often
        //during the game loop for race position update
        newStruct->length3D = vec3D.getLength();
        vec3D.normalize();

        newStruct->LinkDirectionVec = vec3D;
        newStruct->LinkDirectionVec.normalize();

        //Idea: extend the lines a little bit further outwards at
        //both ends, so that when we project the players position on
        //the different segments later we always find a valid segment
        LineStruct *lineExt = new LineStruct;

        lineExt->A = newLineStr->A - newStruct->LinkDirectionVec * 0.5f;
        lineExt->B = newLineStr->B + newStruct->LinkDirectionVec * 0.5f;

        lineExt->name = new char[100];
        sprintf(lineExt->name, "");

        //set white as default color
        lineExt->color = mParentPlayer->mRace->mDrawDebug->white;

        newStruct->pLineStructExtended = lineExt;

        //newStruct->pStartEntity = whichChargingStation->enterEntityItem;
        newStruct->pStartEntity = whichChargingStation->exitEntityItem;
        newStruct->pEndEntity = nextEntityInFrontOfPlayer;

        newStruct->minOffsetShiftEnd = -10.0f;
        newStruct->maxOffsetShiftEnd = 10.0f;
        newStruct->minOffsetShiftStart = -10.0f;
        newStruct->maxOffsetShiftStart = 10.0f;

       // mParentPlayer->mRace->dbgCoord = newStruct->pLineStruct->B;

        //precalculate a direction vector which stands at a 90 degree
        //angle at the original waypoint direction vector, and always points
        //to the right direction when looking into race direction
        //this direction vector is later used during the game to offset the player
        //path sideways
        newStruct->offsetDirVec = newStruct->LinkDirectionVec.crossProduct(-*mParentPlayer->mRace->yAxisDirVector).normalize();

        CheckAndRemoveNoCommand();

        CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
        newCmd->cmdType = CMD_EXIT_CHARGINGSTATION;

        //also store this temporary waypointlink struct info
        //in the command, so that we can cleanup after this command was
        //executed
        newCmd->targetWaypointLink = newStruct;

        //mark waypoint link as temporary created, so that
        //we delete it again after command was fully executed
        newCmd->WayPointLinkTemporary = true;

        //add the new command to the end of the command list
        this->cmdList->push_back(newCmd);

        mSetupPathToExitChargingStation = false;
}

void CpuPlayer::AddCommand(uint8_t cmdType, Collectable* whichCollectable) {
    CheckAndRemoveNoCommand();

    CPCOMMANDENTRY* newCmd = new CPCOMMANDENTRY();
    newCmd->cmdType = cmdType;
    newCmd->targetCollectible = whichCollectable;

    //add the new command to the end of the command list
    this->cmdList->push_back(newCmd);
}

void CpuPlayer::CheckAndRemoveNoCommand() {
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

void CpuPlayer::RemoveAllPendingCommands() {
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

                       LineStruct* pntrLineExtStruct = pntrCmd->targetWaypointLink->pLineStructExtended;

                       delete[] pntrLineExtStruct->name;
                       delete pntrLineExtStruct;

                       delete pntrCmd->targetWaypointLink;
                   }
               }
           }

           delete pntrCmd;
       }
    }

    currCommand = NULL;
}

bool CpuPlayer::DoIWantToChargeShield() {
     if (mParentPlayer->mPlayerStats->shieldVal < (0.4 * mParentPlayer->mPlayerStats->shieldMax))
         return true;

     return false;
}

bool CpuPlayer::DoIWantToChargeFuel() {
    if (mParentPlayer->mPlayerStats->gasolineVal < (0.5 * mParentPlayer->mPlayerStats->gasolineMax))
       return true;

    return false;
}

bool CpuPlayer::DoIWantToChargeAmmo() {
    if (mParentPlayer->mPlayerStats->ammoVal < (0.3 * mParentPlayer->mPlayerStats->ammoMax))
        return true;

    return false;
}

void CpuPlayer::DebugDraw() {

 /* mParentPlayer->mRace->mDrawDebug->Draw3DLine(this->mParentPlayer->mRace->topRaceTrackerPointerOrigin, debugPathPnt1, this->mParentPlayer->mRace->mDrawDebug->green);
  mParentPlayer->mRace->mDrawDebug->Draw3DLine(this->mParentPlayer->mRace->topRaceTrackerPointerOrigin, debugPathPnt2, this->mParentPlayer->mRace->mDrawDebug->red);
  mParentPlayer->mRace->mDrawDebug->Draw3DLine(this->mParentPlayer->mRace->topRaceTrackerPointerOrigin, debugPathPnt3, this->mParentPlayer->mRace->mDrawDebug->blue);*/

  if (mCurrentPathSeg.size() > 0) {
       std::vector<WayPointLinkInfoStruct*>::iterator itPathEl;

       for (itPathEl = mCurrentPathSeg.begin(); itPathEl != mCurrentPathSeg.end(); ++itPathEl) {
             mParentPlayer->mRace->mDrawDebug->Draw3DLine((*itPathEl)->pLineStruct->A, (*itPathEl)->pLineStruct->B, (*itPathEl)->pLineStruct->color);
        }
  }
}

WayPointLinkInfoStruct* CpuPlayer::CpPlayerWayPointLinkSelectionLogic(std::vector<WayPointLinkInfoStruct*> availLinks) {
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
    if (linkForShield != NULL) {
        //do we need shield? nothing more important!
        if (DoIWantToChargeShield()) {
            //get exact region info for charger
            ChargingStation* chargingStation =
                    this->mParentPlayer->mRace->mPath->GetChargingStationAhead(linkForShield, LEVELFILE_REGION_CHARGER_SHIELD);

            //make sure pointer is unequal to NULL!
            if (chargingStation != NULL) {

                    //request charging at charging station
                    //if request is granted we receive return value true, false otherwise
                    if (chargingStation->RequestCharging(this->mParentPlayer, mAssignedChargingStall)) {
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

                            mParentPlayer->LogMessage((char*)"Command to charge Shield");

                            //I need shield
                            AddCommand(CMD_GOTO_CHARGINGSTATION, chargingStation, mAssignedChargingStall);
                            mAssignedChargingStation = chargingStation;

                            //we want to accel/deaccelerate computer player craft
                            //now much quicker, so that if we reach the charging area
                            //finally we stop fast enough and do not overshoot the
                            //charging area itself
                            mCpCurrentDeaccelRate = CP_PLAYER_ACCELDEACCEL_RATE_CHARGING;
                            return (linkForShield);
                    } else {
                        //request for charging was not granted
                         mParentPlayer->LogMessage((char*)"Shield charging station did not grant charging request");
                    }
            }
        }
    }

    //do we have link to fuel?
    if (linkForFuel != NULL) {
        //do we need fuel?
        if (DoIWantToChargeFuel()) {
            //get exact region info for charger
            ChargingStation* chargingStation =
                    this->mParentPlayer->mRace->mPath->GetChargingStationAhead(linkForFuel, LEVELFILE_REGION_CHARGER_FUEL);

            //make sure pointer is unequal to NULL!
            if (chargingStation != NULL) {

                    //request charging at charging station
                    //if request is granted we receive return value true, false otherwise
                    if (chargingStation->RequestCharging(this->mParentPlayer, mAssignedChargingStall)) {

                            //please see important comment in shield section above
                            //why we do this
                            RemoveAllPendingCommands();

                            //I need fuel
                            AddCommand(CMD_GOTO_CHARGINGSTATION, chargingStation, mAssignedChargingStall);
                            mAssignedChargingStation = chargingStation;

                            mParentPlayer->LogMessage((char*)"Command to charge Fuel");

                            //we want to accel/deaccelerate computer player craft
                            //now much quicker, so that if we reach the charging area
                            //finally we stop fast enough and do not overshoot the
                            //charging area itself
                            mCpCurrentDeaccelRate = CP_PLAYER_ACCELDEACCEL_RATE_CHARGING;
                            return (linkForFuel);
                    } else {
                        //request for charging was not granted
                         mParentPlayer->LogMessage((char*)"Fuel charging station did not grant charging request");
                    }
            }
        }
    }

    //do we have link to ammo?
    if (linkForAmmo != NULL) {
        //do we need ammo?
        if (DoIWantToChargeAmmo()) {
            //get exact region info for charger
            ChargingStation* chargingStation =
                    this->mParentPlayer->mRace->mPath->GetChargingStationAhead(linkForAmmo, LEVELFILE_REGION_CHARGER_AMMO);

            //make sure pointer is unequal to NULL!
            if (chargingStation != NULL) {

                    //request charging at charging station
                    //if request is granted we receive return value true, false otherwise
                    if (chargingStation->RequestCharging(this->mParentPlayer, mAssignedChargingStall)) {

                        //please see important comment in shield section above
                        //why we do this
                        RemoveAllPendingCommands();

                        //I need ammo
                        AddCommand(CMD_GOTO_CHARGINGSTATION, chargingStation, mAssignedChargingStall);
                        mAssignedChargingStation = chargingStation;

                        mParentPlayer->LogMessage((char*)"Command to charge Ammo");

                        //we want to accel/deaccelerate computer player craft
                        //now much quicker, so that if we reach the charging area
                        //finally we stop fast enough and do not overshoot the
                        //charging area itself
                        mCpCurrentDeaccelRate = CP_PLAYER_ACCELDEACCEL_RATE_CHARGING;
                        return (linkForAmmo);
                   } else {
                        //request for charging was not granted
                         mParentPlayer->LogMessage((char*)"Ammo charging station did not grant charging request");
                    }
            }
        }
    }

    //if there is only one waypoint link available
    //just return this one
    int nrWays = (int)(linksNothingSpecial.size());
    if (nrWays > 0) {

        //choose available path random
        int rNum;
        rNum = mParentPlayer->mInfra->randRangeInt(0, nrWays - 1);

        //LogMessage((char*)"Entered a new (unspecial) Waypoint-Link");

        return linksNothingSpecial.at(rNum);
    }

    //if we do not want to charge anything (we are full),
    //and the only available way is through a charger,
    //at the end select the way through the charger!

    //we still have no way to go
    if (linkForFuel) {
      mParentPlayer->LogMessage((char*)"Only available WayPointLink goes through fuel charger");
      return (linkForFuel);
    }

    if (linkForAmmo) {
      mParentPlayer->LogMessage((char*)"Only available WayPointLink goes through ammo charger");
      return (linkForAmmo);
    }

    if (linkForShield) {
      mParentPlayer->LogMessage((char*)"Only available WayPointLink goes through shield charger");
      return (linkForShield);
    }

    return NULL;
}

bool CpuPlayer::DoISeeACertainCollectable(Collectable* whichItem) {
    std::vector<Collectable*>::iterator it;

    for (it = mParentPlayer->mCollectablesSeenByPlayer.begin(); it != mParentPlayer->mCollectablesSeenByPlayer.end(); ++it) {
        if ((*it) == whichItem)
            return true;
    }

    return false;
}

void CpuPlayer::CpPlayerCollectableSelectionLogic() {
    //does this player want to pickup a collectable right now?
    if (mCpTargetCollectableToPickUp != NULL) {
        //verify that the player still sees the collectable in his view region, if not remove
        //current command again, and return to normal path
        if (!DoISeeACertainCollectable(mCpTargetCollectableToPickUp)) {
            //I do not see it anymore
            //change back to normal race path

            mParentPlayer->LogMessage((char*)"The collectable I wanted to pickup is not visible anymore, continue race");

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

            for (it = mParentPlayer->mCollectablesSeenByPlayer.begin(); (it != mParentPlayer->mCollectablesSeenByPlayer.end() && (wantPickup == NULL)); ++it) {
                if ((*it)->GetIfVisible()) {
                    switch ((*it)->GetCollectableType()) {
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
                            if (mParentPlayer->mPlayerStats->currMinigunUpgradeLevel < 3) {
                                wantPickup = (*it);
                            }

                            break;
                        }

                        case Entity::EntityType::MissileUpgrade: {
                            //we only want to pick this up if missile is not
                            //already at highest level
                            if (mParentPlayer->mPlayerStats->currRocketUpgradeLevel < 3) {
                                wantPickup = (*it);
                            }
                            break;
                        }

                        case Entity::EntityType::BoosterUpgrade: {
                            //we only want to pick this up if booster is not
                            //already at highest level
                            if (mParentPlayer->mPlayerStats->currBoosterUpgradeLevel < 3) {
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
                mParentPlayer->LogMessage((char*)"AddCommand: Pick colletable up");

                //we found something we want to have
                AddCommand(CMD_PICKUP_COLLECTABLE, wantPickup);
            }
        }
    }

    //we do not want to pickup anything
}

void CpuPlayer::CPForceController(irr::f32 deltaTime) {

    mLastCraftDistToWaypointLink = mCurrentCraftDistToWaypointLink;

    //if our player can not move currently come to a stop
    if (!mParentPlayer->mPlayerStats->mPlayerCanMove) {
        computerPlayerTargetSpeed = 0.0f;
    }

    //29.04.2025: for state change we need to take into accont
    //the current frame rate! otherwise the state changes
    //depend heavily on the frame rate of the computer!
    //all values below were defined at constant 60FPS at
    //my development computer
    irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

    if ((computerPlayerTargetSpeed + mTargetSpeedAdjust) > computerPlayerCurrentSpeed) {
        computerPlayerCurrentSpeed += mCpCurrentAccelRate * speedFactor;
    } else if (computerPlayerCurrentSpeed > (computerPlayerTargetSpeed - mTargetSpeedAdjust)) {
        computerPlayerCurrentSpeed -= mCpCurrentDeaccelRate * speedFactor;
    }

    //is there a craft very close in front of us, then go slower
    if (mParentPlayer->mCraftDistanceAvailFront < 5.0f) {
        mTargetSpeedAdjust -= 0.2f * speedFactor;

        if (mTargetSpeedAdjust < 0.0f) {
            mTargetSpeedAdjust = 0.0f;
        }
    } else {
        if (mTargetSpeedAdjust < 0.0f) {
            mTargetSpeedAdjust += 0.2f * speedFactor;

            if (mTargetSpeedAdjust > 0.0f)
            {
                mTargetSpeedAdjust = 0.0f;
            }
        }
    }

    //control computer player speed
    if (mParentPlayer->phobj->physicState.speed < (computerPlayerCurrentSpeed * 0.9f))
    {
        //go faster
        this->CPForward(deltaTime);
    } else if (mParentPlayer->phobj->physicState.speed > (computerPlayerCurrentSpeed * 1.1f)) {
        //go slower
       this->CPBackward(deltaTime);
    }

    //if we are not allowed to move right now, exit here
    //we do not need to steer
    if (!mParentPlayer->mPlayerStats->mPlayerCanMove)
        return;

    if (this->cPCurrentFollowSeg != NULL) {
        //we need to project current computer player craft
        //position onto the current segment we follow
        //this recalculates the current projPlayerPositionClosestWayPointLink member
        //variable for the further calculation steps below
        CPTrackMovement();

        //sometimes we loose our way inside ProjectPlayerAtCurrentSegments,
        //if so just exit, next time we should have a new way again
        if (this->cPCurrentFollowSeg == NULL) {
           this->computerPlayerTargetSpeed = 0.0f;
            mParentPlayer->LogMessage((char*)"CPForceController: lost our segment to follow in ProjectPlayerAtCurrentSegments");

            //this->mParentPlayer->mRace->mGame->StopTime();
            return;
        }

        irr::core::vector3df dirVecToLink = (this->projPlayerPositionFollowSeg - mParentPlayer->phobj->physicState.position);
        dirVecToLink.Y = 0.0f;

        irr::core::vector3df currDirVecCraftSide = mParentPlayer->craftSidewaysToRightVec;
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

        mParentPlayer->mCurrentCraftOrientationAngle =
                mParentPlayer->mRace->GetAbsOrientationAngleFromDirectionVec(mParentPlayer->craftForwardDirVec);

        //UpdateCurrentCraftOrientationAngleAvg();

         irr::f32 angleDotProduct = this->cPCurrentFollowSeg->LinkDirectionVec.dotProduct(mParentPlayer->craftForwardDirVec);

        irr::f32 angleRad = acosf(angleDotProduct);
        mAngleError = (angleRad / irr::core::PI) * 180.0f;

        if (mParentPlayer->craftSidewaysToRightVec.dotProduct(this->cPCurrentFollowSeg->LinkDirectionVec) > 0.0f) {
            mAngleError = -mAngleError;
        }

        irr::f32 currDistanceChangeRate = mCurrentCraftDistToWaypointLink - mLastCraftDistToWaypointLink;

        /***************************************/
        /*  Control Craft absolute angle start */
        /***************************************/

        irr::f32 corrForceOrientationAngle = 500.0f;
        irr::f32 corrDampingOrientationAngle = 50.0f;

        irr::f32 angleVelocityCraftX = mParentPlayer->phobj->GetVelocityLocalCoordPoint(mParentPlayer->LocalCraftForceCntrlPnt).X * corrDampingOrientationAngle;

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
            mParentPlayer->phobj->mRotationalFrictionVal = CP_PLAYER_ANGULAR_DAMPINGMAX;
        } else if (absAngleError > CP_PLAYER_ANGULAR_DAMPING_ANGLEMAX) {
            mParentPlayer->phobj->mRotationalFrictionVal = CP_PLAYER_ANGULAR_DAMPINGMIN;
        } else {
            //interpolate between max and min value depending on the angle error
            mDbgRotationalFrictionVal = CP_PLAYER_ANGULAR_DAMPINGMAX -
                    ((CP_PLAYER_ANGULAR_DAMPINGMAX - CP_PLAYER_ANGULAR_DAMPINGMIN) / (CP_PLAYER_ANGULAR_DAMPING_ANGLEMAX - CP_PLAYER_ANGULAR_DAMPING_ANGLEMIN)) * absAngleError;

             mParentPlayer->phobj->mRotationalFrictionVal = mDbgRotationalFrictionVal;
        }

         mDbgForceAngle = corrForceAngle;
         mDbgAngleVelocityCraftZ = mParentPlayer->phobj->GetVelocityLocalCoordPoint(mParentPlayer->LocalCraftForceCntrlPnt).Z;
         mDbgAngleVelocityCraftX = angleVelocityCraftX;

         mParentPlayer->phobj->AddLocalCoordForce(mParentPlayer->LocalCraftForceCntrlPnt, irr::core::vector3df(corrForceAngle, 0.0f, 0.0f),
                                              PHYSIC_APPLYFORCE_ONLYROT);

        /****************************************************/
        /*  Control Craft distance to current waypoint link */
        /****************************************************/

        irr::f32 corrForceDist = 100.0f;
        irr::f32 corrDampingDist = 2000.0f;

        irr::f32 distError = (mCurrentCraftDistToWaypointLink - mLocalOffset);

        dbgDistError = distError;

        //best line until 30.12.2024
        irr::f32 corrForceDistance = distError * corrForceDist + currDistanceChangeRate * corrDampingDist;

        if (corrForceDistance > 100.0f) {
             corrForceDistance = 100.0f;
         } else if (corrForceDistance < -100.0f) {
             corrForceDistance = -100.0f;
           }

        mDbgFoceDistance = corrForceDistance;

        mParentPlayer->phobj->AddLocalCoordForce(mParentPlayer->LocalCraftOrigin, irr::core::vector3df(corrForceDistance, 0.0f, 0.0f),
                                   PHYSIC_APPLYFORCE_ONLYTRANS);

    } else {
        //no segment to follow, stop craft
        this->computerPlayerTargetSpeed = 0.0f;
        mParentPlayer->LogMessage((char*)"CPForceController: No segement to follow, stop craft");

        //this->mParentPlayer->mRace->mGame->StopTime();
    }
}

void CpuPlayer::CPForward(irr::f32 deltaTime) {
    //if player can not move right now simply
    //exit
    if (!mParentPlayer->mPlayerStats->mPlayerCanMove)
        return;

    //29.04.2025: for throttleVal calculation we need to take into accont
    //the current frame rate! otherwise the throttle change speed
    //depends heavily on the frame rate of the computer!
    irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

        //to accelerate player add force in craft forward direction
        mParentPlayer->phobj->AddLocalCoordForce(mParentPlayer->LocalCraftOrigin, irr::core::vector3df(0.0f, 0.0f, -50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

        if (mParentPlayer->mPlayerStats->throttleVal < mParentPlayer->mPlayerStats->throttleMax) {
            //+1.0f is for constant 60FPS
            mParentPlayer->mPlayerStats->throttleVal += 1.0f * speedFactor;
        }
}

void CpuPlayer::CPBackward(irr::f32 deltaTime) {
    //29.04.2025: for throttleVal calculation we need to take into accont
    //the current frame rate! otherwise the throttle change speed
    //depends heavily on the frame rate of the computer!
    irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

    if (!DEF_PLAYERCANGOBACKWARDS) {
        //we can not go backwards in Hioctane
        //we can only add friction to brake
        mParentPlayer->phobj->AddFriction(1000.0f);

        if (mParentPlayer->mPlayerStats->throttleVal > 0) {
            //-1.0f is for constant 60FPS
            mParentPlayer->mPlayerStats->throttleVal -= 1.0f * speedFactor;
        }
    } else {
        //if player can not move right now simply
        //exit
        if (!mParentPlayer->mPlayerStats->mPlayerCanMove)
            return;

            //go solution during debugging, for example testing collisions, it helps to be able to accelerate backwards
            mParentPlayer->phobj->AddLocalCoordForce(mParentPlayer->LocalCraftOrigin, irr::core::vector3df(0.0f, 0.0f, 50.0f), PHYSIC_APPLYFORCE_REAL,
                                    PHYSIC_DBG_FORCETYPE_ACCELBRAKE);

            if (mParentPlayer->mPlayerStats->throttleVal > 0) {
                //-1.0f is for constant 60FPS
                mParentPlayer->mPlayerStats->throttleVal -= 1.0f * speedFactor;
            }
    }
}

void CpuPlayer::CPTrackMovement() {
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

    mFailedLinks.clear();

    irr::core::vector3df distVec;
    irr::core::vector3df dirVec2D;

    //iterate through all player follow way segments
    for(WayPointLink_iterator = mCurrentPathSeg.begin(); WayPointLink_iterator != mCurrentPathSeg.end(); ++WayPointLink_iterator) {

        //for the workaround later (in case first waypoint link search does not work) also find in parallel the waypoint link that
        //has a start or end-point closest to the current player location
        posH = (*WayPointLink_iterator)->pLineStruct->A;

        distVec = mParentPlayer->phobj->physicState.position - posH;
        distVec.Y = 0.0f;

        startPointDistHlper = distVec.getLengthSQ();

        posH = (*WayPointLink_iterator)->pLineStruct->B;
        distVec = mParentPlayer->phobj->physicState.position - posH;
        distVec.Y = 0.0f;

        endPointDistHlper = distVec.getLengthSQ();

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
        dA = mParentPlayer->phobj->physicState.position - (*WayPointLink_iterator)->pLineStructExtended->A;
        dA.Y = 0.0f;

        dB = mParentPlayer->phobj->physicState.position - (*WayPointLink_iterator)->pLineStructExtended->B;
        dB.Y = 0.0f;

        dirVec2D = (*WayPointLink_iterator)->LinkDirectionVec;

        projecteddA = dA.dotProduct(dirVec2D);
        projecteddB = dB.dotProduct(dirVec2D);

        //if craft position is parallel (sideways) to current waypoint link the two projection
        //results need to have opposite sign; otherwise we are not sideways of this line, and need to ignore
        //this path segment
        if (sgn(projecteddA) != sgn(projecteddB)) {
            //this waypoint is interesting for further analysis
            //calculate distance from player position to this line, where connecting line meets path segment
            //in a 90° angle
            projectedPl =  dA.dotProduct(dirVec2D);

            /*
            (*WayPointLink_iterator)->pLineStruct->debugLine = new irr::core::line3df((*WayPointLink_iterator)->pLineStruct->A +
                                                                                      projectedPl * (*WayPointLink_iterator)->LinkDirectionVec,
                                                                                      player->phobj->physicState.position);*/

            projPlayerPosition = (*WayPointLink_iterator)->pLineStructExtended->A +
                    irr::core::vector3df(projectedPl, projectedPl, projectedPl) * (dirVec2D);

            distance = (projPlayerPosition - mParentPlayer->phobj->physicState.position).getLength();

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
                   // mLastPathSegElementFollowing = closestLink;
                } else {
                    if (distance < minDistance) {
                        minDistance = distance;
                        closestLink = (*WayPointLink_iterator);
                        projPlayerPositionFollowSeg = projPlayerPosition;
                     //   mLastPathSegElementFollowing = closestLink;
                     }
                  }
          }
        } else {
            mFailedLinks.push_back(*WayPointLink_iterator);
        }
    }

    //did we still not find the closest link? try some workaround
    if (closestLink == NULL) {
        mCPTrackMovementNoClearClosestLinkCnter++;
       // closestLink = mLastPathSegElementFollowing;

       // LogMessage((char*)"CPTrackMovement: Reuse last followed Element!");

       //workaround, take the waypoint with either the closest
       //start or end entity
       if (LinkWithClosestStartEndPoint != NULL) {
         closestLink = LinkWithClosestStartEndPoint;
         mParentPlayer->LogMessage((char*)"CPTrackMovement: Workaround closest StartEndPoint");
         //this->mParentPlayer->mRace->mGame->StopTime();
       }
    }

    /*if ((closestLink != NULL) && (whichPlayer == player)) {
        closestLink->pLineStruct->color = mDrawDebug->green;
    }*/

/*    if ((LinkWithClosestStartEndPoint != NULL) && (whichPlayer == player)) {
       LinkWithClosestStartEndPoint->pLineStruct->color = mDrawDebug->red;
    }*/

    cPCurrentFollowSeg = closestLink;

   if (cPCurrentFollowSeg == NULL) {
         mParentPlayer->LogMessage((char*)"CPTrackMovement: cPCurrentFollowSeg is NULL, Workaround");
         //this->mParentPlayer->mRace->mGame->StopTime();

         WorkaroundResetCurrentPath();

         return;
   }

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
       mCPTrackMovementLostProgressCnter++;

       //to fix this situation, completely renew the current overall path
       //WorkaroundResetCurrentPath();
       return;
   } else {
           //have we reached the end of the following path we follow?
           //if (mCurrentPathSegCurrSegmentNr >= (mCurrentPathSegNrSegments -1)) {
           if (mCurrentPathSegCurrSegmentNr >= (mCurrentPathSegNrSegments - 2)) {
              ReachedEndCurrentFollowingSegments();
           }
   }
}

void CpuPlayer::ReachedEndCurrentFollowingSegments() {
  //  if (this->currClosestWayPointLink.first != NULL) {

        //which waypoint options do we have at the end of
        //our closest waypoint link?
        irr::core::vector3df endPointLink;

        //endPointLink = this->currClosestWayPointLink.first->pEndEntity->get_Pos();
        endPointLink = this->mCpFollowThisWayPointLink->pEndEntity->getCenter();

        std::vector<EntityItem*> availWaypoints =
              mParentPlayer->mRace->mPath->FindAllWayPointsInArea(endPointLink, 2.0f);

        mDbgCpAvailWaypointNr = (irr::s32)(availWaypoints.size());

        if (mDbgCpAvailWaypointNr > 0) {
             std::vector<EntityItem*>::iterator it;
             mCpAvailWayPointLinks.clear();

              std::vector<WayPointLinkInfoStruct*> fndLinks;
              std::vector<WayPointLinkInfoStruct*>::iterator it2;

             for (it = availWaypoints.begin(); it != availWaypoints.end(); ++it) {
                 //if ((*it) != this->currClosestWayPointLink.first->pEndEntity) {
                     //important: we need to exclude the waypoint link from the search we
                     //came from! otherwise we can loop around to the link we came from (mCpFollowThisWayPointLink)
                     fndLinks = mParentPlayer->mRace->mPath->FindWaypointLinksForWayPoint((*it), true, false,
                                this->mCpFollowThisWayPointLink);

                     if (fndLinks.size() > 0) {
                         for (it2 = fndLinks.begin(); it2 != fndLinks.end(); ++it2) {
                            mCpAvailWayPointLinks.push_back(*it2);
                         }
                     }
                // }
             }

             mDbgCpAvailWayPointLinksNr = (irr::s32)(mCpAvailWayPointLinks.size());
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

void CpuPlayer::CpStuckDetection(irr::f32 deltaTime) {
    //for a human player simply return
    if (mParentPlayer->mHumanPlayer)
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
            if (mParentPlayer->IsCurrentlyCharging()) {
                mCpPlayerTimeNotMovedSeconds = 0.0f;
                mCpPlayerLastPosition = mParentPlayer->phobj->physicState.position;
                return;
            }

            //if computer player has finished race already also
            //it is not stuck, or if its fuel is empty
            //to refuel fuel there is another logic in place, we do not
            //want to interrupt here
            //in both cases the player is not allowed to move
            if (!mParentPlayer->mPlayerStats->mPlayerCanMove){
               mCpPlayerTimeNotMovedSeconds = 0.0f;
               mCpPlayerLastPosition = mParentPlayer->phobj->physicState.position;
               return;
           }

            //if we have already called a recovery vehicle also leave
            if (mParentPlayer->mRecoveryVehicleCalled || mCpPlayerCurrentlyStuck) {
                mCpPlayerTimeNotMovedSeconds = 0.0f;
                mCpPlayerLastPosition = mParentPlayer->phobj->physicState.position;
                return;
           }

            //next line only for debugging
            //dbgStuckDet = (this->phobj->physicState.position - mCpPlayerLastPosition).getLengthSQ();

            //check if we have not moved since last frame
            if ((mParentPlayer->phobj->physicState.position - mCpPlayerLastPosition).getLengthSQ() < CP_PLAYER_STUCKDETECTION_MINDISTANCE_LIMIT) {
                this->mCpPlayerTimeNotMovedSeconds += addTime;

                //stuck for a long enough time?
                if (this->mCpPlayerTimeNotMovedSeconds > CP_PLAYER_STUCKDETECTION_THRESHOLD_SEC) {
                    //we seem to be stuck
                    //for a workaround call the Recovery vehicle to
                    //put us back properly at the next waypoint link at the track

                    //for this we want to know at which side we are most likely stuck with
                    //the terrain
                    //take the side where we have the least amount of free space to move around
                    bool spaceTightRightSide = (mParentPlayer->mCraftDistanceAvailRight < 1.5f);
                    bool spaceTightLeftSide = (mParentPlayer->mCraftDistanceAvailLeft < 1.5f);

                    mCpPlayerStuckAtSide = CP_PLAYER_WAS_STUCKUNDEFINED;

                    if (spaceTightLeftSide && !spaceTightRightSide) {
                        mCpPlayerStuckAtSide = CP_PLAYER_WAS_STUCKLEFTSIDE;
                    }

                    if (!spaceTightLeftSide && spaceTightRightSide) {
                        mCpPlayerStuckAtSide = CP_PLAYER_WAS_STUCKRIGHTSIDE;
                    }

                    if (spaceTightLeftSide && spaceTightRightSide) {
                        if (mParentPlayer->mCraftDistanceAvailLeft < mParentPlayer->mCraftDistanceAvailRight) {
                            mCpPlayerStuckAtSide = CP_PLAYER_WAS_STUCKLEFTSIDE;
                        } else {
                            mCpPlayerStuckAtSide = CP_PLAYER_WAS_STUCKRIGHTSIDE;
                        }
                    }

                    mParentPlayer->LogMessage((char*)"I am stuck, I call recovery vehicle for help");
                    mParentPlayer->mRace->CallRecoveryVehicleForHelp(mParentPlayer);
                    mParentPlayer->mRecoveryVehicleCalled = true;
                    mCpPlayerCurrentlyStuck = true;

                    mCpPlayerTimeNotMovedSeconds = 0.0f;
                }
            } else {
                //we have move enough, reset time counter again
                this->mCpPlayerTimeNotMovedSeconds = 0.0f;
            }

            mCpPlayerLastPosition = mParentPlayer->phobj->physicState.position;
    }
}

void CpuPlayer::AdvanceDbgColor() {
    if (currDbgColor == mParentPlayer->mRace->mDrawDebug->red)  {
        currDbgColor = mParentPlayer->mRace->mDrawDebug->cyan;
    } else if (currDbgColor == mParentPlayer->mRace->mDrawDebug->cyan)  {
        currDbgColor = mParentPlayer->mRace->mDrawDebug->pink;
    } else if (currDbgColor == mParentPlayer->mRace->mDrawDebug->pink)  {
        currDbgColor = mParentPlayer->mRace->mDrawDebug->green;
    } else if (currDbgColor == mParentPlayer->mRace->mDrawDebug->green)  {
        currDbgColor = mParentPlayer->mRace->mDrawDebug->red;
    }
}

void CpuPlayer::PickupCollectableDefineNextSegment(Collectable* whichCollectable) {
    //create bezier curve
    //start point is the current end point of the path
    //control point is the start point of the link
    //in the path with the specified number
    //end point is the end point of the link in the
    //path with the defined number
    irr::core::vector3df link1Start3D;
    irr::core::vector3df link1End3D;

    //current player position, is start point for bezier curve 1
    irr::core::vector2df bezierPnt1 = mParentPlayer->GetMyBezierCurvePlaningCoord(debugPathPnt1);

    //collectable => is the control point for bezier curve 1
    irr::core::vector2df bezierCntrlPnt1 = whichCollectable->GetMyBezierCurvePlaningCoord(debugPathPnt2);

    //end point for next link is needed to calculate midpoint
    //irr::core::vector2df link1End = nextLink->pEndEntity->GetMyBezierCurvePlaningCoord(link1End3D);

    //take endpoint of link closest to collectable, is the bezier curve 1 end point
    irr::core::vector2df bezierPnt2 =  mCpWayPointLinkClosestToCollectable->pEndEntity->GetMyBezierCurvePlaningCoord(debugPathPnt3);

    std::vector<WayPointLinkInfoStruct*> newPoints;
    newPoints.clear();

    this->AdvanceDbgColor();

    newPoints = mParentPlayer->mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, bezierPnt2, bezierCntrlPnt1,
                                                                    CP_BEZIER_RESOLUTION, currDbgColor);

    std::vector<WayPointLinkInfoStruct*>::iterator it;

    mCurrentPathSeg.clear();

    //add new waypoints to the existing ones
    for (it = newPoints.begin(); it != newPoints.end(); ++it) {
        mCurrentPathSeg.push_back(*it);
    }

    mCurrentPathSegNrSegments = (irr::u32)(mCurrentPathSeg.size());

    CPTrackMovement();

    //this->mRace->mGame->StopTime();
}

//after a computer player is freed from the recovery vehicle, our under
//some error cases where we could lose our current path progress we need
//to try to restart with a new clean path/path status, this is done by this routine
void CpuPlayer::WorkaroundResetCurrentPath() {

    mParentPlayer->LogMessage((char*)"WorkaroundResetCurrentPath was called");

    //which waypoint link is the closest one to us right now?
   std::vector<std::pair <WayPointLinkInfoStruct*, irr::core::vector3df>> closeWaypointLinks =
           mParentPlayer->mRace->mPath->PlayerFindCloseWaypointLinks(mParentPlayer);

   std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> closestLink =
           mParentPlayer->mRace->mPath->PlayerDeriveClosestWaypointLink(closeWaypointLinks);

   if (closestLink.first == NULL) {
       //workaround, just do nothing! TODO: Maybe improve later
       mParentPlayer->LogMessage((char*)"WorkaroundResetCurrentPath: no closest link found");
   } else {
       //setup new path for the race
       AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, closestLink.first);
   }

  computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
}

/*  was just an attempt at 06.06.2025, commented out right now again
//after a computer player is freed from the recovery vehicle, our under
//some error cases where we could lose our current path progress we need
//to try to restart with a new clean path/path status, this is done by this routine
void CpuPlayer::WorkaroundResetCurrentPath() {

    mParentPlayer->LogMessage((char*)"WorkaroundResetCurrentPath was called");

    //which waypoint is closest to me?
    EntityItem* closestWayPoint = this->mParentPlayer->mRace->mPath->FindNearestWayPointToPlayer(this->mParentPlayer);

   if (closestWayPoint == NULL) {
       //workaround, just do nothing! TODO: Maybe improve later
       mParentPlayer->LogMessage((char*)"WorkaroundResetCurrentPath: no closest waypoint found!");
   } else {
       //setup new path for the race
       AddCommand(CMD_FLYTO_TARGETENTITY, closestWayPoint);
   }

  computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
}*/

void CpuPlayer::CpCheckCurrentPathForObstacles() {
    bool updatePath = false;

    //Note 18.01.2025: This function causes me more problems right now as it solves
    //I am not sure if I will use it at the end. I will leave the computer players like the
    //are right now, because I am already working on this stuff for at least several months
    //I am getting really bored now by it
    return;

        irr::f32 missingSpace;

        //first update depending on the waypoint link we follow right now, and the players
        //current progress on this waypoint link in terms of location between start and end entity
        //how much free (movement) space we have to the right and left side of the freely moveable area
        //on the race track right now
        if (currClosestWayPointLink.first == NULL)
            return;

        irr::f32 progress = (currClosestWayPointLink.second - currClosestWayPointLink.first->pLineStruct->A).getLength() /
                currClosestWayPointLink.first->length3D;

        if (progress < 0.0f)
            progress = 0.0f;

        if (progress > 1.0f)
            progress = 1.0f;

        this->mCpFollowedWayPointLinkCurrentSpaceLeftSide = currClosestWayPointLink.first->minOffsetShiftStart +
                (currClosestWayPointLink.first->minOffsetShiftEnd - currClosestWayPointLink.first->minOffsetShiftStart) * progress;

        this->mCpFollowedWayPointLinkCurrentSpaceRightSide = currClosestWayPointLink.first->maxOffsetShiftStart +
                (currClosestWayPointLink.first->maxOffsetShiftEnd - currClosestWayPointLink.first->maxOffsetShiftStart) * progress;

        //are we too close to the race track edge / available space runs out?
        /*if (this->mCraftDistanceAvailLeft < 1.0f) {
            //go more right
            //missingSpace = (1.5f - this->mCraftDistanceAvailLeft);
            missingSpace = 0.5f;

            if (mCpCurrPathOffset < (mCpFollowedWayPointLinkCurrentSpaceRightSide - 0.5f - missingSpace)) {
               // if (!(mCpCurrPathOffset + missingSpace > (0.5f))) {
                    mCpCurrPathOffset += missingSpace;
                    updatePath = true;
                }
            //}
        }

        if (this->mCraftDistanceAvailRight < 1.0f) {
            //go more left
            //missingSpace = (1.5f - this->mCraftDistanceAvailRight);
             missingSpace = 0.5f;
            if (mCpCurrPathOffset > (mCpFollowedWayPointLinkCurrentSpaceLeftSide + 0.5f + missingSpace)) {
            //     if (!(mCpCurrPathOffset - missingSpace < (-0.5f))) {
                   mCpCurrPathOffset -= missingSpace;
                   updatePath = true;
                 }
           // }
        }*/

        //are we too close to the race track edge / available space runs out?
        if (-mCpFollowedWayPointLinkCurrentSpaceLeftSide + mCpCurrPathOffset < 1.0f) {
            //go more right
            //missingSpace = (1.5f - this->mCraftDistanceAvailLeft);
            missingSpace = 0.25f;

            if (mCpCurrPathOffset < (mCpFollowedWayPointLinkCurrentSpaceRightSide - 0.5f - missingSpace)) {
               // if (!(mCpCurrPathOffset + missingSpace > (0.5f))) {
                    mCpCurrPathOffset += missingSpace;
                    updatePath = true;
                }
            //}
        }

        if (mCpFollowedWayPointLinkCurrentSpaceRightSide - mCpCurrPathOffset < 1.0f) {
            //go more left
            //missingSpace = (1.5f - this->mCraftDistanceAvailRight);
             missingSpace = 0.25f;
            if (mCpCurrPathOffset > (mCpFollowedWayPointLinkCurrentSpaceLeftSide + 0.5f + missingSpace)) {
            //     if (!(mCpCurrPathOffset - missingSpace < (-0.5f))) {
                   mCpCurrPathOffset -= missingSpace;
                   updatePath = true;
                 }
           // }
        }

        //only check possible collision with players we do actually see in front of us
        //otherwise we would report an possible collision with our path, when the other
        //player comes close to our current path behind us
        //and we do not want to detect this
        std::vector<Player*> playerISee;
        std::vector<RayHitInfoStruct>::iterator it;

        playerISee.clear();
        for (it = mParentPlayer->PlayerSeenList.begin(); it != mParentPlayer->PlayerSeenList.end(); ++it) {
            if ((*it).HitType == RAY_HIT_PLAYER) {
                playerISee.push_back((*it).HitPlayerPntr);
            }
        }
        if (playerISee.size() > 0) {
            if (!mParentPlayer->mRace->mPath->DoesPathComeTooCloseToAnyOtherPlayer(
                        this->mCurrentPathSeg, playerISee, mParentPlayer->dbgPlayerInMyWay)) {
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

void CpuPlayer::FollowPathDefineNextSegment(WayPointLinkInfoStruct* nextLink, irr::f32 startOffsetWay,
                                         bool updatePathReachedEndWayPointLink) {

    bool freeWayFound = false;
    irr::f32 currOffset = startOffsetWay;

    std::vector<WayPointLinkInfoStruct*> newPoints;
    newPoints.clear();

    this->AdvanceDbgColor();

    irr::f32 freeSpaceLeftOfPath;
    irr::f32 freeSpaceRightOfPath;

    if (updatePathReachedEndWayPointLink) {
        //in which direction should we shift the new way to clear target?
        //in which direction do we have more free space available?
        freeSpaceLeftOfPath = currOffset - nextLink->minOffsetShiftStart;
        freeSpaceRightOfPath = nextLink->maxOffsetShiftStart - currOffset;

        //limit amount of space available, so that we do not completely
        //change the path if there is a wide open area in front of us
        if (freeSpaceRightOfPath > 10.0f)
            freeSpaceRightOfPath = 10.0f;

        if (freeSpaceLeftOfPath > 10.0f)
            freeSpaceLeftOfPath = 10.0f;

        //do we need to change offset, because in front of us there is not enough
        //space available?
        if (freeSpaceLeftOfPath < 1.0f) {
        //if (freeSpaceLeftOfPath < 2.0f) {
            //currOffset = nextLink->minOffsetShift * 0.35f;
            currOffset = currOffset + 0.35f * freeSpaceRightOfPath;
        }

        if (freeSpaceRightOfPath < 1.0f) {
        //if (freeSpaceRightOfPath < 2.0f) {
            //currOffset = nextLink->maxOffsetShift * 0.35f;
             currOffset = currOffset - 0.35f * freeSpaceLeftOfPath;
        }
    }

    //looking from the existing level design of things
    //the safe range (without collision with terrains and getting stuck)
    //seems to be arround -1.0 to 1.0f for currOffset
    if (currOffset > 1.0f) {
        currOffset = 1.0f;
    }

    if (currOffset < -1.0f) {
        currOffset = -1.0f;
    }

    //freeSpaceLeftOfPath = currOffset - nextLink->minOffsetShiftStart;
    //freeSpaceRightOfPath = nextLink->maxOffsetShiftStart - currOffset;

    freeSpaceLeftOfPath = currOffset - this->mCpFollowedWayPointLinkCurrentSpaceLeftSide;
    freeSpaceRightOfPath = nextLink->maxOffsetShiftStart - currOffset;

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
    irr::core::vector2df raceDirection = mParentPlayer->mRace->mPath->WayPointLinkGetRaceDirection2D(nextLink);

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
            irr::core::vector2df bezierPnt1 = mParentPlayer->GetMyBezierCurvePlaningCoord(debugPathPnt1);

            //next link start point => is the control point for bezier curve 1
            irr::core::vector2df bezierCntrlPnt1 = nextLink->pStartEntity->GetMyBezierCurvePlaningCoord(link1Start3D);
            mParentPlayer->mRace->mPath->OffsetWayPointLinkCoordByOffset(bezierCntrlPnt1, link1Start3D, nextLink, currOffset);

            debugPathPnt2 = link1Start3D;

            //end point for next link is needed to calculate midpoint
            irr::core::vector2df link1End = nextLink->pEndEntity->GetMyBezierCurvePlaningCoord(link1End3D);
            mParentPlayer->mRace->mPath->OffsetWayPointLinkCoordByOffset(link1End, link1End3D, nextLink, currOffset);

            //calculate midpoint for next link, is the bezier curve 1 end point
            irr::core::vector2df bezierPnt2 = mParentPlayer->mRace->testBezier->GetBezierCurvePlaningCoordMidPoint(link1Start3D, link1End3D, debugPathPnt3);

            if (mParentPlayer->mRace->mPath->SaniCheckBezierInputPoints(bezierPnt1,bezierCntrlPnt1, bezierPnt2, raceDirection)) {
                 newPoints = mParentPlayer->mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, bezierPnt2, bezierCntrlPnt1,
                                                                                 CP_BEZIER_RESOLUTION, currDbgColor);
             } else {
                //default does not work, could be that next link is the link at which we are currently located
                //try different order
                if (mParentPlayer->mRace->mPath->SaniCheckBezierInputPoints(bezierPnt1,bezierPnt2, link1End, raceDirection)) {
                    //other order works
                    newPoints = mParentPlayer->mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, link1End, bezierPnt2,
                                                                                    CP_BEZIER_RESOLUTION, currDbgColor);

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
                            mParentPlayer->mRace->mPath->OffsetWayPointLinkCoordByOffset(bezierPntNextLinkStart, linkAfterwardsStart3D, pntrLinkAfterwards, currOffset);

                            debugPathPnt2 = linkAfterwardsStart3D;

                            irr::core::vector3df linkAfterwardsEnd3D;

                            //end point for link afterwards is needed to calculate midpoint
                            irr::core::vector2df bezierPntNextLinkEnd = pntrLinkAfterwards->pEndEntity->GetMyBezierCurvePlaningCoord(linkAfterwardsEnd3D);
                            mParentPlayer->mRace->mPath->OffsetWayPointLinkCoordByOffset(bezierPntNextLinkEnd, linkAfterwardsEnd3D, pntrLinkAfterwards, currOffset);

                            //calculate midpoint for link afterwards, is the bezier curve 1 end point
                            irr::core::vector2df curveEndPoint =
                                    mParentPlayer->mRace->testBezier->GetBezierCurvePlaningCoordMidPoint(linkAfterwardsStart3D, linkAfterwardsEnd3D, debugPathPnt3);

                            if (mParentPlayer->mRace->mPath->SaniCheckBezierInputPoints(bezierPnt1,bezierPntNextLinkStart, curveEndPoint, raceDirection)) {
                                //other order works
                                newPoints = mParentPlayer->mRace->testBezier->QuadBezierCurveGetSegments( bezierPnt1, curveEndPoint, bezierPntNextLinkStart,
                                                                                                CP_BEZIER_RESOLUTION, currDbgColor);
                               // this->mRace->mGame->StopTime();
                            } else {
                                //also this does not work
                                //just give up
                            }
                    }
                }
            }

            //now check if the new "way" is free from other players

                //only check possible collision with players we do actually see in front of us
                //otherwise we would report an possible collision with our path, when the other
                //player comes close to our current path behind us
                //and we do not want to detect this
                std::vector<Player*> playerISee;
                std::vector<RayHitInfoStruct>::iterator it;

                playerISee.clear();
                for (it = mParentPlayer->PlayerSeenList.begin(); it != mParentPlayer->PlayerSeenList.end(); ++it) {
                    if ((*it).HitType == RAY_HIT_PLAYER) {
                        playerISee.push_back((*it).HitPlayerPntr);
                    }
                }
                if (playerISee.size() > 0) {

                        if (!mParentPlayer->mRace->mPath->DoesPathComeTooCloseToAnyOtherPlayer(newPoints, playerISee, mParentPlayer->dbgPlayerInMyWay)) {
                                freeWayFound = true;
                        } else {
                                //no free way found
                                updatePathCnter++;

                                if (goleft) {
                                    currOffset -= 0.75f;
                                    //if (currOffset < (nextLink->minOffsetShiftStart + 1.0f)) {
                                    if (currOffset < (this->mCpFollowedWayPointLinkCurrentSpaceLeftSide + 1.0f)) {
                                        leftFailed = true;
                                        goleft = false;
                                    }

                                } else {
                                    //go right
                                    currOffset += 0.75f;

                                  //  if (currOffset > (nextLink->maxOffsetShiftStart - 1.0f)) {
                                     if (currOffset > (this->mCpFollowedWayPointLinkCurrentSpaceRightSide - 1.0f)) {
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
        angleDotProduct = (*it)->LinkDirectionVec.dotProduct(mParentPlayer->craftForwardDirVec);

        angleRad = acosf(angleDotProduct);
        angle = (angleRad / irr::core::PI) * 180.0f;

       if (mParentPlayer->craftSidewaysToRightVec.dotProduct((*it)->LinkDirectionVec) > 0.0f) {
           angle = -angle;
       }

        //if not in opposite direction add new segment path
        if (fabs(angle) < 90.0f) {
            mCurrentPathSeg.push_back(*it);
            mPathHistoryVec.push_back(*it);
        } else {
            mCurrentPathSegSortedOutReverse.push_back(*it);
        }
    }

    //update current player offset path value
    mCpCurrPathOffset = currOffset;

    mCurrentPathSegNrSegments = (irr::u32)(mCurrentPathSeg.size());

    CPTrackMovement();

    //this->mRace->mGame->StopTime();
}

void CpuPlayer::CpAddCommandTowardsNextCheckpoint() {
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
            mParentPlayer->mRace->mPath->FindAllWayPointsInArea(mParentPlayer->phobj->physicState.position, 10.0f);

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
        structPntrVec = mParentPlayer->mRace->mPath->FindWaypointLinksForWayPoint((*it), true, true, NULL);

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
                    make_pair( mParentPlayer->mRace->mPath->CalculateDistanceFromWaypointLinkToNextCheckpoint(*it2), (*it2)));
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
   irr::f32 distStart = (mParentPlayer->phobj->physicState.position - currLink->pStartEntity->getCenter()).getLength();
   irr::f32 distEnd = (mParentPlayer->phobj->physicState.position - currLink->pEndEntity->getCenter()).getLength();

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

void CpuPlayer::CpDefineNextAction() {
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

/* Cp HandleCharging that was working before 29.05.2025 without ChargingStation Class */

//void CpuPlayer::CpHandleCharging() {
//   //what charging do we need to do?
//   switch(currCommand->cmdType) {
//        case CMD_CHARGE_AMMO: {
//          if (mParentPlayer->IsCurrentlyChargingAmmo()) {
//              if (mParentPlayer->mPlayerStats->ammoVal >= (0.95 * mParentPlayer->mPlayerStats->ammoMax)) {
//                  //charging finished
//                  mParentPlayer->LogMessage((char*)"Ammo charging finished");

//                  mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;
//                  CurrentCommandFinished();

//                  //old lines before WorkaroundResetCurrentPath
//                  AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
//                  computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
//              } else {
//                  //lets stop computer player until
//                  //charging is finished
//                  computerPlayerTargetSpeed = 0.0f;
//              }

//              break;
//          }
//       }

//       case CMD_CHARGE_SHIELD: {
//         if (mParentPlayer->IsCurrentlyChargingShield()) {
//             if (mParentPlayer->mPlayerStats->shieldVal >= (0.95 * mParentPlayer->mPlayerStats->shieldMax)) {
//                 //charging finished
//                 mParentPlayer->LogMessage((char*)"Shield charging finished");

//                 mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;
//                 CurrentCommandFinished();

//                 //old lines before WorkaroundResetCurrentPath
//                 AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
//                 computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
//             } else {
//                 //lets stop computer player until
//                 //charging is finished
//                 computerPlayerTargetSpeed = 0.0f;
//             }

//             break;
//         }
//      }

//       case CMD_CHARGE_FUEL: {
//         if (mParentPlayer->IsCurrentlyChargingFuel()) {
//             if (mParentPlayer->mPlayerStats->gasolineVal >= (0.95 * mParentPlayer->mPlayerStats->gasolineMax)) {
//                 //charging finished
//                 mParentPlayer->LogMessage((char*)"Fuel charging finished");

//                 mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;

//                 CurrentCommandFinished();

//               //  this->mRace->mGame->StopTime();

//                 //old lines before WorkaroundResetCurrentPath
//                 AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
//                 computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
//             } else {
//                 //lets stop computer player until
//                 //charging is finished
//                 computerPlayerTargetSpeed = 0.0f;
//             }

//             break;
//         }
//        }
//   }
//}

void CpuPlayer::CpWaitForChargingStallReached() {
    //have we reached the stall?
    irr::core::vector3df distVec = mLocationChargingStall - this->mParentPlayer->phobj->physicState.position;

    //need to ignore the Y-axis, we only want to be aligned in X and Z-axis
    //otherwise for example at the fuel charger at the ramp in level 2 we can
    //never have a match of the position as the craft are far above the fuel
    //charger stalls
    distVec.Y = 0.0f;

    irr::f32 distSq = distVec.getLengthSQ();

    dbgDistVec = distSq;

    if (distSq < 1.0f) {
        irr::f32 dist = distVec.getLength();
        if (dist < 0.5f) {
             mParentPlayer->LogMessage((char*)"Reached the assigned charging stall, stop");

             mReachedChargingStall = true;

             //we reached the assigned stall
             //lets stop computer player until
             //charging is finished
             computerPlayerTargetSpeed = 0.0f;

             CurrentCommandFinished();

             switch (mAssignedChargingStation->GetChargingStationType()) {
                 case LEVELFILE_REGION_CHARGER_SHIELD: {
                       AddCommand(CMD_CHARGE_SHIELD);
                       break;
                 }

                 case LEVELFILE_REGION_CHARGER_FUEL: {
                       AddCommand(CMD_CHARGE_FUEL);
                       break;
                 }

                 case LEVELFILE_REGION_CHARGER_AMMO: {
                       AddCommand(CMD_CHARGE_AMMO);
                       break;
                 }

                 default: {
                     break;
                 }
             }
      }
  }
}

void CpuPlayer::CpWaitForChargingFinished() {
   //what charging do we need to do?
   switch(currCommand->cmdType) {
        case CMD_CHARGE_AMMO: {
          if (mParentPlayer->IsCurrentlyChargingAmmo()) {
              if (mParentPlayer->mPlayerStats->ammoVal >= (0.95 * mParentPlayer->mPlayerStats->ammoMax)) {
                  //charging finished
                  mParentPlayer->LogMessage((char*)"Ammo charging finished");

                  mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;
                  CurrentCommandFinished();

                  //report charging finished to charging station
                  //so that my reserved stall is free again
                  mAssignedChargingStation->ChargingFinished(mParentPlayer);

                  //reenable seperation handling
                  //mHandleSeperation = true;

                  //old lines before WorkaroundResetCurrentPath
                  //next line commented out and replaced at 01.06.2025
                  //AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
                  //AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mAssignedChargingStation->exitWayPointLink);

                  CpCommandPlayerToExitChargingStall(mAssignedChargingStation);

                  computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
              }
          }

           break;
       }

   case CMD_CHARGE_SHIELD: {
     if (mParentPlayer->IsCurrentlyChargingShield()) {
         if (mParentPlayer->mPlayerStats->shieldVal >= (0.95 * mParentPlayer->mPlayerStats->shieldMax)) {
             //charging finished
             mParentPlayer->LogMessage((char*)"Shield charging finished");

             mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;
             CurrentCommandFinished();

             //report charging finished to charging station
             //so that my reserved stall is free again
             mAssignedChargingStation->ChargingFinished(mParentPlayer);

             //reenable seperation handling
             //mHandleSeperation = true;

             //next line commented out and replaced at 01.06.2025
             //AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
             //AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mAssignedChargingStation->exitWayPointLink);

             CpCommandPlayerToExitChargingStall(mAssignedChargingStation);

             computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
         }
     }

      break;
  }


   case CMD_CHARGE_FUEL: {
     if (mParentPlayer->IsCurrentlyChargingFuel()) {
         if (mParentPlayer->mPlayerStats->gasolineVal >= (0.95 * mParentPlayer->mPlayerStats->gasolineMax)) {
             //charging finished
             mParentPlayer->LogMessage((char*)"Fuel charging finished");

             mCpCurrentDeaccelRate = CP_PLAYER_DEACCEL_RATE_DEFAULT;
             CurrentCommandFinished();

             //report charging finished to charging station
             //so that my reserved stall is free again
             mAssignedChargingStation->ChargingFinished(mParentPlayer);

             //reenable seperation handling
             //mHandleSeperation = true;

             //next line commented out and replaced at 01.06.2025
             //AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
             //AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mAssignedChargingStation->exitWayPointLink);

             CpCommandPlayerToExitChargingStall(mAssignedChargingStation);

             computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
         }
     }

      break;
   }
   }
}

void CpuPlayer::FlyTowardsEntityRunComputerPlayerLogic(CPCOMMANDENTRY* currCommand) {
    //if we run this method for human player
    //just exit

    if (mParentPlayer->mHumanPlayer)
        return;

    //is there really a target entity and waypoint link information?
     if ((currCommand->targetWaypointLink != NULL) && (currCommand->targetEntity != NULL)) {
        //yes, there is

        cPCurrentFollowSeg = currCommand->targetWaypointLink;

        irr::core::vector3df entPos = currCommand->targetEntity->getCenter();

        //have we reached the target yet?
        irr::f32 distToTarget = (mParentPlayer->phobj->physicState.position -
                                 entPos).getLength();

        if (distToTarget < 2.0f) {
            //we reached the target
            //this->mHUD->ShowBannerText((char*)"TARGET REACHED", 4.0f);

            mParentPlayer->LogMessage((char*)"FlyTowardsEntityRunComputerPlayerLogic: Target reached");

            EntityItem* pntrItem = currCommand->targetEntity;

            //mark current command as finished, pull the next one
            CurrentCommandFinished();

            cPCurrentFollowSeg = NULL;

            //continue path via next waypoint-links
            //too which waypoint link does this entity item belong too
            std::vector<WayPointLinkInfoStruct*> whichLinksStart =
                    mParentPlayer->mRace->mPath->FindWaypointLinksForWayPoint(pntrItem, true, false, NULL);

            if (whichLinksStart.size() > 0) {
                AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, whichLinksStart.at(0));

                return;
            }

            std::vector<WayPointLinkInfoStruct*> whichLinksEnd =
                    mParentPlayer->mRace->mPath->FindWaypointLinksForWayPoint(pntrItem, false, true, NULL);

            if (whichLinksEnd.size() > 0) {
                if (whichLinksEnd.at(0)->pntrPathNextLink != NULL) {
                    AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, whichLinksEnd.at(0)->pntrPathNextLink);
                }

                return;
            }

            return;
        }
    }
}

//Function to free all currently pending
//commands, and command list
void CpuPlayer::CleanUpCommandList() {
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

                       LineStruct* pntrLineExtStruct = pntrCmd->targetWaypointLink->pLineStructExtended;

                       delete[] pntrLineExtStruct->name;
                       delete pntrLineExtStruct;

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

CPCOMMANDENTRY* CpuPlayer::CreateNoCommand() {
    CPCOMMANDENTRY* newcmd = new CPCOMMANDENTRY();
    newcmd->cmdType = CMD_NOCMD;
    newcmd->targetEntity = NULL;
    newcmd->targetPosition.set(0.0f, 0.0f, 0.0f);
    newcmd->targetWaypointLink = NULL;
    newcmd->targetChargingStation = NULL;

    return newcmd;
}

CPCOMMANDENTRY* CpuPlayer::PullNextCommandFromCmdList() {
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

void CpuPlayer::CurrentCommandFinished() {
    CPCOMMANDENTRY* oldCmd = currCommand;
    this->cmdList->pop_front();

    //if we set currCommand to NULL then the program
    //will pull the next available command in
    //RunComputerPlayerLogic
    currCommand = NULL;

    //19.04.2025: Note: I had a very rare segmentation fault today
    //in the next line, because it seeems currCommand was initially NULL
    //at the top, which caused an access to a NULL in the line below
    //if this occurs again in future, should I adding an exit here if
    //oldCmd == NULL?

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

                LineStruct* pntrLineExtStruct = oldCmd->targetWaypointLink->pLineStructExtended;

                delete[] pntrLineExtStruct->name;
                delete pntrLineExtStruct;

                delete oldCmd->targetWaypointLink;
            }
        }
    }

    //delete old command struct
    delete oldCmd;
}

void CpuPlayer::CpPlayerHandleAttack() {
    //if I do not see any other player, simply return
    if (mParentPlayer->PlayerSeenList.size() < 1)
        return;

    //if we have no target player, just return
    if (mParentPlayer->mTargetPlayer == NULL)
        return;

    //if the target player has already finished the race also
    //do not shot at him
    if (mParentPlayer->mTargetPlayer->mPlayerStats->mHasFinishedRace)
        return;

    //if the first player has not crossed the finished line
    //the first time we also do not want to allow shooting
    if (!mParentPlayer->mRace->RaceAllowsPlayersToAttack())
        return;

    //if we have a (red) perfect lock on another player, we have enough ammo
    //and the target is far enough away fire missile
    if (mParentPlayer->mPlayerStats->ammoVal > 0.0f) {
        if (mParentPlayer->mTargetMissleLock) {
            irr::core::vector3df distVec = mParentPlayer->mTargetPlayer->phobj->physicState.position -
                    mParentPlayer->phobj->physicState.position;

            if (distVec.getLength() > 15.0f) {
                mParentPlayer->mMissileLauncher->Trigger();

                //fire one missile is enough
                //just exit
                return;
            }
        }
    }

    //machine gun currently cool enough
    //if so, do we have a target right now?
    if (!mParentPlayer->mMGun->CoolDownNeeded()) {
            //yes, fire
            mParentPlayer->mMGun->Trigger();
    }
}

void CpuPlayer::CpHandleSeperation(irr::f32 deltaTime) {

    return ;

    if (deltaTime > 0.02f)
        deltaTime = 0.02f;

    irr::f32 speedFactor = (deltaTime / (irr::f32)(1.0f / 60.0f));

    if (mParentPlayer->mCraftDistanceAvailRight < 0.75f) {  //1.0f
        if (mParentPlayer->mCraftDistanceAvailLeft > 2.0f) { //2.0f
            if (mLocalOffset > -0.75f) {
                mLocalOffset -= 0.1f * speedFactor;
            }
        }
    }

    if (mParentPlayer->mCraftDistanceAvailLeft < 0.75f) {  //1.0f
        if (mParentPlayer->mCraftDistanceAvailRight > 2.0f) { //2.0f
           if (mLocalOffset < 0.75f) {
            mLocalOffset += 0.1f * speedFactor;
           }
        }
    }
}

void CpuPlayer::RunPlayerLogic(irr::f32 deltaTime) {
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

        //currently at the charging stall, and waiting until charging
        //is finished
        case CMD_CHARGE_FUEL:
        case CMD_CHARGE_AMMO:
        case CMD_CHARGE_SHIELD: {
            CpWaitForChargingFinished();
            break;
        }

        case CMD_PICKUP_COLLECTABLE: {
            this->mCpTargetCollectableToPickUp = currCommand->targetCollectible;

            if (mCpWayPointLinkClosestToCollectable == NULL) {
                //figure out which target link is closest to this collectable
                std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> wayPointLinkCloseToCollectable =
                        mParentPlayer->mRace->mPath->FindClosestWayPointLinkToCollectible(mCpTargetCollectableToPickUp);

                if (wayPointLinkCloseToCollectable.first != NULL) {
                   mCpWayPointLinkClosestToCollectable = wayPointLinkCloseToCollectable.first;
                }
            }

            break;
        }

        //wait until player comes much closer to the selected
        //charging station
        case CMD_GOTO_CHARGINGSTATION: {
            if (!mSetupPathToChargingStation) {

                //are we close to the charging station we targeted?
                if (currCommand->targetChargingStation->ReachedEntryOfChargingStation(this->currClosestWayPointLink.first)) {
                    //this->mParentPlayer->mRace->mGame->StopTime();

                    CurrentCommandFinished();

                    //reprogram computer player path to the reserved
                    //charging station stall
                    CpCommandPlayerToChargingStall(mAssignedChargingStation, mAssignedChargingStall);

                    //we need to switch seperation handling off here
                    //otherwise we craft can not hit the designated stalls
                    //we also need to set the local offset to 0.0f
                    mHandleSeperation = false;
                    mLocalOffset = 0.0f;

                    mSetupPathToChargingStation = true;
                }
            } else {
                if (!mSetupPathToChargingStall) {
                    mCpFollowThisWayPointLink = currCommand->targetWaypointLink;
                    mCpCurrPathOffset = 0.0f;

                    FollowPathDefineNextSegment(currCommand->targetWaypointLink, mCpCurrPathOffset, false);

                    computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;

                    mSetupPathToChargingStall = true;
                } else {
                    //wait until we reach the stall location
                    CpWaitForChargingStallReached();
                }
            }

//           if (!mSetupPathToChargingStation) {
//                mSetupPathToChargingStation = true;
//                mReachedChargingStall = false;

//                mCpFollowThisWayPointLink = currCommand->targetWaypointLink;
//                //FollowPathDefineNextSegment(mCpLastFollowThisWayPointLink, mCpCurrPathOffset, true);
//                FollowPathDefineNextSegment(currCommand->targetWaypointLink, mCpCurrPathOffset, false);
//                /*if (mHUD != NULL) {
//                  this->mHUD->ShowBannerText((char*)"FOLLOW", 4.0f);
//                }*/
//                computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;
//           } else {
//                 //wait until we reach our charging stand
//                 CpWaitForChargingStallReached(currCommand->targetChargingStation);
//            }
            break;
        }

        case CMD_EXIT_CHARGINGSTATION: {
                if (!mSetupPathToExitChargingStation) {

                    mCpFollowThisWayPointLink = currCommand->targetWaypointLink;
                    mCpCurrPathOffset = 0.0f;

                    FollowPathDefineNextSegment(currCommand->targetWaypointLink, mCpCurrPathOffset, false);

                    computerPlayerTargetSpeed = CP_PLAYER_SLOW_SPEED;

                    mSetupPathToExitChargingStation = true;

                    mParentPlayer->LogMessage((char*)"Charging Station exit path setup done");
                    //this->mParentPlayer->mRace->mGame->StopTime();
                } else {
                   //did we already exit the charging station
                   if (mAssignedChargingStation->PassedExitOfChargingStation(this->currClosestWayPointLink.first)) {
                       mParentPlayer->LogMessage((char*)"Passed exit of assigned charging station");

                       CurrentCommandFinished();

                       //reenable seperation handling
                       mHandleSeperation = true;

                       AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, this->mCpFollowThisWayPointLink);
                   }
                }

               break;
            }

        case CMD_FOLLOW_TARGETWAYPOINTLINK: {
            mCpFollowThisWayPointLink = currCommand->targetWaypointLink;
            //FollowPathDefineNextSegment(mCpLastFollowThisWayPointLink, mCpCurrPathOffset, true);
            FollowPathDefineNextSegment(currCommand->targetWaypointLink, mCpCurrPathOffset, false);
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
    if (CpEnableStuckDetection) {
        CpStuckDetection(deltaTime);
    }

    CpPlayerCollectableSelectionLogic();

    //check for obstacles only every 100 ms
    mCpAbsCheckObstacleTimerCounter += deltaTime;

    if (mCpAbsCheckObstacleTimerCounter >= 0.1f) {
        mCpAbsCheckObstacleTimerCounter -= 0.1f;

        CpCheckCurrentPathForObstacles();
    }

    //if computer players attack, run the routine below
    if (computerPlayersAttack) {
        CpPlayerHandleAttack();
    }

    //mHandleSeperation allows to switch control of seperation
    //off. We especially want to turn it off inside charging stations
    //because otherwise the craft will not be able to reach the defined
    //stall positions
    if (mHandleSeperation) {
        CpHandleSeperation(deltaTime);
    }

    //for all computer players in this race we need to call the
    //CPForceController which has the job to control the crafts movement
    //so that the computer is following the currenty definded target path
    CPForceController(deltaTime);

    return;
}
