#version 430

in vec3 position;
in vec4 particlePosition;
in vec4 particleStatus;

out float lifetime;
out vec3 vPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

float sizes[] = {
	2.0,
	3.0,
	5.0,
	2.5,
	4.0
};

void main()
{
	vPosition = (view * model * vec4(particlePosition.xyz + 0.05 * position, 1.0)).xyz;
	lifetime = particleStatus.a;
	gl_Position = projection * vec4(vPosition, 1.0);
	gl_PointSize = sizes[gl_InstanceID % 5];
	vPosition.z = gl_Position.z;
}