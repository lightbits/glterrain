#include <gl/texture.h>
#include <glimg/glimg.h>
#include <iostream> // for cerr
#include <string>
#include <memory> // for unique_ptr

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

// Loads texture using the glimg library
// See http://glsdk.sourceforge.net/docs/html/group__module__glimg.html for details
// glimg does not convert to power-of-two textures, so be your GPU should support NPOT textures
bool Texture::loadFromFile(const std::string &filename)
{
	try
	{
		// Allocate data for images (deletes itself when no longer used)
		std::unique_ptr<glimg::ImageSet> imgset(glimg::loaders::stb::LoadFromFile(filename));

		create(glimg::CreateTexture(imgset.get(), 0), 
			GL_TEXTURE_2D,
			imgset.get()->GetDimensions().width, 
			imgset.get()->GetDimensions().height);
	}
	catch(glimg::loaders::stb::StbLoaderException &e)
	{
		dispose();
		std::cerr<<"Failure loading texture: "<<e.what()<<"("<<filename<<")"<<std::endl;
		return false;
	}

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

void Texture::getInternalSize(int &width, int &height) const
{
	width = 0; 
	height = 0;
	//bind();
	glGetTexLevelParameteriv(target_, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(target_, 0, GL_TEXTURE_WIDTH, &height);
	//unbind();
}

int Texture::getWidth() const { return width_; }

int Texture::getHeight() const { return height_; }

GLuint Texture::getHandle() const { return handle_; }

void Texture::bind() const { glBindTexture(target_, handle_); }

void Texture::unbind() const { glBindTexture(target_, 0); }