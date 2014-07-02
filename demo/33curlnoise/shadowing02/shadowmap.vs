#version 430

in vec4 position;
out float distance;

uniform mat4 projectionLight;
uniform mat4 viewLight;

void main()
{
	vec4 viewPos = viewLight * vec4(position.xyz, 1.0);
	distance = length(viewPos.xyz);
	gl_Position = projectionLight * viewPos;
	gl_Position.z = 0.0;
	gl_PointSize = 14.0 - 6.0 * (distance - 1.0) / (3.0 - 1.0);
}