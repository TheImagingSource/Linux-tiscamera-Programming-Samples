#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <arv.h>
#include <opencv2/opencv.hpp>
#include "arvhelper.h"

/////////////////////////////////////////////////////////////////////////
// Enumerate GigE cameras
int enumerate_gige_cameras()
{
	int camera_cnt = 0;

	unsigned int n_devices;
	unsigned int i;
	printf("Enumerating GigE cameras\n");

	arv_update_device_list();
	n_devices = arv_get_n_devices();
	for (i = 0; i < n_devices; i++)
	{
		printf("\t%d :  %s\n", i+1, arv_get_device_id(i));
		camera_cnt++;
	}

	return camera_cnt;
}

std::string getErrormessage(ArvBufferStatus Status)
{
	std::string msg;

	switch (Status)
	{
	case ARV_BUFFER_STATUS_SUCCESS:
		msg = "the buffer is cleared";
		break;
	case ARV_BUFFER_STATUS_TIMEOUT:
		msg = "Timeout has been reached before all packets were received";
		break;
	case ARV_BUFFER_STATUS_MISSING_PACKETS:
		msg = "Stream has missing packets";
		break;
	case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
		msg = "Stream has packet with wrong id";
		break;
	case ARV_BUFFER_STATUS_SIZE_MISMATCH:
		msg = "The received image did not fit in the buffer data space";
		break;
	case ARV_BUFFER_STATUS_FILLING:
		msg = "The image is currently being filled";
		break;
	case ARV_BUFFER_STATUS_ABORTED:
		msg = "The filling was aborted before completion";
		break;
	case ARV_BUFFER_STATUS_CLEARED:
		msg = "Buffer cleared";
		break;
    case ARV_BUFFER_STATUS_UNKNOWN:
	default: 
		msg = "This should not happen";
		break;
	}

	return msg;
}

int enterSelection()
{
	int Selection = -1;
	printf("Select : ");
	scanf("%d", &Selection);
	if (Selection < 0)
		return 0;
	return Selection;
}

int select_camera()
{
	enumerate_gige_cameras();
	return enterSelection()-1;
}

int ArvPixelFormat2OpenCVType( ArvBuffer *pBuffer )
{
	ArvPixelFormat apf = arv_buffer_get_image_pixel_format(pBuffer);
	int retval = CV_8UC4;

	switch( apf)
	{
		case ARV_PIXEL_FORMAT_MONO_8:
		case TIS_PolarizedMono8:
		case TIS_PolarizedBayerBG8:
			retval = CV_8UC1;
			break;
		case ARV_PIXEL_FORMAT_MONO_16:
		case TIS_PolarizedMono16:
		case TIS_PolarizedBayerBG16:
			retval = CV_16UC1;
			break;
		case ARV_PIXEL_FORMAT_RGB_8_PACKED:
		case ARV_PIXEL_FORMAT_BGR_8_PACKED:
			retval = CV_8UC3;
		case TIS_YUV422:
			retval = CV_16UC1;
			break;

 
	}
	return retval;
}

gint64 select_pixelformat(ArvCamera *pCamera)
{
	printf("Available pixel formats:\n");

	guint formatcount = 0;
	const char **formats = arv_camera_get_available_pixel_formats_as_display_names( pCamera, &formatcount);
	gint64 *formatsint = arv_camera_get_available_pixel_formats( pCamera, &formatcount);
	for( uint i = 0; i < formatcount; i++)
	{
		printf("\t%d: %s   0x%08x\n", i+1, formats[i], formatsint[i]);
	}
	int selected = enterSelection() - 1;
	if( selected >= 0)
	{
		return (formatsint[selected]);
	}
	return 0;
}