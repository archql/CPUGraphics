#include "plotter.h"

#include <QElapsedTimer>
#include <QDebug>

#include <cmath>

Plotter::Plotter(QSize sz, QObject *parent)
    : QObject{parent}
    , backbuffer(sz, QImage::Format_RGB32)
    , zbuffer(sz.height() * sz.width())
    , clearClr{Qt::black}
    , wireframeClr{"darkorange"}
    , camera{new Camera{0, 0, 0}}
{
    // cunning optimization???? (temp)
    //backbuffer.setColor(0, clearClr.rgb());
    //backbuffer.setColor(1, wireframeClr.rgb());
    this->sz = sz;
    //matScale.scale(0.3, 0.3, 0.3);
    zoom(0.3);
    matRotate.loadIdentity();
    //matTranslate.translate(2, 0, 0);
    move(2, 0, 2);
    matViewport.viewport(0, 0, sz.width(), sz.height());
    matProjection.perspective((float)sz.width() / (float)sz.height(), 120, 1, 100.);
    matUnProjection = matProjection.inversed();
    //matView.view(camera);
    //makeFrustrum();
    makeFrustrum(1, 100.); // uses matUnProjection

    timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, this, &Plotter::plot);
    togglePause();
}

void Plotter::togglePause()
{
    // im running in sep thread
    static bool paused = true;
    paused ^= 1;
    if (paused) {
        timer->stop();
    } else {
        timer->start(1000 / 60); // Start at 60 fps
    }
}

void Plotter::setData(QVector<Math::Vec3> data, QVector<QVector<int>> indexes, QVector<Math::Vec3> normals)
{
    this->data.clear();
    this->data = data;

    this->indexes.clear();
    this->indexes = indexes;

    // precompute normals for model
    this->polygons.clear();
    for (const auto &ids : qAsConst(indexes)) {
        // get plane normal
        const auto &a = data[ids[0]];
        const auto &b = data[ids[1]];
        const auto &c = data[ids[2]];
        //const auto normal = Math::Vec3::cross(a - b, a - c).normalized(); // it is normal
        //this->normals.append(normal);
        //this->normalOrigins.append((a + b + c) / 3.);
        this->polygons.append({ids,
                               wireframeClr,
                               Math::Vec3::cross(a - b, a - c).normalized(),
                               (a + b + c) / 3.
        });
    }
    //


    //plot();
}

void Plotter::rotate(float dx, float dy, float dz)
{
    static float x_rot = 0.0;
    static float y_rot = 0.0;
    static float z_rot = 0.0;
    x_rot += dx;
    y_rot += dy;
    z_rot += dz;
    Math::Mat4 t1, t2, t3;
    t1.rotateX(x_rot); t2.rotateY(y_rot);
    t3.rotateZ(z_rot);
    matRotate = t1*t2*t3;

    //plot();
}

void Plotter::move(float dx, float dy, float dz)
{
    static float x_mov = 0.0;
    static float y_mov = 0.0;
    static float z_mov = 0.0;
    x_mov += dx;
    y_mov += dy;
    z_mov += dz;
    matTranslate.translate(x_mov, y_mov, z_mov);
    //plot();
}

void Plotter::zoom(float factor)
{
    static float f = 1.0;
    f *= factor;
    matScale.scale(f, f, f);
    //plot();
}

void Plotter::drawLines(QVector<Math::Vec3> trData)
{
    for (const auto &ids : qAsConst(indexes)) {
        const auto &a = trData[ids[0]];
        const auto &b = trData[ids[1]];

        // DDA-line
        float x = a[0];
        float y = a[1];
        float z = a[2];
        float x2 = b[0];
        float y2 = b[1];
        float z2 = b[2];

//        if ((abs(z) > 1 || (x < 0) || (x > sz.width() - 1) || (y < 0) || (y > sz.height() - 1)) &&
//            (abs(z2) > 1 || (x2 < 0) || (x2 > sz.width() - 1) || (y2 < 0) || (y2 > sz.height() - 1))) {
//            continue;
//        }
        //
        const float w = x2 - x;
        const float h = y2 - y;
        const float d = z2 - z;
        const float l = fmax(abs(w), abs(h));
        const float dx = w / l;
        const float dy = h / l;
        const float dz = d / l;
        //
        for (size_t i = l + 1; i > 0; i--) {
            // Draw pixel algorithm
            const int intX = round(x), intY = round(y);
            const int zindex = intX + intY * sz.width();
            // get z
            if (z < zbuffer.at(zindex) && abs(z) < 1) {
                zbuffer[zindex] = z;
                backbuffer.setPixelColor(intX, intY, wireframeClr);
            }
            x += dx;
            y += dy;
            z += dz;
        }
    }
}


