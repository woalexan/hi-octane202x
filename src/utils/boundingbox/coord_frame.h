#ifndef COORD_FRAME_H
#define COORD_FRAME_H

//This code came from the following article:
//https://www.gamedeveloper.com/programming/c-data-structures-for-rigid-body-physics
//According to the article page the author is Miguel Gomez

//28.03.2025 Wolf Alexander: I made changes to the original source code of the author to prevent warnings about
//type conversions (possible loss of data) under Visual Studio MSVC compiler. Therefore the code below is not the
//original authors source anymore. I also changed the typedef name "POINT" to "POINT3D", because there is already
//a POINT type in windef.h, which will for sure cause trouble

#include "basis.h"

// A coordinate frame (basis and origin) with respect to a parent
//

class COORD_FRAME : public BASIS {

public:

    POINT3D O; //this coordinate frame’s origin, relative to its parent frame

public:

    COORD_FRAME()
    {}

    COORD_FRAME(const POINT3D& o,
                const VECTOR& v0,
                const VECTOR& v1,
                const VECTOR& v2
               ) : O ( o ),

    BASIS ( v0, v1, v2 )
    {}

    COORD_FRAME(const POINT3D& o,
                const BASIS& b
               ): O ( o ),
    BASIS ( b )
    {}

    const POINT3D& position() const { return O; }
    void position( const POINT3D& p ) { O = p; }

    const POINT3D transformPointToLocal( const POINT3D& p ) const {
        //translate to this frame’s origin, then project onto this basis
        return transformVectorToLocal( p - O );
    }

    const POINT3D transformPointToParent( const POINT3D& p ) const {
        //transform the coordinate vector and translate by this origin
        return transformVectorToParent( p ) + O;

    }

    //translate the origin by the given vector
    void translate( const VECTOR& v ) {
        O += v;
    }

}; 

#endif // COORD_FRAME_H
