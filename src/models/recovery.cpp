/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "recovery.h"

Recovery::Recovery(Race* race, irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr) {
    ready = false;
    mSmgr = smgr;
    mRace = race;

    mCurrentState = STATE_RECOVERY_IDLE;
    mCurrentMovementState = STATE_RECOVERY_MOVE_STATIONARY;

    mPosition.X = x;
    mPosition.Y = y;
    mPosition.Z = z;

    //set my initial orientation
    mOrientation.set(irr::core::vector3df(0.0f, 0.0f, 0.0f));

    mCurrentSpeed = 0.0f;
    mTargetSpeed = 0.0f;

    //remember the starting position, we need it later
    mStartingPosition = mPosition;

    RecoveryMesh = smgr->getMesh("extract/models/recov0-0.obj");
    Recovery_node = smgr->addMeshSceneNode(RecoveryMesh);

    Recovery_node->setScale(irr::core::vector3d<irr::f32>(1,1,1));
    Recovery_node->setMaterialFlag(irr::video::EMF_LIGHTING, mRace->mGame->enableLightning);

    if (this->mRace->mGame->enableShadows) {
        // add shadow
        NodeShadow = Recovery_node->addShadowVolumeSceneNode();
    }

    //first model position and orientation
    //update
    UpdateSceneNode();

    //setup the local coordinate of my claw
    //is the coordinate where I will attach to the
    //player model
    localCoordClaw.set(0.0f, -2.3f, 0.0f);
}

bool Recovery::CurrentlyReadyforMission() {
    //we are ready for a new mission if we either are currently idling at our home
    //(starting) position, or if we are currently on the way back to the starting position
    if ((this->mCurrentState == STATE_RECOVERY_IDLE) ||
            (this->mCurrentState == STATE_RECOVERY_GOBACKTOIDLEPOSITION))
                return true;

    //in all other states we are currently helping somebody
    //and can not start another mission
    return false;
}

irr::core::vector3df Recovery::GetCurrentPosition() {
    return mPosition;
}

bool Recovery::WayPointLinkAcceptableForDropOf(WayPointLinkInfoStruct* link) {
    if (link == NULL)
        return false;

    irr::f32 minDistanceRight = link->maxOffsetShiftEnd;

    if (link->maxOffsetShiftStart < minDistanceRight) {
        minDistanceRight = link->maxOffsetShiftStart;
    }

    //the values in minOffset variables are negative!
    //keep this is mind; so smaller space is actually more positive
    irr::f32 minDistanceLeft = link->minOffsetShiftEnd;

    if (link->minOffsetShiftStart > minDistanceLeft) {
        minDistanceLeft = link->minOffsetShiftStart;
    }

    //only pick this waypoint link as a drop of target if both
    //free spaces left and right are higher then a certain threshold
    if ((fabs(minDistanceLeft) > RECOVERY_VEHICLE_DROPOFTARGET_FREESPACE_REQ)
            && (fabs(minDistanceRight) > RECOVERY_VEHICLE_DROPOFTARGET_FREESPACE_REQ)) {
        //dropof waypointlink acceptable (has enough free space around it)
        return true;
    }

    return false;
}