void Plotter::plot()
{
    //
    qDebug() << "plot!";
    //
    QElapsedTimer t;
    t.start();
    // clear backbuffer with clear color and zbuffer
    backbuffer.fill(clearClr);
    zbuffer.fill(std::numeric_limits<float>::max());
    // get transform matrix
    // matView = camera->view();
    //qInfo() << camera->view() * matTranslate * matRotate * matScale;
    //qInfo() << matProjection;
    //qInfo() << matProjection * camera->view() * matTranslate * matRotate * matScale;
    const Math::Mat4 world_mat = matTranslate * matRotate * matScale; // to world cords
    const Math::Mat4 cam_mat = camera->view() * world_mat;
    const Math::Mat4 proj_mat = matViewport * matProjection;
    // convert points to cam proj
    QVector<Math::Vec3> trData(data);
    for (auto &p : trData) {
        p = cam_mat.mul(p); // todo remove assignment
    }

    triangles.clear();
    for (const auto &p : qAsConst(polygons)) {
        // get polygon points
        QVector<Math::Vec3> points(p.indexes.size());
        std::transform(p.indexes.cbegin(), p.indexes.cend(), points.begin(), [&](int i){return trData[i];});

        // Discard polygons that are not facing the camera (back-face culling).
        const float dot = Math::Vec3::dot(Math::Vec3::cross(points[1], points[2]).normalized(), points[0]);
        if (dot > 1e-4f) continue;
        // color
        QColor color = QColor::fromHsvF(wireframeClr.hsvHueF(), 1, std::clamp(abs(dot), 0.f, 1.f));

        // Clip polygon
        for(auto& p: points)
        {
            auto x = matProjection.mulOrthoDiv(p);
            if (abs(x.x()) > 1 || abs(x.y()) > 1)
                continue;
        }
        for(const Math::Plane& plane : qAsConst(clippingPlanes)) clipPolygon(plane, points);
        // If the polygon is no longer a surface, don’t try to render it.
        if(points.size() < 3) continue;
        // Perspective-project remaining points
        for(auto& p: points)
        {
            p = proj_mat.mulOrthoDiv(p);
        }
        // Tesselate polygon
        tesselatePolygon(points, [&](const Math::Vec3 &a, const Math::Vec3 &b, const Math::Vec3 &c) {
            Triangle tr(a, b, c, color);
            triangles.push_back(tr);
        });
    }
    //qInfo() << "minz" << minz << "maxz" << maxz;
    // draw wireframe
    //drawLines(std::move(trData)/*, std::move(trNormals), std::move(trNormalOrigins)*/);
    drawTrianglesNew();
    // notify about buffer change
    emit plotChanged(backbuffer, t.elapsed());
}

void Plotter::makeFrustrum(float znear, float zfar)
{
    static constexpr float zany = -0.1f; // TODO mat * vec multiplication check!!!!
    // TODO why i div on zfar??
    znear *= 1.0001f;
    const std::vector<Math::Vec3> corners {
        {-0.5f/0.5001f, -0.5f/0.5001f, zany}, // WHY 0.50001???
        { 0.5f/0.5001f, -0.5f/0.5001f, zany},
        { 0.5f/0.5001f,  0.5f/0.5001f, zany},
        {-0.5f/0.5001f,  0.5f/0.5001f, zany}
    };
    // znear = near clipping plane distance, zany = arbitrary z value
    //clippingPlanes.clear();
    clippingPlanes.append( {{0,0,-znear}, {0,1,-znear}, {1,0,-znear}} );
    clippingPlanes.append( {{0,0,-zfar }, {1,0,-zfar }, {0,1,-zfar }} );
    // Iterate through all successive pairs of corner points (including last->first)
    for(auto begin = corners.begin(), end = corners.end(), current = begin; current != end; ++current)
    {
        auto next = std::next(current); if(next == end) next = begin;
        clippingPlanes.append({ matUnProjection.mul(*next), matUnProjection.mul(*current), {0,0,0}} );
    }
}
