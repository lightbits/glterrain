#include <gl/framebuffer.h>
static const std::string FRAMEBUFFER_NOT_BOUND = "Framebuffer not bound";

// The currently bound framebuffer
Framebuffer *Framebuffer::bound = nullptr;

Framebuffer::Framebuffer() : handle(0) { }

void Framebuffer::create()
{
	dispose();
	glGenFramebuffers(1, &handle);
}

void Framebuffer::dispose()
{
	glDeleteFramebuffers(1, &handle);
}

void Framebuffer::attachTexture2D(GLenum attachmentPoint, const Texture &texture, GLint level)
{
	if (!bound) throw std::runtime_error(FRAMEBUFFER_NOT_BOUND);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D, texture.getHandle(), level);
}

void Framebuffer::detachTexture2D(GLenum attachmentPoint)
{
	if (!bound) throw std::runtime_error(FRAMEBUFFER_NOT_BOUND);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D, 0, 0);
}

void Framebuffer::attachRenderbuffer(GLenum attachmentPoint, const Renderbuffer &renderbuffer)
{
	if (!bound) throw std::runtime_error(FRAMEBUFFER_NOT_BOUND);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentPoint, GL_RENDERBUFFER, renderbuffer.getHandle());
}

void Framebuffer::detachRenderbuffer(GLenum attachmentPoint)
{
	if (!bound) throw std::runtime_error(FRAMEBUFFER_NOT_BOUND);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentPoint, GL_RENDERBUFFER, 0);
}

GLenum Framebuffer::checkStatus()
{
	if (!bound) throw std::runtime_error(FRAMEBUFFER_NOT_BOUND);
	return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

void Framebuffer::bind()
{
	bound = this;
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void Framebuffer::unbind()
{
	bound = nullptr;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}