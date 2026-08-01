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

#include "vvehicle.h"
#include "../race.h"
#include "../game.h"
#include "../vanilla/vcalc.h"
#include "../draw/drawdebug.h"
#include "../vanilla/vtrack.h"
#include "../vanilla/vcamera.h"
#include "debug/dbginterface.h"
#include "debug/memdump.h"
#include "debug/structs/thing.h"
#include "debug/structs/thingvehicle.h"
#include "debug/structs/cam.h"
#include "debug/datatools.h"
#include "../utils/boundingbox/collision.h"
#include "../models/collectable.h"
#include "../audio/sound.h"
#include "../models/collectablespawner.h"
#include "spritething.h"
#include "../resources/texture.h"
#include "../models/levelterrain.h"
#include "../draw/hud.h"
#include "../models/particle.h"
#include "../resources/mapentry.h"

//according to the emulator this function is supposed
//to run periodically every ~45 ms
void VVehicle::processWeaponBooster() {
    int32_t v12;
    bool v8;
    bool v9;

    if (!Booster.Trigger || Booster.TriggerRestrictionCount ||
        (Booster.TriggerTime >= 100) || (Stats.Weapons < 6) || (Stats.Health < 6))
    {
        if ((Booster.TriggerTime > 0) && (!FlightModel.Flag.Airbourn)) {
            if (!Booster.TriggerRestrictionCount) {
                if (TurboSound != nullptr) {
                    TurboSound->stop();
                    TurboSound = nullptr;
                }

                //we can activate the booster
                mRace->mSoundEngine->PlaySound(SRES_GAME_BOOSTER);
                ThingData.AffectStatus |= 0x40u;
            }

            v12 = static_cast<uint16_t>(Booster.TriggerTime) - 1;
            Booster.TriggerTime = static_cast<int16_t>(v12);

            if ((v12 << 16) > 0) {
                ++Booster.TriggerRestrictionCount;
            } else {
                Booster.TriggerRestrictionCount = 0;
            }
        }

        Booster.Trigger = 0;
    } else {
        if (!Booster.Upgrade || (v8 = Booster.Upgrade < 0, v9 = Booster.Upgrade < 4, !v8) && v9) {
           Stats.Weapons -= 3;
           Stats.Fuel -= 3;
        }

        Conditions.FuelUsed += 3;
        Conditions.WeaponsUsed += 3;
        Booster.TriggerTime += (2 + Booster.Upgrade);
        //sample_play(v4, 10);
        //sample_set_pitch(v4, 10, 2 * Booster.TriggerTime + 100);

        if (TurboSound == nullptr) {
            switch (Booster.Upgrade) {
                case 0: {
                    TurboSound = mRace->mSoundEngine->PlaySound(SRES_GAME_TURBO, 0.5714f, false);
                    break;
                }
                case 1: {
                    TurboSound = mRace->mSoundEngine->PlaySound(SRES_GAME_TURBO, 0.8955f, false);
                    break;
                }
                case 2: {
                    TurboSound = mRace->mSoundEngine->PlaySound(SRES_GAME_TURBO, 1.25f, false);
                    break;
                }
                case 3: {
                    TurboSound = mRace->mSoundEngine->PlaySound(SRES_GAME_TURBO, 1.6666f, false);
                    break;
                }
                default: {
                    TurboSound = mRace->mSoundEngine->PlaySound(SRES_GAME_TURBO, 1.0f, false);
                    break;
                }
            }
        } else {
            irr::f32 pitch;
            switch (Booster.Upgrade) {
                case 0: {
                   pitch = (irr::f32)((2.0f * (irr::f32)(Booster.TriggerTime) + 200.0f) / 350.0f);
                   break;
                }
                case 1: {
                   pitch = (irr::f32)((2.25f * (irr::f32)(Booster.TriggerTime) + 300.0f) / 335.0f);
                   break;
                }
                case 2: {
                   pitch = (irr::f32)((2.35f * (irr::f32)(Booster.TriggerTime) + 400.0f) / 320.0f);
                   break;
                }
                case 3: {
                   pitch = (irr::f32)((2.5f * (irr::f32)(Booster.TriggerTime) + 500.0f) / 300.0f);
                   break;
                }
                default: {
                   pitch = 1.0f;
                   break;
                }
            }

            TurboSound->setPitch(pitch);

            if (TurboSound->getStatus() == sf::SoundSource::Status::Stopped) {
                TurboSound = nullptr;
            }
        }

        Booster.Trigger = 0;
    }

    //we need to stop Turbo sound in case the booster
    //key is released somewhere mid way
    if (!KeyPressedBooster) {
        if (TurboSound != nullptr) {
            TurboSound->stop();
            TurboSound = nullptr;
        }
    }
}

void VVehicle::TestCamera() {
    mRace->mVDbgInterface->Init(std::string("angle/angle2.bin"), std::string(""), std::string("extract/level0-1/level0-1-unpacked.dat"));
    mRace->mVDbgInterface->SetVehicleStatePlayerFromMemDump(*this, mRace->mVDbgInterface->newDump);

    mRace->mVDbgInterface->newDump->ReadEngineCamera();
      mRace->mGame->mSmgr->setActiveCamera(mRace->vanTestCam);

    mRace->mVDbgInterface->newDump->EngineCamera->Print();

    mRace->mVCamera->SetIrrlichtCamera(mRace->vanTestCam, mRace->mVDbgInterface->newDump->EngineCamera);
    mRace->mGame->StopTime();
}

bool VVehicle::ShouldAmmoBarBlink() {
    return mLowAmmoWarningAlreadyShown;
}

bool VVehicle::ShouldFuelBarBlink() {
    return (mLowFuelWarningAlreadyShown || mEmptyFuelWarningAlreadyShown);
}

bool VVehicle::ShouldShieldBarBlink() {
    return mLowShieldWarningAlreadyShown;
}

void VVehicle::SetName(char* playerName) {
    strcpy(this->Stats.name, playerName);
}

void VVehicle::UpdateEngineSound() {
    irr::f32 vel = Stats.Velocity;
    irr::f32 pitch;

    if (vel >= 0.70703125f) {
        vel = 0.703125f;
    }

    //highest possible value for pitch seems to be
    //1.90909090909090909091f
    //add a very small number to pitch, so that
    //pitch does not get 0, because it seems SFML ignores
    //a pitch parameter exactly with zero value
    pitch = ((300.0f * vel) / (110.0f)) + 0.01f;

    //if this players camera is currently selected to be followed
    //set engine sound to be non spatial, otherwise we get a weird directional
    //sound effect when the player rotates around his axis
    if (this->mRace->currPlayerFollow == this) {
        this->mRace->mSoundEngine->UpdateVehicleState(this, pitch);
    } else {
        //is not the main player (player that we follow right now)
        //use spatial engine sound

        //we need to take the vanilla game coordinates, and convert them to my 3D Irrlicht coordinate system
        //because also the sound will use the Irrlicht Coordinate system
        irr::core::vector3df irrPos = mRace->mVCalc->VanillaToIrrlichtCoord(ThingData.Position);

        this->mRace->mSoundEngine->UpdateVehicleState(this, pitch, irrPos);
    }
}

bool VVehicle::AllowedToCollectPowerUp() {
    //Player is only allowed to collect powerUps if
    //Action == 1 is set (means normal racing mode)
    return (ThingData.Action == 0x1);
}

//Is called initially once after race start to further initialize the
//vehicle. Call happens because the initial reset value of the ThingData.Action
//is zero
void VVehicle::vehicle_execute_action0x0_initialize() {
    irr::core::vector3df position;
    uint32_t status;
    uint32_t v21;

    /* I left quite some code out here, because I believe this code is searching
     * for start positions in the map and I want to do this somewhere else */

    position = ThingData.Position;
    position.Z = mRace->mVCalc->map_altitude_lowest(position);

    //mapwho_move(thing, &position);
    //below: currently the alternative
    ThingData.Position = position;

    FlightModel.FrontLeft.Zpos = ThingData.Position.Z;
    FlightModel.FrontRight.Zpos = ThingData.Position.Z;
    FlightModel.RearLeft.Zpos = ThingData.Position.Z;
    FlightModel.RearRight.Zpos = ThingData.Position.Z;
    ThingData.Movement.AngleXZ = 0.0f;
    ThingData.Movement.AngleXY = 0.0f;
    ThingData.Movement.AngleZY = 0.0f;
    Momentum.AngleXY = 0.0f;
    Momentum.DeltaX = 0.0f;
    Momentum.DeltaY = 0.0f;
    //Bump.Xpos = 0.0f;
    //Bump.Ypos = 0.0f;
    Stats.Health = 10000;
    Stats.Fuel = 10000;
    Stats.Weapons = 10000;

    //is weird code below, but this is from the original
    //--Machinegun.Upgrade;
    //--Missile.Upgrade;
    --Booster.Upgrade;

    // if (Machinegun.Upgrade < 0) {
    //     Machinegun.Upgrade = 0;
    // }

    // if (Missile.Upgrade < 0) {
    //     Missile.Upgrade = 0;
    // }

    if (Booster.Upgrade < 0) {
         Booster.Upgrade = 0;
    }

    ThingData.AffectStatus |= 0x80u;

    //Craft controlled by computer player?
    if ((ControlOrigin & 8) != 0) {
        status = ThingData.Status;
        v21 = (status | 8);
        if ((status & 8) != 0)
        {
vehicle_execute_action0_initialize_LABEL_23:
            ThingData.Status = v21;
            goto vehicle_execute_action0_initialize_LABEL_107;
        }
        if (!ThingData.mTimeSlice) {
            v21 = (status | 8);
            goto vehicle_execute_action0_initialize_LABEL_23;
        }
    } else {
vehicle_execute_action0_initialize_LABEL_107:
        ThingData.Action = 0x1;
    }
}

//This is the default action of the vehicle while racing, and
//under the assumption currently nothing special happens
//otherwise
void VVehicle::vehicle_execute_action0x1_defaultracing() {
    /**********************************
     * Did we run out of fuel?        *
     * Call the repair vehicle?       *
     **********************************/

    if (Stats.Fuel <= 0) {
        //reduce counter inside Vehicle Thing
        //I guess if the counter expires until fuel is restablished
        //we call the repair vehicle
        ThingData.Count -= 1;

        if (ThingData.Count <= 0) {
            ThingData.Count = 200;
        } else if (ThingData.Count == 1) {
            //Set flag that we are out of Fuel
            FlightModel.Flag.FuelDeath = true;
            ThingData.Action = 0x14;
            ThingData.Count = 0;
        }
    }

    /**********************************
     * Did we run out of health?      *
     * Tumble and Explode?            *
     **********************************/

    //lower health?
    if (Stats.Health < 3001) {
        //are we completely out of health?
        if (Stats.Health <= 0) {
            //next lines seem to create a thing that damage craft permanently at this place?
            //v26 = thing_initialise(&thing->Position, &thing->Movement.Angle, 2, 2, thing->Id);
            // if (v26) {
            //   v26->Colide.Group = 0;
            // }

            ThingData.Action = 0x9;
            vehicle_setup_tumble();
            FlightModel.Flag.HealthDeath = true;
        } else {
            //Health is low, but still some health is remaining
            //create some smoke behind the craft

            //TODO: Add the smoke effect later
        }
    }

    if (FlightModel.FunctionFlag.Pad3) {
        vehicle_control();
    }
}

//This action happens always shortly before exploding of the craft
//This action means the BarrelRoll of the craft takes place
void VVehicle::vehicle_execute_action0x9_beforeexploding() {
    int16_t v29;

    if (FlightModel.Flag.BarrelRoll) {
        v29 = Tumble.Count;
        Tumble.Count = v29 - 1;
        if (v29 < 2) {
            FlightModel.Flag.BarrelRoll = false;
        }
    }

    //done with the BarrelRoll?
    if (!FlightModel.Flag.BarrelRoll) {
        ThingData.Action = 0x11;
    }

    vehicle_set_camera();
    vehicle_get_checkpoint();
}

//During this action the vehicle spawn powerups before
//exploding
void VVehicle::vehicle_execute_action0x11_spawnpowerups() {
    //the initialization values of this variables
    //define how many powerups are spawned
    int32_t v31 = 4000;
    int32_t v32 = 4000;
    int32_t v37 = 4000;

    //25.07.2026: The original game does only spawn powerups if a
    //human player is close to the vehicle that wants to spawn
    //Implement later the same way?

    //if a human is close is periodically checked in method
    //"thing_check_for_human_in_sight" for vehicles
    //if (ThingData.Status & 0x400) != 0) {

    std::vector<Entity::EntityType> powerUpList;
    powerUpList.clear();

    do {
        powerUpList.push_back(Entity::ExtraShield);
        v31 -= 1000;
    } while (v31 >= 1001);

    do {
        powerUpList.push_back(Entity::ExtraFuel);
        v32 -= 1000;
    } while (v32 >= 1001);

    do {
        powerUpList.push_back(Entity::ExtraAmmo);
        v37 -= 1000;
    } while (v37 >= 1001);

    //Add later when Minigun exists
    //if (Minigun.Upgrade) {
    //    //Spawn Minigun Upgrade
    //}

    //Add later when Missile exists
    //if (Missile.Upgrade) {
    //    //Spawn Missile Upgrade
    //}

    if (Booster.Upgrade) {
        powerUpList.push_back(Entity::BoosterUpgrade);
    }

    mRace->SpawnCollectiblesForPlayer(this, powerUpList);

    //}

    ThingData.Action = 0x13;
    ThingData.Count = 5;
}

void VVehicle::vehicle_execute_action0x13_exploding() {
    int16_t v47;

    //TODO: add effects here

    vehicle_get_checkpoint();
    v47 = ThingData.Count - 1;
    ThingData.Count = v47;
    if (v47 < 0) {
        ThingData.Action = 0x14;
        ThingData.Count = 0;
    }
}

//action0x14 is the vehicle action that calls for
//a recovery vehicle for help. This state can only be exit when a
//recovery vehicle is there to help, and advances the action of this
//vehicle forward to action 0x16
void VVehicle::vehicle_execute_action0x14_callAndWaitForRecoveryVehicle() {
    irr::core::vector3df position;
    uint8_t timeslice;

    ThingData.Movement.AngleXZ *= 0.9765625f;
    ThingData.Movement.AngleZY *= 0.9765625f;
    Momentum.DeltaX *= 0.9375f;
    Momentum.DeltaY *= 0.9375f;
    MovementInput.AngleXY = 0.0f;
    MovementInput.SpeedActual = 0.0f;

    if (!FlightModel.Flag.HealthDeath) {
        vehicle_get_checkpoint();
        return;
    }

    //HealthDeath Flag is set, which means we had no health
    //anymore and therefore this vehicle is now stuck
    MovementInput.AngleXY = 0.0f;
    MovementInput.AngleXZ = 0.0f;
    MovementInput.AngleZY = 0.0f;
    MovementInput.SpeedActual = 0.0f;

    position.X = ThingData.Position.X;
    position.Y = ThingData.Position.Y;
    position.Z = ThingData.Position.Z;
    timeslice = ThingData.mTimeSlice;

    if ((timeslice & 3) == 0) {
        //Emit more smoke
    }

    position.X += 0.078125f;

    if ((ThingData.mTimeSlice & 7) == 0) {
        //Emit more smoke
    }

    vehicle_get_checkpoint();
    return;
}

//Recovery vehicle advances this vehicles action
//forward to 0x16 when it starts to help
void VVehicle::vehicle_execute_action0x16() {
    irr::core::vector3df position;
    uint8_t timeslice;

    Increment.AngleXZ = 0.0f;
    MovementInput.AngleXY = 0.0f;
    MovementInput.AngleZY = 0.0f;
    MovementInput.SpeedActual = 0.0f;

    Momentum.DeltaX *= 0.8984375f;
    Momentum.DeltaY *= 0.8984375f;

    position.X = ThingData.Position.X;
    position.Y = ThingData.Position.Y;
    position.Z = ThingData.Position.Z;

    if (!FlightModel.Flag.HealthDeath) {
        vehicle_get_checkpoint();
        return;
    }

    timeslice = ThingData.mTimeSlice;

    if ((timeslice & 3) == 0) {
        //Emit more smoke
    }

    position.X += 0.078125f;

    if ((ThingData.mTimeSlice & 7) == 0) {
        //Emit more smoke
    }

    vehicle_get_checkpoint();
}

void VVehicle::vehicle_execute_action0x17_rescue() {
    ThingData.Movement.AngleZY *= 0.9765625f;
    ThingData.Movement.AngleZY *= 0.984375f;

    if (FlightModel.Flag.HealthDeath) {
        //we want to reposition the craft
        //with the repair vehicle
        FlightModel.Flag.Reposition = true;

        ThingData.Movement.SpeedActual = 0.0f;
        Increment.SpeedActual = 0.0f;
        Stats.Velocity = 0.0f;

        FlightModel.Flag.AutoRearm = false;
        FlightModel.Flag.AutoRepair = false;
        FlightModel.Flag.AutoRefuel = false;

        if (Stats.Health < 10000) {
            Stats.Health += 1000;
            FlightModel.Flag.AutoRepair = true;
            //sample_play(v66, 11);

            vehicle_get_checkpoint();
            return;
        }

        if (Stats.Fuel < 10000) {
            Stats.Fuel += 1000;
            FlightModel.Flag.AutoRefuel = true;
            //sample_play(v66, 11);

            vehicle_get_checkpoint();
            return;
        }

        if (Stats.Weapons < 10000) {
            Stats.Weapons += 1000;
            FlightModel.Flag.AutoRearm = true;
            //sample_play(v66, 11);

            vehicle_get_checkpoint();
            return;
        }

        //sample_stop(thing, 11);
    } else {
        Stats.Fuel += 2000;
    }

    ThingData.Action = 0x18;
    vehicle_get_checkpoint();
}

void VVehicle::vehicle_execute_action0x18() {
    if (FlightModel.Flag.HealthDeath) {
        Damage.BulletCount = 0;
        Damage.BulletHoles = 0;
        Damage.MissileCount = 0;
    }

    vehicle_get_checkpoint();
}

