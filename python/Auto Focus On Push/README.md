# Auto Focus On Push in Python
This sample shows how to control the lens of a The Imaging Source auto focus camera, e.g. 
* DFK AFU420-L62
* DFK Z12GP013 
* DFK AFUJ006.
* DxK AFUX178-M12
* DxK 72AUC02-F
* and many more.

The cameras have different capabilities, so the USB cameras have no zoom lens, but the sample shows also, how to use the zoom of GigE cameras.

Programming language : Python

## Prerequisites
The sample uses the GStreamer modules from the [tiscamera](https://github.com/TheImagingSource/tiscamera) repository as wrapper around the
[GStreamer](https://gstreamer.freedesktop.org/) code and property handling. It also uses OpenCV for image processing.

## Files
### TIS.py
In this file the "TIS" class is implemented, which is a wrapper around the GStreamer code. 
### Program.py
This is the main file of the sample. It opens the camera, configures the video format. The live video is shown in an own window on the desktop. 
A little menu is shown. Following strings  can be entered, followed by enter key:

| Input string | Expected result |
| --- | --- |
| f | Run the auto focus one push |
| f+ | Increase focus by 10 |
| f- | Devrease focus by 10 |
| z+ | Increas zoom by 1, GigE cameras only |
| z- | Decrease zoom by 1, GigE cameras only |
| q | Quit program |



 The code is
``` Python
# Add path to python-common/TIS.py to the import path
import sys
sys.path.append("../python-common")

import TIS
if not Tis.select_device():
        quit(0)
Tis.set_sink_format(TIS.SinkFormats.BGRA) #Use BGRA format in the Memory
Tis.start_pipeline()  # Start the pipeline so the camera streams

while True:
        key = raw_input("f : Auto Focus\nf+ : increase focus\nf- : decrease focus\nz+ : increase zoom\n"
                        "z- : decrease zoom\nq : quit\nPlease enter:")
        if key == "f":
                Tis.set_property("Focus Auto", True)

        if key == "f+":
                focus = Tis.get_property("Focus").value
                focus = 10
                Tis.set_property("Focus", focus)

        if key == "f-":
                focus = Tis.get_property("Focus").value
                focus -= 10
                Tis.set_property("Focus", focus)

        if key == "z+":
                zoom = Tis.get_property("Zoom").value
                zoom += 1
                Tis.set_property("Zoom", zoom)

        if key == "z-":
                zoom = Tis.get_property("Zoom").value
                zoom -= 1
                Tis.set_property("Zoom", zoom)

        if key == "q":
                break

Tis.stop_pipeline()
print('Program ends')

``` 

## Detailed documentation
In the constructur of TIS an existing The Imaging Source camera can be selected.

``` Python
if not Tis.select_device():
        quit(0)
``` 

After the TIS class instantiation the live video can be started
``` Python
Tis.start_pipeline()
``` 
The main loop can be started. It implements the main menu.

### Auto Focus
The cameras do not have a permanent auto focus, because the sharpness is estimated on an image. Then the a new focus value is set and the sharpness of the next image is compared with the previous one. This repeats, until a sharp image is achieved. 
Running the auto focus on push is simple. Only a True must be set to the "Focus auto" property:
``` Python
Tis.set_property("Focus Auto", True)
``` 
Currently it is unknown how long the algorithm will run. 

The other property functions are so far simple. Getting the current value of a property and change it accordingly to the input from menu.


``` Python
Tis.stop_pipeline()
cv2.destroyAllWindows()
print('Program ends')
``` 





