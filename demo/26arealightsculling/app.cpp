/*
Arealight implementation based on 
	* http://stackoverflow.com/questions/17021264/improved-area-lighting-in-webgl-threejs
	* http://www.gamedev.net/topic/552315-glsl-area-light-implementation/
This demo uses deferred shading to render multiple arealights.
*/
												  
#include "app.h"
#include <gl/gbuffer.h>
using namespace transform;

VertexArray vao;
ShaderProgram 
	shader_fp, // First pass (geometry)
	shader_sp, // Second pass (shading)
	shader_color; // Render colored planes

Model 
	cube,
	cube_light,
	plane;

MeshBuffer 
	cube_buffer,
	cube_light_buffer,
	plane_buffer,
	screen_quad;

mat4 
	projection,
	view;

GBuffer gbuffer;

struct AreaLight
{
	AreaLight() : 
		m_transform(1.0), 
		m_position(0.0), 
		m_intensity(0.0) 
	{ }

	AreaLight(
		float width, 
		float height, 
		vec3  intensity, 
		vec3  position,
		vec3  orientation)
	{
		m_transform = 
			translate(position) * 
			rotateX(orientation.x) * 
			rotateY(orientation.y) * 
			rotateZ(orientation.z) *
			scale(width, 1.0, height);
		m_position = position;
		m_intensity = intensity;
	}
	mat4 m_transform;
	vec3 m_position;
	vec3 m_intensity;
};

vector<AreaLight> lights;

bool load()
{
	if (!shader_fp.loadFromFile("./demo/26arealightsculling/first_pass") ||
		!shader_sp.loadFromFile("./demo/26arealightsculling/second_pass") ||
		!shader_color.loadAndLinkFromFile("./demo/26arealightsculling/color"))
		return false;

	// Output variables (color numbers)
	shader_fp.bindFragDataLocation("out_p", 0);
	shader_fp.bindFragDataLocation("out_n", 1);
	shader_fp.bindFragDataLocation("out_d", 2);

	if (!shader_fp.linkAndCheckStatus() ||
		!shader_sp.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	cube_buffer.dispose();
	plane_buffer.dispose();
	shader_fp.dispose();
	shader_sp.dispose();
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

	cube_light_buffer.create(Mesh::genUnitCube(false, false, true));
	cube_light = Model(cube_light_buffer);

	screen_quad.create(Mesh::genPlane(vec3(0, 1, 0), vec3(1, 0, 0)));

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

	gbuffer.create(ctx.getWidth(), ctx.getHeight());

	// Setup lights
	lights.push_back(AreaLight(1.0f, 0.3f, vec3(0.8, 0.7, 0.5) * 1.5f, vec3(0.0, 10.2, 0.0), vec3(-0.4, 0.0, 0.0)));
	lights.push_back(AreaLight(10.0f, 10.0f, vec3(0.5, 0.7, 0.8) * 2.5f, vec3(0.0, 7.2, 0.0), vec3(-0.4, 0.0, 0.0)));
	lights.push_back(AreaLight(1.0f, 0.3f, vec3(0.9, 0.3, 0.35) * 1.8f, vec3(0.0, 1.2, 0.0), vec3(0, 0, 0)));
	lights.push_back(AreaLight(10.0f, 10.0f, vec3(0.9, 0.92, 1.0) * 0.04f, vec3(0.0, 10.0, 0.0), vec3(0, 0, 0)));
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	float t = ctx.getElapsedTime();
	view = 
		transform::translate(0.0f, -0.2f, -8.0f) *
		transform::rotateX(-0.5f) *
		transform::rotateY(sin(t) * 0.4);

	// Back light
	lights[0].m_transform = 
		translate(0.8f * sin(t + 0.3f), 1.1f + 0.2f * cos(t + 0.3f), -2.5f) * 
		rotateX(PI / 2.0f - 0.79f) * 
		scale(2.0, 1.0, 1.0);

	// Right light
	lights[1].m_transform = 
		translate(2.5f, 1.1f + 0.2f * cos(t), 0.8f * sin(t)) * 
		rotateZ(-PI / 2.0 + 0.72f) * 
		rotateY(PI / 2.0) * 
		scale(2.0, 1.0, 1.0);

	// Left light
	lights[2].m_transform = 
		translate(-2.5f, 1.1f + 0.2f * cos(t + 0.9f), 0.8f * sin(t + 0.9f)) * 
		rotateZ(PI / 2.0 - 0.75f) * 
		rotateY(PI / 2.0) * 
		scale(2.0, 1.0, 1.0);
}

void renderGeometry(Renderer &gfx, Context &ctx, double dt)
{
	// Floor
	gfx.setUniform("diffuse", vec3(0.5f, 0.5f, 0.5f));
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
	// Geometry pass
	gbuffer.begin();
	gfx.beginCustomShader(shader_fp);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setRasterizerState(RasterizerStates::FillBoth);
	gfx.setBlendState(BlendStates::Default);
	gfx.setClearDepth(1.0);

	// Clear fragment output buffers to all zero (!)
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setUniform("projection", projection);
	gfx.setUniform("view", view);
	renderGeometry(gfx, ctx, dt);
	gfx.endCustomShader();
	gbuffer.end();

	// Deferred pass
	gfx.setDepthTestState(DepthTestStates::Disabled);
	gfx.setCullState(CullStates::CullNone);
	gfx.setBlendState(BlendStates::Additive);
	gfx.setClearColor(0.0f, 0.0f, 0.0f); // Serves as ambient lighting (which we shall have none of!)
	gfx.clearColorBuffer();	

	gfx.beginCustomShader(shader_sp);
	gbuffer.bindTextures();
	gfx.setUniform("inv_projection", glm::inverse(projection));
	//gfx.setUniform("projection", projection);
	gfx.setUniform("view", view);
	gfx.setUniform("tex_p", 0);
	gfx.setUniform("tex_n", 1);
	gfx.setUniform("tex_d", 2);

	for (int i = 0; i < lights.size(); ++i)
	{
		// Yeah this is pretty bad for parallelism!
		cube_light.transform = lights[i].m_transform;
		gfx.setUniform("light_i", lights[i].m_intensity);
		gfx.setUniform("light_m", lights[i].m_transform);
		//cube_light.draw();
		screen_quad.draw();
	}
	gfx.endCustomShader();

	// Render arealights
	gfx.setRasterizerState(RasterizerStates::FillBoth);
	gfx.setBlendState(BlendStates::Default);
	gfx.beginCustomShader(shader_color);
	gfx.setUniform("projection", projection);
	gfx.setUniform("view", view);
	for (int i = 0; i < lights.size(); ++i)
	{
		gfx.setUniform("color", lights[i].m_intensity);
		plane.transform = lights[i].m_transform;
		plane.draw();
	}
	gfx.endCustomShader();
}