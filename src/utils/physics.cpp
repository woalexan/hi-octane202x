/*
 I implemented the first part of the source code in this file based on the following article from Glenn Fiedler
 https://www.gafferongames.com/post/physics_in_3d/
 Honestly I can not confirm that I got this implementation 100% correct, but it seems to work for me. It was for sure
 fun to experiment with it :)
 
 When I got stuck with the first collision detection attempts I found the project/game
 HCraft (which is also based on Irrlicht), and was able to take PhysicsCollisionArea
 and some collision detection/resolution code from it, which allowed me to make it work (big thank you!)
 Original header used in this game: "Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text."
 
 The third big obstacle was ray intersection with the environment, even though Irrlicht seemed to provide a function like this. As it soon turned
 out this function does work, but its implementation is not fully finished/optimized, as the ray intersection test is simply done with all triangles of the
 environment (Irrlicht TriangleSelector) which will not work for the overall loaded HiOctane level. I was able to solve this problem by using
 a function which was taken from the following project, and later was modified to fit my project (and Irrlicht)
 https://github.com/francisengelmann/fast_voxel_traversal
 This project uses a MIT License, Copyright (c) 2019 Francis Engelmann  , Please see physics-fast-voxel-traversal-LICENSE.txt for original license
 
 Copyright (C) 2024 Wolf Alexander              (for the source code based on the Glenn Fiedler article, and the extensions)
 Copyright (c) 2019 Francis Engelmann           (original author of the voxel_traversal function, was later modified by me)
 Copyright (c) 2006-2015 by Michael Zeilfelder  (original author of the collision detection/resolution code taken, code was later modified by me)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "physics.h"

ObjPhysicsDerivative Physics::evaluate( PhysicsObject* pObj, const ObjPhysicsState & initial,
        irr::f32 t, float dt, const ObjPhysicsDerivative & d ) {

    //advance the physics state from t to t+dt using one set of derivatives
    ObjPhysicsState state;

    state.mass = initial.mass;
    state.inverseMass = initial.inverseMass;

    state.inertia = initial.inertia;
    state.inverseInertia = initial.inverseInertia;

    state.momentum = initial.momentum + d.force*dt;
    state.angularMomentum = initial.angularMomentum + d.torque*dt;

    state.position = initial.position + d.velocity*dt;
    state.orientation = initial.orientation + d.spin * dt;

    //momentum has changed, recalculate secondary values!
    state.recalculate();

    //once there recalculate the derivatives at this new state
    ObjPhysicsDerivative output;
    output.velocity = state.velocity;
    output.spin = state.spin;

    //forceVec and torque drive the whole simulation behavior
    output.force = forceVec( pObj, state, t+dt );
    output.torque = torque( pObj, state, t+dt);
    return output;
}

irr::core::vector3df Physics::forceVec( PhysicsObject* pObj, const ObjPhysicsState & state, irr::f32 t ) {
    // const float k = 15.0f;
    // const float b = 0.1f;
    // return -k * state.x - b * state.v;

    //return (irr::core::vector3df(0.0f, 0.0f, 0.0f));

    irr::core::vector3df sumForce(0.0f, 0.0f, 0.0f);

    //calc sum force that acts on rigid body
    std::vector<ObjectPhysicsForce>::iterator it;

    for (it = pObj->currForceVectorWorldCoord.begin(); it != pObj->currForceVectorWorldCoord.end(); ++it) {
       //make sure force should also be applied in a translateral sense, if not ignore force here
       if ((*it).ApplyForceType != PHYSIC_APPLYFORCE_ONLYROT) {
            sumForce += ((*it).ForceEndPoint - (*it).ForceStartPoint);
       }
    }

    //add effect of air friction
    if (state.speed > 0.0f) {
        irr::core::vector3df airFrictionForce;
        airFrictionForce = - state.speed * state.speed * pObj->currAirFrictionCoeff * (state.velocity / state.speed);
        sumForce += airFrictionForce;

        //also add other frictions, for example friction when we collide with wall etc.
        irr::core::vector3df frictionForce(pObj->physicState.velocity);
        frictionForce.normalize();
        frictionForce *= -pObj->currFrictionSum;

        sumForce += frictionForce;
    }

    return sumForce;
}

irr::core::vector3df Physics::torque(PhysicsObject* pObj, const ObjPhysicsState & state, irr::f32 t) {
    //return irr::core::vector3df(1.0f,0.0f,0.0f) - state.angularVelocity * 0.1f;
    //return irr::core::vector3df(1.0f,0.0f,0.0f) - state.angularVelocity * 0.1f;

    //calculate sum torque vector
    irr::core::vector3df sumTorqueVec(0.0f, 0.0f, 0.0f);
    irr::core::vector3df fTorque;
    irr::core::vector3df lastArm;
    irr::core::vector3df currForce;
    std::vector<ObjectPhysicsForce>::iterator it;

    for (it = pObj->currForceVectorWorldCoord.begin(); it != pObj->currForceVectorWorldCoord.end(); ++it) {
        //make sure force should also be applied in a rotational sense, if not ignore force here
        if ((*it).ApplyForceType != PHYSIC_APPLYFORCE_ONLYTRANS) {
            //Ftorque = F x (p - x)
            //Where F is the force being applied at point p in world coordinates, and x is the center of mass of the object.

            lastArm = ((*it).ForceStartPoint - state.position);
            //store lastArm for later debugging
            (*it).DbgLastArmVecStart = state.position;
            (*it).DbgLastArmVecEnd = state.position - lastArm;

            currForce = ((*it).ForceEndPoint - (*it).ForceStartPoint);
            fTorque = lastArm.crossProduct(currForce);

            (*it).DbgFTorqueStart = state.position;
            (*it).DbgFTorqueEnd = state.position + fTorque;

            sumTorqueVec += fTorque;
        }
    }

   // if (pObj->currForceVectorWorldCoord.size() > 0) {
        //damping to prevent to crazy physiscs


       //the next line was sumTorqueVec -= state.angularVelocity * 50.1f;
        //for the human controller I liked most
       //sumTorqueVec -= state.angularVelocity * 800.0f; //2000.0f

        //some value that allows to slow down rotational movement
        //to prevent crazy physics
        sumTorqueVec -= state.angularVelocity * pObj->mRotationalFrictionVal;

   // }

    DbgsumTorqueVec = sumTorqueVec;

    return sumTorqueVec;
}

//Integration using Runge Kutta order 4 (or simply RK4)
void Physics::integrate( PhysicsObject* pObj, ObjPhysicsState & state, irr::f32 t, float dt ) {

    ObjPhysicsDerivative a,b,c,d;

    a = evaluate( pObj, state, t, 0.0f, ObjPhysicsDerivative() );
    b = evaluate( pObj, state, t, dt*0.5f, a );
    c = evaluate( pObj, state, t, dt*0.5f, b );
    d = evaluate( pObj, state, t, dt, c );

    irr::core::vector3df dPositiondt = 1.0f / 6.0f *
        ( a.velocity + 2.0f * ( b.velocity + c.velocity ) + d.velocity );

    irr::core::vector3df dMomentumdt = 1.0f / 6.0f *
        ( a.force + 2.0f * ( b.force + c.force ) + d.force );

    irr::core::vector3df dAngularMomentumdt = 1.0f / 6.0f *
        ( a.torque + 2.0f * ( b.torque + c.torque ) + d.torque );

    //equation is in the form
    /*irr::core::quaternion dOrientationdt = 1.0f / 6.0f *
        ( a.spin + 2.0f * ( b.spin + c.spin ) + d.spin );*/

    irr::core::quaternion hlp = b.spin + c.spin;
    hlp *= 2.0f;

    irr::core::quaternion dOrientationdt = a.spin  + hlp + d.spin;
    dOrientationdt *= (1.0f / 6.0f);

    //final update of state variables
    state.position = state.position + dPositiondt * dt;
    state.momentum = state.momentum + dMomentumdt * dt;
    state.angularMomentum = state.angularMomentum + dAngularMomentumdt * dt;
    state.orientation = state.orientation + dOrientationdt * dt;

    //momentum has changed, recalculate secondary values!
    state.recalculate();
}

