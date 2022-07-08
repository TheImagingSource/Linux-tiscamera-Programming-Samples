#include "camerabase.h"
#include <unistd.h>
#include <pthread.h>
#include "../controls/win.h"
#include <gst/video/navigation.h>
#include <gst/video/video-enumtypes.h>
#include <cassert>

void *MainloopThread(void *threadarg) 
{
    GMainLoop* loop = (GMainLoop*) threadarg;
    g_main_loop_run(loop);
    pthread_exit(NULL);
}

static gboolean bus_call (GstBus* bus, GstMessage* msg, gpointer data)
{
    //GMainLoop* loop = (GMainLoop*) data;
    BUSUSERDATA_t *pData = (BUSUSERDATA_t*) data;

    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_EOS:
            {
                pData->setBusMessage( "bus_call : EOS received.");
                break;
            }
        case GST_MESSAGE_ERROR:
            {
                gchar*  debug;
                GError* error;

                gst_message_parse_error (msg, &error, &debug);
                //pData->setBusMessage( fmt::format("bus_call : Error {0}  {1}\n",error->code, error->message));
                g_free (debug);
                g_error_free (error);
                
                //log("bus_call :  Error:  %d %s\n", error->code, error->message);
                //g_main_loop_quit ( loop );
                
                break;
            }
    /*    case GST_MESSAGE_STATE_CHANGED:
            {
                GstState old_state, new_state;

                gst_message_parse_state_changed (msg, &old_state, &new_state, NULL);
                log("Element %s changed state from %s to %s.\n",
                        GST_OBJECT_NAME (msg->src),
                        gst_element_state_get_name (old_state),
                        gst_element_state_get_name (new_state));
               break;
            }
        */
		case GST_MESSAGE_ELEMENT:
            {
                const GstStructure *s = gst_message_get_structure (msg);
                const gchar *name = gst_structure_get_name (s);
                if( strcmp(name,"GstNavigationMessage") == 0 )
                {
                    GstNavigationMessageType nav_msg_type;
                    nav_msg_type = gst_navigation_message_get_type (msg); 

                    if( nav_msg_type == GST_NAVIGATION_MESSAGE_EVENT )
                    {
                        GstEvent *event;
                        GstNavigationEventType nav_evt_type;
                        gst_navigation_message_parse_event (msg, &event);
                        nav_evt_type = gst_navigation_event_get_type (event);
                        if( nav_evt_type == GST_NAVIGATION_EVENT_KEY_RELEASE)
                        {
                            pData->setKeyPress(true);
                        }
                    }
                                       
                }
                break;
            }
            
            /*
        case GST_MESSAGE_STREAM_STATUS:
            log("Stream Status\n");
            break;

        case  GST_MESSAGE_NEW_CLOCK:
            log("New clock\n");
            break;

        case  GST_MESSAGE_STRUCTURE_CHANGE:
            log("Structure Change\n");
            break;

        case  GST_MESSAGE_ASYNC_DONE:
            log("Async done\n");
            break;
        */
        default:
		{
            //log("bus call messagetype %d\n",GST_MESSAGE_TYPE (msg) );
            break;
		} 
    }
    return true;
}
/*
CameraBase::CameraBase( const Json::Value& camera )
{
    init();
    _name = camera["name"].asString();
    findSerialnumber();
    if(_serial != "")
    {
        log("Serial : %s", _serial.c_str());
    }
    else
    {
        log("not connected.");
    }
    
    
    printf("\n");
}*/

CameraBase::CameraBase( std::string serial, std::string name, int winline  )
{
    init();
    _serial = serial;
    _name = name;
    _windowline = winline;
}

void CameraBase::init() 
{
    _pipeline = NULL;
    _bususerdata = std::make_shared<BUSUSERDATA_t>();
    _bususerdata->loop = NULL;
    _bus_watch_id = 0;
    _LastError = "";
}

