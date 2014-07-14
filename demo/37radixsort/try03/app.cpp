#include "app.h"

ShaderProgram 
	shader_prefix,
	shader_reorder;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!loadComputeShader(shader_prefix, "./demo/37radixsort/prefix.cs") ||
		!loadComputeShader(shader_reorder, "./demo/37radixsort/reorder.cs"))
		return false;

	if (!shader_prefix.linkAndCheckStatus() ||
		!shader_reorder.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{

}

void print_data(GLuint buffer, GLenum target, int size)
{
	glBindBuffer(target, buffer);
	uint32 *data = (uint32*)glMapBufferRange(target, 0, size * sizeof(uint32), GL_MAP_READ_BIT);
	for (int i = 0; i < size; ++i)
		printf("%d ", data[i]);
	printf("\n");
	glUnmapBuffer(target);
	glBindBuffer(target, 0);
}

const uint32 num_keys = 8;
const uint32 num_tiles = 2;
const uint32 tile_size = num_keys / num_tiles;
void scan_tiles(Renderer &gfx, Context &ctx, GLuint keys, GLuint output)
{
	printf("keys: ");
	print_data(keys, GL_SHADER_STORAGE_BUFFER, num_keys);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, keys);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, output);
	gfx.beginCustomShader(shader_prefix);
	glDispatchCompute(num_tiles, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	printf("scan: ");
	print_data(output, GL_SHADER_STORAGE_BUFFER, num_keys);
}

void init(Renderer &gfx, Context &ctx)
{
	BufferObject keys_buffer, prefix_buffer, sorted_buffer, flag_buffer;

	uint32 *zeroes = new uint32[num_keys];
	uint32 *keys = new uint32[num_keys];

	memset(zeroes, 0, num_keys * sizeof(uint32));
	for (int i = 0; i < num_keys; ++i)
		keys[i] = rand() % 4;

	keys_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), keys);
	prefix_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), zeroes);

	scan_tiles(gfx, ctx, keys_buffer.getHandle(), prefix_buffer.getHandle());
}

void update(Renderer &gfx, Context &ctx, double dt)
{


}

void render(Renderer &gfx, Context &ctx, double dt)
{

}