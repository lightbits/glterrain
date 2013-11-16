#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <common/matrix.h>
#include <common/matrixstack.h>
#include <common/vec.h>
#include <graphics/color.h>
#include <graphics/texture.h>
#include <graphics/trimesh.h>
#include <graphics/program.h>
#include <graphics/bufferobject.h>
#include <graphics/vertexformat.h>
#include <graphics/vertexarray.h>
#include <graphics/spritebatch.h>
#include <graphics/bufferedmesh.h>
#include <engine/primitive.h>

namespace graphics
{

class Graphics
{
public:
	void create();
	void dispose();

	void drawPrimitive(const Primitive &primitive);
	void drawMesh(const TriMesh &mesh);
	void drawMeshes(const std::vector<TriMesh> &meshes);
private:

};

}

#endif