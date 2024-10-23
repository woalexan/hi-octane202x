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

Race::Race(irr::IrrlichtDevice* device, irr::video::IVideoDriver *driver, irr::scene::ISceneManager* smgr, MyEventReceiver* eventReceiver, GameText* gameText,
           MyMusicStream* gameMusicPlayerParam, SoundEngine* soundEngine, TimeProfiler* timeProfiler, dimension2d<u32> gameScreenRes, int loadLevelNr) {
    this->mDriver = driver;
    this->mSmgr = smgr;
    this->mDevice = device;
    this->mEventReceiver = eventReceiver;
    this->mGameText = gameText;
    this->mGameScreenRes = gameScreenRes;
    this->mMusicPlayer = gameMusicPlayerParam;
    this->mSoundEngine = soundEngine;
    this->mTimeProfiler = timeProfiler;
    levelNr = loadLevelNr;
    ready = false;

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

    transitionLinkVec = new std::vector<WayPointLinkInfoStruct*>;
    transitionLinkVec->clear();

    steamFountainVec = new std::vector<SteamFountain*>;
    steamFountainVec->clear();

    //my vector of recovery vehicles
    recoveryVec = new std::vector<Recovery*>;
    recoveryVec->clear();

    //my vector of cones on the race track
    coneVec = new std::vector<Cone*>;
    coneVec->clear();

    mPlayerList.clear();

    //load the correct music file for this level
    char musicFileName[60];

    //make assigement level number to music file
    DeliverMusicFileName(loadLevelNr, musicFileName);

    if (!mMusicPlayer->loadGameMusicFile(musicFileName)) {
         cout << "Music load failed" << endl;
    } else {
        //start music playing
        mMusicPlayer->StartPlay();
      }
}

