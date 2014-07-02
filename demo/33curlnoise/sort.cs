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
 * The elements will be sorted by decreasing distance along this axis.
 * i.e., if axis = (0, 0, 1) the particles will be sorted back-to-front
 * relative to the view point (as in traditional back-to-front rendering).
 * If axis = (0, 0, -1), the particles will effectively be rendered
 * front-to-back relative the view point.
*/
uniform vec3 axis;

/*
 * A view transformation is applied to each particle position before
 * comparing their distances along the axis. If this is the camera's
 * view matrix, and the axis is (0, 0, 1), particle's will be rendered
 * back-to-front.
*/
uniform mat4 view;

/*
 * Compares the two coordinates by their distance along the axis vector.
 * Returns true if lhs is further along the axis than rhs.
 * 
*/
bool greater(vec4 lhs, vec4 rhs)
{
    float lhsZ = dot((view * lhs).xyz, axis);
    float rhsZ = dot((view * rhs).xyz, axis);
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