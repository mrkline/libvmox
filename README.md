# libvmox - a _v_ideo _mo_tion e_x_tractor

libvmox is a simple video motion extractor library. Originally inspired by
[this CodeProject post](http://www.codeproject.com/Articles/10248/Motion-Detection-Algorithms),
it works by filtering out moving objects in the video stream then comparing this "static" image
to said stream.

## Detection Algorithm

More specifically,

- Each pixel is given an integer which counts the amount of frames it has been since it changed significantly

- Pixels which haven't changed for a longer period of time than their corresponding pixel in the "static" image are
  copied to the static image.

- Motion detection then becomes as simple as comparing the static image to the current video frame. 

This process can be tweaked by modifying any of three parameters:

1. Sensitivity - the amount (in one byte per channel color space) a channel of a pixel has to differ from its
                 "static" counterpart in order to be considered moving. The values 1 through 127 are accepted.

2. Erosion - To smooth out noise, setting this value to 1 through 8 removes pixels from the motion mask. Setting it
             to 0 disables this smoothing process and reduces the number of passes needed to compute the motion mask
             for each frame.

3. Settle Time - Adjusting this parameter sets the limit on the maximum age (in seconds) of pixels
                 in the "static" image. A larger value makes the algorithm less suseptible to false positives
                 caused by slow moving (or starting and stopping) objects, but can cause false positives
                 wherever an object stops for more than the settle time
                 (this false positive will remain for the length of the settle time until the algorithm "realizes"
                 that the differing pixels are not moving). The values 1 through 60 seconds are accepted, and a small
                 value (1 to 5 seconds) is recommended.

## Additional tools

libvmox also comes with some code for reading video files, via FFmpeg, into its frame format (see `FFmpegVideoReader`).

## License

See `license.md`
