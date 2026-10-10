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

#include "vtrack.h"
#include "../race.h"
#include "../game.h"
#include "../resources/levelfile.h"
#include "../models/levelterrain.h"
#include "../draw/drawdebug.h"
#include "../resources/mapentry.h"
#include "../vanilla/vcalc.h"

VTrack::VTrack(Race* parentRace) {
    mParentRace = parentRace;

    NextColVect = 1;
    NextVectsList = 1;
}

void VTrack::Trap(std::string trapInfo) {
    logging::Error(trapInfo);
    mParentRace->mGame->StopTime();
}

void VTrack::DebugDrawTrackColVectStruct(TrackColVectStruct* whichStruct) {
    irr::core::vector3df vanillaPos1;
    irr::core::vector3df vanillaPos2;
    irr::core::vector3df vanillaMidPoint;
    irr::core::vector3df vanillaMove1;
    irr::core::vector3df vanillaMove2;
    irr::f32 len;

    bool drawAngleTest = false;

    vanillaPos1.X = mParentRace->mVCalc->FixedPointToFloat24D8(static_cast<int32_t>(whichStruct->pos1X));
    vanillaPos1.Y = mParentRace->mVCalc->FixedPointToFloat24D8(static_cast<int32_t>(whichStruct->pos1Y));
    vanillaPos1.Z = mParentRace->mVCalc->FixedPointToFloat24D8(static_cast<int32_t>(whichStruct->pos1Z));

    vanillaPos2.X = mParentRace->mVCalc->FixedPointToFloat24D8(static_cast<int32_t>(whichStruct->pos2X));
    vanillaPos2.Y = mParentRace->mVCalc->FixedPointToFloat24D8(static_cast<int32_t>(whichStruct->pos2Y));
    vanillaPos2.Z = mParentRace->mVCalc->FixedPointToFloat24D8(static_cast<int32_t>(whichStruct->pos2Z));

    if (drawAngleTest) {
        vanillaMidPoint = (vanillaPos1 + vanillaPos2) * irr::core::vector3df(0.5f, 0.5f, 0.5f);
        vanillaMove1 = vanillaMidPoint;
        vanillaMove2 = vanillaMidPoint;
        len = (vanillaPos1 - vanillaPos2).getLength();

        mParentRace->mVCalc->move_xyz(vanillaMove1, whichStruct->Angle, 0.0f, len * 0.25f);
        mParentRace->mVCalc->move_xyz(vanillaMove2, whichStruct->Angle - 180.0f, 0.0f, len * 0.25f);
    }

    irr::core::vector3df irrPos1 = mParentRace->mVCalc->VanillaToIrrlichtCoord(vanillaPos1);
    irr::core::vector3df irrPos2 = mParentRace->mVCalc->VanillaToIrrlichtCoord(vanillaPos2);

    mParentRace->mGame->mDrawDebug->Draw3DLine(irrPos1, irrPos2, mParentRace->mGame->mDrawDebug->blue);

    if (drawAngleTest) {
        irr::core::vector3df irrMidPoint = mParentRace->mVCalc->VanillaToIrrlichtCoord(vanillaMidPoint);
        irr::core::vector3df irrMove1 = mParentRace->mVCalc->VanillaToIrrlichtCoord(vanillaMove1);
        irr::core::vector3df irrMove2 = mParentRace->mVCalc->VanillaToIrrlichtCoord(vanillaMove2);

        mParentRace->mGame->mDrawDebug->Draw3DLine(irrMidPoint, irrMove1, mParentRace->mGame->mDrawDebug->green);
        mParentRace->mGame->mDrawDebug->Draw3DLine(irrMidPoint, irrMove2, mParentRace->mGame->mDrawDebug->orange);
    }
}

