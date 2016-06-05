#ifndef __axbbox_h__
#define __axbbox_h__

#include "gl_cl.h"

class  AxisBBox
{
public:

    // invalid volume
    AxisBBox();

    // volume of point
    AxisBBox( const XRVec4f& point );

    // from another volume
    AxisBBox( const AxisBBox& volume );

    // bounding by limitations 
    AxisBBox( const XRVec4f& min_point, const XRVec4f& max_point );

    // represents uninitialized volume
    bool IsVoid() const { return !isValid; }

    // adds new point 
    void Add( const XRVec4f& vector );
    // combines between volumes
    void Combine( const AxisBBox& volume );

    // returns new by adding a new point to owner
    AxisBBox Added( const XRVec4f& point ) const;
    // returns new created by combining with owner
    AxisBBox Combined (const AxisBBox& volume) const;

    // clear (mark as invalid)
    void Clear();

    // evaluates surface area of object
    float Area() const;

    // returns diagonal
    XRVec4f Size() const;

    // returns minimum in bounding 
    XRVec4f& Min() { return minPoint; }
    // returns maximum in bounding 
    XRVec4f& Max() { return maxPoint; }

private:

    XRVec4f minPoint;      
    XRVec4f maxPoint;

    // validation flag
    bool isValid;
};

#endif // __axbbox_h__
