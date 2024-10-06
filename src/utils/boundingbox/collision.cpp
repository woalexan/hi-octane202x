#include "collision.h"
#include <algorithm>
#include <iostream>
#include <vector>

//This code came from the following 2 articles:
//https://www.gamedeveloper.com/programming/c-data-structures-for-rigid-body-physics
//https://www.gamedeveloper.com/game-platforms/simple-intersection-tests-for-games
//According to both article pages the author is Miguel Gomez

const bool SpherePlaneSweep (const SCALAR r, //sphere radius
                             const VECTOR& C0, //previous position of sphere
                             const VECTOR& C1, //current position of sphere
                             const PLANE& plane, //the plane
                             VECTOR& Ci, //position of sphere when it first touched the plane
                             SCALAR& u //normalized time of collision
                             ) {

    const SCALAR d0 = plane.distanceToPoint( C0 );
    const SCALAR d1 = plane.distanceToPoint( C1 );

    //check if it was touching on previous frame
    if( fabs(d0) <= r ) {
        Ci = C0;
        u = 0;
        return true;
    }

    //check if the sphere penetrated during this frame
    if( d0>r && d1<r ) {
        u = (d0-r)/(d0-d1); //normalized time
        Ci = (1-u)*C0 + u*C1; //point of first contact
        return true;

    }
    return false;
}

//Sweep two AABB's to see if and when they first
//and last were overlapping
const bool AABBSweep (const VECTOR& Ea, //extents of AABB A
                      const VECTOR& A0, //its previous position
                      const VECTOR& A1, //its current position
                      const VECTOR& Eb, //extents of AABB B
                      const VECTOR& B0, //its previous position
                      const VECTOR& B1, //its current position
                      SCALAR& u0, //normalized time of first collision
                      SCALAR& u1 //normalized time of second collision
                     ) {
   const AABB A( A0, Ea );//previous state of AABB A
   const AABB B( B0, Eb );//previous state of AABB B
   const VECTOR va = A1 - A0;//displacement of A
   const VECTOR vb = B1 - B0;//displacement of B

   //the problem is solved in A's frame of reference
   VECTOR v = vb - va;

   //relative velocity (in normalized time)
   VECTOR u_0(0,0,0);

   //first times of overlap along each axis
   VECTOR u_1(1,1,1);

   //last times of overlap along each axis

   //check if they were overlapping
   // on the previous frame
   if( A.overlaps(B) ) {
       u0 = u1 = 0;
       //std::cout << "ov";
       return true;
   } else return(false);

   //find the possible first and last times
   //of overlap along each axis
   for( long i=0 ; i<3 ; i++ ) {
       if( A.max(i)<B.min(i) && v[i]<0 ) {
           u_0[i] = (A.max(i) - B.min(i)) / v[i];
       } else if( B.max(i)<A.min(i) && v[i]>0 ) {
           u_0[i] = (A.min(i) - B.max(i)) / v[i];
       }

       if( B.max(i)>A.min(i) && v[i]<0 ) {
           u_1[i] = (A.min(i) - B.max(i)) / v[i];
       } else if( A.max(i)>B.min(i) && v[i]>0 ) {
           u_1[i] = (A.max(i) - B.min(i)) / v[i];
       }
   }

   //possible first time of overlap
   u0 = std::max( u_0.x, std::max(u_0.y, u_0.z) );

   //possible last time of overlap
   u1 = std::min( u_1.x, std::min(u_1.y, u_1.z) );

   //they could have only collided if
   //the first time of overlap occurred
   //before the last time of overlap
   return u0 <= u1;
}

//Check to see if the sphere overlaps the AABB
const bool AABBOverlapsSphere ( const AABB& B, const SCALAR r, VECTOR& C ) {

   float s, d = 0;

   //find the square of the distance
   //from the sphere to the box
   for( long i=0 ; i<3 ; i++ ) {
    if( C[i] < B.min(i) ) {
           s = C[i] - B.min(i);
           d += s*s;
       } else if( C[i] > B.max(i) ) {
           s = C[i] - B.max(i);
           d += s*s;
       }
   }

   return d <= r*r;
}