void PhysicsObject::AddFriction(irr::f32 addFrictionValue) {
    if (addFrictionValue > 0.0f) {
        this->currFrictionSum += addFrictionValue;
    }
}

void PhysicsObject::SetAirFriction(irr::f32 newAirFrictionValue) {
    currAirFrictionCoeff = newAirFrictionValue;
}

void PhysicsObject::AddWorldCoordForce(irr::core::vector3df startPointWorldCoord, irr::core::vector3df endPointWorldCoord,
                                       irr::u8 applyForceClassification,
                                       irr::u8 dbgForceClassification) {
    ObjectPhysicsForce newStruct;

    newStruct.ForceStartPoint = startPointWorldCoord;
    newStruct.ForceEndPoint = endPointWorldCoord;
    newStruct.DbgForceType = dbgForceClassification;
    newStruct.ApplyForceType = applyForceClassification;

    this->currForceVectorWorldCoord.push_back(newStruct);
}

void PhysicsObject::AddLocalCoordForce(irr::core::vector3df startPointLocalCoord, irr::core::vector3df endPointLocalCoord,
                                       irr::u8 applyForceClassification,
                                       irr::u8 dbgForceClassification) {
    ObjectPhysicsForce newStruct;
    irr::core::vector3df startTransf;
    irr::core::vector3df endTransf;

    //transform specified force vector from local ccordinates to world coordinates
    //this->physicState.localToWorldTransMatrix.transformVect(startTransf, startPointLocalCoord);
    //this->physicState.localToWorldTransMatrix.transformVect(endTransf, endPointLocalCoord);
    irr::core::vector3df pos_in_world_spaceStart(startPointLocalCoord);
    this->sceneNode->getAbsoluteTransformation().transformVect(pos_in_world_spaceStart);

    irr::core::vector3df pos_in_world_spaceEnd(endPointLocalCoord);
    this->sceneNode->getAbsoluteTransformation().transformVect(pos_in_world_spaceEnd);

    newStruct.ForceStartPoint = pos_in_world_spaceStart;
    newStruct.ForceEndPoint = pos_in_world_spaceEnd;
    newStruct.DbgForceType = dbgForceClassification;
    newStruct.ApplyForceType = applyForceClassification;

    this->currForceVectorWorldCoord.push_back(newStruct);
}

irr::core::vector3df PhysicsObject::GetVelocityLocalCoordPoint(irr::core::vector3df pointLocalCoord) {
    irr::core::vector3df result;

    //vpoint = vlinear + vangular cross (p - x)
    //Where p is the point on the rigid body and x is the center of mass of the object.
    irr::core::vector3df pos_in_world_spaceStart(pointLocalCoord);
    this->sceneNode->updateAbsolutePosition();
    this->sceneNode->getAbsoluteTransformation().transformVect(pos_in_world_spaceStart);

    result = this->physicState.velocity + this->physicState.angularVelocity.crossProduct(pos_in_world_spaceStart - this->physicState.position);

    return result;
}

irr::core::vector3df PhysicsObject::ConvertToWorldCoord(irr::core::vector3df pointLocalCoord) {
    irr::core::vector3df pos_in_world(pointLocalCoord);
    this->sceneNode->getAbsoluteTransformation().transformVect(pos_in_world);

    return pos_in_world;
}

irr::core::vector3df PhysicsObject::ConvertToLocalCoord(irr::core::vector3df pointWorldCoord) {
    irr::core::vector3df worldCoord(pointWorldCoord);
    this->sceneNode->updateAbsolutePosition();
    irr::core::matrix4 matr = this->sceneNode->getAbsoluteTransformation();
    matr.makeInverse();

    matr.transformVect(worldCoord);

    return worldCoord;
}

void PhysicsObject::interpolatePhysicState(irr::f32 alphaVal) {
    //now update all physic objects (using interpolation)
    alpha.set(alphaVal, alphaVal, alphaVal);
    invalpha.set(1.0f - alphaVal, 1.0f - alphaVal, 1.0f - alphaVal);

    interpolatedState = physicState;

    interpolatedState.position = physicState.position * alpha +
            previousPhysicState.position * invalpha;

    interpolatedState.velocity = physicState.velocity * alpha +
        previousPhysicState.velocity * invalpha;

    interpolatedState.momentum = physicState.momentum * alpha +
        previousPhysicState.momentum * invalpha;

    interpolatedState.orientation.lerp(previousPhysicState.orientation,
                                       physicState.orientation, alphaVal);

    interpolatedState.angularVelocity = physicState.angularVelocity * alpha +
        previousPhysicState.angularVelocity * invalpha;

    interpolatedState.angularMomentum = physicState.angularMomentum * alpha +
        previousPhysicState.angularMomentum * invalpha;

    interpolatedState.spin = physicState.spin * alpha +
        previousPhysicState.spin * invalpha;
}

