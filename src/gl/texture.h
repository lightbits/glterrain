#ifndef TEXTURE_H
#define TEXTURE_H
#include <gl/opengl.h>
#include <string>

class Texture
{
public:
	Texture();

	void dispose();
	void create(GLuint texture, GLenum target, int width, int height);
	void create(GLenum target, int width, int height);

	/*
	'level' The level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image.

	'internalFormat' The number of color components in the texture. (e.g. GL_RGB)

	'format' The format of the pixel data (e.g. GL_BGR)

	'type' The data type of the pixel data (e.g. GL_FLOAT)
	*/
	void create2d(
		GLint level,
		GLint internalFormat,
		GLsizei width,
		GLsizei height,
		GLenum format,
		GLenum type,
		const GLvoid *data);

	// Replaces a rectangular portion of the texture image
	void copyFromFramebuffer(
		GLint level,			// Level-of-detail number. Level 0 is base image level.
		GLint xoffset,			// A horizontal texel offset within the texture array
		GLint yoffset,			// A vertical texel offset within the texture array
		GLint x, GLint y,		// Window coordinates of lower left corner to begin copying
		GLsizei width,			// Width of texture subimage
		GLsizei height);		// Height of texture subimage

	bool loadFromFile(const std::string &filename);

	int getWidth() const;
	int getHeight() const;
	void getInternalSize(int *width, int *height) const;
	GLuint getHandle() const;

	/*
	wrapS, wrapT = GL_REPEAT: repeats the texture
	wrapS, wrapT = GL_CLAMP_TO_EDGE: does not repeat
	minFilter, magFilter = GL_LINEAR: smooths
	minFilter, magFilter = GL_NEAREST: pixel perfect
	*/
	void setTexParameteri(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT);

	// Bind texture object to the active texture unit location
	void bind() const;

	// Binds null object to the target
	void unbind() const;
private:
	GLenum target_;
	GLuint handle_;
	int width_;
	int height_;
};

#endif