//Seems to be a reset function of the vehicle in case we get
//dropped of by the repair vehicle after a repair action
//or air refueling took place
void VVehicle::vehicle_execute_action0x19_reset() {
    if (FlightModel.Flag.HealthDeath) {
        //is weird code below, but this is from the original
        //--Machinegun.Upgrade;
        //--Missile.Upgrade;
        --Booster.Upgrade;

        // if (Machinegun.Upgrade < 0) {
        //     Machinegun.Upgrade = 0;
        // }

        // if (Missile.Upgrade < 0) {
        //     Missile.Upgrade = 0;
        // }

        if (Booster.Upgrade < 0) {
             Booster.Upgrade = 0;
        }

        ThingData.AffectStatus = 0;
        ThingData.AffectNumber = 0;
        ThingData.AffectWho = 0;

        ThingData.AffectStatus |= 0x80u;
    }

    FlightModel.Flag.Reposition = false;
    FlightModel.Flag.AutoRearm = false;
    FlightModel.Flag.AutoRepair = false;
    FlightModel.Flag.AutoRefuel = false;

    FlightModel.FrontLeft.Zpos = ThingData.Position.Z;
    FlightModel.FrontRight.Zpos = ThingData.Position.Z;
    FlightModel.RearLeft.Zpos = ThingData.Position.Z;
    FlightModel.RearRight.Zpos = ThingData.Position.Z;

    Momentum.DeltaX = 0.0f;
    Momentum.DeltaY = 0.0f;

    FlightModel.Flag.HealthDeath = false;
    FlightModel.Flag.FuelDeath = false;
    ThingData.Count = 0;
    Stats.Weight = 0;

    ThingData.Action = 0x1;
}

void VVehicle::vehicle_do_action() {
    switch (ThingData.Action) {
        case 0: {
            vehicle_execute_action0x0_initialize();
            return;
        }

        case 1: {
            vehicle_execute_action0x1_defaultracing();
            return;
        }

        case 0x9: {
            vehicle_execute_action0x9_beforeexploding();
            return;
        }

        case 0x11: {
            vehicle_execute_action0x11_spawnpowerups();
            return;
        }

        case 0x13: {
            vehicle_execute_action0x13_exploding();
            return;
        }

        case 0x14: {
            vehicle_execute_action0x14_callAndWaitForRecoveryVehicle();
            return;
        }

        case 0x16: {
            vehicle_execute_action0x16();
            return;
        }

        case 0x17: {
            vehicle_execute_action0x17_rescue();
            return;
        }

        case 0x18: {
            vehicle_execute_action0x18();
            return;
        }

        case 0x19: {
            vehicle_execute_action0x19_reset();
            return;
        }

        default: {
            return;
        }
    }
}

void VVehicle::Update(irr::f32 frameDeltaTime) {
    //we want to increment mTimeSlice every 50ms
    //in the original game it starts counting at 0, increases every 50ms
    //and the overflows back from 0xFF to 00
    mAbsTimeIntegrator += frameDeltaTime;
    if (mAbsTimeIntegrator >= 0.05) {
        mAbsTimeIntegrator = 0.0f;
        if (ThingData.mTimeSlice < 0xFF) {
            ThingData.mTimeSlice++;
        } else {
            ThingData.mTimeSlice = 0;
        }

        //should run every ~45ms
        //timing close enough when called
        //here
        processWeaponBooster();

        //do not update engine sound for the first ~300ms
        //of the race to prevent hearing the first height drop
        if (!mUpdateEngineSound) {
            if (ThingData.mTimeSlice >= 4) {
                mUpdateEngineSound = true;
            }
        }

        //This function should be called every
        //50ms
        if (mUpdateEngineSound) {
            UpdateEngineSound();
        }

        //add later, seems to be needed
        //ClosestMissile = 0;
        vehicle_do_action();
    }

    mUpdateVehicleTimeIntegrator += frameDeltaTime;
    if (mUpdateVehicleTimeIntegrator >= 0.05) {
        //the original game does not use DeltaTime which
        //means depending on the speed the CPU is running the
        //game speed completely changes, which is not a good thing
        //I want to improve this by adding DeltaTime correction based on
        //the game timing of the Playstation 1 game
        //This version of the game runs the following functions every ~50ms;
        //If we run them more often (which will be the case most of the time)
        //we need to correct certain flight model parameters by a correction
        //factor we calculate below.
        mDeltaTimeFactor = (mUpdateVehicleTimeIntegrator / 0.05);
        mUpdateVehicleTimeIntegrator = 0.0f;

        irr::core::vector3df delta;

        //Note 25.07.2026: We can only execute the physics model if we are not
        //currently repositioning the craft by other means
        if (!FlightModel.Flag.Reposition) {
            //if (mRace->AdvModel) {
               //mRace->mVDbgInterface->Init(std::string("camera.bin"), std::string(""), std::string("extract/level0-1/level0-1-unpacked.dat"));
               //ParseThing* thing = mRace->mVDbgInterface->newDump->ReturnThingFirstPlayer();
               //mRace->mVDbgInterface->newDump->ThingVehicle->Update(mRace->mVDbgInterface->mDumpLevelStructStart + 0x40B3C + thing->VehicleIndex->mRawValue * 0x1F0);

                // mRace->mVDbgInterface->Init(std::string("testfile.bin"), std::string(""), std::string("extract/level0-1/level0-1-unpacked.dat"));
                //mRace->mVDbgInterface->Init(std::string("beforeangle.bin"), std::string("afterangle.bin"), std::string("extract/level0-1/level0-1-unpacked.dat"));


                //mRace->mVDbgInterface->CompareVehicleStateBetweenMemDumps(mRace->mVDbgInterface->newDump, mRace->mVDbgInterface->newDump2);

                //Playstation 1 emulator executes this vehicle update loop average every 1721634 CPU cycles
                //CPU clocks at 33.8688 MHz, which means lets run this loop every ~50.83 ms

               /* mRace->mVDbgInterface->Init(std::string("coll.bin"), std::string(""), std::string("extract/level0-1/level0-1-unpacked.dat"));
                ParseThing* thing = mRace->mVDbgInterface->newDump->ReturnThingFirstPlayer();
                mRace->mVDbgInterface->newDump->ThingVehicle->Update(mRace->mVDbgInterface->mDumpLevelStructStart + 0x40B3C + thing->VehicleIndex->mRawValue * 0x1F0);

                mRace->mVDbgInterface->SetVehicleStatePlayerFromMemDump(*this, mRace->mVDbgInterface->newDump);*/

                vehicle_get_track_friction();
                vehicle_calculate_angle();

                //mRace->mVDbgInterface->CompareVehicleStatePlayerWithMemDump(*this, mRace->mVDbgInterface->newDump2);

               //  mRace->mVDbgInterface->SetVehicleStatePlayerFromMemDump(*this, mRace->mVDbgInterface->newDump);

                vehicle_calculate_thrust(delta);
                mRace->mVCalc->move_displacement_slope(ThingData.Position, Slope);

                vehicle_calculate_momentum(delta);
                vehicle_calculate_movement_delta(delta);
                vehicle_colide_map(delta);
                vehicle_colide_vectors(delta);
                //Note: 14.06.2026: I can only assume that at one point in time
                //the developers tried to do a different kind of collision detection
                //using the function vehicle_colide below (maybe it needed to much performance?)
                //Unfortunetly I accidently did implement it, but then found out that the final original
                //game does not use it at all anymore. Instead the original game does use
                //"vehicle_colide_final_check_sean". I commented my implementation out which is currently
                //not tested, because I can not compare and debug it with the original game as a reference.
                //vehicle_colide(mRace->mVanillaCraftVec, delta);
                //vehicle_colide_final_check_sean(mRace->mVanillaCraftVec, delta);
                //vehicle_colide_my_attempt(mRace->mVanillaCraftVec, delta);
                vehicle_do_tumble();

                vehicle_move_altitude(delta);
                vehicle_move_roll(delta);
                vehicle_move_tilt(delta);
                vehicle_move_mapwho(delta);
                vehicle_set_camera();
                vehicle_post_process();
                //vehicle_terrain_effect(delta);

               // mRace->AdvModel = false;
        }

        UpdateSceneNode();
        UpdateCoordinates();
    }

    //check if player entered a craft trigger region
    CheckForTriggerCraftRegion();

    //check if player is in an charging station
    CheckForChargingStation();

    CheckDustCloudEmitter();

    mDustBelowCraft->Update(frameDeltaTime);
}

void VVehicle::vehicle_terrain_effect(irr::core::vector3df delta) {
    irr::core::vector3df position;
    uint16_t v4;
    uint16_t v5;
    bool v6;
    uint16_t v7;
    uint16_t v8;
    int32_t v9;
    EffectVehicleThing* newEffectThing;

    position = ThingData.Position + delta;
    //we only create the vehicle terrain effects when
    //we are close to the ground and not airbourne right now
    if (!FlightModel.Flag.Airbourn) {
        //we also need some speed
        if (fabs(ThingData.Movement.SpeedActual) > 0.00390625f) {
           v4 = mRace->mVCalc->map_colide_type(position);
           v5 = v4;
           v6 = (v4 == 0);
           v7 = (v4 & 1);
           if (!v6) {
              v6 = (v7 == 0);
              v8 = (v5 & 2);
              if (!v6) {
                 v8 = (v5 & 2);
                 //Status Flag 0x400 is set when the Thing is visible
                 //to a human player (means a human player is close enough distance wise);
                 //If it is not visible (humans are all far away) then this flag is not set,
                 //an no sprites will be created for this thing during this time.
                 //Is an optimization technique
                 //if ((thing->Status & 0x400) != 0) {
                    v9 = 0;
                    do {
                        newEffectThing = new EffectVehicleThing(mRace->mGame->mSmgr,
                                            mRace,
                                            mRace->mTexLoader->spriteTex.at(17),
                                            ThingData.Position, ThingData.Movement);
                        ++v9;
                        if (newEffectThing != nullptr) {
                            newEffectThing->ThingData.Displacement.X = delta.X;
                            newEffectThing->ThingData.Displacement.Y = delta.Y;
                            newEffectThing->ThingData.Life = 150;

                            mRace->AddSpriteThing(newEffectThing);
                        }
                    } while (v9 < 1 /*4*/);
                    v8 = (v5 & 2);
              }
           }
        }
    }
}

irr::core::vector3df VVehicle::IrrCoordGetDustEmitterPosition() {
    return IrrWorldCraftDustPnt;
}

void VVehicle::UpdateCoordinates() {
    this->mCraftNode->updateAbsolutePosition();
    irr::core::matrix4 trans = this->mCraftNode->getAbsoluteTransformation();

    IrrWorldCraftFrontPnt = IrrLocalCraftFrontPnt;
    trans.transformVect(IrrWorldCraftFrontPnt);

    IrrWorldCraftBackPnt = IrrLocalCraftBackPnt;
    trans.transformVect(IrrWorldCraftBackPnt);

    IrrWorldCraftLeftPnt = IrrLocalCraftLeftPnt;
    trans.transformVect(IrrWorldCraftLeftPnt);

    IrrWorldCraftRightPnt = IrrLocalCraftRightPnt;
    trans.transformVect(IrrWorldCraftRightPnt);

    IrrWorldCraftOrigin = IrrLocalCraftOrigin;
    trans.transformVect(IrrWorldCraftOrigin);

    IrrWorldCraftTriggerSensor = IrrLocalCraftTriggerSensor;
    trans.transformVect(IrrWorldCraftTriggerSensor);

    IrrWorldCraftDustPnt  = IrrLocalCraftDustPnt;
    trans.transformVect(IrrWorldCraftDustPnt);

    IrrWorldDirVecForward = (IrrWorldCraftFrontPnt - IrrWorldCraftBackPnt).normalize();
}

void VVehicle::SetupFlightModelConstants() {
    //setting up flight model constants
    //All values configured as in function initialiseVEHICLE_CAR in original game
    IncrementAdd.AngleXY = mRace->mVCalc->VanillaRawAngleToMyFloatingAngle(200);
    IncrementAdd.SpeedActual = mRace->mVCalc->FixedPointToFloat8D8(26);
    IncrementSub.AngleXY = mRace->mVCalc->VanillaRawAngleToMyFloatingAngle(100);
    IncrementSub.SpeedActual = mRace->mVCalc->FixedPointToFloat8D8(160);
    IncrementLimit.AngleXY = mRace->mVCalc->VanillaRawAngleToMyFloatingAngle(3640);
    IncrementLimit.SpeedActual = mRace->mVCalc->FixedPointToFloat8D8(240);

    mThrustEffectiveness = 100;
    mSideslipFriction = mRace->mVCalc->FixedPointToFloat8D8(7);
    mSideslipToThrust = mRace->mVCalc->FixedPointToFloat8D8(60);
    mBounce = mRace->mVCalc->FixedPointToFloat8D8(50);
    Stats.Behind = 100;
    Stats.MGunUpgrade = 0;
    Stats.MRocketUpgrade = 0;

    mFriction = mRace->mVCalc->FixedPointToFloat8D8(10);
    mFrictionLimit = mRace->mVCalc->FixedPointToFloat8D8(15);

    Booster.InitialThrust = mRace->mVCalc->FixedPointToFloat8D8(150);
    Booster.BurnThrust = mRace->mVCalc->FixedPointToFloat8D8(7);
    Booster.Burn = 0;
    Booster.BurnTime = 70;

    FlightModel.SizeForward = mRace->mVCalc->FixedPointToFloat8D8(60);
    FlightModel.SizeRear = mRace->mVCalc->FixedPointToFloat8D8(60);
    FlightModel.SizeSideways = mRace->mVCalc->FixedPointToFloat8D8(60);
    FlightModel.SizeSensorOffset = mRace->mVCalc->FixedPointToFloat8D8(100);

    FlightModel.RideHeight = mRace->mVCalc->FixedPointToFloat8D8(50);
    FlightModel.BrakePower = mRace->mVCalc->FixedPointToFloat8D8(16);

    FlightModel.FrontLeft.Rebound = mRace->mVCalc->FixedPointToFloat8D8(50);
    FlightModel.FrontLeft.ReboundLimit = mRace->mVCalc->FixedPointToFloat8D8(80);

    FlightModel.FrontRight.Rebound = mRace->mVCalc->FixedPointToFloat8D8(50);
    FlightModel.FrontRight.ReboundLimit = mRace->mVCalc->FixedPointToFloat8D8(80);

    FlightModel.RearLeft.Rebound = mRace->mVCalc->FixedPointToFloat8D8(-56);
    FlightModel.RearLeft.ReboundLimit = mRace->mVCalc->FixedPointToFloat8D8(80);

    FlightModel.RearRight.Rebound = mRace->mVCalc->FixedPointToFloat8D8(-56);
    FlightModel.RearRight.ReboundLimit = mRace->mVCalc->FixedPointToFloat8D8(80);
}

VVehicle::VVehicle(Race* mParentRace, std::string model, irr::core::vector3d<irr::f32> NewPosition,
                   irr::core::vector3d<irr::f32> NewFrontAt, irr::u8 nrLaps, bool humanPlayer) {

   //TODO 17.06.2026: Take care about nrLaps
   mRace = mParentRace;

   //nrLaps + 1 is correct, I saw this
   //also in the original game
   RaceLaps = nrLaps + 1;

   if (humanPlayer) {
       ControlOrigin = 1;
   } else {
       ControlOrigin = 8;
   }

   for (int idx = 0; idx < 8; idx++) {
       Counter[idx] = 0;
   }

   SetupFlightModelConstants();

   //if computer player
   if (ControlOrigin == 8) {
       ComputerPlayer.EnemyIndex = 1;
       ComputerPlayer.Skill = 90;
   }

   //definition of dirt texture elements
   dirtTexIdsVec = new std::vector<irr::s32>{0, 1, 2, 60, 61, 62, 63, 64, 65, 66, 67, 79};

   irr::core::vector3df vanPos = mRace->mVCalc->IrrlichtToVanillaCoord(NewPosition);
   irr::f32 terrHeight = mRace->mVCalc->map_altitude_column_and_floor(vanPos);
   vanPos.Z = terrHeight + 0.5f;

   ThingData.Position = vanPos;
   ThingData.Movement.AngleXZ = 0.0f;
   Momentum.AngleXY = 0.0f;
   Momentum.DeltaX = 0.0f;
   Momentum.DeltaY = 0.0f;
   Bump.X = 0.0f;
   Bump.Y = 0.0f;
   Bump.Z = 0.0f;
   mMaximumZpos = 3.0f / 256.0f;

   FlightModel.FrontLeft.Zpos = ThingData.Position.Z;
   FlightModel.FrontRight.Zpos = ThingData.Position.Z;
   FlightModel.RearLeft.Zpos = ThingData.Position.Z;
   FlightModel.RearRight.Zpos = ThingData.Position.Z;

   FlightModel.Flag.Booster = false;
   FlightModel.Flag.Brake = false;
   FlightModel.Flag.Airbourn = false;
   FlightModel.Flag.BarrelRoll = false;
   FlightModel.Flag.AutoPilot = false;
   FlightModel.Flag.AutoRefuel = false;
   FlightModel.Flag.AutoRearm = false;
   FlightModel.Flag.AutoRepair = false;
   FlightModel.Flag.AutoStop = false;
   FlightModel.Flag.AutoPilotSet = false;
   FlightModel.Flag.AutoDrive = false;
   FlightModel.Flag.HealthDeath = false;
   FlightModel.Flag.FuelDeath = false;
   FlightModel.Flag.Reposition = false;
   FlightModel.Flag.pad1 = false;
   FlightModel.Flag.pad2 = false;

   //Pad1 and Pad2 seem to be used for checkpoint and lap number processing
   FlightModel.FunctionFlag.Pad1 = false;
   FlightModel.FunctionFlag.Pad2 = false;
   FlightModel.FunctionFlag.Pad3 = true;
   FlightModel.FunctionFlag.Pad4 = false;
   FlightModel.FunctionFlag.Pad6 = false;
   FlightModel.FunctionFlag.Pad9 = false;

   //Pad12 seems to be used for checkpoint and lap number processing
   FlightModel.FunctionFlag.Pad12 = false;

   FlightModel.FunctionFlag.Brake = true;
   FlightModel.FunctionFlag.Booster = true;
   FlightModel.FunctionFlag.BarrelRoll = true;

   ComputerPlayer.Count1 = 0;
   ComputerPlayer.Count2 = 0;
   ComputerPlayer.Count3 = 0;
   ComputerPlayer.Param2 = 0;
   ComputerPlayer.Param3 = 0;
   ComputerPlayer.Param4 = 0;

   Stats.Health = 10000;
   Stats.Fuel = 10000;
   Stats.Weapons = 10000;
   Stats.Invisible = 0; //we are not invisible
   Stats.Invincable = 0; //I assume right now it should be zero?
   Stats.VehicleHit = 0; //I assume right now it should be zero?
   Stats.Weight = 0;

   ThingData.AffectStatus = 0;

   vehicle_setup_computer_character();

   mCraftMesh = mRace->mGame->mSmgr->getMesh(model.c_str());
   mCraftNode = mRace->mGame->mSmgr->addMeshSceneNode(mCraftMesh);

   //set player model initial orientation and position, later player craft is only moved by physics engine
   //also current change in Rotation of player craft model compared with this initial orientation is controlled by a
   //quaterion inside the physics engine object for this player craft as well
   mCraftNode->setRotation(((NewFrontAt-NewPosition).normalize()).getHorizontalAngle()+ irr::core::vector3df(0.0f, 180.0f, 0.0f));
   mCraftNode->setPosition(NewPosition);

   //mCraftNode->setDebugDataVisible(EDS_BBOX);
   mCraftNode->setDebugDataVisible(EDS_OFF);

   mCraftNode->setScale(irr::core::vector3d<irr::f32>(1,1,1));
   mCraftNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
   mCraftNode->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
   //mCraftNode->setVisible(true);

   //Prepare the Irrlicht bounding box
   mCraftNode->updateAbsolutePosition();
   mBoundingBox = mCraftNode->getTransformedBoundingBox();

   CalcCraftLocalFeatureCoordinates(NewPosition, NewFrontAt);

   //create my Dust cloud emitter particles system
   mDustBelowCraft = new DustBelowCraft(mRace->mGame->mSmgr, mRace->mGame->mDriver, this, 100);
}

