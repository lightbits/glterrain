#version 430

layout (local_size_x = 16) in;

layout (std140, binding = 0) buffer DataBuffer {
	vec4 Data[];
};

layout (std430, binding = 1) buffer IndexBuffer {
	uint Index[];
};

layout (std430, binding = 2) buffer KeyBuffer {
	float Key[];
};

uniform mat4 view;

void main()
{
	uint index = gl_GlobalInvocationID.x;
	vec3 cameraZ = vec3(0.0, 0.0, 1.0);
	Key[index] = dot((view * Data[index]).xyz, cameraZ);
	Index[index] = index;
}