#include "app.h"
#include "sort.h"
#include <common/noise.h>
using namespace transform;
const int WORK_GROUP_SIZE = 128;
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
	shader_indexkeyfill,
	shader_plane,
	shader_sphere,
	shader_background;
BufferObject 
	position_buffer,
	index_buffer,
	status_buffer,
	spawn_buffer,
	swap_buffer,
	key_buffer;
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
		!loadComputeShader(shader_indexkeyfill, "./demo/33curlnoise/indexkeyfill.cs") ||
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
		!shader_indexkeyfill.linkAndCheckStatus() ||
		!shader_sort.linkAndCheckStatus() ||
		!shader_plane.linkAndCheckStatus() ||
		!shader_sphere.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	index_buffer.dispose();
	key_buffer.dispose();
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
		p.x = (-1.0 + 2.0 * frand());
		p.y = (-1.0 + 2.0 * frand());
		p.z = (-1.0 + 2.0 * frand());
		p = 0.3f * frand() * glm::normalize(p);
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
	particle_lifetime = 1.0f;
	light_pos = vec3(0.45, 0.45, 0.45);
	light_col = vec3(0.7, 0.2, 0.1);
	ambient_col = vec3(0.85, 0.95, 1.0) * 0.02f;

	vao.create();
	vao.bind();

	position_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	status_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	spawn_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(vec4), NULL);
	index_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(unsigned int), NULL);
	key_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(float), NULL);
	swap_buffer = gen_swapindex_buffer(NUM_PARTICLES);

	initParticles(gfx, ctx);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

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
bool front_to_back = false;
vec3 axis;
void sort(Renderer &gfx, Context &ctx)
{
	// Compute light axis
	vec4 l = vec4(0.0, 0.0, 0.0, 1.0);
	l = glm::inverse(mat_light) * l;
	vec3 light = -glm::normalize(vec3(l));

	// Compute view axis
	vec4 v = vec4(0.0, 0.0, 0.0, 1.0);
	v = glm::inverse(mat_view) * v;
	vec3 view = -glm::normalize(vec3(v));

	// We set the sorting axis as the half-angle vector between
	// the light- and view-axis. The following calculations are 
	// approximately correct.
	if (glm::dot(light, view) < 0.0)
	{
		front_to_back = false;
		axis = glm::normalize(light - view);
	}
	else
	{
		front_to_back = true;
		axis = glm::normalize(light + view);
	}

	// Fill index and key buffers
	gfx.beginCustomShader(shader_indexkeyfill);
	gfx.setUniform("axis", axis);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, index_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, key_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	gfx.beginCustomShader(shader_sort);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, index_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, key_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, swap_buffer.getHandle());
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

float sphere_v = 0.0f;
void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, +0.2f, -2.5f) * rotateX(-0.7f) * rotateY(PI / 4.0f);
	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		float s = -1.0f + 2.0f * ctx.getMouseX() / ctx.getWidth();
		s *= PI / 2.0f;
		mat_light = translate(0.0f, 0.0f, -2.5f) * rotateX(s + -PI / 2.0f) * rotateY(PI / 2.0f);
	}

	float t = ctx.getElapsedTime() * 0.5f;
	emitter_pos.x = 0.8f * sin(t * 1.2f);
	emitter_pos.z = 0.8f * cos(t * 0.7f);
	emitter_pos.y = 0.8f * sin(t * 2.0f) * 0.2f;
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
	gfx.setUniform("emitterPos", emitter_pos);
	gfx.setUniform("spherePos", sphere_pos);
	gfx.setUniform("particleLifetime", particle_lifetime);
	gfx.setUniform("time", ctx.getElapsedTime());
	gfx.setUniform("dt", 0.0167f);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spawn_buffer.getHandle());
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	sort(gfx, ctx);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	mat4 projection_light = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	gfx.clear(0x00000000, 1.0);

	rt_shadowmap.begin();
	gfx.clear(0x00000000, 1.0);
	rt_shadowmap.end();

	glDepthMask(GL_FALSE);

	const int BATCH_SIZE = 256;
	const int NUM_BATCHES = NUM_PARTICLES / BATCH_SIZE;
	position_buffer.bind(GL_ARRAY_BUFFER);
	index_buffer.bind(GL_ELEMENT_ARRAY_BUFFER);
	for (int i = 0; i < NUM_BATCHES; ++i)
	{
		// Render particles using shadow information
		rt_shadowmap.bindTexture();
		gfx.beginCustomShader(shader_draw_particle);
		if (front_to_back)
			gfx.setBlendState(BlendState(true, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD));
		else
			gfx.setBlendState(BlendState(true, GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD));
			
		gfx.setUniform("projection", mat_projection);
		gfx.setUniform("view", mat_view);
		gfx.setUniform("particleLifetime", particle_lifetime);
		gfx.setUniform("projectionLight", projection_light);
		gfx.setUniform("viewLight", mat_light);
		gfx.setUniform("shadowmap", 0);
		gfx.setAttributefv("position", 4, 0, 0);
		glDrawElements(GL_POINTS, BATCH_SIZE, GL_UNSIGNED_INT, (GLvoid*)(i * BATCH_SIZE * sizeof(unsigned int)));
		rt_shadowmap.unbindTexture();

		// Render shadow from light's point of view
		rt_shadowmap.begin();
		glViewport(0, 0, shadowmap_res.x, shadowmap_res.y);
		gfx.beginCustomShader(shader_shadowmap);
		gfx.setBlendState(BlendStates::Additive);
		gfx.setUniform("projectionLight", projection_light);
		gfx.setUniform("viewLight", mat_light);
		gfx.setAttributefv("position", 4, 0, 0);
		glDrawElements(GL_POINTS, BATCH_SIZE, GL_UNSIGNED_INT, (GLvoid*)(i * BATCH_SIZE * sizeof(unsigned int)));
		glViewport(0, 0, ctx.getWidth(), ctx.getHeight());
		rt_shadowmap.end();
	}
}