void PhysicsObject::DebugDrawCurrentWorldCoordForces(DrawDebug* drawDebugObj, irr::video::SMaterial* Color, irr::u8 dbgForceClassification) {
     std::vector<ObjectPhysicsForce>::iterator it;

     //draw all currently active force vectors with arrows
     for (it = debugForceVectorWorldCoord.begin(); it != debugForceVectorWorldCoord.end(); ++it) {
         if ((dbgForceClassification == PHYSIC_DBG_FORCETYPE_GENERICALL) || (dbgForceClassification == (*it).DbgForceType)) {
            drawDebugObj->Draw3DLine((*it).DbgLastArmVecStart, (*it).DbgLastArmVecEnd, drawDebugObj->pink);
            drawDebugObj->Draw3DLine((*it).DbgFTorqueStart, (*it).DbgFTorqueEnd, drawDebugObj->brown);
            drawDebugObj->Draw3DArrow((*it).ForceStartPoint, (*it).ForceEndPoint, Color);
         }
     }
}

bool Physics::OrientedBBoxCollision(PhysicsObject* obj1, PhysicsObject* obj2, irr::core::aabbox3df box1, irr::core::aabbox3df box2,
                                    irr::core::vector3df *collNormal, irr::f32 *depth) {

    obj1->sceneNode->updateAbsolutePosition();
    obj2->sceneNode->updateAbsolutePosition();

    VECTOR Pa;
    irr::core::vector3df center1 = obj1->sceneNode->getAbsolutePosition();

    Pa.x = center1.X;
    Pa.y = center1.Y;
    Pa.z = center1.Z;

    irr::core::aabbox3df boxLocal1 = obj1->sceneNode->getBoundingBox();
    irr::core::aabbox3df boxLocal2 = obj2->sceneNode->getBoundingBox();

    irr::core::vector3df ext1 = boxLocal1.getExtent();
    VECTOR a;
    a.x = ext1.X / 2.0f;
    a.y = ext1.Y / 2.0f;
    a.z = ext1.Z / 2.0f;

    VECTOR Pb;
    //irr::core::vector3df center2 = box2.getCenter();
    irr::core::vector3df center2 = obj2->sceneNode->getAbsolutePosition();
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

    irr::core::matrix4 mat = obj1->sceneNode->getAbsoluteTransformation();

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


    mat = obj2->sceneNode->getAbsoluteTransformation();
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

    DbgCollStartVec = center1 + ext1/2.0f;
    DbgCollEndVec = center2 + ext2 / 2.0f;
    
    //return aabox1.overlaps(aabox2);

    //VECTOR* A, //orthonormal basis
    //VECTOR* B //orthonormal basis

    VECTOR resnormal;
    float resdepth;

    bool result = OBBOverlap(a, Pa, &A[0], b, Pb, &B[0], &resnormal, &resdepth);

    if (result) {
        collNormal->X = resnormal.x;
        collNormal->Y = resnormal.y;
        collNormal->Z = resnormal.z;
        *depth = resdepth;
    }

    return result;
}

//Returns true if collision, false otherwise
bool Physics::CheckForCollision(PhysicsObject *obj1, PhysicsObject *obj2,
           irr::core::vector3df *collNormal, irr::f32 *depth) {
    //step 1 of collision detection: Sphere-To-Sphere collision detection
    obj1->sceneNode->updateAbsolutePosition();
    obj2->sceneNode->updateAbsolutePosition();
    irr::core::vector3df pos1Obj = obj1->sceneNode->getAbsolutePosition();
    irr::core::vector3df pos2Obj = obj2->sceneNode->getAbsolutePosition();

    irr::f32 distSquared = (pos1Obj - pos2Obj).getLengthSQ();

    //execute sphere-to-sphere collision detection, if negative return with false
   // if (distSquared > (obj1->objBoundingBoxExtendSquared + obj2->objBoundingBoxExtendSquared))
   //     return false;

    //Step 2: execute AABounding box collision using Irrlicht
    //first update axis aligned bounding boxes of both objects
    obj1->objBoundingBox = obj1->sceneNode->getTransformedBoundingBox();
    obj2->objBoundingBox = obj2->sceneNode->getTransformedBoundingBox();

    DbgRunCollisionDetectionStage2 = 1.0f;
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

    if (OrientedBBoxCollision(obj1, obj2, obj1->objBoundingBox, obj2->objBoundingBox, collNormal, depth)) {
        return true;
    }

   return false;
}

//Returns true if collision, false otherwise
bool Physics::CheckForWallCollision(PhysicsObject *obj1,
           irr::core::vector3df collNormal, irr::f32 depth) {
    //step 1 of collision detection: Sphere-To-Sphere collision detection
    obj1->sceneNode->updateAbsolutePosition();
    irr::core::vector3df pos1Obj = obj1->sceneNode->getAbsolutePosition();

    irr::f32 distSquared;

    std::vector<LineStruct*>::iterator Linedraw_iterator;
    bool checkCol;

    for(Linedraw_iterator = this->ENTWallsegmentsLine_List->begin(); Linedraw_iterator != this->ENTWallsegmentsLine_List->end(); ++Linedraw_iterator) {
        checkCol = false;
         distSquared = (pos1Obj - (*Linedraw_iterator)->A).getLengthSQ();
         if (abs(distSquared) < 1000.0f) {
             checkCol = true;
         } else {
                distSquared = (pos1Obj - (*Linedraw_iterator)->B).getLengthSQ();
                if (abs(distSquared) < 1000.0f) {
                    checkCol = true;
                }
         }

         //collision line element close enough, check collision between craft and wall (line)
         if (checkCol) {
              DbgRunCollisionDetectionStage2++;
            (*Linedraw_iterator)->color = mDebugObj->green;
            //create plane and calculate distance from craft to plane
            irr::core::plane3df plane((*Linedraw_iterator)->A, (*Linedraw_iterator)->B, (*Linedraw_iterator)->B + irr::core::vector3df(0.0f, 1.0f, 0.0f));
            distSquared = plane.getDistanceTo(obj1->physicState.position);
            depth = (distSquared * distSquared) - obj1->objBoundingBoxExtendSquared;
            if (depth < 0) {
                //there is a collision with the wall
                collNormal = plane.Normal.normalize();
                depth = sqrt(-depth);

                 (*Linedraw_iterator)->color = mDebugObj->red;

                return true;
            }
         } else {
           //  (*Linedraw_iterator)->color = mDebugObj->white;
         }
    }

    return false;
}

