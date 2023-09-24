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
    move(-2, 0, -2);
    matViewport.viewport(0, 0, sz.width(), sz.height());
    matProjection.perspective((float)sz.width() / (float)sz.height(), 120, 0.1, 100.);
    //matView.view(camera);
}

void Plotter::setData(QVector<Math::Vec3> data, QVector<QVector<int>> indexes)
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


    plot();
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

    plot();
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
    plot();
}

void Plotter::zoom(float factor)
{
    static float f = 1.0;
    f *= factor;
    matScale.scale(f, f, f);
    plot();
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

        if ((abs(z) > 1 || (x < 0) || (x > sz.width() - 1) || (y < 0) || (y > sz.height() - 1)) &&
            (abs(z2) > 1 || (x2 < 0) || (x2 > sz.width() - 1) || (y2 < 0) || (y2 > sz.height() - 1))) {
            continue;
        }
        // check if out of scope
        if (!clipCohenSuther(x, y, x2, y2, 0, 0, sz.width() - 1, sz.height() - 1)) {
            continue;
        }
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

void Plotter::drawTriangles(QVector<Math::Vec3> trData)
{

    for (const auto &p : qAsConst(polygonsFiltered)) {
        const auto &a = trData[p.indexes[0]];
        const auto &b = trData[p.indexes[1]];
        const auto &c = trData[p.indexes[2]];


        if ((abs(a[2]) > 1 || (a[0] < 0) || (a[0] > sz.width() - 1) || (a[1] < 0) || (a[1] > sz.height() - 1)) &&
            (abs(b[2]) > 1 || (b[0] < 0) || (b[0] > sz.width() - 1) || (b[1] < 0) || (b[1] > sz.height() - 1)) &&
            (abs(c[2]) > 1 || (c[0] < 0) || (c[0] > sz.width() - 1) || (c[1] < 0) || (c[1] > sz.height() - 1))) {
            continue;
        }
 //        if ((abs(a[2]) > 1) &&
 //            (abs(b[2]) > 1) &&
 //            (abs(c[2]) > 1)) {
 //            continue;
 //        }
        // get plane canonical
        // const auto nrml = Math::Vec3::cross(a-b, a-c).normalized();
        // get x and y max min
        int xmin = round(a[0]), xmax = round(b[0]), ymin = round(a[1]), ymax = round(b[1]), x = round(c[0]), y = round(c[1]);
        {
            if (xmax < xmin) {
                std::swap(xmax, xmin);
            }
            if (x > xmax) {
                std::swap(xmax, x);
            } else if (x < xmin) {
                std::swap(xmin, x);
            }
            if (xmin < 0) {
                xmin = 0;
            }
            xmin = std::clamp(xmin, 0, sz.width() - 1);
            xmax = std::clamp(xmax, 0, sz.width() - 1);
            if (ymax < ymin) {
                std::swap(ymax, ymin);
            }
            if (y > ymax) {
                std::swap(ymax, y);
            } else if (y < ymin) {
                std::swap(ymin, y);
            }
            ymin = std::clamp(ymin, 0, sz.height() - 1);
            ymax = std::clamp(ymax, 0, sz.height() - 1);
        }
        //
        for (int i = xmin; i < xmax; i++) {
            for (int j = ymin; j < ymax; j++) {
                float u, v, t;
                if (rayTriangleIntersect(Math::Vec3(i, j, -2).normalized(),
                                         Math::Vec3(i, j, 2).normalized(),
                                         a, b, c, t, u, v)) {
                    const float z = u * a[2] + v * b[2] + (1 - u - v) * c[2];
                    const int zindex = i + j * sz.width(); // TODO simplify
                    if (z < zbuffer.at(zindex)) {
                        zbuffer[zindex] = z;
                        backbuffer.setPixelColor(i, j, p.color);
                    }
                }
            }
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
    const Math::Mat4 wmat = matTranslate * matRotate * matScale; // to world cords
    const Math::Mat4 mat = matViewport * matProjection * camera->view() * wmat;
    // convert points
    QVector<Math::Vec3> trData(data);
    float minz = 100000, maxz = -100000;
    for (auto &p : trData) {
        p = mat * p; // todo remove assignment
        minz = std::min(p[2], minz);
        maxz = std::max(p[2], maxz);
    }
    // calculate colors and camera-dots
    const Math::Vec3 campos = camera->pos();
    polygonsFiltered.clear();
    // const Math::Vec3 light {}; // TODO
    for (const auto &p : qAsConst(polygons)) {
        Math::Vec3 nrml = matRotate * p.normal; // preserve normalization
        Math::Vec3 orgn = wmat * p.origin;

        const float dot = Math::Vec3::dot(nrml, (orgn - campos).normalized());
        if (dot >= 0) {
            Polygon copyP = p;
            copyP.color = QColor::fromHsvF(wireframeClr.hsvHueF(), 1, std::clamp(abs(dot), 0.f, 1.f));
            polygonsFiltered.push_back(copyP);
        }
    }
    qInfo() << "minz" << minz << "maxz" << maxz;
    // draw wireframe
    //drawLines(std::move(trData)/*, std::move(trNormals), std::move(trNormalOrigins)*/);
    drawTrianglesNew(std::move(trData));
    // notify about buffer change
    emit plotChanged(backbuffer, t.elapsed());
}

bool Plotter::clipCohenSuther(float &x1, float &y1, float &x2, float &y2,
                     float rx1, float ry1, float rx2, float ry2) {
    static auto computeCode = [rx1, ry1, rx2, ry2](float x, float y) {
        unsigned int code = INSIDE;
        if (x < rx1)
            code |= LEFT;
        else if (x > rx2)
            code |= RIGHT;
        if (y < ry1)
            code |= BOTTOM;
        else if (y > ry2)
            code |= TOP;
        return code;
    };
    auto code1 = computeCode(x1, y1);
    auto code2 = computeCode(x2, y2);
    //
    bool result = false;
    while (true) {
        if ((code1 == 0) && (code2 == 0)) {
            // If both endpoints lie within rectangle
            result = true;
            break;
        }
        else if (code1 & code2) {
            // If both endpoints are outside rectangle,
            // in same region
            break;
        }
        else {
            // Some segment of line lies within the
            // rectangle
            int code_out;
            float x, y;

            // At least one endpoint is outside the
            // rectangle, pick it.
            if (code1 != 0)
                code_out = code1;
            else
                code_out = code2;

            // Find intersection point;
            // using formulas y = y1 + slope * (x - x1),
            // x = x1 + (1 / slope) * (y - y1)
            if (code_out & TOP) {
                // point is above the clip rectangle
                x = x1 + (x2 - x1) * (ry2 - y1) / (y2 - y1);
                y = ry2;
            }
            else if (code_out & BOTTOM) {
                // point is below the rectangle
                x = x1 + (x2 - x1) * (ry1 - y1) / (y2 - y1);
                y = ry1;
            }
            else if (code_out & RIGHT) {
                // point is to the right of rectangle
                y = y1 + (y2 - y1) * (rx2 - x1) / (x2 - x1);
                x = rx2;
            }
            else if (code_out & LEFT) {
                // point is to the left of rectangle
                y = y1 + (y2 - y1) * (rx1 - x1) / (x2 - x1);
                x = rx1;
            }

            // Now intersection point x, y is found
            // We replace point outside rectangle
            // by intersection point
            if (code_out == code1) {
                x1 = x;
                y1 = y;
                code1 = computeCode(x1, y1);
            }
            else {
                x2 = x;
                y2 = y;
                code2 = computeCode(x2, y2);
            }
        }
    }
    return result;
}
