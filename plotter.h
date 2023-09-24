#ifndef PLOTTER_H
#define PLOTTER_H

#include "camera.h"
#include "mat4.h"

#include <QFile>
#include <QImage>
#include <QObject>
#include <QVector>

class Polygon {
public:
    QVector<int> indexes;
    QColor color;
    Math::Vec3 normal;
    Math::Vec3 origin;
};


class Slope
{
    float begin, step;
public:
    Slope() {}
    Slope(float from, float to, int num_steps)
    {
        float inv_step = 1.f / num_steps;
        begin = from;                   // Begin here
        step  = (to - from) * inv_step; // Stepsize = (end-begin) / num_steps
    }
    float get() const { return begin; }
    void advance()    { begin += step; }
};


class Plotter : public QObject
{
    Q_OBJECT

protected:
    static constexpr unsigned int INSIDE = 0;
    static constexpr unsigned int LEFT =   0b0001;
    static constexpr unsigned int RIGHT =  0b0010;
    static constexpr unsigned int BOTTOM = 0b0100;
    static constexpr unsigned int TOP =    0b1000;
public:
    explicit Plotter(QSize sz, QObject *parent = nullptr);

public:
    // TODO move to sep file
    bool loadFromObj(QFile objFile);
    void setData(QVector<Math::Vec3> data, QVector<QVector<int>> indexes);
    void rotate(float dx, float dy, float dz = 0.0);
    void move(float dx, float dy, float dz);
    void zoom(float factor);

public:
    SharedCamera getCamera() const {return camera;};

protected:
    void drawLines(QVector<Math::Vec3> trData);
    void drawTriangles(QVector<Math::Vec3> trData);

protected:
    bool clipCohenSuther(float &x1, float &y1, float &x2, float &y2,
                         float rx1, float ry1, float rx2, float ry2);
    bool edgeFunction(float ax, float ay, float bx, float by, float cx, float cy) const
    {
        return ((cx - ax) * (by - ay) - (cy - ay) * (bx - ax) >= 0);
    }

    void Plot(int x, int y, float z, QColor color) {
        //
        if (x < 0 || x >= sz.width() || y < 0  || y >= sz.height()) return;
        // Draw pixel algorithm
        const int zindex = x + y * sz.width();
        // get z
        if (z < zbuffer.at(zindex) && abs(z) < 1) {
            zbuffer[zindex] = z;
            backbuffer.setPixelColor(x, y, color);
        }
    }

    template<bool CULLING = true>
    bool rayTriangleIntersect(
        const Math::Vec3 &orig, const Math::Vec3 &dir,
        const Math::Vec3 &a, const Math::Vec3 &b, const Math::Vec3 &c,
        float &t, float &u, float &v)
    {
        static constexpr float kEpsilon = 0.00001f;
        const Math::Vec3 ab = b - a;
        const Math::Vec3 ac = c - a;
        const Math::Vec3 pvec = Math::Vec3::cross(dir, ac);
        const float det = Math::Vec3::dot(ab, pvec);

        if constexpr (CULLING) {
            // if the determinant is negative, the triangle is 'back facing'
            // if the determinant is close to 0, the ray misses the triangle
            if (det < kEpsilon) return false;
        }
        else
        {
            // ray and triangle are parallel if det is close to 0
            if (fabs(det) < kEpsilon) return false;
        }

        const float invDet = 1 / det;

        const Math::Vec3 tvec = orig - a;
        u = Math::Vec3::dot(tvec, pvec) * invDet;
        if (u < 0 || u > 1) return false;

        const Math::Vec3 qvec = Math::Vec3::cross(tvec, ab);
        v = Math::Vec3::dot(dir, qvec) * invDet;
        if (v < 0 || u + v > 1) return false;

        t = Math::Vec3::dot(ac, qvec) * invDet;

        return true; // this ray hits the triangle
    }

