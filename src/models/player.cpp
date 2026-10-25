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


//Get current weapon shooting target for this player
//Returns true if there was a target found, False otherwise
// bool Player::GetWeaponTarget(RayHitTriangleInfoStruct &shotTarget) {
//     //built a ray cast 3d line from physics object position to direction we want to go
//     irr::core::vector3df startPnt(this->phobj->physicState.position);
//     irr::core::vector3df endPnt(this->phobj->physicState.position + this->craftForwardDirVec * irr::core::vector3df(50.0f, 50.0f, 50.0f));

//     std::vector<irr::core::vector3di> voxels;

//     std::vector<RayHitTriangleInfoStruct*> allHitTriangles;

//     //with ReturnOnlyClosestTriangles = true!
//     allHitTriangles = this->mRace->mRay->ReturnTrianglesHitByRay( this->mRace->mRay->mRayTargetSelectors,
//                                   startPnt, endPnt, 1, true);

//     int vecSize = (int)(allHitTriangles.size());
//     std::vector<RayHitTriangleInfoStruct*>::iterator it;

//     if (vecSize < 1)
//         return false;

//     if (vecSize == 1) {
//         it = allHitTriangles.begin();
//         //only one target found, no shorting over distance necessary
//         shotTarget = *(*it);

//         //cleanup triangle hit information again
//         //otherwise we have a memory leak!
//         this->mRace->mRay->EmptyTriangleHitInfoVector(allHitTriangles);

//         return true;
//     }

//     //we have more then one triangle, we need to find the closest one to the player
//     irr::f32 minDistance;
//     bool firstElement = true;
//     irr::f32 currDist;
//     RayHitTriangleInfoStruct* nearestTriangleHit;

//     for (it = allHitTriangles.begin(); it != allHitTriangles.end(); ++it) {
//         currDist = (*it)->distFromRayStartSquared;
//         if (firstElement) {
//             firstElement = false;
//             minDistance = currDist;
//             nearestTriangleHit = (*it);
//         } else if (currDist < minDistance) {
//             minDistance = currDist;
//             nearestTriangleHit = (*it);
//         }
//     }

//     shotTarget = *nearestTriangleHit;

//     //cleanup triangle hit information again
//     //otherwise we have a memory leak!
//     this->mRace->mRay->EmptyTriangleHitInfoVector(allHitTriangles);

//     return true;
// }

Player::~Player() {
    //free memory of all player stats

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

    //delete mMovingAvgPlayerLeaningAngleLeftRightCalc;
    //delete mMovingAvgPlayerPositionCalc;
}

bool Player::AllAnimatorsDone() {
    //return (this->mMGun->AllAnimationsFinished());
    return true;
}

Player::Player(Race* race, std::string model, irr::core::vector3d<irr::f32> NewPosition,
               irr::core::vector3d<irr::f32> NewFrontAt,
               irr::u8 nrLaps, bool humanPlayer) {

    mRace = race;

    //definition of dirt texture elements
    dirtTexIdsVec = new std::vector<irr::s32>{0, 1, 2, 60, 61, 62, 63, 64, 65, 66, 67, 79};

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

    //CalcCraftLocalFeatureCoordinates(NewPosition, NewFrontAt);

    //create a moving average calculation helper for craft leaning angle left/right with average over
    //20 values
    //mMovingAvgPlayerLeaningAngleLeftRightCalc = new MovingAverageCalculator(MVG_AVG_TYPE_IRRFLOAT32, 20);

    //create a moving average calculation helper for craft position over 10 samples
    //mMovingAvgPlayerPositionCalc = new MovingAverageCalculator(MVG_AVG_TYPE_IRRCOREVECT3DF, 10);
}

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

/*void Player::LogMessage(char *msgTxt) {
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
}*/

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

void Player::SetTarget(Player* newTarget) {
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

void Player::DebugSelectionBox(bool boundingBoxVisible) {
    if (boundingBoxVisible) {
        this->Player_node->setDebugDataVisible(E_DEBUG_SCENE_TYPE::EDS_BBOX);
    } else {
        this->Player_node->setDebugDataVisible(E_DEBUG_SCENE_TYPE::EDS_OFF);
    }
}
