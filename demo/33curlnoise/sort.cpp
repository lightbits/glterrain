#include "sort.h"

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

BufferObject gen_swapindex_buffer(int n)
{
	// Generate the indices
	int num_stages = glm::round(glm::log2((float)n));
	int num_passes = num_stages * (num_stages + 1) / 2;
	int num_indices = num_passes * n / 2;
	vector< vector<vec2i> > indices(num_passes);
	mergesort(0, n, num_stages - 1, indices);

	// Create a buffer containing the index-pairs
	// Note that we use vec4i instead of vec2i. This is because we need
	// to pad the elements in order to use std140 layout.
	BufferObject buffer;
	buffer.create(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW, num_indices * sizeof(vec4i), NULL);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.getHandle());
	vec4i *pair = (vec4i*)glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER, 0,
		num_indices * sizeof(vec4i),
		GL_MAP_WRITE_BIT |
		GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < indices.size(); ++i)
	{
		for (int j = 0; j < indices[i].size(); ++j)
			*(pair++) = vec4i(indices[i][j], 0, 0);
		for (int j = indices[i].size(); j < n / 2; ++j)
			*(pair++) = vec4i(0, 0, 0, 0); // Padding
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	
	return buffer;
}