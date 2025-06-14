/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "editorsession.h"
#include "utils/logger.h"
#include "utils/tprofile.h"
#include "models/levelblocks.h"
#include "models/levelterrain.h"
#include "editor.h"
#include "resources/texture.h"
#include "models/morph.h"
#include "utils/logging.h"
#include "models/column.h"
#include "models/collectable.h"
#include "draw/drawdebug.h"
#include "input/input.h"
#include "resources/mapentry.h"
#include "resources/columndefinition.h"

EditorSession::EditorSession(Editor* parentEditor, irr::u8 loadLevelNr) {
    mParentEditor = parentEditor;

    mLevelNrLoaded = loadLevelNr;
    ready = false;

//    //create empty checkpoint info vector
//    checkPointVec = new std::vector<CheckPointInfoStruct*>;
//    checkPointVec->clear();

      //create an empty waypoint link info vector
//    wayPointLinkVec = new std::vector<WayPointLinkInfoStruct*>;
//    wayPointLinkVec->clear();

//    steamFountainVec = new std::vector<SteamFountain*>;
//    steamFountainVec->clear();

//    //my vector of recovery vehicles
//    recoveryVec = new std::vector<Recovery*>;
//    recoveryVec->clear();

//    //my vector of extended region data
//    mExtRegionVec = new std::vector<ExtendedRegionInfoStruct*>;
//    mExtRegionVec->clear();

//    //my vector of charging stations
//    mChargingStationVec = new std::vector<ChargingStation*>;
//    mChargingStationVec->clear();

//    mCollectableSpawnerVec.clear();

//    //my vector of player that need help
//    //of a recovery vehicle and are currently waiting
//    //for it
//    mPlayerWaitForRecoveryVec = new std::vector<Player*>;
//    mPlayerWaitForRecoveryVec->clear();

//    //my vector of cones on the race track
//    coneVec = new std::vector<Cone*>;
//    coneVec->clear();

//    mPlayerVec.clear();
//    mPlayerPhysicObjVec.clear();
//    playerRaceFinishedVec.clear();
//    mTriggerRegionVec.clear();
//    mPendingTriggerTargetGroups.clear();
//    mTimerVec.clear();
//    mExplosionEntityVec.clear();
//    mType2CollectableForCleanupLater.clear();

//    //for the start of the race we want to trigger
//    //target group 1 once
//    mPendingTriggerTargetGroups.push_back(1);

}

void EditorSession::UpdateMorphs(irr::f32 frameDeltaTime) {
    std::list<Morph*>::iterator itMorph;

    for (itMorph = Morphs.begin(); itMorph != Morphs.end(); ++itMorph) {
        (*itMorph)->Update(frameDeltaTime);
    }
}

EditorSession::~EditorSession() {
    delete mRayTerrain;
    delete mRayColumns;

    CleanUpMorphs();

    CleanUpEntities();
}

void EditorSession::Init() {
    //we want to adjust the keymap for the free movable camera
    SKeyMap keyMap[4];

    keyMap[0].Action=EKA_MOVE_FORWARD;   keyMap[0].KeyCode=KEY_KEY_W;
    keyMap[1].Action=EKA_MOVE_BACKWARD;  keyMap[1].KeyCode=KEY_KEY_S;
    keyMap[2].Action=EKA_STRAFE_LEFT;    keyMap[2].KeyCode=KEY_KEY_A;
    keyMap[3].Action=EKA_STRAFE_RIGHT;   keyMap[3].KeyCode=KEY_KEY_D;

    //create a free moving camera that the user can use to
    //investigate the level/map, not used in actual game
    mCamera = mParentEditor->mSmgr->addCameraSceneNodeFPS(0, 100.0f,0.05f ,-1 ,
                                            keyMap, 4, false, 0.0f);

    //create a DrawDebug object
    this->mDrawDebug = new DrawDebug(mParentEditor->mDriver);

    mRayTerrain = new Ray(mDrawDebug);
    mRayColumns = new Ray(mDrawDebug);

    if (!LoadLevel()) {
        //there was an error loading the level
        return;
    }

    //level was loaded ok, we can continue setup

    //create the object for path finding and services
    //mPath = new Path(this, mDrawDebug);

    //get player start locations from the level file
    /*mPlayerStartLocations =
        this->mLevelTerrain->GetPlayerRaceTrackStartLocations();*/

    mParentEditor->mSmgr->setActiveCamera(mCamera);

    //SetupTopRaceTrackPointerOrigin();

    //create a new Bezier object for testing
    //testBezier = new Bezier(mLevelTerrain, mDrawDebug);

    ready = true;

    //only to test if we can save a levelfile properly!
    //std::string testsaveName("testsave.dat");
    //this->mLevelRes->Save(testsaveName);
}

