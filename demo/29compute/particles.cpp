#include "app.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram 
	shader_render,
	shader_compute;
VertexArray vao;
MeshBuffer cube_buffer;
MeshBuffer sphere_buffer;
Mesh cube_mesh;
Mesh sphere_mesh;

const int NUM_PARTICLES = 50000;
const int WORK_GROUP_SIZE = 16;
BufferObject position_buffer;
BufferObject velocity_buffer;

mat4
	mat_projection,
	mat_view;

bool load()
{
	GLenum types[] = { GL_COMPUTE_SHADER };
	string paths[] = { "./demo/29compute/particles.cs" };
	if (!shader_render.loadFromFile("./demo/29compute/render") ||
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
	mat_projection = glm::perspective(PI / 3.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.1f, 25.0f);
	mat_view = mat4(1.0f);

	vao.create();
	vao.bind();

	sphere_mesh = Mesh::genUnitSphere(32, 32);
	sphere_buffer = MeshBuffer(sphere_mesh);
	cube_mesh = Mesh::genUnitCube(false, false, true);
	cube_buffer = MeshBuffer(cube_mesh);

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	velocity_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);

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
		position[i] = vec4(0.0f, 1.5f, 0.0f, 1.0f);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	velocity_buffer.bind();
	vec4 *velocity = (vec4*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		NUM_PARTICLES * sizeof(vec4),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		velocity[i] = vec4(2.0f * frand() - 1.0f, 2.0f * frand() - 1.0f, 2.0f * frand() - 1.0f, 0.0f);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.5f, -0.3f, -2.0f) * rotateX(-0.75f) * rotateY(ctx.getElapsedTime() * 0.4f);

	gfx.beginCustomShader(shader_compute);
	gfx.setUniform("dt", dt);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocity_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.beginCustomShader(shader_render);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullClockwise);
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", scale(0.005f));

	cube_buffer.bind();
	gfx.setAttributefv("position", 3, 0, 0);

	position_buffer.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("particlePosition", 4, 0, 0);
	gfx.setAttributeDivisor("particlePosition", 1);
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, cube_mesh.getIndexCount(), GL_UNSIGNED_INT, 0, NUM_PARTICLES, 0);

	gfx.setUniform("model", scale(1.0f));
	gfx.unsetAttribute("particlePosition");
	sphere_buffer.bind();
	gfx.setAttributefv("position", 3, 0, 0);
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, sphere_mesh.getIndexCount(), GL_UNSIGNED_INT, 0, 1, 0);
}