bool Physics::DidObjCollideWithObjLastIteration(PhysicsObject* obj1, PhysicsObject* obj2) {
   std::vector<PhyCollisionPair>::iterator it;
   for (it = this->currObjToObjCollisionPairs.begin(); it != this->currObjToObjCollisionPairs.end(); ++it) {
       if ((((*it).obj1 == obj1) && ((*it).obj2 == obj2)) ||
           (((*it).obj2 == obj1) && ((*it).obj1 == obj2))) {
           return true;
       }
   }

   return false;
}

void Physics::AddObjToObjCollisionPair(PhysicsObject* obj1, PhysicsObject* obj2) {
    //make sure to not add collision pairs twice
  //  if (!DidObjCollideWithObjLastIteration(obj1, obj2)) {
       PhyCollisionPair* newPair = new PhyCollisionPair;
       newPair->obj1 = obj1;
       newPair->obj2 = obj2;
       currObjToObjCollisionPairs.push_back(*newPair);
 //   }
}

void Physics::RemoveObjToObjCollisionPair(PhysicsObject* obj1, PhysicsObject* obj2) {
    //is this collision pair really present?
    std::vector<PhyCollisionPair>::iterator it;
    for (it = this->currObjToObjCollisionPairs.begin(); it != this->currObjToObjCollisionPairs.end();) {
        if ((((*it).obj1 == obj1) && ((*it).obj2 == obj2)) ||
            (((*it).obj2 == obj1) && ((*it).obj1 == obj2))) {
            PhyCollisionPair* removePair = &(*it);

            it = currObjToObjCollisionPairs.erase(it);

            //TODO: I need to comment this, otherwise game crashes
            //but fix this!
            //delete removePair;
        } else
            ++it;
    }
}

void Physics::HandleObjToObjCollision(PhysicsObject *currObj, irr::f32 deltaTime) {
   std::vector<PhysicsObject*>::iterator it;
   std::vector<PhysicsObject*>::iterator it2;

   DbgRunCollisionDetectionStage2 = 0.0f;
   DbgCollisionDetected = 0.0f;

   irr::core::vector3df collNormal(0.0f, 0.0f, 0.0f);
   irr::f32 collDepth = 0.0f;

   std::vector<PhysicsObject*>::iterator startit;
   bool found = false;

   //which object is the current one?
   for (it = this->PhysicObjectVec.begin(); it != this->PhysicObjectVec.end(); ++it) {
       if ((*it) == currObj) {
           //current object found
           startit = it;
           found = true;
           break;
       }
   }

   if (!found) {
       return;
   }

   //we do not want to check this object
   //with itself, therefore move to next object
   startit++;

   for (it2 = startit; it2 != this->PhysicObjectVec.end(); ++it2) {

       //check only for collision if both objects are "active"
       if ((*it)->mActive && (*it2)->mActive) {

         if (CheckForCollision((*it), (*it2), &collNormal, &collDepth)) {
             //yes, we found an object to object collision
            //DbgCollisionDetected = collDepth;

            /* if ((*it)->CollidedOtherObjectLastTime == false) {
                 (*it)->AddFriction(20.0f);
                 (*it)->CollidedOtherObjectLastTime = true;
             }

             if ((*it2)->CollidedOtherObjectLastTime == false) {
                 (*it2)->AddFriction(20.0f);
                 (*it2)->CollidedOtherObjectLastTime = true;
             }*/

             //penetration resolution (immediately seperate objects)
            //(*it)->physicState.position -= collNormal * collDepth;
            //(*it2)->physicState.position += collNormal * collDepth;

             if (!DidObjCollideWithObjLastIteration((*it), (*it2))) {

                      bool somethingNan = false;
                      somethingNan |= std::isnan(collNormal.X);
                      somethingNan |= std::isnan(collNormal.Y);
                      somethingNan |= std::isnan(collNormal.Z);
                      somethingNan |= std::isnan(collDepth);

                      if (!somethingNan) {
                              AddObjToObjCollisionPair((*it), (*it2));

                              irr::f32 elasticity = 0.3f;

                              irr::f32 Ua = (*it)->physicState.velocity.dotProduct(collNormal);
                              irr::f32 Ub = -(*it2)->physicState.velocity.dotProduct(collNormal);

                              irr::f32 Jn = ((*it)->physicState.mass*(*it2)->physicState.mass)/((*it)->physicState.mass+(*it2)->physicState.mass);
                              Jn = Jn * (1+elasticity)*(Ub-Ua);

                              irr::f32 velChange1 = Jn / ((*it)->physicState.mass);
                              irr::f32 velChange2 = -Jn / ((*it2)->physicState.mass);

                              irr::f32 force1 = (velChange1 * (*it)->physicState.mass) / deltaTime;
                              irr::f32 force2 = (velChange2 * (*it2)->physicState.mass) / deltaTime;

                              //better limit forces, because otherwise horrible
                              //things may happen with the world
                              if (fabs(force1) > 2000.0f) {
                                  force1 = sgn(force1) * 2000.0f;
                              }

                              if (fabs(force2) > 2000.0f) {
                                  force2 = sgn(force2) * 2000.0f;
                              }

                              (*it)->AddFriction(5000.0f);
                              (*it2)->AddFriction(5000.0f);

                            (*it)->AddWorldCoordForce((*it)->physicState.position, collNormal * force1, PHYSIC_APPLYFORCE_ONLYTRANS,
                                                    PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);

                              (*it2)->AddWorldCoordForce((*it2)->physicState.position, collNormal * force2, PHYSIC_APPLYFORCE_ONLYTRANS,
                                                        PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);


                              (*it)->CollidedOtherObjectLastTime = true;
                              (*it2)->CollidedOtherObjectLastTime = true;
                      }
             }

            //TODO: calculate new velocities
            //see this page: https://www.euclideanspace.com/physics/dynamics/collision/index.htm


            //try to resolve collision
            /*(*it)->AddWorldCoordForce((*it)->physicState.position, ((*it)->physicState.position - collNormal) * 1.0f, PHYSIC_APPLYFORCE_ONLYTRANS,
                                      PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);
            (*it2)->AddWorldCoordForce((*it2)->physicState.position, ((*it2)->physicState.position + collNormal) * 1.0f, PHYSIC_APPLYFORCE_ONLYTRANS,
                                       PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);*/
         } else {
             RemoveObjToObjCollisionPair((*it), (*it2));
         }
       }
     }
}

void Physics::AddCollisionMesh(irr::scene::ITriangleSelector* selector_)
{
    if ( selector_)
    {
        selector_->grab();
        mCollisionSelectors.push_back(selector_);
    }
}

