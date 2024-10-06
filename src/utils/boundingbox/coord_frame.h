#ifndef COORD_FRAME_H
#define COORD_FRAME_H

//This code came from the following article:
//https://www.gamedeveloper.com/programming/c-data-structures-for-rigid-body-physics
//According to the article page the author is Miguel Gomez

#include "basis.h"

// A coordinate frame (basis and origin) with respect to a parent
//

class COORD_FRAME : public BASIS {

public:

    POINT O; //this coordinate frame’s origin, relative to its parent frame

public:

    COORD_FRAME()
    {}

    COORD_FRAME(const POINT& o,
                const VECTOR& v0,
                const VECTOR& v1,
                const VECTOR& v2
               ) : O ( o ),

    BASIS ( v0, v1, v2 )
    {}

    COORD_FRAME(const POINT& o,
                const BASIS& b
               ): O ( o ),
    BASIS ( b )
    {}

    const POINT& position() const { return O; }
    void position( const POINT& p ) { O = p; }

    const POINT transformPointToLocal( const POINT& p ) const {
        //translate to this frame’s origin, then project onto this basis
        return transformVectorToLocal( p - O );
    }

    const POINT transformPointToParent( const POINT& p ) const {
        //transform the coordinate vector and translate by this origin
        return transformVectorToParent( p ) + O;

    }

    //translate the origin by the given vector
    void translate( const VECTOR& v ) {
        O += v;
    }

}; 

#endif // COORD_FRAME_H
