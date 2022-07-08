# GTK, Glade, OpenCV, Tiscamera in C++
GTK, like QT5, is a Graphical User Interface (GUI) library. [Glade](https://glade.gnome.org/) is a graphical tool for creating nice GUIs in a more or less simple way. Its advantage is the extremely reduced coding effort and declaring of all the widgets, signals and connections automatically. That is done by the [GtkBuilder](https://developer.gnome.org/gtk3/stable/GtkBuilder.html). This works very fine on plain C projects. However, OpenCV wants C++. The C++ port for the GtkBuilder is [Gtkmm](https://www.gtkmm.org/en/). But this does not make the signal connections to the widget event handlers automatically. One advantage of using Glade is away.

This sample shows, how to use all these tools together in a C++ project.

The program consists of the glade file, the main.cpp and the makefile. In the makefile the path to the header files and the libraries to be linked are queried with the [pgk-config](https://www.freedesktop.org/wiki/Software/pkg-config/) tool 

## makefile

```
GTKLIB=`pkg-config --cflags --libs gtk+-3.0` 
GSTLIB=`pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0`
```

The use of OpenCV is optional, therefore, in the make file it is checked, whether OpenCV is installed. If so, pkg-config is used again. Additionally a define for the compiler is made, so the OpenCV code is compiled:
```
# Check for OpenCV being installed. 
PACK=opencv
PACK:=$(shell pkg-config --exists $(PACK) && echo '$(PACK)')
ifneq ($(PACK),)
	OPENCVLIB=`pkg-config --cflags --libs $(PACK);` 
endif

ifneq ($(OPENCVLIB), )
	FLAGS = -DHAVEOPENCV
else
$(info OpenCV not found )
endif
```
OpenCV is installed with
```
sudo apt install libopencv-dev
```

The tiscamera headers are needed too.
```
TCAMLIB=`pkg-config --cflags --libs tcam` -rdynamic 
```
The "-rdynamic" flag must be added for the GtkBuilder. I suppose, it is wrong here, but it works.

The remaining part in the makefile is standard with all variables passed.

## Use Glade in C++
The trick doing that was found at Stackoverflow's article [I'm missing something obvious with glade gtkbuilder and connecting signals. Help?](https://stackoverflow.com/questions/4098636/im-missing-something-obvious-with-glade-gtkbuilder-and-connecting-signals-help). One of the comments are about using extern "C"{}, which indeed does the job. All functions used as handler for widgets specified in the Glade generated XML file must be within the extern "C" code. In the main.cpp that is
```C++
/////////////////////////////////////////////////////////////////
// Widget event handlers. Must be extern "C"

extern "C" 
{
//////////////////////////////////////////////////////////////////
// Called from quit menu
void filemenuend(GtkMenuItem *menuitem, gpointer data)
{
	CleanUpandEndProgram((GtkMessageCustomData*)data);
}

////////////////////////////////////////////////////////////////////
// Called by close window button
void mainwindow_destroy(GtkWidget *object, gpointer data)
{
	CleanUpandEndProgram((GtkMessageCustomData*)data);
}

//////////////////////////////////////////////////////////////////
// Called from "Save Image" menu
void on_menuSaveImage_activate(GtkWidget *object, gpointer data)
{
	SaveImage( (GtkMessageCustomData*)data );
}

} //extern "C"
```
Either things can be handled in there or other functions outside this extern "C" can be called. Once figured that out, we can use OpenCV and enjoy the advantages to the GtkBuilder together with Glade.

## Display Live Video on a Gtk Widget
For this task the Gstreamer [GtkSink](https://gstreamer.freedesktop.org/documentation/gtk/gtksink.html?gi-language=c) and a [GtkBox](https://developer.gnome.org/gtk3/stable/GtkBox.html) is used. The trick is getting the widget out of GtkSink and add it to the GtkBox. The following line retrieves the GtkSink element from the pipeline:
```C++
displaysink = gst_bin_get_by_name(GST_BIN(GtkMessageData.pipeline), "display");
// From the sink get the widget.
g_object_get(G_OBJECT (displaysink), "widget", &videowidget, NULL);
g_object_unref(displaysink);
```
The widget of the GtkSink is queried by a g_object_get call on the property "widget". The pointer in "videowidget" points to that widget now.

Then the GtkBox is querried from the GtkBuilder object:
```C++
videobox = GTK_WIDGET(gtk_builder_get_object(builder,"VideoBox"));
```
The GtkBox was named "Videobox" in Glade. Last step is adding the videowidget to the GtkBox:
```C++
gtk_container_add((GtkContainer*)videobox,videowidget);
g_object_unref(videowidget);
```
The adapted code was copied from the [Gstreamer Documentation](https://gstreamer.freedesktop.org/documentation/tutorials/basic/toolkit-integration.html?gi-language=c)


All other parts, like creating the cv::Mat in the pipeline's [appsink](https://gstreamer.freedesktop.org/documentation/app/appsink.html?gi-language=c) image callback are standard, already handled in the other samples of this repository.

I do not recommend to access the Gtk Widgets from within the appsink's callback, because Gtk seems not to be threadsafe and therefore segmentation faults can happen.

## Get and process an Image 
The sample uses an event, code from [here](https://github.com/moya-lang/Event9). The SaveIamge function "arms" the callback, so the next image will be copied into a cv::Mat. The function waits in the Event.wait() for a time period, so a timeout can be handled too.

```C++
void SaveImage(GtkMessageCustomData* MsgData)
{
	std::chrono::seconds sec(3); // 3 seconds timeout
	using shakes = std::chrono::duration<int, std::ratio<1, 100000000>>;

	// Arm the callback for saving an image into the cv::mat
	MsgData->data.pImageEvent->reset();
	MsgData->data.bImageReceived = false;
	MsgData->data.snapImage = true;

	MsgData->data.pImageEvent->wait(shakes(sec));

	if( MsgData->data.bImageReceived )
	{
		printf("Image reveived\n");
#ifdef HAVEOPENCV 		
		char ImageFileName[256];
        sprintf(ImageFileName,"image%05d.jpg", MsgData->data.ImageCount);
        cv::imwrite(ImageFileName,MsgData->data.OpenCVImage);
#endif
	}
	else
	{
		printf("Timeout\n");
	}
}
```
The event is set in the appsink's callback, after the cv::Mat has been created.

Sample Written by Stefan Geißler, 02/2020
