/*
 ----------------------------------------------------------------------------------------------------------------------------
 Function CastRayDDA was initially taken from the great project/tutorial (and later modified slightly by me)
 For license also see worldaware-CastRayDDA-OLC3-LICENCE.md
 
 Fast Ray Casting Using DDA
//  "Itchy Eyes... Not blinking enough..." - javidx9

 Video: https://youtu.be/NbSee-XM7WA

 License (OLC-3)
 ~~~~~~~~~~~~~~~

    Copyright 2018 - 2021 OneLoneCoder.com

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions or derivations of source code must retain the above
    copyright notice, this list of conditions and the following disclaimer.

    2. Redistributions or derivative works in binary form must reproduce
    the above copyright notice. This list of conditions and the following
    disclaimer must be reproduced in the documentation and/or other
    materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Links
    ~~~~~
    YouTube:	https://www.youtube.com/javidx9
                https://www.youtube.com/javidx9extra
    Discord:	https://discord.gg/WhwHUMV
    Twitter:	https://www.twitter.com/javidx9
    Twitch:		https://www.twitch.tv/javidx9
    GitHub:		https://www.github.com/onelonecoder
    Homepage:	https://www.onelonecoder.com

    Author
    ~~~~~~
    David Barr, aka javidx9, ©OneLoneCoder 2019, 2020, 2021
    
 ----------------------------------------------------------------------------------------------------------------------------
 Function DrawLine source code taken from great article from Josh Beam
 https://joshbeam.com/articles/simple_line_drawing/
 He also has a GitHub repo for this
 https://github.com/joshb/linedrawing
 
 For the license please see the following file:
 worldaware-DrawLine-README.md
 
 ----------------------------------------------------------------------------------------------------------------------------
 
 Other source code in this file:
    
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "worldaware.h"

void WorldAwareness::CreateStaticWorld() {
   //create a new image for the static
   //world
   //contains all the wall segments which define the edge of the race tracks
   //and all the outlines for the columns
   staticWorld =
       mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
                irr::core::dimension2d<irr::u32>(worldSizeX, worldSizeY));

   //draw all wallsegments in red
   //This will be used to derive craft distances to
   //the outside of the race track
   //we need this current distances for turning
   //decisions, so that we do not turn into a wall
   std::vector<LineStruct*>::iterator it;

   //projected points on the 2Dplane
   irr::core::vector2d<irr::f32> p1;
   irr::core::vector2d<irr::f32> p2;

   colorEmptySpace = new irr::video::SColor(255, 0, 0, 0);
   colorRed = new irr::video::SColor(255, 255, 0, 0);
   colorPlayer = new irr::video::SColor(255, 0, 0, 255);
   colorPlayer2 = new irr::video::SColor(255, 0, 255, 0);

   staticWorld->fill(*colorEmptySpace);

   for (it = mRace->ENTWallsegmentsLine_List->begin(); it != mRace->ENTWallsegmentsLine_List->end(); it++) {
         //p1 = Project3DPointTo2DPlane((*it)->A, *XZPlane);
         //p2 = Project3DPointTo2DPlane((*it)->B, *XZPlane);

         p1.X = -(*it)->A.X * 5.0f;
         p1.Y = (*it)->A.Z * 5.0f;

         p2.X = -(*it)->B.X * 5.0f;
         p2.Y = (*it)->B.Z * 5.0f;

         //draw 2D line in static world
         DrawLine(*staticWorld, *colorRed, p1.X, p1.Y, p2.X, p2.Y);
   }

   ColumnVerticeInfo vert1;
   //ColumnVerticeInfo vert2;
   ColumnVerticeInfo vert3;
   //ColumnVerticeInfo vert4;

   std::vector<ColumnsByPositionStruct>::iterator itCol;
   for (itCol = this->mRace->mLevelBlocks->ColumnsByPosition.begin(); itCol != this->mRace->mLevelBlocks->ColumnsByPosition.end(); ++itCol){
       //check if lowest block (block A, index 0) is in collision mesh or not (value 0 is not part of collision mesh)
       //if it is part of collision mesh, draw rectangle where lowest block sits in world
       if ((*itCol).pColumn->Definition->mInCollisionMesh[0] != 0) {
           //each cube of the colum has 24 vertices,
           //use vertices 16 up to 19 //which are vertices of bottom or top rectangle of cube
           //top or bottom is necessary as we create the 2D map from locking from the top down on player field
           vert1 = (*itCol).pColumn->GeometryInfoList->vertices.at(16);
           //vert2 = (*itCol).pColumn->GeometryInfoList->vertices.at(17);
           vert3 = (*itCol).pColumn->GeometryInfoList->vertices.at(18);
           //vert4 = (*itCol).pColumn->GeometryInfoList->vertices.at(19);

           //our universe X axis is mirrored!
           //therefore the - in X coordinate
           DrawRectangle(*staticWorld,
                         *colorRed,
                         -vert1.currPosition.X * PixelScaleFactor,
                         vert1.currPosition.Z * PixelScaleFactor,
                         -vert3.currPosition.X * PixelScaleFactor,
                         vert3.currPosition.Z * PixelScaleFactor);
       }

   }

   //only for debugging, save picture on disk
   //DebugSavePicture((char*)"dbgStaticWorld.png", staticWorld);

   //create vector with obstacle information for DDA out
   //of this picture
   CreateStaticWorldMap();

   //create dynamic world map variable
   mDynamicWorldMap = new std::vector<uint8_t>();

   int maxX = mRace->mLevelTerrain->get_width();
   int maxY = mRace->mLevelTerrain->get_heigth();

   //resize vector correctly
   mDynamicWorldMap->resize(maxX * maxY);
}

void WorldAwareness::UpdateDynamicWorldMap(Player* whichPlayer) {
  //first fill dynamic world map with all zeros
  //to remove all existing player locations
  std::fill(mDynamicWorldMap->begin(), mDynamicWorldMap->end(), 0);

  if (whichPlayer != mRace->player)
    DrawPlayerDynamicWorldMap(1, mRace->player);

  if (whichPlayer != mRace->player2)
    DrawPlayerDynamicWorldMap(2, mRace->player2);
}

void WorldAwareness::CreateStaticWorldMap() {
    mStaticWorldMap = new std::vector<uint8_t>();

    int currIdxX;
    int currIdxY = 0;
    int maxX = mRace->mLevelTerrain->get_width();
    int maxY = mRace->mLevelTerrain->get_heigth();

    int sqrIdxX;
    int sqrIdxY;
    int sqrIdxEx;
    int sqrIdxEy;

    //resize vector correctly
    mStaticWorldMap->resize(maxX * maxY);

    irr::video::SColor currCol;
    bool oF;

    //go through all map entry coordinates, too see if we have an obstacle
    //in the square with PixelScaleFactor pixels times PixelScaleFactor pixels
    for (currIdxX = 0; currIdxX < maxX; currIdxX++)
      for (currIdxY = 0; currIdxY < maxY; currIdxY++) {

          //which is the last pixel we need to check
          sqrIdxEx = (currIdxX + 1) * PixelScaleFactor;
          sqrIdxEy = (currIdxY + 1) * PixelScaleFactor;

          oF = false;

          //check the current locations pixel square for obstacle
          for (sqrIdxX = currIdxX * PixelScaleFactor; sqrIdxX < sqrIdxEx; sqrIdxX++)
            for (sqrIdxY = currIdxY * PixelScaleFactor; sqrIdxY < sqrIdxEy; sqrIdxY++) {
               currCol = staticWorld->getPixel(sqrIdxX, sqrIdxY);

               if (currCol != *colorEmptySpace) {
                   //we found an obstacle
                   mStaticWorldMap->at(currIdxX + currIdxY * maxX) = 1;
                   oF = true;

                   //we can break out of this two innermost nested loops
                   sqrIdxX = sqrIdxEx;
                   break;
               }
            }

          if (!oF) {
              //no obstacle found withhin PixelScaleFactor x PixelScaleFactor area
              mStaticWorldMap->at(currIdxX + currIdxY * maxX) = 0;
          }
       }

    //map creation finished
}

/*
void WorldAwareness::CreateStaticWorld() {
   //create a new image for the static
   //world
   staticWorld =
       mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
                irr::core::dimension2d<irr::u32>(worldSizeX, worldSizeY));

         //p1 = Project3DPointTo2DPlane((*it)->A, *XZPlane);
         //p2 = Project3DPointTo2DPlane((*it)->B, *XZPlane);

   int maxX = mRace->mLevelTerrain->get_width();
   int maxY = mRace->mLevelTerrain->get_heigth();

   irr::f32 h1;
   irr::f32 h2;
   irr::f32 h3;
   irr::f32 h4;

   for (int ix = 1; ix < maxX; ix++)
       for (int iy = 1; iy < maxY; iy++) {

           h1 = mRace->mLevelTerrain->GetMapEntry(ix - 1, iy - 1)->m_Height;
           h2 = mRace->mLevelTerrain->GetMapEntry(ix , iy - 1)->m_Height;
           h3 = mRace->mLevelTerrain->GetMapEntry(ix, iy)->m_Height

           if (mRace->mLevelTerrain->GetMapEntry(ix, iy)->m_Height > 2.0f) {
                DrawRectangle(*staticWorld, *colorRed, ix * PixelScaleFactor, iy * PixelScaleFactor, (ix + 1) * PixelScaleFactor, (iy +1)* PixelScaleFactor);
           }
    }

   //only for debugging, save picture on disk
   DebugSavePicture((char*)"dbgStaticWorld.png", staticWorld);
}*/

