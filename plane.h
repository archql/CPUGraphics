#ifndef MATH_PLANE_H
#define MATH_PLANE_H

#include "vec3.h"

namespace Math {

class Plane
{
public:
    Plane(const Vec3 &a, const Vec3 &b, const Vec3 &c);

public:
    float distanceTo(const Vec3 &p) const;

protected:
    Vec3 m_normal;
    float m_distance; // D in A*x + B*y+ C*z + D
};

} // namespace Math

#endif // MATH_PLANE_H
