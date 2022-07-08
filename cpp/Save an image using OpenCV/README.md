# Save an Image using OpenCV in C++
This sample shows, how to save images from a [The Imaging Source](https://www.theimagingsource.com/) camera using a callback and OpenCV. It works for USB as well for GigE cameras.

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

## General Program Flow
The program opens and configures a camera. It also adds a callback function, which is used to handle the incoming image data. A data structure is used to control, when a frame should be saved.

## Handling the Camera
The tiscamera repository provides the TcamCamera class, which wraps all the GStreamer code. The TcamCamera class is instantiated as follows:
``` C++
TcamCamera cam("00001234");
``` 
The video format resolution and frame rate is set with
``` C++
// Set video format, resolution and frame rate
cam.set_capture_format("BGRx", FrameSize{640,480}, FrameRate{30,1});
``` 
The tcam-ctrl -c <serial> program is used for getting a list of supported video format resoutions and frame rates.

The programmer can decide, whether a live video window should be shown:
```C++
// Comment following line, if no live video display is wanted.
cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));
```
In the next step the callback function is passed to the camera class:
```C++
// Register a callback to be called for each new frame
cam.set_new_frame_callback(new_frame_cb, &CustomData);
```

Now the live video can be started:
```C++
// Start the camera
cam.start();
```
It is stopped at program end with
```C++
// Start the camera
cam.stop();
```

## The Custom Data Struct
```C++
// Create a custom data structure to be passed to the callback function. 
typedef struct
{
    int ImageCounter;  // Counter for images
    bool SaveNextImage; // Flag, whether the next incoming image will be saved
    bool busy; // Busy flag
    cv::Mat frame;  // OpenCV Matrix for the image processing
} CUSTOMDATA;
```
This structure is passed to the callback function and will be used for controling of image saving. If the flag ```SaveNextImage```is set to false, no images will be saved. It if is set to true, the next image will be saved and set to false in the callback function.


## The Callback
The callback's function is declared as follows:
``` C++
GstFlowReturn new_frame_cb(GstAppSink *appsink, gpointer data)
```
| Parameter | Description |
| - | - |
| appsink | The Tcamcamera class uses an [appsink](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-libs/html/gst-plugins-base-libs-appsink.html) as callback handerl. Its pointer is passed to the callback function. |
| data | This is a pointer to user data. |

The function must be passed to the Tcamcamera object:
``` C++
cam.set_new_frame_callback(new_frame_cb, null);
```

If user data is used, then its pointer is passed, e.g:
``` C++
int Counter = 0
cam.set_new_frame_callback(new_frame_cb, &Counter);
```


The above mentioned custom data struct is passed to the callback 
``` C++
// Register a callback to be called for each new frame
cam.set_new_frame_callback(new_frame_cb, &CustomData);
```

The complete callback function is as follows.
``` C++
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
```

First of all, the image sample must be pulled from the appsink. It is needed to access its properties like pixel format, width and height and also the image data.
``` C++
    GstSample *sample = gst_app_sink_pull_sample(appsink);
```
The caps structure provides the image properties like pixel format, width and height from the sample. They are converted into a string, which is interpreted by [gst_structure_get_string](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstStructure.html#gst-structure-get-string) and [gst_structure_get_int](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstStructure.html#gst-structure-get-int).

``` C++
GstCaps *caps = gst_sample_get_caps(sample);
str = gst_caps_get_structure (caps, 0);  

if( strcmp( gst_structure_get_string (str, "format"),"BGRx") == 0) 
{
    gst_structure_get_int (str, "width", &width);
    gst_structure_get_int (str, "height", &height);
```
Only if the pixel format is BGRx, the images shall be saved. Then width and height are queried.

The image data iteself is in the [GstBuffer](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstBuffer.html) object, which is querried from the sample. That must be mapped to a [GstMapInfo](https://developer.gnome.org/gstreamer/stable/gstreamer-GstMemory.html#GstMapInfo) object, which contains the pointer to the image data.

``` C++
GstMapInfo info;
GstBuffer *buffer = gst_sample_get_buffer(sample);
gst_buffer_map(buffer, &info, GST_MAP_READ);
```

Image saving is done using OpenCV. The GSTBuffer is copied to an [cv::Mat](https://docs.opencv.org/3.1.0/d3/d63/classcv_1_1Mat.html). Now all neccessary information for creating the cv::Mat is gathered and the image can be saved in the cv::Mat. 
``` C++
pCustomData->frame.create(height,width,CV_8UC(4));
memcpy( pCustomData->frame.data, info.data, width*height*4);
``` 
cv::Mat::create() wont allocate new memory, if the width, height and bytes per pixels did not change.

Now the image file name is created an the image is saved.
``` C++
char ImageFileName[256];
sprintf(ImageFileName,"image%05d.jpg", pCustomData->ImageCounter);
cv::imwrite(ImageFileName,pCustomData->frame);
```
Very important: The references to the GStreamer objects must be cleared afterwards:

``` C++
gst_buffer_unmap (buffer, &info);
gst_sample_unref(sample);
```

## The main function
The program starts the camera, saves every two seconds on image and ends when 10 images have been saved.
``` C++
int main(int argc, char **argv)
{
    gst_init(&argc, &argv);
    // Declare custom data structure for the callback
    CUSTOMDATA CustomData;

    CustomData.ImageCounter = 0;
    CustomData.SaveNextImage = false; // Set to false, because we do not want to save an image now.

    
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
```

### Annotation
We are aware this sample is somewhat complicated, because a programmer may expect a "snapImage" function rather than a callback. We hope, a "snapImage" will be implemented soon in the TcamCamera class.
