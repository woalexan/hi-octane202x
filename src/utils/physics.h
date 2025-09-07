/*
 I implemented the first part of the source code in this file based on the following article from Glenn Fiedler
 https://www.gafferongames.com/post/physics_in_3d/
 Honestly I can not confirm that I got this implementation 100% correct, but it seems to work for me. It was for sure
 fun to experiment with it :)

 When I got stuck with the first collision detection attempts I found the project/game
 HCraft (which is also based on Irrlicht), and was able to take PhysicsCollisionArea
 and some collision detection/resolution code from it, which allowed me to make it work (big thank you!)
 Original header used in this game: "Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text."

 Copyright (C) 2024-2025 Wolf Alexander              (for the source code based on the Glenn Fiedler article, and the extensions)
 Copyright (c) 2006-2015 by Michael Zeilfelder  (original author of the collision detection/resolution code taken, code was later modified by me)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef PHYSICS_H
#define PHYSICS_H

#include <irrlicht.h>
#include <vector>
#include "../definitions.h"

#define PHYSICS_MAX_COLLISION_TRIANGLES 4000

//the following forces classes should improve debugging capability
//as it allows to differentiate between different kind of forces
//that are used throughout the game
#define PHYSIC_DBG_FORCETYPE_GENERICALL 0
#define PHYSIC_DBG_FORCETYPE_HEIGHTCNTRL 1
#define PHYSIC_DBG_FORCETYPE_ACCELBRAKE 2
#define PHYSIC_DBG_FORCETYPE_STEERING 3
#define PHYSIC_DBG_FORCETYPE_COLLISIONRESOLUTION 4

#define PHYSIC_APPLYFORCE_REAL 0  //applies a force both translateral and rotational to rigid body (default real world behavior)
#define PHYSIC_APPLYFORCE_ONLYTRANS 1 //applies a force only translateral to rigid body
#define PHYSIC_APPLYFORCE_ONLYROT 2 //applies a force only rotational to rigid body

/************************
 * Forward declarations *
 ************************/

class PhysicsObject;
class Race;
class DrawDebug;
struct ColorStruct;

//PhysicsCollisionArea taken from game HCraft (which is also based on Irrlicht), and modified by me
//Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.
struct PhysicsCollisionArea
{
    PhysicsCollisionArea() : mCollisionTrianglesSize(0)
    {}

    PhysicsCollisionArea(const irr::core::line3d<float>& line) : mCollisionTrianglesSize(0)
    {
        mBox.reset(line.start);
        mBox.addInternalPoint(line.end);
    }

    void set(const irr::core::line3d<float>& line)
    {
        mBox.reset(line.start);
        mBox.addInternalPoint(line.end);
        mCollisionTrianglesSize = 0;
    }

    void set(const irr::core::vector3df& center, const irr::core::aabbox3d<irr::f32> &box)
    {
        mBox = box;
        mCollisionTrianglesSize = 0;
    }

    void set(const irr::core::vector3df& center, irr::f32 boxBoundingRadius)
    {
        mBox = irr::core::aabbox3d<irr::f32>(center.X-boxBoundingRadius, center.Y-boxBoundingRadius, center.Z-boxBoundingRadius, center.X+boxBoundingRadius, center.Y+boxBoundingRadius, center.Z+boxBoundingRadius);
        mCollisionTrianglesSize = 0;
    }

    irr::core::aabbox3d<irr::f32> mBox;	// Box used to find the triangles
    int mCollisionTrianglesSize;    	// Nr of collision triangles which we have
    irr::core::triangle3df mCollisionTriangles[PHYSICS_MAX_COLLISION_TRIANGLES];  // triangles we found

    /*
    void makeTriangleNormals()
    {
        for ( int i=0; i<mCollisionTrianglesSize; ++i)
        {
            mCollisionTriangleNormals[i] = mCollisionTriangles[i].getNormal().normalize();
        }
    }
    const vector3d<irr::f32> mCollisionTriangleNormals[PHYSICS_MAX_COLLISION_TRIANGLES];
    */
};

