/*
This command line example demonstrates, how to use the ic-barcode library.
An image file with barcodes in (of course) is passed to the library and
the barcodes are decoded.

The sample uses stb_image - v2.22 - public domain image loader - http://nothings.org/stb / https://github.com/nothings/stb
                                
Building:
make
*/

#include <cstdio>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <ic_barcode.h>

int main(int argc, char*argv[])
{
	if (argc != 2)
	{
		printf("No image file to load specified.\n");
		printf("Usage:\n");
		printf("\t./ic-barcode-cli <imagefile>\n");
		printf("Example:\n");
		printf("\t./ic-barcode-cli barcode.bmp\n");

		return 1;
	}

	// Prepare loading of the image.
	std::string file_path = argv[1];

    int width = 0;
    int height = 0;
    int bpp = 0;

	// Read the image and get a pointer to the image dta in *rgb
    unsigned char* rgb = stbi_load(file_path.c_str(), &width, &height, &bpp, 4);
    
	if (rgb == nullptr)
	{
		printf("Unable to load image.\n");
		return 1;
	}
	
	// Memory for the gray scale image buffer is allocated:
    std::vector<unsigned char> buffer( width * height );

	// Create a gray scale image from the read, that will be passed to the barcode scanner
    ICBarcode_Transform_BGRA32_to_Y800(
		buffer.data(),
		rgb,
		width,
		height,
		width * 4,
		width);

	stbi_image_free(rgb);

	// Create the barcode scanner.
	printf("Creating barcode scanner\n");
	ICBarcode_Scanner *pScanner = ICBarcode_CreateScanner();
	
	// Configure the scanner to check checksums
	ICBarcode_SetParam(pScanner, IC_BARCODEPARAMS_CHECK_CHAR, 1);

        // Configure the scanner to try harder, which might be helpful
	// on hard to read images.
	ICBarcode_SetParam(pScanner, IC_BARCODEPARAMS_TRY_HARDER, 1);


	// Add a few barcode formats to be detected.
	// The less formats specified, the faster and more reliable
	// are the results.
	int formats = 0;
	formats |= ICBarcode_Format::IC_BARCODEFORMAT_CODE_128;
	formats |= ICBarcode_Format::IC_BARCODEFORMAT_CODE_39;
	formats |= ICBarcode_Format::IC_BARCODEFORMAT_CODE_93;
	formats |= ICBarcode_Format::IC_BARCODEFORMAT_EAN_13;
	formats |= ICBarcode_Format::IC_BARCODEFORMAT_EAN_8;
	formats |= ICBarcode_Format::IC_BARCODEFORMAT_UPC_A;
	formats |= ICBarcode_Format::IC_BARCODEFORMAT_INTERLEAVED_2_OF_5;
	formats |= ICBarcode_Format::IC_BARCODEFORMAT_QR_CODE;

	ICBarcode_SetBarcodeFormats(pScanner, formats);

	// Create the list of found bardcodes. We will find up
	// to 100 barcodes in one image, if there are so much in.
	std::vector<ICBarcode_Result> foundBarcodes( 100 );
	// Now the scanning is done. Count contains the amount of found barcodes.
	printf("Start scanning\n");
	int count = ICBarcode_FindBarcodes(pScanner,
									  buffer.data(),
									  width, height, width,
									  foundBarcodes.data(), foundBarcodes.size());

	/* If count is lower the 0, then an error occured. Usually it is the 
		license error, which means the tiscamera modules are not built and 
		installed or no The Imaging Source camera is connected.
		The cameras work like a dongle.
	*/
	if( count < 0 )
	{
		printf("No license.\n");
		printf("- Is a The Imaging Source camera connected?\n");
		printf("- Is a tiscamera repository built and installed?\n");
		printf("  Please refer to https://github.com/TheImagingSource/tiscamera\n");

		return 1;
	}

	if (count > 0)
	{
		// Some barcodes are found, lets print them out.
		printf("Found %d barcodes:\n", count);
		for( int i = 0; i < count; i++)
		{
			printf("%s\n", foundBarcodes[i].Text);
		}
	}
	else
	{
		printf("No barcode found in %s\n", file_path.c_str());
	}

	// Clean up the barcode scanner object.
	ICBarcode_DestroyScanner(pScanner);
	return 0;
}
