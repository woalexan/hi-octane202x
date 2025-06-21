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

#include "ray.h"
#include "../draw/drawdebug.h"
#include "../definitions.h"

Ray::Ray(DrawDebug* drawDbg) {
    mDrawDebug = drawDbg;
}

Ray::~Ray() {
}

void Ray::AddRayTargetMesh(irr::scene::ITriangleSelector* selector_)
{
    if ( selector_)
    {
        selector_->grab();
        mRayTargetSelectors.push_back(selector_);
    }
}

bool Ray::RemoveRayTargetMesh(irr::scene::ITriangleSelector* selector_)
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

void Ray::DrawSelectedRayTargetMeshTriangles(std::vector<RayHitTriangleInfoStruct*> hitInfoTriangles) {

    std::vector<RayHitTriangleInfoStruct*>::iterator it;
    for (it = hitInfoTriangles.begin(); it != hitInfoTriangles.end(); ++it) {
          mDrawDebug->Draw3DTriangle(&(*it)->hitTriangle, irr::video::SColor(0, 255, 0,127));
    }
}

//Returns all triangles hit by a defined ray from ray start to ray end point
std::vector<RayHitTriangleInfoStruct*> Ray::ReturnTrianglesHitByRay(std::vector<irr::scene::ITriangleSelector*> triangleSelectorVector,
                               irr::core::vector3df rayStart, irr::core::vector3df rayEnd, size_t nrClosestTriangleTargetCnt, bool ReturnOnlyClosestTriangles) {

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
        if (ReturnOnlyClosestTriangles && (mRaySelectedTriangles.size() >= nrClosestTriangleTargetCnt)) {
            return mRaySelectedTriangles;
        }

      }

      return mRaySelectedTriangles;
}

//clean up triangle hit information struct
void Ray::EmptyTriangleHitInfoVector(std::vector<RayHitTriangleInfoStruct*> &hitInfoTriangles) {
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

std::vector<irr::core::vector3di> Ray::voxel_traversal(irr::core::vector3df ray_start, irr::core::vector3df ray_end) {
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
