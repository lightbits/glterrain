#version 430

layout (local_size_x = 16) in;

layout (std430, binding = 0) buffer IndexBuffer {
	uint Index[];
};

layout (std430, binding = 1) buffer KeyBuffer {
	float Key[];
};

layout (std140, binding = 2) buffer ComparisonIndexBuffer {
	ivec4 IndicesToCompare[];
};

uniform int offset;

void main()
{
	ivec2 pair = ivec2(IndicesToCompare[gl_GlobalInvocationID.x + offset].xy);
	float lhsKey = Key[pair.x];
	float rhsKey = Key[pair.y];
	if (lhsKey > rhsKey)
	{
		Key[pair.x] = rhsKey;
		Key[pair.y] = lhsKey;
		uint lhsIndex = Index[pair.x];
		uint rhsIndex = Index[pair.y];
		Index[pair.x] = rhsIndex;
		Index[pair.y] = lhsIndex;
	}
}