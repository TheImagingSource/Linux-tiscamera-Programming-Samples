#include "ctcamproperty.h"
#include <stdexcept>

CTcamProperty::CTcamProperty(GstElement *tcamsrc, std::string name)
{
    _tcamsrc = tcamsrc;
    _name = name;
}

//////////////////////////////////////////////////////////////////////////
//
void CTcamProperty::set(std::string value)
{
    GError *error = nullptr;

    TcamPropertyBase* base_property = getBaseProperty(TCAM_PROPERTY_TYPE_ENUMERATION);
    checkLocked(base_property);

    TcamPropertyEnumeration* property_enum = TCAM_PROPERTY_ENUMERATION(base_property);
    tcam_property_enumeration_set_value(property_enum, value.c_str(), &error);
    if( error )
    {
        g_object_unref(base_property);
        throw std::invalid_argument( "Set property \"" + _name + "\" to \"" + value + "\" error: " + error->message );
    }
    g_object_unref(base_property); 
}

//////////////////////////////////////////////////////////////////////////
//
void CTcamProperty::set(gint64 value)
{
    GError *error = nullptr;
    char errortext[1024];
    TcamPropertyBase* base_property = getBaseProperty( TCAM_PROPERTY_TYPE_INTEGER);
    checkLocked(base_property);

    TcamPropertyInteger* integer = TCAM_PROPERTY_INTEGER(base_property);
    gint64 min;
    gint64 max;
    gint64 step;
    tcam_property_integer_get_range(integer, &min, &max, &step, &error);

    if( min <= value && value <= max)
    {
        tcam_property_integer_set_value(integer, value, &error);
        if( error )
        {
            sprintf(errortext,"Set property \"%s\" to %ld error: %s\n",_name.c_str(), value,error->message);
            g_object_unref(base_property);
            throw std::invalid_argument( errortext );
        }
    }
    else
    {
        sprintf(errortext,"Set property \"%s\" to %ld error: value out of range of %ld - %ld \n",_name.c_str(), value,min, max);
        g_object_unref(base_property);
        throw std::invalid_argument( errortext );
    }
    g_object_unref(base_property);
}

//////////////////////////////////////////////////////////////////////////
//
void CTcamProperty::set(double value)
{
    GError *error = nullptr;
    char errortext[1024];

    TcamPropertyBase* base_property = getBaseProperty( TCAM_PROPERTY_TYPE_FLOAT);
    checkLocked(base_property);

    TcamPropertyFloat* floatprop = TCAM_PROPERTY_FLOAT(base_property);
    gdouble min;
    gdouble max;
    gdouble step;
    tcam_property_float_get_range(floatprop, &min, &max, &step, &error);

    if( min <= value && value <= max)
    {
        tcam_property_float_set_value(floatprop, value, &error);
        if( error )
        {
            sprintf(errortext,"Set property \"%s\" to %f error: %s\n",_name.c_str(), value,error->message);
            g_object_unref(base_property);
            throw std::invalid_argument( errortext );
        }
    }
    else
    {
        sprintf(errortext,"Set property \"%s\" to %f error: value out of range of %f - %f \n",_name.c_str(), value,min, max);
        g_object_unref(base_property);
        throw std::invalid_argument( errortext );
    }
    g_object_unref(base_property);

}

//////////////////////////////////////////////////////////////////////////
//
void CTcamProperty::set(bool value)
{
    GError *error = nullptr;
    TcamPropertyBase* base_property = getBaseProperty( TCAM_PROPERTY_TYPE_BOOLEAN);
    checkLocked(base_property);

    TcamPropertyBoolean* prop = TCAM_PROPERTY_BOOLEAN(base_property);
    tcam_property_boolean_set_value(prop, value, &error);
    if( error )
    {
        g_object_unref(base_property);
        throw std::invalid_argument( "Set property \"" + _name +  "\" error: " + error->message );
    }
    g_object_unref(base_property); 
}

//////////////////////////////////////////////////////////////////////////
//
void CTcamProperty::set()
{
    GError *error = nullptr;
    TcamPropertyBase* base_property = getBaseProperty( TCAM_PROPERTY_TYPE_COMMAND);
    checkLocked(base_property);

    TcamPropertyCommand* prop = TCAM_PROPERTY_COMMAND(base_property);
    tcam_property_command_set_command(prop, &error);
    if( error )
    {
        g_object_unref(base_property);
        throw std::invalid_argument( "Set property \"" + _name + "\" error: " + error->message );
    }
    g_object_unref(base_property); 
}

