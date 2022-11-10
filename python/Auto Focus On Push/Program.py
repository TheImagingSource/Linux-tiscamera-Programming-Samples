# Add path to python-common/TIS.py to the import path
import sys
sys.path.append("../python-common")

import TIS

# This sample shows, how to control the focus and, if availabe the zoom
# of a motorized lens
# needed packages:
# pyhton-gst-1.0
# tiscamera

Tis = TIS.TIS()
# The following line opens and configures the video capture device.
#Tis = Tis.openDevice("00001234", 640, 480, "30/1",TIS.SinkFormats.BGRA, True)

# The next line is for selecting a device, video format and frame rate.
if not Tis.select_device():
    quit(0)

Tis.set_sink_format(TIS.SinkFormats.BGRA) #Use BGRA format in the Memory
Tis.list_properties()  # List available properties

Tis.start_pipeline()  # Start the pipeline so the camera streams



while True:
    key = input("f : Auto Focus\nf+ : increase focus\nf- : decrease focus\nz+ : increase zoom\n"
                "z- : decrease zoom\nq : quit\nPlease enter:")

    try:               
        if key == "f":
            Tis.set_property("FocusAuto", "Once")

        if key == "f+":
            focus = Tis.get_property("Focus")
            focus += 10
            Tis.set_property("Focus", focus)

        if key == "f-":
            focus = Tis.get_property("Focus")
            focus -= 10
            Tis.set_property("Focus", focus)

        if key == "z+":
            zoom = Tis.get_property("Zoom")
            zoom += 1
            Tis.set_property("Zoom", zoom)

        if key == "z-":
            zoom = Tis.get_property("Zoom")
            zoom -= 1
            Tis.set_property("Zoom", zoom)

    except Exception as error:
        print(error)

    if key == "q":
        break

Tis.stop_pipeline()
print('Program ends')
