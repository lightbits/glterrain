#include "app.h"
#include "sort.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram 
	shader_sort,
	shader_particle,
	shader_shadowmap,
	shader_texture;

BufferObject
	buffer_position,
	buffer_comparison;

MeshBuffer
	quad;

mat4
	mat_projection,
	mat_view,
	mat_view_light,
	mat_projection_light;

GLuint
	shadow_map_fbo,
	shadow_map_tex;

int shadow_map_width = 512;
int shadow_map_height = 512;
int shadow_map_depth = 32;

const int NUM_PARTICLES = 1 << 14;
const int LOCAL_SIZE = 128;
const int NUM_GROUPS = NUM_PARTICLES / LOCAL_SIZE;
const int NUM_STAGES = glm::round(glm::log2((float)NUM_PARTICLES));
const int NUM_PASSES = NUM_STAGES * (NUM_STAGES + 1) / 2;
const int GROUPS_PER_PASS = (NUM_PARTICLES / 2) / LOCAL_SIZE;

VertexArray vao;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!loadComputeShader(shader_sort, "./demo/38volshadow/sort.cs") ||
		!shader_shadowmap.loadFromFile("./demo/38volshadow/shadowmap") ||
		!shader_texture.loadFromFile("./demo/38volshadow/texture") ||
		!shader_particle.loadFromFile("./demo/38volshadow/particle"))
		return false;

	if (!shader_sort.linkAndCheckStatus() ||
		!shader_shadowmap.linkAndCheckStatus() ||
		!shader_texture.linkAndCheckStatus() ||
		!shader_particle.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

void initShadowmap()
{
	glGenTextures(1, &shadow_map_tex);
	glBindTexture(	GL_TEXTURE_2D_ARRAY, shadow_map_tex);
	glTexStorage3D(	GL_TEXTURE_2D_ARRAY, 1, GL_R8, shadow_map_width, shadow_map_height, shadow_map_depth);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenFramebuffers(1, &shadow_map_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadow_map_tex, 0, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "whoops!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_3D, 0);
}

void initParticles(Renderer &gfx, Context &ctx)
{
	buffer_position.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * 4 * sizeof(GLfloat), NULL);
	buffer_position.bind();
	vec4 *position = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 
		NUM_PARTICLES * sizeof(vec4), 
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		float z = 0.7f * (-1.0f + 2.0f * frand());
		float x = 0.7f * (-1.0f + 2.0f * frand());
		float y = 0.7f * (-1.0f + 2.0f * frand());
		vec3 p = vec3(x, y, z);
		p = 0.7f * frand() * glm::normalize(p);
		position[i] = vec4(p.x, p.y, p.z, 1.0f);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	buffer_position.unbind();
}

void sort(Renderer &gfx, Context &ctx)
{
	vec4 lp = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	lp = glm::inverse(mat_view_light) * lp;
	vec3 sort_axis = -glm::normalize(vec3(lp));
	gfx.beginCustomShader(shader_sort);
	gfx.setUniform("axis", sort_axis);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_position.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer_comparison.getHandle());
	for (int pass = 0; pass < NUM_PASSES; ++pass)
	{
		gfx.setUniform("offset", pass * NUM_PARTICLES / 2);
		glDispatchCompute(GROUPS_PER_PASS, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	initParticles(gfx, ctx);
	initShadowmap();
	buffer_comparison = gen_swapindex_buffer(NUM_PARTICLES);	
	quad = Mesh::genScreenSpaceTexQuad();
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_projection = glm::perspective(PI / 4.0f, float(ctx.getWidth()) / ctx.getHeight(), 0.1f, 10.0f);
	mat_view = translate(0.0f, 0.0f, -3.0f) * rotateX(0.0f) * rotateY(ctx.getElapsedTime() * 0.2f);

	mat_projection_light = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 1.0f, 3.0f);
	mat_view_light = translate(0.0f, 0.0f, -2.0f) * rotateX(-PI / 2.0f);

	sort(gfx, ctx);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	gfx.beginCustomShader(shader_shadowmap);
	gfx.setUniform("projection", mat_projection_light);
	gfx.setUniform("view", mat_view_light);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_position.getHandle());
	gfx.setAttributefv("position", 4, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);
	glViewport(0, 0, shadow_map_width, shadow_map_height);
	glClear(GL_COLOR_BUFFER_BIT);
	for (int i = 0; i < shadow_map_depth; ++i)
	{
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadow_map_tex, 0, i);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_POINTS, 0, (i + 1) * NUM_PARTICLES / shadow_map_depth);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_map_tex);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, ctx.getWidth(), ctx.getHeight());
	glClear(GL_COLOR_BUFFER_BIT);
	//glDisable(GL_BLEND);
	//gfx.beginCustomShader(shader_texture);
	//gfx.setUniform("tex", 0);
	//gfx.setUniform("layer", 31.0f * (0.5f + 0.5f * sin(ctx.getElapsedTime())));
	//quad.draw();

	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // back to front
	glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE); // front to back
	gfx.beginCustomShader(shader_particle);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projectionLight", mat_projection_light);
	gfx.setUniform("viewLight", mat_view_light);
	gfx.setUniform("shadowMap", 0);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_position.getHandle());
	gfx.setAttributefv("position", 4, 0, 0);
	for (int i = 0; i < shadow_map_depth; ++i)
	{
		gfx.setUniform("layer", float(i));
		glDrawArrays(GL_POINTS, i * NUM_PARTICLES / shadow_map_depth, NUM_PARTICLES / shadow_map_depth);
	}
}