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

#include "vthing.h"
#include "../race.h"
#include "../game.h"
#include "../draw/drawdebug.h"
#include "../resources/levelfile.h"
#include "../models/levelterrain.h"
#include "../resources/mapentry.h"
#include "../vanilla/vvehicle.h"

VThingManager::VThingManager(Race* parentRace) {
    mParentRace = parentRace;

    for (int idx = 0; idx < 1000; idx++) {
        ResetThingValues(&Thing[idx]);
    }
}

void VThingManager::ResetThingValues(VThing* whichThing) {
    if (whichThing == nullptr)
        return;

    whichThing->Position.set(0.0f, 0.0f, 0.0f);
    whichThing->Movement.AngleXY = 0.0f;
    whichThing->Movement.AngleXZ = 0.0f;
    whichThing->Movement.AngleZY = 0.0f;
    whichThing->Movement.SpeedActual = 0.0f;
    whichThing->Displacement.set(0.0f, 0.0f, 0.0f);
    whichThing->CollideSize.set(0.0f, 0.0f, 0.0f);

    whichThing->Id = 0;
    whichThing->Index = 0;
    whichThing->Child = 0;
    whichThing->Parent = 0;

    whichThing->AffectStatus = 0;
    whichThing->AffectNumber = 0;
    whichThing->AffectWho = 0;
    whichThing->ColideGroup = 0;
    whichThing->Life = 0;
    whichThing->Seed = 0;
    whichThing->Count = 0;
    whichThing->Status = 0;
    whichThing->Target = 0;
    whichThing->Upgrade = 0;
    whichThing->Member = 0;
    whichThing->Action = 0;
    whichThing->Group = 0;
    whichThing->TimeSlice = 0;
    whichThing->vVehiclePnter = nullptr;
}

VThingManager::~VThingManager() {
}

void VThingManager::thing_delete(VThing* whichThing) {
    whichThing->Status |= 4u;
}

void VThingManager::thing_remove(VThing* whichThing) {
    //call remove function of thing

    mapwho_delete(whichThing);

    //setting Thing group value to 0
    //actually removes/deactivates a not needed
    //thing, until it is reused again
    whichThing->Group = 0;

    int32_t v3 = mParentRace->mLevelRes->mThingFree->Index + 1;
    mParentRace->mLevelRes->mThingFree->Index = v3;

    mParentRace->mLevelRes->mThingFree->Thing[v3] =
            ((int16_t)(whichThing - &Thing[0]));

}

VThing* VThingManager::thing_initialise_member(irr::core::vector3df position,
                                irr::f32 angleXY,
                                irr::f32 angleZY,
                                irr::f32 angleXZ,
                                int8_t group,
                                int8_t member,
                                int16_t id) {
    int32_t v12;
    VThing* v11 = nullptr;

    int32_t index = mParentRace->mLevelRes->mThingFree->Index;
    if (index < 0) {
        //no more free index for creating more Things
        logging::Error("Ran out of free things to create a new thing!");
        return nullptr;
    }

    mParentRace->mLevelRes->mThingFree->Index = (index - 1);
    v11 = &Thing[mParentRace->mLevelRes->mThingFree->Thing[index]];

    if (v11 == nullptr)
        return v11;

    //preinit all thing variables with 0 values
    //original game does this with a memset_0 functional call
    ResetThingValues(v11);

    v12 = ((int32_t)(v11 - &Thing[0]));

    v11->Index = v12;
    v11->Seed = (uint16_t)(v12);
    v11->TimeSlice = v12;
    v11->Id = (v11->Index + 1000);

    //setting a Thing group other then value 0
    //actually makes the thing "alive"
    v11->Group = group;

    //skipped some special code specific to vehicles

    v11->Member = member;
    v11->Action = 0;

    v11->Movement.AngleXY = angleXY;
    v11->Movement.AngleXZ = angleXZ;
    v11->Movement.AngleZY = angleZY;

    mapwho_add(v11, position);

    if (id != -1) {
        v11->Id = id;
    }

    //call init routine of thing

    return v11;
}

VThing* VThingManager::thing_initialise(irr::core::vector3df position,
                                irr::f32 angleXY,
                                irr::f32 angleZY,
                                irr::f32 angleXZ,
                                int8_t group,
                                int8_t member,
                                int16_t id) {

    return thing_initialise_member(position, angleXY, angleZY, angleXZ, group, member, id);
}

int32_t VThingManager::GetNumberThingsUsed() {
    int32_t index = mParentRace->mLevelRes->mThingFree->Index;
    return ((999 - index) - 1);
}

int32_t VThingManager::GetNumberThingsFree() {
    return mParentRace->mLevelRes->mThingFree->Index + 1;
}

//call this function every ~ 50ms! so that the TimeSlice variable
//in the Thing will be advanced. This should occur right after
//the thing was processed as in the original game
void VThingManager::UpdateTimeSlice(VThing* whichThing) {
    //we want to increment TimeSlice every 50ms
    //in the original game it starts counting at 0, increases every 50ms
    //and the overflows back from 0xFF to 00
    if (whichThing->TimeSlice < 0xFF) {
        whichThing->TimeSlice++;
    } else {
        whichThing->TimeSlice = 0;
    }
}