void VVehicle::vehicle_get_track_friction() {
    if (FlightModel.Flag.Airbourn) {
        //if we are currently air bourne there is no friction
        //logging::Info("AirBourn");
        mFriction = 0.0f;
        return;
    }

    //Run the following code only every 100ms once
    //Original game does the same
    if ((ThingData.mTimeSlice & 1) == 0) {
        //no, we are currently not air bourne
        //get the current friction value from the tile below
        uint16_t tileFriction = mRace->mVCalc->map_colide_friction(ThingData.Position);
        if (!tileFriction) {
            //friction is 0, set 15 (original game does the same)
            tileFriction = 15;
        }

        irr::f32 tileFrictionFloating =
                mRace->mVCalc->FixedPointToFloat8D8((int16_t)(tileFriction));

        irr::f32 newFriction;

        if (mFriction >= tileFrictionFloating) {
            newFriction = mFriction - 0.00390625f;
            if (tileFrictionFloating >= mFriction) {
                return;
            }
        } else {
            newFriction = mFriction + 0.00390625f;
        }

        mFriction = newFriction;

        //std::ostringstream outputMsg;
        //outputMsg << "New Friction value = " << mFriction;
        //logging::Info(outputMsg.str());
    }
}

void VVehicle::vehicle_calculate_angle() {
    //is the input angle != 0?
    if (fabs(MovementInput.AngleXY) > 0.0054931640625f) {
         irr::f32 v5 = 0.0f;
         irr::f32 vHelp = 0.0f;
         if (fabs(Increment.AngleXY) > 0.0054931640625f) {
           v5 = Increment.AngleXY / fabs(Increment.AngleXY);
         }

        if (fabs(MovementInput.AngleXY) > 0.0054931640625f) {
           vHelp = (MovementInput.AngleXY / fabs(MovementInput.AngleXY));
           if (fabs(v5 - vHelp) > 0.0054931640625f) {
               goto LABEL_15_vehicle_calculate_angle;
           }
       } else if (fabs(v5) > 0.0054931640625f) {
LABEL_15_vehicle_calculate_angle:
           Increment.AngleXY *= (0.84375f);
           goto LABEL_16_vehicle_calculate_angle;
       }

LABEL_16_vehicle_calculate_angle:
       Increment.AngleXY += MovementInput.AngleXY;
       goto LABEL_18_vehicle_calculate_angle;
    }

    Increment.AngleXY *= (0.890625f);

LABEL_18_vehicle_calculate_angle:
    //Increment.AngleXY seems to be a relative angle
    //limit the maximum possible range
    if (Increment.AngleXY >= -IncrementLimit.AngleXY) {
        if (IncrementLimit.AngleXY < Increment.AngleXY) {
            Increment.AngleXY = IncrementLimit.AngleXY;
        }
    } else {
        Increment.AngleXY = -IncrementLimit.AngleXY;
    }

    ThingData.Movement.AngleXY += Increment.AngleXY;

    mRace->mVCalc->UnwrapPhaseSigned(ThingData.Movement.AngleXY);
}

void VVehicle::vehicle_calculate_thrust(irr::core::vector3df& delta) {
    irr::f32 v6;
    irr::f32 v13;
    irr::f32 v14;

    if ((Stats.Health > 0) || (FlightModel.Flag.AutoDrive)) {
       v6 = MovementInput.SpeedActual;

       if (fabs(v6) > 0.00390625f) {
           Increment.SpeedActual += v6;

           if (MovementInput.SpeedActual < 0.0f) {
               //set the brake flag
               FlightModel.Flag.Brake = true;
           } else {
               //clear the brake flag
               FlightModel.Flag.Brake = false;
           }
       }
    } else {
        //Health is negative (so human player must not steer,
        //and Autodrive is currently not set)
        //so force zero speed
        Increment.SpeedActual = 0.0f;
    }

    if (Increment.SpeedActual >= 0.0f) {
        //we are not Deaccelerating right now
        if (IncrementLimit.SpeedActual < Increment.SpeedActual) {
            Increment.SpeedActual = IncrementLimit.SpeedActual;
        }
    } else {
        //we are deaccelerating right now
        Increment.SpeedActual = 0.0f;
    }

    //Are we out of fuel?
    if (Stats.Fuel <= 0) {
      v13 = Increment.SpeedActual;

      if (v13 >= 0.0f) {
          v14 = IncrementLimit.SpeedActual * 0.5f;
          if (v14 < v13) {
              Increment.SpeedActual = v14;
          }
      } else {
          Increment.SpeedActual = 0.0f;
      }
    }

    ThingData.Movement.SpeedActual = ((irr::f32)(mThrustEffectiveness) * Increment.SpeedActual) / 100.0f;
    mRace->mVCalc->move_displacement_set(delta, ThingData.Movement.AngleXY, 0.0f, (ThingData.Movement.SpeedActual / 16.0f));
    ThingData.Movement.SpeedActual = Increment.SpeedActual;
}

void VVehicle::vehicle_calculate_momentum(irr::core::vector3df& delta) {
    irr::core::vector3df v44;
    irr::core::vector3df v50;
    irr::core::vector3df position;

    delta.X += (Slope.X / 64.0f);
    delta.Y += (Slope.Y / 64.0f);
    v44.X = (Momentum.DeltaX / 4.0f);
    v44.Y = (Momentum.DeltaY / 4.0f);

    //TODO: add later if I care about a Bonus
    //Bonus.Xpos = v44.X * (Stats.Behind - 100) / 100
    //Bonus.Ypos = v44.Y * (Stats.Behind - 100) / 100

    irr::f32 v7;
    irr::f32 YPos;

    if (!FlightModel.Flag.Airbourn) {
        irr::f32 v6 = sqrt(v44.Y * v44.Y + v44.X * v44.X);
        if (v6 < 0.390625f) {
            v7 = 0.1953125f - v6;
            if (v7 >= 0.0f) {
                if (v7 >= 0.1953125) {
                    v7 = 0.1953125;
                }
            } else {
                v7 = 0.0f;
            }

            YPos = delta.Y;
            delta.X += delta.X * (v7 / 0.390625f);
            delta.Y = YPos + YPos * (v7 / 0.390625f);
        }

        Momentum.DeltaX += delta.X;
        Momentum.DeltaY += delta.Y;

        //AffectStatus Flag 0x40 means that the booster
        //was triggered
        if (((ThingData.AffectStatus & 0x40) != 0) &&
                (FlightModel.FunctionFlag.Booster)) {
            FlightModel.Flag.Booster = true;

            Booster.BurnSetting = Booster.TriggerTime;
            Booster.Burn = Booster.BurnTime;

            irr::f32 hlpValue = Booster.InitialThrust * ((irr::f32)(Booster.BurnSetting) / 100.0f) *
                    ((irr::f32)(mThrustEffectiveness) / 100.0f);

            mRace->mVCalc->move_displacement_set(position, ThingData.Movement.AngleXY,
                                                 0.0f, hlpValue);

            Momentum.DeltaX += position.X;
            Momentum.DeltaY += position.Y;
        } else {
            if (FlightModel.Flag.Booster) {
               if (Booster.Burn) {
                   irr::f32 hlpValue2 = Booster.BurnThrust * ((irr::f32)(Booster.BurnSetting) / 100.0f) *
                           ((irr::f32)(mThrustEffectiveness) / 100.0f);
                   mRace->mVCalc->move_displacement_set(position, ThingData.Movement.AngleXY,
                                                        0.0f, hlpValue2);

                   Momentum.DeltaX += position.X;
                   Momentum.DeltaY += position.Y;
                   --Booster.Burn;
               } else {
                   //Booster stopped burning
                   FlightModel.Flag.Booster = false;
               }
            }

            //Keep double for DeltaX and DeltaY!
            double DeltaX = (double)(Momentum.DeltaX);
            double DeltaY = (double)(Momentum.DeltaY);

            irr::f32 speedValConst = 4.0f;

            int8_t v55 =
                    mRace->mVCalc->move_displacement_set(v50, ThingData.Movement.AngleXY, 0.0f, speedValConst);

            //with the code below the disassembler had some issues, and the Pseudo-C Code
            //was unusable; Therefore this is based on a longer assembly study session, and stepping
            //with the Playstation 1 Emulator debugger, Pretty weird code
            double floatV50_X = (double)(v50.X);
            double floatV50_Y = (double)(v50.Y);

            floatV50_X = floatV50_X / speedValConst;
            floatV50_Y = floatV50_Y / speedValConst;

            double multXRes = floatV50_X * DeltaX * 256.0;
            double multYRes = floatV50_Y * DeltaY * 256.0;

            double sum = multXRes + multYRes;

            double multXResSum = floatV50_X * sum;
            double multYResSum = floatV50_Y * sum;

            int32_t multXResSumInt = (int32_t)(multXResSum);
            int32_t multYResSumInt = (int32_t)(multYResSum);

            v44.X = mRace->mVCalc->FixedPointToFloat8D8(static_cast<int16_t>(multXResSumInt));
            v44.Y = mRace->mVCalc->FixedPointToFloat8D8(static_cast<int16_t>(multYResSumInt));
            v44.Z = 0.0f;

            double v45_var70 = DeltaX - (double)(v44.X);
            double v45_var70_plus2 = DeltaY - (double)(v44.Y);
            double v46_var6C = (double)(v44.Z);

            v44.X *= (0.9765625f - mFriction);
            v44.Y *= (0.9765625f - mFriction);

            Momentum.DeltaX = v44.X;
            Momentum.DeltaY = v44.Y;

            double v47 = v45_var70_plus2;
            double v48 = v46_var6C;

            v45_var70 *= (0.9765625 - (double)(mSideslipFriction) - (double)(mFriction));
            v45_var70_plus2 *= (0.9765625 - (double)(mSideslipFriction) - (double)(mFriction));

            Momentum.DeltaX += (irr::f32)(v45_var70);
            Momentum.DeltaY += (irr::f32)(v45_var70_plus2);

            double v35 = (v47 - v45_var70_plus2) * (v47 - v45_var70_plus2);
            v45_var70_plus2 = v47 - v45_var70_plus2;
            v45_var70 = v47 - v45_var70_plus2;
            v46_var6C = v48 - v46_var6C;

            double v36 = sqrt(v35 + v45_var70 * v45_var70);
            mRace->mVCalc->move_displacement_set(v44, ThingData.Movement.AngleXY, 0.0f, (irr::f32)(v36));

            Momentum.DeltaX += v44.X * (mSideslipFriction / 100.0f);
            Momentum.DeltaY += v44.Y * (mSideslipFriction / 100.0f);
        }

        if (FlightModel.Flag.Brake) {
           if (FlightModel.FunctionFlag.Brake) {
                Momentum.DeltaX *= (0.9765625f - FlightModel.BrakePower);
                Momentum.DeltaY *= (0.9765625f - FlightModel.BrakePower);
           }
        }
    }
}

void VVehicle::vehicle_calculate_movement_delta(irr::core::vector3df& delta) {
    irr::core::vector3df position1(0.0f, 0.0f, 0.0f);
    irr::f32 v5;
    irr::f32 v7;
    irr::f32 v9;

    //TODO: add the Bump and Bonus four commented out lines below, if I know what
    //the do exactly

    delta.X = Momentum.DeltaX / 4.0f;
    delta.Y = Momentum.DeltaY / 4.0f;
    delta.Z = Displacement.Z;
    //delta.X += Bump.X;
    //delta.Y += Bump.Y;
    Stats.Velocity = mRace->mVCalc->distance_get_xyz(position1, delta);
    //delta.X += Bonus.X;
    irr::f32 Xpos = delta.X;
    //delta.Y += Bonus.Y;

    if (Xpos < -0.9765625f) {
        v5 = -0.9765625f;
vehicle_calculate_movement_delta_LABEL_4:
        delta.X = v5;
        goto vehicle_calculate_movement_delta_LABEL_5;
    }
    v5 = 0.9765625f;
    if (Xpos >= 0.98046875f) {
        goto vehicle_calculate_movement_delta_LABEL_4;
    }
vehicle_calculate_movement_delta_LABEL_5:

    irr::f32 Ypos = delta.Y;
    v7 = -0.9765625f;

    if ( (Ypos < -0.9765625f) || (v7 = 0.9765625f, Ypos >= 0.98046875f)) {
        delta.Y = v7;
    }

    irr::f32 Zpos = delta.Z;
    v9 = -0.9765625f;
    if ( (Zpos < -0.9765625f) || (v9 = 0.9765625f, Zpos >= 0.98046875f)) {
        delta.Z = v9;
    }

    Displacement.X = Momentum.DeltaX / 4.0f;
    Displacement.Y = Momentum.DeltaY / 4.0f;
}

void VVehicle::vehicle_move_altitude(irr::core::vector3df& delta) {
     irr::core::vector3df position;

     position.X = ThingData.Position.X + delta.X;
     position.Y = ThingData.Position.Y + delta.Y;
     position.Z = ThingData.Position.Z + delta.Z;

     irr::f32 craftHeightSum = FlightModel.FrontLeft.Zpos + FlightModel.FrontRight.Zpos;
     position.Z = craftHeightSum / 2.0f;

     irr::f32 terrainHeight = mRace->mVCalc->map_altitude_column_and_floor(position);

     irr::f32 distCraftAboveTerrain = position.Z - terrainHeight;

     //Are we currently Airbourne?
     if (distCraftAboveTerrain < 0.58984375f) {
         //We are not Airbourn currently
         FlightModel.Flag.Airbourn = false;
         if (distCraftAboveTerrain < 0.390625f) {
             position.Z = terrainHeight + 0.390625f;
         }
     } else {
         //We are Airbourn currently
        FlightModel.Flag.Airbourn = true;
     }

     Displacement.Z = position.Z - ThingData.Position.Z;
     delta.Z = position.Z - ThingData.Position.Z;
}

void VVehicle::vehicle_move_tilt(irr::core::vector3df& delta) {
    irr::core::vector3df position;
    irr::f32 v11;
    irr::f32 v6;
    irr::f32 v8;
    bool v10;
    irr::f32 craftTerrainAvgDist;
    irr::f32 terrainHeight;

    position.X = ThingData.Position.X + delta.X;
    position.Y = ThingData.Position.Y + delta.Y;
    position.Z = ThingData.Position.Z + delta.Z;

    if (FlightModel.Flag.Airbourn) {
        //yes, we are in the air right now
        ThingData.Movement.AngleZY += 0.2471923828125f;

        v11 = -45.0f;

        if (ThingData.Movement.AngleZY < -45.0f) {
            goto vehicle_move_tilt_LABEL_10;
        }

        v11 = 45.0f;

        if (ThingData.Movement.AngleZY >= 45.0054931640625f) {
            goto vehicle_move_tilt_LABEL_10;
        }
    } else {
        //we are not AirBourn
        //get terrain height below
        terrainHeight = mRace->mVCalc->map_altitude_column_and_floor(position);
        v6 = FlightModel.SizeForward;
        craftTerrainAvgDist = terrainHeight - ((FlightModel.FrontRight.ZposFloor
                                               + FlightModel.FrontLeft.ZposFloor) * 0.5f);

        if (fabs(craftTerrainAvgDist) < v6) {
          //not sure if for craftTerrainAvgDist parameter in arctanPlusMultiply32 call below
          //I need to give whole float number, or only fractional part? If bug clarify later
          v8 =
            ThingData.Movement.AngleZY +
               (mRace->mVCalc->arctanPlusMultiply32(craftTerrainAvgDist, -v6) - ThingData.Movement.AngleZY) / 4.0f;

          ThingData.Movement.AngleZY = v8;
          v11 = -24.9993896484375f;
          v10 = (v8 < v11);

          if (!v10) {
              if (v8 >= 25.0048828125f) {
                  ThingData.Movement.AngleZY = 24.9993896484375f;
              }
              return;
          }

vehicle_move_tilt_LABEL_10:
          ThingData.Movement.AngleZY = v11;
        }
    }
}

void VVehicle::vehicle_move_roll(irr::core::vector3df& delta) {
    irr::f32 v10;
    irr::f32 v8;
    irr::f32 comp;
    irr::f32 craftHeightDiff;
    irr::f32 absCraftHeightDiff;
    bool v9;
    int32_t intPart;
    irr::f32 fracPart;

    if (FlightModel.Flag.Airbourn) {
        //yes, we are in the air right now
        v10 = 0.9375f * ThingData.Movement.AngleXZ;
        goto vehicle_move_roll_LABEL_9;
    }

    craftHeightDiff = FlightModel.FrontLeft.ZposFloor -
                                 FlightModel.FrontRight.ZposFloor;

    absCraftHeightDiff = fabs(craftHeightDiff);

    intPart = (int32_t)(craftHeightDiff);
    fracPart = craftHeightDiff - (irr::f32)(intPart);

    if (absCraftHeightDiff < (2.0f * FlightModel.SizeSideways)) {
        v10 = ThingData.Movement.AngleXZ +
                (mRace->mVCalc->arctanPlusMultiply32(fracPart, -2.0f * FlightModel.SizeSideways)
                 -ThingData.Movement.AngleXZ) / 8.0f;

        ThingData.Movement.AngleXZ = v10;
        v8 = v10;
        comp = -24.9993896484375f;
        v9 = (v10 < comp);
        v10 = comp;

        if (!v9) {
              if (v8 >= 25.0048828125f) {
                  ThingData.Movement.AngleXZ = 24.9993896484375f;
              }
              return;
        }

vehicle_move_roll_LABEL_9:
          ThingData.Movement.AngleXZ = v10;
    }
}

