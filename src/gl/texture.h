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

	void create2d(GLint level,	// Level-of-detail number
		GLint internalFormat,	// Color component type in the texture
		GLsizei width,			// Width of the texture
		GLsizei height,			// Height of the texture
		GLenum format,			// The format of the pixel data
		GLenum type,			// The data type of the pixel data
		const GLvoid *data);	// A pointer to the pixel data in memory

	bool loadFromFile(const std::string &filename);

	int getWidth() const;
	int getHeight() const;
	void getInternalSize(int &width, int &height) const;
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