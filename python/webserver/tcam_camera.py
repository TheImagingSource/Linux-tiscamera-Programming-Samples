
import time
import gi
gi.require_version("Gst", "1.0")
gi.require_version("Tcam", "1.0")
from gi.repository import GLib, GObject, Gst, Tcam

'''
A very small camera class, that creates the pipeline with an sink.
In the pipeline the image is rezised to 640x480 resolution. 
The JPEG Quality is set to 60%.
'''

class tcam_camera(object):

    def __init__(self):
        Gst.init([])
        self.sample = None
        self.samplelocked = False
        self.newsample = False

        self.pipeline = Gst.parse_launch("tcambin ! videoconvert ! videoscale ! video/x-raw, width=640, height=480 ! jpegenc quality=60 ! appsink name=sink")

        self.appsink = self.pipeline.get_by_name('sink')
        self.appsink.set_property("max-buffers",5)
        self.appsink.set_property("drop",1)
        self.appsink.set_property("emit-signals",1)
        self.appsink.connect('new-sample', self.on_new_buffer)
        self.pipeline.set_state(Gst.State.PLAYING)
        self.pipeline.get_state(40000000)

        self.image = None
        
    def on_new_buffer(self, appsink):
        '''
        Callback for new images.
        :param appsink: The sink, that called the callback
        '''
        self.newsample = True
        if self.samplelocked is False:
            try:
                self.sample = appsink.get_property('last-sample')
            except GLib.Error as error:
                print("Error on_new_buffer pipeline: {0}".format(error))
                self.newsample = False
                raise
        return False

    def wait_for_buffer(self,timeout):
        ''' Wait for a new image with timeout
        :param timeout: wait time in second, should be a float number
        :return:
        '''
        tries = 10
        while tries > 0 and not self.newsample:
            tries -= 1
            time.sleep(float(timeout) / 10.0)       

    def snap_image(self, timeout):
        '''
        Snap an image from stream using a timeout.
        :param timeout: wait time in second, should be a float number. Not used
        :return: The image data or None.
        '''
        self.wait_for_buffer(timeout)
        if( self.sample != None and self.newsample == True):
            buf = self.sample.get_buffer()
            mem = buf.get_all_memory()
            success, info = mem.map(Gst.MapFlags.READ)
            if success:
                data = info.data
                mem.unmap(info)
                self.image = data 
                return self.image
            
        return None