void CameraBase::findSerialnumber()
{
    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    /* retrieve a single linked list of serials of the available devices */
    GSList* serials = tcam_prop_get_device_serials(TCAM_PROP(source));

    for (GSList* elem = serials; elem; elem = elem->next)
    {

        const char* device_serial = (gchar*)elem->data;

        char* name;
        char* identifier;
        char* connection_type;

        gboolean ret = tcam_prop_get_device_info(TCAM_PROP(source),
                                                  device_serial,
                                                 &name,
                                                 &identifier,
                                                 &connection_type);

        if (ret) // get_device_info was successful
        {
            if( _name == std::string(name ))
            {
                _serial = (gchar*)elem->data;
            }
            g_free( name );
            g_free( identifier );
            g_free( connection_type );
        }
    }

    g_slist_free_full(serials, g_free);
    gst_object_unref(source);

}

bool CameraBase::createPipeline( std::string pipelinestring)
{
    // Parse the pipeline
	GError* err = NULL;
	_pipeline = gst_parse_launch(pipelinestring.c_str(), &err);
	if(err != NULL)
	{	
	    _LastError = "Parse camera pipeline failed: " + std::string( err->message);
		return false;
	}	

    tcambin_ = gst_bin_get_by_name(GST_BIN(_pipeline), "source");
    if( tcambin_ == NULL)
	{	
	    _LastError = "source is not defined";
		return false;
	}	

    return true;
}

void CameraBase::createMainLoop()
{
    if( _bususerdata->loop == NULL )
    {
        _bususerdata->setBusMessage("");

        _bususerdata->loop = g_main_loop_new (NULL, FALSE);
         _bus = gst_pipeline_get_bus (GST_PIPELINE (_pipeline));
        _bus_watch_id = gst_bus_add_watch (_bus, bus_call, _bususerdata.get());
        gst_object_unref (_bus);

        pthread_create(&_mainloopthread, NULL, MainloopThread, (void *)_bususerdata->loop);
    }
}

bool CameraBase::preparePipeline()
{
    createMainLoop();
    if( !changeState( GST_STATE_READY) )
    {
		_LastError = "Failed to prepare the pipeline " + _name;
        return false;
    }
	return true;
}

bool CameraBase::startPipeline()
{
    createMainLoop();

   	if( !changeState( GST_STATE_READY) )
    {
   		_LastError = _LastError + " Failed to start the pipeline to READY " + _name;
        return false;
    }
   if( !changeState( GST_STATE_PLAYING) )
    {
		_LastError = _LastError +  "Failed to start the pipeline PLAYING " + _name;
        return false;
    }

    queryChildElements();

	return true;
}

bool CameraBase::stopPipeline()
{
    /*
    if( _bususerdata->getBusMessage() != "");
    {
        log( _bususerdata->getBusMessage() );
    }
    */
   	if( !changeState( GST_STATE_READY) )
    {
   		_LastError = "Failed to stop the pipeline to READY " + _name;
        return false;
    }

   	if( !changeState( GST_STATE_NULL) )
    {
   		_LastError = "Failed to stop the pipeline to NULL " + _name;
        return false;
    }

	return true;
}

void CameraBase::unrefPipeline()
{
    if( _bususerdata->loop != NULL)
    {
        if( _bus_watch_id > 0)
			g_source_remove (_bus_watch_id);

        g_main_loop_quit (_bususerdata->loop);
		g_main_loop_unref(_bususerdata->loop);
		_bususerdata->loop = NULL;
    }

    changeState( GST_STATE_NULL);
    gst_object_unref(_pipeline);
    gst_object_unref(tcambin_);
    _pipeline = NULL;
    tcambin_ = NULL;
}


