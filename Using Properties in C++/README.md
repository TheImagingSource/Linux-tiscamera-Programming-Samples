# Using Properties in C++
This sample shows how to use The Imaging Source camera properties. While running the sample, the live video is shown in a separate window.
The used properties are exposure and gain. (Because if exposure auto is off and gain auto is on, setting a new exposure value will not have a visible effect, except there is more noise in the image by higher gain value set by gain automatic.)

Programming language : C++

## Building
In order to build the sample, open a terminal, enter the sample's directory. Then enter
```
mkdir build
cd build 
cmake ..
make
./using-properties
```

## General Program Flow
The program opens a camera by its serial number and configures a video format. Then the pipeline is started and a live video is shown. For some cameras the automatic properties are available only after the pipeline has been stareted, because they are done in software. e.g. DFK 72BUC02 needs this. The newer 33U and 33G series perform automatics on board, therefore these properties exist even before the pipeline was started.


## Code Documentation
Before starting with the sample search in "main.cpp" search the line which contents
```TcamCamera cam(nnnnn);```
and exchange the serial number there by the serial number of your camera.

The camera properties will be handled by pointers. They are declared as follows:
```C++
    std::shared_ptr<Property> ExposureAuto = NULL;
    std::shared_ptr<Property> ExposureValue = NULL;
    std::shared_ptr<Property> GainAuto = NULL;
    std::shared_ptr<Property> GainValue = NULL;
```
First the camera is opened, configured and started.
```C++
    TcamCamera cam("48610605");
    cam.set_capture_format("BGRx", FrameSize{640,480}, FrameRate{30,1});
    cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));
    cam.start();
```

The pointers are querried:
```C++
    try
    {
        ExposureAuto = cam.get_property("Exposure Auto");
    }
    catch(std::exception &ex)    
    {
        printf("Error %s : %s\n",ex.what(), "Exposure Automatic");
    }
```
If a property does not exist, an exception is thrown by the ```TcamCamera``` class, which is cought here. 
The function ```cam.get_property``` receives as parameter the name of a property. A list of available properties can be shown by a call to ```ListProperties(cam)```, which is implemented in this sample too.

Now proceed with the remaining three properties:
```C++
    try
    {
        ExposureValue = cam.get_property("Exposure");
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
```

On success, the program can enable and disable the automatics for gain and exposure and it can set values for these properties, after their automatics have been disabled. Also the current values can be read. Whether values can be read, while automation is on, depends on the used camera model. The following code shows how to evaluate the value of an automatic setting:
```C++
    if( ExposureAuto != NULL)
    {
        int Auto;
        ExposureAuto->get(cam,Auto);
        if( Auto == 1)
            printf("Current exposure automatic is On.\n");
        else
            printf("Current exposure automatic is Off.\n");
    }
```
The call to ```ExposureAuto->get(cam,Auto);``` returns the current value in the variable auto. Since this is a integer variable, an ```int``` value is passed. Parameters to ```property.get()``` are

| Parameter | Description |
| --- | --- |
| TcamCamera& | The TcamCamera object, on which the property is queried. | 
| value& | The varialbe, that recevies the current value. Different types are allowed for value. |

The value types are
* int
* double
* std::string

Which type must be used depends on the property to be queried. The ```ListProperties(cam)``` shows the type. (BOOL and bool are equal to int).

The follwing code lines show, how to query the numeric value of a property, here it is exposure time:

```C++
    if( ExposureValue != NULL)
    {
        int ExposureTime;
        ExposureValue->get(cam,ExposureTime);
        printf("Current exposure time is %d.\n",ExposureTime);
    }
```

Setting the properties is similar:
```C++
    if( ExposureAuto != NULL){
        ExposureAuto->set(cam,0);
    }
```
The code above disables the exposure automatic by passing a 0 as second parameter. Passing a 1 will enable the automatic again. Setting  new value to exposure time is similar:
```C++
    if( ExposureValue != NULL){
        ExposureValue->set(cam,333);
    }
```

Parameters to ```property.set()``` are

|Parameter | Description |
|---| ---|
|TcamCamera&| The TcamCamera object, on which the property is queried| 
| value | The value to be written for the property.|


The value types are
* int
* double
* std::string

The limits of a property can be shown with ```ListProperties(cam)```. The units, e.g. seconds, ms, dB etc depend on the used camera model.
