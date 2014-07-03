#version 430

layout (local_size_x = 64) in;

layout (std140, binding = 0) buffer PositionBuffer {
	vec4 Position[];
};

layout (std140, binding = 1) buffer ComparisonIndexBuffer {
	ivec4 IndicesToCompare[];
};

uniform int offset;

/*
 * The elements will be sorted by decreasing distance along this axis in world-space.
*/
uniform vec3 axis;

/*
 * Compares the two coordinates by their distance along the axis vector.
 * Returns true if lhs is further along the axis than rhs.
 * 
*/
bool greater(vec4 lhs, vec4 rhs)
{
    float lhsZ = dot(lhs.xyz, axis);
    float rhsZ = dot(rhs.xyz, axis);
    return lhsZ > rhsZ;
}

void main()
{
	ivec2 pair = ivec2(IndicesToCompare[gl_GlobalInvocationID.x + offset].xy);
	vec4 lhs = Position[pair.x];
	vec4 rhs = Position[pair.y];
	if (greater(lhs, rhs))
	{
		Position[pair.x] = rhs;
		Position[pair.y] = lhs;
	}
}