bool Physics::RemoveCollisionMesh(irr::scene::ITriangleSelector* selector_)
{
    for ( CollisionSelectorVector::iterator it  = mCollisionSelectors.begin(); it != mCollisionSelectors.end(); ++it )
    {
        if ( *it == selector_ )
        {
            (*it)->drop();
            mCollisionSelectors.erase(it);
            return true;
        }
    }
    return false;
}

void Physics::AddRayTargetMesh(irr::scene::ITriangleSelector* selector_)
{
    if ( selector_)
    {
        selector_->grab();
        mRayTargetSelectors.push_back(selector_);
    }
}

bool Physics::RemoveRayTargetMesh(irr::scene::ITriangleSelector* selector_)
{
    for ( RayTargetSelectorVector::iterator it  = mRayTargetSelectors.begin(); it != mRayTargetSelectors.end(); ++it )
    {
        if ( *it == selector_ )
        {
            (*it)->drop();
            mRayTargetSelectors.erase(it);
            return true;
        }
    }
    return false;
}

void Physics::FillCollisionArea(PhysicsCollisionArea& area) const
{
    area.mCollisionTrianglesSize = 0;
    for ( size_t i=0; i<mCollisionSelectors.size(); ++i )
    {
        irr::scene::ITriangleSelector* selector = mCollisionSelectors[i];
        int trianglesReceived = 0;

        selector->getTriangles( &area.mCollisionTriangles[area.mCollisionTrianglesSize],
                PHYSICS_MAX_COLLISION_TRIANGLES-area.mCollisionTrianglesSize, trianglesReceived, area.mBox, /*transform*/ 0 );
        area.mCollisionTrianglesSize += trianglesReceived;
    }
}

//Returns all triangles hit by a defined ray from ray start to ray end point
std::vector<RayHitTriangleInfoStruct*> Physics::ReturnTrianglesHitByRay(std::vector<irr::scene::ITriangleSelector*> triangleSelectorVector,
                               irr::core::vector3df rayStart, irr::core::vector3df rayEnd, bool ReturnOnlyClosestTriangles) {

    std::vector<RayHitTriangleInfoStruct*> mRaySelectedTriangles;

    //built a ray cast 3d line from physics object position to direction we want to go
    std::vector<irr::core::vector3di> voxels;

    //find out which voxels are visited by the ray
    //from start to end
    voxels = voxel_traversal(rayStart, rayEnd);

    irr::core::line3df rayLine(rayStart.X, rayStart.Y, rayStart.Z, rayEnd.X, rayEnd.Y, rayEnd.Z);
    irr::core::vector3df rayDirVector = rayLine.getVector().normalize();

    std::vector<irr::core::vector3di>::iterator it;
    irr::core::vector3df boxVertex1;
    irr::core::vector3df boxVertex2;

    irr::f32 segSize = DEF_SEGMENTSIZE;

    mRaySelectedTriangles.clear();

      for (it = voxels.begin(); it != voxels.end(); ++it) {

          mRayTargetTrianglesSize = 0;

        for ( size_t i=0; i<triangleSelectorVector.size(); ++i )
        {
            irr::scene::ITriangleSelector* selector = triangleSelectorVector[i];
            int trianglesReceived = 0;

            //make region for selecting triangles slightly bigger to make sure we find all triangles, also at the border
            boxVertex1.X = (irr::f32)((*it).X) * segSize - segSize * 0.5f;
            boxVertex1.Y = (irr::f32)((*it).Y) * segSize - segSize * 0.5f;
            boxVertex1.Z = (irr::f32)((*it).Z) * segSize - segSize * 0.5f;

            boxVertex2.X = (irr::f32)((*it).X + 1) * segSize + segSize * 0.5f;
            boxVertex2.Y = (irr::f32)((*it).Y + 1) * segSize + segSize * 0.5f;
            boxVertex2.Z = (irr::f32)((*it).Z + 1) * segSize + segSize * 0.5f;

            irr::core::aabbox3df box(boxVertex1.X, boxVertex1.Y, boxVertex1.Z, boxVertex2.X, boxVertex2.Y, boxVertex2.Z);

            //get all triangles that are hit by this 3D line, does not seem to be implemented properly, just seems to return all triangles => forget
            selector->getTriangles( &mRayTargetTriangles[mRayTargetTrianglesSize],
                    PHYSICS_MAX_RAYTARGET_TRIANGLES - mRayTargetTrianglesSize, trianglesReceived, box, /*transform*/ 0 );

            mRayTargetTrianglesSize += trianglesReceived;
        }

        // find nearest collision
        for ( int i=0; i < mRayTargetTrianglesSize; i++ )
        {
            irr::core::vector3df pointOnPlane;
            if ( mRayTargetTriangles[i].getIntersectionWithLimitedLine( rayLine, pointOnPlane) )
            {
                if ( mRayTargetTriangles[i].isPointInsideFast(pointOnPlane) ) {
                    RayHitTriangleInfoStruct* newHit = new RayHitTriangleInfoStruct();
                    newHit->hitTriangle = mRayTargetTriangles[i];
                    newHit->hitPointOnTriangle = pointOnPlane;
                    newHit->rayDirVec = rayDirVector;
                    newHit->distFromRayStartSquared = (pointOnPlane - rayStart).getLengthSQ();

                    mRaySelectedTriangles.push_back(newHit);
                }
            }
        }

        //if we only should return the closest hit triangles exit here (if we found already at least one hit triangle)
        //otherwise continue finding more triangles hit by the ray further away
        if (ReturnOnlyClosestTriangles && (mRaySelectedTriangles.size() > 0)) {
            return mRaySelectedTriangles;
        }

      }

      return mRaySelectedTriangles;
}

