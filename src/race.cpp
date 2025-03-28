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

#include "race.h"

Race::Race(InfrastructureBase* infra,  Game* mParentGame, MyMusicStream* gameMusicPlayerParam,
           SoundEngine* soundEngine, int loadLevelNr, irr::u8 nrLaps, bool demoMode, bool skipStart,
           bool useAutoGenMiniMapParam) {
    this->mMusicPlayer = gameMusicPlayerParam;
    this->mSoundEngine = soundEngine;
    this->mDemoMode = demoMode;
    this->mInfra = infra;

    mRaceNumberOfLaps = nrLaps;

    if (skipStart) {
        this->mCurrentPhase = DEF_RACE_PHASE_RACING;
    } else {
        this->mCurrentPhase = DEF_RACE_PHASE_START;
    }

    levelNr = loadLevelNr;
    ready = false;
    useAutoGenMinimap = useAutoGenMiniMapParam;
    mGame = mParentGame;

    //IrrlichtStats("before Race constructor");

    //create the predefined axis direction vectors
    xAxisDirVector = new irr::core::vector3df(1.0f, 0.0f, 0.0f);
    yAxisDirVector = new irr::core::vector3df(0.0f, 1.0f, 0.0f);
    zAxisDirVector = new irr::core::vector3df(0.0f, 0.0f, 1.0f);

    //create empty checkpoint info vector
    checkPointVec = new std::vector<CheckPointInfoStruct*>;
    checkPointVec->clear();

    //create an empty waypoint link info vector
    wayPointLinkVec = new std::vector<WayPointLinkInfoStruct*>;
    wayPointLinkVec->clear();

    steamFountainVec = new std::vector<SteamFountain*>;
    steamFountainVec->clear();

    //my vector of recovery vehicles
    recoveryVec = new std::vector<Recovery*>;
    recoveryVec->clear();

    mCollectableSpawnerVec.clear();

    //my vector of player that need help
    //of a recovery vehicle and are currently waiting
    //for it
    mPlayerWaitForRecoveryVec = new std::vector<Player*>;
    mPlayerWaitForRecoveryVec->clear();

    //my vector of cones on the race track
    coneVec = new std::vector<Cone*>;
    coneVec->clear();

    mPlayerVec.clear();
    mPlayerPhysicObjVec.clear();
    playerRaceFinishedVec.clear();
    mTriggerRegionVec.clear();
    mPendingTriggerTargetGroups.clear();
    mTimerVec.clear();
    mExplosionEntityVec.clear();
    mType2CollectableForCleanupLater.clear();

    //for the start of the race we want to trigger
    //target group 1 once
    mPendingTriggerTargetGroups.push_back(1);
}

void Race::IrrlichtStats(char* text) {
      cout << "----- " << std::string(text) << "----- " << std::endl << std::flush;
      cout << "Mesh count loaded: " << mInfra->mSmgr->getMeshCache()->getMeshCount() << std::endl << std::flush;
      cout << "Textures loaded: " << mInfra->mSmgr->getVideoDriver()->getTextureCount() << std::endl << std::flush;
      core::array<scene::ISceneNode*> outNodes;

      //get list of all existing sceneNodes
      mInfra->mSmgr->getSceneNodesFromType(ESCENE_NODE_TYPE::ESNT_ANY, outNodes, 0);

      cout << "Scenenode count (all): " << outNodes.size() << std::endl << std::flush;
      outNodes.clear();

      //get list of all existing Billboard sceneNodes
      mInfra->mSmgr->getSceneNodesFromType(ESCENE_NODE_TYPE::ESNT_BILLBOARD, outNodes, 0);

      cout << "Scenenode count (Billboard): " << outNodes.size() << std::endl << std::flush;

      //get list of all existing Mesh sceneNodes
      mInfra->mSmgr->getSceneNodesFromType(ESCENE_NODE_TYPE::ESNT_MESH, outNodes, 0);

      cout << "Scenenode count (Mesh): " << outNodes.size() << std::endl << std::flush;

      outNodes.clear();

      //get list of all existing light sceneNodes
      mInfra->mSmgr->getSceneNodesFromType(ESCENE_NODE_TYPE::ESNT_LIGHT, outNodes, 0);

      cout << "Scenenode count (light): " << outNodes.size() << std::endl << std::flush;

      outNodes.clear();

      //get list of all existing camera sceneNodes
      mInfra->mSmgr->getSceneNodesFromType(ESCENE_NODE_TYPE::ESNT_CAMERA, outNodes, 0);

      cout << "Scenenode count (camera): " << outNodes.size() << std::endl << std::flush;

      outNodes.clear();

      irr::s32 texCnt;
      texCnt =  mInfra->mSmgr->getVideoDriver()->getTextureCount();

      return;

      //now we have our output filename
       char finalpath[50];

      strcpy(finalpath, text);
      strcat(finalpath, ".txt");

      FILE* oFile = fopen(finalpath, "w");
      if (oFile == NULL) {
         return;
      }

      for (int i = 0; i < texCnt; i++) {
        //cout << std::string(this->mSmgr->getVideoDriver()->getTextureByIndex(i)->getName().getInternalName().c_str()) << std::endl << std::flush;
         fprintf(oFile, "%s\n", mInfra->mSmgr->getVideoDriver()->getTextureByIndex(i)->getName().getInternalName().c_str());
      }

      //close file
      fclose(oFile);
}

Race::~Race() {
    //delete my axis direction vectors
    delete xAxisDirVector;
    delete yAxisDirVector;
    delete zAxisDirVector;

    //unregister existing HUD in all players
    std::vector<Player*>::iterator it;

    for (it = this->mPlayerVec.begin(); it != this->mPlayerVec.end(); ++it) {
        (*it)->SetMyHUD(NULL);
    }

    //now we can free the HUD
    delete Hud1Player;

    //delete physics and all
    //linked physics Objects
    //also the PhysicObjects are deleted here
    //therefore we MUST NOT delete them here
    //again! otherwise we corrupt memory
    delete mPhysics;

    //free all players
    Player* playerPntr;

    for (it = this->mPlayerVec.begin(); it != this->mPlayerVec.end();) {
        playerPntr = (*it);

        it = mPlayerVec.erase(it);

        delete playerPntr;
    }

    //remove camera SceneNode
    mCamera->remove();

    //TODO: ExplosionLauncher does not have
    //a deconstructor, does not clean up inside!
    delete mExplosionLauncher;

    delete mWorldAware;

    //clean collision mesh and SceneNodes
    //remove Scenenode
    wallCollisionMeshSceneNode->remove();

    //free the mesh
    mInfra->mSmgr->getMeshCache()->removeMesh(wallCollisionMesh);

    delete mPath;

    CleanUpMorphs();
    CleanUpSteamFountains();
    CleanUpCollectableSpawners();
    CleanUpEntities();

    CleanUpRecoveryVehicles();
    CleanUpCones();

    CleanUpWayPointLinks(*this->wayPointLinkVec);
    CleanUpAllCheckpoints();
    CleanUpSky();
    CleanMiniMap();
    CleanUpTriggers();
    CleanUpTimers();
    CleanUpCameras();
    CleanUpExplosionEntities();

    //free lowlevel level data
    delete mLevelBlocks;
    delete mLevelTerrain;
    delete mLevelRes;

    delete mDrawDebug;

    //free all loaded textures
    delete mTexLoader;

    //IrrlichtStats("After race destructor");
}

void Race::CleanUpMorphs() {
    if (Morphs.size() > 0) {
        std::list<Morph*>::iterator it;
        Morph* pntr;
        for (it = Morphs.begin(); it != Morphs.end(); ) {
            pntr = (*it);
            it = Morphs.erase(it);

            //delete Morph itself
            delete pntr;
        }
    }
}