void VThingManager::RunHousekeeping() {
    //Remove Things which are not needed anymore
    size_t nextIndex = (size_t)(mParentRace->mLevelRes->mThingFree->Index);
    int16_t pntrIdx = 0;

    for (size_t idx = 999; idx > nextIndex; idx--) {
        //a thing that can be deleted and is not used
        //anymore has Flag 0x4 set
        pntrIdx = mParentRace->mLevelRes->mThingFree->Thing[idx];

        if (pntrIdx != 0) {
            if ((Thing[pntrIdx].Status & 4) != 0) {
                thing_remove(&Thing[pntrIdx]);
            }
        }
    }
}

void VThingManager::DebugDrawParentInfo() {
    size_t nextIndex = (size_t)(mParentRace->mLevelRes->mThingFree->Index);
    int16_t pntrIdx = 0;
    irr::core::vector3df parentPos;
    irr::core::vector3df startPos;
    irr::core::vector3df irrParentPos;
    irr::core::vector3df irrStartPos;

    for (size_t idx = 999; idx > nextIndex; idx--) {
        pntrIdx = mParentRace->mLevelRes->mThingFree->Thing[idx];

        if (pntrIdx != 0) {
            //does this Thing have a parent?
            if (Thing[pntrIdx].Parent != 0) {
                startPos = Thing[pntrIdx].Position;
                parentPos = Thing[Thing[pntrIdx].Parent].Position;

                irrStartPos = mParentRace->mVCalc->VanillaToIrrlichtCoord(startPos);
                irrParentPos = mParentRace->mVCalc->VanillaToIrrlichtCoord(parentPos);

                mParentRace->mGame->mDrawDebug->Draw3DArrow(irrStartPos, irrParentPos, 0.5f, mParentRace->mGame->mDrawDebug->orange, 1.0f);
            }
        }
    }
}

/***************************************************
 * Other map related stuff                         *
 ***************************************************/

uint8_t VThingManager::mapwho_delete(VThing* whichThing) {
    if ((whichThing->Status & 1) == 0) {
        return 0;
    }

    int mCurrPosCellX;
    int mCurrPosCellY;
    MapEntry* entry = nullptr;

    if (whichThing->Parent) {
        Thing[whichThing->Parent].Child = whichThing->Child;
    } else {
        mCurrPosCellX = (int)(whichThing->Position.X / mParentRace->mLevelTerrain->segmentSize);
        mCurrPosCellY = (int)(whichThing->Position.Y / mParentRace->mLevelTerrain->segmentSize);

        entry = mParentRace->mLevelRes->pMap[mCurrPosCellX][mCurrPosCellY];
        entry->mChild = whichThing->Child;
    }

    if (whichThing->Child > 0) {
        Thing[whichThing->Child].Parent = whichThing->Parent;
    }

    whichThing->Status &= ~1u;
    return 1;
}

uint8_t VThingManager::mapwho_add(VThing* whichThing, irr::core::vector3df position) {
    uint8_t result = 0;
    irr::core::vector3df v8;
    int intPosThingY;
    int intPosThingX;
    int intPosv8Y;
    bool v6;

    v8 = position;

    if ((whichThing->Status & 1) == 0) {
        intPosThingY = (int)(whichThing->Position.Y / mParentRace->mLevelTerrain->segmentSize);
        intPosThingX = (int)(whichThing->Position.X / mParentRace->mLevelTerrain->segmentSize);

        intPosv8Y = (int)(v8.Y / mParentRace->mLevelTerrain->segmentSize);
        //Important: I am note sure about the next 2 if constructs, the look
        //weird!
        if (((intPosThingY - 2) < 7) && (intPosv8Y < 2u) || (intPosv8Y >= 0x9Eu)) {
            v8.Y = 2.0f;
        }

        if ((intPosThingY > 150) && (intPosv8Y >= 0x9Eu)) {
            v8.Y = 157.99609375f;
        }

        whichThing->Position = v8;

        //Update the cell coordinates again!
        intPosThingY = (int)(whichThing->Position.Y / mParentRace->mLevelTerrain->segmentSize);
        intPosThingX = (int)(whichThing->Position.X / mParentRace->mLevelTerrain->segmentSize);
        whichThing->Parent = 0;

        MapEntry* entry = mParentRace->mLevelRes->pMap[intPosThingX][intPosThingY];
        whichThing->Child = entry->mChild;

        v6 = (entry->mChild <= 0);

        if (!v6) {
            Thing[entry->mChild].Parent = whichThing->Index;
        }

        entry->mChild = whichThing->Index;
        result = 1;
        whichThing->Status |= 1u;
    }

    return result;
}

uint8_t VThingManager::mapwho_move(VThing* whichThing, irr::core::vector3df position) {
    uint8_t result;

    if (((int)(whichThing->Position.X) == (int)(position.X))
        && ((int)(whichThing->Position.Y) == (int)(position.Y))) {
        //The Thing does not leave the current Terrain cell
        whichThing->Position = position;
        result = 0;
        whichThing->Status &= ~0x40;
    } else {
        //The Thing changes the current cell
        mapwho_delete(whichThing);
        mapwho_add(whichThing, position);
        result = 1;
        whichThing->Status |= 0x40;
    }

    return result;
}

