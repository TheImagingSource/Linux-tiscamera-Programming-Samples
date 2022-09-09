#include "tcamimage.h"
#include <unistd.h>
#include <chrono>
#include <cstring>

using namespace gsttcam;

TcamImage::TcamImage(std::string serial) : TcamCamera(serial)
{
    _CustomData.ImageCounter = 0;
    _CustomData.SaveNextImage = false;
    _CustomData.bpp = 4;
    _CustomData.width = 0;
    _CustomData.height = 0;
    _CustomData.image_data = NULL;
}

TcamImage::~TcamImage()
{
    if(_CustomData.image_data != NULL)
    {
        delete _CustomData.image_data;
    }
}

void TcamImage::set_capture_format(std::string format, FrameSize size, FrameRate framerate)
{
    // Allocate memory for one image buffer.
    if( format == "GRAY8")
        _CustomData.bpp = 1;
    else
        _CustomData.bpp = 4;

    _CustomData.width = size.width;
    _CustomData.height = size.height;
    //_CustomData.image_data.resize( size.width * size.width * _CustomData.bpp );

    TcamCamera::set_capture_format(format, size, framerate);
}

void TcamImage::start()
{
    // Register a callback to be called for each new frame
    set_new_frame_callback(new_frame_cb, &_CustomData);
    if(_CustomData.image_data != NULL)
    {
        delete _CustomData.image_data;
    }
    _CustomData.image_data = new unsigned char[getImageDataSize()] ;
    TcamCamera::start();
}

bool TcamImage::snapImage(int timeout_ms)
{
    std::unique_lock<std::mutex> lck(_CustomData.mtx);
    _CustomData.SaveNextImage = true;
    if( _CustomData.con.wait_for(lck,std::chrono::milliseconds(timeout_ms) ) != std::cv_status::timeout ){
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////
// Callback called for new images by the internal appsink
GstFlowReturn TcamImage::new_frame_cb(GstAppSink *appsink, gpointer data)
{
    // Cast gpointer to CUSTOMDATA*
    CUSTOMDATA *pCustomData = (CUSTOMDATA*)data;
    if( !pCustomData->SaveNextImage)
        return GST_FLOW_OK;
    std::lock_guard<std::mutex> lck(pCustomData->mtx);

    pCustomData->SaveNextImage = false;
    pCustomData->ImageCounter++;

    // The following lines demonstrate, how to access the image
    // data in the GstSample.
    GstSample *sample = gst_app_sink_pull_sample(appsink);

    GstBuffer *buffer = gst_sample_get_buffer(sample);

    GstMapInfo info;

    gst_buffer_map(buffer, &info, GST_MAP_READ);
    
    if (info.data != NULL) 
    {
        // info.data contains the image data as blob of unsigned char 
        memcpy( pCustomData->image_data, info.data, pCustomData->width * pCustomData->height * pCustomData->bpp);
        //memcpy( pCustomData->image_data.data(), info.data, pCustomData->width * pCustomData->height * pCustomData->bpp);
    }
    
    // Calling Unref is important!
    gst_buffer_unmap (buffer, &info);
    gst_sample_unref(sample);

    pCustomData->con.notify_all();
    // Set our flag of new image to true, so our main thread knows about a new image.
    return GST_FLOW_OK;
}
