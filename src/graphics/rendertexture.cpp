#include <graphics/rendertexture.h>

RenderTexture::RenderTexture() { }

void RenderTexture::create(int width, int height)
{
	colorBuffer.create(0, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	colorBuffer.bind();
	colorBuffer.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	colorBuffer.unbind();

	depthBuffer.create();
	depthBuffer.bind();
	depthBuffer.storage(GL_DEPTH_COMPONENT, width, height);
	depthBuffer.unbind();

	frameBuffer.create();
	frameBuffer.bind();
	frameBuffer.attachTexture2D(GL_COLOR_ATTACHMENT0, colorBuffer, 0);
	frameBuffer.attachRenderbuffer(GL_DEPTH_ATTACHMENT, depthBuffer);
	GLenum status = frameBuffer.checkStatus();
	if(status != GL_FRAMEBUFFER_COMPLETE)
		throw std::exception("Framebuffer not complete");
	frameBuffer.unbind();
}

void RenderTexture::create(GLint level,
		GLint internalFormat,
		GLsizei width,
		GLsizei height,
		GLenum format,
		GLenum type,
		const GLvoid *data)
{
	colorBuffer.create(level, internalFormat, width, height, format, type, data);

	frameBuffer.create();
	frameBuffer.bind();
	frameBuffer.attachTexture2D(GL_COLOR_ATTACHMENT0, colorBuffer, 0);
	GLenum status = frameBuffer.checkStatus();
	if(status != GL_FRAMEBUFFER_COMPLETE)
		throw std::exception("Framebuffer not complete");
	frameBuffer.unbind();
}

void RenderTexture::create(Texture2D color_buffer)
{
	colorBuffer = color_buffer;

	depthBuffer.create();
	depthBuffer.bind();
	depthBuffer.storage(
		GL_DEPTH_COMPONENT, 
		color_buffer.getWidth(), 
		color_buffer.getHeight());
	depthBuffer.unbind();

	frameBuffer.create();
	frameBuffer.bind();
	frameBuffer.attachTexture2D(GL_COLOR_ATTACHMENT0, colorBuffer, 0);
	frameBuffer.attachRenderbuffer(GL_DEPTH_ATTACHMENT, depthBuffer);
	GLenum status = frameBuffer.checkStatus();
	if(status != GL_FRAMEBUFFER_COMPLETE)
		throw std::exception("Framebuffer not complete");
	frameBuffer.unbind();
}

void RenderTexture::dispose()
{
	colorBuffer.dispose();
	frameBuffer.dispose();
	depthBuffer.dispose();
}

void RenderTexture::begin()
{
	//glViewport(0, 0, );
	frameBuffer.bind();
	//depthBuffer.bind();
}

void RenderTexture::end()
{
	frameBuffer.unbind();
	depthBuffer.unbind();
}

void RenderTexture::bindTexture(GLenum unit)
{
	colorBuffer.bind(unit);
}

void RenderTexture::unbindTexture()
{
	colorBuffer.unbind();
}