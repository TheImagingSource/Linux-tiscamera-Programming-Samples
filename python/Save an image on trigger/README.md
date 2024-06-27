# Save an image on  software trigger
This sample shows how to capture from a triggered camera.
The main purpose of this sample is to use the camera's software trigger for snapping an image. The advantage is, that while no software trigger occurs, the camera does not send images, so there is no USB or GigE data transfer and also there is no CPU load for an ongoing image stream. 

This kind of using the software trigger makes it possible to use many cameras in sequence in a more or less fast way even on weak hardware.

As for all trigger applications, it is recommended to disable the automatics, because the react unexpected, if there are long pauses between two triggers.


Programming language: Python

## Prerequisites
The sample uses the GStreamer modules from the *tiscamera* repository as wrapper around the
[GStreamer](https://gstreamer.freedesktop.org/) code and property handling. It also uses OpenCV for image processing.

## Files
### TIS.py
In this file the "TIS" class is implemented, which is a wrapper around the GStreamer code. 
### Program.py
This is the main file of the sample.

## Basics of TIS class
The device selection is done as follows:

``` Python
if not Tis.select_device():
        quit(0)
``` 

After TIS constrcution the live video can be started
``` Python
Tis.start_pipeline()
``` 
The live video is stopped, with
``` Python
Tis.stop_pipeline()
``` 

If these three lines are used without anything else, no live video will be shown, because the pipeline in the TIS class does not use a display sink (e.g. ximagesink). The images are only saved in a ring buffer in memory.

## Triggering
If a camera is triggered, it is always recommended to use a callback function for received images. This avoids a blocking wait for images. The callback function looks like follows:
``` Python
def on_new_image(tis, userdata):
        '''
        Callback function, which will be called by the TIS class
        :param tis: the camera TIS class, that calls this callback
        :param userdata: This is a class with user data, filled by this call.
        :return:
        '''
        # Avoid being called, while the callback is busy
        if userdata.busy is True:
                return

        userdata.busy = True
        userdata.newImageReceived = True
        userdata.image = tis.Get_image()
        userdata.busy = False
``` 
The parameters are a reference to the calling TIS class and a class passed to userdata. The user data should be declared as follows:
``` Python
class CustomData:
        ''' Example class for user data passed to the on new image callback function
        '''

        def __init__(self, newImageReceived, image):
                self.newImageReceived = newImageReceived
                self.image = image
                self.busy = False
``` 

The callback function sample copies the received image into to ```image``` member of the ```CustomData``` and sets the ```newImageReceived``` flag to true. That will be evaluated by the main program. Of course, there can be done a lot more image processing in the callback, if needed. (I am currently not too sure, whether the buffer image wont be overwritten.)

The ```CustomData``` instance is created as:
``` Python
CD = CustomData(False, None)
```
The prerequisites for the callback are finished, now the TIS class must get this information. Before we start the live video, the callback is passed to the TIS class:
``` Python
Tis.Set_Image_Callback(on_new_image, CD)
```
## Camera properties
Camera automatics should be disabled, while the camera runs in trigger mode. Therefore some properties must be set. This is done with 
``` Python
Tis.set_property(Propertyname, Propertyvalue)
```
A list of properties can be shown by a call to 
``` Python
Tis.list_properties()
```
### Trigger Mode
``` Python
# Tis.set_property("Trigger Mode", "On") # Use this line for GigE cameras
Tis.set_property("Trigger Mode", True)
```
Unfortunately the Trigger Mode differs between USB and GigE cameras. The line above enables the trigger mode.
Also there is another condition: 
```Tis.start_pipeline()``` does not return, if the Trigger Mode is enabled. Therefore, the Trigger Mode is disabled, then the pipeline is started and the Trigger Mode is enabled again. That is also the reason, why the ```CustomeData.busy``` flag is set to ```True``` before the pipeline is stated. It avoid the callback doing anything, while the trigger is not enabled. 
So the start sequence is:
``` Python
Tis.set_property("Trigger Mode", False)
CD.busy = True 
Tis.start_pipeline()
Tis.set_property("Trigger Mode", True)
CD.busy = False
```

### Software Trigger
The software trigger lets the camera expose one image and send it to the computer. It does the same, as a hardware trigger would do. Therefore, this sample will work with hardware trigger as well, only the software trigger property must not be set. The software trigger is set with following line of code:
``` Python
Tis.set_property("Software Trigger",1) # Send a software trigger
```
That is all, because the function triggers only an action in the camera.

### White Balance, Exposure and Gain
As mentioned above,the automatics in the camera should be disabled, while the camera is in trigger mode.
``` Python
# White Balance properties, in case a color camera is in use:
Tis.set_property("Whitebalance Auto", False)
Tis.set_property("Whitebalance Red", 64)
Tis.set_property("Whitebalance Green", 50)
Tis.set_property("Whitebalance Blue", 64)

# Query the gain auto and current value :
print("Gain Auto : %s " % Tis.get_property("Gain Auto").value)
print("Gain : %d" % Tis.get_property("Gain").value)

# Check, whether gain auto is enabled. If so, disable it.
if Tis.get_property("Gain Auto").value :
        Tis.set_property("Gain Auto",False)
        print("Gain Auto now : %s " % Tis.get_property("Gain Auto").value)

Tis.Set_Property("Gain",0)

# Now do the same with exposure. Disable automatic if it was enabled
# then set an exposure time.
if Tis.get_property("Exposure Auto").value :
        Tis.set_property("Exposure Auto", False)
        print("Exposure Auto now : %s " % Tis.get_property("Exposure Auto").value)

Tis.Set_Property("Exposure", 3000)
```

## Mainloop
The main loop fires the software trigger and processes the image returned in ```CustomData```
``` Python
try:
        while lastkey != 27 and error < 5:
                time.sleep(1)
                Tis.set_property("Software Trigger",1) # Send a software trigger

                # Wait for a new image. Use 10 tries.
                tries = 10
                while CD.newImageReceived is False and tries > 0:
                        time.sleep(0.1)
                        tries -= 1

                # Check, whether there is a new image and handle it.
                if CD.newImageReceived is True:
                        CD.newImageRecevied = False
                        cv2.imshow('Window', CD.image)
                else:
                        print("No image received")

                lastkey = cv2.waitKey(10)

except KeyboardInterrupt:
        cv2.destroyWindow('Window')
```
Of course this looks like polling for an image, which in fact it is. Using software trigger is a nice way to make sure an image was received for each software trigger. Also `cv2.imshow` did not work as expected in the callback.

However, if there is hardware trigger used, then e.g image processing or image saving can be done in the callback function. Then there would be no code in the main loop, except user input handling.




