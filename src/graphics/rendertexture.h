#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTURE_H
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
	void create(GLint level,
		GLint internalFormat,
		GLsizei width,
		GLsizei height,
		GLenum format,
		GLenum type,
		const GLvoid *data);
	void create(Texture2D color_buffer);
	void dispose();

	// Enables the framebuffer as the active render target
	// Obs! Remember to set glViewport to match the dimensions
	// of the color buffer output (and to reset it afterwards)
	void begin();

	// Enables default active render target (the window)
	void end();

	// Binds the color buffer
	void bindTexture(GLenum unit = GL_TEXTURE0);

	// Binds the default color buffer (0)
	void unbindTexture();

	Texture2D    &getColorBuffer() { return colorBuffer; }
	Renderbuffer &getDepthBuffer() { return depthBuffer; }
	Framebuffer  &getFramebuffer() { return frameBuffer; }
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