void Race::IrrlichtStats(char* text) {
      cout << "----- " << std::string(text) << "----- " << std::endl << std::flush;
      cout << "Mesh count loaded: " << this->mSmgr->getMeshCache()->getMeshCount() << std::endl << std::flush;
      cout << "Textures loaded: " << this->mSmgr->getVideoDriver()->getTextureCount() << std::endl << std::flush;

      irr::s32 texCnt;
      texCnt =  this->mSmgr->getVideoDriver()->getTextureCount();

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
         fprintf(oFile, "%s\n", this->mSmgr->getVideoDriver()->getTextureByIndex(i)->getName().getInternalName().c_str());
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
    player->SetMyHUD(NULL);
    player2->SetMyHUD(NULL);

    //now we can free the HUD
    delete Hud1Player;

    //delete physics and all
    //linked physics Objects
    delete mPhysics;

    //free all players
    delete player;
    delete player2;

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
    mSmgr->getMeshCache()->removeMesh(wallCollisionMesh);

    CleanUpMorphs();
    CleanUpSteamFountains();
    CleanUpEntities();

    CleanUpRecoveryVehicles();
    CleanUpCones();

    CleanUpWayPointLinks(*this->wayPointLinkVec);
    CleanUpWayPointLinks(*this->transitionLinkVec);
    CleanUpAllCheckpoints();
    CleanUpSky();
    CleanMiniMap();

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

   if (ENTCollectables_List->size() > 0) {
       std::list<Collectable*>::iterator it;
       Collectable* pntr;
       for (it = ENTCollectables_List->begin(); it != ENTCollectables_List->end(); ) {
           pntr = (Collectable*)(*it);
           it = ENTCollectables_List->erase(it);

           //delete Collectable itself
           //this frees SceneNode and texture inside
           //collectable implementation
           delete pntr;
       }
   }

   delete ENTCollectables_List;
   ENTCollectables_List = NULL;
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

void Race::StopMusic() {
    if ((mMusicPlayer->getStatus() == mMusicPlayer->Playing) ||
       (mMusicPlayer->getStatus() == mMusicPlayer->Paused)) {
            //stop music
            mMusicPlayer->StopPlay();
    }
}

void Race::StopAllSounds() {
    //make sure all warning sounds are off
    player->StopPlayingWarningSound();
    player2->StopPlayingWarningSound();

    //make sure engine sounds are off
    //this function internally also stops
    //engine sound
    mSoundEngine->StopAllSounds();
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
void Race::DamagePlayer(Player* targetToHit, irr::f32 damageVal, Player* attacker) {
    bool targetDied;

    if (targetToHit != NULL) {
        targetDied = targetToHit->Damage(damageVal);

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
                //player was the attacker
                targetToHit->ShowPlayerBigGreenHudText(killMessage, 5.0f);
            }

            //trigger explosion at location of killed player
            this->mExplosionLauncher->Trigger(targetToHit->phobj->physicState.position);
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

void Race::PlayerFindClosestWaypointLink(Player* whichPlayer) {
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

    if (whichPlayer == player) {

    //first reset colors of all waypoint links to white for debugging (white for unselected lines)
    for(WayPointLink_iterator = wayPointLinkVec->begin(); WayPointLink_iterator != wayPointLinkVec->end(); ++WayPointLink_iterator) {
        (*WayPointLink_iterator)->pLineStruct->color = mDrawDebug->white;
       // (*WayPointLink_iterator)->pLineStruct->debugLine = NULL;
    }
    }

    //iterate through all waypoint links
    for(WayPointLink_iterator = wayPointLinkVec->begin(); WayPointLink_iterator != wayPointLinkVec->end(); ++WayPointLink_iterator) {

        //for the workaround later (in case first waypoint link search does not work) also find in parallel the waypoint link that
        //has a start or end-point closest to the current player location
        posH = (*WayPointLink_iterator)->pStartEntity->get_Pos();
        posH.X = -posH.X;

         startPointDistHlper = ((whichPlayer->phobj->physicState.position - posH)).getLengthSQ();

         posH = (*WayPointLink_iterator)->pEndEntity->get_Pos();
         posH.X = -posH.X;

         endPointDistHlper = ((whichPlayer->phobj->physicState.position - posH)).getLengthSQ();

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
        dA = whichPlayer->phobj->physicState.position - (*WayPointLink_iterator)->pLineStruct->A;
        dB = whichPlayer->phobj->physicState.position - (*WayPointLink_iterator)->pLineStruct->B;

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

            distance = (projPlayerPosition - whichPlayer->phobj->physicState.position).getLength();

            //(*WayPointLink_iterator)->pLineStruct->color = mDrawDebug->pink;

            //prevent picking far away waypoint links
            //accidently (this happens especially when we are between
            //the end of the current waypoint link and the start
            //of the next one)
            if (distance < 10.0f) {

            if (firstElement) {
                minDistance = distance;
                closestLink = (*WayPointLink_iterator);
                whichPlayer->projPlayerPositionClosestWayPointLink = projPlayerPosition;
                firstElement = false;
            } else {
                if (distance < minDistance) {
                    minDistance = distance;
                    closestLink = (*WayPointLink_iterator);
                    whichPlayer->projPlayerPositionClosestWayPointLink = projPlayerPosition;
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

    if (closestLink != NULL) {
        whichPlayer->SetCurrClosestWayPointLink(closestLink);
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

void Race::AddPlayer() {

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
     ****************************************************/

    //declaring vector of pairs containing number laps finished
    //and pointer to player
    vector< pair <irr::s32, Player*> > vecLapsFinished;

    for (it = mPlayerList.begin(); it != mPlayerList.end(); ++it) {
          vecLapsFinished.push_back( make_pair((*it)->mPlayerStats->currLapNumber, (*it)));
    }

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

   for (it2 = vecLapsFinished.rbegin(); it2 != vecLapsFinished.rend(); ++it2) {

       playerPntr = (it2)->second;

       //is the next player in the same current lap number?
       if ((*it2).first == currLapNr) {
          //yes, add it to the next sorting list for next expected check point
          vecNextCheckPointExpected.push_back( make_pair(playerPntr->nextCheckPointValue, playerPntr));
       } else {
           //the next player has not so much laps done yet
           //go to the next sorting stage
            UpdatePlayerRacePositionRankingHelper2(vecNextCheckPointExpected);

            vecNextCheckPointExpected.clear();
            vecNextCheckPointExpected.push_back( make_pair(playerPntr->nextCheckPointValue, playerPntr));
       }

       currLapNr = (it2)->first;
   }

   if (vecNextCheckPointExpected.size() > 0) {
       UpdatePlayerRacePositionRankingHelper2(vecNextCheckPointExpected);
   }

   int currPos = 1;
   int numPlayers = mPlayerList.size();

   //all player rankings are done and stored in this->playerRanking
   //Update player objects with this new ranking information
   for (it = this->playerRanking.begin(); it != this->playerRanking.end(); ++it) {
       (*it)->mPlayerStats->currRacePlayerPos = currPos;
       (*it)->mPlayerStats->overallPlayerNumber = numPlayers;
       currPos++;
   }
}

void Race::UpdatePlayerDistanceToNextCheckpoint(Player* whichPlayer) {
    irr::f32 sumDistance = 0.0f;
    WayPointLinkInfoStruct* currLink;
    irr::f32 len;

    //start at current waypoint link closest to current player
    //then follow this link forward until we hit the next checkpoint
    //create sum of all distances
    if (whichPlayer->currClosestWayPointLink != NULL) {
        //we have currently a closest waypoint link for this player
        currLink = whichPlayer->currClosestWayPointLink;

        //The next line is for debugging
        //currLink->pLineStruct->color = mDrawDebug->green;

        //calculate length of vector from current 3D Player projected position on waypoint line to start of line
        len = (whichPlayer->projPlayerPositionClosestWayPointLink - currLink->pLineStruct->A).getLength();

        //first part of the distance is the part from player position
        //on current waypoint link to end of this waypoint link
        sumDistance += (currLink->length3D - len);

        //now follow the waypoint links forward until we hit the next checkpoint
        while (currLink->pntrCheckPoint == NULL) {  //follow one link after another until we hit the next checkpoint
            currLink = currLink->pntrPathNextLink;

            if (currLink != NULL) {
                if (currLink->pntrCheckPoint == NULL) {
                    //The next line is for debugging
                    //currLink->pLineStruct->color = mDrawDebug->blue;

                    //for this links add up the whole length
                    sumDistance += currLink->length3D;
                } else {
                    //there is a checkpoint within this waypoint link
                    //for this one add only the distance from the start point until the checkpoint location
                    sumDistance += currLink->distanceStartLinkToCheckpoint;
                    //currLink->pLineStruct->color = mDrawDebug->red;
                    break;
                }
            }
        }
        //set currently remaining distance from player location to next checkpoint
        //into the player object
        whichPlayer->remainingDistanceToNextCheckPoint = sumDistance;
    }
}

void Race::removePlayerTest() {
    HUD* plHUD;
    //get possible pointer from player than an HUD
    //if no HUD connection is there we will get NULL
    plHUD = player2->GetMyHUD();

    //player has an HUD attached
    if (plHUD != NULL) {
        //tell HUD to stop monitoring player we want to remove
        plHUD->SetMonitorWhichPlayer(NULL);

        //remove HUD pnter also from player object
        //we want to remove
        player2->SetMyHUD(NULL);
    }

    //remove Player2 from physics
    mPhysics->RemoveObject(player2->Player_node);

    //reset pointer in player to physics-object
    player2->SetPlayerObject(NULL);

    //remove Scenenode from Irrlicht SceneManager
    player2->Player_node->remove();

    delete player2;

    player2Removed = true;
}

irr::core::dimension2di Race::CalcPlayerMiniMapPosition(Player* whichPlayer) {
    irr::core::dimension2di miniMapLocation;

    //calculate current cell below player
    int current_cell_calc_x, current_cell_calc_y;

    //calculate player Terrain cell coordinates
    current_cell_calc_y = (whichPlayer->phobj->physicState.position.Z / mLevelTerrain->segmentSize);
    current_cell_calc_x = -(whichPlayer->phobj->physicState.position.X / mLevelTerrain->segmentSize);

    irr::f32 posPlWidth = miniMapPixelPerCellW * (irr::f32)(current_cell_calc_x - this->miniMapStartW);
    irr::f32 posPlHeight = miniMapPixelPerCellH * (irr::f32)(current_cell_calc_y -+ this->miniMapStartH);

    miniMapLocation.Width = miniMapDrawLocation.X + posPlWidth;
    miniMapLocation.Height = miniMapDrawLocation.Y + posPlHeight;

    return miniMapLocation;
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
    irr::video::IImage* baseMiniMapPic = mLevelTerrain->CreateMiniMapInfo(miniMapStartW, miniMapEndW, miniMapStartH, miniMapEndH);

    //file does not exist?
    if (FileExists(outputFilename) != 1) {
       //does not exist yet, create it

        //create new file for writting
        irr::io::IWriteFile* outputPic = mDevice->getFileSystem()->createAndWriteFile(outputFilename, false);

        mDriver->writeImageToFile(baseMiniMapPic, outputPic);

        //close output file
        outputPic->drop();
    }

    //minimap does exist already
    //just load it

    mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    baseMiniMap = mDriver->getTexture(outputFilename);

    //for transparency
    mDriver->makeColorKeyTexture(baseMiniMap, irr::core::position2d<irr::s32>(0,0));

    miniMapSize = baseMiniMap->getSize();
    miniMapDrawLocation.X = this->mGameScreenRes.Width - miniMapSize.Width;
    miniMapDrawLocation.Y = this->mGameScreenRes.Height - miniMapSize.Height;

    mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    //do some precalculations so that we have to repeat them not unnecessary all the time
    //during the game
    miniMapPixelPerCellW = (irr::f32)(miniMapSize.Width) / ((irr::f32)(miniMapEndW) - (irr::f32)(miniMapStartW));
    miniMapPixelPerCellH = (irr::f32)(miniMapSize.Height) / ((irr::f32)(miniMapEndH) - (irr::f32)(miniMapStartH));
}

/*int Race::getNrTrianglesCollected() {
    return mPhysics->mRayTargetTrianglesSize;
}

int Race::getNrHitTrianglesRay() {
    return  TestRayTrianglesSelector.size();
}*/

void Race::HandleCraftHeightMapCollisions() {
 //   player->UpdateHMapCollisionPointData(&player->mHMapCollPntData);

    //Execute the terrain heightmap tile collision detection
    //only if we do this morphing will also have an effect
    //on the craft movements
 //   player->HeightMapCollision(&player->mHMapCollPntData);
}

void Race::Init() {
    //create my main camera
    mCamera = mSmgr->addCameraSceneNodeFPS(0,100.0f,0.05f,-1,
                                                       0,0,false,0.0f);

    //create a DrawDebug object
    this->mDrawDebug = new DrawDebug(this->mDriver);

    if (!LoadLevel(levelNr)) {
        //there was an error loading the level
        ready = false;
    } else {
        //level was loaded ok, we can continue setup

        InitMiniMap(levelNr);

        /***********************************************************/
        /* Init single player HUD                                  */
        /***********************************************************/
        Hud1Player = new HUD(mDevice, mDriver, mGameScreenRes, mGameText);

        //create my overall physics object
        //also handover pointer to my DrawDebug object
        this->mPhysics = new Physics(this, this->mDrawDebug);

        //handover pointer to wall collision line (based on level file entities) data
        this->mPhysics->SetLevelCollisionWallLineData(ENTWallsegmentsLine_List);

        //create my players and setup their physics
        createPlayers(levelNr);

        //HUD should show main player stats
        Hud1Player->SetMonitorWhichPlayer(player);
        Hud1Player->SetHUDState(DEF_HUD_STATE_RACE);

        //the next line is only for computerPlayer movement debugging
        //remove this line later again!
        player2->SetMyHUD(Hud1Player);

        //give physics the triangle selectors for overall collision detection
        this->mPhysics->AddCollisionMesh(triangleSelectorWallCollision);
        this->mPhysics->AddCollisionMesh(triangleSelectorColumnswCollision);

        //give physics the triangle selector for weapon targeting (ray casting at terrain/blocks)
        this->mPhysics->AddRayTargetMesh(triangleSelectorColumnswCollision);
        this->mPhysics->AddRayTargetMesh(triangleSelectorColumnswoCollision);
        this->mPhysics->AddRayTargetMesh(triangleSelectorTerrain);

        //activate collisionResolution in physics
        //can be disabled for debugging purposes
        mPhysics->collisionResolutionActive = true;

        //set initial camera location
        mCamera->setPosition(playerPhysicsObj->physicState.position + irr::core::vector3df(2.0f, 2.0f, 00.0f));
        mCamera->setTarget(playerPhysicsObj->physicState.position);

        this->mSmgr->setActiveCamera(mCamera);

        //create the world awareness class
        mWorldAware = new WorldAwareness(this->mDevice, this->mDriver, this);

        //create my ExplosionLauncher
        mExplosionLauncher = new ExplosionLauncher(this, mSmgr, mDriver);

        ready = true;
    }
}

void Race::TestVoxels() {
    irr::core::vector3df testStart(-20.0f, 16.0f, 60.0f);
    irr::core::vector3df testEnd;
    std::vector<irr::core::vector3di> voxels;

    testEnd = player->phobj->physicState.position;

    voxels = mPhysics->voxel_traversal(testStart, testEnd);

    irr::video::SColor col2(255, 0, 255, 0);

    mDriver->setMaterial(*mDrawDebug->green);
    mDriver->draw3DLine(testStart, testEnd, col2);

    std::vector<irr::core::vector3di>::iterator it;
    irr::core::vector3df boxVertex1;
    irr::core::vector3df boxVertex2;

    irr::video::SColor col(255, 0, 0, 255);

    for (it = voxels.begin(); it != voxels.end(); ++it) {
       boxVertex1.X = (*it).X * DEF_SEGMENTSIZE;
       boxVertex1.Y = (*it).Y * DEF_SEGMENTSIZE;
       boxVertex1.Z = (*it).Z * DEF_SEGMENTSIZE;

       boxVertex2.X = ((*it).X + 1) * DEF_SEGMENTSIZE;
       boxVertex2.Y = ((*it).Y + 1) * DEF_SEGMENTSIZE;
       boxVertex2.Z = ((*it).Z + 1) * DEF_SEGMENTSIZE;

       irr::core::aabbox3df box(boxVertex1.X, boxVertex1.Y, boxVertex1.Z, boxVertex2.X, boxVertex2.Y, boxVertex2.Z);

       mDriver->setMaterial(*mDrawDebug->blue);
       mDriver->draw3DBox(box, col);
    }
}

void Race::AdvanceTime(irr::f32 frameDeltaTime) {
    float progressMorph;

    //if frameDeltaTime is too large we could get
    //weird physics effects, therefore clamp time to max
    //allowed level
    if (frameDeltaTime > 0.1f)
      frameDeltaTime = 0.01f;

    //run morphs
    if (runMorph)
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

    mTimeProfiler->Profile(mTimeProfiler->tIntMorphing);

    //update players
    player->Update(frameDeltaTime);
    if (!player2Removed)
        player2->Update(frameDeltaTime);

    mTimeProfiler->Profile(mTimeProfiler->tIntUpdatePlayers);

    //advance physics time and update sceneNode positions and orientations
    mPhysics->AdvancePhysicsTime(frameDeltaTime);

    mTimeProfiler->Profile(mTimeProfiler->tIntAdvancePhysics);

    irr::core::vector3df worldCoordPlayerCam;
     irr::core::vector3df worldCoordPlayerCamTarget;

    //move camera
    if (this->currPlayerFollow != NULL) {
        if (!this->currPlayerFollow->mFirstPlayerCam) {
            //outside 3rd person camera selected
            worldCoordPlayerCam = currPlayerFollow->phobj->ConvertToWorldCoord(currPlayerFollow->LocalTopLookingCamPosPnt);
            worldCoordPlayerCamTarget = currPlayerFollow->phobj->ConvertToWorldCoord(currPlayerFollow->LocalTopLookingCamTargetPnt);
        } else {
            //1st person camera selected
            worldCoordPlayerCam = currPlayerFollow->phobj->ConvertToWorldCoord(currPlayerFollow->Local1stPersonCamPosPnt);
            worldCoordPlayerCamTarget = currPlayerFollow->phobj->ConvertToWorldCoord(currPlayerFollow->Local1stPersonCamTargetPnt);
        }
    }

    if (playerCamera) {
       mCamera->setPosition(worldCoordPlayerCam);
       mCamera->setTarget(worldCoordPlayerCamTarget);
     }

    player->AfterPhysicsUpdate();

    mTimeProfiler->Profile(mTimeProfiler->tIntAfterPhysicsUpdate);

    //important, empty old triangle hit information first!
    //otherwise we have a memory leak
    //mPhysics->EmptyTriangleHitInfoVector(TestRayTrianglesSelector);

    //mPhysics->FindRayTargetTriangles(*player->phobj, player->craftForwardDirVec);
    /*TestRayTrianglesSelector = mPhysics->ReturnTrianglesHitByRay( mPhysics->mRayTargetSelectors,
                                  player->phobj->physicState.position, player->phobj->physicState.position + player->craftForwardDirVec * irr::core::vector3df(50.0f, 50.0f, 50.0f),
                                                                  true);*/

    PlayerFindClosestWaypointLink(player);
    PlayerFindClosestWaypointLink(player2);
    UpdatePlayerDistanceToNextCheckpoint(player);
    UpdatePlayerDistanceToNextCheckpoint(player2);

    std::vector<CheckPointInfoStruct*>::iterator it;

    irr::core::aabbox3d<f32> playerBox = this->player->Player_node->getTransformedBoundingBox();
    irr::core::aabbox3d<f32> checkPointBox;

    for (it = this->checkPointVec->begin(); it != this->checkPointVec->end(); ++it) {
        checkPointBox = (*it)->SceneNode->getTransformedBoundingBox();
        if (playerBox.intersectsWithBox(checkPointBox)) {

            //player crosses waypoint, figure out if player crosses waypoint in the normal
            //race direction, if not ignore event
            irr::core::vector3df velNormalized = player->phobj->physicState.velocity.normalize();

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
                player->CrossedCheckPoint((*it)->value, checkPointVec->size());
            }
        }
    }

    CheckPlayerCollidedCollectible(player, playerBox);

    //update player race position ranking
    UpdatePlayerRacePositionRanking();

    mTimeProfiler->Profile(mTimeProfiler->tIntPlayerMonitoring);

    //update all particle systems
    UpdateParticleSystems(frameDeltaTime);

    //update all current explosions
    mExplosionLauncher->Update(frameDeltaTime);

    mTimeProfiler->Profile(mTimeProfiler->tIntUpdateParticleSystems);

    mWorldAware->Analyse(player);
    mWorldAware->Analyse(player2);

    mTimeProfiler->Profile(mTimeProfiler->tIntWorldAware);
}

void Race::UpdateParticleSystems(irr::f32 frameDeltaTime) {
    //update all steam fontains
    std::vector <SteamFountain*>::iterator it;

    for (it = steamFountainVec->begin(); it != steamFountainVec->end(); ++it) {
        (*it)->TriggerUpdate(frameDeltaTime);
    }
}

void Race::HandleComputerPlayers() {
   // player2->RunComputerPlayerLogic();
   // player2->CPForceController();
   /* if (player2->currClosestWayPointLink != NULL) {
    player2->FlyTowardsEntityRunComputerPlayerLogic(player2->currClosestWayPointLink->pEndEntity);
    }*/
}

void Race::HandleInput() {
    bool playerNoTurningKeyPressed = true;

    //only for debugging purposes, to trigger
    //a breakpoint via a keyboard press
    DebugHitBreakpoint = false;

    if(this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_3))
    {
        // testPerm();
        player->mHUD->AddGlasBreak();
    }

    if(this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_4))
    {
      //   remoPerm();
       // player->mHUD->RepairGlasBreaks();
      if (player->altCntrlMode == false) {
          player->altCntrlMode = true;
      } else player->altCntrlMode = false;
    }

    if(this->mEventReceiver->IsKeyDown(irr::KEY_KEY_W)) {
         player->Forward();
    }

    if(this->mEventReceiver->IsKeyDown(irr::KEY_KEY_S))
    {
          player->Backward();
    }

    if(this->mEventReceiver->IsKeyDown(irr::KEY_KEY_M))
    {
         runMorph =true;
    }

    if(this->mEventReceiver->IsKeyDown(irr::KEY_SPACE))
    {
         player->IsSpaceDown(true);
    } else {
        player->IsSpaceDown(false);
    }

    if(this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_1))
    {
         this->currPlayerFollow = player;
    }

    if(this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_2))
    {
         this->currPlayerFollow = player2;
    }

    if(this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_Z))
    {
         DebugHitBreakpoint = true;
    }

    if(this->mEventReceiver->IsKeyDown(irr::KEY_KEY_A)) {
         player->Left();
           player->firstNoKeyPressed = true;
         playerNoTurningKeyPressed = false;
    }
    if(this->mEventReceiver->IsKeyDown(irr::KEY_KEY_D)) {
         player->Right();
         player->firstNoKeyPressed = true;
         playerNoTurningKeyPressed = false;
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_L)) {
        player2->CpTriggerTurn(CP_TURN_LEFT, 20.0f);
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_R)) {
        player2->CpTriggerTurn(CP_TURN_RIGHT, 20.0f);
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_P)) {
        playerCamera = !playerCamera;

    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_T)) {
        mLevelTerrain->SwitchViewMode();
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_B)) {
        mLevelBlocks->SwitchViewMode();
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_H)) {
        player->mPlayerStats->shieldVal -= 10.0f;
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_J)) {
        this->mWorldAware->WriteOneDbgPic = true;
    }

   /* if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_R)) {
        this->removePlayerTest();
    }*/

    //if player has not pressed any turning key run this code
    //as well
    if (playerNoTurningKeyPressed) {
        player->NoTurningKeyPressed();
    }

    if(this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_C)) {
        //toggle collision resolution active state
        mPhysics->collisionResolutionActive = !mPhysics->collisionResolutionActive;
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
        this->exitRace = true;
    }

    if (this->mEventReceiver->IsKeyDown(irr::KEY_KEY_Y)) {
        player->mMGun->Trigger();
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_X)) {
        player->mMissileLauncher->Trigger();
    }

    if (this->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_F)) {
        if (this->currPlayerFollow != NULL)
        {
            this->currPlayerFollow->mFirstPlayerCam = !this->currPlayerFollow->mFirstPlayerCam;
        }
    }

}

