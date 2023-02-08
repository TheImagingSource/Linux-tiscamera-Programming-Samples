# Add path to python-common/TIS.py to the import path
import cv2
import numpy as np
import os
import sys
import time
sys.path.append("../python-common")

import TIS


# This sample shows, how to get an image in a callback and use trigger or software trigger
# needed packages:
# pyhton-opencv
# pyhton-gst-1.0
# tiscamera

class CustomData:
    ''' Example class for user data passed to the on new image callback function
    '''
    def __init__(self, newImageReceived, image):
        self.newImageReceived = newImageReceived
        self.image = image
        self.busy = False

CD = CustomData(False, None)

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
    userdata.image = tis.get_image()
    userdata.busy = False

Tis = TIS.TIS()

# The following line opens and configures the video capture device.
#Tis.open_device("03020073", 640, 480, "15/1",TIS.SinkFormats.BGRA, True)

# The next line is for selecting a device, video format and frame rate.
if not Tis.select_device():
    quit(0)

#Tis.list_properties()
Tis.set_image_callback(on_new_image, CD)

Tis.set_property("TriggerMode", "On")
Tis.start_pipeline()

# Remove comment below in oder to get a propety list.
# Tis.list_properties()

# In case a color camera is used, the white balance automatic must be
# disabled, because this does not work good in trigger mode
try:
    Tis.set_property("BalanceWhiteAuto", "Off")
    Tis.set_property("BalanceWhiteRed", 1.2)
    Tis.set_property("BalanceWhiteGreen", 1.0)
    Tis.set_property("BalanceWhiteBlue", 1.4)
except Exception as error:
    print(error)

try:
    # Query the gain auto and current value :
    print("GainAuto : %s " % Tis.get_property("GainAuto"))
    print("Gain : %d" % Tis.get_property("Gain"))

    # Check, whether gain auto is enabled. If so, disable it.
    if Tis.get_property("GainAuto"):
        Tis.set_property("GainAuto", "Off")
        print("Gain Auto now : %s " % Tis.get_property("GainAuto"))

    Tis.set_property("Gain", 0)

    # Now do the same with exposure. Disable automatic if it was enabled
    # then set an exposure time.
    if Tis.get_property("ExposureAuto") :
        Tis.set_property("ExposureAuto", "Off")
        print("Exposure Auto now : %s " % Tis.get_property("ExposureAuto"))

    Tis.set_property("ExposureTime", 24000)

except Exception as error:
    print(error)
    quit()    

error = 0
print('Press Esc to stop')
lastkey = 0
cv2.namedWindow('Window',cv2.WINDOW_NORMAL)

try:
    while lastkey != 27 and error < 5:
        time.sleep(1)
        Tis.execute_command("TriggerSoftware") # Send a software trigger

        # Wait for a new image. Use 10 tries.
        tries = 10
        while CD.newImageReceived is False and tries > 0:
            time.sleep(0.1)
            tries -= 1

        # Check, whether there is a new image and handle it.
        if CD.newImageReceived is True:
            CD.newImageReceived = False
            cv2.imshow('Window', CD.image)
        else:
            print("No image received")

        lastkey = cv2.waitKey(10)

except KeyboardInterrupt:
    cv2.destroyWindow('Window')

# Stop the pipeline and clean ip
Tis.stop_pipeline()
cv2.destroyAllWindows()
print('Program ends')