bool Physics::HandleSphereWallCollision(const PhysicsCollisionArea& collArea, PhysicsObject *obj, /*irr::core::vector3df &center_,*/ float radius_, irr::core::triangle3df &nearestTriangle_,
                                    irr::core::vector3df &repulsionNormal_)
{
    irr::core::vector3df *center = &obj->physicState.position;

    bool hasCollision = false;
    bool findMoreCollisions = true;
    int triesCountdown = 20;	// getting out of one collision can mean getting into another collision
    while ( findMoreCollisions && triesCountdown >= 0 )
    {
        --triesCountdown;
        findMoreCollisions = false;

        double nearestDistSq = radius_*radius_;
        irr::core::vector3df nearestPoint;
        irr::core::vector3df repulsion;
        irr::core::triangle3df triangleNearest;

        // find nearest collision
        for ( int i=0; i < collArea.mCollisionTrianglesSize; i++ )
        {
            //if ( mDebuggingEnabled )
            //{
            //   driver->draw3DTriangle(mCollisionTriangles[i], video::SColor(0, 255, 0,127));
            //}

            irr::core::vector3df pointOnPlane;
            if ( collArea.mCollisionTriangles[i].getIntersectionOfPlaneWithLine (*center, collArea.mCollisionTriangles[i].getPlane().Normal, pointOnPlane) )
            {
                irr::core::vector3df pointOnTriangle;
                if ( collArea.mCollisionTriangles[i].isPointInsideFast(pointOnPlane) )
                    pointOnTriangle = pointOnPlane;
                else
                    pointOnTriangle = collArea.mCollisionTriangles[i].closestPointOnTriangle(pointOnPlane);
                double distSq = center->getDistanceFromSQ(pointOnTriangle);
                if ( distSq < nearestDistSq )	// nearest collision inside the sphere radius
                {
                    findMoreCollisions = true;
                    nearestDistSq = distSq;
                    triangleNearest = collArea.mCollisionTriangles[i];
                    repulsion =  (*center - pointOnTriangle);
                    nearestPoint = pointOnTriangle;
                }
            }
        }
        if ( findMoreCollisions )
        {
            nearestTriangle_ = triangleNearest;
            repulsionNormal_ = repulsion;
            repulsionNormal_.normalize();
            if ( !hasCollision )
            {
                hasCollision = true;
                mWallNormal = repulsionNormal_;
            }
            else if ( fabs(repulsionNormal_.Y) < fabs(mWallNormal.Y) )
            {
                mWallNormal = repulsionNormal_;
            }
//          core::plane3df collisionPlane(nearestPoint, lineToTriangle);
            repulsion.setLength(0.001f+radius_-(float)(sqrt(nearestDistSq)));

            if (collisionResolutionActive) {
             *center += repulsion;
             irr::core::vector3df collResolutionForce = repulsion;
             collResolutionForce.normalize();

             //float impulse = obj->physicState.mass * obj->physicState.speed * 0.05f;

             //add a force that pushes the object away from the object we collided with

             //line used until 10.11.2024, felt a little bit too much force
             //obj->AddWorldCoordForce(*center, *center + collResolutionForce * 2.0f, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);

             //new line since 10.11.2024
             obj->AddWorldCoordForce(*center, *center + collResolutionForce * 1.3f, PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION);

             //Also slow the craft down considerably by adding friction
             //because otherwise the player gets no penalty by colliding with walls
             obj->AddFriction(2.0f);

            }

//          printf("radius: %.2f nearestDistSq:%.2f sqrt(nearestDistSq):%.2f\n", radius_, nearestDistSq, sqrt(nearestDistSq));

            // debug
           /* if ( mDebuggingEnabled )
            {
                // nearest collision triangle
                triangleNearest.pointA.Y += 10.f;
                triangleNearest.pointB.Y += 10.f;
                triangleNearest.pointC.Y += 10.f;
                driver->draw3DTriangle(triangleNearest, video::SColor(255, 0, 0,127));

                // push back vector
                driver->draw3DLine (nearestPoint, center_, video::SColor(0, 0, 255, 255));

                // nearest point (as cross)
                driver->draw3DLine (core::vector3df(nearestPoint.X-50, nearestPoint.Y, nearestPoint.Z), core::vector3df(nearestPoint.X+50, nearestPoint.Y, nearestPoint.Z), video::SColor(0, 255, 255, 255));
                driver->draw3DLine (core::vector3df(nearestPoint.X, nearestPoint.Y, nearestPoint.Z-50), core::vector3df(nearestPoint.X, nearestPoint.Y, nearestPoint.Z+50), video::SColor(0, 255, 255, 255));
            }*/
        }
    }

    return hasCollision;
}

void Physics::DrawSelectedCollisionMeshTriangles(const PhysicsCollisionArea& collArea) {

    DbgmCollisionTrianglesSize = collArea.mCollisionTrianglesSize;

    for ( int i=0; i < collArea.mCollisionTrianglesSize; i++ )
    {
      this->mDebugObj->Draw3DTriangle(&collArea.mCollisionTriangles[i], irr::video::SColor(0, 255, 0,127));
    }
}

void Physics::DrawSelectedRayTargetMeshTriangles(std::vector<RayHitTriangleInfoStruct*> hitInfoTriangles) {

    std::vector<RayHitTriangleInfoStruct*>::iterator it;
    for (it = hitInfoTriangles.begin(); it != hitInfoTriangles.end(); ++it) {
          this->mDebugObj->Draw3DTriangle(&(*it)->hitTriangle, irr::video::SColor(0, 255, 0,127));
    }
}

//clean up triangle hit information struct
void Physics::EmptyTriangleHitInfoVector(std::vector<RayHitTriangleInfoStruct*> &hitInfoTriangles) {
   std::vector<RayHitTriangleInfoStruct*>::iterator itHit;

   RayHitTriangleInfoStruct* pntr;

   //delete all existing hit info in vector
   itHit = hitInfoTriangles.begin();

   while (itHit != hitInfoTriangles.end()) {
          pntr = (*itHit);
          itHit = hitInfoTriangles.erase(itHit);

          //delete hit information object itself
          delete pntr;
   }
}

void Physics::HandleWallCollision(PhysicsObject& obj)
{
    PhysicsCollisionArea& collArea = obj.GetCollisionArea();

    const float BOX_SPACING = 2*obj.mRadius; // we get the collision triangles just once - therefore make it bigger than the radius

    //collArea.set(obj.mCurrentStepCollCenter, BOX_SPACING);

    collArea.set(obj.physicState.position, BOX_SPACING);
    FillCollisionArea( collArea );

    //obj.mHasTouchedWorldGeometry = HandleSphereCollision(collArea, obj.mCurrentStepCollCenter, obj.mRadius, obj.mNearestTriangle, obj.mRepulsionNormal);

    obj.mHasTouchedWorldGeometry = HandleSphereWallCollision(collArea, &obj, /*obj.physicState.position*/ BOX_SPACING, obj.mNearestTriangle, obj.mRepulsionNormal);

    if ( obj.mHasTouchedWorldGeometry )
    {
        obj.CollidedOtherObjectLastTime = true;
//        DbgCollisionDetected = 1.0f;
    } /*
        obj.mHasTouchedWorldGeometryLastUpdate = true;
        obj.mHasTouchedWall = fabs(mWallNormal.Y) < mSettings.mWallBoundary ? true : false;

        if( obj.mHasTouchedWall )
        {
            // check if we're above a floor
            core::line3d<f32> ray;
            ray.start = obj.mCurrentStepCollCenter;
            ray.end = obj.mCurrentStepCollCenter + core::vector3df(0, -1000, 0); // below
            if ( !HasCollision( ray ) )
            {
                obj.mHasTouchedWall = false;
            }
        }

        if ( obj.mHasTouchedWall )
        {
            obj.mHasTouchedWallLastUpdate = true;
        }
    }*/
}

