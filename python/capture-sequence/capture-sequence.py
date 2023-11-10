import time
import TIS
import cv2

# This sample shows, how to capture many images into memory
# and save them after capturing to single image files.
# It is a very simple approach without error checking.
# The number of frames to capture is passed to the TIS class
# by setting the member
# Tis.framestocapture
# Afterwards the program simply waits, until Tis.framestocapture
# is 0.
# During the capture process, the incoming image data is saved
# internally in an array.
# By calling Tis.get_image(imagenr) the wanted image is converted
# from data type to numpy array and returned. Then the image can
# be processed and saved by OpenCV
#
# Needed packages:
# pyhton-gst-1.0
# python-opencv
# tiscamera

Tis = TIS.TIS()
# The following line opens and configures the video capture device.
# Tis.open_device("33910666", 640, 480, "110/1",TIS.SinkFormats.BGRA, True)

# The next line is for selecting a device, video format and frame rate.
if not Tis.select_device():
    quit(0)


Tis.set_sink_format(TIS.SinkFormats.BGRA) #Use BGRA format in the Memory
# Tis.list_properties()  # List available properties

Tis.start_pipeline()  # Start the pipeline so the camera streams

# Capture 100 frames
Tis.framestocapture = 100
# Wait for all frames being captured
while Tis.framestocapture > 0:
    time.sleep(0.1)

# Save the images now.
print("Saving images.")
for imgnr in range(Tis.get_captured_image_count()):
    image = Tis.get_image(imgnr)
    if image is not None:
        cv2.imwrite("test{0}.png".format(imgnr),
                    image)

Tis.stop_pipeline()
print('Program ends')
