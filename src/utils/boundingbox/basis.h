#ifndef BASIS_H
#define BASIS_H

//This code came from the following article:
//https://www.gamedeveloper.com/programming/c-data-structures-for-rigid-body-physics
//According to the article page the author is Miguel Gomez

#include "vector.h"
#include "matrix.h"

// An orthonormal basis with respect to a parent
//

class BASIS {

public:

    MATRIX R;

public:

    BASIS() {}

    BASIS(const VECTOR& v0,
          const VECTOR& v1,
          const VECTOR& v2
         ) : R( v0, v1, v2 )
    {}

    BASIS( const MATRIX& m ) : R( m )
    {}

    const VECTOR& operator [] ( long i ) const { return R.C[i]; }
    const VECTOR& x() const { return R.C[0]; }
    const VECTOR& y() const { return R.C[1]; }
    const VECTOR& z() const { return R.C[2]; }
    const MATRIX& basis() const { return R; }

    void basis( const VECTOR& v0, const VECTOR& v1, const VECTOR& v2 ) {
        this->R[0] = v0;
        this->R[1] = v1;
        this->R[2] = v2;
    }

    // Right-Handed Rotations
    void rotateAboutX( const SCALAR& a ) {
        if( 0 != a ) { //don’t rotate by 0
            VECTOR b1 = this->y()*cos(a) + this->z()*sin(a);
            VECTOR b2 = -this->y()*sin(a) + this->z()*cos(a);

            //set basis
            this->R[1] = b1;
            this->R[2] = b2;
            //x is unchanged
        }
    }

    void rotateAboutY( const SCALAR& a ) {
        if( 0 != a ) {//don’t rotate by 0
            VECTOR b2 = this->z()*cos(a) + this->x()*sin(a); //rotate z
            VECTOR b0 = -this->z()*sin(a) + this->x()*cos(a); //rotate x

            //set basis
            this->R[2] = b2;
            this->R[0] = b0;
            //y is unchanged
          }
    }

    void rotateAboutZ( const SCALAR& a ) {
        if( 0 != a ) {//don’t rotate by 0
            //don’t over-write basis before calculation is done
            VECTOR b0 = this->x()*cos(a) + this->y()*sin(a); //rotate x
            VECTOR b1 = -this->x()*sin(a) + this->y()*cos(a); //rotate y

            //set basis
            this->R[0] = b0;
            this->R[1] = b1;
            //z is unchanged
        }
    }

    //rotate the basis about the unit axis u by theta (radians)
    void rotate( const SCALAR& theta, const VECTOR& u );

    //rotate, length of da is theta, unit direction of da is u
    void rotate( const VECTOR& da );

    // Transformations

    const VECTOR transformVectorToLocal( const VECTOR& v ) const {
        return VECTOR( R.C[0].dot(v), R.C[1].dot(v), R.C[2].dot(v) );
    }

    const POINT transformVectorToParent( const VECTOR& v ) const {
        return R.C[0] * v.x + R.C[1] * v.y + R.C[2] * v.z;
    }

};

#endif // BASIS_H
