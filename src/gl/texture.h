#ifndef TEXTURE_H
#define TEXTURE_H
#include <common/matrix.h> // for vec2i
#include <gl/opengl.h>

class Texture2D
{
public:
	Texture2D();

	void dispose();
	bool loadFromFile(const char *filename);

	/*
	'level' The level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image.
	'internalFormat' The number of color components in the texture. (e.g. GL_RGB16f)
	'format' The format of the pixel data (i.e. GL_RGB)
	'type' The data type of the pixel data (i.e. GL_FLOAT)
	*/
	void create(
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

	int    getWidth()  const;
	int    getHeight() const;
	vec2i  getSize()   const;
	GLuint getHandle() const;

	/*
	wrapS, wrapT = GL_REPEAT: repeats the texture
	wrapS, wrapT = GL_CLAMP_TO_EDGE: does not repeat
	minFilter, magFilter = GL_LINEAR: smooths
	minFilter, magFilter = GL_NEAREST: pixel perfect
	*/
	void setTexParameteri(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT);

	void bind()             const;
	void bind(GLint unit)   const;

	// Bind the default texture (0) to the default unit (GL_TEXTURE0)
	static void unbind();
private:
	const static Texture2D *bound;
	GLuint m_handle;
	int    m_width;
	int    m_height;
};

#endif