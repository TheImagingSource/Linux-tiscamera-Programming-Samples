# Tcam Stereo Capture
This sample shows how to capture and save images from two cameras. The cameras are synchronized by software trigger,

## Prerequisites
The sample uses the the examples/cpp/common/tcamcamera.cpp and .h files of the *tiscamera* repository as wrapper around the
[GStreamer](https://gstreamer.freedesktop.org/) code and property handling. Adapt the CMakeList.txt accordingly.
Image saving is done using OpenCV. The GSTBuffer is copied to an [cv::Mat](https://docs.opencv.org/3.1.0/d3/d63/classcv_1_1Mat.html)


In "main.cpp" search the line which contents
``` C++
TcamCamera cam1("00001234");
TcamCamera cam2("00001234");
```
and exchange "00001234" by the serial numbers of your cameras. The following documentation is mainly for one camera, the very same we do with cam1 we must do with cam2 too.

## Building
In order to build the sample, open a terminal, enter the sample's directory. Then enter
``` bash
mkdir build
cd build 
cmake ..
make
./tcamstereocapture
```

## The Camera Object
The tiscamera repository provides the TcamCamera class, which wraps all the GStreamer code. Since there are two cameras to be handled, two instances of this class are needed:
``` C++
TcamCamera cam1("00001234");
TcamCamera cam2("00005678");
```

## Using the Softwaretrigger
For the software trigger, two properties are needed. The "Trigger Mode", which is used to enable or disabe the trigger mode in the camera and the "Softwaretrigger" property.
### Getting the Trigger Mode Property
First of al the declaration.
``` C++
std::shared_ptr<Property> TriggerMode;
```
Not all camera models support trigger mode. The the camera does not support trigger mode, the TcamCamera object will throw an exception. Therefore, the property is queried in a try..catch block:

``` C++
try
{
    TriggerMode = cam1.get_property("Trigger Mode");
}
catch(...)
{
    printf("Your camera does not support triggering.\n");
}
```
The trigger mode is set with different parameters for USB and GigE cameras (unfortunally). The TriggerMode property must be querried only once, because the TcamCamera object is passed to the set and get functions.

Use the string parameters "On" and "Off for GigE cameras:

``` C++
TriggerMode->set(cam1,"On"); // Use this line for GigE cameras
``` 
Use the interger parameter 1 and 0 for USB cameras.

``` C++
TriggerMode->set(cam1,1); // Use this line for USB cameras
```
If is is not sure what to use, then the ```ListProperties(cam);``` function shows, which is correct for the currently used camera.

### Getting the Software Trigger Property
The same method as shown at Trigger Mode is used for the software trigger:

``` C++
std::shared_ptr<Property> Softwaretrigger;
try
{
    Softwaretrigger = cam1.get_property("Software Trigger");
}
catch(...)
{
    printf("Your camera does not support software triggering.\n");
}
```

**Attention**: The "Software Trigger" property name can be different on USB and GigE camera. The ```ListProperties(cam);``` is here helpful again for listing available properties.

The SoftwareTrigger property must be querried only once, because the TcamCamera object is passed to the set and get functions.

The software trigger is a so called push property. That means it is pushed like a button. Therefore there is an integer value set to release the trigger pulse. So the following line releases the trigger and the camera is mentioned to sent one image:

``` C++
Softwaretrigger->set(cam1,1);
```
## The Callback
It always is recommened to use a callback, if a camera runs triggered. The callback's function is declared as follows:
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

The sample uses a struct named "CUSTOMDATA".
``` C++
typedef struct
{
    int ImageCounter;       // Counter used for filename generation
    bool ReceivedAnImage;   // indicates a new image handled
    bool busy;              // indicates the callback being busy
    char imageprefix[55];   // Prefix for the image file names.
    cv::Mat frame;          // OpenCV Mat used for image saving.
} CUSTOMDATA;
```
The CUSTOMDATA struct will be instantiate for each camera:
``` C++
CUSTOMDATA CustomData1;  // Customdata for camera 1
CUSTOMDATA CustomData2;  // Customdata for camera 2
```

It is passed to the callback for each camera as follows:
``` C++
// Register a callback to be called for each new frame
cam1.set_new_frame_callback(new_frame_cb, &CustomData1);
cam2.set_new_frame_callback(new_frame_cb, &CustomData2);
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

    if( pCustomData->busy) // Return, if will are busy. Will result in frame drops
       return GST_FLOW_OK;

    pCustomData->busy = true;

    // The following lines demonstrate, how to acces the image
    // data in the GstSample.
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    GstCaps *caps = gst_sample_get_caps(sample);

    str = gst_caps_get_structure (caps, 0);    

    if( strcmp( gst_structure_get_string (str, "format"),"BGRx") == 0)  
    {
        gst_structure_get_int (str, "width", &width);
        gst_structure_get_int (str, "height", &height);

        // Get the image data
        GstMapInfo info;
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        gst_buffer_map(buffer, &info, GST_MAP_READ);
        if (info.data != NULL) 
        {
            // Create the cv::Mat
            pCustomData->frame.create(height,width,CV_8UC(4));
            // Copy the image data from GstBuffer intot the cv::Mat
            memcpy( pCustomData->frame.data, info.data, width*height*4);
            // Set the flag for received and handled an image.
            pCustomData->ReceivedAnImage = true;
        }
        gst_buffer_unmap (buffer, &info);
    }    
    // Calling Unref is important!
    gst_sample_unref(sample);

    // Set our flag of new image to true, so our main thread knows about a new image.
    pCustomData->busy = false;
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

Now all neccessary information for creating the cv::Mat is gathered and the image can be saved in the cv::Mat. 
``` C++
pCustomData->frame.create(height,width,CV_8UC(4));
memcpy( pCustomData->frame.data, info.data, width*height*4);
``` 
cv::Mat::create() wont allocate new memory, if the width, height and bytes per pixels did not change.

Very important: The references to the GStreamer objects must be cleared afterwards:

``` C++
gst_buffer_unmap (buffer, &info);
gst_sample_unref(sample);
``` 

## The Main Program
In the main program the neccessary camera properties for trigger mode and software trigger are querried. In the first step, the trigger mode is disabled and the cameras are started. Since we have the code
``` C++
CustomData1.busy = true;
CustomData2.busy = true;
``` 
the callback function will return without saving images. The cameras are running free now and can be pointed correctly to the scene. The program waits for an Enter-key press. If this key is pressed, the cameras will be set into trigger mode:

``` C++
// Enable trigger mode
TriggerMode->set(cam1,1);
TriggerMode->set(cam2,1);
``` 
Now the live stream stops, until trigger pulses, in our case software trigger pulses will be send to the cameras.

Also the callback function must be advised to save the images:
``` C++
CustomData1.busy = false;
CustomData2.busy = false;
``` 

For simplyfying the trigger and capture process here, there is a simple loop, that runs some times.
Before the software triggers are sent, the flag ReceivedAnImage is reset to false.
``` C++
// Enable trigger mode
CustomData1.ReceivedAnImage = false;
CustomData2.ReceivedAnImage = false;
```
This flag will be set to true in the callback function after a new image was saved in the cv::Mat of the CustomData structs. Now the software triggers are fired.
``` C++
Softwaretrigger->set(cam1,1);
Softwaretrigger->set(cam2,1);
```


The program waits for the images of both cameras by evaluating the ReceivedAnImage flags:
``` C++
// Wait with timeout until we got images from both cameras.
int tries = 50;
while( !( CustomData1.ReceivedAnImage || CustomData2.ReceivedAnImage) && tries >= 0)
{
    usleep(100000); 
    tries--;
}
```

It is a loop with timeout, because it may happens, we do not get a complete iamges from the camera, which results into frame drops. In case there are too many frames not received, try higher initial values for tries.
After the timeout loop, it is checked, whether there images from both cameras and the images are processed.
``` C++
// If there are images received from both cameras, save them.
if(CustomData1.ReceivedAnImage && CustomData1.ReceivedAnImage)
{
    SaveImage(&CustomData1);
    SaveImage(&CustomData2);
}
```
If both ReceivedAnImage flags are true, an image pair was received. It will be saved then. If not, there is a check, which camera did not send images. 

``` C++
// Check, from which camera we may did not receive an image. 
// It is for convinience only and could be deleted.
if(!CustomData1.ReceivedAnImage)
    printf("Did not receive an image from camera 1.\n");
    
if(!CustomData2.ReceivedAnImage)
    printf("Did not receive an image from camera 2.\n");
```

After this saving there is no sleep or other waiting. Therefore, the program captures and saves the images as fast as possible. This speed depends on the camera's frame rate, exposure time and the speed of the computer's hard disc.

## ImageSaving
The CustomData1 and CustomData2 structures contain the cv::Mat with the image, so it can be processed when images where received. Here it the cv::Mat is saved:
``` C++
////////////////////////////////////////////////////////////////////
// Increase the frame count and save the image in CUSTOMDATA
void SaveImage(CUSTOMDATA *pCustomData)
{
    char ImageFileName[256];
    pCustomData->ImageCounter++;

    sprintf(ImageFileName,"%s%05d.jpg", pCustomData->imageprefix,  pCustomData->ImageCounter);
    cv::imwrite(ImageFileName,pCustomData->frame);
}
```
The Image file name is created using the data passed by the pCustomData structure. It contains the image prefix, so it is known, which data comes from cam1 and which from cam2. The image counter is increased, so the image pairs can be found in the directory of the jpeg files.



