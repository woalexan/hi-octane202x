/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "recovery.h"

Recovery::Recovery(irr::f32 x, irr::f32 y, irr::f32 z, irr::scene::ISceneManager* smgr) {
    ready = false;
    mSmgr = smgr;

    mCurrentState = STATE_RECOVERY_IDLE;

    //my Irrlicht coordinate system is swapped at the x axis; correct this issue
    mPosition.X = -x;
    mPosition.Y = y;
    mPosition.Z = z;

    //remember the starting position, we need it later
    mStartingPosition = mPosition;

    RecoveryMesh = smgr->getMesh("extract/models/recov0-0.obj");
    Recovery_node = smgr->addMeshSceneNode(RecoveryMesh);

    Recovery_node->setPosition(mPosition);
    Recovery_node->setScale(irr::core::vector3d<irr::f32>(1,1,1));
    Recovery_node->setMaterialFlag(irr::video::EMF_LIGHTING, false);

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

void Recovery::Update(irr::f32 deltaTime) {
    //update my claws position
    this->Recovery_node->updateAbsolutePosition();

    irr::core::vector3df pos_in_world(localCoordClaw);
    this->Recovery_node->getAbsoluteTransformation().transformVect(pos_in_world);

    this->worldCoordClaw = pos_in_world;

    //depending on my current state lets do what we need to do
    if (mCurrentState == STATE_RECOVERY_MOVETODUTY) {
        //first we need to go to the player we need to help
        //use my claw coordinate as a reference to go there
        irr::core::vector3df vecMov = (repairTarget->phobj->physicState.position - worldCoordClaw);

        //have we reached the target?
        if (vecMov.getLength() < 0.1f) {
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

            //where do we want to drop of player again?
            //find closest waypoint to player
            EntityItem* closestWayPoint = this->repairTarget->mRace->mPath->FindNearestWayPointToPlayer(this->repairTarget);

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
                    //we did find the waypoint link, get the drop off direction vector
                    irr::core::vector3df mPlayerDropOfDirVec = wayPointLinks.at(0)->LinkDirectionVec.normalize();

                    //derive a craft absolute orientation angle for drop off we want to achieve
                    mPlayerDropOfAbsAngle = repairTarget->mRace->GetAbsOrientationAngleFromDirectionVec(mPlayerDropOfDirVec);

                    mPlayerDropOfDirVecFound = true;
                }

                this->mPlayerDropOfPosition = closestWayPoint->get_Pos();

                //x-coord is mirrored in my project compared with level file data
                this->mPlayerDropOfPosition.X = -this->mPlayerDropOfPosition.X;
            }

            irr::core::vector2di cellOut;

            //get height of terrain at the drop zone
            this->mPlayerDropOfPosition.Y =
                    this->repairTarget->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
                        this->mPlayerDropOfPosition.X, this->mPlayerDropOfPosition.Z, cellOut);

            //we want to put the player craft in the default hover height above the terrain
            this->mPlayerDropOfPosition.Y += repairTarget->GetHoverHeight();

        } else {

          irr::core::vector3df dirVec = vecMov.normalize();

          //move further in direction of this jobs target
          irr::f32 speed = RECOVERY_VEHICLE_SPEED;

         //make a simple cinematic movement here
         this->mPosition += dirVec * speed * deltaTime;

         //set new location for recovery vehicle model
         this->Recovery_node->setPosition(mPosition);
       }
    } else if (mCurrentState == STATE_RECOVERY_PLAYERGRABBED) {
        //go to the location where we want to drop the player off again
        //at waypoint where we can drop the player of again
        //use my claw coordinate as a reference to go there
        irr::core::vector3df vecMovDropOff = (this->mPlayerDropOfPosition - worldCoordClaw);
        irr::f32 absAngleError;

        //calculate also current absolute orientation error if possible, and
        //fix it as well while flying
        if (mPlayerDropOfDirVecFound) {
            irr::f32 currPlayerAbsAngle = repairTarget->mRace->GetAbsOrientationAngleFromDirectionVec(
                        repairTarget->craftForwardDirVec);

            absAngleError = mPlayerDropOfAbsAngle - currPlayerAbsAngle;

            //allow max 2 deg error
            if (abs(absAngleError) > 2.0f) {
                //we have to correct current recovery & player craft angle of orientation

                //calculate local Wheel steering angle, starting point is orientation of ship
                irr::core::quaternion currCraftOrientation = repairTarget->phobj->physicState.orientation;

                if (absAngleError > 2.0f)
                    absAngleError = 2.0f;
                else if (absAngleError < -2.0f)
                    absAngleError = -2.0f;

                irr::core::quaternion rotateFurther;
                rotateFurther.fromAngleAxis((-absAngleError / 180.0f) * irr::core::PI, irr::core::vector3df(0.0f, 1.0f, 0.0f));
                rotateFurther.normalize();

                currCraftOrientation *= rotateFurther;
                currCraftOrientation.normalize();

                irr::core::vector3df euler;

                currCraftOrientation.toEuler(euler);

                //only allow to rotate recovery vehicle model around Y-AXIS
                //otherwise the model would look strange when it has the orientation of the
                //player model at the end
                this->Recovery_node->setRotation(euler * RADTODEG * irr::core::vector3df(0.0f, 1.0f, 0.0f));

                this->repairTarget->Player_node->setRotation(euler * RADTODEG );

                //also update orientation in the physics object of player craft
                repairTarget->phobj->physicState.orientation *= rotateFurther;
                repairTarget->phobj->physicState.orientation.normalize();
            }
        }

        //have we reached the player dropoff location?
        if ((vecMovDropOff.getLength() < 0.1f) &&
                (!mPlayerDropOfDirVecFound || (mPlayerDropOfDirVecFound && (abs(absAngleError) < 2.0f)))) {
            //yes we have
            mCurrentState = STATE_RECOVERY_PUTPLAYERBACK;

            //free the player again from this recovery vehicle
            this->repairTarget->FreedFromRecoveryVehicleAgain();

            //make sure all momentums of player object
            //are zero before droping player back
            this->repairTarget->phobj->physicState.momentum = irr::core::vector3df(0.0f, 0.0f, 0.0f);
            this->repairTarget->phobj->physicState.angularMomentum = irr::core::vector3df(0.0f, 0.0f, 0.0f);

            //update all other physic states as well, this will also
            //set velocity to zero
            this->repairTarget->phobj->physicState.recalculate();

            //activate normal physics for this player physicobject again
            this->repairTarget->phobj->mActive = true;

            this->mCurrentState = STATE_RECOVERY_GOBACKTOIDLEPOSITION;
        } else {

          irr::core::vector3df dirVec = vecMovDropOff.normalize();

          //move further in direction of this jobs target
          irr::f32 speed = RECOVERY_VEHICLE_SPEED;

         //make a simple cinematic movement here
         this->mPosition += dirVec * speed * deltaTime;

         //set new location for recovery vehicle model
         this->Recovery_node->setPosition(mPosition);

         //update player craft location to new claw position
         this->repairTarget->Player_node->setPosition(worldCoordClaw);

         //we also need to update the position of the player inside the phyics
         //object data!
         this->repairTarget->phobj->physicState.position = worldCoordClaw;
       }
    } else if (mCurrentState == STATE_RECOVERY_GOBACKTOIDLEPOSITION) {
        //go back to main original (default) idling location
        //reference is here the recovery vehicle model origina
        irr::core::vector3df vecMovIdlePosition = (this->mStartingPosition - this->mPosition);

        //have we reached the default idle location again?
        if (vecMovIdlePosition.getLength() < 0.1f) {
            //yes we have, go back to idle state
            mCurrentState = STATE_RECOVERY_IDLE;
        } else {
          irr::core::vector3df dirVec = vecMovIdlePosition.normalize();

          //move further in direction of this jobs target
          irr::f32 speed = RECOVERY_VEHICLE_SPEED;

         //make a simple cinematic movement here
         this->mPosition += dirVec * speed * deltaTime;

         //set new location for recovery vehicle model
         this->Recovery_node->setPosition(mPosition);
       }
    }
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