//creates final TriangleSelectors to be able to do ray
//intersection from user mouse pointer to level environment
//for object selection
void EditorSession::createTriangleSelectors() {
   triangleSelectorColumnswCollision = mParentEditor->mSmgr->createOctreeTriangleSelector(
                this->mLevelBlocks->blockMeshForCollision, this->mLevelBlocks->BlockCollisionSceneNode, 128);
   this->mLevelBlocks->BlockCollisionSceneNode->setTriangleSelector(triangleSelectorColumnswCollision);

   triangleSelectorColumnswoCollision = mParentEditor->mSmgr->createOctreeTriangleSelector(
                this->mLevelBlocks->blockMeshWithoutCollision, this->mLevelBlocks->BlockWithoutCollisionSceneNode, 128);
   this->mLevelBlocks->BlockWithoutCollisionSceneNode->setTriangleSelector(triangleSelectorColumnswoCollision);

   triangleSelectorStaticTerrain = mParentEditor->mSmgr->createOctreeTriangleSelector(
               this->mLevelTerrain->myStaticTerrainMesh, this->mLevelTerrain->StaticTerrainSceneNode, 128);
  this->mLevelTerrain->StaticTerrainSceneNode->setTriangleSelector(triangleSelectorStaticTerrain);

   triangleSelectorDynamicTerrain = mParentEditor->mSmgr->createOctreeTriangleSelector(
               this->mLevelTerrain->myDynamicTerrainMesh, this->mLevelTerrain->DynamicTerrainSceneNode, 128);
  this->mLevelTerrain->DynamicTerrainSceneNode->setTriangleSelector(triangleSelectorDynamicTerrain);
}

bool EditorSession::LoadLevel() {
    if ((mLevelNrLoaded < 1) || (mLevelNrLoaded > 9)) {
        logging::Error("Level number only possible from 1 up to 9!");
        return false;
    }

    int load_texnr = mLevelNrLoaded;
    if (mLevelNrLoaded == 7) load_texnr = 1; // original game has this hardcoded too

   /***********************************************************/
   /* Load selected level file                                */
   /***********************************************************/
   char levelfilename[50];
   char str[20];

   strcpy(levelfilename, "extract/level0-");
   sprintf(str, "%d", mLevelNrLoaded);
   strcat(levelfilename, str);
   strcat(levelfilename, "/level0-");
   strcat(levelfilename, str);
   strcat(levelfilename, "-unpacked.dat");

   char texfilename[50];
   strcpy(texfilename, "extract/level0-");
   sprintf(str, "%d", load_texnr);
   strcat(texfilename, str);
   strcat(texfilename, "/tex");

   char spritefilename[50];
   strcpy(spritefilename, "extract/sprites/tmaps");

   /***********************************************************/
   /* Load level textures                                     */
   /***********************************************************/
   mTexLoader = new TextureLoader(mParentEditor->mDriver, texfilename, spritefilename);

   //load the level data itself
   this->mLevelRes = new LevelFile(levelfilename);

   //was loading level data succesful? if not interrupt
   if (!this->mLevelRes->get_Ready()) {
       logging::Error("EditorSession::LoadLevel failed, exiting");
       return false;
   }

   char terrainname[50];
   strcpy(terrainname, "Terrain1");

   //Test map save
   //this->mLevelRes->Save(std::string("mapsave.dat"));

   /***********************************************************/
   /* Prepare level terrain                                   */
   /***********************************************************/
   //for the level editor do not optimize the Terrain mesh!
   //08.06.2025: TODO, right now still optimize, Terrain Mesh not able to handle more
   //tehn 65k Vertices because of MeshBuffer, fix later!
   this->mLevelTerrain = new LevelTerrain(this->mParentEditor, terrainname, this->mLevelRes, mTexLoader, true, false);

   /***********************************************************/
   /* Create building (cube) Mesh                             */
   /***********************************************************/
   //this routine also generates the column/block collision information inside that
   //we need for collision detection later
   this->mLevelBlocks = new LevelBlocks(this->mParentEditor, this->mLevelTerrain, this->mLevelRes, mTexLoader,
                                        DebugShowWallCollisionMesh, false);

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

  createTriangleSelectors();

  //Add triangle selector of terrain to rayTarget mesh so that we can figure out
  //at which terrain cells the users mouse it pointing at currently
  mRayTerrain->AddRayTargetMesh(this->triangleSelectorStaticTerrain);
  mRayTerrain->AddRayTargetMesh(this->triangleSelectorDynamicTerrain);

  //do the same for all columns (blocks) in the level, so that the user
  //can select columns (blocks)
  mRayColumns->AddRayTargetMesh(this->triangleSelectorColumnswoCollision);
  mRayColumns->AddRayTargetMesh(this->triangleSelectorColumnswCollision);

  // driver->setFog(video::SColor(0,138,125,81), video::EFT_FOG_LINEAR, 100, 250, .03f, false, true);

  //Final data calculation for all checkpoints
  //Find for each waypoint normal direction of race
  //so that we know if the player passes checkpoints
  //in forwards or reverse direction
  //CheckPointPostProcessing();

  //create existing charging stations
  //CreateChargingStations();

  //create a bounding box for valid player
  //location testing
  mLevelTerrain->StaticTerrainSceneNode->updateAbsolutePosition();

  return true;
}

