/* 
 A big obstacle was ray intersection with the environment, even though Irrlicht seemed to provide a function like this. As it soon turned
 out this function does work, but its implementation is not fully finished/optimized, as the ray intersection test is simply done with all triangles of the
 environment (Irrlicht TriangleSelector) which will not work for the overall loaded HiOctane level. I was able to solve this problem by using
 a function which was taken from the following project, and later was modified to fit my project (and Irrlicht)
 https://github.com/francisengelmann/fast_voxel_traversal
 This project uses a MIT License, Copyright (c) 2019 Francis Engelmann  , Please see physics-fast-voxel-traversal-LICENSE.txt for original license
 
 Copyright (C) 2024-2025 Wolf Alexander              (Extend, combine and adjust initial source codes)
 Copyright (c) 2019 Francis Engelmann           (original author of the voxel_traversal function, was later modified by me)
 Copyright (c) 2006-2015 by Michael Zeilfelder  (I reused parts of the source code from HCraft also for ray intersection)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#ifndef RAY_H
#define RAY_H

#include <irrlicht.h>
#include <vector>

#define PHYSICS_MAX_RAYTARGET_TRIANGLES 4000

struct RayHitTriangleInfoStruct {
    irr::core::triangle3df hitTriangle;
    irr::core::vector3df hitPointOnTriangle;
    irr::core::vector3df rayDirVec;
    irr::f32 distFromRayStartSquared = 0.0f;
};

/************************
 * Forward declarations *
 ************************/

class DrawDebug;

class Ray {

private:
    DrawDebug* mDrawDebug;

    typedef std::vector<irr::scene::ITriangleSelector*> RayTargetSelectorVector;

    //Returns indices of all voxels that a defined ray from ray_start to ray_end visits on its path
    std::vector<irr::core::vector3di> voxel_traversal(irr::core::vector3df ray_start, irr::core::vector3df ray_end);
    
public:
    Ray(DrawDebug* drawDbg);
    ~Ray();

    std::vector<irr::scene::ITriangleSelector*> mRayTargetSelectors;
    
    void DrawSelectedRayTargetMeshTriangles(std::vector<RayHitTriangleInfoStruct*> hitInfoTriangles);

    void AddRayTargetMesh(irr::scene::ITriangleSelector* selector_);
    bool RemoveRayTargetMesh(irr::scene::ITriangleSelector* selector_);

    int mRayTargetTrianglesSize = 0;    	// Nr of ray cast triangles which we have
    irr::core::triangle3df mRayTargetTriangles[PHYSICS_MAX_RAYTARGET_TRIANGLES];  // triangles which are targeted currently

    std::vector<RayHitTriangleInfoStruct*> ReturnTrianglesHitByRay( std::vector<irr::scene::ITriangleSelector*> triangleSelectorVector,
                     irr::core::vector3df rayStart, irr::core::vector3df rayEnd, bool ReturnOnlyClosestTriangles = false);

    void EmptyTriangleHitInfoVector(std::vector<RayHitTriangleInfoStruct*> &hitInfoTriangles);

    //void FindRayTargetTriangles(PhysicsObject& physObj, irr::core::vector3df dirVector);

    irr::core::line3df DbgRayTargetLine;
};

#endif // RAY_H
 
 
