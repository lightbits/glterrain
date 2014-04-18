#include <gl/gbuffer.h>

void GBuffer::create(int width, int height)
{
	// Create texture that will hold fragment position output
	texPosition.create(0, GL_RGB16F, width, height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	texPosition.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	// Create texture that will hold fragment normal output
	texNormal.create(0, GL_RGB16F, width, height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	texNormal.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	// Create texture that will hold fragment diffuse output
	texDiffuse.create(0, GL_RGBA16F, width, height, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	texDiffuse.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	// The renderbuffer is actually just used to depth testing
	depthbuffer.create();
	depthbuffer.bind();
	depthbuffer.storage(GL_DEPTH_COMPONENT, width, height);
	depthbuffer.unbind();

	// Create framebuffer that will be used to render to these textures
	framebuffer.create();
	framebuffer.bind();
	framebuffer.attachTexture2D(GL_COLOR_ATTACHMENT0, texPosition, 0); // position is colornumber 0
	framebuffer.attachTexture2D(GL_COLOR_ATTACHMENT1, texNormal, 0); // normal is colornumber 1
	framebuffer.attachTexture2D(GL_COLOR_ATTACHMENT2, texDiffuse, 0); // diffuse is colornumber 2
	framebuffer.attachRenderbuffer(GL_DEPTH_ATTACHMENT, depthbuffer);

	// Specify output targets
	GLenum drawBuffers[] = { 
		GL_COLOR_ATTACHMENT0, 
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2 
	};
	glDrawBuffers(3, drawBuffers);

	GLenum status = framebuffer.checkStatus();
	if (status != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer not complete");

	framebuffer.unbind();
}

void GBuffer::dispose()
{
	framebuffer.dispose();
	depthbuffer.dispose();
	texPosition.dispose();
	texNormal.dispose();
	texDiffuse.dispose();
}

void GBuffer::begin()
{
	framebuffer.bind();
}

void GBuffer::end()
{
	framebuffer.unbind();
}

void GBuffer::bindTextures()
{
	texPosition.bind(GL_TEXTURE0);
	texNormal.bind(GL_TEXTURE1);
	texDiffuse.bind(GL_TEXTURE2);
}