/*
 * Copyright 2015 bvtest <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "mainwindow.h"
#include "cdeviceselectiondlg.h"
#include "cpropertiesdialog.h"
#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    // Instance of our TcamCamera class
    _pCam = NULL;

    window = new QWidget(this);
    setCentralWidget( window );
    mainLayout = new QGridLayout;
    window->setLayout(mainLayout);

    // Create the file menu
    _fileMenu = menuBar()->addMenu(tr("&File"));

    _exitAct = new QAction(tr("&Exit"), this);
    _exitAct->setShortcuts(QKeySequence::Close);
    _exitAct->setStatusTip(tr("Exit program"));
    connect(_exitAct, &QAction::triggered, this, &QWidget::close);
    _fileMenu->addAction(_exitAct);


    // Create the Device menu
    _deviceMenu = menuBar()->addMenu(tr("&Device"));

    _DeviceSelectAct = new QAction(tr("&Select"), this);
    _DeviceSelectAct->setStatusTip(tr("Select and start a device"));
    connect(_DeviceSelectAct, &QAction::triggered, this, &MainWindow::OnSelectDevice);
    _deviceMenu->addAction(_DeviceSelectAct);

    _DevicePropertiesAct = new QAction(tr("&Properties"), this);
    _DevicePropertiesAct->setStatusTip(tr("Show device property diallg"));
    connect(_DevicePropertiesAct, &QAction::triggered, this, &MainWindow::OnDeviceProperties);
    _deviceMenu->addAction(_DevicePropertiesAct);


    // Create the video display Widget
    _VideoWidget = new QWidget();
    _VideoWidget->setAttribute(Qt::WA_PaintOnScreen);
	_VideoWidget->setAttribute(Qt::WA_OpaquePaintEvent);
    _VideoWidget->setMinimumSize(320,240);
    mainLayout->addWidget(_VideoWidget);
    LoadLastUsedDevice();
}


MainWindow::~MainWindow()
{
    if( _pCam != NULL)
    {
        _pCam->stop();
        delete _pCam;
    }
}


void MainWindow::OnSelectDevice()
{

    // Create the device selection dialog
    CDeviceSelectionDlg Dlg(window );
    if( Dlg.exec() == 1 )
    {
        OpenandStartDevice(Dlg.getSerialNumber(),
                           Dlg.getWidth(),Dlg.getHeight(),
                           Dlg.getFPSNominator(),Dlg.getFPSDeNominator());

        SaveLastUsedDevice(Dlg.getSerialNumber(),
                           Dlg.getWidth(),Dlg.getHeight(),
                           Dlg.getFPSNominator(),Dlg.getFPSDeNominator());
    }
}


void MainWindow::OnDeviceProperties()
{
    if( _pCam != NULL )
    {
        CPropertiesDialog Properties(nullptr);
        TcamProp* ptcam = (TcamProp*)_pCam->getTcamBin();
        // Pass the tcambin element to the properties dialog
        // so in knows, which device do handle
        Properties.SetCamera(ptcam);
        Properties.exec();
    }
}

//////////////////////////////////////////////////////////////////////////////
// Open the camea set format and fps and start.
//
void MainWindow::OpenandStartDevice(std::string serial, int width, int height, int fps1, int fps2)
{
        if( _pCam != NULL)
        {
            _pCam->stop();
            delete _pCam;
        }
        // Instantiate the TcamCamera object with the serial number
        // of the selected device
        _pCam = new gsttcam::TcamCamera(serial);

        // Set video format, resolution and frame rate. We display color.
        _pCam->set_capture_format("BGRx", 
                            gsttcam::FrameSize{width,height}, 
                            gsttcam::FrameRate{fps1,fps2});

        // Get the window handle of the video widget
        WId xwinid = _VideoWidget->winId();

        // Create the display sink
        GstElement *imagesink = gst_element_factory_make("ximagesink", NULL);

        // Assing the window handle
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(imagesink) , xwinid);

        // Pass the display sink to the TcamCamera object
        _pCam->enable_video_display(imagesink);

        // Show the live vieo
        _pCam->start();

}



void MainWindow::SaveLastUsedDevice(std::string serial, int width, int height, int fps1, int fps2)
{
    FILE *pFile = fopen("lastuseddevice.txt", "w");
    if( pFile )
    {
        fprintf(pFile,"%s\n%d\n%d\n%d\n%d\n", serial.c_str(), width,height,fps1,fps2);
        fclose( pFile);
    }
}

void MainWindow::LoadLastUsedDevice()
{
    char serial[50];
    int width;
    int height;
    int fps1;
    int fps2;

    FILE *pFile = fopen("lastuseddevice.txt", "r");
    if( pFile )
    {
        fscanf(pFile,"%s", serial );
        fscanf(pFile,"%d", &width);
        fscanf(pFile,"%d", &height);
        fscanf(pFile,"%d", &fps1);
        fscanf(pFile,"%d", &fps2);
        fclose( pFile);

        OpenandStartDevice( serial,  width,  height,  fps1,  fps2);
    }


}