//check if two oriented bounding boxes overlap
const bool OBBOverlap(
                     //A
                     VECTOR& a, //extents
                     VECTOR& Pa, //position
                     VECTOR* A, //orthonormal basis

                     //B
                     VECTOR& b, //extents
                     VECTOR& Pb, //position
                     VECTOR* B, //orthonormal basis,

                     VECTOR* normal,
                     float* resdepth
                     ) {

   //translation, in parent frame
   VECTOR v = Pb - Pa;

   //translation, in A's frame
   VECTOR T( v.dot(A[0]), v.dot(A[1]), v.dot(A[2]) );

   //B's basis with respect to A's local frame
   SCALAR R[3][3];
   float ra, rb, t;
   long i, k;
   float depth;

   std::vector<VECTOR> normals;
   std::vector<float> depths;

   //calculate rotation matrix
   for( i=0 ; i<3 ; i++ )
       for( k=0 ; k<3 ; k++ )
           R[i][k] = A[i].dot(B[k]);

           /*ALGORITHM: Use the separating axis test for all 15 potential
           separating axes. If a separating axis could not be found, the two
           boxes overlap. */

           //A's basis vectors
           for( i=0 ; i<3 ; i++ ) {
               ra = a[i];

               rb = b[0]*fabs(R[i][0]) + b[1]*fabs(R[i][1]) + b[2]*fabs(R[i][2]);

               t = fabs( T[i] );

               if( t > ra + rb )
               return false;

               depth = (ra + rb) - t;
               depths.push_back(depth);
               normals.push_back(A[i]);
           }

           //B's basis vectors
           for( k=0 ; k<3 ; k++ ) {
               ra = a[0]*fabs(R[0][k]) + a[1]*fabs(R[1][k]) + a[2]*fabs(R[2][k]);

               rb = b[k];

               t = fabs( T[0]*R[0][k] + T[1]*R[1][k] + T[2]*R[2][k] );

               if( t > ra + rb )
               return false;

               depth = (ra + rb) - t;
               depths.push_back(depth);
               normals.push_back(B[k]);
           }

           //9 cross products
           //L = A0 x B0
           ra = a[1]*fabs(R[2][0]) + a[2]*fabs(R[1][0]);

           rb = b[1]*fabs(R[0][2]) + b[2]*fabs(R[0][1]);

           t =  fabs( T[2]*R[1][0] - T[1]*R[2][0] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[0].cross(B[0]));

           //L = A0 x B1
           ra = a[1]*fabs(R[2][1]) + a[2]*fabs(R[1][1]);

           rb = b[0]*fabs(R[0][2]) + b[2]*fabs(R[0][0]);

           t = fabs( T[2]*R[1][1] - T[1]*R[2][1] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[0].cross(B[1]));

           //L = A0 x B2
           ra = a[1]*fabs(R[2][2]) + a[2]*fabs(R[1][2]);

           rb = b[0]*fabs(R[0][1]) + b[1]*fabs(R[0][0]);

           t = fabs( T[2]*R[1][2] - T[1]*R[2][2] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[0].cross(B[2]));

           //L = A1 x B0
           ra = a[0]*fabs(R[2][0]) + a[2]*fabs(R[0][0]);

           rb = b[1]*fabs(R[1][2]) + b[2]*fabs(R[1][1]);

           t = fabs( T[0]*R[2][0] - T[2]*R[0][0] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[1].cross(B[0]));

           //L = A1 x B1
           ra = a[0]*fabs(R[2][1]) + a[2]*fabs(R[0][1]);

           rb =  b[0]*fabs(R[1][2]) + b[2]*fabs(R[1][0]);

           t = fabs( T[0]*R[2][1] - T[2]*R[0][1] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[1].cross(B[1]));

           //L = A1 x B2
           ra = a[0]*fabs(R[2][2]) + a[2]*fabs(R[0][2]);

           rb = b[0]*fabs(R[1][1]) + b[1]*fabs(R[1][0]);

           t = fabs( T[0]*R[2][2] - T[2]*R[0][2] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[1].cross(B[2]));

           //L = A2 x B0
           ra = a[0]*fabs(R[1][0]) + a[1]*fabs(R[0][0]);

           rb = b[1]*fabs(R[2][2]) + b[2]*fabs(R[2][1]);

           t = fabs( T[1]*R[0][0] - T[0]*R[1][0] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[2].cross(B[0]));

           //L = A2 x B1
           ra = a[0]*fabs(R[1][1]) + a[1]*fabs(R[0][1]);

           rb = b[0] *fabs(R[2][2]) + b[2]*fabs(R[2][0]);

           t = fabs( T[1]*R[0][1] - T[0]*R[1][1] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[2].cross(B[1]));

           //L = A2 x B2
           ra = a[0]*fabs(R[1][2]) + a[1]*fabs(R[0][2]);

           rb = b[0]*fabs(R[2][1]) + b[1]*fabs(R[2][0]);

           t = fabs( T[1]*R[0][2] - T[0]*R[1][2] );

           if( t > ra + rb )
           return false;

           depth = (ra + rb) - t;
           depths.push_back(depth);
           normals.push_back(A[2].cross(B[2]));

           /*no separating axis found,
           the two boxes overlap */

           //derive depth and normal for collision
           //resolution, we want to find the minimum depth
           //this depth and normal is used for collision resolution
           std::vector<float>::iterator it;
           std::vector<float>::iterator minit = depths.begin();
           uint elcnt = 0;
           uint minel = 0;

           for (it = depths.begin(); it != depths.end(); ++it) {
             if ((*it) < (*minit)) {
                 minit = it;
                 minel = elcnt;
             }
             elcnt++;
           }

           //minit contains smallest element
           *resdepth = (*minit);
           VECTOR intnormal;
           intnormal = normals[minel];

           if (intnormal.dot(v) < 0) {
               intnormal = - intnormal;
           }

           intnormal.normalize();
           *normal = intnormal;

           return true;
}

