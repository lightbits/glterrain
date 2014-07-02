#version 430

in vec3 position;
in vec4 particlePosition;

out float lifetime;
out float distance;

uniform mat4 projection;
uniform mat4 view;

float sizes[] = {
	2.0,
	3.0,
	5.0,
	2.5,
	4.0
};

void main()
{
	vec4 viewPos = view * vec4(particlePosition.xyz + 0.05 * position, 1.0);
	distance = length(viewPos.xyz);
	lifetime = particlePosition.w;
	gl_Position = projection * viewPos;
	float scale = 4.0 - 0.5 * (distance - 0.1) / (5.0 - 0.1);

	// Constant size pretty much eliminates flickering due to sorting
	// gl_PointSize = 8.0;

	// Looks sort of better, but some flickering
	gl_PointSize = 12.0 - 4.0 * (distance - 1.0) / (3.0 - 1.0);

	// This looks really good static, but causes alot of flickering when being sorted
	// float scale = 5.5 - 4.5 * (distance - 0.1) / (5.0 - 0.1);
	// gl_PointSize = scale * sizes[gl_InstanceID % 5];
}