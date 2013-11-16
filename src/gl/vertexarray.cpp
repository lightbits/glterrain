#include <gl/vertexarray.h>

VertexArray::VertexArray() : vao(0)
{

}

void VertexArray::dispose()
{
	glDeleteVertexArrays(1, &vao);
}

void VertexArray::create()
{
	dispose();
	glGenVertexArrays(1, &vao);
}

void VertexArray::bind()
{
	glBindVertexArray(vao);
}

void VertexArray::unbind()
{
	glBindVertexArray(0);
}