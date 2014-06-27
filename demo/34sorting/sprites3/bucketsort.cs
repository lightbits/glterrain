#version 430

layout (local_size_x = 16) in;

layout (std140, binding = 0) buffer PositionBuffer {
	vec4 position[];
} positionBuffer;

layout (std140,  binding = 1) buffer SortedPositionBuffer {
	int index[];
} sortedPositionBuffer;

uniform int numZSlices;
uniform float zMax;
uniform float zMin;

void main()
{
	uint index = gl_GlobalInvocationID.x;
    vec3 pos = positionBuffer.position[index].xyz;
    

    inputBuffer.data[index] = int(index);
}