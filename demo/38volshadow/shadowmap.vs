#version 430

in vec4 position;

out vec4 mask0;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	vec4 viewPos = view * position;
	gl_Position = projection * viewPos;
	gl_PointSize = 20.0;

	// float z = (viewPos.z + 1.0) / (-3.0 + 1.0);
	float z = 0.5 + 0.5 * gl_Position.z;

	// GPUs don't like branches!
	mask0 = clamp(floor( mod(vec4(z) + vec4(1.00, 0.75, 0.50, 0.25), vec4(1.25)) ), vec4(0.0), vec4(1.0));
}
