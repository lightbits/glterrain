#version 430

in vec4 position;

out float lifetime;
out float distance;
out vec2 shadowTexel;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 projectionLight;
uniform mat4 viewLight;

void main()
{
	vec4 viewPos = view * vec4(position.xyz, 1.0);
	distance = length(viewPos.xyz);
	lifetime = position.w;
	gl_Position = projection * viewPos;
	float scale = 4.0 - 0.5 * (distance - 0.1) / (5.0 - 0.1);

	// Constant size pretty much eliminates flickering due to sorting
	// gl_PointSize = 8.0;

	// Project the particle position onto the corresponding location in the shadowmap
	vec4 posFromLight = projectionLight * viewLight * vec4(position.xyz, 1.0);
	posFromLight *= 1.0 / posFromLight.w;
	shadowTexel = posFromLight.xy * 0.5 + vec2(0.5);

	// Looks sort of better with almost no flickering
	gl_PointSize = 16.0 - 6.0 * (distance - 1.0) / (3.0 - 1.0);
	gl_PointSize = 10.0;
}