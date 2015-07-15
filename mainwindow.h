#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPainter>
#include "pxcsensemanager.h"
#include "pxcfaceconfiguration.h"

#define MAX_FACES 5

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // Timer to get images from the camera
    QTimer *timer;

    // RealSense stuff
    PXCSenseManager *sm;
    PXCFaceModule* faceModule;
    PXCFaceData *faceData;
    PXCFaceConfiguration* config;
public slots:
    void onStart();
    void onStop();
    void onNewImage();
};

#endif // MAINWINDOW_H
