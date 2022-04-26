#pragma once
#include <QVector3D>
#include <QMatrix4x4>

class Camera
{
public:
    Camera(const QVector3D& pos); 
    void yaw(float degrees);
    void pitch(float degrees);
    void walk(float amount);
    void strafe(float amount);
    QVector3D getPosition() const;
    QMatrix4x4 getViewMatrix() const;

private:
    QVector3D forward, right, up, pos;     
    float y, p;     
    QMatrix4x4 yawMatrix, pitchMatrix; 
};