/*
//check if two oriented bounding boxes overlap
const bool OBBOverlap(
                     //A
                     VECTOR& a, //extents
                     VECTOR& Pa, //position
                     VECTOR* A, //orthonormal basis

                     //B
                     VECTOR& b, //extents
                     VECTOR& Pb, //position
                     VECTOR* B //orthonormal basis
                     ) {

   //translation, in parent frame
   VECTOR v = Pb - Pa;

   //translation, in A's frame
   VECTOR T( v.dot(A[0]), v.dot(A[1]), v.dot(A[2]) );

   //B's basis with respect to A's local frame
   SCALAR R[3][3];
   float ra, rb, t;
   long i, k;

   //calculate rotation matrix
   for( i=0 ; i<3 ; i++ )
       for( k=0 ; k<3 ; k++ )
           R[i][k] = A[i].dot(B[k]);

           /*ALGORITHM: Use the separating axis test for all 15 potential
           separating axes. If a separating axis could not be found, the two
           boxes overlap. */
/*
           //A's basis vectors
           for( i=0 ; i<3 ; i++ ) {
               ra = a[i];

               rb = b[0]*fabs(R[i][0]) + b[1]*fabs(R[i][1]) + b[2]*fabs(R[i][2]);

               t = fabs( T[i] );

               if( t > ra + rb )
               return false;
           }

           //B's basis vectors
           for( k=0 ; k<3 ; k++ ) {
               ra = a[0]*fabs(R[0][k]) + a[1]*fabs(R[1][k]) + a[2]*fabs(R[2][k]);

               rb = b[k];

               t = fabs( T[0]*R[0][k] + T[1]*R[1][k] + T[2]*R[2][k] );

               if( t > ra + rb )
               return false;
           }

           //9 cross products
           //L = A0 x B0
           ra = a[1]*fabs(R[2][0]) + a[2]*fabs(R[1][0]);

           rb = b[1]*fabs(R[0][2]) + b[2]*fabs(R[0][1]);

           t =  fabs( T[2]*R[1][0] - T[1]*R[2][0] );

           if( t > ra + rb )
           return false;

           //L = A0 x B1
           ra = a[1]*fabs(R[2][1]) + a[2]*fabs(R[1][1]);

           rb = b[0]*fabs(R[0][2]) + b[2]*fabs(R[0][0]);

           t = fabs( T[2]*R[1][1] - T[1]*R[2][1] );

           if( t > ra + rb )
           return false;

           //L = A0 x B2
           ra = a[1]*fabs(R[2][2]) + a[2]*fabs(R[1][2]);

           rb = b[0]*fabs(R[0][1]) + b[1]*fabs(R[0][0]);

           t = fabs( T[2]*R[1][2] - T[1]*R[2][2] );

           if( t > ra + rb )
           return false;

           //L = A1 x B0
           ra = a[0]*fabs(R[2][0]) + a[2]*fabs(R[0][0]);

           rb = b[1]*fabs(R[1][2]) + b[2]*fabs(R[1][1]);

           t = fabs( T[0]*R[2][0] - T[2]*R[0][0] );

           if( t > ra + rb )
           return false;

           //L = A1 x B1
           ra = a[0]*fabs(R[2][1]) + a[2]*fabs(R[0][1]);

           rb =  b[0]*fabs(R[1][2]) + b[2]*fabs(R[1][0]);

           t = fabs( T[0]*R[2][1] - T[2]*R[0][1] );

           if( t > ra + rb )
           return false;

           //L = A1 x B2
           ra = a[0]*fabs(R[2][2]) + a[2]*fabs(R[0][2]);

           rb = b[0]*fabs(R[1][1]) + b[1]*fabs(R[1][0]);

           t = fabs( T[0]*R[2][2] - T[2]*R[0][2] );

           if( t > ra + rb )
           return false;

           //L = A2 x B0
           ra = a[0]*fabs(R[1][0]) + a[1]*fabs(R[0][0]);

           rb = b[1]*fabs(R[2][2]) + b[2]*fabs(R[2][1]);

           t = fabs( T[1]*R[0][0] - T[0]*R[1][0] );

           if( t > ra + rb )
           return false;

           //L = A2 x B1
           ra = a[0]*fabs(R[1][1]) + a[1]*fabs(R[0][1]);

           rb = b[0] *fabs(R[2][2]) + b[2]*fabs(R[2][0]);

           t = fabs( T[1]*R[0][1] - T[0]*R[1][1] );

           if( t > ra + rb )
           return false;

           //L = A2 x B2
           ra = a[0]*fabs(R[1][2]) + a[1]*fabs(R[0][2]);

           rb = b[0]*fabs(R[2][1]) + b[1]*fabs(R[2][0]);

           t = fabs( T[1]*R[0][2] - T[0]*R[1][2] );

           if( t > ra + rb )
           return false;

           /*no separating axis found,
           the two boxes overlap */
