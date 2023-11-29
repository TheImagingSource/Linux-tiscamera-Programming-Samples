#ifndef __ARVHELPER
#define __ARVHELPER

#define TIS_PolarizedMono8 0x8108000a
#define TIS_PolarizedMono12p 0x810c000b
#define TIS_PolarizedMono12Packed 0x810c0010
#define TIS_PolarizedMono16 0x8110000c

#define TIS_PolarizedBayerBG8 0x8108000d
#define TIS_PolarizedBayerBG12p 0x810c000e
#define TIS_PolarizedBayerBG16 0x8110000f

#define TIS_YUV422 0x0210003b

int enumerate_gige_cameras();
std::string getErrormessage(ArvBufferStatus Status);
int ArvPixelFormat2OpenCVType( ArvBuffer *pBuffer );
int select_camera();
gint64 select_pixelformat(ArvCamera *pCamera);
#endif