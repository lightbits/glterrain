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

void init(Renderer &gfx, Context &ctx)
{
	BufferObject keys_buffer, prefix_buffer, sorted_buffer, flag_buffer;

	const uint32 num_keys = 64;
	const uint32 num_bits = 2;
	const uint32 max_value = 1 << num_bits;

	uint32 *zeroes = new uint32[num_keys];
	memset(zeroes, 0, num_keys * sizeof(uint32));

	uint32 *keys = new uint32[num_keys];
	for (int i = 0; i < num_keys; ++i)
		keys[i] = rand() % 4;

	keys_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), keys);
	sorted_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), zeroes);
	prefix_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), zeroes);
	flag_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(uint32), zeroes);

	// To measure time
	glFinish();
	GLuint64 start, stop;
	GLuint queryID[2];
	glGenQueries(2, queryID);
	glQueryCounter(queryID[0], GL_TIMESTAMP);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, keys_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prefix_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, flag_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, sorted_buffer.getHandle());
	for (int radix = 0; radix < 4; ++radix)
	{
		gfx.beginCustomShader(shader_prefix);
		gfx.setUniform("radix", radix);
		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		gfx.beginCustomShader(shader_reorder);
		glDispatchCompute(num_keys / 8, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);

	glQueryCounter(queryID[1], GL_TIMESTAMP);
	GLint timeAvail = 0;
	while (!timeAvail) glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &timeAvail);
	glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &start);
	glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stop);
	printf("Time spent on the GPU: %f ms\n", (stop - start) / 1000000.0);

	prefix_buffer.bind(GL_SHADER_STORAGE_BUFFER);
	uint32 *result = (uint32*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num_keys * sizeof(uint32), GL_MAP_READ_BIT);
	for (int i = 0; i < num_keys; ++i)
		std::cout << result[i] << " ";
	std::cout << "\n\n";
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	prefix_buffer.unbind();

	sorted_buffer.bind(GL_SHADER_STORAGE_BUFFER);
	result = (uint32*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num_keys * sizeof(uint32), GL_MAP_READ_BIT);
	for (int i = 0; i < num_keys; ++i)
		std::cout << result[i] << " ";
	std::cout << "\n\n";
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	sorted_buffer.unbind();
}

void update(Renderer &gfx, Context &ctx, double dt)
{


}

void render(Renderer &gfx, Context &ctx, double dt)
{

}