//the routine below is from:
//https://irrlicht.sourceforge.io/forum/viewtopic.php?t=43565
//https://irrlicht.sourceforge.io/forum//viewtopic.php?p=246138#246138
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

    corner[0] = irr::core::vector2df(position.X,position.Y);
    corner[1] = irr::core::vector2df(position.X+sourceRect.getWidth()*scale.X,position.Y);
    corner[2] = irr::core::vector2df(position.X,position.Y+sourceRect.getHeight()*scale.Y);
    corner[3] = irr::core::vector2df(position.X+sourceRect.getWidth()*scale.X,position.Y+sourceRect.getHeight()*scale.Y);

    // Rotate corners
    if (rotation != 0.0f)
        for (int x = 0; x < 4; x++)
            corner[x].rotateBy(rotation,irr::core::vector2df(rotationPoint.X, rotationPoint.Y));


    // Find the uv coordinates of the sourceRect
    irr::core::vector2df uvCorner[4];
    uvCorner[0] = irr::core::vector2df(sourceRect.UpperLeftCorner.X,sourceRect.UpperLeftCorner.Y);
    uvCorner[1] = irr::core::vector2df(sourceRect.LowerRightCorner.X,sourceRect.UpperLeftCorner.Y);
    uvCorner[2] = irr::core::vector2df(sourceRect.UpperLeftCorner.X,sourceRect.LowerRightCorner.Y);
    uvCorner[3] = irr::core::vector2df(sourceRect.LowerRightCorner.X,sourceRect.LowerRightCorner.Y);
    for (int x = 0; x < 4; x++) {
        float uvX = uvCorner[x].X/(float)texture->getSize().Width;
        float uvY = uvCorner[x].Y/(float)texture->getSize().Height;
        uvCorner[x] = irr::core::vector2df(uvX,uvY);
    }

    // Vertices for the image
    irr::video::S3DVertex vertices[4];
    irr::u16 indices[6] = { 0, 1, 2, 3 ,2 ,1 };

    // Convert pixels to world coordinates
    float screenWidth = driver->getScreenSize().Width;
    float screenHeight = driver->getScreenSize().Height;
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
        /*mDriver->draw2DImage(mSkyImage, irr::core::vector2di(0, 0), irr::core::recti(0, 0, mGameScreenRes.Width, mGameScreenRes.Height)
                        , 0, irr::video::SColor(255,255,255,255), true);*/

        draw2DImage(mDriver, mSkyImage ,irr::core::recti(0, 0, mGameScreenRes.Width, mGameScreenRes.Height),
             irr::core::vector2di(0, 0), irr::core::vector2di( mGameScreenRes.Width / 2.0, mGameScreenRes.Height / 2.0),
             player->currPlayerCraftLeaningAngleDeg, irr::core::vector2df(1.0f, 1.0f), false, irr::video::SColor(255,255,255,255));
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
    //Draw race base of minimap
    mDriver->draw2DImage(baseMiniMap, miniMapDrawLocation,
          irr::core::rect<irr::s32>(0,0, miniMapSize.Width, miniMapSize.Height), 0,
          irr::video::SColor(255,255,255,255), true);

    miniMapAbsTime += frameDeltaTime * 1000.0f;
    if (miniMapAbsTime > 500.0f) {
        miniMapAbsTime = 0.0f;
        miniMapBlinkActive = !miniMapBlinkActive;
    }

    /**********************************
     * MiniMap for Player 1           *
     * ******************************** */

    //draw player position
    irr::video::SColor player1LocationBlock(255, 254, 254, 250);  //the main location block inside color, is always visible
    irr::video::SColor player1LocationFrameColor(255, 194, 189, 206);  //this color is drawn around the main white block to create a blinking effect

    irr::core::dimension2di player1Location = CalcPlayerMiniMapPosition(player);

    //for blinking effect draw bigger frame block for player 1
    //only draw it for blinking effect
    if (miniMapBlinkActive) {
        mDriver->draw2DRectangle(player1LocationFrameColor,
                             core::rect<s32>(player1Location.Width - 5, player1Location.Height -5,
                                             player1Location.Width + 5, player1Location.Height + 5));
    }

    //draw main position marker block for player 1
    mDriver->draw2DRectangle(player1LocationBlock,
                             core::rect<s32>(player1Location.Width - 3, player1Location.Height - 3,
                                             player1Location.Width + 3, player1Location.Height + 3));

    /**********************************
     * MiniMap for Player 2           *
     * ******************************** */

    //draw player position
    irr::video::SColor player2LocationBlock(255, 254, 254, 250);  //the main location block inside color, is always visible
    irr::video::SColor player2LocationFrameColor(255, 194, 189, 206);  //this color is drawn around the main white block to create a blinking effect

    irr::core::dimension2di player2Location = CalcPlayerMiniMapPosition(player2);

    //for blinking effect draw bigger frame block for player 2
    //only draw it for blinking effect
    if (miniMapBlinkActive) {
        mDriver->draw2DRectangle(player2LocationFrameColor,
                             core::rect<s32>(player2Location.Width - 5, player2Location.Height -5,
                                             player2Location.Width + 5, player2Location.Height + 5));
    }

    //draw main position marker block for player 2
    mDriver->draw2DRectangle(player2LocationBlock,
                             core::rect<s32>(player2Location.Width - 3, player2Location.Height - 3,
                                             player2Location.Width + 3, player2Location.Height + 3));
}

