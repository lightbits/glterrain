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
	'internalFormat' The format in which the pixels should be stored on the GPU (e.g. GL_RGB16f)
	'format' The format of the pixel data 'data' (i.e. GL_RGB)
	'type' The data type of the pixel data 'data' (i.e. GL_FLOAT)
	'data' The pixel data (can be NULL)
	*/
	void create(
		GLint level,
		GLint internalFormat,
		GLsizei width,
		GLsizei height,
		GLenum format,
		GLenum type,
		const GLvoid *data);

	/*
	Redefines a subregion of an existing texture, replacing the portion with x indices
	xoffset and xoffset + width - 1, and y indices yoffset and yoffset + height - 1.
	The pixel data is expected to have the format 'format', with each component of datatype 'type'.
	*/
	void update(
		const GLvoid *data,
		GLenum format,
		GLenum type,
		int xoffset = 0,
		int yoffset = 0,
		int width   = 0,
		int height  = 0,
		GLint level = 0);

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
	// Should disable copying
	//void operator=(const Texture2D &rhs) { }
	//Texture2D(const Texture2D &copy) { }

	const static Texture2D *bound;
	GLuint m_handle;
	int    m_width;
	int    m_height;
};

#endif