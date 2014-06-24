#include "app.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram 
	shader_particle,
	shader_compute,
	shader_spawn;
BufferObject 
	position_buffer, 
	status_buffer,
	spawn_buffer;
VertexArray 
	vao;
BufferObject 
	sprite_buffer;
mat4 
	mat_projection, 
	mat_view;
vec3
	emitter;
const int NUM_PARTICLES = 16 * 2500;
const int WORK_GROUP_SIZE = 16;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!shader_particle.loadFromFile("./demo/33curlnoise/particle") ||
		!loadComputeShader(shader_compute, "./demo/33curlnoise/curlnoise.cs") ||
		!loadComputeShader(shader_spawn, "./demo/33curlnoise/spawn.cs"))
		return false;

	if (!shader_particle.linkAndCheckStatus() ||
		!shader_compute.linkAndCheckStatus() ||
		!shader_spawn.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	mat_projection = glm::perspective(PI / 3.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.3f, 15.0f);
	mat_view = mat4(1.0f);

	vao.create();
	vao.bind();

	float data[] = { 0.0f, 0.0f, 0.0f };
	sprite_buffer.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(data), data);

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	status_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	spawn_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);

	// Map the buffer to client memory to fill it with data
	position_buffer.bind();
	vec4 *position = (vec4*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		NUM_PARTICLES * sizeof(vec4),
		GL_MAP_WRITE_BIT | // We wish to write to the buffer
		GL_MAP_INVALIDATE_BUFFER_BIT); // And discard the previous content

	int n = NUM_PARTICLES - 1;
	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		vec3 p = 0.5f * frand() * glm::normalize(vec3(-1.0f) + 2.0f * vec3(frand(), frand(), frand()));
		position[i] = vec4(p.x, p.y, p.z, 1.0f);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// Status buffer contains velocity and lifetime
	status_buffer.bind();
	vec4 *status = (vec4*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		NUM_PARTICLES * sizeof(vec4),
		GL_MAP_WRITE_BIT | // We wish to write to the buffer
		GL_MAP_INVALIDATE_BUFFER_BIT); // And discard the previous content

	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		status[i] = vec4(0.0, 0.0, 0.0, 1.0 + 4.0 * i / NUM_PARTICLES);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	emitter = vec3(0.0, 0.0, 0.0);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, -0.0f, -3.0f) * rotateX(-0.35f) * rotateY(sin(PI / 2.0f));
	emitter.x = sin(ctx.getElapsedTime());
	emitter.z = cos(ctx.getElapsedTime());
	emitter.y = 0.4 * sin(ctx.getElapsedTime() * 0.5);

	// Generate respawn info
	gfx.beginCustomShader(shader_spawn);
	gfx.setUniform("emitter", emitter);
	gfx.setUniform("time", ctx.getElapsedTime());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, spawn_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Update particles
	gfx.beginCustomShader(shader_compute);
	gfx.setUniform("dt", dt);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, status_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spawn_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.beginCustomShader(shader_particle);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::AlphaBlend);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", mat4(1.0f));
	sprite_buffer.bind();
	gfx.setAttributefv("position", 3, 0, 0);
	position_buffer.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("particlePosition", 4, 0, 0);
	gfx.setAttributeDivisor("particlePosition", 1);
	status_buffer.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("particleStatus", 4, 0, 0);
	gfx.setAttributeDivisor("particleStatus", 1);
	glDrawArraysInstancedBaseInstance(GL_POINTS, 0, 1, NUM_PARTICLES, 0);
}