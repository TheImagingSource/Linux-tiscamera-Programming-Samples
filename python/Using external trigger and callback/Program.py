import sys
sys.path.append("../python-common")

import cv2 as cv2
import numpy as np
import os
import TIS
import time
from collections import namedtuple


# This sample shows, how to get an image in a callback and use trigger 
# needed packages:
# pyhton-opencv
# pyhton-gst-1.0
# tiscamera


class CustomData:
    ''' Example class for user data passed to the on new image callback function
        It is used for an image counter only. Also for a busy flag, so the callback
        is not called, while a previons callback call is still active.
    '''
    def __init__(self, ):
        self.imagecounter = 0
        self.busy = False

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
    image = tis.Get_image()

    # Doing a sample image processing
    kernel = np.ones((5, 5), np.uint8)
    image = cv2.erode(image, kernel, iterations=5) 

    # Create a file name with a running number:
    userdata.imagecounter += 1;
    filename = "./image{:04}.jpg".format(userdata.imagecounter)
    
    # Save the image as jpeg. Instead of saving, there could be an
    # image processing.
    cv2.imwrite(filename, image)
    userdata.busy = False


Tis = TIS.TIS()
# The following line opens and configures the video capture device.
#Tis.openDevice("00001234", 640, 480, "30/1",TIS.SinkFormats.BGRA, True)

# The next line is for selecting a device, video format and frame rate.
if not Tis.selectDevice():
    quit(0)

# Create an instance of the CustomData class
CD = CustomData()

# Set the callback function
Tis.Set_Image_Callback(on_new_image, CD)

Tis.Set_Property("TriggerMode", "On")  # Use this line for GigE cameras

# Remove comment below in oder to get a propety list.
# Tis.List_Properties()

# In case a color camera is used, the white balance automatic must be
# disabled, because this does not work good in trigger mode
try:
    Tis.Set_Property("BalanceWhiteAuto", "Off")
    Tis.Set_Property("BalanceWhiteRed", 1.2)
    Tis.Set_Property("BalanceWhiteGreen", 1.0)
    Tis.Set_Property("BalanceWhiteBlue", 1.4)
except Exception as error:
    print(error)

try:
    # Disable gain and exposure automatic
    Tis.Set_Property("GainAuto", "Off")
    Tis.Set_Property("Gain",0)
    Tis.Set_Property("ExposureAuto", "Off")
    Tis.Set_Property("ExposureTime", 24000)
except Exception as error:
    print(error)

Tis.Start_pipeline()
Tis.Set_Property("TriggerMode", "On")  # Use this line for GigE cameras

# The main loop does nothing, except waiting for an end.
while True:
    key = input("q : quit\nPlease enter:")
    if key == "q":
        break

Tis.Stop_pipeline()
print('Program end')