#ifndef PLOTTER_H
#define PLOTTER_H

#include "camera.h"
#include "mat4.h"
#include "texinfo.h"
#include "plane.h"

#include <QFile>
#include <QImage>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVector>

#include <execution>

class Polygon {
public:
    // defines indexes of associated vertices
    QVector<int> indexes;
    // defines color
    QColor color;
    Math::Vec3 normal;
    Math::Vec3 origin;
};

struct Point {
    Math::Vec3 vertex;
    Math::Vec3 normal;
    Math::Vec3 color;
    Math::Vec3 pos;
    // cordinates
    Math::Vec3 tex;
    int texId;

    Point operator+(const Point &other) const
    {
        Point n = *this;
        n.vertex += other.vertex;
        n.normal += other.normal;
        n.color += other.color;
        n.pos += other.pos;
        n.tex += other.tex;
        return n;
    }
    Point operator-(const Point &other) const
    {
        Point n = *this;
        n.vertex -= other.vertex;
        n.normal -= other.normal;
        n.color -= other.color;
        n.pos -= other.pos;
        n.tex -= other.tex;
        return n;
    }
    Point operator*(float value) const
    {
        Point n = *this;
        n.vertex *= value;
        n.normal *= value;
        n.color *= value;
        n.pos *= value;
        n.tex *= value;
        return n;
    }
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

public:
    explicit Plotter(QSize sz, QObject *parent = nullptr);

public:
    void togglePause();

public:
    // TODO move to sep file
    bool loadFromObj(QFile objFile);
    void setData(QVector<Math::Vec3> data,
                 QVector<QVector<std::tuple<int, int, int>>> indexes,
                 QVector<Math::Vec3> normals,
                 QVector<Math::Vec3> colors,
                 QVector<Math::Vec3> textures,
                 QVector < int > texIDs,
                 QVector<TexInfo> texture);
    void rotate(float dx, float dy, float dz = 0.0);
    void move(float dx, float dy, float dz);
    void zoom(float factor);

public:
    SharedCamera getCamera() const {return camera;};

protected:
    void drawLines(QVector<Math::Vec3> trData);
    void drawTriangles(QVector<Math::Vec3> trData);

protected:

