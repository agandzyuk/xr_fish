#include "scene.h"
#include "color.h"

using namespace std;
using namespace Fisher;

Scene::Scene() : terrain_(0), grid_(0)
{
    rotate_.x = 6;
    rotate_.y = 32;
}

Scene::~Scene()
{}

void Scene::create_terrain( GLint sz )
{
    GLfloat sz2 = sz*0.9;
    VECTOR points[] = { {  0,  sz,  0 },
                        {-sz2, -sz,  sz2 },
                        { sz2, -sz,  sz2 },
                        { sz2, -sz, -sz2 },
                        {-sz2, -sz, -sz2 } };

    GLuint n = glGenLists(1);
    glNewList(n, GL_COMPILE);


    glBegin(GL_TRIANGLE_FAN);
        Color::set( blue );
        glVertex3fv(points[0]);

        Color::set( darkgray );
        glVertex3fv(points[1]);

        Color::set( green );
        glVertex3fv(points[2]);

        Color::set( yellow );
        glVertex3fv(points[3]);

        Color::set( lightred );
        glVertex3fv(points[4]);

        Color::set( darkgray );
        glVertex3fv(points[1]);
    glEnd();

    glBegin(GL_QUADS);
        Color::set( darkgray );
        glVertex3fv(points[1]);
        Color::set( green );
        glVertex3fv(points[2]);
        Color::set( yellow );
        glVertex3fv(points[3]);
        Color::set( lightred );
        glVertex3fv(points[4]);
    glEnd();

    glEndList();

    terrain_ = n;
}

void Scene::create_grid(GLint size)
{
    // ������ ������ �� 100 ������� ����� ����������� � ������������� �� ���� X � Z

    GLuint n = glGenLists(1);
    glNewList(n, GL_COMPILE);
 
    // ������ ����� 1�1 ����� ����
    for(float i = -size; i <= size; i+=1)
    {
        glBegin(GL_LINES);
            Color::set( green );

            // ��� �
            glVertex3f(-size, 0, i);
            glVertex3f(size, 0, i);
 
            // ��� Z
            glVertex3f(i, 0, -size);
            glVertex3f(i, 0, size);
        glEnd();
    }

    glEndList();
    grid_ = n;
}

void Scene::redraw_terrain()
{
    glPushMatrix();
    
    //glTranslatef(0, 0, -3);
    glRotatef( rotate_.x, 0, 1 ,0 );
    glRotatef( rotate_.y, 1, 0 ,0 );

    glCallList( terrain_ );

    glPopMatrix();
    update_ |= true;
}


void Scene::redraw_grid()
{
    glPushMatrix();
    glCallList( grid_ );
    glPopMatrix();

    update_ |= true;
}
