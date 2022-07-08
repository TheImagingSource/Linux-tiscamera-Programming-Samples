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

#include <unistd.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include "opencv2/opencv.hpp"


// Create a custom data structure to be passed to the callback function. 
typedef struct
{
    int ImageCounter;
    bool SaveNextImage;
    bool newImage;
   	cv::Mat frame; 
} CUSTOMDATA;


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

        // Now query the width and height of the image
        gst_structure_get_int (str, "width", &width);
        gst_structure_get_int (str, "height", &height);

        // Get the pixelformat of the Buffer
        std::string format = gst_structure_get_string (str, "format");

        if( format == "BGRx")  
        {
            // Create a cv::Mat, copy image data into that and save the image.
            pCustomData->frame.create(height,width,CV_8UC(4));
            memcpy( pCustomData->frame.data, info.data, width*height*4);        
        }
        else if( format == "BGR" )
        {
            pCustomData->frame.create(height,width,CV_8UC(3));
            memcpy( pCustomData->frame.data, info.data, width*height*3);        
        }
        else if( format == "GRAY8" )
        {
            pCustomData->frame.create(height,width,CV_8UC(1));
            memcpy( pCustomData->frame.data, info.data, width*height);        
        }
        else if( format == "GRAY16_LE" )
        {
            pCustomData->frame.create(height,width,CV_16UC(1));
            memcpy( pCustomData->frame.data, info.data, width*height*2);        
        }
        else if( format == "RGBx64" )
        {
            pCustomData->frame.create(height,width,CV_16UC(4));
            memcpy( pCustomData->frame.data, info.data, width*height*8);        
        }
        else 
        {
            printf("Unknown format %s\n",format.c_str());
        }
        
        pCustomData->newImage = true;
    }
    
    // Calling Unref is important!
    gst_buffer_unmap (buffer, &info);
    gst_sample_unref(sample);

    // Set our flag of new image to true, so our main thread knows about a new image.
    return GST_FLOW_OK;
}

/////////////////////////////////////////////////////////////////////
// State change call with some more error handling.
bool WaitForChangeState(GstElement *pipeline, GstState newState)
{
	char ErrorText[1024];
	ErrorText[0] ='\0';
	
	GstStateChangeReturn Result = gst_element_set_state( pipeline, newState);
	if( Result == GST_STATE_CHANGE_FAILURE )
	{
		printf("FAILURE set state() () change to state %d failed\n", newState);
		return false;
	}

	Result = gst_element_get_state( pipeline, NULL, NULL, 4000000000ll);

	if( Result == GST_STATE_CHANGE_FAILURE )
	{
		printf("FAILURE WaitForChangeState() change to state %d failed\n", newState);
		return false;
	}

	if( Result == GST_STATE_CHANGE_ASYNC )
	{
		printf("ASYNC WaitForChangeState() change to state %d failed\n", newState);
		return false;
	}
	return true;
}                   

///////////////////////////////////////////////////////////////////////////////////////////////////
// Create the pipeline and add the sink callback
// cbdata is callback user data.
GstElement* CreatePipeline( std::string PipeLineString, void* cbdata)
{
    printf("%s\n",PipeLineString.c_str());

    GError *error = NULL;         
 	GstElement *pipeline = gst_parse_launch (PipeLineString.c_str(), &error);            

    if (error)
	{
		printf( "Error building Camera pipeline: %s\n", error->message);
		return NULL;
	}        


	GstAppSink *appsink = (GstAppSink*)gst_bin_get_by_name(GST_BIN(pipeline), "sink");
	if( appsink != NULL )
	{
		gst_app_sink_set_max_buffers(appsink, 5);
		gst_app_sink_set_drop(appsink, true); 
		
		GstAppSinkCallbacks callbacks;
		callbacks.eos = NULL;
		callbacks.new_preroll = NULL;
		callbacks.new_sample = new_frame_cb;
		
		gst_app_sink_set_callbacks( appsink, &callbacks, (gpointer) cbdata,NULL);
		
		gst_object_unref(appsink);
	}
	else
    {
		printf("No appsink found. Forgot \" appsink name=sink\"?\n");
        return NULL;
    }

    return pipeline;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    gst_init(&argc, &argv);
    // Declare custom data structure for the callback
    CUSTOMDATA CustomData;

    CustomData.ImageCounter = 0;
    CustomData.SaveNextImage = false;
    CustomData.newImage = false;

    std::string p = "tcambin name=source ! video/x-raw,format=GRAY16_LE,width=640,height=480,framerate=30/1 ! appsink sync=false name=sink";

    auto pipeline = CreatePipeline( p , &CustomData );
    if( pipeline == NULL)
        return 1;

    if( !WaitForChangeState(pipeline, GST_STATE_PLAYING))
        return 1;

    // Get the next image
    CustomData.newImage = false;
    CustomData.SaveNextImage = true;
    while( !CustomData.newImage )
    {
        usleep(100);
    }

    if( CustomData.newImage)
    {
        // Check the type of OpenCV mat:
        int type = CustomData.frame.type();
        std::string r; 
        uchar depth = type & CV_MAT_DEPTH_MASK;
        uchar chans = 1 + (type >> CV_CN_SHIFT);

        switch ( depth ) {
            case CV_8U:  r = "8U"; break;
            case CV_8S:  r = "8S"; break;
            case CV_16U: r = "16U"; break;
            case CV_16S: r = "16S"; break;
            case CV_32S: r = "32S"; break;
            case CV_32F: r = "32F"; break;
            case CV_64F: r = "64F"; break;
            default:     r = "User"; break;
        }

        r += "C";
        r += (chans+'0');
        
        cv::FileStorage fs("file.yml", cv::FileStorage::WRITE);
        fs << "data" << CustomData.frame;
        
    }
    
    // Simple implementation of "getch()"
    printf("Press Enter to end the program");
    char dummyvalue[10];
    scanf("%c",dummyvalue);
    
    WaitForChangeState(pipeline, GST_STATE_NULL);

    return 0;
}