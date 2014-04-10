#include <gl/texture.h>
#include <SOIL.h>
#include <iostream>
#include <string>

Texture::Texture() : width_(0), height_(0), handle_(0), target_(GL_TEXTURE_2D)
{

}

void Texture::dispose()
{
	glDeleteTextures(1, &handle_);
	width_ = 0;
	height_ = 0;
	target_ = GL_TEXTURE_2D;
}

void Texture::create(GLuint handle, GLenum target, int width, int height)
{
	dispose();
	target_ = target;
	handle_ = handle;
	width_ = width;
	height_ = height;
}

void Texture::create(GLenum target, int width, int height)
{
	dispose();
	glGenTextures(1, &handle_);
	target_ = target;
	width_ = width;
	height_ = height;
}

void Texture::create2d(GLint level,
		GLint internalFormat, 
		GLsizei width, 
		GLsizei height, 
		GLenum format, 
		GLenum type, 
		const GLvoid *data)
{
	dispose();
	create(GL_TEXTURE_2D, width, height);
	bind();
	glTexImage2D(target_, level, internalFormat, width, height, 0, format, type, data);
	unbind();
}

void Texture::copyFromFramebuffer(			
		GLint level,			
		GLint xoffset,			
		GLint yoffset,			
		GLint x, GLint y,		
		GLsizei width,			
		GLsizei height)
{
	// http://www.opengl.org/sdk/docs/man/xhtml/glCopyTexSubImage2D.xml
	glCopyTexSubImage2D(target_, level, xoffset, yoffset, x, y, width, height);
}

bool Texture::loadFromFile(const std::string &filename)
{
	GLuint tex = SOIL_load_OGL_texture(
		filename.c_str(), SOIL_LOAD_RGBA,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y);

	if (tex == 0)
	{
		std::cerr << "Failed to load texture (" << filename << "): " << SOIL_last_result() << std::endl;
		return false;
	}

	// Retreive the dimensions
	glBindTexture(GL_TEXTURE_2D, tex);
	GLint w = 0;
	GLint h = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	glBindTexture(GL_TEXTURE_2D, 0);

	create(tex, GL_TEXTURE_2D, w, h);
	
	return true;
}

void Texture::setTexParameteri(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT)
{
	//bind();
	glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(target_, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(target_, GL_TEXTURE_WRAP_T, wrapT);
	//unbind();
}

void Texture::getInternalSize(int *width, int *height) const
{
	*width = 0; 
	*height = 0;
	//bind();
	glGetTexLevelParameteriv(target_, 0, GL_TEXTURE_WIDTH, width);
	glGetTexLevelParameteriv(target_, 0, GL_TEXTURE_WIDTH, height);
	//unbind();
}

int Texture::getWidth() const { return width_; }

int Texture::getHeight() const { return height_; }

GLuint Texture::getHandle() const { return handle_; }

void Texture::bind() const { glBindTexture(target_, handle_); }

void Texture::unbind() const { glBindTexture(target_, 0); }