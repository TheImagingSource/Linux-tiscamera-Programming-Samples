# ic-barcode Simple Demo
This demo program shows how to use the ic-barcode library. An image containing barcodes, for example the "barcode.bmp" is passed to the program and the library lists the found barcodes.

Note: A The Imaging Source camera must be connected to the computer, because the library works only, if a camera is connected, even if it is not used in this sammple. Please refer to [tiscamera repository](https://github.com/TheImagingSource/Linux-tiscamera-Programming-Samples)

The ic-barcode dev package can be downloaded from [IC-Barcode](https://www.theimagingsource.com/support/downloads-for-linux/). The dev package installes the header and .so files into the standard Linux directories, so no additonal link and include directories must be specified.

Programming language : C++

gcc 6 or higher is required. If you use Ubuntu 16.04, you check with gcc --version the gcc version. If it is below 6, please follow the instructions at  https://askubuntu.com/questions/781972/how-can-i-update-gcc-5-3-to-6-1".

## Building
In order to build the sample, open a terminal, enter the sample's directory. Then enter
```
make
./ic-barcode-cli barcode.bmp
```
## Source Code main.cpp
The stb_image - v2.22 library is used for loading the passed image file. It returns a colored image, which will be converted to gray scale, because ic-barcode handles gray scale images only:

```C++
    unsigned char* rgb = stbi_load(file_path.c_str(), &width, &height, &bpp, 4);
    

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
```
The image buffer is in the `buffer` variable now. In the next step, the bacode scanner is created:

```C++
ICBarcode_Scanner *pScanner = ICBarcode_CreateScanner();
```
`pScanner` is a handle to the barcode scanner, which is used for configuring and scanning. It needs to be created only once and can be reused.
Now the barcode scanner will be configured to check barcode checksums and "to try harder".
```C++
	// Configure the scanner to check checksums
	ICBarcode_SetParam(pScanner, IC_BARCODEPARAMS_CHECK_CHAR, 1);

	// Configure the scanner to try harder, which might be helpful
	// on hard to read images.
	ICBarcode_SetParam(pScanner, IC_BARCODEPARAMS_TRY_HARDER, 1);
```
The type of barcodes to be detected is be configured now:
```C++
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
```
The less formats are specified, the more reliable is the result and the faster is the scanning.
At least a memory buffer is needed, in which the found barcodes are sstored.
```C++
	std::vector<ICBarcode_Result> foundBarcodes( 100 );
```
The call to scan the image is:
```C++
	int count = ICBarcode_FindBarcodes(pScanner,
					buffer.data(),
					width, height, width,
					foundBarcodes.data(), foundBarcodes.size());
```
If `count` is greater than 0, then barcodes were found and are printed:
```C++
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
```
If `count` is lower than 0, then an error occurred. Please check, whether a The Imaging Source camera is connected to the computer or network and the tiscamera repository from  https://github.com/TheImagingSource/tiscamera has been built and installed.

At program's end, the barcode handle should be cleaned up:
```C++
	// Clean up the barcode scanner object.
	ICBarcode_DestroyScanner(pScanner);
```
