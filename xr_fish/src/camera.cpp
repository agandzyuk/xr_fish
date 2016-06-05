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

    ALLOC3f direction; // ����. ������ ����������� �������

    // �������� ������ �����������. ����� �������� ������ �� 2� 
    // �����, �� �������� ������ ����� �� ������.
    // ��� ���� ��� �����������, ���� �� �������. 
    // ����� �� ������� �-�, ����������� �����������
    // ��-�������.
    //�������� ����������� ������� (����-�, ���� �� ��������� "�����")
    direction.x = view_.x - position_.x;
    direction.y = view_.y - position_.y;
    direction.z = view_.z - position_.z;

    // ��������� ��� ������� ������ ������ ��� �����.
    // �� ���������� � �������� ��������� ����������� �������, ����������� �� ��������.
    // ����� ����, �� �������, ��� ����� ���� �� ������ ��������� � ������� ��������. ��,
    // ������ ��� ��������� - �� �������� ����� �� ��� �. �� ��� ������ �������� ��������, ���
    // ���������� �����������. �������� ���.
    // ����, ���� �� ������� � ����������� 45 ��������, �� � ������ � ���� �����������.
    // ���� �������� ����� - ������ ������� � �-� ������������� ��������.
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

    // ������� ��� ������ ������� (�����������, ���� �� �������)
    vView.x = view_.x - position_.x;    //����������� �� X
    vView.y = view_.y - position_.y;    //����������� �� Y
    vView.z = view_.z - position_.z;    //����������� �� Z

    // ������, ���� ������ �������, ���������� � "vView", �� ����� ������� ���.
    // ��� �-� ����� ���������, ����� ����� ���������� ������-�������.
    // ����, ��� ����� ��������� ������ ����� �������. ����������� ��� �������� ���:
    // ������, �� ����� �� �����, ������ ������. �� ������� � �����-�� �����, �����?
    // ������, ���� �� �������� ������ ������ ��� �������, ����������� ������� ���������.
    // �������������� ������������ �����, �� ������� �� ������� (������ �������).
    // ��� ������ �� �������� view_ - ������ ��� ��� � ���� ������ 
    // �������. �� ����� ������� view_ ������ position_
    // �� �����, ����� ����������� �� ���.
    // ����� ������� ���-��, ���������� ����� � �������. 
    //
    // ����� ����������� �������� ������, �� ����� ������������ axis-angle ��������.
    // � ������ ������������, ��� ������� ��� �������� �������� �� ����� ������, ��
    // �������� �� ����� ����. Axis-angle �������� ��������� ��� ������� ����� � ������������
    // ������ ������ ���. ��� ������, ��� �� ����� ����� ���� ����� ������� (view_) �
    // ������� ������ ����� �������.
    // ����� ����� ������ ��������� ��������, ������� ��� ���������� ���������
    // ��������: http://astronomy.swin.edu.au/~pbourke/geometry/rotate/

    // ���������� 1 ��� ����� � ������� ����������� ����
    float cosTheta = (float)cos(angle);
    float sinTheta = (float)sin(angle);

    GLfloat x = vw.x;
    GLfloat y = vw.y;
    GLfloat z = vw.z;

    // ������ ����� ������� X ��� ��������� �����
    vNewView.x  = (cosTheta + (1 - cosTheta) * x * x)   * vView.x;
    vNewView.x += ((1 - cosTheta) * x * y - z * sinTheta)   * vView.y;
    vNewView.x += ((1 - cosTheta) * x * z + y * sinTheta)   * vView.z;

    // ������ ������� Y
    vNewView.y  = ((1 - cosTheta) * x * y + z * sinTheta)   * vView.x;
    vNewView.y += (cosTheta + (1 - cosTheta) * y * y)   * vView.y;
    vNewView.y += ((1 - cosTheta) * y * z - x * sinTheta)   * vView.z;

    // � ������� Z
    vNewView.z  = ((1 - cosTheta) * x * z - y * sinTheta)   * vView.x;
    vNewView.z += ((1 - cosTheta) * y * z + x * sinTheta)   * vView.y;
    vNewView.z += (cosTheta + (1 - cosTheta) * z * z)   * vView.z;

    // ������ ������ ������� ����� ������ �������� � ����� �������, �����
    // ���������� ����� ������ ������.
    view_.x = position_.x + vNewView.x;
    view_.y = position_.y + vNewView.y;
    view_.z = position_.z + vNewView.z;

    update_ = true;
}