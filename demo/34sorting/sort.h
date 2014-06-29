#ifndef SORT_H
#define SORT_H
#include <common/matrix.h>
#include <common/typedefs.h>

/*
Generates an array of arrays. Each inner array contains a set of index-pairs
that can be used in a single parallell compare-swap pass.
*/
vector< vector<vec2i> > sort_generate_indices(int n);

#endif