RayHitInfoStruct WorldAwareness::CastRay(IImage &image, irr::core::vector3df startPos, irr::core::vector3df dirVec) {
   RayHitInfoStruct result;

   irr::core::vector2df currRayPos;
   bool rayHitObject = false;
   bool seenEmptySpace = false;
   irr::core::vector2df StartPos2D;

   irr::core::vector2df hitPos;
   irr::f32 px;
   irr::f32 py;
   irr::f32 dx;
   irr::f32 dy;

   //in our world x coordinate is negative! (swapped!)
   StartPos2D.X = -startPos.X * PixelScaleFactor;
   StartPos2D.Y = startPos.Z * PixelScaleFactor;
   currRayPos = StartPos2D;
   dx = -dirVec.X;
   dy = dirVec.Z;

   irr::video::SColor currCol;

   //if debugging capability is enabled
   //copy input picture into debugging picture
   if (WA_ALLOW_DEBUGGING) {
       image.copyTo(debugWorld);
   }

   //first make sure we are in "free space" at the start
   //position, and we do not start in the craft or in any wall
   while (!rayHitObject) {
       px = currRayPos.X;
       py = currRayPos.Y;

       if ((px < 0) || (py < 0) || (px > worldSizeX) || (py > worldSizeY)) {
           //we have left the world image, it seems we have nothing hit
           hitPos = StartPos2D;
           result.HitType = RAY_HIT_NOTHING;
           rayHitObject = true;
       }

       currCol = image.getPixel(px, py);

       if (currCol != *colorEmptySpace) {
           //we hit something
           if (seenEmptySpace) {
            hitPos = currRayPos;
            rayHitObject = true;

            //what have we hit?
            if (currCol == *colorRed) {
                //we hit the terrain
                result.HitType = RAY_HIT_TERRAIN;
            } else if (currCol == *colorPlayer) {
                //we hit first player
                result.HitType = RAY_HIT_PLAYER;
                result.HitPlayerPntr = this->mRace->player;
            } else if (currCol == *colorPlayer2) {
                //we hit second player
                result.HitType = RAY_HIT_PLAYER;
                result.HitPlayerPntr = this->mRace->player2;
            }
           } else {
               //we did not seen free space yet
               seenEmptySpace = true;
           }
       } else {
           if (WA_ALLOW_DEBUGGING) {
               //debugging function is enabled, draw ray in debugging
               //image
               debugWorld->setPixel(px,py, *colorRed);
           }
           seenEmptySpace = true;
           currRayPos.X += dx;
           currRayPos.Y += dy;
       }
   }

   irr::f32 rayDistance = (hitPos - StartPos2D).getLength();
   result.HitDistance = rayDistance;

   return result;
}

