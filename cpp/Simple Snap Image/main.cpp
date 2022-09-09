////////////////////////////////////////////////////////////////////////
/* Simple snapimage Example
This sample shows, how to snap a image from the live stream.

It uses the the examples/cpp/common/tcamcamera.* files as wrapper around the
GStreamer code and property handling. Adapt the CMakeList.txt accordingly.

As sample image processing an OpenCV cv::Mat is generated and saved as JPEG
*/

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
 
#include "tcamimage.h"

#include "opencv2/opencv.hpp"

using namespace gsttcam;

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    gst_init(&argc, &argv);

    cv::Mat OpenCVImage;
    // Initialize our TcamCamera object "cam" with the serial number
    // of the camera, which is to be used in this program.
    std::string serial = "41910044";

    TcamImage cam(serial);
    if( !cam.SerialExists(serial))
    {
        std::cout << "Serialnumber " << serial << " does not exist." << std::endl;
        return 1;
    }
    
    // Set a color video format, resolution and frame rate
    cam.set_capture_format("BGRx", FrameSize{640,480}, FrameRate{30,1});


    // Comment following line, if no live video display is wanted.
    cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));
    
    // Start the camera
    cam.start();

    sleep(1);
    printf("Start Snap\n");
    // Snap an Image with 60 ms timeout. Should be set accordingly to the
    // frame rate.
    if( cam.snapImage(60) )
    {
        // On succes do something with the image data. Here we create
        // a cv::Mat and save the image
        OpenCVImage.create( cam.getHeight(),cam.getWidth(),CV_8UC(cam.getBytesPerPixel()));
        memcpy( OpenCVImage.data, cam.getImageData(), cam.getImageDataSize());
        cv::imwrite("test.jpg",OpenCVImage);
    }
    else
    {
        printf("Timeout at snapImage()\n");
    }

    printf("Press enter key to end program.");
    // Simple implementation of "getch()", wait for enter key.
    char dummyvalue[10];
    scanf("%c",dummyvalue);

    cam.stop();

    return 0;
}