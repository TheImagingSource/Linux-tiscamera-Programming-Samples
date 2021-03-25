////////////////////////////////////////////////////////////////////
/*
Terminal property dialog for cameras, based on the famous ncurses library.

sudo apt-get install libncurses5-dev
sudo apt-get install libpthread-stubs0-dev

If tiscamera was installed as deb pakacge only:
sudo apt-get install libgstreamer1.0-0 gstreamer1.0-dev gstreamer1.0-tools gstreamer1.0-doc
sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
sudo apt-get install libgirepository1.0-dev

Building with
mkdir build
cd build
cmake ..
make
./tcam-dialog
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <gst/gst.h>
#include <iostream>
#include <fstream>
#include "tcamprop.h"
#include <unistd.h> 
#include "controls/win.h"
#include "camera/camerabase.h"

using namespace std;

WINDOWS_t Windows;


struct CMDLINEPARAM_t
{
    std::string passedSerial = "";
    std::string pipeline = "tcambin name=source ! fakesink";
    bool streaming = false;
};



static void show_usage()
{
    std::cerr << "Usage: tcam-tester <option(s)> "
              << "Options:\n"
              << "\t-h\t\tShow this help message\n"
              << "\t-play starts the (internal) pipeline. That will show software properties.\n"
              << "\t-p <pipline> pass a pipeline. It must start with \"tcambin name=source ! \".\n"
              << "\t\t The default pipeline is \"tcambin name=source ! fakesink\".\n"
              << "\t-d Uses a simple display pipline.\n"
              << std::endl;

    exit(0);
}

int parseCmdLineparams(int argc, char **argv, CMDLINEPARAM_t &p)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) 
        {
            show_usage();
            return 0;
        } 
        else if (arg == "-p")
        {
            if (i + 1 < argc)
            { 
                p.pipeline = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
                p.streaming = true;
            } 
            else 
            { 
                  std::cerr << "-p needs a pipeline string!" << std::endl;
                return 1;
            }  
        } 
        else if (arg == "-play")
        {
            p.streaming = true;
        } 
        else if (arg == "-d")
        {
            p.pipeline = "tcambin name=source ! video/x-raw, format=BGRx ! videoconvert ! videoscale ! video/x-raw, width=320, height=240 ! ximagesink";
            p.streaming = true;
        } 
        else 
        {
            std::cerr << "unknown parameter: " << argv[i] << std::endl;
            show_usage();
            return 1;
        }  

    }
    return 0;
}

// comparison, not case sensitive.
bool compare_cameras( CameraBase& first,  CameraBase& second)
{
  unsigned int i=0;
  std::string firstname = first.Name().substr(4);
  std::string secondname = second.Name().substr(4);

  while ( (i<firstname.length()) && (i<secondname.length()) )
  {
    if (tolower(firstname[i])<tolower(secondname[i])) return true;
    else if (tolower(firstname[i])>tolower(secondname[i])) return false;
    ++i;
  }
  return ( firstname.length() < secondname.length() );
}


void queryAvailableCameras(std::vector <CameraBase> &connectedcameras, const CMDLINEPARAM_t cmdlineparameter)
{
    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    /* retrieve a single linked list of serials of the available devices */
    GSList* serials = tcam_prop_get_device_serials(TCAM_PROP(source));
    int i = 0;
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
            connectedcameras.push_back(CameraBase((gchar*)elem->data, std::string(name),-1 ));
            g_free( name );
            g_free( identifier );
            g_free( connection_type );
        }
    }

    g_slist_free_full(serials, g_free);
    gst_object_unref(source);

}

