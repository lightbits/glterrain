#version 430

layout (local_size_x = 16) in;
layout (std430, binding = 0) buffer InputArray {
	uint Input[];
};

layout (std430, binding = 1) buffer OutputArray {
	uint Output[];
};

shared uint sharedData[gl_WorkGroupSize.x];

void main()
{
    const uint global_i = gl_GlobalInvocationID.x;
    const uint local_i = gl_LocalInvocationID.x;
    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;

    const uint first = Input[global_i];
    sharedData[local_i] = first;
    barrier();

    for (uint step = 0; step < steps; step++)
    {
        uint rd_id = (1 << step) * (local_i >> step) - 1;
        uint mask = (local_i & (1 << step)) >> step;
        sharedData[local_i] += sharedData[rd_id] * mask;
        barrier();
    }

    Output[global_i] = sharedData[local_i] - first;
}