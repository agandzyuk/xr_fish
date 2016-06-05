#include "color.h"

using namespace Fisher;

void Color::clear( const GLubyte* cr )
{
    glClearColor( cr[0], cr[1], cr[2], 1.0 );
}

void Color::clear( int cr )
{
    glClearColor( clamp_r(cr) , clamp_g(cr), clamp_b(cr), 1.0 );
}

void Color::set( int cr )
{
    glColor3ub( byte_r(cr) , byte_g(cr), byte_b(cr) );
}
