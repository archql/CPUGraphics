#ifndef CAMERA_H
#define CAMERA_H

#include "mat4.h"
#include "vec3.h"

#include <QObject>

class Camera;
using SharedCamera = QSharedPointer<Camera>;

class Camera : public QObject
{
    Q_OBJECT
public:
    Camera(float x = 0.0, float y = 0.0, float z = 0.0, float sensitivity = 0.001);

public:
    Math::Mat4 view() const;
    const Math::Vec3 &dir() const {return direction;}; // test
    const Math::Vec3 &pos() const {return eye;}; // test

public:
    void moveSide(float left);
    void moveForward(float forward);
    void moveUp(float top);
    void rotate(float x, float y);
    void reset(float x, float y);

protected:
    float lastX;
    float lastY;

    float pitch; // in rads
    float yaw;
    float sensitivity;

    Math::Vec3 eye;
    Math::Vec3 direction;
    Math::Vec3 up;

};

#endif // CAMERA_H
