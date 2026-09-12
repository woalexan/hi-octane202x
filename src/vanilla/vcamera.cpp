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

#include "vcamera.h"
#include "../models/camera.h"
#include "vvehicle.h"
#include "vthing.h"
#include "debug/structs/cam.h"
#include "debug/structs/basicstructs.h"
#include "debug/datatools.h"
#include "../race.h"
#include "../game.h"

VCamera::VCamera(Race* parentRace) {
    mParentRace = parentRace;

    camera_setup();
}

void VCamera::SetIrrlichtCamera(irr::scene::ICameraSceneNode* whichCamera, ParseCamera* setToState) {
    if (whichCamera == nullptr) {
        return;
    }

    whichCamera->setUpVector(irr::core::vector3df(0.0f, 1.0f, 0.0f));
    whichCamera->updateAbsolutePosition();

    irr::core::vector3df vanCamPos;
    vanCamPos.X = setToState->Position->XPos->mFloatValue;
    vanCamPos.Y = setToState->Position->YPos->mFloatValue;
    vanCamPos.Z = setToState->Position->ZPos->mFloatValue;

    irr::core::vector3df irrCamPos = mParentRace->mVCalc->VanillaToIrrlichtCoord(vanCamPos);
    whichCamera->setPosition(irrCamPos);

    //Important: we need to first update the absolute position
    //before updating the camera Rotation!
    whichCamera->updateAbsolutePosition();

    irr::core::matrix4 m;
    m.makeIdentity();

    irr::core::matrix4 n;
    n.setRotationDegrees(irr::core::vector3df(0.0f, setToState->AngleXY->mFloatValue + 180.0f, 0.0f));
    m *= n;

    n.setRotationDegrees(irr::core::vector3df(setToState->AngleZY->mFloatValue, 0.0f, 0.0f));
    m *= n;

    n.setRotationDegrees(irr::core::vector3df(0.0f, 0.0f, setToState->AngleXZ->mFloatValue));
    m *= n;

    irr::core::vector3df finalRotation = m.getRotationDegrees();
    whichCamera->setRotation(finalRotation);

    whichCamera->updateAbsolutePosition();

    irr::core::vector3df rotationDirVec = finalRotation.rotationToDirection(whichCamera->getUpVector());
    rotationDirVec.normalize();

    irr::core::vector3df camZAxisDirection = (whichCamera->getTarget() - whichCamera->getAbsolutePosition());
    camZAxisDirection.normalize();

    irr::core::vector3df camYAxisDirection = whichCamera->getUpVector();
    camYAxisDirection.normalize();

    irr::core::vector3df camXAxisDirection = camZAxisDirection.crossProduct(camYAxisDirection);
    camXAxisDirection.normalize();

    irr::core::vector3df projVec = mParentRace->mVCalc->VectorProjection(rotationDirVec, camXAxisDirection);

    whichCamera->setUpVector(irr::core::vector3df(0.0f, 1.0f, 0.0f) - projVec);
}

void VCamera::SetIrrlichtCamera(irr::scene::ICameraSceneNode* whichCamera, VCameraStruct* setToState) {
    if (whichCamera == nullptr) {
        return;
    }

    whichCamera->setUpVector(irr::core::vector3df(0.0f, 1.0f, 0.0f));
    whichCamera->updateAbsolutePosition();

    irr::core::vector3df vanCamPos;
    vanCamPos.X = setToState->Position.X;
    vanCamPos.Y = setToState->Position.Y;
    vanCamPos.Z = setToState->Position.Z;

    irr::core::vector3df irrCamPos = mParentRace->mVCalc->VanillaToIrrlichtCoord(vanCamPos);
    whichCamera->setPosition(irrCamPos);

    //Important: we need to first update the absolute position
    //before updating the camera Rotation!
    whichCamera->updateAbsolutePosition();

    irr::core::matrix4 m;
    m.makeIdentity();

    irr::core::matrix4 n;
    n.setRotationDegrees(irr::core::vector3df(0.0f, setToState->AngleXY + 180.0f, 0.0f));
    m *= n;

    n.setRotationDegrees(irr::core::vector3df(setToState->AngleZY, 0.0f, 0.0f));
    m *= n;

    n.setRotationDegrees(irr::core::vector3df(0.0f, 0.0f, setToState->AngleXZ));
    m *= n;

    irr::core::vector3df finalRotation = m.getRotationDegrees();
    whichCamera->setRotation(finalRotation);

    whichCamera->updateAbsolutePosition();

    irr::core::vector3df rotationDirVec = finalRotation.rotationToDirection(whichCamera->getUpVector());
    rotationDirVec.normalize();

    irr::core::vector3df camZAxisDirection = (whichCamera->getTarget() - whichCamera->getAbsolutePosition());
    camZAxisDirection.normalize();

    irr::core::vector3df camYAxisDirection = whichCamera->getUpVector();
    camYAxisDirection.normalize();

    irr::core::vector3df camXAxisDirection = camZAxisDirection.crossProduct(camYAxisDirection);
    camXAxisDirection.normalize();

    irr::core::vector3df projVec = mParentRace->mVCalc->VectorProjection(rotationDirVec, camXAxisDirection);

    whichCamera->setUpVector(irr::core::vector3df(0.0f, 1.0f, 0.0f) - projVec);
}