//The following code (idea) was taken and modified from
/*
    Fast Ray Casting Using DDA
    "Itchy Eyes... Not blinking enough..." - javidx9

    Video: https://youtu.be/NbSee-XM7WA

    License (OLC-3)
    ~~~~~~~~~~~~~~~

    Copyright 2018 - 2021 OneLoneCoder.com

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions or derivations of source code must retain the above
    copyright notice, this list of conditions and the following disclaimer.

    2. Redistributions or derivative works in binary form must reproduce
    the above copyright notice. This list of conditions and the following
    disclaimer must be reproduced in the documentation and/or other
    materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Links
    ~~~~~
    YouTube:	https://www.youtube.com/javidx9
                https://www.youtube.com/javidx9extra
    Discord:	https://discord.gg/WhwHUMV
    Twitter:	https://www.twitter.com/javidx9
    Twitch:		https://www.twitch.tv/javidx9
    GitHub:		https://www.github.com/onelonecoder
    Homepage:	https://www.onelonecoder.com

    Author
    ~~~~~~
    David Barr, aka javidx9, ©OneLoneCoder 2019, 2020, 2021
*/
RayHitInfoStruct WorldAwareness::CastRayDDA(IImage &image, irr::core::vector3df startPos,
                                            irr::core::vector3df dirVec, irr::f32 maxRange,
                                            std::vector<irr::core::vector2di> &visitedCells) {
   RayHitInfoStruct result;

   irr::core::vector2df currRayPos;

   irr::core::vector2df vRayStart;
   irr::core::vector2df vRayDir;

   irr::core::vector2df hitPos;

   //in our world x coordinate is negative! (swapped!)
   vRayStart.X = -startPos.X;
   vRayStart.Y = startPos.Z;
   currRayPos = vRayStart;

   vRayDir.X = -dirVec.X;
   vRayDir.Y = dirVec.Z;

   vRayDir.normalize();

   irr::video::SColor currCol;

   //if debugging capability is enabled
   //copy input picture into debugging picture
   if (WA_ALLOW_DEBUGGING) {
       image.copyTo(debugWorld);
   }

   // Lodev.org also explains this additional optimistaion (but it's beyond scope of video)
   // olc::vf2d vRayUnitStepSize = { abs(1.0f / vRayDir.x), abs(1.0f / vRayDir.y) };

   irr::core::vector2df vRayUnitStepSize =
   { sqrt(1 + (vRayDir.Y / vRayDir.X) * (vRayDir.Y / vRayDir.X)), sqrt(1 + (vRayDir.X / vRayDir.Y) * (vRayDir.X / vRayDir.Y)) };
   irr::core::vector2di vMapCheck;

   vMapCheck.X = vRayStart.X;   //truncates to integer
   vMapCheck.Y = vRayStart.Y;   //truncates to integer

   irr::core::vector2df vRayLength1D;
   irr::core::vector2di vStep;
   irr::core::vector2di vMapSize;

   vMapSize.X = mRace->mLevelTerrain->get_width();
   vMapSize.Y = mRace->mLevelTerrain->get_heigth();

   // Establish Starting Conditions
   if (vRayDir.X < 0)
     {
       vStep.X = -1;
       vRayLength1D.X = (vRayStart.X - float(vMapCheck.X)) * vRayUnitStepSize.X;
      } else
         {
           vStep.X = 1;
           vRayLength1D.X = (float(vMapCheck.X + 1) - vRayStart.X) * vRayUnitStepSize.X;
         }

   if (vRayDir.Y < 0)
     {
      vStep.Y = -1;
      vRayLength1D.Y = (vRayStart.Y - float(vMapCheck.Y)) * vRayUnitStepSize.Y;
     }
      else
     {
       vStep.Y = 1;
       vRayLength1D.Y = (float(vMapCheck.Y + 1) - vRayStart.Y) * vRayUnitStepSize.Y;
    }

   // Perform "Walk" until collision or range check
   bool bTileFound = false;
   float fDistance = 0.0f;
   bool bOutsideMap = false;
   bool playerHit = false;
   int playerVal;

   while (!bTileFound && !bOutsideMap && (fDistance < maxRange))
    {
      // Walk along shortest path
      if (vRayLength1D.X < vRayLength1D.Y)
        {
          vMapCheck.X += vStep.X;
          fDistance = vRayLength1D.X;
          vRayLength1D.X += vRayUnitStepSize.X;
        }
         else {
              vMapCheck.Y += vStep.Y;
              fDistance = vRayLength1D.Y;
              vRayLength1D.Y += vRayUnitStepSize.Y;
           }

       // Test tile at new test point
       if (vMapCheck.X >= 0 && vMapCheck.X < vMapSize.X && vMapCheck.Y >= 0 && vMapCheck.Y < vMapSize.Y)
        {
           //remember that we visited this cell
           //we could need this information later, for example
           //to check what entities (collectables) the computer
           //player is able to see in its view field
           visitedCells.push_back(irr::core::vector2di(vMapCheck.X, vMapCheck.Y));

         if (this->mStaticWorldMap->at(vMapCheck.Y * vMapSize.X + vMapCheck.X) == 1)
          {
            bTileFound = true;
          } else if (this->mDynamicWorldMap->at(vMapCheck.Y * vMapSize.X + vMapCheck.X) > 0)
         {
           bTileFound = true;
           playerHit = true;
           playerVal = this->mDynamicWorldMap->at(vMapCheck.Y * vMapSize.X + vMapCheck.X);
         }

         if (WA_ALLOW_DEBUGGING) {
             //debugging function is enabled, draw ray in debugging
             //image
             DrawLine(*debugWorld, *colorRed, vRayStart.X * PixelScaleFactor,
                      vRayStart.Y * PixelScaleFactor,
                      (vRayStart.X + vRayDir.X * fDistance) * PixelScaleFactor,
                      (vRayStart.Y + vRayDir.Y * fDistance) * PixelScaleFactor);

             DrawRectangle(*debugWorld, *colorRed, vMapCheck.X * PixelScaleFactor, vMapCheck.Y * PixelScaleFactor,
                           (vMapCheck.X + 1) * PixelScaleFactor, (vMapCheck.Y + 1) * PixelScaleFactor);
         }
        } else {
           //we exited valid map region
           bOutsideMap = true;
       }
   }

   // Calculate intersection location
   irr::core::vector2df vIntersection;
   if (playerHit) {
       vIntersection = vRayStart + vRayDir * fDistance;
       result.HitType = RAY_HIT_PLAYER;
       result.HitDistance = fDistance;
       if (playerVal == 1) {
           result.HitPlayerPntr = this->mRace->player;
       } else if (playerVal == 2) {
           result.HitPlayerPntr = this->mRace->player2;
       }
   } else if (bTileFound)
     {
       vIntersection = vRayStart + vRayDir * fDistance;
       result.HitType = RAY_HIT_TERRAIN;
       result.HitDistance = fDistance;
     } else {
       result.HitType = RAY_HIT_NOTHING;
       result.HitDistance = 0.0f;
   }

   //return the result
   return result;
}