void EditorSession::createLevelEntities() {

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
        createEntity(*loopi, this->mLevelRes, this->mLevelTerrain, this->mLevelBlocks, mParentEditor->mDriver);
    }
}

void EditorSession::createEntity(EntityItem *p_entity,
                        LevelFile *levelRes, LevelTerrain *levelTerrain, LevelBlocks* levelBlocks, irr::video::IVideoDriver *driver) {
    //Line line;
    irr::f32 w, h;
    Collectable* collectable;
    //Box box;

    //make local variable which points on pointer
    EntityItem entity = *p_entity;
    EntityItem *next = nullptr;

//    if (!GroupedEntities.ContainsKey(entity.Group)) GroupedEntities.Add(entity.Group, new List<EntityItem>());
//    GroupedEntities[entity.Group].Add(entity);

    float boxSize = 0;
    collectable = nullptr;

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
            //TODO:
            //AddWayPoint(p_entity, next);
            break;
        }

        case Entity::EntityType::WallSegment: {

            if (next != nullptr) {
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
            //TODO: AddTrigger(p_entity);
            break;
       }

       case Entity::EntityType::TriggerTimed: {
                //TODO: AddTimer(p_entity);
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
                                             this->mLevelTerrain, this->mLevelBlocks);
                    std::vector<Column*>::iterator colIt;

                    for (colIt = targetColumns.begin(); colIt != targetColumns.end(); ++colIt) {
                        morph->Columns.push_back(*colIt);
                    }

                    Morphs.push_back(morph);

                    // source
                    morph = new Morph(entity.get_ID(), p_entity, source, (int)w, (int)h,
                                      entity.getEntityType() == Entity::EntityType::MorphPermanent,
                                      this->mLevelTerrain, this->mLevelBlocks);
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
            /*TODO: Recovery *recov1 =
                    new Recovery(this, entity.getCenter().X, entity.getCenter().Y + 6.0f, entity.getCenter().Z, mInfra->mSmgr);

            //remember all recovery vehicles in a vector for later use
            this->recoveryVec->push_back(recov1);*/

            break;
        }

        case Entity::EntityType::Cone: {
            /*TODO: irr::core::vector3df center = entity.getCenter();
            Cone *cone = new Cone(this, center.X, center.Y + 0.104f, center.Z, mInfra->mSmgr);

            //remember all cones in a vector for later use
            this->coneVec->push_back(cone);
                */
            break;
        }

        case Entity::EntityType::Checkpoint:     {
            //30.05.2025: It seems in Level 7 the map
            //designer made a mistake, and added a second
            //checkpoint with value 4, but with DX = 0 and DY = 0
            //This additional (faulty) checkpoint prevents my
            //lap counting from working properly
            //to fix this here make sure that if DX = 0 and DY = 0
            //we do not add this fault checkpoint
            if ((entity.getOffsetX() != 0.0f) || (entity.getOffsetY() != 0.0f))
              {
                //TODO:
                //AddCheckPoint(entity);
               }
            break;
        }

        case Entity::EntityType::Camera: {
            //TODO: AddCamera(p_entity);
            break;
        }

        case Entity::EntityType::Explosion: {
                //TODO: AddExplosionEntity(p_entity);
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
                    //if entity type is invalid for a collectable the function below will fallback
                    //to sprite number 42, which is a sprite I did not know the purpose of
                    irr::u16 spriteNr = GetCollectableSpriteNumber(entity.getEntityType());

                    //Point to the correct (billboard) texture
                    collectable = new Collectable(this->mParentEditor, p_entity, entity.getCenter(), mTexLoader->spriteTex.at(spriteNr), false);
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
               //TODO:
//               irr::core::vector3d<irr::f32> newlocation = entity.getCenter();
//               SteamFountain *sf = new SteamFountain(this, p_entity, mInfra->mSmgr, driver, newlocation , 100);

//               //only for first testing
//               //sf->Activate();

//               //it seems when SteamFountains are created the are not
//               //active yet in the game, the are normally triggered to be
//               //active by a craft trigger or similar

//               //add new steam fontain to my list of fontains
//               steamFountainVec->push_back(sf);
               break;
        }

        case Entity::EntityType::SteamLight: {
               //TODO:
//               irr::core::vector3d<irr::f32> newlocation = entity.getCenter();
//               SteamFountain *sf = new SteamFountain(this, p_entity, mInfra->mSmgr, driver, newlocation , 50);

//               //only for first testing
//               //sf->Activate();

//               //it seems when SteamFountains are created the are not
//               //active yet in the game, the are normally triggered to be
//               //active by a craft trigger or similar

//               //add new steam fontain to my list of fontains
//               steamFountainVec->push_back(sf);
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
irr::u16 EditorSession::GetCollectableSpriteNumber(Entity::EntityType mEntityType) {
    irr::u16 nrSprite;

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

  return nrSprite;
}

void EditorSession::DrawOutlineSelectedCell(irr::core::vector2di selCellCoordinate, SMaterial* color) {
    irr::core::vector3df pos1 = mLevelTerrain->pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert1->Pos;
    pos1.X = -pos1.X;
    pos1.Y = -pos1.Y;

    irr::core::vector3df pos2 = mLevelTerrain->pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert2->Pos;
    pos2.X = -pos2.X;
    pos2.Y = -pos2.Y;

    irr::core::vector3df pos3 = mLevelTerrain->pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert3->Pos;
    pos3.X = -pos3.X;
    pos3.Y = -pos3.Y;

    irr::core::vector3df pos4 = mLevelTerrain->pTerrainTiles[selCellCoordinate.X][selCellCoordinate.Y].vert4->Pos;
    pos4.X = -pos4.X;
    pos4.Y = -pos4.Y;

    mDrawDebug->Draw3DRectangle(pos1, pos2, pos3, pos4, color);
}

void EditorSession::DrawOutlineSelectedColumn(Column* selColumnPntr, int nrBlockFromBase, SMaterial* color) {
    if (selColumnPntr == nullptr)
        return;

    int idx1 = nrBlockFromBase * 24;
    int nrVertices = selColumnPntr->GeometryInfoList->vertices.size();

    irr::core::vector3df pos1;
    irr::core::vector3df pos2;
    irr::core::vector3df pos3;
    irr::core::vector3df pos4;

    if ((idx1 >= 0) && (idx1 < (nrVertices - 23))) {
        for (int idx = 0; idx < 6; idx++) {
            irr::core::vector3df pos1 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + idx * 4).currPosition;
            irr::core::vector3df pos2 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + idx * 4 + 1).currPosition;
            irr::core::vector3df pos3 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + idx * 4 + 2).currPosition;
            irr::core::vector3df pos4 = selColumnPntr->GeometryInfoList->vertices.at(idx1 + idx * 4 + 3).currPosition;

            mDrawDebug->Draw3DRectangle(pos1, pos2, pos3, pos4, color);
        }
     }
}

