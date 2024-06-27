# Using Multiple Cameras in Python
These samples show how to use multiple cameras in a Python program. The main goals are
* Use a JSON file for the camera confiruation
* Make the Python code as camera independent as possible
* Configure all cameras individually using the JSON file
* Save images from each camera in Trigger Mode
* Work around the property interface differences between GigE and v4l2 cameras

## Step 1: Using JSON for cameras
The first step shows, how to read the cameras1.json file, create a TIS.TIS object for each camera configured in there and show the live video stream.

The cameras1.json configures two cameras. The serial number for identifcation, the pixel format of the images in memory of computer, width and height and at least the frame rate:
```JSON
{
    "cameras":[
        {
            "serial": "46610159",
			"format" : "video/x-raw",
			"pixelformat" : "BGRx",
			"width" : 640,
			"height" : 480,
            "framerate":"30/1"
        },
        {
            "serial": "41910044",
			"format" : "video/x-raw",
			"pixelformat" : "BGRx",
			"width" : 640,
			"height" : 480,
            "framerate":"30/1"       
        }
    ]
}
```
The "cameras" tag starts an array. There can be added as many as available cameras. 

The Python code for reading in `multi-camera-simple.py` is quite short:

```Python 
import TIS
import json

with open("cameras1.json") as jsonFile:
    cameraconfigs = json.load(jsonFile)
    jsonFile.close()

cameras = list()

for cameraconfig in cameraconfigs['cameras']:
    print( "Creating camera serial {}".format( cameraconfig['serial']) )

    camera = TIS.TIS()
    camera.open_device(cameraconfig['serial'],
                                cameraconfig['width'],
                                cameraconfig['height'],
                                cameraconfig['framerate'],
                                TIS.SinkFormats.fromString(cameraconfig['pixelformat']),
                                True)
    cameras.append(camera)


for camera in cameras:
    camera.start_pipeline()

key = input("Enter to end program")

for camera in cameras:
    camera.stop_pipeline()
```
First of all the `cameras1.json` is opened and a `cameraconfigs` JSON object is created. That is read for each camera. A `TIS.TIS()` object is instatiated, the camera is opened and the object is added to a list. 

All cameras are started in a simple loop:
```Python
for camera in cameras:
    camera.start_pipeline()
```
And also stopped:
```Python
for camera in cameras:
    camera.stop_pipeline()
```
The Python script shows a live video window of all cameras. 

## Step 2: Setting Camera Properties
In the second step camera properties are added to the json file:
```JSON
{
    "cameras":[
        {
            "serial": "28120175",
			"format" : "video/x-raw",
			"pixelformat" : "BGRx",
			"width" : 640,
			"height" : 480,
            "framerate":"30/1",
            "properties":
            [
                {"property":"Brightness",  "value":20},
                {"property":"Exposure Auto","value":"Off"},
                {"property":"Exposure", "value":33000},
                {"property":"Gain Auto", "value":"Off"},
                {"property":"Gain", "value":0},
                {"property":"Whitebalance Auto", "value":"Off"},
                {"property":"Balance Ratio Selector", "value":"Red"},
                {"property":"Balance Ratio", "value":1.0},
                {"property":"Balance Ratio Selector", "value":"Blue"},
                {"property":"Balance Ratio", "value":1.0},
                {"property":"Balance Ratio Selector", "value":"Green"},
                {"property":"Balance Ratio", "value":1.0}

            ]            
        },
        {
            "serial": "41910044",
			"format" : "video/x-raw",
			"pixelformat" : "BGRx",
			"width" : 640,
			"height" : 480,
            "framerate":"30/1",
            "properties":
            [
                {"property":"Brightness",  "value":20},
                {"property":"Exposure Auto","value":false},
                {"property":"Exposure", "value":700},
                {"property":"Gain Auto", "value":false},
                {"property":"Gain", "value":0},
                {"property":"Whitebalance Auto", "value":false},
                {"property":"Whitebalance Red", "value":128},
                {"property":"Whitebalance Green", "value":64},
                {"property":"Whitebalance Blue", "value":128}
            ]                        
        }
    ]
}
```
The `cameras2.json` file configures two cameras. A DFK Z30GP031 and a DFK 33GX178.

