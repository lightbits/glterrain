#include "app.h"
#include "sort.h"
#include <common/noise.h>
using namespace transform;
const int WORK_GROUP_SIZE = 64;
const int NUM_PARTICLES = 1 << 13;
const int NUM_GROUPS = NUM_PARTICLES / WORK_GROUP_SIZE;
const int NUM_STAGES = glm::round(glm::log2((float)NUM_PARTICLES));
const int NUM_PASSES = NUM_STAGES * (NUM_STAGES + 1) / 2;
const int GROUPS_PER_PASS = (NUM_PARTICLES / 2) / WORK_GROUP_SIZE;
const int SORT_PASSES_PER_FRAME = 91;

ShaderProgram 
	shader_update_particle,
	shader_spawn_particle,
	shader_draw_particle,
	shader_sort,
	shader_plane,
	shader_sphere;
BufferObject 
	position_buffer, 
	status_buffer,
	spawn_buffer,
	swap_buffer;
MeshBuffer
	plane,
	sphere;
VertexArray 
	vao;
BufferObject 
	sprite_buffer;
mat4 
	mat_projection, 
	mat_view;
float
	sphere_radius,
	particle_lifetime;
vec3
	emitter_pos,
	sphere_pos,
	light_pos,
	light_col,
	ambient_col;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!shader_draw_particle.loadFromFile("./demo/33curlnoise/drawparticle") ||
		!shader_plane.loadFromFile("./demo/33curlnoise/plane") ||
		!shader_sphere.loadFromFile("./demo/33curlnoise/sphere") ||
		!loadComputeShader(shader_update_particle, "./demo/33curlnoise/updateparticle.cs") ||
		!loadComputeShader(shader_spawn_particle, "./demo/33curlnoise/spawnparticle.cs") ||
		!loadComputeShader(shader_sort, "./demo/33curlnoise/sort.cs"))
		return false;

	if (!shader_update_particle.linkAndCheckStatus() ||
		!shader_spawn_particle.linkAndCheckStatus() ||
		!shader_draw_particle.linkAndCheckStatus() ||
		!shader_sort.linkAndCheckStatus() ||
		!shader_plane.linkAndCheckStatus() ||
		!shader_sphere.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

/*
Initialize the particle positions and lifetime.
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
		vec3 p;
		p.x = -1.0 + 2.0 * frand();
		p.y = -1.0 + 2.0 * frand();
		p.z = -1.0 + 2.0 * frand();
		p = 0.4f * glm::normalize(p);
		p += emitter_pos;
		
		//p.x += 0.85 * (-1.0 + 2.0 * frand());
		//p.y += 0.1 * (-1.0 + 2.0 * frand());
		//p.z += 0.85 * (-1.0 + 2.0 * frand());
		float lifetime = (1.0 + 0.25 * frand()) * particle_lifetime;
		position[i] = vec4(p, lifetime);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void init(Renderer &gfx, Context &ctx)
{
	mat_projection = glm::perspective(PI / 4.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.3f, 15.0f);
	mat_view = mat4(1.0f);
	emitter_pos = vec3(0.0, 0.0, 0.0);
	sphere_pos = vec3(0.0, 0.5, 0.0);
	sphere_radius = 0.25f;
	particle_lifetime = 3.0f;
	light_pos = vec3(0.8, 2.0, 0.0);
	light_col = vec3(0.7, 0.65, 0.59);
	ambient_col = vec3(0.02, 0.03, 0.05);

	vao.create();
	vao.bind();

	float data[] = { 0.0f, 0.0f, 0.0f };
	sprite_buffer.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(data), data);

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	status_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	spawn_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	swap_buffer = gen_swapindex_buffer(NUM_PARTICLES);

	initParticles(gfx, ctx);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	Mesh planemesh;
	planemesh.addPosition(-1.0f, 0.0f, +1.0f); planemesh.addNormal(0.0f, 1.0f, 0.0f);
	planemesh.addPosition(+1.0f, 0.0f, +1.0f); planemesh.addNormal(0.0f, 1.0f, 0.0f);
	planemesh.addPosition(+1.0f, 0.0f, -1.0f); planemesh.addNormal(0.0f, 1.0f, 0.0f);
	planemesh.addPosition(-1.0f, 0.0f, -1.0f); planemesh.addNormal(0.0f, 1.0f, 0.0f);
	uint32 indices[] = { 0, 1, 2, 2, 3, 0 };
	planemesh.addIndices(indices, 6);
	plane = MeshBuffer(planemesh);
	sphere = MeshBuffer(Mesh::genUnitSphere(24, 24));
}

/*
The sorting shader takes the position buffer as input, and sorts the buffer
in back-to-front order relative to the camera. We can perform the sorting over 
multiple frames. <SORT_PASSES_PER_FRAME> can be lowered to increase performance.
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
	mat_view = translate(0.0f, +0.2f, -3.0f) * rotateX(-0.6f) * rotateY(ctx.getElapsedTime() * 0.1f);

	emitter_pos.x = sin(ctx.getElapsedTime() * 0.5f);
	emitter_pos.z = cos(ctx.getElapsedTime() * 0.75f);
	emitter_pos.y = 0.1f + sin(ctx.getElapsedTime() * 1.0f) * 0.2f;

	// Generate respawn info
	gfx.beginCustomShader(shader_spawn_particle);
	gfx.setUniform("time", ctx.getElapsedTime());
	gfx.setUniform("emitterPos", emitter_pos);
	gfx.setUniform("particleLifetime", particle_lifetime);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, spawn_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, position_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Update particles
	gfx.beginCustomShader(shader_update_particle);
	gfx.setUniform("seed", vec3(13.0, 127.0, 4243.0));
	gfx.setUniform("spherePos", sphere_pos);
	//gfx.setUniform("sphereRadius", sphere_radius);
	gfx.setUniform("particleLifetime", particle_lifetime);
	gfx.setUniform("time", ctx.getElapsedTime());
	gfx.setUniform("dt", dt);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spawn_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	sort(gfx, ctx);
	if (ctx.isKeyPressed('s'))
	{
	}
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	glDepthMask(GL_TRUE);
	gfx.clear(0x2a2a2aff, 1.0);

	gfx.beginCustomShader(shader_plane);
	gfx.setBlendState(BlendStates::Opaque);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("lightPos", light_pos);
	gfx.setUniform("lightColor", light_col);
	gfx.setUniform("ambientColor", ambient_col);

	//// Left wall
	//gfx.setUniform("color", vec3(0.63f, 0.06f, 0.04f));
	//gfx.setUniform("model", translate(-1.0, 0.0, 0.0) * rotateZ(-PI / 2.0) * scale(0.5, 1.0, 1.0));
	//plane.draw();

	// Floor
	gfx.setUniform("color", vec3(0.55, 0.53, 0.5));
	gfx.setUniform("model", translate(0.0, -0.5, 0.0) * scale(1.0f));
	plane.draw();

	//// Right wall
	//gfx.setUniform("color", vec3(0.15f, 0.48f, 0.09f));
	//gfx.setUniform("model", translate(1.0, 0.0, 0.0) * rotateZ(PI / 2.0) * scale(0.5, 1.0, 1.0));
	//plane.draw();

	//// Back wall
	//gfx.setUniform("color", vec3(0.76f, 0.75f, 0.5f));
	//gfx.setUniform("model", translate(0.0, 0.0, -1.0) * rotateX(-PI / 2.0) * scale(1.0, 1.0, 0.5));
	//plane.draw();
	
	//gfx.beginCustomShader(shader_sphere);
	//gfx.setUniform("projection", mat_projection);
	//gfx.setUniform("view", mat_view);
	//gfx.setUniform("model", translate(sphere_pos) * scale(sphere_radius));
	//gfx.setUniform("color", vec3(1.0, 0.55, 0.4));
	//gfx.setUniform("lightPos", light_pos);
	//gfx.setUniform("lightColor", light_col);
	//gfx.setUniform("ambientColor", ambient_col);
	//sphere.draw();

	gfx.beginCustomShader(shader_draw_particle);
	gfx.setBlendState(BlendStates::AlphaBlend);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	gfx.setUniform("particleLifetime", particle_lifetime);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	sprite_buffer.bind();
	gfx.setAttributefv("position", 3, 0, 0);
	position_buffer.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("particlePosition", 4, 0, 0);
	gfx.setAttributeDivisor("particlePosition", 1);
	glDrawArraysInstancedBaseInstance(GL_POINTS, 0, 1, NUM_PARTICLES, 0);
}