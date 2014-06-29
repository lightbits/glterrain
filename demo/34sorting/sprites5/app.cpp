#include "app.h"
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
	if (!loadComputeShader(shader_sort, "./demo/34sorting/oddevenmerge.cs") ||
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
	std::cout << "random: ";
	for (int i = 0; i < NUM_SPRITES; ++i)
	{
		float z = -1.0f + 2.0f * frand();
		position[i] = vec4(0.0, 0.0, z, 1.0);
		std::cout << z << " ";
	}
	std::cout << std::endl;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

/*
The sorting shader takes the position buffer as input, and sorts the buffer
in order of increasing z-value. The z-value is converted, in shader, to an integer by
key = int((z - zMin) / (zMax - zMin)).
*/
void sort(Renderer &gfx, Context &ctx)
{
	int num_stages = int(glm::log2((float)NUM_SPRITES));
	int num_passes = num_stages * (num_stages + 1) / 2;
	gfx.beginCustomShader(shader_sort);
	for (int i = 0; i < num_passes; ++i)
	{
		gfx.setUniform("offset", i % 2);
		gfx.setUniform("zMin", -1.0f);
		gfx.setUniform("zMax", 1.0f);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_pos.getHandle());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer_indices.getHandle());
		glDispatchCompute(NUM_SPRITES / 2, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

void merge(int lo, int hi, int r, int n, int stage, int index, vector< vector<vec2i> > &indices)
{
	int step = 2 * r;
	int buffer_index = stage * (stage + 1) / 2 + index;
	if (n > 2)
	{
		merge(lo, hi, step, n / 2, stage, index - 1, indices);
		merge(lo + r, hi, step, n / 2, stage, index - 1, indices);
		for (int i = lo + r; i < hi - r; i += step)
		{
			indices[buffer_index].push_back(vec2i(i, i + r));
		}
	}
	else
	{
		indices[buffer_index].push_back(vec2i(lo, lo + r));
	}
}

void mergesort(int lo, int hi, int stage, vector< vector<vec2i> > &indices)
{
	if (hi - lo > 1)
	{
		int mid = lo + (hi - lo) / 2;
		mergesort(lo, mid, stage - 1, indices);
		mergesort(mid, hi, stage - 1, indices);
		merge(lo, hi, 1, hi - lo, stage, stage, indices);
	}
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	buffer_pos.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(vec4), NULL);

	// Generate a buffer containing the indices of elements to be compare-swap'ed
	int num_stages = int(glm::log2((float)NUM_SPRITES));
	int num_passes = num_stages * (num_stages + 1) / 2;
	vector< vector<vec2i> > indices(num_passes);
	mergesort(0, NUM_SPRITES, num_stages - 1, indices);

	// This is more indices than we actually end up with
	// But we do some padding to take use of larger local group size
	int num_indices = num_passes * (NUM_SPRITES / 2);

	vec2i *index_buffer = new vec2i[num_indices];
	vec2i *index = index_buffer;
	for (int i = 0; i < num_passes; ++i)
	{
		int j = 0;
		while (j < indices[i].size())
		{
			*(index++) = indices[i][j];
			j++;
		}

		while (j < NUM_SPRITES / 2)
		{
			*(index++) = vec2i(0, 0);
			j++;
		}	
	}
	buffer_indices.create(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW, num_indices * sizeof(vec2i), index_buffer);

	initParticles(gfx, ctx);
	sort(gfx, ctx);

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

bool sorted = false;
void update(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isKeyPressed('s') && !sorted)
	{
		sorted = true;
		sort(gfx, ctx);

		// Read back the values to console (for convenience)
		buffer_pos.bind();
		vec4 *result = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_SPRITES * sizeof(vec4), GL_MAP_READ_BIT);
		std::cout << "\nsorted: ";
		for (int i = 0; i < NUM_SPRITES; ++i)
			std::cout << result[i].z << " ";
		std::cout << std::endl;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
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
	buffer_pos.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("spritePosition", 4, 0, 0);
	gfx.setAttributeDivisor("spritePosition", 1);
	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, NUM_SPRITES, 0);
}