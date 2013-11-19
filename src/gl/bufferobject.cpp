#include <gl/bufferobject.h>
#include <iostream>

BufferObject::BufferObject() : bound(false), handle(0), target(GL_ARRAY_BUFFER), usage(GL_STATIC_DRAW)
{

}

void BufferObject::dispose()
{
	bound = false;
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
	bound = true;
	glBindBuffer(target, handle);
}

void BufferObject::unbind()
{
	bound = false;
	glBindBuffer(target, 0);
}