void VVehicle::vehicle_sensor_point_process(VehicleSensorPointStruct& sensor, irr::core::vector3df& slope,
                                            int8_t terrain) {
    irr::f32 RideHeight = FlightModel.RideHeight;
    irr::f32 ZposDisplacement;

    irr::f32 Zpos = sensor.Zpos;
    irr::f32 Xpos;
    irr::f32 Ypos;

    irr::f32 v9;
    irr::f32 v10;

    if (Zpos >= (RideHeight + sensor.ZposFloor)) {
        ZposDisplacement = sensor.ZposDisplacement;
    } else {
        if ((terrain & 4) == 0) {
            Xpos = fabs(slope.X);
            if (Xpos >= 1.95703125f) {
                sensor.CollideFlags |= 1u;
            }

            Ypos = fabs(slope.Y);
            if (Ypos >= 1.95703125f) {
                sensor.CollideFlags |= 2u;
            }
        }

        v9 = RideHeight + sensor.ZposFloor - sensor.Zpos;
        v10 = v9 * sensor.Rebound;
        sensor.ZposDisplacement = v9;
        v10 /= 256.0f;
        sensor.ZposDisplacement = v10;
        if (sensor.ReboundLimit < v10) {
            sensor.ZposDisplacement = sensor.ReboundLimit;
        }
        Zpos = sensor.ZposDisplacement;
        ZposDisplacement = RideHeight + sensor.ZposFloor;
    }

    sensor.Zpos = ZposDisplacement + Zpos;
    irr::f32 v12 = sensor.Zpos - (RideHeight + sensor.ZposFloor);
    sensor.ZposDiff = v12;

    if (mMaximumZpos < v12) {
        sensor.ZposDisplacement -= mMaximumZpos * ((float)(Stats.Behind) / 100.0f);
    }

    irr::f32 v16 = -0.2734375f * ((float)(Stats.Behind) / 100.0f);

    if (sensor.ZposDisplacement >= v16) {
        if (sensor.ZposDisplacement >= 0.39453125f) {
            sensor.ZposDisplacement = 0.390625f;
        }
    } else {
        sensor.ZposDisplacement = v16;
    }

    if (sensor.ZposDisplacement >= 3.91015625f) {
        logging::Warning("vehicle_sensor_point_process: Z TOO BIG!");
    }
}

void VVehicle::vehicle_sensor_point_projection(irr::core::vector3df& delta) {
    irr::core::vector3df position;
    irr::core::vector3df new_position;
    irr::core::vector3df displacement;

    irr::f32 Forward = FlightModel.SizeForward;
    irr::f32 Sideways = FlightModel.SizeSideways;

    position.X = ThingData.Position.X;
    position.Y = ThingData.Position.Y;
    position.Z = ThingData.Position.Z;
    new_position.X = ThingData.Position.X;
    new_position.Y = ThingData.Position.Y;
    new_position.Z = ThingData.Position.Z;

    /* Sensor Front Right */
    mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY + 90.0f, ThingData.Movement.AngleXZ, Sideways);
    mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY, ThingData.Movement.AngleZY, Forward);
    new_position = position + delta;

    FlightModel.FrontRight.Position.X = position.X;
    FlightModel.FrontRight.Position.Y = position.Y;
    FlightModel.FrontRight.Position.Z = position.Z;
    FlightModel.FrontRight.CollideFlags = mRace->mVCalc->map_colide_direction_xy(position, new_position);
    mRace->mVCalc->move_displacement_slope(new_position, displacement);
    FlightModel.FrontRight.ZposFloor = mRace->mVCalc->map_altitude_column_and_floor(new_position);
    int8_t v13 = mRace->mVCalc->map_colide_type(new_position);
    vehicle_sensor_point_process(FlightModel.FrontRight, displacement, v13);

    /* Sensor Front Left */

    Sideways *= 2.0f;
    mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY - 90.0f, -ThingData.Movement.AngleXZ, Sideways);
    new_position = position + delta;

    FlightModel.FrontLeft.Position.X = position.X;
    FlightModel.FrontLeft.Position.Y = position.Y;
    FlightModel.FrontLeft.Position.Z = position.Z;
    FlightModel.FrontLeft.CollideFlags = mRace->mVCalc->map_colide_direction_xy(position, new_position);
    mRace->mVCalc->move_displacement_slope(new_position, displacement);
    FlightModel.FrontLeft.ZposFloor = mRace->mVCalc->map_altitude_column_and_floor(new_position);
    int8_t v16 = mRace->mVCalc->map_colide_type(new_position);
    vehicle_sensor_point_process(FlightModel.FrontLeft, displacement, v16);

    /* Sensor Rear Left */

    mRace->mVCalc->move_xyz(position, -ThingData.Movement.AngleXY, -ThingData.Movement.AngleZY,
             FlightModel.SizeRear + Forward);
    new_position = position + delta;

    FlightModel.RearLeft.Position.X = position.X;
    FlightModel.RearLeft.Position.Y = position.Y;
    FlightModel.RearLeft.Position.Z = position.Z;
    FlightModel.RearLeft.CollideFlags = mRace->mVCalc->map_colide_direction_xy(position, new_position);
    mRace->mVCalc->move_displacement_slope(new_position, displacement);
    FlightModel.RearLeft.ZposFloor = mRace->mVCalc->map_altitude_column_and_floor(new_position);
    int8_t v19 = mRace->mVCalc->map_colide_type(new_position);
    vehicle_sensor_point_process(FlightModel.RearLeft, displacement, v19);

    /* Sensor Rear Right */

    mRace->mVCalc->move_xyz(position, ThingData.Movement.AngleXY + 90.0f, ThingData.Movement.AngleXZ, Sideways);
    new_position = position + delta;

    FlightModel.RearRight.Position.X = position.X;
    FlightModel.RearRight.Position.Y = position.Y;
    FlightModel.RearRight.Position.Z = position.Z;
    FlightModel.RearRight.CollideFlags = mRace->mVCalc->map_colide_direction_xy(position, new_position);
    mRace->mVCalc->move_displacement_slope(new_position, displacement);
    FlightModel.RearRight.ZposFloor = mRace->mVCalc->map_altitude_column_and_floor(new_position);
    int8_t v22 = mRace->mVCalc->map_colide_type(new_position);
    vehicle_sensor_point_process(FlightModel.RearRight, displacement, v22);
}

void VVehicle::vehicle_colide_map(irr::core::vector3df& delta) {
    vehicle_sensor_point_projection(delta);

    //Note 02.05.2026: The original code is really weird here with a lot of int type conversions
    //and logical combinations; I used a pencil and paper, and I believe whats actually going on
    //is the code I have written here below; But of course this change could have introduced a bug!
    int8_t v9 = (FlightModel.FrontRight.CollideFlags | FlightModel.FrontLeft.CollideFlags |
                 FlightModel.RearLeft.CollideFlags | FlightModel.RearRight.CollideFlags);

    //only enter if there is a collision indicated right now
    if (v9) {
        irr::f32 Xpos;

        //any collision Flag bit location 0 set?
        if ((v9 & 1) != 0) {
           Xpos = fabs(delta.X);
           if (Xpos >= 0.04296875f) {
               ThingData.AffectStatus |= 0x200u;
           }
           delta.X = 0.0f;
           Momentum.DeltaX *= mBounce;
           Momentum.DeltaX = -Momentum.DeltaX;
        }

        //any collision Flag bit location 1 set?
        if ((v9 & 2) != 0) {
            irr::f32 Ypos;

            Ypos = fabs(delta.Y);
            if (Ypos >= 0.04296875f) {
                ThingData.AffectStatus |= 0x200u;
            }
            delta.Y = 0.0f;
            Momentum.DeltaY *= mBounce;
            Momentum.DeltaY = -Momentum.DeltaY;
        }
    }

   if (FlightModel.FrontLeft.CollideFlags) {
       mRace->mVCalc->move_xyz(delta, ThingData.Movement.AngleXY + 90.0f, 0.0f, 0.05859375f);
   }

   if (FlightModel.FrontRight.CollideFlags) {
       mRace->mVCalc->move_xyz(delta, ThingData.Movement.AngleXY - 90.0f, 0.0f, 0.05859375f);
   }

   if (FlightModel.RearLeft.CollideFlags) {
       mRace->mVCalc->move_xyz(delta, ThingData.Movement.AngleXY + 90.0f, 0.0f, 0.05859375f);
   }

   if (FlightModel.RearRight.CollideFlags) {
       mRace->mVCalc->move_xyz(delta, ThingData.Movement.AngleXY - 90.0f, 0.0f, 0.05859375f);
   }
}

void VVehicle::vehicle_move_mapwho(irr::core::vector3df& delta) {
    irr::f32 Xpos;
    irr::f32 Ypos;
    irr::f32 Zpos;
    irr::f32 v6;
    irr::f32 v8;
    irr::f32 v10;

    irr::core::vector3df position;

    Xpos = delta.X;
    v6 = -0.8984375f;
    if (Xpos < -0.8984375f || (v6 = 0.8984375f, Xpos >= 0.90234375f)) {
        delta.X = v6;
    }

    Ypos = delta.Y;
    v8 = -0.8984375f;
    if (Ypos < -0.8984375f || (v8 = 0.8984375f, Ypos >= 0.90234375f)) {
        delta.Y = v8;
    }

    Zpos = delta.Z;
    v10 = -0.5859375f;
    if (Zpos < -0.5859375f || (v10 = 0.5859375f, Zpos >= 0.58984375f)) {
        delta.Z = v10;
    }

    position.X = ThingData.Position.X + delta.X;
    position.Y = ThingData.Position.Y + delta.Y;
    position.Z = ThingData.Position.Z + delta.Z;
    //mapwho_move(thing, &position);

    //Remove line below later again, happens in mapwho_move above if implemented correctly
    //later
    ThingData.Position = position;
}

void VVehicle::vehicle_control_from_player() {
    irr::f32 v13;
    irr::f32 v14;

    MovementInput.AngleXY = 0.0f;
    MovementInput.AngleXZ = 0.0f;
    MovementInput.AngleZY = 0.0f;
    MovementInput.SpeedActual = 0.0f;

    if (KeyPressedAccel) {
         MovementInput.SpeedActual = IncrementAdd.SpeedActual;
    } else if (KeyPressedDeaccel) {
         MovementInput.SpeedActual = -IncrementAdd.SpeedActual;
    }

    if (KeyPressedTurnRight || KeyPressedTurnLeft) {
        v13 = IncrementAdd.AngleXY;
        if (KeyPressedTurnRight) {
            if (fabs(MovementInput.AngleXY) > 0.0054931640625f) {
                MovementInput.AngleXY = ((v13 + (v13 / 32768.0f)) * 0.5f) +
                        ((MovementInput.AngleXY + (MovementInput.AngleXY / 32768.0f)) * 0.5f);
                goto vehicle_control_from_player_LABEL_28;
            } else {
                v14 = v13 + (v13 / 32768.0f);
                MovementInput.AngleXY = v14 * 0.5f;
            }
        }

        if (KeyPressedTurnLeft) {
            if (fabs(MovementInput.AngleXY) > 0.0054931640625f) {
                MovementInput.AngleXY = ((-v13 - (v13 / 32768.0f)) * 0.5f) -
                        ((MovementInput.AngleXY + (MovementInput.AngleXY / 32768.0f)) * 0.5f);
                goto vehicle_control_from_player_LABEL_28;
            } else {
                v14 = -v13 - (v13 / 32768.0f);
                MovementInput.AngleXY = v14 * 0.5f;
            }
        }
    }  else {
        //in case neither the left or right turn
        //key was pressed
        MovementInput.AngleXY = 0.0f;
    }

vehicle_control_from_player_LABEL_28:

    //add missing code below later; there is more for weapons trigger
    //and something regarding friction

    //Handle Booster key
    if (KeyPressedBooster) {
        if (Stats.Fuel > 0) {
            ++Booster.Trigger;
        }
    }
}

uint8_t VVehicle::vehicle_control_from_autopilot() {
    irr::f32 velocity;
    irr::f32 decisionDistance;
    uint16_t v11;
    uint16_t v17;
    uint16_t currentWaypoint;
    irr::f32 v19;
    int32_t v19Fixed;
    uint32_t v28;
    irr::f32 v40;
    irr::f32 v45;
    irr::f32 v46;
    irr::f32 v47;
    irr::f32 xy;
    irr::f32 difference;
    uint8_t result;
    uint16_t v21;

    irr::core::vector3df position;

    //TODO: Add vehicle_check_vehicle_movement_status later;

    result = 1;

    //If there is no current waypoint
    //make sure to exit
    if (!CurrentWaypoint) {
        return result;
    }

    velocity = Stats.Velocity;
    decisionDistance = 3.0f;

    if (velocity >= 0.1953125f) {
      if (velocity >= 0.390625f) {
          decisionDistance = 6.0f;
          if (velocity < 0.5859375f) {
             decisionDistance = 5.0f;
          }
      } else {
          decisionDistance = 4.0f;
      }
    }

    //are we close enough to the current waypoint?
    if (mRace->mVTrack->track_waypoint_distance(ThingData.Position, CurrentWaypoint)
            < decisionDistance) {
           FlightModel.Flag.pad1 = false;
           FlightModel.Flag.pad2 = false;

           v11 = mRace->mVTrack->track_waypoint_type(CurrentWaypoint);
           if (v11 == 8) {  //is Waypoint of type 8?
              FlightModel.Flag.pad1 = true;
           } else {
              if (v11 < 9) {
                if (v11 == 7) {  //waypoint of type 7?
                   ComputerPlayer.Count2 = 3;
                }
vehicle_control_from_autopilot_LABEL22:
                  LastWayPoint = CurrentWaypoint;
                  CurrentWaypoint = mRace->mVTrack->track_waypoint_child(CurrentWaypoint);
                  //No result found for child waypoint?
                  if (!CurrentWaypoint) {
                      CurrentWaypoint = mRace->mVTrack->track_waypoint_absolute_nearest(ThingData.Position);
                  }

                  v17 = 2;
                  if (FlightModel.Flag.AutoRefuel) {
                     currentWaypoint = CurrentWaypoint;
                  } else {
                    v17 = 4;
                    if (FlightModel.Flag.AutoRepair) {
                         currentWaypoint = CurrentWaypoint;
                  } else {
                    v17 = 3;
                    if (!FlightModel.Flag.AutoRearm) {
vehicle_control_from_autopilot_LABEL31:
                        //controlled by computer player?
                        if (ControlOrigin == 8) {
                            v19 = ThingData.Position.X + ThingData.Position.Y;
                            //The next operation first seems to be tricky in floating point
                            //therefore initial solution: keep it in fixed point arithmetic
                            v19Fixed = mRace->mVCalc->FloatToFixedPoint24D8(v19);
                            //Remove the lowest 2 bits (we loose accuracy)
                            v19Fixed = v19Fixed >> 2;
                            //Shift back 2 bits to the left, lowest 2 bits are
                            //set 0 value
                            v19Fixed = v19Fixed << 2;
                            v19Fixed &= 0xFFFFFFFC;

                            if (v19Fixed == (v19Fixed - 1)) {
                                CurrentWaypoint =
                                        mRace->mVTrack->track_waypoint_junction_exists(CurrentWaypoint, 6);
                            }
                        }
                        goto vehicle_control_from_autopilot_LABEL34;
                    }

                    currentWaypoint = CurrentWaypoint;
                 }
            }
            CurrentWaypoint = mRace->mVTrack->track_waypoint_junction_exists(currentWaypoint, v17);
            goto vehicle_control_from_autopilot_LABEL31;
        }
        if ( v11 != 9) {
            goto vehicle_control_from_autopilot_LABEL22;
        }
        //Note 13.06.2026: Setting pad1 & pad2 flags is a bit tricky
        //here; if something does not work reinvestigate this later, issue could
        //be here
        FlightModel.Flag.pad2 = true;
    }

    goto vehicle_control_from_autopilot_LABEL22;
 }

vehicle_control_from_autopilot_LABEL34:

    v21 = CurrentWaypoint;

    //There is some DeathMatch related code here that I will skip

    //Continue with the non DeathMatch case source code
    if (ComputerPlayer.Count1 != 0) {
        goto vehicle_control_from_autopilot_LABEL48;
    }

    //13.06.2026: It seems there is more Autotarget related stuff I skipped
    //right now here

    if (!FlightModel.FunctionFlag.Pad9) {
      //Skip implementation right now
    }

    if ((!ComputerPlayer.Count1) && (FlightModel.FunctionFlag.Pad4)) {
        ComputerPlayer.Count1 = 23;
        ComputerPlayer.Count2 = 1;
    }

vehicle_control_from_autopilot_LABEL48:
    FlightModel.FunctionFlag.Pad4 = false;
    if (ComputerPlayer.Count1) {
        --ComputerPlayer.Count1;
    }
    if (ComputerPlayer.Count2) {
        --ComputerPlayer.Count2;
    }

    FlightModel.Flag.AutoStop = false;

    if (FlightModel.Flag.AutoRefuel) {
        v28 = ThingData.AffectStatus & 0x10;
    } else if (FlightModel.Flag.AutoRearm) {
        v28 = ThingData.AffectStatus & 0x8;
    } else {
       if (!FlightModel.Flag.AutoRepair) {
           goto vehicle_control_from_autopilot_LABEL60;
       }
       v28 = ThingData.AffectStatus & 0x20;
    }

    if (v28) {
        FlightModel.Flag.AutoStop = true;
    }

vehicle_control_from_autopilot_LABEL60:
    if (FlightModel.Flag.AutoStop) {
        MovementInput.SpeedActual = -IncrementAdd.SpeedActual;
        Momentum.DeltaX *= 0.9375f;
        Momentum.DeltaY *= 0.9375f;
        MovementInput.AngleXY = 0.0f;
        if (FlightModel.Flag.AutoRefuel && Stats.Fuel >= 10000) {
            FlightModel.Flag.AutoRefuel = false;
            FlightModel.Flag.AutoStop = false;
        }

        if (FlightModel.Flag.AutoRearm && Stats.Weapons >= 10000) {
            FlightModel.Flag.AutoRearm = false;
            FlightModel.Flag.AutoStop = false;
        }

        result = 1;
        if (FlightModel.Flag.AutoRepair) {
            if (Stats.Health < 10000) {
               return 1;
            }
            FlightModel.Flag.AutoRepair = false;
vehicle_control_from_autopilot_LABEL135:
            FlightModel.Flag.AutoStop = false;
            return 1;
        }
        return result;
    }

    v40 = 7.109375f;
    if (ComputerPlayer.Count1 < 20) {
        mRace->mVTrack->track_waypoint_position_set(position, v21);
        xy = mRace->mVCalc->angle_get_xy(ThingData.Position, position);
        difference = mRace->mVCalc->angle_get_difference(ThingData.Movement.AngleXY, xy);
        v40 = difference / 32.0f;
        if ( difference < 0.0f) {
            v40 = (difference + 0.12109375f) / 32.0f;
        }
    }
    if (ComputerPlayer.Count2) {
        MovementInput.SpeedActual = -IncrementAdd.SpeedActual;
    } else {
        MovementInput.SpeedActual = IncrementAdd.SpeedActual;
    }

    v45 = IncrementAdd.AngleXY;
    v46 = ((irr::f32)(v45 > 0.0f) - v45) * 0.5f;
    if ((v40 < v46) || (v46 = (v45 + (IncrementAdd.AngleXY / 32768.0f)) * 0.5f),
                         v47 = v40 * 65536.0f, v46 < v40) {
        v40 = v46;
        v47 = v46 * 65536.0f;
    }

    if (v47 >= 0.0f) {
        MovementInput.AngleXY = v40 + (MovementInput.AngleXY / 8.0f);
    } else {
        MovementInput.AngleXY = v40 - (MovementInput.AngleXY / 8.0f);
    }

vehicle_control_from_autopilot_LABEL129:
    if (Stats.Fuel < 3000) {
        FlightModel.Flag.AutoRefuel = true;
    }

    if (Stats.Health < 5000) {
        FlightModel.Flag.AutoRepair = true;
    }

    result = 1;
    if (Stats.Weapons < 3000) {
        FlightModel.Flag.AutoRearm = true;
        goto vehicle_control_from_autopilot_LABEL135;
    }

    return result;
}