void WorldAwareness::DebugSavePicture(char* fileName, IImage* image) {

    //create new file for writting
    irr::io::IWriteFile* outputPic = mDevice->getFileSystem()->createAndWriteFile(fileName, false);

    mDriver->writeImageToFile(image, outputPic);

    //close output file
    outputPic->drop();
}

void WorldAwareness::Analyse(Player *whichPlayer) {
    RayHitInfoStruct rayInfo;

    if (WA_ALLOW_DEBUGGING) {
        CreateDynamicWorld(whichPlayer);
    }

    UpdateDynamicWorldMap(whichPlayer);

    std::vector<irr::core::vector2di> visitedCells;
    visitedCells.clear();

    //when finding out the free movement space around the player craft we do not care how far the distance is
    //if there is no obstruction in a certain direction the ray will in the worst case exit the 2D Map, and CastRay
    //will return with RAY_HIT_NOTHING and the HitDistance in this case will be 0.0f, which means the player will not go into this direction
    //as 0.0f means no movement space/distance available in this direction
    //If Ray hits a wallsegment (marks outer edge of race track), or a colum CastRay will return RAY_HIT_TERRAIN, and
    //HitDistance will contain distance to this point we hit
    //If Ray hits another player, CastRay will return RAY_HIT_PLAYER, and HitDistance is the distance to the player (ray length)
    //But here we do not really care if we hit terrain, or a player, as this are all just things in our movement way

    //rayInfo = CastRay(*dynamicWorld, whichPlayer->WorldCoordCraftRightPnt, whichPlayer->craftSidewaysToRightVec);
    //rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->WorldCoordCraftRightPnt, whichPlayer->craftSidewaysToRightVec, 1000.0f);
    rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->phobj->physicState.position,
                         whichPlayer->craftSidewaysToRightVec, 1000.0f, visitedCells);
    whichPlayer->mCraftDistanceAvailRight = rayInfo.HitDistance;

    if (WA_ALLOW_DEBUGGING) {
        //for ray debugging
        if (WriteOneDbgPic && whichPlayer == mRace->player) {
            DebugSavePicture((char*)"rayRight.png", debugWorld);
        }
    }

    //rayInfo = CastRay(*dynamicWorld, whichPlayer->WorldCoordCraftLeftPnt, -whichPlayer->craftSidewaysToRightVec);
    //rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->WorldCoordCraftLeftPnt, -whichPlayer->craftSidewaysToRightVec, 1000.0f);
    rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->phobj->physicState.position,
                         -whichPlayer->craftSidewaysToRightVec, 1000.0f, visitedCells);
    whichPlayer->mCraftDistanceAvailLeft = rayInfo.HitDistance;

    if (WA_ALLOW_DEBUGGING) {
        //for ray debugging
        if (WriteOneDbgPic && whichPlayer == mRace->player) {
            DebugSavePicture((char*)"rayLeft.png", debugWorld);
        }
    }

    //rayInfo = CastRay(*dynamicWorld, whichPlayer->WorldCoordCraftFrontPnt, whichPlayer->craftForwardDirVec);
   // rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->WorldCoordCraftFrontPnt, whichPlayer->craftForwardDirVec, 1000.0f);
    rayInfo = CastRayDDA(*dynamicWorld,  whichPlayer->phobj->physicState.position,
                         whichPlayer->craftForwardDirVec, 1000.0f, visitedCells);
    whichPlayer->mCraftDistanceAvailFront = rayInfo.HitDistance;

     if (WA_ALLOW_DEBUGGING) {
        //for ray debugging
          if (WriteOneDbgPic && whichPlayer == mRace->player) {
            DebugSavePicture((char*)"rayFront.png", debugWorld);
            }
        }

    //rayInfo = CastRay(*dynamicWorld, whichPlayer->WorldCoordCraftBackPnt, -whichPlayer->craftForwardDirVec);
    //rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->WorldCoordCraftBackPnt, -whichPlayer->craftForwardDirVec, 1000.0f);
    rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->phobj->physicState.position,
                         -whichPlayer->craftForwardDirVec, 1000.0f, visitedCells);
    whichPlayer->mCraftDistanceAvailBack = rayInfo.HitDistance;

    if (WA_ALLOW_DEBUGGING) {
        //for ray debugging
        if (WriteOneDbgPic && whichPlayer == mRace->player) {
               DebugSavePicture((char*)"rayBack.png", debugWorld);
        }
    }

    visitedCells.clear();

    //now simulate the view of the player forwards, and send raycasts forward with different angles and with a defined max length
    //we want to figure out which players the current player can see (important for tagging of opponents)

    irr::f32 fieldOfView = 60.0f;
    int8_t nrRays = 19;
    int8_t currRay;
    irr::core::quaternion rotateFurther;
    irr::f32 deltaAngle = fieldOfView / ((irr::f32)(nrRays - 1));

    irr::f32 currRAngle = - (nrRays / 2) * deltaAngle;

    irr::core::vector3df forwVect;
    irr::core::matrix4 matrx;
    irr::f32 maxViewDistance = 40.0f; //The max distance which we allow to "see" things (other players, ...)

    std::vector<RayHitInfoStruct> PlayerSeenList;
    std::vector<irr::f32> PlayerSeenAngleList;
    PlayerSeenList.clear();
    PlayerSeenAngleList.clear();

    for (currRay = 0; currRay < nrRays; currRay++) {
        forwVect = whichPlayer->craftForwardDirVec;

        //rotate forward directon vec further defined by current iteration angle
        rotateFurther.fromAngleAxis((currRAngle / 180.0f) * irr::core::PI, irr::core::vector3df(0.0f, 1.0f, 0.0f));

        rotateFurther.getMatrix_transposed(matrx);
        matrx.rotateVect(forwVect);
        forwVect.normalize();

        //forwVect is the direction we want the ray to shoot in
        //rayInfo = CastRay(*dynamicWorld, whichPlayer->WorldCoordCraftFrontPnt, forwVect);
        //rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->WorldCoordCraftFrontPnt, forwVect, 1000.0f);
        rayInfo = CastRayDDA(*dynamicWorld, whichPlayer->phobj->physicState.position,
                             forwVect, 1000.0f, visitedCells);
        if (rayInfo.HitType == RAY_HIT_PLAYER) {
            if ((rayInfo.HitDistance < maxViewDistance) && (rayInfo.HitPlayerPntr != whichPlayer)) {
                //we see an opponement player!
                PlayerSeenList.push_back(rayInfo);

                //also remember current view angle under which this player was
                //seen to be able to give players more in the middle of the screen more
                //priority when deciding which other player to target with weapons
                PlayerSeenAngleList.push_back(currRAngle);
            }
        }

        if (WA_ALLOW_DEBUGGING) {
            //for ray debugging
            if (WriteOneDbgPic && whichPlayer == mRace->player) {
                   char dbgFileName[60];
                   sprintf (dbgFileName, "rayView%0*d.png", 2, currRay);

                   DebugSavePicture(dbgFileName, debugWorld);
            }
           }

        //increase current angle by delta Angle
        currRAngle += deltaAngle;
    }

    //store this player seen information also in the player
    //object itself
    whichPlayer->PlayerSeenList = PlayerSeenList;
    whichPlayer->PlayerSeenAngleList = PlayerSeenAngleList;

    //now run other player selection/targeting logic

    Player* resPlayer = NULL;

    //did we see any other player in or view field?
    if (PlayerSeenAngleList.size() > 0) {
        //yes, select the player which is closest to the middle of the screen (which means
        //we search an angle closest to 0.0, and with a distance small enough)
        std::vector<irr::f32>::iterator itAngle;
        irr::f32 currMinVal = abs(PlayerSeenAngleList[0]);
        int8_t minElIndx = 0;
        int8_t currElIndx = 0;

        for (itAngle = PlayerSeenAngleList.begin(); itAngle != PlayerSeenAngleList.end(); ++itAngle) {
            if (abs((*itAngle)) < currMinVal) {
                //is the distance small enough? otherwise skip entry
                if (PlayerSeenList[currElIndx].HitDistance < maxViewDistance) {
                    minElIndx = currElIndx;
                    currMinVal = abs((*itAngle));
                }
            }

            currElIndx++;
        }

        resPlayer = PlayerSeenList[minElIndx].HitPlayerPntr;
    }

    //tell player if/which player he has targeted
    whichPlayer->SetTarget(resPlayer);

    if (WA_ALLOW_DEBUGGING) {
        if (WriteOneDbgPic && whichPlayer == mRace->player) {
          if (whichPlayer == this->mRace->player) {
           if (WriteOneDbgPic) {
               WriteOneDbgPic = false;
           }
         }
        }
    }

    //now analyze which entities (more precise collectables) the
    //computer player is able to see in the view field
    whichPlayer->mCpCollectablesSeenByPlayer.clear();

    if (!whichPlayer->mHumanPlayer) {
        std::vector<Collectable*> CollToVerify;
        CollToVerify.clear();

        //first determine which collectibles are even close enough to the
        //player position so that we should consider them
        std::vector<Collectable*>::iterator itCollect;

        irr::f32 distanceSQ;
        //we want to reduce view distance for computer players and collectables
        //a little bit more, because otherwise the computer player will do
        //sometimes trying to catch crazy collectables far away
        irr::f32 maxViewDistance2ndStage = 15.0f;
        irr::f32 distanceSQMaxLimit = maxViewDistance2ndStage * maxViewDistance2ndStage;
        irr::core::vector3df posCol;

        for (itCollect = this->mRace->ENTCollectablesVec->begin();
             itCollect != this->mRace->ENTCollectablesVec->end();
             ++itCollect) {
                posCol = (*itCollect)->Position;
                posCol.X = -posCol.X;
               distanceSQ = (posCol - whichPlayer->phobj->physicState.position).getLengthSQ();

               if (distanceSQ < distanceSQMaxLimit) {
                   //collectible is close enough to the player
                   //we need to check it later afterwards
                   CollToVerify.push_back(*itCollect);
               }
        }

        std::vector<irr::core::vector2di>::iterator itCells;

        irr::core::vector2di currCell;
        irr::core::vector2di cellPos;

        if (visitedCells.size() > 0) {
            for (itCells = visitedCells.begin(); itCells != visitedCells.end(); ++itCells) {
               currCell = (*itCells);

               for (itCollect = CollToVerify.begin();
                    itCollect != CollToVerify.end();
                    ++itCollect) {
                        cellPos.X = (int)((*itCollect)->Position.X);
                        cellPos.Y = (int)((*itCollect)->Position.Z);

                        if (currCell == cellPos) {
                           whichPlayer->mCpCollectablesSeenByPlayer.push_back(*itCollect);
                    }
               }
            }
        }
    }
}