bool CameraBase::changeState( GstState state)
{
//    Stopwatch sw(false);

    if(_pipeline == NULL)
	{
        _LastError = "Pipeline is NULL in changeState() " + _name;
		return false;
	}	
	gst_element_set_state(_pipeline, state);
    
    GstState currentstate;
    GstState pending;
    GstStateChangeReturn x;
    int tries = 1;
    //sw.start();
    do
    {   
        GstClockTime timeout;
        timeout = 2000*1000*1000;
        
        x = gst_element_get_state(_pipeline, &currentstate, &pending, timeout);
        tries--;
    } while ( x != GST_STATE_CHANGE_SUCCESS && tries > 0 && x != GST_STATE_CHANGE_FAILURE);
    //sw.stop();

    if(  x == GST_STATE_CHANGE_FAILURE )
    {
        _LastError = "Failed to change state";
        return false;
    }

	return true;
}

std::string CameraBase::getStateName(GstState x)
{
    std::string name = "unknown";
    switch(x)
    {
        case GST_STATE_VOID_PENDING:
            name = "GST_STATE_VOID_PENDING";
            break;
        case GST_STATE_NULL:
            name = "GST_STATE_NULL";
            break;
        case GST_STATE_READY:
            name = "GST_STATE_READY";
            break;
        case GST_STATE_PAUSED:
            name = "GST_STATE_PAUSED";
            break;
        case GST_STATE_PLAYING:
            name = "GST_STATE_PLAYING";
            break;
    }
    return name;
}

std::string CameraBase::getStateChangeReturnName(GstStateChangeReturn x)
{
    std::string name = "unknown";
    switch(x)
    {
        case GST_STATE_CHANGE_FAILURE:
            name = "GST_STATE_CHANGE_FAILURE";
            break;
        case GST_STATE_CHANGE_SUCCESS:
            name = "GST_STATE_CHANGE_SUCCESS";
            break;
        case GST_STATE_CHANGE_ASYNC:
            name = "GST_STATE_CHANGE_ASYNC";
            break;
        case GST_STATE_CHANGE_NO_PREROLL:
            name = "GST_STATE_CHANGE_NO_PREROLL";
            break;

    }
    return name;
}

bool CameraBase::setSerialNumber(std::string serial)
{

    if(_pipeline == NULL)
	{
        _LastError = "setSerialNumber() : Pipeline is NULL: " + _name;
		return false;
	}	

    if( !isDeviceAvailable(serial))
    {
        _LastError = "setSerialNumber() device is not available: " + _name;
        return false;
    }

    GstElement* tcambin = gst_bin_get_by_name(GST_BIN(_pipeline), "source");

    if( !tcambin)
	{
        _LastError = "setSerialNumber() tcambin not found. Did you forget \"name=tcambin\" after tcambin in your pipeline?: " + _name;
		return false;
	}

    GValue val= {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, _serial.c_str());

    g_object_set_property(G_OBJECT(tcambin), "serial", &val);
    g_value_unset(&val);
    
    gst_object_unref(tcambin); 
    return true;
}

bool CameraBase::isDeviceAvailable(std::string serial)
{
    bool bAvailable = false;
    _LastError = "Device is not available " + _name;

    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    /* retrieve a single linked list of serials of the available devices */
    GSList* serials = tcam_prop_get_device_serials(TCAM_PROP(source));

    for (GSList* elem = serials; elem; elem = elem->next)
    {

        const char* device_serial = (gchar*)elem->data;
        if( serial  == std::string(device_serial ) )
        {
            bAvailable = true;
        }
    }

    g_slist_free_full(serials, g_free);
    gst_object_unref(source);
    if( !bAvailable )
        _LastError = "Device is not available " + _name;
    return bAvailable;
}

