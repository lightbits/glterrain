#include "app.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram shader_particle, shader_color, shader_compute, shader_sphere;
BufferObject position_buffer, status_buffer;
VertexArray vao;
BufferObject sprite_buffer;
MeshBuffer grid, sphere;
mat4 mat_projection, mat_view;
const int NUM_PARTICLES = 40000;
const int WORK_GROUP_SIZE = 16;
vec3 attractor;

bool load()
{
	GLenum types[] = { GL_COMPUTE_SHADER };
	string paths[] = { "./demo/33curlnoise/curlnoise.cs" };
	if (!shader_particle.loadFromFile("./demo/33curlnoise/particle") ||
		!shader_color.loadFromFile("./demo/33curlnoise/simple") ||
		!shader_sphere.loadFromFile("./demo/33curlnoise/sphere") ||
		!shader_compute.loadFromFile(paths, types, 1))
		return false;

	if (!shader_particle.linkAndCheckStatus() ||
		!shader_color.linkAndCheckStatus() ||
		!shader_sphere.linkAndCheckStatus() ||
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

	grid = MeshBuffer(Mesh::genUnitGrid(Color::fromHex(0x999999ff), 8));
	sphere = MeshBuffer(Mesh::genUnitSphere(32, 32));

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	status_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);

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
		//float a = (float)i / (NUM_PARTICLES - 1);
		//float x = sin(a * 32 * PI) * 0.2f;
		//float z = cos(a * 32 * PI) * 0.2f;
		//float y = (float)(i / (NUM_PARTICLES / 16)) * 0.05f + frand() * 0.05f;
		//x -= frand() * 0.15f;
		//z -= frand() * 0.15f;
		//position[i] = vec4(x, y, z, 1.0f);
		float x = (float)(i % 128);
		float y = (float)(i / 128);
		position[i] = vec4(3.0 + 2.0 * x / 128.0, y / 128.0, 0.0, 1.0);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Status buffer contains velocity and lifetime
	status_buffer.bind();
	vec4 *status = (vec4*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		NUM_PARTICLES * sizeof(vec4),
		GL_MAP_WRITE_BIT | // We wish to write to the buffer
		GL_MAP_INVALIDATE_BUFFER_BIT); // And discard the previous content

	for (int i = 0; i < NUM_PARTICLES; ++i)
		status[i] = vec4(0.0, 0.0, 0.0, 10.0);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	attractor = vec3(0.0, 0.5, 0.0);
}

float sink = 0.0f;
void update(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isKeyPressed('s'))
	{
		if (sink > -5.0f) 
			sink -= 5.0f * dt;
	}
	else
	{
		sink = 4.0f;
	}

	mat_view = translate(0.0f, -0.8f, -4.0f) * rotateX(-0.2f) * rotateY(0.2f);
	gfx.beginCustomShader(shader_compute);
	//gfx.setUniform("sink", sink);
	gfx.setUniform("attractor", attractor);
	//gfx.setUniform("seed", vec3(11.0, 127.0, 3583.0));
	//gfx.setUniform("time", ctx.getElapsedTime());
	gfx.setUniform("dt", dt);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, status_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::Default);
	gfx.setCullState(CullStates::CullCounterClockwise); // CullClockwise for cool effect!
	gfx.clear(0x2a2a2aff, 1.0);

	gfx.beginCustomShader(shader_color);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", scale(2.0f));
	grid.draw();

	gfx.beginCustomShader(shader_sphere);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", translate(attractor) * scale(1.0f));
	gfx.setUniform("sinkSourceBlend", (sink + 5.0f) / (4.0f + 5.0f));
	sphere.draw();

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
	//status_buffer.bind(GL_ARRAY_BUFFER);
	//gfx.setAttributefv("particleStatus", 4, 0, 0);
	//gfx.setAttributeDivisor("particleStatus", 1);
	glDrawArraysInstancedBaseInstance(GL_POINTS, 0, 1, NUM_PARTICLES, 0);
}