void VTrack::DrawDebugVectors() {
    for (size_t idx = 0; idx < this->NextColVect; idx++) {
        DebugDrawTrackColVectStruct(&this->ColVects[idx]);
    }
/*
    DrawDbgOutput(DbgUsedColVecMe1, mParentRace->mGame->mDrawDebug->cyan);
   DrawDbgOutput(DbgUsedColVecMe2, mParentRace->mGame->mDrawDebug->orange);
    DrawDbgOutput(DbgUsedColVecMe3, mParentRace->mGame->mDrawDebug->pink);*/
}

void VTrack::add_collision_to_single_mapwho(irr::f32 x, irr::f32 y) {
    int16_t nextVectsList = this->NextVectsList;
    int16_t nextColVect;
    int16_t v6;

    int cellX = (int)(x / mParentRace->mLevelTerrain->segmentSize);
    int cellY = (int)(y / mParentRace->mLevelTerrain->segmentSize);

    //get pntr to this tile
    MapEntry *pntr = mParentRace->mLevelTerrain->levelRes->pMap[cellX][cellY];

    if ((pntr != nullptr) && ((nextVectsList + 1) < 10000)) {
        nextColVect = this->NextColVect;
        if (ColVectsList[pntr->mVector].Vect != nextColVect) {
            ColVectsList[nextVectsList].Vect = nextColVect;
            ColVectsList[this->NextVectsList].NextColList = pntr->mVector;
          /*  if (pntr->mVector != 0) {
                std::cout << "Val = " << std::hex << pntr->mVector << std::endl;
            }*/
            v6 = this->NextVectsList;
            this->NextVectsList = v6 + 1;
            pntr->mVector = v6;
            //std::cout << "X: " << std::dec << cellX << "Y: " << std::dec << cellY << std::endl;
        }
    }
}

void VTrack::insert_vect(irr::core::vector3df position1, irr::core::vector3df position2) {
   //better use fixed point arithmetic here
   int32_t pos1X = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(position1.X));
   int32_t pos1Y = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(position1.Y));
   int32_t pos1Z = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(position1.Z));

   int32_t pos2X = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(position2.X));
   int32_t pos2Y = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(position2.Y));
   int32_t pos2Z = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(position2.Z));

   int32_t diffX = (pos2X - pos1X);
   int32_t diffY = (pos2Y - pos1Y);

   int32_t v23;
   int32_t v25;
   int32_t v26;
   int32_t v22;
   int32_t v19;
   int32_t v24;
   int32_t v20;
   int32_t v21;
   int32_t v27;
   int32_t v28;
   int32_t v29;
   int32_t v30;

   if (diffX < 0) {
       diffX = (pos1X - pos2X);
   }

   if (diffY < 0) {
       diffY = (pos1Y - pos2Y);
   }

   if ((diffX <= 0x7FFF) && (diffY <= 0x7FFF) &&
           ((this->NextColVect + 1) < 250) && ((this->NextVectsList + 1) < 10000)) {
        this->ColVects[this->NextColVect].pos1X = pos1X;
        this->ColVects[this->NextColVect].pos1Y = pos1Y;
        this->ColVects[this->NextColVect].pos1Z = pos1Z;
        this->ColVects[this->NextColVect].pos2X = pos2X;
        this->ColVects[this->NextColVect].pos2Y = pos2Y;
        this->ColVects[this->NextColVect].pos2Z = pos2Z;
        this->ColVects[this->NextColVect].Angle =
                mParentRace->mVCalc->angle_get_xy(position1, position2);

        int32_t v15 = (pos2X - pos1X) << 9;
        int32_t v16 = abs(v15);
        int32_t v17 = (pos2Y - pos1Y) << 9;
        int32_t v18 = abs(v17);

        if (v16 < v18) {
           v23 = 0;

           if (v18) {
               if ((v18 == -1) && (v17 == 0x80000000)) {
                   Trap(std::string("Trap insert_vect 1"));
                   return;
               }
               v23 = (v17 / v18) << 8;
           }

           v25 = v18;
           v26 = abs(v23);
           if ((v26 == -1) && (v25 == 0x80000000)) {
               Trap(std::string("Trap insert_vect 2"));
               return;
           }

           v22 = v25 / v26;

           if (!(v25 / v26)) {
              // break;
              std::cout << "break!" << std::endl;
           }

           if ((v22 == -1) && (v15 == 0x80000000)) {
               Trap(std::string("Trap insert_vect 3"));
               return;
           }
           v19 = v15 / v22;
           v24 = pos1X;
        } else {
           v19 = 0;

           if (v16) {
               if ((v16 == -1) && (v15 == 0x80000000)) {
                   Trap(std::string("Trap insert_vect 4"));
                   return;
               }
               v19 = (v15 / v16) << 8;
           }

           v20 = v16;
           v21 = abs(v19);
           if ((v21 == -1) && (v20 == 0x80000000)) {
               Trap(std::string("Trap insert_vect 5"));
               return;
           }

           v22 = v20 / v21;

           if (!(v20 / v21)) {
             //  break;
             std::cout << "break!" << std::endl;
           }

           if ((v22 == -1) && (v17 == 0x80000000)) {
               Trap(std::string("Trap insert_vect 1"));
               return;
           }

           v23 = v17 / v22;
           v24 = pos1X;
        }

        v27 = v24 << 9;
        v28 = pos1Y << 9;

        if (v19 == v23) {
            v28 += 512;
        }

        irr::f32 floatXPos;
        irr::f32 floatYPos;

        while (v22 >= 0) {
            v29 = v27 / 512;
            v30 = v28 / 512;
            --v22;
            v27 += v19;
            v28 += v23;

            floatXPos = mParentRace->mVCalc->FixedPointToFloat24D8(v29);
            floatYPos = mParentRace->mVCalc->FixedPointToFloat24D8(v30);

            add_collision_to_single_mapwho(floatXPos, floatYPos);
        }
        ++this->NextColVect;
   }
}

