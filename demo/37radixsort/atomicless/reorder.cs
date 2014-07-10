#version 430

layout (local_size_x = 4) in;

layout (std430, binding = 0) buffer InputBuffer {
	uint Input[];
};

layout (std430, binding = 1) buffer PrefixSumBuffer {
	uint PrefixSum[];
};

layout (std430, binding = 2) buffer OutputBuffer {
	uint Output[];
};

uniform int offset;

void main()
{
    uint index = gl_GlobalInvocationID.x;
    uint offset = PrefixSum[index];
    Output[offset] = Input[index];
}