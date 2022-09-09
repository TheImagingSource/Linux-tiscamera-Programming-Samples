

#include "tcamcamera.h"
#include "ctcamproperty.h"
#include <vector>
#include <string>
#include <stdexcept>

#include <iostream>

#include <assert.h>

using namespace gsttcam;

std::vector<CameraInfo> gsttcam::get_device_list()
{
    if (!gst_is_initialized())
        throw std::runtime_error("GStreamer is not initialized! gst_init(...) needs to be called before using this function.");

    std::vector<CameraInfo> ret;
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);
    GList* devices = gst_device_monitor_get_devices(monitor);

    for (GList* elem = devices; elem; elem = elem->next)
    {
        GstDevice* device = (GstDevice*) elem->data;
        GstStructure* devstruc = gst_device_get_properties( device );
        std::string foundSerial = gst_structure_get_string(devstruc, "serial");

        CameraInfo info;
        info.serial =  gst_structure_get_string(devstruc, "serial");
        info.name =  gst_structure_get_string(devstruc, "model");
        info.identifier =  "";
        info.connection_type =  gst_structure_get_string(devstruc, "type");
        ret.push_back(info);
        gst_structure_free(devstruc);
    }
    g_list_free_full(devices, gst_object_unref);

    return ret;
}

////////////////////////////////////////////////////////////
// Check, whether the passed serial number exists. 
bool TcamCamera::SerialExists(std::string serial)
{
    bool result = false;
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);
    GList* devices = gst_device_monitor_get_devices(monitor);  
    for (GList* elem = devices; elem; elem = elem->next)
    {
        GstDevice* device = (GstDevice*) elem->data;
        GstStructure* devstruc = gst_device_get_properties( device );
        std::string foundSerial = gst_structure_get_string(devstruc, "serial");
        if( foundSerial != serial)
        {
            foundSerial += "-";
            foundSerial += gst_structure_get_string(devstruc, "type");
            if( foundSerial == serial)
            {
                result = true;
                break;
            }
        }
        else
        {
            result = true;
            break;
        }
        gst_structure_free(devstruc);
    }
    g_list_free_full(devices, gst_object_unref);
    return result;
}

////////////////////////////////////////////////////
// Get the serial number of the first found device.
std::string TcamCamera::getFirstDeviceSerialNumber()
{
    std::string foundSerial = "";
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);
    GList* devices = gst_device_monitor_get_devices(monitor);  

    if( devices != NULL)
    {
        GstDevice* device = (GstDevice*) devices->data;
        GstStructure* devstruc = gst_device_get_properties( device );
        foundSerial = gst_structure_get_string(devstruc, "serial");
        foundSerial += "-";
        foundSerial += gst_structure_get_string(devstruc, "type");
        gst_structure_free(devstruc);
    }

    g_list_free_full(devices, gst_object_unref);

    return foundSerial;
}

TcamCamera::TcamCamera(std::string serial = "")
{
    if (!gst_is_initialized())
        throw std::runtime_error("GStreamer is not initialized! gst_init(...) needs to be called before using this function.");

    _serialnumber = serial;
    create_pipeline();
}

TcamCamera::~TcamCamera()
{
    g_print("pipeline refcount at cleanup: %d\n", GST_OBJECT_REFCOUNT_VALUE(pipeline_));
    gst_object_unref(pipeline_);
}

////////////////////////////////////////////////////////////////////////////////
// Create the pipeline with appsink only. 
bool TcamCamera::create_pipeline()
{
    GError *error = nullptr;
    pipeline_ = gst_parse_launch("tcambin name=source ! capsfilter name=caps ! tee name=t t. ! queue ! appsink name=sink", &error);

   	if (error)
	{
		printf( "Error building camera pipeline: %s\n", (char*)error->message);
		return false;
	}

    tcambin_ =  gst_bin_get_by_name(GST_BIN(pipeline_), "source");
    if (!tcambin_)
        throw std::runtime_error("'tcambin' could not be initialized! Check tiscamera installation");

    g_object_set(tcambin_, "serial", _serialnumber.c_str(), nullptr);

    capturecapsfilter_ = gst_bin_get_by_name(GST_BIN(pipeline_), "caps");
    tee_ = gst_bin_get_by_name(GST_BIN(pipeline_), "t");

    capturesink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "sink");
    g_object_set(capturesink_, "max-buffers", 4, "drop", true, nullptr);

    gst_element_set_state(tcambin_, GST_STATE_READY);
    gst_element_get_state(tcambin_, NULL, NULL, 4000000000ll);

    return true;
}

std::vector<std::string> TcamCamera::get_camera_property_list()
{
    std::vector<std::string> pptylist;
    
    g_object_set(tcambin_, "serial", _serialnumber.c_str(), nullptr);

    GError* error = NULL;
    GSList* names =  tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(tcambin_), &error);

    if(error)
    {
		printf( "Error list properties: %s\n", (char*)error->message);
        return pptylist;
    }

    for (unsigned int i = 0; i < g_slist_length(names); ++i)
    {
        char* name = (char*)g_slist_nth(names, i)->data;
        std::string property = name;
        TcamPropertyBase* base_property = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(tcambin_), name, &error);
        TcamPropertyType type = tcam_property_base_get_property_type(base_property);

        property += " ";
        property += CTcamProperty::getPropertyTypeName(type);

        pptylist.push_back(property);
        g_object_unref(base_property);
    }
    g_slist_free_full(names, g_free);
    return pptylist; 
}

