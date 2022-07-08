# Modul TIS.py
This module is a wrapper around the tiscamera GStreamer modules. It is used in all Python samples of this repository.

## Import:
``` Python
import sys
sys.path.append("../python-common")
import TIS
```
"../python-common" is valid for the samples. If the TIS module "TIS.py" used from other projects, the file "TIS.py" can be copied into the project's directory or the string "../python-common" must be adapted.

## Documentation
### Opening a video capture device
If the serialnumber, format and frame rate is known, then following code can be used:
``` Python
Tis = TIS.TIS()
Tis = Tis.openDevice("00001234", 640, 480, "30/1",TIS.SinkFormats.BGRA, True)
```
The parameters are:
* Serial number of the camera
* Width of the video format
* Height of the video format
* Framerate as string
* Pixelformat of the images in memory (sink)
* A flag, if True an own window for live display is shown

If the device shall be selected from a menu in the terminal:

``` Python
Tis = TIS.TIS()
if not Tis.selectDevice():
        quit(0)
```
(Annotation: The first selection of GRAY8,GRAY16_LE and a pattern like bggr has no effect currently.s)
The sink format is BGRA by default, the live display window is shown also by default.