uint8_t VThingManager::thing_overlapping(VThing* thing1, VThing* thing2) {
    uint8_t result = 0;
    irr::f32 v7;
    irr::f32 v9;
    irr::f32 v10;
    irr::f32 v11;
    irr::f32 v12;
    irr::f32 zPos;

    irr::f32 v4 = fabs(thing1->Position.X - thing2->Position.X);
    bool v5 = (v4 >= (thing1->CollideSize.X + thing2->CollideSize.X));

    if (!v5) {
        v7 = fabs(thing1->Position.Y - thing2->Position.Y);
        v5 = (v7 >= (thing1->CollideSize.Y + thing2->CollideSize.Y));
        result = 0;
        if (!v5) {
           zPos = thing1->CollideSize.Z;
           v9 = thing2->CollideSize.Z;
           v10 = zPos + v9;
           v11 = thing1->Position.Z + zPos;
           v12 = thing2->Position.Z + v9;
           if ((v11 - v12) < 0.0f) {
               return ((v12 - v11) < v10);
           } else {
               return ((v11 - v12) < v10);
           }
        }
    }

    return result;
}

//whichThing is the Effect-Thing that affects player
//vehicles
uint8_t VThingManager::affect_thing(VThing* whichThing) {
    int16_t thingIdx;
    VThing* v11;
    int32_t v6;
    uint16_t id;

    if (!AffectListIndex) {
        return 0;
    }

    for (thingIdx = AffectList[AffectListIndex]; AffectListIndex > 0 ; thingIdx = AffectList[AffectListIndex]) {
        v11 = &Thing[thingIdx];
        if (AffectListIndex < 0) {
            break;
        }
        if (!v11->AffectWho) {
            v11->AffectWho = (uint16_t)(whichThing->Id);
        }
        v11->AffectNumber += whichThing->AffectNumber;
        v6 = (v11->Member | (v11->Group << 16));
        v11->AffectStatus |= whichThing->AffectStatus;
        //is it a Vehicle (Group = 10), and a player (Member = 9)?
        //in the original game this makes sure no repair vehicle or
        //other Thing is hit. In my implementation this can not happen anyway
        if (v6 == 655360) {
            //Explaination for contents of whichThing-Id at this point: The Id in the vehicleThings is changed so that
            //it reflects the number of the player, first player has Id = 1, second player has Id = 2 and so
            //on; Then this Id is always transfered to the child objects; For example the MachineGun Id will
            //also have the same value. And if a bullet is fired the bullet has the same Id again that
            //reflects the player that has fired the shot. This is very important because at the end (exactly here at
            //this code location) when the bullet/effect hits another player this bullet Id is then used for the targeted player
            //to remember which player has targeted him how often.
            id = static_cast<uint16_t>(whichThing->Id);
            if ((id - 1) < 8) {
                ++v11->vVehiclePnter->AutoTarget.HitMeTrigger[id-1];
            }
        }
        AffectListIndex -= 1;
    }

    return 1;
}

//effect is the Effect-Thing that affects player
//vehicles
int16_t VThingManager::effect_affect_vehicle_exclusive(VThing* effect) {
    AffectListIndex = 0;

    std::vector<VVehicle*>::iterator it;

    for (it = mParentRace->mVanillaCraftVec.begin(); it != mParentRace->mVanillaCraftVec.end(); ++it) {
        if ((!(*it)->ThingData->Member) && (effect->Id != (*it)->ThingData->Id) && thing_overlapping(effect, (*it)->ThingData)) {
            AffectListIndex = AffectListIndex + 1;
            AffectList[AffectListIndex] = (*it)->ThingData->Index;
        }
    }

    return AffectListIndex;
}

int16_t VThingManager::thing_touching_anything(VThing* whichThing) {
    AffectListIndex = 0;

    //First part: for player vehicles
    std::vector<VVehicle*>::iterator it;

    for (it = mParentRace->mVanillaCraftVec.begin(); it != mParentRace->mVanillaCraftVec.end(); ++it) {
        if ((!(*it)->ThingData->Member) && (whichThing->Id != (*it)->ThingData->Id) && thing_overlapping(whichThing, (*it)->ThingData)) {
            AffectListIndex = AffectListIndex + 1;
            AffectList[AffectListIndex] = (*it)->ThingData->Index;
        }
    }

    //Second part: For Things in Group 8, and Member = 3
    std::vector<VThing*>::iterator it2;

    for (it2 = mParentRace->mGroup8ThingsVec.begin(); it2 != mParentRace->mGroup8ThingsVec.end(); ++it2) {
        if (((*it2)->Member == 3) && (whichThing->Id != (*it2)->Id) && thing_overlapping(whichThing, (*it2))) {
            AffectListIndex = AffectListIndex + 1;
            AffectList[AffectListIndex] = (*it2)->Index;
        }
    }

    return AffectListIndex;
}
