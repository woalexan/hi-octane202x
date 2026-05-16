/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

//This source code was implemented by me based on the insight I gained into the original game. Some parts do more closely follow
//the Pseudo-C Code that is available to me, and other parts are heavily modified to hopefully work inside my existing project
//in the near future.
//The original game uses fixed point arithmetic for performance reasons (it had to run on 80486 after all), and is optimized everywhere you
//look at. I do not want to do the same, because the fixed point arithmetic makes it very difficult to read and understand. And performance should not
//be the problem nowadays. Therefore the biggest change which I have to do is to change all the logic to floating point arithmetic,
//while hopefully not breaking the concept behind everything.
//Additionally I will only try to reimplement the most important mechanisms and concepts from the original game where it benefits the overall project,
//but I will not change the overall project to use the same structs and data structures as the original. This would not work, and also does not
//make sense in my opinion.

//Important note: What makes this source code very difficult to handle is the fact, that my coordinate system in this existing project is
//completely different to the one in the original game. The original uses X and Y axis for the tile map, and Z is the height.
//For the levelfile and 2D map stuff I also use X any Y axis for the tile map most of the time.
//My 3D world setup (for rendering) using Irrlicht has X and Z for the tile map, and Y is the height. And to make things worse my Irrlicht vertice X and Y coordinates
//have a swapped sign (are negative) currently.

//I have decided to also use the original games coordinate system in all vanilla calculations. At the interface between
//original game calculations and Irrlicht 3D coordinate system I have then to convert from one coordinate system setup to the other.
//Thats the drawback I will have.

//I really want to thank aybe for giving me the opportunity to look much deeper into the original game inner workings as I was ever able before.
//Without this support I would not have been able to hopefully advance the current project more true to the original.

#include "vcalc.h"

#include "../models/levelterrain.h"
#include "../models/levelblocks.h"
#include "../resources/columndefinition.h"
#include "../models/column.h"
#include "../resources/levelfile.h"
#include "../resources/mapentry.h"
#include "../infrabase.h"
#include "../utils/logging.h"
#include "../draw/drawdebug.h"
#include <cmath>

VCalculations::VCalculations(InfrastructureBase* infra, LevelFile* levelFile, LevelTerrain* levelTerrain, LevelBlocks* levelBlocks)
{
    mInfra = infra;
    mLevelFile = levelFile;
    mLevelTerrain = levelTerrain;
    mLevelBlocks = levelBlocks;

    testTex = mInfra->mDriver->getTexture("extract/sprites/tmaps0029.png");
}

irr::core::vector3df VCalculations::VanillaToIrrlichtCoord(irr::core::vector3df vanillaCoord) {
    irr::core::vector3df result;

    result.X = -vanillaCoord.X;
    result.Y = vanillaCoord.Z;
    result.Z = vanillaCoord.Y;

    return result;
}

irr::core::vector3df VCalculations::IrrlichtToVanillaCoord(irr::core::vector3df irrlichtCoord) {
    irr::core::vector3df result;

    result.X = -irrlichtCoord.X;
    result.Y = irrlichtCoord.Z;
    result.Z = irrlichtCoord.Y;

    return result;
}

void VCalculations::AddTestObject(irr::core::vector3df position) {
    this->testThing1 = new ThingDataStruct();
    testThing1->Position = IrrlichtToVanillaCoord(position);
    testThing1->Stationary = false;
    /*testThing1->Movement.SpeedActual =
         FixedPointToFloat((int16_t)(mInfra->randRangeInt(80, 80 + 0x50)));
         -FixedPointToFloat((int16_t)(mInfra->randRangeInt(3640, 3640 + 0x31C7)));*/

    //move_displacement_set(testThing1->Displacement, 90.0f, 135.0f, 1.0f);
    move_displacement_set(testThing1->Displacement, 150.0f, 135.0f, 1.0f);

    testThing1->Life = 200;

    this->testNode  = mInfra->mSmgr->addBillboardSceneNode();
    this->testNode->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
    this->testNode->setMaterialTexture(0, testTex);

    //Important: let collectables (Billboards) unaffected by lightning,
    //otherwise there are sometimes not good to see for the player
    this->testNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    this->testNode->setMaterialFlag(irr::video::EMF_ZBUFFER, true);

    this->testNode->setPosition(VanillaToIrrlichtCoord(testThing1->Position));
    this->testNode->setSize(irr::core::dimension2d<irr::f32>(0.45f, 0.45f));

    //get bounding box for this collectible
    this->testNode->updateAbsolutePosition();
   // this->boundingBox = this->billSceneNode->getTransformedBoundingBox();
}

void VCalculations::Update() {
    UpdateTestObject(0.1f, *testThing1);
}

void VCalculations::DebugDrawDisplacement(ThingDataStruct& whichThing) {
    irr::core::vector3df dirVec = VanillaToIrrlichtCoord(whichThing.Displacement);
    dirVec.normalize();

    irr::core::vector3df irrCoord1 = VanillaToIrrlichtCoord(whichThing.Position);
    mInfra->mDrawDebug->Draw3DLine(irrCoord1, irrCoord1 + dirVec * irr::core::vector3df(1.0f, 1.0f, 1.0f), mInfra->mDrawDebug->orange);
}

void VCalculations::DebugDraw() {
    DebugDrawDisplacement(*testThing1);
}

