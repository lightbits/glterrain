#version 430

in vec3 position;
in vec4 particlePosition;

out vec3 vColor;
out vec3 vPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	vPosition = (view * model * vec4(particlePosition.xyz + 0.05 * position, 1.0)).xyz;
	gl_Position = projection * vec4(vPosition, 1.0);
	vPosition.z = gl_Position.z;
}