    QColor calcPhongColor(Math::Vec3 color,
                          Math::Vec3 normal,
                          Math::Vec3 pos,
                          Math::Vec3 tex,
                          int texId) {
        //qInfo() << "tex " << tex.z() << pos.z();
        auto &curTexBump = texture[texId].tBump;
        auto &curTexDiffuse = texture[texId].tDiffuse;
        auto &curTexNormal = texture[texId].tNormal;

        Math::Vec3 texClr = Math::Vec3(1, 1, 1);
        if (!curTexDiffuse.isNull()) {
            auto w = curTexDiffuse.width(), h = curTexDiffuse.height();
            auto tx = (int)((tex.x()) * (w)) % w;
            auto ty = ((8*h-1) + (int)((-tex.y()) * (h))) % h;
            texClr = curTexDiffuse.pixelColor(tx, ty);
        }
        float kSpecularT = kSpecular;
        if (!curTexBump.isNull()) {
            auto w = curTexBump.width(), h = curTexBump.height();
            auto tx = (int)((tex.x()) * (w)) % w;
            auto ty = ((8*h-1) + (int)((-tex.y()) * (h))) % h;
            kSpecularT = curTexBump.pixelColor(tx, ty).redF();
        }
        normal = normal.normalized();
        if (!curTexNormal.isNull()) {
            auto w = curTexNormal.width(), h = curTexNormal.height();
            auto tx = (int)((tex.x()) * (w)) % w;
            auto ty = ((8*h-1) + (int)((-tex.y()) * (h))) % h;
            auto n = Math::Vec3(curTexNormal.pixelColor(tx, ty));
            normal[0] += n.x() * 2 - 1.0f;
            normal[1] += n.y() * 2 - 1.0f;
            normal[2] += n.z() * 2 - 1.0f;
        }
        normal = normal.normalized();
        auto tmp2 = camera->pos() - pos;
        auto dst = std::max(tmp2.len2(), 0.01f);
        auto tmp = (tmp2).normalized(); // lightDir
        float diff = std::clamp(Math::Vec3::dot(tmp, normal), 0.f, 1.f); // normal
        float spec = pow(std::clamp(Math::Vec3::dot((tmp - (normal * (2 * diff))).normalized(), (pos - camera->pos()).normalized()), 0.f, 1.f), powSpecular);
        Math::Vec3 res = lAmbient*kAmbient +
                         lDiffuse*(kDiffuse * diff / dst) +
                         lSpecular*(kSpecularT * spec / dst);
        auto x = texClr.x() * color.x() * res.x();
        auto y = texClr.y() * color.y() * res.y();
        auto z = texClr.z() * color.z() * res.z();
        // tone mapping
//        x = x / (1+x);
//        y = y / (1+y); // формула Рейнхарда
//        z = z / (1+z);
        float a = 2.51f;
        float b = 0.03f;
        float c = 2.43f;
        float d = 0.59f;
        float e = 0.14f;
        x = std::clamp((x * (a*x + b)) / (x * (c * x + d) + e), 0.f, 1.f);
        y = std::clamp((y * (a*y + b)) / (y * (c * y + d) + e), 0.f, 1.f);
        z = std::clamp((z * (a*z + b)) / (z * (c * z + d) + e), 0.f, 1.f);
        // gamma correction
        x = pow(x, 1/ 2.2);
        y = pow(y, 1/ 2.2);
        z = pow(z, 1/ 2.2);
        return QColor(x * 255,
                      y * 255,
                      z * 255
                      );
//        return QColor(std::clamp(color.x(), 0.f, 1.f) * 255,
//                      std::clamp(color.y(), 0.f, 1.f) * 255,
//                      std::clamp(color.z(), 0.f, 1.f) * 255
//                      );
    }
    void plotPixel(int x, int y, float z, QColor color) {
        //
        // if (x < 0 || x >= sz.width() || y < 0  || y >= sz.height()) return;
        // Draw pixel algorithm
        const int zindex = x + y * sz.width();

        std::unique_lock l(mutexes.at(zindex));
        // get z
        if (z < zbuffer.at(zindex)) {
            zbuffer[zindex] = z;
            backbuffer.setPixelColor(x, y, color);
        }

    }

    void makeFrustrum(float znear, float zfar);

