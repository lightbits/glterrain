#version 430

/*
 * Finally, we shuffle the elements to their correct positions.
 * The new position is given by the prefix
 * sum value in the element's scan array.
 * 
 * For example, if our keys (for the current stage) are as following:
 *  1 3 2 0 1 0 2 2
 *  
 * We get these digit scan arrays
 *  0 0 0 0 1 1 2 2 <- 0
 *  2 3 3 3 3 4 4 4 <- 1
 *  4 4 4 5 5 5 5 6 <- 2
 *  7 7 8 8 8 8 8 8 <- 3
 *  
 * And these flags
 *  0 0 0 1 0 1 0 0 <- 0
 *  1 0 0 0 1 0 0 0 <- 1
 *  0 0 1 0 0 0 1 1 <- 2
 *  0 1 0 0 0 0 0 0 <- 3
 *  
 * (Note that the columns of these are stored as uvec4s in the buffers)
 * 
 * In column 1 we have a flag in the second row, because the key is (1) in the first position.
 * Looking at the scan array belonging to (1) we find that the offset is 2.
 * So the first element should be reordered to position 2 in the sorted array.
*/

layout (local_size_x = 128) in; // Must equal <local_size_x> in reorder(...) func
layout (std430, binding = 0) buffer ScanBuffer {
	uvec4 Scan[];
};

layout (std430, binding = 2) buffer InputBuffer {
	vec4 Input[];
};

layout (std430, binding = 3) buffer FlagBuffer {
	uvec4 Flag[];
};

layout (std430, binding = 4) buffer SortedInputBuffer {
	vec4 SortedInput[];
};

void main()
{
    uint global_i = gl_GlobalInvocationID.x;
    vec4 position = Input[global_i];
    uvec4 scan = Scan[global_i]; // Serves as the offset
    uvec4 flag = Flag[global_i]; // Used to decide which element to reorder
    uint offset = uint(dot(scan, flag));
    SortedInput[offset] = position;
}