void Race::CleanUpEntities() {

   if (ENTWallsegmentsLine_List->size() > 0) {
       std::vector<LineStruct*>::iterator it;
       LineStruct* pntr;
       for (it = ENTWallsegmentsLine_List->begin(); it != ENTWallsegmentsLine_List->end(); ) {
           pntr = (LineStruct*)(*it);
           it = ENTWallsegmentsLine_List->erase(it);

           //free name inside LineStruct
           delete[] pntr->name;

           //delete LineStruct itself
           delete pntr;
       }
   }

   delete ENTWallsegmentsLine_List;
   ENTWallsegmentsLine_List = NULL;

   if (ENTWaypoints_List->size() > 0) {
       std::vector<EntityItem*>::iterator it;

       for (it = ENTWaypoints_List->begin(); it != ENTWaypoints_List->end(); ) {
           it = ENTWaypoints_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTWaypoints_List;
   ENTWaypoints_List = NULL;

   if (ENTWallsegments_List->size() > 0) {
       std::list<EntityItem*>::iterator it;

       for (it = ENTWallsegments_List->begin(); it != ENTWallsegments_List->end(); ) {
           it = ENTWallsegments_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTWallsegments_List;
   ENTWallsegments_List = NULL;

   if (ENTTriggers_List->size() > 0) {
       std::list<EntityItem*>::iterator it;

       for (it = ENTTriggers_List->begin(); it != ENTTriggers_List->end(); ) {
           it = ENTTriggers_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTTriggers_List;
   ENTTriggers_List = NULL;

   if (ENTCollectablesVec->size() > 0) {
       std::vector<Collectable*>::iterator it;
       Collectable* pntr;
       for (it = ENTCollectablesVec->begin(); it != ENTCollectablesVec->end(); ) {
           pntr = (Collectable*)(*it);
           it = ENTCollectablesVec->erase(it);

           //delete Collectable itself
           //this frees SceneNode and texture inside
           //collectable implementation
           delete pntr;
       }
   }

   delete ENTCollectablesVec;
   ENTCollectablesVec = NULL;

   //delete remaining type2 collectable items
   //which were dynamically spawned before
   if (mType2CollectableForCleanupLater.size() > 0) {
       std::vector<Collectable*>::iterator it;
       Collectable* pntr;
       for (it = mType2CollectableForCleanupLater.begin(); it != mType2CollectableForCleanupLater.end(); ) {
           pntr = (Collectable*)(*it);
           it = mType2CollectableForCleanupLater.erase(it);

           //delete Collectable itself
           //this frees SceneNode and texture inside
           //collectable implementation
           delete pntr;
       }
   }
}

void Race::CleanUpSteamFountains() {
    std::vector<SteamFountain*>::iterator it;
    SteamFountain* pntr;

    if (steamFountainVec->size() > 0) {
        for (it = steamFountainVec->begin(); it != steamFountainVec->end(); ) {
            pntr = (*it);

            it = steamFountainVec->erase(it);

            //delete the steamFountain
            delete pntr;
        }
    }

    //delete also the vector itself
    delete steamFountainVec;
    steamFountainVec = NULL;
}

void Race::CleanUpRecoveryVehicles() {
    std::vector<Recovery*>::iterator it;
    Recovery* pntr;

    if (recoveryVec->size() > 0) {
        for (it = recoveryVec->begin(); it != recoveryVec->end(); ) {
            pntr = (*it);

            it = recoveryVec->erase(it);

            //delete the recovery vehicle
            delete pntr;
        }
    }

    //delete also the vector itself
    delete recoveryVec;
    recoveryVec = NULL;

    //clean up also waiting list for recovery
    delete mPlayerWaitForRecoveryVec;
    mPlayerWaitForRecoveryVec = NULL;
}

void Race::CallRecoveryVehicleForHelp(Player *whichPlayer) {
    this->mPlayerWaitForRecoveryVec->push_back(whichPlayer);
}


//Code below was never used, not sure if it works
//at one point in time I thought I need it, but later it turned out I do not need it right now
//void Race::SetPlayerLocationAndAlignToTrackHeight(Player* player, irr::core::vector3df newLocation,
//                                                  irr::core::vector3df newFrontDirVec) {

//    //move the player to the new location
//    player->phobj->physicState.position = newLocation + irr::core::vector3df(0.0f, HOVER_HEIGHT, 0.0f);
//    player->Player_node->setPosition(newLocation + irr::core::vector3df(0.0f, HOVER_HEIGHT, 0.0f));

//    irr::f32 hFront;
//    irr::f32 hBack;
//    irr::f32 hLeft;
//    irr::f32 hRight;

//    //based on this calculate terrain angles at this location
//    player->GetHeightRaceTrackBelowCraft(hFront, hBack, hLeft, hRight);

//    //using this tilt rotate player model
//    irr::core::vector3df currRotation;
//    currRotation = player->Player_node->getRotation();

//    player->Player_node->setRotation(irr::core::vector3df(-player->terrainTiltCraftLeftRightDeg,
//                                                        irr::core::radToDeg(currRotation.Y), -player->terrainTiltCraftFrontBackDeg));

// /*   irr::core::vector3df pos_in_worldspace_originPos(player->LocalCraftOrigin);
//    player->Player_node->updateAbsolutePosition();
//    irr::core::matrix4 matr = player->Player_node->getAbsoluteTransformation();

//    matr.transformVect(pos_in_worldspace_originPos);

//    irr::core::vector2di outCellOrigin;
//    irr::f32 origin = mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(
//                pos_in_worldspace_originPos.X,
//                pos_in_worldspace_originPos.Z,
//                outCellOrigin);

//    pos_in_worldspace_originPos.Y = origin + HOVER_HEIGHT;

//    player->Player_node->setPosition(pos_in_worldspace_originPos);
//    player->phobj->physicState.position = pos_in_worldspace_originPos;
//*/
//    player->phobj->physicState.orientation =
//                   player->GetQuaternionFromPlayerModelRotation();

//    //make sure physics model velocities are zeroed out
//    //because we move player somewhere else, the history does not matter anymore
//    player->phobj->physicState.momentum = irr::core::vector3df(0.0f, 0.0f, 0.0f);
//    player->phobj->physicState.angularMomentum = irr::core::vector3df(0.0f, 0.0f, 0.0f);

//    //update all other physic states as well, this will also
//    //set velocity to zero
//    player->phobj->physicState.recalculate();
//}

void Race::UpdateRecoveryVehicles(irr::f32 deltaTime) {
    //does at least one player need help?
    if (mPlayerWaitForRecoveryVec->size() > 0) {
        vector< pair <irr::f32, Recovery*> > vecAvailRecoveryVehicles;

        std::vector<Player*>::iterator itPlayer;
        //go through all players that are currently waiting for help
        for (itPlayer = mPlayerWaitForRecoveryVec->begin(); itPlayer != mPlayerWaitForRecoveryVec->end(); ) {
            //yes, search the nearest recovery vehicle that is available
            //right now

            std::vector<Recovery*>::iterator it;
            irr::f32 distance;

            vecAvailRecoveryVehicles.clear();

            for (it = this->recoveryVec->begin(); it != this->recoveryVec->end(); ++it) {
                //currently available?
                if ((*it)->CurrentlyReadyforMission()) {
                    //yes it is, calculate distance between player and recovery vehicle
                    //we want to call the closest one
                    distance = ((*itPlayer)->phobj->physicState.position - (*it)->GetCurrentPosition()).getLength();

                    vecAvailRecoveryVehicles.push_back( make_pair(distance, (*it)));
                }
            }

            //if there is at least one recovery vehicle available sort them by descending distance,
            //and select one for this player, otherwise do nothing
            if (vecAvailRecoveryVehicles.size() > 0) {
                //sort vector pairs in descending value of distance
               std::sort(vecAvailRecoveryVehicles.rbegin(), vecAvailRecoveryVehicles.rend());

               //start with the last element in sorted vector (which is the closest recovery vehicle
               //to this player)
               auto it4 = vecAvailRecoveryVehicles.rbegin();

               //now command the selected recovery vehicle to help this player
               (*it4).second->SentToRepairMission(*itPlayer);

               //delete this player from the waiting list
               itPlayer = this->mPlayerWaitForRecoveryVec->erase(itPlayer);
            } else {
                //no more recovery vehicles available
                //we can stop to look
                break;
            }

            //more recovery vehicles available
            //search one for the next player in need
     }
   }

   //update all available recovery vehicles
   std::vector<Recovery*>::iterator it;

   for (it = this->recoveryVec->begin(); it != this->recoveryVec->end(); ++it) {
       (*it)->Update(deltaTime);
   }
}

void Race::CleanUpCones() {
    std::vector<Cone*>::iterator it;
    Cone* pntr;

    if (coneVec->size() > 0) {
        for (it = coneVec->begin(); it != coneVec->end(); ) {
            pntr = (*it);

            it = coneVec->erase(it);

            //delete the cone
            delete pntr;
        }
    }

    //delete also the vector itself
    delete coneVec;
    coneVec = NULL;
}

void Race::CleanUpTimers() {
    std::vector<Timer*>::iterator it;
    Timer* pntr;

    if (mTimerVec.size() > 0) {
        for (it = mTimerVec.begin(); it != mTimerVec.end(); ) {
            pntr = (*it);

            it = mTimerVec.erase(it);

            //delete the timer as well
            delete pntr;
        }
    }
}

void Race::CleanUpCameras() {
    std::vector<Camera*>::iterator it;
    Camera* pntr;

    if (mCameraVec.size() > 0) {
        for (it = mCameraVec.begin(); it != mCameraVec.end(); ) {
            pntr = (*it);

            it = mCameraVec.erase(it);

            //delete the camera as well
            delete pntr;
        }
    }
}

void Race::CleanUpExplosionEntities() {
    std::vector<ExplosionEntity*>::iterator it;
    ExplosionEntity* pntr;

    if (mExplosionEntityVec.size() > 0) {
        for (it = mExplosionEntityVec.begin(); it != mExplosionEntityVec.end(); ) {
            pntr = (*it);

            it = mExplosionEntityVec.erase(it);

            //delete the explosion entity as well
            delete pntr;
        }
    }
}

void Race::CleanUpTriggers() {
    std::vector<MapTileRegionStruct*>::iterator it;
    MapTileRegionStruct* pntr;

    if (mTriggerRegionVec.size() > 0) {
        for (it = mTriggerRegionVec.begin(); it != mTriggerRegionVec.end(); ) {
            pntr = (*it);

            it = mTriggerRegionVec.erase(it);

            //delete the MapTileRegionStruct
            delete pntr;
        }
    }
}

void Race::StopMusic() {
    if ((mMusicPlayer->getStatus() == mMusicPlayer->Playing) ||
       (mMusicPlayer->getStatus() == mMusicPlayer->Paused)) {
            //stop music
            mMusicPlayer->StopPlay();
    }
}

void Race::StopAllSounds() {
    //make sure all warning sounds are off
    std::vector<Player*>::iterator itPlayer;

    for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
        (*itPlayer)->StopPlayingWarningSound();
    }

    //make sure engine sounds are off
    //this function internally also stops
    //engine sound
    mSoundEngine->StopAllSounds();
}

bool Race::GetWasRaceFinished() {
    return mRaceWasFinished;
}

//ends the current race
void Race::End() {
    //stop music and all sounds
    StopMusic();
    StopAllSounds();
}

//attacker is the enemy player that does damage the player targetToHit
//for damage that an entity does cause (for example steamFountain) attacker is set
//to NULL
void Race::DamagePlayer(Player* targetToHit, irr::f32 damageVal, irr::u8 damageType, Player* attacker) {
    bool targetDied;

    if (targetToHit != NULL) {
        targetDied = targetToHit->Damage(damageVal, damageType);

        //if the attacked/damaged player died let the player ship explode
        //if there was an attacker increase its kill counter
        //spawn entities in the level from the target
        if (targetDied) {

            if (attacker != NULL) {
                //increase kill count of attacker
                attacker->mPlayerStats->currKillCount++;

                char killMessage[80];

                strcpy(killMessage, "KILLED BY ");
                strcat(killMessage, attacker->mPlayerStats->name);

                //show player that died a message in HUD, which other
                //player was the attacker, is a permanent message, and not blinking
                targetToHit->ShowPlayerBigGreenHudText(killMessage, -1.0f, false);
            }

            //trigger explosion at location of killed player
            this->mExplosionLauncher->Trigger(targetToHit->phobj->physicState.position);

            //spawn collectibles at location of killed player
            SpawnCollectiblesForPlayer(targetToHit);
        }
    }
}

void Race::DeliverMusicFileName(unsigned int levelNr, char *musicFileName) {
    switch (levelNr) {
    case 1: {
        strcpy(musicFileName, "extract/music/TGAME1.XMI");
        break;
    }
    case 2: {
        strcpy(musicFileName, "extract/music/TGAME2.XMI");
        break;
    }
    case 3: {
        strcpy(musicFileName, "extract/music/TGAME3.XMI");
        break;
    }
    case 4: {
        strcpy(musicFileName, "extract/music/TGAME4.XMI");
        break;
    }
    case 5: {
        strcpy(musicFileName, "extract/music/TGAME1.XMI");
        break;
    }
    case 6: {
        strcpy(musicFileName, "extract/music/TGAME2.XMI");
        break;
    }
    case 7: {
        strcpy(musicFileName, "extract/music/TGAME3.XMI");
        break;
    }
    case 8: {
        strcpy(musicFileName, "extract/music/TGAME4.XMI");
        break;
    }
    case 9: {
        strcpy(musicFileName, "extract/music/TGAME1.XMI");
        break;
    }
    default: {
        strcpy(musicFileName, "extract/music/TGAME1.XMI");
        break;
    }
  }
}

//Stage 3 of player race ranking sorting: Sort by ascending remaining distance to next checkpoint
void Race::UpdatePlayerRacePositionRankingHelper3(vector< pair <irr::f32, Player*> > vecRemainingDistanceToNextCheckpoint) {

    //sort vector pairs in descending value for remaining distance to next checkpoint
   std::sort(vecRemainingDistanceToNextCheckpoint.rbegin(), vecRemainingDistanceToNextCheckpoint.rend());

   //start with the last element in sorted vector (which is the player with the
   //least remaining distance to next checkpoint
   //this player is currently number one player
   auto it4 = vecRemainingDistanceToNextCheckpoint.rbegin();

   for (it4 = vecRemainingDistanceToNextCheckpoint.rbegin(); it4 != vecRemainingDistanceToNextCheckpoint.rend(); ++it4) {
       playerRanking.push_back((*it4).second);
   }
}

//Stage 2 of player race ranking sorting: Sort by ascending number of next expected checkpoint value
void Race::UpdatePlayerRacePositionRankingHelper2(vector< pair <irr::s32, Player*> > vecNextCheckPointExpected) {

    //sort vector pairs in ascending value of next expected checkpoint
    std::sort(vecNextCheckPointExpected.begin(), vecNextCheckPointExpected.end());

    //start with the last element in sorted vector (which is the player with the highest value
    //for next expected checkpoint)
    auto it3 = vecNextCheckPointExpected.rbegin();

    irr::s32 nextExpectedCheckPointValue;
    Player* playerPntr;

    nextExpectedCheckPointValue = (it3)->first;
    playerPntr = (it3)->second;

    vector< pair <irr::f32, Player*> > vecRemainingDistanceToNextCheckpoint;

    for (it3 = vecNextCheckPointExpected.rbegin(); it3 != vecNextCheckPointExpected.rend(); ++it3) {

        playerPntr = (it3)->second;

        //does the next player have the same value for next expected checkpoint?
        if ((*it3).first == nextExpectedCheckPointValue) {
            //yes, add it to the next sorting list for remaining distance to next checkpoint
            vecRemainingDistanceToNextCheckpoint.push_back( make_pair(playerPntr->remainingDistanceToNextCheckPoint, playerPntr));
        } else {
            //the next player has a different expected next checkpoint
            //go to the next sorting stage
            UpdatePlayerRacePositionRankingHelper3(vecRemainingDistanceToNextCheckpoint);

            vecRemainingDistanceToNextCheckpoint.clear();
            vecRemainingDistanceToNextCheckpoint.push_back( make_pair(playerPntr->remainingDistanceToNextCheckPoint, playerPntr));
        }

        nextExpectedCheckPointValue = (it3)->first;
    }

    if (vecRemainingDistanceToNextCheckpoint.size() > 0) {
        UpdatePlayerRacePositionRankingHelper3(vecRemainingDistanceToNextCheckpoint);
    }
}

//if the first player crosses the finish line after start
//the race state changes to final Racing state
void Race::PlayerCrossesFinishLineTheFirstTime() {
    //only execute this code if we are still in the pre-racing state
    //and the start itself was not skipped (for example in game debugging mode)
    if (mCurrentPhase == DEF_RACE_PHASE_FIRSTWAYTOWARDSFINISHLINE) {
        mCurrentPhase = DEF_RACE_PHASE_RACING;

        //also set the players to this new mode
        //this internally enables the HUD drawing, and allows computer players to
        //finally attack
        std::vector<Player*>::iterator it;

        for (it = this->mPlayerVec.begin(); it != this->mPlayerVec.end(); ++it) {
            (*it)->SetupToSkipStart();
        }
    }
}

void Race::AddPlayer(bool humanPlayer, char* name, std::string player_model) {
    Player* newPlayer;

    //***************************************************
    // Create and add a new player to the race          *
    //***************************************************

    //already more players then available starting locations in map
    //if so simply exit
    if (mPlayerVec.size() >=  mPlayerStartLocations.size())
        return;

    //for the start just get hardcoded starting positions for the player
    irr::core::vector3d<irr::f32> Startpos;
    irr::core::vector3d<irr::f32> Startdirection;

    //get new player map start location
    Startpos = mPlayerStartLocations.at(mPlayerVec.size());

    Startdirection.X = Startpos.X;
    Startdirection.Y = Startpos.Y;

    //the start direction for the player is simply derived by subtracting
    //from the Z position of the player origin; This is only possible because in HiOctane
    //all level start locations always go in the same direction; There does not seem to be
    //any way to change/set the start direction in the level file; There is only a start
    //point location
    Startdirection.Z = Startpos.Z - 1.0f; //attempt beginning from 04.09.2024

    //create the new player
    newPlayer = new Player(this, mInfra, player_model, Startpos, Startdirection,
                          this->mRaceNumberOfLaps, humanPlayer);

    //Setup physics for new player, we handover pointer to Irrlicht
    //player node, as the node (3D model) is now fully controlled
    //by physics
    this->mPhysics->AddObject(newPlayer->Player_node);

    //retrieve a pointer to the player physics object that the physics code has
    //created for me, we need this pointer to get access to get player info/control cameras, etc...
    PhysicsObject* newPlayerPhysicsObj = mPhysics->GetObjectPntr(newPlayer->Player_node);

    //setup player physic properties
    //give the computer player slightly different values
    //for optimization
    if (newPlayerPhysicsObj != NULL) {
        SetupPhysicsObjectParameters(*newPlayerPhysicsObj, humanPlayer);

        newPlayerPhysicsObj->physicState.position = Startpos;
        newPlayerPhysicsObj->physicState.momentum = {0.0f, 0.0f, 0.0f};
        newPlayerPhysicsObj->physicState.orientation.set(irr::core::vector3df(0.0f, 0.0f, 0.0f));

        newPlayerPhysicsObj->physicState.recalculate();
        newPlayerPhysicsObj->SetAirFriction(CRAFT_AIRFRICTION_NOTURBO);
    }

    //give the player a pointer to its physics object
    newPlayer->SetPlayerObject(newPlayerPhysicsObj);
    newPlayer->SetName(name);

    //add new player to the vector of available players
    //the same is true for the player physics object
    mPlayerVec.push_back(newPlayer);
    mPlayerPhysicObjVec.push_back(newPlayerPhysicsObj);

    if (!humanPlayer) {
            //for a computer player additional add first player command
            EntityItem* entItem = this->mPath->FindFirstWayPointAfterRaceStartPoint();
            if (entItem != NULL) {
               //get waypoint link for this waypoint
                std::vector<WayPointLinkInfoStruct*> foundLinks;

               foundLinks = this->mPath->FindWaypointLinksForWayPoint(entItem, true, false, NULL);

               if (foundLinks.size() > 0) {
                   newPlayer->AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, foundLinks.at(0));

                   //we also want to preset the waypoint link follow logic offset
                   //to prevent collision of all the player craft at the tight start situation
                   //preset by X coordinate difference between player origin and found waypoint link start entity
                   irr::core::vector3df startWayPoint = foundLinks.at(0)->pStartEntity->getCenter();

                   irr::core::vector3df deltaVec = Startpos - startWayPoint;
                   //deltaVec.Y = 0.0f;
                   //deltaVec.Z = 0.0f;

                   newPlayer->mCpCurrPathOffset = -deltaVec.X;
               }
            }
    }

    //if we do not skip start set player mode
    //accordingly; this also sets the Hud view mode
    //correctly via player state
    if (this->mCurrentPhase == DEF_RACE_PHASE_START) {
        newPlayer->SetupForStart();
    } else if (mCurrentPhase == DEF_RACE_PHASE_RACING) {
        newPlayer->SetupToSkipStart();
    }

    //request new engine sound for new player
    this->mSoundEngine->RequestEngineSoundForPlayer(newPlayer);

    //start engine sound for new player
    this->mSoundEngine->StartEngineSoundForPlayer(newPlayer);
}

void Race::SetupPhysicsObjectParameters(PhysicsObject &phyObj, bool humanPlayer) {
    if (humanPlayer) {
        phyObj.physicState.SetMass(3.0f);
        phyObj.physicState.SetInertia(30.0f);

        //best value for human player to have best
        //player craft handling
        phyObj.mRotationalFrictionVal = 50.1f;
    } else {
       //best values until 30.12.2024
       phyObj.physicState.SetMass(5.0f);
       phyObj.physicState.SetInertia(30.0f);

       //this value is necessary for computer controlled craft,
       //to stabilizie it against unwanted sideway movements and
       //"oscillations"
       //but because this will is too much to control the craft during
       //steep turns, we will dynamically set it depending on the angle error
       //in turns in the player class code
       phyObj.mRotationalFrictionVal = CP_PLAYER_ANGULAR_DAMPINGMAX;
    }
}

//Ranks the active players in order of their race progress
//Number laps finished // next expected checkpoint number // remaining distance to next checkpoint
void Race::UpdatePlayerRacePositionRanking() {
    //first put the players with the highest amount
    //of finished laps at the top, with decreasing sorting order

    playerRanking.clear();

    std::vector<Player*>::iterator it;

    /****************************************************
     * Stage 1: Sort players by number of laps finished
     *  Important: only look at players that have not
     *  yet finished the race
     ****************************************************/

    //declaring vector of pairs containing number laps finished
    //and pointer to player
    vector< pair <irr::s32, Player*> > vecLapsFinished;

    for (it = mPlayerVec.begin(); it != mPlayerVec.end(); ++it) {
        if ((*it)->mPlayerStats->mPlayerCurrentState != STATE_PLAYER_FINISHED) {
          vecLapsFinished.push_back( make_pair((*it)->mPlayerStats->currLapNumber, (*it)));
        }
    }

    //if all players are finished, just exit
   if (vecLapsFinished.size() < 1)
       return;

    //sort vector pairs in ascending current lap number
   std::sort(vecLapsFinished.begin(), vecLapsFinished.end());

   //start with player with highest lap number
   auto it2 = vecLapsFinished.rbegin();

   irr::s32 currLapNr;
   vector< pair <irr::s32, Player*> > vecNextCheckPointExpected;
   vector< pair <irr::f32, Player*> > vecRemainingDistanceToNextCheckpoint;
   Player* playerPntr;

   currLapNr = (it2)->first;
   playerPntr = (it2)->second;
   int nextCheckPointValueHelper;

   for (it2 = vecLapsFinished.rbegin(); it2 != vecLapsFinished.rend(); ++it2) {

       playerPntr = (it2)->second;

       //is the next player in the same current lap number?
       if ((*it2).first == currLapNr) {
          //yes, add it to the next sorting list for next expected check point
          //18.01.2025: we need to keep something in mind to not get wrong results:
          //nextCheckPointValue inside player rolls over at the end of the lap in front of the
          //finish line back to 0, and start counting upwards again; That means during the race (after finish
          //line was first passed by the player, the value 0 for nextCheckPointValue means actually more progress
          //for the player then the highest possible check point value for this race track. If the player has passed
          //the finish line already or not, is stored in player variable lastCrossedCheckPointValue
          nextCheckPointValueHelper = playerPntr->nextCheckPointValue;

          //is the next expected checkpoint 0?
          if (nextCheckPointValueHelper == 0) {
              //has the player already at least one time crossed the finish line?
              if (playerPntr->lastCrossedCheckPointValue != 0) {
                  //yes, player crossed finish line at least once
                  //this means a next checkpoint value of 0 means actually more race progress then
                  //all higher numbers, fix nextCheckPointValue for the race progress sorting but just setting
                  //the number of next expected checkpoint higher
                  nextCheckPointValueHelper = (int)(this->checkPointVec->size());
              }
          }

          vecNextCheckPointExpected.push_back( make_pair(nextCheckPointValueHelper, playerPntr));
       } else {
           //the next player has not so much laps done yet
           //go to the next sorting stage
            UpdatePlayerRacePositionRankingHelper2(vecNextCheckPointExpected);

            vecNextCheckPointExpected.clear();

            nextCheckPointValueHelper = playerPntr->nextCheckPointValue;

            //is the next expected checkpoint 0?
            if (nextCheckPointValueHelper == 0) {
                //has the player already at least one time crossed the finish line?
                if (playerPntr->lastCrossedCheckPointValue != 0) {
                    //yes, player crossed finish line at least once
                    //this means a next checkpoint value of 0 means actually more race progress then
                    //all higher numbers, fix nextCheckPointValue for the race progress sorting but just setting
                    //the number of next expected checkpoint higher
                    nextCheckPointValueHelper = (int)(this->checkPointVec->size());
                }
            }

            vecNextCheckPointExpected.push_back( make_pair(nextCheckPointValueHelper, playerPntr));
       }

       currLapNr = (it2)->first;
   }

   if (vecNextCheckPointExpected.size() > 0) {
       UpdatePlayerRacePositionRankingHelper2(vecNextCheckPointExpected);
   }

   //what is the current overall position the players
   //are still racing for?
   int currPos = (int)(playerRaceFinishedVec.size()) + 1;

   int numPlayers = (int)(mPlayerVec.size());

   //all player rankings are done and stored in this->playerRanking
   //Update player objects with this new ranking information
   for (it = this->playerRanking.begin(); it != this->playerRanking.end(); ++it) {
       (*it)->mPlayerStats->currRacePlayerPos = currPos;
       (*it)->mPlayerStats->overallPlayerNumber = numPlayers;
       currPos++;
   }
}

void Race::DebugResetColorAllWayPointLinksToWhite() {
    std::vector<WayPointLinkInfoStruct*>::iterator it;

    for (it = this->wayPointLinkVec->begin(); it != this->wayPointLinkVec->end(); ++it) {
        (*it)->pLineStruct->color = mDrawDebug->white;
    }
}

void Race::UpdatePlayerDistanceToNextCheckpoint(Player* whichPlayer) {
    irr::f32 sumDistance = 0.0f;
    WayPointLinkInfoStruct* currLink;
    irr::f32 len;
    irr::f32 partLen;
    CheckPointInfoStruct* pntrChkPoint;

    //only for debugging!
    //DebugResetColorAllWayPointLinksToWhite();

    //start at current waypoint link closest to current player
    //then follow this link forward until we hit the next checkpoint
    //create sum of all distances
    if (whichPlayer->currClosestWayPointLink.first != NULL) {
        //we have currently a closest waypoint link for this player
        currLink = whichPlayer->currClosestWayPointLink.first;

        //The next line is for debugging
        //currLink->pLineStruct->color = mDrawDebug->green;

        //calculate length of vector from current 3D Player projected position on waypoint line to start of line
        len = (whichPlayer->projPlayerPositionClosestWayPointLink - currLink->pLineStruct->A).getLength();

        //first part of the distance is the part from player position
        //on current waypoint link to end of this waypoint link
        partLen = currLink->length3D - len;

        if (partLen < 0.0f)
            partLen = 0.0f;

        sumDistance += partLen;

        //already a checkpoint at the end of this waypoint link which we are currently
        //in, and we are distance wise before the expected waypoint?
        //if we are progress wise already after the checkpoint location, continue search for next
        //checkpoint
        if ((currLink->pntrCheckPoint != NULL) && (len < currLink->distanceStartLinkToCheckpoint)) {
            //yes, exit here
            whichPlayer->remainingDistanceToNextCheckPoint = sumDistance;

            return;
        }

        //sometimes the checkpoint is placed in the level maps exactly at a waypoint, so at the end I get
        //two waypoint links one after another with a pointer to the same checkpoint; This would through the distance
        //calculation of. To fix this issue we need to continue following waypoint links until we first find a "gap" of
        //waypoint links without any checkpoint there. Only after the we can be sure that we found the next (different)
        //waypoint

        pntrChkPoint = currLink->pntrCheckPoint;
        bool cont = true;

        //we need to look at the next following waypoint link
        currLink = currLink->pntrPathNextLink;

        //now follow the waypoint l whichPlayer->mLeave = 1;inks forward until we hit the next (but different) checkpoint
        while (cont) {  //follow one link after another until we hit the next checkpoint
            if (currLink != NULL) {
                if (currLink->pntrCheckPoint == NULL) {
                    //The next line is for debugging
                    //currLink->pLineStruct->color = mDrawDebug->blue;

                    //for this links add up the whole length
                    sumDistance += currLink->length3D;
                } else {
                    //there is a (different then the last) checkpoint within this waypoint link
                    //for this one add only the distance from the start point until the checkpoint location
                    sumDistance += currLink->distanceStartLinkToCheckpoint;
                    pntrChkPoint = currLink->pntrCheckPoint;

                    //currLink->pLineStruct->color = mDrawDebug->red;
                    cont = false;
                }
            } else {
                cont = false;
            }

               currLink = currLink->pntrPathNextLink;
        }

        //set currently remaining distance from player location to next checkpoint
        //into the player object
        whichPlayer->remainingDistanceToNextCheckPoint = sumDistance;
    }
}

void Race::removePlayerTest() {
//    HUD* plHUD;
//    //get possible pointer from player than an HUD
//    //if no HUD connection is there we will get NULL
//    plHUD = player2->GetMyHUD();

//    //player has an HUD attached
//    if (plHUD != NULL) {
//        //tell HUD to stop monitoring player we want to remove
//        plHUD->SetMonitorWhichPlayer(NULL);

//        //remove HUD pnter also from player object
//        //we want to remove
//        player2->SetMyHUD(NULL);
//    }

//    //remove Player2 from physics
//    mPhysics->RemoveObject(player2->Player_node);

//    //reset pointer in player to physics-object
//    player2->SetPlayerObject(NULL);

//    //remove Scenenode from Irrlicht SceneManager
//    player2->Player_node->remove();

//    delete player2;

//    player2Removed = true;
}

void Race::CleanupRaceStatistics(std::vector<RaceStatsEntryStruct*>* pntr) {
     std::vector<RaceStatsEntryStruct*>::iterator it;

     RaceStatsEntryStruct* pntrStruct;

     if (pntr->size() > 0) {
         for (it = pntr->begin(); it != pntr->end(); ) {
             pntrStruct = (*it);

             it = pntr->erase(it);

             delete  pntrStruct;
         }
     }
}

void Race::PlayerHasFinishedLastLapOfRace(Player *whichPlayer) {
    if (whichPlayer != NULL) {
        this->playerRaceFinishedVec.push_back(whichPlayer);
    }
}

//helper function which creates and returns the final race statistics
std::vector<RaceStatsEntryStruct*>* Race::RetrieveFinalRaceStatistics() {
    std::vector<RaceStatsEntryStruct*>* result = new std::vector<RaceStatsEntryStruct*>;

    result->clear();

    std::vector<Player*>::iterator itPlayer;
    std::vector <LAPTIMEENTRY>::iterator itLap;
    irr::u32 sumLapTimes;
    bool firstLapTime;
    irr::u16 minLapTime;

    for (itPlayer = this->mPlayerVec.begin(); itPlayer != this->mPlayerVec.end(); ++itPlayer) {
          RaceStatsEntryStruct* newEntry = new RaceStatsEntryStruct();
          firstLapTime = true;
          sumLapTimes = 0;

          //process lap time data
          for (itLap = (*itPlayer)->mFinalPlayerStats->lapTimeList.begin(); itLap != (*itPlayer)->mFinalPlayerStats->lapTimeList.end(); ++itLap) {
              sumLapTimes += (*itLap).lapTimeMultiple40mSec;

              if (firstLapTime) {
                  firstLapTime = false;
                  minLapTime = (*itLap).lapTimeMultiple40mSec;
              } else {
                  if ((*itLap).lapTimeMultiple40mSec < minLapTime) {
                      minLapTime = (*itLap).lapTimeMultiple40mSec;
                  }
              }
          }

          strcpy(newEntry->playerName, (*itPlayer)->mFinalPlayerStats->name);
          newEntry->nrKills = (*itPlayer)->mFinalPlayerStats->currKillCount;
          newEntry->nrDeaths = (*itPlayer)->mFinalPlayerStats->currDeathCount;
          newEntry->raceTime = sumLapTimes;
          newEntry->bestLapTime = minLapTime;

          irr::f32 avgLapTime = (irr::f32)(sumLapTimes) / (irr::f32)((*itPlayer)->mFinalPlayerStats->lapTimeList.size());
          newEntry->avgLapTime = (irr::u16)(avgLapTime);
          newEntry->racePosition = (*itPlayer)->mFinalPlayerStats->currRacePlayerPos;

          irr::u32 nrShootsfired = (*itPlayer)->mFinalPlayerStats->shootsHit + (*itPlayer)->mFinalPlayerStats->shootsMissed;
          irr::f32 accuracy;

          if (nrShootsfired > 0) {
                    accuracy = ((irr::f32)((*itPlayer)->mFinalPlayerStats->shootsHit) / (irr::f32)(nrShootsfired)) * 100.0f;
          } else {
              accuracy = 0.0f;
          }

          newEntry->hitAccuracy = (irr::u8)(accuracy);

          //plausi check
          if (newEntry->hitAccuracy < 0)
              newEntry->hitAccuracy = 0;

          if (newEntry->hitAccuracy > 100)
              newEntry->hitAccuracy = 100;

          //TODO: calculate later!
          //rating goes from lowest 1 (worst) up to
          //20 (best player)
          newEntry->rating = 1;

          result->push_back(newEntry);
    }

    return (result);
}

irr::core::dimension2di Race::CalcPlayerMiniMapPosition(Player* whichPlayer) {
    irr::core::dimension2di miniMapLocation;

    irr::f32 posPlWidth = miniMapPixelPerCellW * (irr::f32)(whichPlayer->mCurrPosCellX - this->miniMapStartW);
    irr::f32 posPlHeight = miniMapPixelPerCellH * (irr::f32)(whichPlayer->mCurrPosCellY - this->miniMapStartH);

    miniMapLocation.Width = miniMapDrawLocation.X + (irr::s32)(posPlWidth);
    miniMapLocation.Height = miniMapDrawLocation.Y + (irr::s32)(posPlHeight);

    return miniMapLocation;
}

//Initializes the games original
//minimap
void Race::InitMiniMapOriginal(irr::u32 levelNr) {
    //first define correct filename to load image
    //of minimap based on level number
    char mapFilename[50];

    switch (levelNr) {
       case 1: {
            strcpy(mapFilename, "extract/minimaps/track0-1.png");
            break;
        }

        case 2: {
             strcpy(mapFilename, "extract/minimaps/track0-2.png");
             break;
         }

        case 3: {
             strcpy(mapFilename, "extract/minimaps/track0-1-0005.bmp");
             break;
         }

         case 4: {
              strcpy(mapFilename, "extract/minimaps/track0-4.png");
              break;
          }

        case 5: {
             strcpy(mapFilename, "extract/minimaps/track0-5.png");
             break;
         }

         case 6: {
           strcpy(mapFilename, "extract/minimaps/track0-1-0010.bmp");
           break;
       }
    }

    //we need to establish the parameters for the level size
    //this is where we can reuse this function, even though we do not
    //need the autogenerated picture at the end
    irr::video::IImage* baseMiniMapPic = mLevelTerrain->CreateMiniMapInfo(miniMapStartW, miniMapEndW, miniMapStartH, miniMapEndH);

    /* Dbg start */
    /*
    irr::f32 hlpFloat = DEF_SEGMENTSIZE;

    dbgMiniMapPnt1.X = -(float)(miniMapStartW) * hlpFloat;
    dbgMiniMapPnt1.Z = (float)(miniMapStartH) * hlpFloat;
    dbgMiniMapPnt1.Y = 0.0f;

    dbgMiniMapPnt2.X = -(float)(miniMapStartW) * hlpFloat;
    dbgMiniMapPnt2.Z = (float)(miniMapEndH) * hlpFloat;
    dbgMiniMapPnt2.Y = 0.0f;

    dbgMiniMapPnt3.X = -(float)(miniMapEndW) * hlpFloat;
    dbgMiniMapPnt3.Z = (float)(miniMapEndH) * hlpFloat;
    dbgMiniMapPnt3.Y = 0.0f;

    dbgMiniMapPnt4.X = -(float)(miniMapEndW) * hlpFloat;
    dbgMiniMapPnt4.Z = (float)(miniMapStartH) * hlpFloat;
    dbgMiniMapPnt4.Y = 0.0f;
    */
    /*Dbg end */

    //just drop picture again, we do not need it
    baseMiniMapPic->drop();

    //load original games mini map
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    baseMiniMap = mInfra->mDriver->getTexture(mapFilename);

    //for transparency
    mInfra->mDriver->makeColorKeyTexture(baseMiniMap, irr::core::position2d<irr::s32>(0,0));

    bool successUsedArea;
    miniMapImageUsedArea = FindMiniMapImageUsedArea(baseMiniMap, successUsedArea);

    //miniMapSize = baseMiniMap->getSize();
    //miniMapDrawLocation.X = this->mGameScreenRes.Width - miniMapSize.Width;
    //miniMapDrawLocation.Y = this->mGameScreenRes.Height - miniMapSize.Height;

    miniMapSize.Width = miniMapImageUsedArea.getWidth();
    miniMapSize.Height = miniMapImageUsedArea.getHeight();

    miniMapDrawLocation.X = mInfra->mScreenRes.Width - miniMapSize.Width;
    miniMapDrawLocation.Y = mInfra->mScreenRes.Height - miniMapSize.Height;

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    //do some precalculations so that we have to repeat them not unnecessary all the time
    //during the game
    miniMapPixelPerCellW = (irr::f32)(miniMapSize.Width) / ((irr::f32)(miniMapEndW) - (irr::f32)(miniMapStartW));
    miniMapPixelPerCellH = (irr::f32)(miniMapSize.Height) / ((irr::f32)(miniMapEndH) - (irr::f32)(miniMapStartH));
}

//Searches for the used space inside the minimap picture
//while removing unnecessary transparent columns of pixels
//Parameters:
//  miniMapTexture = pointer to the minimap texture
//In case of an unexpected error this function returns succesFlag = false, True otherwise
irr::core::rect<irr::s32> Race::FindMiniMapImageUsedArea(irr::video::ITexture* miniMapTexture, bool &succesFlag) {
 bool wholeColumnTrans;
 bool wholeLineTrans;
 irr::s32 firstUsedColumn = -1;
 irr::s32 lastUsedColumn = -1;
 irr::s32 firstUsedLine = -1;
 irr::s32 lastUsedLine = -1;

 //we need to know the used pixel color format
  irr::video::ECOLOR_FORMAT format = miniMapTexture->getColorFormat();

  //we can only handle this format right now
  if(irr::video::ECF_A8R8G8B8 == format)
    {
        //lock texture for just reading of pixel data
        irr::u8* datapntr = (irr::u8*)miniMapTexture->lock(irr::video::ETLM_READ_ONLY);
        irr::u32 pitch = miniMapTexture->getPitch();

        //all minimaps should have the transparent color at the upper leftmost pixel (0,0)
        //get this value, as a reference where there is no pixel of the actual minimap itself
        irr::video::SColor* texelTrans = (irr::video::SColor *)(datapntr + ((0 * pitch) + (0 * sizeof(irr::video::SColor))));

        irr::s32 texWidth = miniMapTexture->getSize().Width;
        irr::s32 texHeight = miniMapTexture->getSize().Height;

        //iterate through all colums from left to right
        for (irr::s32 x = 0; x < texWidth; x++) {
            //check current column from top to bottom to see if we only find
            //black (unused pixels)
            wholeColumnTrans = true;

            for (irr::s32 y = 0; y < texHeight; y++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeColumnTrans = false;
                }
            }

            if (!wholeColumnTrans) {
                if (firstUsedColumn == -1)
                    firstUsedColumn = x;
                break;
            }
        }

        //iterate through all colums from right to left
        for (irr::s32 x = (texWidth - 1); x > -1 ; x--) {
            //check current column from top to bottom to see if we only find
            //black (unused pixels)
            wholeColumnTrans = true;

            for (irr::s32 y = 0; y < texHeight; y++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeColumnTrans = false;
                }
            }

            if (!wholeColumnTrans) {
                if (lastUsedColumn == -1)
                    lastUsedColumn = x;
                break;
            }
        }

        //iterate through all lines from top to bottom
        for (irr::s32 y = 0; y < texHeight; y++) {
            //check current lines from left to right to see if we only find
            //black (unused pixels)
            wholeLineTrans = true;

            for (irr::s32 x = 0; x < texWidth; x++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeLineTrans = false;
                }
            }

            if (!wholeLineTrans) {
                if (firstUsedLine == -1)
                    firstUsedLine = y;
                break;
            }
        }

        //iterate through all lines from bottom to top
        for (irr::s32 y = (texHeight - 1); y > -1 ; y--) {
            //check current line from left to right to see if we only find
            //black (unused pixels)
            wholeLineTrans = true;

            for (irr::s32 x = 0; x < texWidth; x++) {
                //texel is the pixel color at position x and y

                irr::video::SColor* texel = (irr::video::SColor *)(datapntr + ((y * pitch) + (x * sizeof(irr::video::SColor))));
                if (*texel != *texelTrans) {
                    wholeLineTrans = false;
                }
            }

            if (!wholeLineTrans) {
                if (lastUsedLine == -1)
                    lastUsedLine = y;
                break;
            }
        }

        //unlock texture again!
        miniMapTexture->unlock();

       //if possible leave one fully transparent column left of the minimap texture data
       if (firstUsedColumn > 0)
           firstUsedColumn--;

       //if no minimap image was found at all return "full" empty minimap image
       if (firstUsedColumn == -1)
           firstUsedColumn = 0;

       if (lastUsedColumn == -1)
           lastUsedColumn = texWidth;

         //if possible leave one fully transparent column right of the minimap image
       if (lastUsedColumn <  texWidth)
           lastUsedColumn++;

       //if possible leave one fully transparent column left of the minimap texture data
       if (firstUsedLine > 0)
           firstUsedLine--;

       //if no minimap image was found at all return "full" empty minimap image
       if (firstUsedLine == -1)
           firstUsedLine = 0;

       if (lastUsedLine == -1)
           lastUsedLine = texHeight;

         //if possible leave one fully transparent column right of the minimap image
       if (lastUsedLine <  texHeight)
           lastUsedLine++;

        //return optimized size of minimap
        irr::core::rect<irr::s32> result(firstUsedColumn, firstUsedLine, lastUsedColumn, lastUsedLine);
        succesFlag = true;
        return result;
  } else {
      //unsupported pixel color format!
      //just return full minimap image, and set successFlag to False
      irr::s32 texWidth = miniMapTexture->getSize().Width;
      irr::s32 texHeight = miniMapTexture->getSize().Height;

      irr::core::rect<irr::s32> result(0, 0, texWidth - 1, texHeight - 1);
      succesFlag = false;
      return result;
  }
}

void Race::InitMiniMap(irr::u32 levelNr) {
    //we need to write base minimap image to disk so that we can
    //load it as texture again. As stupid as this sounds
    char outputFilename[50];
    strcpy(outputFilename, "extract/");

    char dirName[30];

    sprintf (dirName, "level0-%d", levelNr);
    strcat(outputFilename, dirName);
    strcat(outputFilename, "/baseminimap.bmp");

    //see if the auto generated minimap is already present
    //if so we do not need to rewrite it again

    //create minimap from level data
    //is not perfect, but better than nothing
    irr::video::IImage* baseMiniMapPic = 
        mLevelTerrain->CreateMiniMapInfo(miniMapStartW, miniMapEndW, miniMapStartH, miniMapEndH);

    //file does not exist?
    if (FileExists(outputFilename) != 1) {
       //does not exist yet, create it

        //create new file for writting
        irr::io::IWriteFile* outputPic = mInfra->mDevice->getFileSystem()->createAndWriteFile(outputFilename, false);

        mInfra->mDriver->writeImageToFile(baseMiniMapPic, outputPic);

        //close output file
        outputPic->drop();
    }

    //drop the autogenerated minimap
    //we do not need it anymore
    baseMiniMapPic->drop();

    //minimap does exist already
    //just load it

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    baseMiniMap = mInfra->mDriver->getTexture(outputFilename);

    //for transparency
    mInfra->mDriver->makeColorKeyTexture(baseMiniMap, irr::core::position2d<irr::s32>(0,0));

    bool successUsedArea;
    miniMapImageUsedArea = FindMiniMapImageUsedArea(baseMiniMap, successUsedArea);

    //miniMapSize = baseMiniMap->getSize();
    miniMapSize.Width = miniMapImageUsedArea.getWidth();
    miniMapSize.Height = miniMapImageUsedArea.getHeight();

    miniMapDrawLocation.X = mInfra->mScreenRes.Width - miniMapSize.Width;
    miniMapDrawLocation.Y = mInfra->mScreenRes.Height - miniMapSize.Height;

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    //do some precalculations so that we have to repeat them not unnecessary all the time
    //during the game
    miniMapPixelPerCellW = (irr::f32)(miniMapSize.Width) / ((irr::f32)(miniMapEndW) - (irr::f32)(miniMapStartW));
    miniMapPixelPerCellH = (irr::f32)(miniMapSize.Height) / ((irr::f32)(miniMapEndH) - (irr::f32)(miniMapStartH));
}

void Race::HandleCraftHeightMapCollisions() {
    std::vector<Player*>::iterator itPlayer;

    for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
        (*itPlayer)->ExecuteHeightMapCollisionDetection();
    }

    //player->ExecuteHeightMapCollisionDetection();
}

