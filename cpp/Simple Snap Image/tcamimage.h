
#ifndef __TCAMIMAGE__
#define __TCAMIMAGE__

#include "tcamcamera.h"
#include <mutex>
#include <condition_variable>
#include <vector>

class TcamImage : public gsttcam::TcamCamera
{
    public:
        TcamImage(std::string serial = "");
        ~TcamImage();
        void set_capture_format(std::string format, gsttcam::FrameSize size, gsttcam::FrameRate framerate);
        bool start();
        bool snapImage(int timeout_ms);
        int getWidth()
        {
            return _CustomData.width;
        }
        int getHeight()
        {
            return _CustomData.height;
        }
        int getBytesPerPixel()
        {
            return _CustomData.bpp;
        }
        unsigned char* getImageData()
        {
            return _CustomData.image_data;
            //return _CustomData.image_data.data();
        }
        int getImageDataSize()
        {
            return _CustomData.width * _CustomData.height *_CustomData.bpp;
            //return _CustomData.image_data.size();
        }

    private:
        typedef struct
        {
            int ImageCounter;
            bool SaveNextImage;
            std::mutex mtx;
            std::condition_variable con;

            //std::vector<uint8_t> image_data;

            unsigned char* image_data;
            int width;
            int height;
            int bpp;
        } CUSTOMDATA;

        CUSTOMDATA _CustomData;

        static GstFlowReturn new_frame_cb(GstAppSink *appsink, gpointer data);
}; 

#endif