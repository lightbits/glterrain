#version 140

in vec4 position;

out float shadow;

uniform mat4 lightMatrix;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform sampler3D shadowMap;
uniform float depth;

void main()
{
	vec4 viewPos = view * model * vec4(position.xyz, 1.0);
	gl_PointSize = 1.0 - length(viewPos) / 6.0;
	gl_PointSize *= 20.0;
	gl_Position = projection * viewPos;

	vec4 lightPos = lightMatrix * vec4(position.xyz, 1.0);
	vec2 shadowTexel = vec2(0.5) + 0.5 * lightPos.xy / lightPos.w;
	shadow = texture(shadowMap, vec3(shadowTexel, depth)).r;
}
