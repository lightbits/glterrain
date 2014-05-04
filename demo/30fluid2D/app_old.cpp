#include "app.h"

struct Slab
{
	RenderTexture ping;
	RenderTexture pong;
};

VertexArray vao;
ShaderProgram 
	shader_jacobi,
	shader_texture;
BufferObject vbo_quad;

bool load()
{
	if (!shader_jacobi.loadFromFile(  "./demo/30fluid2D/quad.vs", "./demo/30fluid2D/jacobi.fs") ||
		!shader_texture.loadFromFile( "./demo/30fluid2D/quad.vs", "./demo/30fluid2D/texture.fs"))
		return false;

	if (!shader_jacobi.linkAndCheckStatus() ||
		!shader_texture.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

void initFBO(int i)
{
	fbo[i].create();
	fbo[i].bind();
	fbo[i].attachTexture2D(GL_COLOR_ATTACHMENT0, tex_velocity[i], 0);
	fbo[i].attachTexture2D(GL_COLOR_ATTACHMENT1, tex_pressure[i], 0);

	// Specify output targets
	GLenum targets[] = { 
		GL_COLOR_ATTACHMENT0, 
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, targets);

	GLenum status = fbo[i].checkStatus();
	if (status != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer not complete");

	fbo[i].unbind();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	//int tex_res_x = ctx.getWidth();
	//int tex_res_y = ctx.getHeight();

	//float init_velocity[] = {
	//	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 0.0f,	0.7f, 0.4f, 0.4f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,
	//	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,	0.4f, 0.7f, 0.4f,	0.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f
	//};

	//float init_pressure[] = {
	//	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,
	//	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f
	//};

	tex_velocity[0].create(0, GL_RGB32F, tex_res_x, tex_res_y, GL_RGB, GL_FLOAT, NULL);
	tex_velocity[1].create(0, GL_RGB32F, tex_res_x, tex_res_y, GL_RGB, GL_FLOAT, NULL);

	tex_pressure[0].create(0, GL_RGB32F, tex_res_x, tex_res_y, GL_RGB, GL_FLOAT, NULL);
	tex_pressure[1].create(0, GL_RGB32F, tex_res_x, tex_res_y, GL_RGB, GL_FLOAT, NULL);

	tex_velocity[0].setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_velocity[1].setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_pressure[0].setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_pressure[1].setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	initFBO(0);
	initFBO(1);

	// Create quad covering the entire buffer except a one pixel line
	// surrounding it. Note that the offset is applied to the TEXELS and POSITIONS,
	// The texels determine where we READ from the buffer - we want to 
	// ignore the one-pixel boundary of size dx, dy.

	// The positions determine where we WRITE to the buffer - we only want to
	// write to the quad minus the one-pixel boundary. Because we input NDC (actually
	// clip-space, but w is set to 1) coordinates, we need to subtract the equivalent of
	// one pixel on each side, which is 2dx and 2dy.
	float dx = 1.0f / tex_res_x;
	float dy = 1.0f / tex_res_y;
	float tdx = 2.0f * dx;
	float tdy = 2.0f * dy;
	float quad1[] = {
		-1.0f + tdx, -1.0f + tdy,	0.0f + dx, 0.0f + dy,
		 1.0f - tdx, -1.0f + tdy,	1.0f - dx, 0.0f + dy,
		 1.0f - tdx,  1.0f - tdy,	1.0f - dx, 1.0f - dy,
		 1.0f - tdx,  1.0f - tdy,	1.0f - dx, 1.0f - dy,
		-1.0f + tdx,  1.0f - tdy,	0.0f + dx, 1.0f - dy,
		-1.0f + tdx, -1.0f + tdy,	0.0f + dx, 0.0f + dy
	};
	vbo_inner_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad1), quad1);

	float quad2[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f
	};
	vbo_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad2), quad2);

	// Create the line that surrounds the quad
	float line[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f
	};
	vbo_outer_line.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line), line);
}

void update(Renderer &gfx, Context &ctx, float dt)
{
	// Enable the framebuffer of inputIndex as output and
	// the textures of outputIndex as input.
	fbo[outputIndex].bind();
	glViewport(0, 0, tex_res_x, tex_res_y);
	tex_velocity[inputIndex].bind(GL_TEXTURE0);
	tex_pressure[inputIndex].bind(GL_TEXTURE1);

	// Do interior computation
	vbo_inner_quad.bind();
	gfx.beginCustomShader(shader_interior);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_pressure", 1);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);

	// Do boundary computation
	vbo_outer_line.bind();
	gfx.beginCustomShader(shader_boundary);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_pressure", 1);
	gfx.drawVertexBuffer(GL_LINE_STRIP, 5);

	fbo[outputIndex].unbind();
	glViewport(0, 0, ctx.getWidth(), ctx.getHeight());

	inputIndex = outputIndex;
	outputIndex = (outputIndex + 1) % 2;
}

void render(Renderer &gfx, Context &ctx, float dt)
{
	// Visualize (using the newly updated textures as input)
	vbo_quad.bind();
	tex_velocity[inputIndex].bind(GL_TEXTURE0);
	gfx.beginCustomShader(shader_visualize);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setUniform("tex", 0);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
}