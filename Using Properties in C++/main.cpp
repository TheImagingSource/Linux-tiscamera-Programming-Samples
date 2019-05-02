////////////////////////////////////////////////////////////////////////
/* Using Property Example
This sample shows, how to set Gain and Expoures properties programmatically

It uses the the examples/cpp/common/tcamcamera.* files as wrapper around the
GStreamer code and property handling. Adapt the CMakeList.txt accordingly.

For some cameras, the automatic properties are available, when the camera's 
GStreamer pipeline is in READY state. Currently this is, after the live video 
has been started.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stdexcept>

#include "tcamcamera.h"

using namespace gsttcam;


//////////////////////////////////////////////////////////////////////////
// List available properties helper function. Call this functions, if a
// list of available camera properties is needed.
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


///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    gst_init(&argc, &argv);

    // Declaration of the Pointers to the properties.
    std::shared_ptr<Property> ExposureAuto = NULL;
    std::shared_ptr<Property> ExposureValue = NULL;
    std::shared_ptr<Property> GainAuto = NULL;
    std::shared_ptr<Property> GainValue = NULL;

    // Initialize our TcamCamera object "cam" with the serial number
    // of the camera, which is to be used in this program.
    TcamCamera cam("42719953");
    //TcamCamera cam("00001234");
    
    // Set a color video format, resolution and frame rate
    cam.set_capture_format("BGRx", FrameSize{640,480}, FrameRate{30,1});

    // Comment following line, if no live video display is wanted.
    cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));
    
    // Start the camera
    cam.start();

    // Uncomment following line, if properties shall be listed. Sometimes
    // the property names of USB and GigE cameras differ.
    // ListProperties(cam);

    // Query the pointers to the white balance properties. If a properties
    // does not exist an exception is thrown.
    try
    {
        ExposureAuto = cam.get_property("Exposure Auto");
    }
    catch(std::exception &ex)    
    {
        printf("Error %s : %s\n",ex.what(), "Exposure Automatic");
    }

    try
    {
        ExposureValue = cam.get_property("Exposure Time (us)");
    }
    catch(std::exception &ex)    
    {
        printf("Error %s : %s\n",ex.what(), "Exposure Value");
    }

    try
    {
        GainAuto = cam.get_property("Gain Auto");
    }
    catch(std::exception &ex)    
    {
        printf("Error %s : %s\n",ex.what(), "Gain Automatic");
    }

    try
    {
        GainValue = cam.get_property("Gain");
    }
    catch(std::exception &ex)    
    {
        printf("Error %s : %s\n",ex.what(), "Gain Value");
    }

    // Now get the current property values:
    if( ExposureAuto != NULL)
    {
        int Auto;
        ExposureAuto->get(cam,Auto);
        if( Auto == 1)
            printf("Current exposure automatic is On.\n");
        else
            printf("Current exposure automatic is Off.\n");
    }

    if( ExposureValue != NULL)
    {
        int ExposureTime;
        ExposureValue->get(cam,ExposureTime);
        printf("Current exposure time is %d.\n",ExposureTime);
    }

    if( GainAuto != NULL)
    {
        int Auto;
        GainAuto->get(cam,Auto);
        if( Auto == 1)
            printf("Current gain automatic is On.\n");
        else
            printf("Current gain  automatic is Off.\n");
    }

    if( GainValue != NULL)
    {
        int gain;
        GainValue->get(cam,gain);
        printf("Current gain value is %d.\n",gain);
    }

    // Disable automatics, so the property values can be set 
    if( ExposureAuto != NULL){
        ExposureAuto->set(cam,0);
    }

     if( GainAuto != NULL){
        GainAuto->set(cam,0);
     }


    // set a value
    if( ExposureValue != NULL){
        ExposureValue->set(cam,333);
    }

    if( GainValue != NULL){
        GainValue->set(cam,400);
    }

    printf("Press enter key to end program.");

    // Simple implementation of "getch()", wait for enter key.
    char dummyvalue[10];
    scanf("%c",dummyvalue);

    cam.stop();

    return 0;
}