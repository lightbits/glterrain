#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <common/matrix.h>
#include <common/matrixstack.h>
#include <common/vec.h>

namespace graphics
{

class Graphics
{
public:
	void create();
	void dispose();

	// void drawPrimitive(const Primitive &primitive);
	// void drawMesh(const TriMesh &mesh);
	// void drawMeshes(const std::vector<TriMesh> &meshes);
private:

};

}

#endif