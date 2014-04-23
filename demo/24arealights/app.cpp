#include "app.h"
using namespace transform;

VertexArray vao;
ShaderProgram shader_arealight;

Model cube;
MeshBuffer cube_buffer;

Model plane;
MeshBuffer plane_buffer;

mat4 
	projection,
	view;

bool load()
{
	if (!shader_arealight.loadAndLinkFromFile("./demo/24arealights/arealight"))
		return false;

	return true;
}

void free()
{
	cube_buffer.dispose();
	plane_buffer.dispose();
	shader_arealight.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();
	
	cube_buffer.create(Mesh::genUnitCube(false, true));
	cube = Model(cube_buffer);

	Mesh mesh_plane = Mesh::genPlane(1.0f, 1.0f);
	mesh_plane.addNormal(0.0f, 1.0f, 0.0f);
	mesh_plane.addNormal(0.0f, 1.0f, 0.0f);
	mesh_plane.addNormal(0.0f, 1.0f, 0.0f);
	mesh_plane.addNormal(0.0f, 1.0f, 0.0f);
	plane_buffer.create(mesh_plane);
	plane = Model(plane_buffer);

	view = translate(0.0f, 0.0f, -5.0f);
	projection = perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 10.0f);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	view = 
		transform::translate(0.0f, 0.0f, -5.0f) *
		transform::rotateY(sin(ctx.getElapsedTime())) * 
		transform::rotateX(-0.3f);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.71f, 0.68f, 0.68f);
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullClockwise);

	gfx.beginCustomShader(shader_arealight);
	gfx.setUniform("view", view);
	gfx.setUniform("projection", projection);
	plane.transform = scale(4.0f);
	plane.draw();
	plane.transform = translate(0.0f, 0.5f, -0.5f) * rotateX(-0.4f) * scale(1.0f, 1.0f, 0.5f);
	plane.draw();
	gfx.endCustomShader();
}