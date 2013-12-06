#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image.h"
#include "videocapture.h"

bool savePNG(const char *filename, 
			 const unsigned char *pixels, // RGB, unsigned byte
			 unsigned int width, 
			 unsigned int height)
{
	bool status = stbi_write_png(filename, width, height, 3, pixels, width * 3) != 0;
	return status;
}