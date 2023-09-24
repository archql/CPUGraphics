#include "camera.h"

#include <numbers>

Camera::Camera(float x, float y, float z, float sensitivity)
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

void Camera::moveForward(float forward)
{
    eye += direction * forward;
}
void Camera::moveSide(float left)
{
    eye += Math::Vec3::cross(direction, up) * left;
}
void Camera::moveUp(float top)
{
    eye += up * top;
}

void Camera::rotate(float x, float y)
{
    float dx = x - lastX;
    float dy = y - lastY;
    lastX = x;
    lastY = y;
    pitch = std::clamp(pitch - dy * sensitivity, -std::numbers::pi_v<float> * 0.499f, std::numbers::pi_v<float> * 0.499f);  // -pi/2 to pi/2
    yaw -= dx * sensitivity;
    //yaw = std::clamp(yaw + dy * sensitivity, -std::numbers::pi * 0.249, std::numbers::pi * 0.249);
    const float ax = cos(yaw) * cos(pitch);
    const float ay = sin(pitch);
    const float az = sin(yaw) * cos(pitch);
    direction = Math::Vec3{ax, ay, az}.normalized();
    const float bx = - cos(yaw) * sin(pitch);
    const float by = cos(pitch);
    const float bz = - sin(yaw) * sin(pitch);
    up = Math::Vec3{bx, by, bz}.normalized();

    //qInfo() << "direction" << direction;
    //qInfo() << "up" << up;

}

void Camera::reset(float x, float y)
{
    lastX = x;
    lastY = y;
}
