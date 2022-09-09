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
        std::cout << prop << std::endl;
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
    gst_debug_set_default_threshold(GST_LEVEL_WARNING);

    // Declare custom data structure for the callback
    CUSTOMDATA CustomData;

    CustomData.ImageCounter = 0;
    CustomData.SaveNextImage = false;

    // Query the serial number of the first found device. 
    // will return an emtpy string, if no device was found.
    std::string serialnumber = TcamCamera::getFirstDeviceSerialNumber();
    printf("Tcam OpenCV Image Sample\n");
    
    if( serialnumber != "")
    {
        printf( "Found serial number \"%s\".\n", serialnumber.c_str());
    }

    // Check, whether the passed serial number matches a device.
    // End program, if the serial number does not exist.
    if( !TcamCamera::SerialExists( serialnumber ) )
    {
        printf("Serial number \"%s\" does not exist.\n", serialnumber.c_str());
        return 1;
    }

    // Open camera by serial number
    TcamCamera cam(serialnumber);
    
    // Set video format, resolution and frame rate
    cam.set_capture_format("BGRx", FrameSize{640,480}, FrameRate{30,1});

    // Comment following line, if no live video display is wanted.
    GstElement *xi = gst_element_factory_make("ximagesink", NULL);
    cam.enable_video_display(xi);

    // Register a callback to be called for each new frame
    cam.set_new_frame_callback(new_frame_cb, &CustomData);

    // Uncomment following line, if properties shall be listed. Many of the
    // properties that are done in software re available after the stream 
    // has started. Focus Auto is one of them.
    // ListProperties(cam);

    // If some properties are to be set, uncomment the following code part
    // and set the properties you need to set.
    /*
    try
    {
        cam.set_property("Denoise",0 );
        cam.set_property("GainAuto", "Off");
        cam.set_property("ExposureAuto", "Off");
        cam.set_property("Gain",0.0 );
        cam.set_property("ExposureTime",33333.0 );
        
        double exp;
        cam.get_property("ExposureTime",exp );
        printf("Exposure Time is %f,ms\n", exp);
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
    */
    // Start the camera
    
    if( cam.start() )
    {

        for( int i = 0; i< 1; i++)
        {
            CustomData.SaveNextImage = true; // Save the next image in the callcack call
            sleep(2);
        }


        // Simple implementation of "getch()"
        printf("Press Enter to end the program");
        char dummyvalue[10];
        scanf("%c",dummyvalue);

        cam.stop();
    }

    return 0;
}