void Race::Init() {
    //we want to adjust the keymap for the free movable camera
    SKeyMap keyMap[4];

    keyMap[0].Action=EKA_MOVE_FORWARD;   keyMap[0].KeyCode=KEY_KEY_W;
    keyMap[1].Action=EKA_MOVE_BACKWARD;  keyMap[1].KeyCode=KEY_KEY_S;
    keyMap[2].Action=EKA_STRAFE_LEFT;    keyMap[2].KeyCode=KEY_KEY_A;
    keyMap[3].Action=EKA_STRAFE_RIGHT;   keyMap[3].KeyCode=KEY_KEY_D;

    //create a free moving camera that the user can use to
    //investigate the level/map, not used in actual game
    mCamera = mInfra->mSmgr->addCameraSceneNodeFPS(0, 100.0f,0.05f ,-1 ,
                                            keyMap, 4, false, 0.0f);

    //mCamera->setFOV(PI / 2.5);

    //create a DrawDebug object
    this->mDrawDebug = new DrawDebug(mInfra->mDriver);

    if (!LoadLevel(levelNr)) {
        //there was an error loading the level
        return;
    }
        
    //level was loaded ok, we can continue setup

    //predefine mini map marker colors
    //for max 8 players
    mMiniMapMarkerColors.clear();
    mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 254, 254, 250)); //player 1 marker color (human player)
    mMiniMapMarkerColors.push_back(new irr::video::SColor(255,  70, 121,  99)); //player 2 marker color
    mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 114,  53,  60)); //player 3 marker color
    mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 235, 112,  46)); //player 4 marker color
    mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 251, 220,  56)); //player 5 marker color
    mMiniMapMarkerColors.push_back(new irr::video::SColor(255, 182,  32, 130)); //player 6 marker color
    mMiniMapMarkerColors.push_back(new irr::video::SColor(255,  57,  77, 145)); //player 7 marker color
    mMiniMapMarkerColors.push_back(new irr::video::SColor(255,   4,   6,   8)); //player 8 marker color

    if (useAutoGenMinimap) {
        //try to auto generate a own minimap
        //based on the level file
        //is not perfect
        InitMiniMap(levelNr);
    } else {
        //use the original game supplied minimap
        //drawings
        InitMiniMapOriginal(levelNr);
    }

    /***********************************************************/
    /* Init single player HUD                                  */
    /***********************************************************/
    Hud1Player = new HUD(mInfra);

    //create my overall physics object
    //also handover pointer to my DrawDebug object
    this->mPhysics = new Physics(this, this->mDrawDebug);

    //handover pointer to wall collision line (based on level file entities) data
    this->mPhysics->SetLevelCollisionWallLineData(ENTWallsegmentsLine_List);

    //create the object for path finding and services
    mPath = new Path(this, mDrawDebug);

    //create my players and setup their physics
    //Wolf 22.12.2024: commented out, since add player we have no player object
    //here anymore
    //createPlayers(levelNr);

    //get player start locations from the level file
    mPlayerStartLocations =
        this->mLevelTerrain->GetPlayerRaceTrackStartLocations();

    //HUD should show main player stats
    //Wolf 22.12.2024: commented out, since add player we have no player object
    //here anymore
    if (mDemoMode) {
        //if we do not skip the race start, switch the Hud
        //to "start" mode
        if (mCurrentPhase == DEF_RACE_PHASE_START) {
            Hud1Player->SetHUDState(DEF_HUD_STATE_STARTSIGNAL);

            //0.. means no light lit
            //with increasing value the start signal
            //advances towards the final state
            Hud1Player->SetStartSignalState(0);
        } else {
                //in demo mode we do not want to draw a HUD
                //in case we skip the start
                Hud1Player->SetHUDState(DEF_HUD_STATE_NOTDRAWN);
               }
    } else {
              //if we do not skip the race start, switch the Hud
              //to "start" mode
              if (mCurrentPhase == DEF_RACE_PHASE_START) {
                Hud1Player->SetHUDState(DEF_HUD_STATE_STARTSIGNAL);

                //0.. means no light lit
                //with increasing value the start signal
                //advances towards the final state
                Hud1Player->SetStartSignalState(0);
              } else {
                        //in normal race mode we draw the HUD
                        //if we skip the start we can already show the
                        //race Hud
                        Hud1Player->SetHUDState(DEF_HUD_STATE_RACE);
                     }
           }

    //give physics the triangle selectors for overall collision detection
    this->mPhysics->AddCollisionMesh(triangleSelectorWallCollision);
    this->mPhysics->AddCollisionMesh(triangleSelectorColumnswCollision);

    //give physics the triangle selector for weapon targeting (ray casting at terrain/blocks)
    this->mPhysics->AddRayTargetMesh(triangleSelectorColumnswCollision);
    this->mPhysics->AddRayTargetMesh(triangleSelectorColumnswoCollision);
    this->mPhysics->AddRayTargetMesh(triangleSelectorStaticTerrain);
    this->mPhysics->AddRayTargetMesh(triangleSelectorDynamicTerrain);

    //activate collisionResolution in physics
    //can be disabled for debugging purposes
    mPhysics->collisionResolutionActive = true;

    //set initial camera location
    //Wolf 22.12.2024: commented out, since add player we have no player object
    //here anymore
    //mCamera->setPosition(playerPhysicsObj->physicState.position + irr::core::vector3df(2.0f, 2.0f, 00.0f));
    //mCamera->setTarget(playerPhysicsObj->physicState.position);

    mInfra->mSmgr->setActiveCamera(mCamera);

    SetupTopRaceTrackPointerOrigin();

    //create the world awareness class
    mWorldAware = new WorldAwareness(mInfra->mDevice, mInfra->mDriver, this);

    //now use the new world aware class to further analyze all
    //waypoint links for computer player movement control later
    mWorldAware->PreAnalyzeWaypointLinksOffsetRange();

    //create my ExplosionLauncher
    mExplosionLauncher = new ExplosionLauncher(this, mInfra->mSmgr, mInfra->mDriver);

    //create a new Bezier object for testing
    testBezier = new Bezier(mLevelTerrain, mDrawDebug);

    //load the correct music file for this level
    char musicFileName[60];

    //make assigement level number to music file
    DeliverMusicFileName(levelNr, musicFileName);

    if (!mMusicPlayer->loadGameMusicFile(musicFileName)) {
        cout << "Music load failed" << endl;
        return;
    } else {
             //start music playing
             mMusicPlayer->StartPlay();
           }

    ready = true;
  
    //only to test if we can save a levelfile properly!
    //std::string testsaveName("testsave.dat");
    //this->mLevelRes->Save(testsaveName);
}

void Race::SetupTopRaceTrackPointerOrigin() {
    //get race track terrain bounding box
    this->mLevelTerrain->StaticTerrainSceneNode->updateAbsolutePosition();
    irr::core::aabbox3df bbox = this->mLevelTerrain->StaticTerrainSceneNode->getTransformedBoundingBox();

    irr::f32 addYcoord = bbox.getExtent().Y / 2.0f;

    this->topRaceTrackerPointerOrigin = bbox.getCenter() + irr::core::vector3df(0.0f, addYcoord, 0.0f);
}

//void Race::TestVoxels() {
//    irr::core::vector3df testStart(-20.0f, 16.0f, 60.0f);
//    irr::core::vector3df testEnd;
//    std::vector<irr::core::vector3di> voxels;

//    testEnd = player->phobj->physicState.position;

//    voxels = mPhysics->voxel_traversal(testStart, testEnd);

//    irr::video::SColor col2(255, 0, 255, 0);

//    mDriver->setMaterial(*mDrawDebug->green);
//    mDriver->draw3DLine(testStart, testEnd, col2);

//    std::vector<irr::core::vector3di>::iterator it;
//    irr::core::vector3df boxVertex1;
//    irr::core::vector3df boxVertex2;

//    irr::video::SColor col(255, 0, 0, 255);

//    for (it = voxels.begin(); it != voxels.end(); ++it) {
//       boxVertex1.X = (*it).X * DEF_SEGMENTSIZE;
//       boxVertex1.Y = (*it).Y * DEF_SEGMENTSIZE;
//       boxVertex1.Z = (*it).Z * DEF_SEGMENTSIZE;

//       boxVertex2.X = ((*it).X + 1) * DEF_SEGMENTSIZE;
//       boxVertex2.Y = ((*it).Y + 1) * DEF_SEGMENTSIZE;
//       boxVertex2.Z = ((*it).Z + 1) * DEF_SEGMENTSIZE;

//       irr::core::aabbox3df box(boxVertex1.X, boxVertex1.Y, boxVertex1.Z, boxVertex2.X, boxVertex2.Y, boxVertex2.Z);

//       mDriver->setMaterial(*mDrawDebug->blue);
//       mDriver->draw3DBox(box, col);
//    }
//}

void Race::ControlStartPhase(irr::f32 frameDeltaTime) {
    mStartPhaseTimeCounter += frameDeltaTime;

    irr::u8 currentSignalState = Hud1Player->GetStartSignalState();

    //delay initial switch to red light longer then afterwards
    if (((currentSignalState == 0) && (mStartPhaseTimeCounter >= 3.0f)) ||
        ((currentSignalState > 0) && (mStartPhaseTimeCounter >= 1.0f))) {
        mStartPhaseTimeCounter = 0.0f;

        //advance current signal state to next state
        currentSignalState++;

        //when advancing to red and yellow light play
        //START1 sound
        //when advancing to final green light play
        //START2 sound
        if ((currentSignalState == 1) || (currentSignalState == 2)) {
            //we change to red or yellow light
            this->mSoundEngine->PlaySound(SRES_GAME_START1, false);
        } else if (currentSignalState == 3) {
            //we change to green light
            this->mSoundEngine->PlaySound(SRES_GAME_START2, false);
        }

        //advance start light to the next phase
        Hud1Player->SetStartSignalState(currentSignalState);

        //state 3 means green is lit
        if (currentSignalState == 3) {
            //start is now over, players are now in next phase where the
            //are traveling the first time towards the finish line; HUD is still not shown
            //and as far as I have seen computer players do not attack here yet
            //as soon as the first player crosses the finish line in this state, we finally
            //will change to full on race state somewhere else
            this->mCurrentPhase = DEF_RACE_PHASE_FIRSTWAYTOWARDSFINISHLINE;

            //switch all players to first way to finish line mode
            std::vector<Player*>::iterator it;
            for (it = this->mPlayerVec.begin(); it != this->mPlayerVec.end(); ++it) {
                ((*it)->SetupForFirstWayToFinishLine());
            }
        }
    }
}

