#include "app.h"
using namespace transform;

VertexArray vao;
ShaderProgram 
	shader_billboard,
	shader_simple;

Model cube;
MeshBuffer cube_buffer;

mat4 
	projection,
	view;

vec3 quad_p;

bool load()
{
	if (!shader_billboard.loadAndLinkFromFile("./demo/23billboards/billboard") ||
		!shader_simple.loadAndLinkFromFile("./demo/23billboards/simple"))
		return false;

	return true;
}

void free()
{
	cube_buffer.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();
	
	Mesh cube_mesh = Mesh::genUnitCube(false, false);
	cube_buffer.create(cube_mesh);
	cube = Model(cube_buffer);

	quad_p = vec3(0.5f, 0.5f, 0.5f);
	view = translate(0.0f, 0.0f, -5.0f);
	projection = perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 10.0f);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	view = 
		transform::translate(0.0f, 0.0f, -5.0f) *
		transform::rotateY(sin(ctx.getElapsedTime())) * 
		transform::rotateX(cos(ctx.getElapsedTime()));
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.71f, 0.68f, 0.68f);
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullClockwise);
	gfx.setRasterizerState(RasterizerStates::LineBoth);

	gfx.beginCustomShader(shader_simple);
	gfx.setUniform("view", view);
	gfx.setUniform("projection", projection);
	cube.transform = scale(2.0f);
	cube.draw();
	gfx.endCustomShader();

	gfx.beginCustomShader(shader_billboard);
	gfx.setUniform("view", view);
	gfx.setUniform("projection", projection);

	// Draw a viewer-oriented quad at the top-front-right corner of the cube
	gfx.setUniform("scale", vec2(1.0, 1.0));
	gfx.setUniform("model", scale(2.0f) * translate(0.5f, 0.5f, 0.5f));
	gfx.drawQuad(-1.0f, -1.0f, 2.0f, 2.0f); // Draw fullscreen quad

	// Top-back-left corner
	gfx.setUniform("scale", vec2(0.5, 0.5));
	gfx.setUniform("model", scale(2.0f) * translate(-0.5f, 0.5f, -0.5f));
	gfx.drawQuad(-1.0f, -1.0f, 2.0f, 2.0f);

	gfx.endCustomShader();
}