#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "plotter.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *ekey) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

public Q_SLOTS:
    void plotChanged(QImage p, qint64 t);

protected:
    QImage backbuffer;
    Plotter *plotter;

    qint64 drawtime;
    qint64 verticescount;
    qint64 polycount;

    std::array<qint64, 100> drawtimes{0};
    qint64 drawtimeTimes = 0;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
