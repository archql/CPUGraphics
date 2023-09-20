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
    Camera(double x = 0.0, double y = 0.0, double z = 0.0, double sensitivity = 0.001);

public:
    Math::Mat4 view() const;
    const Math::Vec3 &dir() const {return direction;}; // test
    const Math::Vec3 &pos() const {return eye;}; // test

public:
    void moveSide(double left);
    void moveForward(double forward);
    void moveUp(double top);
    void rotate(double x, double y);
    void reset(double x, double y);

protected:
    double lastX;
    double lastY;

    double pitch; // in rads
    double yaw;
    double sensitivity;

    Math::Vec3 eye;
    Math::Vec3 direction;
    Math::Vec3 up;

};

#endif // CAMERA_H
