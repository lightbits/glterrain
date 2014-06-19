#include "app.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram shader_render, shader_compute;
BufferObject position_buffer;
VertexArray vao;
BufferObject sprite_buffer;
mat4 mat_projection, mat_view;
const int NUM_PARTICLES = 40000;
const int WORK_GROUP_SIZE = 16;

bool load()
{
	GLenum types[] = { GL_COMPUTE_SHADER };
	string paths[] = { "./demo/33curlnoise/curlnoise.cs" };
	if (!shader_render.loadFromFile("./demo/33curlnoise/rendercube") ||
		!shader_compute.loadFromFile(paths, types, 1))
		return false;

	if (!shader_render.linkAndCheckStatus() ||
		!shader_compute.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	mat_projection = glm::perspective(PI / 3.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.3f, 8.0f);
	mat_view = mat4(1.0f);

	vao.create();
	vao.bind();

	float data[] = { 0.0f, 0.0f, 0.0f };
	sprite_buffer.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(data), data);

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);

	// Map the buffer to client memory to fill it with data
	position_buffer.bind();
	vec4 *position = (vec4*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		NUM_PARTICLES * sizeof(vec4),
		GL_MAP_WRITE_BIT | // We wish to write to the buffer
		GL_MAP_INVALIDATE_BUFFER_BIT); // And discard the previous content

	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		float a = (float)i / (NUM_PARTICLES - 1);
		float x = sin(a * 32 * PI) * 0.2f;
		float z = cos(a * 32 * PI) * 0.2f;
		float y = (float)(i / (NUM_PARTICLES / 16)) * 0.05f + frand() * 0.05f;
		x -= frand() * 0.15f;
		z -= frand() * 0.15f;
		position[i] = vec4(x, y, z, 1.0f);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, -0.8f, -3.0f) * rotateX(-0.55f) * rotateY(sin(ctx.getElapsedTime() * 0.4f));
	gfx.beginCustomShader(shader_compute);
	gfx.setUniform("seed", vec3(11.0, 127.0, 3583.0));
	gfx.setUniform("time", ctx.getElapsedTime());
	gfx.setUniform("dt", dt);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.beginCustomShader(shader_render);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullClockwise);
	gfx.clear(0x2a2a2aff, 1.0);

	sprite_buffer.bind();
	gfx.setAttributefv("position", 3, 0, 0);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", mat4(1.0f));
	position_buffer.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("particlePosition", 4, 0, 0);
	gfx.setAttributeDivisor("particlePosition", 1);
	glPointSize(2.0f);
	glDrawArraysInstancedBaseInstance(GL_POINTS, 0, 1, NUM_PARTICLES, 0);
}