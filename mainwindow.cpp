#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "objLoader.h"

#include <QPaintEvent>
#include <QPainter>
#include <QThread>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , drawtime(0)
    , verticescount(0)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Setup plotter
    plotter = new Plotter(QSize(1280, 640));
    // temp
    QVector < Math::Vec3 > vertices;
    QVector < QPair<size_t, size_t> > indices;
    QVector < Math::Vec3 > normals;
    if (loadOBJ(QFile("test.obj"), vertices, indices, normals))
    {
        qDebug() << "Data loaded";
        plotter->setData(vertices, indices);
        verticescount = vertices.size();
    }
    else
    {
        qDebug() << "Failed to load data, using custom";
//        plotter->setData({{0.0, 1.0, 1.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0},
//                          {1.0, 1.0, 1.0}, {1.0, 0.0, 1.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}},
//                         {{0, 1}, {1, 2}, {2, 3}, {3, 0},
//                          {4, 5}, {5, 6}, {6, 7}, {7, 4},
//                          {0, 4}, {1, 5}, {2, 6}, {3, 7}});
        plotter->setData({{-1.0, 1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0},
                          {1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, -1.0, -1.0}, {1.0, 1.0, -1.0}},
                         {{0, 1}, {1, 2}, {2, 3}, {3, 0},
                          {4, 5}, {5, 6}, {6, 7}, {7, 4},
                          {0, 4}, {1, 5}, {2, 6}, {3, 7}});
    }


    QThread *thread = new QThread;
    plotter->moveToThread(thread);

    // do connections
    QObject::connect(thread, &QThread::started, plotter, &Plotter::plot);
    QObject::connect(plotter, &Plotter::plotChanged, this, &MainWindow::plotChanged);
    QObject::connect(plotter, &Plotter::cleanup, thread, &QThread::quit);
    QObject::connect(thread, &QThread::finished, plotter, &Plotter::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &Plotter::deleteLater);

    thread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    qInfo() << "paint!";
    QPainter painter(this);
    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(QFont("times",10));
    painter.drawImage(event->rect(), backbuffer); // will scale and render backbuffer
    painter.drawText(0, 0, 500, 50, 0, QString::number(drawtime) + "ms; v " + QString::number(verticescount));
}

void MainWindow::keyPressEvent(QKeyEvent *ekey)
{
    switch(ekey->key()) {
        case Qt::Key_Left: plotter->rotate(-1.0, 0.0); break;
        case Qt::Key_Right: plotter->rotate(1.0, 0.0); break;
        case Qt::Key_Up: plotter->rotate(0.0, -1.0); break;
        case Qt::Key_Down: plotter->rotate(0.0, 1.0); break;
        case Qt::Key_N: plotter->rotate(0.0,  0.0, -1.0); break;
        case Qt::Key_M: plotter->rotate(0.0,  0.0, 1.0); break;
        case Qt::Key_W: plotter->move(-0.1,  0.0); break;
        case Qt::Key_S: plotter->move(0.1,  0.0); break;
        case Qt::Key_A: plotter->move(0.0,  -0.1); break;
        case Qt::Key_D: plotter->move(0.0,  0.1); break;
        }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    plotter->zoom(event->angleDelta().y() / 1200. + 1.0);
}

void MainWindow::plotChanged(QImage p, qint64 t)
{
    backbuffer = p;
    drawtime = t;
    repaint();
}