void Physics::SetLevelCollisionWallLineData(std::vector<LineStruct*> *newList) {
    this->ENTWallsegmentsLine_List = newList;
}

void Physics::AdvancePhysicsTime(const irr::f32 frameDeltaTime) {
    std::vector<PhysicsObject*>::iterator it;
    std::vector<ObjectPhysicsForce>::iterator itForce;

       DbgCollisionDetected = 0.0f;

    /*
      std::list<LineStruct*>::iterator Linedraw_iterator;
    for(Linedraw_iterator = ENTWallsegmentsLine_List->begin(); Linedraw_iterator != ENTWallsegmentsLine_List->end(); ++Linedraw_iterator) {
        (*Linedraw_iterator)->color = mDebugObj->white;
    }*/

     for (it = this->PhysicObjectVec.begin(); it != this->PhysicObjectVec.end(); ++it) {

          if ((*it)->mActive) {
            /*   PhysicsObject& obj = itObjects->second;
            obj.mHasTouchedWorldGeometryLastUpdate = false;
            obj.mHasTouchedWallLastUpdate = false;
            obj.mHasTouchedObjectLastUpdate = false;
            obj.mCushioning = 0.f;*/

            (*it)->CollidedOtherObjectLastTime = false;

            (*it)->sceneNode->updateAbsolutePosition();
            (*it)->mModelCollCenter = (*it)->sceneNode->getTransformedBoundingBox().getCenter();
            (*it)->mRadius = (*it)->sceneNode->getTransformedBoundingBox().getExtent().getLength() * 0.15f;
          }

           // We handle objects moved outside the physics, so that it seems like they had always been in that other position in the past.
           // Rewriting history, to make it fit to current needs, a very political algorithm ;-)
          /* core::vector3df movedOutsidePhysics( obj.mModelCollCenter - obj.mInterpolatedCollCenter );
           obj.mCurrentStepCollCenter += movedOutsidePhysics;
           obj.mLastStepCollCenter += movedOutsidePhysics;*/
       }

    //increase physics time accumulator by last frameDeltaTime
    //to advance physics by frameDeltaTime
    physicsAccumulator += frameDeltaTime;

    irr::core::vector3df rot2;

    //advance physics time
    while ( physicsAccumulator >= dt ) {

             //process all existing physic objects in the world one after each other
             for (it = this->PhysicObjectVec.begin(); it != this->PhysicObjectVec.end(); ++it) {

                 if ((*it)->mActive) {
                    //execute collision detection between physics objects
                    //and wall/blocks (via sphere and triangle selector)
                    HandleWallCollision(*(*it));

                    //execute collision detection and resolution
                    //between physics objects themselves (via bounding boxes)
                    HandleObjToObjCollision((*it), dt);

                    //Wolf Alexander 24.10.2024: For the heightmap collision to work we
                    //need to immediately update the sceneNodes here inside the loop
                    //which is kind of dirty, but regarding the CPU load I did not
                    //see any negative impact of doing this, so I decided to just do it :)

                    //update sceneNode position and orientation
                    (*it)->sceneNode->setPosition((*it)->physicState.position);

                    (*it)->physicState.orientation.toEuler(rot2);
                    (*it)->sceneNode->setRotation(rot2 * irr::core::RADTODEG);

                    (*it)->sceneNode->updateAbsolutePosition();

                    //execute craft terrain height map collisions
                    this->mParentRace->HandleCraftHeightMapCollisions(dt, (*it));

                    (*it)->previousPhysicState = (*it)->physicState;
                    this->integrate((*it), (*it)->physicState, t, dt);
                 }
             }

             t += dt;
             physicsAccumulator -= dt;
    }

    irr::f32 alpha = (physicsAccumulator / dt);
    irr::core::vector3df rot;

    //interpolate states for rendering etc...
    //interpolate all existing physic objects (calculate render state)
    for (it = this->PhysicObjectVec.begin(); it != this->PhysicObjectVec.end(); ++it) {

        if ((*it)->mActive) {
            (*it)->interpolatePhysicState(alpha);

            //update sceneNode position and orientation
            (*it)->sceneNode->setPosition((*it)->interpolatedState.position);

            (*it)->interpolatedState.orientation.toEuler(rot);
            (*it)->sceneNode->setRotation(rot * irr::core::RADTODEG);

            //copy debug force information to debug vector
            (*it)->debugForceVectorWorldCoord.clear();

            for (itForce = (*it)->currForceVectorWorldCoord.begin(); itForce != (*it)->currForceVectorWorldCoord.end(); ++itForce) {
                 (*it)->debugForceVectorWorldCoord.push_back(*itForce);
            }

            //clear all current force vectors
            (*it)->currForceVectorWorldCoord.clear();

            //remove current frictions again
            (*it)->currFrictionSum = 0.0f;
        }
    }
}

Physics::Physics(Race* parentRace, DrawDebug* drawDbg) {
    PhysicObjectVec.clear();

    mParentRace = parentRace;
    mDebugObj = drawDbg;

    //setup gravity
    mGravityVec.set(0.0f, -9.81f, 0.0f);
}

Physics::~Physics() {
    //Remove all my linked physics objects
    RemoveAllObjects();

    //now we are ready to be deleted outself
}

void Physics::AddObject(irr::scene::ISceneNode* sceneNode) {
   PhysicsObject* newObj = new PhysicsObject();
   newObj->sceneNode = sceneNode;

  // newObj->cHull = CreateConvexHullBoundingBox(newObj);

   //get initial bounding box for the SceneNode and extend for step 1 of collision detection
   newObj->objBoundingBox = newObj->sceneNode->getBoundingBox();
   newObj->objBoundingBoxExtendSquared = newObj->objBoundingBox.getExtent().getLengthSQ();

   this->PhysicObjectVec.push_back(newObj);
}

void Physics::RemoveObject(irr::scene::ISceneNode* sceneNode) {
    std::vector<PhysicsObject*>::iterator it;
    PhysicsObject* pntrObj;

    for (it = this->PhysicObjectVec.begin(); it != this->PhysicObjectVec.end(); ) {
        if ((*it)->sceneNode == sceneNode) {
            pntrObj = (*it);
            //we found the correct scenenode, erase it
            it = PhysicObjectVec.erase(it);

            //delete the physics object itself as well
            delete pntrObj;

            break;
        } else ++it;
    }
}

