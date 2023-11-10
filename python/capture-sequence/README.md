# Capture an Image Sequence
This sample demonstrates how to capture images from the camera into
an array of numpy as fast as possible. After the sequence has been 
captured, the images are saved to hard disc usig OpenCV.
The image capture is done very fast, so high camera frame rates
can be used.

It is an extremely simple sample without extensive error handling,
e.g. check for enough memory.

## Documentation
The number of frames to capture is passed to the TIS class
by setting the member Tis.framestocapture:

```Python
Tis.framestocapture = 100
```

Afterwards the program simply waits, until Tis.framestocapture
is 0.
```Python
# Wait for all frames being captured
while Tis.framestocapture > 0:
    time.sleep(0.1)
```

When done, the images are saved:

```Python
print("Saving images.")
for imgnr in range(Tis.get_captured_image_count()):
    image = Tis.get_image(imgnr)
    if image is not None:
        cv2.imwrite("test{0}.png".format(imgnr),
                    image)
```

The image data sequence is generated in the TIS.py TIS class. It is done  in the callback of the `appsink`:

```Python
    def __on_new_buffer(self, appsink):
        sample = appsink.get_property('last-sample')
        if self.framestocapture > 0:
            self.framestocapture -= 1
            if sample is not None:
                buf = sample.get_buffer()
                self.image_caps = sample.get_caps()
                # Append the image data to the data array
                self.image_data.append(buf.extract_dup(0, buf.get_size()))
                
        return Gst.FlowReturn.OK
```

The conversion of the image data to a numpy array is done in the
TIS.get_image(imagenr) function: 

```Python
    def get_image(self, image_nr : int):
        '''
        Return a numpy array which contains the image data of the 
        image at array position image_nr.
        If image_nr is out of bounds, then None is returned.
        '''
        if image_nr < 0 and image_nr > len(self.image_data):
            return None
        if self.image_caps is None:
            return None
        
        return self.__convert_to_numpy(self.image_data[image_nr],
                                       self.image_caps)
```

This approach is used to let the new buffer callback of the `appsink` last as short as possible.
