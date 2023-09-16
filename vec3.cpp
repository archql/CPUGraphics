#include "vec3.h"

#include <algorithm>
#include <assert.h>

using namespace Math;

Vec3::Vec3()
{

}

Vec3::Vec3(double x, double y, double z)
    : mData{x, y, z}
{

}

double Vec3::dot(const Vec3 &a, const Vec3 &b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vec3 Vec3::cross(const Vec3 &a, const Vec3 &b)
{
    return {a[1]*b[2] - b[1]*a[2],
            -a[0]*b[2] + b[0]*a[2],
            a[0]*b[1] - b[0]*a[1]};
}

double Vec3::x() const
{
    return mData[0];
}
double Vec3::y() const
{
    return mData[1];
}
double Vec3::z() const
{
    return mData[2];
}

Vec3 Vec3::normalized() const
{
    Vec3 temp{*this};
    const double len = sqrt(mData[0]*mData[0] + mData[1]*mData[1] + mData[2]*mData[2]);
    temp[0] /= len;
    temp[1] /= len;
    temp[2] /= len;
    return temp;
}

Vec3 Vec3::operator+(const Vec3 &other) const
{
    Vec3 res;
    std::transform(other.mData.begin(), other.mData.end(), mData.begin(), res.mData.begin(), std::plus<>{});
    return res;
}
Vec3 Vec3::operator-(const Vec3 &other) const
{
    Vec3 res;
    std::transform(other.mData.begin(), other.mData.end(), mData.begin(), res.mData.begin(), std::minus<>{});
    return res;
}

Vec3 &Vec3::operator*=(double value)
{
    std::for_each(mData.begin(), mData.end(), [value](double &v){v *= value;});
    return *this;
}

double &Vec3::operator[](size_t i)
{
    assert((i < N) && "vector index can not be > 3");
    return mData[i];
}

double Vec3::operator[](size_t i) const
{
    assert((i < N) && "matrix index can not be > 3");
    return mData[i];
}
