#include "camera.h"

#include <numbers>

Camera::Camera(double x, double y, double z, double sensitivity)
    : pitch(0)
    , yaw(0)
    , sensitivity{sensitivity}
    , eye{x, y, z}
    , direction{1, 0, 0} // {1, 0, 0}
    , up{0, 1, 0} // {0, 1, 0}
{

}

Math::Mat4 Camera::view() const
{
    Math::Mat4 mat;
    mat.view(eye, eye + direction, up);
    return mat;
}

void Camera::moveForward(double forward)
{
    eye += direction * forward;
}
void Camera::moveSide(double left)
{
    eye += Math::Vec3::cross(direction, up) * left;
}

void Camera::rotate(double x, double y)
{
    double dx = x - lastX;
    double dy = y - lastY;
    lastX = x;
    lastY = y;
    pitch = std::clamp(pitch - dy * sensitivity, -std::numbers::pi * 0.499, std::numbers::pi * 0.499);  // -pi/2 to pi/2
    yaw -= dx * sensitivity;
    //yaw = std::clamp(yaw + dy * sensitivity, -std::numbers::pi * 0.249, std::numbers::pi * 0.249);
    const double ax = cos(yaw) * cos(pitch);
    const double ay = sin(pitch);
    const double az = sin(yaw) * cos(pitch);
    direction = Math::Vec3{ax, ay, az}.normalized();
    const double bx = - cos(yaw) * sin(pitch);
    const double by = cos(pitch);
    const double bz = - sin(yaw) * sin(pitch);
    up = Math::Vec3{bx, by, bz}.normalized();

    //qInfo() << "direction" << direction;
    //qInfo() << "up" << up;

}

void Camera::reset(double x, double y)
{
    lastX = x;
    lastY = y;
}