//////////////////////////////////////////////////////////////////////////
//

void CTcamProperty::get(std::string &value)
{
    GError *error = nullptr;
    TcamPropertyBase* base_property = getBaseProperty( TCAM_PROPERTY_TYPE_ENUMERATION);
    TcamPropertyEnumeration* prop = TCAM_PROPERTY_ENUMERATION(base_property);

    value = tcam_property_enumeration_get_value(prop, &error);
    if( error )
    {
        g_object_unref(base_property);
        throw std::invalid_argument( "Get property \"" + _name + "\" error: " + error->message );
    }
    g_object_unref(base_property); 
}

void CTcamProperty::get(gint64 &value)
{
    GError *error = nullptr;
    TcamPropertyBase* base_property = getBaseProperty( TCAM_PROPERTY_TYPE_INTEGER);
    TcamPropertyInteger* prop = TCAM_PROPERTY_INTEGER(base_property);
    value = tcam_property_integer_get_value(prop, &error);
    if( error )
    {
        g_object_unref(base_property);
        throw std::invalid_argument( "Get property \"" + _name + "\" error: " + error->message );
    }
    g_object_unref(base_property); 
}
void CTcamProperty::get(double &value)
{
    GError *error = nullptr;
    TcamPropertyBase* base_property = getBaseProperty( TCAM_PROPERTY_TYPE_FLOAT);
    TcamPropertyFloat* prop = TCAM_PROPERTY_FLOAT(base_property);
    value = tcam_property_float_get_value(prop, &error);
    if( error )
    {
        g_object_unref(base_property);
        throw std::invalid_argument( "Get property \"" + _name + "\" error: " + error->message );
    }
    g_object_unref(base_property); 

}
void CTcamProperty::get(bool &value)
{
    GError *error = nullptr;
    TcamPropertyBase* base_property = getBaseProperty( TCAM_PROPERTY_TYPE_BOOLEAN);
    TcamPropertyBoolean* prop = TCAM_PROPERTY_BOOLEAN(base_property);
    value = tcam_property_boolean_get_value(prop, &error);
    if( error )
    {
        g_object_unref(base_property);
        throw std::invalid_argument( "Get property \"" + _name + "\" error: " + error->message );
    }
    g_object_unref(base_property); 
}



//////////////////////////////////////////////////////////////////////////
//
std::string CTcamProperty::getPropertyTypeName(TcamPropertyType type)
{
    switch(type)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
            return "integer";
            break;
        case TCAM_PROPERTY_TYPE_FLOAT:                
            return  "float";
            break;
        case TCAM_PROPERTY_TYPE_ENUMERATION:
            return "enumeration";
            break;
        case TCAM_PROPERTY_TYPE_BOOLEAN:
            return  "boolean";
            break;
        case TCAM_PROPERTY_TYPE_COMMAND:
            return  "command";
            break;
        default:
            return  "unknown";
            break;
    }
    return  "unknown";
}

//////////////////////////////////////////////////////////////////////////
//
TcamPropertyBase* CTcamProperty::getBaseProperty( TcamPropertyType type)
{
    GError *error = nullptr;
    std::string errormessage;
    TcamPropertyBase* base_property = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(_tcamsrc), _name.c_str(), &error);
    if( error )
    {
        errormessage ="Query Property \"" + _name + "\" error: " + error->message;

        throw std::invalid_argument( errormessage );
        //printf("Set property \"%s\" error: %s\n",name.c_str(), error->message);
        return nullptr;
    }

    if( tcam_property_base_get_property_type(base_property) != type )
    {
        errormessage ="Query Property \"" + _name + "\" error: wrong type. Correct is " +
                getPropertyTypeName( tcam_property_base_get_property_type(base_property));
        throw std::invalid_argument( errormessage );
        return nullptr;
    }
    return base_property;
}

//////////////////////////////////////////////////////////////////////////
//
void CTcamProperty::checkLocked(TcamPropertyBase* base_property)
{
    GError *error = nullptr;
    if( tcam_property_base_is_locked(base_property, &error)) 
        throw std::invalid_argument( "Set property \"" + _name + "\" is locked" );
}