std::string getPropertyValue(TcamProp* tcamprop, const char* propname, FIELD *pField )
{
    std::string retval;
    GValue value = {};
    GValue min = {};
    GValue max = {};
    GValue default_value = {};
    GValue step_size = {};
    GValue type = {};
    GValue flags = {};
    GValue category = {};
    GValue group = {};

    gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(tcamprop),
                                                propname,
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
        printf("Could not query property '%s'\n", propname);
        return "n/a";
    }

    const char* t = g_value_get_string(&type);
    if (strcmp(t, "integer") == 0)
    {
        retval = std::to_string(g_value_get_int(&value));
        // set_field_type( pField, TYPE_INTEGER, 0,g_value_get_int(&min), g_value_get_int(&max) );
        set_field_type( pField, TYPE_INTEGER, 1,0,0);
    }
    else if (strcmp(t, "double") == 0)
    {
        retval = std::to_string(g_value_get_double(&value));

        //set_field_type( pField, TYPE_NUMERIC, 1,(int) g_value_get_double(&min), (int)( g_value_get_double(&max) + 0.5) );
        set_field_type( pField, TYPE_NUMERIC, 2,g_value_get_double(&min),( g_value_get_double(&max) + 0.5) );
    }

    else if (strcmp(t, "string") == 0)
    {
        retval = g_value_get_string(&value); 
        set_field_type( pField, TYPE_ALNUM, 35 );
    }
    else if (strcmp(t, "enum") == 0)
    {
        retval = g_value_get_string(&value); 
        // Query the allowed string / menu values for this property
        GSList* entries = tcam_prop_get_tcam_menu_entries(tcamprop, propname);
        
        // Allocate memory for the validation list
        char **vals = (char**)calloc( g_slist_length(entries) + 1, sizeof(char*));

        for (unsigned int x = 0; x < g_slist_length(entries); ++x)
        {
            // Allocate memory for each string
            vals[x] = new char[strlen( (char*)g_slist_nth(entries, x)->data)+1];
            strcpy(vals[x],(char*)g_slist_nth(entries, x)->data );
            vals[x+1] = NULL; // Last entry in validation list must be NULL
        }
        g_slist_free_full( entries, g_free );

        set_field_type( pField, TYPE_ENUM, vals, 1,1);
    }
    else if (strcmp(t, "boolean") == 0)
    {
        retval = g_value_get_boolean(&value) ? "true" : "false"; 

        char **vals = (char**)calloc(3, sizeof(char*));
        vals[0] = new char[7];
        strcpy(vals[0],"false");
        vals[1] = new char[7];
        strcpy(vals[1],"true");
        vals[2] = NULL;
        set_field_type( pField, TYPE_ENUM, vals, 1,1);

    }
    else if (strcmp(t, "button") == 0)
    {
        retval = "Button";
        set_field_type( pField, TYPE_ALNUM,6 );
    }
    g_value_unset( &value );
    g_value_unset( &min );
    g_value_unset( &max );
    g_value_unset( &default_value );
    g_value_unset( &step_size );
    g_value_unset( &type );
    g_value_unset( &flags );
    g_value_unset( &category );
    g_value_unset( &group );
    
    return retval;
}

///////////////////////////////////////////////////////////////////////
// Copy all property names into a std::list, sort the list and
// return it.
std::list<std::string> getSortedPropertyNames(TcamProp* tcamprop)
{
    std::list<std::string> propertynames;
    GSList* names = tcam_prop_get_tcam_property_names(tcamprop);

    // Copy the property names into a std::list, so we can sort them
    for ( GSList* cur = names; cur != NULL; cur = cur->next )
    {
        propertynames.push_back((char*)cur->data);
    }
    g_slist_free_full(names,g_free);

    propertynames.sort();
    return propertynames;
}

//////////////////////////////////////////////////////////////
// Return the field's buffer without whitespaces
std::string getFieldBuffer(FIELD *pField)
{
    const std::string& chars = "\t\n\v\f\r ";
    std::string buffer =  field_buffer(pField,0);
    buffer.erase(0, buffer.find_first_not_of(chars));
    buffer.erase(buffer.find_last_not_of(chars) + 1);
    return buffer;
}

////////////////////////////////////////////////////////////////////////////
// Return the type of a property.
std::string getPropertyType( std::string propertname, TcamProp* tcamprop)
{
    GValue value = {};
    GValue min = {};
    GValue max = {};
    GValue default_value = {};
    GValue step_size = {};
    GValue type = {};
    GValue flags = {};
    GValue category = {};
    GValue group = {};

    gboolean ret = tcam_prop_get_tcam_property(tcamprop,
                                                propertname.c_str(),
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
        printf("Could not query property '%s'\n", propertname.c_str());
        return "";
    }

    std::string t = g_value_get_string(&type);

    g_value_unset( &value );
    g_value_unset( &min );
    g_value_unset( &max );
    g_value_unset( &default_value );
    g_value_unset( &step_size );
    g_value_unset( &type );
    g_value_unset( &flags );
    g_value_unset( &category );
    g_value_unset( &group );

    return t; // That is a string containig the type.

}

