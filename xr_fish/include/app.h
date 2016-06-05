#ifndef __fisher_app_h__
#define __fisher_app_h__

#include "timer.h"
#include "task.h"
#include "scene.h"
#include "camera.h"
#include "textinfo.h"

namespace Fisher {

class App : public Task
{
public:
    App(int& argc, char* argv[]);
    ~App(void);

    void start();

    static void on_draw();
    static void on_reshape(GLint w, GLint h);

    static void on_mouse_click(int button, int state, int x, int y);
    static void on_mouse_motion(int x, int y);
    static void on_mouse_passive(int x, int y, int z);
    static void on_keyboard(unsigned char key, int x, int y);
    static void on_window_status(int stat);

    virtual void exec();

protected:
    void init(int& argc, char* argv[]);

public:
    static Scene scene_;
    static Camera camera_;
    static Timer timer_;
    static TextInfo textinfo_;
    static RECT clientRect_;
};

}

#endif // __fisher_app_h__
