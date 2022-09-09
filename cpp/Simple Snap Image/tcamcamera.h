#ifndef __TCAMCAMERA_H__
#define __TCAMCAMERA_H__

#include <vector>
#include <string>
#include <memory>
#include <functional>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/videooverlay.h>
#include "tcam-property-1.0.h" /* gobject introspection interface */


namespace gsttcam {

struct FrameRate
{
    int numerator;
    int denominator;
};

struct FrameSize
{
    int width;
    int height;
};

class TcamCamera;

/*
*
*/
struct CameraInfo
{
    std::string serial;
    std::string name;
    std::string identifier;
    std::string connection_type;
};

class TcamCamera
{
    public:
        TcamCamera(std::string serial);
        ~TcamCamera();

        static bool SerialExists(std::string serial);

        /*
        * Return the serial number of the first found device
        */
        static std::string getFirstDeviceSerialNumber();

        /*
        * Get a list of all properties supported by the device
        */
        std::vector<std::string> get_camera_property_list();
       
        /*
        * Property set methods
        */
      
        void set_property(std::string name, const char* value);
        void set_property(std::string name, std::string valuel);
        void set_property(std::string name, int value);
        void set_property(std::string name, gint64 value);
        void set_property(std::string name, double value);
        void set_property(std::string name, bool value);
        void set_property(std::string name);

        void get_property(std::string name, std::string &value);
        void get_property(std::string name, gint64 &value);
        void get_property(std::string name, double &value);
        void get_property(std::string name, bool &value);


        /*
        * Set the video format for capturing
        */
        void set_capture_format(std::string format, FrameSize size, FrameRate framerate);
        /*
        * Set a callback to be called for each new frame
        */
        void set_new_frame_callback(std::function<GstFlowReturn(GstAppSink *appsink, gpointer data)>callback,
                                    gpointer data);;
        /*
        * Start capturing video data
        */
        bool start();
        /*
        * Stop capturing video data
        */
        bool stop();
        /*
        * Get the GStreamer pipeline used for video capture
        */
        GstElement *get_pipeline();
        /*
        * Connect a video display sink element to the capture pipeline
        */
        void enable_video_display(GstElement *displaysink);
        /*
        * Disconnect the video display sink element from the capture pipeline
        */
        void disable_video_display();

    private:
        GstElement *pipeline_ = nullptr;
        GstElement *tcambin_ = nullptr;
        GstElement *capturecapsfilter_ = nullptr;
        GstElement *tee_ = nullptr;
        GstElement *capturesink_ = nullptr;
        GstElement *displaybin_ = nullptr;
        GstElement *displaysink_ = nullptr;
        std::function<GstFlowReturn(GstAppSink *appsink, gpointer data)>callback_;
        gpointer callback_data_ = nullptr;
        guintptr window_handle_ = 0;
        std::string _serialnumber;

        static GstFlowReturn new_frame_callback(GstAppSink *appsink, gpointer data);
        bool WaitForChangeState(GstState newState);
        bool create_pipeline();
};

std::vector<CameraInfo>get_device_list();

};

#endif//__TCAMCAMERA_H__