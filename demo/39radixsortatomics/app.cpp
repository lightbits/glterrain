#include "app.h"

ShaderProgram 
	shader_prefix,
	shader_count,
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
		!loadComputeShader(shader_count, "./demo/37radixsort/count.cs") ||
		!loadComputeShader(shader_reorder, "./demo/37radixsort/reorder.cs"))
		return false;

	if (!shader_prefix.linkAndCheckStatus() ||
		!shader_count.linkAndCheckStatus() ||
		!shader_reorder.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{

}

const uint32 num_keys = 1 << 6;
const uint32 radix_bits = 4;
const uint32 num_counters = 1 << radix_bits;
const uint32 num_bits = 8;
const uint32 num_passes = num_bits / radix_bits;
void histogram(Renderer &gfx, Context &ctx, int bit, int input_size, GLuint input, GLuint output)
{
	const int work_group_size = 8;
	gfx.beginCustomShader(shader_count);
	gfx.setUniform("bit", bit);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, output);
	glDispatchCompute(input_size / work_group_size, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, 0);

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, output);
	uint32 *temp = (uint32*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, num_counters * sizeof(uint32), GL_MAP_READ_BIT);
	std::cout << "counters: ";
	for (int i = 0; i < num_counters; ++i)
		std::cout << temp[i] << " ";
	std::cout << "\n";
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

// Produces a prefix-sum table of the input array
void scan(Renderer &gfx, Context &ctx, GLuint input, GLuint output)
{
	//const int work_group_size = input_size
	gfx.beginCustomShader(shader_prefix);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, output);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, output);
	uint32 *temp = (uint32*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num_counters * sizeof(uint32), GL_MAP_READ_BIT);
	std::cout << "counters: ";
	for (int i = 0; i < num_counters; ++i)
		std::cout << temp[i] << " ";
	std::cout << "\n";
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void reorder(Renderer &gfx, Context &ctx, int bit, int input_size, GLuint count, GLuint prefix, GLuint input, GLuint output)
{
	const int work_group_size = 8;
	gfx.beginCustomShader(shader_reorder);
	gfx.setUniform("bit", bit);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prefix);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, output);
	glDispatchCompute(input_size / work_group_size, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
}

void init(Renderer &gfx, Context &ctx)
{
	BufferObject input_table, count_table, prefix_table, sorted_table;
	uint32 *keys = new uint32[num_keys];
	for (int i = 0; i < num_keys; ++i)
	{
		keys[i] = rand() % 256;
		printf("%x ", keys[i]);
	}
	printf("\n");

	uint32 zeroes[num_counters];
	memset(zeroes, 0, num_counters * sizeof(uint32));

	input_table.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), keys);
	count_table.create(GL_ATOMIC_COUNTER_BUFFER, GL_DYNAMIC_DRAW, num_counters * sizeof(uint32), zeroes);
	prefix_table.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_counters * sizeof(uint32), NULL);
	sorted_table.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), keys);

	// To measure time
	glFinish();
	GLuint64 start, stop;
	GLuint queryID[2];
	glGenQueries(2, queryID);
	glQueryCounter(queryID[0], GL_TIMESTAMP);

	for (int i = 0; i < num_passes; ++i)
	{
		histogram(
			gfx, ctx,
			i * radix_bits,
			num_keys,
			input_table.getHandle(),
			count_table.getHandle());

		scan(
			gfx, ctx,
			count_table.getHandle(),
			prefix_table.getHandle());

		reorder(
			gfx, ctx,
			i * radix_bits,
			num_keys,
			count_table.getHandle(),
			prefix_table.getHandle(),
			input_table.getHandle(),
			sorted_table.getHandle());
	}

	glQueryCounter(queryID[1], GL_TIMESTAMP);
	GLint timeAvail = 0;
	while (!timeAvail)
		glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &timeAvail);
	
	glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &start);
	glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stop);
	printf("Time spent on the GPU: %f ms\n", (stop - start) / 1000000.0);
	delete[] keys;

	sorted_table.bind(GL_SHADER_STORAGE_BUFFER);
	uint32 *result = (uint32*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num_keys * sizeof(uint32), GL_MAP_READ_BIT);
	for (int i = 0; i < num_keys; ++i)
		printf("%x ", result[i]);
	std::cout << '\n';
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	sorted_table.unbind();
}

void update(Renderer &gfx, Context &ctx, double dt)
{


}

void render(Renderer &gfx, Context &ctx, double dt)
{

}