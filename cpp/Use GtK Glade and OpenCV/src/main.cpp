#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gdk/gdkkeysyms.h>
#include <gst/app/gstappsink.h>

#include "Event.h"

#ifdef HAVEOPENCV 
	#include "opencv2/opencv.hpp" 
#endif	 
 

// This custom data is passed to the GStreamer appsink
typedef struct CustomGstData_
{
	GtkWidget *lblBrigthness;
	int ImageCount;
	Moya::Event *pImageEvent;
	bool bImageReceived;
	bool snapImage;

#ifdef HAVEOPENCV 
	cv::Mat OpenCVImage;
#endif	 
} CustomGstData_;


// This custom data is passed to the GTK Messages.
typedef struct GtkMessageCustomData
{
	GstElement* pipeline; // The Gstreamer pipeline.
	CustomGstData_ data;
} GtkMessageCustomData;
 


GstFlowReturn new_frame_cb(GstAppSink *appsink, gpointer data)
{
	CustomGstData_ *d = (CustomGstData_*)data;

	d->ImageCount++;
	if( d->lblBrigthness != NULL)
	{
		char szText[20];
		sprintf(szText,"%d",d->ImageCount );
	}

	if( d->snapImage )
	{
		d->snapImage = false;

#ifdef HAVEOPENCV 
		int width, height ;
		const GstStructure *str;

		// The following lines demonstrate, how to acces the image
		// data in the GstSample.
		GstSample *sample = gst_app_sink_pull_sample(appsink);

		GstBuffer *buffer = gst_sample_get_buffer(sample);

		GstMapInfo info;

		gst_buffer_map(buffer, &info, GST_MAP_READ);
		
		if (info.data != NULL) 
		{
			// info.data contains the image data as blob of unsigned char 
			GstCaps *caps = gst_sample_get_caps(sample);
			// Get a string containg the pixel format, width and height of the image        
			str = gst_caps_get_structure (caps, 0);    

			if( strcmp( gst_structure_get_string (str, "format"),"BGRx") == 0)  
			{
				// Now query the width and height of the image
				gst_structure_get_int (str, "width", &width);
				gst_structure_get_int (str, "height", &height);

				// Create a cv::Mat, copy image data into that and save the image.
				d->OpenCVImage.create(height,width,CV_8UC(4));
				memcpy( d->OpenCVImage.data, info.data, width*height*4);
			}
		}
		
		// Calling Unref is important!
		gst_buffer_unmap (buffer, &info);
		gst_sample_unref(sample);

#endif
		d->bImageReceived = true; 
		d->pImageEvent->set(); // Set the event, so the image will be processed
	}
    return GST_FLOW_OK;
}

gboolean on_timer(gpointer data)
{
	GtkMessageCustomData* d = (GtkMessageCustomData*)data;
	char szText[20];
	sprintf(szText,"%d",d->data.ImageCount );
	gtk_label_set_text( (GtkLabel*)d->data.lblBrigthness,szText);
    return true;
}


int main(int argc, char *argv[])
{
    GtkBuilder      *builder; 
    GtkWidget       *window;
    GtkWidget       *videowidget;
    GtkWidget    *videobox;
	GstElement *appsink;
  	GstElement *displaysink;     
	
	GtkMessageCustomData GtkMessageData;

    char pipe[] = "tcambin name=tcambin ! video/x-raw,format=BGRx, width=640, height=480, framerate=30/1 ! tee name=t t. ! queue ! videoconvert ! gtksink name=display t. ! queue ! appsink name=sink";

    gtk_init(&argc, &argv);
    gst_init( &argc,&argv);

    GtkMessageData.pipeline = gst_parse_launch(pipe, NULL);
	// Get the sinks
	displaysink = gst_bin_get_by_name(GST_BIN(GtkMessageData.pipeline), "display");
	appsink = gst_bin_get_by_name(GST_BIN(GtkMessageData.pipeline), "sink");

	// Create the list of appsink callbacks. 
	GstAppSinkCallbacks callbacks = {NULL, NULL, new_frame_cb};
	gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, &GtkMessageData.data, NULL);
	g_object_set(GST_APP_SINK(appsink), "max-buffers", 4, "drop", true, nullptr);
	
	GtkMessageData.data.ImageCount = 0;
	GtkMessageData.data.lblBrigthness = NULL;
	GtkMessageData.data.pImageEvent = new Moya::Event(false,false);
	GtkMessageData.data.bImageReceived = false;
	GtkMessageData.data.snapImage = false;

	
	// From the sink get the widget.
	g_object_get (G_OBJECT (displaysink), "widget", &videowidget, NULL);
	g_object_unref(displaysink);

    builder = gtk_builder_new_from_file("glade/glade-sample.glade");

    gtk_builder_connect_signals(builder,&GtkMessageData);

	window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

	GtkMessageData.data.lblBrigthness = GTK_WIDGET(gtk_builder_get_object(builder, "lblAverageBrightness"));

	// Get our video box GtkBox element to draw the video in.
	videobox = GTK_WIDGET(gtk_builder_get_object(builder,"VideoBox"));

	// Add the widget of the gtksink to the video box
	gtk_container_add((GtkContainer*)videobox,videowidget);
	g_object_unref(videowidget);

	gtk_widget_realize (videowidget);
	// Important: use "show_all" here!
	gtk_widget_show_all(window);         

	// Start the stream
    gst_element_set_state(GtkMessageData.pipeline, GST_STATE_PLAYING);
    // 4 seconds timeout for start
    if( gst_element_get_state(GtkMessageData.pipeline, NULL, NULL, 400000000) != GST_STATE_CHANGE_SUCCESS )
    {
        gst_element_set_state(GtkMessageData.pipeline, GST_STATE_NULL);
		printf("Failed to start camera pipeline\n");
        return 1;
    }

	g_timeout_add(50, on_timer, &GtkMessageData);

    gtk_main();
    return 0;
}
 
///////////////////////////////////////////////////////////////
// Stop the pipeline and end program.
void CleanUpandEndProgram(GtkMessageCustomData* m)
{
	// Stop pipeline
 	gst_element_set_state( m->pipeline, GST_STATE_NULL);
    gst_element_get_state( m->pipeline, NULL, NULL, 400000000); 
  	gst_object_unref(m->pipeline);
	gtk_main_quit();
}
 

///////////////////////////////////////////////////////////////
// Snap and save an image from the stream. The image is saved
// in the OpenCV mat in the appsink's callback
// The "event" in MsgData->data is used for waiting, until the image was
// copeied
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

/////////////////////////////////////////////////////////////////
// Event hanlders. Must be extern "C"

extern "C" 
{
///////////////////////////////////////////////////////////////////
// Called from menu
void filemenuend(GtkMenuItem *menuitem, gpointer data)
{
	CleanUpandEndProgram((GtkMessageCustomData*)data);
}

///////////////////////////////////////////////////////////////////
// Called from close window button
void mainwindow_destroy(GtkWidget *object, gpointer data)
{
	CleanUpandEndProgram((GtkMessageCustomData*)data);
}

///////////////////////////////////////////////////////////////////
// Called from save image menu
void on_menuSaveImage_activate(GtkWidget *object, gpointer data)
{
	SaveImage( (GtkMessageCustomData*)data );
}

} //extern "C"