void Race::AdvanceTime(irr::f32 frameDeltaTime) {
    //are we in Race start phase, if so also call
    //race start control function
    if (mCurrentPhase == DEF_RACE_PHASE_START) {
        ControlStartPhase(frameDeltaTime);
    }

    //if we are in final race phase (where we delay the exit of race)
    //until all animators are done animating, so that we can clean them
    //up during deconstructor of race object, hande this final phase here
    if (mCurrentPhase == DEF_RACE_PHASE_WAITUNTIL_ANIMATORS_DONE) {
        HandleExitRace();
    }

    float progressMorph;

    irr::f32 physicsFrameDeltaTime = frameDeltaTime;

    //if physicsFrameDeltaTime is too large we could get
    //weird physics effects, therefore clamp time to max
    //allowed level
    if (physicsFrameDeltaTime > 0.1f)
      physicsFrameDeltaTime = 0.01f;

    //if we are in race phase already handle morphs,
    //and update timers

    if (mCurrentPhase == DEF_RACE_PHASE_RACING) {
            //run morphs
            if (AllowStartMorphsPerKey && runMorph)
                {
                  absTimeMorph += frameDeltaTime;
                  progressMorph = (float)fmin(1.0f, fmax(0.0f, 0.5f + sin(absTimeMorph)));

                  std::list<Morph*>::iterator itMorph;

                  for (itMorph = Morphs.begin(); itMorph != Morphs.end(); ++itMorph) {
                      (*itMorph)->setProgress(progressMorph);
                      this->mLevelTerrain->ApplyMorph((**itMorph));
                      (*itMorph)->MorphColumns();
                  }

                  //mark column vertices as dirty
                  mLevelBlocks->SetColumnVerticeSMeshBufferVerticePositionsDirty();
                }

            if (!AllowStartMorphsPerKey) {
                //update level morphs
                UpdateMorphs(frameDeltaTime);
            }

            //update timer
            UpdateTimers(frameDeltaTime);
    }

    //update external race track cameras
    UpdateExternalCameras();

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntMorphing);

    //process pending triggers
    ProcessPendingTriggers();

    //update all players
    std::vector<Player*>::iterator itPlayer;

    //reset variable mOtherPlayerHasMissleLockAtMe for all players
    //we use this variable to find out which player is currently targeted
    //by a missile lock of another player, we need to know to be able to play
    //the warning sound
    for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
          (*itPlayer)->mOtherPlayerHasMissleLockAtMe = false;
    }

    //in the player Update function the mOtherPlayerHasMissleLockAtMe is set
    //to true by other players, if the have currently a missile lock at this player
    //the warning sound is then finally triggered in the AfterPhysicsUpdate routine
    //call below
    for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
          (*itPlayer)->Update(frameDeltaTime);
    }

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntUpdatePlayers);

    //advance physics time and update sceneNode positions and orientations
    mPhysics->AdvancePhysicsTime(physicsFrameDeltaTime);

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntAdvancePhysics);

    if (!mDemoMode) {
        //camera control normal race
        ManagePlayerCamera();
    } else {
        //camera control for demo mode
        ManageCameraDemoMode(frameDeltaTime);
    }

    for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
          (*itPlayer)->AfterPhysicsUpdate();
    }

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntAfterPhysicsUpdate);

    //important, empty old triangle hit information first!
    //otherwise we have a memory leak
    //mPhysics->EmptyTriangleHitInfoVector(TestRayTrianglesSelector);

    //mPhysics->FindRayTargetTriangles(*player->phobj, player->craftForwardDirVec);
    /*TestRayTrianglesSelector = mPhysics->ReturnTrianglesHitByRay( mPhysics->mRayTargetSelectors,
                                  player->phobj->physicState.position, player->phobj->physicState.position + player->craftForwardDirVec * irr::core::vector3df(50.0f, 50.0f, 50.0f),
                                                                  true);*/

     for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
        (*itPlayer)->currCloseWayPointLinks = mPath->PlayerFindCloseWaypointLinks((*itPlayer));
        (*itPlayer)->SetCurrClosestWayPointLink(mPath->PlayerDeriveClosestWaypointLink((*itPlayer)->currCloseWayPointLinks));
     }

     for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
        UpdatePlayerDistanceToNextCheckpoint(*itPlayer);
     }

     for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
        irr::core::aabbox3d<f32> playerBox = (*itPlayer)->Player_node->getTransformedBoundingBox();
        CheckPlayerCrossedCheckPoint((*itPlayer), playerBox);
        CheckPlayerCollidedCollectible((*itPlayer), playerBox);
    }

    //update player race position ranking
    UpdatePlayerRacePositionRanking();

    //update recovery vehicle logic
    UpdateRecoveryVehicles(frameDeltaTime);

    CheckRaceFinished(frameDeltaTime);

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntPlayerMonitoring);

    //update all particle systems
    UpdateParticleSystems(frameDeltaTime);

    //update all collectable spawners
    UpdateCollectableSpawners(frameDeltaTime);

    //update all type2 collectables
    UpdateType2Collectables(frameDeltaTime);

    //update all current explosions
    mExplosionLauncher->Update(frameDeltaTime);

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntUpdateParticleSystems);

    for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
        mWorldAware->Analyse(*itPlayer);
    }

    mInfra->mTimeProfiler->Profile(mInfra->mTimeProfiler->tIntWorldAware);
}

void Race::PlayerEnteredCraftTriggerRegion(Player* whichPlayer, MapTileRegionStruct* whichRegion) {
    //yes, player is currently inside this region
    if (DebugShowTriggerEvents) {
        char triggerMessage[80];
        char triggerID[10];

        strcpy(triggerMessage, whichPlayer->mPlayerStats->name);
        strcat(triggerMessage, " TRIGGERED ");
        sprintf(triggerID, "%d", whichRegion->regionId);
        strcat(triggerMessage, triggerID);

        whichPlayer->ShowPlayerBigGreenHudText(triggerMessage, 5.0f, false);
    }

    //store trigger in pending trigger list, for processing during
    //the next advance game routine call
    this->mPendingTriggerTargetGroups.push_back(whichRegion->mTargetGroup);
}

void Race::PlayerMissileHitMissileTrigger(Player* whichPlayer, MapTileRegionStruct* whichRegion) {
    if (DebugShowTriggerEvents) {
       char triggerMessage[80];
       char triggerID[10];

       strcpy(triggerMessage, whichPlayer->mPlayerStats->name);
       strcat(triggerMessage, " MISSILE HIT ");
       sprintf(triggerID, "%d", whichRegion->regionId);
       strcat(triggerMessage, triggerID);

       whichPlayer->ShowPlayerBigGreenHudText(triggerMessage, 5.0f, false);
    }

    //store trigger in pending trigger list, for processing during
    //the next advance game routine call
    this->mPendingTriggerTargetGroups.push_back(whichRegion->mTargetGroup);
}

void Race::TimedTriggerOccured(Timer* whichTimer) {
    if (DebugShowTriggerEvents) {
        char triggerMessage[80];
        char triggerID[10];

        sprintf(triggerID, "TIMER %d ", whichTimer->mEntityItem->get_ID());
        strcpy(triggerMessage, triggerID);
        strcat(triggerMessage, " TRIGGERED ");
        sprintf(triggerID, "%d", whichTimer->mEntityItem->getTargetGroup());
        strcat(triggerMessage, triggerID);

        this->mPlayerVec.at(0)->ShowPlayerBigGreenHudText(triggerMessage, 5.0f, false);
    }

    //store trigger in pending trigger list, for processing during
    //the next advance game routine call
    this->mPendingTriggerTargetGroups.push_back(whichTimer->mEntityItem->getTargetGroup());
}

void Race::ProcessPendingTriggers() {
    //any trigger pending?
    if (this->mPendingTriggerTargetGroups.size() > 0) {
        std::vector<int16_t>::iterator it;
        std::vector<Collectable*>::iterator itCollect;
        std::vector<Timer*>::iterator itTimer;
        std::list<Morph*>::iterator itMorph;
        std::vector<SteamFountain*>::iterator itSteam;
        std::vector<ExplosionEntity*>::iterator itExplosion;

        for (it = mPendingTriggerTargetGroups.begin(); it != mPendingTriggerTargetGroups.end(); ) {
            //check all collectables
            for (itCollect = this->ENTCollectablesVec->begin(); itCollect != this->ENTCollectablesVec->end(); ++itCollect) {
                //Note 02.02.2025: Today I introduced a second type of collectable object, which is for spawned
                //temporary collectables (for example spawned by the collectablespawner then a player craft is destroyed)
                //For this type of collectable there is no entityItem object in the background, as there is no map file entry
                //behind this collectable; This type of collectable has also no trigger, and therefore we need to skip collectables
                //here which have an entityItem of NULL!
                if ((*itCollect)->mEntityItem != NULL) {
                    //need to check this collectable, is normal (type 1), has an
                    //EntityItem in the background, and a trigger
                    if ((*itCollect)->mEntityItem->getGroup() == (*it)) {
                        //this collectable belongs to the group we need to
                        //trigger according to the target trigger
                        (*itCollect)->Trigger();
                    }
                }
            }

            //check all timers
            for (itTimer = this->mTimerVec.begin(); itTimer != this->mTimerVec.end(); ++itTimer) {
                if ((*itTimer)->mEntityItem->getGroup() == (*it)) {
                    //this timer belongs to the group we need to
                    //trigger according to the target trigger
                    (*itTimer)->Trigger();
                }
            }

            //check all morphs
            for (itMorph = this->Morphs.begin(); itMorph != this->Morphs.end(); ++itMorph) {
                if ((*itMorph)->Source->getGroup() == (*it)) {
                    //this morph belongs to the group we need to
                    //trigger according to the target trigger
                    (*itMorph)->Trigger();
                }
            }

            //check all SteamFountains
            for (itSteam = this->steamFountainVec->begin(); itSteam != this->steamFountainVec->end(); ++itSteam) {
                if ((*itSteam)->mEntityItem->getGroup() == (*it)) {
                    //this SteamFountain belongs to the group we need to
                    //trigger according to the target trigger
                    (*itSteam)->Trigger();
                }
            }

            //check all explosion entities
            for (itExplosion = this->mExplosionEntityVec.begin(); itExplosion != this->mExplosionEntityVec.end(); ++itExplosion) {
                if ((*itExplosion)->mEntityItem->getGroup() == (*it)) {
                    //this explosion entity belongs to the group we need to
                    //trigger according to the target trigger
                    (*itExplosion)->Trigger();
                }
            }

          //remove entry, was processed already
            it = mPendingTriggerTargetGroups.erase(it);
        }
    }
}

void Race::CheckPlayerCrossedCheckPoint(Player* whichPlayer, irr::core::aabbox3d<f32> playerBox) {

    std::vector<CheckPointInfoStruct*>::iterator it;
    irr::core::aabbox3d<f32> checkPointBox;

    for (it = this->checkPointVec->begin(); it != this->checkPointVec->end(); ++it) {
        checkPointBox = (*it)->SceneNode->getTransformedBoundingBox();
        if (playerBox.intersectsWithBox(checkPointBox)) {

            //player crosses waypoint, figure out if player crosses waypoint in the normal
            //race direction, if not ignore event
            irr::core::vector3df velNormalized = whichPlayer->phobj->physicState.velocity.normalize();

            //if the player crosses checkPoint in normal RaceDirection
            //the dotProduct should be positive
            irr::f32 dotProduct = velNormalized.dotProduct((*it)->RaceDirectionVec);

            if (dotProduct > 0.0f) {
                //next lines only for debugging of checkpoint functionality
                //TODO: The char below produces a memory leak! bug for debugging the value is helpful!
                //char *txt = new char[20];
                //char nrtxt[4];
                //strcpy(txt, "CHECKPOINT ");
                //sprintf(nrtxt, "%d", (*it)->value);
                //strcat(txt, nrtxt);

                //player->GetMyHUD()->ShowBannerText(&txt[0], 0.2f);

                //tell player object about crossed checkpoint
                whichPlayer->CrossedCheckPoint((*it)->value, (irr::s32)(checkPointVec->size()));
            }
        }
    }
}

void Race::UpdateParticleSystems(irr::f32 frameDeltaTime) {
    //update all steam fontains
    std::vector <SteamFountain*>::iterator it;

    for (it = steamFountainVec->begin(); it != steamFountainVec->end(); ++it) {
        (*it)->TriggerUpdate(frameDeltaTime);
    }
}

void Race::HandleComputerPlayers(irr::f32 frameDeltaTime) {
    std::vector<Player*>::iterator itPlayer;

    for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
        (*itPlayer)->dbgPlayerInMyWay.clear();

        if (!(*itPlayer)->mHumanPlayer) {
           (*itPlayer)->RunComputerPlayerLogic(frameDeltaTime);
        }
    }
}

void Race::HandleBasicInput() {

    //only for debugging purposes, to trigger
    //a breakpoint via a keyboard press
    DebugHitBreakpoint = false;

    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_F2)) {
      this->mGame->StopTime();
    }

    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_F1)) {
        this->mGame->StartTime();
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_F3))
    {
         this->mGame->AdvanceFrame(1);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_F4))
    {
         if (mInfra->mLogger->IsWindowHidden()) {
             mInfra->mLogger->ShowWindow();
         } else {
             mInfra->mLogger->HideWindow();
         }
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_1))
    {
       DebugSelectPlayer(0);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_2))
    {
       DebugSelectPlayer(1);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_3))
    {
        DebugSelectPlayer(2);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_4))
    {
       DebugSelectPlayer(3);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_5))
    {
       DebugSelectPlayer(4);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_6))
    {
       DebugSelectPlayer(5);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_7))
    {
       DebugSelectPlayer(6);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_8))
    {
       DebugSelectPlayer(7);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_9))
    {
       DebugSelectPlayer(8);
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_Z))
    {
         DebugHitBreakpoint = true;
    }

    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_P)) {
        playerCamera = !playerCamera;

        //unhide all player models
        std::vector<Player*>::iterator it;
        for (it = mPlayerVec.begin(); it != mPlayerVec.end(); ++it) {
            (*it)->UnhideCraft();
        }

        if (playerCamera) {
           //hide the camera of the selected player
           if (currPlayerFollow != NULL) {
               currPlayerFollow->HideCraft();
           }
        }
    }

    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_T)) {
        mLevelTerrain->SwitchViewMode();
    }

    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_B)) {
        mLevelBlocks->SwitchViewMode();
    }

    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
        //only for debugging!
        //this->mRaceWasFinished = true;

        //24.03.2025: Add a final race phase where we wait until
        //all currently working animators are finished
        //this->exitRace = true;
        InitiateExitRace();
    }

    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_F)) {
        if (this->currPlayerFollow != NULL)
        {
            this->currPlayerFollow->ChangeViewMode();
        }
    }
}

//unfortunetly it seems we can not remove SceneNodes from SceneManager
//that still have an animation going on; Animations are for example used
//by the machinegun and missiles/explosions
//The make sure that this animations are done when we finally exit the race
//and the destructor of the race is called, I decided to add another final
//race phase where the players are prevented to fire more machinguns and
//missiles, so that no new animators are triggered
//Then we delay the end of the race in the final phase as long, until all currently
//running animations are done; Only then we finally exit the race
//The function below is used to initiate this final phase of the race
void Race::InitiateExitRace() {
   //from now on prevent all players from firing
   //the machine gun or another missile
   //so that the animators have all time to finish
   std::vector<Player*>::iterator it;

   for (it = this->mPlayerVec.begin(); it != this->mPlayerVec.end(); ++it) {
       (*it)->DeactivateAttack();
   }

   //new race phase, we only wait until all animators have finished
   this->mCurrentPhase = DEF_RACE_PHASE_WAITUNTIL_ANIMATORS_DONE;
}

void Race::HandleExitRace() {
   //only if all animators are done, we finally really
   //exit the race
   bool allAnimatorsDone = true;

   //all machine gun animators done
   std::vector<Player*>::iterator it;

   for (it = this->mPlayerVec.begin(); it != this->mPlayerVec.end(); ++it) {
       allAnimatorsDone &= (*it)->AllAnimatorsDone();
   }

   if (allAnimatorsDone) {
       //now we can really exit the game
       //all animators have finished
       exitRace = true;
   }
}

void Race::HandleInput() {
     if (mPlayerVec.at(0)->mHumanPlayer) {
            bool playerNoTurningKeyPressed = true;

             if(mInfra->mEventReceiver->IsKeyDown(irr::KEY_UP)) {
                mPlayerVec.at(0)->Forward();
             }

             if(mInfra->mEventReceiver->IsKeyDown(irr::KEY_DOWN))
             {
                mPlayerVec.at(0)->Backward();
             }

             if(mInfra->mEventReceiver->IsKeyDown(irr::KEY_SPACE))
             {
                mPlayerVec.at(0)->IsSpaceDown(true);
             } else {
                mPlayerVec.at(0)->IsSpaceDown(false);
             }

             if(mInfra->mEventReceiver->IsKeyDown(irr::KEY_LEFT)) {
                  mPlayerVec.at(0)->Left();
                  mPlayerVec.at(0)->firstNoKeyPressed = true;
                  playerNoTurningKeyPressed = false;
             }
             if(mInfra->mEventReceiver->IsKeyDown(irr::KEY_RIGHT)) {
                  mPlayerVec.at(0)->Right();
                  mPlayerVec.at(0)->firstNoKeyPressed = true;
                  playerNoTurningKeyPressed = false;
             }

             //if player has not pressed any turning key run this code
             //as well
             if (playerNoTurningKeyPressed) {
                 mPlayerVec.at(0)->NoTurningKeyPressed();
             }

             if (mInfra->mEventReceiver->IsKeyDown(irr::KEY_KEY_Y)) {
                 mPlayerVec.at(0)->mMGun->Trigger();
             }

             if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_X)) {
                 mPlayerVec.at(0)->mMissileLauncher->Trigger();
             }
     }

     if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_R)) {
         if (currPlayerFollow != NULL) {
            this->SpawnCollectiblesForPlayer(currPlayerFollow);
         }
     }

     if (AllowStartMorphsPerKey) {
            if(mInfra->mEventReceiver->IsKeyDown(irr::KEY_KEY_M))
            {
                 runMorph =true;
            }
     }

    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_J)) {
        this->mWorldAware->WriteOneDbgPic = true;
    }

    if(mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_C)) {
        //toggle collision resolution active state
        mPhysics->collisionResolutionActive = !mPhysics->collisionResolutionActive;
    } 
}

//the routine below is from:
//https://irrlicht.sourceforge.io/forum/viewtopic.php?t=43565
//https://irrlicht.sourceforge.io/forum//viewtopic.php?p=246138#246138
//But I had to modify it, to remove warnings "'argument': conversion from 'T' to 'T', possible loss of data"
//in Visual Studio
void Race::draw2DImage(irr::video::IVideoDriver *driver, irr::video::ITexture* texture ,
     irr::core::rect<irr::s32> sourceRect, irr::core::position2d<irr::s32> position,
     irr::core::position2d<irr::s32> rotationPoint, irr::f32 rotation, irr::core::vector2df scale, bool useAlphaChannel, irr::video::SColor color) {

    irr::video::SMaterial material;

    // Store and clear the projection matrix
    irr::core::matrix4 oldProjMat = driver->getTransform(irr::video::ETS_PROJECTION);
    driver->setTransform(irr::video::ETS_PROJECTION,irr::core::matrix4());

    // Store and clear the view matrix
    irr::core::matrix4 oldViewMat = driver->getTransform(irr::video::ETS_VIEW);
    driver->setTransform(irr::video::ETS_VIEW,irr::core::matrix4());

    // Find the positions of corners
    irr::core::vector2df corner[4];

    corner[0] = irr::core::vector2df((irr::f32)(position.X),(irr::f32)(position.Y));
    corner[1] = irr::core::vector2df((irr::f32)(position.X)+(irr::f32)(sourceRect.getWidth())*scale.X,(irr::f32)(position.Y));
    corner[2] = irr::core::vector2df((irr::f32)(position.X),(irr::f32)(position.Y)+(irr::f32)(sourceRect.getHeight())*scale.Y);
    corner[3] = irr::core::vector2df((irr::f32)(position.X)+(irr::f32)(sourceRect.getWidth())*scale.X,(irr::f32)(position.Y)+(irr::f32)(sourceRect.getHeight())*scale.Y);

    // Rotate corners
    if (rotation != 0.0f)
        for (int x = 0; x < 4; x++)
            corner[x].rotateBy(rotation,irr::core::vector2df((irr::f32)(rotationPoint.X), (irr::f32)(rotationPoint.Y)));


    // Find the uv coordinates of the sourceRect
    irr::core::vector2df uvCorner[4];
    uvCorner[0] = irr::core::vector2df((irr::f32)(sourceRect.UpperLeftCorner.X),(irr::f32)(sourceRect.UpperLeftCorner.Y));
    uvCorner[1] = irr::core::vector2df((irr::f32)(sourceRect.LowerRightCorner.X),(irr::f32)(sourceRect.UpperLeftCorner.Y));
    uvCorner[2] = irr::core::vector2df((irr::f32)(sourceRect.UpperLeftCorner.X),(irr::f32)(sourceRect.LowerRightCorner.Y));
    uvCorner[3] = irr::core::vector2df((irr::f32)(sourceRect.LowerRightCorner.X),(irr::f32)(sourceRect.LowerRightCorner.Y));
    for (int x = 0; x < 4; x++) {
        float uvX = uvCorner[x].X/(float)(texture->getSize().Width);
        float uvY = uvCorner[x].Y/(float)(texture->getSize().Height);
        uvCorner[x] = irr::core::vector2df(uvX,uvY);
    }

    // Vertices for the image
    irr::video::S3DVertex vertices[4];
    irr::u16 indices[6] = { 0, 1, 2, 3 ,2 ,1 };

    // Convert pixels to world coordinates
    float screenWidth = (float)(driver->getScreenSize().Width);
    float screenHeight = (float)(driver->getScreenSize().Height);
    for (int x = 0; x < 4; x++) {
        float screenPosX = ((corner[x].X/screenWidth)-0.5f)*2.0f;
        float screenPosY = ((corner[x].Y/screenHeight)-0.5f)*-2.0f;
        vertices[x].Pos = irr::core::vector3df(screenPosX,screenPosY,1);
        vertices[x].TCoords = uvCorner[x];
        vertices[x].Color = color;
    }

    material.Lighting = false;
    material.ZWriteEnable = false;
    material.ZBuffer = false;
    material.TextureLayer[0].Texture = texture;
    //the following line did not work, therefore I commented it out
    /*material.MaterialTypeParam = irr::video::pack_texureBlendFunc
            (irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_TEXTURE | irr::video::EAS_VERTEX_COLOR);
*/
    if (useAlphaChannel)
        material.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
    else
        material.MaterialType = irr::video::EMT_SOLID;

    driver->setMaterial(material);
    driver->drawIndexedTriangleList(&vertices[0],4,&indices[0],2);

    // Restore projection and view matrices
    driver->setTransform(irr::video::ETS_PROJECTION,oldProjMat);
    driver->setTransform(irr::video::ETS_VIEW,oldViewMat);
}

