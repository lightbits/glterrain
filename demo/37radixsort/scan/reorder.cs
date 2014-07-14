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


layout (std430, binding = 3) buffer OutputBuffer {
	uint Output[];
};

uniform int offset;

void main()
{
    uint index = gl_GlobalInvocationID.x;
    uint offset = PrefixSum[index];
    if (Flag[index] == 1)
        Output[offset] = Key[index];
}