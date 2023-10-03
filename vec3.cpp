#include "vec3.h"

#include <algorithm>
#include <assert.h>

using namespace Math;

Vec3::Vec3()
{

}

Vec3::Vec3(float x, float y, float z)
    : mData{x, y, z}
{

}

float Vec3::dot(const Vec3 &a, const Vec3 &b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vec3 Vec3::cross(const Vec3 &a, const Vec3 &b)
{
    return {a[1]*b[2] - b[1]*a[2],
            -a[0]*b[2] + b[0]*a[2],
            a[0]*b[1] - b[0]*a[1]};
}

float Vec3::x() const
{
    return mData[0];
}
float Vec3::y() const
{
    return mData[1];
}
float Vec3::z() const
{
    return mData[2];
}

float Vec3::len() const
{
    return sqrt(mData[0]*mData[0] + mData[1]*mData[1] + mData[2]*mData[2]);
}

float Vec3::w() const
{
    static_assert(N >= 3 && "Vec len must be >= 3 to use w");
    return mData[3];
}

Vec3 Vec3::normalized() const
{
    Vec3 temp{*this};
    const float l = len();
    temp[0] /= l;
    temp[1] /= l;
    temp[2] /= l;
    return temp;
}

Vec3 Vec3::operator+(const Vec3 &other) const
{
    Vec3 res;
    std::transform(mData.cbegin(), mData.cend(), other.mData.cbegin(), res.mData.begin(), std::plus<>{});
    return res;
}
Vec3 Vec3::operator-(const Vec3 &other) const
{
    Vec3 res;
    std::transform(mData.cbegin(), mData.cend(), other.mData.cbegin(), res.mData.begin(), std::minus<>{});
    return res;
}

Vec3 Vec3::operator*(float value) const
{
    Vec3 v = *this;
    return v*=value;
}

Vec3 Vec3::operator/(float value) const
{
    Vec3 v = *this;
    return v/=value;
}

Vec3 &Vec3::operator*=(float value)
{
    std::for_each(mData.begin(), mData.end(), [value](float &v){v *= value;});
    return *this;
}

Vec3 &Vec3::operator/=(float value)
{
    std::for_each(mData.begin(), mData.end(), [value](float &v){v /= value;});
    return *this;
}

Vec3 &Vec3::operator+=(const Vec3 &other)
{
    std::transform(other.mData.cbegin(), other.mData.cend(), mData.cbegin(), mData.begin(), std::plus<>{});
    return *this;
}

float &Vec3::operator[](size_t i)
{
    assert((i < N) && "vector index can not be > 3");
    return mData[i];
}

float Vec3::operator[](size_t i) const
{
    assert((i < N) && "matrix index can not be > 3");
    return mData[i];
}