/*
cv::Mat CameraBase::snapImage()
{
    cv::Mat frame; 
    GstElement* appsink = gst_bin_get_by_name(GST_BIN(_pipeline), "sink");
    GstSample* sample = NULL;
    
    g_signal_emit_by_name(appsink, "pull-sample", &sample, NULL);
    if (sample)
    {
        int width, height ;
        const GstStructure *str;

        GstBuffer *buffer = gst_sample_get_buffer(sample);

        GstMapInfo info;

        gst_buffer_map(buffer, &info, GST_MAP_READ);
        
        if (info.data != NULL) 
        {
            // info.data contains the image data as blob of unsigned char 

            GstCaps *caps = gst_sample_get_caps(sample);
            // Get a string containg the pixel format, width and height of the image        
            str = gst_caps_get_structure (caps, 0);    
                // Now query the width and height of the image
                gst_structure_get_int (str, "width", &width);
                gst_structure_get_int (str, "height", &height);

            if( strcmp( gst_structure_get_string (str, "format"),"BGRx") == 0)  
            {
                // Create a cv::Mat, copy image data into that and save the image.
                frame.create(height,width,CV_8UC(4));
                memcpy( frame.data, info.data, width*height*4);
            }
            if( strcmp( gst_structure_get_string (str, "format"),"BGR") == 0)  
            {
                // Create a cv::Mat, copy image data into that and save the image.
                frame.create(height,width,CV_8UC(3));
                memcpy( frame.data, info.data, width*height*3);
            }
            if( strcmp( gst_structure_get_string (str, "format"),"RGB") == 0)  
            {
                // Create a cv::Mat, copy image data into that and save the image.
                frame.create(height,width,CV_8UC(3));
                memcpy( frame.data, info.data, width*height*3);
            }
        }

        // Calling Unref is important!
        gst_buffer_unmap (buffer, &info);
        gst_sample_unref(sample);
    }

    return frame;
}
*/
///////////////////////////////////////////////////////////////////////////
// Query the child elements in the tcambin, in case we use tcambin
// Save them in a list for later usage.
//
void CameraBase::queryChildElements()
{
    GstElement* tcambin = gst_bin_get_by_name(GST_BIN(_pipeline), "source");
    _tcambinchilds.clear();

    if (GST_IS_BIN(tcambin))
    {
        GstElement *child;
        GValue value = {};
        g_value_init (&value, GST_TYPE_ELEMENT);

        GstIterator *iterator = gst_bin_iterate_elements (GST_BIN( tcambin));
        while (gst_iterator_next (iterator, &value) == GST_ITERATOR_OK)
        {
            child = (GstElement*)g_value_get_object (&value);
            std::string childname = gst_element_get_name(child);
            _tcambinchilds.push_back(childname);
            //log("\t\t%s\n",childname.c_str());
            g_value_reset (&value);
        }
        g_value_unset (&value);
        gst_iterator_free (iterator);
    }
    gst_object_unref(tcambin);

}

void CameraBase::logTcambinChilds()
{
    for( auto &child : getTcambinChilds())
    {
        //log(fmt::format("\t\t{}\n",child));
    }
}


void CameraBase::drawCameraname(WINDOW* win, int color)
{
    char szCamname[255];
    if( win != NULL)
    {
        sprintf( szCamname,"%2d :  %s : %s",_windowline,_name.c_str() , _serial.c_str() );
        
        wattron( win, COLOR_PAIR(color) );
        mvwprintw(win, _windowline, 1, szCamname );
        wattroff( win, COLOR_PAIR(color) );
        wrefresh( win );
    }
}

///////////////////////////////////////////////////////
// Try to load dutils. Return true on success
// false on fail.
bool CameraBase::isDUtilsInstalled()
{
    GstElement *dutils = gst_element_factory_make("tcamdutils", "");
    if( dutils != NULL)
    {
        gst_object_unref(dutils);
        return true;
    }
    return false;
}