/*************************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////
//
void TcamCamera::set_property(std::string name, std::string value)
{
    CTcamProperty p(tcambin_,name);
    p.set(value);
}

void TcamCamera::set_property(std::string name, const char* value)
{
    set_property( name, std::string(value));
}

void TcamCamera::set_property(std::string name, gint64 value)
{
    CTcamProperty p(tcambin_,name);
    p.set(value);
}

void TcamCamera::set_property(std::string name, int value)
{
    set_property(name, (gint64) value);
}

void  TcamCamera::set_property(std::string name, double value)
{
    CTcamProperty p(tcambin_,name);
    p.set(value);
}

void TcamCamera::set_property(std::string name, bool value)
{
    CTcamProperty p(tcambin_,name);
    p.set(value);
}

void TcamCamera::set_property(std::string name)
{
    CTcamProperty p(tcambin_,name);
    p.set();
}

void TcamCamera::get_property(std::string name, std::string &value)
{
    CTcamProperty p(tcambin_,name);
    p.get(value);
}

void TcamCamera::get_property(std::string name, gint64 &value)
{
    CTcamProperty p(tcambin_,name);
    p.get(value);
}

void TcamCamera::get_property(std::string name, double &value)
{
    CTcamProperty p(tcambin_,name);
    p.get(value);
}

void TcamCamera::get_property(std::string name, bool &value)
{
    CTcamProperty p(tcambin_,name);
    p.get(value);
}


/*************************************************************************************/
void TcamCamera::set_capture_format(std::string format, FrameSize size, FrameRate framerate)
{
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "width", G_TYPE_INT, size.width,
                                        "height", G_TYPE_INT, size.height,
                                        "framerate", GST_TYPE_FRACTION, framerate.numerator, framerate.denominator,
                                        nullptr);
    assert(caps);
    if(format != "")
        gst_caps_set_simple(caps, "format", G_TYPE_STRING, format.c_str(), nullptr);

    g_object_set(G_OBJECT(capturecapsfilter_), "caps", caps, nullptr);
    gst_caps_unref(caps);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Helper function for GStreamer state changes with error handling.
//
bool TcamCamera::WaitForChangeState(GstState newState)
{
	GstStateChangeReturn Result = gst_element_set_state( pipeline_, newState);
	if( Result == GST_STATE_CHANGE_FAILURE )
	{
		printf("FAILURE set state() () change to state %d failed\n", newState);
		return false;
	}

	Result = gst_element_get_state( pipeline_, NULL, NULL, 4000000000ll);

	if( Result == GST_STATE_CHANGE_FAILURE )
	{
		printf("FAILURE WaitForChangeState() change to state %d failed\n", newState);
		return false;
	}

	if( Result == GST_STATE_CHANGE_ASYNC )
	{
		printf("ASYNC WaitForChangeState() change to state %d failed\n", newState);
		return false;
	}
	return true;
}


bool TcamCamera::start()
{
    WaitForChangeState(GST_STATE_READY);

    
    //GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline_),
    //                          GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
    return WaitForChangeState(GST_STATE_PLAYING);
}

bool TcamCamera::stop()
{
    return WaitForChangeState(GST_STATE_NULL);
}

GstFlowReturn TcamCamera::new_frame_callback(GstAppSink *appsink, gpointer data)
{
    TcamCamera *this_ = static_cast<TcamCamera *>(data);
    if (this_->callback_)
    {
        return this_->callback_(appsink, this_->callback_data_);
    }
    return GST_FLOW_OK;
}

void TcamCamera::set_new_frame_callback(std::function<GstFlowReturn(GstAppSink *appsink, gpointer data)>callback,
                                   gpointer data)
{
    callback_ = callback;
    callback_data_ = data;
    GstAppSinkCallbacks callbacks = {nullptr, nullptr, new_frame_callback};
    gst_app_sink_set_callbacks(GST_APP_SINK(capturesink_), &callbacks, this, nullptr);
}

GstElement * TcamCamera::get_pipeline()
{
    gst_object_ref(pipeline_);
    return pipeline_;
}

void TcamCamera::enable_video_display(GstElement *displaysink)
{
    if(displaybin_)
        return;

    assert(displaysink);
    displaybin_ = gst_element_factory_make("bin", nullptr);
    GstElement *queue = gst_element_factory_make("queue", nullptr);
    GstElement *convert = gst_element_factory_make("videoconvert", nullptr);
    GstElement *capsfilter = gst_element_factory_make("capsfilter", nullptr);

    assert( tee_ &&displaybin_&& queue && convert && capsfilter && pipeline_);

    gst_bin_add(GST_BIN(pipeline_), displaybin_);
    gst_bin_add_many(GST_BIN(displaybin_), queue, convert, capsfilter, displaysink, nullptr);

    if (!gst_element_link_many(tee_, queue, convert, capsfilter, displaysink, nullptr))
        throw std::runtime_error("Could not link elements of display pipeline");

}

void TcamCamera::disable_video_display()
{
    if(displaybin_)
    {
        gst_object_ref(displaybin_);
        gst_bin_remove(GST_BIN(pipeline_), displaybin_);
        gst_element_set_state(displaybin_, GST_STATE_NULL);
        gst_element_get_state(displaybin_, nullptr, nullptr, GST_CLOCK_TIME_NONE);
        gst_object_unref(displaybin_);
        displaybin_ = nullptr;
    }
}
