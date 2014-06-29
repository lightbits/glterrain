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
	int index = int(gl_GlobalInvocationID.x * 2 + offset);
    vec4 curr = position[index];
    vec4 prev = position[max(index - 1, 0)];

    int currKey = int(65535 * (curr.z - zMin) / (zMax - zMin));
    int prevKey = int(65535 * (prev.z - zMin) / (zMax - zMin));
    if (currKey < prevKey)
    {
        position[max(index - 1, 0)] = curr;
        position[index] = prev;
    }
}