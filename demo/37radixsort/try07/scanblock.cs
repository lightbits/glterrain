#version 430

layout (local_size_x = 128) in; // Must equal <block_size> in app.cpp
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
    barrier(); // Wait for other threads to have done the same

    Flag[global_i] = flag; // Store this for later when shuffling the elements
    
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

    // Store the last + 1 element of the scan in this block
    Sums[block_i] = sharedData[block_size - 1];
}