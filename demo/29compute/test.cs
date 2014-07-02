#version 430

layout (local_size_x = 1) in;

layout (std140, binding = 0) buffer Buffer {
	vec4 data[];
};

void main()
{
	uint index = uint(int(gl_WorkGroupID.x) * 1 + int(gl_LocalInvocationID.x));
	data[index] = 2.0 * data[index];
}