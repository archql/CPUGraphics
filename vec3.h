#ifndef VEC3_H
#define VEC3_H

#include <array>
#include <QDebug>

namespace Math {

// it behaves like vec3 except it has w cordinate for 3d graphics
class Vec3
{
public:
    //static constexpr size_t X = 0;
    //static constexpr size_t Y = 1;
    //static constexpr size_t Z = 2;
    //static constexpr size_t W = 3;
public:
    static constexpr size_t N = 3;
public:
    Vec3();
    Vec3(float x, float y, float z);

public:
    friend QDebug operator<<(QDebug dbg, const Vec3 &m)
    {
        return dbg << '(' << m.mData[0] << m.mData[1] << m.mData[2] << ")";
    }

public:
    static float dot(const Vec3 &a, const Vec3 &b);
    static Vec3 cross(const Vec3 &a, const Vec3 &b);

public:
    float x() const;
    float y() const;
    float z() const;

    float len() const;

public:
    float w() const;

public:
    Vec3 normalized() const;

public:
    Vec3 operator+(const Vec3 &other) const;
    Vec3 operator-(const Vec3 &other) const;
    Vec3 operator*(float value) const;
    Vec3 operator/(float value) const;

    Vec3 &operator*=(float value);
    Vec3 &operator/=(float value);
    Vec3 &operator+=(const Vec3 &other);

public:
    float &operator[](size_t i);
    float operator[](size_t i) const;

private:
    std::array<float, N> mData{0};
};

} // namespace Math

#endif // VEC3_H
