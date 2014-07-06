/*
Alignment is weird.
But basically, if you don't specify a storage qualifier (such as std140),
the alignment will be implementation specific.
If you use std140, the rules are 

1. If the member is a scalar consuming N basic machine units, the base align-
ment is N .
2. If the member is a two- or four-component vector with components consum-
ing N basic machine units, the base alignment is 2N or 4N , respectively.
3. If the member is a three-component vector with components consuming N
basic machine units, the base alignment is 4N .
4. If the member is an array of scalars or vectors, the base alignment and array
stride are set to match the base alignment of a single array element, according
to rules (1), (2), and (3), and rounded up to the base alignment of a vec4. The
array may have padding at the end; the base offset of the member following
the array is rounded up to the next multiple of the base alignment.

For example (std140)
	Consider a SSBO consisting of an array of uints.
	Because a single uint is 4 bytes, rule 1 says the alignment will be 4 bytes.
	But by rule 4, since the member is an array of a scalar, the
	alignment and stride is rounded up to the base alignment of a vec4, 
	which is 16 bytes!

	This means that if you write to Data[0] in the shader, you will write
	to element 0 in the data buffer. But if you write to Data[1] in the shader,
	you write to element 4 in the data buffer!

	This can give rise to hard to find bugs!

If you use std430, alignment and stride is NOT rounded up to match that of a vec4.
*/
#include "app.h"
#include "sort.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram 
	shader_sort,
	shader_randomize,
	shader_keys,
	shader_sprite;
VertexArray vao;
BufferObject 
	buffer_quad,
	buffer_pos,
	buffer_cmp,
	buffer_indices_read,
	buffer_indices_write,
	buffer_keys;
mat4
	mat_view;
const int LOCAL_SIZE = 16;
const int NUM_SPRITES = 1 << 16;
const int NUM_GROUPS = NUM_SPRITES / LOCAL_SIZE;
const int NUM_STAGES = glm::round(glm::log2((float)NUM_SPRITES));
const int NUM_PASSES = NUM_STAGES * (NUM_STAGES + 1) / 2;
const int GROUPS_PER_PASS = (NUM_SPRITES / 2) / LOCAL_SIZE;
const int SORT_PASSES_PER_FRAME = NUM_PASSES;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!loadComputeShader(shader_sort, "./demo/35bettersorting/sort.cs") ||
		!loadComputeShader(shader_randomize, "./demo/35bettersorting/randomize.cs") ||
		!loadComputeShader(shader_keys, "./demo/35bettersorting/keys.cs") ||
		!shader_sprite.loadFromFile("./demo/35bettersorting/sprite"))
		return false;

	if (!shader_sort.linkAndCheckStatus() ||
		!shader_randomize.linkAndCheckStatus() ||
		!shader_keys.linkAndCheckStatus() ||
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
		position[i] = vec4(-1.0f + 2.0f * i / NUM_SPRITES, 0.0, z, 1.0);
		//std::cout << z << " ";
	}
	//std::cout << std::endl;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	buffer_pos.unbind();
}

/*
The sorting shader takes the position buffer as input, and sorts the buffer
in back-to-front order relative to the camera.
*/
int pass = 0;
void sort(Renderer &gfx, Context &ctx)
{
	gfx.beginCustomShader(shader_keys);
	gfx.setUniform("view", mat_view);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_pos.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer_indices_write.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_keys.getHandle());
	glDispatchCompute(NUM_SPRITES / LOCAL_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

	gfx.beginCustomShader(shader_sort);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_indices_write.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer_keys.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_cmp.getHandle());
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	buffer_pos.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * 4 * sizeof(GLfloat),	NULL);
	buffer_indices_write.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(GLuint), NULL);
	buffer_indices_read.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(GLuint),	NULL);
	buffer_keys.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(GLfloat), NULL);

	initParticles(gfx, ctx);

	// Generate a buffer containing the indices of elements to be compared in the sorting algorithm
	buffer_cmp = gen_swapindex_buffer(NUM_SPRITES);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, 0.0f, -3.0f) * rotateX(-0.45f) * rotateY(ctx.getElapsedTime() * 0.01f);
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
	if (ctx.isKeyPressed('r'))
	{
		gfx.beginCustomShader(shader_randomize);
		gfx.setUniform("time", ctx.getElapsedTime());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_pos.getHandle());
		glDispatchCompute(NUM_SPRITES / LOCAL_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
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
	buffer_pos.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("position", 4, 0, 0);
	//glDrawArrays(GL_POINTS, 0, 4);
	buffer_indices_read.bind(GL_ELEMENT_ARRAY_BUFFER);
	glDrawElements(GL_POINTS, 512, GL_UNSIGNED_INT, 0);

	buffer_indices_read.swap(buffer_indices_write);
}