void Recovery::FindPlayerDropOfPosition() {
    //where do we want to drop of player again?
    //find closest waypoint to player
    EntityItem* closestWayPoint =
            this->repairTarget->mRace->mPath->FindNearestWayPointToPlayer(this->repairTarget);

    //if we do not find anything, just drop player off here again
    //Fallback, but should hopefully never happen in game later
    if (closestWayPoint == NULL) {
          this->mPlayerDropOfPosition = this->mPosition;
         mPlayerDropOfDirVecFound = false;
    } else {
        //to which waypoint Link does this waypoint belong too?
        //we need the waypoint link to now the racing direction, because
        //we want to turn the player into race direction when we drop him off again
        std::vector<WayPointLinkInfoStruct*> wayPointLinks =
                this->repairTarget->mRace->mPath->FindWaypointLinksForWayPoint(closestWayPoint, true, true, NULL);

        if (wayPointLinks.size() <= 0) {
            //we did not find anything
            //just drop player off here again
            //Fallback, but should hopefully never happen in game later
            this->mPlayerDropOfPosition = this->mPosition;
            mPlayerDropOfDirVecFound = false;
        } else {
            //Note 25.01.2025: I saw where often the situation that a computer player craft gets stuck at certain
            //places of the race track (were predefined path is too close to terrain or blocks). The computer player
            //stuck detection will kick in, and send a recovery craft for help after some time to place the player again
            //on the track with the hope the computer player can continue its journey, without the human player to notice that :)
            //It is also very important that this mechanism works, because otherwise if the computer player remains stuck the race
            //can not be finished at the end.
            //But I saw that the recovery craft comes for help, repositions the stuck computer player at the same closest waypoint
            //link again, and the computer player gets stuck again exactly at the same location. So at the end still the same problem
            //occurs again and again. To fix this I want to add additional logic here that only allows to pick target repositioning waypoint
            //links here, that have enough free space around them on both sides
            //if the closest waypoint link does not, then this logic should pick the one before which has.
            std::vector<WayPointLinkInfoStruct*>::iterator itLink;

            WayPointLinkInfoStruct* dropOfLink = NULL;
            std::vector<WayPointLinkInfoStruct*> alternatives;

            for (itLink = wayPointLinks.begin(); itLink != wayPointLinks.end(); ++itLink) {
               if (WayPointLinkAcceptableForDropOf(*itLink)) {
                            //good drop of link found => ok
                            dropOfLink = (*itLink);
                            break;
                }
            }

            std::vector<WayPointLinkInfoStruct*>::iterator it2;
            //we did still not find anything?
            //try to go back one waypoint link before
            for (itLink = wayPointLinks.begin(); itLink != wayPointLinks.end(); ++itLink) {
                if (dropOfLink != NULL)
                    break;

                alternatives =
                        this->mRace->mPath->DeliverAllWayPointLinksThatLeadIntpSpecifiedToWayPointLink(*itLink);

                if (alternatives.size() > 0) {
                      for (it2 = alternatives.begin(); it2 != alternatives.end(); ++it2) {
                          if (WayPointLinkAcceptableForDropOf(*it2)) {
                                       //alternative drop of link found => ok
                                       dropOfLink = (*it2);
                                       break;
                           }
                      }
                }
            }

            if (dropOfLink != NULL) {
                //we did find the waypoint link, get the drop off direction vector
                irr::core::vector3df mPlayerDropOfDirVec = dropOfLink->LinkDirectionVec.normalize();

                //derive a craft absolute orientation angle for drop off we want to achieve
                mPlayerDropOfAbsAngle = repairTarget->mRace->GetAbsOrientationAngleFromDirectionVec(mPlayerDropOfDirVec);

                mPlayerDropOfDirVecFound = true;

                //drop player of in the middle of the WaypointLink we found
                this->mPlayerDropOfPosition =
                        (dropOfLink->pLineStruct->B - dropOfLink->pLineStruct->A) *
                        irr::core::vector3df(0.5f, 0.5f, 0.5f) + dropOfLink->pLineStruct->A;

                return;
            }

            //even at the end no drop of waypoint link found
            mPlayerDropOfDirVecFound = false;
        }

        this->mPlayerDropOfPosition = closestWayPoint->getCenter();
    }
}

//returns true if the recovery vehicle has reached the current movement target
//returns false otherwise
bool Recovery::ControlMovement(irr::core::vector3df vecMov, irr::f32 deltaTime) {
    if (mCurrentMovementState == STATE_RECOVERY_MOVE_STATIONARY) {
        mCurrentMovementState = STATE_RECOVERY_MOVE_MOVING;
        mTargetSpeed = RECOVERY_VEHICLE_SPEED;

        //calculate distance threshold where we want to start braking again
        //lets make 50% of the overall distance
        //just calculate one, and store in member variable, so that we do not
        //need to recalculate distance every frame
        mDistanceStartBraking = vecMov.getLength() * 0.50f;

        //if the distance to travel is really short, just start with crawl speed to not
        //overrun the target
        if (mDistanceStartBraking < 5.0f) {
            mCurrentMovementState = STATE_RECOVERY_MOVE_BRAKING;
            mTargetSpeed = RECOVERY_VEHICLE_SPEED_CRAWL;
        }
    }

    irr::f32 distanceToCraft = vecMov.getLength();

    //are we close enough to start braking again to reach
    //crawl speed, and home in on the target?
    if ((distanceToCraft < mDistanceStartBraking) && (mCurrentMovementState == STATE_RECOVERY_MOVE_MOVING)) {
        mCurrentMovementState = STATE_RECOVERY_MOVE_BRAKING;
        mTargetSpeed = RECOVERY_VEHICLE_SPEED_CRAWL;
    }

    //have we reached the target?
    if (distanceToCraft < 0.1f) {
        //suddently stop recovery vehicle
        mCurrentSpeed = 0.0f;
        mTargetSpeed = 0.0f;
        mCurrentMovementState = STATE_RECOVERY_MOVE_STATIONARY;

        return true;
    }

    //not reached the target yet, keep moving forward
    irr::core::vector3df dirVec = vecMov.normalize();

    //move further in direction of this jobs target
    //make a simple cinematic movement here
    this->mPosition += dirVec * mCurrentSpeed * deltaTime;

    return false;
}

