#version 430

in vec3 position;
in vec4 particlePosition;
in vec4 particleStatus;

out float lifetime;
out float distance;

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
	vec4 viewPos = view * model * vec4(particlePosition.xyz + 0.05 * position, 1.0);
	distance = length(viewPos.xyz);
	lifetime = particleStatus.a;
	gl_Position = projection * viewPos;
	float scale = 5.5 - 4.5 * (distance - 0.1) / (5.0 - 0.1);
	gl_PointSize = scale * sizes[gl_InstanceID % 5];
}