void VVehicle::vehicle_setup_computer_character() {
  if (ControlOrigin == 8) {
      //13.06.2026: Implement function, I am to lazy to do it
      //right now
  }
}

uint8_t VVehicle::vehicle_set_autopilot_on() {
    uint8_t result = 0;

    if (!FlightModel.Flag.AutoDrive) {
        if (FlightModel.Flag.AutoPilot) {
            return 0;
        } else {
            //Activate Autopilot
            FlightModel.Flag.AutoPilot = true;
            FlightModel.Flag.AutoPilotSet = true;
            LastWayPoint = CurrentWaypoint;
            return 1;
        }
    }

    return result;
}

void VVehicle::vehicle_set_autodrive_on() {
    FlightModel.Flag.AutoPilot = true;
    FlightModel.Flag.AutoDrive = true;
}

void VVehicle::vehicle_set_autodrive_off() {
    FlightModel.Flag.AutoPilot = false;
    FlightModel.Flag.AutoDrive = false;
}

int32_t VVehicle::vehicle_get_checkpoint() {
   int32_t v5 = 0;
   size_t currClosestCheckPointIdx;
   irr::core::vector3df distance;
   std::vector<ThingDataStruct*>::iterator it;
   int32_t v21;

   //is currently a checkpoint assigned to this vehicle?
   if (CheckPoint) {
       //if (TimeSlice stuff) {
       //    FlightModel.FunctionFlag.Pad12 = false;
       //}

      currClosestCheckPointIdx = 0;

      //check all existing checkpoints
      for (it = mRace->mVanillaCheckpointVec.begin() + 1;
           it != mRace->mVanillaCheckpointVec.end();
           ++it) {

          //get the distance between the vehicle and the current indexed checkpoint
          mRace->mVCalc->distance_get_xy_coords(ThingData.Position, (*it)->Position, distance);
          if (((*it)->CollideSize.X >= distance.X) && ((*it)->CollideSize.Y >= distance.Y)) {
              currClosestCheckPointIdx = (*it)->Index;
              break;
          }
      }

      if ((currClosestCheckPointIdx > 0) &&
            vehicle_process_checkpoint(currClosestCheckPointIdx)) {
          if (!mRace->mVanillaCheckpointVec.at(CheckPoint)->Count) {
              vehicle_checkpoint_next_lap();
              v5 = 1;
          }

          CheckPoint = Counter[2];
      }

      DistanceToNextCheckpoint = mRace->mVCalc->distance_get_rough_xy(
                  ThingData.Position,
                  mRace->mVanillaCheckpointVec.at(CheckPoint)->Position);

      if (LapCounter == (RaceLaps - 1) && v5) {
          FlightModel.FunctionFlag.Pad1 = true;
          FlightModel.FunctionFlag.Pad2 = true;
      } else {
          if (LapCounter == RaceLaps) {
              if (!RacePositionFinish) {
                  Conditions.RacePositionFinishShowTime = 100;
                  RacePositionFinish = RacePosition;
                  TotalRaceTicksFinished = TotalRaceTicks;
              }
          } else {
                FlightModel.FunctionFlag.Pad2 = false;
          }
      }

      //This Status flag is set in vehicle_race_positions
      //As soon as the first player has crossed the finish
      //line the first time this flag is set non zero
      //for all players
      if ((ThingData.Status & 0x800) != 0) {
          v21 = LapTicks + 1;
          LapTicks = v21;
          if (v21 >= 10000) {
              LapTicks = 9999;
          }
          ++TotalRaceTicks;
          ++Conditions.TotalTime;
      }

      if (RacePositionFinish) {
          Stats.Invincable = 10;
      }

      return 1;
   } else {
       //Currently no closest checkpoint assigned
       //we want to find and assign the first checkpoint
       //with Count == 0
       for (it = mRace->mVanillaCheckpointVec.begin() + 1;
            it != mRace->mVanillaCheckpointVec.end();
            ++it) {
           //we found the first checkpoint, assign it to vehicle
           if ((*it)->Count == 0) {
               CheckPoint = (*it)->Index;
               break;
           }
       }

       Counter[2] = CheckPoint;
       return 1;
   }

   return 0;
}

uint8_t VVehicle::vehicle_process_checkpoint(size_t cp_colide) {
    uint8_t v4 = 0;

    //is the specified input checkpoint the same that is currently
    //assigned to this vehicle?
    if (cp_colide == CheckPoint) {
        //yes, it is
        Counter[0] = Counter[2];
        Counter[2] = vehicle_checkpoint_find_next(cp_colide);
        v4 = 1;
    }

    return v4;
}

size_t VVehicle::vehicle_checkpoint_find_next(size_t forCheckPointIdx) {
    size_t index = 0;
    int16_t count;
    int16_t v8;
    bool v9;
    size_t result = 0;
    int16_t i = 1000;

    std::vector<ThingDataStruct*>::iterator it;
    ThingDataStruct* pntr = nullptr;
    ThingDataStruct* pntr2 = nullptr;

    //for which checkpoint do we search the next one?
    for (it = mRace->mVanillaCheckpointVec.begin() + 1;
         it != mRace->mVanillaCheckpointVec.end(); ++it) {
           if ((*it)->Index == forCheckPointIdx) {
               pntr = (*it);
               break;
           }
    }

    if (pntr != nullptr) {
        v8 = pntr->Count + 1;

        for (it = mRace->mVanillaCheckpointVec.begin() + 1;
             it != mRace->mVanillaCheckpointVec.end(); ++it) {
               count = (*it)->Count;
               if ( count == v8) {
                   return (*it)->Index;
               }

               v9 = (v8 < count);
               if (!(*it)->Count) {
                   index = (*it)->Count;
               }

               if (v9 && (count < i)) {
                   pntr2 = (*it);
                   i = (*it)->Count;
               }
        }
    }

    result = index;

    if (pntr2 != nullptr) {
        return pntr2->Index;
    }

    return result;
}

void VVehicle::vehicle_checkpoint_next_lap() {
    int32_t v6;
    int32_t v7;

    if (LapCounter) {
        LastLapTicks = LapTicks;
        v6 = LapTicks;
        v7 = FastestLapTicks;
        //new fastest lap?
        if ((v6 < v7) || !v7) {
            FastestLapTicks = v6;
            //I added this variable FastestLapNr
            //does not exist in original game
            FastestLapNr = LapCounter;
        }

        //Store last lap time in lap time storage
        Conditions.LapTimes[LapCounter] = LapTicks;

        //new lap starts with zero ticks
        LapTicks = 0;
    }

    ++Conditions.LapCount;
    Conditions.FlagNewLap = true;
    ++LapCounter;
}

uint8_t VVehicle::vehicle_set_autopilot_off() {
    uint8_t result = 0;

    if (!FlightModel.Flag.AutoDrive) {
        if (FlightModel.Flag.AutoPilot) {
            FlightModel.Flag.AutoPilot = false;
            FlightModel.Flag.AutoPilotSet = true;
            return 1;
        } else {
            return 0;
        }
    }

    return result;
}

void VVehicle::vehicle_control() {
    //Computer player?
    if (ControlOrigin == 8) {
        if (FlightModel.Flag.AutoPilot) {
            vehicle_control_from_autopilot();
        } else {
            //no, autopilot is not yet active, activate it
            CurrentWaypoint = mRace->mVTrack->track_waypoint_nearest(ThingData.Position);
            vehicle_set_autopilot_on();
        }
        return;
    } else {
        if ((ControlOrigin & 1) != 0) {
            vehicle_control_from_player();
        }
    }

    return;
}

void VVehicle::vehicle_set_camera() {
    //the View is the position and orientation of the
    //player craft model
    View.Position = ThingData.Position;
    View.AngleXY = ThingData.Movement.AngleXY;
    View.AngleZY = ThingData.Movement.AngleZY;
    View.AngleXZ = ThingData.Movement.AngleXZ + 4.0f * Increment.AngleXY;

    //is there currently a BarrelRoll?
    if (FlightModel.Flag.BarrelRoll) {
        View.AngleXY += Tumble.AngleXY * Tumble.Count;
        View.AngleZY += Tumble.AngleXZ * Tumble.Count;
        View.AngleXZ += Tumble.AngleZY * Tumble.Count;
    }
}

void VVehicle::vehicle_setup_tumble() {
    uint8_t v2;
    uint16_t v9;
    uint16_t v3;
    uint16_t v4;
    uint16_t v10;
    irr::core::vector3df position;
    irr::f32 xy;
    irr::f32 v5;
    irr::f32 difference;

    if (!FlightModel.Flag.BarrelRoll) {
       v2 = ThingData.mTimeSlice % 4;
       if (v2 >= 0) {
           if (v2 >= 2) {
               if (v2 >= 4) {
                   return;
               }
               v9 = mRace->mVTrack->track_waypoint_nearest(ThingData.Position);
               v10 = mRace->mVTrack->track_waypoint_child(v9);
               mRace->mVTrack->track_waypoint_position_set(position, v10);
               xy = mRace->mVCalc->angle_get_xy(ThingData.Position, position);
               Tumble.AngleXY =
                       mRace->mVCalc->angle_get_difference(ThingData.Movement.AngleXY, xy) / 40.0f;

               Tumble.AngleZY = 0.19775390625f;
           } else
           {
              v3 = mRace->mVTrack->track_waypoint_nearest(ThingData.Position);
              v4 = mRace->mVTrack->track_waypoint_child(v3);
              mRace->mVTrack->track_waypoint_position_set(position, v4);
              v5 = mRace->mVCalc->angle_get_xy(ThingData.Position, position);
              difference = mRace->mVCalc->angle_get_difference(ThingData.Movement.AngleXY, v5);
              //not sure about the next 2 lines below?
              Tumble.AngleXY = (difference + 360.0f) / 20.0f;
              Tumble.AngleZY = -difference / 7.0f;
           }

           Tumble.Count = 20;
           FlightModel.Flag.BarrelRoll = true;
       }
    }
}

//Returns true if BarrelRoll is not yet fininshed, False
//if BarrelRoll has ended
bool VVehicle::vehicle_do_tumble() {
    int16_t count;

    if (FlightModel.Flag.BarrelRoll) {
        count = Tumble.Count;
        Tumble.Count = (count - 1);
        if (count < 2) {
            //End BarrelRoll again
            FlightModel.Flag.BarrelRoll = false;
        }
    }

    return FlightModel.Flag.BarrelRoll;
}

void VVehicle::vehicle_colide_vectors(irr::core::vector3df& delta) {
    int16_t v5 = 5;
    irr::core::vector3df position2 = ThingData.Position;
    position2 += delta;

    irr::f32 xy;
    irr::f32 trackCollVecAngle;
    irr::f32 angleDiff;
    irr::f32 angleDiff2;
    irr::f32 angleDiff3;
    irr::f32 v11;
    irr::f32 v13;
    irr::f32 v15;
    irr::f32 v16;
    irr::f32 v19;
    irr::f32 v20;
    irr::f32 aligmentErrorSgnd;
    irr::f32 aligmentErrorAbs;
    irr::f32 v24;
    bool v8;
    bool v18;
    irr::f32 thingMoveAngleXY;

    //Pad6 flag seems to be used for vehicle collision
    //with vectors
    FlightModel.FunctionFlag.Pad6 = false;
    while ( mRace->mVTrack->track_vector_collide(ThingData.Position, position2)) {
        if (!--v5) {
            goto vehicle_colide_vectors_LABEL31;
        }
        FlightModel.FunctionFlag.Pad6 = true;

        xy = mRace->mVCalc->angle_get_xy(ThingData.Position, position2);
        mRace->mVCalc->UnwrapPhaseSigned(xy);

        angleDiff3 = mRace->mVCalc->angle_get_difference(xy, mRace->mVTrack->TrackCollisionVectorAngle);

        if (angleDiff3 < 0.0f) {
            v8 = ((-angleDiff3) < 90.0054931640625f);
        } else {
            v8 = (angleDiff3 < 90.0054931640625f);
        }

        if (v8) {
            trackCollVecAngle = mRace->mVTrack->TrackCollisionVectorAngle;
        } else {
            trackCollVecAngle = mRace->mVTrack->TrackCollisionVectorAngle + 180.0f;
        }

        v11 = trackCollVecAngle;
        angleDiff = mRace->mVCalc->angle_get_difference(xy, trackCollVecAngle);
        v13 = sqrt(delta.X * delta.X + delta.Y * delta.Y);
        v15 = angleDiff;

        delta.X = 0.0f;
        delta.Y = 0.0f;
        delta.Z = 0.0f;

        if ( fabs(v15) > 0.0054931640625f) {
           v16 = fabs(v15);
           v11 += 4.998779296875f * (v15 / v16);
        }

        //uncomment next line for testing of second part
        //v13 = 0.0f;
        mRace->mVCalc->move_displacement_set(delta, v11, 0.0f, v13);

        //2nd Part: The second part below is a control loop which aligns the players
        //current flight direction/view along the "barrier"/vector so that the craft
        //moves along the vector

        thingMoveAngleXY = ThingData.Movement.AngleXY;
        mRace->mVCalc->UnwrapPhaseSigned(thingMoveAngleXY);

        angleDiff2 = mRace->mVCalc->angle_get_difference(thingMoveAngleXY, mRace->mVTrack->TrackCollisionVectorAngle);

        //std::cout << "Thing Move AngleXY: " << ThingData.Movement.AngleXY << " TrackCollVecAngle: " << mRace->mVTrack->TrackCollisionVectorAngle << " angleDiff2 = " << angleDiff2 << " v13 = " << v13 << std::endl;

        if (angleDiff2 < 0.0f) {
              v18 = (-angleDiff2 < 90.0054931640625f);
        } else {
            v18 = (angleDiff2 < 90.0054931640625f);
        }

        if (v18) {
            v19 = mRace->mVTrack->TrackCollisionVectorAngle;
        } else {
            v19 = mRace->mVTrack->TrackCollisionVectorAngle + 180.0f;
        }

        aligmentErrorSgnd = mRace->mVCalc->angle_get_difference(thingMoveAngleXY, v19);

        v20 = aligmentErrorSgnd;
        aligmentErrorAbs = fabs(aligmentErrorSgnd);

        if ( aligmentErrorAbs >= 0.999755859375f) {
            //std::cout << "Error: " << v20 << std::endl;
            v24 = v20 / 16.0f;
            if (v20 < 0.0f) {
                v24 = (v20 + 0.0823974609375f) / 16.0f;
            }
        } else {
            //std::cout << "No error" << std::endl;
            //I do not understand why according to original we add something to the angle if there is no
            //error anymore; This causes all kind of weird affects for me; Therefore replaced
            //original next line with 0.0f value below. But maybe this causes other problems I
            //do not know yet
            v24 = 1.99951171875f;
            //v24 = 0.0f;
        }

        ThingData.Movement.AngleXY += v24;

        mRace->mVCalc->UnwrapPhaseSigned(ThingData.Movement.AngleXY);

        position2 = ThingData.Position;
        position2 += delta;
    }

    if (v5)
        return;

vehicle_colide_vectors_LABEL31:
    delta.X = 0.0f;
    delta.Y = 0.0f;
}