void EditorSession::TestDialog() {
    gui.drop();

    // set skin font
    /*IGUIFont* font = env->getFont("fontlucida.png");
    if (font)
        env->getSkin()->setFont(font);*/

    dimension2d<u32> dim ( 300, 200 );

    gui.Window = mParentEditor->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Textures" );

    gui.testButton = mParentEditor->mGuienv->addButton(core::recti(50, 50, 100, 80), gui.Window, -1, L"Test");

//    // create a visible Scene Tree
//    mInfra->mGuienv->addStaticText ( L"Textures:", rect<s32>( 10, 20, dim.Width, 30 ),false, false, gui.Window, -1, false );
//    gui.SceneTree = mInfra->mGuienv->addTreeView(	rect<s32>( 10,  40, dim.Width - 20, dim.Height - 40 ),
//                                        gui.Window, -1, true, true, false );
//    gui.SceneTree->getRoot()->clearChildren();
//    /*gui.SceneTree->addChild( mInfra->mGuienv->addStaticText(L"1", rect<s32>( 0, 0, 20, 20 )));
//    gui.SceneTree->addChild( mInfra->mGuienv->addStaticText(L"2", rect<s32>( 0, 20, 20, 30 )));
//    gui.SceneTree->addChild( mInfra->mGuienv->addStaticText(L"3", rect<s32>( 0, 30, 20, 40 )));*/

//    for (int idx = 0; idx < 10; idx++) {
//       gui.SceneTree->addChild( mInfra->mGuienv->addImage(mTexLoader->levelTex.at(idx), irr::core::vector2d(20, 20 + idx * 64), false, gui.SceneTree, -1));
//    }

   /* IGUIImageList* imageList = mInfra->mGuienv->createImageList( mTexLoader->levelTex.at(0),
                                        dimension2di( 64, 64 ), true );

    if ( imageList )
        {
            gui.SceneTree->setImageList( imageList );
            imageList->drop ();
        }*/

}