    static constexpr size_t slopeDataSz = 13;
    using SlopeData = std::array<Slope, slopeDataSz>;
    SlopeData makeSlope(const Point *from, const Point *to, int num_steps ) const {
        SlopeData result;
        // X coords
        float xbegin = from->vertex[0], xend = to->vertex[0];
        // num of steps = num of scanlines
        result[0] = Slope{ xbegin, xend, num_steps };

        // For the Z coordinate, use the inverted value.
        float zbegin = 1.f / from->vertex[2], zend = 1.f / to->vertex[2];
//        qInfo() << "zbegin " << from->vertex[2] << " zend" << to->vertex[2];
//        qInfo() << "izbegin" << zbegin << "izend" << zend;
//        qInfo() << "iv  x " << from->tex[0] << " y " << to->tex[0];
//        qInfo() << "iv  x " << from->tex[1] << " y " << to->tex[1];
//        qInfo() << "ivz x " << from->tex[0]*zbegin << " y " << to->tex[0]*zend;
//        qInfo() << "ivz x " << from->tex[1]*zbegin << " y " << to->tex[1]*zend;
//        qInfo() << "num_steps" << num_steps;
        //float zbegin = (*from)[2], zend = (*to)[2];
        result[1] = Slope( zbegin, zend, num_steps );
        // Normal
        float b = from->normal[0], e = to->normal[0];
        result[2] = Slope( b * zbegin, e * zend, num_steps );
        b = from->normal[1], e = to->normal[1];
        result[3] = Slope( b * zbegin, e * zend, num_steps );
        b = from->normal[2], e = to->normal[2];
        result[4] = Slope( b * zbegin, e * zend, num_steps );
        // Color
        b = from->color[0], e = to->color[0];
        result[5] = Slope( b * zbegin, e * zend, num_steps );
        b = from->color[1], e = to->color[1];
        result[6] = Slope( b * zbegin, e * zend, num_steps );
        b = from->color[2], e = to->color[2];
        result[7] = Slope( b * zbegin, e * zend, num_steps );
        // Pos
        b = from->pos[0], e = to->pos[0];
        result[8] = Slope( b * zbegin, e * zend, num_steps );
        b = from->pos[1], e = to->pos[1];
        result[9] = Slope( b * zbegin, e * zend, num_steps );
        b = from->pos[2], e = to->pos[2];
        result[10] = Slope( b * zbegin, e * zend, num_steps );
        // Tex
        b = from->tex[0], e = to->tex[0];
        result[11] = Slope( b * zbegin, e * zend, num_steps );
        b = from->tex[1], e = to->tex[1];
        result[12] = Slope( b * zbegin, e * zend, num_steps );
        return result;
    }
    void drawScanLine(float y, SlopeData &left, SlopeData &right, int texId = 0) {
        // Number of steps = number of pixels on this scanline = endx-x
        int x = ceil(left[0].get()), endx = ceil(right[0].get()); // TODO

        // holds all point props (now only inverted z cord)
        std::array<Slope, slopeDataSz - 1> props;
        for(unsigned p=0; p<slopeDataSz - 1; ++p)
        {
            props[p] = Slope( left[p + 1].get(), right[p + 1].get(), endx-x );
        }

        for (; x < endx; ++x) {
            float invz = props[0].get();
            float z = 1.f / invz; // (props[0]) Invert the inverted z-coordinate, producing real z coordinate
            //qInfo() << "a" << props[10].get() << props[11].get();
            //qInfo() << "b" << props[10].get()*z << props[11].get()*z;
            plotPixel(x, y, z, calcPhongColor(Math::Vec3{props[4].get()*z, props[5].get()*z, props[6].get()*z},
                                              Math::Vec3{props[1].get()*z, props[2].get()*z, props[3].get()*z},
                                              Math::Vec3{props[7].get()*z, props[8].get()*z, props[9].get()*z},
                                              Math::Vec3{props[10].get()*z, props[11].get()*z, 0}, texId));
            // After each pixel, update the props by their step-sizes
            for (auto &slope : props) slope.advance();
        }
        // After the scanline is drawn, update the X coordinate and props on both sides
        for(auto& slope: left) slope.advance();
        for(auto& slope: right) slope.advance();
        //for (auto &slope : left) slope.advance();
        //for (auto &slope : right) slope.advance();
    }
    // + color
    void rasterizeTriangle(const Point *p0, const Point *p1, const Point *p2)
    {
        // top-bottom rasterization
        auto [x0, y0, x1, y1, x2, y2] = std::tuple(
            p0->vertex.x(), p0->vertex.y(), p1->vertex.x(), p1->vertex.y(), p2->vertex.x(), p2->vertex.y());
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
        //y2++;
        y0 = ceil(y0);
        y1 = ceil(y1);
        y2 = ceil(y2);
        // Return if it is nothing to draw (no area)
        if (y0 == y2) return;

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
            drawScanLine(y, sides[0], sides[1], p0->texId); // TODO costil to store tex id
        }
    }

    void clipPolygon(const Math::Plane& p, auto &points) const
    {
        bool keepfirst = true;

        // Process each edge of the polygon (line segment between two successive points)
        for(auto current = points.begin(); current != points.end(); )
        {
            auto next = std::next(current);
            if(next == points.end()) { next = points.begin(); }

            auto outside     = p.distanceTo(current->vertex);
            auto outsidenext = p.distanceTo(next->vertex);

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
                if(keep)
                    ++current;
                else
                    current = points.erase(current);
            }
        }
        if(!keepfirst) points.erase(points.begin());
    }
    void tesselatePolygon(auto& points, auto && pushTriangle) const
        requires std::ranges::random_access_range<decltype(points)>
    {
        constexpr size_t limit = 16;
        bool too_many_corners = points.size() >= limit;

        // prev[] and next[] form a double-directional linked list
        unsigned char next[limit], prev[limit];
        for(size_t n=0; n<points.size() && n<limit; ++n)
        {
            next[n] = (n+1)==points.size() ? 0 : (n+1);
            prev[n] = n==0 ? points.size()-1 : (n-1);
        }
        for(size_t cur = 0, remain = points.size(); remain >= 3; --remain)
        {
            unsigned p1 = next[cur], p2 = next[p1];
            unsigned a = cur, b = p1, c = p2, era = cur;
            if(remain > 3 && !too_many_corners)
            {
                unsigned m1 = prev[cur], m2 = prev[m1];
                auto curx = points[cur].vertex.x();
                auto cury = points[cur].vertex.y();

                auto p1x = points[p1].vertex.x();
                auto p1y = points[p1].vertex.y();

                auto p2x = points[p2].vertex.x();
                auto p2y = points[p2].vertex.y();

                auto m1x = points[m1].vertex.x();
                auto m1y = points[m1].vertex.y();

                auto m2x = points[m2].vertex.x();
                auto m2y = points[m2].vertex.y();
                // Three possible tesselations:
                //     prev2-prev1-this (score3)
                //     prev1-this-next1 (score1)
                //     this-next1-next2 (score2)
                // Score indicates how long horizontal lines there are in this triangle
                auto score1 = (std::abs(curx-p1x) + std::abs(p1x-m1x) + std::abs(m1x-curx))
                              - (std::abs(cury-p1y) + std::abs(p1y-m1y) + std::abs(m1y-cury));
                auto score2 = (std::abs(curx-p1x) + std::abs(p1x-p2x) + std::abs(p2x-curx))
                              - (std::abs(cury-p1y) + std::abs(p1y-p2y) + std::abs(p2y-cury));
                auto score3 = (std::abs(curx-m2x) + std::abs(m2x-m1x) + std::abs(m1x-curx))
                              - (std::abs(cury-m2y) + std::abs(m2y-m1y) + std::abs(m1y-cury));
                if(score1 >= score2 && score1 >= score3)      { b = p1; c = m1; /* era = cur; */ }
                else if(score2 >= score1 && score2 >= score3) { /*b = p1; c = p2;*/ era = p1; }
                else                                          { b = m2; c = m1; era = m1; }
            }
        rest:
            pushTriangle(points[a],points[b],points[c]);
            if(too_many_corners)
            {
                b = c++;
                if(c >= remain) return;
                goto rest;
            }
            auto p = prev[era], n = next[era];
            next[p] = n;
            prev[n] = p;
            cur = n;
        }
    }