//Test implementation from powerup_move function
int8_t VCalculations::UpdateTestObject(irr::f32 deltaTime, ThingDataStruct& whichThing) {
   irr::core::vector3df intPosition;
   irr::core::vector3df intDisplacement;
   irr::f32 XPos;
   irr::f32 YPos;
   irr::f32 ZPos;

   bool isNoCollision;
   bool Flag1Set;
   bool Flag2Set;
   bool Flag4Set;

   if (!whichThing.Stationary) {
      intPosition.X = whichThing.Position.X;
      intPosition.Y = whichThing.Position.Y;
      intPosition.Z = whichThing.Position.Z;

      XPos = whichThing.Displacement.X;
      irr::f32 v9 = XPos + (float)(7.0f / 256.0f);
      if (v9 < 0.0f) {
          v9 += (float)(7.0f / 256.0f);
      }

      whichThing.Displacement.X -= (v9 / 8.0f);

      YPos = whichThing.Displacement.Y;
      irr::f32 v11 = YPos + (float)(7.0f / 256.0f);
      if (v11 < 0.0f) {
          v11 += (float)(7.0f / 256.0f);
      }

      whichThing.Displacement.Y -= (v11 / 8.0f);

      ZPos = whichThing.Displacement.Z;
      ZPos -= (float)(8.0f / 256.0f);

      if (ZPos < (float)(-100.0f / 256.0f)) {
          ZPos = (float)(-100.0f / 256.0f);
      }

      whichThing.Displacement.Z = ZPos;

      move_displacement_xyz(intPosition, whichThing.Displacement, 1);

      int8_t collideResult = map_colide_direction(whichThing.Position, intPosition);

      isNoCollision = (collideResult == 0);
      Flag1Set = ((collideResult & 1) != 0);

      if (!isNoCollision) {
          //there is a collision
          isNoCollision = !Flag1Set;
          Flag2Set = ((collideResult & 2) != 0);

          if (!isNoCollision) {
              irr::f32 v18 = 0.0f;
              if (whichThing.Displacement.X > 0.0f) {
                  v18 = (float)(1.0f / 256.0f);
              }

              v18 -= whichThing.Displacement.X;
              v18 = v18 * 0.5f;

              whichThing.Displacement.X = v18;
              intPosition.X = whichThing.Position.X + v18;
          }

           isNoCollision = !Flag2Set;
           Flag4Set = ((collideResult & 4) != 0);

           if (!isNoCollision) {
               irr::f32 v21 = 0.0f;
               if (whichThing.Displacement.Y > 0.0f) {
                   v21 = (float)(1.0f / 256.0f);
               }

               v21 -= whichThing.Displacement.Y;
               v21 = v21 * 0.5f;

               whichThing.Displacement.Y = v21;
               intPosition.Y = whichThing.Position.Y + v21;
           }

           if (Flag4Set) {
               irr::f32 v23 = (float)((-120.0f / 256.0f)) * whichThing.Displacement.Z;
               whichThing.Displacement.Z = v23;

               if (v23 < (float)(10.0f / 256.0f)) {
                   whichThing.Displacement.Z = 0.0f;
               }

              intPosition.Z = whichThing.Displacement.Z + map_floor(intPosition);
              move_displacement_slope(whichThing.Position, intDisplacement);

              irr::f32 v24 = -1.0f;
              if ((intDisplacement.X < -1.0f) ||
                      (v24 = 1.0f, intDisplacement.X >= (float)(257.0f / 256.0f))) {
                  intDisplacement.X = v24;
              }

              irr::f32 v25 = -1.0f;
              if ((intDisplacement.Y < -1.0f) ||
                      (v25 = 1.0f, intDisplacement.Y >= (float)(257.0f / 256.0f))) {
                  intDisplacement.Y = v25;
              }

              whichThing.Displacement.X += intDisplacement.X / 16.0f;
              whichThing.Displacement.Y += intDisplacement.Y / 16.0f;
           }
      }

      //TODO: ? mapwho_move(whichThing, position);
      whichThing.Position = intPosition;
       testNode->setPosition(VanillaToIrrlichtCoord(whichThing.Position));
      return 1;
   }

   testNode->setPosition(VanillaToIrrlichtCoord(whichThing.Position));

   return 0;
}

/***************************************************
 * Map Collide stuff                               *
 ***************************************************/

//Input position: Enter coordinates according to vanilla game coordinate system
//Returns the tile raw friction table value
uint16_t VCalculations::map_colide_type(irr::core::vector3df position) {
   //get integer index of cell
   irr::s32 intX = (irr::s32)(position.X / this->mLevelTerrain->segmentSize);
   irr::s32 intY = (irr::s32)(position.Y / this->mLevelTerrain->segmentSize);

    if ((intX >= (this->mLevelFile->Width() - 1)) ||
        (intY >= (this->mLevelFile->Height() - 1))) {
          return 0.0f;
    }

    if ((intX < 0) || (intY < 0)) {
        return 0.0f;
    }

    //get and return the raw friction table value
    return (mLevelFile->GetFrictionValue(intX, intY));
}

