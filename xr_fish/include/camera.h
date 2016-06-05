#ifndef __fisher_camera_h__
#define __fisher_camera_h__

#include "fisher.h"

namespace Fisher {

/// class Camera ///////////////////////////////////////
class Camera
{
public:
    Camera();
    ~Camera();

    void redraw();
    void placement(const VECTOR& pos, const VECTOR& vw, const VECTOR& upDown);
    void moveForward(GLfloat speed);
    void rotate(GLfloat angle, const VECTOR& vw);

    inline bool has_changes();

    inline const VECTOR& pos() const
    { return position_; }

    inline const VECTOR& view() const
    { return view_; }

private:
    VECTOR position_;
    VECTOR view_;
    VECTOR axisY_;
    bool update_;
};

inline bool Camera::has_changes() {
    __asm {
        mov     al,0
        xchg    [ecx+update_],al
    }
}

}

#endif  // __fisher_camera_h__