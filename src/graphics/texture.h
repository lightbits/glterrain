#ifndef SLGL_GRAPHICS_TEXTURE_H
#define SLGL_GRAPHICS_TEXTURE_H
#include <graphics/opengl.h>
#include <string>

namespace graphics
{

class Texture
{
public:
	Texture();

	void dispose();
	void create2d(int width, int height, const void *data, GLenum dataType, GLenum format);
	bool loadFromFile(const std::string &filename);
	bool loadFromFile(const std::string &filename, GLenum target, GLenum minFilter, 
		GLenum magFilter, GLenum wrapS, GLenum wrapT);

	int getWidth() const;
	int getHeight() const;
	void getInternalSize(int &width, int &height) const;
	GLuint getHandle() const;

	// Disable this to preserve pixel-perfect rendering of textures (Default is enabled)
	// When disabled OpenGL will render the texture using NEAREST filtering
	void setSmooth(bool enabled);

	// Sets the appropriate texture parameters to repeat the texture
	void setRepeat(bool enabled);

	void bind() const;
	void unbind() const;
private:
	GLuint handle;
	int width;
	int height;
};

}

#endif