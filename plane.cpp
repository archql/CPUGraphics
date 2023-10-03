#include "plane.h"

namespace Math {

Plane::Plane(const Vec3 &a, const Vec3 &b, const Vec3 &c)
{
    m_normal = Vec3::cross(b - a, c - a);
    m_distance = Vec3::dot(m_normal, a);
}

float Plane::distanceTo(const Vec3 &p) const
{
    return Vec3::dot(m_normal, p) - m_distance;
}

} // namespace Math
