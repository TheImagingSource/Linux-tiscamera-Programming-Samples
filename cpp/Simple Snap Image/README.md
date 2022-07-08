# Simple Snap Image C++
This sample shows how to snap an image from the video stream of the The Imaging Source camera.
For demonstrating how to use the image data OpenCV is used.

Programming language : C++

## Building
In order to build the sample, open a terminal, enter the sample's directory. Then enter
```
mkdir build
cd build 
cmake ..
make
./simple-snapimages
```
## Files
### main.cpp
This is the main program and will be documented below.

### tcamimage.h and tcamiamge.cpp
In these files the TcamImage class is declared. This class is inherited from TcamCamera and adds the functionallity for snapping the image and providing the image data.



## Code Documentation
Before starting with the sample search in "main.cpp" search the line which contents
```TcamCamera cam("nnnnn");```
and exchange the serial number there by the serial number of your camera.

First the camera is opened, configured and started.
```C++
    TcamImage cam("48610605");
    cam.set_capture_format("BGRx", FrameSize{640,480}, FrameRate{30,1});
    cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));
    cam.start();
```
The image is snapped by a call to ```cam.snapImage(60)```. A timeout in milliseconds is passed. The timeout should be at least the double of the frame rates time. The function returns ```true``` on success. If a timeout occured, it returns ```false```.

Information about the snapped image are retrieved by:
* ```cam.getHeight()```
Image height.
* ```cam.getWidth()```
Image width.
* ```cam.getBytesPerPixel()```
Number of bytes per pixel, which is currently 1 at GRAY8 and 4 at BGRx.
* ```cam.getImageData()```
Pointer to the image data.
* ```cam.getImageDataSize()```
Number of bytes of the image.

Snapping an image and converting it to OpenCV cv::Mat is done as follows:
``` C++
    if( cam.snapImage(60) )
    {
        OpenCVImage.create( cam.getHeight(),cam.getWidth(),CV_8UC(cam.getBytesPerPixel()));
        memcpy( OpenCVImage.data, cam.getImageData(), cam.getImageDataSize());
        cv::imwrite("test.jpg",OpenCVImage);
    }
    else
    {
        printf("Timeout at snapImage()\n");
    }
```
