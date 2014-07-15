#include "sort.h"
#include <gl/shaderprogram.h>

ShaderProgram
	shader_scanblock,
	shader_scansums,
	shader_applysums,
	shader_reorder;

BufferObject
	buf_scan,
	buf_sums,
	buf_flag,
	buf_sorted;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool sort_init()
{
	if (!loadComputeShader(shader_scanblock,	"./demo/37radixsort/scanblock.cs") ||
		!loadComputeShader(shader_scansums,		"./demo/37radixsort/scansums.cs") ||
		!loadComputeShader(shader_applysums,	"./demo/37radixsort/applysums.cs") ||
		!loadComputeShader(shader_reorder,		"./demo/37radixsort/reorder.cs"))
		return false;

	if (!shader_scanblock.linkAndCheckStatus() ||
		!shader_scansums.linkAndCheckStatus() ||
		!shader_applysums.linkAndCheckStatus() ||
		!shader_reorder.linkAndCheckStatus())
		return false;

	uint32 *zeroes = new uint32[4 * num_keys];
	memset(zeroes, 0, 4 * num_keys * sizeof(uint32));

	buf_flag.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(vec4u), zeroes);
	buf_scan.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(vec4u), zeroes);
	buf_sums.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_blocks * sizeof(vec4u), zeroes);
	buf_sorted.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(vec4), zeroes);

	return true;
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

void scan_block(int bit_offset, vec3 axis, float z_min, float z_max, GLuint input, GLuint scan, GLuint sums, GLuint flag)
{
	//printf("keys: ");
	//print_data(keys, GL_SHADER_STORAGE_BUFFER, num_keys);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sums);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, flag);
	shader_scanblock.begin();
	shader_scanblock.setUniform("bitOffset", bit_offset);
	shader_scanblock.setUniform("axis", axis);
	shader_scanblock.setUniform("zMin", z_min);
	shader_scanblock.setUniform("zMax", z_max);
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

void scan_sums(GLuint scan, GLuint sums)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sums);
	shader_scansums.begin();
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	//printf("scan sums0: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 0, 4);
	//printf("scan sums1: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 1, 4);
	//printf("scan sums2: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 2, 4);
	//printf("scan sums3: "); print_data(sums, GL_SHADER_STORAGE_BUFFER, num_blocks * 4, 3, 4);
}

void apply_sums(int bit_offset, vec3 axis, float z_min, float z_max, GLuint input, GLuint scan, GLuint sums)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sums);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input);
	shader_applysums.begin();
	shader_applysums.setUniform("bitOffset", bit_offset);
	shader_applysums.setUniform("axis", axis);
	shader_applysums.setUniform("zMin", z_min);
	shader_applysums.setUniform("zMax", z_max);
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

void reorder(GLuint input, GLuint scan, GLuint flag, GLuint sorted)
{
	const uint32 local_size_x = 128;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, flag);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, sorted);
	shader_reorder.begin();
	glDispatchCompute(num_keys / local_size_x, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);

	//printf("sorted: ");
	//print_data(sorted, GL_SHADER_STORAGE_BUFFER, num_keys);
}

void radix_sort(BufferObject input, vec3 axis, float z_min, float z_max)
{
	//print_data(keys, GL_SHADER_STORAGE_BUFFER, num_keys);
	
	// To measure time
	glFinish();
	GLuint64 start, stop;
	GLuint queryID[2];
	glGenQueries(2, queryID);
	glQueryCounter(queryID[0], GL_TIMESTAMP);

	for (int i = 0; i < 8; i++)
	{
		scan_block(i * 2, axis, z_min, z_max, input.getHandle(), buf_scan.getHandle(), buf_sums.getHandle(), buf_flag.getHandle());
		scan_sums(buf_scan.getHandle(), buf_sums.getHandle());
		apply_sums(i * 2, axis, z_min, z_max, input.getHandle(), buf_scan.getHandle(), buf_sums.getHandle());
		reorder(input.getHandle(), buf_scan.getHandle(), buf_flag.getHandle(), buf_sorted.getHandle());

		// swap for the next digit stage
		// the <input> buffer will in the end hold the latest sorted data
		input.swap(buf_sorted);
	}

	glQueryCounter(queryID[1], GL_TIMESTAMP);
	GLint timeAvail = 0;
	while (!timeAvail)
		glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &timeAvail);

	glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &start);
	glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stop);
	printf("Time spent on the GPU: %f ms\t\r", (stop - start) / 1000000.0);

	//print_data(sorted, GL_SHADER_STORAGE_BUFFER, 1024);
}