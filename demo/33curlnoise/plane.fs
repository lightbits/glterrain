#version 140

in vec3 vPosition;
in vec3 vNormal;
in float dist;

out vec4 outColor;

uniform vec3 color;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform mat4 projectionLight;
uniform mat4 viewLight;
uniform sampler2D shadowMap;

vec3 lighting(vec3 N, vec3 D, vec3 Ld, vec3 Lp, vec3 P)
{
	vec3 diff = Lp - P;
	float len = length(diff);
	vec3 L = diff / len;
	float NdotL = max(dot(N, L), 0.0);
	float Id = NdotL / (len * len);
	return Id * Ld * D + (1.0 - Id) * ambientColor;
}

void main()
{
	vec3 N = normalize(vNormal);
	outColor.rgb = lighting(N, color, lightColor, lightPos, vPosition);
	outColor.a = 1.0;
	outColor.rgb *= 0.00001;

	outColor.rgb += color;
	float r2 = dot(vPosition, vPosition);
	outColor.rgb *= exp(-r2 * 0.3 + 0.8);

	// shadow from particles
	vec4 posFromLight = projectionLight * viewLight * vec4(vPosition, 1.0);
	vec2 shadowTexel = posFromLight.xy * 0.5 + vec2(0.5);
	float shadow = texture(shadowMap, shadowTexel).a;
	outColor.rgb = mix(outColor.rgb, color * vec3(0.1, 0.12, 0.15), shadow);

	// gamma correction
	outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
}