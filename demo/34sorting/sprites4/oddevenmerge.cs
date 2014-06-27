#version 430

layout (local_size_x = 1) in;

layout (binding = 0) buffer KeyBuffer {
	int key[];
};

uniform int offset;

void main()
{
	uint index = gl_GlobalInvocationID.x * 2 + offset;
    int curr = key[index];
    int prev = key[index - 1];
    if (curr < prev)
    {
        key[index - 1] = curr;
        key[index] = prev;
    }
}