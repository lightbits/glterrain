#ifndef BUFFER_OBJECT_H
#define BUFFER_OBJECT_H
#include <gl/opengl.h>
#include <vector>

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

	// Generate a new buffer object with the given data as initial data
	template <typename T>
	void create(GLenum target_, GLenum usage, const std::vector<T> &data) 
	{
		create(target_, usage);
		bind();
		bufferData(data.size() * sizeof(T), &data[0]);
		unbind();
	}

	void create(GLenum target_, GLenum usage, GLsizeiptr size, const void *data)
	{
		create(target_, usage);
		bind();
		bufferData(size, data);
		unbind();
	}

	// Create a new data store for the currently bound buffer object
	void bufferData(GLsizeiptr size, const void *data);

	// Redefine the given region of data for this buffer object.
	// The offset parameter is the byte offset into the buffer to begin copying to.
	// The size parameter is the number of bytes to copy.
	// The function silently returns if the defined region of memory was too large.
	void bufferSubData(GLintptr offset, GLsizeiptr size, const void *data);

	void bind();
	void unbind();

	GLuint getHandle() const;
private:
	//static std::unordered_map<GLenum, GLuint> bound_targets;
	GLenum usage;
	GLenum target;
	GLuint handle;
};

#endif