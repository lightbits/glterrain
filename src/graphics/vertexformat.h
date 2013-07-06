#ifndef SLGL_GRAPHICS_VERTEX_FORMAT_H
#define SLGL_GRAPHICS_VERTEX_FORMAT_H
#include <graphics/opengl.h>
#include <vector>

namespace graphics
{

struct VertexAttrib
{
	VertexAttrib() : location(0), size(0), type(GL_FLOAT), normalized(GL_FALSE), stride(0), byteOffset(0)
	{ }

	VertexAttrib(GLuint location_, GLint size_, GLenum type_, GLboolean normalized_, GLsizei stride_,
		GLsizei byteOffset_) : location(location_), size(size_), type(type_), normalized(normalized_),
		stride(stride_), byteOffset(byteOffset_)
	{ }

	void enable()
	{
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, size, type, normalized, stride, reinterpret_cast<void*>(byteOffset));
	}

	void disable()
	{
		glDisableVertexAttribArray(location);
	}

	// Shader attrib index
	GLuint location;

	// Number of elements (1, 2, 3, 4)
	GLint size;

	// Data type (GL_FLOAT, GL_UNSIGNED_SHORT, ...)
	GLenum type;

	GLboolean normalized;

	// Number of bytes between the first elements of each consecutive vertex attrib
	// If the stride is 0, the data is understood to be tightly packed.
	GLsizei stride;

	// The offset to the first element of the attrib
	GLsizei byteOffset;
};

struct VertexFormat
{
	void clear()
	{
		attribs.clear();
	}

	void enable()
	{
		for(auto i = attribs.begin(); i != attribs.end(); ++i)
			i->enable();
	}

	void disable()
	{
		for(auto i = attribs.begin(); i != attribs.end(); ++i)
			i->disable();
	}

	void addAttrib(GLuint location, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLsizei byteOffset)
	{
		attribs.push_back(VertexAttrib(location, size, type, normalized, stride, byteOffset));
	}

	void addAttrib(const VertexAttrib &attrib)
	{
		attribs.push_back(attrib);
	}

	std::vector<VertexAttrib> attribs;
};

}

#endif