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
	MeshBuffer(Mesh &mesh);
	void dispose();

	void create(Mesh &mesh);

	/* Updates the whole buffer with the new mesh data. 
	The mesh buffer must be bound first. The new mesh
	must be of the same size as the original mesh. */
	void update(Mesh &mesh);

	/* Updates part of the buffer with the vertex data in mesh,
	starting at startIndex and ending at endIndex, for each vertex
	attribute. The mesh buffer must be bound first. The new mesh
	must be of the same size as the original mesh. */
	void update(Mesh &mesh, int startIndex, int endIndex);

	void setupVao();
	bool hasVao() const { return vaoOk; }

	void bind();
	void unbind();
	void draw(GLenum drawMode);

	bool isBound() const { return vbo.isBound() && ibo.isBound(); }
private:
	void bufferMeshBlock(Mesh &mesh/*, int first, int count*/);
	void bufferMeshInterleaved(Mesh &mesh/*, int first, int count*/);

	int stride;
	bool interleavedVertexData; // Does not work
	bool vaoOk; // VAO is not fully implemented
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