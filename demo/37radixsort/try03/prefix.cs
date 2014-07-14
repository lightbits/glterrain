#version 430

layout (local_size_x = 4) in;
layout (std430, binding = 0) buffer KeyBuffer {
	uint Key[];
};

layout (std430, binding = 1) buffer PrefixSumBuffer {
	uint PrefixSum[];
};

shared uint sharedData[gl_WorkGroupSize.x];

void main()
{
    const uint global_i = gl_GlobalInvocationID.x;
    const uint local_i = gl_LocalInvocationID.x;
    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;

    uint key = Key[global_i];
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
    PrefixSum[global_i] = sharedData[local_i] - key;
}