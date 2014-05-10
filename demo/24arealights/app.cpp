/*
Arealight implementation based on 
	* http://stackoverflow.com/questions/17021264/improved-area-lighting-in-webgl-threejs
	* http://www.gamedev.net/topic/552315-glsl-area-light-implementation/
This demo uses forward shading. See demo 25 for deferred shading.
*/

#include "app.h"
using namespace transform;

VertexArray vao;
ShaderProgram 
	shader_arealight, // Render arealit geometry
	shader_color; // Render the light sources as colored planes

Model cube;
MeshBuffer cube_buffer;

Model plane;
MeshBuffer plane_buffer;

mat4 
	projection,
	view;

vec3 
	light_pos[2],
	light_color[2];

mat4 light_m[2];

bool load()
{
	if (!shader_arealight.loadAndLinkFromFile("./demo/24arealights/arealight") || 
		!shader_color.loadAndLinkFromFile("./demo/24arealights/color"))
		return false;

	return true;
}

void free()
{
	cube_buffer.dispose();
	plane_buffer.dispose();
	shader_arealight.dispose();
	shader_color.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	// Default VAO
	vao.create();
	vao.bind();
	
	cube_buffer.create(Mesh::genUnitCube(false, true, true));
	cube = Model(cube_buffer);

	// Create plane mesh with normals
	Mesh mesh_plane = Mesh::genPlane(1.0f, 1.0f);
	mesh_plane.addNormal(0.0f, -1.0f, 0.0f);
	mesh_plane.addNormal(0.0f, -1.0f, 0.0f);
	mesh_plane.addNormal(0.0f, -1.0f, 0.0f);
	mesh_plane.addNormal(0.0f, -1.0f, 0.0f);
	plane_buffer.create(mesh_plane);
	plane = Model(plane_buffer);

	view = translate(0.0f, 0.0f, -5.0f);
	projection = perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 20.0f);

	// Setup lights
	light_pos[0] = vec3(0.0, 1.2, 0.0);
	light_color[0] = vec3(0.8, 0.7, 0.5);
	light_m[0] = translate(light_pos[0]) * rotateX(-0.4) * scale(1.0, 0.01, 0.3);

	light_pos[1] = vec3(0.0, 7.2, 0.0);
	light_color[1] = vec3(0.5, 0.7, 0.8) * 0.3f;
	light_m[1] = translate(light_pos[1]) * rotateX(-0.4) * scale(10.0, 0.01, 10.0);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	float t = ctx.getElapsedTime();
	view = 
		transform::translate(0.0f, -0.2f, -8.0f) *
		transform::rotateX(-0.5f) *
		transform::rotateY(sin(t) * 0.4);

	light_m[0] = translate(light_pos[0]) * rotateX(t * PI * 0.5) * scale(2.0, 0.01, 1.0);
}

void renderGeometry(Renderer &gfx, Context &ctx, double dt)
{
	// Floor
	gfx.setUniform("diffuse", vec3(0.76f, 0.75f, 0.5f));
	cube.transform = scale(8.0f, 0.2f, 8.0f);
	cube.draw();

	// Back wall
	gfx.setUniform("diffuse", vec3(0.76f, 0.75f, 0.5f));
	cube.transform = translate(0.0f, 1.0f, -3.9f) * scale(8.0f, 2.0f, 0.2f);
	cube.draw();

	// Left wall
	gfx.setUniform("diffuse", vec3(0.63f, 0.06f, 0.04f));
	cube.transform = translate(-3.9f, 1.0f, 0.1f) * scale(0.2f, 2.0f, 7.9f);
	cube.draw();

	// Right wall
	gfx.setUniform("diffuse", vec3(0.15f, 0.48f, 0.09f));
	cube.transform = translate(3.9f, 1.0f, 0.1f) * scale(0.2f, 2.0f, 7.9f);
	cube.draw();
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(Color::fromHex(0x1e3b49ff));
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);

	gfx.setRasterizerState(RasterizerStates::FillBoth);
	gfx.beginCustomShader(shader_arealight);
	gfx.setUniform("view", view);
	gfx.setUniform("projection", projection);

	gfx.setUniform("light_color0", light_color[0]);
	gfx.setUniform("light_m0", light_m[0]);

	gfx.setUniform("light_color1", light_color[1]);
	gfx.setUniform("light_m1", light_m[1]);
	renderGeometry(gfx, ctx, dt);
	gfx.endCustomShader();

	// Render arealights
	gfx.setRasterizerState(RasterizerStates::LineBoth);
	gfx.beginCustomShader(shader_color);
	gfx.setUniform("projection", projection);
	gfx.setUniform("view", view);
	for (int i = 0; i < 2; ++i)
	{
		gfx.setUniform("color", light_color[i]);
		plane.transform = light_m[i];
		plane.draw();
	}
	gfx.endCustomShader();
}