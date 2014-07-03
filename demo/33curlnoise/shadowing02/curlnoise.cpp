#include "app.h"
#include "sort.h"
#include <common/noise.h>
using namespace transform;
const int WORK_GROUP_SIZE = 64;
const int NUM_PARTICLES = 1 << 14;
const int NUM_GROUPS = NUM_PARTICLES / WORK_GROUP_SIZE;
const int NUM_STAGES = glm::round(glm::log2((float)NUM_PARTICLES));
const int NUM_PASSES = NUM_STAGES * (NUM_STAGES + 1) / 2;
const int GROUPS_PER_PASS = (NUM_PARTICLES / 2) / WORK_GROUP_SIZE;
const int SORT_PASSES_PER_FRAME = 105;

ShaderProgram 
	shader_update_particle,
	shader_spawn_particle,
	shader_draw_particle,
	shader_shadowmap,
	shader_drawmap,
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

vec2i 
	shadowmap_res;

RenderTexture
	rt_shadowmap;

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
		!shader_shadowmap.loadFromFile("./demo/33curlnoise/shadowmap") ||
		!shader_drawmap.loadFromFile("./demo/33curlnoise/drawmap") ||
		!loadComputeShader(shader_update_particle, "./demo/33curlnoise/updateparticle.cs") ||
		!loadComputeShader(shader_spawn_particle, "./demo/33curlnoise/spawnparticle.cs") ||
		!loadComputeShader(shader_sort, "./demo/33curlnoise/sort.cs"))
		return false;

	if (!shader_update_particle.linkAndCheckStatus() ||
		!shader_spawn_particle.linkAndCheckStatus() ||
		!shader_draw_particle.linkAndCheckStatus() ||
		!shader_background.linkAndCheckStatus() ||
		!shader_shadowmap.linkAndCheckStatus() ||
		!shader_drawmap.linkAndCheckStatus() ||
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
	vec4 *data = new vec4[NUM_PARTICLES];
	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		vec3 p;
		p.x = 0.5 * (-1.0 + 2.0 * (i % (NUM_PARTICLES / 32)) / (NUM_PARTICLES / 32));
		p.y = 0.5 * (-1.0 + 2.0 * (i / (NUM_PARTICLES / 2)) / 2.0);
		p.z = 0.5 * (-1.0 + 2.0 * frand());
		//p.x = 0.3 * (-1.0 + 2.0 * frand());
		//p.y = 0.3 * (-1.0 + 2.0 * frand());
		//p.z = 0.3 * (-1.0 + 2.0 * frand());
		//p = 0.3f * frand() * glm::normalize(p);
		//p += emitter_pos;
		float lifetime = (1.0 + 0.25 * frand()) * particle_lifetime;
		data[i] = vec4(p, lifetime);
	}
	position_buffer.bind();
	position_buffer.bufferData(NUM_PARTICLES * sizeof(vec4), data);
}

void init(Renderer &gfx, Context &ctx)
{
	mat_projection = glm::perspective(PI / 4.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.3f, 15.0f);
	mat_view = mat4(1.0f);
	mat_light = translate(0.0f, 0.0f, -1.5f) * rotateX(-0.8f) * rotateY(PI / 2.0f);
	emitter_pos = vec3(0.0, 0.0, 0.0);
	sphere_pos = vec3(0.0, -0.5, 0.0);
	sphere_radius = 0.2f;
	particle_lifetime = 1.5f;
	light_pos = vec3(0.45, 0.45, 0.45);
	light_col = vec3(0.7, 0.2, 0.1);
	ambient_col = vec3(0.85, 0.95, 1.0) * 0.02f;

	vao.create();
	vao.bind();

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	status_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	spawn_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	swap_buffer = gen_swapindex_buffer(NUM_PARTICLES);

	initParticles(gfx, ctx);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

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
	quadmesh.addPosition(-1.0f, -1.0f, 0.0f); quadmesh.addTexel(0.0f, 0.0f);
	quadmesh.addPosition(+1.0f, -1.0f, 0.0f); quadmesh.addTexel(1.0f, 0.0f);
	quadmesh.addPosition(+1.0f, +1.0f, 0.0f); quadmesh.addTexel(1.0f, 1.0f);
	quadmesh.addPosition(-1.0f, +1.0f, 0.0f); quadmesh.addTexel(0.0f, 1.0f);
	quadmesh.addTriangle(0, 1, 2);
	quadmesh.addTriangle(2, 3, 0);
	quad = MeshBuffer(quadmesh);

	shadowmap_res = vec2i(512, 512);
	rt_shadowmap.create(shadowmap_res.x, shadowmap_res.y);
	rt_shadowmap.getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
}

