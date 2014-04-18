#ifndef G_BUFFER_H
#define G_BUFFER_H
#include <gl/framebuffer.h>

/*
Geometry-buffer for use in deferred shading.
It stores three color buffer textures in the following fragment outputs:
	0. RGB16f  Position buffer
	1. RGB16f  Normal buffer
	2. RGBA16f Diffuse buffer
*/
class GBuffer
{
public:
	void create(int width, int height);
	void dispose();

	// Bind the framebuffer and enable offscreen rendering
	void begin();

	// Bind the default framebuffer
	void end();

	// Bind the color buffers to their respective texture units
	void bindTextures();
private:
	Framebuffer  framebuffer;
	Renderbuffer depthbuffer;
	Texture2D    texPosition;
	Texture2D    texNormal;
	Texture2D    texDiffuse;
};

#endif