/*
           return true;
}*/


//Important note!!! The following function assumes the line segment has already been transformed to box space.
const bool AABB_LineSegmentOverlap (
                                    const VECTOR& l, //line direction
                                    const VECTOR& mid, //midpoint of the line

                                    // segment
                                    const SCALAR hl, //segment half-length
                                    const AABB& b //box
                                   ) {
   /* ALGORITHM: Use the separating axis
   theorem to see if the line segment
   and the box overlap. A line
   segment is a degenerate OBB. */

   const VECTOR T = b.P - mid;
   VECTOR v;
   SCALAR r;

   //do any of the principal axes
   //form a separating axis?

   if( fabs(T.x) > b.E.x + hl*fabs(l.x) )
   return false;

   if( fabs(T.y) > b.E.y + hl*fabs(l.y) )
   return false;

   if( fabs(T.z) > b.E.z + hl*fabs(l.z) )
   return false;

   /* NOTE: Since the separating axis is
   perpendicular to the line in these
   last four cases, the line does not
   contribute to the projection. */

   //l.cross(x-axis)?

   r = b.E.y*fabs(l.z) + b.E.z*fabs(l.y);

   if( fabs(T.y*l.z - T.z*l.y) > r )
   return false;

   //l.cross(y-axis)?

   r = b.E.x*fabs(l.z) + b.E.z*fabs(l.x);

   if( fabs(T.z*l.x - T.x*l.z) > r )
   return false;

   //l.cross(z-axis)?

   r = b.E.x*fabs(l.y) + b.E.y*fabs(l.x);

   if( fabs(T.x*l.y - T.y*l.x) > r )
   return false;

   return true;
}