void VCamera::camera_setup() {
    mCameraWindow.Camera.Action = 7;  //internal cockpit view
    mCameraWindow.Camera.Distance = 0.0f;
    mCameraWindow.Camera.Zoom = 1.171875f;
}

void VCamera::camera_process(/*int32_t player_number*/ VVehicle* whichVehicle, int32_t force_camera_action, int32_t cameraIndex) {

    irr::f32 v24;
    irr::f32 v25;
    irr::f32 v26;
    irr::f32 v21;
    irr::f32 v22;
    irr::f32 v36;
    irr::f32 v48;
    irr::f32 v54;
    irr::f32 v57;
    irr::f32 v58;
    irr::f32 v59;
    irr::f32 v60;
    irr::f32 v92;
    irr::f32 v94;
    irr::f32 difference;
    irr::f32 floorZ;
    int16_t v92Int;
    irr::core::vector3df* v61;
    irr::f32 v64;
    std::vector<Camera*>::iterator it;
    irr::f32 xyz;
    irr::f32 v71;

    /* if (player_number != -1) {
         word_8011EF2C[70 * cameraIndex] = player_number;
     }*/

    //my solution
    if (force_camera_action == -1) {
         //not implemented right now
    } else {
        //word_8011EF32[70 * cameraIndex] = force_camera_action;
        mCameraWindow.Camera.Action = force_camera_action;
    }

    switch (mCameraWindow.Camera.Action) {
        case 1: {  //Looks at the vehicle straight from behind the vehicle
            mCameraWindow.Camera.Position = whichVehicle->View.Position;
            mCameraWindow.ChaseCamera.AngleXY = whichVehicle->View.AngleXY;
            mCameraWindow.ChaseCamera.AngleZY = whichVehicle->View.AngleZY;
            mCameraWindow.ChaseCamera.AngleXZ = whichVehicle->View.AngleXZ;
            v24 = mCameraWindow.Camera.AngleXY;
            v22 = mCameraWindow.ChaseCamera.AngleXY;
            mCameraWindow.ChaseCamera.Distance = 1.0f;
            v21 = mCameraWindow.ChaseCamera.AngleZY;
            mCameraWindow.ChaseCamera.AngleXY = v22;
            v25 = mCameraWindow.ChaseCamera.AngleXY;
            v26 = v21 + 14.996337890625f;
            goto camera_process_LABEL_35;
        }

        case 2: { //Looks at the vehicle from in front of the vehicle
            mCameraWindow.Camera.Position = whichVehicle->View.Position;
            mCameraWindow.ChaseCamera.AngleXY = whichVehicle->View.AngleXY;
            mCameraWindow.ChaseCamera.AngleZY = whichVehicle->View.AngleZY;
            mCameraWindow.ChaseCamera.AngleXZ = whichVehicle->View.AngleXZ;
            mCameraWindow.ChaseCamera.Distance = 1.0f;
            v24 = mCameraWindow.Camera.AngleXY;
            mCameraWindow.ChaseCamera.AngleXY += 180.0f;
            v25 = mCameraWindow.ChaseCamera.AngleXY;
            v26 = mCameraWindow.ChaseCamera.AngleZY + 14.996337890625f;
            goto camera_process_LABEL_35;
        }

        case 3: { //Looks at the vehicle from the left side (side view)
            mCameraWindow.Camera.Position = whichVehicle->View.Position;
            mCameraWindow.ChaseCamera.AngleXY = whichVehicle->View.AngleXY;
            mCameraWindow.ChaseCamera.AngleZY = whichVehicle->View.AngleZY;
            mCameraWindow.ChaseCamera.AngleXZ = whichVehicle->View.AngleXZ;
            v24 = mCameraWindow.Camera.AngleXY;
            v36 = mCameraWindow.ChaseCamera.AngleXY;
            mCameraWindow.ChaseCamera.Distance = 1.0f;
            mCameraWindow.ChaseCamera.AngleXY = v36 + 90.0f;
            v25 = mCameraWindow.ChaseCamera.AngleXY;
            v26 = mCameraWindow.ChaseCamera.AngleZY + 14.996337890625f;
            goto camera_process_LABEL_35;
        }

        case 4: { //Looks at the vehicle from the right side (side view)
            mCameraWindow.Camera.Position = whichVehicle->View.Position;
            mCameraWindow.ChaseCamera.AngleXY = whichVehicle->View.AngleXY;
            mCameraWindow.ChaseCamera.AngleZY = whichVehicle->View.AngleZY;
            mCameraWindow.ChaseCamera.AngleXZ = whichVehicle->View.AngleXZ;
            v24 = mCameraWindow.Camera.AngleXY;
            v36 = mCameraWindow.ChaseCamera.AngleXY;
            mCameraWindow.ChaseCamera.Distance = 1.0f;
            mCameraWindow.ChaseCamera.AngleXY = v36 - 90.0f;
            v25 = mCameraWindow.ChaseCamera.AngleXY;
            v26 = mCameraWindow.ChaseCamera.AngleZY + 14.996337890625f;
            goto camera_process_LABEL_35;
        }

        case 5: { //Looks at the vehicle from the top (birds view)
            mCameraWindow.Camera.Position = whichVehicle->View.Position;
            mCameraWindow.ChaseCamera.AngleXY = whichVehicle->View.AngleXY;
            mCameraWindow.ChaseCamera.AngleZY = whichVehicle->View.AngleZY;
            mCameraWindow.ChaseCamera.AngleXZ = whichVehicle->View.AngleXZ;
            v24 = mCameraWindow.Camera.AngleXY;
            v25 = mCameraWindow.ChaseCamera.AngleXY;
            v48 = mCameraWindow.ChaseCamera.AngleZY;
            mCameraWindow.ChaseCamera.Distance = 1.0f;
            v26 = v48 + 90.0f;
camera_process_LABEL_35:
            mCameraWindow.ChaseCamera.AngleZY = v26;
            difference = mParentRace->mVCalc->angle_get_difference(v24, v25);
            mCameraWindow.Camera.AngleXY += (difference / 8.0f);
            v54 = mParentRace->mVCalc->angle_get_difference(mCameraWindow.Camera.AngleZY,
                                                            mCameraWindow.ChaseCamera.AngleZY);
            mCameraWindow.Camera.AngleZY += (v54 / 8.0f);
            v57 = mParentRace->mVCalc->angle_get_difference(mCameraWindow.Camera.AngleXZ,
                                                            mCameraWindow.ChaseCamera.AngleXZ);
            mCameraWindow.Camera.AngleXZ += (v57 / 8.0f);
            v58 = mCameraWindow.Camera.Distance;
            v59 = mCameraWindow.ChaseCamera.Distance - v58;
            v60 = (v59 / 8.0f);
            if (v59 < 0.0f) {
                v60 = ((v59 + 0.02734375f) / 8.0f);
            }
            mCameraWindow.Camera.Distance = (v58 + v60);
            break;
        }

        case 6: {  //This seems to be the default external view from a camera
            v61 = nullptr;
            v64 = 32.0f;

            //Go through all existing camera in the level and search the one that is closest to
            //the vehicle, and which can see the vehicle
            for (it = mParentRace->mCameraVec.begin(); it != mParentRace->mCameraVec.end(); ++it) {
                xyz = mParentRace->mVCalc->distance_get_xyz(whichVehicle->ThingData->Position,
                                                            (*it)->ThingData->Position);
                if (xyz < v64) {
                    if (move_point_see_point(whichVehicle->ThingData->Position, (*it)->ThingData->Position)) {
                        v61 = &((*it)->ThingData->Position);
                        v64 = xyz;
                    }
                }
            }

            //did we find a suitable external camera?
            if (v61 == nullptr) {
                //no we did not
                //fallback to the default internal cockpit
                //view of the player
                goto camera_process_LABEL_46;
            }

            //we found a suitable camera, use it instead
            mCameraWindow.Camera.AngleXY = mParentRace->mVCalc->angle_get_xy(*v61, whichVehicle->View.Position);
            mCameraWindow.Camera.AngleZY = mParentRace->mVCalc->angle_get_zy(*v61, whichVehicle->View.Position);
            mCameraWindow.Camera.AngleXZ = 0.0f;
            mCameraWindow.Camera.Position = *v61; //whichVehicle->View.Position;

            v71 = v64 * mCameraWindow.Camera.Zoom;
            //mCameraWindow.Camera.Detail.Sky = 0;
            mCameraWindow.Camera.Distance = (v71 / 1000.0f);
            break;
        }

        case 7: {   //This is the default internal cockpit view
camera_process_LABEL_46:
            mCameraWindow.Camera.Position = whichVehicle->View.Position;
            mCameraWindow.Camera.Distance = 0.0f;

            mCameraWindow.Camera.AngleXY = whichVehicle->View.AngleXY;
            mCameraWindow.Camera.AngleZY = whichVehicle->View.AngleZY;
            mCameraWindow.Camera.AngleXZ = whichVehicle->View.AngleXZ;
            //something weird with ThingVehicle.VehicleDamage.ShimmerCount missing, implement later?
            break;
        }

        case 8: { //This is the first external view behind the craft, with the smallest distance to vehicle
            camera_process_position(whichVehicle, mCameraWindow, 0.46875f, 0.0823974609375f);
            break;
        }

        case 9: { //This is the 2nd external view behind the craft, with middle distance to vehicle
            camera_process_position(whichVehicle, mCameraWindow, 0.9375f, 0.054931640625f);
            break;
        }

        case 0xA: { //This is the 3rd external view behind the craft, with the highest distance to vehicle
            camera_process_position(whichVehicle, mCameraWindow, 1.40625f, 0.0274658203125f);
            break;
        }

        case 0xB: {  //This is a camera that follows the vehicle closely, but the angleXY that we look at it
                     //is fixed to a constant direction. The direction can be altered by setting the
                     //unknown bytes below. This code does not seem to get used
            //is the code below really used?
            //if (byte_801F396F) {
            //  mCameraWindow.ChaseCamera.AngleXY -= 2.999267578125f;
            //}
            //if (byte_801F3971) {
            //  mCameraWindow.ChaseCamera.AngleXY += 2.999267578125f;
            //}
            //if (byte_801F3950) {
            //  mCameraWindow.ChaseCamera.AngleXZ -= 2.999267578125f;
            //}
            //if (byte_801F3951) {
            //  mCameraWindow.ChaseCamera.AngleXZ += 2.999267578125f;
            //}
            //if (byte_801F396C) {
            //  mCameraWindow.ChaseCamera.AngleZY -= 2.999267578125f;
            //}
            //if (byte_801F3974) {
            //  mCameraWindow.ChaseCamera.AngleZY += 2.999267578125f;
            //}
            mCameraWindow.Camera.Position = whichVehicle->View.Position;
            mCameraWindow.Camera.Distance =
                    (mParentRace->mVCalc->FixedPointToFloat8D8(mCameraWindow.Camera.Zoom) * 8.064f + 0.5f);
            mCameraWindow.Camera.AngleXY = mCameraWindow.ChaseCamera.AngleXY;
            mCameraWindow.Camera.AngleZY = mCameraWindow.ChaseCamera.AngleZY;
            mCameraWindow.Camera.AngleXZ = mCameraWindow.ChaseCamera.AngleXZ;
            break;
        }

        default: {
            break;
        }
    }

    if (!mCameraWindow.Camera.Action) {
         SetIrrlichtCamera(mParentRace->vanTestCam, &mCameraWindow.Camera);
    }

    if (((mCameraWindow.Camera.Action != 7) || (fabs(mCameraWindow.Camera.Distance) > 0.00390625f))
            && (fabs(mCameraWindow.Camera.Distance) > 0.00390625f)) {
      mParentRace->mVCalc->move_xyz(this->mCameraWindow.Camera.Position,
                                    mCameraWindow.Camera.AngleXY,
                                    mCameraWindow.Camera.AngleZY,
                                    -mCameraWindow.Camera.Distance);

      v94 = mCameraWindow.Camera.Position.Z;

      floorZ = (mParentRace->mVCalc->map_floor(this->mCameraWindow.Camera.Position) + 0.5f);

      if (v94 < floorZ) {
          mCameraWindow.Camera.Position.Z = floorZ;
      }

       SetIrrlichtCamera(mParentRace->vanTestCam, &mCameraWindow.Camera);
    } else {
        v92 = whichVehicle->ThingData->Position.Z;
        v92Int = mParentRace->mVCalc->FloatToFixedPoint8D8(v92);
        v92Int |= 2;
        irr::f32 saveVal =  whichVehicle->ThingData->Position.Z;
        whichVehicle->ThingData->Position.Z = mParentRace->mVCalc->FixedPointToFloat8D8(v92Int);
        SetIrrlichtCamera(mParentRace->vanTestCam, &mCameraWindow.Camera);
        whichVehicle->ThingData->Position.Z = saveVal;
    }
}