////////////////////////////////////////////////////////////////////////////
// Return the type of a property.
std::string getPropertyRange( std::string propertyname, TcamProp* tcamprop, int *imin, int *imax)
{
    *imin = 0;
    *imax = 0;
    std::string retval = "";

    GValue value = {};
    GValue min = {};
    GValue max = {};
    GValue default_value = {};
    GValue step_size = {};
    GValue type = {};
    GValue flags = {};
    GValue category = {};
    GValue group = {};

    gboolean ret = tcam_prop_get_tcam_property(tcamprop,
                                                propertyname.c_str(),
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
        printf("Could not query property '%s'\n", propertyname.c_str());
        return "";
    }

    const char* t = g_value_get_string(&type);
    if (strcmp(t, "integer") == 0)
    {
        *imin = g_value_get_int(&min);
        *imax = g_value_get_int(&max); 
        retval = "  (" + std::to_string(*imin ) + " ~ " + std::to_string(*imax) + ")";
    }
    else if (strcmp(t, "double") == 0)
    {
        *imin = (int)g_value_get_double(&min);
        *imax = (int)(g_value_get_double(&max) +0.5); 
        retval = "  (" + std::to_string(*imin ) + " ~ " + std::to_string(*imax) + ")";
    }



    g_value_unset( &value );
    g_value_unset( &min );
    g_value_unset( &max );
    g_value_unset( &default_value );
    g_value_unset( &step_size );
    g_value_unset( &type );
    g_value_unset( &flags );
    g_value_unset( &category );
    g_value_unset( &group );

    return retval; // That is a string containig the type.

}


////////////////////////////////////////////////////////////////
// Set a property to the value in a field.
//
void setProperty(FIELD *pField, TcamProp* tcamprop  )
{
    std::string buffer = getFieldBuffer( pField);
    std::string propertyname = std::string( (char*)field_userptr( pField) ) ;
    std::string type = getPropertyType( propertyname, tcamprop);

    GValue gval = G_VALUE_INIT;

    if( type == "integer")
    {
        g_value_init(&gval, G_TYPE_INT);
        g_value_set_int(&gval, atoi( buffer.c_str()));
    }else
    if( type == "double")
    {
        g_value_init(&gval, G_TYPE_DOUBLE);
        g_value_set_double(&gval, atof( buffer.c_str()));
    }else
    if( type == "boolean")
    {
        g_value_init(&gval, G_TYPE_BOOLEAN);
        g_value_set_boolean(&gval, buffer=="true" ? true:false );
    }else
    if( type == "enum")
    {
        g_value_init(&gval, G_TYPE_STRING);
        g_value_set_string(&gval,(gchar*)buffer.c_str());

    }else
    if( type == "button")
    {
        g_value_init(&gval, G_TYPE_BOOLEAN);
        g_value_set_boolean(&gval, true);
    }else
    {
        return;
    }

    tcam_prop_set_tcam_property(TCAM_PROP(tcamprop),
                                      (gchar*)propertyname.c_str(),
                                      &gval);
}


