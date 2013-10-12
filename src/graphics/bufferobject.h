#ifndef SLGL_GRAPHICS_BUFFER_OBJECT_H
#define SLGL_GRAPHICS_BUFFER_OBJECT_H
#include <graphics/opengl.h>
#include <graphics/trimesh.h>

namespace graphics
{

/*
glBufferData(...) takes a usage hint for how to maintain the data on the GPU.
GL_STATIC_DRAW tells OpenGL that we intend to only set the data in the buffer object once
GL_STREAM_DRAW tells OpenGL that we intend to modify the data generally once per frame
GL_DYNAMIC_DRAW tells OpenGL that we intend to modify the data alot and use it alot
*/

class BufferObject
{
public:
	BufferObject();

	// Delete the buffer
	void dispose();

	// Generate a new buffer object.
	// See header for usage hints.
	void create(GLenum target_, GLenum usage);

	// Create a new data store for the currently bound buffer object
	void bufferData(GLsizeiptr size, const void *data);

	// Redefine the given region of data for this buffer object.
	// The offset parameter is the byte offset into the buffer to begin copying to.
	// The size parameter is the number of bytes to copy.
	// The function silently returns if the defined region of memory was too large.
	void bufferSubData(GLintptr offset, GLsizeiptr size, const void *data);

	void bind();

	// Binds the default (0) buffer for the set target buffer
	void unbind();
private:
	GLenum usage;
	GLenum target;
	GLuint handle;
};

}

#endif