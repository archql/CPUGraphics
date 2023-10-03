#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"

#include <QDebug>
#include <array>

namespace Math {

#pragma pack(push, 1)
struct MatVals4 {
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float m30, m31, m32, m33;
};
#pragma pack(pop)

class Mat4
{
public:
    static constexpr size_t N = 4;
public:
    Mat4();
//    Mat4(float m00, float m01, float m02, float m03,
//         float m10, float m11, float m12, float m13,
//         float m20, float m21, float m22, float m23,
//         float m30, float m31, float m32, float m33
//        );
    Mat4(std::array<float, N*N> data);

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

    void translate(float x, float y, float z);
    void translate(const Vec3 &tr);

    void scale(float x, float y, float z);
    void scale(const Vec3 &sc);

    void rotateX(float x);
    void rotateY(float y);
    void rotateZ(float z);

    void orto(float w, float h, float zNear, float zFar);
    void perspective(float aspect, float fov, float zNear, float zFar);
    void viewport(float x, float y, float w, float h);
    void view(const Vec3 &eye, const Vec3 &target, const Vec3 &up);
public:
    Mat4 &operator+=(const Mat4 &other);
    Mat4 &operator+=(float value);
    Mat4 &operator*=(float value);

    Mat4 operator+(const Mat4 &other) const;
    Mat4 operator+(float value) const;
    Mat4 operator*(const Mat4 &other) const;
    Mat4 operator*(float value) const;

    // ambigious
    //Vec3 operator*(const Vec3 &other) const;

public:
    Vec3 mul(const Vec3 &other) const;
    Vec3 mulOrthoDiv(const Vec3 &other) const;

public:
    Mat4 inversed() const;

public:
    float &operator[](size_t i);
    float operator[](size_t i) const;

    //float operator[](const size_t &&i) const;

public:
    static constexpr size_t index(const size_t &&i, const size_t &&j)
    {
        return i * N + j;
    }

private:
    std::array<float, N*N> mData{0};
};

} // namespace Math

#endif // MAT4_H
