#include <graphics/texture.h>
#include <glimg/glimg.h>
#include <iostream> // for cerr
#include <string>
#include <memory> // for unique_ptr
using namespace graphics;

Texture::Texture() : handle(0), width(0), height(0)
{
	//textureUnit = 0; // Default texture unit (corresponds to GL_TEXTURE0)
	setRepeat(false);
	setSmooth(false);
}

void Texture::dispose()
{
	glDeleteTextures(1, &handle);
}

int Texture::getWidth() const
{
	return width;
}

int Texture::getHeight() const
{
	return height;
}

GLuint Texture::getHandle() const
{
	return handle;
}

void Texture::setRepeat(bool enabled)
{
	glBindTexture(GL_TEXTURE_2D, handle);
	GLuint wrap = enabled ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::setSmooth(bool enabled)
{
	glBindTexture(GL_TEXTURE_2D, handle);
	GLuint filter = enabled ? GL_LINEAR : GL_NEAREST;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, handle); // Bind texture object to the active texture unit location
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
		handle = glimg::CreateTexture(imgset.get(), 0);
		width = imgset.get()->GetDimensions().width;
		height = imgset.get()->GetDimensions().height;
	}
	catch(glimg::loaders::stb::StbLoaderException &e)
	{
		handle = 0;
		width = 0;
		height = 0;
		std::cerr<<"Failure loading texture: "<<e.what()<<"("<<filename<<")"<<std::endl;
		return false;
	}

	std::cout<<"Loaded texture: "<<filename<<" ("<<width<<", "<<height<<")"<<std::endl;

	return true;
}