#include "app.h"
#include <camera/fancycamera.h>

VertexArray vao;
ShaderProgram 
	shader_simple,
	shader_background;
Model 
	skybox,
	cube;
MeshBuffer 
	skybox_buffer,
	cube_buffer;

FancyCamera camera;

mat4 
	mat_view,
	mat_projection;

Cubemap cubemap;

bool load()
{
	if (!shader_simple.loadAndLinkFromFile("./demo/11camera/simple") ||
		!shader_background.loadAndLinkFromFile("./demo/11camera/background"))
		return false;

	if (!cubemap.loadFromFile("./data/cubemaps/iceland_", ".jpg"))
		return false;

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	cubemap.setTexParameteri(
		GL_LINEAR, 
		GL_LINEAR, 
		GL_CLAMP_TO_EDGE, 
		GL_CLAMP_TO_EDGE, 
		GL_CLAMP_TO_EDGE);

	return true;
}

void free()
{
	skybox_buffer.dispose();
	cube_buffer.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	cube_buffer.create(Mesh::genUnitCube(false, true, true));
	cube = Model(cube_buffer);

	skybox_buffer.create(Mesh::genUnitCube(false, false, true));
	skybox = Model(skybox_buffer);

	mat_view = mat4(1.0f);
	mat_projection = glm::perspective(PI / 5.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.05f, 15.0f);

	camera.reset(0.0f, 0.0f, vec3(0.0f, 0.2f, 1.0f));
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	camera.update(gfx, ctx, dt);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.2f, 0.2f, 0.3f);
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);

	gfx.beginCustomShader(shader_simple);
	gfx.setUniform("view", camera.getViewMatrix());
	gfx.setUniform("projection", mat_projection);
	cube.transform = transform::scale(0.3f);
	cube.draw();
	gfx.endCustomShader();

	// Render a 10x10x10 cube centered at the camera,
	// displaying the cubemap as a skybox
	gfx.beginCustomShader(shader_background);
	gfx.setUniform("view", camera.getViewMatrix());
	gfx.setUniform("projection", mat_projection);
	skybox.transform = transform::scale(10.0f);
	skybox.draw();
	gfx.endCustomShader();
}