void Physics::RemoveAllObjects() {
    std::vector<PhysicsObject*>::iterator it;
    PhysicsObject* pntrObj;

    for (it = this->PhysicObjectVec.begin(); it != this->PhysicObjectVec.end(); ) {
            pntrObj = (*it);
            it = PhysicObjectVec.erase(it);

            //delete the physics object itself as well
            delete pntrObj;
    }
}

//will return NULL if no physicsObject is found that belongs to the
//specified sceneNode
PhysicsObject* Physics::GetObjectPntr(irr::scene::ISceneNode* sceneNode) {
  std::vector<PhysicsObject*>::iterator it;

  for (it = this->PhysicObjectVec.begin(); it != this->PhysicObjectVec.end(); ++it) {
      if ((*it)->sceneNode == sceneNode) {
          //we found the correct scenenode, return
          return (*it);
      }
  }

  //nothing found, return NULL
  return NULL;
}

//the following function was taken from, and modified to fit my code (and Irrlicht)
//https://github.com/francisengelmann/fast_voxel_traversal
/*MIT License

Copyright (c) 2019 Francis Engelmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

/**
 * @brief returns all the voxels that are traversed by a ray going from start to end
 * @param start : continous world position where the ray starts
 * @param end   : continous world position where the ray end
 * @return vector of voxel ids hit by the ray in temporal order
 *
 * J. Amanatides, A. Woo. A Fast Voxel Traversal Algorithm for Ray Tracing. Eurographics '87
 */

std::vector<irr::core::vector3di> Physics::voxel_traversal(irr::core::vector3df ray_start, irr::core::vector3df ray_end) {
  std::vector<irr::core::vector3di> visited_voxels;

  irr::f32 _bin_size = DEF_SEGMENTSIZE;

  // This id of the first/current voxel hit by the ray.
  // Using floor (round down) is actually very important,
  // the implicit int-casting will round up for negative numbers.
  irr::core::vector3di current_voxel((irr::s32)(std::floor(ray_start.X/_bin_size)),
                                (irr::s32)(std::floor(ray_start.Y/_bin_size)),
                                (irr::s32)(std::floor(ray_start.Z/_bin_size)));

  // The id of the last voxel hit by the ray.
  // TODO: what happens if the end point is on a border?
  irr::core::vector3di last_voxel((irr::s32)(std::floor(ray_end.X/_bin_size)),
                             (irr::s32)(std::floor(ray_end.Y/_bin_size)),
                             (irr::s32)(std::floor(ray_end.Z/_bin_size)));

  // Compute normalized ray direction.
  irr::core::vector3df ray = ray_end-ray_start;
  //ray.normalize();

  // In which direction the voxel ids are incremented.
  irr::f32 stepX = (ray.X >= 0) ? 1.0f:-1.0f; // correct
  irr::f32 stepY = (ray.Y >= 0) ? 1.0f:-1.0f; // correct
  irr::f32 stepZ = (ray.Z >= 0) ? 1.0f:-1.0f; // correct

  // Distance along the ray to the next voxel border from the current position (tMaxX, tMaxY, tMaxZ).
  irr::f32 next_voxel_boundary_x = (current_voxel.X+stepX)*_bin_size; // correct
  irr::f32 next_voxel_boundary_y = (current_voxel.Y+stepY)*_bin_size; // correct
  irr::f32 next_voxel_boundary_z = (current_voxel.Z+stepZ)*_bin_size; // correct

  // tMaxX, tMaxY, tMaxZ -- distance until next intersection with voxel-border
  // the value of t at which the ray crosses the first vertical voxel boundary
  //Wolf 22.03.2025: change end of the next 3 lines from DBL_MAX to now FLT_MAX, DBL_MAX does not
  //make sense. If this source code breaks this could be the reason
  irr::f32 tMaxX = (ray.X!=0) ? (next_voxel_boundary_x - ray_start.X)/ray.X : FLT_MAX; //
  irr::f32 tMaxY = (ray.Y!=0) ? (next_voxel_boundary_y - ray_start.Y)/ray.Y : FLT_MAX; //
  irr::f32 tMaxZ = (ray.Z!=0) ? (next_voxel_boundary_z - ray_start.Z)/ray.Z : FLT_MAX; //

  // tDeltaX, tDeltaY, tDeltaZ --
  // how far along the ray we must move for the horizontal component to equal the width of a voxel
  // the direction in which we traverse the grid
  // can only be FLT_MAX if we never go in that direction
  //Wolf 22.03.2025: change end of the next 3 lines from DBL_MAX to now FLT_MAX, DBL_MAX does not
  //make sense. If this source code breaks this could be the reason
  irr::f32 tDeltaX = (ray.X!=0) ? _bin_size/ray.X*stepX : FLT_MAX;
  irr::f32 tDeltaY = (ray.Y!=0) ? _bin_size/ray.Y*stepY : FLT_MAX;
  irr::f32 tDeltaZ = (ray.Z!=0) ? _bin_size/ray.Z*stepZ : FLT_MAX;

  irr::core::vector3di diff(0,0,0);
  bool neg_ray=false;
  if (current_voxel.X!=last_voxel.X && ray.X<0) { diff.X--; neg_ray=true; }
  if (current_voxel.Y!=last_voxel.Y && ray.Y<0) { diff.Y--; neg_ray=true; }
  if (current_voxel.Z!=last_voxel.Z && ray.Z<0) { diff.Z--; neg_ray=true; }
  visited_voxels.push_back(current_voxel);
  if (neg_ray) {
    current_voxel+=diff;
    visited_voxels.push_back(current_voxel);
  }

  while(last_voxel != current_voxel) {
    if (tMaxX < tMaxY) {
      if (tMaxX < tMaxZ) {
        current_voxel.X += (irr::s32)(stepX);
        tMaxX += tDeltaX;
      } else {
        current_voxel.Z += (irr::s32)(stepZ);
        tMaxZ += tDeltaZ;
      }
    } else {
      if (tMaxY < tMaxZ) {
        current_voxel.Y += (irr::s32)(stepY);
        tMaxY += tDeltaY;
      } else {
        current_voxel.Z += (irr::s32)(stepZ);
        tMaxZ += tDeltaZ;
      }
    }
    visited_voxels.push_back(current_voxel);
  }
  return visited_voxels;
}
