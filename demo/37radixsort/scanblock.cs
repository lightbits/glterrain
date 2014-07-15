#version 430

layout (local_size_x = 128) in; // Must equal <block_size> in sort.cpp
layout (std430, binding = 0) buffer ScanBuffer {
	uvec4 Scan[];
};

layout (std430, binding = 1) buffer SumsBuffer {
	uvec4 Sums[];
};

layout (std430, binding = 2) buffer KeyBuffer {
	uint Key[];
};

layout (std430, binding = 3) buffer FlagBuffer {
	uvec4 Flag[];
};

shared uvec4 sharedData[gl_WorkGroupSize.x];

uniform int bitOffset;

/*
 * The main idea of the sorting algorithm is based on counting.
 * For example, say that we need to put the number 3 in sorted order,
 * and we know that there are in total 5 numbers less than 3 in the input.
 * Then we know that our number must come after all of these - that is, in position 5!
 * 
 * To determine such a value for each number in our input, we use a prefix sum
 * (also known as a scan).
 * 
 * It works like this, let's make this our input that we want to sort:
 *  1 3 2 0 1 0 2 2
 * 
 * (the actual input may have values greater than 3, but the scan only operates
 * on 2 bit values, because the radix sort works on 2-bit stages. We mask out
 * the interesting digit based on the <bitOffset> value.)
 * 
 * We then construct four flag arrays, one for each possible digit,
 * that has a <1> where the key matches the digit, and 0 elsewhere:
 *  0 0 0 1 0 1 0 0 <- 0
 *  1 0 0 0 1 0 0 0 <- 1
 *  0 0 1 0 0 0 1 1 <- 2
 *  0 1 0 0 0 0 0 0 <- 3
 *  
 * If we do an exclusive prefix sum over these arrays (carrying over the sum
 * from each array to the next) we get:
 *  0 0 0 0 1 1 2 2 <- 0
 *  2 3 3 3 3 4 4 4 <- 1
 *  4 4 4 5 5 5 5 6 <- 2
 *  7 7 8 8 8 8 8 8 <- 3 (note that 7 was carried over instead of 6)
 *  
 * Now we have all we need!
 * We then go through each element in the input, and look at this table to find
 * out where the element should go in the sorted output.
 * 
 * For example, the first 0 is located in the fourth column (as marked by the flag).
 * The scan array that corresponds to 0 contains the number 0 at this columns.
 * Thus, the first 0 should go to location 0.
 * 
 * Not too bad!
 * What about the first 1?
 * 
 * It is masked in the first column, second row.
 * We then look at the second row to determine its offset.
 * The scan value there is <2>. So the first 1 should go to index 2 in the output.
*/

/*
 * For efficiency, we perform the prefix sum in blocks.
 * For example, with blocks of 4-by-4 elements, the prefix sum of
 *  0 0 0 1 0 1 0 0
 * is done by first scanning each block individually
 *  0 0 0 0 | 0 0 1 1
 *  
 * To merge these two we need to add the sum of elements in the first block,
 * to each element in the second block. With more than two blocks,
 * we need to accumulate the sum we need to add, as we go along (also a prefix sum)
 * 
 * So we store the sum (which is the last element of the inclusive prefix sum of the block):
 *  1 | 1
 * These sums are stored in the <SUMS> array, and are applied at a later stage.
 *  
 * Taking the prefix sum of this array gives us:
 *  0 | 1
 *  
 * So 0 should be added to each element in the first block,
 * and 1 should be added to each element in the second.
 * 
 * Because we need to perform four prefix sums (as we use 2-bit radices),
 * the sums array is a uvec4 array. The xyzw components correspond to the digit 0, 1, 2 and 3
 * scan arrays, respectively.
*/
void main()
{
    const uint global_i = gl_GlobalInvocationID.x;
    const uint block_i = gl_WorkGroupID.x;
    const uint block_size = gl_WorkGroupSize.x;
    const uint local_i = gl_LocalInvocationID.x;
    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;
    const uint radix = 4;

    uint key = Key[global_i];
    
    // Mask out the current digit
    key = (key >> bitOffset) & (radix - 1);

    // Interleave the flag bits
    uvec4 flag = uvec4(key == 0 ? 1 : 0, key == 1 ? 1 : 0, key == 2 ? 1 : 0, key == 3 ? 1 : 0);
    sharedData[local_i] = flag;
    barrier(); // Wait for other threads within the block to have done the same

    Flag[global_i] = flag; // Store this for later when shuffling the elements
    
    // The prefix sum routine
    for (uint step = 0; step < steps; step++)
    {
        uint rd_id = (1 << step) * (local_i >> step) - 1;
        uint mask = (local_i & (1 << step)) >> step;
        sharedData[local_i] += sharedData[rd_id] * mask;
        barrier();
    }

    // Subtract initial value to get exclusive prefix sum
    Scan[global_i] = sharedData[local_i] - flag;
    barrier();

    // Store the sum of all elements in the current block
    // (which is equal to the last element in the inclusive prefix sum)
    Sums[block_i] = sharedData[block_size - 1];
}