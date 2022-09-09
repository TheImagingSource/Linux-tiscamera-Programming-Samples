#include <gst/gst.h>
#include "tcam-property-1.0.h" /* gobject introspection interface */
#include <string>

class CTcamProperty
{
    public:
        CTcamProperty(GstElement *tcamsrc, std::string name);
        void set(std::string value);
        void set(gint64 value);
        void set(double value);
        void set(bool value);
        void set();

        void get(std::string &value);
        void get(gint64 &value);
        void get(double &value);
        void get(bool &value);

        static std::string getPropertyTypeName(TcamPropertyType type);

    private:
        TcamPropertyBase* getBaseProperty(TcamPropertyType type);
        void checkLocked(TcamPropertyBase* base_property);

        GstElement *_tcamsrc;
        std::string _name;
};