//Helper function for function verification
bool VCalculations::Verify_map_colide_type_step(int tileX, int tileY, uint16_t expResult,
            int16_t whichSeqCaseTested) {

    //construct a 3D position where we can find this tile
    //in level1
    irr::core::vector3df pos;
    pos.X = tileX * mLevelTerrain->segmentSize + 0.5f;
    pos.Y = tileY * mLevelTerrain->segmentSize + 0.5f;

    //map_colide_type takes in position in original game coordinate
    //system!
    uint16_t result = map_colide_type(pos);

    if (expResult != result) {
        std::string infoTxt("Verify_map_colide_type_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " Result Difference: ";
        infoTxt += std::to_string((int)(result - expResult));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    return true;
}

//Returns true if map_colide_type works as expected, False
//otherwise
//This function can only work if the unmodified original level1 is loaded
//as the result is based on the loaded level file
bool VCalculations::Verify_map_colide_type() {
    bool overallResult = true;

    overallResult &= Verify_map_colide_type_step(25, 102, 0x1C01, 1); //checks for TextureId 0 (Dirt)
    overallResult &= Verify_map_colide_type_step(12, 92,  0x0804, 2);  //checks for road Tile (Texture Id 144)

    if (overallResult) {
        logging::Info("Verify_map_colide_type: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_map_colide_type: At least one testcase failed");
    }

    return false;
}

//Input position: Enter coordinates according to vanilla game coordinate system
//Returns the tile friction value at the specified location
uint16_t VCalculations::map_colide_friction(irr::core::vector3df position) {
    uint16_t rawVal = map_colide_type(position);

    //get and return the friction value of the current loaded
    //map
    return ((rawVal & 0x3C00) >> 10);
}

//Helper function for function verification
bool VCalculations::Verify_map_colide_friction_step(int tileX, int tileY, uint16_t expResult,
            int16_t whichSeqCaseTested) {

    //construct a 3D position where we can find this tile
    //in level1
    irr::core::vector3df pos;
    pos.X = tileX * mLevelTerrain->segmentSize + 0.5f;
    pos.Y = tileY * mLevelTerrain->segmentSize + 0.5f;

    uint16_t result = map_colide_friction(pos);

    if (expResult != result) {
        std::string infoTxt("Verify_map_colide_friction_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " Result Difference: ";
        infoTxt += std::to_string((int)(result - expResult));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    return true;
}

//Returns true if map_colide_friction works as expected, False
//otherwise
//This function can only work if the unmodified original level1 is loaded
//as the result is based on the loaded level file
bool VCalculations::Verify_map_colide_friction() {
    bool overallResult = true;

    overallResult &= Verify_map_colide_friction_step(0x0B, 0x5C, 0x02, 1); //checks for TextureId 144 (road tile)

    if (overallResult) {
        logging::Info("Verify_map_colide_friction: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_map_colide_friction: At least one testcase failed");
    }

    return false;
}

//Function programmed to have similar behavior as function "collide_map"
//in original game. But instead of using fixed point arithmetic I am using
//floating point calculations.
//Input posX & posY: Enter coordinates according to vanilla game coordinate system
//Returns Z-coordinate in original game coordinate system
irr::f32 VCalculations::collide_map(irr::f32 posX, irr::f32 posY)
{
    //get integer index of cell
    irr::s32 intX = (irr::s32)(posX / this->mLevelTerrain->segmentSize);
    irr::s32 intY = (irr::s32)(posY / this->mLevelTerrain->segmentSize);

    irr::f32 fracX = posX - (irr::f32)(intX);
    irr::f32 fracY = posY - (irr::f32)(intY);

    if ((intX >= (this->mLevelFile->Width() - 1)) ||
        (intY >= (this->mLevelFile->Height() - 1))) {
          return 0.0f;
    }

    if ((intX < 0) || (intY < 0)) {
        return 0.0f;
    }

    //get pntr to this tile
    TerrainTileData *pntr = &mLevelTerrain->pTerrainTiles[intX][intY];

    if (pntr == nullptr) {
        return (0.0f);
    }

    int16_t idx = intX + intY;
    irr::s32 rem = (idx % 2);

    irr::f32 zRes;
    irr::f32 slopeX;
    irr::f32 slopeZ;

    //Is the specified position in a tile with an even X and Y coordinate?
    if (rem == 0) {
       if (fracY >= fracX) {
           //Sequence case 1 (Helpful for unit test and debugging)
           slopeX = -pntr->vert3CurrPositionY + pntr->vert4CurrPositionY;
           slopeZ = -pntr->vert4CurrPositionY + pntr->vert1CurrPositionY;

           zRes = -pntr->vert1CurrPositionY + slopeX * fracX + slopeZ * fracY;
           return zRes;
       }

       //Sequence case 2 (Helpful for unit test and debugging)
       slopeX = -pntr->vert2CurrPositionY + pntr->vert1CurrPositionY;
       slopeZ = -pntr->vert3CurrPositionY + pntr->vert2CurrPositionY;

       zRes = -pntr->vert1CurrPositionY + slopeX * fracX + slopeZ * fracY;
       return zRes;
    }

    if ((fracX + fracY) >= 1.0f) {
        //Sequence case 3 (Helpful for unit test and debugging)
        slopeX = -pntr->vert3CurrPositionY + pntr->vert4CurrPositionY;
        slopeZ = -pntr->vert2CurrPositionY + pntr->vert3CurrPositionY;

        zRes = -pntr->vert4CurrPositionY + slopeX * fracX + slopeZ * (1.0f - fracY);
        return zRes;
    }

    //Sequence case 4 (Helpful for unit test and debugging)
    slopeX = -pntr->vert2CurrPositionY + pntr->vert1CurrPositionY;
    slopeZ = -pntr->vert4CurrPositionY + pntr->vert1CurrPositionY;

    zRes = -pntr->vert1CurrPositionY + slopeX * fracX + slopeZ * fracY;
    return zRes;
}

//Helper function to test one specific case for Verify_collide_map function
bool VCalculations::Verify_collide_map_step(int16_t posXFixedPnt, int16_t posYFixedPnt, int16_t expZResultFixedPnt, int16_t whichSeqCaseTested) {
    irr::f32 posX = FixedPointToFloat8D8(posXFixedPnt);
    irr::f32 posY = FixedPointToFloat8D8(posYFixedPnt);

    irr::f32 posZRes = collide_map(posX, posY);
    int16_t posZResInt = FloatToFixedPoint8D8(posZRes);

    int16_t errorInt = abs(posZResInt - expZResultFixedPnt);

    //allow maximum 1 count deviation in terms of fixed point
    //fractional part between my floating point calculation
    //and the expected result from the original game (using fixed point calculation)
    if (errorInt > 1) {
        std::string infoTxt("Verify_collide_map_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " Difference: ";
        infoTxt += std::to_string((int)(expZResultFixedPnt - posZResInt));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    return true;
}

//Returns true of verify collide map works as expected, False
//otherwise
//Important: Only works correctly if unmodified (original) level1
//file is loaded
bool VCalculations::Verify_collide_map() {
    bool overallResult = true;

    //Sequence case 1 Testcases
    overallResult &= Verify_collide_map_step(0x133A, 0x6F7C, 0x0C7D, 1);
    overallResult &= Verify_collide_map_step(0x136B, 0x6FB2, 0x0CA5, 1);
    overallResult &= Verify_collide_map_step(0x1308, 0x6FDA, 0x0CC3, 1);

    //Sequence case 2 Testcases
    overallResult &= Verify_collide_map_step(0x139E, 0x6F46, 0x0C54, 2);
    overallResult &= Verify_collide_map_step(0x1352, 0x6F33, 0x0C46, 2);
    overallResult &= Verify_collide_map_step(0x139D, 0x6F3A, 0x0C4B, 2);

    //Sequence case 3 Testcases
    overallResult &= Verify_collide_map_step(0x0B80, 0x7280, 0x0EC0, 3);
    overallResult &= Verify_collide_map_step(0x139D, 0x6E6A, 0x0BB9, 3);
    overallResult &= Verify_collide_map_step(0x136A, 0x6EC9, 0x0BFA, 3);

    //Sequence case 4 Testcases
    overallResult &= Verify_collide_map_step(0x1352, 0x6E8A, 0x0BCE, 4);
    overallResult &= Verify_collide_map_step(0x1352, 0x6E70, 0x0BBD, 4);
    overallResult &= Verify_collide_map_step(0x1352, 0x6E63, 0x0BB4, 4);

    if (overallResult) {
        logging::Info("Verify_collide_map: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_collide_map: At least one testcase failed");
    }

    return false;
}

//Function programmed to have similar behavior as function "map_floor"
//in original game.
//Input position: Enter coordinates according to vanilla game coordinate system
//Returns Z-coordinate in original games coordinate system
irr::f32 VCalculations::map_floor(irr::core::vector3df position)
{
    //Height of the tile below us
    irr::f32 result = collide_map(position.X, position.Y);

    return result;
}

irr::f32 VCalculations::map_altitude_column_and_floor(irr::core::vector3df position)
{
    //get height of the tile below us
    irr::f32 result = collide_map(position.X, position.Y);

    vector2di cell((irr::s32)(position.X / this->mLevelTerrain->segmentSize),
                   (irr::s32)(position.Y / this->mLevelTerrain->segmentSize));

    mLevelTerrain->ForceTileGridCoordRange(cell);

    //is the floor height below us lower then our own height above ground?
    //if so we could currently drive on a column (cube)
    if (result < position.Z) {
       //if there is no column then just return result
       MapEntry* entry = mLevelFile->pMap[cell.X][cell.Y];

       if (entry->get_Column() == nullptr) {
           return result;
       }

       irr::f32 diff = position.Z - result;
       irr::s32 diffInt = (irr::s32)(diff);

       result = result + (irr::f32)(diffInt);

       //is there at least one block height difference
       //if not there can be no block below us
       if (diffInt > 0) {
           //yes, there seems to be at least one block of
           //a column below us
           ColumnDefinition* colDef = entry->get_Column();

           int blockIdx = (int)(diffInt) - 1;
           int16_t blockVal;

           while (1) {
               switch (blockIdx) {
                   case 0: {
                     blockVal = colDef->get_A();
                     break;
                   }

                   case 1: {
                     blockVal = colDef->get_B();
                     break;
                   }

                   case 2: {
                     blockVal = colDef->get_C();
                     break;
                   }

                   case 3: {
                     blockVal = colDef->get_D();
                     break;
                   }

                   case 4: {
                     blockVal = colDef->get_E();
                     break;
                   }

                   case 5: {
                     blockVal = colDef->get_F();
                     break;
                   }

                   case 6: {
                     blockVal = colDef->get_G();
                     break;
                   }

                   case 7: {
                     blockVal = colDef->get_H();
                     break;
                   }

                   default: {
                       blockVal = 0;
                       break;
                   }
               }

               blockIdx--;

               //does this block exist?
               //if so exit, craft must be on
               //top of this block/cube
               if (blockVal != 0) {
                   break;
               }

               result -= mLevelTerrain->segmentSize;

               if (blockIdx < 0) {
                   break;
               }
           }
       }
    }

    return result;
}

//This function was written to behave similar to the function
//map_colide in the original game
//Does determine if a 3D input position is in free space, or if it does
//collide with terrain or a column block
//Returns 0 for no collision (free space)
//Returns 8 for position below terrain height
//Returns 16 for collision with existing column block
int8_t VCalculations::map_colide(irr::core::vector3df position)
{
    int8_t result = 16;

    //If Y coordinate is outside allowed map area return
    //always a collision
    if (position.Y >= (float(mLevelTerrain->get_heigth()) - 2.0f))
        return result;

    irr::f32 distanceFromTerrain = position.Z - map_floor(position);
    irr::s32 diffInt = (irr::s32)(distanceFromTerrain);

    if (distanceFromTerrain >= 0.0f) {
        //no collision, above terrain
        result = 0;
    } else {
        //collision, below terrain
        result = 8;
        diffInt = 0;
    }

    //if we are more then 8.0f away from the terrain, then there
    //can not be a column anymore we have to verify for
    if (diffInt >= 8) {
        return result;
    }

    //Is the a column present?
    //what cell are we in?
    vector2di cell((irr::s32)(position.X / this->mLevelTerrain->segmentSize),
                   (irr::s32)(position.Y / this->mLevelTerrain->segmentSize));

    mLevelTerrain->ForceTileGridCoordRange(cell);

    MapEntry* entry = mLevelFile->pMap[cell.X][cell.Y];

    if ((entry->get_Column() != nullptr) && (diffInt >= 0)) {
        uint8_t shape = entry->get_Column()->get_Shape();

        //in shape most significant bit is highest block in the column (Block H)
        //LSB is lowest block at terrain level (Block A)
        for (irr::s32 idx = 0; idx < diffInt; idx++) {
            shape = shape >> 1;
        }

        if ((shape & 1) != 0) {
           //there is a solid block there, we collide
           return (result | 0x10);
        }
    }

    return result;
}

//This function was written to behave similar to the function
//map_colide_direction in the original game
//Returns 0 if no collision occurs
//Depending on which coordinate axis the collision occurs
//sets flags 1, 2, 4
int8_t VCalculations::map_colide_direction(irr::core::vector3df oldPosition, irr::core::vector3df newPosition)
{
    //does we collide at the new position?
    //Returns 0 if we do not collide
    int result = map_colide(newPosition);

    if (!result) {
        //we do not colide at new position
        //no problem, immediately return
        return 0;
    }

    //find out which coordinate change is the issue
    irr::core::vector3df position = oldPosition;
    position.Z = newPosition.Z;
    if (map_colide(position)) {
        result |= 4;
    }

    position = oldPosition;
    position.X = newPosition.X;
    if ((map_colide(position) & 0x10) != 0) {
         result |= 1;
    }

    position = oldPosition;
    position.Y = newPosition.Y;
    if ((map_colide(position) & 0x10) != 0) {
         result |= 2;
    }

    return result;
}

//This function was written to behave similar to the function
//map_colide_direction_xy in the original game
int8_t VCalculations::map_colide_direction_xy(irr::core::vector3df oldPosition, irr::core::vector3df newPosition)
{
    irr::core::vector3df position;

    int8_t result = map_colide(newPosition);
    if (!result) {
        return 0;
    }

    position.X = newPosition.X;
    position.Y = oldPosition.Y;
    position.Z = oldPosition.Z;

    if ((map_colide(position) & 0x10) != 0) {
        result |= 1u;
    }

    position.X = oldPosition.X;
    position.Y = newPosition.Y;
    position.Z = oldPosition.Z;

    if ((map_colide(position) & 0x10) != 0) {
        result |= 2u;
    }

    return (result & 0xF7);
}

/***************************************************
 * Movement stuff                                  *
 ***************************************************/

//This function was written to behave similar to the function
//move_displacement_xyz in the original game
//moves a 3D position by a certain displacement, direction
//is also defined by the sign
void VCalculations::move_displacement_xyz(irr::core::vector3df& position,
                                          irr::core::vector3df displacement,
                                          int8_t sign)
{
    if (sign > 0) {
        position += displacement;
        return;
    }

    if (sign < 0) {
        position -= displacement;
    }
}

//This function was written to behave similar to the function
//move_displacement_slope in the original game
//Calculates and returns the displacement for a terrain cell at the specified
//3D position
void VCalculations::move_displacement_slope(irr::core::vector3df position,
                                            irr::core::vector3df& displacement)
{
    //what cell are we in?
    vector2di cell((irr::s32)(position.X / this->mLevelTerrain->segmentSize),
                   (irr::s32)(position.Y / this->mLevelTerrain->segmentSize));

    mLevelTerrain->ForceTileGridCoordRange(cell);

    //get pntr to this tile
    TerrainTileData *pntr = &mLevelTerrain->pTerrainTiles[cell.X][cell.Y];

    if (pntr != nullptr) {
        //we need to swap sign of all the current vertice Y coordinates (height)
        //we get from Irrlicht
        displacement.X = -pntr->vert1CurrPositionY;
        displacement.Y = -pntr->vert1CurrPositionY;

        displacement.X += pntr->vert2CurrPositionY;
        displacement.Y -= pntr->vert2CurrPositionY;

        displacement.X += pntr->vert3CurrPositionY;
        displacement.Y += pntr->vert3CurrPositionY;

        displacement.X -= pntr->vert4CurrPositionY;
        displacement.Y += pntr->vert4CurrPositionY;
    }
}

//In original game: angleXY and angleZY are a 16 bit integer, with range from 0 up to 0xFFFF
//Angles have 8.8 Fixed Point Format; contain the absolute angle on a unit circle with 256 steps for a whole circle;
//In the original game this integer value is then divided further by a factor of 32 (shift 5 bits to the right)
//to get the index into the original sine table
//Speed is a fixed point number with 8.8 format;
//In my project: angleXY and angleZY is the absolute angle in deg, range 0.0f up to 360.0f
//speed has a possible range from 0.0 up to 255.99609375, resolution in original game
//is 0.00390625 for speed
int8_t VCalculations::move_displacement_set(irr::core::vector3df& position, irr::f32 angleXY,
                             irr::f32 angleZY, irr::f32 speed) {   //18.04.2026: Function verified to be correct

    irr::f32 v4 = speed;

    //Displacement = Change in position from one iteration to the next => first derivation of position

    //1: Very small speed => no displacement
    //2: Very steep movement upwards (angleZY <<) => no upwards/downward movement anymore in Z-direction

    //if smaller then speed resolution in original game
    //then speed is basically zero
    if (fabs(speed) < 0.00390625f) {
        //almost no speed, no displacement anymore => stop movement of object
        position.set(0.0f, 0.0f, 0.0f);
        return 0;
    } else {
        if (!(fabs(angleZY) < 0.175784)) {
            irr::f32 angle = (angleZY / 180.0f) * M_PI;
            //position.Z (displacement in Z-direction) is the first derivation, therefore we get -sin here
            position.Z = -speed * sin(angle);
            //displacement vector component (first derivation) in XY-Plane, initial sin becomes cos because
            //of derivation => checks out
            v4 = speed * cos(angle);
        } else {
            //object moves very steep upwards to sky, set
            //zero displacement for Z-axis
            position.Z = 0.0f;
        }

        //now further seperate components in XY-Plane
        //angleXY = angle starting from X-Axis going towards Y-Axis
        irr::f32 angle2 = (angleXY / 180.0f) * M_PI;
        position.X = v4 * sin(angle2);
        position.Y = -v4 * cos(angle2);

        return 1;
    }
}

//Helper function to test one specific case for Verify_collide_map function
bool VCalculations::Verify_move_displacement_set_step(int16_t expXPos, int16_t expYPos, int16_t expZPos, bool verifyZPos,
           int16_t inputAngleXY, int16_t inputAngleZY, int16_t inputSpeed,
            int16_t whichSeqCaseTested) {
    irr::f32 inputAngleXYFloating = VanillaRawAngleToMyFloatingAngle(inputAngleXY);
    irr::f32 inputAngleZYFloating = VanillaRawAngleToMyFloatingAngle(inputAngleZY);
    irr::f32 inputSpeedFloating = FixedPointToFloat8D8(inputSpeed);

    irr::core::vector3df outputPos;

    move_displacement_set(outputPos, inputAngleXYFloating, inputAngleZYFloating, inputSpeedFloating);

    int32_t posXReceived = FloatToFixedPoint8D8(outputPos.X);
    int32_t posYReceived = FloatToFixedPoint8D8(outputPos.Y);
    int32_t posZReceived = FloatToFixedPoint8D8(outputPos.Z);

    int32_t errorIntPosX = abs(posXReceived - expXPos);
    int32_t errorIntPosY = abs(posYReceived - expYPos);
    int32_t errorIntPosZ = abs(posZReceived - expZPos);

    //allow maximum 1 count deviation in terms of fixed point
    if (errorIntPosX > 1) {
        std::string infoTxt("Verify_move_displacement_set_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " XPos Difference: ";
        infoTxt += std::to_string((int)(expXPos - posXReceived));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    if (errorIntPosY > 1) {
        std::string infoTxt("Verify_move_displacement_set_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " YPos Difference: ";
        infoTxt += std::to_string((int)(expYPos - posYReceived));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    //Not for all debugging data in Playstation Emulator (gathered by Breakpoint and Stepping)
    //I have expected ZPos values available, For the ones where I do not have this values do
    //not verify result here
    if (verifyZPos & (errorIntPosZ > 1)) {
        std::string infoTxt("Verify_move_displacement_set_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " ZPos Difference: ";
        infoTxt += std::to_string((int)(expZPos - posZReceived));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    return true;
}

//Returns true of verify move_displacement_set works as expected, False
//otherwise
bool VCalculations::Verify_move_displacement_set() {
    bool overallResult = true;

    overallResult &= Verify_move_displacement_set_step(0x00FE, 0x0007, 0x0000, true, 0x4128, 0xFFFD, 0x100, 1);
    overallResult &= Verify_move_displacement_set_step(0xFF48, 0x00AF, 0x0000, true, 0xA114, 0xFFFD, 0x100, 2);
    overallResult &= Verify_move_displacement_set_step(0xFF43, 0xFF56, 0x0000, true, 0xDDE6, 0xFFFD, 0x100, 3);
    overallResult &= Verify_move_displacement_set_step(0x00AC, 0xFF45, 0x0000, true, 0x1E6D, 0xFFFD, 0x100, 4);
    overallResult &= Verify_move_displacement_set_step(0xFFE5, 0xFF0F, 0x0050, true, 0xFB6A, 0xF303, 0x100, 5);
    overallResult &= Verify_move_displacement_set_step(0x0011, 0x00E9, 0xFF99, true, 0x7CF3, 0x1119, 0x100, 6);

    overallResult &= Verify_move_displacement_set_step(0x06, 0xFFFC, 0x0000, false, 0x26C6, 0xC42A, 0x57, 7);
    overallResult &= Verify_move_displacement_set_step(0xFFF9, 0x0014, 0x0000, false, 0x8E2C, 0xC892, 0x6D, 8);
    overallResult &= Verify_move_displacement_set_step(0xFFD7, 0xFFE7, 0x0000, false, 0xD686, 0xD1BF, 0x77, 9);

    if (overallResult) {
        logging::Info("Verify_move_displacement_set: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_move_displacement_set: At least one testcase failed");
    }

    return false;
}

//In original game: angleXY and angleZY are a 16 bit integer, with range from 0 up to 0xFFFF
//Angles have 8.8 Fixed Point Format; contain the absolute angle on a unit circle with 256 steps for a whole circle;
//In the original game this integer value is then divided further by a factor of 32 (shift 5 bits to the right)
//to get the index into the original sine table
//Speed is a fixed point number with 8.8 format;
//In my project: angleXY and angleZY is the absolute angle in deg, range 0.0f up to 360.0f
//speed has a possible range from 0.0 up to 255.99609375, resolution in original game
//is 0.00390625 for speed
int8_t VCalculations::move_xyz(irr::core::vector3df& position, irr::f32 angleXY,
                             irr::f32 angleZY, irr::f32 speed) {

    irr::f32 v4 = speed;

    //if speed is way too low we do not need to move, just exit
    if (fabs(speed) < 0.00390625f) {
        return 0;
    }

    if (!(fabs(angleZY) < 0.175784)) {
            irr::f32 angle = (angleZY / 180.0f) * M_PI;
            position.Z -= speed * sin(angle);
            v4 = speed * cos(angle);
    }

    irr::f32 angle2 = (angleXY / 180.0f) * M_PI;
    position.X += v4 * sin(angle2);
    position.Y -= v4 * cos(angle2);

    return 1;
}

//Helper function for function verification
bool VCalculations::Verify_move_xyz_step(int16_t startXPos, int16_t startYPos, int16_t startZPos,
                                    int16_t endXPos, int16_t endYPos, int16_t endZPos,
           int16_t inputAngleXY, int16_t inputAngleZY, int16_t inputSpeed,
            int16_t whichSeqCaseTested) {
    irr::f32 inputAngleXYFloating = VanillaRawAngleToMyFloatingAngle(inputAngleXY);
    irr::f32 inputAngleZYFloating = VanillaRawAngleToMyFloatingAngle(inputAngleZY);
    irr::f32 inputSpeedFloating = FixedPointToFloat8D8(inputSpeed);

    irr::core::vector3df pos;
    pos.X = FixedPointToFloat8D8(startXPos);
    pos.Y = FixedPointToFloat8D8(startYPos);
    pos.Z = FixedPointToFloat8D8(startZPos);

    move_xyz(pos, inputAngleXYFloating, inputAngleZYFloating, inputSpeedFloating);

    int32_t posXResult = FloatToFixedPoint8D8(pos.X);
    int32_t posYResult = FloatToFixedPoint8D8(pos.Y);
    int32_t posZResult = FloatToFixedPoint8D8(pos.Z);

    int32_t errorIntPosX = abs(posXResult - endXPos);
    int32_t errorIntPosY = abs(posYResult - endYPos);
    int32_t errorIntPosZ = abs(posZResult - endZPos);

    //allow maximum 2 count deviation in terms of fixed point
    if (errorIntPosX > 2) {
        std::string infoTxt("Verify_move_xyz_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " XPos Difference: ";
        infoTxt += std::to_string((int)(endXPos - posXResult));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    if (errorIntPosY > 2) {
        std::string infoTxt("Verify_move_xyz_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " YPos Difference: ";
        infoTxt += std::to_string((int)(endYPos - posYResult));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    if (errorIntPosZ > 2) {
        std::string infoTxt("Verify_move_xyz_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " ZPos Difference: ";
        infoTxt += std::to_string((int)(endZPos - posZResult));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    return true;
}

//Returns true of verify move_xyz works as expected, False
//otherwise
bool VCalculations::Verify_move_xyz() {
    bool overallResult = true;

    overallResult &= Verify_move_xyz_step(0x09BC, 0x7544, 0x0FE0, 0x0944, 0x7544, 0x0FE0, 0xC000, 0x0, 0x0078, 1);
    overallResult &= Verify_move_xyz_step(0x1170, 0x6883, 0x092C, 0x11AA, 0x6880, 0x092B, 0x3D83, 0x00CA, 0x003C, 2);
    overallResult &= Verify_move_xyz_step(0x1208, 0x686B, 0x0925, 0x1202, 0x67FA, 0x0903, 0xFD82, 0x0BEC, 0x0078, 3);
    overallResult &= Verify_move_xyz_step(0x1260, 0x67E4, 0x08FC, 0x11EA, 0x67DE, 0x08FE, 0xC27E, 0xFF2A, 0x0078, 4);
    overallResult &= Verify_move_xyz_step(0x12EE, 0x67F9, 0x089E, 0x12EE, 0x67F9, 0x083A, 0xB67B, 0x4000, 0x0064, 5);
    overallResult &= Verify_move_xyz_step(0x12EE, 0x67F9, 0x089E, 0x12EE, 0x67F9, 0x083A, 0x7FBE, 0x4000, 0x0064, 6);

    if (overallResult) {
        logging::Info("Verify_move_xyz: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_move_xyz: At least one testcase failed");
    }

    return false;
}

/***************************************************
 * Distance calculation stuff                      *
 ***************************************************/

irr::f32 VCalculations::distance_get_xyz(irr::core::vector3df position1, irr::core::vector3df position2) {
    //simply let Irrlicht do it :)
    irr::core::vector3df irrCoordSysCoord1 = VanillaToIrrlichtCoord(position1);
    irr::core::vector3df irrCoordSysCoord2 = VanillaToIrrlichtCoord(position2);
    return ((irrCoordSysCoord2-irrCoordSysCoord1).getLength());
}

/***************************************************
 * Angle calculation stuff                         *
 ***************************************************/

//Careful: This function returns the angle in degress for a 360° unit circle
//The original game uses inside a 256° (step) unit circle!
irr::f32 VCalculations::arctanPlusMultiply32(irr::f32 x, irr::f32 y) {
    irr::f32 result = atan2(x, y);

    //convert result from RAD to deg
    result = (result / irr::core::PI) * 180.0f;

    //I compared the results from the "normal" atan2 function
    //with the results from the arctan function in the original game
    //for some iterations (stepping with emulator), and saw that I have
    //to additionally to do the follow to make the results fit
    if (result >= 0.0) {
        result = (180.0f - result);
    } else {
        result = (-180.0f - result);
    }

    return result;
}

//Helper function for function verification
bool VCalculations::Verify_arctanPlusMultiply32(int16_t xVal, int16_t yVal, int16_t expResult,
            int16_t whichSeqCaseTested) {
    irr::f32 inputValX = FixedPointToFloat8D8(xVal);
    irr::f32 inputValY = FixedPointToFloat8D8(yVal);

    //Careful: This function returns the angle in degress for a 360° unit circle
    //The original game uses inside a 256° (step) unit circle!
    irr::f32 angleResultFloating = arctanPlusMultiply32(inputValX, inputValY);
    angleResultFloating = (angleResultFloating / 360.0f) * 256.0f;

    int16_t angleResultFixed256UnitCircle = FloatToFixedPoint8D8(angleResultFloating);

    int32_t errorAngle = abs(expResult - angleResultFixed256UnitCircle);

    //allow maximum 0.1° error between my solution and the original implementation
    if (errorAngle > 18) {
        std::string infoTxt("Verify_arctanPlusMultiply32: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " Angle Difference: ";
        infoTxt += std::to_string((int)(expResult - angleResultFixed256UnitCircle));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    return true;
}

//Returns true of verify arctanPlusMultiply32 works as expected, False
//otherwise
bool VCalculations::Verify_arctanPlusMultiply32() {
    bool overallResult = true;

    //Test data captured in original game using emulator and debugger
    overallResult &= Verify_arctanPlusMultiply32(0x1200, 0x0000, 0x4000, 1);
    overallResult &= Verify_arctanPlusMultiply32(0x200, 0x100, 0x52E0, 2);
    overallResult &= Verify_arctanPlusMultiply32(0xFD00, 0x600, 0x92E0, 3);
    overallResult &= Verify_arctanPlusMultiply32(0xFA00, 0x500, 0xA3C0, 4);
    overallResult &= Verify_arctanPlusMultiply32(0xFE00, 0xFC00, 0xED20, 5);
    overallResult &= Verify_arctanPlusMultiply32(0x100, 0xF800, 0x500, 6);

    if (overallResult) {
        logging::Info("Verify_arctanPlusMultiply32: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_arctanPlusMultiply32: At least one testcase failed");
    }

    return false;
}

//Careful: This function returns the angle in degress for a 360° unit circle
//The original game uses inside a 256° (step) unit circle!
irr::f32 VCalculations::angle_get_xy(irr::core::vector3df position_from, irr::core::vector3df position_to) {
    //Note for me: The multiplication with 32 in the original game in this function
    //is actually part of the result calculation in arctan function of original game
    //itself; I moved it inside arctanPlusMultiply32, and so we are not allowed to
    //accidently apply it again!
    return arctanPlusMultiply32(position_to.X - position_from.X,
                                position_to.Y - position_from.Y);
}

/***************************************************
 * Conversion Routines                             *
 ***************************************************/

//Convert a float into a fixed point number (8.8 format)
int16_t VCalculations::FloatToFixedPoint8D8(irr::f32 floatVal) {
    int16_t result = (int16_t)(floatVal * 256.0f);
    return result;
}

//Convert a fixed point number (8.8 format) to a float
irr::f32 VCalculations::FixedPointToFloat8D8(int16_t fixedPntVal) {
    int8_t intPart = static_cast<int8_t>((fixedPntVal >> 8) & 0x00FF);
    float result = (float)(intPart);
    result += (fixedPntVal & 0x00FF) / 256.0f;
    return result;
}

//Convert a float into a fixed point number (16.16 format)
int32_t VCalculations::FloatToFixedPoint16D16(irr::f32 floatVal) {
    int32_t result = (int32_t)(floatVal * 65536.0f);
    return result;
}

//Convert a fixed point number (16.16 format) to a float
irr::f32 VCalculations::FixedPointToFloat16D16(int32_t fixedPntVal) {
    int16_t intPart = static_cast<int16_t>((fixedPntVal >> 16) & 0x0000FFFF);
    float result = (float)(intPart);
    result += (fixedPntVal & 0x0000FFFF) / 65536.0f;
    return result;
}

//original "raw" angles (for example in thing->Movement.Angle.ZY, is struct Angle)
//contain the absolute angle for a unit circle of 256.0, in fixed point
//arithmetic format of 8.8
//this function simply converts this angle into a floating point angle value
//with a unit circle of 360.0f I use in my project and Irrlicht
irr::f32 VCalculations::VanillaRawAngleToMyFloatingAngle(uint16_t rawVanillaAngle) {
   uint16_t val = ((uint16_t)(rawVanillaAngle) & 0xFF00) >> 8;
   irr::f32 result = (irr::f32)(val);

   val = ((uint16_t)(rawVanillaAngle) & 0x00FF);
   result += (irr::f32)(val) / 256.0f;

   result = (result / 256.0f) * 360.0f;

   if (result > 360.0f) {
       result -= 360.0f;
   }

   if (result < 0.0f) {
       result += 360.0f;
   }

   return result;
}

/***************************************************
 * Overall validation and verification             *
 ***************************************************/

//Returns true if all vanilla calculations work as expected, False otherwise
//Important: Only works correctly if unmodified (original) level1
//file is loaded
bool VCalculations::Verify_vanilla_calculations() {
    bool overallResult = true;

    //Verify_collide_map
    overallResult &= Verify_collide_map();

    //Verify_move_displacement_set
    overallResult &= Verify_move_displacement_set();

    //Verify_move_xyz
    overallResult &= Verify_move_xyz();

    //Verify map_colide_type
    overallResult &= Verify_map_colide_type();

    //Verify map_colide_friction
    overallResult &= Verify_map_colide_friction();

    //Verify arctanPlusMultiply32
    overallResult &= Verify_arctanPlusMultiply32();

    if (overallResult) {
        logging::Info("Verify_vanilla_calculations: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_vanilla_calculations: At least one testcase failed");
    }

    return false;
}