//DrawLine source code taken from https://joshbeam.com/articles/simple_line_drawing/
void WorldAwareness::DrawRectangle(irr::video::IImage &image, irr::video::SColor &color, irr::f32 x1, irr::f32 y1,
                     irr::f32 x2, irr::f32 y2)
{
    DrawLine(image, color, x1, y1, x2, y1);
    DrawLine(image, color, x2, y1, x2, y2);
    DrawLine(image, color, x2, y2, x1, y2);
    DrawLine(image, color, x1, y2, x1, y1);
}

//DrawLine source code taken from https://joshbeam.com/articles/simple_line_drawing/
void WorldAwareness::DrawLine(irr::video::IImage &image, irr::video::SColor &color, irr::f32 x1, irr::f32 y1,
                     irr::f32 x2, irr::f32 y2)
{
    irr::f32 xdiff = (x2 - x1);
    irr::f32 ydiff = (y2 - y1);

    if(xdiff == 0.0f && ydiff == 0.0f) {
        image.setPixel( (u32)(x1), (u32)(y1), color);
        return;
    }

    if(fabs(xdiff) > fabs(ydiff)) {
            irr::f32 xmin, xmax;

            // set xmin to the lower x value given
            // and xmax to the higher value
            if(x1 < x2) {
                xmin = x1;
                xmax = x2;
            } else {
                xmin = x2;
                xmax = x1;
            }

            // draw line in terms of y slope
            irr::f32 slope = ydiff / xdiff;
            for(irr::f32 x = xmin; x <= xmax; x += 1.0f) {
                irr::f32 y = y1 + ((x - x1) * slope);
                image.setPixel( (u32)(x), (u32)(y), color);
            }
        } else {
            irr::f32 ymin, ymax;

            // set ymin to the lower y value given
            // and ymax to the higher value
            if(y1 < y2) {
                ymin = y1;
                ymax = y2;
            } else {
                ymin = y2;
                ymax = y1;
            }

            // draw line in terms of x slope
            irr::f32 slope = xdiff / ydiff;
            for(irr::f32 y = ymin; y <= ymax; y += 1.0f) {
                irr::f32 x = x1 + ((y - y1) * slope);
                image.setPixel((u32)(x), (u32)(y), color);
            }
        }
}

