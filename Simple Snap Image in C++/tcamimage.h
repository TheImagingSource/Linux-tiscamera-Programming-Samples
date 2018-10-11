#ifndef __TCAMIMAGE__
#define __TCAMIMAGE__
#endif

#include "tcamcamera.h"
#include <mutex>
#include <condition_variable>


class TcamImage : public gsttcam::TcamCamera
{
    public:
        TcamImage(std::string serial = "");
        void set_capture_format(std::string format, gsttcam::FrameSize size, gsttcam::FrameRate framerate);
        bool start();
        bool snapImage(int timeout_ms);
        int getWidth()
        {
            return _CustomData.width;
        };
        int getHeight()
        {
            return _CustomData.height;
        };
        int getBytesPerPixel()
        {
            return _CustomData.bpp;
        }
        unsigned char* getImageData()
        {
            return _CustomData.ImageData;
        }
        int getImageDataSize()
        {
            return _CustomData.width * _CustomData.height * _CustomData.bpp;
        }

    private:
        typedef struct
        {
            int ImageCounter;
            bool SaveNextImage;
            bool busy;
            std::mutex mtx;
            std::condition_variable con;

            unsigned char* ImageData;
            int width;
            int height;
            int bpp;
        } CUSTOMDATA;

        CUSTOMDATA _CustomData;
        
        std::condition_variable _con;


        static GstFlowReturn new_frame_cb(GstAppSink *appsink, gpointer data);
}; 