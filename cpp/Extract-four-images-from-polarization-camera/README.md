# Extract four images from Polarization Camera
This sample shows how to extract the four images of the different polarization directions from the source image.

* Programming language : C++
* Polarization Camera needed
* Aravis 0.8 needed

In order to keep the sample simple, it handles MONO8 polarization format only, even if other formats can be seleted in the program. Changing to MONO16 should be easy.  

Handling the color formats must take care of the Bayer Pattern organisation of the sensor. That is currently not part of the sample.
