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
//completely different to the one in the original game. The original uses X and Y axis for the tile map, and Z is the height. I have
//X and Z for the tile map, and Y is the height. And to make things worse my Irrlicht vertices Y coordinates have a swapped sign (are negative)
//currently. I will need to find a way to either adjust the source code below without introducing new bugs, or to change my project
//to use the same coordinate system soon.

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

VCalculations::VCalculations(InfrastructureBase* infra, LevelFile* levelFile, LevelTerrain* levelTerrain, LevelBlocks* levelBlocks)
{
    mInfra = infra;
    mLevelFile = levelFile;
    mLevelTerrain = levelTerrain;
    mLevelBlocks = levelBlocks;

    testTex = mInfra->mDriver->getTexture("extract/sprites/tmaps0029.png");
}

void VCalculations::AddTestObject(irr::core::vector3df position) {
    this->testThing1 = new TestThing();
    testThing1->Position = position;
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

    this->testNode->setPosition(testThing1->Position);
    this->testNode->setSize(irr::core::dimension2d<irr::f32>(0.45f, 0.45f));

    //get bounding box for this collectible
    this->testNode->updateAbsolutePosition();
   // this->boundingBox = this->billSceneNode->getTransformedBoundingBox();
}

void VCalculations::Update() {
    UpdateTestObject(0.1f, *testThing1);
}

void VCalculations::DebugDrawDisplacement(TestThing* whichThing) {
    irr::core::vector3df dirVec = whichThing->Displacement;
    dirVec.normalize();

    mInfra->mDrawDebug->Draw3DLine(whichThing->Position, whichThing->Position + dirVec * irr::core::vector3df(1.0f, 1.0f, 1.0f), mInfra->mDrawDebug->orange);
}

void VCalculations::DebugDraw() {
    DebugDrawDisplacement(testThing1);
}

//Convert a float into a fixed point number (8.8 format)
int16_t VCalculations::FloatToFixedPoint8D8(irr::f32 floatVal) {
    int16_t result = (int16_t)(floatVal) << 8;
    result &= 0xFF00;

    irr::f32 remainder = floatVal - (irr::f32)(result);
    int16_t remainderInt = (int16_t)(remainder * 256.0f);

    result |= remainderInt;
    return result;
}

//Convert a fixed point number (8.8 format) to a float
irr::f32 VCalculations::FixedPointToFloat8D8(int16_t fixedPntVal) {
    irr::f32 result = (irr::f32)((fixedPntVal >> 8) & 0x00FF);
    int16_t remainderInt = (int16_t)(fixedPntVal & 0x00FF);

    result += (irr::f32)(remainderInt / 256.0f);
    return result;
}

//Convert a float into a fixed point number (16.16 format)
int32_t VCalculations::FloatToFixedPoint16D16(irr::f32 floatVal) {
    int32_t result = (int32_t)(floatVal) << 16;
    result &= 0xFFFF0000;

    irr::f32 remainder = floatVal - (irr::f32)(result);
    int32_t remainderInt = (int32_t)(remainder * 65536.0f);

    result |= remainderInt;
    return result;
}

//Convert a fixed point number (16.16 format) to a float
irr::f32 VCalculations::FixedPointToFloat16D16(int32_t fixedPntVal) {
    irr::f32 result = (irr::f32)((fixedPntVal >> 16) & 0x0000FFFF);
    int32_t remainderInt = (int32_t)(fixedPntVal & 0x0000FFFF);

    result += (irr::f32)(remainderInt / 65536.0f);
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

//Helper function to test one specific case for Verify_collide_map function
bool VCalculations::Verify_collide_map_step(int16_t posXFixedPnt, int16_t posZFixedPnt, int16_t expYResultFixedPnt, int16_t whichSeqCaseTested) {
    irr::f32 posX = -FixedPointToFloat8D8(posXFixedPnt);
    irr::f32 posZ = FixedPointToFloat8D8(posZFixedPnt);

    irr::f32 posYRes = collide_map(posX, posZ);
    int16_t posYResInt = FloatToFixedPoint8D8(posYRes);

    int16_t errorInt = abs(posYResInt - expYResultFixedPnt);

    //allow maximum 1 count deviation in terms of fixed point
    //fractional part between my floating point calculation
    //and the expected result from the original game (using fixed point calculation)
    if (errorInt > 1) {
        std::string infoTxt("Verify_collide_map_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " Difference: ";
        infoTxt += std::to_string((int)(expYResultFixedPnt - posYResInt));
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

    if (overallResult) {
        logging::Info("Verify_vanilla_calculations: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_vanilla_calculations: At least one testcase failed");
    }

    return false;
}

int8_t VCalculations::UpdateTestObject(irr::f32 deltaTime, TestThing& whichThing) {
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

      ZPos = whichThing.Displacement.Z;
      irr::f32 v11 = ZPos + (float)(7.0f / 256.0f);
      if (v11 < 0.0f) {
          v11 += (float)(7.0f / 256.0f);
      }

      whichThing.Displacement.Z -= (v11 / 8.0f);

      YPos = whichThing.Displacement.Y;
      YPos -= (float)(8.0f / 256.0f);

      if (YPos < (float)(-100.0f / 256.0f)) {
          YPos = (float)(-100.0f / 256.0f);
      }

      whichThing.Displacement.Y = YPos;

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
               if (whichThing.Displacement.Z > 0.0f) {
                   v21 = (float)(1.0f / 256.0f);
               }

               v21 -= whichThing.Displacement.Z;
               v21 = v21 * 0.5f;

               whichThing.Displacement.Z = v21;
               intPosition.Z = whichThing.Position.Z + v21;
           }

           if (Flag4Set) {
               irr::f32 v23 = (float)((-120.0f / 256.0f)) * whichThing.Displacement.Y;
               whichThing.Displacement.Y = v23;

               if (v23 < (float)(10.0f / 256.0f)) {
                   whichThing.Displacement.Y = 0.0f;
               }

              intPosition.Y = whichThing.Displacement.Y + map_floor(intPosition);
              move_displacement_slope(whichThing.Position, intDisplacement);

              irr::f32 v24 = -1.0f;
              if ((intDisplacement.X < -1.0f) ||
                      (v24 = 1.0f, intDisplacement.X >= (float)(257.0f / 256.0f))) {
                  intDisplacement.X = v24;
              }

              irr::f32 v25 = -1.0f;
              if ((intDisplacement.Z < -1.0f) ||
                      (v25 = 1.0f, intDisplacement.Z >= (float)(257.0f / 256.0f))) {
                  intDisplacement.Z = v25;
              }

              whichThing.Displacement.X += intDisplacement.X / 16.0f;
              whichThing.Displacement.Z += intDisplacement.Z / 16.0f;
           }
      }

      //TODO: ? mapwho_move(whichThing, position);
      whichThing.Position = intPosition;
       testNode->setPosition(whichThing.Position);
      return 1;
   }

   testNode->setPosition(whichThing.Position);

   return 0;
}

