#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTUER_H
#include <gl/framebuffer.h>
#include <gl/texture.h>

class RenderTexture
{
public:
	RenderTexture(int width, int height);
	~RenderTexture();

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
	Texture colorBuffer;
	Renderbuffer depthBuffer;
	Framebuffer frameBuffer;
};

#endif