#! /usr/bin/sh
lxterminal -e 'bash -c "echo h264 streaming can be stopped with Ctrl+C;echo Start streaming by pressing ENTER;read;gst-launch-1.0 tcambin ! video/x-raw,format=BGR,framerate=30/1,width=1920,height=1080 ! v4l2h264enc ! h264parse config-interval=1 ! rtph264pay ! udpsink host=239.0.0.1 port=5004; bash"'