void VTrack::DrawDbgOutput(std::vector<irr::core::line3df>& dbgOutput, ColorStruct* color) {
    std::vector<irr::core::line3df>::iterator it;

    for (it = dbgOutput.begin(); it != dbgOutput.end(); ++it) {
        mParentRace->mGame->mDrawDebug->Draw3DLine((*it).start , (*it).end, color);
    }

}

uint8_t VTrack::do_move_colide(irr::f32 x1Float, irr::f32 y1Float, irr::f32 x2Float, irr::f32 y2Float, MapEntry* me)
{
    uint8_t v7;
    int16_t v6;
    int32_t v8;
    int32_t v9;
    int32_t v10;
    int32_t v13;
    int32_t v15;
    int32_t v16;
    int32_t v17;
    int32_t v19;
    int32_t v20;
    int32_t v21;
    int32_t v22;
    int32_t v23;
    int32_t v24;
    int32_t v25;
    int32_t v26;
    int32_t v27;
    int32_t v28;
    int32_t v29;

    int32_t Xpos;
    int32_t Ypos;

    bool v18;
    bool v30;

    TrackColVectStruct* v11;

    //better do this in fixed point arithmetic
    int32_t x1 = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(x1Float));
    int32_t y1 = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(y1Float));

    int32_t x2 = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(x2Float));
    int32_t y2 = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint24D8(y2Float));

    int16_t Vector;
    v7 = 0;

    if (me->mVector) {
        Vector = me->mVector;
        v6 = Vector;
        v8 = x1;
        v9 = x2;
        v10 = x1 - x2;
        do {
            v11 = &ColVects[ColVectsList[v6].Vect];
            Xpos = ColVects[ColVectsList[v6].Vect].pos2X;
            v13 = v11->pos1X;
            Ypos = ColVects[ColVectsList[v6].Vect].pos1Y;
            v15 = ColVects[ColVectsList[v6].Vect].pos2Y;

            v16 = Xpos - v13;
            v17 = v13;
            if ((Xpos - v13) < 0) {
                v17 = ColVects[ColVectsList[v6].Vect].pos2X;
                Xpos = v11->pos1X;
            }
            if (v10 <= 0) {
               v18 = Xpos < v8;
               v19 = 0;
               if (v18) {
                   goto do_move_colide_LABEL36;
               }
               v19 = 0;
               if (v9 < v17) {
                   goto do_move_colide_LABEL36;
               }
               v20 = v15 - Ypos;
            } else {
               v18 = Xpos < v9;
               v19 = 0;
               if (v18) {
                   goto do_move_colide_LABEL36;
               }
               v20 = v15 - Ypos;
               if (v8 < v17) {
                   v19 = 0;
                   goto do_move_colide_LABEL36;
               }
            }

            v21 = y1 - y2;
            if (v20 >= 0) {
                v23 = v15;
                v22 = Ypos;
            } else {
                v22 = v15;
                v23 = Ypos;
            }

            if (v21 <= 0) {
                v18 = v23 < y1;
                v19 = 0;
                if (v18) {
                   goto do_move_colide_LABEL36;
                }
                v19 = 0;
                if (y2 < v22) {
                   goto do_move_colide_LABEL36;
                }
                v24 = v13 - v8;
            } else {
                v18 = v23 < y2;
                v19 = 0;
                if (v18) {
                   goto do_move_colide_LABEL36;
                }
                v24 = v13 - v8;
                if (y1 < v22) {
                    v19 = 0;
                    goto do_move_colide_LABEL36;
                }
            }

            v25 = Ypos - y1;
            v26 = v21 * v24 - v10 * v25;
            v27 = v20 * v10 - v16 * v21;
            if (v27 <= 0) {
                v19 = 0;
                if (v26 <= 0) {
                    v28 = v16 * v25;
                    if (v26 >= v27) {
do_move_colider_LABEL28:
                        v29 = v28 - v20 * v24;
                        if (v27 <= 0) {
                           v18 = v29 > 0;
                           v30 = v29 < v27;
                           if (!v18) {
do_move_colider_LABEL32:
                               if (!v30) {
                                 v19 = 2;
                                 if (!v27) {
                                     v19 = 1;
                                 }
                                 goto do_move_colide_LABEL36;
                               }
                           }
                        } else {
                            v18 = v29 < 0;
                            v30 = v27 < v29;
                            if (!v18) {
                                goto do_move_colider_LABEL32;
                            }
                        }
                    }
do_move_colider_LABEL33:
                        v19 = 0;
                    }
                } else {
                  if (v26 < 0) {
                      goto do_move_colider_LABEL33;
                  }
                  v28 = v16 * v25;
                  if (v27 >= v26) {
                      goto do_move_colider_LABEL28;
                  }
                  v19 = 0;
                }
do_move_colide_LABEL36:
                if (v19) {
                   TrackCollisionVectorX = v11->pos2X - v11->pos1X;
                   TrackCollisionVectorY = v11->pos2Y - v11->pos1Y;
                   v7 = 1;
                   TrackCollisionVectorAngle = v11->Angle;
                }
                Vector = ColVectsList[Vector].NextColList;
                v6 = Vector;
            } while (Vector && !v7);
        }

        return v7;
}

