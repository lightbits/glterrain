#ifndef SLGL_GRAPHICS_BUFFERED_MESH_H
#define SLGL_GRAPHICS_BUFFERED_MESH_H
#include <graphics/opengl.h>
#include <graphics/trimesh.h>
#include <graphics/vertexformat.h>
#include <graphics/bufferobject.h>
#include <unordered_map>
#include <string>

namespace graphics
{

// A list of common attrib indices and uniform inputs
struct ShaderLayout
{
	std::unordered_map<std::string, GLint> attribs;
	std::unordered_map<std::string, GLint> uniforms;

	GLint getAttribLoc(const std::string &name) const
	{
		std::unordered_map<std::string, GLint>::const_iterator i = attribs.find(name);
		return i != attribs.end() ? i->second : -1;
	}

	GLint getUniformLoc(const std::string &name) const
	{
		std::unordered_map<std::string, GLint>::const_iterator i = uniforms.find(name);
		return i != uniforms.end() ? i->second : -1;
	}
};

// TODO: Add interlaced vertex format for batching possibilities (static buffered mesh)
// This class is more useful for run-time buffer generation, as we simply dump data in blocks
class BufferedMesh
{
public:
	BufferedMesh();

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