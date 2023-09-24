#include "mat4.h"

#include <algorithm>
#include <numbers>
#include <cmath>
#include <assert.h>

using namespace Math;

Mat4::Mat4()
{

}

//Mat4::Mat4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
//{
//    mData[0*N+0] = m00;
//}

Mat4::Mat4(std::array<float, N*N> data)
{
    mData.swap(data);
}

void Mat4::loadIdentity()
{
    // initialize as identity matrix
    mData[0*N + 0] = 1;
    mData[1*N + 1] = 1;
    mData[2*N + 2] = 1;
    mData[3*N + 3] = 1;
}

void Mat4::loadZero()
{
    // initialize as zero matrix
    std::fill(mData.begin(), mData.end(), 0.0);
}

void Mat4::translate(float x, float y, float z)
{
    loadIdentity();
    mData[0*N + 3] = x;
    mData[1*N + 3] = y;
    mData[2*N + 3] = z;
}

void Mat4::translate(const Vec3 &tr)
{
    loadIdentity();
    mData[0*N + 3] = tr[0];
    mData[1*N + 3] = tr[1];
    mData[2*N + 3] = tr[2];
}

void Mat4::scale(float x, float y, float z)
{
    mData[0*N + 0] = x;
    mData[1*N + 1] = y;
    mData[2*N + 2] = z;
    mData[3*N + 3] = 1;
}

void Mat4::scale(const Vec3 &sc)
{
    mData[0*N + 0] = sc[0];
    mData[1*N + 1] = sc[1];
    mData[2*N + 2] = sc[2];
    mData[3*N + 3] = 1;
}

void Mat4::rotateX(float x)
{
    const float s = sin(x * (std::numbers::pi / 180.0));
    const float c = cos(x * (std::numbers::pi / 180.0));
    mData[0*N + 0] = 1;
    mData[1*N + 1] = c;
    mData[2*N + 2] = c;
    mData[1*N + 2] = -s;
    mData[2*N + 1] = s;
    mData[3*N + 3] = 1;
}

void Mat4::rotateY(float y)
{
    const float s = sin(y * (std::numbers::pi / 180.0));
    const float c = cos(y * (std::numbers::pi / 180.0));
    mData[0*N + 0] = c;
    mData[1*N + 1] = 1;
    mData[2*N + 2] = c;
    mData[0*N + 2] = s;
    mData[2*N + 0] = -s;
    mData[3*N + 3] = 1;
}

void Mat4::rotateZ(float z)
{
    const float s = sin(z * (std::numbers::pi / 180.0));
    const float c = cos(z * (std::numbers::pi / 180.0));
    mData[0*N + 0] = c;
    mData[1*N + 1] = c;
    mData[2*N + 2] = 1;
    mData[0*N + 1] = -s;
    mData[1*N + 0] = s;
    mData[3*N + 3] = 1;
}

void Mat4::orto(float w, float h, float zNear, float zFar)
{
    const float temp2 = zNear - zFar;
    mData[0*N + 0] = 2 / w;
    mData[1*N + 1] = 2 / h;
    mData[2*N + 2] = zFar / temp2;
    mData[2*N + 3] = zNear * zFar / temp2;
    mData[3*N + 3] = 1;
}

void Mat4::perspective(float aspect, float fov, float zNear, float zFar)
{
    //
    const float temp2 = zNear - zFar;
    const float temp = 1.0 / tan(fov * (std::numbers::pi / 360.0));
    //
    mData[0*N + 0] = temp / aspect;
    mData[1*N + 1] = temp;
    mData[2*N + 2] = zFar / temp2;
    mData[2*N + 3] = zNear * zFar / temp2;
    mData[3*N + 2] = -1; //
}

void Mat4::viewport(float x, float y, float w, float h)
{
    // initialize
    loadZero();
    mData[0*N + 0] = w / 2;
    mData[0*N + 3] = x + w / 2;
    mData[1*N + 1] = -h / 2;
    mData[1*N + 3] = y + h / 2;
    mData[2*N + 2] = 1;
    mData[3*N + 3] = 1;
}

void Mat4::view(const Vec3 &eye, const Vec3 &target, const Vec3 &up)
{
    auto zAxis = (eye - target).normalized();
    auto xAxis = (Vec3::cross(up, zAxis)).normalized();
    // initialize as identity matrix
    loadZero();
    mData[0*N + 0] = xAxis.x();
    mData[0*N + 1] = xAxis.y();
    mData[0*N + 2] = xAxis.z();
    mData[1*N + 0] = up.x();
    mData[1*N + 1] = up.y();
    mData[1*N + 2] = up.z();
    mData[2*N + 0] = zAxis.x();
    mData[2*N + 1] = zAxis.y();
    mData[2*N + 2] = zAxis.z();

    mData[0*N + 3] = -(Vec3::dot(xAxis, eye));
    mData[1*N + 3] = -(Vec3::dot(up, eye));
    mData[2*N + 3] = -(Vec3::dot(zAxis, eye));
    mData[3*N + 3] = 1;
}


Math::Mat4 &Math::Mat4::operator+=(const Mat4 &other)
{
    std::transform(other.mData.begin(), other.mData.end(), mData.begin(), mData.begin(), std::plus<>{});
    return *this;
}

Mat4 &Mat4::operator+=(float value)
{
    std::for_each(mData.begin(), mData.end(), [value](float &v){v += value;});
    return *this;
}

Mat4 &Mat4::operator*=(float value)
{
    std::for_each(mData.begin(), mData.end(), [value](float &v){v *= value;});
    return *this;
}