void Recovery::State_MoveToDuty(irr::f32 deltaTime) {
    //first we need to go to the player we need to help
    //use my claw coordinate as a reference to go there
    irr::core::vector3df vecMov = (repairTarget->phobj->physicState.position - worldCoordClaw);

    bool targetReached = ControlMovement(vecMov, deltaTime);

    if (targetReached) {
        //yes we have
        mCurrentState = STATE_RECOVERY_PLAYERGRABBED;

        //from this moment on the player craft model must be
        //locked to this claw until the player is freed again
        //first lets deactivate player physics object, so that its
        //physic calculations, collision detections, and Irrlicht sceneNode
        //updates inside physics are not done anymore
        //Then we can attach the SceneNode of the player to the Recovery craft
        //SceneNode (parent), and the player craft should be moved together with
        //the recovery craft model
        this->repairTarget->phobj->mActive = false;

        //tell the player object additional that it is currently grabed by this
        //recovery vehicle
        //Player is also refueled (repaired) inside this function
        this->repairTarget->SetGrabedByRecoveryVehicle(this);

        //call method to determine player dropof position
        FindPlayerDropOfPosition();

        irr::core::vector2di cellOut;

        //get height of terrain at the drop zone
        this->mPlayerDropOfPosition.Y =
                this->repairTarget->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                    this->mPlayerDropOfPosition.X, this->mPlayerDropOfPosition.Z, cellOut);

        //we want to put the player craft in the default hover height above the terrain
        this->mPlayerDropOfPosition.Y += repairTarget->GetHoverHeight();
    }
}

void Recovery::State_PlayerGrabbed(irr::f32 deltaTime) {
    //go to the location where we want to drop the player off again
    //at waypoint where we can drop the player of again
    //use my claw coordinate as a reference to go there
    irr::core::vector3df vecMovDropOff = (this->mPlayerDropOfPosition - worldCoordClaw);
    irr::f32 absAngleError;

    bool targetReached = ControlMovement(vecMovDropOff, deltaTime);

    //calculate also current absolute orientation error if possible, and
    //fix it as well while flying
    if (mPlayerDropOfDirVecFound) {
        irr::f32 currPlayerAbsAngle = repairTarget->mRace->GetAbsOrientationAngleFromDirectionVec(
                    repairTarget->craftForwardDirVec);

        absAngleError = mPlayerDropOfAbsAngle - currPlayerAbsAngle;

        //allow max 5 deg error
        if (abs(absAngleError) > 5.0f) {
            //we have to correct current recovery & player craft angle of orientation

            //calculate local angle, starting point is orientation of ship
            irr::core::quaternion currCraftOrientation = repairTarget->phobj->physicState.orientation;

            if (absAngleError > 2.0f)
                absAngleError = 2.0f;
            else if (absAngleError < -2.0f)
                absAngleError = -2.0f;

            irr::core::quaternion rotateFurther;

            //only allow to rotate recovery vehicle model around Y-AXIS
            //otherwise the model would look strange when it has the orientation of the
            //player model at the end
            rotateFurther.fromAngleAxis((-absAngleError / 180.0f) * irr::core::PI, irr::core::vector3df(0.0f, 1.0f, 0.0f));
            rotateFurther.normalize();

            currCraftOrientation *= rotateFurther;
            currCraftOrientation.normalize();

            //also rotate the recovery vehicle itself
            mOrientation *= rotateFurther;
            mOrientation.normalize();

            irr::core::vector3df euler;

            currCraftOrientation.toEuler(euler);
            this->repairTarget->Player_node->setRotation(euler * RADTODEG);

            //also update orientation in the physics object of player craft
            repairTarget->phobj->physicState.orientation *= rotateFurther;
            repairTarget->phobj->physicState.orientation.normalize();
        }
    }

    //have we reached the player dropoff location?
    if (targetReached &&
            (!mPlayerDropOfDirVecFound || (mPlayerDropOfDirVecFound && (abs(absAngleError) < 5.0f)))) {
        //yes we have
        mCurrentState = STATE_RECOVERY_PUTPLAYERBACK;

        //free the player again from this recovery vehicle
        this->repairTarget->FreedFromRecoveryVehicleAgain();

        //make sure all momentums of player object
        //are zero before droping player back
        this->repairTarget->phobj->physicState.momentum = irr::core::vector3df(0.0f, 0.0f, 0.0f);
        this->repairTarget->phobj->physicState.angularMomentum = irr::core::vector3df(0.0f, 0.0f, 0.0f);

        //we also need to update the position of the player inside the phyics
        //object data!
        this->repairTarget->phobj->physicState.position = worldCoordClaw;
        this->repairTarget->Player_node->setPosition(worldCoordClaw);

        //update all other physic states as well, this will also
        //set velocity to zero
        this->repairTarget->phobj->physicState.recalculate();

        //activate normal physics for this player physicobject again
        this->repairTarget->phobj->mActive = true;

        this->mCurrentState = STATE_RECOVERY_GOBACKTOIDLEPOSITION;
    } else {

     //update player craft location to new claw position
     this->repairTarget->Player_node->setPosition(worldCoordClaw);

     //we also need to update the position of the player inside the phyics
     //object data!
     this->repairTarget->phobj->physicState.position = worldCoordClaw;
   }
}

