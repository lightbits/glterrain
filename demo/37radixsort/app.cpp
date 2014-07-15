#include "app.h"
#include "sort.h"
#include <common/noise.h>

BufferObject buf_input;

bool load()
{
	if (!sort_init())
		return false;
	return true;
}

void free()
{
	
}

void init(Renderer &gfx, Context &ctx)
{
	vec4 *input = new vec4[num_keys];
	for (int i = 0; i < num_keys; ++i)
	{
		float x = -1.0f + 2.0f * frand();
		float y = -1.0f + 2.0f * frand();
		float z = -1.0f + 2.0f * frand();
		input[i] = vec4(x, y, z, 1.0f);
	}

	buf_input.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(vec4), input);

	radix_sort(buf_input, vec3(0.0f, 0.0f, 1.0f), -1.0f, 1.0f);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf_input.getHandle());
	vec4 *data = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num_keys * sizeof(vec4), GL_MAP_READ_BIT);
	for (int i = 0; i < num_keys; i++)
		printf("%.2f\t", data[i].z);
	printf("\n");
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	radix_sort(buf_input, vec3(0.0f, 0.0f, 1.0f), -1.0f, 1.0f);	
}

void render(Renderer &gfx, Context &ctx, double dt)
{

}