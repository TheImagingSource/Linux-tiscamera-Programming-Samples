'''
This very  sample shows, how read camera configurations from a json file
and create for each camera in the file a TIS.TIS() camera object.

It also shows, how to set the camera parameters

The created objects
are stored in a list.

After the list has been created, all cameras are started for a live video stream and
ended after a key was hit.
'''
import json
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
    '''
    def __init__(self, properties):
        super().__init__()
        self.properties = properties

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


with open("cameras2.json") as jsonFile:
    cameraconfigs = json.load(jsonFile)
    jsonFile.close()

cameras = list()

for cameraconfig in cameraconfigs['cameras']:
    print("Creating camera serial {}".format( cameraconfig['serial']))

    camera = CAMERA(cameraconfig['properties'])

    camera.open_device(cameraconfig['serial'],
                       cameraconfig['width'],
                       cameraconfig['height'],
                       cameraconfig['framerate'],
                       TIS.SinkFormats[cameraconfig['pixelformat']].value,
                       True)
    cameras.append(camera)

for camera in cameras:
    camera.set_property("TriggerMode", "Off")
    camera.start_pipeline()
    camera.applyProperties()

key = input("Enter to end program")

for camera in cameras:
    camera.stop_pipeline()
