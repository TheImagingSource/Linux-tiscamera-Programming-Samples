# Save an Image using OpenCV in C++
This sample shows, how to get  images from a [The Imaging Source](https://www.theimagingsource.com/) camera using a callback and OpenCV. It works for USB as well for GigE cameras.
In particular it shows, how to get 16 bit gray scale images, so the 10 or 12 bit formats of a sensor can be used.

Allowed formats are
* GRAY8
* GRAY16_LE
* BGR
* BGRx
* RGBx64 (tcamdutils must be installed)

The image data is saved into a cv::FileStorage.

The sample also implements all GStreamer related coded in `main.cpp`.

Programming language : C++

## Building
In order to build the sample, open a terminal, enter the sample's directory. Then enter
```
mkdir build
cd build 
cmake ..
make
./tcamopencvsaveimage
```
Possible pipelines are

16bit grayscale
```
tcambin name=source ! video/x-raw,format=GRAY16_LE,width=640,height=480,framerate=30/1 ! appsink sync=false name=sink
```
RGB64
```
tcambin name=source ! video/x-raw,format=RGBx64,width=640,height=480,framerate=30/1 ! appsink sync=false name=sink
```

RGB
```
tcambin name=source ! video/x-raw,format=BGR,width=640,height=480,framerate=30/1 ! appsink sync=false name=sink
```

GRAY8
```
tcambin name=source ! video/x-raw,format=BGR,width=640,height=480,framerate=30/1 ! appsink sync=false name=sink
```