/*
The sorting shader takes the position buffer as input, and sorts the buffer
in back-to-front order relative to the camera. We can perform the sorting over 
multiple frames. <SORT_PASSES_PER_FRAME> can be lowered to increase performance.
*/
int pass = 0;
void sort(Renderer &gfx, Context &ctx)
{
	// compute sorting axis
	vec4 p = vec4(0.0, 0.0, 0.0, 1.0);
	p = glm::inverse(mat_light) * p;
	vec3 axis = glm::normalize(vec3(0.0) - vec3(p));

	gfx.beginCustomShader(shader_sort);
	gfx.setUniform("axis", axis);
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
	mat_view = translate(0.0f, +0.2f, -2.5f) * rotateX(-0.7f) * rotateY(PI / 4.0f);
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
	mat_light = translate(0.0f, 0.0f, -3.5f) * rotateX(-PI / 2.0f) * rotateY(PI / 2.0f);
	light_pos = vec3((glm::inverse(mat_light) * vec4(0.0, 0.0, 0.0, 1.0)));

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
	gfx.setUniform("seed", vec3(13.0, 59.0, 449.0));
	//gfx.setUniform("seed", vec3(7.0, 113.0, 811.0));
	gfx.setUniform("spherePos", sphere_pos);
	//gfx.setUniform("sphereRadius", sphere_radius);
	gfx.setUniform("particleLifetime", particle_lifetime);
	gfx.setUniform("time", ctx.getElapsedTime());
	gfx.setUniform("dt", dt);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spawn_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if (ctx.isKeyPressed('s'))
		sort(gfx, ctx);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x00000000, 1.0);

	rt_shadowmap.begin();
	gfx.clear(0x00000000, 1.0);
	rt_shadowmap.end();

	mat4 projection_light = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	const int BATCH_SIZE = 64;
	const int NUM_BATCHES = NUM_PARTICLES / BATCH_SIZE;
	glPointSize(7.0f);
	for (int i = 0; i < NUM_BATCHES; ++i)
	{
		// Render particles using shadow information
		rt_shadowmap.bindTexture();
		gfx.beginCustomShader(shader_draw_particle);
		gfx.setBlendState(BlendState(true, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD));
		gfx.setUniform("projection", mat_projection);
		gfx.setUniform("view", mat_view);
		gfx.setUniform("projectionLight", projection_light);
		gfx.setUniform("viewLight", mat_light);
		gfx.setUniform("shadowmap", 0);
		position_buffer.bind(GL_ARRAY_BUFFER);
		gfx.setAttributefv("position", 4, 0, 0);
		glDrawArrays(GL_POINTS, i * BATCH_SIZE, BATCH_SIZE);
		rt_shadowmap.unbindTexture();

		// Render shadow from light's point of view
		rt_shadowmap.begin();
		glViewport(0, 0, shadowmap_res.x, shadowmap_res.y);
		gfx.beginCustomShader(shader_shadowmap);
		gfx.setBlendState(BlendStates::Additive);
		gfx.setUniform("projectionLight", projection_light);
		gfx.setUniform("viewLight", mat_light);
		position_buffer.bind(GL_ARRAY_BUFFER);
		gfx.setAttributefv("position", 4, 0, 0);
		glDrawArrays(GL_POINTS, i * BATCH_SIZE, BATCH_SIZE);
		glViewport(0, 0, ctx.getWidth(), ctx.getHeight());
		rt_shadowmap.end();
	}

	gfx.setBlendState(BlendState(true, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD));
	rt_shadowmap.bindTexture();
	gfx.setCullState(CullStates::CullClockwise);
	//gfx.setBlendState(BlendStates::Opaque);
	//gfx.beginCustomShader(shader_drawmap);
	//gfx.setUniform("tex", 0);
	//quad.draw();
	gfx.beginCustomShader(shader_plane);
	gfx.setUniform("shadowmap", 0);
	gfx.setUniform("projectionLight", projection_light);
	gfx.setUniform("viewLight", mat_light);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("lightPos", light_pos);
	gfx.setUniform("lightColor", light_col);
	gfx.setUniform("ambientColor", ambient_col);

	// Floor
	gfx.setUniform("color", vec3(0.76f, 0.75f, 0.5f) * 0.5f);
	gfx.setUniform("model", translate(0.0, -1.0, 0.0) * scale(2.0f));
	plane.draw();
}