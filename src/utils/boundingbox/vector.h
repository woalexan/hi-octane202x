#ifndef VECTOR_H
#define VECTOR_H

//This code came from the following article:
//https://www.gamedeveloper.com/programming/c-data-structures-for-rigid-body-physics
//According to the article page the author is Miguel Gomez

//28.03.2025 Wolf Alexander: I made changes to the original source code of the author to prevent warnings about
//type conversions (possible loss of data) under Visual Studio MSVC compiler. Therefore the code below is not the
//original authors source anymore. I also changed the typedef name "POINT" to "POINT3D", because there is already
//a POINT type in windef.h, which will for sure cause trouble

#include "math.h"

// A floating point number
//
typedef float SCALAR;

// A 3D vector
//

class VECTOR {

    public:

        SCALAR x,y,z; //x,y,z coordinates

        VECTOR() : x(0.0f), y(0.0f), z(0.0f)  {}

        VECTOR( const SCALAR& a, const SCALAR& b, const SCALAR& c )  : x(a), y(b), z(c)   {}

        //index a component
        //NOTE: returning a reference allows
        //you to assign the indexed element
        SCALAR& operator [] ( const long i )   {
            return *((&x) + i);
        }

        //compare
        const bool operator == ( const VECTOR& v ) const {
            return (v.x==x && v.y==y && v.z==z);
        }

        const bool operator != ( const VECTOR& v ) const {
            return !(v == *this);
        }

        //negate
        const VECTOR operator - () const {
            return VECTOR( -x, -y, -z );
        }

        //assign
        const VECTOR& operator = ( const VECTOR& v ) {
            x = v.x;
            y = v.y;
            z = v.z;

            return *this;
        }

        //increment
        const VECTOR& operator += ( const VECTOR& v ) {
            x+=v.x;
            y+=v.y;
            z+=v.z;

            return *this;
        }

        //decrement
        const VECTOR& operator -= ( const VECTOR& v ) {
            x-=v.x;
            y-=v.y;
            z-=v.z;

            return *this;
        }

        //self-multiply
        const VECTOR& operator *= ( const SCALAR& s ) {
            x*=s;
            y*=s;
            z*=s;

            return *this;
        }

        //self-divide
        const VECTOR& operator /= ( const SCALAR& s ) {
            const SCALAR r = 1 / s;

            x *= r;
            y *= r;
            z *= r;

            return *this;
        }

        //add
        const VECTOR operator + ( const VECTOR& v ) const {
            return VECTOR(x + v.x, y + v.y, z + v.z);
        }

        //subtract
        const VECTOR operator - ( const VECTOR& v ) const {
            return VECTOR(x - v.x, y - v.y, z - v.z);
        }

        //post-multiply by a scalar
        const VECTOR operator * ( const SCALAR& s ) const {
            return VECTOR( x*s, y*s, z*s );
        }

        //pre-multiply by a scalar
        friend inline const VECTOR operator * ( const SCALAR& s, const VECTOR& v ) {
            return v * s;
        }

        //divide
        const VECTOR operator / (SCALAR s) const {
            s = 1/s;

            return VECTOR( s*x, s*y, s*z );
        }

        //cross product
        const VECTOR cross( const VECTOR& v ) const {
            //Davis, Snider, "Introduction to Vector Analysis", p. 44
            return VECTOR( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x );
        }

        //scalar dot product
        const SCALAR dot( const VECTOR& v ) const {
            return x*v.x + y*v.y + z*v.z;
        }

        //length
        const SCALAR length() const {
            return (SCALAR)sqrt( (double)this->dot(*this) );
        }

        //unit vector
        const VECTOR unit() const {
            return (*this) / length();
        }

        //make this a unit vector
        void normalize() {
            (*this) /= length();
        }

        //equal within an error ‘e’
        const bool nearlyEquals( const VECTOR& v, const SCALAR e ) const {
            return fabs(x-v.x);
        }
};

//
// A 3D position
//
typedef VECTOR POINT3D; 

#endif // VECTOR_H