void WorldAwareness::SetPixelDynamicWorldMap(int playerNr, irr::f32 x1, irr::f32 y1) {
    int x = (x1 / (float)(PixelScaleFactor));
    int y = (y1 / (float)(PixelScaleFactor));

    int sizeX = mRace->mLevelTerrain->get_width();
    int sizeY = mRace->mLevelTerrain->get_heigth();

    if (x >= sizeX) return;
    if (y >= sizeY) return;

    mDynamicWorldMap->at(sizeX * y + x) = playerNr;
}

//DrawLine source code taken from https://joshbeam.com/articles/simple_line_drawing/
void WorldAwareness::DrawLineIntoDynamicWorldMap(int playerNr, irr::f32 x1, irr::f32 y1,
                     irr::f32 x2, irr::f32 y2)
{
    irr::f32 xdiff = (x2 - x1);
    irr::f32 ydiff = (y2 - y1);

    if(xdiff == 0.0f && ydiff == 0.0f) {
        SetPixelDynamicWorldMap(playerNr, (u32)(x1), (u32)(y1));
        return;
    }

    if(fabs(xdiff) > fabs(ydiff)) {
            irr::f32 xmin, xmax;

            // set xmin to the lower x value given
            // and xmax to the higher value
            if(x1 < x2) {
                xmin = x1;
                xmax = x2;
            } else {
                xmin = x2;
                xmax = x1;
            }

            // draw line in terms of y slope
            irr::f32 slope = ydiff / xdiff;
            for(irr::f32 x = xmin; x <= xmax; x += 1.0f) {
                irr::f32 y = y1 + ((x - x1) * slope);
                SetPixelDynamicWorldMap(playerNr, (u32)(x), (u32)(y));
            }
        } else {
            irr::f32 ymin, ymax;

            // set ymin to the lower y value given
            // and ymax to the higher value
            if(y1 < y2) {
                ymin = y1;
                ymax = y2;
            } else {
                ymin = y2;
                ymax = y1;
            }

            // draw line in terms of x slope
            irr::f32 slope = xdiff / ydiff;
            for(irr::f32 y = ymin; y <= ymax; y += 1.0f) {
                irr::f32 x = x1 + ((y - y1) * slope);
                SetPixelDynamicWorldMap(playerNr, (u32)(x), (u32)(y));
            }
        }
}

