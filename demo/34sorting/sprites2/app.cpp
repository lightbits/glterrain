#include "app.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram shader_sort, shader_sprite, shader_writeback;
VertexArray vao;
BufferObject 
	buffer_quad,
	buffer_keys,
	buffer_pos, 
	buffer_pos_sorted;
const int LOCAL_SIZE = 1;
const int NUM_SPRITES = 8;
const int NUM_GROUPS = NUM_SPRITES / LOCAL_SIZE;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!loadComputeShader(shader_sort, "./demo/34sorting/oddeven.cs") ||
		!loadComputeShader(shader_writeback, "./demo/34sorting/writeback.cs") ||
		!shader_sprite.loadFromFile("./demo/34sorting/sprite"))
		return false;

	if (!shader_sort.linkAndCheckStatus() ||
		!shader_writeback.linkAndCheckStatus() ||
		!shader_sprite.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

/*
Generate a row of particles with random z-depth, ranging from -1 to +1.
In addition we create an associated key-buffer, which contains ivec2's of
(particle_index_in_position_buffer, particle_depth_along_axis), which is
used when sorting.
*/
void initParticles(Renderer &gfx, Context &ctx)
{
	buffer_pos.bind();
	vec4 *position = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 
		NUM_SPRITES * sizeof(vec4), 
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < NUM_SPRITES; ++i)
	{
		float z = -1.0 + 2.0 * frand();
		position[i] = vec4(0.0, 0.0, z, 1.0);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	buffer_keys.bind();
	vec2i *keys = (vec2i*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
		NUM_SPRITES * sizeof(vec2i),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	std::cout << "random: ";
	//int seq[NUM_SPRITES] = { 27, 49, 84, 91, 17, 32, 53, 63 };
	for (int i = 0; i < NUM_SPRITES; ++i)
	{
		float z = position[i].z;
		int key = int(65535 * (z + 1.0) / (1.0 + 1.0));
		keys[i] = vec2i(i, key);
		//keys[i] = vec2i(i, seq[i]);
		std::cout << keys[i].y << " ";
	}
	std::cout << std::endl;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

/*
The sorting shader takes as input an index-key buffer, where
	index = a particle's index in the position buffer
	key = that particle's z-depth (converted to an integer)
and outputs the buffer sorted by the key.
*/
void sort(Renderer &gfx, Context &ctx)
{
	gfx.beginCustomShader(shader_sort);
	for (int i = 0; i < NUM_SPRITES; ++i)
	{
		gfx.setUniform("offset", i % 2);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_keys.getHandle());
		glDispatchCompute(NUM_SPRITES / 2, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

/*
buffer_pos contains the particle positions, in unsorted order.
buffer_keys contains the indices of the particles, sorted by the particle's depth.
To get a sorted position buffer, to use with instanced rendering, we copy the position
data over to a new buffer, but in such a way that consecutive position data is sorted by depth.
*/
void writebackPosition(Renderer &gfx, Context &ctx)
{
	gfx.beginCustomShader(shader_writeback);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_pos.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer_keys.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_pos_sorted.getHandle());
	glDispatchCompute(NUM_SPRITES, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

//vector<int> merge(vector<int> a)
//{
//	if (a.size() > 2)
//	{
//		
//	}
//	vector<int> b;
//
//}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	buffer_pos.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(vec4), NULL);
	buffer_keys.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(vec2i), NULL);
	buffer_pos_sorted.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(vec4), NULL);

	initParticles(gfx, ctx);
	sort(gfx, ctx);
	writebackPosition(gfx, ctx);

	// Read back the values to console (for convenience)
	buffer_keys.bind();
	vec2i *result = (vec2i*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_SPRITES * sizeof(vec2i), GL_MAP_READ_BIT);
	std::cout << "\nsorted: ";
	for (int i = 0; i < NUM_SPRITES; ++i)
		std::cout << result[i].y << " ";
	std::cout << std::endl;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	float quad[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};
	buffer_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad), quad);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.beginCustomShader(shader_sprite);
	//gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::AlphaBlend);
	gfx.setUniform("projection", glm::perspective(PI / 2.0f, 720.0f / 480.0f, 0.1f, 5.0f));
	gfx.setUniform("view", translate(0.0f, 0.0f, -3.0f) * rotateX(-0.45f) * rotateY(sin(ctx.getElapsedTime())));
	gfx.setUniform("model", mat4(1.0f));
	buffer_quad.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("position", 3, 0, 0);

	if (ctx.isKeyPressed('s'))
		buffer_pos_sorted.bind(GL_ARRAY_BUFFER);
	else
		buffer_pos.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("spritePosition", 4, 0, 0);
	gfx.setAttributeDivisor("spritePosition", 1);
	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, NUM_SPRITES, 0);
}