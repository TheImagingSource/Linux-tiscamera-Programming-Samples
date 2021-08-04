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
Please change the serial number in the TcamCamera contructor call in main.cpp main() to your camera's serial number. This is documented below in detail.