void WorldAwareness::DrawRectangleDynamicWorldMap(int playerNr, irr::f32 x1, irr::f32 y1,
                     irr::f32 x2, irr::f32 y2)
{
    DrawLineIntoDynamicWorldMap(playerNr, x1, y1, x2, y1);
    DrawLineIntoDynamicWorldMap(playerNr, x2, y1, x2, y2);
    DrawLineIntoDynamicWorldMap(playerNr, x2, y2, x1, y2);
    DrawLineIntoDynamicWorldMap(playerNr, x1, y2, x1, y1);
}

void WorldAwareness::DrawPlayer(IImage &image, irr::video::SColor &color, Player* whichPlayer) {
    irr::core::aabbox3df bbox = whichPlayer->Player_node->getTransformedBoundingBox();
    irr::core::vector3df edges[8];

    bbox.getEdges(&edges[0]);

    //our universe has swapped X axis
    DrawRectangle(image, color, -edges[1].X * PixelScaleFactor, edges[1].Z * PixelScaleFactor, -edges[7].X * PixelScaleFactor, edges[7].Z * PixelScaleFactor);
}

void WorldAwareness::DrawPlayerDynamicWorldMap(int playerNr, Player* whichPlayer) {
    irr::core::aabbox3df bbox = whichPlayer->Player_node->getTransformedBoundingBox();
    irr::core::vector3df edges[8];

    bbox.getEdges(&edges[0]);

    //our universe has swapped X axis
    DrawRectangleDynamicWorldMap(playerNr, -edges[1].X * PixelScaleFactor, edges[1].Z * PixelScaleFactor, -edges[7].X * PixelScaleFactor, edges[7].Z * PixelScaleFactor);
}