void Race::Render() {
    //we need to draw sky image first, the remaining scene will be drawn on top of it
    //DrawSky();

    //draw 3D world coordinate axis with arrows
    mDrawDebug->DrawWorldCoordinateSystemArrows();

    //draw currently active world coordinate forces on player ship
    //playerPhysicsObj->DebugDrawCurrentWorldCoordForces(mDrawDebug, mDrawDebug->green, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);
    //player2PhysicsObj->DebugDrawCurrentWorldCoordForces(mDrawDebug, mDrawDebug->green, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);

    //DrawTestShape();

    //player->DebugCraftLocalFeatureCoordinates();

    //player->Player_node->updateAbsolutePosition();
    //drawDebug.Draw3DArrow(player->Player_node->getAbsolutePosition(), player->Player_node->getAbsolutePosition() + player->targetSteerDir, drawDebug.white);
   // irr::core::vector3df ShipFrontWorldCoor = player->phobj->ConvertToWorldCoord(player->LocalCraftFrontPnt);
   // drawDebug.Draw3DArrow(ShipFrontWorldCoor, ShipFrontWorldCoor + player->WorldCoordWheelDir * 2.0f, drawDebug.blue);

    //drawDebug.Draw3DArrow(physics.DbgCollStartVec, physics.DbgCollEndVec, drawDebug.blue);

    //drawDebug.Draw3DArrow(player->pos_in_worldspace_backPos, player->pos_in_worldspace_frontPos, drawDebug.white);
    //drawDebug.Draw3DArrow(player->pos_in_worldspace_rightPos, player->pos_in_worldspace_leftPos, drawDebug.blue);

    //player->Position = playerShipState.position;
    //player->Player_node->setPosition(interpolatedRenderState.position);

    //core::vector3df rot;
    //interpolatedRenderState.orientation.toEuler(rot);
    //player->Player_node->setRotation(rot * core::RADTODEG);
    //  driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);

    //draw coordinate system at 3d origin
   // driver->setMaterial(*red);
    //driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
   // driver->draw3DLine(vector3df(0.0f, 0.0f, 0.0f), vector3df(10.0f, 0.0f, 0.0f));

    //driver->setMaterial(*green);
    //driver->draw3DLine(vector3df(0.0f, 0.0f, 0.0f), vector3df(0.0f, 10.0f, 0.0f));

    //driver->setMaterial(*blue);
    //driver->draw3DLine(vector3df(0.0f, 0.0f, 0.0f), vector3df(0.0f, 0.0f, 10.0f));

    //driver->setMaterial(*white);
    //driver->draw3DLine(   debugvec1,    debugvec2);


    //wchar_t* text2 = new wchar_t[50];
    /*swprintf(text2, 50, L"%lf %lf %lf %lf", player->phobj->physicState.angularVelocity.X,
             player->phobj->physicState.angularVelocity.Y, player->phobj->physicState.angularVelocity.Z,
             player->phobj->physicState.angularVelocity.getLength());
    swprintf(text2, 50, L"%lf %lf %lf %lf", player->phobj->physicState.position.Y-player->currHeightFront,
             player->phobj->physicState.position.Y-player->currHeightBack,
             player->phobj->physicState.position.Y-player->currHeightLeft,
             player->phobj->physicState.position.Y-player->currHeightRight);*/
    /*dbgText2->setText(text2);
    free(text2);*/

    //swprintf(text, 50, L"%lf %lf %lf", playerShipState.position.X, playerShipState.position.Y, playerShipState.position.Z);

     //swprintf(text, 50, L"%lf %lf %lf", playerShipState.orientation.X, playerShipState.orientation.Y, playerShipState.orientation.Z);

  /*     swprintf(text, 50, L"%lf %lf %lf", playerShipState.spin.X, playerShipState.spin.Y, playerShipState.spin.Z);
    dbgText->setText(text);
    free(text);*/

  /*  if (player2->currClosestWayPointLink != NULL) {
        mDriver->setMaterial(*mDrawDebug->white);
        mDriver->draw3DLine(player2->phobj->physicState.position, player2->projPlayerPositionClosestWayPointLink);
    }

    if (player2->computerCurrFollowWayPointLink != NULL) {
      player2->computerCurrFollowWayPointLink->pLineStruct->color = mDrawDebug->blue;
    }*/

  /*  if (player2->cPTargetEntity != NULL) {
        mDriver->setMaterial(*mDrawDebug->green);
        irr::core::vector3df vecHlp = player2->cPTargetEntity->get_Center();
        vecHlp.X = -vecHlp.X;
        mDriver->draw3DLine(player2->phobj->physicState.position, vecHlp);
    }*/


/*
    if (player2->computerCurrFollowWayPointLink != NULL) {
        mDriver->setMaterial(*mDrawDebug->blue);
        mDriver->draw3DLine(player2->phobj->physicState.position,
                            (player2->computerCurrFollowWayPointLink->pLineStruct->A + player2->computerCurrFollowWayPointLink->pLineStruct->B) * irr::core::vector3d<irr::f32>(0.5f, 0.5f, 0.5f));
    }*/

    std::list<LineStruct*>::iterator Linedraw_iterator;
    std::vector<CheckPointInfoStruct*>::iterator CheckPoint_iterator;

    std::vector<WayPointLinkInfoStruct*>::iterator WayPointLink_iterator;

    if (DebugShowWaypoints) {
      //draw all connections between map waypoints for debugging purposes;
      //mDriver->setMaterial(*mDrawDebug->white);
      for(WayPointLink_iterator = wayPointLinkVec->begin(); WayPointLink_iterator != wayPointLinkVec->end(); ++WayPointLink_iterator) {
          mDriver->setMaterial(*(*WayPointLink_iterator)->pLineStruct->color);
          /*
          if ((*WayPointLink_iterator)->pntrCheckPoint != NULL) {
                mDriver->setMaterial(*mDrawDebug->red);
          } else mDriver->setMaterial(*mDrawDebug->blue);*/
          mDriver->draw3DLine((*WayPointLink_iterator)->pLineStruct->A, (*WayPointLink_iterator)->pLineStruct->B);

        /*  if ((*WayPointLink_iterator)->pLineStruct->debugLine != NULL) {
            mDriver->setMaterial(*mDrawDebug->brown);
             mDriver->draw3DLine((*WayPointLink_iterator)->pLineStruct->debugLine->start, (*WayPointLink_iterator)->pLineStruct->debugLine->end);
          }*/
      }
     }

    if (DebugShowTransitionLinks) {
      //draw all automatic generated transition links
      mDriver->setMaterial(*mDrawDebug->green);
      for(WayPointLink_iterator = transitionLinkVec->begin(); WayPointLink_iterator != transitionLinkVec->end(); ++WayPointLink_iterator) {
          /*
          if ((*WayPointLink_iterator)->pntrCheckPoint != NULL) {
                mDriver->setMaterial(*mDrawDebug->red);
          } else mDriver->setMaterial(*mDrawDebug->blue);*/
          mDriver->draw3DLine((*WayPointLink_iterator)->pLineStruct->A, (*WayPointLink_iterator)->pLineStruct->B);

        /*  if ((*WayPointLink_iterator)->pLineStruct->debugLine != NULL) {
            mDriver->setMaterial(*mDrawDebug->brown);
             mDriver->draw3DLine((*WayPointLink_iterator)->pLineStruct->debugLine->start, (*WayPointLink_iterator)->pLineStruct->debugLine->end);
          }*/
      }
     }

      std::vector<LineStruct*>::iterator Linedraw_iterator2;

    if (DebugShowWallSegments) {
      //draw all wallsegments for debugging purposes
     mDriver->setMaterial(*mDrawDebug->red);
     for(Linedraw_iterator2 = ENTWallsegmentsLine_List->begin(); Linedraw_iterator2 != ENTWallsegmentsLine_List->end(); ++Linedraw_iterator2) {
          mDriver->setMaterial(*mDrawDebug->red);
          mDriver->draw3DLine((*Linedraw_iterator2)->A, (*Linedraw_iterator2)->B);
       }
     }

    if (DebugShowCheckpoints) {
      //draw all checkpoint lines for debugging purposes
      mDriver->setMaterial(*mDrawDebug->blue);
      for(CheckPoint_iterator = checkPointVec->begin(); CheckPoint_iterator != checkPointVec->end(); ++CheckPoint_iterator) {
          mDriver->draw3DLine((*CheckPoint_iterator)->pLineStruct->A, (*CheckPoint_iterator)->pLineStruct->B);
      }
    }

      if (DEBUG_PLAYER_HEIGHT_CONTROL == true) {
        //for debugging of player height calculation
        mDriver->setMaterial(*mDrawDebug->blue);
        for(Linedraw_iterator = player->debug_player_heightcalc_lines.begin(); Linedraw_iterator !=  player->debug_player_heightcalc_lines.end(); ++Linedraw_iterator) {
            mDriver->draw3DLine((*Linedraw_iterator)->A, (*Linedraw_iterator)->B);
        }
      }

      //mPhysics->DrawSelectedCollisionMeshTriangles(player->phobj->GetCollisionArea());
      //mPhysics->DrawSelectedRayTargetMeshTriangles(TestRayTrianglesSelector);

      /*mDriver->setMaterial(*mDrawDebug->green);
      mDriver->draw3DLine(mPhysics->DbgRayTargetLine.start, mPhysics->DbgRayTargetLine.end);*/

      if (DEF_DBG_WALLCOLLISIONS) {
              mDrawDebug->Draw3DTriangle(&playerPhysicsObj->mNearestTriangle,  irr::video::SColor(0, 255, 0,127));
      }

      //Draw debug stuff for player Terrain Collision
    //  if (this->player != NULL) {
      if (false) {
         /* if (this->player->currTileBelowPlayer != NULL) {
            DebugDrawHeightMapTileOutline(this->player->currTileBelowPlayer->get_X(),
                                          this->player->currTileBelowPlayer->get_Z(),
                                          this->mDrawDebug->blue);
          }*/

          if (this->player->mNextNeighboringTileInFrontOfMe != NULL) {
            DebugDrawHeightMapTileOutline(this->player->mNextNeighboringTileInFrontOfMe->get_X(),
                                          this->player->mNextNeighboringTileInFrontOfMe->get_Z(),
                                          this->mDrawDebug->green);

//            mDrawDebug->Draw3DLine(this->player->dbgcollPlanePos1, this->player->dbgcollPlanePos1 + irr::core::vector3df(0.0f, 1.0f, 0.0f)
//                                   , this->mDrawDebug->pink);

//            mDrawDebug->Draw3DLine(this->player->dbgcollPlanePos2, this->player->dbgcollPlanePos2 + irr::core::vector3df(0.0f, 1.0f, 0.0f)
//                                   , this->mDrawDebug->pink);

//            mDrawDebug->Draw3DLine(this->player->phobj->physicState.position, this->player->phobj->physicState.position + this->player->dbgcollResolutionDirVec
//                                   , this->mDrawDebug->blue);

            mDrawDebug->Draw3DLine(this->player->WorldCoordCraftFrontPnt, this->player->WorldCoordCraftFrontPnt + irr::core::vector3df(0.0f, 1.0f, 0.0f)
                                   , this->mDrawDebug->green);

            mDrawDebug->Draw3DLine(this->player->WorldCoordCraftBackPnt, this->player->WorldCoordCraftBackPnt + irr::core::vector3df(0.0f, 1.0f, 0.0f)
                                   , this->mDrawDebug->blue);

            mDrawDebug->Draw3DLine(this->player->WorldCoordCraftRightPnt, this->player->WorldCoordCraftRightPnt + irr::core::vector3df(0.0f, 1.0f, 0.0f)
                                   , this->mDrawDebug->red);

            mDrawDebug->Draw3DLine(this->player->WorldCoordCraftLeftPnt, this->player->WorldCoordCraftLeftPnt + irr::core::vector3df(0.0f, 1.0f, 0.0f)
                                   , this->mDrawDebug->pink);


            //lets also draw surface normal of tile
            /*irr::core::vector3df surfNormal = -mLevelTerrain->computeTileSurfaceNormalFromPositionsBuffer(this->player->mNextNeighboringTileInFrontOfMe->get_X(),
                                                                       this->player->mNextNeighboringTileInFrontOfMe->get_Z());
            surfNormal.Y = 0.0f;*/
            /*if (surfNormal.dotProduct(this->player->craftForwardDirVec) > 0.0f) {
                surfNormal = -surfNormal;
            }*/


           /* surfNormal.normalize();
            irr::core::vector3df middleTile;
            irr::f32 segSize = this->mLevelTerrain->segmentSize;
            middleTile.X = -this->player->mNextNeighboringTileInFrontOfMe->get_X() * segSize + 0.5f * segSize;
            middleTile.Z = this->player->mNextNeighboringTileInFrontOfMe->get_Z() * segSize + 0.5f * segSize;
            middleTile.Y = this->player->mNextNeighboringTileInFrontOfMe->m_Height;

            mDrawDebug->Draw3DLine(middleTile, middleTile + surfNormal * 2.0f, this->mDrawDebug->pink);*/

          }

          if (this->player->mNextNeighboringTileBehindOfMe != NULL) {
            DebugDrawHeightMapTileOutline(this->player->mNextNeighboringTileBehindOfMe->get_X(),
                                          this->player->mNextNeighboringTileBehindOfMe->get_Z(),
                                          this->mDrawDebug->blue);
          }

          if (this->player->mNextNeighboringTileLeftOfMe != NULL) {
            DebugDrawHeightMapTileOutline(this->player->mNextNeighboringTileLeftOfMe->get_X(),
                                          this->player->mNextNeighboringTileLeftOfMe->get_Z(),
                                          this->mDrawDebug->pink);
          }

          if (this->player->mNextNeighboringTileRightOfMe != NULL) {
            DebugDrawHeightMapTileOutline(this->player->mNextNeighboringTileRightOfMe->get_X(),
                                          this->player->mNextNeighboringTileRightOfMe->get_Z(),
                                          this->mDrawDebug->red);
          }




          /*
          if (this->player->m2ndNextNeigboringTileInFrontOfMe != NULL) {
            DebugDrawHeightMapTileOutline(this->player->m2ndNextNeigboringTileInFrontOfMe->get_X(),
                                          this->player->m2ndNextNeigboringTileInFrontOfMe->get_Z(),
                                          this->mDrawDebug->red);


          }*/

          //this->player->phobj->DebugDrawCurrentWorldCoordForces(this->mDrawDebug, this->mDrawDebug->red, PHYSIC_DBG_FORCETYPE_GENERICALL);

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


      /*mDrawDebug->Draw3DLine(irr::core::vector3df(0.0f, 0.0f, 0.0f), this->player->dbgInterset,
                             this->mDrawDebug->green);*/

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

EntityItem* Race::FindNearestWayPointToPlayer(Player* whichPlayer) {
   std::vector<EntityItem*>::iterator it;
   irr::f32 minDistance;
   bool firstElement = true;
   irr::f32 currDist;
   EntityItem* nearestWayPoint;

   if (whichPlayer == NULL)
       return NULL;

   irr::core::vector3df playerPos = whichPlayer->phobj->physicState.position;
   irr::core::vector3df wPos;

   if (ENTWaypoints_List->size() <= 0)
       return NULL;

   for (it = ENTWaypoints_List->begin(); it != ENTWaypoints_List->end(); ++it) {
       wPos = (*it)->get_Center();
       //my X-axis is flipped
       wPos.X = -wPos.X;
       currDist = (wPos - playerPos).getLengthSQ();
       if (firstElement) {
           firstElement = false;
           minDistance = currDist;
           nearestWayPoint = (*it);
       } else if (currDist < minDistance) {
           minDistance = currDist;
           nearestWayPoint = (*it);
       }
   }

   return nearestWayPoint;
}

std::vector<WayPointLinkInfoStruct*> Race::FindWaypointLinksForWayPoint(EntityItem* wayPoint) {
   std::vector<WayPointLinkInfoStruct*>::iterator it;

   std::vector<WayPointLinkInfoStruct*> res;

   if (wayPointLinkVec->size() <= 0)
       return res;

   for (it = wayPointLinkVec->begin(); it != wayPointLinkVec->end(); ++it) {
       if (((*it)->pStartEntity == wayPoint) || ((*it)->pEndEntity == wayPoint)) {
           res.push_back(*it);
       }
   }

   return res;
}

//returns true if succesfull, false otherwise
bool Race::LoadSkyImage(int levelNr, irr::video::IVideoDriver* driver, irr::core::dimension2d<irr::u32> screenResolution) {
    char filename[50];

    strcpy(filename, (char*)"extract/sky/");

    //which sky to load for which level?
    switch (levelNr) {
        case 1: {strcat(filename, (char*)"sky0-0.png"); break;}
        case 2: {strcat(filename, (char*)"sky0-1.png"); break;}
        case 3: {strcat(filename, (char*)"sky0-2.png"); break;}
        case 4: {strcat(filename, (char*)"sky0-3.png"); break;}
        case 5: {strcat(filename, (char*)"sky0-4.png"); break;}
        case 6: {strcat(filename, (char*)"sky0-5.png"); break;}
        case 7: {strcat(filename, (char*)"sky0-0.png"); break;} //I do not know which sky this map uses right now TODO
        case 8: {strcat(filename, (char*)"sky0-0.png"); break;} //I do not know which sky this map uses right now TODO
        case 9: {strcat(filename, (char*)"sky0-0.png"); break;} //I do not know which sky this map uses right now TODO
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
        mDriver->removeTexture(mSkyImage);
        mSkyImage = NULL;
    }
}

void Race::CleanMiniMap() {
    if (baseMiniMap != NULL) {
        //free this texture
        mDriver->removeTexture(baseMiniMap);
        baseMiniMap = NULL;
    }
}

//according to HioctaneTools the level start positions are most
//likely not contained within a map file, but could be instead
//hardcoded in the game code itself
void Race::getPlayerStartPosition(int levelNr, irr::core::vector3d<irr::f32> &startPos, irr::core::vector3d<irr::f32> &startDirection) {
    switch (levelNr) {
        case 1: {startPos.set(-12.0f, 16.5f, 118.0f);  startDirection.set(-12.0f, 16.5f, 116.0f);break;}
        case 2: {startPos.set(-79.0f, 7.0f, 114.0f);  startDirection.set(-79.0f, 7.0f, 111.0f);break;}
        case 3: {startPos.set(-69.0f, 12.0f, 114.0f);  startDirection.set(-69.0f, 12.0f, 112.0f);break;}
        case 4: {startPos.set(-14.0f, 8.5f, 96.0f);  startDirection.set(-14.0f, 8.5f, 94.0f);break;}
        case 5: {startPos.set(-22.0f, 6.0f, 94.0f);  startDirection.set(-22.0f, 6.0f, 92.0f);break;}
        case 6: {startPos.set(-56.0f, 9.0f, 20.0f);  startDirection.set(-56.0f, 9.0f, 17.0f);break;}
        case 7: {startPos.set(-68.5f, 9.0f, 152.5f);  startDirection.set(-68.5f, 9.0f, 150.5f);break;}
        case 8: {startPos.set(-74.5f, 8.5f, 100.5f);  startDirection.set(-74.5f, 8.5f, 98.0f);break;}
        case 9: {startPos.set(-19.5f, 7.0f, 41.5f);  startDirection.set(-19.5f, 7.0f, 39.5f);break;}
    }
}

void Race::createPlayers(int levelNr) {

    //***************************************************
    // Player 1 (main human player)                     *
    //***************************************************

    //std::string player_model("/TANK0-0.obj");
    std::string player_model("extract/models/car0-0.obj");

    //for the start just get hardcoded starting positions for the player
    irr::core::vector3d<irr::f32> Startpos;
    irr::core::vector3d<irr::f32> Startdirection;

    //get player start locations from the level file
    std::vector<irr::core::vector3df> playerStartLocations = this->mLevelTerrain->GetPlayerRaceTrackStartLocations();

    //getPlayerStartPosition(levelNr, Startpos, Startdirection);
    Startpos = playerStartLocations.at(0);
    Startdirection.X = Startpos.X;
    Startdirection.Y = Startpos.Y;
  //Startdirection.Z = Startpos.Z - 2.0f;  original line 04.09.2024, worked best until now

    Startdirection.Z = Startpos.Z - 1.0f; //attempt beginning from 04.09.2024

    //create the main player (controlled by human)
    player = new Player(this, player_model, Startpos, Startdirection, this->mSmgr, true);

    //Setup physics for player1, we handover pointer to Irrlicht
    //player node, as the node (3D model) is now fully controlled
    //by physics
    this->mPhysics->AddObject(player->Player_node);

    //retrieve a pointer to the player physics object that the physics code has
    //created for me, we need this pointer to get access to get player info/control cameras, etc...
    playerPhysicsObj = mPhysics->GetObjectPntr(player->Player_node);

    //setup player physic properties
    if (playerPhysicsObj != NULL) {
        playerPhysicsObj->physicState.SetMass(3.0f);   //3.0f
        playerPhysicsObj->physicState.SetInertia(30.0f);  //30.0f
        playerPhysicsObj->physicState.position = Startpos;
        playerPhysicsObj->physicState.momentum = {0.0f, 0.0f, 0.0f};

        playerPhysicsObj->physicState.orientation.set(irr::core::vector3df(0.0f, 0.0f, 0.0f));

        playerPhysicsObj->physicState.recalculate();

        playerPhysicsObj->SetAirFriction(CRAFT_AIRFRICTION_NOTURBO);

        //playerPhysicsObj->AddWorldCoordForce(obj->physicState.position + irr::core::vector3df(1.0f, 00.0f, 0.0f), obj->physicState.position + irr::core::vector3df(1.0f, 2.0f, 0.0f));
        //playerPhysicsObj->AddLocalCoordForce(irr::core::vector3df(2.0f, 00.0f, 0.0f), irr::core::vector3df(1.0f, 2.0f, 0.0f));
    }

    //give the player a pointer to its physics object
    player->SetPlayerObject(playerPhysicsObj);
    player->SetName((char*)"PLAYER");

    mPlayerList.push_back(player);

    //***************************************************
    // Player 2 (for debugging right now)               *
    //***************************************************

    //create a second player as well
    //std::string player_model("extract/models/jet0-0.obj");
    std::string player_model2("extract/models/bike0-0.obj");
    //std::string player_model("extract/models/car0-0.obj");
    //std::string player_model("extract/models/jugga0-0.obj");
    //std::string player_model("extract/models/marsh0-0.obj");
    //std::string player_model("extract/models/skim0-0.obj");

    irr::core::vector3d<irr::f32> Startpos2;
    irr::core::vector3d<irr::f32> Startdirection2;

    //getPlayerStartPosition(levelNr, Startpos2, Startdirection2);
    Startpos2 = playerStartLocations.at(1);
    Startdirection2.X = Startpos2.X;
    Startdirection2.Y = Startpos2.Y;
    Startdirection2.Z = Startpos2.Z - 2.0f;

    //Startpos2.Z -= 5.0f;

    player2 = new Player(this, player_model2, Startpos2, Startdirection2, this->mSmgr, false);

    this->mPhysics->AddObject(player2->Player_node);

    //setup player 2 physics properties
    player2PhysicsObj = this->mPhysics->GetObjectPntr(player2->Player_node);
    if (player2PhysicsObj != NULL) {
        player2PhysicsObj->physicState.SetMass(3.0f);   //3.0f
        player2PhysicsObj->physicState.SetInertia(30.0f);  //30.0f
        player2PhysicsObj->physicState.position = Startpos2;
        player2PhysicsObj->physicState.momentum = {0.0f, 0.0f, 0.0f};

        player2PhysicsObj->physicState.orientation.set(irr::core::vector3df(0.0f, 0.0f, 0.0f));
        player2PhysicsObj->physicState.recalculate();

       playerPhysicsObj->SetAirFriction(CRAFT_AIRFRICTION_NOTURBO);
    }

    //inform player control object about its physics object pointer
    player2->SetPlayerObject(player2PhysicsObj);
    player2->SetName((char*)"KI");

    mPlayerList.push_back(player2);

    currPlayerFollow = player;

    //add first command for computer player player2
    EntityItem* nearestWayPoint = FindNearestWayPointToPlayer(player2);
    if (nearestWayPoint != NULL) {
        std::vector<WayPointLinkInfoStruct*> foundLinks;
        foundLinks = FindWaypointLinksForWayPoint(nearestWayPoint);
        if (foundLinks.size() > 0) {
            player2->AddCommand(CMD_FLYTO_TARGETENTITY, foundLinks[0]->pEndEntity);
            //player2->AddCommand(CMD_FOLLOW_TARGETWAYPOINTLINK, wl);
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

   if (loadLevelNr < 7) {
       strcpy(levelfilename, "extract/level0-");
       sprintf(str, "%d", loadLevelNr);
       strcat(levelfilename, str);
       strcat(levelfilename, "/level0-");
       strcat(levelfilename, str);
       strcat(levelfilename, "-unpacked.dat");
   } else {
       strcpy(levelfilename, "addedData/level0-");
       sprintf(str, "%d", loadLevelNr);
       strcat(levelfilename, str);
       strcat(levelfilename, ".dat");
   }

   char texfilename[50];
   strcpy(texfilename, "extract/level0-");
   sprintf(str, "%d", load_texnr);
   strcat(texfilename, str);
   strcat(texfilename, "/tex");

   /***********************************************************/
   /* Load level textures                                     */
   /***********************************************************/
   mTexLoader = new TextureLoader(this->mDriver, texfilename);

   this->mLevelRes = new LevelFile(levelfilename);

   char terrainname[50];
   strcpy(terrainname, "Terrain1");

   //Test map save
   //this->mLevelRes->Save(std::string("mapsave.dat"));

   /***********************************************************/
   /* Prepare level terrain                                   */
   /***********************************************************/
   this->mLevelTerrain = new LevelTerrain(terrainname, this->mLevelRes, this->mSmgr, this->mDriver, mTexLoader,
                                          this);

   if (this->mLevelTerrain->Terrain_ready == false) {
       //something went wrong with the terrain loading, exit application
       return false;
   }

   //load sky image for selected level
   if (!LoadSkyImage(loadLevelNr, this->mDriver, this->mGameScreenRes)) {
       //error loading sky image, do something about it!
       return false;
   }

  // driver->setFog(video::SColor(0,138,125,81), video::EFT_FOG_LINEAR, 100, 250, .03f, false, true);

  /***********************************************************/
  /* Create building (cube) Mesh                             */
  /***********************************************************/
  //this routine also generates the column/block collision information inside that
  //we need for collision detection later
  this->mLevelBlocks = new LevelBlocks(this->mLevelTerrain, this->mLevelRes, this->mSmgr, this->mDriver, mTexLoader);

  //create all level entities
  //this are not only items to pickup by the player
  //but also waypoints, collision information, checkpoints
  //level morph information and so on...
  createLevelEntities();

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
    newStruct.SceneNode = this->mSmgr->addMeshSceneNode(checkPointMesh, 0);

    //hide the collision mesh that the player does not see it
    newStruct.SceneNode->setVisible(false);
    newStruct.SceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, true);
    //newStruct.SceneNode->setDebugDataVisible(EDS_BBOX);
}

void Race::AddCheckPoint(EntityItem entity) {
    LineStruct *line = new LineStruct;
    line->A = entity.get_Center();
    line->A.X = -line->A.X;    //my Irrlicht coordinate system is swapped at the x axis; correct this issue

    line->B = entity.get_Center() + irr::core::vector3d<irr::f32>(entity.get_OffsetX(), 0, entity.get_OffsetY());
     //my Irrlicht coordinate system is swapped at the x axis; correct this issue
    line->B.X = -line->B.X;    //my Irrlicht coordinate system is swapped at the x axis; correct this issue

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
    newStruct->value = entity.get_Value();

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
           mSmgr->getMeshCache()->removeMesh(pntr->Mesh);

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
        }
    }

    //free also the vector itself
    delete &vec;
}

void Race::CreateTransitionLink(WayPointLinkInfoStruct* startLink, WayPointLinkInfoStruct* endLink) {
    WayPointLinkInfoStruct* newTransLink = new WayPointLinkInfoStruct();

    newTransLink->pStartEntity = startLink->pStartEntity;
    newTransLink->pEndEntity = endLink->pEndEntity;

    //create and store a 3D line for later drawing and
    //debugging, also calculations will be done with this struct
    LineStruct *line = new LineStruct;

    line->A = newTransLink->pStartEntity->get_Center();
    line->A.X = -line->A.X;    //my Irrlicht coordinate system is swapped at the x axis; correct this issue
    line->B = newTransLink->pEndEntity->get_Center();
    //my Irrlicht coordinate system is swapped at the x axis; correct this issue
   line->B.X = -line->B.X;    //my Irrlicht coordinate system is swapped at the x axis; correct this issue

    //line->name.clear();
    //line->name.append("Transition Link Waypoint line ");
    //line->name.append(std::to_string(newTransLink->pStartEntity->get_ID()));
    //line->name.append(" to ");
    //line->name.append(std::to_string(newTransLink->pEndEntity->get_ID()));
    line->name = new char[100];
    sprintf(&line->name[0], "Transition Link Waypoint line %d to %d",
            newTransLink->pStartEntity->get_ID(), newTransLink->pEndEntity->get_ID());

    //set white as default color
    line->color = mDrawDebug->white;

    irr::core::vector3df vec3D = (line->B - line->A);

    //precalculate and store length
    newTransLink->length3D = vec3D.getLength();
    vec3D.normalize();

    newTransLink->pLineStruct = line;
    //store precalculated direction vector
    newTransLink->LinkDirectionVec = vec3D;

    //add new waypoint link info struct to vector of all
    //interpolated transition waypoint links
    this->transitionLinkVec->push_back(newTransLink);

    //also keep a pointer in the start segment to the new transition
    //element to find it faster during the race later
    startLink->pntrTransitionLink = newTransLink;
}

/*
WayPointLinkInfoStruct* Race::SearchTransitionLink(WayPointLinkInfoStruct* startLink, WayPointLinkInfoStruct* endLink) {

    int16_t startId = startLink->pStartEntity->get_ID();
    int16_t endId = endLink->pEndEntity->get_ID();

    std::vector<WayPointLinkInfoStruct*>::iterator it;
    for (it = transitionLinkVec->begin(); it != transitionLinkVec->end(); ++it) {
        if (((*it)->pStartEntity->get_ID() == startId) && (((*it)->pEndEntity->get_ID() == endId)))
            return (*it);
    }

    return NULL;
}*/

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
               (*it1)->PathNextLinkDirectionVec = (*it2)->LinkDirectionVec;

               //also create a transition link
               //this information is used for computer player control logic
               //when transitioning from one waypoint link to the other
               CreateTransitionLink((*it1), (*it2));
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

                //precalculate and store distance waypointLink Start
                //point to point where line crosses the checkpoint
                //do a very rough calculation and simply assume that the line crosses
                //the checkpoint in the middle
                (*it2)->distanceStartLinkToCheckpoint =
                        (((*it)->pLineStruct->A + (*it)->pLineStruct->B) * irr::core::vector3df(0.5f, 0.5f, 0.5f) - (*it2)->pLineStruct->A).getLength();

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
    wallCollisionMeshSceneNode = this->mSmgr->addOctreeSceneNode(wallCollisionMesh, 0, IDFlag_IsPickable);

    //hide the collision mesh that the player does not see it
    wallCollisionMeshSceneNode->setVisible(DEF_DBG_WALLCOLLISIONS);

    wallCollisionMeshSceneNode->setMaterialFlag(EMF_BACK_FACE_CULLING, true);
    //wallCollisionMeshSceneNode->setDebugDataVisible(EDS_BBOX);
}

