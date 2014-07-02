#version 430

layout (local_size_x = 64) in;

layout (std140, binding = 0) buffer PositionBuffer {
	vec4 Position[];
};

layout (std140, binding = 1) buffer ComparisonIndexBuffer {
	ivec4 IndicesToCompare[];
};

uniform int offset;
uniform mat4 view;

/*
 * Compares the two coordinates by their distance along the back-to-front view-vector.
 * Returns true if lhs is closer to the camera than rhs.
*/
bool greater(vec4 lhs, vec4 rhs)
{
    vec3 cameraZ = vec3(0.0, 0.0, 1.0);
    float lhsZ = dot((view * lhs).xyz, cameraZ);
    float rhsZ = dot((view * rhs).xyz, cameraZ);
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