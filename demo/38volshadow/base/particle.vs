#version 430

in vec4 position;

out vec3 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 projectionLight;
uniform mat4 viewLight;
uniform sampler2DArray shadowMap;
uniform float layer;

void main()
{
	vec4 viewPos = view * vec4(position.xyz, 1.0);
	float distance = length(viewPos.xyz);
	gl_Position = projection * viewPos;
	gl_PointSize = 20.0;

	vec4 lvp = viewLight * position;
	vec4 lpvp = projectionLight * lvp;
	vec3 shadowTexel;
	shadowTexel.xy = vec2(0.5) + 0.5 * lpvp.xy / lpvp.w;
	shadowTexel.z = layer;
	float shadow = texture(shadowMap, shadowTexel).r;

	color = vec3(0.88, 0.78, 0.75) * (1.0 - shadow) + vec3(0.1, 0.12, 0.18) * shadow;
	color *= 0.001;
	color += vec3(0.5) + 0.5 * vec3(lpvp.z / lpvp.w);
	// color = vec3(0.5) + 0.5 * position.xyz;
}
