#version 430

layout (local_size_x = 1) in;

layout (std140, binding = 0) buffer DataBuffer {
	vec4 Data[];
};

layout (std140, binding = 1) buffer ComparisonIndexBuffer {
	vec4 ComparisonIndices[];
};

uniform int offset;

void main()
{
	ivec2 indices = ivec2(ComparisonIndices[gl_GlobalInvocationID.x + offset].xy);
	vec4 lhs = Data[indices.x];
	vec4 rhs = Data[indices.y];
	if (rhs.z < lhs.z)
	{
		Data[indices.x] = rhs;
		Data[indices.y] = lhs;
	}
}