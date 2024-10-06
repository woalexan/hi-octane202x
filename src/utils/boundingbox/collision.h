#ifndef COLLISION_H
#define COLLISION_H

//This code came from the following 2 articles:
//https://www.gamedeveloper.com/programming/c-data-structures-for-rigid-body-physics
//https://www.gamedeveloper.com/game-platforms/simple-intersection-tests-for-games
//According to both article pages the author is Miguel Gomez

#include "vector.h"
#include "coord_frame.h"
//#include <cstdint>

class PLANE {

public:

   VECTOR N;
   //unit normal

   SCALAR D;
   //distance from the plane to the origin from a
   //normal and a point

   PLANE( const VECTOR& p0, const VECTOR& n ): N(n), D(-N.dot(p0))
   {}
   //from 3 points

   PLANE( const VECTOR& p0, const VECTOR& p1,
          const VECTOR& p2 ): N((p1-p0).cross(p2-p0).unit()),
          D(-N.dot(p0))
   {}
   //signed distance from the plane topoint 'p' along
   //the unit normal

   const SCALAR distanceToPoint( const VECTOR& p ) const {
       return N.dot(p) + D;
   }

};

// An axis-aligned bounding box
class AABB {
   public:

   VECTOR P; //position
   VECTOR E; //x,y,z extents

   AABB( const VECTOR& p,
         const VECTOR& e ): P(p), E(e)

   {}

    //returns true if this is overlapping b
    const bool overlaps( const AABB& b ) const {

        const VECTOR T = b.P - P;//vector from A to B

        return fabs(T.x) <= (E.x + b.E.x)  && fabs(T.y) <= (E.y + b.E.y) && fabs(T.z) <= (E.z + b.E.z);
    }

    //NOTE: since the vector indexing operator is not const,
    //we must cast away the const of the this pointer in the
    //following min() and max() functions
    //min x, y, or z

    const SCALAR min( long i ) const {
        return ((AABB*)this)->P[i] - ((AABB*)this)->E[i];
    }

    //max x, y, or z
    const SCALAR max( long i ) const {
        return ((AABB*)this)->P[i] + ((AABB*)this)->E[i];
    }
};

class OBB : public COORD_FRAME {

public:
   VECTOR E; //extents
   OBB( const VECTOR& e ): E(e) {}

};

const bool SpherePlaneSweep (const SCALAR r, //sphere radius
                             const VECTOR& C0, //previous position of sphere
                             const VECTOR& C1, //current position of sphere
                             const PLANE& plane, //the plane
                             VECTOR& Ci, //position of sphere when it first touched the plane
                             SCALAR& u //normalized time of collision
                             );

const bool AABBSweep (const VECTOR& Ea, //extents of AABB A
                      const VECTOR& A0, //its previous position
                      const VECTOR& A1, //its current position
                      const VECTOR& Eb, //extents of AABB B
                      const VECTOR& B0, //its previous position
                      const VECTOR& B1, //its current position
                      SCALAR& u0, //normalized time of first collision
                      SCALAR& u1 //normalized time of second collision
                     );

const bool AABBOverlapsSphere ( const AABB& B, const SCALAR r, VECTOR& C );

const bool OBBOverlap(
                     //A
                     VECTOR& a, //extents
                     VECTOR& Pa, //position
                     VECTOR* A, //orthonormal basis

                     //B
                     VECTOR& b, //extents
                     VECTOR& Pb, //position
                     VECTOR* B, //orthonormal basis

                     VECTOR* normal, //normal for collision resolution
                     float* resdepth  //depth for collision resolution
                     );

const bool AABB_LineSegmentOverlap (
                                    const VECTOR& l, //line direction
                                    const VECTOR& mid, //midpoint of the line

                                    // segment
                                    const SCALAR hl, //segment half-length
                                    const AABB& b //box
                                   );

#endif // COLLISION_H