struct ObjectPhysicsForce {
    irr::u8 DbgForceType = PHYSIC_DBG_FORCETYPE_GENERICALL;
    irr::u8 ApplyForceType = PHYSIC_APPLYFORCE_REAL;
    irr::core::vector3df ForceStartPoint; //Start point is the point on the object where the force vector does start (attach)
    irr::core::vector3df ForceEndPoint; //End point of force vector
    irr::core::vector3df DbgLastArmVecStart; //vector for lastarm (used for debugging)
    irr::core::vector3df DbgLastArmVecEnd; //vector for lastarm (used for debugging)
    irr::core::vector3df DbgFTorqueStart; //vector for torque caused by this force (used for debugging)
    irr::core::vector3df DbgFTorqueEnd; //vector for torque caused by this force (used for debugging)
};

struct PhyCollisionPair {
    PhysicsObject* obj1 = nullptr;
    PhysicsObject* obj2 = nullptr;
};

struct ObjPhysicsState {
       // primary values, for linear movement/translation
       irr::core::vector3df position;
       irr::core::vector3df momentum;

       //primary values, for orientation/rotation
       irr::core::quaternion orientation;
       irr::core::vector3df angularMomentum;

       // secondary values, for linear movement/translation
       // every time a primary value changes we need to make sure
       // that secondary values are recalculated (stay in sync with
       // primary values), otherwise this will not work
       irr::core::vector3df velocity;

       //secondary values, for orientation/rotation
       irr::core::quaternion spin;
       irr::core::vector3df angularVelocity;

       // constant values, for linear movement/translation
       float mass = 1.0f;
       float inverseMass = 1.0f;

       //constant values, for orientation/rotation
       float inertia = 1.0f;
       float inverseInertia = 1.0f;

       //speed
       float speed = 0.0f;

       //for local coordinates to world coordinates transformation
       //irr::core::matrix4 localToWorldTransMatrix;

       void SetMass(float newMass) {
           if (newMass != 0.0f) {
                mass = newMass;
                inverseMass = (1.0f / mass);
           }
       }

       void SetInertia(float newInertia) {
           if (newInertia != 0.0f) {
                inertia = newInertia;
                inverseInertia = (1.0f / inertia);
           }
       }

       //recalculates secondary values
       void recalculate()
       {
           velocity = momentum * inverseMass;

           speed = velocity.getLength();

           angularVelocity = angularMomentum * inverseInertia;

           orientation.normalize();

           irr::core::quaternion q(angularVelocity.X,
                                   angularVelocity.Y, angularVelocity.Z, 0.0f);

            q *= 0.5f;
            q *= orientation;
            spin = q;

            //update objects local coordinate to world coordinate projection matrix
            //localToWorldTransMatrix = orientation.getMatrix();
            //irr::core::matrix4 transl;
            //transl.setTranslation(position);
            //localToWorldTransMatrix *= transl;
       }
};

class PhysicsObject {
public:
    //if set to false this physicsObject is temporarily
    //not taken into account during physics calculations
    //this means its position, speed etc. is not updated
    //anymore, its SceneNode is not updated anymore, and it is
    //also ignored during collision detection
    bool mActive = true;

    irr::scene::ISceneNode* sceneNode = nullptr;

    //current axis aligned bounding box for the physics object
    //just used in step2 of collision detection, if step1 shows possible collision
    irr::core::aabbox3df objBoundingBox;

    //is the extend of the objects bounding box, is only used in step 1 of collision detection
    //(sphere to sphere collision detection) to save calculation effort
    irr::f32 objBoundingBoxExtendSquared = 0.1f;

    ObjPhysicsState physicState;
    ObjPhysicsState previousPhysicState;
    ObjPhysicsState interpolatedState;

    //air friction coefficient
    irr::f32 currAirFrictionCoeff = 0.0f;
    irr::f32 currFrictionSum = 0.0f;

    irr::f32 mRotationalFrictionVal = 50.1f;