uint16_t VTrack::track_vector_collide(irr::core::vector3df position1, irr::core::vector3df position2) {
    int v5;
    int v8;

    int cell1X = (int)(position1.X / mParentRace->mLevelTerrain->segmentSize);
    int cell1Y = (int)(position1.Y / mParentRace->mLevelTerrain->segmentSize);

    int cell2X = (int)(position2.X / mParentRace->mLevelTerrain->segmentSize);
    int cell2Y = (int)(position2.Y / mParentRace->mLevelTerrain->segmentSize);

    v5 = cell2X - cell1X;
    v8 = cell2Y - cell1Y;

    MapEntry *me1 = mParentRace->mLevelTerrain->levelRes->pMap[cell1X][cell1Y];
    MapEntry *me2 = mParentRace->mLevelTerrain->levelRes->pMap[cell1X][cell2Y];
    MapEntry *me3 = mParentRace->mLevelTerrain->levelRes->pMap[cell1X + v5][cell1Y + v8];

    //do_move_colide returns 1 if a collision with a wallsegment occurs
    uint8_t v9 = do_move_colide(position1.X, position1.Y, position2.X, position2.Y, me1);

    if (v8 && !v9) {
        v9 = do_move_colide(position1.X, position1.Y, position2.X, position2.Y, me2);
    }
    if (v5 && !v9) {
        v9 = do_move_colide(position1.X, position1.Y, position2.X, position2.Y, me3);
    }

    return v9;
}

