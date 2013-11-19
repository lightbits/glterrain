#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H
#include <gl/opengl.h>

class VertexArray
{
public:
	VertexArray();
	void dispose();
	void create();
	void bind() const;
	void unbind() const;
private:
	GLuint vao;
};

#endif