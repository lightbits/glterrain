#version 430

layout (local_size_x = 64) in; // Must equal <num_blocks> in app.cpp
layout (std430, binding = 0) buffer ScanBuffer {
	uvec4 Scan[];
};

layout (std430, binding = 1) buffer SumsBuffer {
	uvec4 Sums[];
};

// The entire SUMS array must fit inside the shared data storage
// If we set a block size of 256, we get N / 256 elements in the SUMS array.
// So if N is 65536, we get another 256 elements in the SUMS array, which we
// can perform a prefix sum on.
shared uvec4 sharedData[gl_WorkGroupSize.x];

void main()
{
    const uint global_i = gl_GlobalInvocationID.x;
    const uint local_i = gl_LocalInvocationID.x;
    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;

    uvec4 key = Sums[global_i];
    sharedData[local_i] = key;
    barrier();

    for (uint step = 0; step < steps; step++)
    {
        uint rd_id = (1 << step) * (local_i >> step) - 1;
        uint mask = (local_i & (1 << step)) >> step;
        sharedData[local_i] += sharedData[rd_id] * mask;
        barrier();
    }

    // Subtract key to get exclusive prefix sum
    Sums[global_i] = sharedData[local_i] - key;
}