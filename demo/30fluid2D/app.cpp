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

Slab temperature;
RenderTexture rt_free;
const int GRID_SIZE = 256;

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

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	float *data = new float[GRID_SIZE * GRID_SIZE];
	for (int i = 0; i < GRID_SIZE * GRID_SIZE; ++i)
		data[i] = 0.0f;
	float dx = 1.0f / GRID_SIZE;
	for (int i = 0; i < GRID_SIZE; ++i)
	{
		float t = i * dx;
		data[i] = sin(PI * t);
		data[i + (GRID_SIZE - 1) * GRID_SIZE] = sin(PI * t) * exp(-PI);
	}

	Texture2D temp0, temp1;
	temp0.create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RED, GL_FLOAT, data);
	temp0.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	temp1.create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RED, GL_FLOAT, data);
	temp1.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	temperature.ping.create(temp0);
	temperature.pong.create(temp1);

	rt_free.create(GRID_SIZE, GRID_SIZE);

	delete[] data;

	float quad_data[] = {
		-1.0f + 2.0f * dx, -1.0f + 2.0f * dx, 0.0f + dx, 0.0f + dx,
		 1.0f - 2.0f * dx, -1.0f + 2.0f * dx, 1.0f - dx, 0.0f + dx,
		 1.0f - 2.0f * dx,  1.0f - 2.0f * dx, 1.0f - dx, 1.0f - dx,
		 1.0f - 2.0f * dx,  1.0f - 2.0f * dx, 1.0f - dx, 1.0f - dx,
		-1.0f + 2.0f * dx,  1.0f - 2.0f * dx, 0.0f + dx, 1.0f - dx,
		-1.0f + 2.0f * dx, -1.0f + 2.0f * dx, 0.0f + dx, 0.0f + dx
	};

	vbo_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad_data), quad_data);
}

void jacobi(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture, // Output texture
	RenderTexture &sourceTexture,  // B texture
	float alpha, float beta, // Iteration parameters
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	inputTexture.bindTexture(GL_TEXTURE0);
	sourceTexture.bindTexture(GL_TEXTURE1);
	gfx.beginCustomShader(shader_jacobi);
	gfx.setUniform("tex_input", 0);
	gfx.setUniform("tex_source", 1);
	gfx.setUniform("alpha", alpha);
	gfx.setUniform("beta", beta);
	gfx.setUniform("dx", 1.0f / GRID_SIZE);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();
}

void update(Renderer &gfx, Context &ctx, float dt)
{
	glViewport(0, 0, GRID_SIZE, GRID_SIZE);
	jacobi(temperature.ping, temperature.pong, rt_free, 0.0f, 4.0f, gfx, ctx, dt);
	temperature.ping.swap(temperature.pong);
}

void render(Renderer &gfx, Context &ctx, float dt)
{
	glViewport(0, 0, ctx.getWidth(), ctx.getHeight());
	vbo_quad.bind();
	temperature.pong.bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_texture);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setUniform("tex", 0);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
}