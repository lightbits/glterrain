#include <graphics/rendertexture.h>

RenderTexture::RenderTexture() { }

void RenderTexture::create(int width, int height)
{
	colorBuffer.create2d(0, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	colorBuffer.bind();
	colorBuffer.setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
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

void RenderTexture::dispose()
{
	colorBuffer.dispose();
	frameBuffer.dispose();
	depthBuffer.dispose();
}

void RenderTexture::begin()
{
	//glViewport();
	frameBuffer.bind();
	//depthBuffer.bind();
}

void RenderTexture::end()
{
	frameBuffer.unbind();
	depthBuffer.unbind();
}

void RenderTexture::bindTexture()
{
	colorBuffer.bind();
}

void RenderTexture::unbindTexture()
{
	colorBuffer.unbind();
}