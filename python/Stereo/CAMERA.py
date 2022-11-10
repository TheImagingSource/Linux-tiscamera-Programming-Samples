import json
import time
import cv2
import sys
sys.path.append("../python-common")
import TIS


class CAMERA(TIS.TIS):
    '''
    A camera class is needed for having the relation of camera and its 
    json properties, because some properties can be set only, after
    the live stream has been started.
    This class is inherited from TIS.TIS class.
    These properties all the ones, that are created by software:
    - white balance automatic, exposure automatic and gain automatic for older USB 2.0 cameras
    - all properties made by tcam-dutils module, e.g. Tone Mapping.

    The class has a new attribute "triggerproperty". It is configured in the json file and contains the 
    property name and the values of enable and disable trigger mode. this is necessary, because USB v4l2
    and GigEVision / USB3Vision use different properties and values for enable trigger mode.

    The CAMERA class has a saveImage ,ethod, which saves the last received image. The file name is
    created from the imageprefix specified in the cameras.json file and a running nummer.
    The saveImage method is called from on_new_image() callback, which was passed to the 
    appsink in the pipeline in the TIS.TIS() base class. 
    '''
    def __init__(self, properties, imageprefix):
        '''
        Constructor of the CAMERA class
        :param properties: JSON object, that contains the list of to set properites
        :param triggerproperty: JSON object, which contains the trigger property name and the enable and disable values
        :param imageprefix: Used to create the file names of the images to be saved.
        '''
        super().__init__()
        self.properties = properties 
        self.imageprefix = imageprefix
        self.busy = False
        self.imageCounter = 0


    def applyProperties(self):
        '''
        Apply the properties in self.properties to the used camera
        The properties are applied in the sequence they are saved
        int the json file.
        Therefore, in order to set properties, that have automatiation
        the automation must be disabeld first, then the value can be set.
        '''
        for prop in self.properties:
            try:
                self.set_property(prop['property'],prop['value'])
            except Exception as error:
                print(error)


    def enableTriggerMode(self, onoff):
        '''
        Enable or disable the trigger mode
        :param bool onoff: "On" or "Off"
        '''
        try:
            self.set_property("TriggerMode", onoff)
        except Exception as error:
            print(error)


    def saveImage(self):
        '''
        Save the last received image
        '''
        # Avoid being called, while the callback is busy
        if self.busy is True:
            return

        self.busy = True
        self.imageCounter += 1
        
        imagefilename = "{0}_{1:04d}.jpg".format(self.imageprefix, self.imageCounter)
        print(imagefilename)
        image = self.get_image()
        cv2.imwrite(imagefilename, image)
        self.busy = False
