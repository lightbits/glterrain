#version 430

layout (local_size_x = 1) in;

layout (binding = 0) buffer KeyBuffer {
	ivec2 indexKey[];
};

uniform int offset;

void main()
{
	uint index = gl_GlobalInvocationID.x * 2 + offset;
    ivec2 curr = indexKey[index];
    ivec2 prev = indexKey[index - 1];
    if (curr.y < prev.y)
    {
        indexKey[index - 1] = curr;
        indexKey[index] = prev;
    }
}