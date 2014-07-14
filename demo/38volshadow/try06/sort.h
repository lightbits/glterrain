#ifndef SORT_H
#define SORT_H
#include <common/matrix.h>
#include <common/typedefs.h>
#include <gl/bufferobject.h>

/*
Generates a buffer of index-pairs to be compared for sorting
<n> elements. Pairs within the same pass can be processed in parallell.

The buffer contains <num_passes> sets of <n> / 2 index-pairs, where
num_passes = log2(<n>) * (log2(<n>) + 1) / 2. This is more data than 
necessary, but for simplicity later we pad each set that has fewer than 
<n> / 2 pairs with (0, 0).
*/
BufferObject gen_swapindex_buffer(int n);

#endif