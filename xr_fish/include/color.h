#ifndef __fisher_color_h__
#define __fisher_color_h__

#include "gl/glut.h"

namespace Fisher {

/// RGB popular colors///////////////////////////////////
enum RGB 
{
    black           = 0x000000,
    white           = 0xFFFFFF,

    gray            = 0x808080,
    lightgray       = 0xC0C0C0,
    darkgray        = 0x303030,

    red             = 0xC00000,
    lightred        = 0xFF0000,
    darkred         = 0x300000,

    green           = 0x00C000,
    lightgreen      = 0x00FF00,
    darkgreen       = 0x003000,

    blue            = 0x0000C0,
    lightblue       = 0xC000FF,
    darkblue        = 0x000030,

    yellow          = 0xFFFF00,
    orange          = 0xC0C000,
    brown           = 0xA06020,
    darkbrown       = 0x402000,
    bluish          = 0x00FFFF,
    aqua            = 0x20FFD0,
    purple          = 0xFF00FF,
    magenta         = 0xC040D0,
    softpurple      = 0xA060B4,
    pastelrose      = 0xFB0AB4,
};

/// Helpers for RGB casts ////////////////////////////////
inline GLclampf clamp_r( int cr )
{
    return (0.0039 * ((cr&0xFF0000)>>16));
}

inline GLclampf clamp_g( int cr )
{
    return (0.0039 * ((cr&0xFF00)>>8));
}

inline GLclampf clamp_b( int cr )
{
    return (0.0039 * (cr&0xFF));
}

inline GLubyte byte_r( int cr )
{
    return (cr&0xFF0000)>>16;
}

inline GLubyte byte_g( int cr )
{
    return (cr&0xFF00)>>8;
}

inline GLubyte byte_b( int cr )
{
    return cr&0xFF;
}

/// struct Color ////////////////////////////////
struct Color
{
    GLubyte clr[3];

    GLubyte& r() { return clr[0]; }
    GLubyte& g() { return clr[1]; }
    GLubyte& b() { return clr[2]; }

    inline operator GLubyte* ();
    inline GLubyte& operator[](int idx);
    inline GLubyte operator[](int idx) const;

    static void clear( const GLubyte* cr );
    static void clear( int cr );
    static void set( int cr );
};

/// Color inlines //////////////////////////////
inline Color::operator GLubyte* ()
{ 
    return &clr[0]; 
}

inline GLubyte& Color::operator[](int idx)
{ 
    return *(clr+idx); 
}

inline GLubyte Color::operator[](int idx) const
{ 
    return clr[idx]; 
}

}
#endif  // __fisher_color_h__