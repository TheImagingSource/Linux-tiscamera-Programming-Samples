# Snap an Image and Convert to OpenCV
This sample shows how to capture an image from a video stream and convert it to an OpenCV image. An example image processing is done, the result is displayed.

Programming language : Python

## Prerequisites
The sample uses the GStreamer modules from the [tiscamera](https://github.com/TheImagingSource/tiscamera) repository as wrapper around the
[GStreamer](https://gstreamer.freedesktop.org/) code and property handling. It also uses OpenCV for image processing.

## Files
### TIS.py
In this file the "TIS" class is implemented, which is a wrapper around the GStreamer code. 
### Program.py
This is the main file of the sample. The code is
``` Python
import sys
sys.path.append("../python-common")
import cv2
import numpy as np
import TIS

Tis = TIS.TIS()
# The next line is for selecting a device, video format and frame rate.
if not Tis.select_device():
        quit(0)
        
Tis.start_pipeline()  # Start the pipeline so the camera streams

print('Press Esc to stop')
lastkey = 0

cv2.namedWindow('Window')  # Create an OpenCV output window

kernel = np.ones((5, 5), np.uint8)  # Create a Kernel for OpenCV erode function

while lastkey != 27:
        if Tis.snap_image(1) is True:  # Snap an image with one second timeout
                image = Tis.get_image()  # Get the image. It is a numpy array
                image = cv2.erode(image, kernel, iterations=5)  # Example OpenCV image processing
                cv2.imshow('Window', image)  # Display the result

        lastkey = cv2.waitKey(10)

# Stop the pipeline and clean ip
Tis.stop_pipeline()
cv2.destroyAllWindows()
print('Program ends')
``` 

## Detailed documentation

After the TIS class constrcution the live video can be started
``` Python
Tis.start_pipeline()
``` 
Now some initialization for the main loop an the image processing is done.
``` Python
lastkey = 0
cv2.namedWindow('Window')  # Create an OpenCV output window
kernel = np.ones((5, 5), np.uint8)  # Create a Kernel for OpenCV erode function
``` 
The main loop can be started. It will run, until the Esc key is pressed on the OpenCV window. In the mainloop an image is snapped

``` Python
if Tis.snap_image(1) is True:
``` 
The ```Tis.Snap_image(timeout)``` waits for an image for the in timout passed time in seconds. The value can be e.g float 0.5 for half a second. The funtion returns True, if there was a new image and False, if no image was received.
If there was a new image, the call to 
``` Python
                image = Tis.get_image()  # Get the image. It is a numpy array
``` 
returns the image in the ```image``` varialbe. This can be used for furhter image processing and display. In the sample an erosion is done:
``` Python
                image = cv2.erode(image, kernel, iterations=5)  # Example OpenCV image processing
                cv2.imshow('Window', image)  # Display the result
``` 

If the main loop is ended by the Esc key press, the pipeline is cleaned up and the program ends.
``` Python
Tis.stop_pipeline()
cv2.destroyAllWindows()
print('Program ends')
``` 