void Recovery::State_GoBackToIdleState(irr::f32 deltaTime) {
    //go back to main original (default) idling location
    //reference is here the recovery vehicle model original position
    irr::core::vector3df vecMovIdlePosition = (this->mStartingPosition - this->mPosition);

    bool targetReached = ControlMovement(vecMovIdlePosition, deltaTime);

    //have we reached the default idle location again?
    if (targetReached) {
        //yes we have, go back to idle state
        mCurrentState = STATE_RECOVERY_IDLE;
    }
}

void Recovery::Update(irr::f32 deltaTime) {
    //update my claws position
    this->Recovery_node->updateAbsolutePosition();

    irr::core::vector3df pos_in_world(localCoordClaw);
    this->Recovery_node->getAbsoluteTransformation().transformVect(pos_in_world);

    this->worldCoordClaw = pos_in_world;

    if (mCurrentSpeed < mTargetSpeed) {
        mCurrentSpeed += RECOVERY_VEHICLE_ACCELDEACCELRATE;

        if (mCurrentSpeed > RECOVERY_VEHICLE_SPEED) {
            mCurrentSpeed = RECOVERY_VEHICLE_SPEED;
        }
    } else if (mCurrentSpeed > mTargetSpeed) {
        mCurrentSpeed -= RECOVERY_VEHICLE_ACCELDEACCELRATE;

        if (mCurrentSpeed < 0.0f) {
            mCurrentSpeed = 0.0f;
        }
    }

    //depending on my current state lets do what we need to do
    switch (mCurrentState) {
        case STATE_RECOVERY_MOVETODUTY: {
                 State_MoveToDuty(deltaTime);

                 //update recovery craft model
                 UpdateSceneNode();
                 break;
        }

        case STATE_RECOVERY_PLAYERGRABBED: {
                 State_PlayerGrabbed(deltaTime);

                 //update recovery craft model
                 UpdateSceneNode();
                 break;
        }

        case STATE_RECOVERY_GOBACKTOIDLEPOSITION: {
                 State_GoBackToIdleState(deltaTime);

                 //update recovery craft model
                 UpdateSceneNode();
                 break;
        }
    }
}

void Recovery::UpdateSceneNode() {
    //set new location for recovery vehicle model
    this->Recovery_node->setPosition(mPosition);

    irr::core::vector3df rot;

    //set new rotation
    this->mOrientation.toEuler(rot);
    this->Recovery_node->setRotation(rot * irr::core::RADTODEG);
}

void Recovery::SentToRepairMission(Player* repairTarget) {
    //we are now on our way
    this->mCurrentState = STATE_RECOVERY_MOVETODUTY;

    //remember the repair target (player which needs help)
    this->repairTarget = repairTarget;
}

Recovery::~Recovery() {
    //cleanup recovery stuff

    //remove SceneNode
    this->Recovery_node->remove();

    //remove mesh
    mSmgr->getMeshCache()->removeMesh(RecoveryMesh);
}
