#include "app.h"

VertexArray vao;
ShaderProgram 
	shader_simple,
	shader_background,
	shader_reflect;
Model cube;
MeshBuffer cube_buffer;

mat4 
	mat_view,
	mat_projection;

Cubemap cubemap;

bool load()
{
	if (!shader_simple.loadAndLinkFromFile("./demo/13cubemap/simple") ||
		!shader_background.loadAndLinkFromFile("./demo/13cubemap/background") ||
		!shader_reflect.loadAndLinkFromFile("./demo/13cubemap/reflect"))
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
	cube_buffer.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	cube_buffer.create(Mesh::genUnitCube(false, false, true));
	cube = Model(cube_buffer);

	mat_view = mat4(1.0f);
	mat_projection = glm::perspective(PI / 4.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.05f, 15.0f);
}

//vec2 camera_r = vec2(0.0f, 0.0f);
//vec2 camera_v = vec2(0.0f, 0.0f);
vec2i last_mouse_pos = vec2i(0, 0);
float r = 0.0f;
float v = 0.0f;
void update(Renderer &gfx, Context &ctx, double dt)
{
	//r += v * dt;

	//if (ctx.isKeyPressed('d'))
	//{
	//	if (v < 1.5f)
	//		v += 1.5f * dt;
	//}
	//else if (ctx.isKeyPressed('a'))
	//{
	//	if (v > -1.5f)
	//		v -= 1.5 * dt;
	//}
	//else
	//{
	//	v -= 3.0f * v * dt + 0.1f * v * v * dt;
	//}

	vec2i mouse_pos = ctx.getMousePos();
	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		vec2i delta = mouse_pos - last_mouse_pos;
		std::cout << dt * 1000.0 << std::endl;
		v = 0.1f * delta.x * dt;
	}
	else
	{
		last_mouse_pos = mouse_pos;
		v -= 3.0f * v * dt;
	}

	r += v * dt;

	mat_view = 
		transform::translate(0.0f, 0.0f, -3.5f) * 
		transform::rotateY(-r);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.2f, 0.2f, 0.3f);
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);

	// Render a 10x10x10 cube centered at the camera,
	// displaying the cubemap as a skybox
	gfx.beginCustomShader(shader_background);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
	cube.transform = transform::scale(10.0f);
	cube.draw();
	gfx.endCustomShader();

	//gfx.beginCustomShader(shader_reflect);
	//gfx.setUniform("view", mat_view);
	//gfx.setUniform("projection", mat_projection);
	//cube.transform = transform::rotateY(ctx.getElapsedTime());
	//cube.draw();
	//gfx.endCustomShader();
}