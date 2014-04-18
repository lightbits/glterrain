#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTUER_H
#include <gl/framebuffer.h>
#include <gl/texture.h>

/*
A specialized framebuffer intended as a target for offscreen rendering.
The renderered data can be obtained as a Texture by calling 'bindTexture'.
*/
class RenderTexture
{
public:
	RenderTexture();
	void create(int width, int height);
	void dispose();

	// Enables the framebuffer as the active render target
	void begin();

	// Enables default active render target (the window)
	void end();

	// Binds the color buffer
	void bindTexture();

	// Binds the default color buffer (0)
	void unbindTexture();
private:
	// Disable copying
	RenderTexture(const RenderTexture &copy) { }
	RenderTexture &operator=(const RenderTexture &copy) { }
private:
	Texture2D colorBuffer;
	Renderbuffer depthBuffer;
	Framebuffer frameBuffer;
};

#endif