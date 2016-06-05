#ifndef __fisher_scene_h__
#define __fisher_scene_h__

#include "fisher.h"

#include <wchar.h>
#include <string>

namespace Fisher {

class Scene
{
public:
    Scene();
    virtual ~Scene();

    void create_terrain(GLint size);
    void create_grid(GLint size);

    void redraw_terrain();
    void redraw_grid();

    inline bool has_changes();

    inline VECTOR& position()
    { return position_; }

    inline VECTOR& rotate()
    { return rotate_; }

    inline bool has_terrain() const
    { return (terrain_ != 0); }

    inline bool has_grid() const
    { return (grid_ != 0); }

private:
    VECTOR rotate_;
    VECTOR position_;

    GLuint terrain_;
    GLuint grid_;
    bool   update_;
};

inline bool Scene::has_changes() {
    __asm {
        mov     al,0
        xchg    [ecx+update_],al
    }
}

}
#endif // __fisher_scene_h__
