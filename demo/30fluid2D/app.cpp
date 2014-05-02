#include "app.h"

VertexArray vao;
ShaderProgram 
	shader_interior,
	shader_boundary,
	shader_visualize;

/* Note that we need two textures for each field.
One is needed to use as input while the other is used as output.
After a computation we perform a handle swap between the two,
so that the latest output can be used as the next input. */
Framebuffer fbo[2];
Texture2D 
	tex_velocity[2],
	tex_pressure[2];

int tex_res_x = 16;
int tex_res_y = 16;

int inputIndex = 0;
int outputIndex = 1;

/* These primitives define the area of computation for the
interior algorithm and the boundary algorithm. */
BufferObject 
	vbo_inner_quad,
	vbo_outer_line,
	vbo_quad;

bool load()
{
	if (!shader_interior.loadFromFile(  "./demo/30fluid2D/quadsample.vs", "./demo/30fluid2D/interior.fs") ||
		!shader_boundary.loadFromFile(  "./demo/30fluid2D/quadsample.vs", "./demo/30fluid2D/boundary.fs") ||
		!shader_visualize.loadFromFile( "./demo/30fluid2D/quadsample.vs", "./demo/30fluid2D/showtex.fs"))
		return false;

	shader_interior.bindFragDataLocation("out_velocity", 0);
	shader_interior.bindFragDataLocation("out_pressure", 1);

	shader_boundary.bindFragDataLocation("out_velocity", 0);
	shader_boundary.bindFragDataLocation("out_pressure", 1);

	if (!shader_interior.linkAndCheckStatus() ||
		!shader_boundary.linkAndCheckStatus() ||
		!shader_visualize.linkAndCheckStatus())
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

void update(Renderer &gfx, Context &ctx, double dt)
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

void render(Renderer &gfx, Context &ctx, double dt)
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