Mat4 Mat4::operator+(const Mat4 &other) const
{
    Mat4 mat;
    return mat += *this;
}

Mat4 Mat4::operator+(float value) const
{
    Mat4 mat;
    return mat += value;
}

Mat4 Mat4::operator*(float value) const
{
    Mat4 mat;
    return mat *= value;
}

float &Mat4::operator[](size_t i)
{
    assert((i < N*N) && "matrix index can not be > 15");
    return mData[i];
}

float Mat4::operator[](size_t i) const
{
    assert((i < N*N) && "matrix index can not be > 15");
    return mData[i];
}

Math::Mat4 Math::Mat4::operator*(const Mat4 &other) const
{
    Mat4 mat;
    mat.mData[0*N+0] = other.mData[0*N+0]*mData[0*N+0] + other.mData[1*N+0]*mData[0*N+1] + other.mData[2*N+0]*mData[0*N+2] + other.mData[3*N+0]*mData[0*N+3];
    mat.mData[1*N+0] = other.mData[0*N+0]*mData[1*N+0] + other.mData[1*N+0]*mData[1*N+1] + other.mData[2*N+0]*mData[1*N+2] + other.mData[3*N+0]*mData[1*N+3];
    mat.mData[2*N+0] = other.mData[0*N+0]*mData[2*N+0] + other.mData[1*N+0]*mData[2*N+1] + other.mData[2*N+0]*mData[2*N+2] + other.mData[3*N+0]*mData[2*N+3];
    mat.mData[3*N+0] = other.mData[0*N+0]*mData[3*N+0] + other.mData[1*N+0]*mData[3*N+1] + other.mData[2*N+0]*mData[3*N+2] + other.mData[3*N+0]*mData[3*N+3];

    mat.mData[0*N+1] = other.mData[0*N+1]*mData[0*N+0] + other.mData[1*N+1]*mData[0*N+1] + other.mData[2*N+1]*mData[0*N+2] + other.mData[3*N+1]*mData[0*N+3];
    mat.mData[1*N+1] = other.mData[0*N+1]*mData[1*N+0] + other.mData[1*N+1]*mData[1*N+1] + other.mData[2*N+1]*mData[1*N+2] + other.mData[3*N+1]*mData[1*N+3];
    mat.mData[2*N+1] = other.mData[0*N+1]*mData[2*N+0] + other.mData[1*N+1]*mData[2*N+1] + other.mData[2*N+1]*mData[2*N+2] + other.mData[3*N+1]*mData[2*N+3];
    mat.mData[3*N+1] = other.mData[0*N+1]*mData[3*N+0] + other.mData[1*N+1]*mData[3*N+1] + other.mData[2*N+1]*mData[3*N+2] + other.mData[3*N+1]*mData[3*N+3];

    mat.mData[0*N+2] = other.mData[0*N+2]*mData[0*N+0] + other.mData[1*N+2]*mData[0*N+1] + other.mData[2*N+2]*mData[0*N+2] + other.mData[3*N+2]*mData[0*N+3];
    mat.mData[1*N+2] = other.mData[0*N+2]*mData[1*N+0] + other.mData[1*N+2]*mData[1*N+1] + other.mData[2*N+2]*mData[1*N+2] + other.mData[3*N+2]*mData[1*N+3];
    mat.mData[2*N+2] = other.mData[0*N+2]*mData[2*N+0] + other.mData[1*N+2]*mData[2*N+1] + other.mData[2*N+2]*mData[2*N+2] + other.mData[3*N+2]*mData[2*N+3];
    mat.mData[3*N+2] = other.mData[0*N+2]*mData[3*N+0] + other.mData[1*N+2]*mData[3*N+1] + other.mData[2*N+2]*mData[3*N+2] + other.mData[3*N+2]*mData[3*N+3];

    mat.mData[0*N+3] = other.mData[0*N+3]*mData[0*N+0] + other.mData[1*N+3]*mData[0*N+1] + other.mData[2*N+3]*mData[0*N+2] + other.mData[3*N+3]*mData[0*N+3];
    mat.mData[1*N+3] = other.mData[0*N+3]*mData[1*N+0] + other.mData[1*N+3]*mData[1*N+1] + other.mData[2*N+3]*mData[1*N+2] + other.mData[3*N+3]*mData[1*N+3];
    mat.mData[2*N+3] = other.mData[0*N+3]*mData[2*N+0] + other.mData[1*N+3]*mData[2*N+1] + other.mData[2*N+3]*mData[2*N+2] + other.mData[3*N+3]*mData[2*N+3];
    mat.mData[3*N+3] = other.mData[0*N+3]*mData[3*N+0] + other.mData[1*N+3]*mData[3*N+1] + other.mData[2*N+3]*mData[3*N+2] + other.mData[3*N+3]*mData[3*N+3];
    return mat;
}

Vec3 Math::Mat4::operator*(const Vec3 &other) const
{
    Vec3 vec;
    vec[0] = mData[0*N+0]*other[0] + mData[0*N+1]*other[1] + mData[0*N+2]*other[2] + mData[0*N+3];
    vec[1] = mData[1*N+0]*other[0] + mData[1*N+1]*other[1] + mData[1*N+2]*other[2] + mData[1*N+3];
    vec[2] = mData[2*N+0]*other[0] + mData[2*N+1]*other[1] + mData[2*N+2]*other[2] + mData[2*N+3];
    const float w = mData[3*N+0]*other[0] + mData[3*N+1]*other[1] + mData[3*N+2]*other[2] + mData[3*N+3];
    vec /= w;
    return vec;
}


