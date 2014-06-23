#include "app.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram shader_particle, shader_color, shader_compute, shader_sphere;
BufferObject position_buffer;
VertexArray vao;
BufferObject sprite_buffer;
MeshBuffer grid, plane, sphere;
mat4 mat_projection, mat_view;
const int NUM_PARTICLES = 16 * 2500;
const int WORK_GROUP_SIZE = 16;

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
	mat_projection = glm::perspective(PI / 3.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.3f, 15.0f);
	mat_view = mat4(1.0f);

	vao.create();
	vao.bind();

	float data[] = { 0.0f, 0.0f, 0.0f };
	sprite_buffer.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(data), data);

	grid = MeshBuffer(Mesh::genUnitGrid(Color::fromHex(0x000000ff), 16));
	plane = MeshBuffer(Mesh::genUnitColoredPlane(Color(0.44f, 0.40f, 0.38f)));

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);

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
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, -0.0f, -3.0f) * rotateX(-0.35f) * rotateY(sin(PI / 2.0f));
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
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::Default);
	gfx.clear(0x2a2a2aff, 1.0);

	gfx.beginCustomShader(shader_color);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view * translate(0.0f, -1.0f, 0.0f));
	gfx.setUniform("model", scale(8.0f));
	plane.draw();
	glLineWidth(2.0f);
	gfx.setUniform("model", scale(4.0f));
	grid.draw();

	gfx.beginCustomShader(shader_particle);
	//gfx.setDepthTestState(DepthTestStates::Disabled);
	gfx.setBlendState(BlendStates::AlphaBlend);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", mat4(1.0f));
	sprite_buffer.bind();
	gfx.setAttributefv("position", 3, 0, 0);
	position_buffer.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("particlePosition", 4, 0, 0);
	gfx.setAttributeDivisor("particlePosition", 1);
	glDrawArraysInstancedBaseInstance(GL_POINTS, 0, 1, NUM_PARTICLES, 0);
}