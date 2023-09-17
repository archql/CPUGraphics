#ifndef PLOTTER_H
#define PLOTTER_H

#include "camera.h"
#include "mat4.h"

#include <QFile>
#include <QImage>
#include <QObject>
#include <QVector>


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
    void setData(QVector<Math::Vec3> data, QVector<QPair<size_t, size_t>> indexes);
    void rotate(double dx, double dy, double dz = 0.0);
    void move(double dx, double dy, double dz);
    void zoom(double factor);

public:
    SharedCamera getCamera() const {return camera;};

protected:
    bool clipCohenSuther(double &x1, double &y1, double &x2, double &y2,
                         double rx1, double ry1, double rx2, double ry2);

public Q_SLOTS:
    void plot();

Q_SIGNALS:
    void plotChanged(QImage p, qint64 t);
    void cleanup();

protected:
    QSize sz;
    QImage backbuffer;
    QVector<double> zbuffer;
    QColor clearClr;
    QColor wireframeClr;

protected:
    SharedCamera camera;

protected:
    QVector<Math::Vec3> data;
    QVector<QPair<size_t, size_t>> indexes;

    Math::Mat4 matScale;
    Math::Mat4 matRotate;
    Math::Mat4 matTranslate;
    Math::Mat4 matView;
    Math::Mat4 matViewport;
    Math::Mat4 matProjection;
};

#endif // PLOTTER_H