//takes the precalculated wall and column collision data
//and creates the final triangle selector out of it for the
//physics later
void Race::createFinalCollisionData() {

   triangleSelectorWallCollision = this->mSmgr->createOctreeTriangleSelector(
                wallCollisionMesh, wallCollisionMeshSceneNode, 128);
   wallCollisionMeshSceneNode->setTriangleSelector(triangleSelectorWallCollision);

   //only add blocks with collision detection to our column triangle selector
   //so that blocks that should not have collision detection are not part of it
   triangleSelectorColumnswCollision = this->mSmgr->createOctreeTriangleSelector(
                this->mLevelBlocks->blockMeshForCollision, this->mLevelBlocks->BlockCollisionSceneNode, 128);
   this->mLevelBlocks->BlockCollisionSceneNode->setTriangleSelector(triangleSelectorColumnswCollision);

   //also create a triangle selector for blocks without collision detection
   //we use this for ray casting (for example to find target of machine gun)
   triangleSelectorColumnswoCollision = this->mSmgr->createOctreeTriangleSelector(
                this->mLevelBlocks->blockMeshWithoutCollision, this->mLevelBlocks->BlockWithoutCollisionSceneNode, 128);
   this->mLevelBlocks->BlockWithoutCollisionSceneNode->setTriangleSelector(triangleSelectorColumnswoCollision);

   triangleSelectorTerrain = this->mSmgr->createOctreeTriangleSelector(
               this->mLevelTerrain->myTerrainMesh, this->mLevelTerrain->TerrainSceneNode, 128);
  this->mLevelTerrain->TerrainSceneNode->setTriangleSelector(triangleSelectorTerrain);
}

