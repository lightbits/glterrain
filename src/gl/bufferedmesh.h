/*
http://stackoverflow.com/questions/18853713/does-interleaving-in-vbos-speed-up-performance-when-using-vaos
*/

#ifndef BUFFERED_MESH_H
#define BUFFERED_MESH_H
#include <gl/opengl.h>
#include <gl/vertexformat.h>
#include <gl/vertexarray.h>
#include <gl/bufferobject.h>
#include <gl/program.h>
#include <graphics/mesh.h>
#include <graphics/trimesh.h>

class ShaderProgram;

class MeshBuffer
{
public:
	MeshBuffer();
	void dispose();

	void create(Mesh &mesh, bool interleaved=false);

	void setupVao();
	bool hasVao() const { return vaoOk; }

	void bind();
	void unbind();
	void draw(GLenum drawMode);
private:
	int stride;
	bool interleavedVertexData;
	bool vaoOk;
	VertexArray vao;
	BufferObject vbo;
	BufferObject ibo;
	const Mesh *meshPtr;
};

// TODO: Add intereaved vertex format for batching possibilities (static buffered mesh)
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

#endif