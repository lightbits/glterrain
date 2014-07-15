#include "app.h"

ShaderProgram 
	shader_scanblock,
	shader_scansums,
	shader_applysums,
	shader_reorder;

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
		!loadComputeShader(shader_applysums, "./demo/37radixsort/applysums.cs") ||
		!loadComputeShader(shader_reorder, "./demo/37radixsort/reorder.cs"))
		return false;

	if (!shader_scanblock.linkAndCheckStatus() ||
		!shader_scansums.linkAndCheckStatus() ||
		!shader_applysums.linkAndCheckStatus() ||
		!shader_reorder.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{

}

void print_data(GLuint buffer, GLenum target, int size, int offset = 0, int stride = 1)
{
	glBindBuffer(target, buffer);
	uint32 *data = (uint32*)glMapBufferRange(target, 0, size * sizeof(uint32), GL_MAP_READ_BIT);
	for (int i = offset; i < size; i += stride)
		printf("%d ", data[i]);
	printf("\n");
	glUnmapBuffer(target);
	glBindBuffer(target, 0);
}

const uint32 num_keys = 1 << 8;
const uint32 radix = 4;
const uint32 block_size = 4; // Remember to update local_size in shader
const uint32 num_blocks = num_keys / block_size;
void scan_block(Renderer &gfx, Context &ctx, int bit_offset, GLuint keys, GLuint scan, GLuint sums, GLuint flag)
{
	//printf("keys: ");
	//print_data(keys, GL_SHADER_STORAGE_BUFFER, num_keys);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sums);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, keys);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, flag);
	gfx.beginCustomShader(shader_scanblock);
	gfx.setUniform("bitOffset", bit_offset);
	glDispatchCompute(num_blocks, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);

	//printf("scan0: "); print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys * 4, 0, 4);
	//printf("scan1: "); print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys * 4, 1, 4);
	//printf("scan2: "); print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys * 4, 2, 4);
	//printf("scan3: "); print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys * 4, 3, 4);

	//printf("sums0: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 0, 4);
	//printf("sums1: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 1, 4);
	//printf("sums2: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 2, 4);
	//printf("sums3: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 3, 4);
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

	//printf("scan sums0: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 0, 4);
	//printf("scan sums1: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 1, 4);
	//printf("scan sums2: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 2, 4);
	//printf("scan sums3: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 3, 4);
}

// The workgroup size must equal the block size
// We fire of num_blocks to cover the entire array
// The elements in SUMS are applied to each element in the respective blocks
void apply_sums(Renderer &gfx, Context &ctx, int bit_offset, GLuint keys, GLuint scan, GLuint sums)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sums);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, keys);
	gfx.beginCustomShader(shader_applysums);
	gfx.setUniform("bitOffset", bit_offset);
	glDispatchCompute(num_blocks, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

	//printf("final scan0: "); print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys * 4, 0, 4);
	//printf("final scan1: "); print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys * 4, 1, 4);
	//printf("final scan2: "); print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys * 4, 2, 4);
	//printf("final scan3: "); print_data(scan, GL_SHADER_STORAGE_BUFFER, num_keys * 4, 3, 4);
}

// Finally the elements are shuffled based on the prefix sum values
void reorder(Renderer &gfx, Context &ctx, GLuint keys, GLuint scan, GLuint flag, GLuint sorted)
{
	const uint32 work_group_size = 4;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, keys);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, flag);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, sorted);
	gfx.beginCustomShader(shader_reorder);
	glDispatchCompute(num_keys / work_group_size, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);

	//printf("sorted: ");
	//print_data(sorted, GL_SHADER_STORAGE_BUFFER, num_keys);
}

void init(Renderer &gfx, Context &ctx)
{
	BufferObject buf_key, buf_scan, buf_sums, buf_flag, buf_sorted;

	uint32 *zeroes = new uint32[4 * num_keys];
	memset(zeroes, 0, 4 * num_keys * sizeof(uint32));

	uint32 *keys = new uint32[num_keys];
	for (int i = 0; i < num_keys; ++i)
		keys[i] = (rand() % 65536);

	buf_key.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), keys);
	buf_scan.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(vec4u), zeroes);
	buf_sums.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_blocks * sizeof(vec4u), zeroes);
	buf_flag.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(vec4u), zeroes);
	buf_sorted.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), zeroes);

	// To measure time
	glFinish();
	GLuint64 start, stop;
	GLuint queryID[2];
	glGenQueries(2, queryID);
	glQueryCounter(queryID[0], GL_TIMESTAMP);

	for (int i = 0; i < 8; i++)
	{
		scan_block(gfx, ctx, i * 2, buf_key.getHandle(), buf_scan.getHandle(), buf_sums.getHandle(), buf_flag.getHandle());
		scan_sums(gfx, ctx, buf_scan.getHandle(), buf_sums.getHandle());
		apply_sums(gfx, ctx, i * 2, buf_key.getHandle(), buf_scan.getHandle(), buf_sums.getHandle());
		reorder(gfx, ctx, buf_key.getHandle(), buf_scan.getHandle(), buf_flag.getHandle(), buf_sorted.getHandle());

		if (i < 8 - 1)
			buf_key.swap(buf_sorted);
	}

	glQueryCounter(queryID[1], GL_TIMESTAMP);
	GLint timeAvail = 0;
	while (!timeAvail)
		glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &timeAvail);

	glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &start);
	glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stop);
	printf("Time spent on the GPU: %f ms\n", (stop - start) / 1000000.0);

	printf("sorted: ");
	print_data(buf_sorted.getHandle(), GL_SHADER_STORAGE_BUFFER, num_keys);
}

void update(Renderer &gfx, Context &ctx, double dt)
{


}

void render(Renderer &gfx, Context &ctx, double dt)
{

}