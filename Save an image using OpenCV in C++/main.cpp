//////////////////////////////////////////////////////////////////
/*
Tcam Software Trigger
This sample shows, how to trigger the camera by software and use a callback for image handling.

Prerequisits
It uses the the examples/cpp/common/tcamcamera.cpp and .h files of the *tiscamera* repository as wrapper around the
GStreamer code and property handling. Adapt the CMakeList.txt accordingly.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "tcamcamera.h"
#include <unistd.h>

#include "opencv2/opencv.hpp"

using namespace gsttcam;

// Create a custom data structure to be passed to the callback function. 
typedef struct
{
    int ImageCounter;
    bool SaveNextImage;
    bool busy;
   	cv::Mat frame; 
} CUSTOMDATA;

////////////////////////////////////////////////////////////////////
// List available properties helper function.
void ListProperties(TcamCamera &cam)
{
    // Get a list of all supported properties and print it out
    auto properties = cam.get_camera_property_list();
    std::cout << "Properties:" << std::endl;
    for(auto &prop : properties)
    {
        std::cout << prop->to_string() << std::endl;
    }
}

////////////////////////////////////////////////////////////////////
// Callback called for new images by the internal appsink
GstFlowReturn new_frame_cb(GstAppSink *appsink, gpointer data)
{
    int width, height ;
    const GstStructure *str;

    // Cast gpointer to CUSTOMDATA*
    CUSTOMDATA *pCustomData = (CUSTOMDATA*)data;
    if( !pCustomData->SaveNextImage)
        return GST_FLOW_OK;
    pCustomData->SaveNextImage = false;

    pCustomData->ImageCounter++;

    // The following lines demonstrate, how to acces the image
    // data in the GstSample.
    GstSample *sample = gst_app_sink_pull_sample(appsink);

    GstBuffer *buffer = gst_sample_get_buffer(sample);

    GstMapInfo info;

    gst_buffer_map(buffer, &info, GST_MAP_READ);
    
    if (info.data != NULL) 
    {
        // info.data contains the image data as blob of unsigned char 

        GstCaps *caps = gst_sample_get_caps(sample);
        // Get a string containg the pixel format, width and height of the image        
        str = gst_caps_get_structure (caps, 0);    

        if( strcmp( gst_structure_get_string (str, "format"),"BGRx") == 0)  
        {
            // Now query the width and height of the image
            gst_structure_get_int (str, "width", &width);
            gst_structure_get_int (str, "height", &height);

            // Create a cv::Mat, copy image data into that and save the image.
            pCustomData->frame.create(height,width,CV_8UC(4));
            memcpy( pCustomData->frame.data, info.data, width*height*4);
            char ImageFileName[256];
            sprintf(ImageFileName,"image%05d.jpg", pCustomData->ImageCounter);
            cv::imwrite(ImageFileName,pCustomData->frame);
        }

    }
    
    // Calling Unref is important!
    gst_buffer_unmap (buffer, &info);
    gst_sample_unref(sample);

    // Set our flag of new image to true, so our main thread knows about a new image.
    return GST_FLOW_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    gst_init(&argc, &argv);
    // Declare custom data structure for the callback
    CUSTOMDATA CustomData;

    CustomData.ImageCounter = 0;
    CustomData.SaveNextImage = false;

    
    printf("Tcam OpenCV Image Sample\n");

    // Open camera by serial number
    TcamCamera cam("10610452");
    
    
    // Set video format, resolution and frame rate
    cam.set_capture_format("BGRx", FrameSize{640,480}, FrameRate{30,1});

    // Comment following line, if no live video display is wanted.
    cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));

    // Register a callback to be called for each new frame
    cam.set_new_frame_callback(new_frame_cb, &CustomData);
    
    // Start the camera
    cam.start();

    // Uncomment following line, if properties shall be listed. Many of the
    // properties that are done in software are available after the stream 
    // has started. Focus Auto is one of them.
    // ListProperties(cam);

    for( int i = 0; i< 10; i++)
    {
        CustomData.SaveNextImage = true; // Save the next image in the callcack call
        sleep(2);
    }


    // Simple implementation of "getch()"
    printf("Press Enter to end the program");
    char dummyvalue[10];
    scanf("%c",dummyvalue);

    cam.stop();

    return 0;
}