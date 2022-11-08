import logging
import time
# pip3 install flask
from flask import Flask, Response, abort
from tcam_camera import TcamCamera

app = Flask(__name__)

# Create the camera object and start the stream
camera = TcamCamera()

camera.set_max_rate(5)

# reference point in time to calculate current FPS
t_fps_start = None
fps_frame_count = 0


def imagegenerator():
    global t_fps_start
    global fps_frame_count
    try:
        while True:
            frame = camera.snap_image(1000)
            if frame is not None:
                fps_frame_count += 1
                if t_fps_start is None:
                    t_fps_start = time.time()
                dt = time.time() - t_fps_start
                if dt >= 10.0:
                    fps = fps_frame_count / dt
                    logging.info("Current FPS: %.1f", fps)
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