void VCamera::camera_process_position(VVehicle* whichVehicle, VCameraWindowStruct& vanillaOutputCameraWindow,
                                      irr::f32 distance, irr::f32 degrees) {
    irr::core::vector3df position;

    vanillaOutputCameraWindow.Camera.Position = whichVehicle->View.Position;
    vanillaOutputCameraWindow.ChaseCamera.AngleXY = whichVehicle->View.AngleXY;
    vanillaOutputCameraWindow.ChaseCamera.AngleXZ = whichVehicle->View.AngleXZ;
    vanillaOutputCameraWindow.ChaseCamera.AngleZY = whichVehicle->View.AngleZY;
    vanillaOutputCameraWindow.ChaseCamera.Distance = distance;

    irr::f32 v14 = whichVehicle->View.AngleZY;
    irr::f32 v15 = whichVehicle->View.AngleXY;
    irr::f32 v16 = -distance;
    irr::f32 v17 = -distance;
    vanillaOutputCameraWindow.Camera.Position.Z += 0.5f;
    irr::f32 v18 = v14 + degrees;
    irr::f32 v20 = fabs(v17);
    vanillaOutputCameraWindow.ChaseCamera.AngleZY = v18;
    irr::f32 v21 = v18;
    irr::f32 v22;
    irr::f32 v23;
    irr::f32 v24;
    irr::f32 v27;
    irr::f32 v28;
    int8_t v26;
    int8_t v30;
    irr::f32 v32;
    bool v34;
    irr::f32 v35;
    irr::f32 difference;
    irr::f32 v41;
    irr::f32 v44;
    irr::f32 v46;
    irr::f32 v47;
    int16_t v33;

    if ((v20 - 0.03125f) >= 9.97265625f) {
        goto camera_process_position_LABEL16;
    }

    if (v17 < 0.0f) {
       v22 = -0.03125f;
       v23 = v16; // * 65536.0f;
       if (v17 <= -0.03125f) {
           v24 = v17;
           do {
               //TODO: the next line is just an assumption, could be
               //completely wrong!
               position = vanillaOutputCameraWindow.Camera.Position;
               mParentRace->mVCalc->move_xyz(position, v15, v21, v22);
               position.Z += 0.0625f;

               v26 = mParentRace->mVCalc->map_colide_4point(position, 0.5f, 0.5f);
               v17 = v22;
               if (v26) {
                   goto camera_process_position_LABEL16;
               }
               v22 -= 0.03125f;
               v23 = v16; // * 65536.0f;
           } while (v22 >= v24);
       }
camera_process_position_LABEL15:
       v17 = v23; // / 65536.0f;
       goto camera_process_position_LABEL16;
    }
    v27 = 0.03125f;
    if (v17 < 0.03125f) {
camera_process_position_LABEL14:
        v23 = v16; //* 65536.0f;
        goto camera_process_position_LABEL15;
    }
    v28 = v17;
    while (1) {
        //TODO: the next line is just an assumption, could be
        //completely wrong!
        position = vanillaOutputCameraWindow.Camera.Position;
        mParentRace->mVCalc->move_xyz(position, v15, v21, v27);
        v30 = mParentRace->mVCalc->map_colide_4point(position, 0.5f, 0.5f);
        v17 = v27;
        if (v30) {
            break;
        }
        v27 += 0.03125f;
        if (v28 < v27) {
            goto camera_process_position_LABEL14;
        }
    }
camera_process_position_LABEL16:
    v32 = -v17;
    if (fabs(v32 - vanillaOutputCameraWindow.ChaseCamera.Distance) < 0.00390625f) {
        vanillaOutputCameraWindow.ChaseCamera.Static = 0;
    } else {
        v33 = vanillaOutputCameraWindow.ChaseCamera.Static + 1;
        vanillaOutputCameraWindow.ChaseCamera.Static = v33;
        v34 = v33 < 9;
        v35 = v32 - 0.25f;
        if (!v34) {
           vanillaOutputCameraWindow.ChaseCamera.Distance = v35;
           if (v35 < 0.0625f) {
               vanillaOutputCameraWindow.ChaseCamera.Distance = 0.0625f;
           }
           vanillaOutputCameraWindow.ChaseCamera.Static  = 8;
           vanillaOutputCameraWindow.ChaseCamera.AngleZY -= degrees;
        }
    }

    difference =
        mParentRace->mVCalc->angle_get_difference(vanillaOutputCameraWindow.Camera.AngleXY,
                                                  vanillaOutputCameraWindow.ChaseCamera.AngleXY);

    vanillaOutputCameraWindow.Camera.AngleXY += (difference / 8.0f);

    v41 = mParentRace->mVCalc->angle_get_difference(vanillaOutputCameraWindow.Camera.AngleXZ,
                                                    vanillaOutputCameraWindow.ChaseCamera.AngleXZ);

    vanillaOutputCameraWindow.Camera.AngleXZ += (v41 / 8.0f);

    v44 = mParentRace->mVCalc->angle_get_difference(vanillaOutputCameraWindow.Camera.AngleZY,
                                                    vanillaOutputCameraWindow.ChaseCamera.AngleZY);

    vanillaOutputCameraWindow.Camera.AngleZY += (v44 / 8.0f);

    v46 = vanillaOutputCameraWindow.ChaseCamera.Distance -
            vanillaOutputCameraWindow.Camera.Distance;

    v47 = (v46 / 8.0f);
    if (v46 < 0.0f) {
        v47 = (v46 + 0.02734375f) / 8.0f;
    }

    vanillaOutputCameraWindow.Camera.Distance += v47;
}

