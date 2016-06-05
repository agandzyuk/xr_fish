#include "camera.h"

using namespace Fisher;

VECTOR iniPos   = {0, 0, 0};
VECTOR iniView  = {0, 1, 0.5f};
VECTOR iniAxisY = {0, 0, 1};

////////////////////////////////////////////////
Camera::Camera()
    : position_(iniPos),
    view_(iniView),
    axisY_(iniAxisY),
    update_(true)
{}

Camera::~Camera()
{}

void Camera::redraw()
{
    gluLookAt( position_.x, position_.y, position_.z, 
               view_.x, view_.y, view_.z,
               axisY_.x, axisY_.y, axisY_.z );
}

void Camera::placement(const VECTOR& pos, const VECTOR& vw, const VECTOR& upDown)
{
    if( position_ != pos ) {
        position_ = pos;
        update_ = true;
    }
    if( view_ != vw ) {
        view_ = vw;
        update_ = true;
    }
    if( axisY_ != upDown ) {
        axisY_ = upDown;
        update_ = true;
    }
}

void Camera::moveForward(GLfloat speed)
{
    speed *= -0.0015f;

    ALLOC3f direction; // Иниц. вектор направления взгляда

    // Получаем вектор направления. Чтобы получить вектор из 2х 
    // точек, мы вычитаем первую точку из второй.
    // Это дает нам направление, куда мы смотрим. 
    // Позже мы напишем ф-ю, вычисляющую направление
    // по-другому.
    //Получаем направление взгляда (напр-е, куда мы повернуты "лицом")
    direction.x = view_.x - position_.x;
    direction.y = view_.y - position_.y;
    direction.z = view_.z - position_.z;

    // Следующий код двигает камеру вперед или назад.
    // Мы прибавляем к текущему положению направление взгляда, помноженное на скорость.
    // Может быть, вы думаете, что проще было бы просто прибавить к позиции скорость. Да,
    // сейчас это сработает - вы смотрите прямо по оси Х. Но как только начнется вращение, код
    // перестанет действовать. Поверьте мне.
    // Итак, если мы смотрим в направлении 45 градусов, мы и пойдем в этом направлении.
    // Если движемся назад - просто передаём в ф-ю отрицательную скорость.
    position_.x += direction.x * speed;
    position_.z += direction.z * speed;
    view_.x += direction.x * speed;
    view_.y += direction.z * speed;

    update_ = true;
}

void Camera::rotate(GLfloat angle, const VECTOR& vw)
{
    angle *= -0.0015f;

    VECTOR vNewView;
    VECTOR vView;

    // Получим наш вектор взгляда (направление, куда мы смотрим)
    vView.x = view_.x - position_.x;    //направление по X
    vView.y = view_.y - position_.y;    //направление по Y
    vView.z = view_.z - position_.z;    //направление по Z

    // Теперь, имея вектор взгляда, хранящийся в "vView", мы можем вращать его.
    // Эта ф-я будет вызыватся, когда нужно повернутся налево-направо.
    // Итак, нам нужно вращаться вокруг нашей позиции. Представьте это примерно так:
    // скажем, мы стоим на месте, смотря вперед. Мы смотрим в какую-то точку, верно?
    // Теперь, если мы повернем голову налево или направо, направление взгляда изменится.
    // Соответственно переместится точка, на которую мы смотрим (вектор взгляда).
    // Вот почему мы изменяем view_ - потому что это и есть вектор 
    // взгляда. Мы будем вращать view_ вокруг position_
    // по кругу, чтобы реализовать всё это.
    // Чтобы вращать что-то, используем синус и косинус. 
    //
    // Чтобы реализовать вращение камеры, мы будем использовать axis-angle вращение.
    // Я должен предупредить, что формулы для рассчета вращения не очень просты, но
    // занимают не много кода. Axis-angle вращение позволяет нам вращать точку в пространстве
    // вокруг нужной оси. Это значит, что мы можем взять нашу точку взгляда (view_) и
    // вращать вокруг нашей позиции.
    // Чтобы лучше понять следующие рассчеты, советую вам посмотреть детальное
    // описание: http://astronomy.swin.edu.au/~pbourke/geometry/rotate/

    // Рассчитаем 1 раз синус и косинус переданного угла
    float cosTheta = (float)cos(angle);
    float sinTheta = (float)sin(angle);

    GLfloat x = vw.x;
    GLfloat y = vw.y;
    GLfloat z = vw.z;

    // Найдем новую позицию X для вращаемой точки
    vNewView.x  = (cosTheta + (1 - cosTheta) * x * x)   * vView.x;
    vNewView.x += ((1 - cosTheta) * x * y - z * sinTheta)   * vView.y;
    vNewView.x += ((1 - cosTheta) * x * z + y * sinTheta)   * vView.z;

    // Найдем позицию Y
    vNewView.y  = ((1 - cosTheta) * x * y + z * sinTheta)   * vView.x;
    vNewView.y += (cosTheta + (1 - cosTheta) * y * y)   * vView.y;
    vNewView.y += ((1 - cosTheta) * y * z - x * sinTheta)   * vView.z;

    // И позицию Z
    vNewView.z  = ((1 - cosTheta) * x * z - y * sinTheta)   * vView.x;
    vNewView.z += ((1 - cosTheta) * y * z + x * sinTheta)   * vView.y;
    vNewView.z += (cosTheta + (1 - cosTheta) * z * z)   * vView.z;

    // Теперь просто добавим новый вектор вращения к нашей позиции, чтобы
    // установить новый взгляд камеры.
    view_.x = position_.x + vNewView.x;
    view_.y = position_.y + vNewView.y;
    view_.z = position_.z + vNewView.z;

    update_ = true;
}