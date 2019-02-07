#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>
#include <QtGui>

#include "tcamcamera.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void OnSelectDevice();
    void OnDeviceProperties();
    void SaveLastUsedDevice(std::string serial, int width, int height, int fps1, int fps2);
    void LoadLastUsedDevice();
    void OpenandStartDevice(std::string serial, int width, int height, int fps1, int fps2);

private:
    QWidget *window;
    QGridLayout *mainLayout;
    QWidget* _VideoWidget;
    QMenu *_fileMenu;
    QMenu *_deviceMenu;
    QAction *_exitAct;
    QAction *_DeviceSelectAct;
    QAction *_DevicePropertiesAct;

    gsttcam::TcamCamera *_pCam;
};

#endif // MAINWINDOW_H