void Race::CheckPlayerCollidedCollectible(Player* player, irr::core::aabbox3d<f32> playerBox) {
    std::list<Collectable*>::iterator it;

    for (it = ENTCollectables_List->begin(); it != ENTCollectables_List->end(); ++it) {
        //only allow player to collect currently visible collectibles
        if ((*it)->GetIfVisible()) {
            //does player bounding box intersect the bounding box of the collectible
            //item?
            if (playerBox.intersectsWithBox((*it)->boundingBox)) {
                //yes, player collected the collectible

                //tell Collectible that is was collected
                (*it)->TriggerCollected();

                //tell player what he collected to alter
                //his stats
                player->CollectedCollectable((*it));
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

    ENTCollectables_List = new std::list<Collectable*>;
    ENTCollectables_List->clear();

    //create all level entities
    for(std::vector<EntityItem*>::iterator loopi = this->mLevelRes->Entities.begin(); loopi != this->mLevelRes->Entities.end(); ++loopi) {
        createEntity(*loopi, this->mLevelRes, this->mLevelTerrain, this->mLevelBlocks, this->mDriver);
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

        line->A = entity->get_Center();
        line->A.X = -line->A.X;   //my Irrlicht coordinate system is swapped at the x axis; correct this issue
        line->B = next->get_Center();
        line->B.X = -line->B.X;   //my Irrlicht coordinate system is swapped at the x axis; correct this issue

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

        //add new waypoint link info struct to vector of all
        //waypoint links
        this->wayPointLinkVec->push_back(newStruct);
    }

    //we also keep a list of all waypoint pointers
    ENTWaypoints_List->push_back(entity);
}

void Race::createEntity(EntityItem *p_entity, LevelFile *levelRes, LevelTerrain *levelTerrain, LevelBlocks* levelBlocks, irr::video::IVideoDriver *driver) {
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

    int next_ID = entity.get_NextID();
    bool exists;

    if (next_ID != 0) {
        //see if a entity with this ID exists
        exists = levelRes->ReturnEntityItemWithId(next_ID, &next);
    }

    EntityItem::EntityType type = entity.get_GameType();

    switch (type) {
        case entity.EntityType::WaypointAmmo:
        case entity.EntityType::WaypointFuel:
        case entity.EntityType::WaypointShield:
        case entity.EntityType::WaypointShortcut:
        case entity.EntityType::WaypointSpecial1:
        case entity.EntityType::WaypointSpecial2:
        case entity.EntityType::WaypointSpecial3:
        case entity.EntityType::WaypointFast:
        case entity.EntityType::WaypointSlow: {
            //add a level waypoint
            AddWayPoint(p_entity, next);
            break;
        }

        case entity.EntityType::WallSegment: {

            if (next != NULL) {
                LineStruct *line = new LineStruct;
                line->A = entity.get_Center();
                line->A.X = -line->A.X;   //my Irrlicht coordinate system is swapped at the x axis; correct this issue
                line->B = next->get_Center();
                line->B.X = -line->B.X;   //my Irrlicht coordinate system is swapped at the x axis; correct this issue
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

      /*  case entity.EntityType::TriggerCraft:
        case entity.EntityType::TriggerRocket:
            w = entity.OffsetX + 1f;
            h = entity.OffsetY + 1f;
            box = new Box(0, 0, 0, w, 2, h, new Vector4(0.9f, 0.3f, 0.6f, 0.5f));
            box.Position = entity.Pos + Vector3.UnitY * 0.01f;
            Entities.AddNode(box);
            break;
        case entity.EntityType::TriggerTimed:
            Billboard timer = new Billboard("images/stopwatch.png", 0.4f, 0.4f);
            timer.Position = entity.Center;
            Entities.AddNode(timer);
            break;*/


    case entity.EntityType::MorphOnce:
    case entity.EntityType::MorphPermanent: {
            w = entity.get_OffsetX() + 1.0f;
            h = entity.get_OffsetY() + 1.0f;
            //box = new Box(0, 0, 0, w, 1, h, new Vector4(0.1f, 0.3f, 0.9f, 0.5f));
            //box.Position = entity.Pos + Vector3.UnitY * 0.01f;
            //AddNode(box);

            EntityItem* source;

            //see if a entity with this ID exists
            bool entFound = levelRes->ReturnEntityItemWithId(entity.get_NextID(), &source);

            // morph for this entity and its linked source
            std::vector<Column*> targetColumns = levelBlocks->ColumnsInRange((int)entity.get_X() , (int)entity.get_Z(), w, h);
            std::vector<Column*> sourceColumns = levelBlocks->ColumnsInRange((int)source->get_X() , (int)source->get_Z(), w, h);

            // regular morph
            if (targetColumns.size() == sourceColumns.size())
            {
                    for (uint i = 0; i < targetColumns.size(); i++)
                    {
                        targetColumns[i]->MorphSource = sourceColumns[i];
                        sourceColumns[i]->MorphSource = targetColumns[i];
                    }
            }
            else
            {
                // permanent morphs dont destroy buildings, instead they morph the column based on terrain height
                if (entity.get_GameType() == entity.EntityType::MorphPermanent)
                {
                    // we need to update surrounding columns too because they could be affected (one side of them)
                    // (problem comes from not using terrain height for all columns in realtime)
                    targetColumns = levelBlocks->ColumnsInRange((int)entity.get_X() - 1, (int)entity.get_Z() - 1, w + 1, h + 1);

                    // create dummy morph source columns at source position
                    std::vector<Column*>::iterator colIt;

                    for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
                        vector3d<irr::f32> colPos(0.0f, 0.0f, 0.0f);
                        colPos.X = source->get_X() + ((*colIt)->Position.X - entity.get_X());
                        colPos.Y = 0.0f;
                        colPos.Z = source->get_Z() + ((*colIt)->Position.Z - entity.get_Z());

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
            Morph* morph = new Morph(entity.get_ID(), source, p_entity, (int)w, (int)h, entity.get_GameType() == entity.EntityType::MorphPermanent);
            std::vector<Column*>::iterator colIt;

            for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
                morph->Columns.push_back(*colIt);
            }

            Morphs.push_back(morph);

            // source
            morph = new Morph(entity.get_ID(), p_entity, source, (int)w, (int)h, entity.get_GameType() == entity.EntityType::MorphPermanent);
            for (colIt = sourceColumns.begin(); colIt != sourceColumns.end(); ++colIt) {
                morph->Columns.push_back(*colIt);
            }

            Morphs.push_back(morph);
            break;
    }

        case entity.EntityType::MorphSource1:
    case entity.EntityType::MorphSource2: {
            // no need to display morph sources since they are handled above by their targets
            break;
    }

        case entity.EntityType::RecoveryTruck: {
            Recovery *recov1 = new Recovery(entity.get_Center().X, entity.get_Center().Y + 6.0f, entity.get_Center().Z, this->mSmgr);

            //remember all recovery vehicles in a vector for later use
            this->recoveryVec->push_back(recov1);

            break;
        }

        case entity.EntityType::Cone: {
            Cone *cone = new Cone(entity.get_X() + 0.5f, entity.get_Y() + 0.104f, entity.get_Z() + 0.5f, this->mSmgr);

            //remember all cones in a vector for later use
            this->coneVec->push_back(cone);

            break;
        }

        case entity.EntityType::Checkpoint:     {
            AddCheckPoint(entity);
            break;
        }
/*
        case entity.EntityType::Explosion:
            BillboardAnimation explosion = new BillboardAnimation("images/tmaps/explosion.png", 1f, 1f, 88, 74, 10);

            explosion.Position = entity.Center;
            Entities.AddNode(explosion);
            break;
*/
        case entity.EntityType::ExtraFuel:
    {
            collectable = new Collectable(p_entity, 29, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }

        case entity.EntityType::FuelFull:
    {
            collectable = new Collectable(p_entity, 30, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }
        case entity.EntityType::DoubleFuel:
    {
            collectable = new Collectable(p_entity, 31, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }

        case entity.EntityType::ExtraAmmo:
    {
            collectable = new Collectable(p_entity, 32, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }
        case entity.EntityType::AmmoFull:
    {
            collectable = new Collectable(p_entity, 33, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }
        case entity.EntityType::DoubleAmmo:
    {
            collectable = new Collectable(p_entity, 34, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }

        case entity.EntityType::ExtraShield:
    {
            collectable = new Collectable(p_entity, 35, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }
        case entity.EntityType::ShieldFull:
    {
            collectable = new Collectable(p_entity, 36, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }
        case entity.EntityType::DoubleShield:
    {
            collectable = new Collectable(p_entity, 37, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }

        case entity.EntityType::BoosterUpgrade:
    {
            collectable = new Collectable(p_entity, 40, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }
        case entity.EntityType::MissileUpgrade:
    {
            collectable = new Collectable(p_entity, 39, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }
        case entity.EntityType::MinigunUpgrade:
    {
            collectable = new Collectable(p_entity, 38, entity.get_Center(), this->mSmgr, driver);
            ENTCollectables_List->push_back(collectable);
            break;
    }

        case entity.EntityType::UnknownShieldItem:
    {
           //uncomment the next 2 lines to show this items also to the player
           // collectable = new Collectable(41, entity.get_Center(), color, driver);
           // ENTCollectables_List.push_back(collectable);
            break;
    }

        case entity.EntityType::UnknownItem:
        case entity.EntityType::Unknown:
    {
           //uncomment the next 2 lines to show this items also to the player
           // collectable = new Collectable(50, entity.get_Center(), color, driver);
           // ENTCollectables_List.push_back(collectable);
            break;
    }

    case entity.EntityType::SteamStrong: {
           irr::core::vector3d<irr::f32> newlocation = entity.get_Center();
           //for my corrdinate system X axis is negative
           newlocation.X = - newlocation.X;
           SteamFountain *sf = new SteamFountain(this->mSmgr, driver, newlocation , 12);
           sf->Activate();

           //add new steam fontain to my list of fontains
           steamFountainVec->push_back(sf);
           break;
    }

    case entity.EntityType::SteamLight: {
           irr::core::vector3d<irr::f32> newlocation = entity.get_Center();
           //for my corrdinate system X axis is negative
           newlocation.X = - newlocation.X;
           SteamFountain *sf = new SteamFountain(this->mSmgr, driver, newlocation , 7);
           sf->Activate();

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


   // if (collectable != NULL) {
    //    Entities.AddNode(collectable);
   // }

   // if (boxSize > 0f)
  //  {
   //     box = new Box(boxSize, entity.Center - 0.5f * boxSize * Vector3.One, color);
    //    Entities.AddNode(box);
   // }
}
