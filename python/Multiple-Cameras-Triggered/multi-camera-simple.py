'''
This very simple sample shows, how read camera configurations from a json file 
and create for each camera in the file a TIS.TIS() camera object. The created objects
are stored in a list.

After the list has been created, all cameras are started for a live video stream and
ended after a key was hit.
'''
import TIS
import json

Gst.init([])

with open("cameras1.json") as jsonFile:
    cameraconfigs = json.load(jsonFile)
    jsonFile.close()

cameras = list()

for cameraconfig in cameraconfigs['cameras']:
    print( "Creating camera serial {}".format( cameraconfig['serial']) )

    camera = TIS.TIS()
    camera.openDevice(cameraconfig['serial'],
                                cameraconfig['width'],
                                cameraconfig['height'],
                                cameraconfig['framerate'],
                                TIS.SinkFormats.fromString(cameraconfig['pixelformat']),
                                True)
    cameras.append(camera)


for camera in cameras:
    camera.Start_pipeline()

key = input("Enter to end program")

for camera in cameras:
    camera.Stop_pipeline()