int32_t VCamera::move_point_see_point(irr::core::vector3df position_from,
                                      irr::core::vector3df position_to) {
    irr::f32 rough_xyz_float;
    int32_t rough_xyz;
    int32_t v6;
    int32_t v8;
    irr::f32 v8Float;
    int32_t v9;
    int32_t v10;

    irr::core::vector3df position;
    irr::core::vector3df distance;
    int32_t result = 1;

    rough_xyz_float = mParentRace->mVCalc->distance_get_rough_xyz(position_from, position_to);

    //better do this in fixed point arithmetic
    rough_xyz = static_cast<int32_t>(mParentRace->mVCalc->FloatToFixedPoint8D8(rough_xyz_float));

    if (rough_xyz < 257) {
         v6 = rough_xyz + 64;
         if (v6 < 0) {
             v6 = rough_xyz + 191;
         }

         position = position_from;
         v8 = (v6 >> 7);
         mParentRace->mVCalc->distance_set_xyz(distance, position_from, position_to);
         v8Float = mParentRace->mVCalc->FixedPointToFloat8D8(static_cast<int16_t>(v8));
         v9 = 0;
         distance.X = (distance.X / v8Float);
         distance.Y = (distance.Y / v8Float);
         distance.Z = (distance.Z / v8Float);
         if (v8 > 0) {
            while (!mParentRace->mVCalc->map_colide(position)) {
               ++v9;
               position += distance;
               if (v9 >= v8) {
                   goto move_point_see_point_LABEL_18;
               }
            }
            v10 = (v9 ^ v8);
            if (!v9) {
                return (v10 == 0);
            }
            position -= distance;
         }
move_point_see_point_LABEL_18:
         v10 = (v9 ^ v8);
         return (v10 == 0);
    }

    return result;
}

VCamera::~VCamera() {
}