void VTrack::UpdateWaypointHeightInformation() {
    size_t idx;

    for (idx = 1; idx < NextWaypoint; idx++) {
        TrackWaypointList[idx].Position.Z = mParentRace->mVCalc->map_floor(TrackWaypointList[idx].Position);
    }
}

int8_t VTrack::track_initialise_waypoint(uint16_t wp_type, irr::core::vector3df pos1,
                                         irr::core::vector3df pos2) {
    size_t v5;
    uint16_t v12;
    uint16_t v15;
    size_t v10;
    uint16_t nextWaypoint;

    size_t v17;
    uint16_t v20;
    uint16_t v21;
    size_t v18;
    uint16_t v22;
    uint16_t v25;
    uint16_t v28;
    uint16_t peer;
    uint16_t v30;
    uint16_t v31;
    uint16_t v33;
    uint16_t v38;
    uint16_t v36;
    uint16_t v37;
    uint16_t v35;
    uint16_t v42;
    uint16_t v45;
    uint16_t v46;
    uint16_t v47;

    uint16_t pointType;
    int8_t result;

    bool v39;

    v5 = 1;

    if (NextWaypoint <= 1) {
track_initialise_waypoint_LABEL_6:
        v12 = 0;
    } else {
        v10 = 1;
        while (1) {
            if (fabs(TrackWaypointList[v10].Position.X - pos2.X) < 0.00390625) {
               v12 = v5;
               if (fabs(TrackWaypointList[v10].Position.Y - pos2.Y) < 0.00390625) {
                   break;
               }
            }
            ++v5;
            v10 = v5;
            if (v5 >= NextWaypoint) {
                goto track_initialise_waypoint_LABEL_6;
            }
        }
    }

    if (!v12) {
        nextWaypoint = NextWaypoint;
        //can we store one additional waypoint?
        if (nextWaypoint < 0xC8) {
            NextWaypoint = nextWaypoint + 1;
            v12 = nextWaypoint;
        }
        if (!v12) {
            return 0;
        }

        v15 = v12;
        TrackWaypointList[v12].Position = pos2;
        TrackWaypointList[v12].Position.Z =
                mParentRace->mVCalc->map_floor(TrackWaypointList[v12].Position);
        TrackWaypointList[v12].Child = 0;
        TrackWaypointList[v12].PointType = wp_type;
        TrackWaypointList[v12].Peer = 0;
    }

    v17 = 1;

    if (NextWaypoint <= 1) {
track_initialise_waypoint_LABEL_18:
        v20 = 0;
    } else {
        v18 = 1;
        while (1) {
            if (fabs(TrackWaypointList[v18].Position.X - pos1.X) < 0.00390625) {
               v20 = v17;
               if (fabs(TrackWaypointList[v18].Position.Y - pos1.Y) < 0.00390625) {
                   break;
               }
            }
            ++v17;
            v18 = v17;
            if (v17 >= NextWaypoint) {
                goto track_initialise_waypoint_LABEL_18;
            }
        }
    }

    v21 = v20;
    if (v20) {
        TrackWaypointList[v21].Position = pos1;
        if (TrackWaypointList[v21].Child) {
            return 0;
        }

        TrackWaypointList[v21].Child = v12;
        return 1;
    }

    v22 = NextWaypoint;
    //can we store one additional waypoint?
    if (v22 < 200) {
        NextWaypoint = v22 + 1;
        v20 = v22;
    }

    if (!v20) {
        return 0;
    }

    TrackWaypointList[v20].Position = pos1;
    TrackWaypointList[v20].Position.Z =
            mParentRace->mVCalc->map_floor(TrackWaypointList[v20].Position);
    TrackWaypointList[v20].Child = v12;
    TrackWaypointList[v20].PointType = wp_type;
    TrackWaypointList[v20].Peer = 0;

    if (wp_type >= 2) {
        v25 = 1;
        if ((wp_type < 5) || (wp_type == 6)) {
            if (NextWaypoint > 1) {
                v28 = 1;
                while (1) {
                   if (( v25 != v20) &&
                      (mParentRace->mVCalc->distance_get_rough_xy(TrackWaypointList[v20].Position,
                                                 TrackWaypointList[v28].Position) < 1.953125f)) {
                       peer = TrackWaypointList[v20].Peer;
                       v30 = 1;
                       if (TrackWaypointList[v20].Peer) {
                          do {
                              if (peer == v25)  {
                                  v30 = 0;
                              }
                              peer = TrackWaypointList[peer].Peer;
                           } while (peer);
                       }
                       v31 = v25;
                       if (v30) {
                           goto track_initialise_waypoint_LABEL_63;
                       }
                   }
                   ++v25;
                   v28++;
                   if (v25 >= NextWaypoint) {
                       v31 = 0;
                       goto track_initialise_waypoint_LABEL_63;
                   }
                }
            }

            goto track_initialise_waypoint_LABEL_62;
        }
    }

    v33 = 1;
    if (NextWaypoint <= 1) {
track_initialise_waypoint_LABEL_47:
        v38 = 0;
    } else {
        v35 = 1;
        while (1) {
           if (( v33 != v20) &&
              (mParentRace->mVCalc->distance_get_rough_xy(TrackWaypointList[v20].Position,
                                         TrackWaypointList[v35].Position) < 1.953125f)) {
               v36 = TrackWaypointList[v20].Peer;
               v37 = 1;
               if (TrackWaypointList[v20].Peer) {
                  do {
                      if (v36 == v33)  {
                          v37 = 0;
                      }
                      v36 = TrackWaypointList[v36].Peer;
                   } while (v36);
               }
               v38 = v33;
               if (v37) {
                   break;
               }
           }
           ++v33;
           v35++;
           if (v33 >= NextWaypoint) {
               goto track_initialise_waypoint_LABEL_47;
           }
        }
    }

    v39 = (v38 == 0);
    if (v39) {
        return 1;
    }

    pointType = TrackWaypointList[v38].PointType;
    result = 1;

    if (pointType >= 2) {
       v42 = 1;
       if ((pointType < 5) || (result = 1, pointType == 6)) {
           if (NextWaypoint > 1) {
               v45 = 1;
               while (1) {
                  if (( v42 != v20) &&
                     (mParentRace->mVCalc->distance_get_rough_xy(TrackWaypointList[v20].Position,
                                                TrackWaypointList[v45].Position) < 1.953125f)) {
                      v46 = TrackWaypointList[v20].Peer;
                      v47 = 1;
                      if (TrackWaypointList[v20].Peer) {
                         do {
                             if (v46 == v42)  {
                                 v47 = 0;
                             }
                             v46 = TrackWaypointList[v46].Peer;
                          } while (v46);
                      }
                      v31 = v42;
                      if (v47) {
                          break;
                      }
                  }
                  ++v42;
                  v45++;
                  if (v42 >= NextWaypoint) {
                      goto track_initialise_waypoint_LABEL_62;
                  }
               }
track_initialise_waypoint_LABEL_63:
        if (v31) {
            TrackWaypointList[v20].Peer = TrackWaypointList[v31].Peer;
            TrackWaypointList[v31].Peer = v20;
            return 1;
        }
        return 1;
    }
track_initialise_waypoint_LABEL_62:
           v31 = 0;
           goto track_initialise_waypoint_LABEL_63;
    }
  }

  return result;
}