void EditorSession::Render() {
    //if we do not use XEffects we can simply render the sky
    //with XEffect this does not work, need a solution for this!
//    if (!mInfra->mUseXEffects) {
//        //we need to draw sky image first, the remaining scene will be drawn on top of it
//        DrawSky();
//    }

    //draw 3D world coordinate axis with arrows
    mDrawDebug->DrawWorldCoordinateSystemArrows();

    if (DebugShowWaypoints) {
        //DebugDrawWayPointLinks(false);
     }

    std::vector<LineStruct*>::iterator Linedraw_iterator2;

    if (DebugShowWallSegments) {
         //draw all wallsegments for debugging purposes
         mParentEditor->mDriver->setMaterial(*mDrawDebug->red);
         for(Linedraw_iterator2 = ENTWallsegmentsLine_List->begin(); Linedraw_iterator2 != ENTWallsegmentsLine_List->end(); ++Linedraw_iterator2) {
              mParentEditor->mDriver->setMaterial(*mDrawDebug->red);
              mParentEditor->mDriver->draw3DLine((*Linedraw_iterator2)->A, (*Linedraw_iterator2)->B);
         }
     }

    if (DebugShowLowLevelTriangleSelection) {
        if (triangleHitByMouse) {
             mDrawDebug->Draw3DTriangleOutline(&triangleMouseHit.hitTriangle, mDrawDebug->white);
        }

        if (secondTriangleHitByMouse) {
             mDrawDebug->Draw3DTriangleOutline(&secondTriangleMouseHit.hitTriangle, mDrawDebug->red);
        }
    }

    if (mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
        DrawOutlineSelectedCell(mCurrSelectedItem.mCellCoordSelected, mDrawDebug->white);
    } else if (mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
        DrawOutlineSelectedColumn(mCurrSelectedItem.mColumnSelected, mCurrSelectedItem.mSelBlockNrStartingFromBase, mDrawDebug->cyan);
    }

    /*if (mCellSelectedByMouse) {
        DrawOutlineSelectedCell(mCellCoordSelectedByMouse, mDrawDebug->white);
    }*/

//    if (DebugShowCheckpoints) {
//          //draw all checkpoint lines for debugging purposes
//          mInfra->mDriver->setMaterial(*mDrawDebug->blue);
//          for(CheckPoint_iterator = checkPointVec->begin(); CheckPoint_iterator != checkPointVec->end(); ++CheckPoint_iterator) {
//              mInfra->mDriver->draw3DLine((*CheckPoint_iterator)->pLineStruct->A, (*CheckPoint_iterator)->pLineStruct->B);
//          }
//    }

//        if (DebugShowRegionsAndPointOfInterest) {
//                std::list<MapPointOfInterest>::iterator it;

//                for (it = this->mLevelRes->PointsOfInterest.begin(); it != this->mLevelRes->PointsOfInterest.end(); ++it) {
//                    mDrawDebug->Draw3DLine(this->topRaceTrackerPointerOrigin, (*it).Position,
//                                           this->mDrawDebug->pink);
//                }

//                IndicateMapRegions();
//        }

//        if (DebugShowTriggerRegions) {
//            IndicateTriggerRegions();
//        }

       /* if (mChargingStationVec->size() > 0) {
            std::vector<ChargingStation*>::iterator it;
            for (it = mChargingStationVec->begin(); it != mChargingStationVec->end(); ++it) {
                (*it)->DebugDraw();
            }
        }*/
}