//////////////////////////////////////////////////////////////////////////////
// Creates the lables and input fiels. Returns the number of created pages
int createFields( FIELD **field,  std::list<std::string>propertynames, TcamProp* tcamprop,int lines )   
{
    int cur_y = 0, cur_x = 1; 
    int i = 0;
    int pages = 0;
    int rangemin = 0;
    int rangemax = 0;
    std::string tmpname;

    for( auto name : propertynames )
    {
        // Lable
        field[i] = new_field(1, 40, cur_y, cur_x, 0, 0);
        set_field_opts(field[i], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
        
        tmpname = name + getPropertyRange(name, tcamprop, &rangemin, &rangemax );

		set_field_buffer(field[i], 0, tmpname.c_str());
        set_field_status(field[i],FALSE);

        i++;
        if (cur_y % lines == 0) {

			cur_y = 0;
			set_new_page(field[i-1], TRUE);
			move_field(field[i-1], cur_y, cur_x);
            pages++;
		}
        
        // Input field
        field[i] = new_field(1, 37, cur_y, cur_x + 42, 0, 0);
		set_field_back(field[i], A_UNDERLINE);
        set_field_opts(field[i], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
		set_field_buffer(field[i], 0, getPropertyValue(tcamprop, name.c_str(), field[i]).c_str());
//        set_field_fore(field[i], COLOR_PAIR(color));
//	    set_field_back(field[i], COLOR_PAIR(color));

        char *userptr = new char[name.size()+1];
        strcpy( userptr,name.c_str());
        set_field_userptr(field[i], userptr);

        i++;
        cur_y+=1;
    }
    field[i] = NULL;
    return pages;
} 



////////////////////////////////////////////////////////////////
// from https://gist.github.com/alan-mushi/bdc831e0c33ad5db8025
// https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/forms.html
//
void showProperties(CameraBase cam, CMDLINEPARAM_t &params)
{
    int i = 1;
    int fieldCount = 0;
    int lines = 18;
    int cols = 80;
    int pages = 1;

    WINDOW *wbody = newwin( lines+6, cols+2, 1, 31 ); // create a new window
	box( wbody, 0, 0 ); // sets default borders for the window
	mvwprintw(wbody,0,1, ( cam.Name() + " " + cam.Serial() + " Properties").c_str());   

    mvwprintw(wbody,lines+2,2,"Use cursor left and right to select at strings");   
    mvwprintw(wbody,lines+3,2,"Hit Enter Key on Button properties");   
    mvwprintw(wbody,lines+4,2,"Press Esc to return to device selection.");   

    wrefresh( wbody );
    int cur_page = 0;


    WINDOW *inner;
    FIELD **field;
    FORM *form;
    //int cur_y = 0, cur_x = 1; 

    keypad(wbody, TRUE);

    cam.createPipeline(params.pipeline);
    //cam.createPipeline("tcambin name=source ! video/x-raw, format=BGRx ! videoconvert ! videoscale ! video/x-raw, format=BGRx,width=320,height=240 ! ximagesink");
    cam.setSerialNumber(cam.Serial());
    if( params.streaming)
    {
        if( !cam.startPipeline() )
        {
            mvwprintw(wbody,2,2,"Pipeline could not start!"); 
            wrefresh( wbody );
          	delwin(wbody);
            return;
        }
    }

    TcamProp* tcamprop = cam.getTcamProp();
    if(tcamprop == NULL)
    {
        mvwprintw(wbody,2,2,"tcambin or tcamsrc not found.! Forgot \"tcambin name=source\"?"); 
        wrefresh( wbody );
        delwin(wbody);
        return;
    }

    std::list<std::string>propertynames = getSortedPropertyNames( tcamprop );

    // Labels and input fields, + 1 because last entry is NULL.
    fieldCount = propertynames.size() * 2 + 1;
    field = (FIELD**)calloc(fieldCount , sizeof(FIELD *));

    pages = createFields(field, propertynames, tcamprop, lines );

    form = new_form(field);
    set_form_win(form, wbody);
	inner = derwin(wbody, lines, cols, 1 ,1);

	set_form_sub(form, inner);

	post_form(form);
    char cpages[20];
    sprintf(cpages,"Page %d/%d", cur_page+1,pages);
    mvwprintw(wbody, 0,cols -12  , cpages); 
    wrefresh(wbody);

    form_driver(form,REQ_FIRST_FIELD);
    curs_set(2);
    int ch;
    bool bEnd = false;
    while (!bEnd) 
    {
        ch = wgetch(wbody);
		switch (ch) 
        {
			case KEY_DOWN:
				form_driver(form, REQ_NEXT_FIELD);
				form_driver(form, REQ_END_LINE);
				break;

			case KEY_UP:
				form_driver(form, REQ_PREV_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
			
			case KEY_NPAGE:
                if( cur_page < pages -1 )
                {
				    form_driver(form, REQ_NEXT_PAGE);
                    set_form_page(form, ++cur_page);
                    sprintf(cpages,"Page %d/%d", cur_page+1,pages);
                    mvwprintw(wbody,0,cols -12 , cpages); 
                }
				break;

			case KEY_PPAGE:
                if( cur_page > 0 )
                {
                    form_driver(form, REQ_PREV_PAGE);
                    set_form_page(form, --cur_page);
                    sprintf(cpages,"Page %d/%d", cur_page+1,pages);
                    mvwprintw(wbody,0,cols -12  , cpages); 
                }
				break;

            case KEY_LEFT:
                {
                    FIELD *f = current_field( form );
                    if( field_type( f) == TYPE_ENUM)
                    {
                        form_driver(form,REQ_PREV_CHOICE);
                        form_driver(form, REQ_VALIDATION);
                        setProperty(f, tcamprop);
                    }
                    else
                        form_driver(form, REQ_PREV_CHAR);
                }
                break;

            case KEY_RIGHT:
                {
                    FIELD *f = current_field( form );
                    if( field_type( f) == TYPE_ENUM)
                    {
                        form_driver(form,REQ_NEXT_CHOICE);
                        form_driver(form, REQ_VALIDATION);
                        setProperty(f, tcamprop);
                    }
                    else
                        form_driver(form, REQ_NEXT_CHAR);
                }
                break;

            // Delete the char before cursor
            case KEY_BACKSPACE:
            case 127:
                form_driver(form, REQ_DEL_PREV);
                break;

            // Delete the char under the cursor
            case KEY_DC:
                form_driver(form, REQ_DEL_CHAR);
                break;
            case 27:
                bEnd = true;
                break;

            case KEY_ENTER:
            case 10:     
                form_driver(form, REQ_VALIDATION);
                setProperty(current_field( form ), tcamprop);
                break;

            default:
                form_driver(form,ch);
                break;
		}


	}
    curs_set(1);


    unpost_form(form);

	for (i = 0; i < fieldCount; i++) {
		free(field_buffer(field[i], 0));
		free_field(field[i]);
	}

	free_form(form);
	delwin(wbody);
	delwin(inner);

    if( params.streaming)
        cam.stopPipeline();
    cam.unrefPipeline();
}

















////////////////////////////////////////////////////////////
//
int main(int argc, char **argv)  
{
    CMDLINEPARAM_t cmdlineparameter;
    std::vector <CameraBase> connectedcameras;
    std::vector <std::string >cameraitems;
	gst_init(&argc, &argv);
    bool bEnd = false;
    WINDOW *w;
	int ch, i = 0; 


/*  
    PI does not like that
    setenv("TCAM_GIGE_PACKET_SIZE","9000",1); 
    setenv("TCAM_ARV_PACKET_REQUEST_RATIO","0.5",1); 
  */  

    if( parseCmdLineparams(argc,argv,cmdlineparameter) )
        return 1;

    createWindows();

    queryAvailableCameras(connectedcameras,cmdlineparameter);
    //connectedcameras.sort(compare_cameras);



    w = newwin( 24, 30, 1, 1 ); // create a new window
	box( w, 0, 0 ); // sets default borders for the window
	mvwprintw(w,0,1,"Select Device");
    // now print all the menu items and highlight the first one

  	for( i=0; i< connectedcameras.size(); i++ ) 
    {
		if( i == 0 ) 
			wattron( w, A_STANDOUT ); // highlights the first item.
		else
			wattroff( w, A_STANDOUT );
        cameraitems.push_back(connectedcameras[i].Name() + " " + connectedcameras[i].Serial());

		mvwprintw( w, i+1, 2, "%s", cameraitems[i].c_str());
    }

    cameraitems.push_back("Quit");
	mvwprintw( w, i+1, 2, cameraitems[i].c_str());


	wrefresh( w ); // update the terminal screen

	i = 0;
	noecho(); // disable echoing of characters on the screen
	keypad( w, TRUE ); // enable keyboard input for the window.
	curs_set( 0 ); // hide the default screen cursor.
	
       // get the input
	while( !bEnd )
    { 
        ch = wgetch(w);
        // right pad with spaces to make the items appear with even width.
        mvwprintw( w, i+1, 2, "%s", cameraitems[i].c_str());
        // use a variable to increment or decrement the value based on the input.
        switch( ch ) 
        {
            case KEY_UP:
                        i--;
                        i = ( i < 0 ) ? 4 : i;
                        break;
            case KEY_DOWN:
                        i++;
                        i = ( i >= cameraitems.size() ) ? 0 : i;
                        break;

            case KEY_ENTER:
            case 10:
                    if( i == cameraitems.size() -1 )
                        bEnd = true;
                    else
                    {
                        wattroff( w, A_STANDOUT );
                        mvwprintw( w, i+1, 2, "%s", cameraitems[i].c_str());
                        wrefresh(w);
                        showProperties(connectedcameras[i], cmdlineparameter );
                    }
                        break;

        }
        // now highlight the next item in the list.
        wattron( w, A_STANDOUT );
			
        mvwprintw( w, i+1, 2, "%s", cameraitems[i].c_str());
    
        wattroff( w, A_STANDOUT );
	}
    
	delwin( w );
    endwin();
    return 0;
}
