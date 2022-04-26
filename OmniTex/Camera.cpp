#include "camera.h"

Camera::Camera(const QVector3D& pos)
    : forward(0.0f, 0.0f, -1.0f),
    right(1.0f, 0.0f, 0.0f),
    up(0.0f, 1.0f, 0.0f),
    pos(pos),
    y(0.0f),
    p(0.0f)
{
}

void Camera::yaw(float degrees)
{
    y += std::clamp(degrees, -359.99f, 359.99f);
    yawMatrix.setToIdentity();
    yawMatrix.rotate(y, 0, 1, 0);

    QMatrix4x4 rotMat = pitchMatrix * yawMatrix;
    forward = (QVector4D(0.0f, 0.0f, -1.0f, 0.0f) * rotMat).toVector3D();
    right = (QVector4D(1.0f, 0.0f, 0.0f, 0.0f) * rotMat).toVector3D();
}

void Camera::pitch(float degrees)
{
    p += std::clamp(degrees, -359.99f, 359.99f);;
    pitchMatrix.setToIdentity();
    pitchMatrix.rotate(p, 1, 0, 0);

    QMatrix4x4 rotMat = pitchMatrix * yawMatrix;
    forward = (QVector4D(0.0f, 0.0f, -1.0f, 0.0f) * rotMat).toVector3D();
    up = (QVector4D(0.0f, 1.0f, 0.0f, 0.0f) * rotMat).toVector3D();
}

void Camera::walk(float amount)
{
    pos[0] += amount * forward.x();
    pos[2] += amount * forward.z();
}

void Camera::strafe(float amount)
{
    pos[0] += amount * right.x();
    pos[2] += amount * right.z();
}

QVector3D Camera::getPosition() const
{
    return pos;
}

QMatrix4x4 Camera::getViewMatrix() const
{
    QMatrix4x4 m = pitchMatrix * yawMatrix;
    m.translate(-pos);
    return m;
}
