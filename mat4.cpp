#include "mat4.h"

#include <algorithm>
#include <numbers>
#include <cmath>
#include <assert.h>

using namespace Math;

Mat4::Mat4()
{

}

//Mat4::Mat4(double m00, double m01, double m02, double m03, double m10, double m11, double m12, double m13, double m20, double m21, double m22, double m23, double m30, double m31, double m32, double m33)
//{
//    mData[0*N+0] = m00;
//}

Mat4::Mat4(std::array<double, N*N> data)
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

void Mat4::translate(double x, double y, double z)
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

void Mat4::scale(double x, double y, double z)
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

void Mat4::rotateX(double x)
{
    const double s = sin(x * (std::numbers::pi / 180.0));
    const double c = cos(x * (std::numbers::pi / 180.0));
    mData[0*N + 0] = 1;
    mData[1*N + 1] = c;
    mData[2*N + 2] = c;
    mData[1*N + 2] = -s;
    mData[2*N + 1] = s;
    mData[3*N + 3] = 1;
}

void Mat4::rotateY(double y)
{
    const double s = sin(y * (std::numbers::pi / 180.0));
    const double c = cos(y * (std::numbers::pi / 180.0));
    mData[0*N + 0] = c;
    mData[1*N + 1] = 1;
    mData[2*N + 2] = c;
    mData[0*N + 2] = s;
    mData[2*N + 0] = -s;
    mData[3*N + 3] = 1;
}

void Mat4::rotateZ(double z)
{
    const double s = sin(z * (std::numbers::pi / 180.0));
    const double c = cos(z * (std::numbers::pi / 180.0));
    mData[0*N + 0] = c;
    mData[1*N + 1] = c;
    mData[2*N + 2] = 1;
    mData[0*N + 1] = -s;
    mData[1*N + 0] = s;
    mData[3*N + 3] = 1;
}

void Mat4::perspective(double aspect, double fov, double zNear, double zFar)
{
    //
    const double temp2 = zNear - zFar;
    const double temp = 1.0 / tan(fov * (std::numbers::pi / 360.0));
    // initialize as identity matrix
    mData[0*N + 0] = temp * aspect;
    mData[1*N + 1] = temp;
    mData[2*N + 2] = zFar / temp2;
    mData[2*N + 3] = zNear * zFar / temp2;
    mData[3*N + 2] = -1;
}

void Mat4::viewport(double x, double y, double w, double h)
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

Mat4 &Mat4::operator+=(double value)
{
    std::for_each(mData.begin(), mData.end(), [value](double &v){v += value;});
    return *this;
}

Mat4 &Mat4::operator*=(double value)
{
    std::for_each(mData.begin(), mData.end(), [value](double &v){v *= value;});
    return *this;
}

Mat4 Mat4::operator+(const Mat4 &other) const
{
    Mat4 mat;
    return mat += *this;
}

Mat4 Mat4::operator+(double value) const
{
    Mat4 mat;
    return mat += value;
}

Mat4 Mat4::operator*(double value) const
{
    Mat4 mat;
    return mat *= value;
}

double &Mat4::operator[](size_t i)
{
    assert((i < N*N) && "matrix index can not be > 15");
    return mData[i];
}

double Mat4::operator[](size_t i) const
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
    return vec;
}


