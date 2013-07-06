#ifndef SLGL_GRAPHICS_RENDER_MODEL_H
#define SLGL_GRAPHICS_RENDER_MODEL_H
#include <graphics/opengl.h>
#include <graphics/trimesh.h>
#include <graphics/vertexformat.h>
#include <graphics/bufferobject.h>

namespace graphics
{

// A list of common attrib indices and uniform inputs
struct ShaderLayout
{
	GLint positionAttribIndex;
	GLint colorAttribIndex;
	GLint normalAttribIndex;
	GLint texelAttribIndex;
	GLint projectionUniform;
	GLint viewUniform;
	GLint modelUniform;
	GLint texBlendUniform;
};

//
// //// Uses interlaced vertex format for batching possibilities
////class StaticRenderModel
////{
////public:
////	StaticRenderModel();
////
////	void create(const TriMesh &mesh)
////	{
////		glGenVertexArrays(1, &vao);
////		glBindVertexArray(vao);
////
////		ibo.bind(); // The binding to GL_ELEMENT_ARRAY_BUFFER is stored in the vao
////
////		glBindVertexArray(0);
////		ibo.unbind();
////	}
////private:
////	BufferObject vbo;
////	BufferObject ibo;
////	VertexFormat fmt;
////	int indexCount;
////	GLuint vao;
////};
//
class DynamicRenderModel
{
public:
	DynamicRenderModel();

	void dispose();
	void create(const TriMesh &mesh, const ShaderLayout &layout);

	void draw();
private:
	BufferObject vbo;
	BufferObject ibo;
	VertexFormat fmt;
	int indexCount;
};

}

#endif