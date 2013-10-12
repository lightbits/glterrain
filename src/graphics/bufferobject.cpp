#include <graphics/bufferobject.h>
#include <iostream>
using namespace graphics;

BufferObject::BufferObject() : handle(0), target(GL_ARRAY_BUFFER), usage(GL_STATIC_DRAW)
{

}

void BufferObject::dispose()
{
	glDeleteBuffers(1, &handle);
}

void BufferObject::create(GLenum target_, GLenum usage_)
{
	target = target_;
	usage = usage_;
	glGenBuffers(1, &handle);
}

void BufferObject::bufferData(GLsizeiptr size, const void *data)
{
	glBufferData(target, size, data, usage);
}

void BufferObject::bufferSubData(GLintptr offset, GLsizeiptr size, const void *data)
{
	glBufferSubData(target, offset, size, data);
}

void BufferObject::bind()
{
	glBindBuffer(target, handle);
}

void BufferObject::unbind()
{
	glBindBuffer(target, 0); // Deprecated? (0 is a non-application generated buffer object)
}