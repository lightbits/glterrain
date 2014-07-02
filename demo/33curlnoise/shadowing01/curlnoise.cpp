#include "app.h"
#include "sort.h"
#include <common/noise.h>
using namespace transform;
const int WORK_GROUP_SIZE = 64;
const int NUM_PARTICLES = 1 << 15;
const int NUM_GROUPS = NUM_PARTICLES / WORK_GROUP_SIZE;
const int NUM_STAGES = glm::round(glm::log2((float)NUM_PARTICLES));
const int NUM_PASSES = NUM_STAGES * (NUM_STAGES + 1) / 2;
const int GROUPS_PER_PASS = (NUM_PARTICLES / 2) / WORK_GROUP_SIZE;
const int SORT_PASSES_PER_FRAME = 12;

ShaderProgram 
	shader_update_particle,
	shader_spawn_particle,
	shader_draw_particle,
	shader_sort,
	shader_plane,
	shader_sphere,
	shader_background;
BufferObject 
	position_buffer, 
	status_buffer,
	spawn_buffer,
	swap_buffer;
MeshBuffer
	quad,
	plane,
	sphere,
	cube;
VertexArray 
	vao;
BufferObject 
	sprite_buffer;
mat4 
	mat_projection, 
	mat_view,
	mat_light;
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
		!shader_background.loadFromFile("./demo/33curlnoise/background") ||
		!loadComputeShader(shader_update_particle, "./demo/33curlnoise/updateparticle.cs") ||
		!loadComputeShader(shader_spawn_particle, "./demo/33curlnoise/spawnparticle.cs") ||
		!loadComputeShader(shader_sort, "./demo/33curlnoise/sort.cs"))
		return false;

	if (!shader_update_particle.linkAndCheckStatus() ||
		!shader_spawn_particle.linkAndCheckStatus() ||
		!shader_draw_particle.linkAndCheckStatus() ||
		!shader_background.linkAndCheckStatus() ||
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
		p.x = 0.5 * (-1.0 + 2.0 * frand());
		p.y = 0.5 * (-1.0 + 2.0 * frand());
		p.z = 0.5 * (-1.0 + 2.0 * frand());
		//p += emitter_pos;
		float lifetime = (1.0 + 0.25 * frand()) * particle_lifetime;
		position[i] = vec4(p, lifetime);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void init(Renderer &gfx, Context &ctx)
{
	mat_projection = glm::perspective(PI / 4.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.3f, 15.0f);
	mat_view = mat4(1.0f);
	mat_light = translate(0.0f, 0.0f, -1.0f) * rotateY(PI / 2.0f);
	emitter_pos = vec3(0.0, 0.0, 0.0);
	sphere_pos = vec3(0.0, -0.5, 0.0);
	sphere_radius = 0.2f;
	particle_lifetime = 2.5f;
	light_pos = vec3(0.45, 0.45, 0.45);
	light_col = vec3(0.7, 0.2, 0.1);
	ambient_col = vec3(0.85, 0.95, 1.0) * 0.02f;

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

	Mesh cubemesh;
	cubemesh.setDrawMode(GL_LINES);
	float vertices[] = {
		-1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
	};
	uint32 indices2[] = {
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};
	cubemesh.addPositions((vec3*)vertices, 24);
	cubemesh.addIndices(indices2, 24);
	cube = MeshBuffer(cubemesh);

	Mesh quadmesh;
	quadmesh.addPosition(-1.0f, -1.0f, 0.0f);
	quadmesh.addPosition(+1.0f, -1.0f, 0.0f);
	quadmesh.addPosition(+1.0f, +1.0f, 0.0f);
	quadmesh.addPosition(-1.0f, +1.0f, 0.0f);
	quadmesh.addTriangle(0, 1, 2);
	quadmesh.addTriangle(2, 3, 0);
	quad = MeshBuffer(quadmesh);
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

// Cast a ray from the point of view onto the ground plane
vec3 raycast(int x, int y, int w, int h)
{
	float xndc = -1.0f + 2.0f * x / w;
	float yndc = 1.0f - 2.0f * y / h;

	vec4 view = glm::inverse(mat_projection) * vec4(xndc, yndc, 1.0f, 1.0f);

	vec3 origin = vec3(glm::inverse(mat_view) * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	vec3 dir = glm::normalize(vec3(glm::inverse(mat_view) * view));
	float t = -origin.y / dir.y;
	return origin + t * dir;
}

float sphere_v = 0.0f;
void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, +0.2f, -2.5f) * rotateX(-0.6f) * rotateY(0.5f);
	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		vec3 p = raycast(ctx.getMouseX(), ctx.getMouseY(), ctx.getWidth(), ctx.getHeight());
		p.x = clamp(p.x, -2.0f + sphere_radius, 2.0f - sphere_radius);
		p.z = clamp(p.z, -2.0f + sphere_radius, 2.0f - sphere_radius);
		sphere_pos.x = p.x;
		sphere_pos.z = p.z;
		sphere_pos.y = p.y;
	}

	float t = ctx.getElapsedTime();
	emitter_pos.x = 0.8f * sin(t * 1.2f);
	emitter_pos.z = 0.8f * cos(t * 0.7f);
	emitter_pos.y = 0.8f * sin(t * 2.0f) * 0.2f;
	//light_pos = emitter_pos;
	light_pos = vec3(1.0, 0.5, 0.0);

	//// Generate respawn info
	//gfx.beginCustomShader(shader_spawn_particle);
	//gfx.setUniform("time", ctx.getElapsedTime());
	//gfx.setUniform("emitterPos", emitter_pos);
	//gfx.setUniform("particleLifetime", particle_lifetime);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, spawn_buffer.getHandle());
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, position_buffer.getHandle());
	//glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//// Update particles
	//gfx.beginCustomShader(shader_update_particle);
	//gfx.setUniform("seed", vec3(13.0, 59.0, 449.0));
	////gfx.setUniform("seed", vec3(7.0, 113.0, 811.0));
	//gfx.setUniform("spherePos", sphere_pos);
	////gfx.setUniform("sphereRadius", sphere_radius);
	//gfx.setUniform("particleLifetime", particle_lifetime);
	//gfx.setUniform("time", ctx.getElapsedTime());
	//gfx.setUniform("dt", dt);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spawn_buffer.getHandle());
	//glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if (ctx.isKeyPressed('s'))
		sort(gfx, ctx);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x000000ff, 1.0);
	gfx.setBlendState(BlendStates::Opaque);
	gfx.setCullState(CullStates::CullClockwise);
	//gfx.beginCustomShader(shader_background);
	//gfx.setUniform("view", mat_view);
	//quad.draw();

	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	//gfx.beginCustomShader(shader_plane);
	//gfx.setUniform("projection", mat_projection);
	//gfx.setUniform("view", mat_view);
	//gfx.setUniform("lightPos", light_pos);
	//gfx.setUniform("lightColor", light_col);
	//gfx.setUniform("ambientColor", ambient_col);

	//// Floor
	//gfx.setUniform("color", vec3(0.76f, 0.75f, 0.5f) * 0.5f);
	//gfx.setUniform("model", translate(0.0, -1.0, 0.0) * scale(2.0f));
	//plane.draw();

	//// Left wall
	//gfx.setUniform("color", vec3(0.63f, 0.06f, 0.04f));
	//gfx.setUniform("model", translate(-2.0, -0.5, 0.0) * rotateZ(-PI / 2.0) * scale(0.5, 1.0, 2.0));
	//plane.draw();

	//// Right wall
	//gfx.setUniform("color", vec3(0.63f, 0.06f, 0.04f));
	//gfx.setUniform("model", translate(2.0, -0.5, 0.0) * rotateZ(PI / 2.0) * scale(0.5, 1.0, 2.0));
	//plane.draw();

	//// Back wall
	//gfx.setUniform("color", vec3(0.63f, 0.06f, 0.04f));
	//gfx.setUniform("model", translate(0.0, -0.5, -2.0) * rotateX(-PI / 2.0) * scale(2.0, 1.0, 0.5));
	//plane.draw();

	//// Front wall
	//gfx.setUniform("color", vec3(0.63f, 0.06f, 0.04f));
	//gfx.setUniform("model", translate(0.0, -0.5, +2.0) * rotateX(PI / 2.0) * scale(2.0, 1.0, 0.5));
	//plane.draw();

	//gfx.setUniform("color", vec3(1.0f));
	//gfx.setUniform("model", translate(sphere_pos) * scale(sphere_radius));
	//cube.draw();

	gfx.beginCustomShader(shader_sphere);
	gfx.setCullState(CullStates::CullNone);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", glm::inverse(mat_light) * scale(0.1f));
	gfx.setUniform("color", vec3(0.2, 0.35, 1.0));
	gfx.setUniform("lightPos", light_pos);
	gfx.setUniform("lightColor", light_col);
	gfx.setUniform("ambientColor", ambient_col);
	sphere.draw();

	const int BATCH_SIZE = 64;
	const int NUM_BATCHES = NUM_PARTICLES / BATCH_SIZE;
	for (int i = 0; i < NUM_BATCHES; ++i)
	{
		// Render particles using shadow information
		gfx.beginCustomShader(shader_draw_particle);
		gfx.setBlendState(BlendStates::AlphaBlend);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glDepthRange(0.0, 1.0);
		gfx.setUniform("particleLifetime", particle_lifetime);
		gfx.setUniform("projection", mat_projection);
		gfx.setUniform("view", mat_view);
		position_buffer.bind(GL_ARRAY_BUFFER);
		gfx.setAttributefv("position", 4, 0, 0);
		glDrawArrays(GL_POINTS, i * BATCH_SIZE, BATCH_SIZE);

		// Render shadow from light's point of view
		gfx.beginCustomShader(shader_draw_particle);
		gfx.setBlendState(BlendStates::AlphaBlend);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glDepthRange(0.0, 1.0);
		gfx.setUniform("particleLifetime", particle_lifetime);
		gfx.setUniform("projection", mat_projection);
		gfx.setUniform("view", mat_view);
		position_buffer.bind(GL_ARRAY_BUFFER);
		gfx.setAttributefv("position", 4, 0, 0);
		glDrawArrays(GL_POINTS, i * BATCH_SIZE, BATCH_SIZE);
	}
}