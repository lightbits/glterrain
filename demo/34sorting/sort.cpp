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

vector< vector<vec2i> > sort_generate_indices(int n)
{
	int num_stages = int(glm::log2((float)n));
	int num_passes = num_stages * (num_stages + 1) / 2;
	vector< vector<vec2i> > indices(num_passes);
	mergesort(0, n, num_stages - 1, indices);
	return indices;
}