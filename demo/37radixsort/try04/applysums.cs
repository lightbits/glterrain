#version 430

layout (local_size_x = 128) in; // Must equal block size
layout (std430, binding = 0) buffer ScanBlockBuffer {
	uint ScanBlock[];
};

layout (std430, binding = 1) buffer SumsBuffer {
	uint Sums[];
};

shared uint sum;

void main()
{
    const uint global_i = gl_GlobalInvocationID.x;
    const uint local_i = gl_LocalInvocationID.x;
    const uint block_i = gl_WorkGroupID.x;

    sum = Sums[block_i];
    ScanBlock[global_i] += sum;
}