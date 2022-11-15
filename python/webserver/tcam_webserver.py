import logging
import time
import sys
# pip3 install flask
from flask import Flask, Response, abort
sys.path.append("../python-common")

from TIS import TIS, SinkFormats

app = Flask(__name__)


DEVICE_FRAMERATE = "15/1"
OUTPUT_FRAMERATE = "10/1"

# Create the camera object and start the stream
camera = TIS()
camera.open_device(None, 640, 480, f"{DEVICE_FRAMERATE}", SinkFormats.BGRA, False,
                   conversion=f"videorate ! video/x-raw,framerate={OUTPUT_FRAMERATE} ! videoconvert ! jpegenc quality=60")
camera.start_pipeline()

# reference point in time to calculate current FPS
t_fps_start = None
fps_frame_count = 0
current_fps = 0.0


def imagegenerator():
    global t_fps_start
    global fps_frame_count
    global current_fps
    try:
        while True:
            frame = camera.snap_image(1000)
            if frame is not None:
                fps_frame_count += 1
                if t_fps_start is None:
                    t_fps_start = time.time()
                dt = time.time() - t_fps_start
                if dt >= 10.0:
                    current_fps = fps_frame_count / dt
                    logging.info("Current FPS: %.1f", current_fps)
                    fps_frame_count = 0
                    t_fps_start = time.time()
                # Create the boundary between the sent jpegs
                yield (b'--imagingsource\r\n'
                       b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
            else:
                logging.info("No frame received.")

    finally:
        logging.info("Stream closed")


@app.route('/mjpeg_stream')
def imagestream():
    logging.info("Starting mjpeg stream")
    return Response(imagegenerator(), mimetype='multipart/x-mixed-replace; boundary=imagingsource')


@app.route('/', methods=['GET', 'POST'])
def index():
    try:
        with open("index.html", "r", encoding="UTF-8") as f:
            return Response(f.read(), mimetype="text/html")
    except FileNotFoundError as e:
        abort(404, str(e))
    return None


def main():
    logging.basicConfig(level=logging.INFO)
    logging.info("Logging enabled")

    app.run(host='0.0.0.0', threaded=True)


if __name__ == "__main__":
    main()
