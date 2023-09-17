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
    matProjection.perspective(1.2, 120, 0.1, 100.);
    //matView.view(camera);
}

void Plotter::setData(QVector<Math::Vec3> data, QVector<QPair<size_t, size_t> > indexes)
{
    this->data.clear();
    this->data = data;

    this->indexes.clear();
    this->indexes = indexes;

    plot();
}

void Plotter::rotate(double dx, double dy, double dz)
{
    static double x_rot = 0.0;
    static double y_rot = 0.0;
    static double z_rot = 0.0;
    x_rot += dx;
    y_rot += dy;
    z_rot += dz;
    Math::Mat4 t1, t2, t3;
    t1.rotateX(x_rot); t2.rotateY(y_rot);
    t3.rotateZ(z_rot);
    matRotate = t1*t2*t3;

    plot();
}

void Plotter::move(double dx, double dy, double dz)
{
    static double x_mov = 0.0;
    static double y_mov = 0.0;
    static double z_mov = 0.0;
    x_mov += dx;
    y_mov += dy;
    z_mov += dz;
    matTranslate.translate(x_mov, y_mov, z_mov);
    plot();
}

void Plotter::zoom(double factor)
{
    static double f = 1.0;
    f *= factor;
    matScale.scale(f, f, f);
    plot();
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
    zbuffer.fill(std::numeric_limits<double>::max());
    // get transform matrix
    // matView = camera->view();
    //qInfo() << camera->view() * matTranslate * matRotate * matScale;
    //qInfo() << matProjection;
    //qInfo() << matProjection * camera->view() * matTranslate * matRotate * matScale;
    const Math::Mat4 mat = matViewport * matProjection * camera->view() * matTranslate * matRotate * matScale;
    // convert points
    QVector<Math::Vec3> trData(data);
    // TEMP
    double minZ = 100000, maxZ = -100000;
    for (auto &p : trData) {
        p = mat * p; // todo remove assignment
        //if (abs(p[2]) < 1){
        //p *= 1 / p[3]; // WHY???
        //p[2] = z; // preserve z
        minZ = std::min(minZ, p[2]);
        maxZ = std::max(maxZ, p[2]);
        //}
    }
    qInfo() << "minZ" << minZ << "maxZ" << maxZ;
    // draw wireframe
    for (const auto &ids : qAsConst(indexes)) {
        const auto &a = trData[ids.first];
        const auto &b = trData[ids.second];

        if ((abs(a[2]) > 1 && abs(b[2]) > 1)) {
            continue;
        }
        // DDA-line
        double x = a[0];
        double y = a[1];
        double z = a[2];
        double x2 = b[0];
        double y2 = b[1];
        double z2 = b[2];

        if ((abs(z) > 1 || (x < 0) || (x > sz.width() - 1) || (y < 0) || (y > sz.height() - 1)) &&
            (abs(z2) > 1 || (x2 < 0) || (x2 > sz.width() - 1) || (y2 < 0) || (y2 > sz.height() - 1))) {
            continue;
        }
        // check if out of scope
        if (!clipCohenSuther(x, y, x2, y2, 0, 0, sz.width() - 1, sz.height() - 1)) {
            continue;
        }
        //
        const double w = x2 - x;
        const double h = y2 - y;
        const double l = fmax(abs(w), abs(h));
        const double dx = w / l;
        const double dy = h / l;
        //
        for (size_t i = l + 1; i > 0; i--) {
            // Draw pixel algorithm
            const int intX = round(x), intY = round(y);
            const int zindex = intX + intY * sz.width();
            // get z
            //if (zbuffer.at(zindex) < p[2] && p[2] > 0) {
                //zbuffer[zindex] = z;
                backbuffer.setPixelColor(intX, intY, wireframeClr);
            //}
            x+= dx;
            y+= dy;
        }
    }
    // notify about buffer change
    emit plotChanged(backbuffer, t.elapsed());
}

bool Plotter::clipCohenSuther(double &x1, double &y1, double &x2, double &y2,
                     double rx1, double ry1, double rx2, double ry2) {
    static auto computeCode = [rx1, ry1, rx2, ry2](double x, double y) {
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
            double x, y;

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
