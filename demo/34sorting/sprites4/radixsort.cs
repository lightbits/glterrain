#version 430

layout (local_size_x = 16) in;
layout (binding = 0) buffer InputBuffer {
	int data[];
} inputBuffer;

//shared int sharedData[gl_WorkGroupSize.x];

void main()
{
	uint index = gl_GlobalInvocationID.x;
    
    inputBuffer.data[index] = int(index);
}