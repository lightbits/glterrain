/*
http://www.songho.ca/opengl/gl_fbo.html
*/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <gl/opengl.h>
#include <gl/texture.h>

/*
Allows to render a scene directly to a renderbuffer object, instead of rendering to a texture object. 
Renderbuffer is simply a data storage object containing a single image of a renderable internal format. 
It is used to store OpenGL logical buffers that do not have corresponding texture format, such as stencil or depth buffer.
*/
class Renderbuffer
{
public:
	Renderbuffer() : handle(0) { }

	void create()
	{
		dispose();
		glGenRenderbuffers(1, &handle);
	}

	void dispose()
	{
		glDeleteRenderbuffers(1, &handle);
	}

	/*
	Allocate memory space. 
	internalFormat is either color-renderable (GL_RGB, GL_RGBA, etc.), 
	depth-renderable (GL_DEPTH_COMPONENT), or stencil-renderable formats (GL_STENCIL_INDEX).

	The width and height are dimension of the renderbuffer image in pixels.
	*/
	void storage(GLenum internalFormat, GLsizei width, GLsizei height)
	{
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
	}

	/*
	Various parameters can be get from the currently bound renderbuffer object. 
	param is the name of the parameter, and must be one of the following:
	GL_RENDERBUFFER_WIDTH
	GL_RENDERBUFFER_HEIGHT
	GL_RENDERBUFFER_INTERNAL_FORMAT
	GL_RENDERBUFFER_RED_SIZE
	GL_RENDERBUFFER_GREEN_SIZE
	GL_RENDERBUFFER_BLUE_SIZE
	GL_RENDERBUFFER_ALPHA_SIZE
	GL_RENDERBUFFER_DEPTH_SIZE
	GL_RENDERBUFFER_STENCIL_SIZE
	*/
	GLint getParameteriv(GLenum param) const
	{
		GLint value;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, param, &value);
		return value;
	}

	GLint getWidth() const { return getParameteriv(GL_RENDERBUFFER_WIDTH); }
	GLint getHeight() const { return getParameteriv(GL_RENDERBUFFER_HEIGHT); }
	GLuint getHandle() const { return handle; }

	void bind()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, handle);
	}

	void unbind()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
private:
	GLuint handle;
};

class Framebuffer
{
public:
	Framebuffer();
	void create();
	void dispose();

	/*
	'attachmentPoint' Where to connect the texture image. 
	Must be either a color attachment point (GL_COLOR_ATTACHMENT<n>),
	GL_DEPTH_ATTACHMENT or GL_STENCIL_ATTACHMENT.

	'level' The mipmap level of the texture to be attached.
	*/
	void attachTexture2D(GLenum attachmentPoint, const Texture2D &texture, GLint level);

	/*
	If a texture object is deleted while it is still attached to a FBO, then, 
	the texture image will be automatically detached from the currently bound FBO. 
	However, if it is attached to multiple FBOs and deleted, then it will be detached 
	from only the bound FBO, but will not be detached from any other un-bound FBOs.
	*/
	void detachTexture2D(GLenum attachmentPoint);

	void attachRenderbuffer(GLenum attachmentPoint, const Renderbuffer &renderbuffer);

	/*
	If a renderbuffer object is deleted while it is still attached in a FBO, 
	then it will be automatically detached from the bound FBO. However, 
	it will not be detached from any other non-bound FBOs.
	*/
	void detachRenderbuffer(GLenum attachmentPoint);

	/*
	Once attachable images (textures and renderbuffers) are attached to a FBO and 
	before performing FBO operation, you must validate if the FBO status is complete or incomplete.

	If all requirements are satisfied, the framebuffer is complete, and the function returns
	GL_FRAMEBUFFER_COMPLETE. Otherwise it returns an error enum.

	The rules of FBO completeness are:

	*	The width and height of framebuffer-attachable image must be not zero.
	*	If an image is attached to a color attachment point, then the image must have a color-renderable internal format. (GL_RGBA, GL_DEPTH_COMPONENT, GL_LUMINANCE, etc)
	*	If an image is attached to GL_DEPTH_ATTACHMENT, then the image must have a depth-renderable internal format. (GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, etc)
	*	If an image is attached to GL_STENCIL_ATTACHMENT, then the image must have a stencil-renderable internal format. (GL_STENCIL_INDEX, GL_STENCIL_INDEX8, etc)
	*	FBO must have at least one image attached.
	*	All images attached a FBO must have the same width and height.
	*	All images attached the color attachment points must have the same internal format.

	Note that even though all conditions above may be satisfied, your OpenGL driver might not support all
	combinations of internal formats and parameters. In that case, the function returns GL_FRAMEBUFFER_UNSUPPORTED.
	*/
	GLenum checkStatus();

	/* Once a FBO is bound, all OpenGL operations affect onto the current bound framebuffer object. */
	void bind();

	/* Binds default window-system framebuffer */
	void unbind();
private:
	static Framebuffer *bound;
	GLuint handle;	
};

#endif