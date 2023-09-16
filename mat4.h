#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"

#include <QDebug>
#include <array>

namespace Math {

#pragma pack(push, 1)
struct MatVals4 {
    double m00, m01, m02, m03;
    double m10, m11, m12, m13;
    double m20, m21, m22, m23;
    double m30, m31, m32, m33;
};
#pragma pack(pop)

class Mat4
{
public:
    static constexpr size_t N = 4;
public:
    Mat4();
//    Mat4(double m00, double m01, double m02, double m03,
//         double m10, double m11, double m12, double m13,
//         double m20, double m21, double m22, double m23,
//         double m30, double m31, double m32, double m33
//        );
    Mat4(std::array<double, N*N> data);

public:
    friend QDebug operator<<(QDebug dbg, const Mat4 &m)
    {
        return dbg << '(' << m.mData[0*N+0] << m.mData[0*N+1] << m.mData[0*N+2] << m.mData[0*N+3] << ")\n"
                   << '(' << m.mData[1*N+0] << m.mData[1*N+1] << m.mData[1*N+2] << m.mData[1*N+3] << ")\n"
                   << '(' << m.mData[2*N+0] << m.mData[2*N+1] << m.mData[2*N+2] << m.mData[2*N+3] << ")\n"
                   << '(' << m.mData[3*N+0] << m.mData[3*N+1] << m.mData[3*N+2] << m.mData[3*N+3] << ")";
    }

public:
    void loadIdentity();
    void loadZero();

    void translate(double x, double y, double z);
    void translate(const Vec3 &tr);

    void scale(double x, double y, double z);
    void scale(const Vec3 &sc);

    void rotateX(double x);
    void rotateY(double y);
    void rotateZ(double z);

    void perspective(double aspect, double fov, double zNear, double zFar);
    void viewport(double x, double y, double w, double h);
    void view(const Vec3 &eye, const Vec3 &target, const Vec3 &up);
public:
    Mat4 &operator+=(const Mat4 &other);
    Mat4 &operator+=(double value);
    Mat4 &operator*=(double value);

    Mat4 operator+(const Mat4 &other) const;
    Mat4 operator+(double value) const;
    Mat4 operator*(const Mat4 &other) const;
    Mat4 operator*(double value) const;

    Vec3 operator*(const Vec3 &other) const;

public:
    double &operator[](size_t i);
    double operator[](size_t i) const;

private:
    std::array<double, N*N> mData{0};
};

} // namespace Math

#endif // MAT4_H
