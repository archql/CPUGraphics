#ifndef VEC3_H
#define VEC3_H

#include <array>
#include <QDebug>

namespace Math {

class Vec3
{
public:
    static constexpr size_t N = 3;
public:
    Vec3();
    Vec3(double x, double y, double z);

public:
    friend QDebug operator<<(QDebug dbg, const Vec3 &m)
    {
        return dbg << '(' << m.mData[0] << m.mData[1] << m.mData[2] << ")";
    }

public:
    static double dot(const Vec3 &a, const Vec3 &b);
    static Vec3 cross(const Vec3 &a, const Vec3 &b);

public:
    double x() const;
    double y() const;
    double z() const;

public:
    Vec3 normalized() const;

public:
    Vec3 operator+(const Vec3 &other) const;
    Vec3 operator-(const Vec3 &other) const;

    Vec3 &operator*=(double value);

public:
    double &operator[](size_t i);
    double operator[](size_t i) const;

private:
    std::array<double, N> mData{0};
};

} // namespace Math

#endif // VEC3_H
