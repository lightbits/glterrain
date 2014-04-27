#include <gl/bufferobject.h>
#include <iostream>

const std::string BUFFER_NOT_BOUND = "Buffer not bound";
const BufferObject *BufferObject::bound = nullptr;

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
	if (!bound)
		throw std::runtime_error(BUFFER_NOT_BOUND);
	glBufferData(target, size, data, usage);
}

void BufferObject::bufferSubData(GLintptr offset, GLsizeiptr size, const void *data)
{
	if (!bound)
		throw std::runtime_error(BUFFER_NOT_BOUND);
	glBufferSubData(target, offset, size, data);
}

void BufferObject::bind()
{
	bound = this;
	glBindBuffer(target, handle);
}

void BufferObject::unbind()
{
	bound = nullptr;
	glBindBuffer(target, 0);
}

GLuint BufferObject::getHandle() const
{
	return handle;
}

bool BufferObject::isBound() const
{
	return bound != nullptr;
}