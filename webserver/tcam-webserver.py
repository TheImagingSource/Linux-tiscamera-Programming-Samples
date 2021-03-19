# pip3 install flask

import sys
from flask import Flask, Response
import logging
import tcam_camera
import time

app = Flask(__name__)

# Create the camera object and start the stream
camera = tcam_camera.tcam_camera()

def imagegenerator():
    try:
        while True:
            frame = camera.snap_image(1000)
            if frame is not None:
                # Create the boundary between the sent jpegs
                yield (b'--imagingsource\r\n'
                    b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
            else:
                logging.info("No frame received.")

            # Send 10 frames per second only
            time.sleep(0.1) 
    finally:
        logging.info("Stream closed")


def loadfile(name):
    f = open(name,"r")
    if f is not None:
        return f.read()
    return "File " + name + " not found"

@app.route('/mjpeg_stream')
def imagestream():
    logging.info("Starting mjpeg stream")
    return Response( imagegenerator() , mimetype='multipart/x-mixed-replace; boundary=imagingsource')

@app.route('/',methods=['GET', 'POST'])
def index():
    logging.info('Send index')
    return loadfile("index.html")
   
def main(argv):
    loglevel = "INFO"

    numeric_level = getattr(logging, loglevel,None)
    logging.basicConfig(level=numeric_level)
    logging.info("Logging enabled")
    
    app.run(host='0.0.0.0', threaded=True)

if __name__ == "__main__":
    main(sys.argv[1:])
