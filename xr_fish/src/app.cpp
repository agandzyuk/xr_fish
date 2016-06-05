#include "fisher.h"

#include "app.h"
#include "color.h"
#include "thread.h"

using namespace Fisher;

/// External /////////////////////////////////////////
Timer    App::timer_;
Scene    App::scene_;
Camera   App::camera_;
TextInfo App::textinfo_;
RECT     App::clientRect_;

App::App( int& argc, char* argv[] )
{
    init(argc, argv);
}

App::~App()
{}

void App::start()
{
    scene_.create_terrain(Fisher::SCENE_SIZE);
    scene_.create_grid(30);
    
    //timer_.set_interval(1000);
    timer_.set_task(this);
    timer_.start(true);
    textinfo_.addFont("arial.ttf",16,16);

    glutMainLoop();
}

void App::on_reshape(GLint w, GLint h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    clientRect_.left = 0;
    clientRect_.top = 0;
    clientRect_.right = w;
    clientRect_.bottom = h;
    glViewport (0, 0, w, h);
    //glOrtho(0, WINDOW_X, WINDOW_Y, 0, -1, 1);

    // Вычислим перспективу нашего окна
    // Параметры:
    // (угол взгляда, отношение ширины и высоты,
    // Ближайшее расстояние обьекта до камеры, при котором он виден,
    // и дальнейшее расстояние, при котороом происходит отрисовка
    gluPerspective(45.0f, (GLfloat)clientRect_.right/(GLfloat)clientRect_.bottom, 0.1f, 150.0f);

    //glFrustum(-1, 1, -1, 1, 1, 10);
    glMatrixMode(GL_MODELVIEW);

    on_draw();
} 

static bool lButtonDown = false;
static bool rButtonDown = false;

void App::on_mouse_click(int button, int state, int x, int y)
{
    switch (button) 
    {
        case GLUT_LEFT_BUTTON:
            lButtonDown = (state == GLUT_DOWN);
            break;
        case GLUT_RIGHT_BUTTON:
            rButtonDown = (state == GLUT_DOWN);
            break;
        break;
        default:
            lButtonDown = rButtonDown = false;
        break;
    }

    if (lButtonDown) {
        //scene_.position().x = x;
        //scene_.position().y = y;
    }
}

void App::on_mouse_motion(int x, int y)
{
    static int prevX = x;
    static int prevY = y;

    int deltaX = prevX - x;
    int deltaY = prevY - y;

    if( lButtonDown )
    {
        bool xModified = false;
        bool yModified = false;

        if( deltaX > 0 && deltaX < 15 && (xModified=true))
            scene_.rotate().x -= GLfloat(abs(deltaX)) * 0.5;
        else if( deltaX < 0 && deltaX > -15 && (xModified=true) )
            scene_.rotate().x += GLfloat(abs(deltaX)) * 0.5;


        if ( deltaY > 0 && deltaY < 15 && (yModified=true) )
            scene_.rotate().y += GLfloat(abs(deltaY)) * 0.7;
        else if ( deltaY < 0 && deltaY > -15 && (yModified=true) )
            scene_.rotate().y -= GLfloat(abs(deltaY)) * 0.7;

        on_draw();
        //scene_.position().x = x;
        //scene_.position().y = y;

        if( xModified && scene_.rotate().x > 360.0 )
            scene_.rotate().x -= 360.0;
        else if( xModified && scene_.rotate().x < 360.0 )
            scene_.rotate().x += 360.0;

        if( yModified && scene_.rotate().y > 360.0 )
            scene_.rotate().y -= 360.0;
        else if( yModified && scene_.rotate().y < 360.0 )
            scene_.rotate().y += 360.0;
    }
    else if( rButtonDown )
    {
        if(abs(deltaY) > 1 && abs(deltaY) < 30)
            camera_.moveForward(deltaY);

        if(abs(deltaX) > 1 && abs(deltaX) < 30)
            camera_.rotate(deltaX, ALLOC3f(0,1,0));

        on_draw();
    }
    prevX = x;
    prevY = y;
}

void App::on_mouse_passive(int x, int y, int z)
{
}

void App::on_keyboard(unsigned char key, int x, int y)
{
}

void App::on_window_status(int stat)
{
    int a = stat;
    a = a;
}
 
void App::exec()
{
    if ( push_frame() )
        glutPostRedisplay();
}

void App::on_draw()
{
    timer_.get_task().lock();
    
    int enqued = timer_.get_task().unlock();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity();

    camera_.redraw();

    if( scene_.has_grid() )
        scene_.redraw_grid();

    if( scene_.has_terrain() )
        scene_.redraw_terrain();

    textinfo_.calculateFrameRate(enqued);

    glutSwapBuffers();
}

void App::init(int& argc, char* argv[]) 
{
    const GLubyte* ver = glGetString(GL_VERSION);

    glutInit(&argc, argv);

    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );
    glutInitWindowSize( WINDOW_SIZE_X, WINDOW_SIZE_Y );
    glutInitWindowPosition( WINDOW_POS_X, WINDOW_POS_Y );

    glutCreateWindow("3D Color Histogram");
    Color::clear( black );

  //glOrtho(0, WINDOW_X, WINDOW_Y, 0 , -1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH/*GL_FLAT*/);
    camera_.placement(ALLOC3f(0,0.5f,6), ALLOC3f(0,0.5f,0), ALLOC3f(0,1,0));
    
    glutDisplayFunc( on_draw );
    glutReshapeFunc( on_reshape );
    glutMouseFunc( on_mouse_click );
    glutMotionFunc( on_mouse_motion );
    glutSpaceballMotionFunc( on_mouse_passive );
    glutKeyboardFunc( on_keyboard );
    glutWindowStatusFunc( on_window_status );
}
