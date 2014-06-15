#include <video/stb_image_write.h>
#include <video/videocapture.h>
#include <iostream>
#include <sstream>
#include <gl/opengl.h>
#include <app/log.h>
#include <iomanip>

// Memory to hold videodata
unsigned char *vid_mem_start = nullptr;
unsigned char *vid_mem_end   = nullptr;

// Video parameters
int vid_width;
int vid_height;
int vid_seconds;
int vid_fps;

// Timers
unsigned int current_frame;
unsigned int frame_count;
double video_elapsed_time; // Time video has been recording
double video_dump_time; // Timer until next frame grab
double frametime; // Seconds between each frame grab

void VideoCapture::reserve(int width, int height, int fps, int seconds)
{
	vid_width			= width;
	vid_height			= height;
	vid_seconds			= seconds;
	vid_fps				= fps;

	video_elapsed_time	= 0.0;
	video_dump_time		= 0.0;
	frametime			= 1.0 / vid_fps;

	current_frame		= 0;
	frame_count			= vid_seconds * vid_fps;

	// Allocate memory buffer
	vid_mem_start = new unsigned char[vid_width * vid_height * 3 * frame_count];
	vid_mem_end   = vid_mem_start;

	int bytes = vid_width * vid_height * 3 * frame_count;
	std::cout << "Reserving " << bytes << " (" << (bytes / (1024 * 1024)) << " MB)" << std::endl;
	std::cout << width << "x" << height << " " << fps << " fps " << seconds << " seconds" << std::endl;
	std::cout << "OK? ";
	std::cin.get();
}

void VideoCapture::record(double dt)
{
	if (current_frame >= frame_count)
		return;

	video_elapsed_time += dt;
	video_dump_time += dt;
	if (video_dump_time > frametime)
	{
		// Copy framebuffer into 24-bit rgb image
		glReadPixels(0, 0, vid_width, vid_height, GL_RGB, GL_UNSIGNED_BYTE, vid_mem_end);

		// Move video pointer along by one frame
		vid_mem_end += vid_width * vid_height * 3;
		video_dump_time -= frametime;
		current_frame++;

		unsigned int bytesize = (unsigned int)(vid_mem_end - vid_mem_start);
		std::cout << "\rElapsed time: " << std::fixed << std::setprecision(2) << video_elapsed_time << "s (" << bytesize << ")";
	}

	if (current_frame >= frame_count)
		std::cout << "... done!" << std::endl;
}

void VideoCapture::dump(const char *path)
{
	unsigned char *vid_mem_ptr = vid_mem_start;
	int i = 0;
	std::cout << "\n";
	while (vid_mem_ptr < vid_mem_end)
	{
		std::stringstream ss;
		ss << path << "/capture" << i << ".bmp";
		int status = stbi_write_bmp(ss.str().c_str(), vid_width, vid_height, 3, vid_mem_ptr);
		if (!status)
		{
			APP_LOG << "Failed to write frame " << i << '\n';
			break;
		}

		// Advance by the size of a single frame
		vid_mem_ptr += vid_width * vid_height * 3;
		i++;

		// Print progress
		std::cout << "\rFrame " << i << "/" << frame_count << "%)";

	}

	std::cout << "... done!\n";

	// Deallocate video memory
	delete[] vid_mem_start;
}