#version 430

layout (local_size_x = 64) in;

layout (binding = 0) buffer IndexBuffer {
	uint Index[];
};

layout (binding = 1) buffer KeyBuffer {
	float Key[];
};

layout (std140, binding = 2) buffer ComparisonIndexBuffer {
	ivec4 IndicesToCompare[];
};

uniform int offset;

void main()
{
	// ivec2 pair = ivec2(IndicesToCompare[gl_GlobalInvocationID.x + offset].xy);
    ivec2 pair = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.x + offset);
	float lhs = Key[pair.x];
	float rhs = Key[pair.y];
	if (lhs > rhs)
	{
        Key[pair.x] = rhs;
        Key[pair.y] = lhs;
        uint ix = Index[pair.x];
        uint iy = Index[pair.y];
        Index[pair.x] = iy;
        Index[pair.y] = ix;
	}
}