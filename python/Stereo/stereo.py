'''

After the list has been created, all cameras are started for a live video stream and
ended after a key was hit.
'''
from gi.repository import Gst
import json
import time
import cv2
import TIS
import CAMERA
import numpy as np



class CustomData:
        ''' Example class for user data passed to the on new image callback function
        '''
        def __init__(self):
                self.imageleft = None
                self.imageright = None
                self.dummy = None
                self.busy = False

def on_new_image(camera, userdata):
    if camera.imageprefix == "left":
        userdata.imageleft = camera.Get_image()
    else:
        userdata.imageright = camera.Get_image()


def createAnaglyphimage( images ):
    if images.dummy is None:
        if images.imageleft is None:
            size = images.imageright.shape[0], images.imageright.shape[1], 1
        else:
            size = images.imageleft.shape[0], images.imageleft.shape[1], 1
        images.dummy = np.zeros(size, dtype=np.uint8)
       
    anaglyph = cv2.merge((images.dummy, images.imageright, images.imageleft) )    
    cv2.imshow('Stereo', anaglyph)


Gst.init([])


CD = CustomData()

with open("cameras.json") as jsonFile:
    cameraconfigs = json.load(jsonFile)
    jsonFile.close()

cameras = list()

for cameraconfig in cameraconfigs['cameras']:
    camera = CAMERA.CAMERA(cameraconfig['properties'],cameraconfig['trigger'],cameraconfig['imageprefix'])
    
    camera.openDevice(cameraconfig['serial'],
                                cameraconfig['width'],
                                cameraconfig['height'],
                                cameraconfig['framerate'],
                                TIS.SinkFormats.fromString(cameraconfig['pixelformat']),
                                False)

    camera.Set_Image_Callback(on_new_image, CD)                                
    cameras.append(camera)


# cameras[0].List_Properties()
CD.busy = True

for camera in cameras:
    camera.enableTriggerMode( False )
    camera.busy = True
    camera.Start_pipeline()
    camera.applyProperties()
    camera.enableTriggerMode( True )


# Streams have been started.
# Wait for the pipelines become empty
time.sleep(0.5)

y = cameras[0].Get_Property("Offset Y").value
x1= cameras[0].Get_Property("Offset X").value
x2= cameras[1].Get_Property("Offset X").value

error = 0
print('Press Esc to stop')
lastkey = 0
cv2.namedWindow('Stereo',cv2.WINDOW_NORMAL)


try:
    while lastkey != 27 and error < 5:
        CD.imageleft = None
        CD.imageright = None

        for camera in cameras:
            camera.Set_Property("Software Trigger",1)

        # Wait for a new image. Use 10 tries.
        tries = 10
        while CD.imageleft is None and CD.imageright is None and tries > 0:
                time.sleep(0.1)
                tries -= 1

        # Check, whether there is a new image and handle it.
        if CD.imageleft is not None and CD.imageright is not None:
            error = 0
            createAnaglyphimage(CD)
        else:
            error += 1

        if lastkey == 119:            
            y -= 2
            cameras[0].Set_Property("Offset Y",y)
            print("y  {} x1 {}  x2 {}".format( y, x1 ,x2 ) )

        if lastkey == 115:
            y += 2
            cameras[0].Set_Property("Offset Y",y)
            print("y  {} x1 {}  x2 {}".format( y, x1 ,x2 ) )

        if lastkey == 97:
            x1 -= 2
            cameras[0].Set_Property("Offset X",x1)
            x2 += 2
            cameras[1].Set_Property("Offset X",x2)
            print("y  {} x1 {}  x2 {}".format( y, x1 ,x2 ) )

        if lastkey == 100:
            x1 += 2
            cameras[0].Set_Property("Offset X",x1)
            x2 -= 2
            cameras[1].Set_Property("Offset X",x2)
            print("y  {} x1 {}  x2 {}".format( y, x1 ,x2 ) )

        lastkey = cv2.waitKey(1)

except KeyboardInterrupt:
    cv2.destroyWindow('Stereo')

for camera in cameras:
    #camera.Set_Property("ActionSchedulerCancel",1)
    camera.enableTriggerMode( False )
    camera.Stop_pipeline()

