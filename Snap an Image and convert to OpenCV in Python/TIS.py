import numpy
import gi
import time

from collections import namedtuple

gi.require_version("Gst", "1.0")
gi.require_version("Tcam", "0.1")

from gi.repository import Tcam, Gst, GLib, GObject


DeviceInfo = namedtuple("DeviceInfo", "status name identifier connection_type")
CameraProperty = namedtuple("CameraProperty", "status value min max default step type flags category group")


class TIS:
    'The Imaging Source Camera'

    def __init__(self,serial, width, height, framerate, color):
        ''' Constructor
        :param serial: Serial number of the camera to be used.
        :param width: Width of the wanted video format
        :param height: Height of the wanted video format
        :param framerate: Numerator of the frame rate. /1 is added automatically
        :param color: True = 8 bit color, False = 8 bit mono. ToDo: Y16
        :return: none
        '''
        Gst.init([])
        self.height = height
        self.width = width
        self.sample = None
        self.samplelocked = False
        self.newsample = False
        self.img_mat = None
        self.ImageCallback = None

        pixelformat = "BGRx"
        if color is False:
            pixelformat="GRAY8"

        p = 'tcambin serial="%s" name=source ! video/x-raw,format=%s,width=%d,height=%d,framerate=%d/1' % (serial,pixelformat,width,height,framerate,)
        p += ' ! appsink name=sink'

        print(p)
        try:
            self.pipeline = Gst.parse_launch(p)
        except GLib.Error as error:
            print("Error creating pipeline: {0}".format(err))
            raise

        self.pipeline.set_state(Gst.State.READY)
        self.pipeline.get_state(Gst.CLOCK_TIME_NONE)
        # Query a pointer to our source, so we can set properties.
        self.source = self.pipeline.get_by_name("source")

        # Query a pointer to the appsink, so we can assign the callback function.
        self.appsink = self.pipeline.get_by_name("sink")
        self.appsink.set_property("max-buffers",5)
        self.appsink.set_property("drop",1)
        self.appsink.set_property("emit-signals",1)
        self.appsink.connect('new-sample', self.on_new_buffer)

    def on_new_buffer(self, appsink):
        self.newsample = True
        if self.samplelocked is False:
            try:
                self.sample = appsink.get_property('last-sample')
                if self.ImageCallback is not None:
                    self.__convert_sample_to_numpy()
                    self.ImageCallback(self, *self.ImageCallbackData);

            except GLib.Error as error:
                print("Error on_new_buffer pipeline: {0}".format(err))
                raise
        return False

    def Start_pipeline(self):
        try:
            self.pipeline.set_state(Gst.State.PLAYING)
            self.pipeline.get_state(Gst.CLOCK_TIME_NONE)

        except GLib.Error as error:
            print("Error starting pipeline: {0}".format(err))
            raise

    def __convert_sample_to_numpy(self):
        ''' Convert a GStreamer sample to a numpy array
            Sample code from https://gist.github.com/cbenhagen/76b24573fa63e7492fb6#file-gst-appsink-opencv-py-L34

            The result is in self.img_mat.
        :return:
        '''
        self.samplelocked = True
        buf = self.sample.get_buffer()
        caps = self.sample.get_caps()
        bpp = 4;
        dtype = numpy.uint8
        bla = caps.get_structure(0).get_value('height')
        if( caps.get_structure(0).get_value('format') == "BGRx" ):
            bpp = 4;

        if(caps.get_structure(0).get_value('format') == "GRAY8" ):
            bpp = 1;

        if(caps.get_structure(0).get_value('format') == "GRAY16_LE" ):
            bpp = 1;
            dtype = numpy.uint16

        self.img_mat = numpy.ndarray(
            (caps.get_structure(0).get_value('height'),
             caps.get_structure(0).get_value('width'),
             bpp),
            buffer=buf.extract_dup(0, buf.get_size()),
            dtype=dtype)
        self.newsample = False
        self.samplelocked = False

    def wait_for_image(self,timeout):
        ''' Wait for a new image with timeout
        :param timeout: wait time in second, should be a float number
        :return:
        '''

        tries = 10
        while tries > 0 and not self.newsample:
            tries -= 1
            time.sleep(float(timeout) / 10.0)

    def Snap_image(self, timeout):
        '''
        Snap an image from stream using a timeout.
        :param timeout: wait time in second, should be a float number. Not used
        :return: bool: True, if we got a new image, otherwise false.
        '''
        if self.ImageCallback is not None:
            print("Snap_image can not be called, if a callback is set.")
            return False

        self.wait_for_image(timeout)
        if( self.sample != None and self.newsample == True):
            self.__convert_sample_to_numpy()
            return True

        return False

    def Get_image(self):
        return self.img_mat

    def Stop_pipeline(self):
        self.pipeline.set_state(Gst.State.PAUSED)
        self.pipeline.set_state(Gst.State.READY)
        self.pipeline.set_state(Gst.State.NULL)

    def List_Properties(self):
        for name in self.source.get_tcam_property_names():
            print( name )

    def Get_Property(self, PropertyName):
        try:
            return CameraProperty(*self.source.get_tcam_property(PropertyName))
        except GLib.Error as error:
            print("Error get Property {0}: {1}",PropertyName, format(err))
            raise

    def Set_Property(self, PropertyName, value):
        try:
            self.source.set_tcam_property(PropertyName,GObject.Value(type(value),value))
        except GLib.Error as error:
            print("Error set Property {0}: {1}",PropertyName, format(err))
            raise

    def Set_Image_Callback(self, function, *data):
        self.ImageCallback = function
        self.ImageCallbackData = data;
