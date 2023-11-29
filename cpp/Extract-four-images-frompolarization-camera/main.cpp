#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "opencv2/opencv.hpp"


#include <glib.h>
#include <arv.h>
#include "arvhelper.h"


using namespace std;

#define ARAVIS_STREAM_BUFFER_NB 10
#define TRIGGERINTEVALL 1000

ArvDevice *Device;
ArvCamera *Camera;
ArvStream *CameraStream;
void (*old_sigint_handler)(int);

typedef struct
{
	GMainLoop *main_loop;
    int Packettimeouts;
	bool ImageReceived;
	bool error;
	cv::Mat displayImage;
	cv::Mat grayImage[4];
	cv::Mat colorImage[4];
	bool mat_created;
	char displayWindowName[255];

} _MyData;

_MyData MyData;

void CreatePolarImages(int width, int height, _MyData *pData)
{
   if (!pData->mat_created)
   {
		int h = height / 2;
		int w = width / 2;
		for (int i = 0; i < 4; i++)
		{
			pData->grayImage[i].create( h, w, CV_8UC1);
			pData->colorImage[i].create(h , w, CV_8UC4);
			char WindowName[256];
			sprintf(WindowName, "Polarization %d", i + 1);
			cv::namedWindow(WindowName);
		}
		pData->mat_created = true;
	}
}

void ShowPolarImages(_MyData *pData)
{
   for (int i = 0; i < 4; i++)
   {
      char WindowName[256];
      sprintf(WindowName, "Polarization %d", i + 1);
      //cv::imshow(WindowName, _PolarImagesGray[i]);
      cv::imshow(WindowName, pData->colorImage[i]);
   }
   cv::waitKey(1);
}



void StreamCallback(void* , ArvStreamCallbackType type, ArvBuffer* /*buffer*/)
{
	if (type == ARV_STREAM_CALLBACK_TYPE_INIT)
	{
		if (!arv_make_thread_realtime(10))
		{
			if (!arv_make_thread_high_priority(-10))
			{
				printf("Unable to make aravis capture thread real time or high priority");
			}
		}
	}
};


/////////////////////////////////////////////////////////////////////////
//
void AcquisitionCallback(ArvStream *pStream, void *pVoidData)
{
	_MyData *pMyData = (_MyData *)pVoidData;

	ArvBuffer *pBuffer = arv_stream_try_pop_buffer(pStream);

	if (pBuffer != NULL)
	{
		ArvBufferStatus Status = arv_buffer_get_status(pBuffer);
		if (Status == ARV_BUFFER_STATUS_SUCCESS)
		{
			pMyData->ImageReceived = true;

			int height = arv_buffer_get_image_height( pBuffer);
			int width = arv_buffer_get_image_width( pBuffer);

			
			pMyData->displayImage.create( height, width,
										   ArvPixelFormat2OpenCVType( pBuffer )
										   );

			size_t buffersize;										   
			const void* bytes = arv_buffer_get_data(pBuffer, &buffersize);
			memcpy( pMyData->displayImage.data, bytes, buffersize );

			if( strcmp(pMyData->displayWindowName,"") != 0)
			{
				cv::imshow(pMyData->displayWindowName, pMyData->displayImage);
				cv::waitKey(1);
			}
			CreatePolarImages(width,height, pMyData);

			unsigned char* sourcePixel = (unsigned char*)bytes;
			unsigned char* destPixel[4];

			for (int i = 0; i < 4; i++)
			{
				destPixel[i] = pMyData->grayImage[i].data;
			}

			for (int y = 0; y < height; y += 2)
			{
				for (int x = 0; x < width; x += 2)
				{
					*destPixel[1]++ = *sourcePixel++;
					*destPixel[0]++ = *sourcePixel++;
				}
				for (int x = 0; x < width; x += 2)
				{
					*destPixel[2]++ = *sourcePixel++;
					*destPixel[3]++ = *sourcePixel++;
				}
			}

			for (int i = 0; i < 4; i++)
			{
				cv::cvtColor(pMyData->grayImage[i], pMyData->colorImage[i], cv::COLOR_BayerRG2BGR);
			}

			ShowPolarImages(pMyData);
			

			arv_stream_push_buffer(pStream, pBuffer);
		}
		else
		{
            if( Status == ARV_BUFFER_STATUS_TIMEOUT)
            {
                pMyData->Packettimeouts++;
                //printf("Paket timeouts %d\n", pMyData->Packettimeouts);
            }
            else
			{
                printf("%s\n",getErrormessage(Status).c_str());
            }
            arv_stream_push_buffer(pStream, pBuffer);
			pMyData->error = true;
		}
	}
	else
	{
		printf("Buffer is null\n");
	}

}