//The original function takes a pointer to a thing, and uses its Position inside
//I want to use a coordinate instead as the input parameter, because I do not
//really have things yet
//Returns the index into the TrackWayPointList array of the entry that
//Was found as the result. Returns value 0 if no correct entry was found
uint16_t VTrack::track_waypoint_absolute_nearest(irr::core::vector3df inputPosition) {
    uint16_t v2 = 0;
    uint16_t v3 = 1;
    uint16_t v5;

    irr::f32 currMinVal;
    bool firstRunDone = false;
    irr::f32 xy;

    if (NextWaypoint > 1) {
        v5 = 1;
        do {
            if (TrackWaypointList[v3].Child) {
                xy = mParentRace->mVCalc->distance_get_xy(TrackWaypointList[v5].Position, inputPosition);
                if (!firstRunDone) {
                    currMinVal = xy;
                    v2 = v3;
                    firstRunDone = true;
                } else {
                    if (xy < currMinVal) {
                        currMinVal = xy;
                        v2 = v3;
                    }
                }
        }
           ++v3;
           v5++;
      } while (v3 < NextWaypoint);
    }

    return v2;
}

irr::f32 VTrack::track_waypoint_distance(irr::core::vector3df position, uint16_t waypointIdx) {
    return mParentRace->mVCalc->distance_get_rough_xy(position, TrackWaypointList[waypointIdx].Position);
}