void EditorSession::HandleBasicInput() {

    if(mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_F4))
    {
         if (mParentEditor->mLogger->IsWindowHidden()) {
             mParentEditor->mLogger->ShowWindow();
         } else {
             mParentEditor->mLogger->HideWindow();
         }

         if (mParentEditor->mTimeProfiler->IsWindowHidden()) {
             mParentEditor->mTimeProfiler->ShowWindow();
         } else {
             mParentEditor->mTimeProfiler->HideWindow();
         }
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_K)) {
        TestDialog();
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_T)) {
        mLevelTerrain->SwitchViewMode();
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_KEY_B)) {
        mLevelBlocks->SwitchViewMode();
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
        this->exitEditorSession = true;
    }

    if (mParentEditor->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_SPACE)) {
         scene::ICameraSceneNode * camera =
                 mParentEditor->mDevice->getSceneManager()->getActiveCamera();
                 if (camera)
                    {
                        camera->setInputReceiverEnabled( !camera->isInputReceiverEnabled() );
                    }
              }
}

//derives more high level information about the selected terrain cell
void EditorSession::DeriveSelectedTerrainCellInformation(RayHitTriangleInfoStruct* hitTriangleInfo) {
    //a terrain cell is selected currently
    mCurrSelectedItem.SelectedItemType = DEF_EDITOR_SELITEM_CELL;
    mCurrSelectedItem.RayHitInfo = *hitTriangleInfo;
    mCurrSelectedItem.mColumnSelected = nullptr;

    int closestVertice;

    //which cell coordinate is selected
    mCurrSelectedItem.mCellCoordSelected =
            this->mLevelTerrain->GetClosestTileGridCoordToMapPosition(hitTriangleInfo->hitPointOnTriangle, closestVertice);

    mCurrSelectedItem.mCellCoordVerticeNrSelected = closestVertice;
}

//derives more high level information about the selected block
void EditorSession::DeriveSelectedBlockInformation(RayHitTriangleInfoStruct* hitTriangleInfo, RayHitTriangleInfoStruct* hitSecondClosestTriangleInfo) {
    //a block of a column is selected currently
    mCurrSelectedItem.SelectedItemType = DEF_EDITOR_SELITEM_BLOCK;
    mCurrSelectedItem.RayHitInfo = *hitTriangleInfo;

    int closestVertice;

    //if we want to find the column position closer to the middle we can use the 2 hit triangles we got, from the enter and exit point
    //we just need to use the arithmetic middle value of enter and exit point
    irr::core::vector3df avgPos = (hitTriangleInfo->hitPointOnTriangle + hitSecondClosestTriangleInfo->hitPointOnTriangle) / 2.0f;

    //according to the coordinates of the hitpoint, in which cell coordinate are way?
    irr::core::vector2di cellCoord =
        this->mLevelTerrain->GetClosestTileGridCoordToMapPosition(avgPos, closestVertice);

    //is at the cellCoord a column?
    MapEntry* entry = this->mLevelTerrain->GetMapEntry(cellCoord.X, cellCoord.Y);

    ColumnDefinition* columDef = entry->get_Column();
    int nrSkippedBlocks = 0;

    if (columDef != nullptr) {
        mCurrSelectedItem.mColumnDefinitionSelected = columDef;

        //which column is this?
        //there is a "position" key we can use for search
        int posKey =  cellCoord.X + cellCoord.Y * this->mLevelTerrain->levelRes->Width();
        Column* columnPntr;

        if (mLevelBlocks->searchColumnWithPosition(posKey, columnPntr)) {
            //we found the column
            mCurrSelectedItem.mColumnSelected = columnPntr;

            std::vector<int>::iterator it;
            for (it = columDef->mInCollisionMesh.begin(); it != columDef->mInCollisionMesh.end(); ++it) {
                if ((*it) == 0) {
                    nrSkippedBlocks++;
                } else {
                    break;
                }
            }
        } else {
            mCurrSelectedItem.mColumnSelected = nullptr;
        }

        irr::f32 heightOfTerrain =
                this->mLevelTerrain->GetMapEntry(cellCoord.X,cellCoord.Y)->m_Height;

        irr::f32 heightAboveTerrain = avgPos.Y - heightOfTerrain;

        int nrBlocks = (int)(heightAboveTerrain / (irr::f32)(this->mLevelTerrain->segmentSize)) - nrSkippedBlocks;

        mCurrSelectedItem.mSelBlockNrStartingFromBase = nrBlocks;
    }
}

