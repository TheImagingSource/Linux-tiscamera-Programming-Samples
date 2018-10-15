import TIS

# This sample shows, how to control the focus and, if availabe the zoom
# of a motorized lens
# needed packages:
# pyhton-gst-1.0
# tiscamera


# Open camera, set video format, framerate and determine, whether the sink is color or bw
# Parameters: Serialnumber, width, height, framerate (numerator only) , color, videowindow
# If color is False, then monochrome / bw format is in memory. If color is True, then RGB32
# colorformat is in memory

Tis = TIS.TIS("00001234", 640, 480, 30, True, True)
# the camera with serial number 10710286 uses a 640x480 video format at 30 fps and the image is converted to
# RGBx, which is similar to RGB32.

Tis.Start_pipeline()  # Start the pipeline so the camera streams

# Tis.List_Properties()  # List available properties


while True:
        key = raw_input("f : Auto Focus\nf+ : increase focus\nf- : decrease focus\nz+ : increase zoom\n"
                        "z- : decrease zoom\nq : quit\nPlease enter:")
        if key == "f":
                Tis.Set_Property("Focus Auto", True)

        if key == "f+":
                focus = Tis.Get_Property("Focus").value
                focus = 10
                Tis.Set_Property("Focus", focus)

        if key == "f-":
                focus = Tis.Get_Property("Focus").value
                focus -= 10
                Tis.Set_Property("Focus", focus)

        if key == "z+":
                zoom = Tis.Get_Property("Zoom").value
                zoom += 1
                Tis.Set_Property("Zoom", zoom)

        if key == "z-":
                zoom = Tis.Get_Property("Zoom").value
                zoom -= 1
                Tis.Set_Property("Zoom", zoom)

        if key == "q":
                break

Tis.Stop_pipeline()
print('Program ends')


