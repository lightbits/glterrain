#version 430

layout (local_size_x = 8) in;

layout (binding = 0, offset = 0) uniform atomic_uint counter[16];

layout (std430, binding = 1) buffer PrefixSumBuffer {
	uint PrefixSum[];
};

layout (std430, binding = 2) buffer InputBuffer {
	uint Input[];
};

layout (std430, binding = 3) buffer OutputBuffer {
	uint Output[];
};

uniform int bit;

void main()
{
    const uint index = gl_GlobalInvocationID.x;
    uint input = Input[index];
    uint key = (input >> bit) & 0xf;
    uint offset = PrefixSum[key];
    uint count = atomicCounterDecrement(counter[key]);
    Output[offset + count] = input;
}