The DFK Z30GP031 is not very powerful, therefore all automatics are done in software. The DFK 33GX178 is more powerful, nearly all properties are done in the camera. Of course, the tcam-dutils properties like Tone Mapping are still done in software and available after the live stream has been started.

As a consequence, the properties of these cameras are defined differently, in particular white balance. The DFK Z30GP031 has the set
```JSON
                {"property":"Whitebalance Auto", "value":false},
                {"property":"Whitebalance Red", "value":128},
                {"property":"Whitebalance Green", "value":64},
                {"property":"Whitebalance Blue", "value":128}
```                
These properties are implemented by the tcamwhitebalance module. This module is used for all cameras, that have no own computing power, e.g. DFK 22, DFK 42, DFK 72, DFK 27U, DFK 23G.

The DFK 33GX178 uses the GigEVision white balance properties that looks like follows:
```JSON
                {"property":"Whitebalance Auto", "value":"Off"},
                {"property":"Balance Ratio Selector", "value":"Red"},
                {"property":"Balance Ratio", "value":2.0},
                {"property":"Balance Ratio Selector", "value":"Blue"},
                {"property":"Balance Ratio", "value":2.0},
                {"property":"Balance Ratio Selector", "value":"Green"},
                {"property":"Balance Ratio", "value":1.0}
```                
We recommend to use tcam-capture for checking the available camera properties and adjust the values to the needs of the task. 

The Python script `multi-camera-parameters.py` implements a new class inherited from TIS.TIS class. This is necessary for setting the properties after the live stream has been started. Therefore, the JSON object, which contains the properties is an attribute of the new class:
```Python 
for cameraconfig in cameraconfigs['cameras']:
    camera = CAMERA(cameraconfig['properties'])
    (...)
```
The new `CAMERA` class has a method for applying the properties from the Json object to the connected camera:
```Python 
    def applyProperties(self):
        for property in self.properties:
            self.set_property(property['property'], property['value'])
```
As one can see, thist is very simple and short, because the JSON object handles the correct property value types already. But this ports the incoherent camera properties interface of tiscamera for v4l2 and GigE cameras to the JSON file "cameras1.json". Which has the advantage, that one does not need to handle that be many "if.." statements in the source code.

The properties are applied in the sequence they are saved in the json file.
Therefore, in order to set properties, that have automatation the automation must be disabled first, then the value can be set. "Exposure Auto" must be disabled, before an "Exposure" values can be set. Therefore, "Exposure Auto" is in the line before "Exposure" in the `cameras2.json` file. Same goes for Gain and White Balance. 

The camera start sequence with setting properties is shown by following code:
```Python
for camera in cameras:
    camera.set_property("Trigger Mode", "Off")
    camera.start_pipeline()
    camera.applyProperties()
```
The line 
```Python
camera.set_property("Trigger Mode", "Off") 
```
makes sure that the pipeline can be started. It will not start, if a camera is trigger mode, because it needs a frame running through the pipeline once in order to configure all GStreamer modules.

That works for GigE cameras, USB v4l2 cameras will use `False` instead of `"Off"`. (So there should be "Triggermodeon/off" definition in the cameras.json too, but this is not implemented yet, that comes in Step 3.)

## Step 3: Saving Images on Trigger
In this step the Pyhton script configures the cameras in trigger mode, sets a callback and saves images if the cameras are triggered.

The `cameras3.json` is enhanced with a definition of the trigger mode for each camera:
```JSON
            "trigger":
                {
                    "property":"Trigger Mode",
                    "on" : "On",
                    "off" : "Off"
                }
```
This is for GigE cameras. v4l2 cameras will use:
```JSON
            "trigger":
                {
                    "property":"Trigger Mode",
                    "on" : true,
                    "off" : false
                }
```
The definition is passed as new attribute to the `CAMERA` class:
```Python
for cameraconfig in cameraconfigs['cameras']:
    camera = CAMERA(cameraconfig['properties'],cameraconfig['trigger'],cameraconfig['imageprefix'])
```
The `CAMERA` class got a new method for Trigger Mode enable:
```Python
    def enableTriggerMode(self, onoff):
        if onoff == True:
            self.Set_Property(self.triggerproperty['property'], self.triggerproperty['on'])
        else:
            self.Set_Property(self.triggerproperty['property'], self.triggerproperty['off'])
```
It is used as follows:
```Python
for camera in cameras:
    camera.enableTriggerMode( False )
```
Now all camera models, regardless whether v4l2 or GigE can be used in the same Python script without changing the script at all.