    void SetAirFriction(irr::f32 newAirFrictionValue);
    void AddFriction(irr::f32 addFrictionValue);

    void interpolatePhysicState(irr::f32 alphaVal);

    std::vector<ObjectPhysicsForce> currForceVectorWorldCoord;
    std::vector<ObjectPhysicsForce> debugForceVectorWorldCoord;

    void DebugDrawCurrentWorldCoordForces(DrawDebug* drawDebugObj, ColorStruct* Color, irr::u8 dbgForceClassification = PHYSIC_DBG_FORCETYPE_GENERICALL);

    //current force vector that applies to this physics object
    //this forces are all in world coordinates

    void AddWorldCoordForce(irr::core::vector3df startPointWorldCoord, irr::core::vector3df endPointWorldCoord, irr::u8 applyForceClassification,
                            irr::u8 dbgForceClassification = PHYSIC_DBG_FORCETYPE_GENERICALL);
    void AddLocalCoordForce(irr::core::vector3df startPointLocalCoord, irr::core::vector3df endPointLocalCoord, irr::u8 applyForceClassification,
                            irr::u8 dbgForceClassification = PHYSIC_DBG_FORCETYPE_GENERICALL);
    irr::core::vector3df GetVelocityLocalCoordPoint(irr::core::vector3df pointLocalCoord);
    irr::core::vector3df ConvertToWorldCoord(irr::core::vector3df pointLocalCoord);
    irr::core::vector3df ConvertToLocalCoord(irr::core::vector3df pointWorldCoord);

    bool ForceValid(irr::core::vector3df forceVec);

    inline const PhysicsCollisionArea& GetCollisionArea() const     { return  mCollArea; }
    inline PhysicsCollisionArea& GetCollisionArea()                 { return  mCollArea; }

    bool mHasTouchedWorldGeometry = false; // internal value, can be set/unset several times within one update
    bool CollidedOtherObjectLastTime = false;

    PhysicsCollisionArea mCollArea;	// polygons used for last collision

    irr::core::triangle3df  mNearestTriangle;   // nearest triangle from last collision
    irr::core::vector3df    mRepulsionNormal;   // repulsion normal for sliding plane from last collision

    //is used for collision detection with blocks/edge of race track (collision mesh)
    //for this the race craft is modelled as a simple sphere with radius mRadius
    float mRadius = 0.1f;

    irr::core::vector3df mModelCollCenter;           // the value we get from the model at the start of update

private:

    irr::core::vector3df alpha;
    irr::core::vector3df invalpha;
    
    irr::core::vector3df mCurrentStepCollCenter;     // actual result from current physics step
};

//this struct holds the derivative of the primary state values above
struct ObjPhysicsDerivative {
       irr::core::vector3df velocity = {0.0f, 0.0f, 0.0f};   // dposition / dt = velocity
       irr::core::vector3df force = {0.0f, 0.0f, 0.0f};      // dmomentum / dt = force

       irr::core::quaternion spin = {0.0f, 0.0f, 0.0f, 0.0f};
       irr::core::vector3df torque = {0.0f, 0.0f, 0.0f};
};

class Physics {

private:
    ObjPhysicsDerivative evaluate( PhysicsObject* pObj, const ObjPhysicsState & initial,
        irr::f32 t, float dt, const ObjPhysicsDerivative & d );

    irr::core::vector3df forceVec(PhysicsObject* pObj, const ObjPhysicsState & state, irr::f32 t );
    irr::core::vector3df torque(PhysicsObject* pObj, const ObjPhysicsState & state, irr::f32 t);

    std::vector<PhysicsObject*> PhysicObjectVec;

    irr::f32 t = 0.0f;   //absolute time for physics calculations
    irr::f32 dt = 0.001f;  //0.001 // first stepsize I had, worked good for physics but not for collision at higher speeds  0.01; //stepsize for physics calculations
    irr::f32 physicsAccumulator = 0.0f;

    void integrate( PhysicsObject* pObj, ObjPhysicsState & state, irr::f32 t, float dt );