    using SlopeData = Slope;// std::array<Slope, 2>;
    SlopeData makeSlope(const Math::Vec3 *from, const Math::Vec3 *to, int num_steps ) {
        SlopeData result;
        // X coords
        //float beginZ = (*from)[2], endZ = (*to)[2];
        // num of steps = num of scanlines
        result = Slope{ (*from)[0], (*to)[0], num_steps };
        //result[1] = Slope{ beginZ, endZ, num_steps };
        return result;
    }
    void drawScanLine(float y, SlopeData &left, SlopeData &right, const QColor &color) {
        int x = left.get(), endx = right.get();
        for (; x < endx; ++x) {
            Plot(x, y, 0.0, color);
        }
        left.advance();
        right.advance();
        //for (auto &slope : left) slope.advance();
        //for (auto &slope : right) slope.advance();
    }
    // + color
    void rasterizeTriangle(const Math::Vec3 *p0, const Math::Vec3 *p1, const Math::Vec3 *p2, const QColor &color)
    {
        // top-bottom rasterization
        auto [x0, y0, x1, y1, x2, y2] = std::tuple(p0->x(), p0->y(), p1->x(), p1->y(), p2->x(), p2->y());
        // order points by Y cordinate
        if (std::tie(y1, x1) < std::tie(y0, x0)) {
            std::swap(x0, x1); std::swap(y0, y1);
            std::swap(p0, p1); //
        }
        if (std::tie(y2, x2) < std::tie(y0, x0)) {
            std::swap(x0, x2); std::swap(y0, y2);
            std::swap(p0, p2); //
        }
        if (std::tie(y2, x2) < std::tie(y1, x1)) {
            std::swap(x1, x2); std::swap(y1, y2);
            std::swap(p1, p2); //
        }
        // Return if it is nothing to draw (no area)
        if (int(y0) == int(y2)) return;

        // determine whether the short side ison the left or on the right
        bool shortside = (y1 - y0) * (x2 - x0) < (x1 - x0) * (y2 - y0);

        // create 2 slopes: p0 - p1 (short) and p0 - p2 (long)
        SlopeData sides[2];
        sides[!shortside] = makeSlope(p0, p2, y2 - y0); // slope for the long side

        // rasterization loop
        for (auto y = y0, endy = y0; ; ++y) {
            if (y >= endy) {
                // if y of p2 reached, triangle is complete
                if (y >= y2) break;
                // recalculate slope for shortside again
                if (y < y1) {
                    sides[shortside] = makeSlope(p0, p1, y1 - y0);
                    endy = y1;
                } else {
                    sides[shortside] = makeSlope(p1, p2, y2 - y1);
                    endy = y2;
                }
            }
            drawScanLine(y, sides[0], sides[1], color);
        }
    }
    void drawTrianglesNew(QVector<Math::Vec3> trData)
    {
        for (const auto &p : qAsConst(polygonsFiltered)) {
            const auto &a = trData[p.indexes[0]];
            const auto &b = trData[p.indexes[1]];
            const auto &c = trData[p.indexes[2]];

            rasterizeTriangle(&a, &b, &c, p.color);
        }
    }

public Q_SLOTS:
    void plot();

Q_SIGNALS:
    void plotChanged(QImage p, qint64 t);
    void cleanup();

protected:
    QSize sz;
    QImage backbuffer;
    QVector<float> zbuffer;
    QColor clearClr;
    QColor wireframeClr;

protected:
    SharedCamera camera;

protected:
    QVector<Math::Vec3> data;
    QVector<Polygon> polygons;
    QVector<Polygon> polygonsFiltered;

    QVector<QVector<int>> indexes;

    Math::Mat4 matScale;
    Math::Mat4 matRotate;
    Math::Mat4 matTranslate;
    Math::Mat4 matView;
    Math::Mat4 matViewport;
    Math::Mat4 matProjection;
};

#endif // PLOTTER_H
