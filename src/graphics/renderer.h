#ifndef SLGL_GRAPHICS_RENDERER_H
#define SLGL_GRAPHICS_RENDERER_H
#include <graphics/opengl.h>
#include <graphics/trimesh.h>

namespace graphics
{

class Renderer /*GLRenderer : public Renderer */
{
public:
	void draw(const TriMesh &mesh);
private:

};

}

#endif