bool VVehicle::OrientedBBoxCollision(VVehicle* vehicle1, VVehicle* vehicle2, irr::core::vector3df& collNormal, irr::f32& depth) {

    vehicle1->mCraftNode->updateAbsolutePosition();
    vehicle2->mCraftNode->updateAbsolutePosition();

    VECTOR Pa;
    irr::core::vector3df center1 = vehicle1->mCraftNode->getAbsolutePosition();

    Pa.x = center1.X;
    Pa.y = center1.Y;
    Pa.z = center1.Z;

    irr::core::aabbox3df boxLocal1 = vehicle1->mCraftNode->getBoundingBox();
    irr::core::aabbox3df boxLocal2 = vehicle2->mCraftNode->getBoundingBox();

    irr::core::vector3df ext1 = boxLocal1.getExtent();
    VECTOR a;
    a.x = ext1.X / 2.0f;
    a.y = ext1.Y / 2.0f;
    a.z = ext1.Z / 2.0f;

    VECTOR Pb;
    //irr::core::vector3df center2 = box2.getCenter();
    irr::core::vector3df center2 = vehicle2->mCraftNode->getAbsolutePosition();
    Pb.x = center2.X;
    Pb.y = center2.Y;
    Pb.z = center2.Z;

    irr::core::vector3df ext2 = boxLocal2.getExtent();
    VECTOR b;
    b.x = ext2.X / 2.0f;
    b.y = ext2.Y / 2.0f;
    b.z = ext2.Z / 2.0f;

    VECTOR A[3];
    //A.x = 1;
    //A.y = 1;
    //A.z = 1;

    irr::core::matrix4 mat = vehicle1->mCraftNode->getAbsoluteTransformation();

    irr::core::vector3df origin(0.0f, 0.0f, 0.0f);
    mat.transformVect(origin);

    irr::core::vector3df hlp(1.0f, 0.0f, 0.0f);
    mat.transformVect(hlp);

    A[0].x = hlp.X - origin.X;
    A[0].y = hlp.Y - origin.Y;
    A[0].z = hlp.Z - origin.Z;

    hlp.set(0.0f, 1.0f, 0.0f);
    mat.transformVect(hlp);

    A[1].x = hlp.X - origin.X;
    A[1].y = hlp.Y - origin.Y;
    A[1].z = hlp.Z - origin.Z;

    hlp.set(0.0f, 0.0f, 1.0f);
    mat.transformVect(hlp);

    A[2].x = hlp.X - origin.X;
    A[2].y = hlp.Y - origin.Y;
    A[2].z = hlp.Z - origin.Z;


    mat = vehicle2->mCraftNode->getAbsoluteTransformation();
    origin.set(0.0f, 0.0f, 0.0f);
    mat.transformVect(origin);

    hlp.set(1.0f, 0.0f, 0.0f);
    mat.transformVect(hlp);

    VECTOR B[3];
    B[0].x = hlp.X - origin.X;
    B[0].y = hlp.Y - origin.Y;
    B[0].z = hlp.Z - origin.Z;

    hlp.set(0.0f, 1.0f, 0.0f);
    mat.transformVect(hlp);

    B[1].x = hlp.X - origin.X;
    B[1].y = hlp.Y - origin.Y;
    B[1].z = hlp.Z - origin.Z;

    hlp.set(0.0f, 0.0f, 1.0f);
    mat.transformVect(hlp);

    B[2].x = hlp.X - origin.X;
    B[2].y = hlp.Y - origin.Y;
    B[2].z = hlp.Z - origin.Z;

    //AABB aabox1(Pa, a);
    //AABB aabox2(Pb, b);

    //DbgCollStartVec = center1 + ext1/2.0f;
    //DbgCollEndVec = center2 + ext2 / 2.0f;

    //return aabox1.overlaps(aabox2);

    //VECTOR* A, //orthonormal basis
    //VECTOR* B //orthonormal basis

    VECTOR resnormal;
    float resdepth;

    bool result = OBBOverlap(a, Pa, &A[0], b, Pb, &B[0], &resnormal, &resdepth);

    if (result) {
        collNormal.X = resnormal.x;
        collNormal.Y = resnormal.y;
        collNormal.Z = resnormal.z;
        depth = resdepth;
    }

    return result;
}

//Returns true if collision, false otherwise
bool VVehicle::VehiclesCheckForCollision(VVehicle* vehicle1, VVehicle* vehicle2,
           irr::core::vector3df& collNormal, irr::f32& depth) {
    //step 1 of collision detection: Sphere-To-Sphere collision detection
    vehicle1->mCraftNode->updateAbsolutePosition();
    vehicle2->mCraftNode->updateAbsolutePosition();
    irr::core::vector3df pos1Obj = vehicle1->mCraftNode->getAbsolutePosition();
    irr::core::vector3df pos2Obj = vehicle2->mCraftNode->getAbsolutePosition();

    irr::f32 distSquared = (pos1Obj - pos2Obj).getLengthSQ();

    //execute sphere-to-sphere collision detection, if negative return with false
   // if (distSquared > (obj1->objBoundingBoxExtendSquared + obj2->objBoundingBoxExtendSquared))
   //     return false;

    //Step 2: execute AABounding box collision using Irrlicht
    //first update axis aligned bounding boxes of both objects
   // obj1->objBoundingBox = obj1->sceneNode->getTransformedBoundingBox();
   // obj2->objBoundingBox = obj2->sceneNode->getTransformedBoundingBox();

   // DbgRunCollisionDetectionStage2 = 1.0f;
/*    irr::core::vector3df mtv;

    if (returnMTV(obj1, obj2, mtv)) {
        collNormal = mtv;
        return true;
    }*/
/*
    if (obj1->objBoundingBox.intersectsWithBox(obj2->objBoundingBox))
    {
        //for normal and collision depth simply assume sphere-sphere collision again
        collNormal = (pos2Obj - pos1Obj);
        depth = (collNormal.getLength() - sqrt(obj1->objBoundingBoxExtendSquared) - sqrt(obj2->objBoundingBoxExtendSquared));

        collNormal.normalize();

        return true;
    }*/

    if (OrientedBBoxCollision(vehicle1, vehicle2, collNormal, depth)) {
        return true;
    }

   return false;
}

void VVehicle::vehicle_post_process() {    
    irr::f32 speedFixed;
    int16_t number;
    int16_t v12;

    /********************************
     * Fuel reduction due to moving *
     ********************************/

    //LapCounter value is set to nonzero if the vehicle
    //crossed the finish line checkpoint the first time
    //after the beginning of the race
    if (LapCounter) {
        speedFixed = mRace->mVCalc->FloatToFixedPoint8D8(ThingData.Movement.SpeedActual) + 39;
        Stats.Fuel -= (speedFixed / 40);

        if (Stats.Fuel < 0) {
            Stats.Fuel = 0;
        }

        Conditions.FuelUsed += (speedFixed / 40);
    } else {
        //Vehicle has not yet crossed the finish line checkpoint
        //after beginning of the race
        ThingData.AffectStatus |= 0x80u;
    }

    if (Stats.Invincable > 0) {
        --Stats.Invincable;
    }

    if (Stats.Invisible <= 0) {
        ThingData.Status &= ~0x2u;
    } else {
        --Stats.Invisible;
        ThingData.Status |= 0x2u;
    }

    /********************************
     * Are we dealt any damage?     *
     ********************************/

    if (CollisionSound != nullptr) {
        if (CollisionSound->getStatus() == sf::SoundSource::Status::Stopped) {
            CollisionSound = nullptr;
        }
    }

    //If we collide this 0x200 flag is set
    if ((ThingData.AffectStatus & 0x200) != 0) {
        //if Human player play collision sound
        if (ControlOrigin == 1) {
               if (CollisionSound == nullptr) {
                  CollisionSound = mRace->mSoundEngine->PlaySound(SRES_GAME_COLLISION, false);
               }
        }
    }

    if (((ThingData.AffectStatus & 0x607u) != 0) && (Stats.Invincable <= 0)) {
        number = ThingData.AffectNumber;  //contains the value of damage dealt by a certain event
        v12 = -10000;
        if ((number < -10000) || (v12 = 10000, number >= 10001)) {
            ThingData.AffectNumber = v12;
        }

        //Subtract dealt damage from health
        Stats.Health -= ThingData.AffectNumber;

        if (Stats.Health < 0) {
            Stats.Health = 0;
        }

        Damage.ShimmerCount += ThingData.AffectNumber;
        if (Damage.ShimmerCount >= 1001) {
            Damage.ShimmerCount = 1000;
        }

        //Add up taken damage for stat calculations
        Conditions.HealthUsed += ThingData.AffectNumber;

        //sample_play(thing, 16);

        //it seems Flag 0x4 in AffectStatus means we have taken a machinegun bullet
        if ((ThingData.AffectStatus & 4) != 0) {
            ++Damage.BulletCount;
        }

        //it seems Flag 0x1000000 in AffectStatus means we have taken a missile
        if ((ThingData.AffectStatus & 0x1000000) != 0) {
            ++Damage.MissileCount;
        }

        //what does this do?
        //if ((ThingData.AffectStatus & 0x1) != 0) {
            //sample_set_pitch(thing, 16, 240);
        //}

        if ((ThingData.AffectStatus & 0x200) != 0) {
            ++Conditions.BumpAmount;

            //if Human player play collision sound
            if (ControlOrigin == 1) {
                   if (CollisionSound == nullptr) {
                      CollisionSound = mRace->mSoundEngine->PlaySound(SRES_GAME_COLLISION, false);
                   }
            }
        }

        ThingData.AffectNumber = 0;
    }

    Damage.BulletHoles = (Damage.BulletCount / 0xAu);

    //Has this player already driven through the finish line checkpoint
    //at least once after start of the race? Means the race has started
    //for this player
    if ((ThingData.AffectStatus & 0x800) != 0) {
        if (Stats.Invincable <= 0) {
            Stats.Health -= ThingData.AffectNumber;

            if (Stats.Health < 0) {
                Stats.Health = 0;
            }

            Damage.ShimmerCount += ThingData.AffectNumber;
            if (Damage.ShimmerCount >= 1001) {
                Damage.ShimmerCount = 1000;
            }

            //Add up taken damage for stat calculations
            Conditions.HealthUsed += ThingData.AffectNumber;

            if (Stats.VehicleHit) {
                if (Stats.VehicleHit == 1) {
                    //what does this exactly play?
                    //sample_play(thing, 26);
                    //sample_set_pitch(thing, 26, 60);
                    Stats.VehicleHit = 2;
                }
            } else {
                Stats.VehicleHit = 1;
                //what does this exactly play?
                //sample_play(thing, 16);
            }
        }
    } else {   //Vehicle has not yet passed the finish line checkpoint the first time
               //after race start
        Stats.VehicleHit = 0;
        //sample_stop(thing, 26);
    }

    //if (Stats.Health < 7500) {
      //sample_play(thing, 0);
    //}

    //if (Conditions.FlagKill) {
        //sample_play(thing, 0);
    //}

    //if (Conditions.FlagDeath) {
    //    sample_play(thing, 0);
    //}

    /********************************
     * Charging station effects     *
     ********************************/

    bool atCharger = false;

    mCurrChargingFuel = false;
    mCurrChargingAmmo = false;
    mCurrChargingShield = false;

    //only allow charging
    //if vehicle action is currently 1
    if (ThingData.Action == 0x1) {
        //Are we currently in an rearming station?
        if ((ThingData.AffectStatus & 0x8) != 0) {
            if (Stats.Weapons < 10000) {
                //TODO: This code is supposed to run approx. every
                //~50ms in the Playstation1 version of the game
                //If we run with different speed we need to adjust the
                //value we add below to keep charging speed constant
                Stats.Weapons += 200;

                atCharger = true;
                mCurrChargingAmmo = true;
            } else {

                    /*if (Conditions.WeaponsRechargeCounter) {
                    Conditions.WeaponsFullCounter = 50;
                   }*/

                if (mHUD != nullptr) {
                        if (!mBlockAdditionalAmmoFullMsg) {
                            mHUD->CancelAllPermanentBannerTextMsg();
                            this->mHUD->ShowBannerText((char*)"AMMO FULL", 4.0f);
                            mBlockAdditionalAmmoFullMsg = true;
                        }
                }
            }
        }

        //Are we currently in an fuel charging station?
        if ((ThingData.AffectStatus & 0x10) != 0) {
            if (Stats.Fuel < 10000) {
                //TODO: This code is supposed to run approx. every
                //~50ms in the Playstation1 version of the game
                //If we run with different speed we need to adjust the
                //value we add below to keep charging speed constant
                Stats.Fuel += 200;

                atCharger = true;
                mCurrChargingFuel = true;
            } else {

                /*if (Conditions.FuelRechargeCounter) {
                Conditions.FuelFullCounter = 50;
                }*/

                if (mHUD != nullptr) {
                       if (!mBlockAdditionalFuelFullMsg) {
                           mHUD->CancelAllPermanentBannerTextMsg();
                           this->mHUD->ShowBannerText((char*)"FUEL FULL", 4.0f);
                           mBlockAdditionalFuelFullMsg = true;
                       }
                }
            }
        }

        //Are we currently in an shield repair station?
        if ((ThingData.AffectStatus & 0x20) != 0) {
            if (Stats.Health > 0) {
                if (Stats.Health < 10000) {
                    //TODO: This code is supposed to run approx. every
                    //~50ms in the Playstation1 version of the game
                    //If we run with different speed we need to adjust the
                    //value we add below to keep charging speed constant
                    Stats.Health += 200;

                    atCharger = true;
                    mCurrChargingShield = true;

                    //TODO: something still not implemented with BulletCount and
                    //MissileCount
                } else {

                        /*if (Conditions.HealthRechargeCounter) {
                               Conditions.HealthFullCounter = 50;
                        }*/

                        Damage.BulletCount = 0;
                        Damage.MissileCount = 0;

                        if (mHUD != nullptr) {
                                if (!this->mBlockAdditionalShieldFullMsg) {
                                    mHUD->CancelAllPermanentBannerTextMsg();
                                    this->mHUD->ShowBannerText((char*)"SHIELD FULL", 4.0f);
                                    mBlockAdditionalShieldFullMsg = true;
                                }
                        }
                }
            }
        }

    } //End of If vehicle Action == 1

    if (ThingData.Action == 0x17) {
        if ((ThingData.AffectStatus & 8) != 0) {
            if (Stats.Weapons < 10000) {
                mCurrChargingAmmo = true;
                atCharger = true;
                Stats.Weapons += 200;
            }
        }

        if ((ThingData.AffectStatus & 0x10) != 0) {
            if (Stats.Fuel < 10000) {
                mCurrChargingFuel = true;
                atCharger = true;
                Stats.Fuel += 200;
            }
        }

        if ((ThingData.AffectStatus & 0x20) != 0) {
            Stats.Invincable = 2;

            if ((Stats.Health > 0) || (FlightModel.Flag.AutoDrive)) {
                if (Stats.Health < 10000) {
                    mCurrChargingShield = true;
                    atCharger = true;
                    Stats.Health += 200;
                }
            }
        }
    }

    //This is mode code implementation, not done this way in the original game!
    if (mCurrChargingFuel != mLastChargingFuel) {
        if (mCurrChargingFuel) {
            //charging fuel started
             if (atCharger) {
               if (mHUD != nullptr) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"FUEL RECHARGING", -1.0f);

                mBlockAdditionalFuelFullMsg = false;
               }
             }
        } else {
            if (mHUD != nullptr) {
                this->mHUD->CancelAllPermanentBannerTextMsg();
            }
        }
    }

    if (mCurrChargingAmmo != mLastChargingAmmo) {
        if (mCurrChargingAmmo) {
            //charging Ammo started
             if (atCharger) {
               if (mHUD != nullptr) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"AMMO RECHARGING", -1.0f);

                mBlockAdditionalAmmoFullMsg = false;
               }
             }
        } else {
            if (mHUD != nullptr) {
                this->mHUD->CancelAllPermanentBannerTextMsg();
            }
        }
    }

    if (mCurrChargingShield != mLastChargingShield) {
        if (mCurrChargingShield) {
            //charging shield started
             if (atCharger) {
               if (mHUD != nullptr) {
                //make this a permanent message by specification of showDurationSec = -1.0f
                this->mHUD->ShowBannerText((char*)"SHIELD RECHARGING", -1.0f);

                mBlockAdditionalShieldFullMsg = false;
               }
             }
        } else {
            if (mHUD != nullptr) {
                this->mHUD->CancelAllPermanentBannerTextMsg();
            }
        }
    }

    mLastChargingFuel = mCurrChargingFuel;
    mLastChargingAmmo = mCurrChargingAmmo;
    mLastChargingShield = mCurrChargingShield;

    if (atCharger) {
         if (mPlayerCurrentlyCharging == false) {
                mPlayerCurrentlyCharging = true;

                //play sound
                //only for human player
                if (ControlOrigin == 1) {
                    //we need to keep a pntr to the looping sound source to be able to stop it
                    //later again!
                    mChargingSoundSource = this->mRace->mSoundEngine->PlaySound(SRES_GAME_REFUEL, true);
                }
        }
    } else {
        if (mPlayerCurrentlyCharging == true) {
               mPlayerCurrentlyCharging = false;

               //stop playing sound from looping sound source
               //only for human player
               if (ControlOrigin == 1) {
                   this->mRace->mSoundEngine->StopLoopingSound(mChargingSoundSource);
                   mChargingSoundSource = nullptr;
               }
       }
    }

    //only use collectables if Action == 1
    //if vehicle action is currently 1
    if (ThingData.Action == 1) {

            /********************************
             * Collectable effects          *
             ********************************/

            //Player picks up Invisible powerup?
            if ((ThingData.AffectStatus & 0x100) != 0) {
                Stats.Invisible = 250;
            }

            //Player picks up Invincable powerup?
            //This AffectStatus 0x80 flag seems to be also
            //set before the first player passes the final race
            //check point the first time.
            if ((ThingData.AffectStatus & 0x80) != 0) {
                Stats.Invincable = 100;
            }

            //Player picks up a minigun upgrade?
            if ((ThingData.AffectStatus & 0x1000) != 0) {
                ++Stats.MGunUpgrade;
            }

            //Player picks up a rocket upgrade?
            if ((ThingData.AffectStatus & 0x2000) != 0) {
                ++Stats.MRocketUpgrade;
            }

            //Player picks up a booster upgrade?
            if ((ThingData.AffectStatus & 0x4000) != 0) {
                ++Booster.Upgrade;
            }

            //Player picks up a HealthExtra PowerUp?
            if ((ThingData.AffectStatus & 0x8000) != 0) {
                Stats.Health += 2500;

                if (Damage.BulletCount < 0xB) {
                    Damage.BulletCount = 0;
                } else {
                    Damage.BulletCount -= 10;
                }
            }

            //Player picks up a HealthFull PowerUp?
            if ((ThingData.AffectStatus & 0x10000) != 0) {
                Stats.Health = 10000; //Full health equals to 10000
                Damage.BulletCount = 0;
                Damage.BulletHoles = 0;
                Damage.MissileCount = 0;
            }

            //Player picks up a HealthDouble PowerUp?
            if ((ThingData.AffectStatus & 0x20000) != 0) {
                Stats.Health = 20000; //Double health equals to 20000
                Damage.BulletCount = 0;
                Damage.BulletHoles = 0;
                Damage.MissileCount = 0;
            }

            //Player picks up a Ammo PowerUp?
            if ((ThingData.AffectStatus & 0x40000) != 0) {
                Stats.Weapons += 2500;
            }

            //Player picks up a Ammo Full PowerUp?
            if ((ThingData.AffectStatus & 0x80000) != 0) {
                Stats.Weapons = 10000;
            }

            //Player picks up a Ammo Double PowerUp?
            if ((ThingData.AffectStatus & 0x100000) != 0) {
                Stats.Weapons = 20000;
            }

            //Player picks up a Fuel Extra PowerUp?
            if ((ThingData.AffectStatus & 0x200000) != 0) {
                Stats.Fuel += 2500;
            }

            //Player picks up a Fuel Full PowerUp?
            if ((ThingData.AffectStatus & 0x400000) != 0) {
                Stats.Fuel = 10000;
            }

            //Player picks up a Fuel Double PowerUp?
            if ((ThingData.AffectStatus & 0x800000) != 0) {
                Stats.Fuel = 20000;
            }

            /*********************************
             * Keeping track of death counts *
             *********************************/

            if (Stats.Health <= 0) {
                //Add this stuff later!
            }

    }   //End of If vehicle Action == 1

   /* if (currChargingAmmo) {
        ++Conditions.WeaponsRechargeCounter;
    } else {
        Conditions.WeaponsRechargeCounter = 0;
    }

    if (currChargingHealth) {
        ++Conditions.HealthRechargeCounter;
    } else {
        Conditions.HealthRechargeCounter = 0;
    }

    if (currChargingFuel) {
        ++Conditions.FuelRechargeCounter;
    } else {
        Conditions.FuelRechargeCounter = 0;
    }

    if (currChargingAmmo || currChargingFuel || currChargingHealth) {
        //only play sound for Human controlled player
        if (ControlOrigin == 1) {
            this->mRace->mSoundEngine->PlaySound(SRES_GAME_REFUEL);
        }
    }*/

    /********************************
     * Fuel Low/Empty warnings      *
     ********************************/

    int8_t fuelLowCounter;

    if ((Stats.Fuel - 1) >= 4000) {
        Conditions.FuelLowCounter = 0;
        mLowFuelWarningAlreadyShown = false;
    } else {
        fuelLowCounter = Conditions.FuelLowCounter;
        if (fuelLowCounter < 64) {
            Conditions.FuelLowCounter = fuelLowCounter + 1;
        }
    }

    if (!mEmptyFuelWarningAlreadyShown && (Stats.Fuel <= 0)) {
        if (mHUD != nullptr) {
            this->mHUD->ShowBannerText((char*)"FUEL EMPTY", 4.0f, true);
        }
        mEmptyFuelWarningAlreadyShown = true;
    }

    if (mEmptyFuelWarningAlreadyShown && (Stats.Fuel > 0)) {
          mEmptyFuelWarningAlreadyShown = false;
    }

    if ((!mLowFuelWarningAlreadyShown) && (!mEmptyFuelWarningAlreadyShown) && (Conditions.FuelLowCounter == 1)) {
        if (mHUD != nullptr) {
            this->mHUD->ShowBannerText((char*)"FUEL LOW", 4.0f, true);
        }
        mLowFuelWarningAlreadyShown = true;
    }

    /********************************
     * Shield Low/Empty warnings    *
     ********************************/

    int8_t healthLowCounter;

    if ((Stats.Health - 1) >= 4000) {
        Conditions.HealthLowCounter = 0;
        mLowShieldWarningAlreadyShown = false;
    } else {
        healthLowCounter = Conditions.HealthLowCounter;
        if (healthLowCounter < 64) {
            Conditions.HealthLowCounter = healthLowCounter + 1;
        }
    }

    if ((!mLowShieldWarningAlreadyShown) && (Conditions.HealthLowCounter == 1)) {
        if (mHUD != nullptr) {
            this->mHUD->ShowBannerText((char*)"SHIELD LOW", 4.0f, true);
        }
        mLowShieldWarningAlreadyShown = true;
    }

    /********************************
     * Ammo Low/Empty warnings      *
     ********************************/

    int8_t ammoLowCounter;

    if ((Stats.Weapons - 1) >= 4000) {
        Conditions.WeaponsLowCounter = 0;
        mLowAmmoWarningAlreadyShown = false;
    } else {
        ammoLowCounter = Conditions.WeaponsLowCounter;
        if (ammoLowCounter < 64) {
            Conditions.WeaponsLowCounter = ammoLowCounter + 1;
        }
    }

    if ((!mLowAmmoWarningAlreadyShown) && (Conditions.WeaponsLowCounter == 1)) {
        if (mHUD != nullptr) {
            this->mHUD->ShowBannerText((char*)"AMMO LOW", 4.0f, true);
        }
        mLowAmmoWarningAlreadyShown = true;
    }

    //makes sure to remove all existing effects
    //on the vehicle
    ThingData.AffectStatus = 0;
    ThingData.AffectNumber = 0;
    ThingData.AffectWho = 0;
    vehicle_get_checkpoint();
    Stats.Velocity *= 1.3f;
}

