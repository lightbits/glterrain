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
	float z = 0.5 + 0.5 * gl_Position.z / gl_Position.w;
	mask0 = vec4(0.0);
	if (z >= 0.75) mask0.w = 1.0;
	else if (z >= 0.50) mask0.z = 1.0;
	else if (z >= 0.25) mask0.y = 1.0;
	else if (z >= 0.00) mask0.x = 1.0;
}
