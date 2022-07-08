#ifndef __CAMERABASE
#define __CAMERABASE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <vector>
#include <memory>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include "tcamprop.h"
#include <curses.h>
#include <mutex>
#include <memory>


class CameraBase;

class Property
{
public:
    std::string name;
    std::string category;
    std::string group;
    std::string type;

    virtual std::string to_string();

    // Convenience getters / setters
    virtual bool get(CameraBase &cam, int &value) {return false;}
    virtual bool get(CameraBase &cam, double &value) {return false;}
    virtual bool get(CameraBase &cam, std::string &value) {return false;}
    virtual bool set(CameraBase &cam, int value)  {return false;}
    virtual bool set(CameraBase &cam, double value) {return false;}
    virtual bool set(CameraBase &cam, std::string value) {return false;}
};

class IntegerProperty : public Property
{
public:
    int value;
    int default_value;
    int min;
    int max;
    int step_size;

	virtual std::string to_string() override;
	virtual bool get( CameraBase &cam, int &value ) override;
	virtual bool set(CameraBase &cam, int value) override;
};

class DoubleProperty : public Property
{
public:
    double value;
    double default_value;
    double min;
    double max;
    double step_size;

    virtual std::string to_string() override;
    virtual bool get(CameraBase &cam, double &value) override;
    virtual bool set(CameraBase &cam, double value) override;
};


class StringProperty : public Property
{
public:
    std::string value;
    std::string default_value;

    virtual std::string to_string();
    virtual bool get(CameraBase &cam, std::string &value) override;
    virtual bool set(CameraBase &cam, std::string value) override;
};

class EnumProperty : public StringProperty
{
public:
    std::vector<std::string> values;

    virtual std::string to_string() override;
};

class BooleanProperty : public Property
{
public:
    bool value;
    bool default_value;

    virtual std::string to_string() override;
    virtual bool get(CameraBase &cam, int &value) override;
    virtual bool set(CameraBase &cam, int value) override;
};


class ButtonProperty : public BooleanProperty
{
public:
    virtual bool set(CameraBase &cam, int value=true) override;
private:
    virtual bool get(CameraBase &cam, int &value) override;
};

struct BUSUSERDATA_t
{
    BUSUSERDATA_t()
    {
        loop = NULL;
        _keypressed = false;
    }

    void setBusMessage( std::string msg )
    {
        std::lock_guard<std::mutex> lck(busmutex);
        lastBusMessage = msg;
    }

    std::string getBusMessage()
    {
        std::lock_guard<std::mutex> lck(busmutex);
        return lastBusMessage;
    }

    void setKeyPress(bool set)
    {
        std::lock_guard<std::mutex> lck(busmutex);
        _keypressed = set;
    }

    bool getKeyPress()
    {
        std::lock_guard<std::mutex> lck(busmutex);
        return _keypressed;
    }


    GMainLoop* loop;
    private:
        std::string lastBusMessage;
        std::mutex busmutex;
        bool _keypressed;

};


class CameraBase
{
    public:
        //CameraBase( const Json::Value& camera );
        CameraBase( std::string serial, std::string name, int winline = 0 );
        //CameraBase( const CameraBase& cb) = delete;

        std::string Serial(){return _serial;};
        std::string Name(){return _name;};
        std::string getLastError(){return _LastError;};
        std::string getLastBusMessage(){return _bususerdata->getBusMessage();};

        bool getKeyPress(){return _bususerdata->getKeyPress(); }
        void setKeyPress(bool set){ _bususerdata->setKeyPress(set); }
        void run();

        void findSerialnumber();
        bool isDeviceAvailable(std::string serial);

        bool createPipeline( std::string pipelinestring);
        bool preparePipeline();
        bool startPipeline();
        bool stopPipeline();
        void unrefPipeline();
        void logTcambinChilds();

        TcamProp* getTcamProp(){return TCAM_PROP(tcambin_); };

        bool setSerialNumber(std::string serial);
        //cv::Mat snapImage();

        static bool isDUtilsInstalled();

        std::list<std::string> &getTcambinChilds(){return _tcambinchilds;};

        std::shared_ptr<Property> get_property(std::string name);
        bool set_property(std::string name, GValue &value);

        std::string _name;
        std::string _serial;
       
        GstElement* _pipeline;

        //Windowing Stuff
        void setCameraWindowLine( int line){ _windowline = line;};
        void drawCameraname(WINDOW* win, int color);

    private:
        void init();
        bool changeState( GstState state);
        std::string getStateName(GstState x);
        std::string getStateChangeReturnName(GstStateChangeReturn x);
        void queryChildElements();
        void createMainLoop();
    
        std::list<std::string> _tcambinchilds;
        std::string _LastError;
        std::shared_ptr<BUSUSERDATA_t> _bususerdata;
    
		GstBus* _bus;
		guint _bus_watch_id;
        pthread_t _mainloopthread;

        GstElement *tcambin_;
        //static gboolean bus_call (GstBus* bus, GstMessage* msg, gpointer data);

        //Windowing stuff
        int _windowline; 


};

#endif