std::shared_ptr<Property>
CameraBase::get_property(std::string name)
{
    GValue value = G_VALUE_INIT;
    GValue min = G_VALUE_INIT;
    GValue max = G_VALUE_INIT;
    GValue default_value = G_VALUE_INIT;
    GValue step_size = G_VALUE_INIT;
    GValue type = G_VALUE_INIT;
    GValue flags = G_VALUE_INIT;
    GValue category = G_VALUE_INIT;
    GValue group = G_VALUE_INIT;

    gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(tcambin_),
                                               (gchar*)name.c_str(),
                                               &value,
                                               &min,
                                               &max,
                                               &default_value,
                                               &step_size,
                                               &type,
                                               &flags,
                                               &category,
                                               &group);

    if (!ret)
    {
        throw std::runtime_error("Failed to query property");
    }
    std::shared_ptr<Property> prop;
    const char *t = g_value_get_string(&type);
    if(!g_strcmp0(t, "integer"))
    {
        std::shared_ptr<IntegerProperty> intprop(new IntegerProperty);
        intprop->value = g_value_get_int(&value);
        intprop->min = g_value_get_int(&min);
        intprop->max = g_value_get_int(&max);
        intprop->default_value = g_value_get_int(&default_value);
        intprop->step_size = g_value_get_int(&step_size);
        prop = intprop;
    }
    else if(!g_strcmp0(t, "double"))
    {
        std::shared_ptr<DoubleProperty> dblprop(new DoubleProperty);
        dblprop->value = g_value_get_double(&value);
        dblprop->min = g_value_get_double(&min);
        dblprop->max = g_value_get_double(&max);
        dblprop->default_value = g_value_get_double(&default_value);
        dblprop->step_size = g_value_get_double(&step_size);
        prop = dblprop;
    }
    else if(!g_strcmp0(t, "string"))
    {
        std::shared_ptr<StringProperty> strprop(new StringProperty);
        strprop->value = std::string(g_value_get_string(&value));
        strprop->default_value = std::string(g_value_get_string(&default_value));
        prop = strprop;
    }
    else if(!g_strcmp0(t, "enum"))
    {
        std::shared_ptr<EnumProperty> enumprop(new EnumProperty);
        enumprop->value = std::string(g_value_get_string(&value));
        enumprop->default_value = std::string(g_value_get_string(&default_value));
        GSList *entries = tcam_prop_get_tcam_menu_entries(TCAM_PROP(tcambin_),
                                                          (gchar*)name.c_str());
        for(GSList *entry = entries; entry; entry = g_slist_next(entry))
        {
            enumprop->values.push_back((char*)entry->data);
        }
        prop = enumprop;
    }
    else if(!g_strcmp0(t, "boolean") || !g_strcmp0(t, "button"))
    {
        std::shared_ptr<BooleanProperty> booleanprop(new BooleanProperty);
        booleanprop->value = g_value_get_boolean(&value);
        booleanprop->default_value = g_value_get_boolean(&default_value);
        prop = booleanprop;
    }
    else
    {
        prop = std::shared_ptr<Property> (new Property);
    }
    prop->name = name;
    prop->group = std::string(g_value_get_string(&group));
    prop->category = std::string(g_value_get_string(&category));
    prop->type = std::string(t);
    return prop;
}



bool
CameraBase::set_property(std::string name, GValue &value)
{
    bool ret;
    ret = tcam_prop_set_tcam_property(TCAM_PROP(tcambin_),
                                      (gchar*)name.c_str(),
                                      &value);
    return ret;
}



//////////////////////////////////////////////////////////////////////////////////

std::string
Property::to_string()
{
    return std::string(type + " property '" + name + "': ");
}

std::string
IntegerProperty::to_string()
{
    return Property::to_string() + "value = " + std::to_string(value) + " " +
           "default = " + std::to_string(default_value) + " " +
           "min = " + std::to_string(min) + " " +
           "max = " + std::to_string(max) + " " +
           "step_size = " + std::to_string(step_size);
}