//Input position: Enter coordinates according to my game drawing coordinate
//system (where X coordinates are mirrored at X-Axis, and posZ is my 2nd 2D coordinate-axis)
//Returns the tile raw friction table value
uint16_t VCalculations::map_colide_type(irr::core::vector3df position) {
   //get integer index of cell
   irr::s32 intX = (irr::s32)(-position.X / this->mLevelTerrain->segmentSize);
   irr::s32 intY = (irr::s32)(position.Z / this->mLevelTerrain->segmentSize);

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

//Input position: Enter coordinates according to my game drawing coordinate
//system (where X coordinates are mirrored at X-Axis, and posZ is my 2nd 2D coordinate-axis)
//Returns the tile friction value at the specified location
uint16_t VCalculations::map_colide_friction(irr::core::vector3df position) {
    uint16_t rawVal = map_colide_type(position);

    //get and return the friction value of the current loaded
    //map
    return ((rawVal & 0x3C00) >> 10);
}

//Function programmed to have similar behavior as function "collide_map"
//in original game. But instead of using fixed point arithmetic I am using
//floating point calculations.
//Input posX & posZ: Enter coordinates according to my game drawing coordinate
//system (where X coordinates are mirrored at X-Axis, and posZ is my 2nd 2D coordinate-axis)
//Returns my Y-coordinate in Irrlicht coordinate system
irr::f32 VCalculations::collide_map(irr::f32 posX, irr::f32 posZ)  //06.04.2026: Function verified to be correct
{
    //get integer index of cell
    irr::s32 intX = (irr::s32)(-posX / this->mLevelTerrain->segmentSize);
    irr::s32 intY = (irr::s32)(posZ / this->mLevelTerrain->segmentSize);

    irr::f32 fracX = -posX - (irr::f32)(intX);
    irr::f32 fracY = posZ - (irr::f32)(intY);

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

    irr::f32 yRes;
    irr::f32 slopeX;
    irr::f32 slopeZ;

    //Is the specified position in a tile with an even X and Y coordinate?
    if (rem == 0) {
       if (fracY >= fracX) {
           //Sequence case 1 (Helpful for unit test and debugging)
           slopeX = -pntr->vert3CurrPositionY + pntr->vert4CurrPositionY;
           slopeZ = -pntr->vert4CurrPositionY + pntr->vert1CurrPositionY;

           yRes = -pntr->vert1CurrPositionY + slopeX * fracX + slopeZ * fracY;
           return yRes;
       }

       //Sequence case 2 (Helpful for unit test and debugging)
       slopeX = -pntr->vert2CurrPositionY + pntr->vert1CurrPositionY;
       slopeZ = -pntr->vert3CurrPositionY + pntr->vert2CurrPositionY;

       yRes = -pntr->vert1CurrPositionY + slopeX * fracX + slopeZ * fracY;
       return yRes;
    }

    if ((fracX + fracY) >= 1.0f) {
        //Sequence case 3 (Helpful for unit test and debugging)
        slopeX = -pntr->vert3CurrPositionY + pntr->vert4CurrPositionY;
        slopeZ = -pntr->vert2CurrPositionY + pntr->vert3CurrPositionY;

        yRes = -pntr->vert4CurrPositionY + slopeX * fracX + slopeZ * (1.0f - fracY);
        return yRes;
    }

    //Sequence case 4 (Helpful for unit test and debugging)
    slopeX = -pntr->vert2CurrPositionY + pntr->vert1CurrPositionY;
    slopeZ = -pntr->vert4CurrPositionY + pntr->vert1CurrPositionY;

    yRes = -pntr->vert1CurrPositionY + slopeX * fracX + slopeZ * fracY;
    return yRes;
}

//Function programmed to have similar behavior as function "map_floor"
//in original game.
//Input position: Enter coordinates according to my game drawing coordinate
//system (where X coordinates are mirrored at X-Axis, and posZ is my 2nd 2D coordinate-axis)
//Returns my Y-coordinate in Irrlicht coordinate system
irr::f32 VCalculations::map_floor(irr::core::vector3df position)   //06.04.2026: Function verified to be correct
{
    //Height of the tile below us
    irr::f32 result = collide_map(position.X, position.Z);

    return result;
}

irr::f32 VCalculations::map_altitude_column_and_floor(irr::core::vector3df position)
{
    //get height of the tile below us
    irr::f32 result = collide_map(position.X, position.Z);

    vector2di cell((irr::s32)( -position.X / this->mLevelTerrain->segmentSize),
                   (irr::s32)(position.Z / this->mLevelTerrain->segmentSize));

    mLevelTerrain->ForceTileGridCoordRange(cell);

    //is the floor height below us lower then our own height above ground?
    //if so we could currently drive on a column (cube)
    if (result < position.Y) {
       //if there is no column then just return result
       MapEntry* entry = mLevelFile->pMap[cell.X][cell.Y];

       if (entry->get_Column() == nullptr) {
           return result;
       }

       irr::f32 diff = position.Y - result;
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

    //The original game seems to cap max object height, if object is in
    //height > 158.0f game returns collision with value 16
    if (position.Y >= 158.0f)
        return result;

    irr::f32 distanceFromTerrain = position.Y - map_floor(position);
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
    vector2di cell((irr::s32)( - position.X / this->mLevelTerrain->segmentSize),
                   (irr::s32)(position.Z / this->mLevelTerrain->segmentSize));

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
    position.Y = newPosition.Y;
    if (map_colide(position)) {
        result |= 4;
    }

    position = oldPosition;
    position.X = newPosition.X;
    if ((map_colide(position) & 0x10) != 0) {
         result |= 1;
    }

    position = oldPosition;
    position.Z = newPosition.Z;
    if ((map_colide(position) & 0x10) != 0) {
         result |= 2;
    }

    return result;
}

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
    vector2di cell((irr::s32)( - position.X / this->mLevelTerrain->segmentSize),
                   (irr::s32)(position.Z / this->mLevelTerrain->segmentSize));

    mLevelTerrain->ForceTileGridCoordRange(cell);

    //get pntr to this tile
    TerrainTileData *pntr = &mLevelTerrain->pTerrainTiles[cell.X][cell.Y];

    if (pntr != nullptr) {
        //we need to swap sign of all the current vertice coordinates
        //from Irrlicht
        //18.04.2026: not sure if the signs below are correct
        displacement.X = pntr->vert1CurrPositionY;
        displacement.Z = -pntr->vert1CurrPositionY;

        displacement.X -= pntr->vert2CurrPositionY;
        displacement.Z -= pntr->vert2CurrPositionY;

        displacement.X -= pntr->vert3CurrPositionY;
        displacement.Z += pntr->vert3CurrPositionY;

        displacement.X += pntr->vert4CurrPositionY;
        displacement.Z += pntr->vert4CurrPositionY;
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

    //allow maximum 1 count deviation in terms of fixed point
    if (errorIntPosX > 1) {
        std::string infoTxt("Verify_move_xyz_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " XPos Difference: ";
        infoTxt += std::to_string((int)(endXPos - posXResult));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    if (errorIntPosY > 1) {
        std::string infoTxt("Verify_move_xyz_step: Testcase for following sequence failed: ");
        infoTxt += std::to_string((int)(whichSeqCaseTested));
        infoTxt += " YPos Difference: ";
        infoTxt += std::to_string((int)(endYPos - posYResult));
        infoTxt += " dec!";

        logging::Error(infoTxt);
        return false;
    }

    if (errorIntPosZ > 1) {
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

    overallResult &= Verify_move_xyz_step(0x0C71, 0x0001, 0x08A8, 0x0C70, 0x0001, 0x0894, 0xFFB8, 0x071D, 0x0078, 1);

    if (overallResult) {
        logging::Info("Verify_move_xyz: All testcases pass");
        return true;
    } else {
        logging::Error("Verify_move_xyz: At least one testcase failed");
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
    //TODO: The implementation in the original game looks
    //like the next line was implemented, but then a jump operation
    //jumps over it, which renders it inactive at the end
    //According to one debugging session the value for position.Y
    //really seems to be not modified. So I need to comment the next
    //line out as well, until I understand the situation better
    //position.Y -= v4 * cos(angle2);

    return 1;
}
