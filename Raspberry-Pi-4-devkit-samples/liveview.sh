#! /usr/bin/sh
lxterminal -e 'bash -c "echo Live image can be stopped by closing video window;echo Start live view by pressing ENTER;read;gst-launch-1.0 tcambin ! ximagesink; bash"'
