#ifndef SLGL_GRAPHICS_BUFFERED_MESH_H
#define SLGL_GRAPHICS_BUFFERED_MESH_H
#include <graphics/opengl.h>
#include <graphics/trimesh.h>
#include <graphics/vertexformat.h>
#include <graphics/bufferobject.h>
#include <graphics/program.h>

namespace graphics
{

// TODO: Add interlaced vertex format for batching possibilities (static buffered mesh)
// This class is more useful for run-time buffer generation, as we simply dump data in blocks
class BufferedMesh
{
public:
	BufferedMesh();

	void dispose();
	void create(const TriMesh &mesh, const ProgramLayout &layout);
	void draw();
private:
	BufferObject vbo;
	BufferObject ibo;
	VertexFormat fmt;
	unsigned int indexCount;
};

}

#endif