#version 430

layout (local_size_x = 8) in;
layout (std430, binding = 0) buffer KeyBuffer {
	uint Key[];
};

layout (std430, binding = 1) buffer PrefixSumBuffer {
	uint PrefixSum[];
};

layout (std430, binding = 2) buffer FlagBuffer {
	uint Flag[];
};

shared uint sharedData[gl_WorkGroupSize.x];
uniform int radix;

void main()
{
    const uint global_i = gl_GlobalInvocationID.x;
    const uint local_i = gl_LocalInvocationID.x;
    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;

    // Flag digits that match the radix as 1, essentially
    // counting the number of occurances of <radix> in the input
    uint flag = Key[global_i] == radix ? 1 : 0;
    Flag[global_i] = flag;

    // The last value of the previous prefix sum for the previous radix
    // is added to the next prefix sum, to get the correct offset later
    uint last_value = PrefixSum[gl_WorkGroupSize.x - 1];
    uint temp = Key[gl_WorkGroupSize.x - 1];
    last_value += temp == (radix - 1) ? 1 : 0; // Incase there was a flagged value at the end

    sharedData[local_i] = flag;
    barrier();

    for (uint step = 0; step < steps; step++)
    {
        uint rd_id = (1 << step) * (local_i >> step) - 1;
        uint mask = (local_i & (1 << step)) >> step;
        sharedData[local_i] += sharedData[rd_id] * mask;
        barrier();
    }

    // Subtract input_i to get non-inclusive prefix sum
    PrefixSum[global_i] = sharedData[local_i] - flag + last_value;
}