void Race::DrawSky() {
    if (mSkyImage != NULL) {
        //Draw sky
        irr::s32 moveX = 0;

        irr::f32 hlp = ((currPlayerFollow->mCurrentAvgPlayerLeaningAngleLeftRightValue - 180.0f) / 180.0f) * 150.0f;
        moveX = (irr::s32)(hlp);

        irr::core::recti locMovingWindow( mSkyImage->getSize().Width / 2 - (1024 / 2) - 75 + moveX , (mSkyImage->getSize().Height / 2 - (680 / 2)) +50 ,
                                          mSkyImage->getSize().Width / 2 + (1024 / 2) - 75 + moveX , mSkyImage->getSize().Height / 2 + (680 / 2) + 150 );

        //dbglocMovingWindow = locMovingWindow;

        irr::core::vector2di middlePos = (locMovingWindow.LowerRightCorner - locMovingWindow.UpperLeftCorner) / 2 + locMovingWindow.UpperLeftCorner;

        draw2DImage(mInfra->mDriver, mSkyImage ,locMovingWindow,
             irr::core::vector2di(-200, -200), irr::core::vector2di( middlePos.X, middlePos.Y),
             currPlayerFollow->mCurrentAvgPlayerLeaningAngleLeftRightValue * 0.25f, irr::core::vector2df(1.0f, 1.0f), false, irr::video::SColor(255,255,255,255));
    }
}

void Race::DrawTestShape() {
    //box1.origin = glm::vec3(-0.5f, 0.0f, 0.0f);

    irr::core::vector3df p1, p2, p3, p4, p5;
    irr::f32 mul = 10.0f;

    // The 5 vertices of object 1
    p1.set(0.0f, 0.4f * mul, 0.0f);
    p2.set(-0.2f * mul, 0.0f, 0.0f);
    p3.set(0.0f, 0.0f, 0.2f * mul);
    p4.set(0.2f * mul, 0.0f, 0.0f);
    p5.set(0.0f, 0.2f * mul, -0.2f * mul);

    //mDrawDebug->Draw3DSphere(p1);
    //mDrawDebug->Draw3DSphere(p2);
    //mDrawDebug->Draw3DSphere(p3);
    //mDrawDebug->Draw3DSphere(p4);
    //mDrawDebug->Draw3DSphere(p5);

    std::vector<irr::core::vector3df> edgeSet;
    edgeSet.push_back(p1 - p2);
    edgeSet.push_back(p1 - p3);
    edgeSet.push_back(p1 - p4);
    edgeSet.push_back(p1 - p5);
    edgeSet.push_back(p2 - p3);
    edgeSet.push_back(p3 - p4);
    edgeSet.push_back(p4 - p5);
    edgeSet.push_back(p5 - p2);
    edgeSet.push_back(p4 - p2);
/*
    std::vector<irr::core::vector3df>::iterator it;
        for (it = edgeSet.begin(); it != edgeSet.end(); ++it) {
              mDrawDebug->Draw3DSphere((*it));
        }*/

    mDrawDebug->Draw3DLine(p1, p2, mDrawDebug->red);
    mDrawDebug->Draw3DLine(p2, p3, mDrawDebug->red);
    mDrawDebug->Draw3DLine(p3, p1, mDrawDebug->red);

    mDrawDebug->Draw3DLine(p1, p3, mDrawDebug->green);
    mDrawDebug->Draw3DLine(p3, p4, mDrawDebug->green);
    mDrawDebug->Draw3DLine(p4, p1, mDrawDebug->green);

    mDrawDebug->Draw3DLine(p1, p4, mDrawDebug->blue);
    mDrawDebug->Draw3DLine(p4, p5, mDrawDebug->blue);
    mDrawDebug->Draw3DLine(p5, p1, mDrawDebug->blue);

    mDrawDebug->Draw3DLine(p1, p5, mDrawDebug->pink);
    mDrawDebug->Draw3DLine(p5, p2, mDrawDebug->pink);
    mDrawDebug->Draw3DLine(p2, p1, mDrawDebug->pink);

    mDrawDebug->Draw3DLine(p2, p4, mDrawDebug->white);
    mDrawDebug->Draw3DLine(p4, p3, mDrawDebug->white);
    mDrawDebug->Draw3DLine(p3, p2, mDrawDebug->white);

    mDrawDebug->Draw3DLine(p2, p5, mDrawDebug->brown);
    mDrawDebug->Draw3DLine(p5, p4, mDrawDebug->brown);
    mDrawDebug->Draw3DLine(p4, p2, mDrawDebug->brown);
}

void Race::DrawHUD(irr::f32 frameDeltaTime) {
    this->Hud1Player->DrawHUD1(frameDeltaTime);
}

void Race::DrawMiniMap(irr::f32 frameDeltaTime) {

    //in Demo mode we do not want to draw a minimap
    if (mDemoMode || (this->mCurrentPhase == DEF_RACE_PHASE_START))
        return;

    mInfra->mDriver->draw2DImage(baseMiniMap, miniMapDrawLocation,
             miniMapImageUsedArea, 0,
             irr::video::SColor(255,255,255,255), true);

    //we want to blink the human player location
    //marker
    miniMapAbsTime += frameDeltaTime * 1000.0f;
    if (miniMapAbsTime > 500.0f) {
        miniMapAbsTime = 0.0f;
        miniMapBlinkActive = !miniMapBlinkActive;
    }

    /**********************************
     * Draw MiniMap for all players   *
     * ******************************** */

    std::vector<Player*>::iterator itPlayer;
    int playerIdx = 0;
    irr::core::dimension2di playerLocation;

    for (itPlayer = this->mPlayerVec.begin(); itPlayer != this->mPlayerVec.end(); ++itPlayer) {
        //draw player position
        playerLocation = CalcPlayerMiniMapPosition(*itPlayer);

        //player 0 is the human player if one is in the race
        if (playerIdx == 0) {
            if (mPlayerVec.at(0)->mHumanPlayer) {
                //player 0 is human player, draw blinking cursor
                irr::video::SColor player1LocationFrameColor(255, 194, 189, 206);  //this color is drawn around the main white block to create a blinking effect

                //for blinking effect draw bigger frame block for player 1
                //only draw it for blinking effect
                if (miniMapBlinkActive) {
                    mInfra->mDriver->draw2DRectangle(player1LocationFrameColor,
                                         core::rect<s32>(playerLocation.Width - 5, playerLocation.Height -5,
                                                         playerLocation.Width + 5, playerLocation.Height + 5));
                }
            }
        }

        //draw the default marker for all available players
        mInfra->mDriver->draw2DRectangle(*mMiniMapMarkerColors.at(playerIdx),
                                 core::rect<s32>(playerLocation.Width - 3, playerLocation.Height - 3,
                                                 playerLocation.Width + 3, playerLocation.Height + 3));

        playerIdx++;
    }
}

void Race::DebugDrawWayPointLinks(bool drawFreeMovementSpace) {
    std::vector<WayPointLinkInfoStruct*>::iterator WayPointLink_iterator;

    //draw all connections between map waypoints for debugging purposes;
    for(WayPointLink_iterator = wayPointLinkVec->begin(); WayPointLink_iterator != wayPointLinkVec->end(); ++WayPointLink_iterator) {
        //draw lines a little bit raised from Terrain, so that the are better visible
        irr::core::vector3df incY(0.0f, 0.15f, 0.0f);

        mDrawDebug->Draw3DLine(
                    (*WayPointLink_iterator)->pLineStruct->A + incY,
                    (*WayPointLink_iterator)->pLineStruct->B + incY,
                    (*WayPointLink_iterator)->pLineStruct->color);

        if (drawFreeMovementSpace) {
            //also draw min/max offset shift limit lines for graphical representation of possible computer player
            //movement area
            mDrawDebug->Draw3DLine(
                        (*WayPointLink_iterator)->pLineStruct->A + incY + (*WayPointLink_iterator)->offsetDirVec *
                        (*WayPointLink_iterator)->minOffsetShiftStart,
                        (*WayPointLink_iterator)->pLineStruct->B + incY + (*WayPointLink_iterator)->offsetDirVec *
                        (*WayPointLink_iterator)->minOffsetShiftEnd,
                        this->mDrawDebug->blue);

            mDrawDebug->Draw3DLine(
                        (*WayPointLink_iterator)->pLineStruct->A + incY + (*WayPointLink_iterator)->offsetDirVec *
                        (*WayPointLink_iterator)->maxOffsetShiftStart,
                        (*WayPointLink_iterator)->pLineStruct->B + incY + (*WayPointLink_iterator)->offsetDirVec *
                        (*WayPointLink_iterator)->maxOffsetShiftEnd,
                        this->mDrawDebug->red);
        }
    }
}

void Race::Render() {
    //we need to draw sky image first, the remaining scene will be drawn on top of it
    DrawSky();

    //draw 3D world coordinate axis with arrows
    mDrawDebug->DrawWorldCoordinateSystemArrows();

    //draw currently active world coordinate forces on player ship
    //playerPhysicsObj->DebugDrawCurrentWorldCoordForces(mDrawDebug, mDrawDebug->green, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);
    //player2PhysicsObj->DebugDrawCurrentWorldCoordForces(mDrawDebug, mDrawDebug->green, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);

    std::list<LineStruct*>::iterator Linedraw_iterator;
    std::vector<CheckPointInfoStruct*>::iterator CheckPoint_iterator;

    std::vector<WayPointLinkInfoStruct*>::iterator WayPointLink_iterator;

    if (DebugShowWaypoints) {
        DebugDrawWayPointLinks(DebugShowFreeMovementSpace);
     }

   /* if ((currPlayerFollow != NULL) && (currPlayerFollow->currClosestWayPointLink.first != NULL)) {
        mDrawDebug->Draw3DLine(
                    currPlayerFollow->currClosestWayPointLink.second, currPlayerFollow->currClosestWayPointLink.second
                    + currPlayerFollow->currClosestWayPointLink.first->offsetDirVec * currPlayerFollow->mCpFollowedWayPointLinkCurrentSpaceRightSide,
                    this->mDrawDebug->pink);

        mDrawDebug->Draw3DLine(
                    currPlayerFollow->currClosestWayPointLink.second,  currPlayerFollow->currClosestWayPointLink.second
                    + currPlayerFollow->currClosestWayPointLink.first->offsetDirVec * currPlayerFollow->mCpFollowedWayPointLinkCurrentSpaceLeftSide,
                    this->mDrawDebug->brown);
    }*/

    std::vector<LineStruct*>::iterator Linedraw_iterator2;

    if (DebugShowWallSegments) {
      //draw all wallsegments for debugging purposes
     mInfra->mDriver->setMaterial(*mDrawDebug->red);
     for(Linedraw_iterator2 = ENTWallsegmentsLine_List->begin(); Linedraw_iterator2 != ENTWallsegmentsLine_List->end(); ++Linedraw_iterator2) {
          mInfra->mDriver->setMaterial(*mDrawDebug->red);
          mInfra->mDriver->draw3DLine((*Linedraw_iterator2)->A, (*Linedraw_iterator2)->B);
       }
     }

    if (DebugShowCheckpoints) {
      //draw all checkpoint lines for debugging purposes
      mInfra->mDriver->setMaterial(*mDrawDebug->blue);
      for(CheckPoint_iterator = checkPointVec->begin(); CheckPoint_iterator != checkPointVec->end(); ++CheckPoint_iterator) {
          mInfra->mDriver->draw3DLine((*CheckPoint_iterator)->pLineStruct->A, (*CheckPoint_iterator)->pLineStruct->B);
      }
    }

      //mPhysics->DrawSelectedCollisionMeshTriangles(player->phobj->GetCollisionArea());
      //mPhysics->DrawSelectedRayTargetMeshTriangles(TestRayTrianglesSelector);

      /*mDriver->setMaterial(*mDrawDebug->green);
      mDriver->draw3DLine(mPhysics->DbgRayTargetLine.start, mPhysics->DbgRayTargetLine.end);*/

      if (DEF_DBG_WALLCOLLISIONS) {
              mDrawDebug->Draw3DTriangle(&playerPhysicsObj->mNearestTriangle,  irr::video::SColor(0, 255, 0,127));
      }

    /*  irr::core::vector2di hlpe;

      irr::core::vector3df pnt1 = this->player->WorldCoordCraftFrontPnt;
      pnt1.Y = this->player->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(pnt1.X, pnt1.Z, hlpe);

      irr::core::vector3df pnt2 = this->player->WorldCoordCraftFrontPnt2;
      pnt2.Y = this->player->mRace->mLevelTerrain->GetCurrentTerrainHeightForWorldCoordinate(pnt2.X, pnt2.Z, hlpe);*/

    /*
      mDrawDebug->Draw3DLine(this->player->WorldCoordCraftFrontPnt, this->player->WorldCoordCraftFrontPnt2,
                             this->mDrawDebug->blue);*/

      /*mDrawDebug->Draw3DLine(pnt1, pnt2,
                             this->mDrawDebug->red);
*/
      /*mDrawDebug->Draw3DLine(this->player->debug.A, this->player->debug.B,
                             this->mDrawDebug->blue);*/

  /*    mDrawDebug->Draw3DLine(this->player->mHMapCollPntData.backRight45deg->wCoordPnt1,
                             this->player->mHMapCollPntData.backRight45deg->wCoordPnt2,
                             this->mDrawDebug->red);*/

    /*  mDrawDebug->Draw3DLine(this->player->mHMapCollPntData.frontLeft45deg->wCoordPnt1, this->player->mHMapCollPntData.frontLeft45deg->wCoordPnt2,
                             this->mDrawDebug->blue);*/

      /*mDrawDebug->Draw3DLine(this->player->phobj->physicState.position, this->player->mHMapCollPntData.backRight45deg->intersectionPnt,
                             this->mDrawDebug->green);*/

      /*mDrawDebug->Draw3DLine(this->player->mHMapCollPntData.backRight45deg->planePnt1,
                             this->player->mHMapCollPntData.backRight45deg->planePnt2,
                             this->mDrawDebug->blue);*/

      /*
      if (this->player->minCeilingFound) {
        mDrawDebug->Draw3DLine(this->player->phobj->physicState.position, this->player->dbgCurrCeilingMinPos,
                             this->mDrawDebug->red);
      }

      mDrawDebug->Draw3DLine(this->player->cameraSensor->wCoordPnt1, this->player->cameraSensor->wCoordPnt2,
                                   this->mDrawDebug->green);

      mDrawDebug->Draw3DLine(this->player->cameraSensor2->wCoordPnt1, this->player->cameraSensor2->wCoordPnt2,
                                   this->mDrawDebug->green);

      mDrawDebug->Draw3DLine(this->player->cameraSensor3->wCoordPnt1, this->player->cameraSensor3->wCoordPnt2,
                                   this->mDrawDebug->green);

      mDrawDebug->Draw3DLine(this->player->cameraSensor4->wCoordPnt1, this->player->cameraSensor4->wCoordPnt2,
                                   this->mDrawDebug->green);

      mDrawDebug->Draw3DLine(this->player->cameraSensor5->wCoordPnt1, this->player->cameraSensor5->wCoordPnt2,
                                   this->mDrawDebug->green);

      mDrawDebug->Draw3DLine(this->player->cameraSensor6->wCoordPnt1, this->player->cameraSensor6->wCoordPnt2,
                                   this->mDrawDebug->green);

      mDrawDebug->Draw3DLine(this->player->cameraSensor7->wCoordPnt1, this->player->cameraSensor7->wCoordPnt2,
                                   this->mDrawDebug->green);*/

      /*
      if (currPlayerFollow != NULL) {

            if (currPlayerFollow->mPathHistoryVec.size() > 0) {
              std::vector<WayPointLinkInfoStruct*>::iterator itPathEl;

             for (itPathEl = currPlayerFollow->mPathHistoryVec.begin(); itPathEl != currPlayerFollow->mPathHistoryVec.end(); ++itPathEl) {
                   mDrawDebug->Draw3DLine((*itPathEl)->pLineStruct->A, (*itPathEl)->pLineStruct->B, (*itPathEl)->pLineStruct->color);
              }

             //     mDrawDebug->Draw3DLine(player2->mFollowPath.at(0)->pLineStruct->A, player2->mFollowPath.at(0)->pLineStruct->B, this->mDrawDebug->blue);
          }
      }*/

     /* if (currPlayerFollow != NULL) {

            if (currPlayerFollow->mCurrentPathSeg.size() > 0) {
              std::vector<WayPointLinkInfoStruct*>::iterator itPathEl;

             for (itPathEl = currPlayerFollow->mCurrentPathSeg.begin(); itPathEl != currPlayerFollow->mCurrentPathSeg.end(); ++itPathEl) {
                   mDrawDebug->Draw3DLine((*itPathEl)->pLineStruct->A, (*itPathEl)->pLineStruct->B, mDrawDebug->blue); //(*itPathEl)->pLineStruct->color);
              }

             //     mDrawDebug->Draw3DLine(player2->mFollowPath.at(0)->pLineStruct->A, player2->mFollowPath.at(0)->pLineStruct->B, this->mDrawDebug->blue);
          }
      }*/


      /*if (currPlayerFollow != NULL) {

              if (this->currPlayerFollow->mCpAvailWayPointLinks.size() > 0) {
                  std::vector<WayPointLinkInfoStruct*>::iterator itPathEl;

                  for (itPathEl = currPlayerFollow->mCpAvailWayPointLinks.begin(); itPathEl != currPlayerFollow->mCpAvailWayPointLinks.end(); ++itPathEl) {
                       mDrawDebug->Draw3DLine((*itPathEl)->pLineStruct->A, (*itPathEl)->pLineStruct->B, this->mDrawDebug->red);
                  }
              }
        }*/

    /*  if (this->player2->mCpCollectablesSeenByPlayer.size() > 0) {
          std::vector<Collectable*>::iterator itColl;
          irr::core::vector3df fixedPos;

          for (itColl = player2->mCpCollectablesSeenByPlayer.begin(); itColl != player2->mCpCollectablesSeenByPlayer.end(); ++itColl) {
               fixedPos = (*itColl)->Position;
               fixedPos.X = -fixedPos.X;
               mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, fixedPos, this->mDrawDebug->pink);
          }
      }*/

       /* if (currPlayerFollow != NULL) {

              if (currPlayerFollow->mCpTargetCollectableToPickUp != NULL) {
                   irr::core::vector3df fixedPos = currPlayerFollow->mCpTargetCollectableToPickUp->Position;
                   fixedPos.X = -fixedPos.X;
                   mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, fixedPos,
                                          this->mDrawDebug->pink);
              }

              if (currPlayerFollow->mCpWayPointLinkClosestToCollectable != NULL) {
                  mDrawDebug->Draw3DLine(currPlayerFollow->mCpWayPointLinkClosestToCollectable->pLineStruct->A,
                                         currPlayerFollow->mCpWayPointLinkClosestToCollectable->pLineStruct->B,
                                         this->mDrawDebug->cyan);
              }
        }*/

        if (DebugShowRegionsAndPointOfInterest) {
                std::list<MapPointOfInterest>::iterator it;

                for (it = this->mLevelRes->PointsOfInterest.begin(); it != this->mLevelRes->PointsOfInterest.end(); ++it) {
                    mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, (*it).Position,
                                           this->mDrawDebug->pink);
                }

                IndicateMapRegions();
        }
     /*
        mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, dbgMiniMapPnt1, this->mDrawDebug->red);
        mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, dbgMiniMapPnt2, this->mDrawDebug->cyan);
        mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, dbgMiniMapPnt3, this->mDrawDebug->pink);
        mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, dbgMiniMapPnt4, this->mDrawDebug->orange);*/

        if (currPlayerFollow != NULL) {
            /*if (currPlayerFollow->currClosestWayPointLink.first != NULL) {
                mDrawDebug->Draw3DLine(currPlayerFollow->phobj->physicState.position, currPlayerFollow->currClosestWayPointLink.first->pLineStruct->A,
                                       mDrawDebug->cyan);

                mDrawDebug->Draw3DLine(currPlayerFollow->phobj->physicState.position, currPlayerFollow->currClosestWayPointLink.first->pLineStruct->B,
                                       mDrawDebug->red);

                mDrawDebug->Draw3DLine(currPlayerFollow->phobj->physicState.position, currPlayerFollow->currClosestWayPointLink.second,
                                       mDrawDebug->blue);

                if (currPlayerFollow->cPCurrentFollowSeg != NULL) {
                    irr::core::vector3df incY2(0.0f, 0.15f, 0.0f);

                    mDrawDebug->Draw3DLine(
                                currPlayerFollow->cPCurrentFollowSeg->pLineStruct->A + incY2,
                                currPlayerFollow->cPCurrentFollowSeg->pLineStruct->B + incY2,
                                this->mDrawDebug->orange);
                }
            }*/

            /*if (currPlayerFollow->mFailedLinks.size() > 0) {
                std::vector<WayPointLinkInfoStruct*>::iterator it3;

                for (it3 = currPlayerFollow->mFailedLinks.begin(); it3 != currPlayerFollow->mFailedLinks.end(); ++it3) {
                    mDrawDebug->Draw3DLine((*it3)->pLineStruct->A, (*it3)->pLineStruct->B,
                                           mDrawDebug->orange);
                }
            }*/


           /* if (currPlayerFollow->cPCurrentFollowSeg != NULL) {
                mDrawDebug->Draw3DLine(currPlayerFollow->cPCurrentFollowSeg->pLineStruct->A, currPlayerFollow->cPCurrentFollowSeg->pLineStruct->B,
                                       mDrawDebug->orange);
            }*/

        }

        //DebugShowAllObstaclePlayers();

        if (DebugShowTriggerRegions) {
            IndicateTriggerRegions();
        }
}

void Race::DebugShowAllObstaclePlayers() {
    std::vector<Player*>::iterator itPlayer;
    std::vector<Player*>::iterator itPlayerObstacle;

    for (itPlayer = this->mPlayerVec.begin(); itPlayer != this->mPlayerVec.end(); ++itPlayer) {
        for (itPlayerObstacle = (*itPlayer)->dbgPlayerInMyWay.begin(); itPlayerObstacle != (*itPlayer)->dbgPlayerInMyWay.end(); ++itPlayerObstacle) {
            mDrawDebug->Draw3DLine(((*itPlayer)->phobj->physicState.position), (*itPlayerObstacle)->phobj->physicState.position, mDrawDebug->orange);
        }
    }
}

