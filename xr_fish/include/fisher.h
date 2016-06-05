#ifndef __fisher_h__
#define __fisher_h__

#ifdef _WIN32
#pragma once
#include <windows.h>
#endif

#include <math.h>

#include "gl/glut.h"

namespace Fisher {

/// Structures /////////////////////////////////////////////
struct VECTOR
{
    GLfloat x;
    GLfloat y;
    GLfloat z;

    operator GLfloat* () { return &x; }
    bool operator== (const VECTOR& r) const { return (r.x == x && r.y == y && r.z == z); }
    bool operator!= (const VECTOR& r) const { return !(r.x == x && r.y == y && r.z == z); }
};

struct ALLOC3f : public VECTOR
{
    ALLOC3f() { x = 0; y = 0; z = 0; }
    ALLOC3f(GLfloat vx, GLfloat vy, GLfloat vz) { x = vx; y = vy; z = vz; }
};

struct VERTEX 
{
    GLint x;
    GLint y;
    GLint z;

    operator GLint* () { return &x; }
};

/// Constants /////////////////////////////////////////
const GLuint SCENE_SIZE     = 1;
const short DELTA_T         = 10000; // mks
const short WINDOW_SIZE_X   = 800;
const short WINDOW_SIZE_Y   = 600;
const short WINDOW_POS_X    = 200;
const short WINDOW_POS_Y    = 100;
const char  FRAME_RATE      = 25;

/// Defines /////////////////////////////////////////
#define DEFAULT_PROFILE_NAME   L"Babay"


}
#endif  // __fisher_h__
