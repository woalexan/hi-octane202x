#ifndef MATRIX_H
#define MATRIX_H

//This code came from the following article:
//https://www.gamedeveloper.com/programming/c-data-structures-for-rigid-body-physics
//According to the article page the author is Miguel Gomez

//28.03.2025 Wolf Alexander: I made changes to the original source code of the author to prevent warnings about
//type conversions (possible loss of data) under Visual Studio MSVC compiler. Therefore the code below is not the
//original authors source anymore. I also changed the typedef name "POINT" to "POINT3D", because there is already
//a POINT type in windef.h, which will for sure cause trouble

#include "vector.h"

// A 3x3 matrix
//
class MATRIX {

    public:

        VECTOR C[3]; //column vectors

    public:

    MATRIX() {

        //identity matrix
        C[0].x = 1.0f;
        C[1].y = 1.0f;
        C[2].z = 1.0f;
    }

    MATRIX( const VECTOR& c0, const VECTOR& c1, const VECTOR& c2 ) {
        C[0] = c0;
        C[1] = c1;
        C[2] = c2;
    }

    //index a column, allow assignment
    //NOTE: using this index operator along with the vector index
    //gives you M[column][row], not the standard M[row][column]
    VECTOR& operator [] ( long i ) {
        return C[i];
    }

    //compare
    const bool operator == ( const MATRIX& m ) const {
        return C[0]==m.C[0] && C[1]==m.C[1] && C[2]==m.C[2];
    }

    const bool operator != ( const MATRIX& m ) const {
        return !(m == *this);
    }

    //assign
    const MATRIX& operator = ( const MATRIX& m ) {
        C[0] = m.C[0];
        C[1] = m.C[1];
        C[2] = m.C[2];

        return *this;
    }

    //increment
    const MATRIX& operator +=( const MATRIX& m ) {
        C[0] += m.C[0];
        C[1] += m.C[1];
        C[2] += m.C[2];

        return *this;
    }

    //decrement
    const MATRIX& operator -=( const MATRIX& m ) {
        C[0] -= m.C[0];
        C[1] -= m.C[1];
        C[2] -= m.C[2];

        return *this;
    }

    //self-multiply by a scalar
    const MATRIX& operator *= ( const SCALAR& s ) {
        C[0] *= s;
        C[1] *= s;
        C[2] *= s;

        return *this;
    }

    //self-multiply by a matrix
    const MATRIX& operator *= ( const MATRIX& m ) {
        //NOTE: don’t change the columns
        //in the middle of the operation

        MATRIX temp = (*this);
        C[0] = temp * m.C[0];
        C[1] = temp * m.C[1];
        C[2] = temp * m.C[2];

        return *this;
    }

    //add
    const MATRIX operator + ( const MATRIX& m ) const {
        return MATRIX( C[0] + m.C[0], C[1] + m.C[1], C[2] + m.C[2] );
    }

    //subtract
    const MATRIX operator - ( const MATRIX& m ) const {
        return MATRIX( C[0] - m.C[0], C[1] - m.C[1], C[2] - m.C[2] );
    }

    //post-multiply by a scalar
    const MATRIX operator * ( const SCALAR& s ) const {
        return MATRIX( C[0]*s, C[1]*s, C[2]*s );
    }

    //pre-multiply by a scalar
    friend inline const MATRIX operator * ( const SCALAR& s, const MATRIX& m ) {
        return m * s;
    }

    //post-multiply by a vector
    const VECTOR operator * ( const VECTOR& v ) const {
        return( C[0]*v.x + C[1]*v.y + C[2]*v.z );
    }

    //pre-multiply by a vector
    inline friend const VECTOR operator * ( const VECTOR& v, const MATRIX& m ) {
        return VECTOR( m.C[0].dot(v), m.C[1].dot(v), m.C[2].dot(v) );
    }

    //post-multiply by a matrix
    const MATRIX operator * ( const MATRIX& m ) const {
        return MATRIX( (*this) * m.C[0], (*this) * m.C[1], (*this) * m.C[2] );
    }

    //transpose
    MATRIX transpose() const {
        //turn columns on their side
        return MATRIX( VECTOR( C[0].x, C[1].x, C[2].x ), //column 0
               VECTOR( C[0].y, C[1].y, C[2].y ), //column 1
               VECTOR( C[0].z, C[1].z, C[2].z ) //column 2
        );
    }

    //scalar determinant
    const SCALAR determinant() const {
        //Lang, "Linear Algebra", p. 143
        return C[0].dot( C[1].cross(C[2]) );
    }

    //matrix inverse
    //const MATRIX inverse() const;
    };

#endif // MATRIX_H
