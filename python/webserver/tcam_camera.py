from typing import Optional
import gi
gi.require_version("Gst", "1.0")
gi.require_version("Tcam", "1.0")
from gi.repository import Gst


class TcamCamera:
    '''
    A small camera class for easier gst pipeline handling.
    The video stream gets rate adapted, resized and compressed.
    '''

    def __init__(self, width=640, height=480, quality=60):
        Gst.init([])
        self.pipeline = Gst.parse_launch(
            f"tcambin ! videorate name=videorate ! "
            f"videoconvert ! videoscale ! "
            f"video/x-raw, width={width}, height={height} ! "
            f"jpegenc quality={quality} ! appsink name=sink")

        self.appsink = self.pipeline.get_by_name('sink')
        self.appsink.set_property("max-buffers", 5)
        self.appsink.set_property("drop", 1)
        self.videorate = self.pipeline.get_by_name("videorate")
        self.pipeline.set_state(Gst.State.PLAYING)
        self.pipeline.get_state(40000000)

    def snap_image(self, timeout: float) -> Optional[bytes]:
        '''
        Snap an image from stream using a timeout.
        :param timeout: wait time in seconds
        :return: The image data or None.
        '''
        sample = self.appsink.emit("try-pull-sample", timeout * Gst.SECOND)

        if sample is not None:
            buf = sample.get_buffer()
            mem = buf.get_all_memory()
            success, info = mem.map(Gst.MapFlags.READ)
            if success:
                data = bytes(info.data)
                # casting a MemoryView to bytes implicitly copies the data so we can unmap here
                mem.unmap(info)
                return data

        return None

    def set_max_rate(self, fps: int):
        '''
        Set maximum frames per second to pass through
        '''
        self.videorate.set_property("max-rate", fps)
