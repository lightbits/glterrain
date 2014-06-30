#include "app.h"
#include "sort.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram 
	shader_particle,
	shader_compute,
	shader_spawn,
	shader_sort;
BufferObject 
	position_buffer, 
	status_buffer,
	spawn_buffer,
	swap_buffer;
VertexArray 
	vao;
BufferObject 
	sprite_buffer;
mat4 
	mat_projection, 
	mat_view;
vec3
	emitter;
const int WORK_GROUP_SIZE = 32;
const int NUM_PARTICLES = 1 << 15;
const int NUM_GROUPS = NUM_PARTICLES / WORK_GROUP_SIZE;
const int NUM_STAGES = glm::round(glm::log2((float)NUM_PARTICLES));
const int NUM_PASSES = NUM_STAGES * (NUM_STAGES + 1) / 2;
const int GROUPS_PER_PASS = (NUM_PARTICLES / 2) / WORK_GROUP_SIZE;
const int SORT_PASSES_PER_FRAME = 8;

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
		!loadComputeShader(shader_spawn, "./demo/33curlnoise/spawn.cs") ||
		!loadComputeShader(shader_sort, "./demo/33curlnoise/sort.cs"))
		return false;

	if (!shader_particle.linkAndCheckStatus() ||
		!shader_compute.linkAndCheckStatus() ||
		!shader_spawn.linkAndCheckStatus() ||
		!shader_sort.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

/*
Initialize the particle positions and status.
*/
void initParticles(Renderer &gfx, Context &ctx)
{
	// Map the buffer to client memory to fill it with data
	position_buffer.bind();
	vec4 *position = (vec4*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER, 0,
		NUM_PARTICLES * sizeof(vec4),
		GL_MAP_WRITE_BIT |				// We wish to write to the buffer
		GL_MAP_INVALIDATE_BUFFER_BIT);	// And discard the previous content
	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		vec3 p = vec3(0.0);
		p.x = -1.0 + 2.0 * (i % (NUM_PARTICLES / 32)) / (NUM_PARTICLES / 32);
		p.y = -1.0 + 2.0 * (i / (NUM_PARTICLES / 8)) / 8.0;
		p.z = -1.0 + 2.0 * frand();
		position[i] = vec4(p, 1.0f);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// Status buffer contains velocity and lifetime
	status_buffer.bind();
	vec4 *status = (vec4*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER, 0,
		NUM_PARTICLES * sizeof(vec4),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		status[i] = vec4(0.0, 0.0, 0.0, 1.0);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void init(Renderer &gfx, Context &ctx)
{
	mat_projection = glm::perspective(PI / 3.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.3f, 15.0f);
	mat_view = mat4(1.0f);

	vao.create();
	vao.bind();

	swap_buffer = gen_swapindex_buffer(NUM_PARTICLES);

	float data[] = { 0.0f, 0.0f, 0.0f };
	sprite_buffer.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(data), data);

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	status_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	spawn_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);

	initParticles(gfx, ctx);
	
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	emitter = vec3(0.0, 0.0, 0.0);
}

/*
The sorting shader takes the position buffer as input, and sorts the buffer
in back-to-front order relative to the camera.
*/
int pass = 0;
void sort(Renderer &gfx, Context &ctx)
{
	gfx.beginCustomShader(shader_sort);
	gfx.setUniform("view", mat_view);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, swap_buffer.getHandle());
	int i = 0;
	while (pass < NUM_PASSES && i < SORT_PASSES_PER_FRAME)
	{
		gfx.setUniform("offset", pass * NUM_PARTICLES / 2);
		glDispatchCompute(GROUPS_PER_PASS, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		i++;
		pass++;
	}
	if (pass >= NUM_PASSES)
		pass = 0;
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, -0.0f, -3.0f) * rotateX(0.5 * sin(ctx.getElapsedTime() * 0.8f)) * rotateY(ctx.getElapsedTime() * 0.5f);
	emitter.x = 0.1 * sin(ctx.getElapsedTime());
	emitter.z = 0.1 * cos(ctx.getElapsedTime());
	emitter.y = 0.1 * 0.4 * sin(ctx.getElapsedTime() * 0.5);

	if (ctx.isKeyPressed('s'))
	{
		sort(gfx, ctx);
	}

	//// Generate respawn info
	//gfx.beginCustomShader(shader_spawn);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, spawn_buffer.getHandle());
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, position_buffer.getHandle());
	//glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//// Update particles
	//gfx.beginCustomShader(shader_compute);
	//gfx.setUniform("dt", dt);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, status_buffer.getHandle());
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spawn_buffer.getHandle());
	//glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.beginCustomShader(shader_particle);
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
	//glPointSize(15.0f);
	glDrawArraysInstancedBaseInstance(GL_POINTS, 0, 1, NUM_PARTICLES, 0);
}