//The original function takes a pointer to a thing, and uses its Position inside
//I want to use a coordinate instead as the input parameter, because I do not
//really have things yet
//Returns the index into the TrackWayPointList array of the entry that
//Was found as the result. Returns value 0 if no correct entry was found
uint16_t VTrack::track_waypoint_nearest(irr::core::vector3df inputPosition) {
    uint16_t Child = 0;
    uint16_t result;
    uint16_t v3 = 1;
    uint16_t v5;
    uint16_t v12;
    uint16_t v13;
    irr::f32 v10;
    irr::f32 v14;

    bool firstRunDone = false;
    irr::f32 currMinVal;
    irr::f32 xy;

    if (NextWaypoint > 1) {
        v5 = 1;
        do {
            if (TrackWaypointList[v3].Child) {
              xy = mParentRace->mVCalc->distance_get_xy(TrackWaypointList[v5].Position,
                                                    TrackWaypointList[TrackWaypointList[v3].Child].Position);

              if ((mParentRace->mVCalc->distance_get_xy(TrackWaypointList[v5].Position, inputPosition) < xy)
                    && (mParentRace->mVCalc->distance_get_xy(TrackWaypointList[TrackWaypointList[v3].Child].Position,
                        inputPosition) < xy)) {
                  v10 = mParentRace->mVCalc->point_from_line(
                              TrackWaypointList[v3].Position.X,
                              TrackWaypointList[v3].Position.Y,
                              TrackWaypointList[TrackWaypointList[v3].Child].Position.X - TrackWaypointList[v3].Position.X,
                              TrackWaypointList[TrackWaypointList[v3].Child].Position.Y - TrackWaypointList[v3].Position.Y,
                              inputPosition.X,
                              inputPosition.Y
                              );
                  if (!firstRunDone) {
                      currMinVal = v10;
                      Child = TrackWaypointList[v3].Child;
                      firstRunDone = true;
                  } else {
                      if (v10 < currMinVal) {
                          currMinVal = v10;
                          Child = TrackWaypointList[v3].Child;
                      }
                  }
              }
            }

            v3++;
            v5++;
        } while (v3 < NextWaypoint);
    }

    result = Child;

    if (!Child) {
       v12 = 1;
       result = 0;
       if (NextWaypoint > 1) {
           v13 = 1;
           do {
               if (TrackWaypointList[v12].Child) {
                 v14 = mParentRace->mVCalc->distance_get_xy(TrackWaypointList[v13].Position, inputPosition);
                 if (!firstRunDone) {
                     currMinVal = v14;
                     Child = v12;
                     firstRunDone = true;
                 } else {
                     if (v14 < currMinVal) {
                         currMinVal = v14;
                         Child = v12;
                     }
                 }
               }
               ++v12;
               v13++;
           } while (v12 < NextWaypoint);
           return Child;
       }
    }

    return result;
}

