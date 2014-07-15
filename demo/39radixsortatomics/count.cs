#version 430

layout (local_size_x = 8) in;

layout (std430, binding = 0) buffer KeyBuffer {
	uint Key[];
};

layout (binding = 1, offset = 0) uniform atomic_uint counter[16];

uniform int bit;

void main()
{
	uint index = gl_GlobalInvocationID.x;
    uint key = (Key[index] >> bit) & 0xf;
    atomicCounterIncrement(counter[key]);    
}