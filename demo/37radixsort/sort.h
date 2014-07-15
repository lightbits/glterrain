#ifndef SORT_H
#define SORT_H
#include <common/typedefs.h>
#include <gl/opengl.h>
const uint32 block_size = 128;
const uint32 num_keys = 1 << 14;
const uint32 num_blocks = num_keys / block_size;

// Loads the sorting shaders and initializes buffers.
bool sort_init();

// Generates a 16-bit integer key buffer from the input values.
void gen_keys(GLuint input, GLuint keys);

// Performs a 2-bit radix sort on 16-bit keys.
// The output is stored in the <sorted> buffer.
void radix_sort(GLuint keys, GLuint sorted);

#endif