/////////////////////////////////////////////////////////////////////////
//
void ControlLostCallback(ArvCamera *pDevice, void *pVoidData)
{
	_MyData *pMyData = (_MyData *)pVoidData;
	printf( "ControlLostCallback\n");
	pMyData->error = true;
}

/////////////////////////////////////////////////////////////////////////
//
bool StartAcquisition()
{
	if (Camera != NULL)
	{
		CameraStream = arv_camera_create_stream(Camera, StreamCallback, NULL);
        
		if (CameraStream != NULL)
		{
			gint payload  = 0;
			payload = arv_camera_get_payload(Camera);
			/* Push ARAVIS_STREAM_BUFFER_NB buffers in the stream input buffer queue */
			for (int nImage = 0; nImage < 10; nImage++)
				arv_stream_push_buffer(CameraStream, arv_buffer_new(payload, NULL));

			if( arv_camera_is_gv_device (Camera))
			{
				g_object_set (CameraStream,
						"packet-timeout", 40000,
						"frame-retention", 200000,
						"packet-resend", ARV_GV_STREAM_PACKET_RESEND_ALWAYS,
						NULL);
			}
			
			/* Start the video stream */
			arv_camera_start_acquisition(Camera);

			/* Connect the new-buffer signal */
			g_signal_connect(CameraStream, "new-buffer", G_CALLBACK(AcquisitionCallback), (void *)&MyData);
			/* And enable emission of this signal (it's disabled by default for performance reason) */
			arv_stream_set_emit_signals(CameraStream, TRUE);

			/* Connect the control-lost signal */
			g_signal_connect(Device, "control-lost", G_CALLBACK(ControlLostCallback), (void *)&MyData);
		}
		else
		{
			printf("CameraStream is NULL\n");
			return false;
		}
	}
	else
		return false;

	return true;
}

void StopAcquisition()
{
	arv_camera_stop_acquisition(Camera);
	arv_stream_set_emit_signals (CameraStream, FALSE);
	g_object_unref(CameraStream);
	g_object_unref(Camera);
}


int main(int argc, char **argv)
{
	int Selection = 0;

	std::cout << "Aravis OpenCV" << std::endl;

	MyData.error = false;
	MyData.mat_created = false;

	strcpy(MyData.displayWindowName,"Liveview");
	cv::namedWindow(MyData.displayWindowName);

	Selection = select_camera();

	Camera = arv_camera_new(arv_get_device_id(Selection));
	if (Camera == NULL)
		return 0;

	printf("Camera : %s\n", arv_camera_get_model_name(Camera));
	Device = arv_camera_get_device(Camera);

	gint64 pixelformat = select_pixelformat(Camera);
	if( pixelformat > 0)
		arv_camera_set_pixel_format(Camera,pixelformat);

	arv_camera_set_region(Camera, 0, 0, 2448, 2048);
	arv_camera_set_frame_rate(Camera, 30.0);


	if(	StartAcquisition())
	{
		char input = '\0';
		while(input != 'q')
		{
			printf("\nEnter q for quit :");
			scanf("%c", &input);
		}

		StopAcquisition();
	}
	cv::destroyAllWindows();
}
