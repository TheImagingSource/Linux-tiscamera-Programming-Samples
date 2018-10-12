# Stream over Network to VLC
Here is shown, how to create a [Gstreamer](https://gstreamer.freedesktop.org/) pipeline for sending a live stream over the network to the [VLC player](https://www.videolan.org/vlc/index.de.html) using a The Imaging Source camera.

Programming language : None

## Prerequisits
The [tiscamera](https://github.com/TheImagingSource/tiscamera#the-imaging-source-linux-repository) repository must be built and installed, so the GStreamer module ```tcambin``` exists.

### Send Pipeline
The send pipeline is started in a terminal, so a new terminal should be opened. For running the pipline [gst-launch-1-0](https://gstreamer.freedesktop.org/documentation/tools/gst-launch.html) is used.
The command line is
```
gst-launch-1.0 tcambin ! video/x-raw, format=BGRx, width=640, height=480, framerate=15/1 ! videoconvert ! x264enc tune=zerolatency byte-stream=true bitrate=500 threads=2 ! mpegtsmux ! tcpserversink host=192.168.2.224 port=5004
```

A 640x480 color video format is used at 15 frames per second:
```
video/x-raw, format=BGRx, width=640, height=480, framerate=15/1
```
This should be adapated to the capabilities of the used camera.

The video stream is H264 encoded, inserted into a video container by ```mpegtsmux``` and forwarded to a ```tcpserversink```. The host ip is the IP address of the computer, on which this pipeline runs. The port is optional.

### VLC
VLC can be used in Windows and Linux. Click on "Media" and select "Network stream". At network address enter ```tcp://192.168.2.224:5004``` and start. Adapt the IP address and port to the computer settings on which the pipeline with the camere runs.
There will be a delay of a few seconds, even if the camera pipeline and VLC run on the same computer. If there are suggestions for reducing the delay, please let me know.


