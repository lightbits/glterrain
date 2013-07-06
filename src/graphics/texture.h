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

	int getWidth() const;
	int getHeight() const;
	GLuint getHandle() const;

	// Disable this to preserve pixel-perfect rendering of textures (Default is enabled)
	// When disabled OpenGL will render the texture using NEAREST filtering
	void setSmooth(bool enabled);

	// Sets the appropriate texture parameters to repeat the texture
	void setRepeat(bool enabled);

	void bind() const;

	bool loadFromFile(const std::string &filename);
private:
	GLuint handle;
	int width;
	int height;
};

}

#endif