//Returns true if vehicle is controlled by
//computer player
bool VVehicle::IsControlledByComputer() {
    return (ControlOrigin == 8);
}

uint8_t VVehicle::vehicle_colide_final_check_sean(std::vector<VVehicle*> &vehicleVec, irr::core::vector3df& delta) {
     std::vector<VVehicle*>::iterator it;
     irr::f32 v8;
     irr::f32 v9;
     irr::f32 v10;
     irr::f32 v16;
     irr::f32 v12;
     irr::f32 v14;
     irr::f32 v15;
     irr::f32 v19;
     irr::f32 xy;
     irr::f32 v13;
     irr::f32 difference;
     irr::core::vector3df positionFrom;
     bool collided = false;

     v16 = this->ThingData.Position.Z;
     v14 = this->ThingData.Position.X + delta.X;
     v15 = this->ThingData.Position.Y + delta.Y;
     positionFrom.X = 0.0f;
     positionFrom.Y = 0.0f;
     positionFrom.Z = 0.0f;

     for (it = vehicleVec.begin(); it != vehicleVec.end(); ++it) {
         if ((*it) != this) {
             v8 = fabs(((*it)->ThingData.Position.Z - v16));
             if (v8 < 0.390625f) {
                v9 = fabs(((*it)->ThingData.Position.X - v14));
                if (v9 < 0.3515625f) {
                    v10 = fabs(((*it)->ThingData.Position.Y - v15));
                    if (v10 < 0.3515625f) {
                        collided = true;
                        break;
                    }
                }
             }
         }
     }

     if (!collided) {
         mDbgColl = 0.0f;
         return 0;
     }

     positionFrom.X = -v9;
     positionFrom.Y = -v10;
     //positionFrom.Z = v8;

    //we are collided right now with a craft
    xy = mRace->mVCalc->angle_get_xy(positionFrom, delta);
    difference = mRace->mVCalc->angle_get_difference(this->ThingData.Movement.AngleXY, xy);
    v19 = difference;

    if (fabs(v19) < 0.00390625f) {
       // v19 = 0.00390625f;
    }

    v12 = fabs(v19);

    if (v12 >= 0.00390625f) {
        v13 = -0.078125f * (v19 / v12);
    } else {
        v13 = 0.0f;
    }

    mRace->mVCalc->move_xyz(delta, xy + 90.0f, 0.0f, v13);
    mDbgColl = 1.0f;
    return 1;
}

uint8_t VVehicle::vehicle_colide_my_attempt(std::vector<VVehicle*> &vehicleVec, irr::core::vector3df& delta) {
     std::vector<VVehicle*>::iterator it;
     irr::core::vector3df collNormal;
     irr::f32 collDepth;

    mDbgColl = 0.0f;

     for (it = vehicleVec.begin(); it != vehicleVec.end(); ++it) {
         if ((*it) != this) {
             //did this 2 vehicles collide?
             if (VehiclesCheckForCollision(this, (*it), collNormal, collDepth)) {
                 //collision
                 collNormal.normalize();
                 delta += collNormal * 0.1f;

                 this->ThingData.Position -= collNormal * collDepth * 0.5f;
                 (*it)->ThingData.Position += collNormal * collDepth * 0.5f;
             }

         }
     }

     return 0;
/*
    v12 = fabs(v19);

    if (v12 >= 0.00390625f) {
        v13 = -0.078125f * (v19 / v12);
    } else {
        v13 = 0.0f;
    }

    mRace->mVCalc->move_xyz(delta, xy + 90.0f, 0.0f, v13);
    mDbgColl = 1.0f;
    return 1;*/
}

uint8_t VVehicle::vehicle_colide(std::vector<VVehicle*> &vehicleVec, irr::core::vector3df& delta) {
    std::vector<VVehicle*>::iterator it;
    irr:f32 Zpos;
    irr::f32 v10;
    irr::f32 v5 = 0.0f;
    irr::core::vector3df position2;  //holds the collision point if a collision occurs
    uint16_t v6 = 0;
    uint16_t v8 = 0;
    irr::core::vector3df v25;
    irr::core::vector3df v26;
    irr::core::vector3df u1;
    VVehicle* collVehicle = nullptr;
    irr::f32 v15;
    irr::f32 v17;
    irr::f32 squared_xy;

    position2.X = 0.0f;
    position2.Y = 0.0f;
    position2.Z = 0.0f;
    u1.X = 0.0f;
    u1.Y = 0.0f;
    u1.Z = 0.0f;
    v25.X = 0.0f;
    v25.Y = 0.0f;
    v25.Z = 0.0f;
    v26.X = 0.0f;
    v26.Y = 0.0f;
    v26.Z = 0.0f;

    for (it = vehicleVec.begin(); it != vehicleVec.end(); ++it) {
        if ((*it) != this) {
            Zpos = ((*it)->ThingData.Position.Z);
            v10 = this->ThingData.Position.Z;
            if ((Zpos - v10) < 0.0f) {
               if ((v10 - Zpos) < 0.390625f) {
vehicle_colide_LABEL7:
            if ((fabs((*it)->ThingData.Position.X - this->ThingData.Position.X) < 0.00390625f) &&
                (fabs((*it)->ThingData.Position.Y - this->ThingData.Position.Y) < 0.00390625f)) {
                delta.X = 0.234375f;
                delta.Y = 0.234375f;
            }
            if (mRace->mVCalc->collide_on_circle((*it)->ThingData.Position, this->ThingData.Position,
                                                 delta, 0.234375f, position2)) {

               if (v6) {
                  squared_xy = mRace->mVCalc->distance_get_squared_xy(this->ThingData.Position, position2);
                  if (squared_xy < v5) {
                      v5 = squared_xy;
                      v25 = position2;
                      collVehicle = (*it);
                  }
               } else {
                     v25 = position2;
                     collVehicle = (*it);
                     v5 = mRace->mVCalc->distance_get_squared_xy(this->ThingData.Position, position2);
               }
               ++v6;
            }
        }
    } else if ((Zpos - v10) < 0.390625f) {
                goto vehicle_colide_LABEL7;
            }
        }
        v8 = v6;
    }

    //no collision, return immediately
    if (!v8) {
        return 0;
    }

    //just to make sure that we can not crash!
    if (collVehicle == nullptr) {
        return 0;
    }

    u1.X = (collVehicle->Momentum.DeltaX / 4.0f);
    u1.Y = (collVehicle->Momentum.DeltaY / 4.0f);
    mRace->mVCalc->collide_inelastic(collVehicle->ThingData.Position, this->ThingData.Position,
                                     u1, delta, v26, delta, collVehicle->Bump);

    //Deal "bump damage" to vehicle that we collided with
    v15 = fabs(collVehicle->Bump.X);
    collVehicle->mBumpDamage += v15;

    v17 = fabs(collVehicle->Bump.Y);
    collVehicle->mBumpDamage += v17;

    //Deal "bump damage" to ourself as well
    mBumpDamage += v15;
    mBumpDamage += v17;

    return 1;
}

void VVehicle::UpdateSceneNode() {
    if (this->mCraftNode == nullptr) {
        return;
    }

    mRace->UpdateSceneNodeModel(mCraftNode, &View);
    mBoundingBox = mCraftNode->getTransformedBoundingBox();
}

void VVehicle::DrawDebug() {
    //dbgDraw FrontLeft sensor
    irr::core::vector3df irrSensFrontLeft =
            mRace->mVCalc->VanillaToIrrlichtCoord(FlightModel.FrontLeft.Position);

    irr::core::vector3df irrSensFrontRight =
            mRace->mVCalc->VanillaToIrrlichtCoord(FlightModel.FrontRight.Position);

    irr::core::vector3df irrSensRearLeft =
            mRace->mVCalc->VanillaToIrrlichtCoord(FlightModel.RearLeft.Position);

    irr::core::vector3df irrSensRearRight =
            mRace->mVCalc->VanillaToIrrlichtCoord(FlightModel.RearRight.Position);

    irr::core::vector3df irrCraftPos =
            mRace->mVCalc->VanillaToIrrlichtCoord(ThingData.Position);

    mRace->mGame->mDrawDebug->Draw3DLine(*mRace->mGame->mDrawDebug->origin,
                                         irrCraftPos, mRace->mGame->mDrawDebug->cyan);

    mRace->mGame->mDrawDebug->Draw3DLine(*mRace->mGame->mDrawDebug->origin,
                                         irrSensFrontLeft, mRace->mGame->mDrawDebug->red);

    mRace->mGame->mDrawDebug->Draw3DLine(*mRace->mGame->mDrawDebug->origin,
                                         irrSensFrontRight, mRace->mGame->mDrawDebug->orange);

    mRace->mGame->mDrawDebug->Draw3DLine(*mRace->mGame->mDrawDebug->origin,
                                         irrSensRearLeft, mRace->mGame->mDrawDebug->blue);

    mRace->mGame->mDrawDebug->Draw3DLine(*mRace->mGame->mDrawDebug->origin,
                                         irrSensRearRight, mRace->mGame->mDrawDebug->brown);
}

//NewPosition = New position of player craft center of gravity (world coordinates)
//NewFrontAt = defines where player craft front is located at (world coordinates)
void VVehicle::CalcCraftLocalFeatureCoordinates(irr::core::vector3d<irr::f32> NewPosition, irr::core::vector3d<irr::f32> NewFrontAt) {

    //simply set the craft original coodinates
    IrrLocalCraftOrigin.set(0.0f, 0.0f, 0.0f);

    irr::core::vector3df pos_in_worldspace_frontPos(NewFrontAt);
    this->mCraftNode->updateAbsolutePosition();
    irr::core::matrix4 matr = this->mCraftNode->getAbsoluteTransformation();
    matr.makeInverse();

    matr.transformVect(pos_in_worldspace_frontPos);

    //calculates new local coordinate for front of craft
    IrrLocalCraftFrontPnt = pos_in_worldspace_frontPos;

    irr::core::vector3df WCDirVecFrontToCOG = (NewPosition - NewFrontAt);
    irr::core::vector3df WCDirVecCOGtoBack = NewPosition + WCDirVecFrontToCOG;

    matr.transformVect(WCDirVecCOGtoBack);
    IrrLocalCraftBackPnt = WCDirVecCOGtoBack;

    irr::core::vector3d<irr::f32> VectorUp(0.0f, 1.0f, 0.0f);
    WCDirVecFrontToCOG.normalize();
    irr::core::vector3df sideDirToLeft = WCDirVecFrontToCOG.crossProduct(VectorUp);

    sideDirToLeft.normalize();
    irr::core::vector3df WCDirVecCOGtoLeft = NewPosition - sideDirToLeft * WCDirVecFrontToCOG.getLength();

    matr.transformVect(WCDirVecCOGtoLeft);
    IrrLocalCraftLeftPnt = WCDirVecCOGtoLeft;

    irr::core::vector3df WCDirVecCOGtoRight = NewPosition + sideDirToLeft * WCDirVecFrontToCOG.getLength();
    matr.transformVect(WCDirVecCOGtoRight);
    IrrLocalCraftRightPnt = WCDirVecCOGtoRight;

    //get the size of the craft 3D model bounding box
    irr::core::vector3df hlpVec = this->mCraftNode->getTransformedBoundingBox().getExtent();

    //Create a position slightly in front of the craft that
    //allows to trigger things with the player craft
    IrrLocalCraftTriggerSensor.set(0.0f, 0.0f, -0.5f * hlpVec.Z - this->mRace->mLevelTerrain->segmentSize);

    //define where from the craft dust clouds are emitted, when hovering outside of the race
    //track
    IrrLocalCraftDustPnt.X = 0.0f;
    IrrLocalCraftDustPnt.Y = -hlpVec.Y * 0.3f;
    IrrLocalCraftDustPnt.Z = 0.0f;
}