//function for debugging
void Race::DebugSelectPlayer(int whichPlayerNr) {
    if (whichPlayerNr < this->mPlayerVec.size()) {
       if (currPlayerFollow != NULL) {
            currPlayerFollow->DebugSelectionBox(false);

            //unhide player model
            currPlayerFollow->UnhideCraft();
       }

       currPlayerFollow = this->mPlayerVec.at(whichPlayerNr);

       if (!mDemoMode) {
           //do we need to hide selected player model
           bool hideModel = currPlayerFollow->DoWeNeedHidePlayerModel();

           if (hideModel) {
               currPlayerFollow->HideCraft();
           }

            currPlayerFollow->DebugSelectionBox(true);
       }

       Hud1Player->SetMonitorWhichPlayer(currPlayerFollow);   
    }
}

void Race::IndicateMapRegions() {
    std::vector<MapTileRegionStruct*>::iterator it;
    irr::core::vector3df pos1;
    irr::core::vector3df pos2;
    irr::core::vector3df pos3;
    irr::core::vector3df pos4;
    irr::core::vector2di cell;

    irr::video::SMaterial *color = this->mDrawDebug->red;

    for (it = this->mLevelRes->mMapRegionVec->begin(); it != this->mLevelRes->mMapRegionVec->end(); ++it) {
       pos1.X = -(*it)->tileXmin * DEF_SEGMENTSIZE;
       pos1.Y = this->mLevelRes->pMap[(*it)->tileXmin][(*it)->tileYmin]->m_Height;
       pos1.Z = (*it)->tileYmin * DEF_SEGMENTSIZE;

       pos2.X = -(*it)->tileXmax * DEF_SEGMENTSIZE;
       pos2.Y = this->mLevelRes->pMap[(*it)->tileXmax][(*it)->tileYmax]->m_Height;
       pos2.Z = (*it)->tileYmax * DEF_SEGMENTSIZE;

       pos3.X = -(*it)->tileXmin * DEF_SEGMENTSIZE;
       pos3.Y = this->mLevelRes->pMap[(*it)->tileXmin][(*it)->tileYmax]->m_Height;
       pos3.Z = (*it)->tileYmax * DEF_SEGMENTSIZE;

       pos4.X = -(*it)->tileXmax * DEF_SEGMENTSIZE;
       pos4.Y = this->mLevelRes->pMap[(*it)->tileXmax][(*it)->tileYmin]->m_Height;
       pos4.Z = (*it)->tileYmin * DEF_SEGMENTSIZE;

       if ((*it)->regionType == LEVELFILE_REGION_CHARGER_FUEL) {
           color = this->mDrawDebug->blue;
       } else if ((*it)->regionType == LEVELFILE_REGION_CHARGER_SHIELD) {
           color = this->mDrawDebug->green;
       } else if ((*it)->regionType == LEVELFILE_REGION_CHARGER_AMMO) {
           color = this->mDrawDebug->orange;
       } else if ((*it)->regionType == LEVELFILE_REGION_START) {
           color = this->mDrawDebug->red;
       }

       mDrawDebug->Draw3DRectangle(pos1, pos3, pos2, pos4, color);
   }
}

void Race::IndicateTriggerRegions() {
    std::vector<MapTileRegionStruct*>::iterator it;
    irr::core::vector3df pos1;
    irr::core::vector3df pos2;
    irr::core::vector3df pos3;
    irr::core::vector3df pos4;
    irr::core::vector2di cell;

    irr::video::SMaterial *color = this->mDrawDebug->red;

    for (it = this->mTriggerRegionVec.begin(); it != this->mTriggerRegionVec.end(); ++it) {
       pos1.X = -(*it)->tileXmin * DEF_SEGMENTSIZE;
       pos1.Y = this->mLevelRes->pMap[(*it)->tileXmin][(*it)->tileYmin]->m_Height;
       pos1.Z = (*it)->tileYmin * DEF_SEGMENTSIZE;

       pos2.X = -(*it)->tileXmax * DEF_SEGMENTSIZE;
       pos2.Y = this->mLevelRes->pMap[(*it)->tileXmax][(*it)->tileYmax]->m_Height;
       pos2.Z = (*it)->tileYmax * DEF_SEGMENTSIZE;

       pos3.X = -(*it)->tileXmin * DEF_SEGMENTSIZE;
       pos3.Y = this->mLevelRes->pMap[(*it)->tileXmin][(*it)->tileYmax]->m_Height;
       pos3.Z = (*it)->tileYmax * DEF_SEGMENTSIZE;

       pos4.X = -(*it)->tileXmax * DEF_SEGMENTSIZE;
       pos4.Y = this->mLevelRes->pMap[(*it)->tileXmax][(*it)->tileYmin]->m_Height;
       pos4.Z = (*it)->tileYmin * DEF_SEGMENTSIZE;

       if ((*it)->regionType == LEVELFILE_REGION_TRIGGERCRAFT) {
           color = this->mDrawDebug->cyan;
       } else if ((*it)->regionType == LEVELFILE_REGION_TRIGGERMISSILE) {
           color = this->mDrawDebug->orange;
       }

       mDrawDebug->Draw3DRectangle(pos1, pos3, pos2, pos4, color);
   }
}

void Race::DebugDrawHeightMapTileOutline(int x, int z, irr::video::SMaterial* color) {
    if ((mDrawDebug != NULL) && (this->mLevelTerrain != NULL)) {
        if (x < 0) {
            x = 0;
        }

        if (z < 0) {
            z = 0;
        }

        int w = this->mLevelTerrain->get_width();
        int h = this->mLevelTerrain->get_heigth();

        if (x > w) {
            x = w - 1;
        }

        if (z > h) {
            z = h - 1;
        }

        //our coordinate system is mirrored for X-Axis
        irr::core::vector3df v1 = this->mLevelTerrain->pTerrainTiles[x][z].vert1->Pos;
        v1.X = -v1.X;
        v1.Y = -v1.Y;

        irr::core::vector3df v2 = this->mLevelTerrain->pTerrainTiles[x][z].vert2->Pos;
        v2.X = -v2.X;
        v2.Y = -v2.Y;

        irr::core::vector3df v3 = this->mLevelTerrain->pTerrainTiles[x][z].vert3->Pos;
        v3.X = -v3.X;
        v3.Y = -v3.Y;

        irr::core::vector3df v4 = this->mLevelTerrain->pTerrainTiles[x][z].vert4->Pos;
        v4.X = -v4.X;
        v4.Y = -v4.Y;

        mDrawDebug->Draw3DRectangle(v1, v2, v3, v4, color);
    }
}

//returns true if succesfull, false otherwise
bool Race::LoadSkyImage(int levelNr, irr::video::IVideoDriver* driver, irr::core::dimension2d<irr::u32> screenResolution) {
    char filename[50];

    strcpy(filename, (char*)"extract/sky/");

    //which sky to load for which level?
    switch (levelNr) {
        case 1: {strcat(filename, (char*)"modsky0-0.png"); break;}
        case 2: {strcat(filename, (char*)"modsky0-1.png"); break;}
        case 3: {strcat(filename, (char*)"modsky0-2.png"); break;}
        case 4: {strcat(filename, (char*)"modsky0-3.png"); break;}
        case 5: {strcat(filename, (char*)"modsky0-4.png"); break;}
        case 6: {strcat(filename, (char*)"modsky0-5.png"); break;}
        case 7: {strcat(filename, (char*)"modsky0-0.png"); break;} //I do not know which sky this map uses right now TODO
        case 8: {strcat(filename, (char*)"modsky0-0.png"); break;} //I do not know which sky this map uses right now TODO
        case 9: {strcat(filename, (char*)"modsky0-0.png"); break;} //I do not know which sky this map uses right now TODO
        default: {
            //unknown levelNr
            return false;
        }
    }

    driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    //load sky image
    mSkyImage = driver->getTexture(filename);

    if (mSkyImage == NULL) {
        //there was a texture loading error
        //just return with false
        return false;
    }

    irr::core::dimension2d<irr::u32> skyImageSize;

    skyImageSize = mSkyImage->getSize();
   /* if ((skyImageSize.Width != screenResolution.Width) ||
        (skyImageSize.Height != screenResolution.Height)) {
        //skyImage size does not fit with selected screen resolution
        return false;
    }*/

    driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    //all ok
    return true;
}

void Race::CleanUpSky() {
    if (mSkyImage != NULL) {
        //free this texture
        mInfra->mDriver->removeTexture(mSkyImage);
        mSkyImage = NULL;
    }
}

void Race::CleanMiniMap() {
    if (baseMiniMap != NULL) {
        //free this texture
        mInfra->mDriver->removeTexture(baseMiniMap);
        baseMiniMap = NULL;
    }

    //also clean up all minimap marker colors
    std::vector<irr::video::SColor*>::iterator itColor;
    irr::video::SColor* pntrColor;

    for (itColor = this->mMiniMapMarkerColors.begin(); itColor != this->mMiniMapMarkerColors.end(); ) {
        pntrColor = (*itColor);

        itColor = mMiniMapMarkerColors.erase(itColor);

        delete pntrColor;
    }
}

//function is called periodically to check if
//the race is already finished, and handles
//everything afterwards
void Race::CheckRaceFinished(irr::f32 deltaTime) {
    //all players finished?
    if (mRaceWasFinished) {
        mRaceFinishedWaitTimeCnter += deltaTime;

        if (mRaceFinishedWaitTimeCnter > DEF_RACE_FINISHED_WAITTIME_SEC) {
            //wait time counter finished, exit race

            //24.03.2025: Add a final race phase where we wait until
            //all currently working animators are finished
            //this->exitRace = true;
            InitiateExitRace();
        }
    } else {
        //race is not finished yet
        if (this->playerRaceFinishedVec.size() >= this->mPlayerVec.size()) {
              //all players went through the finish line
              mRaceWasFinished = true;
        }
    }
}

bool Race::LoadLevel(int loadLevelNr) {
    if ((loadLevelNr < 1) || (loadLevelNr > 9)) {
        cout << "Level number only possible from 1 up to 9!" << endl << std::flush;
        return false;
    }

    int load_texnr = loadLevelNr;
    if (loadLevelNr == 7) load_texnr = 1; // original game has this hardcoded too

   /***********************************************************/
   /* Load selected level file                                */
   /***********************************************************/
   char levelfilename[50];
   char str[20];

   strcpy(levelfilename, "extract/level0-");
   sprintf(str, "%d", loadLevelNr);
   strcat(levelfilename, str);
   strcat(levelfilename, "/level0-");
   strcat(levelfilename, str);
   strcat(levelfilename, "-unpacked.dat");

   char texfilename[50];
   strcpy(texfilename, "extract/level0-");
   sprintf(str, "%d", load_texnr);
   strcat(texfilename, str);
   strcat(texfilename, "/tex");

   /***********************************************************/
   /* Load level textures                                     */
   /***********************************************************/
   mTexLoader = new TextureLoader(mInfra->mDriver, texfilename);

   //load the level data itself
   this->mLevelRes = new LevelFile(levelfilename);

   //was loading level data succesful? if not interrupt
   if (!this->mLevelRes->get_Ready()) {
       cout << "Race::LoadLevel failed, exiting" << endl << std::flush;
       return false;
   }

   char terrainname[50];
   strcpy(terrainname, "Terrain1");

   //Test map save
   //this->mLevelRes->Save(std::string("mapsave.dat"));

   /***********************************************************/
   /* Prepare level terrain                                   */
   /***********************************************************/
   this->mLevelTerrain = new LevelTerrain(terrainname, this->mLevelRes, mInfra->mSmgr, mInfra->mDriver, mTexLoader,
                                          this, this->mGame->enableLightning);

   /***********************************************************/
   /* Create building (cube) Mesh                             */
   /***********************************************************/
   //this routine also generates the column/block collision information inside that
   //we need for collision detection later
   this->mLevelBlocks = new LevelBlocks(this->mLevelTerrain, this->mLevelRes, mInfra->mSmgr, mInfra->mDriver, mTexLoader,
                                        this->mGame->enableLightning);

   //create all level entities
   //this are not only items to pickup by the player
   //but also waypoints, collision information, checkpoints
   //level morph information and so on...
   createLevelEntities();

   //The second part of the terrain initialization can only be done
   //after the map entities are loaded in another part of the code
   //Because we need to know where the morph areas are, to be able to put
   //dynamic parts of the terrain mesh into their own Meshbuffers and SceneNodes
   //for performance improvement reasons
   mLevelTerrain->FinishTerrainInitialization();

   if (this->mLevelTerrain->Terrain_ready == false) {
       //something went wrong with the terrain loading, exit application
       return false;
   }

   //load sky image for selected level
   if (!LoadSkyImage(loadLevelNr, mInfra->mDriver, mInfra->mScreenRes)) {
       //error loading sky image, do something about it!
       return false;
   }

  // driver->setFog(video::SColor(0,138,125,81), video::EFT_FOG_LINEAR, 100, 250, .03f, false, true);

  //Final data calculation for all checkpoints
  //Find for each waypoint normal direction of race
  //so that we know if the player passes checkpoints
  //in forwards or reverse direction
  CheckPointPostProcessing();

  createWallCollisionData();

  //create final overall triangle selectors for collision
  //with physics
  createFinalCollisionData();

  return true;
}

//creates all data that is needed to detect player crafts
//flying through defined map checkpoints
void Race::createCheckpointMeshData(CheckPointInfoStruct &newStruct) {

    int vind = 0;
    int iind = 0;
    irr::f32 plane_h = 15.0f; //plane height

    video::SColor cubeColour4(255,255,255,255);

    irr::core::vector3df normal;
    irr::core::vector3df LineUnityVect;
    irr::core::vector3df Y_Unity_vec(0.0f, 1.0f, 0.0f);

    SMesh* checkPointMesh;

    irr::f32 w_th = 0.01f; //define thickness of Mesh

    //create a checkpoint mesh scenenode for each of the checkpoint lines contained within the level
    //create a new mesh for the checkpoint
    checkPointMesh = new SMesh();

    //create a new MeshBuffer
    SMeshBuffer *Mbuf = new SMeshBuffer();
    Mbuf->Indices.set_used(6*2);
    Mbuf->Vertices.set_used(4*2);
    Mbuf->Material.Wireframe = true;

    LineUnityVect = ((*newStruct.pLineStruct).B-(*newStruct.pLineStruct).A).normalize();
    normal = LineUnityVect.crossProduct(Y_Unity_vec).normalize();

    Mbuf->Vertices[vind] = video::S3DVertex(newStruct.pLineStruct->A.X, newStruct.pLineStruct->A.Y - plane_h, newStruct.pLineStruct->A.Z, normal.X, normal.Y, normal.Z, cubeColour4, 0.0f, 1.0f);
    Mbuf->Vertices[vind+1] = video::S3DVertex(newStruct.pLineStruct->B.X, newStruct.pLineStruct->B.Y - plane_h, newStruct.pLineStruct->B.Z, normal.X, normal.Y, normal.Z, cubeColour4, 1.0f, 1.0f);
    Mbuf->Vertices[vind+2] = video::S3DVertex(newStruct.pLineStruct->B.X, newStruct.pLineStruct->B.Y + plane_h, newStruct.pLineStruct->B.Z, normal.X, normal.Y, normal.Z, cubeColour4, 1.0f, 0.0f);
    Mbuf->Vertices[vind+3] = video::S3DVertex(newStruct.pLineStruct->A.X, newStruct.pLineStruct->A.Y + plane_h, newStruct.pLineStruct->A.Z, normal.X, normal.Y, normal.Z, cubeColour4, 0.0f, 0.0f);

    Mbuf->Vertices[vind+4] = video::S3DVertex(newStruct.pLineStruct->A.X - normal.X* w_th , newStruct.pLineStruct->A.Y - plane_h, newStruct.pLineStruct->A.Z - normal.Z* w_th , normal.X, normal.Y, normal.Z, cubeColour4, 0.0f, 1.0f);
    Mbuf->Vertices[vind+5] = video::S3DVertex(newStruct.pLineStruct->B.X - normal.X* w_th, newStruct.pLineStruct->B.Y - plane_h, newStruct.pLineStruct->B.Z - normal.Z* w_th, normal.X, normal.Y, normal.Z, cubeColour4, 1.0f, 1.0f);
    Mbuf->Vertices[vind+6] = video::S3DVertex(newStruct.pLineStruct->B.X - normal.X* w_th, newStruct.pLineStruct->B.Y + plane_h, newStruct.pLineStruct->B.Z - normal.Z* w_th, normal.X, normal.Y, normal.Z, cubeColour4, 1.0f, 0.0f);
    Mbuf->Vertices[vind+7] = video::S3DVertex(newStruct.pLineStruct->A.X - normal.X* w_th, newStruct.pLineStruct->A.Y + plane_h, newStruct.pLineStruct->A.Z - normal.Z* w_th, normal.X, normal.Y, normal.Z, cubeColour4, 0.0f, 0.0f);

    //first triangle to form quad
    Mbuf->Indices[iind] = vind;
    Mbuf->Indices[iind+1] = vind+1;
    Mbuf->Indices[iind+2] = vind+3;

    //2nd triangle to form quad
    Mbuf->Indices[iind+3] = vind+3;
    Mbuf->Indices[iind+4] = vind+1;
    Mbuf->Indices[iind+5] = vind+2;

    //3rd triangle to form quad
    Mbuf->Indices[iind+6] = vind+4;
    Mbuf->Indices[iind+7] = vind+5;
    Mbuf->Indices[iind+8] = vind+7;

    //4nd triangle to form quad
    Mbuf->Indices[iind+9] = vind+7;
    Mbuf->Indices[iind+10] = vind+5;
    Mbuf->Indices[iind+11] = vind+6;

    Mbuf->recalculateBoundingBox();

    checkPointMesh->addMeshBuffer(Mbuf);

    Mbuf->drop();

    checkPointMesh->recalculateBoundingBox();

    newStruct.Mesh = checkPointMesh;

    //now create a MeshSceneNode
    newStruct.SceneNode = mInfra->mSmgr->addMeshSceneNode(checkPointMesh, 0);

    //hide the collision mesh that the player does not see it
    newStruct.SceneNode->setVisible(false);
    newStruct.SceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, true);
    //newStruct.SceneNode->setDebugDataVisible(EDS_BBOX);
}

void Race::AddCheckPoint(EntityItem entity) {
    LineStruct *line = new LineStruct;
    line->A = entity.getCenter();

    //my Irrlicht coordinate system is swapped at the x axis; correct this issue for getOffsetX with a - sign
    line->B = entity.getCenter() + irr::core::vector3d<irr::f32>(-entity.getOffsetX(), 0, entity.getOffsetY());

    //line->name.clear();
    //line->name.append("Checkpoint line ");
    //line->name.append(std::to_string(entity.get_ID()));
    line->name = new char[100];
    sprintf(&line->name[0], "Checkpoint line %d", entity.get_ID());

    //create new Checkpoint info struct
    CheckPointInfoStruct* newStruct = new CheckPointInfoStruct;

    //remember pointer to Entity object
    newStruct->pEntity = &entity;
    //keep 3D line for debugging purposes and
    //mesh creation
    newStruct->pLineStruct = line;
    //store value of checkpoint
    newStruct->value = entity.getValue();

    //Also create and add SceneNode
    createCheckpointMeshData(*newStruct);

    //we can not define the race direction yet for the
    //checkpoint as we are not sure if we have all
    //waypoints in the level defined yet
    //therefore simply set direction vector to all 0 right now
    //this value needs to be figured out later in some kind of
    //postprocessing!
    newStruct->RaceDirectionVec.set(0.0f, 0.0f, 0.0f);

    //all created, add Checkpoint to our list
    this->checkPointVec->push_back(newStruct);
}

//is called in the destructor to free all checkpoint
//related stuff
void Race::CleanUpAllCheckpoints() {
    std::vector<CheckPointInfoStruct*>::iterator it;
    CheckPointInfoStruct* pntr;

    if (this->checkPointVec->size() > 0) {
        for (it = this->checkPointVec->begin(); it != this->checkPointVec->end(); ) {
           pntr = (*it);

           it = this->checkPointVec->erase(it);

           //remove the SceneNode from SceneManager
           pntr->SceneNode->remove();

           //cleanup the Mesh
           mInfra->mSmgr->getMeshCache()->removeMesh(pntr->Mesh);

           //delete name inside LineStruct
           delete[] pntr->pLineStruct->name;

           //delete the LineStruct
           delete pntr->pLineStruct;
        }
    }

    //free also the vector itself
    delete checkPointVec;
}

void Race::CleanUpWayPointLinks(std::vector<WayPointLinkInfoStruct*> &vec) {
    std::vector<WayPointLinkInfoStruct*>::iterator it;
    WayPointLinkInfoStruct* pntr;

    if (vec.size() > 0) {
        for (it = vec.begin(); it != vec.end(); ) {
           pntr = (*it);

           it = vec.erase(it);

           //delete name inside LineStruct
           delete[] pntr->pLineStruct->name;

           //delete the LineStruct
           delete pntr->pLineStruct;

           //delete name inside LineExtStruct
           delete[] pntr->pLineStructExtended->name;

           //delete the LineExtStruct
           delete pntr->pLineStructExtended;
        }
    }

    //free also the vector itself
    delete &vec;
}

//Helper function that is used in many different ways for computer player control
//The purpose of this function is that it takes a 3D direction vector, it ignores
//the Y-Axis (simply looks from the top at the play field in 2D), and then assigns the
//2D orientation of the specified direction vector a unique float number for the angle
//this direction vector creates with the X-Axis; This number starts from 0.0f at the
//X-Axis and increases up to 360.0f in counter-clockwise direction; At 360.0f the value
//wraps over and starts again at 0.0f, and negative angles start again counting down from 360.0f
irr::f32 Race::GetAbsOrientationAngleFromDirectionVec(irr::core::vector3df dirVector, bool correctAngleOutsideRange) {
   irr::f32 dotProductZAxis = dirVector.dotProduct(*this->zAxisDirVector);

   irr::f32 hlpVal = dirVector.dotProduct(*this->xAxisDirVector);
   irr::f32 angleRad = acosf(hlpVal);

   irr::f32 angleResult  = (angleRad / irr::core::PI) * 180.0f;

   if (dotProductZAxis < 0.0f) {
       angleResult = (180.0f - angleResult) + 180.0f;
   }

   if (correctAngleOutsideRange) {

    if (angleResult > 360.0f)
        angleResult -= 360.0f;

    if (angleResult < 0.0f)
        angleResult += 360.0f;
   }

   return angleResult;
}