uint16_t VTrack::track_waypoint_junction_exists(uint16_t point, uint16_t wp_type) {
    uint16_t peer;
    uint16_t result;
    uint16_t v3;
    bool v5;

    if (!TrackWaypointList[point].Peer) {
        return point;
    }

    peer = TrackWaypointList[point].Peer;
    v3 = peer;
    while (1) {
         v5 = (TrackWaypointList[v3].PointType == wp_type);
         result = peer;
         if (v5) {
             break;
         }
         peer = TrackWaypointList[v3].Peer;
         if (!TrackWaypointList[v3].Peer) {
             return point;
         }
         v3 = peer;
    }

    return result;
}

//First function input parameter is the index into the TrackWaypointList, which
//is the waypoint to investigate.
//Returns the index into the TrackWayPointList array of the entry that
//was found. Returns value 0 if no result was found
uint16_t VTrack::track_waypoint_junction(uint16_t point) {
  uint16_t v1 = 1;
  uint16_t v3;
  uint16_t v6;
  uint16_t idx;
  uint16_t peer;
  uint16_t peer2;
  uint16_t result;

  if (NextWaypoint <= 1) {
      return 0;
  }

  v3 = point;
  for (idx = 1; ; idx += 1) {
      if ((v1 != point) && (mParentRace->mVCalc->distance_get_rough_xy(
                  TrackWaypointList[v3].Position, TrackWaypointList[idx].Position) < 1.953125f)) {
          peer = TrackWaypointList[v3].Peer;
          v6 = 1;
          if (TrackWaypointList[v3].Peer) {
              do {
                  if (peer == v1) {
                      v6 = 0;
                  }
                  peer2 = peer;
                  peer = TrackWaypointList[peer].Peer;
              } while (TrackWaypointList[peer2].Peer);
          }
          result = v1;
          if (v6) {
              break;
          }
      }

      if (++v1 >= NextWaypoint) {
         return 0;
      }
  }

  return result;
}

void VTrack::track_waypoint_position_set(irr::core::vector3df& position, uint16_t waypoint) {
    position = TrackWaypointList[waypoint].Position;
}

uint16_t VTrack::track_waypoint_type(uint16_t waypoint) {
    return TrackWaypointList[waypoint].PointType;
}

uint16_t VTrack::track_waypoint_child(uint16_t waypoint) {
    return TrackWaypointList[waypoint].Child;
}

//The original function takes a pointer to a thing, and uses its Position and
//Colide.Size.Xpos and Colide.Side.Ypos inside
//I want to use a coordinate instead and two floats for Size as the input parameter,
//because I do not really have things yet
//Returns the index into the TrackWayPointList array of the entry that
//Was found as the result. Returns value 0 if no correct entry was found
uint16_t VTrack::track_waypoint_colide_area(irr::core::vector3df position, irr::f32 colideSizeX,
                                    irr::f32 colideSizeY) {
    uint16_t v1 = 1;
    uint16_t result;
    irr::core::vector3df distance;

    if (NextWaypoint <= 1) {
        return 0;
    }

    while(1) {
        if (TrackWaypointList[v1].Child) {
            mParentRace->mVCalc->distance_get_xy_coords(position,
                                        TrackWaypointList[v1].Position, distance);
            if (colideSizeX >= distance.X) {
                result = v1;
                if (colideSizeY >= distance.Y) {
                    break;
                }
            }
        }
        if (++v1 >= NextWaypoint) {
            return 0;
        }
    }

    return result;
}

VTrack::~VTrack() {
}