bool IntegerProperty::set(CameraBase &cam, int _value)
{
    bool ret = FALSE;
    if(type == "integer")
    {
        GValue gval = G_VALUE_INIT;
        g_value_init(&gval, G_TYPE_INT);
        g_value_set_int(&gval, _value);
        ret = cam.set_property(name.c_str(), gval);
    }
    return ret;
}
bool IntegerProperty::get(CameraBase &cam, int &_value)
{
    bool ret = FALSE;
    if(type == "integer")
    {
        std::shared_ptr<IntegerProperty> prop =
            std::dynamic_pointer_cast<IntegerProperty>(cam.get_property(name));
        assert(prop);
        value = _value = prop->value;
        min = prop->min;
        max = prop->max;
        default_value = prop->default_value;
        step_size = prop->step_size;
    }
    return ret;
}




std::string
DoubleProperty::to_string()
{
    return Property::to_string() + "value = " + std::to_string(value) + " " +
    "default = " + std::to_string(default_value) + " " +
    "min = " + std::to_string(min) + " " +
    "max = " + std::to_string(max) + " " +
    "step_size = " + std::to_string(step_size);
}

bool DoubleProperty::set(CameraBase &cam, double _value)
{
    bool ret = FALSE;
    if(type == "double")
    {
        GValue gval = G_VALUE_INIT;
        g_value_init(&gval, G_TYPE_DOUBLE);
        g_value_set_double(&gval, _value);
        ret = cam.set_property(name.c_str(), gval);
    }
    return ret;
}

bool DoubleProperty::get(CameraBase &cam, double &_value)
{
    bool ret = FALSE;
    if(type == "double")
    {
        std::shared_ptr<DoubleProperty> prop =
            std::dynamic_pointer_cast<DoubleProperty>(cam.get_property(name));
        assert(prop);
        value = _value = prop->value;
        min = prop->min;
        max = prop->max;
        default_value = prop->default_value;
        step_size = prop->step_size;
    }
    return ret;
}




std::string
StringProperty::to_string()
{
    return Property::to_string() + "value = '" + value + "' default_value = '" + default_value +"'";
}

bool StringProperty::set(CameraBase &cam, std::string _value)
{
    bool ret = FALSE;
    if((type == "string") || (type == "enum"))
    {
        GValue gval = G_VALUE_INIT;
        g_value_init(&gval, G_TYPE_STRING);
        g_value_set_string(&gval, _value.c_str());
        ret = cam.set_property(name.c_str(), gval);
    }
    return ret;
}

bool StringProperty::get(CameraBase &cam, std::string &_value)
{
    bool ret = FALSE;
    if((type == "string") || (type == "enum"))
    {
        std::shared_ptr<StringProperty> prop =
            std::dynamic_pointer_cast<StringProperty>(cam.get_property(name));
        assert(prop);
        value = _value = prop->value;
    }
    return ret;
}



std::string
EnumProperty::to_string()
{
    std::string ret = StringProperty::to_string() + " values = {";
    for(std::string &value : values)
    {
        ret += value + ", ";
    }
    ret += "}";

    return ret;
}



std::string
BooleanProperty::to_string()
{
    return Property::to_string() + "value = " + std::string(value ? "true" : "false") +
    " default_value = " + std::string(default_value ? "true" : "false");
}

bool
BooleanProperty::get(CameraBase &cam, int &value)
{
    bool ret = FALSE;
    if(type == "boolean")
    {
        std::shared_ptr<BooleanProperty> prop =
            std::dynamic_pointer_cast<BooleanProperty>(cam.get_property(name));
        assert(prop);
        value = prop->value ? 1 : 0;
        ret = TRUE;
    }
    return ret;
}

bool
BooleanProperty::set(CameraBase &cam, int value)
{
    bool ret = FALSE;
    if((type == "boolean") || (type == "button"))
    {
        GValue gval = G_VALUE_INIT;
        g_value_init(&gval, G_TYPE_BOOLEAN);
        g_value_set_boolean(&gval, value ? TRUE : FALSE);
        ret = cam.set_property(name.c_str(), gval);
    }
    return ret;
}




