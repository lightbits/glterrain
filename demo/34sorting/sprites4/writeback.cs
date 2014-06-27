#version 430

layout (local_size_x = 1) in;

layout (std140, binding = 0) buffer PositionBuffer {
	vec4 position[];
};

layout (binding = 1) buffer IndexKeyBuffer {
	ivec2 indexKey[];
};

layout (std140, binding = 2) buffer PositionSortedBuffer {
	vec4 positionSorted[];
};

void main()
{
    uint wbIndex = gl_GlobalInvocationID.x;
    uint rdIndex = indexKey[wbIndex].x;
    positionSorted[wbIndex] = position[rdIndex];
}