//This routine uses all defined waypoints to figure out for each Checkpoint
//what the normal expected race direction is
//we also try to link all waypoint links together, that means each link
//points to the next link in a path
void Race::CheckPointPostProcessing() {
    std::vector<CheckPointInfoStruct*>::iterator it;
    irr::core::line3df line3D;
    WayPointLinkInfoStruct *fndLink = NULL;
    std::vector<WayPointLinkInfoStruct*>::iterator it1;
    std::vector<WayPointLinkInfoStruct*>::iterator it2;

    for (it1 = this->wayPointLinkVec->begin(); it1 != this->wayPointLinkVec->end(); ++it1) {
       for (it2 = this->wayPointLinkVec->begin(); it2 != this->wayPointLinkVec->end(); ++it2) {
           //are this 2 waypoints links linked together?
           if ((*it1)->pEndEntity->get_ID() == (*it2)->pStartEntity->get_ID()) {
              //yes, link them
              (*it1)->pntrPathNextLink = (*it2);

               //also store direction vector of next waypoint
               //segment in this struct, to allow easier
               //computer player control later
               //(*it1)->PathNextLinkDirectionVec = (*it2)->LinkDirectionVec;
           }
        }
    }

    //secondly find normal race direction through all checkpoints
    //iterate through all available checkpoints
    for (it = checkPointVec->begin(); it != checkPointVec->end(); ++it) {
        //to find normal travel direction the idea is the following
        //there should be a waypoint line that intersects with our checkPoint Mesh
        //if we look at the values of the two waypoints at the end and beginning of this line
        //we can figure out the normal race direction, as the values will be increasing integers
        //in default travel direction
        irr::core::aabbox3d bbox = (*it)->SceneNode->getTransformedBoundingBox();

        fndLink = NULL;
        irr::core::line3df linepiece;
        irr::core::vector3df vecPiece;
        int pieceFound;

        //iterate through all available waypoint lines
        for (it2 = this->wayPointLinkVec->begin(); it2 != this->wayPointLinkVec->end(); ++it2) {
            //we should find a waypoint link line that intersects with our checkpoint Mesh
            //from this line we can find the direction
            line3D.start = (*it2)->pLineStruct->A;
            line3D.end = (*it2)->pLineStruct->B;

            if (bbox.intersectsWithLine(line3D)) {
                //we found an intersecting line
                //remember checkpoint in current line as well
                (*it2)->pntrCheckPoint = (*it);

                //figure out and store distance waypointLink Start
                //point to point where line crosses the checkpoint
                //to do this just iterate over much shorter line pieces, and
                //see where the shorter pieces hit the checkpoint bounding box

                vecPiece = (line3D.end - line3D.start) / 10.0f;

                pieceFound = -1;

                for (int piece = 0; piece < 10; piece++) {
                    linepiece.start = line3D.start + vecPiece * piece;
                    linepiece.end = line3D.start + vecPiece * (piece + 1);

                    if (bbox.intersectsWithLine(linepiece)) {
                        pieceFound = piece;
                        break;
                    }
                }

                if (pieceFound == -1) {
                    //did not find correct location, place into middle as a workaround!
                    (*it2)->distanceStartLinkToCheckpoint =
                            (((*it)->pLineStruct->A + (*it)->pLineStruct->B) * irr::core::vector3df(0.5f, 0.5f, 0.5f) - (*it2)->pLineStruct->A).getLength();
                } else {
                    //we found the location, set exact location of checkpoint
                    (*it2)->distanceStartLinkToCheckpoint =
                            ((linepiece.start + linepiece.end) * irr::core::vector3df(0.5f, 0.5f, 0.5f) - (*it2)->pLineStruct->A).getLength();
                }

                //just set fndLink first time
               // if (fndLink == NULL) {
                    fndLink = (*it2);
                    //break;
                //}
            }
        }

        if (fndLink != NULL) {
              //calculate and set direction vector for checkpoint
              (*it)->RaceDirectionVec = (fndLink->pLineStruct->B - fndLink->pLineStruct->A).normalize();
        }
    }

    //now run a third step, where we look at situations where a two waypoint links after one another
    //are both assigned the same checkpoint, we do not want to have this situation, because this leads to jumpd
    //when we calculate the remaining distance to the next checkpoint for each player; we need this calculation for
    //race position determination later
    //if we have the same checkpoint assigned to two links in sequence, then remove the second occurence again
    //important note: it must be possible to link a checkpoint to two or more waypoint links (an example is the ammo charger
    //area in level 1 where this occurs) which are not one after another, but as in this case parallel independent
    //ways/paths for the player
    for (it1 = this->wayPointLinkVec->begin(); it1 != this->wayPointLinkVec->end(); ++it1) {
           if (((*it1)->pntrCheckPoint != NULL) && ((*it1)->pntrPathNextLink != NULL)) {
              if ((*it1)->pntrCheckPoint == (*it1)->pntrPathNextLink->pntrCheckPoint) {
                  //we have found this situation
                  //prefer to delete link to checkpoint in the waypoint link that follows after the checkpoint
                  (*it1)->pntrPathNextLink->pntrCheckPoint = NULL;
                  (*it1)->pntrPathNextLink->distanceStartLinkToCheckpoint = 0.0f;
              }
        }
    }
}

//creates all data that is needed for level wall collision detection
//based on collision info contained within the level file entities
void Race::createWallCollisionData() {

     std::vector<LineStruct*>::iterator Line_it;

     int vind = 0;
     int iind = 0;
     irr::f32 plane_h = 15.0f; //plane height

     video::SColor cubeColour4(255,255,255,255);

     irr::core::vector3df normal;
     irr::core::vector3df LineUnityVect;
     irr::core::vector3df Y_Unity_vec(0.0f, 1.0f, 0.0f);

     irr::f32 w_th = 0.01f; //define a collision wall thickness

     //create a new wall collision Mesh for Irrlicht
     wallCollisionMesh = new SMesh();

     //now create wall collision mesh based on the level entities
     for(Line_it = ENTWallsegmentsLine_List->begin(); Line_it != ENTWallsegmentsLine_List->end(); ++Line_it) {
        //create a new MeshBuffer
        SMeshBuffer *Mbuf = new SMeshBuffer();
        Mbuf->Indices.set_used(6*2);
        Mbuf->Vertices.set_used(4*2);
        Mbuf->Material.Wireframe = true;

        LineUnityVect = ((*Line_it)->B-(*Line_it)->A).normalize();
        normal = LineUnityVect.crossProduct(Y_Unity_vec).normalize();

        Mbuf->Vertices[vind] = video::S3DVertex((*Line_it)->A.X, (*Line_it)->A.Y - plane_h, (*Line_it)->A.Z, normal.X, normal.Y, normal.Z, cubeColour4, 0.0f, 1.0f);
        Mbuf->Vertices[vind+1] = video::S3DVertex((*Line_it)->B.X, (*Line_it)->B.Y - plane_h, (*Line_it)->B.Z, normal.X, normal.Y, normal.Z, cubeColour4, 1.0f, 1.0f);
        Mbuf->Vertices[vind+2] = video::S3DVertex((*Line_it)->B.X, (*Line_it)->B.Y + plane_h, (*Line_it)->B.Z, normal.X, normal.Y, normal.Z, cubeColour4, 1.0f, 0.0f);
        Mbuf->Vertices[vind+3] = video::S3DVertex((*Line_it)->A.X, (*Line_it)->A.Y + plane_h, (*Line_it)->A.Z, normal.X, normal.Y, normal.Z, cubeColour4, 0.0f, 0.0f);

        Mbuf->Vertices[vind+4] = video::S3DVertex((*Line_it)->A.X - normal.X* w_th , (*Line_it)->A.Y - plane_h, (*Line_it)->A.Z - normal.Z* w_th , normal.X, normal.Y, normal.Z, cubeColour4, 0.0f, 1.0f);
        Mbuf->Vertices[vind+5] = video::S3DVertex((*Line_it)->B.X - normal.X* w_th, (*Line_it)->B.Y - plane_h, (*Line_it)->B.Z - normal.Z* w_th, normal.X, normal.Y, normal.Z, cubeColour4, 1.0f, 1.0f);
        Mbuf->Vertices[vind+6] = video::S3DVertex((*Line_it)->B.X - normal.X* w_th, (*Line_it)->B.Y + plane_h, (*Line_it)->B.Z - normal.Z* w_th, normal.X, normal.Y, normal.Z, cubeColour4, 1.0f, 0.0f);
        Mbuf->Vertices[vind+7] = video::S3DVertex((*Line_it)->A.X - normal.X* w_th, (*Line_it)->A.Y + plane_h, (*Line_it)->A.Z - normal.Z* w_th, normal.X, normal.Y, normal.Z, cubeColour4, 0.0f, 0.0f);

        //first triangle to form quad
        Mbuf->Indices[iind] = vind;
        Mbuf->Indices[iind+1] = vind+1;
        Mbuf->Indices[iind+2] = vind+3;

        //2nd triangle to form quad
        Mbuf->Indices[iind+3] = vind+3;
        Mbuf->Indices[iind+4] = vind+1;
        Mbuf->Indices[iind+5] = vind+2;

        //3rd triangle to form quad
        Mbuf->Indices[iind+6] = vind+4;
        Mbuf->Indices[iind+7] = vind+5;
        Mbuf->Indices[iind+8] = vind+7;

        //4nd triangle to form quad
        Mbuf->Indices[iind+9] = vind+7;
        Mbuf->Indices[iind+10] = vind+5;
        Mbuf->Indices[iind+11] = vind+6;

        Mbuf->recalculateBoundingBox();

        wallCollisionMesh->addMeshBuffer(Mbuf);

        Mbuf->drop();

        wallCollisionMesh->recalculateBoundingBox();
   }

    //now create a OctreeSceneNode for the wall collision Mesh
    wallCollisionMeshSceneNode = mInfra->mSmgr->addOctreeSceneNode(wallCollisionMesh, 0, IDFlag_IsPickable);

    //hide the collision mesh that the player does not see it
    wallCollisionMeshSceneNode->setVisible(DEF_DBG_WALLCOLLISIONS);

    wallCollisionMeshSceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, true);
    //wallCollisionMeshSceneNode->setDebugDataVisible(EDS_BBOX);
}

//takes the precalculated wall and column collision data
//and creates the final triangle selector out of it for the
//physics later
void Race::createFinalCollisionData() {

   triangleSelectorWallCollision = mInfra->mSmgr->createOctreeTriangleSelector(
                wallCollisionMesh, wallCollisionMeshSceneNode, 128);
   wallCollisionMeshSceneNode->setTriangleSelector(triangleSelectorWallCollision);

   //only add blocks with collision detection to our column triangle selector
   //so that blocks that should not have collision detection are not part of it
   triangleSelectorColumnswCollision = mInfra->mSmgr->createOctreeTriangleSelector(
                this->mLevelBlocks->blockMeshForCollision, this->mLevelBlocks->BlockCollisionSceneNode, 128);
   this->mLevelBlocks->BlockCollisionSceneNode->setTriangleSelector(triangleSelectorColumnswCollision);

   //also create a triangle selector for blocks without collision detection
   //we use this for ray casting (for example to find target of machine gun)
   triangleSelectorColumnswoCollision = mInfra->mSmgr->createOctreeTriangleSelector(
                this->mLevelBlocks->blockMeshWithoutCollision, this->mLevelBlocks->BlockWithoutCollisionSceneNode, 128);
   this->mLevelBlocks->BlockWithoutCollisionSceneNode->setTriangleSelector(triangleSelectorColumnswoCollision);

   triangleSelectorStaticTerrain = mInfra->mSmgr->createOctreeTriangleSelector(
               this->mLevelTerrain->myStaticTerrainMesh, this->mLevelTerrain->StaticTerrainSceneNode, 128);
  this->mLevelTerrain->StaticTerrainSceneNode->setTriangleSelector(triangleSelectorStaticTerrain);

   triangleSelectorDynamicTerrain = mInfra->mSmgr->createOctreeTriangleSelector(
               this->mLevelTerrain->myDynamicTerrainMesh, this->mLevelTerrain->DynamicTerrainSceneNode, 128);
  this->mLevelTerrain->DynamicTerrainSceneNode->setTriangleSelector(triangleSelectorDynamicTerrain);
}

void Race::CheckPlayerCollidedCollectible(Player* player, irr::core::aabbox3d<f32> playerBox) {
    //ony allow player which is not currently broken down to collect
    //collectables
    if (player->mPlayerStats->mPlayerCurrentState == STATE_PLAYER_BROKEN)
        return;

    std::vector<Collectable*>::iterator it;

    for (it = ENTCollectablesVec->begin(); it != ENTCollectablesVec->end(); ++it) {
        //only allow player to collect currently visible collectibles
        if ((*it)->GetIfVisible()) {
            //does player bounding box intersect the bounding box of the collectible
            //item?
            if (playerBox.intersectsWithBox((*it)->boundingBox)) {
                //yes, player does touch the collectible

                //tell player object that this item can
                //be collected, and if so alter the players stats
                //function returns true if the player actually collected
                //this item, false otherwise
                if (player->CollectedCollectable((*it))) {
                    //tell Collectible that is was collected
                    (*it)->PickedUp();
                }
            }
        }
    }
}

void Race::createLevelEntities() {

    ENTWaypoints_List = new std::vector<EntityItem*>;
    ENTWaypoints_List->clear();

    ENTWallsegments_List = new std::list<EntityItem*>;
    ENTWallsegments_List->clear();

    ENTWallsegmentsLine_List = new std::vector<LineStruct*>;
    ENTWallsegmentsLine_List->clear();

    ENTTriggers_List = new std::list<EntityItem*>;
    ENTTriggers_List->clear();

    ENTCollectablesVec = new std::vector<Collectable*>;
    ENTCollectablesVec->clear();

    //create all level entities
    for(std::vector<EntityItem*>::iterator loopi = this->mLevelRes->Entities.begin(); loopi != this->mLevelRes->Entities.end(); ++loopi) {
        createEntity(*loopi, this->mLevelRes, this->mLevelTerrain, this->mLevelBlocks, mInfra->mDriver);
    }
}

void Race::UpdateMorphs(irr::f32 frameDeltaTime) {
    std::list<Morph*>::iterator itMorph;

    for (itMorph = Morphs.begin(); itMorph != Morphs.end(); ++itMorph) {
        (*itMorph)->Update(frameDeltaTime);
    }
}

void Race::UpdateTimers(irr::f32 frameDeltaTime) {
    std::vector<Timer*>::iterator itTimer;

    for (itTimer = mTimerVec.begin(); itTimer != mTimerVec.end(); ++itTimer) {
        (*itTimer)->Update(frameDeltaTime);
    }
}

void Race::UpdateExternalCameras() {
    std::vector<Camera*>::iterator itCamera;
    std::vector<Player*>::iterator itPlayer;

    //first iterate through all players, and try to assign an
    //external camera for each player
    for (itPlayer = mPlayerVec.begin(); itPlayer != mPlayerVec.end(); ++itPlayer) {
         (*itPlayer)->externalCamera = NULL;

         for (itCamera = mCameraVec.begin(); itCamera != mCameraVec.end(); ++itCamera) {
             if ((*itCamera)->CanIObserveLocation((*itPlayer)->phobj->physicState.position)) {
                     //yes, this external camera can see this player
                     (*itPlayer)->externalCamera = (*itCamera);

                     //also command this external camera to focus on this specific
                     //player
                     (*itCamera)->SetTargetPlayer(*itPlayer);
                     break;
              } else {
                 //if we are not able to observe this player, but this player is our focus right
                 //now, remove our focus as well
                 if ((*itCamera)->mFocusAtPlayer == (*itPlayer)) {
                     (*itCamera)->SetTargetPlayer(NULL);
                 }
             }
         }
    }
}

void Race::FindNextPlayerToFollowInDemoMode() {
    std::vector<Player*>::iterator it;

    mFollowPlayerDemoMode = NULL;

    for (it = mPlayerVec.begin(); it != mPlayerVec.end(); ++it) {
        //is there an external camera available for this player currently,
        //and the player is not stuck (we do not want to highlight the fact
        //that we have not the best computer player controls and a stuck player :) )
        if (((*it)->externalCamera != NULL) && (!(*it)->mCpPlayerCurrentlyStuck)) {
            //ok, lets follow this player now
            mFollowPlayerDemoMode = (*it);
            mFollowPlayerDemoModeTimeCounter = 0.0f;

            break;
        }
    }
}

void Race::ManageCameraDemoMode(irr::f32 deltaTime) {
    irr::scene::ICameraSceneNode* activeCam = NULL;

    //do we need to find a new player to follow
    //with an external camera?
    if (mFollowPlayerDemoMode == NULL) {
        FindNextPlayerToFollowInDemoMode();
    } else {
        //we currently follow a player
        //is it time to change again? or did the player we follow loose the external
        //camera
        mFollowPlayerDemoModeTimeCounter += deltaTime;

        if ((mFollowPlayerDemoModeTimeCounter > DEF_RACE_DEMOMODE_MAXTIMEFOLLOWPLAYER) ||
            (mFollowPlayerDemoMode->externalCamera == NULL)) {
              //we need to find the next available player to follow
              FindNextPlayerToFollowInDemoMode();
        }
    }

    if (playerCamera) {
        //get active camera of player we currently follow
        //in demo mode
        if (this->mFollowPlayerDemoMode != NULL) {
            //update external camera focus
            this->mFollowPlayerDemoMode->externalCamera->Update();

            activeCam = this->mFollowPlayerDemoMode->externalCamera->mCamSceneNode;
        }
    } else {
        //free moving camera to inspect the level/map
        activeCam = mCamera;
    }

    if (activeCam == NULL)
        return;

    //has the active camera changed?
    //if so change it for rendering
    if (activeCam != currActiveCamera) {
        mInfra->mSmgr->setActiveCamera(activeCam);
        currActiveCamera = activeCam;
    }

    UpdateSoundListener();
}

void Race::ManagePlayerCamera() {
    irr::scene::ICameraSceneNode* activeCam = NULL;
    bool hidePlayerModel;

    if (playerCamera) {
        //get active camera of currently selected
        //player, and check if it has changed
        if (this->currPlayerFollow != NULL) {
            activeCam = this->currPlayerFollow->DeliverActiveCamera();

            //do we need to hide the player model?
            hidePlayerModel = this->currPlayerFollow->DoWeNeedHidePlayerModel();
        }
    } else {
        //free moving camera to inspect the level/map
        activeCam = mCamera;
    }

    if (activeCam == NULL)
        return;

    //has the active camera changed?
    //if so change it for rendering
    if (activeCam != currActiveCamera) {
        //when we change camera and new camera is cockpit view of
        //this player, then hide this players craft so that we do not
        //see it in our own camera
        if (playerCamera) {
            if (hidePlayerModel) {
                this->currPlayerFollow->HideCraft();
            } else {
                this->currPlayerFollow->UnhideCraft();
            }
        }

        mInfra->mSmgr->setActiveCamera(activeCam);
        currActiveCamera = activeCam;
    }

    UpdateSoundListener();
}

void Race::UpdateSoundListener() {
    //tell soundEngine where the current sound listener
    //is positioned (listener is the location of the
    //currently selected camera)
    if (currActiveCamera != NULL) {
        irr::core::vector3df target = currActiveCamera->getTarget();
        irr::core::vector3df camPos = currActiveCamera->getAbsolutePosition();

        irr::core::vector3df camDirVec = (target - camPos);

        this->mSoundEngine->UpdateListenerLocation(camPos, camDirVec);
    }
}

void Race::AddWayPoint(EntityItem *entity, EntityItem *next) {
    //irr::f32 boxSize = 0.04f;

    //if we have a next element to link the two waypoints
    //add link to the waypoint link list here
    if (next != NULL) {
        WayPointLinkInfoStruct* newStruct = new WayPointLinkInfoStruct();

        //store start and end waypoint entity object
        newStruct->pStartEntity = entity;
        newStruct->pEndEntity = next;

        //create and store a 3D line for later drawing and
        //debugging, also calculations will be done with this struct
        LineStruct *line = new LineStruct;

        line->A = entity->getCenter();
        line->B = next->getCenter();

        //line->name.clear();
        //line->name.append("Waypoint line ");
        //line->name.append(std::to_string(entity->get_ID()));
        //line->name.append(" to ");
        //line->name.append(std::to_string(next->get_ID()));
        line->name = new char[100];
        sprintf(&line->name[0], "Waypoint line %d to %d",
                entity->get_ID(), next->get_ID());

        //set white as default color
        line->color = mDrawDebug->white;

        irr::core::vector3df vec3D = (line->B - line->A);

        //precalculate and store length as we will need this very often
        //during the game loop for race position update
        newStruct->length3D = vec3D.getLength();
        vec3D.normalize();

        newStruct->pLineStruct = line;
        //store precalculated direction vector
        newStruct->LinkDirectionVec = vec3D;

        //precalculate a direction vector which stands at a 90 degree
        //angle at the original waypoint direction vector, and always points
        //to the right direction when looking into race direction
        //this direction vector is later used during the game to offset the player
        //path sideways
        newStruct->offsetDirVec = newStruct->LinkDirectionVec.crossProduct(-*yAxisDirVector).normalize();

        //Idea: extend the lines a little bit further outwards at
        //both ends, so that when we project the players position on
        //the different segments later we always find a valid segment
        LineStruct *lineExt = new LineStruct;

        lineExt->A = line->A - newStruct->LinkDirectionVec * 0.5f;
        lineExt->B = line->B + newStruct->LinkDirectionVec * 0.5f;

        lineExt->name = new char[100];
        sprintf(&lineExt->name[0], "Ext Waypoint line %d to %d",
                entity->get_ID(), next->get_ID());

        //set white as default color
        lineExt->color = mDrawDebug->white;

        newStruct->pLineStructExtended = lineExt;

        //add new waypoint link info struct to vector of all
        //waypoint links
        this->wayPointLinkVec->push_back(newStruct);
    }

    //we also keep a list of all waypoint pointers
    ENTWaypoints_List->push_back(entity);
}

void Race::AddTimer(EntityItem *entity) {
    Timer* newTimer = new Timer(entity, this);

    this->mTimerVec.push_back(newTimer);
}

