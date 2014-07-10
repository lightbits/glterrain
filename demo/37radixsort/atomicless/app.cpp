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

void init(Renderer &gfx, Context &ctx)
{
	BufferObject keys_buffer, prefix_buffer, sorted_buffer;

	uint32 zeroes[4];
	memset(zeroes, 0, 4 * sizeof(uint32));

	uint32 keys[4] = { 0, 1, 1, 0 };

	keys_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 4 * sizeof(uint32), keys);
	sorted_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 4 * sizeof(uint32), zeroes);
	prefix_buffer.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 4 * sizeof(uint32), zeroes);

	gfx.beginCustomShader(shader_prefix);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, keys_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prefix_buffer.getHandle());
	for (int radix = 0; radix < 2; ++radix)
	{
		gfx.setUniform("radix", radix);
		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	prefix_buffer.bind(GL_SHADER_STORAGE_BUFFER);
	uint32 *result = (uint32*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(uint32), GL_MAP_READ_BIT);
	for (int i = 0; i < 4; ++i)
		std::cout << result[i] << " ";
	std::cout << "\n\n";
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	prefix_buffer.unbind();

	// reorder
	gfx.beginCustomShader(shader_reorder);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, keys_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prefix_buffer.getHandle());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sorted_buffer.getHandle());
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

	sorted_buffer.bind(GL_SHADER_STORAGE_BUFFER);
	result = (uint32*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(uint32), GL_MAP_READ_BIT);
	for (int i = 0; i < 4; ++i)
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