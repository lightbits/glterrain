/*
http://www.codeproject.com/Articles/543451/Parallel-Radix-Sort-on-the-GPU-using-Cplusplus-AMP
http://http.developer.nvidia.com/GPUGems3/gpugems3_ch39.html
*/
#ifndef SORT_H
#define SORT_H
#include <common/typedefs.h>
#include <common/matrix.h>
#include <gl/opengl.h>
#include <gl/bufferobject.h>

const uint32 block_size = 128;
const uint32 num_keys = 1 << 14;
const uint32 num_blocks = num_keys / block_size;

// Loads the sorting shaders and initializes buffers.
bool sort_init();

// Performs a 2-bit radix sort on the input buffer.
// The input buffer consists of <num_keys> particle positions,
// the sorting key is the distance along the <axis>, converted
// to a 16-bit integer.
void radix_sort(BufferObject input, vec3 axis, float z_min, float z_max);

#endif