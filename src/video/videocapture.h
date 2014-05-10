#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

namespace VideoCapture
{
	// Reserve the video memory.
	// width, height: Dimensions of each frame
	// fps: Frames per seconds
	// seconds: Seconds of footage
	void reserve(int width, int height, int fps, int seconds);

	// Captures a single frame every (1/fps) second
	// dt: Time since last call to this function
	void record(double dt);

	// Saves the video as a series of .bmp images, and
	// deallocates the memory. The image series will be of the form:
	// capture0.bmp
	// capture1.bmp
	// ...
	// in the folder indicated by 'path'.
	// Note that the folder must exist beforehand.
	void dump(const char *path);
}

#endif