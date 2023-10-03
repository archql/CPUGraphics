#ifndef PLOTTER_H
#define PLOTTER_H

#include "camera.h"
#include "mat4.h"
#include "plane.h"

#include <QFile>
#include <QImage>
#include <QObject>
#include <QVector>

class Polygon {
public:
    // defines indexes of associated vertices
    QVector<int> indexes;
    // defines color
    QColor color;
    Math::Vec3 normal;
    Math::Vec3 origin;
};

// already transformed and ready to be drawn
class Triangle {

public:
    // defines transformed vertices
    Math::Vec3 p0, p1, p2;
    // defines color
    QColor color;

public:
    Triangle(const Math::Vec3 &p0,const Math::Vec3 &p1, const Math::Vec3 &p2, const QColor &color)
        : p0(p0)
        , p1(p1)
        , p2(p2)
        , color(color)
    { }
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

    using SlopeData = std::array<Slope, 2>;
    SlopeData makeSlope(const Math::Vec3 *from, const Math::Vec3 *to, int num_steps ) {
        SlopeData result;
        // X coords
        float xbegin = (*from)[0], xend = (*to)[0];
        // num of steps = num of scanlines
        result[0] = Slope{ xbegin, xend, num_steps };

        // For the Z coordinate, use the inverted value.
        float zbegin = 1.f / (*from)[2], zend = 1.f / (*to)[2];
        result[1] = Slope( zbegin, zend, num_steps );

        return result;
    }
    void drawScanLine(float y, SlopeData &left, SlopeData &right, const QColor &color) {
        // Number of steps = number of pixels on this scanline = endx-x
        int x = left[0].get(), endx = right[0].get();

        // holds all point props (now only inverted z cord)
        Slope props; // std::array<Slope, Size-2>
        //for(unsigned p=0; p<Size-2; ++p)
        //{
            props = Slope( left[1].get(), right[1].get(), endx-x );
        //}

        for (; x < endx; ++x) {
            float z = 1.f / props.get(); // (props[0]) Invert the inverted z-coordinate, producing real z coordinate
            Plot(x, y, z, color);
            // After each pixel, update the props by their step-sizes
            props.advance();
        }
        // After the scanline is drawn, update the X coordinate and props on both sides
        for(auto& slope: left) slope.advance();
        for(auto& slope: right) slope.advance();
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


    void drawTrianglesNew()
    {
        for (const auto &tr : qAsConst(triangles)) {
            const auto &a = tr.p0;
            const auto &b = tr.p1;
            const auto &c = tr.p2;
            // clip
            if ((abs(a[2]) > 1 || (a[0] < 0) || (a[0] > sz.width() - 1) || (a[1] < 0) || (a[1] > sz.height() - 1)) &&
                (abs(b[2]) > 1 || (b[0] < 0) || (b[0] > sz.width() - 1) || (b[1] < 0) || (b[1] > sz.height() - 1)) &&
                (abs(c[2]) > 1 || (c[0] < 0) || (c[0] > sz.width() - 1) || (c[1] < 0) || (c[1] > sz.height() - 1))) {
                continue;
            }

            rasterizeTriangle(&a, &b, &c, tr.color);
        }
    }

    void ClipPolygon(const Math::Plane& p, auto& points)
        requires std::ranges::forward_range<decltype(points)>
    {
        bool keepfirst = true;

        // Process each edge of the polygon (line segment between two successive points)
        for(auto current = points.begin(); current != points.end(); )
        {
            auto next = std::next(current);
            if(next == points.end()) { next = points.begin(); }

            auto outside     = p.distanceTo(*current);
            auto outsidenext = p.distanceTo(*next);

            // If this corner is not inside the plane, keep it
            bool keep = outside >= 0;
            if(current == points.begin()) { keepfirst=keep; keep=true; }

            // If this edge of the polygon _crosses_ the plane, generate an intersection point
            if((outside < 0 && outsidenext > 0)
                || (outside > 0 && outsidenext < 0))
            {
                auto factor = outside / (outside - outsidenext);

                // Create a new point b between *current and *next like this: current + (next-current) * factor
                auto b = *current + ((*next - *current) * factor);

                if(keep) { current = std::next(points.insert(std::next(current), std::move(b))); }
                else     { *current = std::move(b); ++current; }
            }
            else
            {
                // Keep or delete the original vertex
                if(keep) ++current; else current = points.erase(current);
            }
        }
        if(!keepfirst) points.erase(points.begin());
    }

protected:
//    const QVector<Math::Plane> clippingPlanes = {
//        Math::Plane(Math::Vec3(0, 0, znear), Math::Vec3(1, 0, znear), Math::Vec3()),

//    };

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
    QVector<Triangle> triangles;

    QVector<QVector<int>> indexes;

    Math::Mat4 matScale;
    Math::Mat4 matRotate;
    Math::Mat4 matTranslate;
    Math::Mat4 matView;
    Math::Mat4 matViewport;
    Math::Mat4 matProjection;
};

#endif // PLOTTER_H
