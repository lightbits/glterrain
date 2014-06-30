#include "app.h"
#include "sort.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram 
	shader_sort, 
	shader_sprite;
VertexArray vao;
BufferObject 
	buffer_quad,
	buffer_pos,
	buffer_indices;
mat4
	mat_view;
const int LOCAL_SIZE = 32;
const int NUM_SPRITES = 1 << 15;
const int NUM_GROUPS = NUM_SPRITES / LOCAL_SIZE;
const int NUM_STAGES = glm::round(glm::log2((float)NUM_SPRITES));
const int NUM_PASSES = NUM_STAGES * (NUM_STAGES + 1) / 2;
const int GROUPS_PER_PASS = (NUM_SPRITES / 2) / LOCAL_SIZE;
const int SORT_PASSES_PER_FRAME = 8;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!loadComputeShader(shader_sort, "./demo/34sorting/sort.cs") ||
		!shader_sprite.loadFromFile("./demo/34sorting/sprite"))
		return false;

	if (!shader_sort.linkAndCheckStatus() ||
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
*/
void initParticles(Renderer &gfx, Context &ctx)
{
	buffer_pos.bind();
	vec4 *position = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 
		NUM_SPRITES * sizeof(vec4), 
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	//std::cout << "random: ";
	for (int i = 0; i < NUM_SPRITES; ++i)
	{
		float z = -1.0f + 2.0f * frand();
		position[i] = vec4(0.0, 0.0, z, 1.0);
		//std::cout << z << " ";
	}
	//std::cout << std::endl;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_pos.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer_indices.getHandle());
	int i = 0;
	while (pass < NUM_PASSES && i < SORT_PASSES_PER_FRAME)
	{
		gfx.setUniform("offset", pass * NUM_SPRITES / 2);
		glDispatchCompute(GROUPS_PER_PASS, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		i++;
		pass++;
	}
	if (pass >= NUM_PASSES)
		pass = 0;
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	buffer_pos.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(vec4), NULL);

	initParticles(gfx, ctx);

	// Generate a buffer containing the indices of elements to be compare-swap'ed
	buffer_indices = gen_swapindex_buffer(NUM_SPRITES);

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
	mat_view = translate(0.0f, 0.0f, -3.0f) * rotateX(-0.45f) * rotateY(ctx.getElapsedTime() * 0.5f);
	if (ctx.isKeyPressed('s'))
	{
		sort(gfx, ctx);

		//// Read back the values to console (for convenience)
		//buffer_pos.bind();
		//vec4 *result = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_SPRITES * sizeof(vec4), GL_MAP_READ_BIT);
		//std::cout << "\nsorted: ";
		//for (int i = 0; i < NUM_SPRITES; ++i)
		//	std::cout << result[i].z << " ";
		//std::cout << std::endl;
		//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.beginCustomShader(shader_sprite);
	gfx.setBlendState(BlendStates::AlphaBlend);
	gfx.setUniform("projection", glm::perspective(PI / 2.0f, 720.0f / 480.0f, 0.1f, 5.0f));
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", mat4(1.0f));
	buffer_quad.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("position", 3, 0, 0);
	buffer_pos.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("spritePosition", 4, 0, 0);
	gfx.setAttributeDivisor("spritePosition", 1);
	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 32, 0);
}