void VVehicle::FinishedLap() {
    // //add new lap time to lap time list
    // //add the item in a way so that the list remains sorted
    // std::vector<LAPTIMEENTRY>::iterator idx;

    // //remember data from the last two laps, we want to access this information
    // //quickly from the Hud without searching in the lap time vector
    // //as we need this data every time we want to render a frame!
    // if (mPlayerStats->currLapNumber > 1) {
    //     mPlayerStats->LapBeforeLastLap.lapNr = mPlayerStats->lastLap.lapNr;
    //     mPlayerStats->LapBeforeLastLap.lapTimeMultiple40mSec = mPlayerStats->lastLap.lapTimeMultiple40mSec;
    // }

    // if (mPlayerStats->currLapNumber > 0) {
    //     mPlayerStats->lastLap.lapNr = mPlayerStats->currLapNumber;
    //     mPlayerStats->lastLap.lapTimeMultiple40mSec = mPlayerStats->currLapTimeMultiple40mSec;
    // }

    // //make sure we have at least one laptime entry
    // if (mPlayerStats->lapTimeList.size() > 0) {
    //     for(idx = mPlayerStats->lapTimeList.begin(); idx < mPlayerStats->lapTimeList.end(); idx++)
    //         {
    //             if (mPlayerStats->currLapTimeMultiple40mSec <  (*idx).lapTimeMultiple40mSec)
    //                 break;
    //         }
    // } else idx = mPlayerStats->lapTimeList.end();

    // LAPTIMEENTRY newEntry;
    // newEntry.lapNr = mPlayerStats->currLapNumber;
    // newEntry.lapTimeMultiple40mSec = mPlayerStats->currLapTimeMultiple40mSec;

    // mPlayerStats->lapTimeList.insert(idx, newEntry);

    //currLapNumber++;

    //LogMessage((char*)"I have finished the current lap");

    //has this player finished the last lap of this race?
    /*if (currLapNumber > raceNumberLaps) {
        FinishedRace();
    }*/

    //do we need to show HUD Message for "final lap"
    //TODO: commented out, does not compile anymore
    // if (mPlayerStats->currLapNumber == mPlayerStats->raceNumberLaps) {
    //     if (this->mRace->currPlayerFollow != nullptr) {
    //         if (this->mRace->currPlayerFollow == this) {
    //             if (mHUD != nullptr) {
    //                 mHUD->ShowGreenBigText((char*)"FINAL LAP", 4.0f, true);
    //             }

    //             //in demo mode prevent the yee-haw sound
    //             //from playing
    //             if (!mRace->mDemoMode) {
    //                 //play the yee-haw sound
    //                 mRace->mSoundEngine->PlaySound(SRES_GAME_FINALLAP, false);
    //             }
    //         }
    //     }
    // }

    //reset current lap time
    //mPlayerStats->currLapTimeExact = 0.0;
    //mPlayerStats->currLapTimeMultiple40mSec = 0;
}

void VVehicle::FinishedRace() {
    // /* after the player is finished with the race
    //  * the game uses the external view, while a
    //  * computer player takes over controlling this craft */
    // this->mCurrentViewMode = CAMERA_EXTERNALVIEW;

    // mPlayerStats->mHasFinishedRace = true;

    // //create a copy of the final player stats
    // *mFinalPlayerStats = *mPlayerStats;

    // //Update a connected HUD as well
    // //so that change of mHasFinishedRace changes
    // //the HUD state
    // UpdateHUDState();

    // LogMessage((char*)"I have finished the race");

    // //also tell the race that I am finished
    // mRace->PlayerHasFinishedLastLapOfRace(this);

    // //if this is a human player we need to reconfigure it
    // //as a computer player, so that the computer player
    // //can continue moving it over the race track
    // if (mHumanPlayer) {
    //     CpTakeOverHuman();
    // }
}

void VVehicle::CheckForTriggerCraftRegion() {
    //remember last trigger region before next update
    mLastCraftTriggerRegion = mCurrentCraftTriggerRegion;

    std::vector<MapTileRegionStruct*>::iterator itRegion;

    mCurrentCraftTriggerRegion = nullptr;

    //16.05.2025: There is a (hidden) shortcut in level 2 that is opened by "driving" into the
    //level wall. Problem is if we use the ships (origin middle) position to calculate the cell for craft trigger (which I did at the beginning),
    //this point does not reach into the trigger area of the shortcut (because the heightmap collision detection and prevention
    //prevents this middle coordinate to penetrate deep enough into the wall), and like this the way only opens when trying a lot of times,
    //and with a lot of luck. It works but not acceptable.
    //To make it work much better I decided to instead use a craft coordinate much further in the front of the craft, so that it can
    //penetrate deep enough, and cause the craft trigger to fire much much easier.
    int mTrigCurrPosCellX = -(int)(IrrWorldCraftTriggerSensor.X / mRace->mLevelTerrain->segmentSize);
    int mTrigCurrPosCellY = (int)(IrrWorldCraftTriggerSensor.Z / mRace->mLevelTerrain->segmentSize);

    //check for each trigger region in level
    for (itRegion = this->mRace->mTriggerRegionVec.begin(); itRegion != this->mRace->mTriggerRegionVec.end(); ++itRegion) {
        //only check for regions which are a playercraft trigger region
        if ((*itRegion)->regionType == LEVELFILE_REGION_TRIGGERCRAFT) {
            //is the player inside this area?
            if (this->mRace->mLevelTerrain->CheckPosInsideRegion(mTrigCurrPosCellX,
                    mTrigCurrPosCellY, (*itRegion))) {

                //assume craft can only be in one region at a certain time
                //craft trigger regions should not overlap!
                mCurrentCraftTriggerRegion = (*itRegion);
                break;
            }
        }
    }

    //did we enter a new trigger region?
    //if so we need to trigger the trigger event and tell the race
    //about it
    if (mCurrentCraftTriggerRegion != nullptr) {
        if (mCurrentCraftTriggerRegion != mLastCraftTriggerRegion) {
            //yes, we hit a new trigger region

            //is this a one time trigger only trigger?
            if (((*itRegion)->mOnlyTriggerOnce && (!(*itRegion)->mAlreadyTriggered))
                    || (!(*itRegion)->mOnlyTriggerOnce)) {
                       if ((*itRegion)->mOnlyTriggerOnce) {
                           (*itRegion)->mAlreadyTriggered = true;
                       }

                       mRace->PlayerEnteredCraftTriggerRegion(this, mCurrentCraftTriggerRegion);
            }
        }
    }
}

//is called when the player collected a collectable item of the
//level
bool VVehicle::CollectedCollectable(Collectable* whichCollectable) {
    //depending on the type of entity/collectable alter player stats
    Entity::EntityType type = whichCollectable->GetCollectableType();

    switch (type) {
        case Entity::EntityType::ExtraFuel:
            //ExtraFuel item can only be picked up by the player, if fuel is currently
            //not at max
            if (Stats.Fuel >= 10000) {
                //fuel is full, can not pick this item up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"EXTRA FUEL", 4.0f);
            }

            //Tell vehicle that we collected ExtraFuel
            ThingData.AffectStatus |= 0x200000;
            break;

        case Entity::EntityType::FuelFull:
            //FuelFull item can only be picked up by the player, if fuel is currently
            //not at max
            if (Stats.Fuel >= 10000) {
                //fuel is full, can not pick this item up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"FUEL FULL", 4.0f);
            }

            //Tell vehicle that we collected ExtraFull
            ThingData.AffectStatus |= 0x400000;
            break;

        case Entity::EntityType::DoubleFuel:
            //DoubleFuel item can only be picked up by the player, if fuel is not
            //already at 20000
            if (Stats.Fuel >= 20000) {
                //player has already doubled fuel level, can not pick this item up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"DOUBLE FUEL", 4.0f);
            }

            //Tell vehicle that we collected DoubleFuel
            ThingData.AffectStatus |= 0x800000;
            break;

        case Entity::EntityType::ExtraAmmo:
            //This item can only be picked up by the player if the ammo
            //is still below max level
            if (Stats.Weapons >= 10000) {
                //ammo already full, collectible is not picked up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"EXTRA AMMO", 4.0f);
            }

            //Tell vehicle that we collected ExtraAmmo
            ThingData.AffectStatus |= 0x40000;
            break;

        case Entity::EntityType::AmmoFull:
            //This item can only be picked up by the player if the ammo
            //is still below max level
            if (Stats.Weapons >= 10000) {
                //ammo already full, collectible is not picked up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"AMMO FULL", 4.0f);
            }

            //Tell vehicle that we collected AmmoFull
            ThingData.AffectStatus |= 0x80000;
            break;

        case Entity::EntityType::DoubleAmmo:
            //DoubleAmmo item can only be picked up by the player, if ammo is not
            //already at 20000
            if (Stats.Weapons >= 20000) {
                //player has already doubled ammo level, can not pick this item up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"DOUBLE AMMO", 4.0f);
            }

            //Tell vehicle that we collected DoubleAmmo
            ThingData.AffectStatus |= 0x100000;
            break;

        case Entity::EntityType::ExtraShield:
            //This item can only be picked up by the player,
            //if the player has not already full shield
            if (Stats.Health >= 10000) {
                //Shield already full, collectible is not picked up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"EXTRA SHIELD", 4.0f);
            }

            ThingData.AffectStatus |= 0x8000;
            break;

        case Entity::EntityType::ShieldFull:
            //This item can only be picked up by the player,
            //if the player has not already full shield
            if (Stats.Health >= 10000) {
                //Shield already full, collectible is not picked up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"SHIELD FULL", 4.0f);
            }

            ThingData.AffectStatus |= 0x10000;
            break;

        case Entity::EntityType::DoubleShield:
            //DoubleShield item can only be picked up by the player, if Shield is not
            //already at 20000
            if (Stats.Health >= 20000) {
                //player has already doubled Shield level, can not pick this item up
                return false;
            }

            if (mHUD != nullptr) {
                this->mHUD->ShowBannerText((char*)"DOUBLE SHIELD", 4.0f);
            }

            ThingData.AffectStatus |= 0x20000;
            break;

        case Entity::EntityType::BoosterUpgrade:
            //can only be picked up if booster upgrade level is not already
            //at max
            if (Booster.Upgrade != 3) {
                //we can make another upgrade
                ThingData.AffectStatus |= 0x4000;

                if (mHUD != nullptr) {
                    this->mHUD->ShowBannerText((char*)"BOOSTER UPGRADED", 4.0f);
                }
            } else {
                return false;
            }

            break;

        case Entity::EntityType::MissileUpgrade:
            //can only be picked up if missile upgrade level is not already
            //at max
            if (Stats.MRocketUpgrade != 3) {
                //we can make another upgrade
                ThingData.AffectStatus |= 0x2000;

                if (mHUD != nullptr) {
                    this->mHUD->ShowBannerText((char*)"MISSILE UPGRADED", 4.0f);
                }
            } else {
                return false;
            }

            break;

        case Entity::EntityType::MinigunUpgrade:
            //can only be picked up if mini-gun upgrade level is not already
            //at max
            if (Stats.MGunUpgrade != 3) {
                //we can make another upgrade
                ThingData.AffectStatus |= 0x1000;

                if (mHUD != nullptr) {
                    this->mHUD->ShowBannerText((char*)"MINIGUN UPGRADED", 4.0f);
                }
            } else {
                return false;
            }

            break;

        case Entity::EntityType::UnknownShieldItem:
            //uncomment the next 2 lines to show this items also to the player
            // collectable = new Collectable(41, entity.get_Center(), color, driver);
            // ENTCollectables_List.push_back(collectable);
            break;

        case Entity::EntityType::UnknownItem:
        case Entity::EntityType::Unknown:
            //uncomment the next 2 lines to show this items also to the player
            // collectable = new Collectable(50, entity.get_Center(), color, driver);
            // ENTCollectables_List.push_back(collectable);
            break;

         //all the other entities we do not care here
        default:
            break;
    }

    //play sound if not computer player
    if (this->ControlOrigin != 8) {
        mRace->mSoundEngine->PlaySound(SRES_GAME_PICKUP);
    }

    //collectible was picked up
    return true;
}

void VVehicle::CheckForChargingStation() {
    bool cShield;
    bool cFuel;
    bool cAmmo;

    int mCurrPosCellX = (int)(ThingData.Position.X / mRace->mLevelTerrain->segmentSize);
    int mCurrPosCellY = (int)(ThingData.Position.Y / mRace->mLevelTerrain->segmentSize);

    //see if we are currently in an charging area with this player
    this->mRace->mLevelTerrain->CheckPosInsideChargingRegion(mCurrPosCellX, mCurrPosCellY,
                                                             cShield, cFuel, cAmmo);

    if (cShield) {
        //Player craft is in shield charging area
        this->ThingData.AffectStatus |= 0x20;
    }

    if (cAmmo) {
       //Player craft is in ammo charging area
       this->ThingData.AffectStatus |= 0x8;
    }

    if (cFuel) {
        //Player craft is in fuel charging area
        this->ThingData.AffectStatus |= 0x10;
    }
}

//checks if current player should emit dust clouds below the craft
//this is the case if the player is above a "dusty" tile next to the race track
void VVehicle::CheckDustCloudEmitter() {
    int mCurrPosCellX = (int)(ThingData.Position.X / mRace->mLevelTerrain->segmentSize);
    int mCurrPosCellY = (int)(ThingData.Position.Y / mRace->mLevelTerrain->segmentSize);

    MapEntry* tilePntr = this->mRace->mLevelTerrain->GetMapEntry(mCurrPosCellX, mCurrPosCellY);
    irr::s32 texId = tilePntr->m_TextureId;

    mEmitDustCloud = false;

    //if craft is close enough to the terrain below
    //(and can) technically emit dust, continue checking for
    //texture Id, otherwise we will not emit Dust
    if ((ThingData.Position.Z - tilePntr->m_Height) < 1.8f) {
        //check if our texture ID is present in the dirt tex id list
        //if so then emit clouds
        for (std::vector<irr::s32>::iterator itTex = dirtTexIdsVec->begin(); itTex != dirtTexIdsVec->end(); ++itTex) {
            if ((*itTex) == texId) {
                mEmitDustCloud = true;
                break;
            }
        }
    }

    if (mEmitDustCloud != mLastEmitDustCloud) {
        if (mEmitDustCloud) {
            this->mDustBelowCraft->Activate();
        } else {
            this->mDustBelowCraft->Deactivate();
        }
    }

    mLastEmitDustCloud = mEmitDustCloud;
}

void VVehicle::SetMyHUD(HUD* pntrHUD) {
    mHUD = pntrHUD;

    //I got a new HUD connected
    //we need to tell the HUD the correct
    //HUD state we want for the current player
    //state we have
    UpdateHUDState();
}

HUD* VVehicle::GetMyHUD() {
    return mHUD;
}

irr::u32 VVehicle::GetCurrentState() {
    return this->mPlayerCurrentState;
}

void VVehicle::SetNewState(irr::u32 newPlayerState) {
    mPlayerCurrentState = newPlayerState;

    switch (newPlayerState) {
        case STATE_PLAYER_BEFORESTART: {
            mPlayerCanMove = false;
            mPlayerCanShoot = false;
            break;
        }

        //This is the inbetween state after green light comes on
        //and the first time a player crosses the finish line
        //in this state the players move towards the start line, and
        //computer players do not seem to attack
        //Human player is allowed to attack
        //Also the HUD is not shown yet
        case STATE_PLAYER_ONFIRSTWAYTOFINISHLINE: {
            mPlayerCanMove = true;
            mPlayerCanShoot = true;
            break;
        }

        case STATE_PLAYER_RACING: {
            mPlayerCanMove = true;
            mPlayerCanShoot = true;
            break;
        }

        case STATE_PLAYER_EMPTYFUEL: {
            mPlayerCanMove = false;
            mPlayerCanShoot = false;
            break;
        }

        case STATE_PLAYER_BROKEN: {
            mPlayerCanMove = false;
            mPlayerCanShoot = false;
            break;
        }

        case STATE_PLAYER_GRABEDBYRECOVERYVEHICLE: {
           mPlayerCanMove = false;
           mPlayerCanShoot = false;
           break;
        }
   }

    //in the finished state the player should be able to
    //move, but not shoot; the human player craft is taken
    //over in this state by the computer player control
    if (mHasFinishedRace) {
        mPlayerCanMove = true;
        mPlayerCanShoot = false;
    }

    //Update a connected HUD as well
    UpdateHUDState();
}

void VVehicle::UpdateHUDState() {
    if (mHUD == nullptr)
        return;

    irr::u32 state = this->GetCurrentState();

    //there is one exception, if we are in demo mode
    //do not draw the normal HUD, only before start
    if (this->mRace->mDemoMode) {
        if ((state != STATE_PLAYER_BEFORESTART) && (state != STATE_PLAYER_ONFIRSTWAYTOFINISHLINE)) {
            mHUD->SetHUDState(DEF_HUD_STATE_NOTDRAWN);
            return;
        }
    }

    //make sure the HUD state if correct for us
    switch (state) {
        case STATE_PLAYER_BEFORESTART:
        case STATE_PLAYER_ONFIRSTWAYTOFINISHLINE:
        {
            mHUD->SetHUDState(DEF_HUD_STATE_STARTSIGNAL);
            break;
        }
    case STATE_PLAYER_EMPTYFUEL:
    case STATE_PLAYER_RACING: {
            //19.04.2025: If the player has already finished the race
            //then do not draw HUD anymore, otherwise draw it again
            if (!mHasFinishedRace) {
                mHUD->SetHUDState(DEF_HUD_STATE_RACE);
            } else {
                mHUD->SetHUDState(DEF_HUD_STATE_BROKENPLAYER);
            }
            break;
        }

    case STATE_PLAYER_GRABEDBYRECOVERYVEHICLE:
    case STATE_PLAYER_BROKEN:  {
        //if there is a connected HUD we need to disable
        //its drawing, because if the player is destroyed there
        //is an outside view at the craft, and for an outside view
        //there is no HUD visible
        mHUD->SetHUDState(DEF_HUD_STATE_BROKENPLAYER);
        break;
    }
  }
}

void VVehicle::StartPlayingWarningSound() {
   //already warning playing?
   if (mWarningSoundSource == nullptr) {
       //no, start playing new warning
       //we need to keep a pntr to the looping sound source to be able to stop it
       //later again!
       mWarningSoundSource = this->mRace->mSoundEngine->PlaySound(SRES_GAME_WARNING, true);
   }
}

void VVehicle::StopPlayingWarningSound() {
   //warning really playing?
   if (mWarningSoundSource != nullptr) {
       //yes, stop it
       this->mRace->mSoundEngine->StopLoopingSound(mWarningSoundSource);
       mWarningSoundSource = nullptr;
   }
}

void VVehicle::SetCurrClosestWayPointLink(std::pair <WayPointLinkInfoStruct*, irr::core::vector3df> newClosestWayPointLink) {
    if (newClosestWayPointLink.first != nullptr) {
        this->currClosestWayPointLink = newClosestWayPointLink;
        this->projPlayerPositionClosestWayPointLink = newClosestWayPointLink.second;
    }
}

VVehicle::~VVehicle() {
    //Remove my Scenenode from
    //Scenemanager
    if (this->mCraftNode != nullptr) {
        mCraftNode->remove();
        mCraftNode = nullptr;
    }

    //Remove my player Mesh
    if (this->mCraftMesh != nullptr) {
       mRace->mGame->mSmgr->getMeshCache()->removeMesh(this->mCraftMesh);
       this->mCraftMesh = nullptr;
    }

    //free my Dust cloud emitter particles system
    delete mDustBelowCraft;
    mDustBelowCraft = nullptr;

    delete dirtTexIdsVec;
    dirtTexIdsVec = nullptr;
}