void Race::AddCamera(EntityItem *entity) {
    Camera* newCamera = new Camera(this, entity, mInfra->mSmgr);

    //only set cameras to active in demo mode
    newCamera->SetActive(true);

    this->mCameraVec.push_back(newCamera);
}

void Race::AddExplosionEntity(EntityItem *entity) {
    ExplosionEntity* newExplosion = new ExplosionEntity(entity, this);

    this->mExplosionEntityVec.push_back(newExplosion);
}

void Race::AddTrigger(EntityItem *entity) {
    /*w = entity.OffsetX + 1f;
    h = entity.OffsetY + 1f;
    box = new Box(0, 0, 0, w, 2, h, new Vector4(0.9f, 0.3f, 0.6f, 0.5f));
    box.Position = entity.Pos + Vector3.UnitY * 0.01f;
    Entities.AddNode(box);*/

    irr::u8 regionType = LEVELFILE_REGION_UNDEFINED;

    MapTileRegionStruct *newTriggerRegion = new MapTileRegionStruct();

    int offsetXCells = (int)(entity->getOffsetX() / DEF_SEGMENTSIZE);
    int offsetYCells = (int)(entity->getOffsetY() / DEF_SEGMENTSIZE);

    irr::core::vector2di tileMin;

    tileMin.X = entity->getCell().X;
    tileMin.Y = entity->getCell().Y;

    irr::core::vector2di tileMax = tileMin;

    if (entity->getEntityType() == Entity::EntityType::TriggerCraft) {
        regionType = LEVELFILE_REGION_TRIGGERCRAFT;

        //craft trigger can trigger so often as it wants to
        newTriggerRegion->mOnlyTriggerOnce = false;
        newTriggerRegion->mAlreadyTriggered = false;

        tileMax.X += offsetXCells;
        tileMax.Y += offsetYCells;
    }

    if (entity->getEntityType() == Entity::EntityType::TriggerRocket) {
        regionType = LEVELFILE_REGION_TRIGGERMISSILE;

        //let rocket trigger only trigger once
        //trigger more often does not really make sense
        newTriggerRegion->mOnlyTriggerOnce = true;
        newTriggerRegion->mAlreadyTriggered = false;

        //in the existing maps it seems default a missile trigger region
        //always has offsetX and offsetY set to 0; this means the trigger is only
        //in a single cell; This is very hard to hit
        //therefore in this case I decided to change offsetX and offsetY to a higher
        //value here
        if ((offsetXCells == 0) || (offsetYCells == 0)) {
            tileMin.X -= 1;
            tileMin.Y -= 1;
            tileMax.X += 1;
            tileMax.Y += 1;
        } else {
            tileMax.X += offsetXCells;
            tileMax.Y += offsetYCells;
        }
    }

    //make sure we only have valid cell numbers in the allowed range
    this->mLevelTerrain->ForceTileGridCoordRange(tileMin);
    this->mLevelTerrain->ForceTileGridCoordRange(tileMax);

    newTriggerRegion->regionId = (irr::u8)(mTriggerRegionVec.size());
    newTriggerRegion->regionType = regionType;
    newTriggerRegion->tileXmin = tileMin.X;
    newTriggerRegion->tileYmin = tileMin.Y;
    newTriggerRegion->tileXmax = tileMax.X;
    newTriggerRegion->tileYmax = tileMax.Y;

    irr::u16 midCoordX;
    irr::u16 midCoordY;

    //calculate region middle cell
    midCoordX = ((tileMax.X - tileMin.X) / 2) + tileMin.X;
    midCoordY = ((tileMax.Y - tileMin.Y) / 2) + tileMin.Y;

    newTriggerRegion->regionCenterTileCoord.set(midCoordX, midCoordY);

    //finally also store trigger target group inside this struct
    //so that we have this information by the hand all the time if we
    //need it
    newTriggerRegion->mTargetGroup = entity->getTargetGroup();

    //add the new region to the region vector
    this->mTriggerRegionVec.push_back(newTriggerRegion);
}

void Race::createEntity(EntityItem *p_entity,
                        LevelFile *levelRes, LevelTerrain *levelTerrain, LevelBlocks* levelBlocks, irr::video::IVideoDriver *driver) {
    //Line line;
    irr::f32 w, h;
    Collectable* collectable;
    //Box box;

    //make local variable which points on pointer
    EntityItem entity = *p_entity;
    EntityItem *next = NULL;

//    if (!GroupedEntities.ContainsKey(entity.Group)) GroupedEntities.Add(entity.Group, new List<EntityItem>());
//    GroupedEntities[entity.Group].Add(entity);

    float boxSize = 0;
    collectable = NULL;

    int next_ID = entity.getNextID();
    bool exists;

    if (next_ID != 0) {
        //see if a entity with this ID exists
        exists = levelRes->ReturnEntityItemWithId(next_ID, &next);
    }

    Entity::EntityType type = entity.getEntityType();

    switch (type) {
        case Entity::EntityType::WaypointAmmo:
        case Entity::EntityType::WaypointFuel:
        case Entity::EntityType::WaypointShield:
        case Entity::EntityType::WaypointShortcut:
        case Entity::EntityType::WaypointSpecial1:
        case Entity::EntityType::WaypointSpecial2:
        case Entity::EntityType::WaypointSpecial3:
        case Entity::EntityType::WaypointFast:
        case Entity::EntityType::WaypointSlow: {
            //add a level waypoint
            AddWayPoint(p_entity, next);
            break;
        }

        case Entity::EntityType::WallSegment: {

            if (next != NULL) {
                LineStruct *line = new LineStruct;
                line->A = entity.getCenter();
                line->B = next->getCenter();
                //line = new Line(entity.Center, next.Center, color);
                //line->name.clear();
                //line->name.append("Wall segment line ");
                //line->name.append(std::to_string(entity.get_ID()));
                //line->name.append(" to ");
                //line->name.append(std::to_string(next->get_ID()));

                line->name = new char[100];
                sprintf(&line->name[0], "Wall segment line %d to %d", entity.get_ID(), next->get_ID());

                //remember a line between both waypoints for debugging purposes
                ENTWallsegmentsLine_List->push_back(line);
            }
           ENTWallsegments_List->push_back(p_entity);
           break;
        }

       case Entity::EntityType::TriggerCraft:
       case Entity::EntityType::TriggerRocket: {
            AddTrigger(p_entity);
            break;
       }

       case Entity::EntityType::TriggerTimed: {
                //Billboard timer = new Billboard("images/stopwatch.png", 0.4f, 0.4f);
                //timer.Position = entity.Center;
                //Entities.AddNode(timer);
                AddTimer(p_entity);
                break;
       }

            case Entity::EntityType::MorphOnce:
            case Entity::EntityType::MorphPermanent: {
                    w = entity.getOffsetX() + 1.0f;
                    h = entity.getOffsetY() + 1.0f;
                    //box = new Box(0, 0, 0, w, 1, h, new Vector4(0.1f, 0.3f, 0.9f, 0.5f));
                    //box.Position = entity.Pos + Vector3.UnitY * 0.01f;
                    //AddNode(box);

                    EntityItem* source;

                    std::vector<Column*> sourceColumns;
                    sourceColumns.clear();

                    //see if a entity with this ID exists
                    bool entFound = levelRes->ReturnEntityItemWithId(entity.getNextID(), &source);

                    if (entFound) {
                        sourceColumns = levelBlocks->ColumnsInRange(source->getCell().X, source->getCell().Y, w, h);
                    }

                    // morph for this entity and its linked source
                    std::vector<Column*> targetColumns = levelBlocks->ColumnsInRange(entity.getCell().X , entity.getCell().Y, w, h);
                    
                    //for morph optimization we want to keep the dynamic changing map parts in their own MeshBuffers and own SceneNodes
                    //for this I decided to mark the dynamic parts of the maps (morph cells) with a bool variable inside the terrain tile data
                    //so that later we can put this cells into their own Meshbuffers/SceneNodes

                    //Additional note 03.01.2025: We need to include a little bit more cells into the dynamic terrain around the
                    //initial morphing area defined in the level (+/- 5 cells as defined below), because otherwise when we run a
                    //morph there will be areas in the terrain at the seems between static & dynamic area that do not behave
                    //correctly (for example hole appear where the player can see through). But including slightly more area into the dynamic
                    //mesh this problem does not arise.
                    irr::u32 baseX = entity.getCell().X - 5;
                    irr::u32 baseY = entity.getCell().Y - 5;

                    irr::core::vector2di cellCoord;
                    for (irr::u32 idxX = 0; idxX < (w + 5); idxX++) {
                        for (irr::u32 idxY = 0; idxY < (h + 5); idxY++) {
                            cellCoord.set(idxX+baseX, idxY+baseY);
                            this->mLevelTerrain->ForceTileGridCoordRange(cellCoord);

                            this->mLevelTerrain->pTerrainTiles[cellCoord.X][cellCoord.Y].dynamicMesh = true;
                        }
                    }

                    if (entFound) {
                        baseX = source->getCell().X - 5;
                        baseY = source->getCell().Y - 5;

                        for (irr::u32 idxX = 0; idxX < (w + 5); idxX++) {
                            for (irr::u32 idxY = 0; idxY < (h + 5); idxY++) {
                                cellCoord.set(idxX + baseX, idxY + baseY);
                                this->mLevelTerrain->ForceTileGridCoordRange(cellCoord);

                                this->mLevelTerrain->pTerrainTiles[cellCoord.X][cellCoord.Y].dynamicMesh = true;
                            }
                        }
                    }

                    // regular morph
                    if (targetColumns.size() == sourceColumns.size())
                    {
                            for (unsigned int i = 0; i < targetColumns.size(); i++)
                            {
                                targetColumns[i]->MorphSource = sourceColumns[i];
                                sourceColumns[i]->MorphSource = targetColumns[i];
                            }
                    }
                    else
                    {
                        // permanent morphs dont destroy buildings, instead they morph the column based on terrain height
                        if (entity.getEntityType() == Entity::EntityType::MorphPermanent)
                        {
                            // we need to update surrounding columns too because they could be affected (one side of them)
                            // (problem comes from not using terrain height for all columns in realtime)
                            targetColumns = levelBlocks->ColumnsInRange(entity.getCell().X - 1, entity.getCell().Y - 1, w + 1, h + 1);

                            // create dummy morph source columns at source position
                            std::vector<Column*>::iterator colIt;

                            for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
                                vector3d<irr::f32> colPos(0.0f, 0.0f, 0.0f);
                                colPos.X = source->getCell().X + ((*colIt)->Position.X - entity.getCell().X);
                                colPos.Y = 0.0f;
                                colPos.Z = source->getCell().Y + ((*colIt)->Position.Z - entity.getCell().Y);

                                (*colIt)->MorphSource = new Column(levelTerrain, levelBlocks, (*colIt)->Definition, colPos, levelRes);
                            }

                            sourceColumns.clear();
                        }
                        else
                        {
                            // in this case (MorphOnce) there are no target columns and
                            // (target and source areas are swapped from game perspective)
                            // and buildings have to be destroyed as soon as the morph starts
                            std::vector<Column*>::iterator colIt;

                            for (colIt = sourceColumns.begin(); colIt != sourceColumns.end(); ++colIt) {
                                (*colIt)->DestroyOnMorph = true;
                            }

                            for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
                                (*colIt)->DestroyOnMorph = true;
                            }
                        }
                    }

                    // create and collect morph instances
                    Morph* morph = new Morph(entity.get_ID(), source, p_entity, (int)w, (int)h,
                                             entity.getEntityType() == Entity::EntityType::MorphPermanent,
                                             this);
                    std::vector<Column*>::iterator colIt;

                    for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
                        morph->Columns.push_back(*colIt);
                    }

                    Morphs.push_back(morph);

                    // source
                    morph = new Morph(entity.get_ID(), p_entity, source, (int)w, (int)h,
                                      entity.getEntityType() == Entity::EntityType::MorphPermanent,
                                      this);
                    for (colIt = sourceColumns.begin(); colIt != sourceColumns.end(); ++colIt) {
                        morph->Columns.push_back(*colIt);
                    }

                    Morphs.push_back(morph);
                    break;
            }

        case Entity::EntityType::MorphSource1:
        case Entity::EntityType::MorphSource2: {
            // no need to display morph sources since they are handled above by their targets

            w = entity.getOffsetX() + 1.0f;
            h = entity.getOffsetY() + 1.0f;

            //for morph optimization we want to keep the dynamic changing map parts in their own MeshBuffers and own SceneNodes
            //for this I decided to mark the dynamic parts of the maps (morph cells) with a bool variable inside the terrain tile data
            //so that later we can put this cells into their own Meshbuffers/SceneNodes
            irr::u32 baseX = entity.getCell().X - 5;
            irr::u32 baseY = entity.getCell().Y - 5;

            irr::core::vector2di cellCoord;
            for (irr::u32 idxX = 0; idxX < (w + 5); idxX++) {
                for (irr::u32 idxY = 0; idxY < (h + 5); idxY++) {
                    cellCoord.set(idxX+baseX, idxY+baseY);
                    this->mLevelTerrain->ForceTileGridCoordRange(cellCoord);

                    this->mLevelTerrain->pTerrainTiles[cellCoord.X][cellCoord.Y].dynamicMesh = true;
                }
            }

            break;
         }

        case Entity::EntityType::RecoveryTruck: {
            Recovery *recov1 =
                    new Recovery(this, entity.getCenter().X, entity.getCenter().Y + 6.0f, entity.getCenter().Z, mInfra->mSmgr);

            //remember all recovery vehicles in a vector for later use
            this->recoveryVec->push_back(recov1);

            break;
        }

        case Entity::EntityType::Cone: {
            irr::core::vector3df center = entity.getCenter();
            Cone *cone = new Cone(this, center.X, center.Y + 0.104f, center.Z, mInfra->mSmgr);

            //remember all cones in a vector for later use
            this->coneVec->push_back(cone);

            break;
        }

        case Entity::EntityType::Checkpoint:     {
            AddCheckPoint(entity);
            break;
        }

        case Entity::EntityType::Camera: {
            AddCamera(p_entity);
            break;
        }

        case Entity::EntityType::Explosion: {
               /* BillboardAnimation explosion = new BillboardAnimation("images/tmaps/explosion.png", 1f, 1f, 88, 74, 10);

                explosion.Position = entity.Center;
                Entities.AddNode(explosion);
                break;
                */
                AddExplosionEntity(p_entity);
                break;
            }

        //this are default collectable items from
        //the map files
        case Entity::EntityType::ExtraFuel:
        case Entity::EntityType::FuelFull:
        case Entity::EntityType::DoubleFuel:
        case Entity::EntityType::ExtraAmmo:
        case Entity::EntityType::AmmoFull:
        case Entity::EntityType::DoubleAmmo:
        case Entity::EntityType::ExtraShield:
        case Entity::EntityType::ShieldFull:
        case Entity::EntityType::DoubleShield:
        case Entity::EntityType::BoosterUpgrade:
        case Entity::EntityType::MissileUpgrade:
        case Entity::EntityType::MinigunUpgrade:  {
                    collectable = new Collectable(this, p_entity, entity.getCenter(), mInfra->mSmgr, driver);
                    ENTCollectablesVec->push_back(collectable);
                    break;
        }

        case Entity::EntityType::UnknownShieldItem:
            {
                   //uncomment the next 2 lines to show this items also to the player
                   // collectable = new Collectable(41, entity.get_Center(), color, driver);
                   // ENTCollectables_List.push_back(collectable);
                    break;
            }

        case Entity::EntityType::UnknownItem:
        case Entity::EntityType::Unknown:
            {
                   //uncomment the next 2 lines to show this items also to the player
                   // collectable = new Collectable(50, entity.get_Center(), color, driver);
                   // ENTCollectables_List.push_back(collectable);
                    break;
            }

        case Entity::EntityType::SteamStrong: {
               irr::core::vector3d<irr::f32> newlocation = entity.getCenter();
               //SteamFountain *sf = new SteamFountain(this->mSmgr, driver, newlocation , 12);
               SteamFountain *sf = new SteamFountain(p_entity, mInfra->mSmgr, driver, newlocation , 48);

               //only for first testing
               //sf->Activate();

               //it seems when SteamFountains are created the are not
               //active yet in the game, the are normally triggered to be
               //active by a craft trigger or similar

               //add new steam fontain to my list of fontains
               steamFountainVec->push_back(sf);
               break;
        }

        case Entity::EntityType::SteamLight: {
               irr::core::vector3d<irr::f32> newlocation = entity.getCenter();
               //SteamFountain *sf = new SteamFountain(this->mSmgr, driver, newlocation , 7);
               SteamFountain *sf = new SteamFountain(p_entity, mInfra->mSmgr, driver, newlocation , 24);

               //only for first testing
               //sf->Activate();

               //it seems when SteamFountains are created the are not
               //active yet in the game, the are normally triggered to be
               //active by a craft trigger or similar

               //add new steam fontain to my list of fontains
               steamFountainVec->push_back(sf);
               break;
        }

        default:
            {
                    boxSize = 0.98f;
                    break;
            }
    }
}

//returns filename of sprite file for collectable
//invalid entity types will revert to sprite number 42
std::string Race::GetCollectableSpriteFileName(Entity::EntityType mEntityType) {
    int nrSprite;
    std::string result;

    result.clear();

    switch (mEntityType) {
        case Entity::EntityType::ExtraFuel:
        {
            nrSprite = 29;
            break;
        }

        case Entity::EntityType::FuelFull:
        {
            nrSprite = 30;
            break;
        }
        case Entity::EntityType::DoubleFuel:
        {
            nrSprite = 31;
            break;
        }

        case Entity::EntityType::ExtraAmmo:
        {
           nrSprite = 32;
           break;
        }
        case Entity::EntityType::AmmoFull:
            {
               nrSprite = 33;
               break;
            }
        case Entity::EntityType::DoubleAmmo:
            {
               nrSprite = 34;
               break;
            }

        case Entity::EntityType::ExtraShield:
            {
               nrSprite = 35;
               break;
            }
        case Entity::EntityType::ShieldFull:
            {
               nrSprite = 36;
               break;
            }
        case Entity::EntityType::DoubleShield:
            {
               nrSprite = 37;
               break;
            }

        case Entity::EntityType::BoosterUpgrade:
            {
               nrSprite = 40;
               break;
            }
        case Entity::EntityType::MissileUpgrade:
            {
              nrSprite = 39;
              break;
            }
        case Entity::EntityType::MinigunUpgrade:
            {
              nrSprite = 38;
              break;
            }

        default: {
            nrSprite = 42;
            break;
        }
    }

    //construct file name
    char fname[20];
    sprintf (fname, "%0*d", 4, nrSprite);

    result.append("extract/sprites/tmaps");
    result.append(fname);
    result.append(".png");

    return result;
}

void Race::SpawnCollectiblesForPlayer(Player* player) {
   if (player == NULL)
       return;

   //spawn collectibles at the current player location
   irr::core::vector3df location = player->phobj->physicState.position;

   CollectableSpawner* newSpawner = new CollectableSpawner(
               this, location, mInfra->mSmgr, mInfra->mDriver);

   newSpawner->AddCollectableToSpawn(Entity::EntityType::ExtraFuel);
   newSpawner->AddCollectableToSpawn(Entity::EntityType::ExtraShield);
   newSpawner->AddCollectableToSpawn(Entity::EntityType::ExtraAmmo);
   newSpawner->AddCollectableToSpawn(Entity::EntityType::ExtraFuel);
   newSpawner->AddCollectableToSpawn(Entity::EntityType::ExtraShield);
   newSpawner->AddCollectableToSpawn(Entity::EntityType::ExtraAmmo);

   newSpawner->Trigger();

   mCollectableSpawnerVec.push_back(newSpawner);
}

void Race::UpdateCollectableSpawners(irr::f32 frameDeltaTime) {
    //are there any collectable spawners right now
    //to update?
    if (mCollectableSpawnerVec.size() > 0) {
        std::vector<CollectableSpawner*>::iterator it;
        CollectableSpawner* pntr;

        //are there any collectable spawners that have finished
        //their job completely, and need to be cleaned up?
        for (it = mCollectableSpawnerVec.begin(); it != mCollectableSpawnerVec.end(); ) {
            if ((*it)->CanBeCleanedUp()) {
                //is finished, needs to be cleaned up
                pntr = (*it);

                it = mCollectableSpawnerVec.erase(it);

                //also delete collectable spawner we do not need
                //anymore
                delete pntr;
            } else {
                //verify next element
                it++;
            }
        }

        //update all remaining active collectible spawners
        for (it = mCollectableSpawnerVec.begin(); it != mCollectableSpawnerVec.end(); ++it) {
            ((*it)->Update(frameDeltaTime));
        }
    }
}

void Race::UpdateType2Collectables(irr::f32 frameDeltaTime) {
    std::vector<Collectable*>::iterator it;

    for (it = ENTCollectablesVec->begin(); it != ENTCollectablesVec->end(); ) {
        //is this a type2 collectable
        if ((*it)->mEntityItem == NULL) {
            //yes, it is, update it
            //this make sure that their lifetime is reduces, and after
            //their lifetime is over, the disappear and are deleted
            (*it)->UpdateType2Collectable(frameDeltaTime);

            //is the lifetime of the item over?
            if ((*it)->GetType2CollectableCleanUpNecessary()) {
                //add item to cleanup list for later
                mType2CollectableForCleanupLater.push_back(*it);

                //erase collectable from update list
                it = ENTCollectablesVec->erase(it);
            } else {
                it++;
            }
        } else {
            it++;
        }
    }
}

void Race::CleanUpCollectableSpawners() {
    //makes sure that we also cleanup any collectable spawners
    //that have not yet finished their job
    //so that we do not have a memory leak
    if (mCollectableSpawnerVec.size() > 0) {
        std::vector<CollectableSpawner*>::iterator it;
        CollectableSpawner* pntr;

        for (it = mCollectableSpawnerVec.begin(); it != mCollectableSpawnerVec.end(); ) {
                pntr = (*it);

                it = mCollectableSpawnerVec.erase(it);

                //delete collectable spawner itself
                delete pntr;
        }
    }
}
