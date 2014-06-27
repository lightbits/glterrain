#version 430

layout (local_size_x = 1) in;

layout (std140, binding = 0) buffer PositionBuffer {
	vec4 position[];
};

uniform int offset;
uniform float zMin;
uniform float zMax;

void main()
{
	uint index = gl_GlobalInvocationID.x * 2 + offset;
    vec4 curr = position[index];
    vec4 prev = position[index - 1];

    int currKey = int(65535 * (curr.z - zMin) / (zMax - zMin));
    int prevKey = int(65535 * (prev.z - zMin) / (zMax - zMin));
    if (currKey < prevKey)
    {
        position[index - 1] = curr;
        position[index] = prev;
    }

    position[index - 1] = prev;
    position[index] = curr;
}