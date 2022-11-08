# Stream into Web Browser

This sample shows how to stream from a camera to a web browser like FireFox, Chrome, Egde and so on.
It is based on Flask.

## Prerequisites:
The [Flask](https://flask.palletsprojects.com/en/1.1.x/) packet must be installed by a call to
```
sudo pip3 install flask
```

## Documentation

### The Server

The main program is `tcam_webserver.py`. It is started by
```
python3 tcam_webserver.py
```

The server serves on port 5000. In your browser add the url
```
http://<ip>:5000
```

If you run the browser on the same computer as the server, you can use
```
http://localhost:5000
```
It must be "http", not "https".
As many as wanted clients (browsers) can connect to the web server.

### `tcam_webserver.py`

This file implements the server part. It reacts to the requests and has the generator for creating the image stream.

### `tcam_camera.py`

This implements a very simple camera class. In there is the pipeline for streaming, resizing and JPEG encoding into an appsink.
The snap_image call waits for as sample from the appsink and extracts the JPEG image data. It is called by the generator in `tcam-webserver.py`

### index.html

The very simple html file that is sent to the browser. The stream is passed to
``` html
<img src="/mjpeg_stream" class="" alt="" />
```

The tcam-webserver.py expects `/mjpeg_stream`.
For reducing complexity we do not use templates here.
