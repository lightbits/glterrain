#include "app.h"

ShaderProgram 
	shader_scanblock,
	shader_scansums,
	shader_applysums;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!loadComputeShader(shader_scanblock, "./demo/37radixsort/scanblock.cs") ||
		!loadComputeShader(shader_scansums, "./demo/37radixsort/scansums.cs") ||
		!loadComputeShader(shader_applysums, "./demo/37radixsort/applysums.cs"))
		return false;

	if (!shader_scanblock.linkAndCheckStatus() ||
		!shader_scansums.linkAndCheckStatus() ||
		!shader_applysums.linkAndCheckStatus())
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

const uint32 num_keys = 1 << 3;
const uint32 block_size = 4;
const uint32 num_blocks = num_keys / block_size;
void scan_block(Renderer &gfx, Context &ctx, GLuint keys, GLuint scan, GLuint sums)
{
	printf("keys: ");
	print_data(keys, GL_SHADER_STORAGE_BUFFER, num_keys);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sums);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, keys);
	gfx.beginCustomShader(shader_scanblock);
	glDispatchCompute(num_blocks, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

	printf("scan: ");
	print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys);

	printf("sums: ");
	print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks);
}

// The sums array must fit inside a single thread block
void scan_sums(Renderer &gfx, Context &ctx, GLuint scan, GLuint sums)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sums);
	gfx.beginCustomShader(shader_scansums);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	printf("scan sums: ");
	print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks);
}

// The workgroup size must equal the block size
// We fire of num_blocks to cover the entire array
// The elements in SUMS are applied to each element in the respective blocks
void apply_sums(Renderer &gfx, Context &ctx, GLuint scan, GLuint sums)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sums);
	gfx.beginCustomShader(shader_applysums);
	glDispatchCompute(num_blocks, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	printf("final scan: ");
	print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys);
}

void init(Renderer &gfx, Context &ctx)
{
	BufferObject buf_key, buf_scan, buf_sums;

	uint32 *zeroes = new uint32[num_keys];
	uint32 *keys = new uint32[num_keys];

	memset(zeroes, 0, num_keys * sizeof(uint32));
	for (int i = 0; i < num_keys; ++i)
		keys[i] = rand() % 4;

	buf_key.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), keys);
	buf_scan.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), zeroes);
	buf_sums.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_blocks * sizeof(uint32), zeroes);

	// To measure time
	glFinish();
	GLuint64 start, stop;
	GLuint queryID[2];
	glGenQueries(2, queryID);
	glQueryCounter(queryID[0], GL_TIMESTAMP);

	scan_block(gfx, ctx, buf_key.getHandle(), buf_scan.getHandle(), buf_sums.getHandle());
	scan_sums(gfx, ctx, buf_scan.getHandle(), buf_sums.getHandle());
	apply_sums(gfx, ctx, buf_scan.getHandle(), buf_sums.getHandle());

	glQueryCounter(queryID[1], GL_TIMESTAMP);
	GLint timeAvail = 0;
	while (!timeAvail)
		glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &timeAvail);

	glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &start);
	glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stop);
	printf("Time spent on the GPU: %f ms\n", (stop - start) / 1000000.0);
}

void update(Renderer &gfx, Context &ctx, double dt)
{


}

void render(Renderer &gfx, Context &ctx, double dt)
{

}