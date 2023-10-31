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
    plotter = new Plotter(QSize(2880 / 2, 1920 / 2 ));
    // temp
    QVector < Math::Vec3 > vertices;
    QVector < QVector<std::tuple<int, int, int>> > indices;
    QVector < Math::Vec3 > normals;
    QVector < Math::Vec3 > colors;
    QVector < Math::Vec3 > textures;
    QVector < int > texIDs;
    QVector < TexInfo > texture;
    // Material Ball/export3dcoat.obj
    // Cyber Mancubus/mancubus.obj
    // Cube/cube.obj
    // Intergalactic Spaceship/Intergalactic_Spaceship-(Wavefront).obj
    // Shovel Knight/shovel_low.obj
    // Robot Steampunk/robot_steampunk.obj
    // Tree man/model.obj
    // Doom Slayer/doomslayer.obj
    // Cat/test.obj
    //
    if (loadOBJ(QFile("./Models/Material Ball/export3dcoat.obj"), vertices, indices, normals, colors, textures, texIDs, texture))
    {
        qDebug() << "Data loaded";
        if (colors.empty()) {
            colors.fill(Math::Vec3{1, 1, 1}, vertices.size());
        }
        plotter->setData(vertices, indices, normals, colors, textures, texIDs, texture);
        verticescount = vertices.size();
        polycount = indices.size();
    }
    else
    {
        qDebug() << "Failed to load data, using custom";
//        plotter->setData({{0.0, 1.0, 1.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0},
//                          {1.0, 1.0, 1.0}, {1.0, 0.0, 1.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}},
//                         {{0, 1}, {1, 2}, {2, 3}, {3, 0},
//                          {4, 5}, {5, 6}, {6, 7}, {7, 4},
//                          {0, 4}, {1, 5}, {2, 6}, {3, 7}});
//        plotter->setData({{-1.0, 1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0},
//                          {1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, -1.0, -1.0}, {1.0, 1.0, -1.0}},
//                         {{0, 1}, {1, 2}, {2, 3}, {3, 0},
//                          {4, 5}, {5, 6}, {6, 7}, {7, 4},
//                          {0, 4}, {1, 5}, {2, 6}, {3, 7}});
//        plotter->setData({{-1.0, 1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0},
//                          {1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, -1.0, -1.0}, {1.0, 1.0, -1.0}},
//                         {{0, 1, 2}, {2, 3, 0}, {7, 4, 0}, {3, 7, 0},
//                          {7, 3, 2}, {2, 6, 7}, {4, 5, 6}, {6, 7, 4},
//                          {1, 2, 6}, {6, 5, 1}, {0, 1, 5}, {5, 4, 0}},
//                         {}, {});
    }


    //QThread *thread = new QThread;
    //plotter->moveToThread(thread);

    // do connections
    //QObject::connect(thread, &QThread::started, plotter, &Plotter::plot);
    QObject::connect(plotter, &Plotter::plotChanged, this, &MainWindow::plotChanged);
    //QObject::connect(plotter, &Plotter::cleanup, thread, &QThread::quit);
    //QObject::connect(thread, &QThread::finished, plotter, &Plotter::deleteLater);
    //QObject::connect(thread, &QThread::finished, thread, &Plotter::deleteLater);

    //thread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    //qInfo() << "paint!";
    auto camera = plotter->getCamera();

    QPainter painter(this);
    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(QFont("times",10));
    painter.drawImage(event->rect(), backbuffer); // will scale and render backbuffer
    painter.drawText(0, 0, 1000, 50, 0, QString::number(drawtime) + "ms; avg "+
                    QString::number(std::accumulate(drawtimes.begin(), drawtimes.end(), 0.0) / 100) +" ms; v " + QString::number(verticescount)
                    + " p " + QString::number(polycount)
                    + " cam pos x " + QString::number(camera->pos().x()) + " y " + QString::number(camera->pos().y()) + " z " + QString::number(camera->pos().z()));
}

void MainWindow::keyPressEvent(QKeyEvent *ekey)
{
    auto camera = plotter->getCamera();
    // rotate model
    //switch(ekey->key()) {
    //    case Qt::Key_Left: plotter->rotate(-1.0, 0.0); break;
    //    case Qt::Key_Right: plotter->rotate(1.0, 0.0); break;
    //    case Qt::Key_Up: plotter->rotate(0.0, -1.0); break;
    //    case Qt::Key_Down: plotter->rotate(0.0, 1.0); break;
    //    case Qt::Key_N: plotter->rotate(0.0,  0.0, -1.0); break;
    //    case Qt::Key_M: plotter->rotate(0.0,  0.0, 1.0); break;
    //    case Qt::Key_W: plotter->move(-0.1,  0.0); break;
    //    case Qt::Key_S: plotter->move(0.1,  0.0); break;
    //    case Qt::Key_A: plotter->move(0.0,  -0.1); break;
    //    case Qt::Key_D: plotter->move(0.0,  0.1); break;
    //}
    // TODO they are called in another thread!!!!!!!
    switch(ekey->key()) {
    case Qt::Key_W: camera->moveForward(0.1); break;
    case Qt::Key_S: camera->moveForward(-0.1); break;
    case Qt::Key_A: camera->moveSide(-0.1); break;
    case Qt::Key_D: camera->moveSide(0.1); break;
    case Qt::Key_Space: camera->moveUp(0.1); break;
    case Qt::Key_Shift: camera->moveUp(-0.1); break;
    case Qt::Key_N: plotter->rotate(0.0,  0.0, -1.0); break;
    case Qt::Key_M: plotter->rotate(0.0,  0.0, 1.0); break;
    case Qt::Key_P: plotter->togglePause(); break;
    }

    //plotter->plot();
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    // TODO they are called in another thread!!!!!!!
    plotter->zoom(event->angleDelta().y() / 1200. + 1.0);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // TODO they are called in another thread!!!!!!!
    //qInfo() << "press";
    plotter->getCamera()->reset(event->globalX(), event->globalY());
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // TODO they are called in another thread!!!!!!!
    //qInfo() << "move";
    plotter->getCamera()->rotate(event->globalX(), event->globalY());
    //plotter->plot();
}

void MainWindow::plotChanged(QImage p, qint64 t)
{
    backbuffer = p;
    drawtime = t;
    drawtimes[drawtimeTimes++ % 100] = t;
    repaint();
}

