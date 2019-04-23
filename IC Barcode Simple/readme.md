# ic-barcode Demo Program
This demo program shows how the tis-barcode library decodes barcode on images provided by a video capture device, e.g. a camera.

Programming language : C++

## Prerequisits
The sample uses  the [*tiscamera*](https://github.com/TheImagingSource/tiscamera) repository and  [GStreamer](https://gstreamer.freedesktop.org/).
Also QT5 is needed for the GUI.
For running the demo program, a The Imaging Source camera must be connected to the computer.

## Location
The sample is saved into /usr/share/theimagingsource/tis-barcode/samples/tis-barcode-demo.
You may create a local copy.

## Building
In order to build the sample, open a terminal, enter the sample's directory. Then enter
```
mkdir build
cd build 
cmake ..
make
./ic-barcode-demo
```

##Source Code Files
###main.cpp
The QT5 main program

###mainwindow.h mainwindow.cpp
This file contains the creation of GUI, camera handling and barcode decoding.

###tcamcamera.h tcamcamera.cpp
These files contain the C++ wrapper for the tiscamera Gstreamer modules. In there is the pipeline for image callbacks, graphical overlay and image scaling. 

###cdevicesettings.h cdevicesettings.cpp
This is the QT5 device video format and frame rate selection dialog.

###cpropertiesdialog.h cpropertiesdialog.cpp
This builds the device properties dialog. The shown properties depend on the properties, the device exports.

