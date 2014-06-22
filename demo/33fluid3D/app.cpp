/*
3D fluid simulation with volume rendering
http://www.cs.columbia.edu/~keenan/Projects/GPUFluid/paper.pdf
http://scrawkblog.com/2014/01/09/gpu-gems-to-unity-3d-fluid-simulation/

Vortex confinement
*/

#include "app.h"
#include <common/noise.h>
#include <common/text.h>
#include "solver.h"
using namespace transform;

VertexArray vao;
ShaderProgram 
	shader_simple, 
	shader_slices,
	shader_planes;
MeshBuffer
	buf_quads,
	buf_cube;
mat4 
	mat_projection, 
	mat_view;

Slab
	slab_pressure,
	slab_velocity,
	slab_divergence;

bool load()
{
	if (!shader_simple.loadAndLinkFromFile("./demo/33fluid3D/simple") ||
		!shader_planes.loadAndLinkFromFile("./demo/33fluid3D/planes") ||
		!shader_slices.loadAndLinkFromFile("./demo/33fluid3D/slices"))
		return false;

	if (!initSolver())
		return false;

	return true;
}

void free()
{
	buf_quads.dispose();
	buf_cube.dispose();
	shader_simple.dispose();
	shader_slices.dispose();
	shader_planes.dispose();
	vao.dispose();
	freeSolver();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	Mesh quads;
	const int NUM_SLICES = GRID_SIZE_Z * 8;
	for (int i = 0; i < NUM_SLICES; ++i)
	{
		float z = -1.0 + 2.0 * i / NUM_SLICES;
		Mesh quad;
		quad.addPosition(-1.0f, -1.0f, z); quad.addTexel(0.0f, 0.0f);
		quad.addPosition( 1.0f, -1.0f, z); quad.addTexel(1.0f, 0.0f);
		quad.addPosition( 1.0f,  1.0f, z); quad.addTexel(1.0f, 1.0f);
		quad.addPosition(-1.0f,  1.0f, z); quad.addTexel(0.0f, 1.0f);
		uint32 indices[] = { 0, 1, 2, 2, 3, 0 };
		quad.addIndices(indices, 6);
		quads.addMesh(quad);
	}
	buf_quads.create(quads);
	buf_cube.create(Mesh::genUnitCube(false, false, true));

	mat_projection = glm::perspective(PI / 4.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 5.0f);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, 0.0f, -2.0f) * rotateX(-0.5f) * rotateY(sin(ctx.getElapsedTime() * 0.3f));


}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.setCullState(CullStates::CullCounterClockwise);

	//gfx.beginCustomShader(shader_simple);
	//gfx.setUniform("projection", mat_projection);
	//gfx.setUniform("view", mat_view);
	//gfx.setUniform("model", scale(1.0f));
	//gfx.setRasterizerState(RasterizerStates::FillBoth);
	//buf_cube.draw();

	//gfx.beginCustomShader(shader_slices);
	//gfx.setCullState(CullStates::CullNone);
	//gfx.setRasterizerState(RasterizerStates::FillBoth);
	//gfx.setBlendState(BlendStates::AlphaBlend);	
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_3D, tex_volume);
	//gfx.setUniform("texVolume", 0);
	//gfx.setUniform("projection", mat_projection);
	//gfx.setUniform("view", mat_view);
	//gfx.setUniform("model", scale(0.5f));
	//buf_quads.draw();
	//gfx.endCustomShader();
	//gfx.setBlendState(BlendStates::Default);
}