WorldAwareness::WorldAwareness(irr::IrrlichtDevice* device, irr::video::IVideoDriver *driver, Race* race) {
   mRace = race;
   mDriver = driver;
   mDevice = device;

   worldSizeX = mRace->mLevelTerrain->get_width() * PixelScaleFactor;
   worldSizeY = mRace->mLevelTerrain->get_heigth() * PixelScaleFactor;
   
   //create static world drawing
   //with only the wall segments
   //that indicate the other lines of the
   //race track
   //also contains positions of columns (but only if a column
   //is part of collision mesh)
   CreateStaticWorld();

   //create a new image for the dynamic
   //world
   dynamicWorld =
       mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
                irr::core::dimension2d<irr::u32>(worldSizeX, worldSizeY));

   if (WA_ALLOW_DEBUGGING) {
        //create a new image for world debugging
        debugWorld =  mDriver->createImage(irr::video::ECOLOR_FORMAT::ECF_A8R8G8B8,
                                      irr::core::dimension2d<irr::u32>(worldSizeX, worldSizeY));
   }
}

void WorldAwareness::CreateDynamicWorld(Player* whichPlayer) {
    //first copy static world into dynamic world
    staticWorld->copyTo(dynamicWorld);

    if (whichPlayer != mRace->player)
        DrawPlayer(*dynamicWorld, *colorPlayer, mRace->player);

    if (whichPlayer != mRace->player2)
        DrawPlayer(*dynamicWorld, *colorPlayer2, mRace->player2);

    //only for debugging, save picture on disk
  //  DebugSavePicture((char*)"dbgDynamicWorld.png", dynamicWorld);
}

WorldAwareness::~WorldAwareness() {
    //delete XZPlane;
    staticWorld->drop();

    delete mStaticWorldMap;
    delete mDynamicWorldMap;

    delete colorRed;
    delete colorPlayer;
    delete colorPlayer2;
    delete colorEmptySpace;
}
