#ifndef VIDEO_MODE_H
#define VIDEO_MODE_H

struct VideoMode
{
	VideoMode(int width = 640, 
			  int height = 480, 
			  int depthBits = 24, 
			  int stencilBits = 0, 
			  int fsaaSamples = 0,
			  int glMajor = 3,
			  int glMinor = 1,
			  bool fullscreen = false,
			  bool vsync = true)
			  : 
		Width(width),
		Height(height),
		DepthBits(depthBits),
		StencilBits(stencilBits),
		FsaaSamples(fsaaSamples),
		GlMajor(glMajor),
		GlMinor(glMinor),
		Fullscreen(fullscreen),
		Vsync(vsync) { }
			  

	int Width, 
		Height,
		DepthBits,
		StencilBits,
		FsaaSamples,
		GlMajor,
		GlMinor;
	bool Fullscreen;
	bool Vsync;
};

#endif