The `cameras3.json` contains a new tag "imageprefix" for each camera too.
```Python
"imageprefix" : "left"
```
It is used for creating the file name of each recevied image. Of course, each camera configured should have a own image prefix. The prefix is passed to the `CAMERA` constructor too.

When creating the `CAMERA` objetcs, a new image callback is passed to the newly created objects:
```Python
for cameraconfig in cameraconfigs['cameras']:
(...)    
    camera = CAMERA(cameraconfig['properties'],cameraconfig['trigger'],cameraconfig['imageprefix'])
(...)   
    camera.Set_Image_Callback(on_new_image, None)                                
``` 
Only one callback function is needed. There must not be an exta function for each `CAMERA` object. 

The callback contains only one line of code:
```Python
def on_new_image(camera, userdata):
    camera.saveImage()
```

The callback function receives the camera object that called it. So the new image will be handledin the CAMERA class. It is saved by using the OpenCV imwrite() function:
```Python
    def saveImage(self):
        '''
        Save the last received image
        '''
        # Avoid being called while the callback is busy
        if self.busy is True:
            return

        self.busy = True
        self.imageCounter += 1
        
        imagefilename = "{0}_{1:04d}.jpg".format(self.imageprefix, self.imageCounter)
        print(imagefilename)
        image = self.get_image()
        cv2.imwrite(imagefilename, image)
        self.busy = False
```
The `self.busy` flag us used in order to avoid, that the method is called twice (which should not happen at all). It is also used to avoid saving images, while the pipeline is started. On pipeline start the trigger mode has to be turned off, because one frame must travel through the pipeline first. Therefore, the `camera.busy` flag is set on pipeline start to `True`. Will be seen later.

The `saveImage` method creates the file name, pulls the image and saves it to hard disc.

The start process for all cameras is:
```Python
for camera in cameras:
    camera.enableTriggerMode( False )
    camera.busy = True
    camera.start_pipeline()
    camera.applyProperties()
    camera.enableTriggerMode( True )
```
The trigger mode is disabled and as mentioned above the `camera.busy` flag is set to `True`. Now the pipeline can be started, a frame can travel though it without being processed in the callback. Then the trigger mode is enabled again. Each start of a pipeline takes some time. Therefore, after the above loop has ended, the program waits:
```Python
time.sleep(0.5)
```
This makes sure, there are no more frames in the pipeline. Now the `camera.busy` flags are set to `False` and the cameras are ready to receive triggers:
```Python
for camera in cameras:
    camera.busy = False
```

At the end of the program the trigger mode is disabled and the pipelines are stopped:
```Python
for camera in cameras:
    camera.enableTriggerMode( False )
    camera.stop_pipeline()
```

### Optional: PTP for Synchronization.
This works on GigE 33G cameras with firmware 2530 or higher only. These cameras can be synchronized and "triggered" internally. Unfortunatelly the part with the `schedulertime` does not work correctly, because this property needs an unsigned int64 data type, which is not supported by tiscamera currently. But the cameras are nearly synchronized.

The cameras must be in Trigger Mode and are started already.
```Python
schedulertime = cameras[0].Get_Property("ActionSchedulerTime").value 
schedulertime += 2000000 # Add two seconds so there is enough time to configure all cameras.
for camera in cameras:
    camera.busy = False    

    camera.set_property("PtpEnable",True)
    camera.set_property("ActionSchedulerTime",schedulertime)
    camera.set_property("ActionSchedulerInterval",1000000) # one image every second.
    camera.set_property("ActionSchedulerCommit",1)
```
Now all cameras send an image every second nearly at the same point of time.

The camera stop must be enhanced with an Action Scheduler Cancel call:
```Python
for camera in cameras:
    camera.set_property("ActionSchedulerCancel",1)
    camera.enableTriggerMode( False )
    camera.stop_pipeline()
```
