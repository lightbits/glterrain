#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H
#include <graphics/opengl.h>

namespace graphics
{

class VertexArray
{
public:
	VertexArray();
	void dispose();
	void create();
	void bind();
	void unbind();
private:
	GLuint vao;
};

}

#endif