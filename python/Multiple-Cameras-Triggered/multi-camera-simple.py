'''
This very simple sample shows, how read camera configurations from a json file 
and create for each camera in the file a TIS.TIS() camera object. The created objects
are stored in a list.

After the list has been created, all cameras are started for a live video stream and
ended after a key was hit.
'''

# Add path to python-common/TIS.py to the import path
import sys
import json
sys.path.append("../python-common")
import TIS


with open("cameras1.json") as jsonFile:
    cameraconfigs = json.load(jsonFile)
    jsonFile.close()

cameras = list()

for cameraconfig in cameraconfigs['cameras']:
    print("Creating camera serial {}".format( cameraconfig['serial']))

    camera = TIS.TIS()
    camera.open_device(cameraconfig['serial'],
                       cameraconfig['width'],
                       cameraconfig['height'],
                       cameraconfig['framerate'],
                       TIS.SinkFormats[cameraconfig['pixelformat']].value,
                       True)
    cameras.append(camera)

for camera in cameras:
    camera.start_pipeline()

key = input("Hit Enter key to end program")

for camera in cameras:
    camera.stop_pipeline()
