#version 430

in vec3 vNormal;
in vec3 vPosition;

out vec4 outColor;

uniform vec3 color;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform mat4 projectionLight;
uniform mat4 viewLight;
uniform sampler2D shadowmap;

void main()
{
	vec3 N = normalize(vNormal);
	outColor.rgb = N * 0.5 + vec3(0.5);
	outColor.a = 1.0;

	// Shadow from particles
	vec4 posFromLight = projectionLight * viewLight * vec4(vPosition, 1.0);
	vec2 shadowTexel = posFromLight.xy * 0.5 + vec2(0.5);
	float shadow = texture(shadowmap, shadowTexel).a;
	outColor.rgb = mix(outColor.rgb, color * vec3(0.1, 0.12, 0.15), shadow);

	outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
}