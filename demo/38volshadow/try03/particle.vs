#version 430

in vec4 position;

out vec3 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 projectionLight;
uniform mat4 viewLight;
uniform sampler2DArray shadowMap;
uniform int layer;

void main()
{
	vec4 viewPos = view * vec4(position.xyz, 1.0);
	float distance = length(viewPos.xyz);
	gl_Position = projection * viewPos;
	gl_PointSize = 16.0 - 6.0 * (distance - 1.0) / (3.0 - 1.0);

	vec4 lvp = viewLight * position;
	vec4 lpvp = projectionLight * lvp;
	vec3 ltexel = vec3(0.5) + 0.5 * lpvp.xyz / lpvp.w;
	float shadow = 0.0;
	for (int i = 0; i < 32; i++)
	{
		float a = i < layer ? 1.0 : 0.0;
		shadow += texture(shadowMap, vec3(ltexel.xy, float(i))).r * a;
	}
	shadow = clamp(shadow, 0.0, 1.0);

	color = vec3(0.88, 0.78, 0.75);
	// color = vec3(0.5) + 0.5 * position.xyz;
	color = color * (1.0 - shadow) + vec3(0.1, 0.12, 0.18) * shadow;
}