protected:
    QVector<Math::Plane> clippingPlanes;

public Q_SLOTS:
    void plot();

Q_SIGNALS:
    void plotChanged(QImage p, qint64 t);
    void cleanup();

protected:
    Math::Vec3 lightDir{0, 0, 1};

    Math::Vec3 lAmbient{0.1, 0.1, 0.1};
    Math::Vec3 lDiffuse{10, 10, 10};
    Math::Vec3 lSpecular{10, 10, 10};

    float kAmbient = 1.f;
    float kDiffuse = 1.f;
    float kSpecular = 1.f;
    float powSpecular = 64.f;

protected:
    QSize sz;
    QImage backbuffer;
    QVector<float> zbuffer;
    std::vector<std::mutex> mutexes;
    QColor clearClr;
    QColor wireframeClr;

protected:
    SharedCamera camera;

protected:
    QVector<Math::Vec3> data;
    QVector<Math::Vec3> normals;
    QVector<Polygon> polygons;
    QVector<Triangle> triangles;
    QVector<Math::Vec3> textures;

    QVector<QVector<std::tuple<int, int, int>>> indexes;
    QVector<Math::Vec3> colors;

    QVector<TexInfo> texture;
    QVector < int > texIDs;

    Math::Mat4 matScale;
    Math::Mat4 matRotate;
    Math::Mat4 matTranslate;
    Math::Mat4 matView;
    Math::Mat4 matViewport;
    Math::Mat4 matProjection;

    Math::Mat4 matUnProjection;

protected:
    QTimer *timer;
};

#endif // PLOTTER_H