void EditorSession::HandleMouse() {
    mCurrentMousePos = mParentEditor->mEventReceiver->MouseState.Position;

    // Create a ray through the mouse cursor.
    core::line3df ray = mParentEditor->mSmgr->getSceneCollisionManager()->getRayFromScreenCoordinates(
                     mCurrentMousePos, mCamera);

    mCurrSelectedItem.SelectedItemType = DEF_EDITOR_SELITEM_NONE;
    mCurrSelectedItem.mColumnSelected = nullptr;
    mCurrSelectedItem.mColumnDefinitionSelected = nullptr;

    /***********************************************************************
     * First try mouse ray intersection with Terrain to find possible      *
     * selected Terrain cell                                               *
     ***********************************************************************/

    triangleHitByMouse = false;
    secondTriangleHitByMouse = false;

    irr::core::vector3df endPoint = ray.start + ray.getVector().normalize() * 100.0f;

    //built a ray cast 3d line to find out at which 3D object the users mouse
    //is pointing at
    std::vector<irr::core::vector3di> voxels;

    std::vector<RayHitTriangleInfoStruct*> allHitTrianglesTerrain;

    //with ReturnOnlyClosestTriangles = true!
    allHitTrianglesTerrain = mRayTerrain->ReturnTrianglesHitByRay( mRayTerrain->mRayTargetSelectors,
                                  ray.start, endPoint, true);

    int vecSize = (int)(allHitTrianglesTerrain.size());
    std::vector<RayHitTriangleInfoStruct*>::iterator it;

    irr::f32 minDistanceTerrain;
    RayHitTriangleInfoStruct* nearestTriangleHitTerrain = nullptr;

    if (vecSize > 0) {
            //we have at least one triangle, we need to find the closest one to the user
            bool firstElement = true;
            irr::f32 currDist;

            for (it = allHitTrianglesTerrain.begin(); it != allHitTrianglesTerrain.end(); ++it) {
                currDist = (*it)->distFromRayStartSquared;
                if (firstElement) {
                    firstElement = false;
                    minDistanceTerrain = currDist;
                    nearestTriangleHitTerrain = (*it);
                } else if (currDist < minDistanceTerrain) {
                    minDistanceTerrain = currDist;
                    nearestTriangleHitTerrain = (*it);
                }
            }
    }

    /***********************************************************************
     * Second try mouse ray intersection with all columms of blocks        *
     * that we have in the level                                           *
     ***********************************************************************/

    std::vector<RayHitTriangleInfoStruct*> allHitTrianglesColumns;

    //with ReturnOnlyClosestTriangles = true!
    allHitTrianglesColumns = mRayColumns->ReturnTrianglesHitByRay( mRayColumns->mRayTargetSelectors,
                                  ray.start, endPoint, true);

    vecSize = (int)(allHitTrianglesColumns.size());

    irr::f32 minDistanceColumns;
    RayHitTriangleInfoStruct* nearestTriangleHitColumns = nullptr;
    RayHitTriangleInfoStruct* secondNearestTriangleHitColumns = nullptr;
    irr::f32 secondMinDistanceColumns;

    //if we hit through a cube we should have at least 2 hit triangles, one for the front where the ray enters,
    //the other one at the opposite site where the ray exits again
    if (vecSize > 1) {
            //we have at least two triangles, we need to find the closest and the second closest to the user
            bool firstElement = true;
            irr::f32 currDist;

            for (it = allHitTrianglesColumns.begin(); it != allHitTrianglesColumns.end(); ++it) {
                currDist = (*it)->distFromRayStartSquared;
                if (firstElement) {
                    firstElement = false;
                    minDistanceColumns = currDist;
                    nearestTriangleHitColumns = (*it);
                } else if (currDist < minDistanceColumns) {
                    minDistanceColumns = currDist;
                    nearestTriangleHitColumns = (*it);
                }
            }

            //now find the seconds closest triangle
            firstElement = true;

            for (it = allHitTrianglesColumns.begin(); it != allHitTrianglesColumns.end(); ++it) {
                //for the second element we need to skip the already found closest element
                //so that we find the 2nd closest triangle
                if ((*it) != nearestTriangleHitColumns) {
                    currDist = (*it)->distFromRayStartSquared;
                    if (firstElement) {
                        firstElement = false;
                        secondMinDistanceColumns = currDist;
                        secondNearestTriangleHitColumns = (*it);
                    } else if (currDist < secondMinDistanceColumns) {
                        secondMinDistanceColumns = currDist;
                        secondNearestTriangleHitColumns = (*it);
                    }
                }
            }
    }

    //for acceptable cube intersection of a column we need to have at least two hit triangles!
    bool foundCubeIntersection = ((nearestTriangleHitColumns != nullptr) && (secondNearestTriangleHitColumns != nullptr));

    //find out if the closest triangle we hit is from the terrain or
    //a column. In case we find a hit triangle for both ray intersections
    //the triangle with the shortest distance to the players mouse pointer
    //wins and is selected
    RayHitTriangleInfoStruct* triangleHit = nullptr;

    if ((nearestTriangleHitTerrain != nullptr) && (!foundCubeIntersection)) {
        triangleHit = nearestTriangleHitTerrain;
        DeriveSelectedTerrainCellInformation(nearestTriangleHitTerrain);
    } else if ((nearestTriangleHitTerrain == nullptr) && foundCubeIntersection) {
        triangleHit = nearestTriangleHitColumns;
        secondTriangleHitByMouse = true;
        secondTriangleMouseHit = *secondNearestTriangleHitColumns;
        DeriveSelectedBlockInformation(nearestTriangleHitColumns, secondNearestTriangleHitColumns);

    } else if ((nearestTriangleHitTerrain != nullptr) && (foundCubeIntersection)) {
        if (minDistanceTerrain < minDistanceColumns) {
            //the terrain is closer, lets select a triangle of the terrain
            triangleHit = nearestTriangleHitTerrain;
            DeriveSelectedTerrainCellInformation(nearestTriangleHitTerrain);
        } else {
            //a column is closer, lets select the column block
            triangleHit = nearestTriangleHitColumns;
            DeriveSelectedBlockInformation(nearestTriangleHitColumns, secondNearestTriangleHitColumns);
            secondTriangleHitByMouse = true;
            secondTriangleMouseHit = *secondNearestTriangleHitColumns;
        }
    }

    //store the triangle hit also for low level ray
    //implementation debugging in render function of level editor
    if (triangleHit != nullptr) {
        triangleHitByMouse = true;
        triangleMouseHit = *triangleHit;
    }

    //cleanup triangle hit information again
    //otherwise we have a memory leak!
    mRayColumns->EmptyTriangleHitInfoVector(allHitTrianglesColumns);

    //cleanup triangle hit information again
    //otherwise we have a memory leak!
    mRayTerrain->EmptyTriangleHitInfoVector(allHitTrianglesTerrain);
}

void EditorSession::End() {
    //empty right now
}

void EditorSession::CleanUpEntities() {

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
   ENTWallsegmentsLine_List = nullptr;

   if (ENTWaypoints_List->size() > 0) {
       std::vector<EntityItem*>::iterator it;

       for (it = ENTWaypoints_List->begin(); it != ENTWaypoints_List->end(); ) {
           it = ENTWaypoints_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTWaypoints_List;
   ENTWaypoints_List = nullptr;

   if (ENTWallsegments_List->size() > 0) {
       std::list<EntityItem*>::iterator it;

       for (it = ENTWallsegments_List->begin(); it != ENTWallsegments_List->end(); ) {
           it = ENTWallsegments_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTWallsegments_List;
   ENTWallsegments_List = nullptr;

   if (ENTTriggers_List->size() > 0) {
       std::list<EntityItem*>::iterator it;

       for (it = ENTTriggers_List->begin(); it != ENTTriggers_List->end(); ) {
           it = ENTTriggers_List->erase(it);

           //the entityItems itself are deleted inside
           //Levelfile source code
       }
   }

   delete ENTTriggers_List;
   ENTTriggers_List = nullptr;

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
   ENTCollectablesVec = nullptr;
}

void EditorSession::CleanUpMorphs() {
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