    //we do collision checks in 2 steps:
    //first we just handle all physics objects as spheres (distance derived from bounding boxes in irrlicht); we do sphere-to-sphere collision detection;
    //only if a sphere-to-sphere collision detection shows the possibility for a collision we execute a more precise collision detection within irrlicht
    //between two axis-aligned bounding boxes (stored in Physicsobject)
    bool CheckForCollision(PhysicsObject *obj1, PhysicsObject *obj2, irr::core::vector3df *collNormal, irr::f32 *depth);
    bool CheckForWallCollision(PhysicsObject *obj1,
               irr::core::vector3df collNormal, irr::f32 depth);

    void HandleObjToObjCollision(PhysicsObject *currObj, irr::f32 deltaTime);

    std::vector<PhyCollisionPair> currObjToObjCollisionPairs;
    void AddObjToObjCollisionPair(PhysicsObject* obj1, PhysicsObject* obj2);
    bool DidObjCollideWithObjLastIteration(PhysicsObject* obj1, PhysicsObject* obj2);
    void RemoveObjToObjCollisionPair(PhysicsObject* obj1, PhysicsObject* obj2);

    //level wall collision 3D line data loaded from level file;
    std::vector<LineStruct*> *ENTWallsegmentsLine_List = nullptr;

    Race *mParentRace = nullptr;
    DrawDebug* mDebugObj = nullptr;

    bool OrientedBBoxCollision(PhysicsObject* obj1, PhysicsObject* obj2,  irr::core::aabbox3df box1, irr::core::aabbox3df box2,
                                        irr::core::vector3df *collNormal, irr::f32 *depth);

    // Find polygons in that area. area.box must be set
    void FillCollisionArea(PhysicsCollisionArea& area) const;

    typedef std::vector<irr::scene::ITriangleSelector*> CollisionSelectorVector;
    std::vector<irr::scene::ITriangleSelector*> mCollisionSelectors;

    bool HandleSphereWallCollision(const PhysicsCollisionArea& collArea, PhysicsObject *obj, /*irr::core::vector3df &center_,*/
                                 float radius_, irr::core::triangle3df &nearestTriangle_, irr::core::vector3df &repulsionNormal_);

    void HandleWallCollision(PhysicsObject& physObj);

    irr::core::vector3df mWallNormal;

public:
    Physics(Race *parentRace, DrawDebug* drawDbg);
    ~Physics();

     void DrawSelectedCollisionMeshTriangles(const PhysicsCollisionArea& collArea);

    irr::core::vector3df DbgCollStartVec;
    irr::core::vector3df DbgCollEndVec;

    //the vector for the defined gravity
    irr::core::vector3df mGravityVec;

    bool collisionResolutionActive = false;

    //advances physics world by specified amount of seconds
    void AdvancePhysicsTime(const irr::f32 frameDeltaTime);

    void AddObject(irr::scene::ISceneNode* sceneNode);
    void RemoveObject(irr::scene::ISceneNode* sceneNode);
    void RemoveAllObjects();

    //will return NULL if no physicsObject is found that belongs to the
    //specified sceneNode
    PhysicsObject* GetObjectPntr(irr::scene::ISceneNode* sceneNode);

    irr::core::vector3df DbgsumTorqueVec;
    irr::f32 DbgRunCollisionDetectionStage2 = 0.0f;
    irr::f32 DbgCollisionDetected = 0.0f;
    irr::u32 DbgmCollisionTrianglesSize = 0;

    void SetLevelCollisionWallLineData(std::vector<LineStruct*> *newList);

    size_t GetNumTriangleSelectors() const  { return mCollisionSelectors.size(); }
    //irr::scene::ITriangleSelector* GetTriangleSelector(size_t index_) const;

    void AddCollisionMesh(irr::scene::ITriangleSelector* selector_);
    bool RemoveCollisionMesh(irr::scene::ITriangleSelector* selector_);

    bool ForceValid(irr::core::vector3df forceVec);
};

#endif // PHYSICS_H
